"""Cross-check a frozen AMT070 firmware and archived KinSeal DAT/USB evidence."""

# //20261122-1 add
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import sys

from stage10_acceptance import _last_receipt, _native_transfer, parse_json, require


ROOT = Path(__file__).resolve().parents[1]
IMAGE_BASE = 0x8001FFE0
FIRMWARE_SHA = "74d510f150dc8edadf8e967bdb3c00ca5790ca68b9da6f906125a5234a485c24"
ARCHIVE = "c80c7d5582634c404852f116f75f34ebc023501b"
FIRMWARE = ROOT / "bin/AMT070W124-L800X480R.bin"
LISTING = ROOT / "firmware_analysis/amt070-disasm-20261119/full_disassembly.asm"
ROW = re.compile(r"^([0-9A-F]{6})\s+([0-9A-F]{8})\s+(ARM|Thumb|DATA)\s+([0-9a-f]+)\s+(.*)$")
FILE_PARAM = re.compile(r"\bPARAM ([0-9a-f]{8})\b", re.IGNORECASE)
PROJECT_DAT = {
    "AddrPara.dat", "AlarmPara.dat", "Collect.dat", "ControlPara.dat",
    "ControlPixel.dat", "Curve.dat", "PicLenPara.dat", "Recipe.dat",
    "SheetPara.dat", "SheetPixel.dat", "SysPara.dat",
}
USB_CASES = (
    ("physical-r1", "static-10", 10),
    ("physical-r2", "dynamic-12", 12),
    ("physical-r2", "macro-14", 14),
)
NATIVE_CASES = {
    "Collect.dat": (
        ("history-data", "history-data-default-r2", 1),
        ("history-data", "history-data-shrunk-r6", 2),
        ("history-data", "history-data-custom-r5", 3),
    ),
    "Curve.dat": (
        ("history-trend", "history-default-r2", 0x2F),
        ("trend", "trend-default-r3", 0x71),
        ("xy-curve", "xy-default-r1", 0x28),
    ),
}

# Bind the projection to the exact instructions that implement the checked rules.
TRACE_WORDS = {
    0x8008D790: 0xE3A00041,  # Collect import, storage selector.
    0x8008D794: 0xEAFE778B,
    0x8006C578: 0xE3A02E7D,  # Collect load length and selector.
    0x8006C580: 0xE3A00041,
    0x8006C584: 0xEBFEFBEE,
    0x8006C58C: 0xE3A02E59,  # Size of the Collect configuration copied into RAM.
    0x8006C594: 0xE1A01004,
    0x8006C598: 0xEBFED197,
    0x8006C59C: 0xEBFFFA45,
    0x8006AECC: 0xE5D4C002,  # Collect item count and storage-unit calculation.
    0x8006AEE0: 0xE0820102,
    0x8006AEE4: 0xE0840180,
    0x8006AEE8: 0xE5D0308C,
    0x8006AEF0: 0xE3530000,
    0x8006AEF4: 0x13530002,
    0x8006AEF8: 0x02811001,
    0x8006AEFC: 0x12811002,
    0x8006AF08: 0xE20100FF,
    0x8006AF18: 0xE5C40001,
    0x8008EFE0: 0xE3A00037,  # Curve import, storage selector.
    0x8008EFE4: 0xEAFE7177,
    0x8006BAAC: 0xE3A00037,  # Curve load, list head, descriptor copy and dispatch.
    0x8006BAB0: 0xEBFEFEA3,
    0x8006BAB4: 0xE5D80000,
    0x8006BAE4: 0xE0860106,
    0x8006BAE8: 0xE0800186,
    0x8006BAEC: 0xE0880100,
    0x8006BAF0: 0xE2801001,
    0x8006BAF4: 0xE3A02034,
    0x8006BAFC: 0xEBFED43E,
    0x8006BB0C: 0xE5D00006,
    0x8006BB10: 0xE3500028,
    0x8006BB14: 0x13500071,
    0x8009AA14: 0xE0801103,  # ControlPara and runtime record addressing.
    0x8009AA1C: 0xE7D20381,
    0x8009AA20: 0xE3A050B9,
    0x8009AA28: 0xE0050594,
    0x8009AA38: 0xE0844185,
    0x8009AA40: 0xE5C40284,
}


def sha256(data):
    """Return the exact byte digest of a firmware, listing or archive blob."""
    return hashlib.sha256(data).hexdigest()


def git_blob(path):
    """Read a fixture from the pinned commit, independent of checkout deletions."""
    return subprocess.run(
        ["git", "show", f"{ARCHIVE}:{path}"], cwd=ROOT, check=True, stdout=subprocess.PIPE
    ).stdout


