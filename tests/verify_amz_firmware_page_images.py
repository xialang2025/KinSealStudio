"""Compile V122 boundary strokes and run the AMZ070W01 image reader on their normal RGB565 pages."""
# 20261128-19 add
import argparse
from copy import deepcopy
import hashlib
import json
import math
from pathlib import Path
import struct
from tempfile import TemporaryDirectory

from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM
from unicorn.arm_const import (
    UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3,
    UC_ARM_REG_LR, UC_ARM_REG_PC, UC_ARM_REG_SP,
)
from verify_native_drawing_drafts import ROOT, REVISION, git_blob, run


REFERENCE = ROOT / "docs/文件规则/样例/AMZ边界画线与固件像素复验_20261128.json"
FRAME_BYTES = 800 * 480 * 2


def digest(data):
    """Identify the complete artifact or decompressed frame."""
    return hashlib.sha256(data).hexdigest()


def firmware_frame(firmware, data):
    """Run the pinned, standalone ARM page decoder on a complete positive-test page."""
    decoder, stop = 0x800AB414, 0x800AB000
    source, destination, stack = 0x1000000, 0x2000000, 0x3000000
    cpu = Uc(UC_ARCH_ARM, UC_MODE_ARM)
    cpu.mem_map(stop, 4096)
    cpu.mem_write(decoder, firmware[0x8B434:0x8B698])
    cpu.mem_map(source, ((len(data) + 4095) // 4096) * 4096)
    cpu.mem_write(source, data)
    cpu.mem_map(destination, ((FRAME_BYTES + 4095) // 4096) * 4096)
    cpu.mem_map(stack, 4096)
    for register, value in ((UC_ARM_REG_R0, source), (UC_ARM_REG_R1, len(data)),
                            (UC_ARM_REG_R2, destination), (UC_ARM_REG_R3, stack),
                            (UC_ARM_REG_SP, stack + 0xFC0), (UC_ARM_REG_LR, stop)):
        cpu.reg_write(register, value)
    cpu.emu_start(decoder, stop, timeout=5000000, count=FRAME_BYTES * 20 + 10000)
    if (cpu.reg_read(UC_ARM_REG_PC) != stop or cpu.reg_read(UC_ARM_REG_R0) != 0 or
            int.from_bytes(cpu.mem_read(stack, 4), "little") != FRAME_BYTES):
        raise ValueError("AMZ decoder did not finish a complete 800 x 480 RGB565 frame")
    return bytes(cpu.mem_read(destination, FRAME_BYTES))


def compile_page(exe, work, source, mode, created, firmware):
    """Read only the page stream and index that would accompany the device project."""
    with TemporaryDirectory(dir=work) as tmp:
        out = Path(tmp) / "parts"
        flags = ["--native-created-drawing"] if created else []
        run(exe, work, mode, source, out, *flags)
        sheet = (out / "SheetPixel.dat").read_bytes()
        lengths = (out / "PicLenPara.dat").read_bytes()
        if len(lengths) != 2400 or struct.unpack_from("<I", lengths)[0] != len(sheet):
            raise ValueError("unexpected firmware page index")
        return sheet, lengths, digest(firmware_frame(firmware, sheet))


def controlled_model(base, case, kind):
    """Place the same translated source endpoints in Lite's normalized geometry."""
    model = deepcopy(base)
    ax, ay, bx, by = case["endpoints"]
    tx, ty = case["translation"]
    left, top = math.floor(min(ax, bx) + tx), math.floor(min(ay, by) + ty)
    width = max(1, math.ceil(max(ax, bx) + tx) - left)
    height = max(1, math.ceil(max(ay, by) + ty) - top)
    obj = model["pages"][0]["objects"][0]
    obj.update(type=kind, x=left, y=top, width=width, height=height)
    obj["graphic"]["points"] = [[(x + tx - left) / width, (y + ty - top) / height]
                                for x, y in ((ax, ay), (bx, by))]
    return model


def verify(exe, work, firmware):
    """Check whole native DATs, source round trips, and independently recorded Qt4 images."""
    reference = json.loads(REFERENCE.read_bytes())
    if reference["revision"] != REVISION or digest(firmware) != reference["firmware"]:
        raise ValueError("firmware or source revision differs from the pinned evidence")
    source, model = work / "page.sm", work / "page.ksp"
    models, originals, controlled = [], [], []
    for index, item in enumerate(reference["originals"]):
        saved = git_blob(item["created"], "input-native.sm")
        assert digest(saved) == item["source"] and saved == git_blob(item["reopened"], "input-native.sm")
        source.write_bytes(saved)
        run(exe, work, "--migrate-project", source, model)
        models.append(json.loads(model.read_bytes()))
        restored = work / f"restore-{index}.sm"
        run(exe, work, "--export-native-project", model, restored)
        assert restored.read_bytes() == saved
        for created in (False, True):
            stage = item["created" if created else "reopened"]
            expected = git_blob(stage, "SheetPixel.dat")
            expected_index = git_blob(stage, "PicLenPara.dat")
            qt = reference["controlled"][index]["qtCreatedRgb565Sha256" if created else "qtRgb565Sha256"]
            assert digest(firmware_frame(firmware, expected)) == qt
            for mode in ("--compile-project", "--compile-native-media-parts"):
                page, lengths, rgb = compile_page(exe, work, source, mode, created, firmware)
                if (page, lengths, rgb) != (expected, expected_index, qt):
                    raise ValueError(f"{stage}, {mode}, created={created}: DAT or RGB565 differs")
                originals.append(dict(source=item["source"], mode=mode, created=created,
                                      sheet=digest(page), lengths=digest(lengths), rgb565=rgb))
    for index, item in enumerate(reference["controlled"]):
        ax, ay, bx, by = item["endpoints"]
        kinds = ["freeline", "polyline"] if ax == bx or ay == by else ["freeline"]
        for kind in kinds:
            variant = controlled_model(models[1 if kind == "polyline" else 0], item, kind)
            model.write_text(json.dumps(variant, ensure_ascii=False), "utf8")
            for created in (False, True):
                page, _, rgb = compile_page(exe, work, model, "--compile-project", created, firmware)
                if rgb != item["qtCreatedRgb565Sha256" if created else "qtRgb565Sha256"]:
                    raise ValueError(f"Qt4 differs for {index}, {kind}, created={created}")
                controlled.append(dict(case=index, kind=kind, created=created, sheet=digest(page), rgb565=rgb))
    # A single fractional point is not the native pen's common translation. A cache state for one
    # object does not establish the other object's render state when replaying a multi-object page.
    edited = controlled_model(models[0], reference["controlled"][0], "freeline")
    edited["pages"][0]["objects"][0]["graphic"]["points"][0][0] += 0.001
    model.write_text(json.dumps(edited), "utf8")
    rejected = work / "rejected"
    run(exe, work, "--compile-project", model, rejected, success=False)
    run(exe, work, "--compile-native-media-parts", model, rejected, success=False)
    run(exe, work, "--compile-project", source, rejected, "--native-created-drawing", "--native-created-drawing", success=False)
    run(exe, work, "--compile-native-media-parts", source, rejected, "--native-created-drawing", "--native-created-drawing", success=False)
    valid = deepcopy(models[0])
    peer = deepcopy(valid["pages"][0]["objects"][0])
    peer.update(id="separate-line", x=350, y=200, width=200, height=100)
    peer["graphic"]["points"] = [[0, 0], [1, 1]]
    valid["pages"][0]["objects"].append(peer)
    model.write_text(json.dumps(valid), "utf8")
    run(exe, work, "--compile-project", model, rejected, "--native-created-drawing", success=False)
    # A large canvas bounding box can contain a tiny actual line in an edited KSP.
    tiny = deepcopy(models[0])
    obj = tiny["pages"][0]["objects"][0]
    obj.update(type="line", x=100, y=100, width=100, height=100)
    obj["graphic"]["points"] = [[0, 0], [0.01, 0.01]]
    model.write_text(json.dumps(tiny), "utf8")
    run(exe, work, "--compile-project", model, rejected, "--native-created-drawing", success=False)
    if rejected.exists():
        raise ValueError("rejected context published output")
    return dict(exeSha256=digest(exe.read_bytes()), firmware=reference["firmware"], rejectedCases=6,
                originalRoutes=originals, controlled=controlled)


def main():
    """Use a caller-selected scratch volume for the offline validation."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--firmware", type=Path, default=ROOT / "bin/AMZ070W01.bin")
    parser.add_argument("--scratch", type=Path, default=ROOT / "build")
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    with TemporaryDirectory(dir=args.scratch.resolve()) as tmp:
        result = verify(args.exe.resolve(), Path(tmp), args.firmware.read_bytes())
    if args.report:
        args.report.write_bytes((json.dumps(result, ensure_ascii=False, indent=2) + "\n").encode())
    print(f"{len(result['originalRoutes'])} source/mode comparisons, {len(result['controlled'])} Qt4/AMZ frame comparisons")


if __name__ == "__main__":
    main()
