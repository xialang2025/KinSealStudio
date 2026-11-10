"""Read entry identities from an owned native process on a private desktop.

#20261110-4 add: No command activation, system input, compile or transfer occurs.
"""
from pathlib import Path
import ctypes
import hashlib
import json
import os
import shutil
import sys
import time
import winreg

import pywintypes
import win32con
import win32gui
import win32process

sys.dont_write_bytecode = True
sys.stdout.reconfigure(encoding="utf-8")
sys.path.insert(0, r"G:\01_Professional\Frida\Installed")
import frida

ROOT = Path(r"Z:\Project\vscode_prj_temp\REA")
VENDOR = Path(r"G:\01_Professional\触摸屏\金玺\KinSealStudio_Setup_V5.1\Installed")
OUT = ROOT / "KinSealStudio/docs/stage0-evidence"
WORK = ROOT / ".scratch/kinseal-stage0-20261110/entries/runtime"
DESKTOP = "KinSealStage0Entries20261110"

JS = r"""
//20261110-4 add: Read only QAction identities and association, preserving failures.
const gui = Process.getModuleByName('QtGui4.dll');
const core = Process.getModuleByName('QtCore4.dll');
const find = new NativeFunction(gui.getExportByName('_ZN7QWidget4findEP6HWND__'), 'pointer', ['pointer']);
const actions = new NativeFunction(gui.getExportByName('_ZNK7QWidget7actionsEv'), 'void', ['pointer', 'pointer']);
const atext = new NativeFunction(gui.getExportByName('_ZNK7QAction4textEv'), 'void', ['pointer', 'pointer']);
const amenu = new NativeFunction(gui.getExportByName('_ZNK7QAction4menuEv'), 'pointer', ['pointer']);
const enabled = new NativeFunction(gui.getExportByName('_ZNK7QAction9isEnabledEv'), 'bool', ['pointer']);
const property = new NativeFunction(core.getExportByName('_ZNK7QObject8propertyEPKc'), 'void', ['pointer', 'pointer', 'pointer']);
const vs = new NativeFunction(core.getExportByName('_ZNK8QVariant8toStringEv'), 'void', ['pointer', 'pointer']);
const vb = new NativeFunction(core.getExportByName('_ZNK8QVariant6toBoolEv'), 'bool', ['pointer']);
const vd = new NativeFunction(core.getExportByName('_ZN8QVariantD1Ev'), 'void', ['pointer']);
const sd = new NativeFunction(core.getExportByName('_ZN7QStringD1Ev'), 'void', ['pointer']);
const shortcutFn = new NativeFunction(gui.getExportByName('_ZNK7QAction8shortcutEv'), 'void', ['pointer','pointer']);
const keyTextFn = new NativeFunction(gui.getExportByName('_ZNK12QKeySequence8toStringENS_14SequenceFormatE'), 'void', ['pointer','pointer','int']);
const keyDestroyFn = new NativeFunction(gui.getExportByName('_ZN12QKeySequenceD1Ev'), 'void', ['pointer']);
function str(s) { const d = s.readPointer(); const n = d.add(8).readInt(); if (n < 0 || n > 100000) { throw Error('invalid QString length'); } return n ? d.add(12).readPointer().readUtf16String(n) : ''; }
function prop(w, name, bool=false) { const v=Memory.alloc(32); property(v,w,Memory.allocUtf8String(name)); if (bool) { const b=vb(v); vd(v); return b; } const s=Memory.alloc(4);vs(s,v);const r=str(s);sd(s);vd(v);return r; }
function props(w) { if(w.isNull()) { return null; } return {objectName:prop(w,'objectName'), text:prop(w,'text'), title:prop(w,'title'), enabled:prop(w,'enabled',true), visible:prop(w,'visible',true)}; }
function readList(w, depth=0) {
    if(w.isNull() || depth>6) { return []; }
    const list=Memory.alloc(4);actions(list,w);const d=list.readPointer();const begin=d.add(8).readInt();const end=d.add(12).readInt();
    if(begin<0 || end<begin || end-begin>500) { throw Error('invalid QList bounds'); }
    const rows=[];
    for(let i=begin;i<end;++i) {
        const row={listIndex:i};
        try {
            const a=d.add(16+i*4).readPointer();row.pointer=a.toString();
            const s=Memory.alloc(4);atext(s,a);row.text=str(s);sd(s);row.objectName=prop(a,'objectName');row.enabled=enabled(a);row.checkable=prop(a,'checkable',true);row.checked=prop(a,'checked',true);row.separator=prop(a,'separator',true);
            try { const k=Memory.alloc(16);shortcutFn(k,a); const t=Memory.alloc(4);keyTextFn(t,k,1);row.shortcutPortable=str(t);sd(t);keyDestroyFn(k); } catch(e) { row.shortcutError=String(e); }
            const m=amenu(a);row.menuPointer=m.toString();row.children=m.isNull()?[]:readList(m,depth+1);
        } catch(e) { row.error=String(e); }
        rows.push(row);
    }
    return rows;
}
rpc.exports={read(hwnd) {const w=find(ptr(hwnd));return {qt:props(w),actions:readList(w)};},props(hwnd) {return props(find(ptr(hwnd)));}};
"""


