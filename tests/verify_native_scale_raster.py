"""Compile original single-scale pages and verify complete vendor files and decoded physical pixels."""
# 20261128-34 add
import argparse
from copy import deepcopy
import hashlib
import json
from pathlib import Path
from tempfile import TemporaryDirectory

from verify_amz_firmware_page_images import firmware_frame
from verify_amz_local_page_io import edit, layout
from verify_native_drawing_drafts import ROOT, REVISION, git_blob, run

SOURCE = "docs/stage4-evidence/scale/native"
CASES = ("default", "direction-down", "equal-zero", "main6", "position-top", "sub4")
FILES = ("SheetPixel.dat", "PicLenPara.dat", "AddrPara.dat", "SheetPara.dat")


def sha(data):
    """Identify the entire file or firmware-decoded pixel buffer."""
    return hashlib.sha256(data).hexdigest()


def verify(exe, work):
    """Compare independent V5.1 outputs and then verify two edits made in the normal canvas model."""
    fw = (ROOT / "bin/AMZ070W01.bin").read_bytes()
    rows, pages = [], {}
    for name in CASES:
        sm = work / (name + ".sm")
        source = git_blob(SOURCE, "scale-" + name + ".sm")
        sm.write_bytes(source)
        folder = SOURCE + "/compiled/scale-" + name + "-r1"
        originals = {file: git_blob(folder, file) for file in FILES}
        original_page = firmware_frame(fw, originals["SheetPixel.dat"])
        pages[name] = original_page
        outputs = {}
        for command in ("--compile-project", "--compile-native-media-parts"):
            output = work / (name + command)
            run(exe, work, command, sm, output)
            report = {}
            for file, ref in originals.items():
                actual = (output / file).read_bytes()
                mismatch = [i for i, (a, b) in enumerate(zip(actual, ref)) if a != b]
                if len(actual) != len(ref) or (mismatch and (file != "SheetPara.dat" or mismatch != [59])):
                    raise ValueError((name, command, file, len(actual), len(ref), mismatch[:8]))
                report[file] = dict(generated=sha(actual), original=sha(ref), differences=mismatch)
            if firmware_frame(fw, (output / "SheetPixel.dat").read_bytes()) != original_page:
                raise ValueError("Firmware decoded a different scale in " + name)
            if (output / "ControlPara.dat").exists() or (output / "ControlPixel.dat").exists():
                raise ValueError("Native static scales have no dynamic record or control frame")
            outputs[command] = report
        rows.append(dict(source=folder, sm=sha(source), rgb565=sha(original_page), routes=outputs))
    ksp = work / "edit.ksp"
    run(exe, work, "--migrate-project", work / "default.sm", ksp)
    model = json.loads(ksp.read_bytes())
    obj = model["pages"][0]["objects"][0]
    obj["scale"]["direction"] = 1
    obj["x"] += 7
    obj["y"] += 9
    ksp.write_text(json.dumps(model, ensure_ascii=False), encoding="utf8")
    saved = work / "moved.sm"
    run(exe, work, "--export-native-project", ksp, saved)
    source_page = pages["direction-down"]
    background = source_page[2*(800+1):2*(800+2)]
    expected = bytearray(source_page)
    for y in range(1, 480):
        for x in range(1, 800):
            off = 2 * (y * 800 + x)
            if source_page[off:off+2] != background:
                expected[off:off+2] = background
    for y in range(110, 230):
        for x in range(150, 370):
            off = 2 * (y * 800 + x)
            if source_page[off:off+2] != background:
                dst = off + 2 * (9 * 800 + 7)
                expected[dst:dst+2] = source_page[off:off+2]
    for path in (ksp, saved):
        output = work / ("moved-" + path.suffix[1:])
        run(exe, work, "--compile-project", path, output)
        if firmware_frame(fw, (output / "SheetPixel.dat").read_bytes()) != expected:
            raise ValueError("Edited scale was not regenerated from its current geometry and direction")
    invalid = deepcopy(model)
    invalid["pages"][0]["objects"][0]["scale"]["fontSize"] = 18
    ksp.write_text(json.dumps(invalid), encoding="utf8")
    rejected = work / "unproved"
    run(exe, work, "--compile-project", ksp, rejected, success=False)
    if rejected.exists():
        raise ValueError("Unverified style published output")
    # The source editor preserves additional original pen and typeface settings. The raster writer must see them.
    historical = work / "default.sm"
    spans = layout(exe, work, historical)
    for index, field in enumerate(("object[0].child[1].stroke.7", "object[0].child[2].font.tail.5")):
        mutated = work / ("altered" + str(index) + ".sm")
        dest = work / ("unsupported" + str(index))
        mutated.write_bytes(edit(historical.read_bytes(), spans, {field: 2 if index == 0 else 75}))
        run(exe, work, "--compile-project", mutated, dest, success=False)
        if dest.exists():
            raise ValueError("Unverified source style published output")
    return dict(exe=sha(exe.read_bytes()), revision=REVISION, firmware=sha(fw), cases=rows, rejectedStyles=3,
                movedReversedPixelHash=sha(expected), comparedPixelsPerRoute=800*480*len(CASES))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--scratch", required=True, type=Path)
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    with TemporaryDirectory(dir=args.scratch.resolve()) as directory:
        data = verify(args.exe.resolve(), Path(directory))
    if args.report:
        args.report.write_text(json.dumps(data, ensure_ascii=False, indent=1) + "\n", encoding="utf8")
    print(len(data["cases"]), "scale sources, two compiler routes and two edited project paths")
