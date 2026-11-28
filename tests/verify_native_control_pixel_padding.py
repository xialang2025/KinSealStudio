"""Replay original control frame padding and test both complete package encodings with rehashed corruptions."""
# 20261128-12 add
import argparse
import hashlib
import io
import json
from pathlib import Path
import struct
import subprocess
import tempfile
import zipfile

from verify_native_drawing_drafts import ROOT, REVISION, git_blob, run


def sha(data):
    """Compute the SHA-256 stored in device file and transfer plan entries."""
    return hashlib.sha256(data).hexdigest()


def reconstruct(background, lengths, original):
    """Use only compressed packet prefixes and the separate background to construct the entire aligned output."""
    if len(lengths) < 2400 or (len(lengths) - 2400) % 12:
        raise ValueError("bad index table size")
    background_len, = struct.unpack_from("<I", lengths)
    control_len, = struct.unpack_from("<I", lengths, 800)
    count, = struct.unpack_from("<I", lengths, 1600)
    if (background_len, control_len, count) != (len(background), len(original), (len(lengths) - 2400) // 12):
        raise ValueError("inconsistent single-page index")
    workspace = bytearray(background)
    product = bytearray()
    tails = []
    for kind, aligned, actual in struct.iter_unpack("<III", lengths[2400:]):
        offset = len(product)
        if kind or not actual or aligned != (actual + 31) & ~31 or offset + aligned > len(original):
            raise ValueError("invalid frame bounds")
        if len(workspace) < aligned:
            workspace.extend(bytes(aligned - len(workspace)))
        workspace[:actual] = original[offset:offset + actual]
        product.extend(workspace[:aligned])
        tails.append((offset + actual, offset + aligned))
    if len(product) != len(original):
        raise ValueError("unindexed bytes")
    return bytes(product), tails


def audit_native():
    """Replay every original single-page archive carrying control pixels in the frozen Git revision."""
    listing = subprocess.check_output(["git", "ls-tree", "-rz", REVISION], cwd=ROOT)
    archives = {}
    rows = []
    for entry in filter(None, listing.split(b"\0")):
        descriptor, path = entry.split(b"\t", 1)
        if path.lower().endswith(b"/load.kns"):
            archives.setdefault(descriptor.split()[2], path.decode())
    with subprocess.Popen(["git", "cat-file", "--batch"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, cwd=ROOT) as pipe:
        for oid, reference in archives.items():
            pipe.stdin.write(oid + b"\n")
            pipe.stdin.flush()
            actual, kind, size = pipe.stdout.readline().split()
            raw = pipe.stdout.read(int(size))
            if oid != actual or kind != b"blob" or len(raw) != int(size) or pipe.stdout.read(1) != b"\n":
                raise ValueError("incomplete archive")
            with zipfile.ZipFile(io.BytesIO(raw)) as arc:
                names = {n.lower(): n for n in arc.namelist()}
                if "controlpixel.dat" not in names or len(arc.read(names["sheetpara.dat"])) != 72:
                    continue
                control = arc.read(names["controlpixel.dat"])
                expected, tails = reconstruct(arc.read(names["sheetpixel.dat"]), arc.read(names["piclenpara.dat"]), control)
            if expected != control:
                raise ValueError("native alignment replay changed the file: " + reference)
            rows.append(dict(archive=reference, controlSha256=sha(control), frames=len(tails),
                             padding=sum(b - a for a, b in tails), nonzeroTails=sum(any(control[a:b]) for a, b in tails)))
        pipe.stdin.close()
    return dict(originalArchives=len(rows), frames=sum(r["frames"] for r in rows),
                padding=sum(r["padding"] for r in rows), nonzeroTails=sum(r["nonzeroTails"] for r in rows), rows=rows)


def verify_packages(exe, scratch):
    """Verify independent native/legacy acceptance and reject frame mixing or corruption after updating both hashes."""
    with tempfile.TemporaryDirectory(dir=scratch) as tmp:
        work = Path(tmp).resolve()
        folder = work / "compiled"
        source = work / "input.sm"
        cases = {}
        fixture = "docs/stage2-evidence/lamp-regression-restore/compiled/lamp-dual-flash0-r1"
        source.write_bytes(git_blob(fixture, "input-native.sm"))
        run(exe, work, "--compile-project", source, folder)
        header = folder / "compiler-manifest.json"
        manifest = json.loads(header.read_bytes())
        control_file = folder / "ControlPixel.dat"
        output = control_file.read_bytes()
        expected, tails = reconstruct((folder / "SheetPixel.dat").read_bytes(), (folder / "PicLenPara.dat").read_bytes(), output)
        if expected != output:
            raise ValueError("normal compile path did not apply the native alignment bytes")
        nonzero = [(a, b) for a, b in tails if any(expected[a:b])]
        if len(nonzero) < 2:
            raise ValueError("fixture must distinguish native/zero padding on two or more frames")
        step = next(s for s in manifest["transferPlan"] if s["name"] == "ControlPixel.dat")
        if step["command"] != 0x10 or step["wireSha256"] != sha(output) or step["wireBytes"] != len(output):
            raise ValueError("USB step does not contain the exact published pixel segment")
        legacy = bytearray(output)
        for a, b in tails:
            legacy[a:b] = bytes(b - a)
        # Test both orders. A verifier that accepts the two encodings independently per frame would accept both.
        a, b = nonzero[0]
        mixed_first_native = bytearray(legacy)
        mixed_first_native[a:b] = expected[a:b]
        mixed_first_zero = bytearray(expected)
        mixed_first_zero[a:b] = bytes(b - a)
        damaged = bytearray(expected)
        damaged[a] = 1 if expected[a] != 1 else 2
        for label, data, success in (("native", output, True), ("legacyZeros", legacy, True),
                                     ("firstNativeOthersZero", mixed_first_native, False),
                                     ("firstZeroOthersNative", mixed_first_zero, False),
                                     ("damagedTail", damaged, False)):
            control_file.write_bytes(data)
            manifest["files"]["ControlPixel.dat"]["sha256"] = sha(data)
            step["wireSha256"] = sha(data)
            header.write_text(json.dumps(manifest, ensure_ascii=False), encoding="utf8")
            response = json.loads(run(exe, work, "--verify-compiled-package", folder, success=success))
            if not success and "补齐" not in response.get("message", ""):
                raise ValueError("failed before exercising the padding check: " + str(response))
            cases[label] = dict(accepted=success, controlSha256=sha(data))
        return dict(sample=fixture, nativeTailFrames=len(nonzero), frames=len(tails), usbCommand=16, cases=cases)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path)
    parser.add_argument("--scratch", type=Path, default=ROOT / "build")
    parser.add_argument("--receipt", type=Path)
    options = parser.parse_args()
    result = audit_native()
    result["sourceRevision"] = REVISION
    if options.exe:
        result["production"] = verify_packages(options.exe.resolve(), options.scratch.resolve())
        result["exeSha256"] = sha(options.exe.read_bytes())
    if options.receipt:
        options.receipt.write_text(json.dumps(result, ensure_ascii=False, indent=1) + "\n", encoding="utf8")
    print(json.dumps({k: v for k, v in result.items() if k != "rows"}, ensure_ascii=True))
