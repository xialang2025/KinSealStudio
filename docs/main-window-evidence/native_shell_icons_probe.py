"""Extract actual Qt4 QAction and toolbox QIcon assets on an owned private desktop."""
from pathlib import Path
import ctypes
import hashlib
import json
import os
import runpy
import shutil
import sys
import time
import pywintypes
import win32api
import win32con
import win32gui
import win32process
import win32ui
from PIL import Image

ROOT=Path(r'Z:\Project\vscode_prj_temp\REA')
VENDOR=Path(r'G:\01_Professional\触摸屏\金玺\KinSealStudio_Setup_V5.1\Installed')
WORK=ROOT/'.scratch/kinseal-main-window-20261111/icons-runtime'
OUT=ROOT/'KinSealStudio/docs/main-window-evidence'
ICONS=ROOT/'KinSealStudio/resources/native-shell/icons'
DESKTOP='KinSealNativeShellIcons20261111'
sys.path.insert(0,r'G:\01_Professional\Frida\Installed')
import frida

BASE=runpy.run_path(str(ROOT/'KinSealStudio/docs/stage3-evidence/native/stage3_native_probe.py'),run_name='icon_probe_seed')['JS']
EXTRA=r"""
const actionIconFn=new NativeFunction(gui.getExportByName('_ZNK7QAction4iconEv'),'void',['pointer','pointer']);
const buttonIconFn=new NativeFunction(gui.getExportByName('_ZNK15QAbstractButton4iconEv'),'void',['pointer','pointer']);
const windowIconFn=new NativeFunction(gui.getExportByName('_ZNK7QWidget10windowIconEv'),'void',['pointer','pointer']);
const iconNullFn=new NativeFunction(gui.getExportByName('_ZNK5QIcon6isNullEv'),'bool',['pointer']);
const iconSizesFn=new NativeFunction(gui.getExportByName('_ZNK5QIcon14availableSizesENS_4ModeENS_5StateE'),'void',['pointer','pointer','int','int']);
const iconPixmapFn=new NativeFunction(gui.getExportByName('_ZNK5QIcon6pixmapERK5QSizeNS_4ModeENS_5StateE'),'void',['pointer','pointer','pointer','int','int']);
const iconDtorFn=new NativeFunction(gui.getExportByName('_ZN5QIconD1Ev'),'void',['pointer']);
const pixmapDtorFn=new NativeFunction(gui.getExportByName('_ZN7QPixmapD1Ev'),'void',['pointer']);
const pixmapWidthFn=new NativeFunction(gui.getExportByName('_ZNK7QPixmap5widthEv'),'int',['pointer']);
const pixmapHeightFn=new NativeFunction(gui.getExportByName('_ZNK7QPixmap6heightEv'),'int',['pointer']);
const pixmapSaveFn=new NativeFunction(gui.getExportByName('_ZNK7QPixmap4saveERK7QStringPKci'),'bool',['pointer','pointer','pointer','int']);
const separatorFn=new NativeFunction(gui.getExportByName('_ZNK7QAction11isSeparatorEv'),'bool',['pointer']);
const actionGeometryFn=new NativeFunction(gui.getExportByName('_ZNK8QToolBar14actionGeometryEP7QAction'),'void',['pointer','pointer','pointer']);
const actionWidgetFn=new NativeFunction(gui.getExportByName('_ZNK8QToolBar15widgetForActionEP7QAction'),'pointer',['pointer','pointer']);
const defaultActionFn=new NativeFunction(gui.getExportByName('_ZNK11QToolButton13defaultActionEv'),'pointer',['pointer']);
const toRectFn=new NativeFunction(core.getExportByName('_ZNK8QVariant6toRectEv'),'void',['pointer','pointer']);
const toSizeFn=new NativeFunction(core.getExportByName('_ZNK8QVariant6toSizeEv'),'void',['pointer','pointer']);
const variantDataFn=new NativeFunction(core.getExportByName('_ZNK8QVariant9constDataEv'),'pointer',['pointer']);
const fontFamilyFn=new NativeFunction(gui.getExportByName('_ZNK5QFont6familyEv'),'void',['pointer','pointer']);
const fontSizeFn=new NativeFunction(gui.getExportByName('_ZNK5QFont10pointSizeFEv'),'double',['pointer']);
const fontPixelsFn=new NativeFunction(gui.getExportByName('_ZNK5QFont9pixelSizeEv'),'int',['pointer']);
const fontWeightFn=new NativeFunction(gui.getExportByName('_ZNK5QFont6weightEv'),'int',['pointer']);
const paletteBrushFn=new NativeFunction(gui.getExportByName('_ZNK8QPalette5brushENS_10ColorGroupENS_9ColorRoleE'),'pointer',['pointer','int','int']);
const colorRgbaFn=new NativeFunction(gui.getExportByName('_ZNK6QColor4rgbaEv'),'uint',['pointer']);
const widgetStyleFn=new NativeFunction(gui.getExportByName('_ZNK7QWidget5styleEv'),'pointer',['pointer']);
const brushVariantFn=new NativeFunction(gui.getExportByName('_ZNK6QBrushcv8QVariantEv'),'void',['pointer','pointer']);
const variantConvertFn=new NativeFunction(core.getExportByName('_ZN8QVariant7convertENS_4TypeE'),'bool',['pointer','int']);
const exported=new Map();
function idSafe(value){return value.replace(/[^a-zA-Z0-9_.-]+/g,'_').replace(/^_+|_+$/g,'')||'unnamed';}
function variantObject(w,name){const v=Memory.alloc(32);propFn(v,w,Memory.allocUtf8String(name));return v;}
function geometry(w){const v=variantObject(w,'geometry');const r=Memory.alloc(16);toRectFn(r,v);const out=[0,4,8,12].map(i=>r.add(i).readInt());vd(v);return {x:out[0],y:out[1],width:out[2]-out[0]+1,height:out[3]-out[1]+1,rawQRect:out};}
function sizeProperty(w,name){const v=variantObject(w,name);const s=Memory.alloc(8);toSizeFn(s,v);const out=[s.readInt(),s.add(4).readInt()];vd(v);return out;}
function font(w){const v=variantObject(w,'font');const p=variantDataFn(v);const s=Memory.alloc(4);fontFamilyFn(s,p);const out={family:string(s),pointSizeF:fontSizeFn(p),pixelSize:fontPixelsFn(p),weight:fontWeightFn(p),rawProperty:prop(w,'font')};sd(s);vd(v);return out;}
function palette(w){const v=variantObject(w,'palette');const p=variantDataFn(v);const roles={Window:10,WindowText:0,Base:9,Button:1,ButtonText:8,Text:6,Highlight:12,HighlightedText:13,Light:2,Mid:4,Dark:5,Shadow:11};const out={};for(const [name,role]of Object.entries(roles)){try{const brush=paletteBrushFn(p,0,role);const converted=Memory.alloc(32);brushVariantFn(converted,brush);const convertedOk=variantConvertFn(converted,67);if(convertedOk){const rgba=colorRgbaFn(variantDataFn(converted));out[name]='#'+rgba.toString(16).padStart(8,'0');}else{out[name]=null;}vd(converted);}catch(e){out[name]={error:String(e)};}}vd(v);return out;}
function common(w){return {pointer:w.toString(),kind:kind(w),id:prop(w,'objectName'),text:prop(w,'text'),toolTip:prop(w,'toolTip'),statusTip:prop(w,'statusTip'),title:prop(w,'windowTitle'),visible:prop(w,'visible',true),enabled:prop(w,'enabled',true),geometry:geometry(w),font:font(w),palette:palette(w),style:prop(widgetStyleFn(w),'objectName'),styleClass:kind(widgetStyleFn(w)),styleSheet:prop(w,'styleSheet')};}
function iconExport(w,id,kindName,output){const sourceObjectId=id;id=idSafe(id==='action_H'?'action_Help':id);if(exported.has(id)){return exported.get(id);}const ico=Memory.alloc(32);if(kindName==='action'){actionIconFn(ico,w);}else if(kindName==='window'){windowIconFn(ico,w);}else{buttonIconFn(ico,w);}const info={id,sourceObjectId,isNull:iconNullFn(ico),availableSizes:[],pixmaps:[]};try{if(!info.isNull){const list=Memory.alloc(4);iconSizesFn(list,ico,0,1);const d=list.readPointer();const begin=d.add(8).readInt(),end=d.add(12).readInt();if(begin>=0&&end>=begin&&end-begin<64){for(let i=begin;i<end;++i){const p=d.add(20+i*4).readPointer();info.availableSizes.push([p.readInt(),p.add(4).readInt()]);}}for(const requested of [24,32]){const size=Memory.alloc(8);size.writeInt(requested);size.add(4).writeInt(requested);const pix=Memory.alloc(256);iconPixmapFn(pix,ico,size,0,1);const filename=id+(requested===24?'':'-32')+'.png';const path=output+'/'+filename;const qpath=Memory.alloc(4);sc(qpath,Memory.allocUtf16String(path),path.length);const width=pixmapWidthFn(pix),height=pixmapHeightFn(pix);const saved=pixmapSaveFn(pix,qpath,Memory.allocUtf8String('PNG'),-1);sd(qpath);pixmapDtorFn(pix);info.pixmaps.push({requestedSize:[requested,requested],actualSize:[width,height],mode:'Normal',state:'Off',saved,path:'resources/native-shell/icons/'+filename});if(requested===24&&saved){info.path='resources/native-shell/icons/'+filename;}}}}catch(e){info.error=String(e);}finally{iconDtorFn(ico);}exported.set(id,info);return info;}
function actionList(w){const list=Memory.alloc(4);actionsFn(list,w);const d=list.readPointer();const b=d.add(8).readInt(),e=d.add(12).readInt();if(b<0||e<b||e-b>500){throw Error('QList actions bound');}const out=[];for(let i=b;i<e;++i){out.push(d.add(20+i*4).readPointer());}return out;}
function actionInfo(a,fallback,output){const q=Memory.alloc(4);textFn(q,a);const id=prop(a,'objectName')||fallback;const out={id,pointer:a.toString(),text:string(q),separator:separatorFn(a),enabled:enabledFn(a),visible:prop(a,'visible',true),checkable:prop(a,'checkable',true),checked:prop(a,'checked',true),shortcut:prop(a,'shortcut'),toolTip:prop(a,'toolTip'),statusTip:prop(a,'statusTip'),icon:iconExport(a,id,'action',output)};sd(q);return out;}
function menuRows(w,path,output,depth=0){if(depth>6){return [];}return actionList(w).map((a,i)=>{const out=actionInfo(a,path+'-'+i,output);const menu=menuFn(a);if(!menu.isNull()){out.children=menuRows(menu,out.id,output,depth+1);}return out;});}
function extractShell(p){const widgets=p.widgets.map(item=>{const w=find(ptr(item.hwnd));if(w.isNull()){return null;}try{return {...item,qt:common(w)};}catch(e){return {...item,error:String(e)};}}).filter(Boolean);const root=find(ptr(p.editor));const menus=menuRows(find(ptr(p.menu)), 'menu',p.output);const toolbars=[];const toolbox=[];for(const item of widgets){const q=item.qt;if(!q){continue;}const w=ptr(q.pointer);if(q.kind==='QToolBar'){const bar={...q,iconSize:sizeProperty(w,'iconSize'),toolButtonStyle:prop(w,'toolButtonStyle'),orientation:prop(w,'orientation'),movable:prop(w,'movable',true),floatable:prop(w,'floatable',true),items:[]};bar.items=actionList(w).map((a,i)=>{const info=actionInfo(a,(q.id||'toolbar')+'-'+i,p.output);const r=Memory.alloc(16);actionGeometryFn(r,w,a);const raw=[0,4,8,12].map(o=>r.add(o).readInt());info.geometry={x:raw[0],y:raw[1],width:raw[2]-raw[0]+1,height:raw[3]-raw[1]+1,rawQRect:raw};const aw=actionWidgetFn(w,a);if(!aw.isNull()){info.widget=common(aw);}return info;});toolbars.push(bar);}if(q.kind==='QToolButton'||q.kind==='QPushButton'){let ancestor=item.parent;let inToolbar=false;let inToolbox=false;let chain=[];for(let n=0;n<15&&ancestor;n++){const parent=widgets.find(v=>v.hwnd===ancestor);if(!parent){break;}chain.push({id:parent.qt?.id,kind:parent.qt?.kind,text:parent.qt?.text});inToolbar||=parent.qt?.kind==='QToolBar';inToolbox||=parent.qt?.kind==='QToolBox'||['groupBox_control','m_toolwidgetCtl'].includes(parent.qt?.id)||(parent.qt?.kind==='myDockWidget'&&parent.qt?.title==='工具栏');ancestor=parent.parent;}if(inToolbox&&!inToolbar){const id=q.id||'toolbox-'+toolbox.length;const captionLabels=widgets.filter(v=>v.parent===item.parent&&v.qt?.kind==='QLabel'&&Math.abs(v.qt.geometry.x+v.qt.geometry.width/2-q.geometry.x-q.geometry.width/2)<5&&v.qt.geometry.y>=q.geometry.y+q.geometry.height&&v.qt.geometry.y<=q.geometry.y+q.geometry.height+20).map(v=>v.qt);const extra={...q,text:q.text||captionLabels.map(v=>v.text).join(' '),captionLabels,icon:iconExport(w,id,'button',p.output),iconSize:sizeProperty(w,'iconSize'),ancestors:chain};if(q.kind==='QToolButton'){extra.toolButtonStyle=prop(w,'toolButtonStyle');const a=defaultActionFn(w);if(!a.isNull()){extra.defaultActionId=prop(a,'objectName');}}toolbox.push(extra);}}}return{applicationStyleSheet:prop(core.getExportByName('_ZN16QCoreApplication4selfE').readPointer(),'styleSheet'),window:common(root),windowIcon:iconExport(root,'windowIcon','window',p.output),menuBar:common(find(ptr(p.menu))),menus,toolbars,toolbox,widgets,icons:[...exported.values()]};}
"""
JS=BASE.replace('function execute(p){','function execute(p){if(p.op===\'shell-extract\'){return extractShell(p);}')+'\n'+EXTRA

