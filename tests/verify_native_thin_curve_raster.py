"""Compare public compiler outputs for the original 1 px curve page and controlled line samples."""
# 20261128-11 add
import argparse
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import tempfile

from verify_native_drawing_drafts import ROOT, REVISION, git_blob, run

REFERENCE = ROOT / "docs/文件规则/样例/V122细线曲线页RGB565原厂对照_20261128.json"


def verify(exe, work):
    """Compile the archived SM through both normal entries and the controlled integer endpoints through Lite."""
    reference = json.loads(REFERENCE.read_bytes())
    if reference["originalRevision"] != REVISION:
        raise ValueError("the pinned original revision changed")
    original = {}
    for name, signature in reference["originalFiles"].items():
        data = git_blob(reference["originalFolder"], name)
        if len(data) != signature["bytes"] or hashlib.sha256(data).hexdigest() != signature["sha256"]:
            raise ValueError("original reference changed: " + name)
        original[name] = data
    source = work / "curve.sm"
    source.write_bytes(original["input-native.sm"])
    for mode in ("--compile-project", "--compile-native-media-parts"):
        output = work / mode.lstrip("-")
        run(exe, work, mode, source, output)
        for name in ("SheetPixel.dat", "PicLenPara.dat"):
            if (output / name).read_bytes() != original[name]:
                raise ValueError(mode + " generated different bytes for " + name)
    model = work / "curve.ksp"
    run(exe, work, "--migrate-project", source, model)
    base = json.loads(model.read_bytes())
    for index, expected in enumerate(reference["controlled"]):
        project = deepcopy(base)
        obj = project["pages"][0]["objects"][0]
        ax, ay, bx, by = expected["endpoints"]
        width, height = max(1, abs(ax-bx)), max(1, abs(ay-by))
        left, top = min(ax, bx), min(ay, by)
        obj.update(x=left, y=top, width=width, height=height)
        obj["graphic"]["points"] = [[(x-left)/width, (y-top)/height] for x, y in ((ax, ay), (bx, by))]
        model.write_text(json.dumps(project, ensure_ascii=False), encoding="utf8")
        output = work / f"controlled-{index}"
        run(exe, work, "--compile-project", model, output)
        encoded = (output / "SheetPixel.dat").read_bytes()
        if len(encoded) != expected["sheetPixelBytes"] or hashlib.sha256(encoded).hexdigest() != expected["sheetPixelSha256"]:
            raise ValueError("unexpected SheetPixel.dat for endpoints " + str(expected["endpoints"]))
    return len(reference["controlled"])


def main():
    """Run the device-format regression without opening windows or accessing USB."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--scratch", type=Path, default=ROOT / "build")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(dir=args.scratch) as tmp:
        count = verify(args.exe.resolve(), Path(tmp).resolve())
    print(f"Two production entries match the archived curve DAT files; {count} controlled line frames match.")


if __name__ == "__main__":
    main()
