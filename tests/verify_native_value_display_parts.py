"""Compare five independent value-display files with V5.1 originals and decode the RGB565 frames on AMZ firmware."""
# 20261128-25 add
import argparse
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import struct
from tempfile import TemporaryDirectory

from verify_amz_firmware_page_images import firmware_frame
from verify_amz_local_page_io import edit, layout
from verify_native_drawing_drafts import ROOT, REVISION, git_blob, run


NAMES = ("AddrPara.dat", "ControlPixel.dat", "PicLenPara.dat", "SheetPara.dat", "SheetPixel.dat")
CLI = "--compile-native-value-display-parts"


def sha(data):
    """Identify each whole file independently."""
    return hashlib.sha256(data).hexdigest()


def color_word(text):
    """Quantize an RGB value independently of Qt."""
    v = int(text[-6:], 16)
    return ((v >> 8) & 0xf800) | ((v >> 5) & 0x07e0) | ((v >> 3) & 0x001f)


def current(model, family):
    """Locate the shared visual properties inside the editor schema."""
    obj = model["pages"][0]["objects"][0]
    props = obj[family + "Display"]
    return obj, props["common"] if family == "ascii" else props


def verify_record(original, model, family):
    """Compare only named runtime fields, retaining their offsets without supplying any original bytes to the encoder."""
    obj, props = current(model, family)
    size = props["fontSize"]
    box = {"number": 0x11e, "ascii": 0x11c, "time": 0x10e}[family]
    fg = {"number": 0x138, "ascii": 0x130, "time": 0x120}[family]
    si = 0x118 if family == "time" else 0x128
    fonts = (12, 16, 24, 48, 64, 96, 192) if family == "time" else (16, 24, 32, 40, 48, 64, 96, 192)
    geometry = struct.pack("<4H", *(obj[k] for k in ("x", "y", "width", "height")))
    frames = 3 if family == "number" else 1
    fields = {0: bytes([{"number": 12, "ascii": 16, "time": 17}[family]]), 4: geometry, box: geometry,
              12: struct.pack("<II", 0, obj["width"] * obj["height"] * 2 * frames),
              si: struct.pack("<H", fonts.index(size)), fg: bytes.fromhex(props["textColor"][-6:]),
              fg+4: bytes.fromhex(props["backgroundColor"][-6:])}
    if family == "time":
        fields.update({0x108: bytes([int(props[k]) for k in ("showDate", "showTime", "showWeek")]),
                       0x11a: struct.pack("<HH", props["dateFormat"], props["timeFormat"])})
    elif family == "ascii":
        fields[0x12a] = struct.pack("<H", obj["asciiDisplay"]["characterCount"])
    else:
        fields.update({0x12a: struct.pack("<HHH", props["totalDigits"], props["decimalDigits"], props["dataType"]),
                       0x140: bytes.fromhex(props["highColor"][-6:]), 0x144: bytes.fromhex(props["lowColor"][-6:]),
                       0x148: struct.pack("<dd", float(props["gain"]), float(props["offset"])),
                       0x168: struct.pack("<dd", float(props["inputMaximum"]), float(props["inputMinimum"]))})
    if family != "time":
        for key, base in (("monitorAddress", 0x178 if family == "number" else 0x138),
                          ("inputAddress", 0x250 if family == "number" else 0x1a4)):
            address = props[key]
            fields[base] = b"\0" + address[1].encode("ascii")
            fields[base + 8] = struct.pack("<I", int(address[2:]))
    if len(original) != 640:
        raise ValueError("unexpected control record length")
    for offset, expected in fields.items():
        if original[offset:offset+len(expected)] != expected:
            raise ValueError(f"{family}: named runtime bytes differ at {offset:#x}")
    return {f"0x{k:03x}": v.hex() for k, v in sorted(fields.items())}


