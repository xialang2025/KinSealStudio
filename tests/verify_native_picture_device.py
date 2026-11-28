"""Verify both output paths against the five original V122 type-15 compilations."""
# 20261128-21 add
import argparse
import base64
import hashlib
import io
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import zlib


ROOT = Path(__file__).resolve().parents[1]
REFERENCE = "e3b72657:docs/stage4-evidence/picture/native/compiled/picture-"
CASES = ("static-red", "multiple-time", "multiple-bit", "multiple-numeric", "distinct-red-green")
FILES = ("ControlPixel.dat", "PicLenPara.dat", "SheetPara.dat", "SheetPixel.dat", "AddrPara.dat")


def original(case, name):
    """Read the full independently committed native output."""
    return subprocess.check_output(["git", "show", REFERENCE + case + "/" + name], cwd=ROOT)


def run(exe, arguments, scratch, expected=True):
    """Run the production command with temporary storage confined to the active verification directory."""
    env = dict(os.environ, TMP=str(scratch), TEMP=str(scratch), TMPDIR=str(scratch))
    result = subprocess.run([str(exe), *map(str, arguments)], env=env, capture_output=True, timeout=60,
                            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
    response = json.loads(result.stdout)
    if (result.returncode == 0) != expected or response.get("ok") != expected:
        raise ValueError(f"{arguments[0]}: {response}")
    return response


def check(exe, scratch):
    """Compare whole images and the active type-specific header from the same original run."""
    rows = []
    for case in CASES:
        source = scratch / (case + ".sm")
        source.write_bytes(original(case, "input-native.sm"))
        native_control = original(case, "ControlPara.dat")
        for media in (False, True):
            mode = "media" if media else "usb"
            parts = scratch / (case + "-" + mode)
            args = ["--compile-native-media-parts" if media else "--compile-project", source, parts]
            if media:
                args.append("--without-com1-driver")
            run(exe, args, scratch)
            hashes = {}
            for name in FILES + (("load.bin",) if media else ()):
                output = (parts / name).read_bytes()
                if output != original(case, name):
                    raise ValueError(f"{case}/{mode}/{name} differs from the original")
                hashes[name] = hashlib.sha256(output).hexdigest()
            output = (parts / "ControlPara.dat").read_bytes()
            if len(output) != 640 or len(native_control) != 640:
                raise ValueError(f"{case}/{mode}: incorrect control size")
            for start, end in ((0, 20), (0x100, 0x110), (0x114, 0x118)):
                if output[start:end] != native_control[start:end]:
                    raise ValueError(f"{case}/{mode}: control fields differ from 0x{start:x}")
            diff = [i for i in range(640) if output[i] != native_control[i]]
            rows.append(dict(source=case, mode=mode, originalSourceSHA256=hashlib.sha256(source.read_bytes()).hexdigest(),
                             exactFiles=hashes, controlDifferenceBytes=len(diff), firstControlDifference=diff[0] if diff else None))
    return rows


def check_edits(exe, work):
    """Test independently edited addresses/intervals and make sure unverified active options fail before publication."""
    source = work / "multiple-bit.sm"
    model = work / "edited.ksp"
    run(exe, ["--migrate-project", source, model], work)
    project = json.loads(model.read_bytes())
    obj = project["pages"][0]["objects"][0]
    properties = obj["imageDisplay"]
    for condition, text, wire in ((2, "LB17", 1), (1, "LW429", 2), (0, "", 0)):
        properties.update(condition=condition, monitorAddress=text, interval=19)
        obj["address"] = text
        model.write_text(json.dumps(project, ensure_ascii=False), encoding="utf-8")
        parts = work / f"edited-{condition}"
        run(exe, ["--compile-project", model, parts], work)
        encoded = (parts / "ControlPara.dat").read_bytes()
        if struct.unpack_from("<6H", encoded, 0x100) != (1, wire, 0, 2, 0, 19):
            raise ValueError("edited type-15 mode, state count or interval differs")
        if text and (encoded[0x10d] != ord(text[1]) or struct.unpack_from("<I", encoded, 0x114)[0] != int(text[2:])):
            raise ValueError("edited type-15 local address differs")
    properties["visibility"]["visibilityEnabled"] = True
    properties["visibility"]["bitEnabled"] = True
    model.write_text(json.dumps(project, ensure_ascii=False), encoding="utf-8")
    rejected = work / "unsupported"
    response = run(exe, ["--compile-project", model, rejected], work, expected=False)
    if rejected.exists() or "显现" not in response["message"]:
        raise ValueError("unsupported visibility was published or returned the wrong diagnostic")


def reference_solid_gif_table():
    """Rebuild the two full-color PNGs from the independent GIF file, including native chunk CRCs and offsets."""
    from PIL import Image

    data = subprocess.check_output(["git", "show", "e3b72657:docs/stage4-evidence/gif/native/probe-red-green.gif"], cwd=ROOT)
    source = Image.open(io.BytesIO(data))
    frames = []
    def chunk(tag, content):
        return struct.pack(">I", len(content)) + tag + content + struct.pack(">I", zlib.crc32(tag + content))
    for index in range(source.n_frames):
        source.seek(index)
        image = source.convert("RGB")
        width, height = image.size
        if len(image.getcolors(2)) != 1:
            raise ValueError("solid original GIF sample changed")
        row = bytes(image.getpixel((0, 0))) + bytes((width - 1) * 3)
        # Sub filter for the first row, Up filter for identical subsequent rows.
        filtered = b"\x01" + row + (b"\x02" + bytes(3 * width)) * (height - 1)
        compressor = zlib.compressobj(level=6, wbits=12)
        pixels = compressor.compress(filtered) + compressor.flush()
        frames.append(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)) +
                      chunk(b"pHYs", struct.pack(">IIB", 3780, 3780, 1)) + chunk(b"IDAT", pixels) + chunk(b"IEND", b""))
    offset = (len(frames) + 1) * 4
    header = bytearray(struct.pack("<I", len(frames)))
    for frame in frames:
        header.extend(struct.pack("<I", offset))
        offset += len(frame)
    return bytes(header) + b"".join(frames)


