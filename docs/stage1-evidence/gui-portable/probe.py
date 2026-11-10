"""#20261110-5: Run only an explicitly frozen portable build on a private desktop.

Preparation is inert. No build, desktop switch, foreground activation, system input,
USB or PLC action is issued. The optional QtTest harness is separate from the app.
"""
from __future__ import annotations

import argparse
import ctypes
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import time
from datetime import datetime, timezone

import pywintypes
import win32con
import win32file
import win32process

PROJECT = Path(__file__).resolve().parents[3]


def sha(path: Path) -> str:
    """Input: file path. Return: exact file SHA256 without normalization."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path: Path, value: dict) -> None:
    """Input: output path/value. Return: None after a complete atomic JSON replacement."""
    temporary = path.with_name(path.name + ".new")
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    os.replace(temporary, path)


def clean_environment(package: Path | None = None) -> dict:
    """Input: optional packaged dependency directory. Return: no installed Qt/compiler PATH."""
    environment = os.environ.copy()
    windows = Path(environment.get("SystemRoot", r"C:\Windows"))
    parts = [str(windows / "System32"), str(windows)]
    if package is not None:
        parts.insert(0, str(package))
    environment["PATH"] = os.pathsep.join(parts)
    for name in ("QT_PLUGIN_PATH", "QT_QPA_PLATFORM_PLUGIN_PATH", "QML2_IMPORT_PATH", "QML_IMPORT_PATH", "QT_QPA_FONTDIR"):
        environment.pop(name, None)
    environment["QT_QPA_PLATFORM"] = "windows"
    if package is not None:
        environment["QT_QPA_PLATFORM_PLUGIN_PATH"] = str(package / "platforms")
    return environment


def supplied_fields_preserved(original, restored) -> bool:
    """Require every supplied source value and ordered list item to survive canonical default expansion."""
    if isinstance(original, dict):
        return isinstance(restored, dict) and all(key in restored and supplied_fields_preserved(value, restored[key])
                                                  for key, value in original.items())
    if isinstance(original, list):
        return isinstance(restored, list) and len(original) == len(restored) and all(
            supplied_fields_preserved(left, right) for left, right in zip(original, restored))
    if type(original) is not type(restored) and not (
            type(original) in (int, float) and type(restored) in (int, float)):
        return False
    return original == restored


class PrivateDesktop:
    """Own process/desktop handles; never attach to or control a user-owned window."""

    def __init__(self, output: Path):
        self.output = output
        self.user32 = ctypes.WinDLL("user32", use_last_error=True)
        self.user32.CreateDesktopW.argtypes = [ctypes.c_wchar_p, ctypes.c_wchar_p, ctypes.c_void_p,
                                              ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p]
        self.user32.CreateDesktopW.restype = ctypes.c_void_p
        self.user32.CloseDesktop.argtypes = [ctypes.c_void_p]
        self.name = "KinSealStage1Portable_" + str(os.getpid()) + "_" + str(int(time.time()))
        self.handle = self.user32.CreateDesktopW(self.name, None, None, 0, 0x000F01FF, None)
        if not self.handle:
            raise ctypes.WinError(ctypes.get_last_error())

    def close(self):
        """Input: None. Return: None after closing only this run's desktop handle."""
        if self.handle:
            self.user32.CloseDesktop(self.handle)
            self.handle = None

    def run(self, name: str, executable: Path, arguments: list[str], environment: dict,
            expected_exit: int = 0, timeout: float = 45) -> dict:
        """Launch an owned process on the private desktop and retain bounded stdout/stderr."""
        startup = win32process.STARTUPINFO()
        startup.lpDesktop = "WinSta0\\" + self.name
        startup.dwFlags = win32con.STARTF_USESHOWWINDOW | win32con.STARTF_USESTDHANDLES
        startup.wShowWindow = win32con.SW_SHOWNORMAL
        attributes = pywintypes.SECURITY_ATTRIBUTES()
        attributes.bInheritHandle = True
        stdout_path = self.output / (name + "-stdout.txt")
        stderr_path = self.output / (name + "-stderr.txt")
        process = thread = stdout = stderr = stdin = None
        record = {"name": name, "executable": str(executable), "arguments": arguments,
                  "expectedExit": expected_exit, "desktop": self.name,
                  "systemInput": False, "desktopSwitched": False, "foregroundActivationCalled": False,
                  "stdout": stdout_path.name, "stderr": stderr_path.name, "modulePathsObserved": []}
        started = time.monotonic()
        try:
            stdout = win32file.CreateFile(str(stdout_path), win32con.GENERIC_WRITE,
                win32con.FILE_SHARE_READ, attributes, win32con.CREATE_ALWAYS, 0, None)
            stderr = win32file.CreateFile(str(stderr_path), win32con.GENERIC_WRITE,
                win32con.FILE_SHARE_READ, attributes, win32con.CREATE_ALWAYS, 0, None)
            stdin = win32file.CreateFile("NUL", win32con.GENERIC_READ,
                win32con.FILE_SHARE_READ | win32con.FILE_SHARE_WRITE, attributes, win32con.OPEN_EXISTING, 0, None)
            startup.hStdOutput = stdout; startup.hStdError = stderr; startup.hStdInput = stdin
            command = subprocess.list2cmdline([str(executable)] + [str(item) for item in arguments])
            process, thread, pid, _ = win32process.CreateProcess(str(executable), command, None, None, True,
                win32process.CREATE_UNICODE_ENVIRONMENT | win32con.CREATE_NO_WINDOW,
                environment, str(executable.parent), startup)
            record["pid"] = pid
            while True:
                code = win32process.GetExitCodeProcess(process)
                if code != 259:
                    break
                try:
                    modules = win32process.EnumProcessModules(process)
                    paths = [win32process.GetModuleFileNameEx(process, module) for module in modules]
                    record["modulePathsObserved"] = sorted(set(record["modulePathsObserved"]) | set(paths))
                except pywintypes.error as exc:
                    record.setdefault("moduleEnumerationLimit", str(exc))
                if time.monotonic() - started > timeout:
                    record["timeout"] = True
                    raise TimeoutError(name + " exceeded bounded wait")
                time.sleep(0.05)
            record["exitCode"] = code
            record["passed"] = code == expected_exit
        except Exception as exc:
            record["error"] = repr(exc)
            record["passed"] = False
        finally:
            if process is not None:
                if win32process.GetExitCodeProcess(process) == 259:
                    win32process.TerminateProcess(process, 125)
                    record["ownedProcessTerminatedAfterFailure"] = True
                process.Close()
            for handle in (thread, stdout, stderr, stdin):
                if handle is not None:
                    handle.Close()
            record["elapsedSeconds"] = round(time.monotonic() - started, 3)
        return record


