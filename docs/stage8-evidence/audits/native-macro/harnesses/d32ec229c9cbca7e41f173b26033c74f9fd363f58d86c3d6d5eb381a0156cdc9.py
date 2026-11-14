"""Stage 8 owned private-desktop UI observations; no compiler/device execution."""
#20261114-7 add
from pathlib import Path
import argparse, ast, ctypes, gzip, hashlib, json, os, shutil, sys, time, winreg
import pywintypes, win32api, win32con, win32event, win32gui, win32process
sys.dont_write_bytecode = True
sys.stdout.reconfigure(encoding="utf-8")
sys.path.insert(0, r"G:\01_Professional\Frida\Installed")
import frida

ROOT = Path(r"Z:\Project\vscode_prj_temp\REA")
BASE = ROOT / ".scratch/kinseal-stage8-20261114/native-macro"
OLD = ROOT / ".scratch/kinseal-stage7-20261114/native-startup"
WT = ROOT / ".scratch/kinseal-stage4-20261112/worktree"
VENDOR = Path(r"G:\01_Professional\触摸屏\金玺\KinSealStudio_Setup_V5.1\Installed")
JS_SOURCE = WT / "docs/stage4-evidence/ascii/physical/ascii-lifecycle-r1/native_screen_probe.py"

def sha(path):
    """Hash the exact bytes used by a run."""
    return hashlib.sha256(path.read_bytes()).hexdigest()

def write(path, value):
    """Save a bounded evidence record without changing source inputs."""
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

def constant(path, name):
    """Read one literal constant without executing an old investigation harness."""
    tree = ast.parse(path.read_text(encoding="utf-8"))
    return ast.literal_eval(next(n.value for n in tree.body if isinstance(n, ast.Assign) and any(isinstance(t, ast.Name) and t.id == name for t in n.targets)))

def normalized(value):
    """Repair historical Qt local-8-bit mojibake in a separate display projection."""
    if isinstance(value, str):
        try:
            return value.encode("latin1").decode("gbk")
        except (UnicodeError, LookupError):
            return value
    if isinstance(value, list):
        return [normalized(x) for x in value]
    if isinstance(value, dict):
        return {k: normalized(v) for k,v in value.items()}
    return value

