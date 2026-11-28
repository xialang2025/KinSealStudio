"""Compare V122 reopened lamp archives with eleven independent original project and media pairs."""
# 20261128-33 add
import argparse
import gzip
import hashlib
import json
from pathlib import Path
import struct
import tempfile
import time
import zlib

from verify_native_drawing_drafts import ROOT, git_blob, run
from verify_native_lamp_monitor_words import FOLDER, SAMPLES
from verify_amz_button_frame_rules import NATIVE_SHA


def field_map(exe, directory, data):
    """Read the independently located wire spans from one complete copy of the source."""
    sm = directory / "inspect.sm"
    output = directory / "fields.json"
    sm.write_bytes(data)
    run(exe, directory, "--inspect-native-source", sm, output)
    return {field["path"].removeprefix("object[0]."): (field["start"], field["end"])
            for field in json.loads(output.read_bytes())["fields"] if field["path"].startswith("object[0].")}


def edit(source, fields, replacements):
    """Replace validated scalar spans in descending order, including variable-length Qt strings."""
    data = bytearray(source)
    for key in sorted(replacements, key=lambda name: fields[name][0], reverse=True):
        start, end = fields[key]
        data[start:end] = replacements[key]
    return bytes(data)


def unpack(archive):
    """Check the archive's big-endian length as well as its compressed payload."""
    data = zlib.decompress(archive[4:])
    if struct.unpack(">I", archive[:4])[0] != len(data):
        raise ValueError("invalid Qt length prefix")
    return data


def original_readers(executable):
    """Execute the shipped x86 constructor, enum reader and V122 monitor-flag instructions on controlled values."""
    import pefile
    from unicorn import Uc, UC_ARCH_X86, UC_MODE_32
    from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_EIP

    binary = executable.read_bytes()
    digest = hashlib.sha256(binary).hexdigest()
    if digest.lower() != NATIVE_SHA.lower():
        raise ValueError("the original x86 reader requires the recorded V5.1 build")
    pe = pefile.PE(data=binary, fast_load=True)
    cpu = Uc(UC_ARCH_X86, UC_MODE_32)
    obj = 0x10000
    cpu.mem_map(obj, 4096)
    cpu.reg_write(UC_X86_REG_EBX, obj)
    for page in (0x657000, 0x699000):
        cpu.mem_map(page, 4096)
        cpu.mem_write(page, pe.get_data(page - pe.OPTIONAL_HEADER.ImageBase, 4096))
    for permission in (0, 1, 2, 0x12345678, 0xffffffff):
        cpu.mem_write(obj + 0x58, bytes.fromhex("aaaaaaaa"))
        cpu.emu_start(0x65795e, 0x657965, count=2)
        cpu.reg_write(UC_X86_REG_EAX, permission)
        cpu.emu_start(0x65723e, 0x65724d, count=10)
        if cpu.reg_read(UC_X86_REG_EIP) != 0x65724d or bytes(cpu.mem_read(obj + 0x58, 4)) != struct.pack("<I", int(permission == 1)):
            raise ValueError("unexpected original access enum result")
    for source in (0, 1, 2, 128, 255):
        cpu.mem_write(obj + 0x257, bytes([source]))
        cpu.mem_write(obj + 0x248, bytes([0xff]))
        cpu.emu_start(0x6993d3, 0x6993e1, count=3)
        if cpu.reg_read(UC_X86_REG_EIP) != 0x6993e1 or bytes(cpu.mem_read(obj + 0x248, 1)) != bytes([int(bool(source))]):
            raise ValueError("unexpected original monitor selector")
    return {"originalSha256": digest, "accessEnumCases": 5, "monitorFlagCases": 5}


def snapshot(exe, directory, sm, name, *context):
    """Generate an editable copy through the public snapshot command."""
    out = directory / (name + ".knr")
    run(exe, directory, "--build-native-media-snapshot", sm, out, *context)
    return out.read_bytes()


def variants(exe, directory, sm, source, expected, context):
    """Exercise non-default enum, address, SVG sizing, independent sizing and ordinary USB/source routes."""
    fields = field_map(exe, directory, source)
    reference = unpack(expected)
    ref_fields = field_map(exe, directory, reference)
    positive = ((0, 0, 59), (1, 1, 73), (0x12345678, 0x80, 117))
    for index, (permission, address_bit, height) in enumerate(positive):
        raw_height = struct.pack(">d", height)
        changed = {"common.58.0": struct.pack(">I", permission), "switch.254.6": bytes([address_bit]),
                   "switch.248.0": bytes([int(not address_bit)]), "switch.22c.0": bytes([0x80]),
                   "rectangle.3": raw_height, "pixmap.5": raw_height}
        sm.write_bytes(edit(source, fields, changed))
        changes_expected = dict(changed, **{"common.58.0": struct.pack(">I", int(permission == 1)),
                                "switch.248.0": bytes([int(bool(address_bit))]), "switch.22c.0": bytes([1]),
                                "pixmap.5": struct.pack(">d", height / 58 * 58)})
        if unpack(snapshot(exe, directory, sm, "edited-" + str(index), *context)) != edit(reference, ref_fields, changes_expected):
            raise ValueError("controlled permissions, bit-address or SVG bounds differ")

    for name in ("custom-resource", "different-extent"):
        if name == "custom-resource":
            qt_string = "custom-image.kvg".encode("utf-16-be")
            changes = {"pixmap.0": struct.pack(">I", len(qt_string)) + qt_string}
        else:
            changes = {"pixmap.5": struct.pack(">d", 42.25)}
        changed = edit(source, fields, changes)
        sm.write_bytes(changed)
        change_fields = field_map(exe, directory, changed)
        result = unpack(snapshot(exe, directory, sm, name, *context))
        out_fields = field_map(exe, directory, result)
        for path in ("pixmap.0", "pixmap.5"):
            if changed[slice(*change_fields[path])] != result[slice(*out_fields[path])]:
                raise ValueError("custom SVG or independent child sizing was overwritten")
    sm.write_bytes(source)
    for name, options in (("no-context", []), ("directory-only", ["--native-work-directory", "Z:\\lamp"])):
        result = unpack(snapshot(exe, directory, sm, name, *options))
        out_fields = field_map(exe, directory, result)
        for path, span in fields.items():
            if source[slice(*span)] != result[slice(*out_fields[path])]:
                raise ValueError("object changed without an explicit reopen context: " + path)
    run(exe, directory, "--compile-project", sm, directory / "usb")
    if unpack((directory / "usb/upLoad.knr").read_bytes()) != source:
        raise ValueError("ordinary USB export did not preserve the input project")
    return {"fieldVariantCases": len(positive), "preservedCustomOrIndependentSvg": 2,
            "unchangedObjectWithoutReopen": 2, "usbSourceExact": True}


