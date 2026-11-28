"""Distinguish original created and reopened scale pages using run context and complete native outputs."""
# 20261128-37 add
import argparse
import hashlib
import json
from pathlib import Path
import struct
import tempfile
import time
import zlib

from verify_amz_firmware_page_images import firmware_frame
from verify_amz_local_page_io import layout
from verify_native_drawing_drafts import ROOT, REVISION, git_blob, run


SCALE = "docs/stage4-evidence/scale"
CREATED = SCALE + "/native/compiled/scale-default-r1"
PHYSICAL = ("scale-lifecycle-r1", "scale-native-baseline-r1")
FILES = ("SheetPixel.dat", "PicLenPara.dat", "AddrPara.dat", "SheetPara.dat")
ROUTE = "--compile-native-media-parts"


def sha(data):
    """Identify a complete input or output file."""
    return hashlib.sha256(data).hexdigest()


def fetch(directory, name):
    """Read one independent original comparison from the archived V5.1 run."""
    return {file: git_blob(directory, file) for file in name}


def compile_files(exe, sm, folder, option, *args):
    """Produce a new directory through the selected public compiler entry point."""
    run(exe, sm.parent, option, sm, folder, *args)
    return {file: (folder / file).read_bytes() for file in FILES}


def compare(actual, reference):
    """Require complete equality of the page pixels, frame lengths, address table and page record."""
    results = {}
    for name in FILES:
        if len(actual[name]) != len(reference[name]):
            raise ValueError("different length in " + name)
        differences = [i for i, (a, b) in enumerate(zip(actual[name], reference[name])) if a != b]
        if differences:
            raise ValueError((name, differences[:12]))
        results[name] = {"actual": sha(actual[name]), "reference": sha(reference[name]), "differentOffsets": differences}
    return results