def main():
    """Perform explicitly supplied GUI actions on one task-owned private process."""
    parser=argparse.ArgumentParser()
    parser.add_argument("--run", required=True)
    parser.add_argument("--commands", required=True)
    parser.add_argument("--source")
    args=parser.parse_args()
    commands=json.loads(Path(args.commands).read_text(encoding="utf-8-sig"))
    out=BASE/args.run;out.mkdir()
    work=out/"runtime";work.mkdir()
    temp=out/"temp";temp.mkdir()
    os.environ.update(TEMP=str(temp),TMP=str(temp),TMPDIR=str(temp))
    env=dict(os.environ, TEMP=str(temp),TMP=str(temp),TMPDIR=str(temp),PYTHONDONTWRITEBYTECODE="1")
    record={"startedEpoch":time.time(),"status":"preparing","privateDesktopOnly":True,"globalInput":False,"hardwareTransferExecuted":False,"sdkExecuted":False,"childLaunchPolicy":"All native child launches blocked", "steps":[],"snapshots":[]}
    process=thread=session=script=desk=None
    try:
        shared=OLD/"shared/KinSealStudio5.1-private.exe"
        assert sha(shared)=="38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89"
        exe=work/shared.name;os.link(shared,exe)
        archived=OLD/"preserved-evidence/configuration"
        for name in ["model.dat","modelInfo.dat","language.ini"]:
            source=next(archived.glob("*-"+name));shutil.copy2(source,work/name)
        old_runtime=OLD/"offline-cancel-r1/runtime"
        for folder in ["codecs","imageformats","iconengines","PlcDll"]:
            (work/folder).mkdir()
            for source in (old_runtime/folder).glob("*.dll"):
                os.link(source,work/folder/source.name)
        blocker_manifest=[]
        for name,digest in [("DownT.dll","83b54e09772f02b41ec40297c085a024773a7f85d39ee0491afe915eb5bb916e"),("libusb0.dll","262f5cd9360a36486789a84521a57232858a9a9157fb6bb556550279926ce8db")]:
            origin=OLD/"preserved-evidence/hardware-blockers"/(digest+".dll.gz")
            destination=work/name;destination.write_bytes(gzip.decompress(origin.read_bytes()));assert sha(destination)==digest
            blocker_manifest.append({"path":str(destination),"sha256":digest,"archive":str(origin)})
        record["hardwareBlockers"]=blocker_manifest
        source=Path(args.source) if args.source else WT/"docs/stage1-evidence/native-analysis/macro-bound-native-resaved.sm"
        if args.source:assert source.resolve().is_relative_to(BASE.resolve()),"Reopen only an owned stage 8 sample"
        target=work/"NativeInput.sm";shutil.copy2(source,target)
        record["input"]={"origin":str(source),"path":str(target),"beforeSha256":sha(target),"beforeBytes":target.stat().st_size}
        js=constant(JS_SOURCE,"JS")
        guard=constant(OLD/"startup_probe.py","MAIN_JS").replace("__ROOT__",json.dumps(str(work))).replace("__ALLOW_OFFLINE__","false")
        js=js.replace("rpc.exports={", "rpc.exports={events(){return {events:trace,overflow};},loadGuardedModule(path){return Module.load(path).name;},moduleIdentities(){return Process.enumerateModules().filter(m=>/^(libusb0|DownT)\\.dll$/i.test(m.name)).map(m=>({name:m.name,path:m.path}));},",1)
        js=js.replace("function execute(p){", "function execute(p){if(p.op==='tree-focus'){return macroFocus(p.hwnd,p.item);}if(p.op==='sci-set'){return macroSciSet(p.hwnd,p.value);}if(p.op==='sci-read'){return macroSci(p.hwnd);}if(p.op==='table-headers'){return macroHeaders(p.hwnd);}if(p.op==='tab-titles'){return macroTabs(p.hwnd);}if(p.op==='module-exports'){return Process.enumerateModules().filter(m=>/sci/i.test(m.name)).map(m=>({name:m.name,exports:m.enumerateExports().filter(e=>/text|Text|length|Length|SendScintilla/i.test(e.name)).map(e=>({name:e.name,address:e.address.toString()}))}));}if(p.op==='meta-all'){return macroMeta(p.hwnd);}if(p.op==='list'){return macroList(p.hwnd);}",1)
        js+=guard+r'''
function macroMeta(hwnd){const object=find(ptr(hwnd));const meta=new NativeFunction(object.readPointer().readPointer(),'pointer',['pointer'])(object);const count=new NativeFunction(core.getExportByName('_ZNK11QMetaObject11methodCountEv'),'int',['pointer'])(meta);const method=new NativeFunction(core.getExportByName('_ZNK11QMetaObject6methodEi'),'uint64',['pointer','int']);const signature=new NativeFunction(core.getExportByName('_ZNK11QMetaMethod9signatureEv'),'pointer',['pointer']);const methods=[];for(let i=0;i<Math.min(count,1500);i++){const value=Memory.alloc(16);value.writeU64(method(meta,i));methods.push({index:i,signature:signature(value).readCString()});}return {kind:kind(object),methods};}
function macroList(hwnd){const w=find(ptr(hwnd));const count=new NativeFunction(gui.getExportByName('_ZNK11QListWidget5countEv'),'int',['pointer'])(w);const item=new NativeFunction(gui.getExportByName('_ZNK11QListWidget4itemEi'),'pointer',['pointer','int']);const data=new NativeFunction(gui.getExportByName('_ZNK15QListWidgetItem4dataEi'),'void',['pointer','pointer','int']);const out=[];for(let i=0;i<Math.min(count,500);i++){const v=Memory.alloc(32);data(v,item(w,i),0);const s=Memory.alloc(4);vs(s,v);out.push({row:i,text:string(s)});sd(s);vd(v);}return out;}
function macroSci(hwnd){const w=find(ptr(hwnd));const module=Process.getModuleByName('qscintilla2.dll');const text=Memory.alloc(4);new NativeFunction(module.getExportByName('_ZNK13QsciScintilla4textEv'),'void',['pointer','pointer'])(text,w);const result=string(text);sd(text);return result;}
function macroSciSet(hwnd,value){const w=find(ptr(hwnd));const module=Process.getModuleByName('qscintilla2.dll');const text=Memory.alloc(4);sc(text,Memory.allocUtf16String(value),value.length);new NativeFunction(module.getExportByName('_ZN13QsciScintilla7setTextERK7QString'),'void',['pointer','pointer'])(w,text);sd(text);new NativeFunction(module.getExportByName('_ZN13QsciScintilla17setCursorPositionEii'),'void',['pointer','int','int'])(w,0,0);return macroSci(hwnd);}
function macroFocus(hwnd,item){const w=find(ptr(hwnd));new NativeFunction(gui.getExportByName('_ZN11QTreeWidget12scrollToItemEPK15QTreeWidgetItemN17QAbstractItemView10ScrollHintE'),'void',['pointer','pointer','int'])(w,ptr(item),3);const r=Memory.alloc(16);treeRect(r,w,ptr(item));return [0,4,8,12].map(i=>r.add(i).readInt());}
function macroHeaders(hwnd){const w=find(ptr(hwnd));const item=new NativeFunction(gui.getExportByName('_ZNK12QTableWidget20horizontalHeaderItemEi'),'pointer',['pointer','int']);const out=[];for(let i=0;i<Number(prop(w,'columnCount'));i++){const p=item(w,i);if(p.isNull()){out.push(null);continue;}const v=Memory.alloc(32);tableData(v,p,0);const s=Memory.alloc(4);vs(s,v);out.push(string(s));sd(s);vd(v);}return out;}
function macroTabs(hwnd){const w=find(ptr(hwnd));const f=new NativeFunction(gui.getExportByName('_ZNK10QTabWidget7tabTextEi'),'void',['pointer','pointer','int']);const out=[];for(let i=0;i<Number(prop(w,'count'));i++){const s=Memory.alloc(4);f(s,w,i);out.push(string(s));sd(s);}return out;}
'''
        shutil.copy2(Path(__file__),out/"harness-source.py")
        write(out/"probe-sources.json",{"baseJs":str(JS_SOURCE),"baseJsSha256":sha(JS_SOURCE),"guardSource":str(OLD/"startup_probe.py"),"guardSourceSha256":sha(OLD/"startup_probe.py"),"harnessSha256":sha(Path(__file__)),"fridaVersion":frida.__version__})
        user=ctypes.WinDLL("user32",use_last_error=True)
        user.CreateDesktopW.argtypes=[ctypes.c_wchar_p,ctypes.c_void_p,ctypes.c_void_p,ctypes.c_ulong,ctypes.c_ulong,ctypes.c_void_p];user.CreateDesktopW.restype=ctypes.c_void_p;user.CloseDesktop.argtypes=[ctypes.c_void_p]
        desktop="KinSealStage8_macro_"+args.run.replace("-","_")
        desk=user.CreateDesktopW(desktop,None,None,0,0x000F01FF,None);assert desk
        startup=win32process.STARTUPINFO();startup.lpDesktop="WinSta0\\"+desktop;startup.dwFlags=win32con.STARTF_USESHOWWINDOW;startup.wShowWindow=win32con.SW_SHOWNORMAL
        env.update(PATH=str(VENDOR)+os.pathsep+env.get("PATH",""),__COMPAT_LAYER="RunAsInvoker")
        process,thread,pid,_=win32process.CreateProcess(str(exe),'"%s" "%s"'%(exe,target),None,None,False,win32process.CREATE_UNICODE_ENVIRONMENT,env,str(work),startup)
        record.update(pid=pid,desktop=startup.lpDesktop)
        def windows():
            found=[]
            win32gui.EnumDesktopWindows(pywintypes.HANDLE(desk),lambda h,_:found.append(h) if win32process.GetWindowThreadProcessId(h)[1]==pid else None,None)
            return found
        editor=menu=None;deadline=time.time()+24
        while time.time()<deadline:
            for top in windows():
                children=[];win32gui.EnumChildWindows(top,lambda h,_:children.append(h),None)
                menus=[h for h in children if win32gui.GetWindowText(h)=="menuBar"]
                if menus:editor=top;menu=menus[0];break
            if editor:break
            time.sleep(.2)
        assert editor,"Owned editor did not open"
        def on_message(message,data):
            if message.get("type")=="error":record.setdefault("fridaErrors",[]).append(message)
            elif message.get("type")=="send":record.setdefault("fridaNotices",[]).append(message["payload"])
        session=frida.attach(pid);script=session.create_script(js);script.on("message",on_message);script.load();api=script.exports_sync
        api.set_thread(win32process.GetWindowThreadProcessId(menu)[0]);api.load_guarded_module(str(work/"DownT.dll"))
        modules=api.module_identities();record["loadedHardwareModules"]=modules
        assert len(modules)==2 and all(Path(x["path"]).resolve().parent==work.resolve() for x in modules)
        def op(request):
            serial=api.queue(request);win32gui.PostMessage(menu,win32con.WM_NULL,0,0);limit=time.time()+6
            while time.time()<limit:
                result=api.poll()
                if result and result["serial"]==serial:
                    record["steps"].append({"request":request,"result":result});assert result["status"]=="completed",result
                    return result["value"]
                time.sleep(.04)
            raise RuntimeError("GUI operation timed out: "+str(request))
        def snapshot(label):
            rows=[]
            for top in windows():
                if not win32gui.IsWindowVisible(top):continue
                children=[top]
                if top!=editor or not record["snapshots"]:win32gui.EnumChildWindows(top,lambda h,_:children.append(h),None)
                rows.append({"hwnd":top,"title":win32gui.GetWindowText(top),"class":win32gui.GetClassName(top),"widgets":op({"op":"batch","hwnds":children})})
            record["snapshots"].append({"label":label,"windows":rows});record["trace"]=api.events();write(out/"run.json",record);write(out/"projection-zh.json",normalized(record))
            return rows
        def flat(rows):
            for row in rows:
                yield row
                yield from flat(row.get("children",[]))
        def widget(command):
            candidates=[x for row in snapshot("locate-"+command.get("name",command.get("kind","widget"))) for x in row["widgets"] if x.get("qt",{}).get("visible")]
            for key in ["name","kind","text"]:
                if key in command:candidates=[x for x in candidates if normalized(x["qt"].get({"name":"objectName","kind":"kind","text":"text"}[key]))==command[key]]
            if "windowKind" in command:
                candidates=[x for x in candidates if any(x in row["widgets"] and any(y.get("qt",{}).get("kind")==command["windowKind"] for y in row["widgets"] if y["hwnd"]==row["hwnd"]) for row in record["snapshots"][-1]["windows"])]
            if "treeRoot" in command:
                candidates=[x for x in candidates if normalized(op({"op":"tree","hwnd":x["hwnd"]}))[0]["text"]==command["treeRoot"]]
            if "tabTitles" in command:
                candidates=[x for x in candidates if normalized(op({"op":"tab-titles","hwnd":x["hwnd"]}))==command["tabTitles"]]
            if "headers" in command:
                candidates=[x for x in candidates if normalized(op({"op":"table-headers","hwnd":x["hwnd"]}))==command["headers"]]
            assert len(candidates)==1,{"selector":command,"matching":candidates}
            return candidates[0]
        record["actions"]=op({"op":"menus","hwnd":menu})
        snapshot("before-entry")
        for command in commands:
            action=command["op"]
            if action=="entry":
                selected=next(x for x in flat(op({"op":"menus","hwnd":menu})) if x["objectName"]==command["name"]);assert selected["enabled"]
                op({"op":"invoke","pointer":selected["pointer"],"slot":"trigger"})
            elif action=="snapshot":pass
            elif action=="read":
                rows=snapshot(command.get("label","read"))
                reads=[]
                for row in rows:
                    for w in row["widgets"]:
                        kind=w.get("qt",{}).get("kind")
                        if kind in ["QTableWidget","QTreeWidget","QListWidget"]:
                            operation={"QTableWidget":"table-read","QTreeWidget":"tree","QListWidget":"list"}[kind]
                            item={"hwnd":w["hwnd"],"objectName":w["qt"]["objectName"],"kind":kind,"rows":op({"op":operation,"hwnd":w["hwnd"]})}
                            if kind=="QTableWidget":item["headers"]=op({"op":"table-headers","hwnd":w["hwnd"]})
                            reads.append(item)
                        elif kind=="QsciScintillaEx":reads.append({"hwnd":w["hwnd"],"kind":kind,"text":op({"op":"sci-read","hwnd":w["hwnd"]})})
                        elif kind=="QTabWidget":reads.append({"hwnd":w["hwnd"],"kind":kind,"tabs":op({"op":"tab-titles","hwnd":w["hwnd"]})})
                        elif kind=="QMenu":reads.append({"hwnd":w["hwnd"],"kind":kind,"actions":op({"op":"menus","hwnd":w["hwnd"]})})
                record.setdefault("reads",[]).append({"label":command.get("label"),"widgets":reads})
            elif action=="close-dialog":
                rows=snapshot("before-close")
                dialogs=[r for r in rows if any(w.get("qt",{}).get("kind") not in ["MainWindow","QWidget"] for w in r["widgets"] if w["hwnd"]==r["hwnd"])]
                assert len(dialogs)==1,dialogs
                op({"op":"invoke","hwnd":dialogs[0]["hwnd"],"slot":"close"})
            elif action=="exports":record["scintillaExports"]=op({"op":"module-exports"})
            elif action=="library-examples":
                tree=widget({"kind":"QTreeWidget","treeRoot":"函数库"})
                sci=widget({"kind":"QsciScintillaEx"})
                entries=normalized(op({"op":"tree","hwnd":tree["hwnd"]}))
                children=[];win32gui.EnumChildWindows(tree["hwnd"],lambda h,_:children.append(h),None)
                viewport=next(h for h in children if win32gui.GetParent(h)==tree["hwnd"] and win32gui.GetWindowText(h)=="qt_scrollarea_viewport")
                examples=[];group=None;seed="/*KS-STAGE8-EXAMPLE-SENTINEL*/\n"
                for item in entries:
                    if item["text"] in ["函数库","图形库"]:group=item["text"];continue
                    if "names" in command and item["text"] not in command["names"]:continue
                    entry={"name":item["text"],"group":group,"seed":seed,"seedSha256":hashlib.sha256(seed.encode()).hexdigest(),"cursor":[0,0],"selection":False}
                    try:
                        op({"op":"sci-set","hwnd":sci["hwnd"],"value":seed})
                        rect=op({"op":"tree-focus","hwnd":tree["hwnd"],"item":item["pointer"]})
                        time.sleep(.2)
                        rect=op({"op":"tree-focus","hwnd":tree["hwnd"],"item":item["pointer"]})
                        left,top,right,bottom=rect;x=(left+right)//2;y=(top+bottom)//2;point=(y<<16)|x
                        for message,wparam in [(win32con.WM_LBUTTONDOWN,win32con.MK_LBUTTON),(win32con.WM_LBUTTONUP,0),(win32con.WM_RBUTTONDOWN,win32con.MK_RBUTTON),(win32con.WM_RBUTTONUP,0)]:win32gui.PostMessage(viewport,message,wparam,point)
                        time.sleep(.18)
                        popups=[]
                        for top in windows():
                            if not win32gui.IsWindowVisible(top):continue
                            root_widget=op({"op":"batch","hwnds":[top]})[0].get("qt",{})
                            if root_widget.get("kind")=="QMenu":popups.append({"hwnd":top,"actions":op({"op":"menus","hwnd":top})})
                        entry["menus"]=popups
                        matching=[a for p in popups for a in normalized(p["actions"]) if a["text"]=="插入例子" and a["enabled"]]
                        assert len(matching)==1,{"item":item,"popups":popups}
                        op({"op":"invoke","pointer":matching[0]["pointer"],"slot":"trigger"})
                        time.sleep(.08)
                        after=op({"op":"sci-read","hwnd":sci["hwnd"]})
                        entry.update(after=after,afterSha256=hashlib.sha256(after.encode()).hexdigest(),seedPreserved=after.endswith(seed),status="inserted")
                        entry["insertedText"]=after[:-len(seed)] if after.endswith(seed) else None
                        if entry["insertedText"] is not None:entry["insertedTextUtf8Sha256"]=hashlib.sha256(entry["insertedText"].encode()).hexdigest()
                        for popup in popups:op({"op":"invoke","hwnd":popup["hwnd"],"slot":"close"})
                    except Exception as error:entry.update(status="partial-or-error",error=repr(error))
                    examples.append(entry);write(out/"library-examples.json",{"sourceRun":str(out/"run.json"),"examples":examples})
                record["libraryExamples"]={"path":str(out/"library-examples.json"),"count":len(examples),"insertedCount":sum(x["status"]=="inserted" for x in examples),"sha256":sha(out/"library-examples.json")}
            elif action=="menu-trigger":
                found=[]
                for row in snapshot("locate-popup-action"):
                    root_widget=next((x.get("qt",{}) for x in row["widgets"] if x["hwnd"]==row["hwnd"]),{})
                    if root_widget.get("kind")=="QMenu":found.extend(op({"op":"menus","hwnd":row["hwnd"]}))
                selected=next(x for x in normalized(found) if x["text"]==command["text"]);assert selected["enabled"]
                op({"op":"invoke","pointer":selected["pointer"],"slot":"trigger"})
            else:
                w=widget(command)
                if action=="click":op({"op":"invoke","hwnd":w["hwnd"],"slot":"click"})
                elif action=="meta":record.setdefault("meta",[]).append({"widget":w,"methods":op({"op":"meta-all","hwnd":w["hwnd"]})})
                elif action=="invoke":op({"op":"invoke","hwnd":w["hwnd"],"slot":command["slot"]})
                elif action=="table-open":op({"op":"table-open","hwnd":w["hwnd"],"row":command["row"],"column":command.get("column",0)})
                elif action=="validator":record.setdefault("validators",[]).append({"widget":w,"validator":op({"op":"validator","hwnd":w["hwnd"]})})
                elif action in ["tree-double","tree-context","tree-right"]:
                    tree_rows=normalized(op({"op":"tree","hwnd":w["hwnd"]}));entry=next(x for x in tree_rows if x["text"]==command["entry"])
                    children=[];win32gui.EnumChildWindows(w["hwnd"],lambda h,_:children.append(h),None)
                    viewport=next(h for h in children if win32gui.GetParent(h)==w["hwnd"] and win32gui.GetWindowText(h)=="qt_scrollarea_viewport")
                    left,top,right,bottom=entry["rect"];x=(left+right)//2;y=(top+bottom)//2;point=(y<<16)|x
                    win32gui.PostMessage(viewport,win32con.WM_LBUTTONDOWN,win32con.MK_LBUTTON,point);win32gui.PostMessage(viewport,win32con.WM_LBUTTONUP,0,point)
                    if action=="tree-double":win32gui.PostMessage(viewport,win32con.WM_LBUTTONDBLCLK,win32con.MK_LBUTTON,point);win32gui.PostMessage(viewport,win32con.WM_LBUTTONUP,0,point)
                    elif action=="tree-context":op({"op":"point-invoke","hwnd":w["hwnd"],"x":x,"y":y})
                    else:
                        win32gui.PostMessage(viewport,win32con.WM_RBUTTONDOWN,win32con.MK_RBUTTON,point);win32gui.PostMessage(viewport,win32con.WM_RBUTTONUP,0,point)
                    record.setdefault("directedTreeEvents",[]).append({"command":command,"entry":entry,"viewport":viewport})
                elif action in ["text","combo","spin","check","tab"]:op({"op":action,"hwnd":w["hwnd"],"value":command["value"]})
                else:raise ValueError(command)
            time.sleep(.3)
            snapshot(command.get("label",action))
        record["status"]="observed"
    except Exception as error:
        record.update(status="partial-or-error",error=repr(error))
    finally:
        if "input" in record and Path(record["input"]["path"]).exists():record["input"]["finalSha256"]=sha(Path(record["input"]["path"]))
        if script:
            try:record["trace"]=script.exports_sync.events()
            except Exception as error:record["traceReadError"]=repr(error)
        if process:
            prior=win32process.GetExitCodeProcess(process)
            if prior==259:win32process.TerminateProcess(process,0)
            win32event.WaitForSingleObject(process,5000)
            record["cleanup"]={"exitCodeBeforeForcedCleanup":prior,"terminated":win32process.GetExitCodeProcess(process)!=259}
        if session:
            try:session.detach()
            except Exception:pass
        for handle in [thread,process]:
            if handle:handle.Close()
        if desk:user.CloseDesktop(desk)
        try:
            with winreg.OpenKey(winreg.HKEY_CURRENT_USER,r"Software\KinSealStudio\recent pro files",0,winreg.KEY_QUERY_VALUE|winreg.KEY_SET_VALUE) as key:
                recent,kind=winreg.QueryValueEx(key,"recentFileList");prefix=str(work).replace("\\","/").casefold()+"/"
                kept=[x for x in recent if not x.replace("\\","/").casefold().startswith(prefix)]
                if kind==winreg.REG_MULTI_SZ and len(kept)!=len(recent):winreg.SetValueEx(key,"recentFileList",0,kind,kept)
                record["ownMruCleanup"]={"removed":len(recent)-len(kept),"otherEntriesPreserved":len(kept)}
        except OSError as error:record["ownMruCleanup"]={"error":repr(error)}
        record["endedEpoch"]=time.time();write(out/"run.json",record);write(out/"projection-zh.json",normalized(record))
    print(json.dumps({"status":record["status"],"error":record.get("error"),"out":str(out)},ensure_ascii=False))
    return 0 if record["status"]=="observed" else 1

if __name__=="__main__":raise SystemExit(main())
