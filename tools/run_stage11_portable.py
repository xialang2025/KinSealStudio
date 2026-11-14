"""Run bounded, private-desktop checks against an explicitly frozen Stage 11 package."""
#20261114-10 add: This acceptance runner has no hardware/serial command path.
from __future__ import annotations

import argparse
import base64
import ctypes
from datetime import datetime, timezone
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import struct
import sys


ROOT = Path(__file__).resolve().parents[1]
SAFE_VERBS = {
    "--release-info", "--help-catalog", "--capture-help", "--capture-preview",
    "--validate-project", "--migrate-project", "--project-capabilities",
    "--export-native-project", "--compile-project", "--verify-compiled-package",
    "--simulation-capabilities", "--simulate-offline", "--compile-macros",
}
EXPECTED_SAMPLES = (
    "samples/stage10/bit-lamp-local.ksp", "samples/stage10/word-lamp-local.ksp",
    "samples/stage10/function-local.ksp", "samples/stage1/stage1-single-page.ksp",
    "samples/stage5/ellipse-default.ksp", "samples/stage7/internal-five-family.ksp",
    "samples/stage8/ordinary-two-macros.ksp",
)


def module_from_path(name: str, path: Path):  #20261114-10 add
    """Load an existing read-only helper without importing from a mutable search path."""
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise ValueError("Cannot load helper: " + str(path))
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


def sha(path: Path) -> str:  #20261114-10 add
    """Hash complete file bytes without newline normalization."""
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def snapshot(root: Path, checker) -> dict:  #20261114-10 add
    """Record a complete package tree after rejecting aliases and nonregular files."""
    return {path.relative_to(root).as_posix(): {"bytes": path.stat().st_size, "sha256": sha(path)}
            for path in checker.tree_files(root)}


def write_json(path: Path, value: object) -> None:  #20261114-10 add
    """Replace only this runner's existing evidence file using a local atomic rename."""
    temporary = path.with_name(path.name + ".new")
    with temporary.open("x", encoding="utf-8", newline="\n") as stream:
        json.dump(value, stream, ensure_ascii=False, indent=2, allow_nan=False)
        stream.write("\n")
    os.replace(temporary, path)


def png_info(path: Path) -> dict:  #20261114-10 add
    """Check the actual PNG signature and dimensions without claiming visual review."""
    header = path.read_bytes()[:24]
    if len(header) != 24 or header[:8] != b"\x89PNG\r\n\x1a\n" or header[12:16] != b"IHDR":
        raise ValueError("Output is not a PNG: " + str(path))
    width, height = struct.unpack(">II", header[16:24])
    if not 0 < width <= 8192 or not 0 < height <= 8192:
        raise ValueError("PNG dimensions exceed expected UI bounds")
    return {"path": path.name, "bytes": path.stat().st_size, "sha256": sha(path),
            "width": width, "height": height, "visuallyReviewed": False}


def supplied_differences(original, saved, path="") -> list[dict]:  #20261114-10 add
    """List every changed or removed supplied value; added canonical defaults are reported separately by the saved file."""
    if isinstance(original, dict):
        if not isinstance(saved, dict):
            return [{"path": path, "before": original, "after": saved}]
        return [change for key, value in original.items() for change in
                ([{"path": path + "/" + key, "before": value, "removed": True}] if key not in saved
                 else supplied_differences(value, saved[key], path + "/" + key))]
    if isinstance(original, list):
        if not isinstance(saved, list) or len(original) != len(saved):
            return [{"path": path, "before": original, "after": saved}]
        return [change for index, (left, right) in enumerate(zip(original, saved))
                for change in supplied_differences(left, right, path + "/" + str(index))]
    return [] if original == saved else [{"path": path, "before": original, "after": saved}]


def canonical_empty_normalization(changes: list[dict]) -> bool:  #20261114-10 add
    """Permit only the independently observed minimal-schema conversion, never arbitrary removed fields or values."""
    expected = [
        {"path": "/animations", "before": {}, "removed": True},
        {"path": "/connections", "before": [], "removed": True},
        {"path": "/schemaVersion", "before": 12, "after": 9},
    ]
    return sorted(changes, key=lambda row: row["path"]) == expected


