"""Download one validated native test project through an isolated vendor runtime.

The vendor editor is launched on a private Windows desktop. This bridge is a
temporary compatibility backend for the verified one-text project format; it
does not claim to implement the vendor's USB protocol independently.
"""

import argparse
import ctypes
import json
import msvcrt
import os
import shutil
import sys
import tempfile
import time
from pathlib import Path

import win32con
import win32gui
import win32process
import win32ui
from PIL import Image, ImageChops, ImageStat


DEFAULT_RUNTIME = next(
    (parent / ".local" / "vendor-runtime"
     for parent in Path(__file__).resolve().parents
     if (parent / ".local" / "vendor-runtime" / "KinSealStudio5.1-private.exe").is_file()),
    Path(__file__).resolve().parent / ".local" / "vendor-runtime",
)  # 20261108-8 change: Resolve the user's private installed runtime from this project.
SUCCESS_TEMPLATE = Path(__file__).with_name("download_success_template.png")


def report(ok, message):  # 20261108-8 add
    """Emit a machine-readable result; input: ok, message; return: None."""
    payload = json.dumps({"ok": ok, "message": message}, ensure_ascii=False)
    sys.stdout.buffer.write(payload.encode("utf-8") + b"\n")
    sys.stdout.buffer.flush()


def top_windows(desktop, pid):  # 20261108-8 add
    """List visible windows on the private desktop; input: desktop, pid; return: handles."""
    rows = []

    def collect(hwnd, unused):
        """Collect only this process's windows; input: hwnd, unused; return: None."""
        del unused
        if win32process.GetWindowThreadProcessId(hwnd)[1] == pid:
            if win32gui.IsWindowVisible(hwnd):
                rows.append(hwnd)

    win32gui.EnumDesktopWindows(desktop, collect, None)
    return rows


def children(hwnd):  # 20261108-8 add
    """List child windows; input: hwnd; return: handles."""
    rows = []
    win32gui.EnumChildWindows(hwnd, lambda child, unused: rows.append(child), None)
    return rows


def click_screen(hwnd, x, y):  # 20261108-8 add
    """Post a click to an isolated window; input: hwnd, screen x/y; return: None."""
    client_x, client_y = win32gui.ScreenToClient(hwnd, (x, y))
    parameter = (client_y << 16) | (client_x & 0xffff)
    win32gui.PostMessage(hwnd, win32con.WM_MOUSEMOVE, 0, parameter)
    win32gui.PostMessage(hwnd, win32con.WM_LBUTTONDOWN, win32con.MK_LBUTTON, parameter)
    win32gui.PostMessage(hwnd, win32con.WM_LBUTTONUP, 0, parameter)


