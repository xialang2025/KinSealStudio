"""Compare V122 QR control files with the native compiler and decode the sprite with AMZ070W01 firmware."""
# 20261128-26 add
import argparse
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import struct
from tempfile import TemporaryDirectory

from verify_amz_button_frame_rules import NATIVE_SHA
from verify_amz_firmware_page_images import firmware_frame
from verify_native_drawing_drafts import ROOT, REVISION, git_blob, run
from verify_native_lamp_monitor_words import ROUTES

FOLDER = "docs/stage4-evidence/qrcode/native"
NAMES = ("default", "length0", "length255", "swap", "description-chinese-ascii", "description-chinese-gb2312")
IMAGE_FILES = ("ControlPixel.dat", "PicLenPara.dat", "SheetPixel.dat", "AddrPara.dat")
FIRMWARE_SHA = "7f625b7a967c1ecc0ff5db1fba4d6ed68cb1d9474216e87009395f1d53a88db5"
PHYSICAL = "docs/stage4-evidence/qrcode/physical/qrcode-lifecycle-r1/compiled/physical-native-reference"


def sha(data):
    """Return the full SHA-256 of one binary."""
    return hashlib.sha256(data).hexdigest()


def frame_pixels(firmware, packed, index, size):
    """Require one native-aligned entry and invoke the actual target decoder on its effective data."""
    if len(index) != 2412 or struct.unpack_from("<I", index, 1600)[0] != 1:
        raise ValueError("QR frame table does not have one entry")
    codec, padded, effective = struct.unpack_from("<III", index, 2400)
    if codec != 0 or not effective or padded != len(packed) or padded != ((effective + 31) & ~31):
        raise ValueError("QR LZO frame extents differ")
    return firmware_frame(firmware, packed[:effective], size)


def active_record(record):
    """Extract the fields used by the original reader and toolbar options; retain disk tag separately."""
    if len(record) != 640 or record[0] != 52 or record[3] not in (6, 14):
        raise ValueError("QR record has the wrong type, size or flag")
    geom = struct.unpack_from("<4H2I", record, 4)
    return (record[0:3], geom, record[0x100:0x108], record[0x10c:0x110],
            record[0x128:0x22e], record[0x230:0x280])


def verify_writer(native):
    """Run the original word-count, options and final record-copy instructions with controlled inputs."""
    import pefile
    from unicorn import Uc, UC_ARCH_X86, UC_MODE_32
    from unicorn.x86_const import UC_X86_REG_EBP, UC_X86_REG_EBX, UC_X86_REG_EIP
    if sha(native.read_bytes()) != NATIVE_SHA:
        raise ValueError("wrong V5.1 native desktop compiler")
    image = pefile.PE(str(native), fast_load=True)
    cpu = Uc(UC_ARCH_X86, UC_MODE_32)
    frame, obj, start = 0x19000, 0x14000, 0xADE000
    cpu.mem_map(0x10000, 0x10000)
    cpu.mem_map(start, 4096)
    cpu.mem_write(start, image.get_data(start - image.OPTIONAL_HEADER.ImageBase, 4096))
    for length in (0, 1, 2, 25, 254, 255):
        for swap in (0, 1):
            for encoding in (0, 1):
                cpu.reg_write(UC_X86_REG_EBP, frame)
                cpu.reg_write(UC_X86_REG_EBX, obj)
                cpu.mem_write(obj + 0x168, bytes([swap, encoding, 0, 0]) + struct.pack("<I", length))
                cpu.emu_start(0xADE1A7, 0xADE1BE, count=16)
                if cpu.reg_read(UC_X86_REG_EIP) != 0xADE1BE or struct.unpack("<I", cpu.mem_read(obj+0x144, 4))[0] != (length+1)//2:
                    raise ValueError("native QR word span mismatch")
                scratch = bytes((i * 71 + 5) % 256 for i in range(0x230))
                cpu.mem_write(frame - 0x394, scratch)
                cpu.mem_write(frame - 0x62c, struct.pack("<I", frame - 0x394))
                cpu.mem_write(frame - 0x614, b"\xA5" * 640)
                cpu.emu_start(0xADE305, 0xADE34E, count=1800)
                expected = bytearray(scratch + bytes(0x50))
                expected[0x228:0x22e] = struct.pack("<IBB", length, swap, encoding)
                if cpu.reg_read(UC_X86_REG_EIP) != 0xADE34E or cpu.mem_read(frame - 0x614, 640) != expected:
                    raise ValueError("native QR final record copy mismatch")


