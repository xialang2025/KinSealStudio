"""Compare sparse device fields with V5.1 records and run the actual AMZ RTC formatting/drawing arguments."""
# 20261128-31 add
import argparse
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import struct
from tempfile import TemporaryDirectory

from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM
from unicorn.arm_const import (UC_ARM_REG_PC, UC_ARM_REG_SP, UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2,
                              UC_ARM_REG_R3, UC_ARM_REG_R4, UC_ARM_REG_R5, UC_ARM_REG_R6, UC_ARM_REG_R7)
from verify_native_drawing_drafts import ROOT, REVISION, git_blob, run
from verify_native_value_display_parts import current


CLI = "--export-native-value-record-fields"
FIRMWARE = "7f625b7a967c1ecc0ff5db1fba4d6ed68cb1d9474216e87009395f1d53a88db5"


def read_fields(path):
    """Reject overlapping, partial-word and out-of-bound field listings instead of treating unspecified bytes as known."""
    obj = json.loads(path.read_bytes())
    if obj["format"] != "V122.ControlPara.fields" or obj["completeRecord"] or obj["recordBytes"] != 640:
        raise ValueError("field map was presented as a complete device file")
    transfer = {}
    names = set()
    for field in obj["fields"]:
        pos, raw = field["offset"], bytes.fromhex(field["bytes"])
        if field["property"] in names or not raw or not 0 <= pos < pos + len(raw) <= 640:
            raise ValueError("invalid field name or interval")
        names.add(field["property"])
        for i, value in enumerate(raw, pos):
            if i in transfer:
                raise ValueError("overlapping fields")
            transfer[i] = value
    if len(transfer) != obj["knownBytes"]:
        raise ValueError("coverage count is not the size of the disjoint union")
    return transfer


def run_rtc(firmware, initialized, supplied, date, clock, week, align, geometry, color):
    """Execute AMZ RTC text assembly and validate the real geometry, color and alignment arguments to the graphics call."""
    proc = Uc(UC_ARCH_ARM, UC_MODE_ARM)
    proc.mem_map(0x80000000, 0x500000)
    proc.mem_write(0x80020000, firmware[32:])
    proc.mem_write(0x8011383c, initialized)
    base, stack, incoming, bitmap = 0x8019c5e4, 0x804ef000, 0x804b0000, 0x804bad00
    proc.mem_write(incoming, supplied)
    proc.mem_write(base + 0x278, bytes([0xa5])*8)
    proc.mem_write(base + 0x284, supplied[:1])
    proc.reg_write(UC_ARM_REG_SP, stack)
    proc.reg_write(UC_ARM_REG_R4, base)
    proc.reg_write(UC_ARM_REG_R1, incoming)
    proc.emu_start(0x800aa9d0, 0x800ab008, timeout=1000000, count=10000)
    if (proc.reg_read(UC_ARM_REG_PC) != 0x800ab008 or bytes(proc.mem_read(base, 0x278)) != supplied[:0x278] or
            bytes(proc.mem_read(base + 0x278, 8)) != bytes([0xa5])*8 or proc.mem_read(base + 0x28a, 1)[0] != 5):
        raise ValueError("AMZ RTC constructor changed the copied record, trailing bytes or bitmap group")
    # Execute the actual type-17 background index/bitmap-rectangle path up to the native graphics call.
    proc.mem_write(0x801583e4, struct.pack("<I", bitmap))
    proc.reg_write(UC_ARM_REG_R0, 0)
    proc.reg_write(UC_ARM_REG_R1, 0)
    proc.emu_start(0x8004718c, 0x80047340, timeout=1000000, count=150)
    draw = proc.reg_read(UC_ARM_REG_SP)
    x, y, w, h = geometry
    if (proc.reg_read(UC_ARM_REG_PC) != 0x80047340 or proc.reg_read(UC_ARM_REG_R2) != w or
            proc.reg_read(UC_ARM_REG_R3) != h or struct.unpack("<III", proc.mem_read(draw + 8, 12)) != (16, w*2, bitmap)):
        raise ValueError("AMZ RTC background selected a different frame or geometry")
    proc.mem_write(0x80179568 + 4, struct.pack("<7I", 5, 7, 16, 5, 20, 11, 2026))
    proc.reg_write(UC_ARM_REG_SP, stack)
    proc.emu_start(0x80078ea8, 0x8007916c, timeout=1000000, count=250000)
    if proc.reg_read(UC_ARM_REG_PC) != 0x8007916c:
        raise ValueError("formatter did not reach the pre-draw stage")
    actual = bytes(proc.mem_read(stack + 8, 64)).split(b"\0", 1)[0].decode("ascii")
    dates = tuple(sep.join(parts) for parts in (("2026", "11", "20"), ("11", "20", "2026"), ("20", "11", "2026"))
                  for sep in ("/", ".", "-"))
    times = ("16:07", "16:07:05", "16-07", "16-07-05")
    expected = (dates[date] + " " if date is not None else "") + ("Friday " if week else "")
    if clock is not None:
        expected += times[clock]
    if actual != expected:
        raise ValueError(f"firmware RTC text {actual!r} differs from {expected!r}")
    # Hardware drawing itself is outside this test. Keep the firmware's RGB load, coordinate and alignment instructions.
    proc.emu_start(0x8007916c, 0x80079178, count=8)
    if proc.reg_read(UC_ARM_REG_R5) != color:
        raise ValueError("AMZ text-color word differs")
    proc.reg_write(UC_ARM_REG_R6, 0)
    proc.reg_write(UC_ARM_REG_R7, 0)
    proc.emu_start(0x800791a4, 0x800791f8, count=64)
    if proc.reg_read(UC_ARM_REG_PC) != 0x800791f8 or proc.reg_read(UC_ARM_REG_R2) != (14, 12, 13)[align]:
        raise ValueError("AMZ horizontal alignment flags differ")
    if struct.unpack("<4h", proc.mem_read(proc.reg_read(UC_ARM_REG_R1), 8)) != (x, y, x + w, y + h):
        raise ValueError("AMZ drawing rectangle differs")
    return actual


