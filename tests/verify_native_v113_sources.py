"""Verify both original V113 Modbus demonstration sources without distributing customer files."""

import argparse
import json
import struct
import tempfile
from pathlib import Path

from verify_native_data_display_sources import ROOT, call, digest, require


def verify(exe, work, source):
    original = source.read_bytes()
    saved = work / "roundtrip.sm"
    report = work / "fields.json"
    edited = work / "edited.sm"
    damaged = work / "damaged.sm"
    call(exe, work, "--inspect-native-source", source, report)
    fields = json.loads(report.read_bytes())["fields"]
    table = {row["path"]: (row["start"], row["end"]) for row in fields}
    require(len(table) == len(fields), "duplicate native field names")
    end = 0
    for field in sorted(fields, key=lambda row: row["start"]):
        require(field["start"] == end, "unparsed or overlapping native bytes")
        end = field["end"]
    require(end == len(original), "trailing bytes outside the field table")

    def field(path):
        start, stop = table[path]
        return original[start:stop]

    def word(path):
        return struct.unpack(">i", field(path))[0]

    def string(path):
        raw = field(path)
        require(struct.unpack(">I", raw[:4])[0] == len(raw) - 4, "QString size mismatch")
        return raw[4:].decode("utf-16be")

    require(word("header.1") == 113 and string("system.identity.1") == "SUP043", "legacy identity")
    require(word("project.pages.count") == 1 and word("page.objects.count") == 11, "ordinary screen count")
    require(string("startup.head.18") == "启动" and word("startup.objects.count") == 0, "startup screen count")
    kinds = [word(f"object[{i}].type") for i in range(11)]
    require(sorted(kinds) == [10] * 4 + [12] * 4 + [28] * 3, "ordinary screen types")
    call(exe, work, "--resave-native-source", source, saved)
    require(saved.read_bytes() == original, "unmodified native save differs")
    title = "V113双向编辑"
    call(exe, work, "--resave-native-source", source, edited, title)
    start, stop = table["trailer.settings.1"]
    encoded = title.encode("utf-16be")
    expected = original[:start] + struct.pack(">I", len(encoded)) + encoded + original[stop:]
    require(edited.read_bytes() == expected, "unexpected bytes changed during the title edit")
    call(exe, work, "--resave-native-source", edited, saved, string("trailer.settings.1"))
    require(saved.read_bytes() == original, "restoring the title did not reconstruct the original")
    for path, data in (("truncation", original[:-1]), ("suffix", original + b"\x00"),
                       ("unknown root type", original[:table["object[0].type"][0]] +
                        struct.pack(">i", 30) + original[table["object[0].type"][1]:])):
        damaged.write_bytes(data)
        call(exe, work, "--resave-native-source", damaged, saved, accepted=False)
        require(saved.read_bytes() == original, f"{path} changed destination after failure")
    return {"file": source.name, "size": len(original), "sha256": digest(original),
            "headerModel": string("header.2"), "systemModel": string("system.identity.1"),
            "ordinaryRootTypes": kinds, "startupRoots": 0, "fieldCount": len(fields),
            "sourceBytesCovered": end, "unchangedSaveEqual": True, "editedBytesEqual": True,
            "restoredOriginalEqual": True, "malformedInputsRejected": 3}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--cases", required=True, type=Path, help="Directory with original master and slave .sm sources.")
    parser.add_argument("--work-root", default=ROOT / "build", type=Path)
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    inputs = sorted(args.cases.glob("*.sm"))
    require(len(inputs) == 2, f"expected two factory projects at {args.cases}")
    exe = args.exe.resolve(strict=True)
    args.work_root.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="legacy113-", dir=args.work_root) as temporary:
        rows = [verify(exe, Path(temporary).resolve(), item.resolve()) for item in inputs]
    require({row["headerModel"] for row in rows} == {"KNS035", "KNS043"}, "expected both original template models")
    result = {"formatVersion": 113, "executableSha256": digest(exe.read_bytes()), "projects": rows}
    if args.receipt:
        args.receipt.write_text(json.dumps(result, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print("Both original V113 sources verified")


if __name__ == "__main__":
    main()
