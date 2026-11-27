"""Verify independently rebuilt Usbload.bin fields and measure source-matched reference pairs."""
# 20261128-6 add
import argparse
import collections
import hashlib
import io
import json
import os
from pathlib import Path
import struct
import tempfile
import zlib
import subprocess
import sys
import zipfile


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from rebuild_usb_metadata_fields import LENGTH_FILES, build_fields
from verify_native_usb_metadata import archives


def compare_archive(raw, label):
    """Use load.bin and the member lengths as inputs, then compare only the documented USB ranges."""
    with zipfile.ZipFile(io.BytesIO(raw)) as archive:
        names = {name.lower(): name for name in archive.namelist()}
        metadata = archive.read(names["load.bin"])
        lengths = {name: archive.getinfo(names[name]).file_size for name in LENGTH_FILES if name in names}
        info = archive.getinfo(names["usbload.bin"])
        seconds = 3600 * info.date_time[3] + 60 * info.date_time[4] + info.date_time[5]
        oracle = archive.read(info)
    fields = build_fields(metadata, lengths)
    if len(oracle) != 1704:
        raise ValueError(f"{label}: unexpected descriptor size")
    if any(oracle[start:start+len(value)] != value for start, value in fields.items()):
        raise ValueError(f"{label}: deterministic project fields differ")
    # DOS timestamps have 2-second precision and refer to the file write, not the earlier call to qrand().
    deltas = [delta for delta in (0, 1, -1) if struct.pack("<I", ((214013 * ((seconds + delta) % 86400) + 2531011) >> 16) & 32767) == oracle[4:8]]
    if not deltas:
        raise ValueError(f"{label}: no adjacent native timestamp matches")
    known = build_fields(metadata, lengths, (seconds + deltas[0]) % 86400)
    if any(oracle[start:start+len(value)] != value for start, value in known.items()):
        raise ValueError(f"{label}: timestamp and project fields differ")
    return {"knownBytes": sum(map(len, known.values())), "sha256": hashlib.sha256(oracle).hexdigest(),
            "differencesInComparedFields": 0, "secondsRelativeToDos": deltas[0]}


def source_matched(revision):
    """Group on byte-identical input SM, media metadata, and each of the four complete input payloads."""
    listing = subprocess.check_output(["git", "ls-tree", "-rz", revision, "--", "docs"], cwd=ROOT)
    directories = {}
    for line in listing.split(bytes(1)):
        if not line:
            continue
        meta, name = line.split(b"\t", 1)
        directory, file = name.decode("utf8").rsplit("/", 1)
        directories.setdefault(directory, {})[file.lower()] = meta.split()[-1].decode()
    groups = collections.defaultdict(list)
    matched_by_source = collections.defaultdict(list)
    def binary(oid):
        return subprocess.check_output(["git", "cat-file", "blob", oid], cwd=ROOT)
    for label, files in directories.items():
        if not {"input-native.sm", "load.bin", "usbload.bin"} <= files.keys():
            continue
        ident = tuple(files.get(name) for name in ("input-native.sm", "load.bin", *LENGTH_FILES))
        groups[ident].append((label, files["usbload.bin"]))
        matched_by_source[ident[0]].append((label, files["usbload.bin"]))
    from functools import lru_cache
    binary = lru_cache(maxsize=500)(binary)
    def summarize(collection):
        result = collections.Counter()
        example = None
        for key, rows in collection.items():
            if len(rows) < 2:
                continue
            result["groups"] += 1
            for i, (label, lhs) in enumerate(rows):
                for other, rhs in rows[i+1:]:
                    result["pairs"] += 1
                    x, y = binary(lhs), binary(rhs)
                    diff = sum(a != b for j, (a, b) in enumerate(zip(x, y)) if j not in range(4, 8))
                    if not diff:
                        result["equalExceptTime"] += 1
                        continue
                    result["differentAfterTimeRemoved"] += 1
                    if example is None:
                        example = {"left": label, "right": other, "leftUsbBlob": lhs, "rightUsbBlob": rhs,
                                   "differentBytesExcludingTime": diff, "groupKey": key}
        return {"counts": dict(result), "firstDifferingPair": example}
    return {"sameSm": summarize(matched_by_source), "sameSmAndFourPayloadsAndLoadMetadata": summarize(groups)}


