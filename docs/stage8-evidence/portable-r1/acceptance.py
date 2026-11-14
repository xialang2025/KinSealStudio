"""Run the frozen Stage8 package on a private desktop with package-only Qt paths."""
#20261114-7 add: Own all processes/evidence and route temporary files to this Z task.
from __future__ import annotations

import base64
from datetime import datetime, timezone
import hashlib
import importlib.util
import json
import os
import struct
from pathlib import Path
import sys

sys.dont_write_bytecode = True
WORKTREE = Path(__file__).resolve().parents[3]
OUTPUT = Path(__file__).resolve().parent / "smoke-r2"
PACKAGE = Path(r"Z:\Project\vscode_prj_temp\REA\KinSealStudio\dist\KinSealStudioLite-Stage8-macro-r1")
EXPECTED = "da8fccee0e615270dfcdf638cd74133fc36a1181dec48da73f0d6a7d2c12ce91"


def sha(path: Path) -> str:
    """Input: file. Return: exact SHA256."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def save(record: dict) -> None:
    """Input: bounded run record. Return: None after saving task-owned evidence."""
    (OUTPUT / "run.json").write_text(json.dumps(record, ensure_ascii=False, indent=2) + "\n",
                                     encoding="utf-8")


def main() -> int:
    """Input: None. Return: zero only after exact content and real process checks pass."""
    executable = PACKAGE / "KinSealStudioLite.exe"
    if sha(executable) != EXPECTED:
        raise ValueError("Frozen package EXE differs.")
    OUTPUT.mkdir(parents=True, exist_ok=False)
    manifest = json.loads((PACKAGE / "manifest.json").read_text(encoding="utf-8"))
    for row in manifest["files"]:
        path = PACKAGE / row["path"]
        if path.stat().st_size != row["bytes"] or sha(path) != row["sha256"] or path.stat().st_nlink != 1:
            raise ValueError("Package content hash/size/link differs: " + str(path))
    oracle = json.loads((WORKTREE / "docs/stage8-evidence/compiler-actual-r2/real-oracle-first.json")
                        .read_text(encoding="utf-8"))
    sdk = Path(oracle["sdk"]["directory"])
    sdk_before = {row["relativePath"]: sha(sdk / row["relativePath"])
                  for row in oracle["sdk"]["requiredFiles"]}
    if any(row["sha256"] != sdk_before[row["relativePath"]] for row in oracle["sdk"]["requiredFiles"]):
        raise ValueError("Frozen external SDK differs.")
    spec = importlib.util.spec_from_file_location(
        "stage8_portable_probe", WORKTREE / "docs/stage1-evidence/gui-portable/probe.py")
    probe = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(probe)
    temporary = OUTPUT / "temporary"
    compiler_temporary = temporary / "compiler"
    compiler_temporary.mkdir(parents=True, exist_ok=False)
    environment = probe.clean_environment(PACKAGE)
    for name in ("TEMP", "TMP", "TMPDIR"):
        environment[name] = str(temporary)
    environment["KINSEAL_MACRO_TEMP_ROOT"] = str(compiler_temporary)
    environment["KINSEAL_ARM_SDK"] = str(sdk)
    for name in ("KINSEAL_MACRO_FIXTURE_MODE", "KINSEAL_MACRO_FIXTURE_STAGE"):
        environment.pop(name, None)
    record = dict(schemaVersion=1, startedAtUtc=datetime.now(timezone.utc).isoformat(),
                  package=str(PACKAGE), executableSha256=EXPECTED, codeCommitAtTest=manifest["codeCommit"],
                  manifestSha256AtTest=sha(PACKAGE / "manifest.json"),
                  sourceContentSha256=manifest["sourceContentSha256"],
                  packageFilesVerified=len(manifest["files"]), everyFileHasOneLink=True,
                  packageOnlyQtPath=True, path=environment["PATH"], temporaryRoot=str(temporary),
                  compilerTemporaryRoot=str(compiler_temporary), externalSdk=str(sdk),
                  sdkRedistributed=False, privateDesktopOnly=True,
                  historicalFirstAttempt=str(OUTPUT.parent / "run.json"),
                  firstAttemptHarnessIssue="首轮三个实际进程均exit0；验收脚本错误假定每宏各有完整头部。此轮独立解析唯一1024B头和连续RAW体；未改生产。",
                  systemInput=False, desktopSwitched=False, foregroundActivationCalled=False,
                  usbTransfer=False, plcIo=False, hmiExecutionConfirmed=False,
                  readOnlyDeviceEnumerationMayOccurInMainWindow=True,
                  runs=[], checks=[])
    save(record)
    desktop = probe.PrivateDesktop(OUTPUT)
    try:
        jobs = [
            ("validate-ordinary-two-macros",
             ["--validate-project", str(PACKAGE / "samples/stage8/ordinary-two-macros.ksp")]),
            ("capture-main",
             ["--capture-preview", str(OUTPUT / "main-window.png"),
              str(PACKAGE / "samples/stage7/macro-not-executed.ksp")]),
            ("compile-ordinary-two-macros",
             ["--compile-macros", str(PACKAGE / "samples/stage8/ordinary-two-macros.ksp"),
              str(OUTPUT / "ordinary-two-macros-report.json")])
        ]
        for name, arguments in jobs:
            run = desktop.run(name, executable, arguments, environment, expected_exit=0, timeout=55)
            run["executableSha256"] = sha(executable)
            record["runs"].append(run)
            save(record)
        image = OUTPUT / "main-window.png"
        compile_report = OUTPUT / "ordinary-two-macros-report.json"
        if compile_report.is_file():
            report = json.loads(compile_report.read_text(encoding="utf-8"))
            indices = report["macroIndices"]
            run_bytes = base64.b64decode(report["marcoRunBase64"], validate=True)
            var_bytes = base64.b64decode(report["marcoVarBase64"], validate=True)
            lengths = [struct.unpack_from("<H", run_bytes, 102 + 2 * index)[0] for index in range(2)]
            raw_hashes = []
            offset = 1024
            for length in lengths:
                raw_hashes.append(hashlib.sha256(run_bytes[offset:offset + length]).hexdigest())
                offset += length
            mappings = report["compiler"]["macros"]
            record["checks"].append(dict(name="real-two-macro-package",
                passed=report.get("ok") is True and indices == {"ClearLB1": 0, "SetLB1": 1}
                       and run_bytes[:2] == bytes.fromhex("02ff") and lengths == [24, 24]
                       and len(run_bytes) == offset == 1072 and var_bytes == bytes(1808)
                       and raw_hashes == [row["rawSha256"] for row in mappings]
                       and raw_hashes[1] == "d6e4f44119b03071a0dea0e8066f084054da6736f8ba5aeb74d443fff914e5da"
                       and report.get("hmiExecutionConfirmed") is False,
                indices=indices, marcoRunBytes=len(run_bytes), marcoVarBytes=len(var_bytes),
                decodedRawLengths=lengths, decodedRawSha256=raw_hashes,
                marcoRunSha256=hashlib.sha256(run_bytes).hexdigest(),
                marcoVarSha256=hashlib.sha256(var_bytes).hexdigest(),
                compilerMappings=report["compiler"].get("compiledMacros", report["compiler"].get("macros"))))
        else:
            record["checks"].append(dict(name="real-two-macro-package", passed=False,
                                         detail="No successful compile report."))
        capture = record["runs"][1]
        qt_modules = [p for p in capture["modulePathsObserved"]
                      if Path(p).name.lower().startswith("qt6") or
                      Path(p).parent.name.lower() in ("platforms", "imageformats", "iconengines", "styles")]
        installed_qt = [p for p in qt_modules if not Path(p).is_relative_to(PACKAGE)]
        record["checks"].append(dict(name="package-only-qt-modules",
            passed=len(qt_modules) >= 4 and not installed_qt,
            modulePaths=qt_modules, unexpectedQtPaths=installed_qt))
        record["checks"].append(dict(name="main-window-image",
            passed=image.is_file() and image.stat().st_size > 10000,
            path=str(image), sha256=sha(image) if image.is_file() else None,
            bytes=image.stat().st_size if image.is_file() else 0))
        sdk_after = {key: sha(sdk / key) for key in sdk_before}
        record["checks"].append(dict(name="external-sdk-unchanged",
                                     passed=sdk_before == sdk_after, fileCount=len(sdk_before)))
        record["compilerTemporaryFilesRemaining"] = [str(p.relative_to(compiler_temporary))
                                                     for p in compiler_temporary.rglob("*")]
        record["passed"] = all(item.get("passed") is True for item in record["runs"] + record["checks"])
    finally:
        desktop.close()
        record["privateDesktopClosed"] = True
        record["completedAtUtc"] = datetime.now(timezone.utc).isoformat()
        save(record)
    print(json.dumps(dict(passed=record["passed"], runs=len(record["runs"]),
                          checks=len(record["checks"]), screenshot=str(OUTPUT / "main-window.png")),
                     ensure_ascii=False))
    return 0 if record["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

