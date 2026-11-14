"""Measure explicit frozen PC CLI workloads on an owned Windows desktop."""
#20261114-9 add: Whole-process measurements never claim firmware performance or long-term stability.
from __future__ import annotations

import argparse
import copy
import ctypes
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import platform
import shutil
import statistics
import subprocess
import time
from datetime import datetime, timezone

import pywintypes
import win32con
import win32file
import win32process


def digest(path: Path) -> str:
    """Input: existing file. Return: SHA256 of its exact bytes."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path: Path, value) -> None:
    """Input: owned destination and JSON value. Return: None after atomic replacement."""
    temporary = path.with_name(path.name + ".new")
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    os.replace(temporary, path)


def inventory(directory: Path, hashes: bool = False) -> list[dict]:
    """Input: exclusively owned directory. Return: logical file inventory without following links."""
    rows = []
    for current, directories, files in os.walk(directory, followlinks=False):
        directories[:] = [name for name in directories if not (Path(current) / name).is_symlink()]
        for name in files:
            path = Path(current) / name
            if path.is_symlink():
                raise ValueError("Unexpected link in owned measurement tree")
            try:
                row = {"path": path.relative_to(directory).as_posix(), "bytes": path.stat().st_size}
                if hashes:
                    row["sha256"] = digest(path)
                rows.append(row)
            except FileNotFoundError:
                # Qt atomic-output leaves may disappear between enumeration and stat.
                continue
    return sorted(rows, key=lambda row: row["path"])


def total_bytes(directory: Path) -> int:
    """Input: owned directory. Return: logical byte count observed at this instant."""
    return sum(row["bytes"] for row in inventory(directory))


def cleanup_generated(directory: Path, allowed_root: Path) -> dict:
    """Input: fresh owned command directory. Return: exact cleanup receipt; stop on any refusal."""
    resolved, root = directory.resolve(strict=True), allowed_root.resolve(strict=True)
    if resolved == root or not resolved.is_relative_to(root):
        raise ValueError("Cleanup target escaped owned root")
    before = inventory(resolved, hashes=True)
    receipt = {"target": str(resolved), "filesBefore": before, "bytesRemoved": 0, "complete": False}
    try:
        for row in before:
            leaf = resolved / row["path"]
            if not leaf.resolve(strict=True).is_relative_to(resolved):
                raise ValueError("Cleanup leaf escaped verified root")
            leaf.unlink()
            receipt["bytesRemoved"] += row["bytes"]
        for current, directories, _ in os.walk(resolved, topdown=False, followlinks=False):
            for name in directories:
                (Path(current) / name).rmdir()
        resolved.rmdir()
        receipt["complete"] = True
    except Exception as exc:
        receipt["error"] = repr(exc)
    receipt["filesRemaining"] = inventory(resolved) if resolved.exists() else []
    return receipt


def make_cases(root: Path, output: Path) -> list[dict]:
    """Input: source/evidence roots. Return: seven real-schema single-axis input cases."""
    source = root / "samples/stage1/stage1-single-page.ksp"
    original = json.loads(source.read_text(encoding="utf-8-sig"))
    base = copy.deepcopy(original)
    text = copy.deepcopy(original["pages"][0]["objects"][0])
    if text["type"] != "text" or "staticText" not in text:
        raise ValueError("Released typed static-text fixture differs")
    text.update(id="performance-text-0", name="性能文字", text="AB12", x=10, y=10,
                width=70, height=36, layer=0)
    text["staticText"].update(texts={"中文": "AB12"}, fontSize=16, spacing=0)
    base["pages"][0]["objects"] = [text]
    base["languages"] = ["中文"]
    base["activeLanguage"] = base["startupLanguage"] = "中文"
    definitions = [("objects", count, 1) for count in (1, 10, 50, 100)]
    definitions += [("languages", 1, count) for count in (2, 4, 8)]
    languages = ["中文", "English", "Deutsch", "Français", "Español", "Italiano", "日本語", "한국어"]
    inputs, cases = output / "inputs", []
    inputs.mkdir()
    for axis, object_count, language_count in definitions:
        project = copy.deepcopy(base)
        project["languages"] = languages[:language_count]
        objects = []
        for index in range(object_count):
            item = copy.deepcopy(text)
            item.update(id="performance-text-" + str(index), x=10 + index % 10 * 78,
                        y=10 + index // 10 * 46)
            item["staticText"]["texts"] = {language: "AB12" for language in project["languages"]}
            objects.append(item)
        project["pages"][0]["objects"] = objects
        case_id = f"{axis}-o{object_count}-l{language_count}"
        path = inputs / (case_id + ".ksp")
        write_json(path, project)
        cases.append({"id": case_id, "axis": axis, "objects": object_count, "languages": language_count,
                      "source": str(path), "sourceSha256": digest(path), "sourceBytes": path.stat().st_size})
    write_json(inputs / "derivation.json", {
        "fixture": str(source), "fixtureSha256": digest(source), "schemaVersion": original["schemaVersion"],
        "selectedObject": original["pages"][0]["objects"][0]["id"],
        "changes": ["Retain released schema9 envelope and first typed text only.",
                    "Normalize common caption AB12, font16, spacing0, geometry70x36.",
                    "Clone unique IDs on fixed10x10 grid inside800x480 for objects axis.",
                    "Use one object and identical AB12 captions in each language for language axis."],
        "baseCaseShared": "objects-o1-l1", "mixedPressureExecuted": False, "noDeviceGateBypass": True,
        "modelLimitsSource": "HmiPage::fromJson:100 objects; HmiProject::validateModel:1..8 languages"})
    return cases


def environment_snapshot() -> dict:
    """Input: None. Return: machine/free-space snapshot taken before workloads."""
    class MemoryStatus(ctypes.Structure):
        _fields_ = [("length", ctypes.c_ulong), ("load", ctypes.c_ulong)] + [
            (name, ctypes.c_ulonglong) for name in ("totalPhys", "availPhys", "totalPage", "availPage",
                                                 "totalVirtual", "availVirtual", "availExtended")]
    memory = MemoryStatus()
    memory.length = ctypes.sizeof(memory)
    memory_ok = bool(ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(memory)))
    return {"capturedAtUtc": datetime.now(timezone.utc).isoformat(), "captureTiming": "before workloads",
            "processorIdentifier": os.environ.get("PROCESSOR_IDENTIFIER"), "memoryQuerySucceeded": memory_ok,
            "physicalMemoryTotalBytes": memory.totalPhys if memory_ok else None,
            "physicalMemoryAvailableBytes": memory.availPhys if memory_ok else None,
            "disk": {drive: {"totalBytes": shutil.disk_usage(drive).total, "freeBytes": shutil.disk_usage(drive).free}
                     for drive in ("C:/", "Z:/")}}


def measure(desktop, name: str, executable: Path, arguments: list[str], environment: dict,
            output: Path, command_root: Path) -> dict:
    """Input: owned process/settings. Return: complete lifecycle timing and resource observations."""
    startup = win32process.STARTUPINFO()
    startup.lpDesktop = "WinSta0\\" + desktop.name
    startup.dwFlags = win32con.STARTF_USESHOWWINDOW | win32con.STARTF_USESTDHANDLES
    startup.wShowWindow = win32con.SW_SHOWNORMAL
    attributes = pywintypes.SECURITY_ATTRIBUTES()
    attributes.bInheritHandle = True
    stdout_path, stderr_path = output / (name + "-stdout.txt"), output / (name + "-stderr.txt")
    handles, samples = [], []
    process = thread = None
    started, launched = time.perf_counter_ns(), None
    record = {"name": name, "arguments": arguments, "startedAtUtc": datetime.now(timezone.utc).isoformat(),
              "stdout": stdout_path.name, "stderr": stderr_path.name, "sampleIntervalRequestedSeconds": 0.020,
              "timeoutGuardSeconds": 240, "timeoutIsPerformanceThreshold": False,
              "desktopSwitched": False, "foregroundActivationCalled": False, "systemInput": False,
              "kernelPeakWorkingSetBytes": None, "kernelPeakPagefileBytes": None,
              "memorySampleErrors": [], "passed": False}
    try:
        for path in (stdout_path, stderr_path):
            handles.append(win32file.CreateFile(str(path), win32con.GENERIC_WRITE, win32con.FILE_SHARE_READ,
                           attributes, win32con.CREATE_ALWAYS, 0, None))
        handles.append(win32file.CreateFile("NUL", win32con.GENERIC_READ,
                       win32con.FILE_SHARE_READ | win32con.FILE_SHARE_WRITE,
                       attributes, win32con.OPEN_EXISTING, 0, None))
        startup.hStdOutput, startup.hStdError, startup.hStdInput = handles
        command = subprocess.list2cmdline([str(executable)] + arguments)
        process, thread, pid, _ = win32process.CreateProcess(str(executable), command, None, None, True,
            win32process.CREATE_UNICODE_ENVIRONMENT | win32con.CREATE_NO_WINDOW,
            environment, str(executable.parent), startup)
        launched = time.perf_counter_ns()
        record["pid"] = pid
        while True:
            observed, memory = time.perf_counter_ns(), {}
            code = win32process.GetExitCodeProcess(process)
            try:
                memory = win32process.GetProcessMemoryInfo(process)
                for source, target in (("PeakWorkingSetSize", "kernelPeakWorkingSetBytes"),
                                       ("PeakPagefileUsage", "kernelPeakPagefileBytes")):
                    record[target] = max(record[target] or 0, memory[source])
            except pywintypes.error as exc:
                if str(exc) not in record["memorySampleErrors"]:
                    record["memorySampleErrors"].append(str(exc))
            samples.append({"elapsedNanoseconds": observed - started,
                            "workingSetBytes": memory.get("WorkingSetSize"),
                            "pagefileUsageBytes": memory.get("PagefileUsage"),
                            "temporaryBytes": total_bytes(command_root / "temp"),
                            "commandTreeBytes": total_bytes(command_root)})
            if code != 259:
                record.update(exitCode=code, passed=code == 0)
                break
            if observed - started > 240_000_000_000:
                raise TimeoutError("Owned process exceeded hang guard")
            time.sleep(0.020)
    except Exception as exc:
        record["error"] = repr(exc)
    finally:
        if process is not None:
            if win32process.GetExitCodeProcess(process) == 259:
                win32process.TerminateProcess(process, 125)
                record["ownedProcessTerminatedAfterFailure"] = True
            process.Close()
        for handle in [thread] + handles:
            if handle is not None:
                handle.Close()
        ended = time.perf_counter_ns()
        record.update(elapsedNanoseconds=ended - started, elapsedSeconds=(ended - started) / 1e9,
                      createProcessReturnedAfterNanoseconds=launched - started if launched is not None else None,
                      sampleCount=len(samples), samples=samples)
        for field, source in (("sampledWorkingSetPeakBytes", "workingSetBytes"),
                              ("sampledTemporaryPeakBytes", "temporaryBytes"),
                              ("sampledCommandTreePeakBytes", "commandTreeBytes")):
            record[field] = max((sample[source] or 0 for sample in samples), default=0)
        record["temporaryFilesAfterExit"] = inventory(command_root / "temp")
        record["generatedFilesAfterExit"] = inventory(command_root, hashes=True)
        record["observedSamplingGapsNanoseconds"] = [right["elapsedNanoseconds"] - left["elapsedNanoseconds"]
                                                    for left, right in zip(samples, samples[1:])]
        for field, path in (("stdout", stdout_path), ("stderr", stderr_path)):
            record[field + "Sha256"], record[field + "Bytes"] = digest(path), path.stat().st_size
        if record["stdoutBytes"]:
            try:
                report = json.loads(stdout_path.read_text(encoding="utf-8-sig"))
                for source, target in (("ok", "cliReportedOk"), ("message", "cliReportedMessage"),
                                       ("hardwareIo", "cliReportedHardwareIo"),
                                       ("hmiBehaviorConfirmed", "cliReportedHmiBehaviorConfirmed")):
                    record[target] = report.get(source)
            except (ValueError, AttributeError):
                pass
    return record


def main() -> int:
    """Input: explicit frozen executable and fresh owned roots. Return: actual measurement status."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--expected-sha", required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--temporary-root", type=Path, required=True)
    args = parser.parse_args()
    executable = args.exe.resolve(strict=True)
    if digest(executable).lower() != args.expected_sha.lower():
        raise ValueError("Executable identity differs; no process started")
    output, temporary_root = args.output.resolve(), args.temporary_root.resolve()
    allowed_temp = Path("Z:/Project/vscode_prj_temp/REA/.stage10-run-20261114/temp").resolve()
    if temporary_root == allowed_temp or not temporary_root.is_relative_to(allowed_temp):
        raise ValueError("TEMP must be an exclusive child of the stage10 owned root outside .scratch")
    output.mkdir(parents=True, exist_ok=False)
    temporary_root.mkdir(parents=True, exist_ok=False)
    root = Path(__file__).resolve().parents[1]
    spec = importlib.util.spec_from_file_location("stage10_private_launcher", root / "docs/stage1-evidence/gui-portable/probe.py")
    launcher = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(launcher)
    environment = launcher.clean_environment(executable.parent)
    cases = make_cases(root, output)
    empty_events = output / "inputs/empty-events.json"
    write_json(empty_events, [])
    desktop, started = launcher.PrivateDesktop(output), time.perf_counter_ns()
    manifest = {"schemaVersion": 1, "stage": "S10-07", "run": output.name,
                "startedAtUtc": datetime.now(timezone.utc).isoformat(),
                "scope": "PC whole-process CLI slice; no firmware or long-term acceptance",
                "executable": str(executable), "executableSha256": digest(executable),
                "sourceBaseline": "daefcdb5", "scriptSha256": digest(Path(__file__)),
                "python": platform.python_version(), "pythonExecutable": os.sys.executable,
                "windows": platform.platform(), "machine": platform.machine(), "logicalCpuCount": os.cpu_count(),
                "environmentBefore": environment_snapshot(),
                "clock": {"name": "time.perf_counter_ns", **vars(time.get_clock_info("perf_counter"))},
                "repeatsPerCaseAndOperation": 3, "cases": cases, "runs": [], "summaries": [], "cleanup": [],
                "temporaryRoot": str(temporary_root), "privateDesktopOnly": True,
                "desktopSwitched": False, "systemInput": False, "hardwareIo": False,
                "hmiPerformanceMeasured": False, "longTermStabilityMeasured": False,
                "performanceThresholdDeclared": False, "priorRunEvidenceIncludedInCounts": False,
                "limitations": ["Whole-process timing includes Qt startup/shutdown and exit detection up to actual polling gap.",
                                "Three ordered repeats share the OS cache; system load is uncontrolled.",
                                "Kernel peak memory differs from sampled current memory; sampled disk can miss transient peaks.",
                                "The240s guard is only hang protection, not a product performance threshold.",
                                "Empty-event offline startup includes final frame and stop; no sustained interactive session.",
                                "Only single-axis object/language counts; no resource, mixed pressure, HMI latency, or long-term test."]}
    operations = ("validate", "save", "native-export", "compile", "offline-start")
    try:
        for case in cases:
            source = case["source"]
            for operation in operations:
                for repeat in range(1, 4):
                    name = f"{case['id']}-{operation}-r{repeat}"
                    command_root = temporary_root / name
                    command_root.mkdir()
                    (command_root / "temp").mkdir()
                    for key in ("TEMP", "TMP", "TMPDIR"):
                        environment[key] = str(command_root / "temp")
                    commands = {"validate": ["--validate-project", source],
                        "save": ["--migrate-project", source, str(command_root / "saved.ksp")],
                        "native-export": ["--export-native-project", source, str(command_root / "source.sm")],
                        "compile": ["--compile-project", source, str(command_root / "compiled")],
                        "offline-start": ["--simulate-offline", source, str(empty_events), str(command_root / "offline.json")]}
                    row = measure(desktop, name, executable, commands[operation], environment, output, command_root)
                    row.update(case=case["id"], operation=operation, repeat=repeat)
                    if operation == "save" and row["passed"]:
                        saved = json.loads((command_root / "saved.ksp").read_text(encoding="utf-8-sig"))
                        row["savedSuppliedFieldsPreserved"] = launcher.supplied_fields_preserved(
                            json.loads(Path(source).read_text(encoding="utf-8-sig")), saved)
                        row["savedObjectCount"], row["savedLanguageCount"] = len(saved["pages"][0]["objects"]), len(saved["languages"])
                        row["passed"] = row["passed"] and row["savedSuppliedFieldsPreserved"]
                    if operation == "offline-start" and row["passed"]:
                        offline = json.loads((command_root / "offline.json").read_text(encoding="utf-8-sig"))
                        row.update(offlineTraceCount=len(offline["trace"]), offlineOk=offline["ok"],
                                   offlineFramePngEncodedBytes=len(offline["finalFramePngBase64"]))
                        row["passed"] = row["passed"] and offline["ok"] and not offline["hardwareIo"]
                    manifest["runs"].append(row)
                    manifest["cleanup"].append(cleanup_generated(command_root, temporary_root))
                    write_json(output / "manifest.json", manifest)
                    print(f"{len(manifest['runs']):3}/105 {name}: exit={row.get('exitCode')} elapsed={row['elapsedSeconds']:.4f}s", flush=True)
        for case in cases:
            for operation in operations:
                rows = [row for row in manifest["runs"] if row["case"] == case["id"] and row["operation"] == operation]
                seconds = [row["elapsedSeconds"] for row in rows]
                manifest["summaries"].append({"case": case["id"], "operation": operation, "samples": len(rows),
                    "successfulProcesses": sum(row["passed"] for row in rows), "exitCodes": [row.get("exitCode") for row in rows],
                    "elapsedSecondsMin": min(seconds), "elapsedSecondsMedian": statistics.median(seconds), "elapsedSecondsMax": max(seconds),
                    "kernelPeakWorkingSetBytesMax": max(row["kernelPeakWorkingSetBytes"] or 0 for row in rows),
                    "sampledTemporaryPeakBytesMax": max(row["sampledTemporaryPeakBytes"] for row in rows),
                    "sampledCommandTreePeakBytesMax": max(row["sampledCommandTreePeakBytes"] for row in rows)})
    except Exception as exc:
        manifest["error"] = repr(exc)
    finally:
        desktop.close()
        manifest.update(privateDesktopHandleClosed=True, completedAtUtc=datetime.now(timezone.utc).isoformat(),
                        elapsedSeconds=(time.perf_counter_ns() - started) / 1e9, executableSha256After=digest(executable),
                        temporaryFilesRemaining=inventory(temporary_root), completedProcessCount=len(manifest["runs"]),
                        successfulProcessCount=sum(row["passed"] for row in manifest["runs"]),
                        cleanupBytesRemoved=sum(row["bytesRemoved"] for row in manifest["cleanup"]))
        manifest["frozenExecutableUnchanged"] = manifest["executableSha256After"] == manifest["executableSha256"]
        manifest["allPlannedProcessesExecuted"] = len(manifest["runs"]) == 105
        manifest["measurementCompleted"] = manifest["allPlannedProcessesExecuted"] and manifest["frozenExecutableUnchanged"] and "error" not in manifest
        write_json(output / "manifest.json", manifest)
    return 0 if manifest["measurementCompleted"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