def check_source_pipeline(raw, executable):
    """Compile load.bin from the adjacent original SM and compare USB fields without providing either old descriptor."""
    with zipfile.ZipFile(io.BytesIO(raw)) as archive, tempfile.TemporaryDirectory(dir=ROOT / "build") as temp:
        members = {item.filename.lower(): item for item in archive.infolist()}
        snapshot = archive.read(members["upload.knr"])
        source_bytes = zlib.decompress(snapshot[4:])
        if len(source_bytes) != int.from_bytes(snapshot[:4], "big"):
            raise ValueError("KNR source length mismatch")
        directory = Path(temp)
        source = directory / "project.sm"
        source.write_bytes(source_bytes)
        compiled = directory / "parts"
        compiled.mkdir()
        for name in ("sheetpara.dat", "controlpara.dat", "modbus_rtu_com1.bin"):
            if name in members:
                item = members[name]
                (compiled / item.filename).write_bytes(archive.read(item))
        context = directory / "context.json"
        context.write_text(json.dumps({"compiledDirectory": str(compiled), "assetPairs": []}), encoding="utf8")
        generated = directory / "load.bin"
        env = dict(os.environ)
        # The Windows release ships qwindows. The known CLI subcommand returns before constructing any window.
        if os.name == "nt" and not (executable.parent / "platforms" / "qoffscreen.dll").is_file() and not env.get("QT_QPA_PLATFORM_PLUGIN_PATH"):
            env["QT_QPA_PLATFORM"] = "windows"
        else:
            env.setdefault("QT_QPA_PLATFORM", "offscreen")
        subprocess.run([str(executable), "--build-native-load-metadata", str(source), str(generated), str(context)],
                       env=env, timeout=65, check=True, capture_output=True,
                       creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        lengths = {name: members[name].file_size for name in LENGTH_FILES if name in members}
        fields = build_fields(generated.read_bytes(), lengths)
        oracle = archive.read(members["usbload.bin"])
        if any(oracle[off:off + len(data)] != data for off, data in fields.items()):
            raise ValueError("Original source through generated load.bin produced different USB fields")
        if generated.read_bytes() != archive.read(members["load.bin"]):
            raise ValueError("Intermediate descriptor differs")
        return {"loadBinExact": True, "exactUsbProjectBytesExcludingTime": sum(map(len, fields.values()))}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--git-revision", default="e3b72657")
    parser.add_argument("--report", type=Path)
    parser.add_argument("--executable", type=Path, help="Additionally compile load.bin from each external archive’s original source")
    parser.add_argument("archives", nargs="*", type=Path)
    options = parser.parse_args()
    cases = []
    extra = []
    for key, raw in archives(options.git_revision, []):
        cases.append(compare_archive(raw, key))
    for path in options.archives:
        raw = path.read_bytes()
        item = compare_archive(raw, path.name)
        if options.executable:
            item["fromOriginalSm"] = check_source_pipeline(raw, options.executable.resolve())
        extra.append(item)
    distribution = collections.Counter(r["knownBytes"] for r in cases + extra)
    if not cases and not extra:
        raise ValueError("No media descriptors compared")
    summary = {"revision": options.git_revision, "gitArchives": len(cases),
               "additionalArchives": extra, "exactKnownBytesPerArchive": distribution,
               "verifiedArchives": len(cases) + len(extra), "fileSize": 1704,
               "sourcePairs": source_matched(options.git_revision)}
    report = json.dumps(summary, indent=2, ensure_ascii=False)
    if options.report:
        options.report.write_text(report + "\n", encoding="utf8")
    print(report)


if __name__ == "__main__":
    main()