def firmware_listing(image, listing):
    """Verify inventory bytes against the image, then retain selected code and path references."""
    require(sha256(image) == FIRMWARE_SHA, "Unexpected AMT070 firmware bytes")
    paths = {}
    for match in re.finditer(rb"compile\\[A-Za-z0-9_.]+\.dat\0", image):
        name = match[0][8:-1].decode("ascii")
        if name in PROJECT_DAT:
            paths[IMAGE_BASE + match.start()] = name

    offset = 0
    instructions = {}
    path_refs = {}
    for number, line in enumerate(listing.decode("utf-8-sig").splitlines(), 1):
        row = ROW.fullmatch(line)
        if row is None:
            continue
        file_offset, image_va = (int(row[i], 16) for i in (1, 2))
        raw = bytes.fromhex(row[4])
        require(file_offset == offset and image_va == IMAGE_BASE + offset,
                f"Noncontiguous mapping at listing line {number}")
        require(image[offset:offset + len(raw)] == raw,
                f"Listing bytes differ from firmware at line {number}")
        offset += len(raw)
        if image_va in TRACE_WORDS:
            require(row[3] == "ARM" and len(raw) == 4
                    and int.from_bytes(raw, "little") == TRACE_WORDS[image_va],
                    f"Unexpected traced instruction at 0x{image_va:08X}")
            instructions[image_va] = {"line": number, "raw": raw.hex(), "assembly": row[5]}
        parameter = FILE_PARAM.search(row[5])
        if parameter and (target := int(parameter[1], 16)) in paths:
            path_refs.setdefault(paths[target], []).append(
                {"address": f"0x{image_va:08X}", "line": number, "pathAddress": f"0x{target:08X}"}
            )
    require(offset == len(image), "Incomplete firmware listing")
    require(set(instructions) == TRACE_WORDS.keys(), "Missing trace instructions")
    require(path_refs.keys() == PROJECT_DAT, "Missing annotated compile path references")
    return instructions, path_refs


def shift(address):
    """Extract the immediate register shift from a checked ARM instruction."""
    return (TRACE_WORDS[address] >> 7) & 31


def operand(address):
    """Return the ARM data-processing immediate after its encoded rotation."""
    word = TRACE_WORDS[address]
    value = word & 0xFF
    rotate = ((word >> 8) & 15) * 2
    return ((value >> rotate) | (value << (32 - rotate))) & 0xFFFFFFFF


def rules_from_code():
    """Recover the offsets, strides and constants used by the selected ARM slices."""
    collect_selector = operand(0x8008D790)
    curve_selector = operand(0x8008EFE0)
    require(collect_selector == operand(0x8006C580)
            and curve_selector == operand(0x8006BAAC), "Import/load selector mismatch")
    curve_stride = (1 + (1 << shift(0x8006BAE4)) + (1 << shift(0x8006BAE8))) << shift(0x8006BAEC)
    require(curve_stride == operand(0x8006BAF4), "Curve stride and copy size differ")
    return {
        "Collect.dat": {
            "storageSelector": collect_selector,
            "runtimeReadRequestBytes": operand(0x8006C578),
            "configurationCopyBytes": operand(0x8006C58C),
            "itemCountOffset": TRACE_WORDS[0x8006AECC] & 0xFFF,
            "classOffsetForFirstItem": TRACE_WORDS[0x8006AEE8] & 0xFFF,
            "itemClassStride": (1 + (1 << shift(0x8006AEE0))) << shift(0x8006AEE4),
            "oneUnitClasses": [operand(0x8006AEF0), operand(0x8006AEF4)],
            "matchedClassUnits": operand(0x8006AEF8),
            "otherClassUnits": operand(0x8006AEFC),
            "runtimeAccumulatorMask": operand(0x8006AF08),
            "runtimeAccumulatorOffset": TRACE_WORDS[0x8006AF18] & 0xFFF,
        },
        "Curve.dat": {
            "storageSelector": curve_selector,
            "descriptorCountOffset": TRACE_WORDS[0x8006BAB4] & 0xFFF,
            "firstDescriptorOffset": operand(0x8006BAF0),
            "descriptorStride": curve_stride,
            "dispatchTagOffset": TRACE_WORDS[0x8006BB0C] & 0xFFF,
            "comparedTags": [operand(0x8006BB10), operand(0x8006BB14)],
        },
        "ControlPara.dat": {
            "fileRecordStride": (1 + (1 << shift(0x8009AA14))) << shift(0x8009AA1C),
            "runtimeRecordStride": operand(0x8009AA20) << shift(0x8009AA38),
            "runtimeTypeOffset": TRACE_WORDS[0x8009AA40] & 0xFFF,
        },
    }