def check(exe, directory, name):
    """Check the original source identity, both complete media outputs and the independent stored reference."""
    directory.mkdir()
    source = git_blob(FOLDER + name, "input-native.sm")
    expected = git_blob(FOLDER + name, "upLoad.knr")
    log = json.loads(git_blob(FOLDER.rsplit("/compiled/", 1)[0], "native-probe-" + name + ".json"))
    origin = log["workingCopyAfter"]
    digest = hashlib.sha256(source).hexdigest()
    if digest.lower() != origin["sha256"].lower():
        raise ValueError("original opened path is linked to different source bytes: " + name)
    sm = directory / "input.sm"
    sm.write_bytes(source)
    context = ("--native-opened-project", origin["path"])
    direct = snapshot(exe, directory, sm, "direct", *context)
    run(exe, directory, "--compile-native-media-parts", sm, directory / "media", *context)
    media = (directory / "media/upLoad.knr").read_bytes()
    if direct != expected or media != expected:
        raise ValueError(name + ": complete original KNR differs, direct=" + str(direct == expected) +
                         " media=" + str(media == expected))
    row = {"sample": name, "sourceSha256": digest, "archiveSha256": hashlib.sha256(expected).hexdigest(),
           "bytes": len(expected), "openedProject": origin["path"], "bothArchivesExact": True}
    if name == "lamp-default-r1":
        row["variants"] = variants(exe, directory, sm, source, expected, context)
    return row


def audit_previous(exe, directory):
    """Replay all previously differing archive paths and record overlapping samples only once."""
    manifest = ROOT / "docs/文件规则/样例/V122显示控件第八次逐工程与逐文件复验_20261128.json"
    previous = json.loads(manifest.read_bytes())["compiledOutputs"]["opened"]["bySource"]
    sm = directory / "previous.sm"
    cases = []
    for digest, row in previous.items():
        if not row["compiled"] or "upLoad.knr" not in row["differences"]:
            continue
        folder = row["reference"]
        source = git_blob(folder, "input-native.sm")
        expected = git_blob(folder, "upLoad.knr")
        log_path = Path(row["openedProjectLog"])
        log = git_blob(log_path.parent.as_posix(), log_path.name)
        origin = json.loads(gzip.decompress(log) if log_path.suffix == ".gz" else log)["workingCopyAfter"]
        if hashlib.sha256(source).hexdigest() != digest or origin["sha256"].lower() != digest:
            raise ValueError("saved run and original source identities differ: " + folder)
        sm.write_bytes(source)
        actual = snapshot(exe, directory, sm, "previous-" + str(len(cases)), "--native-opened-project", origin["path"])
        cases.append({"reference": folder, "compressedExact": actual == expected})
    return {"tested": len(cases), "exact": sum(row["compressedExact"] for row in cases), "rows": cases}


def main():
    """Test both public routes and optionally publish a small evidence receipt."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--native-exe", type=Path, help="also execute ten controlled V5.1 x86 reader cases")
    parser.add_argument("--audit-previous", action="store_true", help="also recompare every prior differing KNR")
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    tmp = tempfile.TemporaryDirectory(prefix="lamp-media-reopen-", dir=ROOT / "build")
    try:
        rows = [check(args.exe.resolve(), Path(tmp.name) / name, name) for name in SAMPLES]
        audit = audit_previous(args.exe.resolve(), Path(tmp.name)) if args.audit_previous else None
    finally:
        # Windows file filters on the mapped drive may briefly hold a just-closed Qt output.
        for attempt in range(6):
            try:
                tmp.cleanup()
                break
            except OSError:
                if attempt == 5:
                    raise
                time.sleep(0.2 * (attempt + 1))
    report = {"independentOriginals": len(rows), "exactPublicRouteOutputs": len(rows) * 2,
              "usbAccessed": False, "sourceRevision": "e3b72657", "rows": rows}
    if args.native_exe:
        report["originalReaderInstructions"] = original_readers(args.native_exe)
    if audit is not None:
        report["previousDifferingArchives"] = audit
    if args.receipt:
        args.receipt.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf8")
    print(f"{len(rows)} independent original KNR files match in both public routes")


if __name__ == "__main__":
    main()
