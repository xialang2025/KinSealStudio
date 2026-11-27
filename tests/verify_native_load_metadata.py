"""Compare generated V122 load.bin with independent frozen compiler output without consuming it as input."""
import argparse
import collections
import concurrent.futures
import json
import os
from pathlib import Path
import subprocess
import struct
import tempfile
import zipfile
import zlib


PROJECT = Path(__file__).resolve().parents[1]
REVISION = "e3b72657"


def inventory(revision):
    listing = subprocess.check_output(["git", "ls-tree", "-rz", revision, "--", "docs"], cwd=PROJECT)
    directories = {}
    for entry in listing.split(b"\0"):
        if not entry:
            continue
        meta, path = entry.decode().split("\t", 1)
        folder, name = path.rsplit("/", 1)
        directories.setdefault(folder, {})[name] = meta.split()[-1]
    return {folder: parts for folder, parts in directories.items() if "load.bin" in parts}


def blob(ident):
    return subprocess.check_output(["git", "cat-file", "blob", ident], cwd=PROJECT)


def check_case(exe, temp, index, parts, read_part=blob):
    folder = temp / str(index)
    folder.mkdir()
    compiled = folder / "parts"
    compiled.mkdir()
    for name in ("SheetPara.dat", "ControlPara.dat", "Modbus_RTU_com1.bin"):
        if name in parts:
            (compiled / name).write_bytes(read_part(parts[name]))
    source = folder / "project.sm"
    if "input-native.sm" in parts:
        source.write_bytes(read_part(parts["input-native.sm"]))
    else:
        snapshot = read_part(parts["upLoad.knr"])
        declared = int.from_bytes(snapshot[:4], "big")
        if declared < 1 or declared > 64 * 1024 * 1024:
            return {"outcome": "unavailable-source", "message": "KNR source size is not supported"}
        decoder = zlib.decompressobj()
        source_bytes = decoder.decompress(snapshot[4:], declared + 1)
        if len(source_bytes) != declared or decoder.unused_data or not decoder.eof:
            return {"outcome": "unavailable-source", "message": "KNR cannot restore an exact SM"}
        source.write_bytes(source_bytes)
    # All the selected V122 reference projects omit extra file pairs and use compiler counter 0 at +0x838.
    # load.bin, Usbload.bin, load.kns and firmware are never provided to the writer.
    ctx = folder / "context.json"
    ctx.write_text(json.dumps({"compiledDirectory": str(compiled.resolve()), "assetPairs": []}), encoding="utf8")
    target = folder / "load.bin"
    proc = subprocess.run([str(exe), "--build-native-load-metadata", str(source), str(target), str(ctx)],
                          capture_output=True, env={**os.environ, "QT_QPA_PLATFORM": "offscreen"}, timeout=65,
                          creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    if proc.returncode:
        try:
            return {"outcome": "rejected", "message": json.loads(proc.stdout).get("message")}
        except ValueError:
            return {"outcome": "error", "message": proc.stderr.decode("utf8", errors="replace")}
    original = read_part(parts["load.bin"])
    generated = target.read_bytes()
    if generated == original:
        if not isinstance(index, int):
            # The four trailing 32-bit scalar/count slots precede page geometry and its final history count.
            sheets = compiled / "SheetPara.dat"
            descriptors = sheets.read_bytes()
            page_count = len(descriptors) // 72
            toggle_at = len(original) - (20 + 16 * page_count)
            if toggle_at < 0 or generated[toggle_at:toggle_at + 4] != bytes(4):
                raise ValueError("The vendor reference must use the default unchecked download option")
            args = [str(exe), "--build-native-load-metadata", str(source), str(target), str(ctx)]
            def run_again():
                return subprocess.run(args, capture_output=True, timeout=65,
                                      env={**os.environ, "QT_QPA_PLATFORM": "offscreen"},
                                      creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            fields = json.loads(ctx.read_text(encoding="utf8"))
            ctx.write_text(json.dumps({**fields, "downloadCharacterLibrary": True}), encoding="utf8")
            expected = generated[:toggle_at] + struct.pack(">i", 1) + generated[toggle_at + 4:]
            if run_again().returncode or target.read_bytes() != expected:
                raise ValueError("The native character-library option changed unrelated bytes")
            ctx.write_text(json.dumps(fields), encoding="utf8")
            corrupt = bytearray(descriptors)
            corrupt[-24] ^= 1  # width at +48 of the last 72-byte page
            sheets.write_bytes(corrupt)
            if not run_again().returncode or target.read_bytes() != expected:
                raise ValueError("Mismatched source/compiled geometry replaced the existing descriptor")
            corrupt = bytearray(descriptors)
            corrupt[-70] ^= 1  # local dynamic-control count at +2 of the same page
            sheets.write_bytes(corrupt)
            if not run_again().returncode or target.read_bytes() != expected:
                raise ValueError("Mismatched source/compiled control length replaced the existing descriptor")
        return {"outcome": "exact", "bytes": len(original)}
    mismatch = next((i for i, (a, b) in enumerate(zip(original, generated)) if a != b),
                    min(len(original), len(generated)))
    return {"outcome": "different", "first": mismatch,
            "expectedSize": len(original), "generatedSize": len(generated)}


def check_manifest_mode(exe, root):
    """Build a typed two-page manifest with independent primitives and check transactional rejection."""
    context = root / "mixed-media.json"
    out = root / "load.bin"
    arrays = {"macroFiles": ["Start.bin"], "driverPaths": ["本地/串口.bin"],
              "assetPairs": [["字库", ""], [None, "B"]], "flags": [True, False, True, False],
              "pageSizes": [[800.0, 480.0], [321.25, 240.5]]}
    model = {**arrays, "model": "AMZ070W01", "pageCount": 2, "startupType": 0, "startupImage": "",
             "macroCount": 1, "panelType": 1, "optionText": None,
             "compilerCounter838": 0, "connectionCount": 2, "compilerCounter688": 5, "historyCount": 1}
    def string_field(value):
        if value is None:
            return struct.pack(">i", -1)
        raw = value.encode("utf-16be")
        return struct.pack(">I", len(raw)) + raw
    expected = (string_field("KinSealStudio-V1.0.2") + struct.pack(">i", 122) + string_field(model["model"]) +
                struct.pack(">ib", 2, 0) + string_field("") + bytes([1, 0, 1, 0]) + struct.pack(">ii", 1, 1) +
                string_field("Start.bin") + struct.pack(">i", 1) + string_field("本地/串口.bin") +
                struct.pack(">i", 2) + string_field("字库") + string_field("") + string_field(None) + string_field("B") +
                struct.pack(">i", 1) + string_field(None) + struct.pack(">iiii", 0, 2, 5, 2) +
                struct.pack(">ddddi", 800.0, 480.0, 321.25, 240.5, 1))
    def invoke(value):
        context.write_text(json.dumps(value, ensure_ascii=False), encoding="utf8")
        return subprocess.run([str(exe), "--build-native-load-metadata", str(context), str(out)],
                              env={**os.environ, "QT_QPA_PLATFORM": "offscreen"}, timeout=45, capture_output=True,
                              creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    proc = invoke(model)
    if proc.returncode or out.read_bytes() != expected:
        raise ValueError("Typed full-manifest encoding failed")
    for overrides in ({"compilerCounter838": 1.5}, {"assetPairs": [[45, 80]]},
                      {"pageCount": 1}, {"macroCount": 0}, {"flags": [0, 1, 0, 1]}):
        if not invoke({**model, **overrides}).returncode or out.read_bytes() != expected:
            raise ValueError(f"Invalid manifest changed an existing descriptor: {overrides}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path)
    parser.add_argument("--report", type=Path)
    parser.add_argument("--native-kns", type=Path, action="append", default=[],
                        help="Check an additional vendor KNS, using only its SM snapshot and independent parts for generation.")
    parser.add_argument("--native-compile", type=Path, action="append", default=[],
                        help="Check an additional uncompressed vendor compile directory containing upLoad.knr and load.bin.")

    parser.add_argument("--revision", default=REVISION)
    parser.add_argument("--temp-parent", type=Path, default=PROJECT / "build")
    options = parser.parse_args()
    exe = options.executable.resolve()
    locations = inventory(options.revision)
    groups = {}
    skipped = []
    for folder, files in locations.items():
        if "input-native.sm" not in files and "upLoad.knr" not in files:
            skipped.append(folder)
            continue
        key = tuple(files.get(name) for name in ("input-native.sm", "load.bin", "SheetPara.dat", "ControlPara.dat", "Modbus_RTU_com1.bin"))
        if "input-native.sm" not in files:
            key = (files["upLoad.knr"], *key[1:])
        groups.setdefault(key, []).append(folder)
    results = {}
    with tempfile.TemporaryDirectory(prefix=".load-v122-verify-", dir=options.temp_parent) as tmp:
        check_manifest_mode(exe, Path(tmp))
        external = {}
        for i, path in enumerate(options.native_kns):
            with zipfile.ZipFile(path) as archive:
                entries = archive.infolist()
                if any(i.file_size > 64 * 1024 * 1024 for i in entries):
                    raise ValueError(f"Unsupported oversized member in {path}")
                files = {i.filename: i for i in entries}
                external[str(path)] = check_case(exe, Path(tmp), f"kns-{i}", files, archive.read)
        for i, path in enumerate(options.native_compile):
            files = {p.name: p for p in path.iterdir() if p.is_file()}
            external[str(path)] = check_case(exe, Path(tmp), f"compile-{i}", files, Path.read_bytes)

        with concurrent.futures.ThreadPoolExecutor(max_workers=3) as thread:
            futures = {thread.submit(check_case, exe, Path(tmp), idx, locations[folders[0]]): folders
                       for idx, folders in enumerate(groups.values())}
            for future in concurrent.futures.as_completed(futures):
                status = future.result()
                for folder in futures[future]:
                    results[folder] = status
    totals = dict(collections.Counter(r["outcome"] for r in results.values()))
    summary = {"revision": options.revision, "archiveDirectories": len(locations),
               "missingAdjacentSource": len(skipped), "distinctComparisons": len(groups), "totals": totals,
               "external": external}
    print(json.dumps(summary, ensure_ascii=False))
    if options.report:
        options.report.write_text(json.dumps({"summary": summary, "results": results, "missingSource": skipped},
                                            indent=1, ensure_ascii=False), encoding="utf8")
    if not locations or len(locations) != totals.get("exact", 0) or any(r["outcome"] != "exact" for r in external.values()):
        raise SystemExit(1)


if __name__ == "__main__":
    main()
