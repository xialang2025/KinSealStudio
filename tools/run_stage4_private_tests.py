"""Run explicitly selected fresh Qt test binaries on an owned Windows desktop."""
#20261112-0 add: Reuse the retained private-desktop launcher without touching user windows or USB.
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
from datetime import datetime, timezone


def main():
    """Input: Build/output paths and target names. Return: Actual test exit and skip counts."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("targets", nargs="+")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    recipe = root / "docs/stage1-evidence/gui-portable/probe.py"
    spec = importlib.util.spec_from_file_location("stage4_private_launcher", recipe)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    desktop = module.PrivateDesktop(output)
    environment = module.clean_environment()
    environment["PATH"] = ("G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;"
                           "G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;" + environment["PATH"])
    record = dict(startedAtUtc=datetime.now(timezone.utc).isoformat(), runs=[], usbAccessed=False,
                  desktopSwitched=False, systemInput=False, passed=False)
    try:
        for name in args.targets:
            if not re.fullmatch(r"KinSeal[A-Za-z0-9]+Tests", name):
                raise ValueError("Unexpected test target name")
            executable = (args.build_dir / (name + ".exe")).resolve()
            log = output / (name + ".txt")
            result = desktop.run(name, executable, ["-o", str(log) + ",txt"], environment, timeout=180)
            result["sha256"] = hashlib.sha256(executable.read_bytes()).hexdigest()
            text = log.read_text(encoding="utf-8-sig", errors="replace") if log.exists() else ""
            totals = re.search(r"Totals: (\d+) passed, (\d+) failed, (\d+) skipped", text)
            result["totals"] = dict(zip(("passed", "failed", "skipped"), map(int, totals.groups()))) if totals else None
            result["passed"] = result["passed"] and totals is not None and totals[2] == "0" and totals[3] == "0"
            record["runs"].append(result)
            print(name, result["totals"], flush=True)
        record["passed"] = all(run["passed"] for run in record["runs"])
    finally:
        desktop.close()
        record["privateDesktopHandleClosed"] = True
        record["completedAtUtc"] = datetime.now(timezone.utc).isoformat()
        (output / "run.json").write_text(json.dumps(record, ensure_ascii=False, indent=2), encoding="utf-8")
        # Empty stdout/stderr are disposable launcher output; keep nonempty diagnostics and all actual Qt logs.
        for path in output.glob("*-std*.txt"):
            if path.stat().st_size == 0:
                path.unlink()
    return 0 if record["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
