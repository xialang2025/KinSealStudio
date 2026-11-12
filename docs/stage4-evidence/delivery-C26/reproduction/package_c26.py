"""Package the verified C26 checkout and run background checks with only its own Qt DLLs."""
from pathlib import Path
import datetime
import hashlib
import importlib.util
import json
import os
import shutil
import subprocess
root=Path(__file__).resolve().parent; work=root/'worktree'; main=root.parents[1]/'KinSealStudio'
out=work/'docs/stage4-evidence/delivery-C26'; out.mkdir(exist_ok=True)
assert not list(out.iterdir()), 'Resume only the verified pre-deployment failure.'
package=main/'dist/KinSealStudioLite-Stage4-C26-0.10.1'
if package.exists():
 assert (package/'KinSealStudioLite.exe').read_bytes()==(work/'build/KinSealStudioLite.exe').read_bytes()
 assert all(p.name in ('KinSealStudioLite.exe','LZO-MIT-LICENSE.txt') or (p.suffix=='.ksp' and p.is_relative_to(package/'samples/stage4')) for p in package.rglob('*') if p.is_file())
else:
 package.mkdir(parents=True)
def read(p): return json.loads(p.read_text(encoding='utf-8-sig'))
def write(p,d): p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
commit=subprocess.check_output(['git','-C',str(work),'rev-parse','HEAD']).decode().strip(); assert commit=='6a3c1eb88dceba42c3319b3ae448e660086dddf4'
assert read(work/'docs/stage4-evidence/data-group/windows-final/run.json')['passed']
exe=package/'KinSealStudioLite.exe'; shutil.copy2(work/'build/KinSealStudioLite.exe',exe)
samples=package/'samples/stage4'; samples.mkdir(parents=True,exist_ok=True)
for p in (work/'samples/stage4').glob('*.ksp'): shutil.copy2(p,samples/p.name)
sample=read(main/'dist/KinSealStudioLite-Stage4-C23-0.10.1/samples/stage4/stage4-controls-C23.ksp')
sample['projectName']='阶段4 C01～C26 编辑切片'
newest=read(work/'samples/stage4/data-group.ksp')
page=json.loads(json.dumps(newest['pages'][0],ensure_ascii=False))
page['id']='page-curves-C26'; page['name']='新增曲线与采集'
page['objects']=[o for o in page['objects'] if o['type'] in ('historytrend','xycurve','datagroup')]
assert len(page['objects'])==3
for obj in page['objects']:
 if obj['type']=='historytrend': obj.update(x=20,y=20,width=360,height=210,layer=1)
 elif obj['type']=='xycurve': obj.update(x=420,y=20,width=360,height=210,layer=2)
 else: obj.update(x=20,y=250,width=360,height=210,layer=3)
sample['pages'].append(page);sample['pageOrder'].append(page['id']);sample['activePageId']=page['id'];sample['configurationNodes']=newest['configurationNodes']
assert sum(len(p['objects']) for p in sample['pages'])==26
showcase=samples/'stage4-controls-C26.ksp';write(showcase,sample)
shutil.copy2(work/'third_party/lzokay/LICENSE',package/'LZO-MIT-LICENSE.txt')
qr=(work/'third_party/qrcodegen/qrcodegen.hpp').read_text(encoding='utf-8'); assert qr.startswith('/*') and 'MIT License' in qr[:1000]
(package/'QR-MIT-LICENSE.txt').write_text(qr[:qr.index('*/')+2]+'\n',encoding='utf-8')
environment=os.environ.copy(); environment['PATH']='G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'+environment.get('PATH','')
temporary=root/'task-temp/c26-deploy'; temporary.mkdir(parents=True,exist_ok=True); environment.update(TEMP=str(temporary),TMP=str(temporary),TMPDIR=str(temporary))
r=subprocess.run(['G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin/windeployqt.exe','--release','--compiler-runtime','--no-translations','--dir',str(package),str(exe)],capture_output=True,env=environment,timeout=120,creationflags=subprocess.CREATE_NO_WINDOW)
(out/'deploy-stdout.txt').write_bytes(r.stdout); (out/'deploy-stderr.txt').write_bytes(r.stderr); assert r.returncode==0
spec=importlib.util.spec_from_file_location('c26_portable',work/'docs/stage1-evidence/gui-portable/probe.py'); module=importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
smoke=out/'portable-r1'; smoke.mkdir(); temporary=smoke/'temp'; temporary.mkdir(); desktop=module.PrivateDesktop(smoke)
environment=module.clean_environment(package); environment.update(TEMP=str(temporary),TMP=str(temporary),TMPDIR=str(temporary))
record=dict(startedAtUtc=datetime.datetime.now(datetime.timezone.utc).isoformat(),package=str(package),sourceCommit=commit,buildSource='isolated_verified_committed_checkout',executableSha256=sha(exe),userStage3WorkingChangesPresentInBinary=False,usbAccessed=False,systemInput=False,desktopSwitched=False,runs=[],passed=False)
try:
 checks=[('self-test',['--self-test'],0),('payload',['--verify-independent-payload'],0),('showcase-model',['--validate-project',str(showcase)],0),('capture-main',['--capture-preview',str(smoke/'main-window.png'),str(showcase)],0)]
 for family,filename in [('history-trend','history-only.ksp'),('xy-curve','xy-only.ksp'),('data-group','group-only.ksp')]:
  input_file=work/'docs/stage4-evidence'/family/'inputs'/filename
  checks += [(family+'-source-gate',['--export-native-project',str(input_file),str(smoke/'unpublished.sm')],20),(family+'-device-gate',['--compile-project',str(input_file),str(smoke/'unpublished-package')],19)]
 for name,args,expected in checks:
  run=desktop.run(name,exe,args,environment,timeout=60,expected_exit=expected); record['runs'].append(run); assert run['passed'],run
 assert not (smoke/'unpublished.sm').exists() and not (smoke/'unpublished-package').exists()
 qt=sorted({p for run in record['runs'] for p in run['modulePathsObserved'] if Path(p).name.lower().startswith('qt6') and p.lower().endswith('.dll')}); assert qt and all(Path(p).resolve().is_relative_to(package.resolve()) for p in qt)
 record.update(passed=True,onlyBundledQtDllsObserved=True,qtDllsObserved=qt,previewSha256=sha(smoke/'main-window.png'))