def firmware_monitor(firmware, record):
    """Load the generated disk record with the ARM loader, then read an independent local-word bank."""
    from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM
    from unicorn.arm_const import UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R4, UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC
    cpu = Uc(UC_ARCH_ARM, UC_MODE_ARM)
    for page in (0x80020000, 0x8007F000, 0x800AA000):
        cpu.mem_map(page, 4096)
        offset = page - 0x80020000 + 0x20
        cpu.mem_write(page, firmware[offset:offset + 4096])
    slot, global_w, _, mode = struct.unpack("<4I", cpu.mem_read(0x8007F7F8, 16))
    source, bank, pointer, stack, stop = 0x15000, 0x16000, 0x18000, 0x19000, 0x1A000
    cpu.mem_map(0x10000, 0x10000)
    for page in set((slot & ~4095, (slot + 0x600) & ~4095, global_w & ~4095, mode & ~4095)):
        cpu.mem_map(page, 4096)
    storage = bytes((pos * 13 + 9) % 251 + 1 for pos in range(4096))
    cpu.mem_write(source, record)
    cpu.mem_write(slot, b"\xA5" * 0x600)
    cpu.mem_write(bank, storage)
    cpu.mem_write(global_w, struct.pack("<I", pointer))
    cpu.mem_write(pointer, struct.pack("<I", bank))
    cpu.reg_write(UC_ARM_REG_R4, slot)
    cpu.reg_write(UC_ARM_REG_R1, source)
    cpu.reg_write(UC_ARM_REG_SP, stack)
    cpu.emu_start(0x800AAF2C, 0x800AAF6C, count=5000)
    expected = bytearray(record[:0x230])
    expected[0x107] = ord("@")
    if (cpu.reg_read(UC_ARM_REG_PC) != 0x800AAF6C or cpu.mem_read(slot, 0x230) != expected or
            cpu.mem_read(slot + 0x230, 1) != b"\xA5" or cpu.mem_read(slot + 0x28A, 2) != bytes([6, 0])):
        raise ValueError("AMZ loader did not initialize the expected type-52 slot")
    cpu.reg_write(UC_ARM_REG_R0, 0)
    cpu.reg_write(UC_ARM_REG_R1, slot + 0x104)
    cpu.reg_write(UC_ARM_REG_LR, stop)
    cpu.emu_start(0x8007F730, stop, count=7000)
    size = struct.unpack_from("<I", record, 0x228)[0]
    offset = 2 * struct.unpack_from("<I", record, 0x10C)[0]
    if (cpu.reg_read(UC_ARM_REG_PC) != stop or cpu.mem_read(slot + 0x28B, size + 1) != storage[offset:offset + size] + b"\0" or
            cpu.mem_read(bank, len(storage)) != storage):
        raise ValueError("AMZ monitor did not deliver the requested local word range unchanged")


def verify_reference(exe, work, firmware, name, original_pixels):
    """Join the frozen original .sm to its native output by digest before exercising both public entries."""
    label = "r2" if name == "default" else "r1"
    manifest = json.loads(git_blob(FOLDER, f"native-probe-qrcode-{name}-{label}.json"))["nativeCompilations"][0]
    source = work / f"{name}.sm"
    source.write_bytes(git_blob(FOLDER, f"qrcode-{name}.sm"))
    if sha(source.read_bytes()) != manifest["sourceSha256"]:
        raise ValueError("native source/compile digest mismatch: " + name)
    native = {item["path"]: item["sha256"] for item in manifest["files"]}
    record = git_blob(FOLDER + "/compiled/qrcode-" + name + "-r1", "ControlPara.dat")
    if sha(record) != native["ControlPara.dat"]:
        raise ValueError("native control record mismatches manifest")
    row = dict(source=name, sourceSha256=manifest["sourceSha256"], originalFileSha256=native)
    for route in ROUTES:
        with TemporaryDirectory(dir=work) as tmp:
            out = Path(tmp) / "parts"
            run(exe, work, route, source, out)
            files = {f.name: f.read_bytes() for f in out.iterdir() if f.is_file() and f.name in native}
        equal = sorted(member for member, data in files.items() if sha(data) == native[member])
        if not set(IMAGE_FILES).issubset(equal):
            raise ValueError(f"{name}: native image, index, background or address file differed ({equal})")
        produced = files["ControlPara.dat"]
        if active_record(produced) != active_record(record):
            raise ValueError(name + ": native control fields differ")
        firmware_monitor(firmware, produced)
        pixels = frame_pixels(firmware, files["ControlPixel.dat"], files["PicLenPara.dat"], len(original_pixels))
        if pixels != original_pixels:
            raise ValueError(name + ": device-decoded RGB565 pixels differ")
        row[route] = dict(equal=equal, different=sorted(set(files) - set(equal)),
                          missing=sorted(set(native) - set(files)), decodedRgb565Sha256=sha(pixels),
                          originalFlag=record[3], emittedFlag=produced[3],
                          differentControlBytes=sum(x != y for x, y in zip(record, produced)),
                          emittedSha256={name: sha(data) for name, data in sorted(files.items())})
    return row


