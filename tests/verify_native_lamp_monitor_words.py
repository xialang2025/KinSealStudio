"""Compare switch monitor words with eleven independent V5.1 builds and edit one source field at a time."""
# 20261128-16 add
import argparse
import hashlib
import json
from pathlib import Path
import struct
import tempfile
import zlib

from verify_amz_button_frame_rules import NATIVE_SHA, run
from verify_native_drawing_drafts import ROOT, git_blob

FOLDER = "docs/stage2-evidence/lamp-regression-restore/compiled/"
SAMPLES = (
    "lamp-default-r1", "lamp-dual-flash0-r1", "lamp-multi2-flash0-r1",
    "lamp-multi5-flash0-r1", "lamp-multi5-flash2-r1", "lamp-multi5-u16-r4",
    "lamp-multi5-u32-r1", "lamp-none-r1", "lamp-wordbit15-r1", "lamp-wordbit7-r1",
    "product-shared-lamp-new-native",
)
MONITOR_FIELDS = {"switch.248.1": 0x188, "switch.248.4": 0x18c, "switch.274.0": 0x194}
ROUTES = ("--compile-project", "--compile-native-media-parts")


def original_writer(pe, expected):
    """Execute the three V5.1 source-member loads and its complete final copy using the original x86 instructions."""
    from unicorn import Uc, UC_ARCH_X86, UC_MODE_32
    from unicorn.x86_const import UC_X86_REG_EBP, UC_X86_REG_EBX, UC_X86_REG_EIP

    cpu = Uc(UC_ARCH_X86, UC_MODE_32)
    frame, obj, output = 0x14000, 0x17000, 0x19000
    cpu.mem_map(0x10000, 0x10000)
    cpu.mem_map(0x693000, 0x2000)
    code = pe.get_data(0x693000 - pe.OPTIONAL_HEADER.ImageBase, 0x2000)
    cpu.mem_write(0x693000, code)
    cpu.reg_write(UC_X86_REG_EBP, frame)
    cpu.reg_write(UC_X86_REG_EBX, obj)
    cpu.mem_write(frame + 12, struct.pack("<I", output))
    cpu.mem_write(frame - 0x8c, bytes([0xad]) * 60)
    for path, offset in (("switch.248.1", 0x24c), ("switch.248.4", 0x26c), ("switch.274.0", 0x274)):
        cpu.mem_write(obj + offset, struct.pack("<I", expected[path]))
    cpu.emu_start(0x693fb9, 0x69401a, count=100)
    if cpu.reg_read(UC_X86_REG_EIP) != 0x69401a:
        raise ValueError("V5.1 did not finish copying the lamp monitoring structure")
    for path, offset in MONITOR_FIELDS.items():
        if cpu.mem_read(output + offset, 4) != struct.pack("<I", expected[path]):
            raise ValueError(f"V5.1 did not copy the source object member for {path}")
    if cpu.mem_read(output + 0x182, 2) != b"\xad\xad":
        raise ValueError("unexpected writer for the adjacent structure padding")


def source_offsets(exe, sm):
    """Read the typed big-endian integers at variable positions in each original file."""
    output = sm.with_suffix(".fields.json")
    run(exe, sm.parent, "--inspect-native-source", sm, output)
    fields = json.loads(output.read_bytes())["fields"]
    result = {}
    for name in MONITOR_FIELDS:
        field, = [row for row in fields if row["path"] == "object[0]." + name]
        if field["kind"] != "i" or field["end"] - field["start"] != 4:
            raise ValueError(f"{sm.name}: invalid typed source field {name}")
        result[name] = field["start"]
    return result


def compile_record(exe, project, folder, route):
    """Compile through the regular file entry point and ensure independent decoding sees a single lamp."""
    run(exe, project.parent, route, project, folder)
    record = (folder / "ControlPara.dat").read_bytes()
    if len(record) != 640 or record[0] != 10 or record[0x106:0x108] != b"\x04\x00":
        raise ValueError(f"{folder}: expected exactly one native switch subtype 4")
    return record


def differences(first, second):
    """Count individual unequal bytes, including trailing lengths."""
    return sum(a != b for a, b in zip(first, second)) + abs(len(first) - len(second))


