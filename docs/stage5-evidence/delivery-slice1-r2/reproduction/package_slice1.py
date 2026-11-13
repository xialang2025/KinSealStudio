"""Package the committed offline compiler slice and exercise actual bundled-runtime CLI operations."""
from pathlib import Path
import copy
import hashlib
import importlib.util
import json
import os
import shutil
import subprocess
import argparse

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';main=root.parents[1]/'KinSealStudio'
parser=argparse.ArgumentParser();parser.add_argument('--run',default='r2');args=parser.parse_args()
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
commit=subprocess.check_output(['git','-C',str(work),'rev-parse','HEAD']).decode().strip()
assert commit=='34a0c4b799859e6bc92086ac446aa24b3ef81691'
assert not subprocess.check_output(['git','-C',str(work),'status','--porcelain','--','src','tests','CMakeLists.txt']).strip()
assert read(work/'docs/stage5-evidence/windows-r5/run.json')['passed']
out=work/'docs/stage5-evidence'/('delivery-slice1-'+args.run);out.mkdir(exist_ok=False)
package=main/'dist'/('KinSealStudioLite-Stage5-compile-slice1-'+args.run);package.mkdir(exist_ok=False)
exe=package/'KinSealStudioLite.exe';shutil.copy2(work/'build/KinSealStudioLite.exe',exe)
samples=package/'samples';samples.mkdir();compiled=package/'compiled';compiled.mkdir()
project=read(work/'samples/stage4/polyline.ksp');project['projectName']='正交折线示例'
obj=project['pages'][0]['objects'][0];obj.update(id='stage5-orthogonal',name='折线1',text='',address='',x=100,y=100,width=150,height=100)
obj['graphic']=dict(lineStyle=1,lineWidth=1,lineColor='#ff000000',alpha=255,endArrow=0,endCap=0,points=[[0,0],[1,0],[1,1]])
write(samples/'orthogonal.ksp',project)
seed=read(work/'samples/stage4/stage4-editor-40-slices.ksp');mixed=copy.deepcopy(project);mixed['projectName']='六类控件混合示例';mixed['languages']=seed['languages'];mixed['pictures']=seed['pictures'];mixed['macros']=seed['macros'];mixed['connections']=seed.get('connections',[])
families=set()
for page in seed['pages']:
    for item in page['objects']:
        if item['type'] not in ('text','switch'):continue
        family='text' if item['type']=='text' else 'word' if 'wordButton' in item else 'lamp' if 'lamp' in item else 'function' if 'functionButton' in item else 'bit'
        if family in families:continue
        item=copy.deepcopy(item);item.update(id='stage5-existing-'+family,x=20+len(families)*130,y=20,width=100,height=50);families.add(family);mixed['pages'][0]['objects'].append(item)