def main() -> int:
    """Require explicit execution and frozen identities before launching anything."""
    parser = argparse.ArgumentParser()
    parser.add_argument("--run", action="store_true", help="Only use after the parent confirms a frozen build.")
    parser.add_argument("--exe", type=Path)
    parser.add_argument("--expected-sha")
    parser.add_argument("--build-exe", type=Path)
    parser.add_argument("--backend-tests", type=Path)
    parser.add_argument("--static-tests", type=Path)
    parser.add_argument("--qt-test-dll", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    if not args.run:
        print("Prepared only: requires --run --exe --expected-sha and a new --output directory.")
        return 0
    if not args.exe or not args.expected_sha or not args.output:
        parser.error("Execution requires --exe --expected-sha --output.")
    executable = args.exe.resolve()
    package = executable.parent
    if sha(executable).lower() != args.expected_sha.lower():
        raise ValueError("Frozen portable executable identity differs; no process started.")
    if args.build_exe and sha(args.build_exe) != sha(executable):
        raise ValueError("Frozen build and portable executable differ; no process started.")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    single_source = PROJECT / "samples/stage1/stage1-single-page.ksp"
    multi_source = PROJECT / "samples/stage1/stage1-two-pages.ksp"
    reordered_source = PROJECT / "samples/stage1/stage1-reordered-languages.ksp"
    configuration_source = PROJECT / "samples/stage1/stage1-inactive-config.ksp"
    legacy_source = PROJECT / "samples/stage0/stage0-baseline.ksp"
    for source in (single_source, multi_source, reordered_source, configuration_source, legacy_source):
        packaged_sample = package / source.relative_to(PROJECT)
        if not packaged_sample.is_file() or sha(packaged_sample) != sha(source):
            raise ValueError("A packaged stage1 sample is missing or differs: " + str(packaged_sample))
    single = output / "single.ksp"; multi = output / "two.ksp"
    reordered = output / "reordered.ksp"; configuration = output / "configuration.ksp"
    legacy = output / "legacy-stage0.ksp"
    shutil.copy2(single_source, single); shutil.copy2(multi_source, multi)
    shutil.copy2(reordered_source, reordered); shutil.copy2(configuration_source, configuration)
    shutil.copy2(legacy_source, legacy)
    environment = clean_environment()
    result = {"date": "2026-11-10", "startedAtUtc": datetime.now(timezone.utc).isoformat(),
              "frozenExecutable": str(executable), "sha256": sha(executable), "path": environment["PATH"],
              "pathContainsInstalledQtOrCompiler": False, "usbOrPlcTransfers": False,
              "readOnlyDeviceEnumerationMayOccurInMainWindow": True,
              "privateDesktopOnly": True, "runs": [], "semanticChecks": [],
              "samples": [{"source": str(path), "sha256": sha(path)} for path in
                          (single_source, multi_source, reordered_source, configuration_source, legacy_source)],
              "scope": "Main EXE portable checks plus a separately identified GUI regression harness; no physical-device acceptance."}
    desktop = PrivateDesktop(output)

    def run(name, arguments, expected=0, exe=executable, env=environment):
        record = desktop.run(name, exe, arguments, env, expected)
        result["runs"].append(record)
        write_json(output / "result.json", result)
        return record

    try:
        run("self-test", ["--self-test"])
        run("fixed-payload", ["--verify-independent-payload"])
        run("single-validate", ["--validate-project", str(single)])
        run("two-validate", ["--validate-project", str(multi)])
        run("reordered-validate", ["--validate-project", str(reordered)])
        run("configuration-validate", ["--validate-project", str(configuration)])
        run("legacy-stage0-validate", ["--validate-project", str(legacy)])
        canonical_legacy = output / "legacy-canonical.ksp"
        record = run("legacy-stage0-migrate", ["--migrate-project", str(legacy), str(canonical_legacy)])
        if record["passed"]:
            migrated_legacy = json.loads(canonical_legacy.read_text(encoding="utf-8-sig"))
            result["semanticChecks"].append({"name": "actual-stage0-empty-macro-table-migration",
                "passed": migrated_legacy.get("macros") == {} and len(migrated_legacy["pages"][0]["objects"]) == 12})
        for name, sample in (("single", single), ("two", multi), ("reordered", reordered), ("configuration", configuration)):
            record = run(name + "-capabilities", ["--project-capabilities", str(sample)])
            if record["passed"]:
                report = json.loads((output / record["stdout"]).read_text(encoding="utf-8-sig"))
                expected_status = "blocked" if name in ("two", "configuration") else "candidate"
                result["semanticChecks"].append({"name": name + "-model-and-backend-gates",
                    "passed": report["modelPersistence"]["allowed"] is True and report["deviceCompile"]["status"] == expected_status,
                    "observed": report})
        migrated = output / "migrated-two.ksp"
        record = run("two-migrate", ["--migrate-project", str(multi), str(migrated)])
        if record["passed"]:
            original_json = json.loads(multi.read_text(encoding="utf-8-sig"))
            migrated_json = json.loads(migrated.read_text(encoding="utf-8-sig"))
            second_migration = output / "migrated-two-second.ksp"
            again = run("two-second-migrate", ["--migrate-project", str(migrated), str(second_migration)])
            idempotent = again["passed"] and json.loads(second_migration.read_text(encoding="utf-8-sig")) == migrated_json
            result["semanticChecks"].append({"name": "two-page-preservation",
                "passed": supplied_fields_preserved(original_json, migrated_json) and idempotent and
                    all(migrated_json.get(key) == original_json.get(key) for key in
                        ("projectId", "pageOrder", "activePageId", "startupPageId", "startupLanguage", "references")),
                "suppliedFieldsPreserved": supplied_fields_preserved(original_json, migrated_json),
                "secondMigrationIdempotent": idempotent,
                "normalizationEvidence": "run-r10-windows-1/two-page-normalization.json records the 24 added default paths; no source value was removed or changed."})
        wrong_suffix = output / "protected.sm"; wrong_suffix.write_bytes(b"previous source sentinel")
        before = sha(wrong_suffix)
        run("migration-refuses-sm", ["--migrate-project", str(single), str(wrong_suffix)], 21)
        result["semanticChecks"].append({"name": "bad-migration-retains-existing-file", "passed": sha(wrong_suffix) == before})
        run("single-compile", ["--compile-project", str(single), str(output / "compiled-single")])
        run("reordered-compile", ["--compile-project", str(reordered), str(output / "compiled-reordered")])
        for name, sample in (("single", single), ("reordered", reordered)):
            system = output / ("compiled-" + name) / "SysPara.dat"
            if system.is_file():
                project_json = json.loads(sample.read_text(encoding="utf-8-sig"))
                expected_language = project_json["languages"].index(project_json["startupLanguage"])
                result["semanticChecks"].append({"name": name + "-explicit-startup-language-index",
                    "passed": int.from_bytes(system.read_bytes()[2172:2174], "little") == expected_language,
                    "expected": expected_language})
        run("two-compile-reject", ["--compile-project", str(multi), str(output / "must-not-compile-two")], 19)
        run("configuration-compile-reject", ["--compile-project", str(configuration), str(output / "must-not-compile-configuration")], 19)
        run("two-legacy-compile-reject", ["--compile-single-text", str(multi), str(output / "must-not-compile-legacy-two")], 19)
        native = output / "single.sm"
        record = run("single-source-export", ["--export-native-project", str(single), str(native)])
        if record["passed"]:
            run("single-source-validate", ["--validate-project", str(native)])
            run("single-source-compile", ["--compile-project", str(native), str(output / "compiled-source")])
            comparisons = []
            for filename in ("ControlPara.dat", "ControlPixel.dat", "PicLenPara.dat", "AddrPara.dat"):
                original = output / "compiled-single" / filename
                restored = output / "compiled-source" / filename
                comparisons.append({"name": filename, "passed": original.is_file() and restored.is_file() and sha(original) == sha(restored)})
            result["semanticChecks"].append({"name": "portable-source-device-data-roundtrip",
                "passed": all(item["passed"] for item in comparisons), "files": comparisons})
        run("two-source-reject", ["--export-native-project", str(multi), str(output / "must-not-export-two.sm")], 20)
        run("configuration-source-reject", ["--export-native-project", str(configuration), str(output / "must-not-export-configuration.sm")], 20)
        run("two-legacy-source-reject", ["--export-native-test", str(multi), str(output / "must-not-export-legacy-two.sm")], 11)
        screenshot = output / "portable-main-window.png"
        record = run("portable-gui-capture", ["--capture-preview", str(screenshot)])
        result["semanticChecks"].append({"name": "portable-gui-png-produced",
            "passed": record["passed"] and screenshot.is_file() and screenshot.stat().st_size > 4096,
            "visualInspectionStillRequired": True, "path": screenshot.name})
        result["semanticChecks"].append({"name": "unsupported-output-directories-not-created",
            "passed": all(not (output / name).exists() for name in
                ("must-not-compile-two", "must-not-compile-configuration", "must-not-compile-legacy-two"))})
        if args.backend_tests or args.static_tests:
            if not args.qt_test_dll or not args.qt_test_dll.is_file():
                raise ValueError("The optional test harness needs an explicitly identified Qt6Test.dll.")
            harness = output / "test-harness"; harness.mkdir()
            (output / "artifacts").mkdir() # The existing tests save into executableDir/../artifacts.
            shutil.copy2(args.qt_test_dll, harness / "Qt6Test.dll")
            result["testHarness"] = {"executables": [],
                "qtTestDependencySource": str(args.qt_test_dll), "qtTestSha256": sha(args.qt_test_dll),
                "partOfPortableProduct": False,
                "note": "Only QtTest is a separate test dependency; Core/Widgets/platforms must resolve from frozen package."}
            harness_environment = clean_environment(package)
            if args.backend_tests:
                test_exe = harness / args.backend_tests.name; shutil.copy2(args.backend_tests, test_exe)
                result["testHarness"]["executables"].append({"path": str(test_exe), "sha256": sha(test_exe)})
                run("private-gui-regressions", ["editorSwitchesPagesWithoutDiscardingHiddenObjects",
                    "resourceDraftFailurePreservesEditorState", "-o", str(output / "private-gui-regressions.txt") + ",txt"],
                    exe=test_exe, env=harness_environment)
            if args.static_tests:
                test_exe = harness / args.static_tests.name; shutil.copy2(args.static_tests, test_exe)
                result["testHarness"]["executables"].append({"path": str(test_exe), "sha256": sha(test_exe)})
                log = output / "static-text-windows.txt"
                record = desktop.run("static-text-full-windows", test_exe, ["-o", str(log) + ",txt"],
                    harness_environment, 0, timeout=240)
                result["runs"].append(record)
                text = log.read_text(encoding="utf-8-sig") if log.is_file() else ""
                totals = re.search(r"Totals: (\d+) passed, (\d+) failed, (\d+) skipped", text)
                result["semanticChecks"].append({"name": "full-static-suite-real-windows-no-skip",
                    "passed": bool(record["passed"] and totals and totals.group(2) == "0" and totals.group(3) == "0" and
                                   "PASS   : StaticTextTests::nativeFontAndFillEvidence()" in text),
                    "totals": totals.group(0) if totals else None,
                    "fontEvidenceMustNotBeSkipped": True})
        result["passed"] = all(item["passed"] for item in result["runs"] + result["semanticChecks"])
    except Exception as exc:
        result["error"] = repr(exc); result["passed"] = False
    finally:
        desktop.close()
        result["privateDesktopHandleClosed"] = True
        result["endedAtUtc"] = datetime.now(timezone.utc).isoformat()
        result["executableSha256After"] = sha(executable)
        result["frozenExecutableUnchanged"] = result["executableSha256After"] == result["sha256"]
        if not result["frozenExecutableUnchanged"]:
            result["passed"] = False
        observed_qt = sorted({path for item in result["runs"] if item["executable"] == str(executable)
                              for path in item["modulePathsObserved"]
                              if Path(path).name.lower().startswith("qt6") or Path(path).name.lower() == "qwindows.dll"})
        unexpected_qt = [path for path in observed_qt if not Path(path).resolve().is_relative_to(package)]
        result["observedMainQtModules"] = observed_qt
        result["unexpectedObservedMainQtModules"] = unexpected_qt
        if unexpected_qt:
            result["passed"] = False
        result["dependencyModuleTraceLimit"] = "Modules are sampled only while each owned process remains alive; a short process can exit before complete enumeration."
        write_json(output / "result.json", result)
    print(json.dumps({"result": str(output / "result.json"), "passed": result["passed"],
                      "runs": len(result["runs"]), "semanticChecks": len(result["semanticChecks"])}, ensure_ascii=False, indent=2))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
