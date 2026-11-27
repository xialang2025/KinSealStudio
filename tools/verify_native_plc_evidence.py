"""Compare a supplied KinSeal V5.1 installation with the frozen PLC evidence."""
# 20261119-12 add: Keep the installed-file, static ABI, and offline Lite claims separate.
import argparse
from collections import Counter
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import tempfile
import xml.etree.ElementTree as ET

from audit_stage9_protocols import audit


ROOT = Path(__file__).resolve().parents[1]
AMT = "AMT070W124-L800X480R"
PROFILE = "Link/serial_amz/MODBUS/Modbus_RTU.ini"
HOST = "onlineSimulation/onlineV51_800x480.exe"
DRIVER = "PlcDll/Modbus_RTU.dll"
HOST_SHA = "49e278605c1af904f6df336c42f834e6041fb7f3b8f2be7171f004f2f2f0efe7"
DRIVER_SHA = "169e6dcc0cdf5b57d002bb09d947952c157e6cf2adf52eda9f33dbe7f74906f5"


def require(condition, reason):
    """Input: assertion and description. Return: None, or raise on a failed comparison."""
    if not condition:
        raise ValueError(reason)


def identity(path):
    """Input: regular file. Return: byte count and SHA-256 of the complete contents."""
    data = path.read_bytes()
    return {"bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()}


def split_values(text):
    """Input: literal XML list value. Return: ordered values, leaving internal empty slots present."""
    return [value.strip() for value in text.split(",")] if text else []


def xml_fields(element):
    """Input: a flat model or PLC element. Return: the trimmed source text of each child."""
    require(element is not None, "Missing expected XML element")
    fields = {child.tag: (child.text or "").strip() for child in element}
    require(len(fields) == len(element), "Duplicate XML fields")
    return fields


def compare_catalog(installation):
    """Input: explicit V5.1 directory. Return: identity, raw schema, and model comparisons."""
    catalog_path = ROOT / "resources/plc-catalog.json"
    frozen_path = ROOT / "resources/plc-stage9-file-audit.json"
    catalog = json.loads(catalog_path.read_bytes())
    frozen = json.loads(frozen_path.read_bytes())
    fresh = audit(catalog_path, installation, AMT)
    require(fresh["catalogSha256"] == frozen["catalogSha256"], "Catalog digest differs from embedded audit")
    require(fresh["rows"] == frozen["rows"], "Installed references differ from embedded audit rows")
    require(fresh["sourceMatches"] == fresh["profileCount"], "Profile source digest differs")
    files = {row[key]["path"]: row[key]["present"] for row in fresh["rows"]
             for key in ("source", "windowsDll", "com1", "com2") if row[key]["path"]}
    roles = {"bit": "BIT_ADDR_REG", "word": "WORT_ADDR_REG", "dword": "DWORT_ADDR_REG"}
    profiles = {profile["id"]: profile for profile in catalog["profiles"]}
    require(len(profiles) == fresh["profileCount"], "Duplicate profile IDs")
    anomalies, field_counts = [], Counter()
    for key, profile in profiles.items():
        element = ET.fromstring((installation / key).read_bytes()).find("PLC")
        fields = xml_fields(element)
        require(element.get("name") == profile["name"] and fields == profile["rawFields"], f"Raw XML mismatch: {key}")
        require(fields["DriverName"] == profile["driverName"], f"Driver mismatch: {key}")
        family_text = fields.get("ALL_ADDR_REG", "")
        families = split_values(family_text)
        radices = split_values(fields.get("ALL_ADDR_RADIX", ""))
        ranges = split_values(fields.get("ADDR_RANGE", ""))
        require(profile["rawLists"] == {"families": family_text, "radices": radices, "ranges": ranges},
                f"Raw list mismatch: {key}")
        role_fields = {role: split_values(fields.get(tag, "")) for role, tag in roles.items()}
        for family in profile["families"]:
            expected_roles = [role for role, names in role_fields.items() if family["name"] in names]
            require(sorted(expected_roles) == sorted(family["roles"]), f"Role mismatch: {key}/{family['name']}")
        if not len(families) == len(radices) == len(ranges):
            anomalies.append({"profileId": key, "counts": {"names": len(families), "radices": len(radices), "ranges": len(ranges)},
                              "extraRadices": radices[len(families):], "extraRanges": ranges[len(families):],
                              "allNamesTagPresent": "ALL_ADDR_REG" in fields})
        field_counts.update(fields.keys())
    indexes = {}
    indexed_profile_ids = set()
    for tree in ("serial", "serial_amz"):
        path = installation / "Link" / tree / "serial_driver.ini"
        index_bytes = path.read_bytes()
        try:
            index = ET.fromstring(index_bytes)
            manufacturers = [item.get("dir") for item in index.findall("manufacturer")]
            parser_error = None
        except ET.ParseError as error:
            # Record named tags as byte-level observations when the original XML cannot load.
            manufacturers = re.findall(r'<manufacturer\s+dir="([^"]+)"', index_bytes.decode("utf-8-sig"))
            parser_error = str(error)
        parent = path.parent
        observed = {file.relative_to(installation).as_posix() for file in parent.rglob("*.ini") if file != path}
        indexed_profile_ids.update(observed)
        indexes[tree] = identity(path) | {"profileFiles": len(observed),
                                        "manufacturerDirectories": manufacturers,
                                        "strictXml": parser_error is None, "xmlParseError": parser_error}
    require(indexed_profile_ids == set(profiles), "Catalog does not cover actual profile INIs")
    info_path = installation / "modelInfo.dat"
    models = ET.fromstring(info_path.read_bytes()).findall("Model")
    by_model = {model.get("name"): xml_fields(model) for model in models}
    require(len(models) == len(by_model) == len(catalog["models"]), "Model count / identity mismatch")
    for saved in catalog["models"]:
        fields = by_model[saved["name"]]
        require(all(fields[key] == value for key, value in saved.items() if key != "name"),
                f"Model projection differs: {saved['name']}")
    model_path = installation / "model.dat"
    model = next(item for item in ET.fromstring(model_path.read_bytes()).findall("Model") if item.get("name") == AMT)
    selected = next(row for row in fresh["rows"] if row["profileId"] == PROFILE)
    native = by_model[AMT]
    identity_fields = ("Device", "drivePath", "com1Suffix", "com2Suffix", "addrRange", "bDataHL", "maxScreen", "maxControl",
                       "SerialPortDown", "USB", "offlineExe", "onlineExe", "fbottomFile", "cbottomFile")
    return {"catalog": identity(catalog_path), "embeddedAudit": identity(frozen_path), "sameAuditRows": len(fresh["rows"]),
            "profilesByTree": indexes, "referencedPaths": len(files), "presentPaths": sum(files.values()),
            "absentPaths": len(files) - sum(files.values()), "rawXmlProfiles": len(profiles), "rawFieldPresence": dict(sorted(field_counts.items())),
            "literalListAnomalies": anomalies, "modelInfo": identity(info_path), "model": identity(model_path),
            "matchedModels": len(models), "modelApplicableProfiles": fresh["modelApplicableProfiles"],
            "amtModelSettings": {key: native[key] for key in identity_fields},
            "modelsWithSameDeviceId": [model["name"] for model in catalog["models"] if model["Device"] == native["Device"]],
            "amtMarketingFields": xml_fields(model), "defaultProfile": selected,
            "defaultRoles": {"rawFields": profiles[PROFILE]["rawFields"], "listCounts": profiles[PROFILE]["listCounts"]},
            "switchSibling": next(row for row in fresh["rows"] if row["profileId"].endswith("serial_amz/MODBUS/Modbus_Switch.ini"))}


def inspect_native_host(installation):
    """Input: explicit V5.1 directory. Return: focused PE, import, export, and instruction evidence."""
    import pefile
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32

    host_id = identity(installation / HOST)
    driver_id = identity(installation / DRIVER)
    require(host_id["sha256"] == HOST_SHA and driver_id["sha256"] == DRIVER_SHA, "ABI evidence requires the exact host and DLL")
    host = pefile.PE(str(installation / HOST), fast_load=True)
    driver = pefile.PE(str(installation / DRIVER), fast_load=True)
    try:
        require(host.FILE_HEADER.Machine == driver.FILE_HEADER.Machine == 0x14c, "Expected x86 PE")
        host.parse_data_directories(directories=[pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_IMPORT"]])
        driver.parse_data_directories(directories=[pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_EXPORT"]])
        engine = Cs(CS_ARCH_X86, CS_MODE_32)

        def read_bytes(binary, va, size):
            return binary.get_data(va - binary.OPTIONAL_HEADER.ImageBase, size)

        def trace(binary, bounds):
            return [{"address": f"0x{i.address:x}", "bytes": i.bytes.hex(), "instruction": f"{i.mnemonic} {i.op_str}".strip()}
                    for i in engine.disasm(read_bytes(binary, bounds[0], bounds[1] - bounds[0]), bounds[0])]

        imports = {item.name.decode(): f"0x{item.address:x}" for entry in host.DIRECTORY_ENTRY_IMPORT for item in entry.imports
                   if item.name in {b"LoadLibraryA", b"GetProcAddress", b"SetupComm", b"GetCommState", b"SetCommState", b"ReadFile", b"WriteFile"}}
        require(imports.get("LoadLibraryA") == "0x47c00c" and imports.get("GetProcAddress") == "0x47c0d4", "Host loader IAT differs")
        exports = [{"name": item.name.decode(), "ordinal": item.ordinal, "rva": f"0x{item.address:x}"}
                   for item in driver.DIRECTORY_ENTRY_EXPORT.symbols if item.name]
        require(exports == [{"name": "DllFun", "ordinal": 1, "rva": "0x118d4"}], "Driver export differs")
        strings = {f"0x{va:x}": read_bytes(host, va, 40).split(b"\0", 1)[0].decode("ascii")
                   for va in (0x59dff0, 0x59e008, 0x59e024, 0x59e044, 0x59e068, 0x59e4e0)}
        require(strings["0x59e4e0"] == "DllFun" and strings["0x59dff0"].endswith("Modbus_RTU.dll"), "Host string references differ")
        first_entry = struct.unpack("<I", read_bytes(host, 0x42d5c8, 4))[0]
        second_entries = struct.unpack("<5I", read_bytes(host, 0x42d63c, 20))
        require(first_entry == 0x42cc45 and second_entries[2] == 0x42cc58, "Default RTU dispatch table differs")
        host_spans = {"dllPathSwitch": (0x42cc00, 0x42cc93), "primaryLoader": (0x43fa19, 0x43fa53),
                      "secondaryLoader": (0x43fadb, 0x43fb15), "primaryCallback": (0x42d6c0, 0x42d709),
                      "secondaryCallback": (0x42d710, 0x42d759), "descriptorBaseInitialization": (0x450c02, 0x450c16),
                      "primaryTxIndexAndLength": (0x42ba10, 0x42ba7f), "primaryAdvance": (0x42bd80, 0x42bd8e),
                      "primaryAdvanceIndex": (0x42be12, 0x42be62), "secondaryAdvance": (0x42bea0, 0x42beae),
                      "secondaryAdvanceIndex": (0x42bf3c, 0x42bf8c), "primaryResponseGate": (0x433964, 0x43399c),
                      "secondaryResponseGate": (0x433eb8, 0x433ef0), "primaryResponseDispatch": (0x43fdc0, 0x43fe9b),
                      "secondaryResponseDispatch": (0x440a70, 0x440b4b)}
        dll_spans = {"exportThunk": (0x100118d4, 0x100118d9), "sevenArgumentCallback": (0x100123de, 0x10012420),
                     "responseResult": (0x10015475, 0x100154a2), "responseLength": (0x100154f3, 0x10015514)}
        return {"method": "PE bytes and x86 disassembly at pinned mapped addresses", "host": host_id | {"relativePath": HOST},
                "driver": driver_id | {"relativePath": DRIVER}, "iat": imports, "exports": exports, "hostStrings": strings,
                "rawRtuDispatch": {"firstDiscriminator": 1, "firstBranch": f"0x{first_entry:x}", "secondDiscriminator": 2,
                                   "secondBranch": f"0x{second_entries[2]:x}", "secondTableEntries": [f"0x{i:x}" for i in second_entries]},
                "hostInstructionSpans": {key: trace(host, bounds) for key, bounds in host_spans.items()},
                "driverInstructionSpans": {key: trace(driver, bounds) for key, bounds in dll_spans.items()}}
    finally:
        host.close()
        driver.close()


def inspect_lite(executable, expected_sha, scratch):
    """Input: pinned Lite executable and caller-owned temporary root. Return: offline app gate results."""
    actual = identity(executable)
    require(actual["sha256"] == expected_sha.lower(), "Lite executable digest differs")
    seed_path = ROOT / "samples/bit-button/Modbus-COM1.ksp"
    seed = json.loads(seed_path.read_bytes())
    catalog = (ROOT / "resources/plc-catalog.json").read_bytes()
    canon_digest = hashlib.sha256(catalog).hexdigest()
    legacy_digest = hashlib.sha256(catalog.replace(b"\n", b"\r\n")).hexdigest()
    with tempfile.TemporaryDirectory(prefix="native-plc-", dir=scratch) as work:
        env = os.environ.copy()
        env.update(TEMP=work, TMP=work, TMPDIR=work, QT_QPA_PLATFORM="windows")

        def run(*arguments, expected_code=0):
            result = subprocess.run([str(executable), *arguments], cwd=work, env=env, capture_output=True,
                                    timeout=40, creationflags=subprocess.CREATE_NO_WINDOW)
            require(result.returncode == expected_code,
                    f"Lite CLI returned {result.returncode} for {arguments[0:2]}: {result.stdout[:800]!r}")
            return json.loads(result.stdout)

        models = {}
        for model in (AMT, "AMZ070W06", "AMZ070W01", "AMZ070W01R", "LT779C", "unmapped-model"):
            result = run("--plc-capabilities", model)
            profiles = result["profiles"]
            allowed = [row["profileId"] for row in profiles if row["deviceCompileEligible"]]
            require(result["ok"] and result["hardwareIo"] is False and len(profiles) == 139, f"Incomplete capability table: {model}")
            require(allowed == ([PROFILE] if model in (AMT, "AMZ070W01", "AMZ070W01R") else []), f"Unexpected admitted profile for {model}")
            # 20261127-11 change: Both native catalog aliases share the tested PC RTU codec, independently of HMI eligibility.
            expected_pc = {PROFILE, PROFILE.replace("serial_amz", "serial")}
            mapped_pc = {row["profileId"] for row in profiles if row["pcProfileMapped"]}
            require(mapped_pc == expected_pc, f"Unexpected PC RTU profile set for {model}")
            require(not any(any(row[key] for key in ("nativeDllAbiImplemented", "physicalPlcVerified", "hmiPlcVerified"))
                            for row in profiles), f"Unexpected physical or ABI declaration for {model}")
            catalog_digest = result["catalogSha256"]
            require(catalog_digest in (canon_digest, legacy_digest), f"Lite used an unexplained catalog digest: {model}")
            models[model] = {"eligibleProfileIds": allowed, "pcMappedProfileIds": sorted(mapped_pc), "profileCount": len(profiles), "catalogSha256": catalog_digest,
                             "catalogByteIdentity": "exact repository bytes" if catalog_digest == canon_digest else "LF to CRLF only"}
        cases = {}
        for key, changed, expected in (
            ("original", {}, "candidate"), ("COM2", {"channel": "COM2"}, "blocked"),
            ("station2", {"station": "2"}, "blocked"), ("unknownNode", {"node": None}, "blocked"),
            ("node2", {"node": 2}, "blocked"), ("switchProfile", {"profileId": PROFILE.replace("_RTU", "_Switch")}, "blocked"),
            ("sameDeviceIdDifferentModel", {"targetModel": "AMZ070W06"}, "sourceRejected"), ("twoConnections", {"append": True}, "blocked"),
        ):
            sample = copy.deepcopy(seed)
            if "targetModel" in changed:
                sample.update(changed)
            elif "append" in changed:
                sample["connections"].append(sample["connections"][0] | {"id": "second", "channel": "COM2"})
            else:
                sample["connections"][0].update(changed)
            path = Path(work) / f"case-{key}.ksp"  # COM2 alone is a reserved Windows device name.
            path.write_text(json.dumps(sample, ensure_ascii=False), encoding="utf-8")
            project_exit = 13 if expected == "sourceRejected" else 0
            result = run("--project-capabilities", str(path), expected_code=project_exit)
            if project_exit:
                require(result["ok"] is False, f"Unexpected project load result for {key}")
                reason = result["message"]
            else:
                scope = result["deviceCompile"]
                require(scope["status"] == expected and scope["usbAccessed"] is False, f"Unexpected project gate for {key}")
                reason = scope["reason"]
            contract = Path(work) / f"case-{key}-contract.json"
            contract_exit = 0 if expected == "candidate" else 24
            run("--transfer-contract", str(path), str(contract), expected_code=contract_exit)
            require(contract.is_file() == (contract_exit == 0), f"Unexpected contract output for {key}")
            cases[key] = {"changed": changed, "fixture": identity(path), "status": expected, "reason": reason,
                          "projectCapabilitiesExit": project_exit, "offlineTransferContractExit": contract_exit}
            if contract_exit == 0:
                payload = json.loads(contract.read_bytes())
                segment = next(row for row in payload["segments"] if row["name"] == "Modbus_RTU_com1.bin")
                cases[key]["defaultProtocolSegment"] = segment
    return {"executable": actual, "seed": identity(seed_path), "modelGates": models, "projectGates": cases}


def main():
    """Input: command-line installation and report paths. Return: 0 if every comparison passes."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--installation", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--lite-exe", type=Path)
    parser.add_argument("--lite-sha")
    parser.add_argument("--temp-root", type=Path)
    args = parser.parse_args()
    require(bool(args.lite_exe) == bool(args.lite_sha) == bool(args.temp_root), "Specify all or none of the Lite options")
    installation = args.installation.resolve(strict=True)
    output = args.output.resolve()
    require(not output.is_relative_to(installation), "Report may not write into the original installation")
    result = {"schemaVersion": 1, "installation": str(installation), "catalogComparison": compare_catalog(installation),
              "staticHostBoundary": inspect_native_host(installation), "vendorExecutableLaunched": False, "hardwareIo": False}
    if args.lite_exe:
        scratch = args.temp_root.resolve(strict=True)
        require(not scratch.is_relative_to(installation), "Temporary files may not write into the original installation")
        result["liteOfflineCli"] = inspect_lite(args.lite_exe.resolve(strict=True), args.lite_sha, scratch)
        installed_com1 = result["catalogComparison"]["defaultProfile"]["com1"]
        compiled_com1 = result["liteOfflineCli"]["projectGates"]["original"]["defaultProtocolSegment"]
        require(compiled_com1["wireSha256"] == installed_com1["sha256"] and compiled_com1["wireBytes"] == installed_com1["bytes"],
                "Lite default protocol segment differs from the installed COM1 binary")
    with output.open("x", encoding="utf-8", newline="\n") as stream:
        json.dump(result, stream, ensure_ascii=False, indent=2)
        stream.write("\n")
    print(f"Verified {result['catalogComparison']['sameAuditRows']} PLC audit rows, native host instructions, and requested Lite gates.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