def verify_frames(fw, output, dimensions, colors, title):
    """Feed each encoded frame into the original ARM decoder, and check all pixels and indexing values."""
    table = (output / "PicLenPara.dat").read_bytes()
    sheet = (output / "SheetPara.dat").read_bytes()
    sheet_pixels = (output / "SheetPixel.dat").read_bytes()
    sprites = (output / "ControlPixel.dat").read_bytes()
    lengths = [struct.unpack_from("<I", table, offset)[0] for offset in (0, 800, 1600)]
    if lengths != [len(sheet_pixels), len(sprites), len(colors)] or len(table) != 2400 + 12*len(colors):
        raise ValueError("incorrect picture indexes")
    if (output / "AddrPara.dat").read_bytes() != bytes(200):
        raise ValueError("local scan table contains a remote read")
    area = dimensions[0] * dimensions[1]
    if (len(sheet) != 72 or struct.unpack_from("<I", sheet, 68)[0] != area * 2 * len(colors) or
            sheet[7:47] != title.encode("cp936")[:40].ljust(40, b"\0")):
        raise ValueError("incorrect ordinary page title or accumulated raw size")
    # Both the top row and left column are black in the actual page image.
    expected = bytes(1600) + (b"\0\0" + struct.pack("<H", 0xc6ff)*799)*479
    if firmware_frame(fw, sheet_pixels) != expected:
        raise ValueError("incorrect page pixels on the AMZ decoder")
    position = 0
    for i, color in enumerate(colors):
        zero, aligned, actual = struct.unpack_from("<III", table, 2400 + 12*i)
        if zero or not 0 < actual <= aligned or aligned != (actual + 31) & ~31:
            raise ValueError("invalid frame alignment")
        raw = firmware_frame(fw, sprites[position:position+actual], area*2)
        if raw != struct.pack("<H", color_word(color)) * area:
            raise ValueError(f"incorrect frame color for state {i}")
        position += aligned
    if position != len(sprites):
        raise ValueError("unindexed sprite data")


