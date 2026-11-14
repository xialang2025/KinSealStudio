"""Revalidate only comment-amended Stage8 samples using the unchanged portable executable."""
#20261114-7 add: Preserve r1/r2 and run the current sample bytes on a private desktop.
from __future__ import annotations
import base64
from datetime import datetime, timezone
import hashlib
import importlib.util
import json
from pathlib import Path
import struct
import sys

sys.dont_write_bytecode = True
WORKTREE = Path(__file__).resolve().parents[3]
EVIDENCE = Path(__file__).resolve().parent
OUTPUT = EVIDENCE / "smoke-r3"
PACKAGE = Path(r"Z:\Project\vscode_prj_temp\REA\KinSealStudio\dist\KinSealStudioLite-Stage8-macro-r1")
EXPECTED = "da8fccee0e615270dfcdf638cd74133fc36a1181dec48da73f0d6a7d2c12ce91"


def sha(path):
    """Input: file. Return: exact SHA256."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def save(record):
    """Input: run record. Return: None after saving this new evidence scope."""
    (OUTPUT / "run.json").write_text(json.dumps(record, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def main():
    """Input: None. Return: zero after current sample validation and actual external compilation pass."""
    executable = PACKAGE / "KinSealStudioLite.exe"
    if sha(executable) != EXPECTED:
        raise ValueError("Frozen production EXE differs.")
    manifest = json.loads((PACKAGE / "manifest.json").read_text(encoding="utf-8"))
    for row in manifest["files"]:
        path = PACKAGE / row["path"]
        if sha(path) != row["sha256"] or path.stat().st_size != row["bytes"] or path.stat().st_nlink != 1:
            raise ValueError("Packaged file identity/link differs: " + row["path"])
    oracle = json.loads((WORKTREE / "docs/stage8-evidence/compiler-actual-r2/real-oracle-first.json").read_text(encoding="utf-8"))
    sdk = Path(oracle["sdk"]["directory"])
    sdk_before = {row["relativePath"]: sha(sdk / row["relativePath"]) for row in oracle["sdk"]["requiredFiles"]}
    if any(sdk_before[row["relativePath"]] != row["sha256"] for row in oracle["sdk"]["requiredFiles"]):
        raise ValueError("External SDK changed.")
    OUTPUT.mkdir(parents=True, exist_ok=False)
    temporary = OUTPUT / "temporary"
    compiler_temporary = temporary / "compiler"
    compiler_temporary.mkdir(parents=True, exist_ok=False)
    spec = importlib.util.spec_from_file_location("stage8_r3_private", WORKTREE / "docs/stage1-evidence/gui-portable/probe.py")
    probe = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(probe)
    environment = probe.clean_environment(PACKAGE)
    for name in ("TEMP", "TMP", "TMPDIR"):
        environment[name] = str(temporary)
    environment["KINSEAL_MACRO_TEMP_ROOT"] = str(compiler_temporary)
    environment["KINSEAL_ARM_SDK"] = str(sdk)
    for name in ("KINSEAL_MACRO_FIXTURE_MODE", "KINSEAL_MACRO_FIXTURE_STAGE"):
        environment.pop(name, None)
    project = PACKAGE / "samples/stage8/ordinary-two-macros.ksp"
    record = dict(schemaVersion=1, startedAtUtc=datetime.now(timezone.utc).isoformat(),
                  executable=str(executable), executableSha256=EXPECTED,
                  manifestSha256AtTest=sha(PACKAGE / "manifest.json"),
                  projectSha256=sha(project), codeCommitAtTest=manifest["codeCommit"],
                  sampleAmendment="sample-refresh-r3.json", productionExecutableChanged=False,
                  historicalScreenshot="../smoke-r2/main-window.png",
                  currentSampleFiles=[row for row in manifest["files"] if row["path"].startswith("samples/stage8/")],
                  privateDesktopOnly=True, path=environment["PATH"], packageOnlyQtPath=True,
                  temporaryRoot=str(temporary), compilerTemporaryRoot=str(compiler_temporary),
                  externalSdk=str(sdk), usbTransfer=False, plcIo=False, systemInput=False,
                  desktopSwitched=False, foregroundActivationCalled=False, hmiExecutionConfirmed=False,
                  runs=[], checks=[])
    save(record)
    desktop = probe.PrivateDesktop(OUTPUT)
    try:
        for name, arguments in (
            ("validate-comment-amended-project", ["--validate-project", str(project)]),
            ("compile-comment-amended-project", ["--compile-macros", str(project), str(OUTPUT / "ordinary-two-macros-report.json")])
        ):
            run = desktop.run(name, executable, arguments, environment, expected_exit=0, timeout=55)
            record["runs"].append(run)
            save(record)
        output = OUTPUT / "ordinary-two-macros-report.json"
        if output.is_file():
            report = json.loads(output.read_text(encoding="utf-8"))
            code = base64.b64decode(report["marcoRunBase64"], validate=True)
            variables = base64.b64decode(report["marcoVarBase64"], validate=True)
            lengths = [struct.unpack_from("<H", code, 102 + 2 * index)[0] for index in range(2)]
            offset = 1024
            raw_sha = []
            for length in lengths:
                raw_sha.append(hashlib.sha256(code[offset:offset + length]).hexdigest())
                offset += length
            record["checks"].append(dict(name="independent-current-two-macro-layout",
                passed=report.get("ok") is True and report["sourceSha256"] == sha(project)
                       and report["macroIndices"] == {"ClearLB1": 0, "SetLB1": 1}
                       and code[:2] == bytes.fromhex("02ff") and lengths == [24, 24]
                       and len(code) == offset == 1072 and variables == bytes(1808)
                       and raw_sha == [row["rawSha256"] for row in report["compiler"]["macros"]]
                       and raw_sha[1] == "d6e4f44119b03071a0dea0e8066f084054da6736f8ba5aeb74d443fff914e5da"
                       and report.get("hmiExecutionConfirmed") is False,
                macroIndices=report["macroIndices"], rawLengths=lengths, rawSha256=raw_sha,
                marcoRunBytes=len(code), marcoVarBytes=len(variables),
                marcoRunSha256=hashlib.sha256(code).hexdigest(), marcoVarSha256=hashlib.sha256(variables).hexdigest()))
            previous = json.loads((EVIDENCE / "smoke-r2/ordinary-two-macros-report.json").read_text(encoding="utf-8"))
            record["checks"].append(dict(name="comments-change-source-identity-without-changing-code-package",
                passed=report["sourceSha256"] != previous["sourceSha256"]
                       and report["marcoRunBase64"] == previous["marcoRunBase64"]
                       and report["marcoVarBase64"] == previous["marcoVarBase64"],
                beforeProjectSha256=previous["sourceSha256"], currentProjectSha256=report["sourceSha256"]))
        else:
            record["checks"].append(dict(name="successful-compile-report", passed=False))
        qt_paths = sorted({path for run in record["runs"] for path in run["modulePathsObserved"]
                           if Path(path).name.lower().startswith("qt6")})
        record["checks"].append(dict(name="observed-qt-paths-remain-packaged",
            passed=bool(qt_paths) and all(Path(path).is_relative_to(PACKAGE) for path in qt_paths),
            modulePaths=qt_paths))
        record["checks"].append(dict(name="sdk-and-compiler-temporary-state",
            passed=sdk_before == {key: sha(sdk / key) for key in sdk_before} and not list(compiler_temporary.rglob("*")),
            sdkFilesChecked=len(sdk_before), temporaryFilesRemaining=[str(p) for p in compiler_temporary.rglob("*")]))
        record["passed"] = all(item.get("passed") is True for item in record["runs"] + record["checks"])
    finally:
        desktop.close()
        record["privateDesktopClosed"] = True
        record["completedAtUtc"] = datetime.now(timezone.utc).isoformat()
        save(record)
    print(json.dumps(dict(passed=record["passed"], runs=len(record["runs"]), checks=len(record["checks"]))))
    return 0 if record["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