def main():
    """Launch a private editor, extract owned Qt assets on its GUI thread, and close it."""
    WORK.mkdir(parents=True,exist_ok=True)
    OUT.mkdir(parents=True,exist_ok=True)
    ICONS.mkdir(parents=True,exist_ok=True)
    original=(VENDOR/'KinSealStudio5.1.exe').read_bytes()
    assert hashlib.sha256(original).hexdigest()=='305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c'
    private=bytearray(original)
    assert private[0x7c4:0x7c7]==bytes.fromhex('5589e5')
    private[0x7c4:0x7c7]=bytes.fromhex('b001c3')
    exe=WORK/'KinSealStudio5.1-private.exe'
    exe.write_bytes(private)
    for name in ['model.dat','modelInfo.dat','language.ini','KinSeal.ico']:shutil.copy2(VENDOR/name,WORK/name)
    for plugin_folder in ['imageformats','iconengines','codecs']:
        destination=WORK/plugin_folder
        destination.mkdir(exist_ok=True)
        for library in (VENDOR/plugin_folder).glob('*4.dll'):
            if not library.name.endswith('d4.dll'):shutil.copy2(library,destination/library.name)
    source=WORK/'PrivateShell.sm'
    shutil.copy2(ROOT/'KinSealStudio/samples/native-device-test/CodexProbeTest.sm',source)
    (WORK/'temp').mkdir(exist_ok=True)
    result={'method':'Actual Qt4 QIcon/geometry/font extraction, invoked on owned GUI thread on private desktop','originalSha256':hashlib.sha256(original).hexdigest(),'runtimeSha256':hashlib.sha256(private).hexdigest(),'libraries':{name:hashlib.sha256((VENDOR/name).read_bytes()).hexdigest() for name in ['QtCore4.dll','QtGui4.dll']},'desktop':DESKTOP,'usbAccessed':False,'systemInput':False,'foregroundActivated':False,'requestedWindowSize':[1353,731]}
    user=ctypes.WinDLL('user32',use_last_error=True)
    user.CreateDesktopW.argtypes=[ctypes.c_wchar_p,ctypes.c_void_p,ctypes.c_void_p,ctypes.c_ulong,ctypes.c_ulong,ctypes.c_void_p]
    user.CreateDesktopW.restype=ctypes.c_void_p
    user.CloseDesktop.argtypes=[ctypes.c_void_p]
    desk=user.CreateDesktopW(DESKTOP,None,None,0,0x000F01FF,None)
    assert desk
    process=thread=session=None
    def windows(pid):
        values=[]
        win32gui.EnumDesktopWindows(pywintypes.HANDLE(desk),lambda h,_:values.append(h) if win32process.GetWindowThreadProcessId(h)[1]==pid else None,None)
        return values
    try:
        startup=win32process.STARTUPINFO()
        startup.lpDesktop='WinSta0\\'+DESKTOP
        startup.dwFlags=win32con.STARTF_USESHOWWINDOW
        startup.wShowWindow=win32con.SW_SHOWNORMAL
        environment=os.environ.copy()
        environment['PATH']=str(VENDOR)+os.pathsep+environment.get('PATH','')
        environment['__COMPAT_LAYER']='RunAsInvoker'
        environment['TEMP']=environment['TMP']=str(WORK/'temp')
        process,thread,pid,_=win32process.CreateProcess(str(exe),'"'+str(exe)+'" "'+str(source)+'"',None,None,False,win32process.CREATE_UNICODE_ENVIRONMENT,environment,str(WORK),startup)
        result['pid']=pid
        deadline=time.time()+24
        editor=menu=None
        while time.time()<deadline:
            for top in windows(pid):
                children=[]
                win32gui.EnumChildWindows(top,lambda h,_:children.append(h),None)
                matches=[h for h in children if win32gui.GetWindowText(h)=='menuBar']
                if matches:editor,menu=top,matches[0];break
            if editor:break
            time.sleep(.25)
        assert editor,'Private editor unavailable'
        win32gui.SetWindowPos(editor,0,0,0,1353,731,win32con.SWP_NOACTIVATE|win32con.SWP_NOMOVE|win32con.SWP_NOZORDER)
        time.sleep(1)
        children=[]
        win32gui.EnumChildWindows(editor,lambda h,_:children.append(h),None)
        widgets=[{'hwnd':h,'parent':win32gui.GetParent(h),'nativeClass':win32gui.GetClassName(h),'nativeRect':win32gui.GetWindowRect(h)} for h in [editor]+children]
        session=frida.attach(pid)
        script=session.create_script(JS)
        script.on('message',lambda message,_:result.setdefault('instrumentationMessages',[]).append(message))
        script.load()
        api=script.exports_sync
        api.set_thread(win32process.GetWindowThreadProcessId(menu)[0])
        serial=api.queue({'op':'shell-extract','editor':editor,'menu':menu,'widgets':widgets,'output':str(ICONS).replace('\\','/')})
        win32gui.PostMessage(menu,win32con.WM_NULL,0,0)
        deadline=time.time()+40
        while time.time()<deadline:
            answer=api.poll()
            if answer and answer['serial']==serial:
                assert answer['status']=='completed',answer
                result.update(answer['value'])
                result['guiThread']=answer.get('guiThread')
                break
            time.sleep(.1)
        else:raise RuntimeError('GUI extraction timeout')
        initial_toolbox=result.get('toolbox',[])
        candidates=[w for w in result['widgets'] if w.get('qt',{}).get('kind')=='QTabWidget' and w['qt']['geometry']['height']>100]
        right_tab=max(candidates,key=lambda w:w['nativeRect'][0])
        def gui_operation(request):
            serial=api.queue(request)
            win32gui.PostMessage(menu,win32con.WM_NULL,0,0)
            deadline=time.time()+30
            while time.time()<deadline:
                response=api.poll()
                if response and response['serial']==serial:
                    assert response['status']=='completed',response
                    return response['value']
                time.sleep(.05)
            raise RuntimeError('GUI basic-tab extraction timeout')
        gui_operation({'op':'tab','hwnd':right_tab['hwnd'],'value':1})
        time.sleep(.4)
        basic_children=[]
        win32gui.EnumChildWindows(editor,lambda h,_:basic_children.append(h),None)
        basic_widgets=[{'hwnd':h,'parent':win32gui.GetParent(h),'nativeClass':win32gui.GetClassName(h),'nativeRect':win32gui.GetWindowRect(h)} for h in [editor]+basic_children]
        basic=gui_operation({'op':'shell-extract','editor':editor,'menu':menu,'widgets':basic_widgets,'output':str(ICONS).replace('\\','/')})
        initial_ids={item['id'] for item in initial_toolbox}
        result['toolboxBasic']=[item for item in basic['toolbox'] if item['id'] not in initial_ids]
        result['basicWidgets']=basic['widgets']
        result['icons']=basic['icons']
        result['toolboxTabControl']={'hwnd':right_tab['hwnd'],'initialIndex':0,'basicIndex':1}
        gui_operation({'op':'tab','hwnd':right_tab['hwnd'],'value':0})
        time.sleep(.2)
        result['status']='completed'
        left,top,right,bottom=win32gui.GetWindowRect(editor)
        hdc=win32gui.GetWindowDC(editor)
        dc=win32ui.CreateDCFromHandle(hdc)
        memory=dc.CreateCompatibleDC()
        bitmap=win32ui.CreateBitmap()
        bitmap.CreateCompatibleBitmap(dc,right-left,bottom-top)
        memory.SelectObject(bitmap)
        try:
            result['PrintWindowResult']=user.PrintWindow(editor,memory.GetSafeHdc(),2)
            info=bitmap.GetInfo()
            image=Image.frombuffer('RGB',(info['bmWidth'],info['bmHeight']),bitmap.GetBitmapBits(True),'raw','BGRX',0,1)
            image.save(OUT/'native-shell.png')
        finally:
            memory.DeleteDC();dc.DeleteDC();win32gui.ReleaseDC(editor,hdc);win32gui.DeleteObject(bitmap.GetHandle())
    except Exception as error:
        result['status']='partial-or-error'
        result['error']=repr(error)
    finally:
        if process and win32process.GetExitCodeProcess(process)==259:win32process.TerminateProcess(process,0)
        if session:
            try:session.detach()
            except Exception as error:result['detachError']=repr(error)
        if process:process.Close();result['ownedProcessTerminated']=True
        if thread:thread.Close()
        user.CloseDesktop(desk)
        result['iconFiles']=[{'path':str(p.relative_to(ROOT/'KinSealStudio')).replace('\\','/'),'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in sorted(ICONS.glob('*.png'))]
        (OUT/'native-shell.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
        print(json.dumps({key:result.get(key) for key in ['status','error','pid','ownedProcessTerminated']}))
        print(json.dumps({'iconFiles':len(result['iconFiles']),'toolbars':len(result.get('toolbars',[])),'toolbox':len(result.get('toolbox',[]))}))

if __name__=='__main__':main()
