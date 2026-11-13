"""Package the verified Stage4 iteration with forty editor slices and an explicit curve-geometry blocker."""
from pathlib import Path
import argparse
import copy
import datetime
import hashlib
import importlib.util
import json
import os
import shutil
import subprocess
root=Path(__file__).resolve().parent;work=root/'worktree';main=root.parents[1]/'KinSealStudio'
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def write(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
parser=argparse.ArgumentParser();parser.add_argument('--commit',required=True);parser.add_argument('--test-run',required=True);parser.add_argument('--run',default='final-r1');args=parser.parse_args()
commit=subprocess.check_output(['git','-C',str(work),'rev-parse','HEAD']).decode().strip();assert commit==args.commit
assert not subprocess.check_output(['git','-C',str(work),'status','--porcelain','--','src','tests','CMakeLists.txt']).strip()
tests=read(work/args.test_run);assert tests['passed'] and all(x['totals']['failed']==x['totals']['skipped']==0 for x in tests['runs']);totals={x['name']:x['totals']['passed'] for x in tests['runs']};assert totals['KinSealNativeSourceTests']==59 and totals['KinSealDeviceMacroTests']==10;assert sha(work/'build/KinSealStudioLite.exe')==read(work/'docs/stage4-evidence/table/backend-gates.json')['executableSha256']
out=work/'docs/stage4-evidence'/('delivery-'+args.run);out.mkdir(exist_ok=False)
package=main/'dist'/('KinSealStudioLite-Stage4-41items-0.10.1-'+args.run);package.mkdir(parents=True,exist_ok=False)
exe=package/'KinSealStudioLite.exe';shutil.copy2(work/'build/KinSealStudioLite.exe',exe)
samples=package/'samples/stage4';samples.mkdir(parents=True)
for path in (work/'samples/stage4').glob('*.ksp'):shutil.copy2(path,samples/path.name)
sample=read(main/'dist/KinSealStudioLite-Stage4-C26-0.10.1/samples/stage4/stage4-controls-C26.ksp')
sample['projectName']='阶段4：40项编辑切片；曲线圆弧几何未恢复'
alarms=read(work/'samples/stage4/history-alarm.ksp');alarm_page=copy.deepcopy(alarms['pages'][0]);alarm_page.update(id='page-alarms-C31',name='报警与报警表')
assert len(alarm_page['objects'])==3
for obj in alarm_page['objects']:
    if obj['type']=='alarmbar':obj.update(x=20,y=20,width=760,height=60,layer=1)
    elif obj['type']=='alarmdisplay':obj.update(x=20,y=110,width=370,height=340,layer=2)
    elif obj['type']=='historyalarm':obj.update(x=410,y=110,width=370,height=340,layer=3)
    else:raise AssertionError(obj['type'])
sample['pages'].append(alarm_page);sample['pageOrder'].append(alarm_page['id'])
for node in alarms['configurationNodes']:
    if node['kind']=='alarm':sample['configurationNodes'].append(copy.deepcopy(node))
history=read(work/'docs/stage4-evidence/history-data/inputs/history-data-only.ksp');recipe=read(work/'docs/stage4-evidence/recipe/inputs/recipe-only.ksp')
# Retain imported caption languages in the showcase rather than deleting the source translations.
sample['languages']=list(dict.fromkeys(sample['languages']+alarms['languages']+history['languages']+recipe['languages']))
assert len(sample['languages'])<=8
for obj in alarm_page['objects']:
    if 'alarmDisplay' in obj:
        captions=obj['alarmDisplay']['captions'];obj['alarmDisplay']['captions']={language:copy.deepcopy(captions.get(language,captions['中文'])) for language in sample['languages']}
for node in sample['configurationNodes']:
    if node['kind']=='alarm':
        for row in node['properties']['rows']:
            row['messages']={language:row['messages'].get(language,'') for language in sample['languages']}
history_obj=next(copy.deepcopy(obj) for page in history['pages'] for obj in page['objects'] if obj['type']=='historydata')
recipe_obj=next(copy.deepcopy(obj) for page in recipe['pages'] for obj in page['objects'] if obj['type']=='recipe')
collector=next(node for node in sample['configurationNodes'] if node['kind']=='dataCollection');items=collector['properties']['items'];assert len(items)==len(history_obj['historyData']['columns'])
history_obj['historyData']['collectionId']=collector['id']
for column,item in zip(history_obj['historyData']['columns'],items):column['sourceItemId']=item['id']
captions=history_obj['historyData']['captions'];history_obj['historyData']['captions']={language:copy.deepcopy(captions.get(language,captions['中文'])) for language in sample['languages']}
for column in history_obj['historyData']['columns']:
    column['names']={language:column['names'].get(language,column['names'].get('中文','')) for language in sample['languages']};column['shown']=True
history_obj['historyData'].update(rows=5,showDate=True,showTime=True);history_obj.update(x=20,y=20,width=760,height=200,layer=1);recipe_obj.update(x=20,y=250,width=760,height=210,layer=2)
data_page=copy.deepcopy(recipe['pages'][0]);data_page.update(id='page-tables-C31',name='历史数据与配方',objects=[history_obj,recipe_obj]);sample['pages'].append(data_page);sample['pageOrder'].append(data_page['id']);sample['activePageId']=data_page['id']
sample['configurationNodes'].extend(copy.deepcopy(node) for node in recipe['configurationNodes'] if node['kind']=='recipe')
assert sum(len(page['objects']) for page in sample['pages'])==31
families=['line','polyline','freeline','rect','roundrect','ellipse','polygon','sector','table']
names={'line':'直线','polyline':'折线','freeline':'自由直线首尾切片','rect':'矩形','roundrect':'圆角矩形','ellipse':'圆与椭圆','polygon':'多边形','sector':'扇形默认角度','table':'纯图像表格'}
for group,title in [(families[:3],'开放线条'),(families[3:],'闭合图形与表格')]:
    page=copy.deepcopy(recipe['pages'][0]);page.update(id='page-final-'+group[0],name=title,objects=[])
    for index,family in enumerate(group):
        source=read(work/'samples/stage4'/(family+'.ksp'));obj=next(copy.deepcopy(obj) for candidate in source['pages'] for obj in candidate['objects'] if obj['type']==family)
        obj.update(id='final-'+family,name=names[family],layer=index+1,text='',address='')
        if len(group)==3:obj.update(x=80,y=40+index*140,width=520,height=80)
        else:obj.update(x=20+(index%3)*260,y=30+(index//3)*230,width=200,height=200 if family=='sector' else 140)
        g=obj['graphic'];g['lineStyle']=0 if family=='sector' else 1;g['alpha']=255
        if 'lineWidth' in g:g['lineWidth']=1
        if 'endArrow' in g:g['endArrow']=0;g['endCap']=0
        if family in ['line','freeline']:g['points']=[[0,0],[1,1]]
        if family=='polyline':g['points']=[[0,0],[1,0],[1,1]]
        if family=='polygon':g['points']=[[0,0],[1,0],[.75,1]]
        if family=='roundrect':g.update(radiusX=10,radiusY=10)
        if 'fillStyle' in g:g['fillStyle']=1;g['foreground']='#ffffffff' if family=='polygon' else '#ff000000';g['background']='#ff000000' if family=='polygon' else '#ffffffff'
        if family=='table':g.update(rows=4,columns=4,horizontal=True,vertical=True,outline=True,background='#ffe0e0e0')
        page['objects'].append(obj)
    sample['pages'].append(page);sample['pageOrder'].append(page['id'])
sample['activePageId']=sample['pages'][-1]['id'];sample['schemaVersion']=max(sample['schemaVersion'],recipe['schemaVersion'])
assert len(sample['pages'])==6 and sum(len(page['objects']) for page in sample['pages'])==40
showcase=samples/'stage4-editor-40-slices.ksp';write(showcase,sample)
shutil.copy2(work/'third_party/lzokay/LICENSE',package/'LZO-MIT-LICENSE.txt');qr=(work/'third_party/qrcodegen/qrcodegen.hpp').read_text(encoding='utf-8');(package/'QR-MIT-LICENSE.txt').write_text(qr[:qr.index('*/')+2]+'\n',encoding='utf-8')
environment=os.environ.copy();environment['PATH']='G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'+environment.get('PATH','')
temporary=root/'task-temp/stage4-final-deploy';temporary.mkdir(parents=True,exist_ok=True);environment.update(TEMP=str(temporary),TMP=str(temporary),TMPDIR=str(temporary),PYTHONIOENCODING='utf-8')
result=subprocess.run(['G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin/windeployqt.exe','--release','--compiler-runtime','--no-translations','--dir',str(package),str(exe)],capture_output=True,env=environment,timeout=120,creationflags=subprocess.CREATE_NO_WINDOW)
(out/'deploy-stdout.txt').write_bytes(result.stdout);(out/'deploy-stderr.txt').write_bytes(result.stderr);assert result.returncode==0
spec=importlib.util.spec_from_file_location('final_portable',work/'docs/stage1-evidence/gui-portable/probe.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
smoke=out/'portable-r1';smoke.mkdir();temporary=smoke/'temp';temporary.mkdir();desktop=module.PrivateDesktop(smoke);environment=module.clean_environment(package);environment.update(TEMP=str(temporary),TMP=str(temporary),TMPDIR=str(temporary))
record=dict(startedAtUtc=datetime.datetime.now(datetime.timezone.utc).isoformat(),package=str(package),sourceCommit=commit,buildSource='isolated_verified_committed_checkout',executableSha256=sha(exe),userStage3WorkingChangesPresentInBinary=False,usbAccessed=False,systemInput=False,desktopSwitched=False,runs=[],passed=False)
try:
    checks=[('self-test',['--self-test'],0),('payload',['--verify-independent-payload'],0),('showcase-model',['--project-capabilities',str(showcase)],0),('showcase-roundtrip',['--migrate-project',str(showcase),str(smoke/'roundtrip.ksp')],0)]
    inputs=smoke/'inputs';inputs.mkdir()
    captures=[]
    for index,page in enumerate(sample['pages']):
        candidate=copy.deepcopy(sample);candidate['activePageId']=page['id'];path=inputs/('page-'+str(index+1)+'.ksp');write(path,candidate);picture=smoke/('page-'+str(index+1)+'.png');captures.append(str(picture));checks.append(('capture-page-'+str(index+1),['--capture-preview',str(picture),str(path)],0))
    for family,filename in [('alarm-bar','alarm-only.ksp'),('alarm-display','alarm-display-only.ksp'),('history-alarm','history-alarm-only.ksp'),('history-data','history-data-only.ksp'),('recipe','recipe-only.ksp')]:
        input_path=work/'docs/stage4-evidence'/family/'inputs'/filename
        checks.extend([(family+'-source-gate',['--export-native-project',str(input_path),str(smoke/'unpublished.sm')],20),(family+'-device-gate',['--compile-project',str(input_path),str(smoke/'unpublished-package')],19)])
    for family in families+['mixed-showcase']:
        path=showcase if family=='mixed-showcase' else samples/(family+'.ksp')
        checks.extend([(family+'-source-gate',['--export-native-project',str(path),str(smoke/'unpublished.sm')],20),(family+'-device-gate',['--compile-project',str(path),str(smoke/'unpublished-package')],19)])
    for name,argv,expected in checks:
        run=desktop.run(name,exe,argv,environment,timeout=60,expected_exit=expected);record['runs'].append(run);assert run['passed'],run
    restored=read(smoke/'roundtrip.ksp')
    assert restored['languages']==sample['languages'] and restored['pageOrder']==sample['pageOrder']
    assert [[o for o in p['objects']] for p in restored['pages']]==[[o for o in p['objects']] for p in sample['pages']]
    assert restored['configurationNodes']==sample['configurationNodes']
    record['roundtripFortyObjectsAndConfigurationsEqual']=True
    assert not (smoke/'unpublished.sm').exists() and not (smoke/'unpublished-package').exists()
    qt=sorted({path for run in record['runs'] for path in run['modulePathsObserved'] if Path(path).name.lower().startswith('qt6') and path.lower().endswith('.dll')});assert qt and all(Path(path).resolve().is_relative_to(package.resolve()) for path in qt)
    shutil.copy2(Path(captures[-1]),smoke/'main-window.png');record.update(passed=True,onlyBundledQtDllsObserved=True,qtDllsObserved=qt,previewSha256=sha(smoke/'main-window.png'),captures=captures)
finally:
    desktop.close();record['privateDesktopClosed']=True;record['temporaryFilesRemaining']=[str(path.relative_to(temporary)) for path in temporary.rglob('*') if path.is_file()];write(smoke/'run.json',record)
shutil.copy2(showcase,work/'samples/stage4/stage4-editor-40-slices.ksp')
(package/'使用说明.txt').write_text('阶段4本轮已逐项处理41项，其中40项为已有/编辑切片；曲线/圆弧真实曲率未恢复，保留阻塞。\n双击 KinSealStudioLite.exe，打开 samples/stage4/stage4-editor-40-slices.ksp，可查看6页混合样例。\n自由直线只支持已证首尾两点，扇形只支持已证默认角度；这两项还有未恢复功能。纯图像表格不支持单元格文字、数据或PLC绑定。\n新增类的完整SM/独立设备编码仍有阻塞，软件会拒绝不支持的输出。本轮逐项实物下载使用原厂基线，其传输成功与复刻版独立生成、完整实屏/触控分别记录。\n本包来自独立已验证提交，不含主目录尚未验收的阶段3改动；本轮处理记录不代表阶段4各层全部通过。\n',encoding='utf-8')
manifest=dict(package=str(package),sourceCommit=commit,files=[dict(path=str(path.relative_to(package)),bytes=path.stat().st_size,sha256=sha(path)) for path in sorted(package.rglob('*')) if path.is_file()],softwareTests=totals,softwareFailed=0,softwareSkipped=0,portableChecksPassed=len(record['runs']),processed=41,editorSlicesIncludingExisting=40,geometryBlocked=['G03'],completeStage4=False,independentNewEncoderVerified=False)
write(out/'package-manifest.json',manifest);(out/'reproduction').mkdir();shutil.copy2(root/'package_stage4_final.py',out/'reproduction/package_stage4_final.py')
(work/'docs/阶段4_41项本轮交付.md').write_text(f'# 阶段4本轮交付：41项处理记录\n\n包路径：`{package}`，源提交`{commit}`，来自独立已验证工作树。41项已逐项处理，其中40项为已有/编辑切片，G03曲线圆弧保持真实曲率阻塞；G02自由直线首尾切片、G07默认角度扇形仍有未恢复功能。完整阶段4各层未宣称通过。\n\n实际控件{totals["KinSealStage4ControlTests"]}、源{totals["KinSealNativeSourceTests"]}、SDK宏{totals["KinSealDeviceMacroTests"]}，失败0、跳过0；便携包{len(record["runs"])}项后台检查通过。六页40对象混合工程保留31类控件及9个已实现图形/表格家族，不含伪曲线。六页均实际启动并截图，Qt DLL均从包内加载。\n\n新增类的完整源/独立设备编码、完整现场显示/触控仍有未决，原厂基线下载不替代这些层通过。样例将C30引用重绑至既有采集并保留语言列名，新增配方/报警定义及图形页面均经当前模型和真实包验证。\n\n[包清单](stage4-evidence/delivery-{args.run}/package-manifest.json) · [后台验证](stage4-evidence/delivery-{args.run}/portable-r1/run.json) · [最新进度](阶段4逐控件进度.md)\n',encoding='utf-8',newline='\n')
progress_path=work/'docs/stage4-evidence/progress.json';progress=read(progress_path);progress['latestDelivery']=dict(evidence='docs/stage4-evidence/delivery-'+args.run+'/package-manifest.json',package=str(package),sourceTree='isolated_verified_committed_checkout',sourceCommit=commit,processed=41,editorSlicesIncludingExisting=40,geometryBlocked=['G03'],total=41,completeStage4=False,mainUnacceptedStage3Failures=2);write(progress_path,progress)
print(json.dumps(dict(package=str(package),checksPassed=len(record['runs']),sourceCommit=commit,onlyBundledQtDlls=True,processed=41,editorSlices=40,geometryBlocked=['G03'])))
