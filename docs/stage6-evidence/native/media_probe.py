from pathlib import Path
import argparse,ast,ctypes,hashlib,json,os,shutil,sys,time,winreg
import pefile,pywintypes,win32con,win32gui,win32process,win32event
sys.dont_write_bytecode=True
sys.stdout.reconfigure(encoding='utf-8')
sys.path.insert(0,r'G:\01_Professional\Frida\Installed')
import frida
ROOT=Path(r'Z:\Project\vscode_prj_temp\REA')
BASE=ROOT/'.scratch/kinseal-stage6-20261114/native-ui-media'
WT=ROOT/'.scratch/kinseal-stage4-20261112/worktree'
VENDOR=Path(r'G:\01_Professional\触摸屏\金玺\KinSealStudio_Setup_V5.1\Installed')
HELPER=WT/'docs/stage4-evidence/ascii/physical/ascii-lifecycle-r1/native_screen_probe.py'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def inventory(p):
 return [{'path':str(f.relative_to(p)),'bytes':f.stat().st_size,'sha256':sha(f)} for f in sorted(p.rglob('*')) if f.is_file()]
def private_libusb(source,target,names):
 raw=bytearray(source.read_bytes());pe=pefile.PE(data=bytes(raw));changes=[]
 for symbol in pe.DIRECTORY_ENTRY_EXPORT.symbols:
  name=symbol.name.decode('ascii') if symbol.name else ''
  if name not in names:continue
  assert not symbol.forwarder
  offset=pe.get_offset_from_rva(symbol.address);stub=b'\x31\xc0\xc3' if names[name]==0 else b'\xb8\xfb\xff\xff\xff\xc3'
  changes.append({'name':name,'rva':symbol.address,'fileOffset':offset,'before':bytes(raw[offset:offset+len(stub)]).hex(),'after':stub.hex(),'result':names[name]})
  raw[offset:offset+len(stub)]=stub
 assert len(changes)==len(names),(source,changes)
 target.write_bytes(raw);return {'original':str(source),'originalSha256':sha(source),'private':str(target),'privateSha256':sha(target),'patches':changes,'abi':'cdecl; original usb_open/bulk/control and DownT wrapper RET instructions verified before harness preparation','scope':'Hardware-blocking test interposition, not native production behavior'}
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--run',required=True);ap.add_argument('--variant',action='store_true');args=ap.parse_args()
 assert args.run in ['baseline-r1','baseline-r1b','baseline-r2','text-r1','upload-r1']
 out=BASE/args.run;out.mkdir(exist_ok=False);work=out/'runtime';work.mkdir();target=out/'media-target';target.mkdir();temp=out/'temp';temp.mkdir()
 env=os.environ.copy();env.update(TEMP=str(temp),TMP=str(temp),TMPDIR=str(temp),PYTHONDONTWRITEBYTECODE='1')
 record={'run':args.run,'status':'preparing','task':'S06-02/S06-05','privateDesktop':True,'globalInput':False,'usbTransferAuthorized':False,'userMediaAccessed':False,'target':str(target),'startedEpoch':time.time(),'steps':[],'snapshots':[]}
 process=thread=session=None;desk=None
 try:
  raw=(VENDOR/'KinSealStudio5.1.exe').read_bytes();assert hashlib.sha256(raw).hexdigest()=='305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c'
  private=bytearray(raw);assert private[0x7c4:0x7c7]==bytes.fromhex('5589e5');private[0x7c4:0x7c7]=bytes.fromhex('b001c3')
  shared=BASE/'shared';shared.mkdir(exist_ok=True);se=shared/'KinSealStudio5.1-private.exe'
  if not se.exists():se.write_bytes(private)
  assert sha(se)=='38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89'
  exe=work/se.name;os.link(se,exe)
  for name in ['model.dat','modelInfo.dat','language.ini']:shutil.copy2(VENDOR/name,work/name)
  for folder in ['codecs','imageformats','iconengines']:
   (work/folder).mkdir()
   for f in (VENDOR/folder).glob('*4.dll'):
    if not f.name.endswith('d4.dll'):shutil.copy2(f,work/folder/f.name)
  (work/'PlcDll').mkdir();shutil.copy2(VENDOR/'PlcDll/Modbus_RTU.dll',work/'PlcDll/Modbus_RTU.dll')
  record['hardwareBlockers']=[private_libusb(VENDOR/'libusb0.dll',work/'libusb0.dll',{'usb_init':0,'usb_find_busses':0,'usb_find_devices':0,'usb_get_busses':0,'usb_open':0,'usb_bulk_read':-5,'usb_bulk_write':-5,'usb_control_msg':-5,'usb_submit_async':-5}),private_libusb(VENDOR/'DownT.dll',work/'DownT.dll',{'Usb_connect':0,'Usb_init':0,'Usb_readRaw':-5,'Usb_writeRaw':-5})]
  source=work/'NativePages.sm';native=(WT/'samples/native-device-test/CodexProbeTest.sm').read_bytes()
  if args.variant:
   assert native[3280:3288]=='TEST'.encode('utf-16-be') and native[3276:3280]==b'\x00\x00\x00\x08'
   native=native[:3280]+'S6M1'.encode('utf-16-be')+native[3288:]
   record['singleVariable']={'fileOffset':3280,'bytes':8,'before':'TEST','after':'S6M1','encoding':'UTF-16BE','scope':'One existing QString payload, constant byte length; semantic field is a text candidate, not inferred solely from offset'}
  source.write_bytes(native);record['input']={'path':str(source),'bytes':len(native),'sha256':sha(source)}
  tree=ast.parse(HELPER.read_text(encoding='utf-8'));js=ast.literal_eval(next(n.value for n in tree.body if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='JS' for t in n.targets)))
  js=js.replace('rpc.exports={','rpc.exports={loadGuardedModule(path){return Module.load(path).name;},moduleIdentities(){return Process.enumerateModules().filter(m=>/^(libusb0|DownT)\\.dll$/i.test(m.name)).map(m=>({name:m.name,path:m.path}));},',1)
  desktop='KinSealStage6Media_'+args.run.replace('-','_');u=ctypes.WinDLL('user32',use_last_error=True);u.CreateDesktopW.argtypes=[ctypes.c_wchar_p,ctypes.c_void_p,ctypes.c_void_p,ctypes.c_ulong,ctypes.c_ulong,ctypes.c_void_p];u.CreateDesktopW.restype=ctypes.c_void_p;u.CloseDesktop.argtypes=[ctypes.c_void_p];desk=u.CreateDesktopW(desktop,None,None,0,0x000F01FF,None);assert desk
  startup=win32process.STARTUPINFO();startup.lpDesktop='WinSta0\\'+desktop;startup.dwFlags=win32con.STARTF_USESHOWWINDOW;startup.wShowWindow=win32con.SW_SHOWNORMAL
  env.update(PATH=str(VENDOR)+os.pathsep+env.get('PATH',''),__COMPAT_LAYER='RunAsInvoker')
  process,thread,pid,_=win32process.CreateProcess(str(exe),'"%s" "%s"'%(exe,source),None,None,False,win32process.CREATE_UNICODE_ENVIRONMENT,env,str(work),startup);record['pid']=pid
  def windows():
   found=[];win32gui.EnumDesktopWindows(pywintypes.HANDLE(desk),lambda h,_:found.append(h) if win32process.GetWindowThreadProcessId(h)[1]==pid else None,None);return found
  editor=menu=None;deadline=time.time()+24
  while time.time()<deadline:
   for top in windows():
    cs=[];win32gui.EnumChildWindows(top,lambda h,_:cs.append(h),None)
    menus=[h for h in cs if win32gui.GetWindowText(h)=='menuBar']
    if menus:editor=top;menu=menus[0];break
   if editor:break
   time.sleep(.2)
  assert editor,'Owned private editor not created'
  session=frida.attach(pid);script=session.create_script(js);script.load();api=script.exports_sync;api.set_thread(win32process.GetWindowThreadProcessId(menu)[0])
  api.load_guarded_module(str(work/'DownT.dll'));modules=api.module_identities();record['loadedHardwareModules']=modules
  assert len(modules)==2 and all(Path(x['path']).resolve().parent==work.resolve() for x in modules),modules
  for x in modules:assert sha(Path(x['path']))==sha(work/x['name'])
  def op(p):
   serial=api.queue(p);win32gui.PostMessage(menu,win32con.WM_NULL,0,0);limit=time.time()+6
   while time.time()<limit:
    r=api.poll()
    if r and r['serial']==serial:
     record['steps'].append({'request':p,'result':r});assert r['status']=='completed',r;return r['value']
    time.sleep(.04)
   raise RuntimeError('GUI operation timeout '+str(p))
  def snapshot(label):
   rows=[]
   for top in windows():
    if not win32gui.IsWindowVisible(top):continue
    cs=[top];win32gui.EnumChildWindows(top,lambda h,_:cs.append(h),None);rows.append({'hwnd':top,'winClass':win32gui.GetClassName(top),'title':win32gui.GetWindowText(top),'widgets':op({'op':'batch','hwnds':cs})})
   record['snapshots'].append({'label':label,'windows':rows});write(out/'run.json',record);return rows
  def widget(name):
   rows=snapshot('find-'+name);matches=[x for r in rows for x in r['widgets'] if x.get('qt',{}).get('objectName')==name and x['qt'].get('visible')];assert len(matches)==1,(name,matches);return matches[0]
  def flat(rows):
   for r in rows:yield r;yield from flat(r.get('children',[]))
  if args.run=='upload-r1':
   opened=op({'op':'invoke','hwnd':editor,'slot':'OnCreatUploadDlg'})
   assert opened.get('queuedMetaInvoke'),opened
   time.sleep(.5)
   rows=snapshot('upload-window-no-start')
   matches=[x for r in rows for x in r['widgets'] if x.get('qt',{}).get('kind')=='QUpLoadDlgEx']
   assert len(matches)==1,'QUpLoadDlgEx not uniquely observed'
   record['status']='upload-window-observed-no-transfer'
   record['uploadButtonInvoked']=False
   record['saveFilter']='not observed: native save picker not opened in this bounded run'
   raise StopIteration()
  command=next(x for x in flat(op({'op':'menus','hwnd':menu})) if x['objectName']=='action_USBdownload');assert command['enabled'];op({'op':'invoke','pointer':command['pointer'],'slot':'trigger'});time.sleep(1)
  deadline=time.time()+25
  while not (work/'compile/SysPara.dat').exists() and time.time()<deadline:time.sleep(.2)
  assert (work/'compile/SysPara.dat').is_file(),'Native compiler did not create SysPara'
  record['compilerBeforeMedia']=inventory(work/'compile')
  ud=widget('radioUdisk')
  if ud['qt'].get('checked')!='true':op({'op':'invoke','hwnd':ud['hwnd'],'slot':'click'});time.sleep(.2)
  for name in ['clear_recipe_CB','clear_hisData_CB','clear_hisAlarm_CB','update_CB','isDownLoadWord']:
   x=widget(name)
   if x['qt'].get('checked')=='true':op({'op':'invoke','hwnd':x['hwnd'],'slot':'click'});time.sleep(.1)
   assert op({'op':'batch','hwnds':[x['hwnd']]})[0]['qt']['checked']=='false',name
  path=widget('edit_path');op({'op':'text','hwnd':path['hwnd'],'value':str(target)})
  rows=snapshot('verified-media-options');props={x['qt']['objectName']:x['qt'] for r in rows for x in r['widgets'] if x.get('qt')}
  assert props['radioUdisk']['checked']=='true' and props['radioUSB']['checked']=='false' and props['edit_path']['text']==str(target)
  assert all(props[n]['checked']=='false' for n in ['clear_recipe_CB','clear_hisData_CB','clear_hisAlarm_CB','update_CB','isDownLoadWord'])
  record['verifiedOptions']={n:props[n] for n in ['radioUdisk','radioUSB','edit_path','clear_recipe_CB','clear_hisData_CB','clear_hisAlarm_CB','update_CB','isDownLoadWord']}
  button=widget('m_downloadbtn');op({'op':'invoke','hwnd':button['hwnd'],'slot':'click'});record['localMediaWriteInvoked']=True
  deadline=time.time()+18;last=None;stable=0
  while time.time()<deadline:
   time.sleep(.4);files=inventory(target)
   if files:
    stable=stable+1 if files==last else 0;last=files
    if stable>=2:break
  rows=snapshot('after-media-write');record['mediaOutput']=inventory(target);record['compilerAfterMedia']=inventory(work/'compile');record['inputUnchangedDuringRun']=sha(source)==record['input']['sha256'];record['status']='media-output-observed' if record['mediaOutput'] else 'media-output-not-created'
 except StopIteration:pass
 except Exception as exc:record['status']='partial-or-error';record['error']=repr(exc)
 finally:
  if process:
   if win32process.GetExitCodeProcess(process)==259:win32process.TerminateProcess(process,0)
   win32event.WaitForSingleObject(process,5000);record['ownedProcessTerminated']=win32process.GetExitCodeProcess(process)!=259
  if session:
   try:session.detach()
   except Exception as e:record['detachError']=repr(e)
  if process:process.Close()
  if thread:thread.Close()
  if desk:u.CloseDesktop(desk)
  try:
   with winreg.OpenKey(winreg.HKEY_CURRENT_USER,r'Software\KinSealStudio\recent pro files',0,winreg.KEY_QUERY_VALUE|winreg.KEY_SET_VALUE) as key:
    recent,kind=winreg.QueryValueEx(key,'recentFileList');prefix=str(work).replace('\\','/').casefold()+'/';left=[x for x in recent if not x.replace('\\','/').casefold().startswith(prefix)]
    if kind==winreg.REG_MULTI_SZ and len(left)!=len(recent):winreg.SetValueEx(key,'recentFileList',0,kind,left)
    record['ownMruCleanup']={'removed':len(recent)-len(left),'otherEntriesPreserved':len(left)}
  except OSError as e:record['ownMruCleanup']={'error':repr(e)}
  record['endedEpoch']=time.time();write(out/'run.json',record)
 print(json.dumps({'run':args.run,'status':record['status'],'error':record.get('error'),'outputs':record.get('mediaOutput'),'out':str(out)},ensure_ascii=False),flush=True)
 return 0 if record['status'] in ['media-output-observed','upload-window-observed-no-transfer'] else 1
if __name__=='__main__':raise SystemExit(main())