def check_gifs(exe, work):
    """Compare the two original 0x22 sources, edited source reload, and the independently generated native frame files."""
    rows = []
    for case in ("gif-default", "gif-bit-LB17-state0"):
        ref = "e3b72657:docs/stage4-evidence/gif/native/compiled/" + case + "/"
        def native(name):
            return subprocess.check_output(["git", "show", ref + name], cwd=ROOT)
        source = work / (case + ".sm")
        model = work / (case + ".ksp")
        restored = work / (case + "-restored.sm")
        source.write_bytes(native("input-native.sm"))
        run(exe, ["--migrate-project", source, model], work)
        run(exe, ["--export-native-project", model, restored], work)
        if restored.read_bytes() != source.read_bytes():
            raise ValueError(f"{case}: re-saved source differs")
        original_model = json.loads(model.read_bytes())
        gif = original_model["pages"][0]["objects"][0]["gifDisplay"]
        table = base64.b64decode(gif["gifData"], validate=True)
        if (table != reference_solid_gif_table() or gif["gifData"] != original_model["animations"][gif["animation"]] or
                struct.unpack_from("<III", table) != (2, 12, 126) or len(table) != 241):
            raise ValueError(f"{case}: original two-frame table or binding was altered")
        for media in (False, True):
            mode = "media" if media else "usb"
            parts = work / (case + "-" + mode)
            args = ["--compile-native-media-parts" if media else "--compile-project", source, parts]
            if media:
                args.append("--without-com1-driver")
            run(exe, args, work)
            hashes = {}
            for name in FILES + (("load.bin",) if media else ()):
                output = (parts / name).read_bytes()
                if output != native(name):
                    raise ValueError(f"{case}/{mode}/{name} differs")
                hashes[name] = hashlib.sha256(output).hexdigest()
            actual, expected = (parts / "ControlPara.dat").read_bytes(), native("ControlPara.dat")
            if len(actual) != 640 or actual[:20] != expected[:20] or actual[0x100:0x118] != expected[0x100:0x118]:
                raise ValueError(f"{case}/{mode}: type 34 control fields differ")
            diff = [i for i in range(640) if actual[i] != expected[i]]
            rows.append(dict(source=case, mode=mode, originalSourceSHA256=hashlib.sha256(source.read_bytes()).hexdigest(),
                             exactFiles=hashes, controlDifferenceBytes=len(diff), firstControlDifference=diff[0] if diff else None))
        gif.update(bitControl=True, activeState=1, controlAddress="LB25")
        original_model["pages"][0]["objects"][0]["x"] += 7
        model.write_text(json.dumps(original_model, ensure_ascii=False), encoding="utf-8")
        run(exe, ["--export-native-project", model, restored], work)
        run(exe, ["--migrate-project", restored, model], work)
        obj = json.loads(model.read_bytes())["pages"][0]["objects"][0]
        if obj["x"] != 167 or obj["gifDisplay"] != gif:
            raise ValueError(f"{case}: geometry, edited gate or saved frames differ")
        candidate = bytearray(table)
        struct.pack_into("<I", candidate, 8, len(table) + 1)
        invalid = source.read_bytes().replace(table, candidate)
        if invalid == source.read_bytes():
            raise ValueError(f"{case}: the frame table was not found in the source")
        restored.write_bytes(invalid)
        response = run(exe, ["--migrate-project", restored, work / (case + "-bad.ksp")], work, expected=False)
        if "帧表" not in response["message"]:
            raise ValueError(f"{case}: corrupted frame boundary returned the wrong diagnostic")
    return rows


