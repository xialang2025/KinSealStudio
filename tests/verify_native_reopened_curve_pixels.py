"""Check original AMZ page bytes from authored, reopened and native-edited curve projects."""
# 20261128-39 add: The two factory exports are independent full-page byte oracles.
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import uuid


ROOT = Path(__file__).resolve().parents[1]
FIXTURES = ROOT / "tests/fixtures/native-source/curve-reopen-amz"
SOURCES = (("AMZ-LIVE-01.ksp", "01"), ("AMZ-LIVE-01.sm", "01"),
           ("native-saved.sm", "01"), ("AMZ-LIVE-02.sm", "02"))
MEMBERS = ("SheetPixel.dat", "PicLenPara.dat")


def digest(data):
    """Identify the complete source, executable, or compiled file."""
    return hashlib.sha256(data).hexdigest()


def run_compiler(executable, command, source, output, expected_exit=0):
    """Run one file action on its own Windows desktop, including any loader or plugin error dialogs."""
    args = [str(executable), command, str(source), str(output)]
    env = dict(os.environ, TEMP=str(output.parent), TMP=str(output.parent), TMPDIR=str(output.parent))
    if os.name != "nt":
        code = subprocess.run(args, env=dict(env, QT_QPA_PLATFORM="offscreen"), cwd=output.parent, timeout=90).returncode
        if code != expected_exit:
            raise RuntimeError(f"{source.name} {command} returned {code}, expected {expected_exit}")
        return
    import win32event
    import win32process
    import win32service

    name = "KsCurveRaster_" + uuid.uuid4().hex
    desktop = win32service.CreateDesktop(name, 0, 0x1ff, None)
    startup = win32process.STARTUPINFO()
    startup.lpDesktop = "WinSta0\\" + name
    process, thread, _, _ = win32process.CreateProcess(str(executable), subprocess.list2cmdline(args), None, None,
                                                      False, subprocess.CREATE_NO_WINDOW,
                                                      dict(env, QT_QPA_PLATFORM="windows"), str(output.parent), startup)
    try:
        if win32event.WaitForSingleObject(process, 90000) != 0:
            win32process.TerminateProcess(process, 1)
            win32event.WaitForSingleObject(process, 5000)
            raise TimeoutError(f"{source.name} {command}")
        code = win32process.GetExitCodeProcess(process)
        if code != expected_exit:
            raise RuntimeError(f"{source.name} {command} returned {code}, expected {expected_exit}")
    finally:
        thread.Close()
        process.Close()
        del desktop


def verify(executable, scratch):
    """Compile each retained input via both normal entry points and compare the two raw members."""
    results = []
    for filename, label in SOURCES:
        source = FIXTURES / filename
        for command in ("--compile-project", "--compile-native-media-parts"):
            with TemporaryDirectory(dir=scratch) as temp:
                directory = Path(temp)
                out = directory / "compiled"
                run_compiler(executable, command, source, out)
                hashes = {}
                for name in MEMBERS:
                    actual = (out / name).read_bytes()
                    expected = (FIXTURES / f"native-{label}-{name}").read_bytes()
                    if actual != expected:
                        raise AssertionError(f"{filename} {command} {name}: {digest(actual)} vs {digest(expected)}")
                    hashes[name] = digest(actual)
                results.append(dict(input=filename, inputSha256=digest(source.read_bytes()), command=command, members=hashes))
    return dict(exeSha256=digest(executable.read_bytes()), checks=results, wholeFileComparisons=len(results)*len(MEMBERS))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--scratch", type=Path, required=True)
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    receipt = verify(args.exe.resolve(), args.scratch.resolve())
    if args.receipt:
        args.receipt.write_text(json.dumps(receipt, ensure_ascii=False, indent=2) + "\n", encoding="utf8")
    print(receipt["wholeFileComparisons"], "complete curve page images and indexes match the factory bytes")
