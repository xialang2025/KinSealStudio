"""Validate an explicitly prepared Stage9 package through actual private-desktop processes."""
#20261114-8 add: Keep all test output on Z; never open a PLC port, invoke vendor code or switch desktops.
from __future__ import annotations

import sys
sys.dont_write_bytecode = True

import argparse
from datetime import datetime, timezone
import hashlib
import importlib.util
import json
import os
from pathlib import Path, PurePosixPath
import re
import stat
import struct
WORKTREE = Path(__file__).resolve().parents[1]
AMT = "AMT070W124-L800X480R"
DEFAULT_PROFILE = "Link/serial_amz/MODBUS/Modbus_RTU.ini"
FUNCTIONS = [1, 2, 3, 4, 5, 6, 15, 16]
PLUGIN_DIRS = {"platforms", "imageformats", "iconengines", "styles", "tls", "networkinformation"}


def file_sha(path: Path) -> str:  #20261114-8 add
    """Function Name: file_sha
    Description: Stream exact file bytes without copying a package or loading an executable.
    Input Parameters: path
    Return Value: Lowercase SHA256.
    Date: 2026.11.14
    """
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def identity(path: Path) -> dict:  #20261114-8 add
    """Function Name: identity
    Description: Bind a retained file to its exact size and digest.
    Input Parameters: path
    Return Value: JSON-compatible identity.
    Date: 2026.11.14
    """
    return dict(path=str(path), bytes=path.stat().st_size, sha256=file_sha(path))


def load_json(path: Path, maximum: int = 8 * 1024 * 1024):  #20261114-8 add
    """Function Name: load_json
    Description: Read bounded UTF8 JSON evidence or a packaged source sample.
    Input Parameters: path, maximum
    Return Value: Decoded JSON; oversized or malformed data raises an explicit error.
    Date: 2026.11.14
    """
    if path.stat().st_size > maximum:
        raise ValueError("JSON exceeds bounded evidence size: " + str(path))
    return json.loads(path.read_text(encoding="utf-8-sig"))


def save_json(path: Path, value) -> None:  #20261114-8 add
    """Function Name: save_json
    Description: Replace one task-owned report atomically without modifying package inputs.
    Input Parameters: path, value
    Return Value: None.
    Date: 2026.11.14
    """
    temporary = path.with_name(path.name + ".new")
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    os.replace(temporary, path)


def safe_relative(text: str) -> PurePosixPath:  #20261114-8 add
    """Function Name: safe_relative
    Description: Reject absolute, ambiguous, Windows-stream or traversing manifest paths.
    Input Parameters: text
    Return Value: Canonical relative POSIX path.
    Date: 2026.11.14
    """
    if not isinstance(text, str) or not text or "\\" in text or ":" in text or "\0" in text:
        raise ValueError("Unsafe manifest path: " + repr(text))
    value = PurePosixPath(text)
    if value.is_absolute() or value.as_posix() != text or any(part in (".", "..") for part in value.parts):
        raise ValueError("Noncanonical manifest path: " + text)
    return value


def ordinary(path: Path, require_file: bool = False) -> None:  #20261114-8 add
    """Function Name: ordinary
    Description: Reject reparse points and file hardlinks before trusting a package entry.
    Input Parameters: path, require_file
    Return Value: None when the entry is ordinary and regular as required.
    Date: 2026.11.14
    """
    info = path.lstat()
    if stat.S_ISLNK(info.st_mode) or getattr(info, "st_file_attributes", 0) & 0x400:
        raise ValueError("Reparse entry is outside acceptance scope: " + str(path))
    if require_file and (not stat.S_ISREG(info.st_mode) or info.st_nlink != 1):
        raise ValueError("File is not an ordinary single-link copy: " + str(path))


def tree_files(root: Path) -> list[Path]:  #20261114-8 add
    """Function Name: tree_files
    Description: Enumerate a package without following directory or file reparse points.
    Input Parameters: root
    Return Value: Exact sorted ordinary file list.
    Date: 2026.11.14
    """
    ordinary(root)
    files = []
    for current, directories, names in os.walk(root, followlinks=False):
        for name in directories:
            ordinary(Path(current) / name)
        for name in names:
            path = Path(current) / name
            ordinary(path, True)
            files.append(path)
    return sorted(files, key=lambda path: path.relative_to(root).as_posix())