def original_case(exe, previous, pe, directory, name):
    """Compare each independent original control with the exact source and both rebuilt device files."""
    directory.mkdir()
    source = git_blob(FOLDER + name, "input-native.sm")
    original = git_blob(FOLDER + name, "ControlPara.dat")
    sm = directory / "original.sm"
    sm.write_bytes(source)
    source_fields = source_offsets(exe, sm)
    expected = {key: struct.unpack_from(">I", source, start)[0] for key, start in source_fields.items()}
    if pe is not None:
        original_writer(pe, expected)
    row = dict(name=name, sourceSha256=hashlib.sha256(source).hexdigest(), monitorWords=expected)
    for key, start in MONITOR_FIELDS.items():
        if struct.unpack_from("<I", original, start)[0] != expected[key]:
            raise ValueError(f"{name}: original writer and source disagree on {key}")
    for route in ROUTES:
        rebuilt = compile_record(exe, sm, directory / route[2:], route)
        for key, start in MONITOR_FIELDS.items():
            if rebuilt[start:start + 4] != original[start:start + 4]:
                raise ValueError(f"{name}: {route} {key} differs from V5.1 at {start:#x}")
        row[route] = differences(original, rebuilt)
    if previous:
        old = compile_record(previous, sm, directory / "before", ROUTES[0])
        row["beforeDifferentBytes"] = differences(original, old)
        fixed = bytearray(old)
        for start in MONITOR_FIELDS.values():
            fixed[start:start + 4] = original[start:start + 4]
        if fixed != rebuilt:
            raise ValueError(f"{name}: differences from the prior build extend beyond the three monitored words")
        if row["beforeDifferentBytes"] <= row[ROUTES[0]]:
            raise ValueError(f"{name}: no reduction in whole-record differences")

    if name in ("lamp-default-r1", "lamp-wordbit7-r1"):
        key, value = ("switch.274.0", 0x9b42d7c6) if name == "lamp-default-r1" else ("switch.248.1", 11)
        active = key == "switch.248.1"
        edited = bytearray(source)
        struct.pack_into(">I", edited, source_fields[key], value)
        sm.write_bytes(edited)
        if pe is not None:
            original_writer(pe, {**expected, key: value})
        original_output = (directory / ROUTES[0][2:] / "ControlPara.dat").read_bytes()
        expected_output = bytearray(original_output)
        struct.pack_into("<I", expected_output, MONITOR_FIELDS[key], value)
        for route in ROUTES:
            actual = compile_record(exe, sm, directory / ("edited-" + route[2:]), route)
            if actual != expected_output:
                raise ValueError(f"{name}: the one-field source edit changed unexpected control bytes")
        row["edit"] = dict(source=key, value=value, active=active, routes=2, fullRecordChecked=True)
    return row


def new_project(exe, directory):
    """Confirm the model and exported SM both compile the source's current words without an input snapshot."""
    source = ROOT / "samples/lamp/双态字位.ksp"
    model = json.loads(source.read_bytes())
    if model.get("nativeSourceSnapshot") or len(model["pages"][0]["objects"]) != 1:
        raise ValueError("expected a single new-model lamp without any source snapshot")
    sm = directory / "new-model.sm"
    run(exe, directory, "--export-native-project", source, sm)
    offsets = source_offsets(exe, sm)
    raw = sm.read_bytes()
    direct = compile_record(exe, source, directory / "new-ksp", ROUTES[0])
    for key, slot in MONITOR_FIELDS.items():
        if direct[slot:slot + 4] != raw[offsets[key]:offsets[key] + 4][::-1]:
            raise ValueError("new-model control disagrees with its own exported source: " + key)
    for route in ROUTES:
        if compile_record(exe, sm, directory / ("new-sm-" + route[2:]), route) != direct:
            raise ValueError("new-model control differs after source export")
    model["pages"][0]["objects"][0]["flipHorizontal"] = True
    flipped = directory / "lite-only.ksp"
    flipped.write_text(json.dumps(model, ensure_ascii=False), encoding="utf8")
    out = directory / "lite-only"
    fallback = compile_record(exe, flipped, out, ROUTES[0])
    archive = (out / "upLoad.knr").read_bytes()
    retained = json.loads(zlib.decompress(archive[4:]))
    if not retained["pages"][0]["objects"][0]["flipHorizontal"]:
        raise ValueError("the Lite-only source did not retain the edited flip")
    values = [struct.unpack_from("<I", fallback, start)[0] for start in MONITOR_FIELDS.values()]
    if values != [7, 0, 0]:
        raise ValueError("the active word-bit selector changed in the Lite-only fallback")
    return dict(project=source.name, routes=3, fullRecordSha256=hashlib.sha256(direct).hexdigest(),
                liteOnlyFallback=dict(flipRetained=True, monitorWords=values))


def main():
    """Report the source words and all residual per-record byte counts."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--before-exe", type=Path)
    parser.add_argument("--native-exe", type=Path, help="also execute the V5.1 monitoring writer with pefile and Unicorn")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    exe = args.exe.resolve()
    previous = args.before_exe.resolve() if args.before_exe else None
    pe = None
    if args.native_exe:
        import pefile
        native = args.native_exe.read_bytes()
        if hashlib.sha256(native).hexdigest() != NATIVE_SHA:
            raise ValueError("unexpected V5.1 executable revision")
        pe = pefile.PE(data=native, fast_load=True)
    with tempfile.TemporaryDirectory(prefix="amz-lamp-monitor-", dir=ROOT / "build") as scratch:
        directory = Path(scratch)
        rows = [original_case(exe, previous, pe, directory / name, name) for name in SAMPLES]
        new = new_project(exe, directory)
    report = dict(exeSha256=hashlib.sha256(exe.read_bytes()).hexdigest(), referenceRevision="e3b72657",
                  referenceDirectory=FOLDER, independentComparedBytes=12 * len(rows), samples=rows, newProject=new)
    if previous:
        report["beforeExeSha256"] = hashlib.sha256(previous.read_bytes()).hexdigest()
    if pe is not None:
        report["nativeWriterSha256"] = NATIVE_SHA
        report["nativeWriterExecutions"] = len(rows) + sum("edit" in row for row in rows)
    if args.receipt:
        args.receipt.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf8")
    print(f"Compared {len(rows)} V5.1 controls, {12 * len(rows)} independent original bytes across two routes.")


if __name__ == "__main__":
    main()
