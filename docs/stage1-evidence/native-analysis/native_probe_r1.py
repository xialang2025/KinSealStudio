"""#20261110-5 add: Bounded native multi-page sampling on an owned desktop."""
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
OUT = ROOT / "KinSealStudio/docs/stage1-evidence/native-analysis"
WORK = ROOT / ".scratch/kinseal-stage1-20261110/native/runtime-r1"
DESKTOP = "KinSealStage1Native20261110"

JS = r"""
//20261110-5 add: Every Qt call below runs from the owned GUI-thread event hook.
const gui=Process.getModuleByName('QtGui4.dll');const core=Process.getModuleByName('QtCore4.dll');
const find=new NativeFunction(gui.getExportByName('_ZN7QWidget4findEP6HWND__'),'pointer',['pointer']);
const actionsFn=new NativeFunction(gui.getExportByName('_ZNK7QWidget7actionsEv'),'void',['pointer','pointer']);
const textFn=new NativeFunction(gui.getExportByName('_ZNK7QAction4textEv'),'void',['pointer','pointer']);
const menuFn=new NativeFunction(gui.getExportByName('_ZNK7QAction4menuEv'),'pointer',['pointer']);
const enabledFn=new NativeFunction(gui.getExportByName('_ZNK7QAction9isEnabledEv'),'bool',['pointer']);
const propFn=new NativeFunction(core.getExportByName('_ZNK7QObject8propertyEPKc'),'void',['pointer','pointer','pointer']);
const vs=new NativeFunction(core.getExportByName('_ZNK8QVariant8toStringEv'),'void',['pointer','pointer']);
const vb=new NativeFunction(core.getExportByName('_ZNK8QVariant6toBoolEv'),'bool',['pointer']);
const vd=new NativeFunction(core.getExportByName('_ZN8QVariantD1Ev'),'void',['pointer']);
const sd=new NativeFunction(core.getExportByName('_ZN7QStringD1Ev'),'void',['pointer']);
const sc=new NativeFunction(core.getExportByName('_ZN7QStringC1EPK5QChari'),'void',['pointer','pointer','int']);
const invokeFn=new NativeFunction(core.getExportByName('_ZN11QMetaObject12invokeMethodEP7QObjectPKcN2Qt14ConnectionTypeE22QGenericReturnArgument16QGenericArgumentS7_S7_S7_S7_S7_S7_S7_S7_S7_'),'bool',['pointer','pointer','int',...Array(22).fill('pointer')]);
const setTextFn=new NativeFunction(gui.getExportByName('_ZN9QLineEdit7setTextERK7QString'),'void',['pointer','pointer']);
const spinFn=new NativeFunction(gui.getExportByName('_ZN8QSpinBox8setValueEi'),'void',['pointer','int']);
const comboSetFn=new NativeFunction(gui.getExportByName('_ZN9QComboBox15setCurrentIndexEi'),'void',['pointer','int']);
const comboSignalFn=new NativeFunction(gui.getExportByName('_ZN9QComboBox9activatedEi'),'void',['pointer','int']);
const comboTextFn=new NativeFunction(gui.getExportByName('_ZNK9QComboBox8itemTextEi'),'void',['pointer','pointer','int']);
let pending=null;let result=null;let guiThread=0;let serial=0;
function string(s){const d=s.readPointer();const n=d.add(8).readInt();if(n<0||n>200000){throw Error('QString bound');}return n?d.add(12).readPointer().readUtf16String(n):'';}
function prop(w,n,b=false){const v=Memory.alloc(32);propFn(v,w,Memory.allocUtf8String(n));if(b){const a=vb(v);vd(v);return a;}const s=Memory.alloc(4);vs(s,v);const a=string(s);sd(s);vd(v);return a;}
function kind(w){const f=new NativeFunction(w.readPointer().readPointer(),'pointer',['pointer']);return f(w).add(4).readPointer().readCString();}
function props(w){const r={pointer:w.toString(),kind:kind(w),objectName:prop(w,'objectName'),text:prop(w,'text'),title:prop(w,'windowTitle'),enabled:prop(w,'enabled',true),visible:prop(w,'visible',true)};for(const n of ['minimum','maximum','value','currentIndex','currentText','count','maxLength','checked','checkable','decimals','singleStep','suffix','prefix']){const v=prop(w,n);if(v!==''){r[n]=v;}}if(r.kind==='QComboBox'){r.items=[];for(let i=0;i<Math.min(Number(r.count),256);++i){const s=Memory.alloc(4);comboTextFn(s,w,i);r.items.push(string(s));sd(s);}}return r;}
function actions(w,depth=0){if(depth>6){return [];}const l=Memory.alloc(4);actionsFn(l,w);const d=l.readPointer();const b=d.add(8).readInt(),e=d.add(12).readInt();if(b<0||e<b||e-b>500){throw Error('QList bound');}const out=[];for(let i=b;i<e;++i){const a=d.add(20+i*4).readPointer();const s=Memory.alloc(4);textFn(s,a);const m=menuFn(a);out.push({pointer:a.toString(),objectName:prop(a,'objectName'),text:string(s),enabled:enabledFn(a),children:m.isNull()?[]:actions(m,depth+1)});sd(s);}return out;}
function invoke(w,slot){return invokeFn(w,Memory.allocUtf8String(slot),2,...Array(22).fill(ptr(0)));}
function execute(p){if(p.op==='batch'){return p.hwnds.map(h=>{try{return {hwnd:h,qt:props(find(ptr(h)))};}catch(e){return {hwnd:h,error:String(e)};}});}if(p.op==='menus'){return actions(find(ptr(p.hwnd)));}if(p.op==='invoke'){const w=p.pointer?ptr(p.pointer):find(ptr(p.hwnd));return {queuedMetaInvoke:invoke(w,p.slot),slot:p.slot,pointer:w.toString()};}const w=find(ptr(p.hwnd));if(p.op==='text'){const s=Memory.alloc(4);sc(s,Memory.allocUtf16String(p.value),p.value.length);setTextFn(w,s);sd(s);return props(w);}if(p.op==='spin'){spinFn(w,p.value);return props(w);}if(p.op==='combo'){comboSetFn(w,p.value);comboSignalFn(w,p.value);return props(w);}throw Error('unsupported operation');}
Interceptor.attach(core.getExportByName('_ZN16QCoreApplication14notifyInternalEP7QObjectP6QEvent'),{onEnter(){if(Process.getCurrentThreadId()!==guiThread||pending===null){return;}const p=pending;pending=null;try{result={serial:p.serial,status:'completed',value:execute(p),guiThread:Process.getCurrentThreadId()};}catch(e){result={serial:p.serial,status:'error',error:String(e)};}}});
rpc.exports={setThread(t){guiThread=t;},queue(p){if(pending!==null){throw Error('operation pending');}p.serial=++serial;pending=p;result=null;return serial;},poll(){return result;}};
"""