def exercise(exe, fw, work, family, variant):
    """Compare real files before editing, then test changed dimensions, colors, name and invalid input."""
    ref = f"docs/stage4-evidence/{family}/native/compiled/{family}-{variant}-r1"
    sm, ksp, out = work/"source.sm", work/"model.ksp", work/"original"
    source = git_blob(ref, "input-native.sm")
    sm.write_bytes(source)
    run(exe, work, "--migrate-project", sm, ksp)
    model = json.loads(ksp.read_bytes())
    run(exe, work, CLI, sm, out)
    if set(x.name for x in out.iterdir()) != set(NAMES):
        raise ValueError("file set differs")
    digests = {}
    for name in NAMES:
        expected = git_blob(ref, name)
        if (out/name).read_bytes() != expected:
            raise ValueError(ref + ": " + name + " differs")
        digests[name] = sha(expected)
    obj, props = current(model, family)
    colors = [props["backgroundColor"]] + ([props["highColor"], props["lowColor"]] if family == "number" else [])
    verify_frames(fw, out, (obj["width"], obj["height"]), colors, model["pages"][0]["name"])
    record = git_blob(ref, "ControlPara.dat")
    modeled = verify_record(record, model, family)
    row = dict(reference=ref, sourceSha256=sha(source), wholeFilesEqual=digests, firmwareDecodedFrames=len(colors)+1,
               originalControlSha256=sha(record), matchedControlFields=modeled,
               matchedControlBytes=sum(len(bytes.fromhex(x)) for x in modeled.values()))
    if variant == "default":
        row["sameSourceNativeCompiles"] = []
        for role in ("lifecycle", "native-baseline"):
            other = f"docs/stage4-evidence/{family}/physical/{family}-{role}-r1/compiled/physical-native-reference"
            if git_blob(other, "input-native.sm") != source:
                raise ValueError("the repeat source differs from the base")
            measured = {name: sha(git_blob(other, name)) for name in NAMES}
            if measured != digests:
                raise ValueError("the five-file build differs from an additional original run")
            repeated = git_blob(other, "ControlPara.dat")
            offsets = [i for i, (a, b) in enumerate(zip(record, repeated)) if a != b]
            if not offsets or verify_record(repeated, model, family) != modeled:
                raise ValueError("repeat control fields differ or repeat proves no independent drift")
            row["sameSourceNativeCompiles"].append(dict(reference=other, wholeFilesEqual=measured,
                  originalControlSha256=sha(repeated), differentControlOffsets=offsets))
        page = model["pages"][0]
        page["name"] = "测试页" + family
        obj["width"], obj["height"] = 133, 61
        props["fontSize"] = 24
        props["backgroundColor"] = "#132b6d"
        colors = [props["backgroundColor"]]
        if family == "number":
            props["highColor"], props["lowColor"] = "#308ef1", "#e61837"
            colors.extend([props["highColor"], props["lowColor"]])
        for attached in (True, False):
            changed = deepcopy(model)
            if not attached:
                changed.pop("nativeSourceSnapshot", None)
                changed.pop("nativeSourceWarnings", None)
            ksp.write_text(json.dumps(changed, ensure_ascii=False), encoding="utf8")
            destination = work / ("attached" if attached else "fresh")
            run(exe, work, CLI, ksp, destination)
            verify_frames(fw, destination, (133, 61), colors, page["name"])
        # The V122 writer supports background, size and geometry changes in all three display families.
        saved = deepcopy(model)
        _, saved_props = current(saved, family)
        serialized_colors = [saved_props["backgroundColor"]]
        if family == "number":
            saved_props["highColor"], saved_props["lowColor"] = "#00ffff", "#ff0000"
            serialized_colors.extend([saved_props["highColor"], saved_props["lowColor"]])
        ksp.write_text(json.dumps(saved, ensure_ascii=False), encoding="utf8")
        run(exe, work, "--export-native-project", ksp, work/"changed.sm")
        run(exe, work, CLI, work/"changed.sm", work/"reopened")
        verify_frames(fw, work/"reopened", (133, 61), serialized_colors, page["name"])
        row["editedSourceBackedAndFresh"] = dict(size=[133,61], rgb565=[color_word(c) for c in colors], title=page["name"])
        row["editedReopenedNativeSm"] = sha((work/"changed.sm").read_bytes())
        # An imported scalar unknown to the editable model must still be checked by the source profile.
        raw_index = layout(exe, work, sm)
        hidden = {"number": "object[0].display.11", "ascii": "object[0].display.9",
                  "time": "object[0].display.13"}[family]
        (work/"unsupported.sm").write_bytes(edit(source, raw_index, {hidden: 3}))
        run(exe, work, CLI, work/"unsupported.sm", work/"invalid-source", success=False)
        if (work/"invalid-source").exists():
            raise ValueError("an unsupported source field was ignored")
        row["hiddenSourceChangeRefused"] = hidden
        props["opacity"] = 128
        ksp.write_text(json.dumps(model, ensure_ascii=False), encoding="utf8")
        run(exe, work, CLI, ksp, work/"invalid", success=False)
        if (work/"invalid").exists():
            raise ValueError("rejected style published a partial directory")
        run(exe, work, CLI, sm, out, success=False)
        if any(sha((out/name).read_bytes()) != value for name, value in digests.items()):
            raise ValueError("an existing destination was modified")
        row["translucencyAndDestinationRefused"] = True
    return row


def main():
    """Run all six original cases and retain one machine-readable receipt."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--scratch", type=Path, default=ROOT/".local")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    exe = args.exe.resolve()
    fw = (ROOT/"bin/AMZ070W01.bin").read_bytes()
    if sha(fw) != "7f625b7a967c1ecc0ff5db1fba4d6ed68cb1d9474216e87009395f1d53a88db5":
        raise ValueError("different AMZ firmware")
    rows = []
    for family in ("number", "ascii", "time"):
        for variant in ("default", "bilingual-style24"):
            with TemporaryDirectory(dir=args.scratch) as temp:
                rows.append(exercise(exe, fw, Path(temp).resolve(), family, variant))
    result = dict(compiler=sha(exe.read_bytes()), originalRevision=REVISION, firmware=sha(fw), cases=rows)
    if args.receipt:
        args.receipt.write_text(json.dumps(result, ensure_ascii=False, indent=1) + "\n", "utf8")
    print(json.dumps(dict(sources=len(rows), originalRuns=sum(1+len(r.get("sameSourceNativeCompiles",[])) for r in rows),
                          equalFiles=sum(len(r["wholeFilesEqual"])*(1+len(r.get("sameSourceNativeCompiles",[]))) for r in rows),
                          controlFieldBytes=sum(r["matchedControlBytes"]*(1+len(r.get("sameSourceNativeCompiles",[]))) for r in rows))))


if __name__ == "__main__":
    main()