def content_digest(rows: list[dict]) -> str:  #20261114-8 add
    """Function Name: content_digest
    Description: Independently verify the manifest's ordered path/hash binding.
    Input Parameters: rows
    Return Value: SHA256 of UTF8(path), NUL, ASCII(hash), LF for every listed entry.
    Date: 2026.11.14
    """
    digest = hashlib.sha256()
    for row in rows:
        safe_relative(row["path"])
        if not isinstance(row.get("sha256"), str) or not re.fullmatch(r"[0-9a-f]{64}", row["sha256"]):
            raise ValueError("Invalid manifest SHA256")
        digest.update(row["path"].encode("utf-8") + b"\0" + row["sha256"].encode("ascii") + b"\n")
    return digest.hexdigest()


def verify_rows(root: Path, rows: list[dict]) -> dict:  #20261114-8 add
    """Function Name: verify_rows
    Description: Verify every declared byte count/hash and reject case-insensitive duplicate paths.
    Input Parameters: root, rows
    Return Value: Bounded identity summary with the declared file set.
    Date: 2026.11.14
    """
    seen = set()
    total = 0
    for row in rows:
        relative = safe_relative(row["path"])
        if str(relative).casefold() in seen:
            raise ValueError("Duplicate manifest path: " + str(relative))
        seen.add(str(relative).casefold())
        path = root.joinpath(*relative.parts)
        ordinary(path, True)
        size = row.get("bytes")
        if type(size) is not int or size < 0 or path.stat().st_size != size or file_sha(path) != row["sha256"]:
            raise ValueError("Manifest identity mismatch: " + str(path))
        total += size
    return dict(fileCount=len(rows), bytes=total, paths=[row["path"] for row in rows])


def verify_package(package: Path, manifest: dict) -> dict:  #20261114-8 add
    """Function Name: verify_package
    Description: Verify complete package membership, links, manifest digests and intended EXE identity.
    Input Parameters: package, manifest
    Return Value: Exact package identity summary; no production process is started here.
    Date: 2026.11.14
    """
    if manifest.get("schemaVersion") != 1 or manifest.get("stage") != 9:
        raise ValueError("An explicit Stage9 package manifest is required")
    rows = manifest["files"]
    actual = [path.relative_to(package).as_posix() for path in tree_files(package)]
    declared = [row["path"] for row in rows] + ["manifest.json"]
    if sorted(actual) != sorted(declared):
        raise ValueError("Package file membership differs from manifest")
    if content_digest(rows) != manifest["fileContentSha256"]:
        raise ValueError("Package file-content digest differs")
    result = verify_rows(package, rows)
    executable = package / "KinSealStudioLite.exe"
    if file_sha(executable) != manifest["executableSha256"]:
        raise ValueError("Manifest EXE identity differs")
    build_exe = WORKTREE / "build/KinSealStudioLite.exe"
    if file_sha(build_exe) != manifest["executableSha256"]:
        raise ValueError("Current intended build and portable EXE differ")
    sources = manifest["sourceFiles"]
    if content_digest(sources) != manifest["sourceContentSha256"]:
        raise ValueError("Manifest source-content digest differs")
    source_result = verify_rows(WORKTREE, sources)
    return dict(**result, manifest=identity(package / "manifest.json"), executable=identity(executable),
                buildExecutable=identity(build_exe), sourceFileCount=source_result["fileCount"],
                sourceContentSha256=manifest["sourceContentSha256"], everyFileHasOneLink=True,
                noReparseEntries=True, completeMembershipVerified=True)