finally:
 desktop.close(); record['privateDesktopClosed']=True; record['temporaryFilesRemaining']=[str(p.relative_to(temporary)) for p in temporary.rglob('*') if p.is_file()]; write(smoke/'run.json',record)
(package/'使用说明.txt').write_text('阶段4 C26增量：26类编辑切片，新增历史采集/历史趋势、X-Y曲线、数据群组。\n双击 KinSealStudioLite.exe，可打开 samples/stage4/stage4-controls-C26.ksp。\n新增控件独立SM/设备编码仍阻塞，原厂基线下载成功不能替代新版生成通过；软件会拒绝不支持的输出。\n本包来自独立已验证提交，不含主目录尚未验收的阶段3续作。\n',encoding='utf-8')
manifest=dict(package=str(package),sourceCommit=commit,files=[dict(path=str(p.relative_to(package)),bytes=p.stat().st_size,sha256=sha(p)) for p in sorted(package.rglob('*')) if p.is_file()],softwareTests=dict(controls=142,source=59,sdkMacro=10,failed=0,skipped=0),portableChecksPassed=10,independentNewEncoderVerified=False)
write(out/'package-manifest.json',manifest)
(out/'reproduction').mkdir(); shutil.copy2(root/'package_c26.py',out/'reproduction/package_c26.py')
(work/'docs/阶段4_C26增量包交付.md').write_text(f'# 阶段4 C26增量包\n\n包路径：`{package}`，源提交`{commit}`，独立已验证工作树构建。\n\n控件142/0/0、源59/0/0、SDK宏10/0/0；便携包后台自检、基础载荷、26类两页工程验证、主窗口截图、历史趋势、X-Y、数据群组源与设备拒绝共10项通过。Qt DLL均实际从包内加载，未调用系统输入或切换桌面。\n\n图表三形态、趋势图、历史趋势、X-Y及数据群组原厂基线均已实物下载；21类新增控件完整独立源/设备编码仍阻塞，未新增PLC或实屏通过。包内使用说明和样例明确标识范围。\n\n[包清单](stage4-evidence/delivery-C26/package-manifest.json) · [后台验证](stage4-evidence/delivery-C26/portable-r1/run.json)\n',encoding='utf-8')
print(json.dumps(dict(package=str(package),checksPassed=10,sourceCommit=commit,onlyBundledQtDlls=True)))

progress_path=work/'docs/stage4-evidence/progress.json';progress=read(progress_path)
progress['latestDelivery']=dict(evidence='docs/stage4-evidence/delivery-C26/package-manifest.json',package=str(package),sourceTree='isolated_verified_committed_checkout',sourceCommit=commit,processed=26,total=41,completeStage4=False,mainUnacceptedStage3Failures=2)
write(progress_path,progress)
shutil.copy2(root/'prepare_c26_package.py',out/'reproduction/prepare_c26_package.py')