def check(exe, work):
    """Recompile three original runs, then disprove profile selection by path, extension or control type alone."""
    original = git_blob(SCALE + "/native", "scale-default.sm")
    sm = work / "renamed-for-test.sm"
    sm.write_bytes(original)
    native = fetch(CREATED, FILES)
    fw = (ROOT / "bin/AMZ070W01.bin").read_bytes()
    created_pixels = firmware_frame(fw, native["SheetPixel.dat"])
    created_log = json.loads(git_blob(SCALE + "/native", "native-probe-scale-default-r1.json"))
    if created_log["nativeCompilations"][0]["sourceSha256"] != sha(original):
        raise ValueError("the creation compile concerns a different source")
    created = {}
    for name, option, args in (("usb", "--compile-project", ()), ("media", ROUTE, ()),
                               ("directoryOnly", ROUTE, ("--native-work-directory", "Z:\\offline-scale"))):
        created[name] = compare(compile_files(exe, sm, work / name, option, *args), native)
    fields = layout(exe, work, sm)
    runs = []
    for name in PHYSICAL:
        folder = SCALE + "/physical/" + name
        log = json.loads(git_blob(folder, "native-probe-" + name + ".json"))
        parts = folder + "/compiled/physical-native-reference"
        reference = fetch(parts, FILES)
        source = git_blob(parts, "input-native.sm")
        opened = log["workingCopyAfter"]
        if source != original or any(value != sha(source) for value in
                (log["input"]["sha256"], opened["sha256"], log["nativeCompilations"][0]["sourceSha256"])):
            raise ValueError("original native open/compile and source are not the same")
        args = ["--native-opened-project", opened["path"]]
        compared = compare(compile_files(exe, sm, work / name, ROUTE, *args), reference)
        pixels = firmware_frame(fw, reference["SheetPixel.dat"])
        differences = [(i//2 % 800, i//1600) for i in range(0, len(pixels), 2)
                       if pixels[i:i+2] != created_pixels[i:i+2]]
        expected = [(241, 128), (180, 142), (241, 156), (180, 171), (241, 185), (180, 200), (241, 214)]
        if differences != expected:
            raise ValueError("unexpected native endpoint changes")
        for x, y in expected:
            i = 2 * (800 * y + x)
            if created_pixels[i:i+2] != bytes(2) or pixels[i:i+2] != bytes.fromhex("ffc6"):
                raise ValueError("the original reopened endpoint did not change from black to background")
        archived = zlib.decompress(git_blob(parts, "upLoad.knr")[4:])
        saved = work / "saved.sm"
        saved.write_bytes(archived)
        after = layout(exe, work, saved)
        varied = [key for key, f in fields.items()
                  if original[f["start"]:f["end"]] != archived[after[key]["start"]:after[key]["end"]]]
        if set(varied) != {"config.head.12", "trailer.settings.0"}:
            raise ValueError("cached child geometry changed in the archived source")
        created_scene = created_log["sceneReads"][0]["data"]["items"]
        opened_scene = log["sceneReads"][0]["data"]["items"]
        def lines(items):
            return [(i["rect"], i["transform"]) for i in items if i["parent"] != "0x0" and
                    (i["rect"][2] == 0 or i["rect"][3] == 0)]
        if lines(opened_scene) != lines(created_scene) or len(lines(opened_scene)) != 8:
            raise ValueError("original source does not have eight identical cached line geometries")
        runs.append({"run": name, "source": sha(source), "openedProject": opened["path"], "files": compared,
                     "changedPixels": expected, "originalArchivedFieldsChanged": varied, "sameSceneLineBounds": 8})

    # An unrelated valid original path with a different basename still selects explicit historical reopen.
    other_path = ["--native-opened-project", "Y:\\manual-check\\other-project.sm"]
    compare(compile_files(exe, sm, work / "different-name", ROUTE, *other_path), reference)
    negative = []
    for name in ("direction-down", "position-top", "sub4", "main6", "equal-zero"):
        sm.write_bytes(git_blob(SCALE + "/native", "scale-" + name + ".sm"))
        ref = fetch(SCALE + "/native/compiled/scale-" + name + "-r1", FILES)
        compare(compile_files(exe, sm, work / ("control-" + name), ROUTE, *other_path), ref)
        negative.append(name)
    # Both edits preserve the root dimensions and nominal scale settings. The exact child checks must still detect them.
    for name, field in (("edited-tick", "object[0].child[1].element[1].1"),
                        ("edited-text", "object[0].child[4].text.tail.10")):
        start = fields[field]["start"]
        changed = bytearray(original)
        struct.pack_into(">d", changed, start, struct.unpack_from(">d", changed, start)[0] + 1.0)
        sm.write_bytes(changed)
        ordinary = compile_files(exe, sm, work / (name + "-usb"), "--compile-project")
        explicit = compile_files(exe, sm, work / (name + "-media"), ROUTE, *other_path)
        if ordinary != explicit or ordinary["SheetPixel.dat"] == native["SheetPixel.dat"]:
            raise ValueError("unverified changed geometry used the reopened tick rule: " + name)
        negative.append(name)
    return {"revision": REVISION, "sourceSha256": sha(original), "creation": created, "reopened": runs,
            "renamedExplicitPathExact": True, "unsupportedLayoutsRetainExistingRaster": negative,
            "firmwareSha256": sha(fw), "usbAccessed": False}


if __name__ == "__main__":
    args = argparse.ArgumentParser(description=__doc__)
    args.add_argument("--exe", required=True, type=Path)
    args.add_argument("--receipt", type=Path)
    options = args.parse_args()
    tmp = tempfile.TemporaryDirectory(prefix="scale-reopen-", dir=ROOT / "build")
    try:
        result = check(options.exe.resolve(), Path(tmp.name))
    finally:
        for attempt in range(6):
            try:
                tmp.cleanup()
                break
            except OSError:
                if attempt == 5:
                    raise
                time.sleep(0.2 * (attempt + 1))
    if options.receipt:
        options.receipt.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf8")
    print("Compared one created run, two original reopened runs and seven non-default geometry/layout controls")