def run_acceptance(args) -> dict:  #20261114-10 add
    """Execute only the frozen portable package in a private desktop and retain truthful layer-specific results."""
    checker = module_from_path("stage11_portable_package_checker", ROOT / "tools/package_stage11.py")
    package = checker.checked_path(args.package)
    output = checker.new_output(args.output)
    temporary = checker.new_output(args.temp_root)
    if checker.overlaps(package, output) or checker.overlaps(package, temporary) or checker.overlaps(output, temporary):
        raise ValueError("Package, evidence and temporary directories must be disjoint")
    verification = checker.verify_package(package, args.expected_manifest_sha.lower())
    if not verification["verified"] or sha(package / "KinSealStudioLite.exe") != args.expected_exe_sha.lower():
        raise ValueError("Frozen package or executable identity differs; no application started")
    manifest = checker.load_json(package / "manifest.json")
    samples = [row["path"] for row in manifest["files"] if row.get("kind") == "sample"]
    if set(samples) != set(EXPECTED_SAMPLES) or len(samples) != 7:
        raise ValueError("This run requires exactly the seven declared Stage 11 samples")
    output.mkdir(parents=True, exist_ok=False)
    temporary.mkdir(parents=True, exist_ok=False)
    probe = module_from_path("stage11_portable_private_probe", ROOT / "docs/stage1-evidence/gui-portable/probe.py")
    before = snapshot(package, checker)
    write_json(output / "package-before.json", before)
    write_json(output / "package-verification-before.json", verification)
    environment = probe.clean_environment(package)
    for name in ("TEMP", "TMP", "TMPDIR", "KINSEAL_MACRO_TEMP_ROOT"):
        environment[name] = str(temporary)
    environment["KINSEAL_ARM_SDK"] = str(temporary / "intentionally-absent-sdk")
    environment["QT_FORCE_STDERR_LOGGING"] = "1"
    environment["QT_ASSUME_STDERR_HAS_CONSOLE"] = "1"
    # Child loader errors inherit this setting and cannot block behind a user-visible system dialog.
    previous_error_mode = ctypes.windll.kernel32.SetErrorMode(0x0001 | 0x0002 | 0x8000)
    result = {"schemaVersion": 1, "stage": 11, "startedAtUtc": datetime.now(timezone.utc).isoformat(),
              "scriptSha256": sha(Path(__file__)), "package": str(package),
              "executableSha256": args.expected_exe_sha.lower(), "manifestSha256Before": sha(package / "manifest.json"),
              "environment": {key: environment.get(key) for key in ("PATH", "QT_QPA_PLATFORM", "QT_QPA_PLATFORM_PLUGIN_PATH", "TEMP", "TMP", "TMPDIR", "KINSEAL_ARM_SDK")},
              "privateDesktopOnly": True, "foregroundActivationCalled": False, "systemInput": False,
              "hardwareIo": False, "serialPortsOpened": False, "sdkInvoked": False,
              "readOnlyUsbEnumerationMayOccurInMainWindow": True,
              "environmentScope": "Windows开发机清洁进程PATH/插件搜索隔离；不是干净虚拟机或卸载Qt/驱动。",
              "runs": [], "checks": [], "samples": [], "screenshots": [], "cleanup": [],
              "notRun": ["独立干净系统/VM", "普通用户权限与只读安装目录", "长路径上限", "多DPI/多显示器",
                         "删除或替换系统字体", "卸载/安装USB驱动与真实设备", "真实PLC/COM串口", "安装/升级/卸载器", "设备T/E与长期性能"]}
    desktop = probe.PrivateDesktop(output)
    executable = package / "KinSealStudioLite.exe"

    def persist():
        """Persist progress so an interrupted run retains every completed process record."""
        write_json(output / "run.json", result)

    def check(name, passed, **details):
        """Append a semantic assertion independently of the child exit code."""
        result["checks"].append({"name": name, "passed": bool(passed), **details})
        persist()

    def run(name, arguments, expected=0, exe=executable, env=environment, allowed=None, timeout=45):
        """Enforce a fixed read-only/offline verb allowlist before every owned child launch."""
        if not arguments or arguments[0] not in SAFE_VERBS:
            raise ValueError("Hardware or unknown CLI verb is forbidden")
        record = desktop.run(name, exe, [str(item) for item in arguments], env, expected, timeout)
        record["allowedExitCodes"] = list(allowed if allowed is not None else [expected])
        code = record.get("exitCode")
        record["exitCodeUnsigned"] = code & 0xFFFFFFFF if isinstance(code, int) else None
        record["matchesAllowedOutcome"] = record["exitCodeUnsigned"] in record["allowedExitCodes"] and "error" not in record
        result["runs"].append(record)
        persist()
        return record

    def json_stdout(record):
        """Decode a real complete UTF-8 JSON stdout, never guess from a zero exit code."""
        return json.loads((output / record["stdout"]).read_text(encoding="utf-8-sig"))

    def remove_owned(path, purpose):
        """Delete only a verified ordinary descendant of this fresh runner-owned temporary root."""
        resolved = checker.checked_path(path)
        if resolved == temporary or not resolved.is_relative_to(temporary):
            raise ValueError("Cleanup target is outside the owned temporary root")
        files = checker.tree_files(resolved) if resolved.is_dir() else [checker.regular_file(resolved)]
        size = sum(path.stat().st_size for path in files)
        if resolved.is_dir():
            shutil.rmtree(resolved)
        else:
            resolved.unlink()
        result["cleanup"].append({"path": str(resolved), "purpose": purpose, "bytesBefore": size,
                                  "bytesAfter": 0, "deleted": not resolved.exists()})

    try:
        info = json_stdout(run("release-info", ["--release-info"]))
        check("release-version-and-declaration", info.get("version") == "0.11.0" and info.get("hardwareIo") is False
              and info.get("installerProvided") is False and info.get("fullReplicaCertified") is False, observed=info)
        catalog = json_stdout(run("help-catalog", ["--help-catalog"]))
        check("packaged-help-catalog", catalog.get("valid") is True and catalog.get("globalEntryCoverageComplete") is False,
              observed=catalog)
        capture = output / "help-mainwindow-action.png"
        help_capture = json_stdout(run("help-mainwindow-action", ["--capture-help", capture]))
        result["screenshots"].append(png_info(capture))
        check("actual-mainwindow-help-action", help_capture.get("version") == "0.11.0" and
              "开始使用" in help_capture.get("helpText", "") and help_capture.get("deviceTransferPerformed") is False)
        for index, relative in enumerate(EXPECTED_SAMPLES, 1):
            sample = package / relative
            prefix = "sample-" + str(index)
            migrated = output / (prefix + "-saved.ksp")
            row = {"path": relative, "sha256": sha(sample)}
            run(prefix + "-validate", ["--validate-project", sample])
            run(prefix + "-load-save", ["--migrate-project", sample, migrated])
            run(prefix + "-saved-validate", ["--validate-project", migrated])
            original_model = json.loads(sample.read_text(encoding="utf-8-sig"))
            saved_model = json.loads(migrated.read_text(encoding="utf-8-sig"))
            row["suppliedFieldsPreserved"] = probe.supplied_fields_preserved(original_model, saved_model)
            changes = supplied_differences(original_model, saved_model)
            row["suppliedFieldChanges"] = changes
            row["onlyKnownEmptyContainerSchemaNormalization"] = canonical_empty_normalization(changes)
            write_json(output / (prefix + "-save-differences.json"), changes)
            saved_again = output / (prefix + "-saved-again.ksp")
            run(prefix + "-second-save", ["--migrate-project", migrated, saved_again])
            row["secondSaveIdempotent"] = saved_model == json.loads(saved_again.read_text(encoding="utf-8-sig"))
            check(prefix + "-save-values-and-idempotence", (row["suppliedFieldsPreserved"] or
                  row["onlyKnownEmptyContainerSchemaNormalization"]) and row["secondSaveIdempotent"],
                  suppliedFieldsPreserved=row["suppliedFieldsPreserved"], exactSuppliedChanges=changes,
                  secondSaveIdempotent=row["secondSaveIdempotent"])
            capabilities = json_stdout(run(prefix + "-capabilities", ["--project-capabilities", sample]))
            row["projectCapabilities"] = capabilities
            simulation = json_stdout(run(prefix + "-simulation-capabilities", ["--simulation-capabilities", sample]))
            row["simulationCapabilities"] = simulation
            screenshot = output / (prefix + "-loaded.png")
            run(prefix + "-actual-mainwindow-load", ["--capture-preview", screenshot, sample])
            result["screenshots"].append(png_info(screenshot))
            exported = output / (prefix + "-export.sm")
            export_record = run(prefix + "-source-export", ["--export-native-project", sample, exported], allowed=[0, 20])
            row["sourceExport"] = {"exitCode": export_record.get("exitCode"), "status": "exported" if export_record.get("exitCode") == 0 else "refused", "outputExists": exported.exists()}
            check(prefix + "-source-publication-boundary", exported.is_file() if export_record.get("exitCode") == 0 else not exported.exists())
            compiled = temporary / (prefix + "-compiled")
            compile_record = run(prefix + "-device-compile", ["--compile-project", sample, compiled], allowed=[0, 19])
            row["deviceCompile"] = {"exitCode": compile_record.get("exitCode"), "status": "compiled" if compile_record.get("exitCode") == 0 else "refused", "stdout": compile_record["stdout"], "sdkAvailable": False}
            if compile_record.get("exitCode") == 0:
                verification_record = run(prefix + "-verify-compiled", ["--verify-compiled-package", compiled])
                row["deviceCompile"]["verification"] = json_stdout(verification_record)
                row["deviceCompile"]["publishedFiles"] = snapshot(compiled, checker)
                remove_owned(compiled, "本样例离线编译已独立verify，保留完整文件哈希及wire证据后清理可再生输出")
            else:
                row["deviceCompile"]["refusalLeftPublishedOutput"] = compiled.exists()
                check(prefix + "-compile-refusal-no-partial-publication", not compiled.exists())
            result["samples"].append(row)
            persist()
        events = output / "simulation-events.json"
        write_json(events, [{"op": "press", "id": "toggle"}, {"op": "release", "inside": True},
                            {"op": "press", "id": "set-word"}, {"op": "release", "inside": True},
                            {"op": "press", "id": "english"}, {"op": "release", "inside": True}])
        simulation_output = output / "simulation-result.json"
        run("independent-offline-simulation", ["--simulate-offline", package / "samples/stage7/internal-five-family.ksp", events, simulation_output])
        simulated = json.loads(simulation_output.read_text(encoding="utf-8-sig"))
        frame = output / "simulation-frame.png"
        frame.write_bytes(base64.b64decode(simulated["finalFramePngBase64"], validate=True))
        result["screenshots"].append(png_info(frame))
        state = simulated.get("stateBeforeStop", {})
        check("independent-offline-trace", simulated.get("ok") is True and simulated.get("hardwareIo") is False and
              simulated.get("hmiBehaviorConfirmed") is False and len(simulated.get("trace", [])) == 6 and
              state.get("LB", {}).get("LB0") is True and state.get("LW", {}).get("LW10") == 5 and
              state.get("language") == "English", observedState=state)
        absent_report = output / "missing-sdk-report.json"
        missing_sdk = json_stdout(run("missing-sdk", ["--compile-macros", package / "samples/stage8/ordinary-two-macros.ksp", absent_report], expected=26))
        check("missing-sdk-scoped-refusal", missing_sdk.get("ok") is False and missing_sdk.get("hardwareIo") is False
              and not absent_report.exists(), observed=missing_sdk)
        clone = temporary / "中文 空格路径" / "便携程序 r2"
        clone.parent.mkdir(parents=True, exist_ok=False)
        shutil.copytree(package, clone)
        check("unicode-copy-identical", snapshot(clone, checker) == before)
        clone_environment = probe.clean_environment(clone)
        for name in ("TEMP", "TMP", "TMPDIR", "KINSEAL_MACRO_TEMP_ROOT", "KINSEAL_ARM_SDK", "QT_FORCE_STDERR_LOGGING", "QT_ASSUME_STDERR_HAS_CONSOLE"):
            clone_environment[name] = environment[name]
        clone_exe = clone / "KinSealStudioLite.exe"
        run("unicode-space-release", ["--release-info"], exe=clone_exe, env=clone_environment)
        run("unicode-space-help", ["--help-catalog"], exe=clone_exe, env=clone_environment)
        run("unicode-space-load-save", ["--migrate-project", clone / EXPECTED_SAMPLES[0], output / "中文 工程保存.ksp"], exe=clone_exe, env=clone_environment)
        remove_owned(clone / "help", "仅自有副本：缺帮助负向用例")
        missing_help = json_stdout(run("missing-help", ["--help-catalog"], expected=28, exe=clone_exe, env=clone_environment))
        check("missing-help-clear-refusal", missing_help.get("valid") is False and bool(missing_help.get("failures")), observed=missing_help)
        shutil.copytree(package / "help", clone / "help")
        check("clone-restored-before-missing-qt", snapshot(clone, checker) == before)
        remove_owned(clone / "Qt6Widgets.dll", "仅自有副本：缺Qt6Widgets负向用例")
        run("missing-qt-runtime", ["--release-info"], expected=0xC0000135, exe=clone_exe, env=clone_environment, timeout=10)
        shutil.copy2(package / "Qt6Widgets.dll", clone / "Qt6Widgets.dll")
        check("clone-restored-before-missing-plugin", snapshot(clone, checker) == before)
        remove_owned(clone / "platforms/qwindows.dll", "仅自有副本：缺Windows平台插件负向用例")
        plugin_record = run("missing-platform-plugin", ["--release-info"], exe=clone_exe, env=clone_environment, allowed=[3, 134, 0xC0000409, 0x40000015], timeout=10)
        plugin_error = (output / plugin_record["stderr"]).read_text(encoding="utf-8-sig", errors="replace")
        diagnostic = "platform plugin" in plugin_error.lower() and "windows" in plugin_error.lower()
        guarded_block = plugin_record.get("timeout") is True and plugin_record.get("ownedProcessTerminatedAfterFailure") is True and diagnostic
        plugin_record["expectedMissingPluginDiagnostic"] = diagnostic
        plugin_record["guardedBlockedStartup"] = guarded_block
        if guarded_block:
            # Preserve the real timeout/error; recognize a separately defined bounded startup-failure observation.
            plugin_record["matchesAllowedOutcome"] = True
            plugin_record["outcomeClassification"] = "明确缺插件诊断后阻塞；10秒终止私有进程，未正常退出"
            result["notRun"].append("缺平台插件后的应用正常退出：Qt致命对话阻塞，已记录并限时终止；不能称正常拒绝返回")
        check("missing-platform-plugin-diagnostic-and-bounded-stop", diagnostic and
              (guarded_block or plugin_record["matchesAllowedOutcome"]), exitCode=plugin_record.get("exitCode"),
              guardedBlockedStartup=guarded_block, normalExitObserved=not guarded_block)
        remove_owned(clone, "全部负向场景完成，保留原日志与包前后快照后删除自有副本")
        clone.parent.rmdir()
    except Exception as error:
        result["error"] = repr(error)
        result["failureArtifactsPreserved"] = True
    finally:
        desktop.close()
        ctypes.windll.kernel32.SetErrorMode(previous_error_mode)
        after = snapshot(package, checker)
        write_json(output / "package-after.json", after)
        result["packageUnchanged"] = before == after
        result["manifestSha256After"] = sha(package / "manifest.json")
        result["unexpectedLoadedModulePaths"] = sorted({path for run_record in result["runs"] for path in run_record.get("modulePathsObserved", [])
            if re.search(r"[\\/]QT6[\\/]|[\\/]mingw1310_64[\\/]|KinSealStudio_Setup", path, re.IGNORECASE)})
        result["allProcessOutcomesAllowed"] = all(run_record["matchesAllowedOutcome"] for run_record in result["runs"])
        result["allSemanticChecksPassed"] = all(row["passed"] for row in result["checks"])
        result["passed"] = "error" not in result and result["packageUnchanged"] and not result["unexpectedLoadedModulePaths"] and result["allProcessOutcomesAllowed"] and result["allSemanticChecksPassed"]
        result["outcome"] = "checks_complete_with_explicit_limits" if result["passed"] else "checks_failed_preserved"
        result["guardedBlockedStartupCount"] = sum(row.get("guardedBlockedStartup") is True for row in result["runs"])
        result["normalExitObservedCount"] = sum("exitCode" in row and not row.get("timeout") for row in result["runs"])
        result["finishedAtUtc"] = datetime.now(timezone.utc).isoformat()
        result["temporaryFilesRemaining"] = [{"path": path.relative_to(temporary).as_posix(), "bytes": path.stat().st_size} for path in checker.tree_files(temporary)]
        persist()
    return result


def main() -> int:  #20261114-10 add
    """Require exact package identities and new Z-drive output roots before running acceptance."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--run", action="store_true")
    parser.add_argument("--package", type=Path)
    parser.add_argument("--expected-manifest-sha")
    parser.add_argument("--expected-exe-sha")
    parser.add_argument("--output", type=Path)
    parser.add_argument("--temp-root", type=Path)
    args = parser.parse_args()
    if not args.run:
        print("Prepared only; use --run with frozen hashes and fresh Z-drive output/temp roots.")
        return 0
    if not all((args.package, args.expected_manifest_sha, args.expected_exe_sha, args.output, args.temp_root)):
        parser.error("Execution requires package, both frozen hashes, output and temp-root.")
    if not re.fullmatch(r"[0-9a-fA-F]{64}", args.expected_manifest_sha) or not re.fullmatch(r"[0-9a-fA-F]{64}", args.expected_exe_sha):
        parser.error("Both frozen identities must be full SHA256 values.")
    result = run_acceptance(args)
    print(json.dumps({key: result.get(key) for key in ("passed", "error", "packageUnchanged", "allProcessOutcomesAllowed", "allSemanticChecksPassed")}, ensure_ascii=False))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
