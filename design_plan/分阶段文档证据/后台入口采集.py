"""Capture owned KinSeal menus on a private desktop without foreground input. 20261110-3 add.

The temporary executable is the previously identified private copy. Its exact
digest is checked before execution. Only the owned process is terminated.
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

import win32api
import win32con
import win32gui
import win32process

sys.dont_write_bytecode = True
sys.stdout.reconfigure(encoding="utf-8")
sys.path.insert(0, r"G:\01_Professional\Frida\Installed")
import frida

ROOT = Path(r"Z:\Project\vscode_prj_temp\REA")
VENDOR = Path(r"G:\01_Professional\触摸屏\金玺\KinSealStudio_Setup_V5.1\Installed")
OUT = ROOT / "KinSealStudio/design_plan/分阶段文档证据"
WORK = ROOT / ".scratch/kinseal-stages-20261110/transient/S02"
DESKTOP = "KinSealStages20261110"

JS = r"""
const gui=Process.getModuleByName('QtGui4.dll');
const core=Process.getModuleByName('QtCore4.dll');
const find=new NativeFunction(gui.getExportByName('_ZN7QWidget4findEP6HWND__'),'pointer',['pointer']);
const actions=new NativeFunction(gui.getExportByName('_ZNK7QWidget7actionsEv'),'void',['pointer','pointer']);
const text=new NativeFunction(gui.getExportByName('_ZNK7QAction4textEv'),'void',['pointer','pointer']);
const menu=new NativeFunction(gui.getExportByName('_ZNK7QAction4menuEv'),'pointer',['pointer']);
const enabled=new NativeFunction(gui.getExportByName('_ZNK7QAction9isEnabledEv'),'bool',['pointer']);
const property=new NativeFunction(core.getExportByName('_ZNK7QObject8propertyEPKc'),'void',['pointer','pointer','pointer']);
const vs=new NativeFunction(core.getExportByName('_ZNK8QVariant8toStringEv'),'void',['pointer','pointer']);
const vb=new NativeFunction(core.getExportByName('_ZNK8QVariant6toBoolEv'),'bool',['pointer']);
const vd=new NativeFunction(core.getExportByName('_ZN8QVariantD1Ev'),'void',['pointer']);
const sd=new NativeFunction(core.getExportByName('_ZN7QStringD1Ev'),'void',['pointer']);
const knownActions={};let pendingAction=null;let queuedResult=null;let guiThread=0;
const activate=new NativeFunction(gui.getExportByName('_ZN7QAction8activateENS_11ActionEventE'),'void',['pointer','int']);
Interceptor.attach(core.getExportByName('_ZN16QCoreApplication14notifyInternalEP7QObjectP6QEvent'),{onEnter(){if(pendingAction!==null&&Process.getCurrentThreadId()===guiThread){const a=pendingAction;pendingAction=null;try{activate(a,0);queuedResult='activated';}catch(e){queuedResult=String(e);}}}});
function str(s){const d=s.readPointer(); const n=d.add(8).readInt(); return n?d.add(12).readPointer().readUtf16String(n):'';}
function prop(w,n,bool=false){const v=Memory.alloc(32);property(v,w,Memory.allocUtf8String(n));if(bool){const r=vb(v);vd(v);return r;} const s=Memory.alloc(4);vs(s,v);const r=str(s);sd(s);vd(v);return r;}
function list(w,depth){if(w.isNull()||depth>4)return []; const out=Memory.alloc(4);actions(out,w);const d=out.readPointer();const begin=d.add(8).readInt(),end=d.add(12).readInt();if(begin<0||end<begin||end-begin>300)throw Error('Unsupported action-list bounds');const rows=[];for(let i=begin;i<end;i++){try{const a=d.add(16+i*4).readPointer();if(a.isNull())continue;const s=Memory.alloc(4);text(s,a);const title=str(s);sd(s);const m=menu(a);const name=prop(a,'objectName');if(name)knownActions[name]=a;const row={text:title,objectName:name,enabled:enabled(a),checkable:prop(a,'checkable',true),checked:prop(a,'checked',true),shortcutStringConversion:prop(a,'shortcut'),children:[]};try{if(!m.isNull())row.children=list(m,depth+1);}catch(e){row.childrenError=String(e);}rows.push(row);}catch(e){rows.push({index:i,error:String(e)});}}return rows;}
rpc.exports={setGuiThread(id){guiThread=id;},menus(hwnd){return list(find(ptr(hwnd)),0);},queue(name){if(!knownActions[name])return 'unknown action';pendingAction=knownActions[name];queuedResult=null;return 'queued';},actionStatus(){return queuedResult;},props(hwnd){const w=find(ptr(hwnd));if(w.isNull())return null;const r={objectName:prop(w,'objectName'),text:prop(w,'text'),title:prop(w,'title'),enabled:prop(w,'enabled',true),visible:prop(w,'visible',true)};try{const meta=new NativeFunction(w.readPointer().readPointer(),'pointer',['pointer'])(w);r.kind=meta.add(4).readPointer().readCString();}catch(e){r.kindError=String(e);}for(const n of ['minimum','maximum','value','currentIndex','currentText','count','maxLength','checked','checkable','decimals','singleStep','suffix','prefix']){try{const v=prop(w,n);if(v!=='')r[n]=v;}catch(e){r[n+'Error']=String(e);}}if(r.kind==='QComboBox'||r.kind==='QFontComboBox'){const count=Number(r.count);r.items=[];for(let i=0;i<Math.min(count,64);i++){const s=Memory.alloc(4);new NativeFunction(gui.getExportByName('_ZNK9QComboBox8itemTextEi'),'void',['pointer','pointer','int'])(s,w,i);r.items.push(str(s));sd(s);}r.itemsTruncated=count>64;}return r;}};
"""


def windows(desktop, pid):
    """Enumerate only top-level windows owned by the captured process."""
    import pywintypes
    found = []
    win32gui.EnumDesktopWindows(pywintypes.HANDLE(desktop),
        lambda h, _: found.append(h) if win32process.GetWindowThreadProcessId(h)[1] == pid else None, None)
    return found


def main():
    """Retain compact JSON evidence; leave temporary files to the stage cleaner."""
    if WORK.exists() and not (WORK / "KinSealStudio5.1-private.exe").exists():
        raise FileExistsError("Existing directory is not this probe's runtime: " + str(WORK))
    WORK.mkdir(parents=True, exist_ok=True)
    original = (VENDOR / "KinSealStudio5.1.exe").read_bytes()
    assert hashlib.sha256(original).hexdigest() == "305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c"
    private = bytearray(original)
    assert private[0x7c4:0x7c7] == bytes.fromhex("5589e5")
    private[0x7c4:0x7c7] = bytes.fromhex("b001c3")
    assert hashlib.sha256(private).hexdigest() == "38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89"
    executable = WORK / "KinSealStudio5.1-private.exe"
    executable.write_bytes(private)
    for name in ("model.dat", "modelInfo.dat", "language.ini"):
        shutil.copy2(VENDOR / name, WORK / name)
    shutil.copy2(ROOT / "KinSealStudio/samples/native-device-test/CodexProbeTest.sm", WORK / "Probe.sm")
    u = ctypes.WinDLL("user32", use_last_error=True)
    u.CreateDesktopW.argtypes = [ctypes.c_wchar_p,ctypes.c_void_p,ctypes.c_void_p,ctypes.c_ulong,ctypes.c_ulong,ctypes.c_void_p]
    u.CreateDesktopW.restype = ctypes.c_void_p
    u.OpenDesktopW.argtypes = [ctypes.c_wchar_p,ctypes.c_ulong,ctypes.c_bool,ctypes.c_ulong]
    u.OpenDesktopW.restype = ctypes.c_void_p
    u.CloseDesktop.argtypes = [ctypes.c_void_p]
    desk = u.OpenDesktopW(DESKTOP,0,False,0x000F01FF) or u.CreateDesktopW(DESKTOP,None,None,0,0x000F01FF,None)
    if not desk:
        raise OSError(ctypes.get_last_error(), "Private desktop unavailable")
    startup = win32process.STARTUPINFO()
    startup.lpDesktop = "WinSta0\\" + DESKTOP
    startup.dwFlags = win32con.STARTF_USESHOWWINDOW
    startup.wShowWindow = win32con.SW_SHOWNORMAL
    env = os.environ.copy()
    env["PATH"] = str(VENDOR) + os.pathsep + env.get("PATH", "")
    env["__COMPAT_LAYER"] = "RunAsInvoker"
    process = thread = session = None
    result = {"date":"2026-11-10", "scope":"Private desktop menus and default drawing dialogs; no compile/download or system input", "original_sha256":hashlib.sha256(original).hexdigest(), "runtime_sha256":hashlib.sha256(private).hexdigest(), "runtime_patch":{"offset":"0x7c4","old":"5589e5","new":"b001c3","purpose":"previously identified startup token check"}, "desktop":DESKTOP, "input_sample":{"retained_path":"KinSealStudio/samples/native-device-test/CodexProbeTest.sm","sha256":hashlib.sha256((WORK/'Probe.sm').read_bytes()).hexdigest()}, "limitations":["Only root executable and small model files copied; dependencies resolved through installed PATH", "Command existence/enabled snapshot does not establish command behavior", "shortcutStringConversion is a QVariant conversion result; empty does not prove absence of shortcut", "Default dialogs only; creation gestures, saving, all branches and device semantics remain untested"]}
    try:
        process,thread,pid,_ = win32process.CreateProcess(str(executable), '"%s" "%s"' % (executable,WORK / "Probe.sm"),None,None,False,win32process.CREATE_UNICODE_ENVIRONMENT,env,str(WORK),startup)
        result["pid"] = pid
        deadline = time.time() + 22
        editor = None
        while time.time() < deadline:
            tops = windows(desk,pid)
            for candidate in tops:
                candidate_children = []
                win32gui.EnumChildWindows(candidate,lambda h,_:candidate_children.append(h),None)
                if any(win32gui.GetWindowText(h)=="menuBar" for h in candidate_children):
                    editor = candidate
                    break
            if editor:
                break
            if win32process.GetExitCodeProcess(process) != 259:
                break
            time.sleep(.3)
        result["exit_code_before_cleanup"] = win32process.GetExitCodeProcess(process)
        result["top_windows"] = [{"hwnd":h,"title":win32gui.GetWindowText(h),"class":win32gui.GetClassName(h),"visible":bool(win32gui.IsWindowVisible(h))} for h in windows(desk,pid)]
        if editor is None:
            result["status"] = "unavailable: editor did not appear within bounded wait"
        else:
            time.sleep(1)
            session = frida.attach(pid)
            native = session.create_script(JS)
            native.load()
            native.exports_sync.set_gui_thread(win32process.GetWindowThreadProcessId(editor)[0])
            children = []
            win32gui.EnumChildWindows(editor,lambda h,_:children.append(h),None)
            result["editor_title"] = win32gui.GetWindowText(editor)
            result["widgets"] = []
            for h in children:
                try:
                    result["widgets"].append({"hwnd":h,"win_text":win32gui.GetWindowText(h),"class":win32gui.GetClassName(h),"qt":native.exports_sync.props(h)})
                except Exception as exc:
                    result["widgets"].append({"hwnd":h,"error":str(exc)})
            menu_bar = next((h for h in children if win32gui.GetWindowText(h)=="menuBar"),None)
            result["menus"] = native.exports_sync.menus(menu_bar) if menu_bar else []
            result["status"] = "captured" if result["menus"] else "partial: no menu bar found"
            result["drawing_dialogs"] = []
            for action_name in ("action_line","action_freeline","action_blight_circle","action_foldline","action_RoundedRect","action_ellipse","action_ArcItem","action_rect","action_polygon","action_table"):
                before = set(windows(desk,pid))
                observation = {"action":action_name,"queue_status":native.exports_sync.queue(action_name)}
                win32gui.PostMessage(editor,win32con.WM_NULL,0,0)
                time.sleep(.6)
                observation["activation_result"] = native.exports_sync.action_status()
                dialogs = [h for h in windows(desk,pid) if h!=editor and win32gui.IsWindowVisible(h) and win32gui.GetClassName(h)=="QWidget"]
                observation["dialogs"] = []
                for dialog in dialogs:
                    record = {"title":win32gui.GetWindowText(dialog),"new_window":dialog not in before,"widgets":[]}
                    items=[]
                    win32gui.EnumChildWindows(dialog,lambda h,_:items.append(h),None)
                    for h in items:
                        try:
                            record["widgets"].append({"win_text":win32gui.GetWindowText(h),"qt":native.exports_sync.props(h)})
                        except Exception as exc:
                            record["widgets"].append({"win_text":win32gui.GetWindowText(h),"error":str(exc)})
                    observation["dialogs"].append(record)
                    win32gui.PostMessage(dialog,win32con.WM_CLOSE,0,0)
                result["drawing_dialogs"].append(observation)
                time.sleep(.12)
            def errors(rows):
                return sum(bool(x.get("error") or x.get("childrenError")) + errors(x.get("children",[])) for x in rows)
            result["menu_read_errors"] = errors(result.get("menus",[]))
            if result["menu_read_errors"]:
                result["status"] = "captured_partial: some menu entries unreadable"
            result["coverage"] = {"status":"bounded-partial","root_menus_returned":len(result.get("menus",[])),"widgets_returned":len(result.get("widgets",[])),"menu_read_errors":result["menu_read_errors"],"drawing_dialogs_captured":sum(len(x["dialogs"]) for x in result["drawing_dialogs"])}
            result["limitations"].append("No dialog after this action-only probe does not prove absence of properties or determine the drawing gesture")
    except Exception as exc:
        result["status"] = "partial/error"
        result["error"] = repr(exc)
    finally:
        if session is not None:
            session.detach()
        if process is not None:
            if win32process.GetExitCodeProcess(process)==259:
                win32process.TerminateProcess(process,0)
            process.Close()
        if thread is not None:
            thread.Close()
        u.CloseDesktop(desk)
        # Remove only this probe's MRU entry; preserve every other current entry.
        try:
            with winreg.OpenKey(winreg.HKEY_CURRENT_USER,r"Software\KinSealStudio\recent pro files",0,winreg.KEY_QUERY_VALUE|winreg.KEY_SET_VALUE) as key:
                recent,kind=winreg.QueryValueEx(key,"recentFileList")
                own=str(WORK/'Probe.sm').replace('\\','/').casefold()
                remaining=[v for v in recent if v.replace('\\','/').casefold()!=own]
                if kind==winreg.REG_MULTI_SZ and len(remaining)!=len(recent):
                    winreg.SetValueEx(key,"recentFileList",0,kind,remaining)
                result["recent_list_cleanup"]={"own_entry_removed":len(recent)-len(remaining),"other_entries_preserved":len(remaining)}
        except OSError as exc:
            result["recent_list_cleanup"]={"error":str(exc)}
        target = OUT / "V51后台入口采集_20261110.json"
        target.write_text(json.dumps(result,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
        print(json.dumps({"status":result["status"],"evidence":str(target),"menus":len(result.get("menus",[])),"widgets":len(result.get("widgets",[])),"temporary_directory":str(WORK)},ensure_ascii=False))


if __name__ == "__main__":
    main()
