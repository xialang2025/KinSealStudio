"""#20261110-4 add: Execute bounded private-window selection/dirty-state probes."""
from pathlib import Path
import ctypes
import hashlib
import importlib.util
import json
import os
import shutil
import sys
import time
import winreg
import win32con
import win32gui
import win32process

sys.dont_write_bytecode = True
sys.stdout.reconfigure(encoding="utf-8")
ROOT = Path(r"Z:\Project\vscode_prj_temp\REA")
OUT = ROOT / "KinSealStudio/docs/stage0-evidence"
spec = importlib.util.spec_from_file_location("entry_read", OUT / "entry-probe-readonly.py")
base = importlib.util.module_from_spec(spec)
spec.loader.exec_module(base)
WORK = ROOT / ".scratch/kinseal-stage0-20261110/entries/states-runtime-r2"
DESKTOP = "KinSealStage0EntriesStates20261110"
SOURCE = OUT / "baseline-run-usb-r2/baseline.sm"

EXTRA_JS = r"""
//20261110-4 add: Normal GUI-thread QAction activation, observed graphics selection.
let pending=null;let last=null;let guiThread=0;let guiMatches=0;
const activate=new NativeFunction(gui.getExportByName('_ZN7QAction8activateENS_11ActionEventE'),'void',['pointer','int']);
const sceneFn=new NativeFunction(gui.getExportByName('_ZNK13QGraphicsView5sceneEv'),'pointer',['pointer']);
const selectedFn=new NativeFunction(gui.getExportByName('_ZNK14QGraphicsScene13selectedItemsEv'),'void',['pointer','pointer']);
const itemsFn=new NativeFunction(gui.getExportByName('_ZNK14QGraphicsScene5itemsEN2Qt9SortOrderE'),'void',['pointer','pointer','int']);
const boundsFn=new NativeFunction(gui.getExportByName('_ZNK13QGraphicsItem17sceneBoundingRectEv'),'void',['pointer','pointer']);
const viewportFn=new NativeFunction(gui.getExportByName('_ZNK19QAbstractScrollArea8viewportEv'),'pointer',['pointer']);
const winIdFn=new NativeFunction(gui.getExportByName('_ZNK7QWidget5winIdEv'),'pointer',['pointer']);
const mapSceneFn=new NativeFunction(gui.getExportByName('_ZNK13QGraphicsView12mapFromSceneERK7QPointF'),'uint64',['pointer','pointer']);
function classes(w){const metaFn=new NativeFunction(w.readPointer().readPointer(),'pointer',['pointer']);let m=metaFn(w);const a=[];for(let i=0;i<24&&!m.isNull();++i){a.push(m.add(4).readPointer().readCString());m=m.readPointer();}return a;}
function itemList(s,selected){const o=Memory.alloc(4);if(selected){selectedFn(o,s);}else{itemsFn(o,s,0);}const d=o.readPointer();const b=d.add(8).readInt();const e=d.add(12).readInt();if(b<0||e<b||e-b>5000){throw Error('invalid graphics item list');}const a=[];for(let i=b;i<e;++i){const p=d.add(20+4*i).readPointer();const r=Memory.alloc(32);boundsFn(r,p);a.push({pointer:p.toString(),bounds:{x:r.readDouble(),y:r.add(8).readDouble(),width:r.add(16).readDouble(),height:r.add(24).readDouble()}});}return a;}
Interceptor.attach(core.getExportByName('_ZN16QCoreApplication14notifyInternalEP7QObjectP6QEvent'),{onEnter(){if(Process.getCurrentThreadId()!==guiThread){return;}++guiMatches;if(pending===null){return;}const p=pending;pending=null;try{const before=enabled(p);if(before){activate(p,0);}last={status:before?'activated':'disabled-not-activated',pointer:p.toString(),guiMatches};}catch(e){last={status:'error',error:String(e),guiMatches};}}});
rpc.exports={...readonlyExports,setGuiThread(id){guiThread=id;},queue(address){pending=ptr(address);last=null;return 'queued';},status(){return {pending:pending===null?null:pending.toString(),last,guiThread,guiMatches};},classes(hwnd){return classes(find(ptr(hwnd)));},sceneInfo(hwnd){const w=find(ptr(hwnd));const s=sceneFn(w);if(s.isNull()){return {scene:null};}const v=viewportFn(w);return {scene:s.toString(),viewportHwnd:winIdFn(v).toString(),items:itemList(s,false),selected:itemList(s,true)};},map(hwnd,x,y){const w=find(ptr(hwnd));const q=Memory.alloc(16);q.writeDouble(x);q.add(8).writeDouble(y);const p=BigInt('0x'+mapSceneFn(w,q).toString(16));return {x:Number(p&0xffffffffn)|0,y:Number((p>>32n)&0xffffffffn)|0};},modified(hwnd){return prop(find(ptr(hwnd)),'windowModified',true);}};
"""