def native_case(exe, work, family, variant, firmware, initialized):
    """Compare every produced byte with independent native outputs, including repeated processes for defaults."""
    folder = f"docs/stage4-evidence/{family}/native/compiled/{family}-{variant}-r1"
    source = git_blob(folder, "input-native.sm")
    sm, ksp, result = work / "input.sm", work / "input.ksp", work / "fields.json"
    sm.write_bytes(source)
    run(exe, work, "--migrate-project", sm, ksp)
    model = json.loads(ksp.read_bytes())
    run(exe, work, CLI, sm, result)
    fields = read_fields(result)
    references = [folder]
    if variant == "default":
        references.extend(f"docs/stage4-evidence/{family}/physical/{family}-{role}-r1/compiled/physical-native-reference"
                          for role in ("lifecycle", "native-baseline"))
    runs = []
    for ref in references:
        original = git_blob(ref, "ControlPara.dat")
        if git_blob(ref, "input-native.sm") != source or len(original) != 640:
            raise ValueError("original source/record mismatch at " + ref)
        if any(original[i] != v for i, v in fields.items()):
            raise ValueError("field byte mismatch at " + ref)
        checked = dict(reference=ref, nativeSHA256=hashlib.sha256(original).hexdigest())
        if family == "time":
            obj, props = current(model, family)
            checked["firmwareText"] = run_rtc(firmware, initialized, original, props["dateFormat"], None, False,
                        props["alignment"], tuple(obj[k] for k in ("x", "y", "width", "height")), 0)
        runs.append(checked)
    original_map = result.read_bytes()
    run(exe, work, CLI, sm, result, success=False)
    if result.read_bytes() != original_map:
        raise ValueError("repeated export overwrote a destination")
    run(exe, work, CLI, sm, work / "ControlPara.dat", success=False)
    if (work / "ControlPara.dat").exists():
        raise ValueError("partial fields were published as a device file")
    return dict(source=hashlib.sha256(source).hexdigest(), family=family, variant=variant,
                knownBytes=len(fields), originalRuns=runs), model