def project_collect(data, rule):
    """Apply the firmware's item-class and unit-total arithmetic to a Collect image."""
    require(len(data) == rule["configurationCopyBytes"], "Unexpected Collect.dat length")
    count = data[rule["itemCountOffset"]]
    positions = [rule["classOffsetForFirstItem"] + i * rule["itemClassStride"] for i in range(count)]
    require(not positions or positions[-1] < len(data), "Collect class field lies outside the file")
    classes = [data[position] for position in positions]
    units = sum(rule["matchedClassUnits"] if value in rule["oneUnitClasses"] else rule["otherClassUnits"]
                for value in classes)
    return {"items": count, "classOffsets": positions, "classes": classes,
            "calculatedRuntimeUnits": units & rule["runtimeAccumulatorMask"]}


def project_curve(data, rule):
    """Follow the firmware's descriptor slices and recover the observed tag bytes."""
    require(len(data) == 501, "Unexpected Curve.dat sample length")
    count = data[rule["descriptorCountOffset"]]
    first = rule["firstDescriptorOffset"]
    stride = rule["descriptorStride"]
    end = first + count * stride
    require(end <= len(data), "Curve descriptors lie outside the file")
    positions = range(first, end, stride)
    return {"descriptors": count, "descriptorSpan": [first, end],
            "tags": [data[pos + rule["dispatchTagOffset"]] for pos in positions]}


def check_manifest_plan(manifest, contract, sample):  # //20261122-2 add
    """Match every compiler step to the audited command and its optional payload."""
    segments = {item["name"]: item for item in contract["segments"]}
    empty_sha = sha256(b"")
    plan = []
    for operation in contract["operations"]:
        name, _, suffix = operation["name"].rpartition("/")
        if suffix != "command":
            continue
        segment = segments.get(name, {})
        plan.append({
            "command": int(operation["commandHex"], 16),
            "name": name,
            "optionsHex": operation["optionsHex"],
            "wireBytes": segment.get("wireBytes", 0),
            "wireSha256": segment.get("wireSha256", empty_sha),
        })
    require(manifest.get("complete") is True and manifest.get("transferPlan") == plan,
            f"Compiler transfer plan differs from contract: {sample}")


def check_usb_case(run_name, sample, expected, rules):  # //20261122-2 change
    """Bind the archived source, compiler package and historical USB receipt to the contract."""
    run_root = f"docs/stage6-evidence/{run_name}"
    package = f"{run_root}/{sample}-compiled"
    manifest = parse_json(git_blob(f"{package}/compiler-manifest.json"))
    run = parse_json(git_blob(f"{run_root}/run.json"))
    contract = parse_json(git_blob(f"{run_root}/{sample}-contract.json"))
    receipt_name = f"{sample}-download-stdout.txt"
    receipt = _last_receipt(git_blob(f"{run_root}/{receipt_name}"))
    audit = _native_transfer(receipt, contract)
    require(audit["transportComplete"] and len(contract["segments"]) == expected,
            f"USB contract did not close for {sample}")
    identity = contract["identity"]
    require(run["executableSha256"] == manifest["compilerExecutableSha256"]
            and run["model"] == manifest["targetModel"] == identity["targetModel"]
            and manifest["inputProjectSha256"] == identity["modelSha256"],
            f"Program, target or project identity mismatch for {sample}")
    source_path = f"samples/stage6/{sample}.ksp"
    source = git_blob(source_path)
    # These snapshots were read by the Windows runner with CRLF after checkout.
    source_crlf = source.replace(b"\r\n", b"\n").replace(b"\n", b"\r\n")
    source_sha = sha256(source_crlf)
    require(source_sha == identity["sourceSha256"], f"Archived KSP source hash mismatch: {sample}")
    check_manifest_plan(manifest, contract, sample)
    downloads = [item for item in run["runs"] if item.get("name") == f"{sample}-download"]
    require(run.get("usbAccessed") is True and len(downloads) == 1
            and downloads[0].get("stdout") == receipt_name and downloads[0].get("exitCode") == 0,
            f"Physical USB runner did not confirm the successful receipt: {sample}")
    require(run.get("firmwareVersion") == identity.get("firmwareVersion") == "not-observed"
            and manifest.get("firmwareVersionConfirmed") is False,
            f"Unexpected firmware version claims in the historical evidence: {sample}")

    files = {}
    matches = {}
    for segment in contract["segments"]:
        name = segment["name"]
        data = git_blob(f"{package}/{name}")
        meta = manifest["files"][name]
        require((len(data), sha256(data)) == (meta["length"], meta["sha256"]),
                f"Compiled bytes failed manifest binding: {sample}/{name}")
        wire = len(data).to_bytes(4, "big") + data if name == "upLoad.knr" else data
        require((len(wire), sha256(wire)) == (segment["wireBytes"], segment["wireSha256"]),
                f"Compiled bytes failed wire binding: {sample}/{name}")
        files[name] = data
        matches[name] = {"bytes": len(data), "sha256": meta["sha256"]}

    require(set(files) == set(manifest["files"]), f"Manifest segment inventory differs for {sample}")
    stride = rules["ControlPara.dat"]["fileRecordStride"]
    control = files.get("ControlPara.dat", b"")
    control_count = files["SheetPara.dat"][2]
    require(len(control) == control_count * stride, f"Control records do not match {sample} page count")
    indices = files["PicLenPara.dat"]
    frame_count = struct.unpack_from("<I", indices, 0x640)[0]
    require(len(indices) == 2400 + 12 * frame_count, f"Image index size mismatch for {sample}")
    return {
        "archiveDirectory": package, "wireSegments": expected, "audit": audit,
        "source": {"path": source_path, "windowsCrlfSha256": source_sha},
        "compilerExecutableSha256": run["executableSha256"],
        "targetModel": identity["targetModel"], "modelSha256": identity["modelSha256"],
        "historicalRunner": {"usbAccessed": run["usbAccessed"], "exitCode": downloads[0]["exitCode"],
                             "firmwareVersion": run["firmwareVersion"]},
        "controlRecords": control_count, "controlTypes": list(control[::stride]),
        "controlFrames": frame_count, "files": matches,
        "collect": project_collect(files["Collect.dat"], rules["Collect.dat"]),
        "curve": project_curve(files["Curve.dat"], rules["Curve.dat"]),
    }