assert len(families)==5 and len(mixed['pages'][0]['objects'])==6
write(samples/'mixed-six.ksp',mixed)
shutil.copy2(work/'samples/word-button/字按钮宏绑定.ksp',samples/'word-macro.ksp')
environment=os.environ.copy();environment['PATH']='G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'+environment.get('PATH','')
environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
deploy=subprocess.run(['G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin/windeployqt.exe','--release','--compiler-runtime','--no-translations','--dir',str(package),str(exe)],capture_output=True,env=environment,timeout=120,creationflags=subprocess.CREATE_NO_WINDOW)
(out/'deploy-stdout.txt').write_bytes(deploy.stdout);(out/'deploy-stderr.txt').write_bytes(deploy.stderr);assert deploy.returncode==0
spec=importlib.util.spec_from_file_location('slice1_private',work/'docs/stage1-evidence/gui-portable/probe.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
desktop=module.PrivateDesktop(out);temp=out/'temp';temp.mkdir();environment=module.clean_environment(package);environment.update(TEMP=str(temp),TMP=str(temp),TMPDIR=str(temp),KINSEAL_ARM_SDK='G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed/script')
record=dict(sourceCommit=commit,executableSha256=sha(exe),package=str(package),runs=[],usbAccessed=False,systemInput=False,desktopSwitched=False,passed=False)
def check(name,args,expected=0):
    result=desktop.run(name,exe,args,environment,timeout=90,expected_exit=expected);record['runs'].append(result)
    assert result['passed'],result
    return result
try:
    check('self-test',['--self-test']);check('fixed-payload',['--verify-independent-payload'])
    for name,count in [('orthogonal',10),('mixed-six',12),('word-macro',14)]:
        path=samples/(name+'.ksp');target=compiled/name
        check(name+'-model',['--project-capabilities',str(path)])
        check(name+'-compile',['--compile-project',str(path),str(target)])
        check(name+'-verify',['--verify-compiled-package',str(target)])
        m=read(target/'compiler-manifest.json');assert len(m['files'])==count and m['deviceAcknowledged'] is False and m['usbAccessed'] is False
    check('capture',['--capture-preview',str(out/'main-window.png'),str(samples/'mixed-six.ksp')])
    check('source-gate',['--export-native-project',str(samples/'orthogonal.ksp'),str(out/'unpublished.sm')],20)
    check('legacy-gate',['--compile-single-text',str(samples/'orthogonal.ksp'),str(out/'unpublished-legacy')],19)
    check('unsupported-table',['--compile-project',str(work/'samples/stage4/table.ksp'),str(out/'unpublished-table')],19)
    old=(compiled/'orthogonal/compiler-manifest.json').read_bytes()
    check('existing-target',['--compile-project',str(samples/'orthogonal.ksp'),str(compiled/'orthogonal')],19)
    assert (compiled/'orthogonal/compiler-manifest.json').read_bytes()==old
    bad=out/'bad-package';shutil.copytree(compiled/'orthogonal',bad);(bad/'SheetPixel.dat').write_bytes(b'corrupted')
    check('bad-package',['--verify-compiled-package',str(bad)],23)
    for name in ('text','bit','word','lamp','function'):
        for role in ('default','boundary'):
            path=work/'docs/stage5-evidence/baseline-cli-r1'/(name+'-'+role+'.ksp');target=out/'existing-five'/(name+'-'+role)
            check(name+'-'+role+'-compile',['--compile-project',str(path),str(target)])
            check(name+'-'+role+'-verify',['--verify-compiled-package',str(target)])
    assert not (out/'unpublished.sm').exists() and not (out/'unpublished-legacy').exists() and not (out/'unpublished-table').exists()
    qt=sorted({p for run in record['runs'] for p in run['modulePathsObserved'] if Path(p).name.lower().startswith('qt6') and p.lower().endswith('.dll')})
    assert qt and all(Path(p).resolve().is_relative_to(package.resolve()) for p in qt)
    record.update(passed=True,onlyBundledQtDllsObserved=True,qtDlls=qt)
finally:
    desktop.close();record['privateDesktopClosed']=True;write(out/'run.json',record)
shutil.copy2(work/'third_party/lzokay/LICENSE',package/'LZO-MIT-LICENSE.txt')
(package/'使用说明.txt').write_text('阶段5首批：新增正交折线的独立离线编译与安全设备包发布。\n打开samples/orthogonal.ksp或mixed-six.ksp查看示例；compiled下分别为10/12/14段纯离线样例。\n支持折线实线/宽1/不透明黑色/无箭头端点0/整数水平垂直线段/不与其它对象重叠。完整SM、其它新类型、多页及活动配置仍拒绝。\n--compile-project 工程.ksp 新输出目录；--verify-compiled-package 输出目录。旧目录不覆盖。\n本批没有访问USB通信，deviceAcknowledged=false；阶段6验证新产物传输，阶段10验证显示/触控。原厂非确定字节没有忽略。\n本程序来自已验证独立提交，不含主目录未验收的阶段3修改。\n',encoding='utf-8')
manifest=dict(sourceCommit=commit,package=str(package),executableSha256=sha(exe),checksPassed=len(record['runs']),completeStage5=False,usbAccessed=False,
    files=[dict(path=str(p.relative_to(package)).replace('\\','/'),bytes=p.stat().st_size,sha256=sha(p)) for p in sorted(package.rglob('*')) if p.is_file()])
write(out/'package-manifest.json',manifest)
repro=out/'reproduction';repro.mkdir();shutil.copy2(root/'package_slice1.py',repro/'package_slice1.py')
sample_out=work/'samples/stage5';sample_out.mkdir(exist_ok=True)
for path in samples.glob('*.ksp'):shutil.copy2(path,sample_out/path.name)
print(json.dumps(dict(sourceCommit=commit,package=str(package),checksPassed=len(record['runs']),onlyBundledQt=True),ensure_ascii=False))
