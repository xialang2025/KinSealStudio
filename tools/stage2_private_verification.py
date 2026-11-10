"""Run stage-2 Qt interactions and a frozen portable capture on an owned private desktop."""
#20261110-7 add: Reuse the prior owned-process launcher; never switch desktops or send system input.
import argparse
import importlib.util
import json
from pathlib import Path
import re
import shutil
from stage1_model_verification import ROOT, now, save, sha


def main():
    """Input: Explicit package EXE and new output directory. Return: Actual private-test status."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    executable, output = args.executable.resolve(), args.output.resolve()
    if output.exists():
        raise RuntimeError("Each run requires a new output directory.")
    if sha(executable) != sha(ROOT / "build/KinSealStudioLite.exe"):
        raise RuntimeError("Portable program differs from the frozen build.")
    output.mkdir(parents=True)
    recipe = ROOT / "docs/stage1-evidence/gui-portable/probe.py"
    spec = importlib.util.spec_from_file_location("private_stage1_launcher", recipe)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    desktop = module.PrivateDesktop(output)
    record = dict(startedAtUtc=now(), executable=str(executable), executableSha256=sha(executable),
                  desktopSwitched=False, systemInput=False, runs=[], passed=False, usbAccessed=False)
    try:
        environment = module.clean_environment(executable.parent)
        record["runs"].append(desktop.run("portable-preview", executable,
            ["--capture-preview", str(output / "portable-main-window.png")], environment))
        harness = output / "harness"
        harness.mkdir()
        qt_test = Path("G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin/Qt6Test.dll")
        shutil.copy2(qt_test, harness / qt_test.name)
        (output / "artifacts").mkdir()
        for name in ("KinSealStage2DialogsTests", "KinSealStage2GlobalToolsTests", "KinSealStage2WorkflowTests", "KinSealStaticTextTests"):
            test = ROOT / "build" / (name + ".exe")
            destination = harness / test.name
            shutil.copy2(test, destination)
            log = output / (name + ".txt")
            result = desktop.run(name, destination, ["-o", str(log) + ",txt"], environment, timeout=180)
            result["sha256"] = sha(test)
            text = log.read_text(encoding="utf-8-sig", errors="replace") if log.exists() else ""
            totals = re.search(r"Totals: (\d+) passed, (\d+) failed, (\d+) skipped", text)
            result["totals"] = dict(zip(("passed", "failed", "skipped"), map(int, totals.groups()))) if totals else None
            result["passed"] = result["passed"] and totals is not None and totals[2] == "0" and totals[3] == "0"
            record["runs"].append(result)
            print(name, result["totals"], flush=True)
        record["passed"] = all(result["passed"] for result in record["runs"])
    except Exception as error:
        record["error"] = repr(error)
    finally:
        desktop.close()
        record["privateDesktopHandleClosed"] = True
        record["executableUnchanged"] = sha(executable) == record["executableSha256"]
        record["passed"] = record["passed"] and record["executableUnchanged"]
        record["completedAtUtc"] = now()
        save(output / "run.json", record)
    print(json.dumps({key: record.get(key) for key in ("passed", "error", "executableUnchanged")}))
    return 0 if record["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
