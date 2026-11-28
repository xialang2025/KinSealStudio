"""Verify independent ordinary-page records against the frozen original archives."""
import argparse
import collections
import hashlib
import io
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import zipfile
import zlib


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from native_sheet_records import encode_sheet, read_page_fields


def page_blocks(controls, count):
    """Use control-picture offset resets to split the six-page reference without looking at its sheet table."""
    if count == 1:
        return [controls]
    starts = [0]
    previous_end = 0
    for pos in range(0, len(controls), 640):
        offset, size = struct.unpack_from("<II", controls, pos + 12)
        if offset == 0 and previous_end > 0:
            starts.append(pos)
            previous_end = 0
        if offset != previous_end:
            raise ValueError("control image offsets do not describe consecutive page blocks")
        previous_end += size
    if len(starts) != count:
        raise ValueError("control offsets do not identify every page boundary")
    return [controls[a:b] for a, b in zip(starts, starts[1:] + [len(controls)])]


def verify(exe, scratch, revision, additional):
    """Compute actual bytes before reading SheetPara.dat, and retain every unequal offset."""
    tree = {}
    listing = subprocess.check_output(["git", "ls-tree", "-rz", revision], cwd=ROOT)
    for row in listing.split(b"\0"):
        if row:
            meta, path = row.split(b"\t", 1)
            tree[path.decode()] = meta.split()[2]
    proc = subprocess.Popen(["git", "cat-file", "--batch"], cwd=ROOT, stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    def blob(oid):
        proc.stdin.write(oid + b"\n")
        proc.stdin.flush()
        size = int(proc.stdout.readline().split()[2])
        data = proc.stdout.read(size)
        if len(data) != size or proc.stdout.read(1) != b"\n":
            raise OSError("incomplete reference blob")
        return data

    def inputs():
        seen = set()
        for path, oid in tree.items():
            if path.lower().endswith("/load.kns") and oid not in seen:
                seen.add(oid)
                sm_oid = tree.get(path.rpartition("/")[0] + "/input-native.sm")
                yield path, blob(oid), blob(sm_oid) if sm_oid else None
        for path in additional:
            yield path.name, path.read_bytes(), None

    parsed = {}
    rows = []
    offsets = collections.Counter()
    try:
        with tempfile.TemporaryDirectory(dir=scratch) as temp:
            sm = Path(temp) / "input.sm"
            for label, packed, source in inputs():
                with zipfile.ZipFile(io.BytesIO(packed)) as archive:
                    origin = "original-sm" if source is not None else "archive-source"
                    if source is None:
                        knr = archive.read("upLoad.knr")
                        source = zlib.decompress(knr[4:])
                        if len(source) != int.from_bytes(knr[:4], "big"):
                            raise ValueError("invalid archived source size")
                    digest = hashlib.sha256(source).hexdigest()
                    if digest not in parsed:
                        sm.write_bytes(source)
                        parsed[digest] = read_page_fields(exe, sm)
                    pages = parsed[digest]
                    controls = archive.read("ControlPara.dat") if "ControlPara.dat" in archive.namelist() else b""
                    blocks = page_blocks(controls, len(pages))
                    actual = b"".join(encode_sheet(page, block, "cp936") for page, block in zip(pages, blocks))
                    expected = archive.read("SheetPara.dat")
                if len(actual) != len(expected):
                    raise ValueError("page-table length mismatch")
                difference = [i for i, (a, b) in enumerate(zip(actual, expected)) if a != b]
                offsets.update(i % 72 for i in difference)
                rows.append(dict(reference=label, origin=origin, source=digest, pages=len(pages),
                                 compiledSHA256=hashlib.sha256(actual).hexdigest(),
                                 originalSHA256=hashlib.sha256(expected).hexdigest(), differentOffsets=difference))
    finally:
        proc.stdin.close()
        proc.wait()
    return dict(revision=revision, archives=len(rows), sourceDigests=len(parsed), pages=sum(x["pages"] for x in rows),
                wholeFilesExact=sum(not x["differentOffsets"] for x in rows),
                mismatchedOffsetsPerPage=dict(sorted(offsets.items())), rows=rows)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("archives", type=Path, nargs="*")
    parser.add_argument("--revision", default="e3b72657")
    parser.add_argument("--scratch", type=Path, default=ROOT / "build")
    parser.add_argument("--report", type=Path)
    parser.add_argument("--require-all", action="store_true")
    args = parser.parse_args()
    result = verify(args.exe.resolve(), args.scratch.resolve(), args.revision, args.archives)
    if args.report:
        args.report.write_text(json.dumps(result, ensure_ascii=False, indent=1) + "\n", encoding="utf8")
    print(json.dumps({key: value for key, value in result.items() if key != "rows"}))
    # A residual is always counted. The ordinary check also requires every observed difference to be at the proven gap.
    if (not result["archives"] or set(result["mismatchedOffsetsPerPage"]) - {59} or
            (args.require_all and result["wholeFilesExact"] != result["archives"])):
        raise SystemExit(1)