def owned_windows(desktop, pid):
    """Return only windows owned by the newly created target PID."""
    found = []
    win32gui.EnumDesktopWindows(pywintypes.HANDLE(desktop), lambda h, _: found.append(h) if win32process.GetWindowThreadProcessId(h)[1] == pid else None, None)
    return found


def run_probe(executable, desktop, mode):
    """Start one owned process, retain before/after identity and close only it."""
    record = {"mode": mode, "input": None, "system_input": False, "commands_activated": [], "startedAtUtc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())}
    startup = win32process.STARTUPINFO()
    startup.lpDesktop = "WinSta0\\" + DESKTOP
    startup.dwFlags = win32con.STARTF_USESHOWWINDOW
    startup.wShowWindow = win32con.SW_SHOWNORMAL
    env = os.environ.copy()
    env["PATH"] = str(VENDOR) + os.pathsep + env.get("PATH", "")
    env["__COMPAT_LAYER"] = "RunAsInvoker"
    command = '"%s"' % executable
    if mode == "opened-probe-unmodified":
        source = WORK / "Probe.sm"
        record["input"] = {"retainedPath": "KinSealStudio/samples/native-device-test/CodexProbeTest.sm", "sha256": hashlib.sha256(source.read_bytes()).hexdigest()}
        command += ' "%s"' % source
    process = thread = session = None
    try:
        process, thread, pid, _ = win32process.CreateProcess(str(executable), command, None, None, False, win32process.CREATE_UNICODE_ENVIRONMENT, env, str(WORK), startup)
        record["pid"] = pid
        record["commandLine"] = command
        deadline = time.time() + 24
        editor = None
        children = []
        while time.time() < deadline and win32process.GetExitCodeProcess(process) == 259:
            for top in owned_windows(desktop, pid):
                items = []
                win32gui.EnumChildWindows(top, lambda h, _: items.append(h), None)
                if any(win32gui.GetWindowText(h) == "menuBar" for h in items):
                    editor, children = top, items
                    break
            if editor:
                break
            time.sleep(.25)
        if editor is None:
            record["status"] = "unavailable-editor-not-observed-within-24s"
            return record
        time.sleep(1.2)
        record["before"] = {"editorHwnd": editor, "editorTitle": win32gui.GetWindowText(editor), "exitCode": win32process.GetExitCodeProcess(process)}
        session = frida.attach(pid)
        script = session.create_script(JS)
        script.load()
        record["widgets"] = []
        record["surfaces"] = []
        for hwnd in children:
            item = {"hwnd": hwnd, "winText": win32gui.GetWindowText(hwnd), "winClass": win32gui.GetClassName(hwnd)}
            try:
                item["qt"] = script.exports_sync.props(hwnd)
                name = (item.get("qt") or {}).get("objectName", "")
                if name == "menuBar" or "toolbar" in name.casefold():
                    surface = dict(item)
                    surface["reading"] = script.exports_sync.read(hwnd)
                    record["surfaces"].append(surface)
            except Exception as exc:
                item["error"] = repr(exc)
            record["widgets"].append(item)
        record["after"] = {"editorTitle": win32gui.GetWindowText(editor), "exitCode": win32process.GetExitCodeProcess(process), "topWindows": [{"hwnd": h, "title": win32gui.GetWindowText(h), "visible": bool(win32gui.IsWindowVisible(h))} for h in owned_windows(desktop, pid)]}
        record["status"] = "bounded-read-only-captured"
    except Exception as exc:
        record["status"] = "partial-or-error"
        record["error"] = repr(exc)
    finally:
        if session:
            session.detach()
        if process:
            record["exitCodeBeforeOwnedCleanup"] = win32process.GetExitCodeProcess(process)
            if record["exitCodeBeforeOwnedCleanup"] == 259:
                win32process.TerminateProcess(process, 0)
            record["ownedProcessTerminated"] = True
            process.Close()
        if thread:
            thread.Close()
        record["endedAtUtc"] = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
    return record


def main():
    """Check exact artifact identities and save a reproducible bounded observation."""
    WORK.mkdir(parents=True, exist_ok=False)
    original = (VENDOR / "KinSealStudio5.1.exe").read_bytes()
    assert hashlib.sha256(original).hexdigest() == "305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c"
    private = bytearray(original)
    assert private[0x7c4:0x7c7] == bytes.fromhex("5589e5")
    private[0x7c4:0x7c7] = bytes.fromhex("b001c3")
    runtime_hash = hashlib.sha256(private).hexdigest()
    assert runtime_hash == "38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89"
    executable = WORK / "KinSealStudio5.1-private.exe"
    executable.write_bytes(private)
    for name in ("model.dat", "modelInfo.dat", "language.ini"):
        shutil.copy2(VENDOR / name, WORK / name)
    shutil.copy2(ROOT / "KinSealStudio/samples/native-device-test/CodexProbeTest.sm", WORK / "Probe.sm")
    u = ctypes.WinDLL("user32", use_last_error=True)
    u.CreateDesktopW.argtypes = [ctypes.c_wchar_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p]
    u.CreateDesktopW.restype = ctypes.c_void_p
    u.CloseDesktop.argtypes = [ctypes.c_void_p]
    desktop = u.CreateDesktopW(DESKTOP, None, None, 0, 0x000F01FF, None)
    if not desktop:
        raise OSError(ctypes.get_last_error(), "private desktop unavailable")
    result = {"date": "2026-11-10", "taskIds": ["S00-02", "S00-05"], "originalPath": str(VENDOR / "KinSealStudio5.1.exe"), "originalSha256": hashlib.sha256(original).hexdigest(), "runtimeSha256": runtime_hash, "patch": {"offset": "0x7c4", "before": "5589e5", "after": "b001c3", "purpose": "known startup token check"}, "desktop": DESKTOP, "probeRecipe": "entry-probe-readonly.py", "limitations": ["Read-only bounded Qt property snapshots; no action behavior, selection count, saving, compilation or device runtime inferred", "Only root EXE and small config copied; dependencies loaded through original installation PATH", "No desktop switching, focus API, SendInput or system keyboard/mouse used", "Unobserved source fields and device consumers remain unknown"], "runs": []}
    try:
        for mode in ("no-project", "opened-probe-unmodified"):
            result["runs"].append(run_probe(executable, desktop, mode))
    finally:
        u.CloseDesktop(desktop)
        try:
            with winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"Software\KinSealStudio\recent pro files", 0, winreg.KEY_QUERY_VALUE | winreg.KEY_SET_VALUE) as key:
                recent, kind = winreg.QueryValueEx(key, "recentFileList")
                own = str(WORK / "Probe.sm").replace("\\", "/").casefold()
                remaining = [v for v in recent if v.replace("\\", "/").casefold() != own]
                if kind == winreg.REG_MULTI_SZ and len(remaining) != len(recent):
                    winreg.SetValueEx(key, "recentFileList", 0, kind, remaining)
                result["ownMruCleanup"] = {"removed": len(recent) - len(remaining), "otherEntriesPreserved": len(remaining)}
        except OSError as exc:
            result["ownMruCleanup"] = {"error": repr(exc)}
        target = OUT / "entry-probe-readonly.json"
        target.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({"evidence": str(target), "runs": [{"mode": r["mode"], "status": r["status"], "pid": r.get("pid"), "error": r.get("error")} for r in result["runs"]]}, ensure_ascii=False))


if __name__ == "__main__":
    main()