def capability_checks(report: dict, model: str, expected_eligible: int, audit: dict, catalog: dict) -> list[dict]:  #20261114-8 add
    """Function Name: capability_checks
    Description: Check public semantics independently against source identities and explicit product gates.
    Input Parameters: report, model, expected_eligible, audit, catalog
    Return Value: Named semantic outcomes with bounded observations.
    Date: 2026.11.14
    """
    profiles = report.get("profiles", [])
    eligible = [row for row in profiles if row.get("deviceCompileEligible") is True]
    by_id = {row["profileId"]: row for row in audit["rows"]}
    catalog_rows = {row["id"]: row for row in catalog["profiles"]}
    raw = report.get("pcRawRtu", {})
    checks = [dict(name="report-envelope", passed=report.get("format") == "KinSealLite-plc-capabilities-v1"
                   and report.get("ok") is True and report.get("model") == model
                   and report.get("profileCount") == len(profiles) == 139),
        dict(name="no-hardware-claim", passed=report.get("hardwareIo") is False
             and report.get("hmiExecutionConfirmed") is False
             and all(row.get("pcProfileMapped") is False and row.get("physicalPlcVerified") is False
                     and row.get("hmiPlcVerified") is False and row.get("nativeDllAbiImplemented") is False
                     for row in profiles)),
        dict(name="explicit-eight-function-pc-scope", passed=raw.get("functions") == FUNCTIONS
             and raw.get("explicitZeroBasedPduAddress") is True and raw.get("serialBackendImplemented") is True
             and raw.get("automaticProjectMapping") is False and raw.get("physicalEndpointVerified") is False
             and raw.get("automaticRetries") == 0),
        dict(name="model-device-gate", passed=len(eligible) == expected_eligible
             and (not eligible or eligible[0].get("profileId") == DEFAULT_PROFILE),
             eligibleProfileIds=[row.get("profileId") for row in eligible]),
        dict(name="catalog-resource-identity", passed=report.get("catalogSha256") == audit["catalogSha256"]
             == file_sha(WORKTREE / "resources/plc-catalog.json")
             and report.get("fileAuditModel") == audit["model"] == AMT)]
    identities_match = len(profiles) == len(by_id) == len(catalog_rows) == 139
    identities_match = identities_match and len({row.get("profileId") for row in profiles}) == 139
    for row in profiles:
        original = by_id.get(row.get("profileId"))
        original_catalog = catalog_rows.get(row.get("profileId"))
        if original is None or original_catalog is None:
            identities_match = False
            continue
        identities_match = identities_match and row.get("filesObserved") == original
        identities_match = identities_match and row.get("driverName") == original_catalog["driverName"]
        identities_match = identities_match and row.get("serialDefaultsRaw") == original_catalog.get("serialDefaults", {})
        families = row.get("families", [])
        identities_match = identities_match and row.get("familyCount") == len(families) == len(original_catalog["families"])
        identities_match = identities_match and row.get("grammarFamilyCount") == sum(
            family.get("addressGrammarImplemented") is True for family in families)
    checks.append(dict(name="all-profile-file-observations-match-audit", passed=identities_match,
                       profilesCompared=len(profiles), auditSha256=file_sha(WORKTREE / "resources/plc-stage9-file-audit.json")))
    if expected_eligible == 1:
        scope = eligible[0].get("deviceScope", {}) if eligible else {}
        checks.append(dict(name="restricted-amt-device-scope", passed=scope == dict(
            channel="COM1", stationText="1", nodeKnown=True, node=1, route=1, singleConnectionSinglePage=True)))
    return checks


