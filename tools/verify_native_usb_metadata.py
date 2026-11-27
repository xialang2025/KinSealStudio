"""Compare independently encoded V5.1 Usbload.bin fields with original media archives."""
# 20261127-11 add
import argparse
import collections
import hashlib
import io
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import zipfile
import zlib


LENGTH_FIELDS = {
    0xF8: "sheetpixel.dat", 0xFC: "controlpixel.dat",
    0x100: "addrpara.dat", 0x104: "piclenpara.dat",
}


def archives(revision, extra_paths):
    """Yield unique archive bytes and labels from the requested Git tree and standalone files."""
    seen = set()
    if revision:
        tree = subprocess.check_output(["git", "ls-tree", "-r", "-z", revision])
        for entry in tree.split(b"\0"):
            if not entry:
                continue
            meta, path = entry.split(b"\t", 1)
            if not path.lower().endswith(b"load.kns"):
                continue
            blob = meta.split()[2].decode("ascii")
            if blob in seen:
                continue
            seen.add(blob)
            yield blob, subprocess.check_output(["git", "cat-file", "blob", blob])
    for path in extra_paths:
        yield path.name, path.read_bytes()


def page_sizes(executable, uploaded_source, scratch):
    """Decode counted page viewports from the native saved with this exact compilation."""
    source = zlib.decompress(uploaded_source[4:])
    if len(source) != int.from_bytes(uploaded_source[:4], "big"):
        raise ValueError("KNR decompressed length mismatch")
    with tempfile.TemporaryDirectory(dir=scratch) as temporary:
        path = Path(temporary) / "project.sm"
        path.write_bytes(source)
        completed = subprocess.run(
            [str(executable), "--inspect-native-source", str(path)],
            capture_output=True, check=True, timeout=120,
            env=dict(os.environ, QT_QPA_PLATFORM="offscreen"),
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
        )
    fields = {item["path"]: item["start"] for item in json.loads(completed.stdout)["fields"]}
    count, = struct.unpack_from(">I", source, fields["project.pages.count"])
    if count > 200:
        raise ValueError("page count exceeds descriptor table")
    sizes = []
    for index in range(count):
        page = "page" if index == 0 else f"page[{index}]"
        width, height = struct.unpack_from(">dd", source, fields[page + ".head.2"])
        sizes.append(int(width * height * 2))
    return sizes


def verify(raw, executable, scratch):
    """Compare exact encoded little-endian bytes at documented offsets; leave other bytes unclassified."""
    with zipfile.ZipFile(io.BytesIO(raw)) as archive:
        names = {name.lower(): name for name in archive.namelist()}
        descriptor = archive.read(names["usbload.bin"])
        timestamp = archive.getinfo(names["usbload.bin"]).date_time
        seconds = timestamp[3] * 3600 + timestamp[4] * 60 + timestamp[5]
        expected = {0x0C: 123}
        for offset, name in LENGTH_FIELDS.items():
            expected[offset] = archive.getinfo(names[name]).file_size if name in names else 0
        for index, size in enumerate(page_sizes(executable, archive.read(names["upload.knr"]), scratch)):
            expected[0x388 + index * 4] = size
    errors = [] if len(descriptor) == 1704 else ["length"]
    errors.extend(hex(offset) for offset, value in expected.items()
                  if descriptor[offset:offset + 4] != struct.pack("<I", value))
    # The ZIP implementation stores local time in two-second increments. Keep both possible inputs explicit.
    seconds_candidates = [second for second in (seconds, seconds + 1)
                          if descriptor[4:8] == struct.pack("<I", ((214013 * second + 2531011) >> 16) & 32767)]
    return dict(sha256=hashlib.sha256(raw).hexdigest(), usbSha256=hashlib.sha256(descriptor).hexdigest(),
                mismatch=errors, secondsFromDosTime=[s - seconds for s in seconds_candidates],
                sourcePages=len(expected) - 1 - len(LENGTH_FIELDS))


def main():
    """Collect machine-readable comparisons without opening a connection to a device."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--git-revision")
    parser.add_argument("--scratch", type=Path)
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("archives", nargs="*", type=Path)
    args = parser.parse_args()
    results = []
    by_digest = set()
    for label, blob in archives(args.git_revision, args.archives):
        digest = hashlib.sha256(blob).digest()
        if digest in by_digest:
            continue
        by_digest.add(digest)
        row = verify(blob, args.exe.resolve(), args.scratch)
        row["input"] = label
        results.append(row)
    counts = collections.Counter(delta for r in results for delta in r["secondsFromDosTime"])
    report = dict(total=len(results), stableFieldsMatch=sum(not r["mismatch"] for r in results),
                  secondsFromDosTime=counts, measuredTimeMatches=sum(bool(r["secondsFromDosTime"]) for r in results),
                  rows=results)
    args.report.write_text(json.dumps(report, ensure_ascii=False, indent=1), encoding="utf-8")
    print(json.dumps({k: v for k, v in report.items() if k != "rows"}))
    return 0 if results and all(not r["mismatch"] for r in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