def main():
    """Keep failed attempts and exact before/after state, touching only this PID."""
    WORK.mkdir(parents=True, exist_ok=False)
    original = (base.VENDOR / "KinSealStudio5.1.exe").read_bytes()
    assert hashlib.sha256(original).hexdigest() == "305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c"
    private = bytearray(original)
    private[0x7c4:0x7c7] = bytes.fromhex("b001c3")
    assert hashlib.sha256(private).hexdigest() == "38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89"
    exe = WORK / "KinSealStudio5.1-private.exe"
    exe.write_bytes(private)
    for name in ("model.dat", "modelInfo.dat", "language.ini"):
        shutil.copy2(base.VENDOR / name, WORK / name)
    source = WORK / "baseline.sm"
    shutil.copy2(SOURCE, source)
    u = ctypes.WinDLL("user32", use_last_error=True)
    u.CreateDesktopW.argtypes = [ctypes.c_wchar_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p]
    u.CreateDesktopW.restype = ctypes.c_void_p
    u.CloseDesktop.argtypes = [ctypes.c_void_p]
    desk = u.CreateDesktopW(DESKTOP, None, None, 0, 0x000F01FF, None)
    if not desk:
        raise OSError(ctypes.get_last_error(), "private desktop unavailable")
    result = {"date": "2026-11-10", "desktop": DESKTOP, "originalSha256": hashlib.sha256(original).hexdigest(), "runtimeSha256": hashlib.sha256(private).hexdigest(), "patch": {"offset": "0x7c4", "before": "5589e5", "after": "b001c3"}, "input": {"path": SOURCE.relative_to(ROOT).as_posix(), "sha256": hashlib.sha256(SOURCE.read_bytes()).hexdigest()}, "systemInput": False, "compileOrUsbOrPlc": False, "steps": [], "snapshots": [], "limitations": ["Selection and dirty-state probes only; not full action behavior or format validation", "Same copied EXE/small config/installed PATH environment as read-only probe", "Read-only/running and prerequisite configuration state not inferred"]}
    process = thread = session = None
    try:
        startup = win32process.STARTUPINFO()
        startup.lpDesktop = "WinSta0\\" + DESKTOP
        startup.dwFlags = win32con.STARTF_USESHOWWINDOW
        startup.wShowWindow = win32con.SW_SHOWNORMAL
        env = os.environ.copy()
        env["PATH"] = str(base.VENDOR) + os.pathsep + env.get("PATH", "")
        env["__COMPAT_LAYER"] = "RunAsInvoker"
        process, thread, pid, _ = win32process.CreateProcess(str(exe), '"%s" "%s"' % (exe, source), None, None, False, win32process.CREATE_UNICODE_ENVIRONMENT, env, str(WORK), startup)
        result["pid"] = pid
        deadline = time.time() + 25
        editor = None
        while time.time() < deadline:
            for top in base.owned_windows(desk, pid):
                children = []
                win32gui.EnumChildWindows(top, lambda h, _: children.append(h), None)
                if any(win32gui.GetWindowText(h) == "menuBar" for h in children):
                    editor = top
                    break
            if editor:
                break
            time.sleep(.25)
        if editor is None:
            raise RuntimeError("editor unavailable within 25s")
        time.sleep(1.5)
        children = []
        win32gui.EnumChildWindows(editor, lambda h, _: children.append(h), None)
        session = base.frida.attach(pid)
        script = session.create_script(base.JS.replace("rpc.exports={read(hwnd)", "const readonlyExports={read(hwnd)") + EXTRA_JS)
        script.load()
        api = script.exports_sync
        menu = next(h for h in children if win32gui.GetWindowText(h) == "menuBar")
        api.set_gui_thread(win32process.GetWindowThreadProcessId(menu)[0])
        views = []
        for h in children:
            try:
                classes = api.classes(h)
                if "QGraphicsView" in classes:
                    views.append({"hwnd": h, "classes": classes, "scene": api.scene_info(h)})
            except Exception as exc:
                result.setdefault("classificationErrors", []).append({"hwnd": h, "error": repr(exc)})
        result["views"] = views
        if not views:
            raise RuntimeError("no QGraphicsView found")
        view = max(views, key=lambda x: len(x["scene"].get("items", [])))
        vh = view["hwnd"]
        viewport = int(view["scene"]["viewportHwnd"], 16)

        def actions():
            rows = []
            for h in children:
                try:
                    info = api.props(h)
                    name = (info or {}).get("objectName", "")
                    if name == "menuBar" or "toolbar" in name.casefold():
                        rows.append({"surface": name, "actions": api.read(h)["actions"]})
                except Exception as exc:
                    rows.append({"hwnd": h, "error": repr(exc)})
            return rows

        def snapshot(label):
            row = {"label": label, "editorTitle": win32gui.GetWindowText(editor), "windowModified": bool(api.modified(editor)), "scene": api.scene_info(vh), "surfaces": actions()}
            row["selectedCount"] = len(row["scene"]["selected"])
            result["snapshots"].append(row)
            return row

        def walk(rows):
            for row in rows:
                yield row
                yield from walk(row.get("children", []))

        def action(name):
            allrows = [r for s in actions() if s.get("actions") for r in walk(s["actions"])]
            row = next(r for r in allrows if r.get("objectName") == name)
            before = snapshot("before:" + name)
            event = {"kind": "normal-QAction-activate", "action": name, "enabledBefore": row["enabled"], "beforeSnapshot": len(result["snapshots"]) - 1, "queued": api.queue(row["pointer"])}
            win32gui.PostMessage(menu, win32con.WM_MOUSEMOVE, 0, 0)
            time.sleep(.45)
            event["activation"] = api.status()
            event["afterSnapshot"] = len(result["snapshots"])
            result["steps"].append(event)
            return snapshot("after:" + name)

        snapshot("initial-opened-baseline")
        multiple = action("action_checkall")
        if multiple["selectedCount"] < 2:
            raise RuntimeError("select-all action did not establish multiple selection")
        item = next((x for x in multiple["scene"]["items"] if x["bounds"]["width"] >= 80 and x["bounds"]["height"] >= 20 and x["bounds"]["y"] > 70), None)
        if item is None:
            raise RuntimeError("no bounded single-object click candidate")
        r = item["bounds"]
        point = api.map(vh, r["x"] + r["width"] / 2, r["y"] + r["height"] / 2)
        if not (-32768 <= point["x"] < 32768 and -32768 <= point["y"] < 32768):
            raise RuntimeError("mapFromScene produced an out-of-range private-window point")
        lp = (point["x"] & 65535) | ((point["y"] & 65535) << 16)
        result["steps"].append({"kind": "private-window-PostMessage-click", "targetHwnd": viewport, "sceneItemCandidate": item, "viewportPoint": point, "messages": ["WM_MOUSEMOVE", "WM_LBUTTONDOWN", "WM_LBUTTONUP"]})
        win32gui.PostMessage(viewport, win32con.WM_MOUSEMOVE, 0, lp)
        win32gui.PostMessage(viewport, win32con.WM_LBUTTONDOWN, win32con.MK_LBUTTON, lp)
        time.sleep(.08)
        win32gui.PostMessage(viewport, win32con.WM_LBUTTONUP, 0, lp)
        time.sleep(.4)
        single = snapshot("after-private-single-click")
        if single["selectedCount"] != 1:
            result["singleSelectionStatus"] = "not-established-by-click"
        else:
            result["singleSelectionStatus"] = "observed-one-selected-item"
        # Send one local key event without modifying the shared Windows clipboard.
        result["steps"].append({"kind": "private-window-PostMessage-key", "targetHwnd": viewport, "key": "VK_RIGHT", "messages": ["WM_KEYDOWN", "WM_KEYUP"]})
        win32gui.PostMessage(viewport, win32con.WM_KEYDOWN, win32con.VK_RIGHT, 1)
        win32gui.PostMessage(viewport, win32con.WM_KEYUP, win32con.VK_RIGHT, 0xc0000001)
        time.sleep(.4)
        dirty = snapshot("after-private-right-key")
        result["dirtyStateStatus"] = "observed-window-modified" if dirty["windowModified"] else "not-established-by-windowModified"
        result["status"] = "bounded-actions-completed"
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
            result["ownedProcessTerminated"] = True
            process.Close()
        if thread:
            thread.Close()
        u.CloseDesktop(desk)
        result["inputSha256After"] = hashlib.sha256(SOURCE.read_bytes()).hexdigest()
        result["workingCopySha256After"] = hashlib.sha256(source.read_bytes()).hexdigest()
        try:
            with winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"Software\KinSealStudio\recent pro files", 0, winreg.KEY_QUERY_VALUE | winreg.KEY_SET_VALUE) as key:
                recent, kind = winreg.QueryValueEx(key, "recentFileList")
                own = str(source).replace("\\", "/").casefold()
                remaining = [v for v in recent if v.replace("\\", "/").casefold() != own]
                if kind == winreg.REG_MULTI_SZ and len(remaining) != len(recent):
                    winreg.SetValueEx(key, "recentFileList", 0, kind, remaining)
                result["ownMruCleanup"] = {"removed": len(recent) - len(remaining), "otherEntriesPreserved": len(remaining)}
        except OSError as exc:
            result["ownMruCleanup"] = {"error": repr(exc)}
        target = OUT / "entry-probe-states.json"
        target.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({"path": str(target), "status": result["status"], "pid": result.get("pid"), "error": result.get("error"), "single": result.get("singleSelectionStatus"), "dirty": result.get("dirtyStateStatus")}, ensure_ascii=False))


if __name__ == "__main__":
    main()