def check_canvas_poster(exe, work):
    """Open a real native source in the main editor twice and check that changing frame zero changes the painted rectangle."""
    from PIL import Image

    source = work / "gif-default.sm"
    swapped = work / "poster-swapped.sm"
    table = reference_solid_gif_table()
    count, first, second = struct.unpack_from("<III", table)
    reverse = struct.pack("<III", count, first, first + len(table) - second) + table[second:] + table[first:second]
    raw = source.read_bytes()
    if raw.count(table) != 1:
        raise ValueError("unexpected native GIF stream identity")
    swapped.write_bytes(raw.replace(table, reverse))
    captures = []
    env = dict(os.environ, QT_QPA_PLATFORM="offscreen", QT_SCALE_FACTOR="1", TMP=str(work), TEMP=str(work), TMPDIR=str(work))
    for index, case in enumerate((source, swapped)):
        output = work / f"poster-{index}.png"
        subprocess.run([str(exe), "--capture-preview", str(output), str(case)], env=env, capture_output=True,
                       creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0), timeout=60, check=True)
        with Image.open(output) as image:
            captures.append(image.convert("RGB"))
    a, b = captures
    if a.size != b.size:
        raise ValueError("editor capture sizes differ")
    # Restrict comparison to pixels changing red to green; unrelated title bar text is outside this range.
    pixels = sum(first == (255, 0, 0) and second == (0, 255, 0)
                 for first, second in zip(a.getdata(), b.getdata()))
    if pixels != 200 * 100:
        raise ValueError(f"native frame-zero canvas area differs: {pixels} pixels")
    return pixels


def main():
    """Write optional receipts only after the original outputs and edited cases have passed."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--scratch", type=Path, default=ROOT / "build")
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(dir=args.scratch, prefix="picture-device-") as folder:
        rows = check(args.exe.resolve(), Path(folder))
        check_edits(args.exe.resolve(), Path(folder))
        rows.extend(check_gifs(args.exe.resolve(), Path(folder)))
        canvas = check_canvas_poster(args.exe.resolve(), Path(folder))
    if args.report:
        args.report.write_text(json.dumps(dict(nativeRevision="e3b72657", checks=rows, canvasPosterChangedPixels=canvas),
                                          indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print("7 native sources, 14 builds, 77 exact files; active control fields, edited source and rejection checks passed.")


if __name__ == "__main__":
    main()
