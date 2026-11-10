"""Run real Windows Qt tests on an isolated desktop without switching user input."""

import ctypes
import os
import time
from pathlib import Path

import win32con
import win32process


def main():  # 20261108-14 add
    """Run the built test binary; input: None; return: test process exit code."""
    root = Path(__file__).resolve().parent.parent
    executable = root / "build/KinSealStaticTextTests.exe"
    log = root / "build/static-text-windows-tests.txt"
    user32 = ctypes.WinDLL("user32", use_last_error=True)
    user32.CreateDesktopW.argtypes = [ctypes.c_wchar_p, ctypes.c_wchar_p, ctypes.c_void_p,
                                     ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p]
    user32.CreateDesktopW.restype = ctypes.c_void_p
    user32.CloseDesktop.argtypes = [ctypes.c_void_p]
    desktop_name = "KinSealStaticTests" + str(os.getpid())
    desktop = user32.CreateDesktopW(desktop_name, None, None, 0, 0x000F01FF, None)
    if not desktop:
        raise ctypes.WinError(ctypes.get_last_error())
    startup = win32process.STARTUPINFO()
    startup.lpDesktop = "WinSta0\\" + desktop_name
    startup.dwFlags = win32con.STARTF_USESHOWWINDOW
    startup.wShowWindow = win32con.SW_SHOWNORMAL
    environment = os.environ.copy()
    environment["QT_QPA_PLATFORM"] = "windows"
    environment["PATH"] = (r"G:\01_Professional\QT6\Installed\6.12.0\mingw_64\bin;"
                           r"G:\01_Professional\QT6\Installed\Tools\mingw1310_64\bin;"
                           + environment["PATH"])
    process = thread = None
    try:
        process, thread, _, _ = win32process.CreateProcess(
            str(executable), f'"{executable}" -o "{log},txt"', None, None, False,
            win32process.CREATE_UNICODE_ENVIRONMENT | win32con.CREATE_NO_WINDOW,
            environment, str(executable.parent), startup)
        deadline = time.monotonic() + 50
        while time.monotonic() < deadline:
            code = win32process.GetExitCodeProcess(process)
            if code != 259:
                print(log.read_text(encoding="utf-8"), flush=True)
                return code
            time.sleep(0.2)
        raise TimeoutError("Private Qt tests did not exit within 50 seconds")
    finally:
        if process is not None:
            if win32process.GetExitCodeProcess(process) == 259:
                win32process.TerminateProcess(process, 1)
            process.Close()
        if thread is not None:
            thread.Close()
        user32.CloseDesktop(desktop)


if __name__ == "__main__":
    raise SystemExit(main())
