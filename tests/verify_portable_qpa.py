"""Exercise a Windows portable installation on an isolated desktop with Qt SDK paths removed."""
# 20261128-41 add
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
import time
import uuid

import pywintypes
import win32api
import win32event
import win32gui
import win32process
import win32service


def launch(executable, desktop, name, work, arguments, platform=None, watch_window=False):
    """Launch one disposable process and read only its own windows on the private desktop."""
    environment = dict(os.environ, TEMP=str(work), TMP=str(work), TMPDIR=str(work))
    for key in ("QT_PLUGIN_PATH", "QT_QPA_PLATFORM_PLUGIN_PATH", "QT_QPA_PLATFORM", "QT_QPA_FONTDIR"):
        environment.pop(key, None)
    system = Path(os.environ["SystemRoot"])
    environment["PATH"] = str(system / "System32") + os.pathsep + str(system)
    if platform:
        environment["QT_QPA_PLATFORM"] = platform
    startup = win32process.STARTUPINFO()
    startup.lpDesktop = "WinSta0\\" + name
    process, thread, pid, _ = win32process.CreateProcess(
        str(executable), subprocess.list2cmdline([str(executable), *map(str, arguments)]),
        None, None, False, subprocess.CREATE_NO_WINDOW, environment, str(work), startup)
    thread.Close()
    dialogs = []
    main_seen = False
    expired = False
    try:
        deadline = time.monotonic() + 18
        while win32event.WaitForSingleObject(process, 55) != win32event.WAIT_OBJECT_0:
            def inspect_window(handle, unused):
                nonlocal main_seen
                if win32process.GetWindowThreadProcessId(handle)[1] != pid or not win32gui.IsWindowVisible(handle):
                    return True
                kind = win32gui.GetClassName(handle)
                if kind == "#32770":
                    messages = []
                    def child(item, unused):
                        if win32gui.GetClassName(item) == "Static":
                            messages.append(win32gui.GetWindowText(item))
                        return True
                    win32gui.EnumChildWindows(handle, child, None)
                    dialogs.append(" ".join(messages))
                elif kind.startswith("Qt") and win32gui.GetWindowText(handle).startswith("KinSealStudio"):
                    main_seen = True
                return True
            try:
                win32api.SetLastError(0)
                handles = desktop.EnumDesktopWindows()
            except pywintypes.error as error:
                # An empty Win32 desktop returns FALSE with ERROR_SUCCESS before Qt creates a window.
                if error.winerror != 0:
                    raise
                handles = []
            for handle in handles:
                inspect_window(handle, None)
            expired = time.monotonic() > deadline
            if dialogs or expired or (watch_window and main_seen):
                break
        if win32event.WaitForSingleObject(process, 0) != win32event.WAIT_OBJECT_0:
            # This probe owns the new process and desktop. Do not send desktop-wide close or input events.
            win32process.TerminateProcess(process, 247 if dialogs or expired else 0)
            win32event.WaitForSingleObject(process, 5000)
        return dict(exit=win32process.GetExitCodeProcess(process), dialogs=dialogs, editorWindow=main_seen, timedOut=expired)
    finally:
        if win32event.WaitForSingleObject(process, 0) != win32event.WAIT_OBJECT_0:
            win32process.TerminateProcess(process, 248)
            win32event.WaitForSingleObject(process, 5000)
        process.Close()


def main():
    """Verify the isolated editor window and offscreen CLI, or record the original missing-plugin failure."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--scratch", type=Path, required=True)
    parser.add_argument("--receipt", type=Path, required=True)
    parser.add_argument("--expect-missing-offscreen", action="store_true")
    args = parser.parse_args()
    name = "KinSealQpa_" + uuid.uuid4().hex
    desktop = win32service.CreateDesktop(name, 0, 0x1FF, None)
    with tempfile.TemporaryDirectory(dir=args.scratch.resolve()) as folder:
        binary = args.exe.resolve()
        result = {"offscreen": launch(binary, desktop, name, Path(folder), ["--self-test"], "offscreen")}
        if args.expect_missing_offscreen:
            r = result["offscreen"]
            assert r["exit"] != 0 and any(
                "no Qt platform plugin could be initialized" in message and "windows" in message
                for message in r["dialogs"]), r
        else:
            result["windows"] = launch(binary, desktop, name, Path(folder), ["--self-test"], "windows")
            result["editor"] = launch(binary, desktop, name, Path(folder), [], watch_window=True)
            assert result["editor"]["editorWindow"], result
            assert all(v["exit"] == 0 and not v["dialogs"] and not v["timedOut"] for v in result.values()), result
    args.receipt.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf8")
    print(json.dumps(result, ensure_ascii=True))


if __name__ == "__main__":
    main()