def main() -> int:  #20261114-8 add
    """Function Name: main
    Description: Run a prepared portable package on one private desktop and preserve every process receipt.
    Input Parameters: --package, --output
    Return Value: Zero only after complete package, CLI, module and startup checks pass.
    Date: 2026.11.14
    """
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    package = args.package.absolute()
    output = args.output.absolute()
    if package.drive.lower() != "z:" or output.drive.lower() != "z:":
        parser.error("Package and new output must be on Z; no temporary files belong on C")
    ordinary(package)
    if output.exists() or output.is_relative_to(package) or package.is_relative_to(output):
        parser.error("Output must be new and separate from the immutable package")
    output.mkdir(parents=True, exist_ok=False)
    temporary = output / "temporary"
    temporary.mkdir()
    macro_temp = temporary / "macro"
    macro_temp.mkdir()
    for name in ("TEMP", "TMP", "TMPDIR"):
        os.environ[name] = str(temporary)
    record = dict(schemaVersion=1, stage=9, startedAtUtc=datetime.now(timezone.utc).isoformat(),
        checker=identity(Path(__file__)), package=str(package), temporaryRoot=str(temporary),
        runs=[], checks=[], privateDesktopOnly=True, systemInput=False, desktopSwitched=False,
        foregroundActivationCalled=False, portOpenRequested=False, usbTransfer=False, plcIo=False,
        hmiExecutionConfirmed=False, readOnlyDeviceEnumerationMayOccurInMainWindow=True,
        scope="Real portable CLI/startup processes with no physical PLC/HMI acceptance", passed=False)
    desktop = None
    manifest = None

    def save() -> None:
        """Input: None. Return: None after retaining the latest task-owned receipt."""
        save_json(output / "run.json", record)

    def check(name: str, passed: bool, **detail) -> None:
        """Input: check identity, outcome and bounded observations. Return: None after preserving evidence."""
        record["checks"].append(dict(name=name, passed=bool(passed), **detail))
        save()

    try:
        manifest = load_json(package / "manifest.json")
        record["packageIdentityBefore"] = verify_package(package, manifest)
        record["codeCommitAtTest"] = manifest.get("codeCommit")
        record["codeCommitStateAtTest"] = manifest.get("codeCommitState")
        check("preflight-complete-byte-and-link-identity", True,
              verifiedFiles=len(manifest["files"]), sourceFiles=len(manifest["sourceFiles"]))
        declared_capabilities = manifest.get("capabilities", {})
        check("manifest-conservative-scope", manifest.get("hardwareIo") is False
              and manifest.get("hmiExecutionConfirmed") is False
              and manifest.get("vendorExecutableRedistributed") is False and manifest.get("sdkRedistributed") is False
              and declared_capabilities.get("rawPcRtuFunctions") == FUNCTIONS
              and all(declared_capabilities.get(key) is False for key in (
                  "automaticProjectMapping", "fullNativeDllAbi", "physicalPlcVerified", "hmiPlcVerified")))
        audit = load_json(WORKTREE / "resources/plc-stage9-file-audit.json")
        catalog = load_json(WORKTREE / "resources/plc-catalog.json")
        record["fileAuditIdentity"] = identity(WORKTREE / "resources/plc-stage9-file-audit.json")
        record["originalInstallationRehashedByThisChecker"] = False
        sample = package / "samples/stage8/ordinary-two-macros.ksp"
        project = load_json(sample)
        check("packaged-stage8-sample-identity", file_sha(sample) == file_sha(WORKTREE / sample.relative_to(package))
              and sorted(project.get("macros", {})) == ["ClearLB1", "SetLB1"], sample=identity(sample))
        probe_path = WORKTREE / "docs/stage1-evidence/gui-portable/probe.py"
        spec = importlib.util.spec_from_file_location("stage9_private_portable_probe", probe_path)
        probe = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(probe)
        environment = probe.clean_environment(package)
        for name in ("TEMP", "TMP", "TMPDIR"):
            environment[name] = str(temporary)
        environment["KINSEAL_MACRO_TEMP_ROOT"] = str(macro_temp)
        for name in ("KINSEAL_ARM_SDK", "KINSEAL_MACRO_FIXTURE_MODE", "KINSEAL_MACRO_FIXTURE_STAGE"):
            environment.pop(name, None)
        record["probe"] = identity(probe_path)
        record["path"] = environment["PATH"]
        record["packageOnlyQtEnvironment"] = True
        record["qtPluginPath"] = environment.get("QT_QPA_PLATFORM_PLUGIN_PATH")
        save()
        desktop = probe.PrivateDesktop(output)
        executable = package / "KinSealStudioLite.exe"
        image = output / "main-window.png"
        jobs = [
            ("capabilities-default", ["--plc-capabilities"], 0),
            ("capabilities-explicit-amt", ["--plc-capabilities", AMT], 0),
            ("capabilities-lt779", ["--plc-capabilities", "LT779"], 0),
            ("capabilities-unknown", ["--plc-capabilities", "KS-UNKNOWN-MODEL-NO-PROFILE"], 0),
            ("capabilities-too-many-arguments", ["--plc-capabilities", AMT, "unexpected"], 2),
            ("validate-packaged-stage8-project", ["--validate-project", str(sample)], 0),
            ("capture-main", ["--capture-preview", str(image), str(sample)], 0)]
        for name, arguments, expected in jobs:
            receipt = desktop.run(name, executable, arguments, environment, expected_exit=expected, timeout=45)
            receipt["executableSha256"] = file_sha(executable)
            for stream in ("stdout", "stderr"):
                path = output / receipt[stream]
                if path.is_file():
                    receipt[stream + "Identity"] = identity(path)
            record["runs"].append(receipt)
            save_json(output / (name + "-process.json"), receipt)
            save()
        parsed = {}
        for index, (name, model, eligible_count) in enumerate([
            ("default", AMT, 1), ("explicit-amt", AMT, 1), ("lt779", "LT779", 0),
            ("unknown", "KS-UNKNOWN-MODEL-NO-PROFILE", 0)]):
            receipt = record["runs"][index]
            try:
                report = load_json(output / receipt["stdout"])
                if not isinstance(report, dict):
                    raise ValueError("Capability stdout is not an object")
                parsed[name] = report
                for outcome in capability_checks(report, model, eligible_count, audit, catalog):
                    check("capabilities-" + name + "/" + outcome.pop("name"), **outcome)
            except Exception as exc:
                check("capabilities-" + name + "/json-and-semantics", False, error=repr(exc))
        check("default-and-explicit-amt-identical", "default" in parsed and "explicit-amt" in parsed
              and parsed["default"] == parsed["explicit-amt"])
        invalid = record["runs"][4]
        check("invalid-extra-argument-exit2", invalid.get("exitCode") == 2)
        capture = record["runs"][-1]
        modules = sorted({path for receipt in record["runs"] for path in receipt.get("modulePathsObserved", [])})
        qt_modules = [path for path in modules if Path(path).name.lower().startswith("qt6")
                      or Path(path).parent.name.lower() in PLUGIN_DIRS]
        outside = [path for path in qt_modules if not Path(path).resolve().is_relative_to(package.resolve())]
        serial_modules = [path for path in qt_modules if Path(path).name.lower() == "qt6serialport.dll"]
        check("all-observed-qt-modules-from-package", len(qt_modules) >= 4 and not outside,
              observedModules=qt_modules, unexpectedModules=outside)
        check("qtserialport-actually-loaded-from-package", bool(serial_modules)
              and all(Path(path).resolve() == (package / "Qt6SerialPort.dll").resolve() for path in serial_modules),
              observedModules=serial_modules)
        png_size = None
        if image.is_file():
            with image.open("rb") as stream:
                header = stream.read(24)
            if len(header) == 24 and header[:8] == b"\x89PNG\r\n\x1a\n" and header[12:16] == b"IHDR":
                png_size = list(struct.unpack(">II", header[16:24]))
        check("main-window-png", capture.get("passed") is True and png_size == [1353, 708]
              and image.stat().st_size > 10000 if image.is_file() else False,
              dimensions=png_size, image=identity(image) if image.is_file() else None,
              visualInspectionStillRequired=True)
        metadata_path = output / "main-window.png.json"
        metadata = load_json(metadata_path) if metadata_path.is_file() else []
        visible_classes = sorted({row.get("class") for row in metadata if row.get("visible")})
        check("main-window-layout-metadata", {"QMenuBar", "QTreeWidget", "ScreenCanvas", "QStatusBar"}.issubset(visible_classes),
              visibleClasses=visible_classes, metadata=identity(metadata_path) if metadata_path.is_file() else None)
        record["packageIdentityAfter"] = verify_package(package, manifest)
        check("immutable-package-after-actual-processes", record["packageIdentityBefore"] == record["packageIdentityAfter"])
        record["temporaryFilesRemaining"] = [str(path.relative_to(temporary)) for path in temporary.rglob("*") if path.is_file()]
        record["passed"] = all(item.get("passed") is True for item in record["runs"] + record["checks"])
    except Exception as exc:
        record["fatalError"] = repr(exc)
        record["passed"] = False
    finally:
        if desktop is not None:
            desktop.close()
            record["privateDesktopClosed"] = True
        record["completedAtUtc"] = datetime.now(timezone.utc).isoformat()
        save()
    print(json.dumps(dict(passed=record["passed"], runs=len(record["runs"]), checks=len(record["checks"]),
                          report=str(output / "run.json")), ensure_ascii=False))
    return 0 if record["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
