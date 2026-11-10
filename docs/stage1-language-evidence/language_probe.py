"""#20261110-6 add: Recover editor/startup language through owned GUI-thread actions."""
from pathlib import Path
import argparse
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
import win32api
import win32gui
import win32process

sys.dont_write_bytecode = True
sys.stdout.reconfigure(encoding="utf-8")
sys.path.insert(0, r"G:\01_Professional\Frida\Installed")
import frida

ROOT = Path(r"Z:\Project\vscode_prj_temp\REA")
VENDOR = Path(r"G:\01_Professional\触摸屏\金玺\KinSealStudio_Setup_V5.1\Installed")
OUT = ROOT / "KinSealStudio/docs/stage1-language-evidence"
WORK = ROOT / ".scratch/kinseal-stage1-20261110/native/runtime-r4"
DESKTOP = "KinSealStage1Language20261110"

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
const tabSetFn=new NativeFunction(gui.getExportByName('_ZN10QTabWidget15setCurrentIndexEi'),'void',['pointer','int']);
const groupSetFn=new NativeFunction(gui.getExportByName('_ZN9QGroupBox10setCheckedEb'),'void',['pointer','bool']);
const groupClickedFn=new NativeFunction(gui.getExportByName('_ZN9QGroupBox7clickedEb'),'void',['pointer','bool']);
const tableCurrentFn=new NativeFunction(gui.getExportByName('_ZN12QTableWidget14setCurrentCellEii'),'void',['pointer','int','int']);
const tableRowYFn=new NativeFunction(gui.getExportByName('_ZNK10QTableView19rowViewportPositionEi'),'int',['pointer','int']);
const tableRowHeightFn=new NativeFunction(gui.getExportByName('_ZNK10QTableView9rowHeightEi'),'int',['pointer','int']);
const tableColumnXFn=new NativeFunction(gui.getExportByName('_ZNK10QTableView22columnViewportPositionEi'),'int',['pointer','int']);
const tableColumnWidthFn=new NativeFunction(gui.getExportByName('_ZNK10QTableView11columnWidthEi'),'int',['pointer','int']);
const treeCount=new NativeFunction(gui.getExportByName('_ZNK11QTreeWidget17topLevelItemCountEv'),'int',['pointer']);
const treeTop=new NativeFunction(gui.getExportByName('_ZNK11QTreeWidget12topLevelItemEi'),'pointer',['pointer','int']);
const treeBelow=new NativeFunction(gui.getExportByName('_ZNK11QTreeWidget9itemBelowEPK15QTreeWidgetItem'),'pointer',['pointer','pointer']);
const treeData=new NativeFunction(gui.getExportByName('_ZNK15QTreeWidgetItem4dataEii'),'void',['pointer','pointer','int','int']);
const treeRect=new NativeFunction(gui.getExportByName('_ZNK11QTreeWidget14visualItemRectEPK15QTreeWidgetItem'),'void',['pointer','pointer','pointer']);
function treeRows(w){let item=treeCount(w)?treeTop(w,0):ptr(0);const out=[];while(!item.isNull()&&out.length<256){const v=Memory.alloc(32);treeData(v,item,0,0);const text=Memory.alloc(4);vs(text,v);const rect=Memory.alloc(16);treeRect(rect,w,item);out.push({pointer:item.toString(),text:string(text),rect:[0,4,8,12].map(i=>rect.add(i).readInt())});sd(text);vd(v);item=treeBelow(w,item);}return out;}
let pending=null;let result=null;let guiThread=0;let serial=0;
let saveAsDestination=null;const saveAsPrefills=[];const hookedShow=new Set();
Interceptor.attach(Process.getModuleByName('ole32.dll').getExportByName('CoCreateInstance'),{onEnter(args){this.selected=false;try{const b=Array.from(new Uint8Array(args[0].readByteArray(16))).map(x=>x.toString(16).padStart(2,'0')).join('');this.selected=b==='f3e2b4c021ba73478dba335ec946eb8b';this.ppv=args[4];}catch(e){}},onLeave(ret){if(!this.selected||ret.toInt32()!==0){return;}const o=this.ppv.readPointer();const show=o.readPointer().add(3*4).readPointer();if(hookedShow.has(show.toString())){return;}hookedShow.add(show.toString());Interceptor.attach(show,{onEnter(args){if(saveAsDestination===null){return;}const self=args[0];const f=new NativeFunction(self.readPointer().add(15*4).readPointer(),'int',['pointer','pointer'],'stdcall');const name=saveAsDestination;saveAsDestination=null;try{const hr=f(self,Memory.allocUtf16String(name));saveAsPrefills.push({destination:name,hresult:hr,thread:Process.getCurrentThreadId(),expectedGuiThread:guiThread,method:'IFileDialog::SetFileName immediately before native Show'});}catch(e){saveAsPrefills.push({destination:name,error:String(e)});}}});}});
function string(s){const d=s.readPointer();const n=d.add(8).readInt();if(n<0||n>200000){throw Error('QString bound');}return n?d.add(12).readPointer().readUtf16String(n):'';}
function prop(w,n,b=false){const v=Memory.alloc(32);propFn(v,w,Memory.allocUtf8String(n));if(b){const a=vb(v);vd(v);return a;}const s=Memory.alloc(4);vs(s,v);const a=string(s);sd(s);vd(v);return a;}
function kind(w){const f=new NativeFunction(w.readPointer().readPointer(),'pointer',['pointer']);return f(w).add(4).readPointer().readCString();}
function props(w){const r={pointer:w.toString(),kind:kind(w),objectName:prop(w,'objectName'),text:prop(w,'text'),title:prop(w,'windowTitle'),enabled:prop(w,'enabled',true),visible:prop(w,'visible',true)};for(const n of ['minimum','maximum','value','currentIndex','currentText','count','rowCount','columnCount','maxLength','checked','checkable','decimals','singleStep','suffix','prefix']){const v=prop(w,n);if(v!==''){r[n]=v;}}if(r.kind==='QComboBox'){r.items=[];for(let i=0;i<Math.min(Number(r.count),256);++i){const s=Memory.alloc(4);comboTextFn(s,w,i);r.items.push(string(s));sd(s);}}return r;}
function actions(w,depth=0){if(depth>6){return [];}const l=Memory.alloc(4);actionsFn(l,w);const d=l.readPointer();const b=d.add(8).readInt(),e=d.add(12).readInt();if(b<0||e<b||e-b>500){throw Error('QList bound');}const out=[];for(let i=b;i<e;++i){const a=d.add(20+i*4).readPointer();const s=Memory.alloc(4);textFn(s,a);const m=menuFn(a);out.push({pointer:a.toString(),objectName:prop(a,'objectName'),text:string(s),enabled:enabledFn(a),children:m.isNull()?[]:actions(m,depth+1)});sd(s);}return out;}
function invoke(w,slot){return invokeFn(w,Memory.allocUtf8String(slot),2,...Array(22).fill(ptr(0)));}
function execute(p){if(p.op==='batch'){return p.hwnds.map(h=>{try{return {hwnd:h,qt:props(find(ptr(h)))};}catch(e){return {hwnd:h,error:String(e)};}});}if(p.op==='menus'){return actions(find(ptr(p.hwnd)));}if(p.op==='invoke'){const w=p.pointer?ptr(p.pointer):find(ptr(p.hwnd));return {queuedMetaInvoke:invoke(w,p.slot),slot:p.slot,pointer:w.toString()};}const w=find(ptr(p.hwnd));if(p.op==='tree'){return treeRows(w);}if(p.op==='table-cell'){return {x:tableColumnXFn(w,p.column)+Math.floor(tableColumnWidthFn(w,p.column)/2),y:tableRowYFn(w,p.row)+Math.floor(tableRowHeightFn(w,p.row)/2),row:p.row,column:p.column};}if(p.op==='table-open'){tableCurrentFn(w,p.row,p.column);const a=Memory.alloc(4);a.writeInt(p.row);const b=Memory.alloc(4);b.writeInt(p.column);const ty=Memory.allocUtf8String('int');return {queuedMetaInvoke:invokeFn(w,Memory.allocUtf8String('cellDoubleClicked'),2,ptr(0),ptr(0),ty,a,ty,b,...Array(16).fill(ptr(0))),row:p.row,column:p.column,selection:'normal control-list current cell and queued cellDoubleClicked signal'};}if(p.op==='text'){const s=Memory.alloc(4);sc(s,Memory.allocUtf16String(p.value),p.value.length);setTextFn(w,s);sd(s);return props(w);}if(p.op==='spin'){spinFn(w,p.value);return props(w);}if(p.op==='tab'){tabSetFn(w,p.value);return props(w);}if(p.op==='group'){groupSetFn(w,p.value?1:0);groupClickedFn(w,p.value?1:0);return props(w);}if(p.op==='combo'){comboSetFn(w,p.value);comboSignalFn(w,p.value);return props(w);}throw Error('unsupported operation');}
for(const symbol of ['DispatchMessageW','DispatchMessageA']){Interceptor.attach(Process.getModuleByName('user32.dll').getExportByName(symbol),{onEnter(){if(Process.getCurrentThreadId()!==guiThread||pending===null){return;}const p=pending;pending=null;try{result={serial:p.serial,status:'completed',value:execute(p),guiThread:Process.getCurrentThreadId(),dispatchHook:symbol};}catch(e){result={serial:p.serial,status:'error',error:String(e)};}}});}
rpc.exports={setThread(t){guiThread=t;},armSaveAs(value){saveAsDestination=value;return 'armed-standard-native-SetFileName';},saveAsPrefills(){return saveAsPrefills;},queue(p){if(pending!==null){throw Error('operation pending');}p.serial=++serial;pending=p;result=null;return serial;},poll(){return result;}};
"""


def windows(desktop, pid):
    """Enumerate only this task's target process on its private desktop."""
    found = []
    win32gui.EnumDesktopWindows(pywintypes.HANDLE(desktop), lambda h, _: found.append(h) if win32process.GetWindowThreadProcessId(h)[1] == pid else None, None)
    return found


