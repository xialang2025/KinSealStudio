"""Verify two V122 overlapping language-text packages, including their original compressed resources."""
# 20261128-14 add
import argparse
from copy import deepcopy
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile

from verify_native_drawing_drafts import REVISION, ROOT, git_blob


def invoke(exe, path, *args, accepted=True):
    """Run public CLI actions without a window on the Windows font backend used by the original samples."""
    result = subprocess.run([str(exe), *map(str, args)], cwd=path, timeout=90, capture_output=True,
                            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
                            env=dict(os.environ, QT_QPA_PLATFORM="windows", TEMP=str(path), TMP=str(path)))
    if (result.returncode == 0) != accepted:
        raise ValueError(str(args[0]) + " failed: " + result.stdout.decode("utf8", "replace"))
    return result


def inspect_case(exe, path, name):
    """Compare entire pixel and index files, each packed frame, and the interpreted parts of both records."""
    folder = "docs/stage3-evidence/native/compiled/static-odd2-" + name
    sm = path / (name + ".sm")
    sm.write_bytes(git_blob(folder, "input-native.sm"))
    findings = []
    for option in ("--compile-project", "--compile-native-media-parts"):
        out = path / (name + option)
        invoke(exe, path, option, sm, out)
        sizes = {}
        for member in ("ControlPixel.dat", "PicLenPara.dat", "SheetPixel.dat"):
            original = git_blob(folder, member)
            if (out / member).read_bytes() != original:
                raise ValueError(name + " differs in " + member)
            sizes[member] = {"bytes": len(original), "sha256": hashlib.sha256(original).hexdigest()}
        indices = (out / "PicLenPara.dat").read_bytes()[2400:]
        pixels = (out / "ControlPixel.dat").read_bytes()
        start = 0
        for index, (reserved, padded, packed) in enumerate(struct.iter_unpack("<III", indices)):
            original = git_blob(folder, f"frame-{index}.lzo")
            if reserved or packed != len(original) or pixels[start:start+packed] != original:
                raise ValueError(f"unexpected frame {index} in {name}")
            start += padded
        if len(indices) != 48 or start != len(pixels):
            raise ValueError("unexpected frame count or trailing resource data")
        original = git_blob(folder, "ControlPara.dat")
        controls = (out / "ControlPara.dat").read_bytes()
        if len(controls) != 1280 or len(original) != len(controls):
            raise ValueError("expected two original 640-byte text controls")
        for first in (0, 640):
            for left, right in ((0, 3), (4, 22), (0x100, 0x104)):
                if controls[first+left:first+right] != original[first+left:first+right]:
                    raise ValueError("control type, geometry, pixel range, visibility or language count differs")
        findings.append(dict(option=option, files=sizes,
                             controlDifferentBytes=sum(a != b for a, b in zip(original, controls))))
    return dict(name=name, sourceSha256=hashlib.sha256(sm.read_bytes()).hexdigest(), routes=findings)


def negative_cases(exe, path):
    """Keep language-dependent, conditional, partially overlapping and three-layer layouts outside this slice."""
    model = path / "positive.ksp"
    invoke(exe, path, "--migrate-project", path / "transparent-top.sm", model)
    base = json.loads(model.read_bytes())
    names = ("lower-language-changes", "upper-conditional", "partial-overlap", "third-layer", "different-opacity")
    for name in names:
        model = deepcopy(base)
        objects = model["pages"][0]["objects"]
        lower, top = objects
        if name == "lower-language-changes":
            lower["staticText"]["texts"]["English"] = "A"
        elif name == "upper-conditional":
            top["staticText"].update(visibilityEnabled=True, bitEnabled=True)
        elif name == "partial-overlap":
            top["x"] = lower["x"] - 1
        elif name == "third-layer":
            extra = deepcopy(top)
            extra.update(id="third-text", name="Third text", layer=3)
            objects.append(extra)
        else:
            top["staticText"].update(style=1, opacity=64)
        project = path / (name + ".ksp")
        project.write_text(json.dumps(model, ensure_ascii=False), encoding="utf8")
        output = path / name
        result = invoke(exe, path, "--compile-project", project, output, accepted=False)
        if output.exists() or "重叠" not in json.loads(result.stdout)["message"]:
            raise ValueError("incorrect refusal for " + name)
    return list(names)


def main():
    """Check both compilers and verify that unsupported overlays still fail before publishing a device image."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--scratch", type=Path, default=ROOT / "build")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    exe = args.exe.resolve()
    with tempfile.TemporaryDirectory(dir=args.scratch) as work:
        path = Path(work).resolve()
        cases = [inspect_case(exe, path, n) for n in ("transparent-top", "opacity128-top")]
        rejected = negative_cases(exe, path)
    report = dict(revision=REVISION, exeSha256=hashlib.sha256(exe.read_bytes()).hexdigest(),
                  cases=cases, rejected=rejected)
    if args.receipt:
        args.receipt.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf8")
    print(f"{len(cases)} original projects, 2 compiler routes, 4 compressed frames each, {len(rejected)} rejected layouts.")


if __name__ == "__main__":
    main()
