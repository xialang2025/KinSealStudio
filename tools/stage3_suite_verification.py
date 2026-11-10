"""Run the actual built Qt suites offscreen and retain every executable identity and raw log."""
#20261110-8 add: All compiler and test temporary files remain on the workspace drive.
import argparse
import os
from pathlib import Path
import re
import subprocess
from stage1_model_verification import ROOT, now, save, sha


def main():
    """Input: New result directory. Return: Nonzero if an executed suite fails or lacks totals."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--collect-build-logs", action="store_true", help="Collect the completed Build.ps1 run instead of repeating passing suites.")
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    (output / "artifacts").mkdir()
    environment = os.environ.copy()
    environment.update(QT_QPA_PLATFORM="offscreen",
        TEMP=str(ROOT.parent / ".scratch/kinseal-stage3-20261110/temp"),
        TMP=str(ROOT.parent / ".scratch/kinseal-stage3-20261110/temp"),
        KINSEAL_ARM_SDK="G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed/script")
    environment["PATH"] = os.pathsep.join(("G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin",
        "G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin", environment["PATH"]))
    record = dict(startedAtUtc=now(), runs=[], usbAccessed=False, platform="offscreen", passed=True)
    for executable in sorted((ROOT / "build").glob("KinSeal*Tests.exe")):
        log = output / (executable.stem + ".txt")
        identity = sha(executable)
        if args.collect_build_logs:
            filename = {"KinSealStaticTextTests":"static-text-tests.txt", "KinSealDeviceTransferTests":"device-transfer-tests.txt"}.get(executable.stem, executable.stem+".txt")
            original = ROOT / "build" / filename
            if original.stat().st_mtime < executable.stat().st_mtime:
                raise RuntimeError("Test log predates the current executable: "+filename)
            log.write_bytes(original.read_bytes())
            exit_code = None
        else:
            process = subprocess.run([str(executable), "-o", str(log)+",txt"], cwd=output,
                env=environment, capture_output=True, timeout=240, creationflags=subprocess.CREATE_NO_WINDOW)
            (output / (executable.stem+"-stdout.txt")).write_bytes(process.stdout)
            (output / (executable.stem+"-stderr.txt")).write_bytes(process.stderr)
            exit_code = process.returncode
        text = log.read_text(encoding="utf-8-sig", errors="replace") if log.exists() else ""
        totals = re.search(r"Totals: (\d+) passed, (\d+) failed, (\d+) skipped", text)
        result = dict(executable=str(executable), sha256=identity, exitCode=exit_code,
            origin="existing build-directory Qt log; Build.ps1 or explicitly re-run suite" if args.collect_build_logs else "executed by this tool",
            totals=dict(zip(("passed", "failed", "skipped"),map(int,totals.groups()))) if totals else None)
        result["passed"] = exit_code in (0,None) and totals is not None and totals[2] == "0" and sha(executable) == identity
        record["runs"].append(result)
        record["passed"] &= result["passed"]
        save(output / "run.json", record)
        print(executable.stem, result["totals"], flush=True)
    record["completedAtUtc"] = now()
    record["suiteCount"] = len(record["runs"])
    record["totals"] = {key:sum((run["totals"] or {}).get(key,0) for run in record["runs"])
        for key in ("passed","failed","skipped")}
    record["passed"] &= record["suiteCount"] == 24
    save(output / "run.json", record)
    print({key:record[key] for key in ("passed","suiteCount","totals")})
    return 0 if record["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