def check_native_cases(rules):
    """Apply the discovered fields to original V5.1 samples with independent settings."""
    evidence = {}
    for name, cases in NATIVE_CASES.items():
        results = {}
        for family, sample, expected in cases:
            path = f"docs/stage4-evidence/{family}/native/compiled/{sample}/{name}"
            blob = git_blob(path)
            if name == "Collect.dat":
                fields = project_collect(blob, rules[name])
                require(fields["items"] == expected and fields["calculatedRuntimeUnits"] == expected,
                        f"Unexpected active Collect sample: {sample}")
            else:
                fields = project_curve(blob, rules[name])
                require(fields["descriptors"] == 1 and fields["tags"] == [expected],
                        f"Unexpected active Curve sample: {sample}")
            results[sample] = {"path": path, "sha256": sha256(blob), **fields}
        evidence[name] = results
    return evidence


def main():
    """Compare exact input identities and emit, store or recheck deterministic JSON evidence."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--firmware", type=Path, default=FIRMWARE)
    parser.add_argument("--listing", type=Path, default=LISTING)
    output = parser.add_mutually_exclusive_group()
    output.add_argument("--write", type=Path, help="Write evidence to a new JSON file")
    output.add_argument("--check", type=Path, help="Compare a JSON report with current results")
    args = parser.parse_args()

    image, listing = args.firmware.read_bytes(), args.listing.read_bytes()
    instructions, paths = firmware_listing(image, listing)
    rules = rules_from_code()
    usb = {sample: check_usb_case(run, sample, count, rules) for run, sample, count in USB_CASES}
    templates = {}
    for name in ("AlarmPara.dat", "Collect.dat", "Curve.dat"):
        variants = [case["files"][name] for case in usb.values()]
        require(all(meta == variants[0] for meta in variants), f"{name} differs across default cases")
        templates[name] = variants[0]

    report = {
        "format": "KinSeal-AMT070-DAT-firmware-crosscheck-v2",
        "archiveCommit": ARCHIVE,
        "firmware": {"length": len(image), "sha256": sha256(image)},
        "listingSha256": sha256(listing),
        "execution": "static ARM rules projected on archived samples",
        "newHardwareIo": False,
        "rules": rules,
        "instructions": {f"0x{addr:08X}": evidence for addr, evidence in instructions.items()},
        "annotatedImportPaths": paths,
        "usbCases": usb,
        "sameDefaultFilesAcrossUsbCases": templates,
        "originalCompilerSamples": check_native_cases(rules),
    }
    rendered = json.dumps(report, indent=2, ensure_ascii=False, sort_keys=True) + "\n"
    if args.check:
        require(json.loads(args.check.read_bytes()) == report, "Evidence JSON differs from replay")
    elif args.write:
        with args.write.open("x", encoding="utf-8", newline="\n") as stream:
            stream.write(rendered)
    else:
        sys.stdout.write(rendered)
    if args.check or args.write:
        print("Verified 3 historical USB packages, 6 original compiler samples, and firmware listing bytes.")


if __name__ == "__main__":
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        raise SystemExit(f"AMT070 cross-check failed: {error}") from error
