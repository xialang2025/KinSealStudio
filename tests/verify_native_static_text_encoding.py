"""Check V122 static-text source fields and complete raster files against frozen V5.1 output."""
# 20261128-10 add
import argparse
import hashlib
import json
import os
import struct
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
REV = "e3b72657"
MARKERS = "docs/stage4-evidence/screen-width-diagnosis/physical/"


def verify(exe, scratch):
    """Read original outputs only on the comparison side and exercise the public project entry points."""
    rows = []
    with tempfile.TemporaryDirectory(dir=scratch) as tmp:
        work = Path(tmp)

        def run(*args):
            proc = subprocess.run([str(exe), *map(str, args)], cwd=work, capture_output=True, timeout=120,
                                  env=dict(os.environ, TMP=tmp, TEMP=tmp, TMPDIR=tmp),
                                  creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            if proc.returncode:
                raise RuntimeError(proc.stdout.decode("utf8", "replace") + proc.stderr.decode("utf8", "replace"))
            return json.loads(proc.stdout)

        for tag in ("marker-a-r1", "marker-b-fixed-r1"):
            prefix = REV + ":" + MARKERS + tag + "/compiled/physical-native-reference/"
            sm = work / (tag + ".sm")
            model = sm.with_suffix(".ksp")
            sm.write_bytes(subprocess.check_output(["git", "cat-file", "blob", prefix + "input-native.sm"], cwd=ROOT))
            run("--migrate-project", sm, model)
            project = json.loads(model.read_bytes())
            assert project["pages"][0]["objects"][0]["staticText"]["alignment"] == 0
            saved = work / (tag + "-roundtrip.sm")
            run("--export-native-project", model, saved)
            assert sm.read_bytes() == saved.read_bytes()
            folder = work / tag
            run("--compile-project", sm, folder)
            outputs = {}
            for filename in ("SheetPixel.dat", "PicLenPara.dat"):
                expected = subprocess.check_output(["git", "cat-file", "blob", prefix + filename], cwd=ROOT)
                assert (folder / filename).read_bytes() == expected, (tag, filename)
                outputs[filename] = hashlib.sha256(expected).hexdigest()
            rows.append(dict(project=tag, source=hashlib.sha256(sm.read_bytes()).hexdigest(),
                             sourceRoundTrip=True, identical=outputs))

        # A two-language odd-width sprite can have identical LZO packets and differing alignment bytes.
        # Use the original length table to delimit the packets, then compare both the packets and tails.
        folder = work / "odd-sprites"
        prefix = REV + ":docs/stage3-evidence/native/compiled/static-odd2-base-only/"
        sm = work / "odd.sm"
        sm.write_bytes(subprocess.check_output(["git", "cat-file", "blob", prefix + "input-native.sm"], cwd=ROOT))
        run("--compile-project", sm, folder)
        vendor_index = subprocess.check_output(["git", "cat-file", "blob", prefix + "PicLenPara.dat"], cwd=ROOT)
        vendor_pixels = subprocess.check_output(["git", "cat-file", "blob", prefix + "ControlPixel.dat"], cwd=ROOT)
        generated = (folder / "ControlPixel.dat").read_bytes()
        assert vendor_index == (folder / "PicLenPara.dat").read_bytes() and len(vendor_index) == 2424
        offset = 0
        frames = []
        for _, aligned, length in struct.iter_unpack("<III", vendor_index[2400:]):
            assert 0 < length <= aligned and aligned == ((length + 31) & ~31)
            end = offset + length
            assert generated[offset:end] == vendor_pixels[offset:end]
            frames.append(dict(offset=offset, compressedBytes=length, storageBytes=aligned,
                               differingPadding=[pos for pos in range(end, offset + aligned)
                                                 if generated[pos] != vendor_pixels[pos]]))
            offset += aligned
        assert offset == len(generated) == len(vendor_pixels)
        assert generated == vendor_pixels  # 20261128-12 add: Check the alignment tails and the LZO bodies.
        sprite = dict(source=hashlib.sha256(sm.read_bytes()).hexdigest(), frames=frames,
                      wholeFileEqual=vendor_pixels == generated)

        # New two-language projects must carry the editor index and the graphic code in their own slots.
        project.pop("nativeSourceSnapshot")
        project.pop("nativeSourceWarnings", None)
        project["languages"].append("English")
        props = project["pages"][0]["objects"][0]["staticText"]
        props["texts"]["English"] = "English"
        for index, graphic_code in ((0, 5), (1, 4), (2, 6)):
            for fill in (0, 1, 2):
                props.update(alignment=index, style=fill)
                model.write_text(json.dumps(project, ensure_ascii=False), encoding="utf8")
                run("--export-native-project", model, saved)
                raw = saved.read_bytes()
                fields = {field["path"]: raw[field["start"]:field["end"]]
                          for field in run("--inspect-native-source", saved)["fields"]}
                expected = {"static.head.2": index, "text.tail.3": graphic_code, "text.tail.4": fill,
                            "rectangle.8": fill, "static.head.10": fill}
                for language in range(2):
                    expected[f"static.194[{language}].8"] = graphic_code
                    expected[f"static.194[{language}].15"] = fill
                for name, value in expected.items():
                    assert int.from_bytes(fields["object[0]." + name], "big") == value, (index, fill, name)
                run("--migrate-project", saved, model)
                actual = json.loads(model.read_bytes())["pages"][0]["objects"][0]["staticText"]
                assert (actual["alignment"], actual["style"], actual["texts"]) == (index, fill, props["texts"])
    return dict(revision=REV, exeSha256=hashlib.sha256(exe.read_bytes()).hexdigest(), originalPages=rows,
                independentBilingualEdits=9, oddWidthSprite=sprite)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--scratch", required=True, type=Path)
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    receipt = verify(args.exe.resolve(), args.scratch.resolve())
    result = json.dumps(receipt, ensure_ascii=False, indent=2)
    if args.report:
        args.report.write_text(result + "\n", encoding="utf8")
    print(result)
