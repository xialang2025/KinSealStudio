"""Package and verify a committed static-table compiler with the actual bundled Qt runtime."""
from pathlib import Path
import argparse
import copy
import hashlib
import importlib.util
import json
import os
import shutil
import subprocess

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';main=root.parents[1]/'KinSealStudio'
parser=argparse.ArgumentParser();parser.add_argument('--run',default='r1');parser.add_argument('--expected-commit',required=True);args=parser.parse_args()
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
commit=subprocess.check_output(['git','-C',str(work),'rev-parse','HEAD']).decode().strip();assert commit==args.expected_commit
assert not subprocess.check_output(['git','-C',str(work),'status','--porcelain','--','src','tests','CMakeLists.txt']).strip()
assert read(work/'docs/stage5-evidence/rect-windows-r1/run.json')['passed']
out=work/'docs/stage5-evidence'/('delivery-slice3-'+args.run);out.mkdir(exist_ok=False)
package=main/'dist'/('KinSealStudioLite-Stage5-compile-slice3-'+args.run);package.mkdir(exist_ok=False)
exe=package/'KinSealStudioLite.exe';shutil.copy2(work/'build/KinSealStudioLite.exe',exe)
samples=package/'samples';samples.mkdir();compiled=package/'compiled';compiled.mkdir()
table=read(work/'samples/stage4/table.ksp');table['projectName']='静态表格默认样例'
obj=table['pages'][0]['objects'][0];obj.update(id='stage5-table',name='表格1',text='',address='',x=100,y=100,width=200,height=100)
obj['graphic']=dict(rows=4,columns=4,horizontal=True,vertical=True,outline=True,lineStyle=1,lineColor='#ff000000',frameColor='#ff000000',background='#ffe0e0e0',alpha=255)
write(samples/'table-default.ksp',table)
open_table=copy.deepcopy(table);open_table['projectName']='无外框静态表格';open_table['pages'][0]['objects'][0]['graphic']['outline']=False;write(samples/'table-no-outline.ksp',open_table)
large=copy.deepcopy(table);large['projectName']='30行30列表格';large['pages'][0]['objects'][0].update(x=0,y=0,width=600,height=300);large['pages'][0]['objects'][0]['graphic'].update(rows=30,columns=30);write(samples/'table-thirty.ksp',large)
rectangle=copy.deepcopy(table);rectangle['projectName']='矩形默认样例';rect=rectangle['pages'][0]['objects'][0];rect.update(type='rect',id='stage5-rectangle',name='矩形1')
rect['graphic']=dict(lineStyle=1,lineWidth=1,lineColor='#ff000000',alpha=255,fillStyle=1,foreground='#ff000000',background='#ffffffff');write(samples/'rectangle-default.ksp',rectangle)
outline=copy.deepcopy(rectangle);outline['projectName']='矩形线框样例';outline['pages'][0]['objects'][0]['graphic']['fillStyle']=0;write(samples/'rectangle-outline.ksp',outline)
mixed=read(work/'samples/stage5/mixed-seven.ksp');mixed['projectName']='八类控件混合示例';item=copy.deepcopy(rect);item.update(x=100,y=300);mixed['pages'][0]['objects'].append(item);write(samples/'mixed-eight.ksp',mixed)
for name in ('orthogonal','word-macro'):shutil.copy2(work/'samples/stage5'/(name+'.ksp'),samples/(name+'.ksp'))
environment=os.environ.copy();environment['PATH']='G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'+environment.get('PATH','')
environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
deploy=subprocess.run(['G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin/windeployqt.exe','--release','--compiler-runtime','--no-translations','--dir',str(package),str(exe)],capture_output=True,env=environment,timeout=120,creationflags=subprocess.CREATE_NO_WINDOW)
(out/'deploy-stdout.txt').write_bytes(deploy.stdout);(out/'deploy-stderr.txt').write_bytes(deploy.stderr);assert deploy.returncode==0
spec=importlib.util.spec_from_file_location('slice2_private',work/'docs/stage1-evidence/gui-portable/probe.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
desktop=module.PrivateDesktop(out);temp=out/'temp';temp.mkdir();environment=module.clean_environment(package);environment.update(TEMP=str(temp),TMP=str(temp),TMPDIR=str(temp),KINSEAL_ARM_SDK='G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed/script')
record=dict(sourceCommit=commit,executableSha256=sha(exe),package=str(package),runs=[],usbAccessed=False,systemInput=False,desktopSwitched=False,passed=False)
def check(name,arguments,expected=0):
    result=desktop.run(name,exe,arguments,environment,timeout=90,expected_exit=expected);record['runs'].append(result)
    assert result['passed'],result
    return result
try:
    check('self-test',['--self-test']);check('fixed-payload',['--verify-independent-payload'])
    for name,count in [('table-default',10),('table-no-outline',10),('table-thirty',10),('mixed-eight',12),('rectangle-default',10),('rectangle-outline',10),('orthogonal',10),('word-macro',14)]:
        source=samples/(name+'.ksp');target=compiled/name
        check(name+'-model',['--project-capabilities',str(source)]);check(name+'-compile',['--compile-project',str(source),str(target)]);check(name+'-verify',['--verify-compiled-package',str(target)])
        manifest=read(target/'compiler-manifest.json');assert len(manifest['files'])==count and manifest['deviceAcknowledged'] is False and manifest['usbAccessed'] is False
    check('capture',['--capture-preview',str(out/'main-window.png'),str(samples/'mixed-eight.ksp')])
    check('source-gate',['--export-native-project',str(samples/'table-default.ksp'),str(out/'unpublished.sm')],20)
    check('legacy-gate',['--compile-single-text',str(samples/'table-default.ksp'),str(out/'unpublished-legacy')],19)
    bad_table=copy.deepcopy(table);bad_table['pages'][0]['objects'][0]['graphic']['alpha']=128;write(out/'unsupported-alpha.ksp',bad_table)
    check('unsupported-alpha',['--compile-project',str(out/'unsupported-alpha.ksp'),str(out/'unpublished-alpha')],19)
    check('unsupported-ellipse',['--compile-project',str(work/'samples/stage4/ellipse.ksp'),str(out/'unpublished-ellipse')],19)
    old=(compiled/'table-default/compiler-manifest.json').read_bytes();check('existing-target',['--compile-project',str(samples/'table-default.ksp'),str(compiled/'table-default')],19);assert (compiled/'table-default/compiler-manifest.json').read_bytes()==old
    bad=out/'bad-package';shutil.copytree(compiled/'table-default',bad);(bad/'SheetPixel.dat').write_bytes(b'corrupted');check('bad-package',['--verify-compiled-package',str(bad)],23)
    for name in ('text','bit','word','lamp','function'):
        for role in ('default','boundary'):
            source=work/'docs/stage5-evidence/baseline-cli-r1'/(name+'-'+role+'.ksp');target=out/'existing-five'/(name+'-'+role)
            check(name+'-'+role+'-compile',['--compile-project',str(source),str(target)]);check(name+'-'+role+'-verify',['--verify-compiled-package',str(target)])
    for name in ('unpublished.sm','unpublished-legacy','unpublished-alpha','unpublished-ellipse'):assert not (out/name).exists()
    qt=sorted({p for run in record['runs'] for p in run['modulePathsObserved'] if Path(p).name.lower().startswith('qt6') and p.lower().endswith('.dll')});assert qt and all(Path(p).resolve().is_relative_to(package.resolve()) for p in qt)
    record.update(passed=True,onlyBundledQtDllsObserved=True,qtDlls=qt)
finally:
    desktop.close();record['privateDesktopClosed']=True;write(out/'run.json',record)
shutil.copy2(work/'third_party/lzokay/LICENSE',package/'LZO-MIT-LICENSE.txt')
license_header=(work/'third_party/qrcodegen/qrcodegen.hpp').read_text(encoding='utf-8').split('*/',1)[0].removeprefix('/*')
(package/'QR-MIT-LICENSE.txt').write_text('\n'.join(line.strip().removeprefix('*').lstrip() for line in license_header.splitlines()).strip()+'\n',encoding='utf-8')
(package/'使用说明.txt').write_text('阶段5第三批：矩形、纯静态表格、正交折线及既有五类控件的受限独立离线编译。\n打开samples/mixed-eight.ksp查看八对象混合示例；compiled下为程序实际生成并验证的10/12/14段样例。\n矩形支持宽高至少100、黑白、宽1、Alpha255、线型0/1与填充0/1，拒绝小尺寸/其它样式/重叠或接触。\n表格支持1..30行列、整数像素单元格、最小40×40、不透明#e0e0e0背景、黑色内部线/外框、无线或实线和全部网格开关，不支持重叠或边缘接触。其它颜色/Alpha/虚线/非整除尺寸保持拒绝。\n--compile-project 工程.ksp 新输出目录；--verify-compiled-package 输出目录。旧目录不覆盖。完整SM、新动态类型、多页及活动配置仍待恢复。\n本批没有访问USB通信，deviceAcknowledged=false；阶段6验证传输，阶段10验证显示/触控。\n',encoding='utf-8')
manifest=dict(sourceCommit=commit,package=str(package),executableSha256=sha(exe),checksPassed=len(record['runs']),completeStage5=False,usbAccessed=False,files=[dict(path=str(p.relative_to(package)).replace('\\','/'),bytes=p.stat().st_size,sha256=sha(p)) for p in sorted(package.rglob('*')) if p.is_file()]);write(out/'package-manifest.json',manifest)
repro=out/'reproduction';repro.mkdir();shutil.copy2(root/'package_slice3.py',repro/'package_slice3.py')
for path in samples.glob('*.ksp'):
    if path.name not in ('orthogonal.ksp','word-macro.ksp'):shutil.copy2(path,work/'samples/stage5'/path.name)
print(json.dumps(dict(sourceCommit=commit,package=str(package),checksPassed=len(record['runs']),onlyBundledQt=True),ensure_ascii=False))
