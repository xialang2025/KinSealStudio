"""Package and verify a committed independent cell-area polygon compiler with the actual bundled Qt runtime."""
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
assert read(work/'docs/stage5-evidence/polygon-windows-r1/run.json')['passed']
out=work/'docs/stage5-evidence'/('delivery-slice7-'+args.run);out.mkdir(exist_ok=False)
package=main/'dist'/('KinSealStudioLite-Stage5-compile-slice7-'+args.run);package.mkdir(exist_ok=False)
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
line=copy.deepcopy(rectangle);line['projectName']='普通直线默认样例';stroke=line['pages'][0]['objects'][0];stroke.update(type='line',id='stage5-line',name='直线1')
stroke['graphic']=dict(lineStyle=1,lineWidth=1,lineColor='#ff000000',alpha=255,endArrow=0,endCap=0,points=[[0,0],[1,1]]);write(samples/'line-default.ksp',line)
descending=copy.deepcopy(line);descending['projectName']='反斜率直线';descending['pages'][0]['objects'][0].update(width=250,height=200);descending['pages'][0]['objects'][0]['graphic']['points']=[[0,1],[1,0]];write(samples/'line-descending.ksp',descending)
tiny=copy.deepcopy(line);tiny['projectName']='原厂一像素对角行为';tiny['pages'][0]['objects'][0].update(width=1,height=1);write(samples/'line-one-pixel.ksp',tiny)
free=copy.deepcopy(line);free['projectName']='自由直线两端点默认样例';free_stroke=free['pages'][0]['objects'][0];free_stroke.update(type='freeline',id='stage5-freeline',name='自由直线1',width=180);write(samples/'freeline-default.ksp',free)
free_boundary=copy.deepcopy(free);free_boundary['projectName']='自由直线40像素边界';free_boundary['pages'][0]['objects'][0].update(width=40,height=1);free_boundary['pages'][0]['objects'][0]['graphic']['points']=[[0,0],[1,0]];write(samples/'freeline-boundary.ksp',free_boundary)
free_near=copy.deepcopy(free);free_near['projectName']='自由直线近屏边';free_near['pages'][0]['objects'][0].update(x=1,y=1,width=200);write(samples/'freeline-near-edge.ksp',free_near)
rounded=copy.deepcopy(rectangle);rounded['projectName']='圆角矩形默认样例';rounded_object=rounded['pages'][0]['objects'][0];rounded_object.update(type='roundrect',id='stage5-roundrect',name='圆角矩形1');rounded_object['graphic'].update(radiusX=10,radiusY=10);write(samples/'roundrect-default.ksp',rounded)
rounded_outline=copy.deepcopy(rounded);rounded_outline['projectName']='圆角矩形线框';rounded_outline['pages'][0]['objects'][0]['graphic']['fillStyle']=0;write(samples/'roundrect-outline.ksp',rounded_outline)
rounded_odd=copy.deepcopy(rounded);rounded_odd['projectName']='圆角矩形奇数尺寸';rounded_odd['pages'][0]['objects'][0].update(width=201,height=101);write(samples/'roundrect-odd.ksp',rounded_odd)
polygon=copy.deepcopy(rectangle);polygon['projectName']='多边形默认样例';polygon_object=polygon['pages'][0]['objects'][0];polygon_object.update(type='polygon',id='stage5-polygon',name='多边形1',height=120);polygon_object['graphic'].update(foreground='#ffffffff',background='#ff000000',points=[[0,0],[1,0],[.75,1]],endArrow=0,endCap=0);write(samples/'polygon-default.ksp',polygon)
polygon_outline=copy.deepcopy(polygon);polygon_outline['projectName']='多边形线框';polygon_outline['pages'][0]['objects'][0]['graphic']['fillStyle']=0;write(samples/'polygon-outline.ksp',polygon_outline)
polygon_concave=copy.deepcopy(polygon);polygon_concave['projectName']='凹五边形';polygon_concave['pages'][0]['objects'][0].update(height=130);polygon_concave['pages'][0]['objects'][0]['graphic']['points']=[[0,0],[1,0],[.55,50/130],[.9,1],[.1,1]];write(samples/'polygon-concave.ksp',polygon_concave)
mixed=read(work/'samples/stage5/mixed-nine.ksp');mixed['projectName']='十二类控件混合示例';item=copy.deepcopy(free_stroke);item.update(x=600,y=350,width=100,height=60);mixed['pages'][0]['objects'].append(item);rounded_mixed=copy.deepcopy(rounded_object);rounded_mixed.update(x=350,y=80);mixed['pages'][0]['objects'].append(rounded_mixed);polygon_mixed=copy.deepcopy(polygon_object);polygon_mixed.update(x=590,y=80,width=100,height=100);mixed['pages'][0]['objects'].append(polygon_mixed);write(samples/'mixed-twelve.ksp',mixed)
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
    for name,count in [('polygon-default',10),('polygon-outline',10),('polygon-concave',10),('roundrect-default',10),('roundrect-outline',10),('roundrect-odd',10),('table-default',10),('table-no-outline',10),('table-thirty',10),('mixed-twelve',12),('freeline-default',10),('freeline-boundary',10),('freeline-near-edge',10),('line-default',10),('line-descending',10),('line-one-pixel',10),('rectangle-default',10),('rectangle-outline',10),('orthogonal',10),('word-macro',14)]:
        source=samples/(name+'.ksp');target=compiled/name
        check(name+'-model',['--project-capabilities',str(source)]);check(name+'-compile',['--compile-project',str(source),str(target)]);check(name+'-verify',['--verify-compiled-package',str(target)])
        manifest=read(target/'compiler-manifest.json');assert len(manifest['files'])==count and manifest['deviceAcknowledged'] is False and manifest['usbAccessed'] is False
    check('polygon-source-gate',['--export-native-project',str(samples/'polygon-default.ksp'),str(out/'unpublished-polygon.sm')],20)
    check('polygon-legacy-gate',['--compile-single-text',str(samples/'polygon-default.ksp'),str(out/'unpublished-polygon-legacy')],19)
    bad_polygon=copy.deepcopy(polygon);bad_polygon['pages'][0]['objects'][0]['graphic']['points']=[[0,0],[1,1],[0,1],[1,0]];write(out/'unsupported-polygon-self-cross.ksp',bad_polygon)
    check('polygon-self-cross-gate',['--compile-project',str(out/'unsupported-polygon-self-cross.ksp'),str(out/'unpublished-polygon-self-cross')],19)
    for refused in ('unpublished-polygon.sm','unpublished-polygon-legacy','unpublished-polygon-self-cross'):assert not (out/refused).exists()
    check('roundrect-source-gate',['--export-native-project',str(samples/'roundrect-default.ksp'),str(out/'unpublished-roundrect.sm')],20)
    check('roundrect-legacy-gate',['--compile-single-text',str(samples/'roundrect-default.ksp'),str(out/'unpublished-roundrect-legacy')],19)
    bad_round=copy.deepcopy(rounded);bad_round['pages'][0]['objects'][0]['graphic']['radiusX']=20;write(out/'unsupported-roundrect-radius.ksp',bad_round)
    check('roundrect-radius-gate',['--compile-project',str(out/'unsupported-roundrect-radius.ksp'),str(out/'unpublished-roundrect-radius')],19)
    for refused in ('unpublished-roundrect.sm','unpublished-roundrect-legacy','unpublished-roundrect-radius'):assert not (out/refused).exists()
    check('capture',['--capture-preview',str(out/'main-window.png'),str(samples/'mixed-twelve.ksp')])
    check('source-gate',['--export-native-project',str(samples/'table-default.ksp'),str(out/'unpublished.sm')],20)
    check('legacy-gate',['--compile-single-text',str(samples/'table-default.ksp'),str(out/'unpublished-legacy')],19)
    check('line-source-gate',['--export-native-project',str(samples/'line-default.ksp'),str(out/'unpublished-line.sm')],20)
    check('line-legacy-gate',['--compile-single-text',str(samples/'line-default.ksp'),str(out/'unpublished-line-legacy')],19)
    bad_line=copy.deepcopy(line);bad_line['pages'][0]['objects'][0]['graphic']['lineWidth']=2;write(out/'unsupported-line-width.ksp',bad_line)
    check('unsupported-line-width',['--compile-project',str(out/'unsupported-line-width.ksp'),str(out/'unpublished-line-width')],19)
    check('freeline-source-gate',['--export-native-project',str(samples/'freeline-default.ksp'),str(out/'unpublished-freeline.sm')],20)
    check('freeline-legacy-gate',['--compile-single-text',str(samples/'freeline-default.ksp'),str(out/'unpublished-freeline-legacy')],19)
    bad_free=copy.deepcopy(free);bad_free['pages'][0]['objects'][0].update(width=1,height=1);write(out/'unsupported-freeline-small.ksp',bad_free)
    check('unsupported-freeline-small',['--compile-project',str(out/'unsupported-freeline-small.ksp'),str(out/'unpublished-freeline-small')],19)
    bad_free=copy.deepcopy(free);bad_free['pages'][0]['objects'][0].update(x=0,y=0);write(out/'unsupported-freeline-origin.ksp',bad_free)
    check('unsupported-freeline-origin',['--compile-project',str(out/'unsupported-freeline-origin.ksp'),str(out/'unpublished-freeline-origin')],19)
    bad_table=copy.deepcopy(table);bad_table['pages'][0]['objects'][0]['graphic']['alpha']=128;write(out/'unsupported-alpha.ksp',bad_table)
    check('unsupported-alpha',['--compile-project',str(out/'unsupported-alpha.ksp'),str(out/'unpublished-alpha')],19)
    check('unsupported-ellipse',['--compile-project',str(work/'samples/stage4/ellipse.ksp'),str(out/'unpublished-ellipse')],19)
    old=(compiled/'table-default/compiler-manifest.json').read_bytes();check('existing-target',['--compile-project',str(samples/'table-default.ksp'),str(compiled/'table-default')],19);assert (compiled/'table-default/compiler-manifest.json').read_bytes()==old
    bad=out/'bad-package';shutil.copytree(compiled/'table-default',bad);(bad/'SheetPixel.dat').write_bytes(b'corrupted');check('bad-package',['--verify-compiled-package',str(bad)],23)
    for name in ('text','bit','word','lamp','function'):
        for role in ('default','boundary'):
            source=work/'docs/stage5-evidence/baseline-cli-r1'/(name+'-'+role+'.ksp');target=out/'existing-five'/(name+'-'+role)
            check(name+'-'+role+'-compile',['--compile-project',str(source),str(target)]);check(name+'-'+role+'-verify',['--verify-compiled-package',str(target)])
    for name in ('unpublished.sm','unpublished-legacy','unpublished-alpha','unpublished-ellipse','unpublished-line.sm','unpublished-line-legacy','unpublished-line-width','unpublished-freeline.sm','unpublished-freeline-legacy','unpublished-freeline-small','unpublished-freeline-origin'):assert not (out/name).exists()
    qt=sorted({p for run in record['runs'] for p in run['modulePathsObserved'] if Path(p).name.lower().startswith('qt6') and p.lower().endswith('.dll')});assert qt and all(Path(p).resolve().is_relative_to(package.resolve()) for p in qt)
    record.update(passed=True,onlyBundledQtDllsObserved=True,qtDlls=qt)