def windows(desktop, pid):
    """Enumerate only this task's target process on its private desktop."""
    found = []
    win32gui.EnumDesktopWindows(pywintypes.HANDLE(desktop), lambda h, _: found.append(h) if win32process.GetWindowThreadProcessId(h)[1] == pid else None, None)
    return found


def main():
    """Create native samples through actual actions and preserve original inputs."""
    WORK.mkdir(parents=True, exist_ok=False)
    original = (VENDOR / "KinSealStudio5.1.exe").read_bytes()
    assert hashlib.sha256(original).hexdigest() == "305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c"
    private = bytearray(original)
    assert private[0x7c4:0x7c7] == bytes.fromhex("5589e5")
    private[0x7c4:0x7c7] = bytes.fromhex("b001c3")
    assert hashlib.sha256(private).hexdigest() == "38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89"
    exe = WORK / "KinSealStudio5.1-private.exe"
    exe.write_bytes(private)
    for name in ("model.dat", "modelInfo.dat", "language.ini"):
        shutil.copy2(VENDOR / name, WORK / name)
    source = WORK / "NativePages.sm"
    shutil.copy2(ROOT / "KinSealStudio/samples/native-device-test/CodexProbeTest.sm", source)
    result = {"date": "2026-11-10", "originalSha256": hashlib.sha256(original).hexdigest(), "runtimeSha256": hashlib.sha256(private).hexdigest(), "patch": {"offset": "0x7c4", "before": "5589e5", "after": "b001c3"}, "desktop": DESKTOP, "input": {"path": "KinSealStudio/samples/native-device-test/CodexProbeTest.sm", "sha256": hashlib.sha256(source.read_bytes()).hexdigest()}, "steps": [], "snapshots": [], "systemInput": False, "usbOrPlc": False, "qtOperationsGuiThreadOnly": True}
    u = ctypes.WinDLL("user32", use_last_error=True)
    u.CreateDesktopW.argtypes = [ctypes.c_wchar_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p]
    u.CreateDesktopW.restype = ctypes.c_void_p
    u.CloseDesktop.argtypes = [ctypes.c_void_p]
    desk = u.CreateDesktopW(DESKTOP, None, None, 0, 0x000F01FF, None)
    if not desk:
        raise OSError(ctypes.get_last_error(), "private desktop unavailable")
    process = thread = session = None
    try:
        startup = win32process.STARTUPINFO()
        startup.lpDesktop = "WinSta0\\" + DESKTOP
        startup.dwFlags = win32con.STARTF_USESHOWWINDOW
        startup.wShowWindow = win32con.SW_SHOWNORMAL
        env = os.environ.copy()
        env["PATH"] = str(VENDOR) + os.pathsep + env.get("PATH", "")
        env["__COMPAT_LAYER"] = "RunAsInvoker"
        process, thread, pid, _ = win32process.CreateProcess(str(exe), '"%s" "%s"' % (exe, source), None, None, False, win32process.CREATE_UNICODE_ENVIRONMENT, env, str(WORK), startup)
        result["pid"] = pid
        deadline = time.time() + 24
        editor = None
        while time.time() < deadline:
            for top in windows(desk, pid):
                children = []
                win32gui.EnumChildWindows(top, lambda h, _: children.append(h), None)
                if any(win32gui.GetWindowText(h) == "menuBar" for h in children):
                    editor = top
                    break
            if editor:
                break
            time.sleep(.25)
        if editor is None:
            raise RuntimeError("editor unavailable within 24 seconds")
        time.sleep(1.5)
        children = []
        win32gui.EnumChildWindows(editor, lambda h, _: children.append(h), None)
        menu = next(h for h in children if win32gui.GetWindowText(h) == "menuBar")
        session = frida.attach(pid)
        script = session.create_script(JS)
        script.on("message", lambda m, _: result.setdefault("instrumentationMessages", []).append(m))
        script.load()
        api = script.exports_sync
        api.set_thread(win32process.GetWindowThreadProcessId(menu)[0])

        def op(request):
            serial = api.queue(request)
            win32gui.PostMessage(menu, win32con.WM_MOUSEMOVE, 0, 0)
            limit = time.time() + 5
            while time.time() < limit:
                r = api.poll()
                if r and r["serial"] == serial:
                    result["steps"].append({"request": request, "result": r})
                    if r["status"] != "completed":
                        raise RuntimeError(str(r))
                    return r["value"]
                time.sleep(.04)
            raise RuntimeError("GUI-thread queued operation timeout: " + str(request))

        def dump(label):
            rows = []
            for top in windows(desk, pid):
                if not win32gui.IsWindowVisible(top):
                    continue
                cs = [top]
                win32gui.EnumChildWindows(top, lambda h, _: cs.append(h), None)
                rows.append({"hwnd": top, "title": win32gui.GetWindowText(top), "widgets": op({"op": "batch", "hwnds": cs})})
            result["snapshots"].append({"label": label, "windows": rows})
            return rows

        def action(name):
            names = []
            for h in children:
                if win32gui.GetWindowText(h) in ("menuBar", "standardToolBar"):
                    def walk(rows):
                        for row in rows:
                            yield row
                            yield from walk(row.get("children", []))
                    names.extend(walk(op({"op": "menus", "hwnd": h})))
            row = next(r for r in names if r["objectName"] == name)
            assert row["enabled"], (name, row)
            r = op({"op": "invoke", "pointer": row["pointer"], "slot": "trigger"})
            assert r["queuedMetaInvoke"]
            time.sleep(.35)

        dump("initial")
        action("action_checkall")
        action("action_delete")
        action("action_save")
        shutil.copy2(source, OUT / "A-empty-one-page.sm")
        dump("A-empty-one-page-saved")
        action("action_new_screen")
        dump("new-second-page-dialog")
        result["status"] = "captured-page-create-dialog"
    except Exception as exc:
        result["status"] = "partial-or-error"
        result["error"] = repr(exc)
    finally:
        if session:
            session.detach()
        if process:
            result["exitCodeBeforeCleanup"] = win32process.GetExitCodeProcess(process)
            if result["exitCodeBeforeCleanup"] == 259:
                win32process.TerminateProcess(process, 0)
            process.Close()
            result["ownedProcessTerminated"] = True
        if thread:
            thread.Close()
        u.CloseDesktop(desk)
        try:
            with winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"Software\KinSealStudio\recent pro files", 0, winreg.KEY_QUERY_VALUE | winreg.KEY_SET_VALUE) as key:
                recent, kind = winreg.QueryValueEx(key, "recentFileList")
                own = str(source).replace("\\", "/").casefold()
                remaining = [x for x in recent if x.replace("\\", "/").casefold() != own]
                if kind == winreg.REG_MULTI_SZ and len(remaining) != len(recent):
                    winreg.SetValueEx(key, "recentFileList", 0, kind, remaining)
                result["ownMruCleanup"] = {"removed": len(recent) - len(remaining), "otherEntriesPreserved": len(remaining)}
        except OSError as exc:
            result["ownMruCleanup"] = {"error": repr(exc)}
        (OUT / "native-probe.json").write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({"status": result["status"], "pid": result.get("pid"), "error": result.get("error"), "evidence": str(OUT / "native-probe.json")}, ensure_ascii=False))


if __name__ == "__main__":
    main()