def edits(exe, work, family, model, firmware, initialized):
    """Generate changed properties independently of original DATs and check numeric, ASCII and RTC behavior."""
    model = deepcopy(model)
    model.pop("nativeSourceSnapshot", None)
    model.pop("nativeSourceWarnings", None)
    obj, props = current(model, family)
    x, y, w, h = 49, 63, 147, 87
    obj.update(x=x, y=y, width=w, height=h)
    props.update(fontSize=24, textColor="#234567", backgroundColor="#bacdef")
    cases = [(None, None, False, 0)]
    if family == "time":
        cases = [(d, c, ((d + c) % 2 == 0), (d + c) % 3) for d in range(9) for c in range(4)]
        cases += [(None, 1, False, 1), (None, None, True, 2), (8, None, False, 0)]
    elif family == "number":
        props.update(totalDigits=4, decimalDigits=2, inputMaximum="1387", inputMinimum="-120",
                     gain="1.75", offset="-2.5", monitorAddress="LW31", inputAddress="LW31",
                     highColor="#238047", lowColor="#538547")
    else:
        obj["asciiDisplay"]["characterCount"] = 9
        props.update(monitorAddress="LW27", inputAddress="LW27")
    rows = []
    for i, (date, clock, week, align) in enumerate(cases):
        if family == "time":
            props.update(showDate=date is not None, showTime=clock is not None, showWeek=week, alignment=align,
                         dateFormat=0 if date is None else date, timeFormat=0 if clock is None else clock)
        path = work / "edit.ksp"
        path.write_text(json.dumps(model, ensure_ascii=False), encoding="utf8")
        output = work / f"edit-{i}.json"
        run(exe, work, CLI, path, output)
        fields = read_fields(output)
        expected = {4: struct.pack("<4H", x, y, w, h), 12: struct.pack("<II", 0, w*h*(6 if family=="number" else 2))}
        if family == "time":
            expected.update({0x108: bytes([date is not None, clock is not None, week]),
                             0x10c: struct.pack("<H", (1, 2, 0)[align]), 0x118: struct.pack("<3H", 2, date or 0, clock or 0),
                             0x120: bytes.fromhex("234567"), 0x124: bytes.fromhex("bacdef")})
        elif family == "ascii":
            expected.update({0x128: struct.pack("<2H", 1, 9), 0x140: struct.pack("<I", 27), 0x1ac: struct.pack("<I", 27),
                             0x130: bytes.fromhex("234567"), 0x134:bytes.fromhex("bacdef")})
        else:
            expected.update({0x128: struct.pack("<4H", 1, 4, 2, 0), 0x148: struct.pack("<dd", 1.75, -2.5),
                             0x168: struct.pack("<dd", 1387, -120), 0x180: struct.pack("<I", 31),
                             0x258: struct.pack("<I", 31), 0x138: bytes.fromhex("234567"), 0x13c: bytes.fromhex("bacdef"),
                             0x140: bytes.fromhex("238047"), 0x144: bytes.fromhex("538547")})
        for pos, value in expected.items():
            if bytes(fields[p] for p in range(pos, pos + len(value))) != value:
                raise ValueError(f"incorrect {family} edited bytes at {pos:#x}")
        if family == "time":
            for fill in (0, 0xa5):
                sample = bytearray([fill])*640
                for pos, value in fields.items():
                    sample[pos] = value
                actual = run_rtc(firmware, initialized, bytes(sample), date, clock, week, align, (x, y, w, h), 0x674523)
            rows.append(dict(dateFormat=date, timeFormat=clock, showWeek=week, alignment=align,
                             unknownFills=["00", "A5"], firmwareText=actual))
        else:
            rows.append(dict(changedFieldOffsets=sorted(expected)))
    props["opacity"] = 127
    path.write_text(json.dumps(model, ensure_ascii=False), encoding="utf8")
    blocked = work/"unsupported.json"
    run(exe, work, CLI, path, blocked, success=False)
    if blocked.exists():
        raise ValueError("rejected feature left a partial map")
    if family in ("ascii", "number"):
        props.update(opacity=255, inputAddress="LW38")
        path.write_text(json.dumps(model, ensure_ascii=False), encoding="utf8")
        run(exe, work, CLI, path, blocked, success=False)
        if family == "number":
            props.update(inputAddress=props["monitorAddress"], dataType=1, inputMinimum="0")
            path.write_text(json.dumps(model, ensure_ascii=False), encoding="utf8")
            run(exe, work, CLI, path, blocked, success=False)
        if blocked.exists():
            raise ValueError("unverified address/number profile published fields")
    return rows


def main():
    """Validate records from two original SM profiles in three families and exercise independent nonbaseline edits."""
    parser = argparse.ArgumentParser()
    parser.add_argument("exe", type=Path)
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    exe = args.exe.resolve()
    firmware = (ROOT/"bin/AMZ070W01.bin").read_bytes()
    initialized = (ROOT/"firmware_analysis/amz070w01-disasm-20261119/RW_8011383C.bin").read_bytes()
    if hashlib.sha256(firmware).hexdigest() != FIRMWARE:
        raise ValueError("unexpected AMZ firmware")
    cases, changed = [], {}
    for family in ("time", "ascii", "number"):
        for variant in ("default", "bilingual-style24"):
            with TemporaryDirectory(dir=ROOT/".local") as tmp:
                row, model = native_case(exe, Path(tmp), family, variant, firmware, initialized)
                cases.append(row)
                if variant == "default":
                    changed[family] = edits(exe, Path(tmp), family, model, firmware, initialized)
    result = dict(compilerSHA256=hashlib.sha256(exe.read_bytes()).hexdigest(), revision=REVISION,
                  firmwareSHA256=FIRMWARE, initializedRWSHA256=hashlib.sha256(initialized).hexdigest(),
                  fieldBytesCompared=sum(row["knownBytes"]*len(row["originalRuns"]) for row in cases),
                  cases=cases, edited=changed)
    if args.receipt:
        args.receipt.write_text(json.dumps(result, ensure_ascii=False, indent=1) + "\n", encoding="utf8")
    print(f"{result['fieldBytesCompared']} field bytes, 12 native runs, {len(changed['time'])} edited RTC runs")


if __name__ == "__main__":
    main()