finally:
    desktop.close();record['privateDesktopClosed']=True;write(out/'run.json',record)
shutil.copy2(work/'third_party/lzokay/LICENSE',package/'LZO-MIT-LICENSE.txt')
license_header=(work/'third_party/qrcodegen/qrcodegen.hpp').read_text(encoding='utf-8').split('*/',1)[0].removeprefix('/*')
(package/'QR-MIT-LICENSE.txt').write_text('\n'.join(line.strip().removeprefix('*').lstrip() for line in license_header.splitlines()).strip()+'\n',encoding='utf-8')
(package/'使用说明.txt').write_text('阶段5第七批：多边形、圆角矩形、两端点自由直线、普通直线、矩形、纯静态表格、正交折线及既有五类控件的受限独立离线编译。\n打开samples/mixed-twelve.ksp查看十二对象混合示例；compiled下为程序实际生成并验证的10/12/14段样例。\n多边形仅3至8点简单整数闭合形，X为2至698，Y为2至398，宽高至少100；黑边/白前景/黑背景、宽1、不透明；拒绝自交、重复、共线、相交及接触。\n圆角矩形支持半径10/10、X/Y至少1、宽高至少100、黑白/宽1/Alpha255、线型0/1×填充0/1，拒绝其它样式/半径/相交或接触；设备采用独立定点扫描。\n自由直线仅两端点，主轴至少40、X/Y至少1，其余黑实线/宽1/Alpha255/无箭头端点0/不重叠；多点采样/小尺寸/场景变换待恢复。\n直线支持黑实线/宽1/Alpha255/无箭头端点0/屏内整数两端点且不重叠；保存重开原厂1×1对角不着色，斜折线及其它样式继续拒绝。\n矩形支持宽高至少100、黑白、宽1、Alpha255、线型0/1与填充0/1，拒绝小尺寸/其它样式/重叠或接触。\n表格支持1..30行列、整数像素单元格、最小40×40、不透明#e0e0e0背景、黑色内部线/外框、无线或实线和全部网格开关，不支持重叠或边缘接触。其它颜色/Alpha/虚线/非整除尺寸保持拒绝。\n--compile-project 工程.ksp 新输出目录；--verify-compiled-package 输出目录。旧目录不覆盖。完整SM、新动态类型、多页及活动配置仍待恢复。\n本批没有访问USB通信，deviceAcknowledged=false；阶段6验证传输，阶段10验证显示/触控。\n',encoding='utf-8')
manifest=dict(sourceCommit=commit,package=str(package),executableSha256=sha(exe),checksPassed=len(record['runs']),completeStage5=False,usbAccessed=False,files=[dict(path=str(p.relative_to(package)).replace('\\','/'),bytes=p.stat().st_size,sha256=sha(p)) for p in sorted(package.rglob('*')) if p.is_file()]);write(out/'package-manifest.json',manifest)
repro=out/'reproduction';repro.mkdir();shutil.copy2(root/'package_slice7.py',repro/'package_slice7.py')
for path in samples.glob('*.ksp'):
    if path.name not in ('orthogonal.ksp','word-macro.ksp'):shutil.copy2(path,work/'samples/stage5'/path.name)
print(json.dumps(dict(sourceCommit=commit,package=str(package),checksPassed=len(record['runs']),onlyBundledQt=True),ensure_ascii=False))
