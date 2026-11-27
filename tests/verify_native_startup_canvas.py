"""Verify normal canvas import and byte-exact field edits on three original V122 boot-artwork projects."""

import argparse
import hashlib
import json
import struct
import tempfile
from pathlib import Path

from verify_native_data_display_sources import ROOT, call, require


CASES = {
    "ASCII显示元件.sm": "ascii",
    "时钟同步/与PLC时间同步.sm": "fx",
    "时钟同步/写时间到PLC.sm": "fx",
}


def qstring(text):
    data = text.encode("utf-16be")
    return struct.pack(">I", len(data)) + data


def verify(exe, source, kind, work):
    original = source.read_bytes()
    raw_index = work / "fields.json"
    model_file = work / "canvas.ksp"
    edited_file = work / "edited.sm"
    reopened = work / "reopened.ksp"
    call(exe, work, "--inspect-native-source", source, raw_index)
    fields = {row["path"]: (row["start"], row["end"]) for row in json.loads(raw_index.read_bytes())["fields"]}
    boot = original[fields["startup.head.0"][0]:fields["history.1e4.count"][0]]
    picture = original[slice(*fields["system.194[0].5"])][4:]
    call(exe, work, "--migrate-project", source, model_file)
    call(exe, work, "--export-native-project", model_file, edited_file)
    require(edited_file.read_bytes() == original, "unchanged canvas save differs")
    model = json.loads(model_file.read_bytes())
    objects = model["pages"][0]["objects"]
    if kind == "ascii":
        target = objects[6]
        require(target["type"] == "ascii" and target["width"] == 179, "original fractional ASCII geometry")
        require(struct.unpack(">d", original[slice(*fields["object[6].child[0].text.head.3"])])[0] == 82.4,
                "original fractional height")
        target["width"] = 196
        replacements = {"object[6].child[0].text.head.2": struct.pack(">d", 196.0)}
        check = {"width": 196}
        ordinal = 6
    else:
        connection = model["connections"][0]
        require(connection["profileId"] == "Link/serial_amz/MITSUBISHI/Mitsubishi_Fx3U.ini"
                and connection["channel"] == "COM1" and connection["station"] == "0"
                and connection["node"] is None, "Mitsubishi clock communication settings")
        time = objects[1]["timeDisplay"]
        require(time["showDate"] and time["showTime"] and not time["showWeek"]
                and time["dateFormat"] == 0 and time["timeFormat"] == 1, "date and time with seconds")
        require([item["address"] for item in objects[4:11]] == [f"@com1:D{i}" for i in range(6, -1, -1)],
                "seven original FX3U D registers")
        target = objects[4]
        target["address"] = "@com1:D27"
        target["numberDisplay"]["monitorAddress"] = target["address"]
        replacements = {"object[4].display.17.0": qstring("[Mitsubishi_Fx3U]D27"),
                        "object[4].display.17.4": qstring("27"),
                        "object[4].display.2": qstring("[Mitsubishi_Fx3U]D27"),
                        "object[4].child[0].text.head.4": qstring("8888\nD27")}
        check = {"address": "@com1:D27", "numberDisplay": target["numberDisplay"]}
        ordinal = 4
    model_file.write_text(json.dumps(model, ensure_ascii=False), encoding="utf8")
    call(exe, work, "--export-native-project", model_file, edited_file)
    expected = original
    for name, encoded in sorted(replacements.items(), key=lambda entry: fields[entry[0]][0], reverse=True):
        lo, hi = fields[name]
        expected = expected[:lo] + encoded + expected[hi:]
    require(edited_file.read_bytes() == expected, f"{source.name}: bytes outside requested edits changed")
    call(exe, work, "--migrate-project", edited_file, reopened)
    restored = json.loads(reopened.read_bytes())["pages"][0]["objects"][ordinal]
    for key, value in check.items():
        require(restored[key] == value, f"{source.name}: {key} not restored")
    call(exe, work, "--export-native-project", reopened, work / "resaved.sm")
    require((work / "resaved.sm").read_bytes() == expected, "edited canvas resave differs")
    # Unverified resource references and contradictory external bindings must retain the last good output.
    untouched = reopened.read_bytes()
    negatives = {"startup.head.8": qstring("unknown_startup_asset")}
    if kind == "fx":
        negatives["object[4].display.17.2"] = b"\x08"
        negatives["system.16c[0].50[0].tail.10"] = qstring("other_driver")
    damaged = work / "unmapped.sm"
    for path, encoded in negatives.items():
        lo, hi = fields[path]
        damaged.write_bytes(original[:lo] + encoded + original[hi:])
        call(exe, work, "--migrate-project", damaged, reopened, accepted=False)
        require(reopened.read_bytes() == untouched, f"failed {path} import changed destination")
    return {"sample": source.name, "sourceSha256": hashlib.sha256(original).hexdigest(),
            "sourceBytes": len(original), "ordinaryControls": len(objects),
            "bootPageSha256": hashlib.sha256(boot).hexdigest(), "bootArtSha256": hashlib.sha256(picture).hexdigest(),
            "expectedEditedFields": list(replacements), "unchangedByteEqual": True,
            "editedWholeFileByteEqual": True, "editedPropertiesRestored": True, "secondSaveByteEqual": True,
            "rejectedConflictingFields": list(negatives)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--cases", type=Path, required=True, help="Factory example project directory")
    parser.add_argument("--work-root", type=Path, default=ROOT / "build")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    exe = args.exe.resolve(strict=True)
    args.work_root.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="boot-canvas-", dir=args.work_root) as temporary:
        rows = [verify(exe, (args.cases / name).resolve(strict=True), variant, Path(temporary).resolve())
                for name, variant in CASES.items()]
    receipt = {"executableSha256": hashlib.sha256(exe.read_bytes()).hexdigest(), "cases": rows}
    if args.receipt:
        args.receipt.write_text(json.dumps(receipt, indent=2, ensure_ascii=False) + "\n", encoding="utf8")
    print("3 original canvas sources imported, edited, reopened and saved byte-exactly")


if __name__ == "__main__":
    main()