def main():
    """Create native samples through actual actions and preserve original inputs."""
    global WORK
    parser = argparse.ArgumentParser()
    parser.add_argument('--run', default='r4')
    parser.add_argument('--input', type=Path, default=ROOT / 'KinSealStudio/samples/native-device-test/CodexProbeTest.sm')
    parser.add_argument('--no-clear', action='store_true')
    parser.add_argument('--page-name')
    parser.add_argument('--page-number', type=int)
    parser.add_argument('--output-name')
    parser.add_argument('--reopen-only', action='store_true')
    parser.add_argument('--cycle-pages', type=int, default=0)
    parser.add_argument('--bit-variant', choices=['operations','notice','external','monitor-disabled','static-level','roundtrip'])
    parser.add_argument('--save-as-output')
    parser.add_argument('--inspect-control-list', action='store_true')
    parser.add_argument('--control-row', type=int)
    parser.add_argument('--language-index', type=int)
    parser.add_argument('--inspect-action')
    parser.add_argument('--menus-only', action='store_true')
    parser.add_argument('--tree-only', action='store_true')
    parser.add_argument('--tree-open')
    parser.add_argument('--settings-language-index', type=int)
    parser.add_argument('--language-count', type=int, default=2, choices=(2,3))
    parser.add_argument('--compile-native', action='store_true')
    args = parser.parse_args()
    WORK = ROOT / '.scratch/kinseal-stage1-language-20261110/native' / ('runtime-' + args.run)
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
    shutil.copy2(args.input, source)
    result = {"date": "2026-11-10", "run":args.run, "originalSha256": hashlib.sha256(original).hexdigest(), "runtimeSha256": hashlib.sha256(private).hexdigest(), "patch": {"offset": "0x7c4", "before": "5589e5", "after": "b001c3"}, "desktop": DESKTOP, "input": {"path": str(args.input), "sha256": hashlib.sha256(source.read_bytes()).hexdigest()}, "steps": [], "snapshots": [], "systemInput": False, "usbOrPlc": False, "qtOperationsGuiThreadOnly": True}
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
            win32gui.PostMessage(menu, win32con.WM_NULL, 0, 0)
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
                if not win32gui.IsWindowVisible(top) or win32gui.GetClassName(top) not in ('QWidget','#32770'):
                    continue
                cs = [top]
                win32gui.EnumChildWindows(top, lambda h, _: cs.append(h), None)
                rows.append({"hwnd": top, "title": win32gui.GetWindowText(top), "winClass":win32gui.GetClassName(top), "widgets": op({"op": "batch", "hwnds": cs}), "nativeChildren":[{'hwnd':h,'text':win32gui.GetWindowText(h),'class':win32gui.GetClassName(h),'controlId':win32gui.GetDlgCtrlID(h)} for h in cs]})
            result["snapshots"].append({"label": label, "windows": rows})
            (OUT / "native-probe-checkpoint.json").write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8")
            print(json.dumps({"snapshot": label, "windows": [r["title"] for r in rows]}, ensure_ascii=False), flush=True)
            return rows

        def action(name):
            names = []
            batch = op({"op": "batch", "hwnds": children})
            for item in batch:
                q = item.get("qt", {})
                if q.get("objectName") == "menuBar" or "toolbar" in q.get("objectName", "").casefold():
                    def walk(rows):
                        for row in rows:
                            yield row
                            yield from walk(row.get("children", []))
                    names.extend(walk(op({"op": "menus", "hwnd": item["hwnd"]})))
            row = next(r for r in names if r["objectName"] == name)
            assert row["enabled"], (name, row)
            r = op({"op": "invoke", "pointer": row["pointer"], "slot": "trigger"})
            assert r["queuedMetaInvoke"]
            time.sleep(.35)

        dump("initial")
        menu_inventory = op({"op": "menus", "hwnd": menu})
        result['menuInventory'] = menu_inventory
        if args.compile_native:
            if args.language_index is not None:
                widgets = op({'op':'batch','hwnds':children})
                language = next(w for w in widgets if w.get('qt',{}).get('kind') == 'QComboBox' and w['qt'].get('items',[])[:2] == ['中文','English'])
                op({'op':'combo','hwnd':language['hwnd'],'value':args.language_index})
                dump('native-compiler-editor-language-selected')
            action('action_USBdownload')
            rows = dump('native-compile-dialog')
            dialog = next(r for r in rows if any(w.get('qt',{}).get('objectName') == 'edit_path' for w in r['widgets']))
            def download_widget(name):
                return next(w['hwnd'] for w in dialog['widgets'] if w.get('qt',{}).get('objectName') == name)
            export_directory = WORK / 'disk-export'
            export_directory.mkdir()
            op({'op':'invoke','hwnd':download_widget('radioUdisk'),'slot':'click'})
            op({'op':'text','hwnd':download_widget('edit_path'),'value':str(export_directory)})
            dump('native-disk-export-selected')
            op({'op':'invoke','hwnd':download_widget('m_downloadbtn'),'slot':'click'})
            deadline = time.time() + 50
            while time.time() < deadline and not (export_directory / 'compile/SysPara.dat').is_file():
                time.sleep(.2)
            time.sleep(1)
            dump('native-disk-export-result')
            result['compiledFiles'] = [dict(path=str(p.relative_to(WORK)),bytes=p.stat().st_size,sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in export_directory.rglob('*') if p.is_file()]
            result['compileWorkFiles'] = [str(p.relative_to(WORK)) for p in WORK.rglob('*') if p.is_file()]
            system = next(iter(export_directory.rglob('SysPara.dat')), None)
            if system:
                result['sysParaLanguageIndex'] = int.from_bytes(system.read_bytes()[2172:2174],'little')
                target = OUT / ('compiled-' + args.run)
                shutil.copytree(system.parent, target)
                result['compiledOutput'] = str(target)
                result['status'] = 'native-offline-compiled'
            else:
                result['status'] = 'native-offline-compile-no-system-output'
            return
        if args.tree_only or args.tree_open or args.settings_language_index is not None:
            widgets = op({'op':'batch', 'hwnds':children})
            tree = next(w for w in widgets if w.get('qt',{}).get('kind') == 'QTreeWidget')
            op({'op':'invoke','hwnd':tree['hwnd'],'slot':'expandAll'})
            time.sleep(.3)
            tree_rows = op({'op':'tree','hwnd':tree['hwnd']})
            result['treeRows'] = tree_rows
            if args.tree_open or args.settings_language_index is not None:
                entry = args.tree_open or 'ÓïÑÔÉèÖÃ'
                item = next(r for r in tree_rows if r['text'] == entry)
                viewport = next(h for h in children if win32gui.GetParent(h) == tree['hwnd'] and win32gui.GetWindowText(h) == 'qt_scrollarea_viewport')
                left,top,right,bottom = item['rect']
                point = win32api.MAKELONG((left+right)//2,(top+bottom)//2)
                for message,flags in ((win32con.WM_LBUTTONDOWN,win32con.MK_LBUTTON),(win32con.WM_LBUTTONUP,0),(win32con.WM_LBUTTONDBLCLK,win32con.MK_LBUTTON),(win32con.WM_LBUTTONUP,0)):
                    win32gui.PostMessage(viewport,message,flags,point)
                time.sleep(.5)
                dump('native-tree-entry-opened')
            if args.settings_language_index is None:
                result['status'] = 'tree-inspected'
                return
            rows = result['snapshots'][-1]['windows']
            dialog = next(r for r in rows if r['title'] == '语言设置')
            def language_widget(name):
                return next(w['hwnd'] for w in dialog['widgets'] if w.get('qt',{}).get('objectName') == name)
            op({'op':'combo','hwnd':language_widget('lanComboBox'),'value':args.language_count-1})
            op({'op':'text','hwnd':language_widget('lineEdit_1'),'value':'中文'})
            op({'op':'text','hwnd':language_widget('lineEdit_2'),'value':'English'})
            if args.language_count == 3:
                op({'op':'text','hwnd':language_widget('lineEdit_3'),'value':'Deutsch'})
            op({'op':'combo','hwnd':language_widget('curLanComboBox'),'value':args.settings_language_index})
            dump('native-language-settings-edited')
            confirm = next(w['hwnd'] for w in dialog['widgets'] if w.get('qt',{}).get('kind') == 'QPushButton' and w['qt'].get('text') in ('确定','È·¶¨'))
            op({'op':'invoke','hwnd':confirm,'slot':'click'})
            time.sleep(.4)
            dump('native-language-settings-accepted')
        if args.menus_only:
            result['status'] = 'menus-inspected'
            return
        if args.inspect_action:
            action(args.inspect_action)
            dump('selected-action-dialog')
            result['status'] = 'action-dialog-inspected'
            return
        if args.language_index is not None:
            widgets = op({'op':'batch','hwnds':children})
            language = next(w for w in widgets if w.get('qt',{}).get('kind') == 'QComboBox' and w['qt'].get('items',[])[:2] == ['中文','English'])
            op({'op':'combo','hwnd':language['hwnd'],'value':args.language_index})
            dump('actual-editor-language-selected')
        if args.inspect_control_list or args.control_row is not None:
            action('action_items_table')
            time.sleep(.5)
            rows = dump('control-list-opened')
            if args.inspect_control_list:
                result['status'] = 'native-control-list-inspected'
                return
            table = next(w for r in rows for w in r['widgets'] if w.get('qt',{}).get('kind') == 'QTableWidget' and w['qt']['visible'])
            assert 0 <= args.control_row < int(table['qt']['rowCount'])
            cell = op({'op':'table-cell','hwnd':table['hwnd'],'row':args.control_row,'column':0})
            viewport = next(w['hwnd'] for r in rows for w in r['widgets'] if w.get('qt',{}).get('objectName') == 'qt_scrollarea_viewport' and win32gui.GetParent(w['hwnd']) == table['hwnd'])
            lparam = win32api.MAKELONG(cell['x'],cell['y'])
            for message,flags in ((win32con.WM_LBUTTONDOWN,win32con.MK_LBUTTON),(win32con.WM_LBUTTONUP,0),(win32con.WM_LBUTTONDBLCLK,win32con.MK_LBUTTON),(win32con.WM_LBUTTONUP,0)):
                win32gui.PostMessage(viewport,message,flags,lparam)
            result['steps'].append({'request':{'op':'native-private-viewport-doubleclick','hwnd':viewport,'cell':cell},'source':'background WM messages to owned control-list viewport; no global input'})
            time.sleep(.6)
            rows = dump('control-list-single-row-opened')
            if not any(w.get('qt',{}).get('objectName') == 'comboBox_operateType' for r in rows for w in r['widgets']):
                list_dialog = next(r for r in rows if r['hwnd'] != editor and any(w.get('qt',{}).get('kind') == 'QTableWidget' for w in r['widgets']))
                list_confirm = next(w['hwnd'] for w in list_dialog['widgets'] if w.get('qt',{}).get('kind') == 'QPushButton' and w['qt'].get('text') in ('确定','È·¶¨'))
                op({'op':'invoke','hwnd':list_confirm,'slot':'click'})
                time.sleep(.4)
                action('action_openpropertiset')
                time.sleep(.5)
                rows = dump('control-list-selection-properties')
            dialog = next(r for r in rows if any(w.get('qt',{}).get('objectName') == 'comboBox_operateType' for w in r['widgets']))
            operation = next(w for w in dialog['widgets'] if w.get('qt',{}).get('objectName') == 'comboBox_operateType')
            original_index = int(operation['qt']['currentIndex'])
            op({'op':'combo','hwnd':operation['hwnd'],'value':(original_index+1)%4})
            op({'op':'combo','hwnd':operation['hwnd'],'value':original_index})
            dump('control-list-bit-operation-restored')
            confirm = next(w['hwnd'] for w in dialog['widgets'] if w.get('qt',{}).get('kind') == 'QPushButton' and w['qt'].get('text') in ('确定','È·¶¨'))
            op({'op':'invoke','hwnd':confirm,'slot':'click'})
            time.sleep(.5)
            dump('control-list-bit-accepted')
            action('action_save')
            assert args.output_name
            destination = OUT / args.output_name
            assert not destination.exists()
            shutil.copy2(source,destination)
            result['output'] = {'path':destination.name,'sha256':hashlib.sha256(destination.read_bytes()).hexdigest(),'origin':'Lite export then native single control-list bit edit/restore and normal save'}
            result['status'] = 'native-mixed-source-resaved'
            return
        if args.save_as_output:
            destination=(OUT/args.save_as_output).resolve()
            assert destination.parent==OUT.resolve() and not destination.exists()
            result['saveAsPrefillArmed']=api.arm_save_as(str(destination))
            action('action_save_as')
            native_wait=time.time()+6
            while time.time()<native_wait:
                if any(win32gui.IsWindowVisible(h) and win32gui.GetClassName(h)=='#32770' for h in windows(desk,pid)):
                    break
                time.sleep(.1)
            time.sleep(.3)
            native_dialogs=[h for h in windows(desk,pid) if win32gui.IsWindowVisible(h) and win32gui.GetClassName(h)=='#32770']
            if native_dialogs:
                dialog_hwnd=native_dialogs[0]
                cs=[]
                win32gui.EnumChildWindows(dialog_hwnd,lambda h,_:cs.append(h),None)
                native_rows=[{'hwnd':h,'text':win32gui.GetWindowText(h),'class':win32gui.GetClassName(h),'controlId':win32gui.GetDlgCtrlID(h),'parent':win32gui.GetParent(h)} for h in cs]
                result['nativeSaveAsDialog']={'hwnd':dialog_hwnd,'title':win32gui.GetWindowText(dialog_hwnd),'thread':win32process.GetWindowThreadProcessId(dialog_hwnd)[0],'children':native_rows}
                result['nativeSaveAsPrefills']=api.save_as_prefills()
                (OUT/('native-saveas-'+args.run+'-controls.json')).write_text(json.dumps(result['nativeSaveAsDialog'],ensure_ascii=False,indent=2),encoding='utf-8')
                edits=[r for r in native_rows if r['class']=='Edit' and ('NativePages' in r['text'] or r['controlId']==1148)]
                if not edits:
                    edits=[r for r in native_rows if r['class']=='Edit' and win32gui.GetDlgCtrlID(r['parent'])==1148]
                if not edits:
                    edits=[r for r in native_rows if r['class']=='Edit' and r['controlId']==1001]
                if len(edits)!=1:
                    raise RuntimeError('Native Save-as filename control not uniquely identified; controls retained')
                destination=(OUT/args.save_as_output).resolve()
                assert destination.parent==OUT.resolve() and not destination.exists()
                if not any(x.get('hresult')==0 and x.get('destination')==str(destination) for x in result['nativeSaveAsPrefills']):
                    raise RuntimeError('Native SetFileName hook did not confirm prefill; no file dialog accepted')
                save=next(r for r in native_rows if r['class']=='Button' and r['controlId']==1)
                result['steps'].append({'request':{'op':'native-file-dialog-WM_SETTEXT-and-IDOK-command','filenameHwnd':edits[0]['hwnd'],'value':str(destination),'filenameAfterWM_SETTEXT':win32gui.GetWindowText(edits[0]['hwnd']),'saveButtonHwnd':save['hwnd'],'message':'WM_COMMAND/IDOK/BN_CLICKED'},'guiThread':win32process.GetWindowThreadProcessId(save['hwnd'])[0],'source':'actual native common Save As dialog controls'})
                win32gui.PostMessage(dialog_hwnd,win32con.WM_COMMAND,1,save['hwnd'])
                limit=time.time()+5
                while time.time()<limit and not destination.is_file():
                    time.sleep(.1)
                result['nativeSaveAsPostState']={'destinationExists':destination.is_file(),'dialogStillVisible':bool(win32gui.IsWindow(dialog_hwnd) and win32gui.IsWindowVisible(dialog_hwnd)),'visibleOwnedWindows':[{'hwnd':h,'title':win32gui.GetWindowText(h),'class':win32gui.GetClassName(h)} for h in windows(desk,pid) if win32gui.IsWindowVisible(h)]}
                assert destination.is_file()
                result['output']={'path':destination.relative_to(ROOT).as_posix(),'sha256':hashlib.sha256(destination.read_bytes()).hexdigest(),'length':destination.stat().st_size,'origin':'Lite-export then actual native UI Save As; not native creation from blank'}
                result['status']='native-save-as-written'
                return
            rows = dump('native-save-as-dialog')
            dialog = next(r for r in rows if r['hwnd'] != editor)
            fields = [w for w in dialog['widgets'] if w.get('qt',{}).get('objectName') == 'fileNameEdit']
            if not fields:
                raise RuntimeError('Save-as filename field requires retained native control investigation')
            destination = (OUT / args.save_as_output).resolve()
            assert destination.parent == OUT.resolve()
            assert not destination.exists()
            op({'op':'text','hwnd':fields[0]['hwnd'],'value':str(destination)})
            dump('native-save-as-filename-set')
            buttons = [w for w in dialog['widgets'] if w.get('qt',{}).get('kind') == 'QPushButton']
            save = next(w for w in buttons if any(s in w['qt'].get('text','') for s in ('保存','±£´æ','Save')))
            op({'op':'invoke','hwnd':save['hwnd'],'slot':'click'})
            limit = time.time()+5
            while time.time()<limit and not destination.is_file():
                win32gui.PostMessage(menu,win32con.WM_NULL,0,0)
                time.sleep(.1)
            assert destination.is_file()
            result['output']={'path':destination.relative_to(ROOT).as_posix(),'sha256':hashlib.sha256(destination.read_bytes()).hexdigest(),'length':destination.stat().st_size,'origin':'Lite-export then actual native UI Save As; not native creation from blank'}
            dump('native-save-as-completed')
            result['status']='native-save-as-written'
            return
        if args.bit_variant:
            indices = range(4) if args.bit_variant == 'operations' else range(3) if args.bit_variant == 'external' else range(1)
            for index in indices:
                action('action_checkall')
                action('action_openpropertiset')
                limit = time.time() + 4
                while time.time() < limit:
                    if any(h != editor and win32gui.IsWindowVisible(h) and win32gui.GetClassName(h) == 'QWidget' for h in windows(desk,pid)):
                        break
                    win32gui.PostMessage(menu,win32con.WM_NULL,0,0)
                    time.sleep(.1)
                rows = dump('oracle-properties-initial-' + str(index))
                dialog = next(r for r in rows if r['hwnd'] != editor and any(w.get('qt',{}).get('kind') == 'QPushButton' for w in r['widgets']))
                def dw(name):
                    matches = [w for w in dialog['widgets'] if w.get('qt',{}).get('objectName') == name]
                    return next((w['hwnd'] for w in matches if w['qt']['visible']),matches[0]['hwnd'])
                if args.bit_variant in ('operations','roundtrip'):
                    original_index=int(next(w['qt']['currentIndex'] for w in dialog['widgets'] if w.get('qt',{}).get('objectName')=='comboBox_operateType'))
                    if args.bit_variant=='roundtrip':
                        op({'op':'combo','hwnd':dw('comboBox_operateType'),'value':(original_index+1)%4})
                        op({'op':'combo','hwnd':dw('comboBox_operateType'),'value':original_index})
                        filename=args.output_name
                        assert filename
                    else:
                        op({'op':'combo','hwnd':dw('comboBox_operateType'),'value':index})
                        filename = 'bit-operation-' + ['set','reset','jog','toggle'][index] + '-native-ui.sm'
                else:
                    tabs = next(w['hwnd'] for w in dialog['widgets'] if w.get('qt',{}).get('kind') == 'QTabWidget')
                    tab_count = int(next(w['qt']['count'] for w in dialog['widgets'] if w['hwnd'] == tabs))
                    op({'op':'tab','hwnd':tabs,'value':1 if args.bit_variant == 'monitor-disabled' else tab_count-1})
                    dialog = next(r for r in dump('oracle-tab-' + str(index)) if r['hwnd'] == dialog['hwnd'])
                    if args.bit_variant == 'notice':
                        op({'op':'group','hwnd':dw('groupBox_Notice'),'value':True})
                        dump('notice-main-enabled')
                        op({'op':'invoke','hwnd':dw('checkBox_NoteW'),'slot':'click'})
                        op({'op':'text','hwnd':dw('edit_noticeAddrW'),'value':'LW0'})
                        op({'op':'text','hwnd':dw('lineEdit_ValueNw'),'value':'7'})
                        filename = 'bit-notice-word7-native-ui.sm'
                    elif args.bit_variant == 'monitor-disabled':
                        op({'op':'group','hwnd':dw('groupBox'),'value':False})
                        filename = 'bit-monitor-disabled-native-ui.sm'
                    elif args.bit_variant == 'static-level':
                        op({'op':'group','hwnd':dw('GroupV'),'value':True})
                        op({'op':'group','hwnd':dw('groupBox_showlevel'),'value':True})
                        op({'op':'combo','hwnd':dw('comboBox_Show_UserLevel'),'value':2})
                        filename = 'static-visible-level3-native-ui.sm'
                    else:
                        op({'op':'group','hwnd':dw('groupBox_Touch'),'value':True})
                        op({'op':'group','hwnd':dw('groupBox_TouchByW'),'value':True})
                        op({'op':'group','hwnd':dw('groupBox_touchlevel'),'value':True})
                        op({'op':'combo','hwnd':dw('comboBox_Touch_UserLevel'),'value':0})
                        op({'op':'combo','hwnd':dw('comboBox_dataT'),'value':[1,3,4][index]})
                        op({'op':'text','hwnd':dw('edit_ctlAddrwT'),'value':'[Modbus_RTU]4x0'})
                        op({'op':'text','hwnd':dw('lineEdit_valueT'),'value':'7'})
                        filename = 'bit-external-' + ['U32','I32','F32'][index] + '-native-ui.sm'
                dump('oracle-properties-edited-' + str(index))
                confirm = next(w['hwnd'] for w in dialog['widgets'] if w.get('qt',{}).get('kind') == 'QPushButton' and w.get('qt',{}).get('text') in ('确定','È·¶¨'))
                op({'op':'invoke','hwnd':confirm,'slot':'click'})
                time.sleep(.4)
                dump('oracle-properties-accepted-' + str(index))
                action('action_save')
                shutil.copy2(source,OUT/filename)
                result.setdefault('oracleOutputs',[]).append({'path':filename,'sha256':hashlib.sha256((OUT/filename).read_bytes()).hexdigest(),'variant':args.bit_variant,'index':index})
            result['status'] = 'native-oracles-saved'
            return
        if args.reopen_only:
            for index in range(args.cycle_pages):
                action('action_nextscreen')
                dump('reopened-next-page-' + str(index + 1))
            result['status'] = 'native-source-reopened-and-pages-cycled'
            result['workingCopySha256After'] = hashlib.sha256(source.read_bytes()).hexdigest()
            return
        if not args.no_clear:
            action("action_checkall")
            action("action_delete")
            action("action_save")
            shutil.copy2(source, OUT / "A-empty-one-page.sm")
            dump("A-empty-one-page-saved")
        action("action_new_screen")
        dialog_deadline = time.time() + 4
        while time.time() < dialog_deadline:
            if any(h != editor and win32gui.IsWindowVisible(h) and win32gui.GetClassName(h) == 'QWidget' for h in windows(desk, pid)):
                break
            win32gui.PostMessage(menu, win32con.WM_NULL, 0, 0)
            time.sleep(.1)
        rows = dump("new-page-dialog")
        dialog = next(r for r in rows if r['hwnd'] != editor and any(w.get('qt',{}).get('objectName') == 'newscreen_name_lineedit' for w in r['widgets']))
        def dialog_widget(name):
            return next(w['hwnd'] for w in dialog['widgets'] if w.get('qt',{}).get('objectName') == name)
        if args.page_name:
            op({'op':'text','hwnd':dialog_widget('newscreen_name_lineedit'),'value':args.page_name})
        if args.page_number is not None:
            op({'op':'spin','hwnd':dialog_widget('spinBox'),'value':args.page_number})
        dump('new-page-edited')
        op({'op':'invoke','hwnd':dialog_widget('pushButton_sure'),'slot':'click'})
        time.sleep(.5)
        dump('after-page-dialog-accepted')
        action('action_save')
        target = OUT / (args.output_name if args.output_name else ('B-two-empty-pages.sm' if not args.no_clear else 'C-third-page-custom.sm'))
        shutil.copy2(source, target)
        result['output']={'path':target.relative_to(ROOT).as_posix(),'sha256':hashlib.sha256(target.read_bytes()).hexdigest(),'length':target.stat().st_size}
        dump('after-native-save')
        result["status"] = "page-created-and-native-source-saved"
    except Exception as exc:
        result["status"] = "partial-or-error"
        result["error"] = repr(exc)
    finally:
        if process and win32process.GetExitCodeProcess(process) == 259:
            win32process.TerminateProcess(process, 0)
        if session:
            try:
                session.detach()
            except Exception as exc:
                result["detachError"] = repr(exc)
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
        (OUT / ('native-probe-' + args.run + '.json')).write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({"status": result["status"], "pid": result.get("pid"), "error": result.get("error"), "evidence": str(OUT / "native-probe.json")}, ensure_ascii=False))


if __name__ == "__main__":
    main()
