"""Capture only an owned editor process on an isolated Windows desktop."""
#20261111-1 add: All screenshots use an owned private desktop; no foreground activation, system input or USB is issued.
import argparse
import importlib.util
import json
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    """Input: Editor EXE, new output directory and optional project. Return: Recorded private capture status."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--sample", type=Path)
    parser.add_argument("--test", type=Path, action="append", default=[])
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    spec = importlib.util.spec_from_file_location("shell_private_desktop", ROOT / "docs/stage1-evidence/gui-portable/probe.py")
    launcher = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(launcher)
    desktop = launcher.PrivateDesktop(output)
    executable = args.executable.resolve()
    qt = Path("G:/01_Professional/QT6/Installed/6.12.0/mingw_64")
    environment = launcher.clean_environment(executable.parent)
    if not (executable.parent / "Qt6Core.dll").exists():
        environment["PATH"] = str(qt / "bin") + os.pathsep + environment["PATH"]
        environment["QT_QPA_PLATFORM_PLUGIN_PATH"] = str(qt / "plugins/platforms")
    environment["QT_SCALE_FACTOR"] = "1"
    environment["KINSEAL_SHELL_CAPTURE_DIR"] = str(output)
    record = dict(executable=str(executable), sha256=launcher.sha(executable), usbAccessed=False,
                  desktopSwitched=False, systemInput=False, foregroundActivationCalled=False, runs=[])
    try:
        arguments = ["--capture-preview", str(output / "main-window.png")]
        if args.sample:
            arguments.append(str(args.sample.resolve()))
        record["runs"].append(desktop.run("shell-capture", executable, arguments, environment))
        for test in args.test:
            record["runs"].append(desktop.run(test.stem, test.resolve(), ["-o", str(output / (test.stem + ".txt")) + ",txt"], environment, timeout=180))
        record["passed"] = all(run["passed"] for run in record["runs"])
    finally:
        desktop.close()
        record["privateDesktopClosed"] = True
        launcher.write_json(output / "run.json", record)
    print(json.dumps(dict(passed=record.get("passed"), captures=str(output))))
    return 0 if record.get("passed") else 1


if __name__ == "__main__":
    raise SystemExit(main())