def check_edits(exe, work, firmware):
    """Exercise ordinary KSP edits, local word spans, CP936 boundaries and atomic refusal through both routes."""
    model = work / "model.ksp"
    run(exe, work, "--migrate-project", work / "default.sm", model)
    original = json.loads(model.read_bytes())
    good = deepcopy(original)
    obj = good["pages"][0]["objects"][0]
    obj.update(width=101, height=293, x=5, y=6, address="LW37")
    qr = obj["qrCode"]
    qr.update(monitorAddress="LW37", description="金" * 127 + "A", encoding=1, characterLength=255, swapBytes=True)
    edited = work / "edited.ksp"
    edited.write_text(json.dumps(good, ensure_ascii=False), encoding="utf-8")
    for route in ROUTES:
        with TemporaryDirectory(dir=work) as tmp:
            out = Path(tmp) / "parts"
            run(exe, work, route, edited, out)
            record = (out / "ControlPara.dat").read_bytes()
            if (struct.unpack_from("<4H", record, 4) != (5, 6, 101, 293) or
                    struct.unpack_from("<I", record, 0x10c)[0] != 37 or record[0x228:0x22e] != bytes.fromhex("ff0000000101") or
                    record[0x128:0x228] != qr["description"].encode("cp936") + b"\0"):
                raise ValueError("edited QR fields did not reach the exported record")
            firmware_monitor(firmware, record)
            frame_pixels(firmware, (out / "ControlPixel.dat").read_bytes(), (out / "PicLenPara.dat").read_bytes(), 101 * 293 * 2)
        for bad in ("金" * 128, "ab\0cd", "弹🚀"):
            invalid = deepcopy(good)
            invalid["pages"][0]["objects"][0]["qrCode"]["description"] = bad
            invalid_path = work / "invalid.ksp"
            invalid_path.write_text(json.dumps(invalid, ensure_ascii=False), encoding="utf-8")
            with TemporaryDirectory(dir=work) as tmp:
                out = Path(tmp) / "parts"
                run(exe, work, route, invalid_path, out, success=False)
                if out.exists() and any(out.iterdir()):
                    raise ValueError("invalid QR description left a partially published package")


def main():
    """Report whole-file equality separately from assigned control fields and target-decoded pixels."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--native", type=Path, required=True)
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    exe = args.exe.resolve()
    compiler_sha = sha(exe.read_bytes())
    verify_writer(args.native)
    firmware = (ROOT / "bin/AMZ070W01.bin").read_bytes()
    if sha(firmware) != FIRMWARE_SHA:
        raise ValueError("wrong target firmware")
    sprite = (ROOT / "tests/fixtures/native-source/qrcode/default-controlpixel.dat").read_bytes()
    table = git_blob(FOLDER + "/compiled/qrcode-default-r1", "PicLenPara.dat")
    pixels = frame_pixels(firmware, sprite, table, 200 * 100 * 2)
    with TemporaryDirectory(prefix="qr-evidence-", dir=ROOT / "build") as temp:
        work = Path(temp)
        cases = [verify_reference(exe, work, firmware, name, pixels) for name in NAMES]
        check_edits(exe, work, firmware)
    default = git_blob(FOLDER + "/compiled/qrcode-default-r1", "ControlPara.dat")
    repeated = git_blob(PHYSICAL, "ControlPara.dat")
    native_delta = [offset for offset, (first, second) in enumerate(zip(default, repeated)) if first != second]
    if (sha(git_blob(PHYSICAL, "input-native.sm")) != cases[0]["sourceSha256"] or
            active_record(default) != active_record(repeated) or not native_delta):
        raise ValueError("second native run does not demonstrate a same-source record difference")
    native_sys = [git_blob(path, "SysPara.dat") for path in (FOLDER + "/compiled/qrcode-default-r1", PHYSICAL)]
    if sha(exe.read_bytes()) != compiler_sha:
        raise ValueError("compiler changed during verification")
    report = dict(referenceRevision=REVISION, compilerSha256=compiler_sha, firmwareSha256=sha(firmware),
                  nativeWriterSha256=NATIVE_SHA, nativeWriterSettingsChecked=24, armLoaderAndLocalWordMonitorChecked=True,
                  sameSourceNativeControlDelta=dict(sha256=[sha(default), sha(repeated)], byteOffsets=native_delta),
                  sameSourceNativeSysDelta=dict(sha256=[sha(data) for data in native_sys],
                                                differentBytes=sum(x != y for x, y in zip(*native_sys))),
                  size=[200, 100], editedSize=[101, 293], testedDescriptionLengths=[255, 256], cases=cases)
    if args.receipt:
        args.receipt.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"{len(cases)} native QR sources x {len(ROUTES)} compiler routes matched image, index, page, address and active fields.")


if __name__ == "__main__":
    main()