def click_center(hwnd):  # 20261108-8 add
    """Click the center of one isolated control; input: hwnd; return: None."""
    left, top, right, bottom = win32gui.GetWindowRect(hwnd)
    click_screen(hwnd, (left + right) // 2, (top + bottom) // 2)


def wait_for(predicate, timeout, description):  # 20261108-8 add
    """Poll private UI state; input: predicate, timeout, description; return: match."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        value = predicate()
        if value is not None:
            return value
        time.sleep(0.15)
    raise RuntimeError("等待原厂窗口超时：" + description)


def capture_window(user32, hwnd, path):  # 20261108-8 add
    """Capture a private window offscreen; input: user32, hwnd, path; return: PIL image."""
    left, top, right, bottom = win32gui.GetWindowRect(hwnd)
    window_dc = win32gui.GetWindowDC(hwnd)
    dc = win32ui.CreateDCFromHandle(window_dc)
    memory_dc = dc.CreateCompatibleDC()
    bitmap = win32ui.CreateBitmap()
    bitmap.CreateCompatibleBitmap(dc, right - left, bottom - top)
    previous_bitmap = memory_dc.SelectObject(bitmap)
    try:
        if user32.PrintWindow(hwnd, memory_dc.GetSafeHdc(), 2) != 1:
            raise RuntimeError("原厂下载窗口无法进行后台状态读取")
        bitmap.SaveBitmapFile(memory_dc, str(path))
    finally:
        memory_dc.SelectObject(previous_bitmap)
        win32gui.ReleaseDC(hwnd, window_dc)
        memory_dc.DeleteDC()
        win32gui.DeleteObject(bitmap.GetHandle())
    with Image.open(path) as image:
        return image.convert("RGB")


def classify_status(image, template):  # 20261108-8 add
    """Check the vendor completion/failure rendering; input: image, template; return: status."""
    if image.size != (620, 594):
        raise RuntimeError("原厂下载窗口尺寸变化，无法可靠判定下载状态")
    red_pixels = sum(1 for y in range(308, 530, 4)
                     for x in range(20, 600, 4)
                     if image.getpixel((x, y))[0] > 200
                     and image.getpixel((x, y))[1] < 90
                     and image.getpixel((x, y))[2] < 90)
    if red_pixels > 50:
        return "failed"
    crop = image.crop((20, 477, 95, 494))
    difference = ImageChops.difference(crop, template)
    mean_difference = max(ImageStat.Stat(difference).mean)
    progress_green = image.getpixel((590, 514))
    if mean_difference < 2.0 and progress_green[1] > 110 and progress_green[0] < 90:
        return "complete"
    return "running"


def run_download(source, runtime):  # 20261108-8 add
    """Compile and download one native source; input: source, runtime; return: None."""
    executable = runtime / "KinSealStudio5.1-private.exe"
    lock_path = runtime / "LiteDownload.lock"
    user32 = ctypes.WinDLL("user32", use_last_error=True)
    user32.CreateDesktopW.argtypes = [ctypes.c_wchar_p, ctypes.c_wchar_p,
                                      ctypes.c_void_p, ctypes.c_ulong,
                                      ctypes.c_ulong, ctypes.c_void_p]
    user32.CreateDesktopW.restype = ctypes.c_void_p
    user32.CloseDesktop.argtypes = [ctypes.c_void_p]
    user32.PrintWindow.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint]
    user32.PrintWindow.restype = ctypes.c_int
    desktop_name = "KinSealLiteUsb" + str(os.getpid())
    desktop = None
    process = None
    thread = None
    target = None
    with lock_path.open("a+b") as lock_file:
        lock_file.seek(0)
        try:
            msvcrt.locking(lock_file.fileno(), msvcrt.LK_NBLCK, 1)
        except OSError as error:
            raise RuntimeError("另一个下载任务正在使用隔离原厂运行时") from error
        try:
            if not executable.is_file():
                raise RuntimeError("隔离原厂运行时不存在：" + str(executable))
            if not source.is_file() or source.suffix.lower() != ".sm":
                raise RuntimeError("原生测试工程不存在或扩展名不是 .sm")
            if source.stat().st_size != 3887:
                raise RuntimeError("原生测试工程大小与已验证格式不一致")
            with Image.open(SUCCESS_TEMPLATE) as image:
                template = image.convert("RGB")
            target = runtime / "Project" / ("LiteDownload_%d.sm" % os.getpid())
            target.parent.mkdir(exist_ok=True)
            shutil.copy2(source, target)
            desktop = user32.CreateDesktopW(desktop_name, None, None, 0,
                                            0x000F01FF, None)
            if not desktop:
                raise OSError(ctypes.get_last_error(), "无法创建独立后台桌面")
            startup = win32process.STARTUPINFO()
            startup.lpDesktop = "WinSta0\\" + desktop_name
            startup.dwFlags = win32con.STARTF_USESHOWWINDOW
            startup.wShowWindow = win32con.SW_SHOWNORMAL
            environment = os.environ.copy()
            environment["__COMPAT_LAYER"] = "RunAsInvoker"
            process, thread, pid, _ = win32process.CreateProcess(
                str(executable), '"%s"' % executable, None, None, False,
                win32process.CREATE_UNICODE_ENVIRONMENT, environment,
                str(runtime), startup)

            def find_main():
                """Find the private editor; input: None; return: handle or None."""
                return next((hwnd for hwnd in top_windows(desktop, pid)
                             if win32gui.GetWindowText(hwnd) == "KinSealStudio"), None)

            main_window = wait_for(find_main, 20, "主窗口")
            win32gui.ShowWindow(main_window, win32con.SW_MAXIMIZE)
            time.sleep(0.4)
            toolbar = next((child for child in children(main_window)
                            if win32gui.GetWindowRect(child)[1:4:2] == (42, 76)), None)
            if toolbar is None:
                raise RuntimeError("无法定位原厂打开工程工具栏")
            click_screen(toolbar, 94, 66)
            dialog = wait_for(
                lambda: next((hwnd for hwnd in top_windows(desktop, pid)
                              if win32gui.GetClassName(hwnd) == "#32770"), None),
                10, "文件对话框")
            filename = win32gui.FindWindowEx(dialog, 0, "ComboBoxEx32", None)
            filename = win32gui.FindWindowEx(filename, 0, "ComboBox", None)
            filename = win32gui.FindWindowEx(filename, 0, "Edit", None)
            if not filename:
                raise RuntimeError("无法定位原厂文件输入框")
            win32gui.SendMessage(filename, win32con.WM_SETTEXT, 0, str(target))
            open_button = next((child for child in children(dialog)
                                if win32gui.GetClassName(child) == "Button"
                                and win32gui.GetWindowRect(child)[:2] == (470, 457)), None)
            if open_button is None:
                raise RuntimeError("无法定位原厂文件打开按钮")
            win32gui.SendMessage(open_button, win32con.BM_CLICK, 0, 0)
            time.sleep(2)
            if "LiteDownload_" not in win32gui.GetWindowText(main_window):
                raise RuntimeError("原厂程序未打开生成的测试工程")
            menu_bar = next((child for child in children(main_window)
                             if win32gui.GetWindowText(child) == "menuBar"), None)
            if menu_bar is None:
                raise RuntimeError("无法定位原厂工具菜单")
            click_screen(menu_bar, 326, 32)
            time.sleep(0.4)
            menu = next((hwnd for hwnd in top_windows(desktop, pid)
                         if win32gui.GetWindowRect(hwnd)[0] == 290), None)
            if menu is None:
                raise RuntimeError("无法定位原厂下载菜单")
            click_screen(menu, 340, 70)
            download = wait_for(
                lambda: next((hwnd for hwnd in top_windows(desktop, pid)
                              if hwnd != main_window
                              and win32gui.GetWindowRect(hwnd)[2]
                              - win32gui.GetWindowRect(hwnd)[0] > 300), None),
                10, "下载对话框")
            usb_radio = next((child for child in children(download)
                              if win32gui.GetWindowText(child) == "radioUSB"), None)
            download_button = next((child for child in children(download)
                                    if win32gui.GetWindowRect(child)[:2] == (588, 619)), None)
            if usb_radio is None or download_button is None:
                raise RuntimeError("无法定位原厂 USB 下载控件")
            click_screen(usb_radio, 410, 126)
            time.sleep(0.5)
            compiled = runtime / "compile" / "load.kns"
            click_screen(download_button, 683, 630)
            with tempfile.TemporaryDirectory(prefix="KinSealBridge-") as temp_dir:
                capture_path = Path(temp_dir) / "status.bmp"
                deadline = time.monotonic() + 60
                last_green = -1
                idle_polls = 0
                while time.monotonic() < deadline:
                    time.sleep(0.5)
                    image = capture_window(user32, download, capture_path)
                    status = classify_status(image, template)
                    if status == "failed":
                        raise RuntimeError("原厂下载窗口报告失败，已停止")
                    green_pixels = sum(1 for x in range(20, 600)
                                       if image.getpixel((x, 514))[1] > 110
                                       and image.getpixel((x, 514))[0] < 90)
                    if green_pixels == last_green:
                        idle_polls += 1
                    else:
                        idle_polls = 0
                        last_green = green_pixels
                    if status == "complete":
                        if not compiled.is_file() or compiled.stat().st_size == 0:
                            raise RuntimeError("界面显示完成，但原厂编译包缺失")
                        time.sleep(1)
                        image = capture_window(user32, download, capture_path)
                        if classify_status(image, template) != "complete":
                            raise RuntimeError("原厂完成状态未稳定")
                        report(True, "原厂隔离运行时报告 USB 下载完成")
                        return
                    if idle_polls >= 20:
                        raise RuntimeError("原厂 USB 下载无进展或设备未连接；未报告成功")
                raise RuntimeError("等待原厂 USB 下载结果超时；未报告成功")
        finally:
            if process is not None:
                if win32process.GetExitCodeProcess(process) == 259:
                    win32process.TerminateProcess(process, 0)
                process.Close()
            if thread is not None:
                thread.Close()
            if desktop is not None:
                user32.CloseDesktop(desktop)
            if target is not None:
                target.unlink(missing_ok=True)
            lock_file.seek(0)
            msvcrt.locking(lock_file.fileno(), msvcrt.LK_UNLCK, 1)


def main():  # 20261108-8 add
    """Parse CLI options and run one isolated download; input: None; return: exit code."""
    parser = argparse.ArgumentParser()
    parser.add_argument("--native-source", type=Path, required=True)
    parser.add_argument("--vendor-runtime", type=Path, default=DEFAULT_RUNTIME)
    args = parser.parse_args()
    try:
        run_download(args.native_source.resolve(), args.vendor_runtime.resolve())
        return 0
    except Exception as error:
        report(False, str(error))
        return 1


if __name__ == "__main__":
    sys.exit(main())
