"""Finalize C28 only after real tests, native download and scoped cleanup have succeeded."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
root=Path(__file__).resolve().parent;work=root/'worktree';base=work/'docs/stage4-evidence/alarm-display'
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def write(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
parser=argparse.ArgumentParser();parser.add_argument('--run',default='windows-r1');args=parser.parse_args()
run=read(base/args.run/'run.json');assert run['passed']
assert [r['totals'] for r in run['runs']]==[dict(passed=156,failed=0,skipped=0),dict(passed=59,failed=0,skipped=0),dict(passed=10,failed=0,skipped=0)]
names=['alarmDisplayDefaultsAndStrictRecords','alarmDisplayCaptionLanguagesAndCommonStyles','alarmDisplaySharedPrerequisitesCopyAndVisibility','alarmDisplayPersistenceLanguagesAndBackendGates','alarmDisplayStaticColumnsFontsAndClipping','alarmDisplayShellGatePlacementEditingAndUndo']
log=(base/args.run/'KinSealStage4ControlTests.txt').read_text(encoding='utf-8-sig');assert all('PASS   : Stage4ControlTests::'+name+'()' in log for name in names)
(base/'windows-final').mkdir(exist_ok=False);write(base/'windows-final/run.json',dict(run,evidenceSource=args.run+'/run.json',aggregationOnlyNoTestsInvented=True))
hardware=read(base/'physical/alarm-display-baseline-r1/run.json');assert all(hardware[k] for k in ['completed','inputUnchanged','sourceAtTransferMatchesInput','ownedProcessTerminated'])
assert hardware['sourceProvenance']=='retained_native_reference_source' and hardware['liteIndependentEncoderVerified'] is False
assert all(not x['afterChecked'] for x in hardware['nativeHardware']['transferOptions'])
for source,destination,count in [(root/'native-alarm-display/native-cleanup-alarm-display-custom-r5.json.result.json',base/'native/cleanup.json',126),(root/'native-physical/physical-cleanup-alarm-display-baseline-r1.json.result.json',base/'physical/alarm-display-baseline-r1/cleanup.json',46)]:
    assert read(source)['deletedFiles']==count;shutil.copy2(source,destination)
manifest=read(base/'native/native-manifest.json');manifest['date']='2026-11-13';write(base/'native/native-manifest.json',manifest)
shutil.copy2(base/args.run/'alarm-display-mixed.ksp',work/'samples/stage4/alarm-display.ksp')
inputs=base/'inputs';inputs.mkdir(exist_ok=True);shutil.copy2(base/args.run/'alarm-display-only.ksp',inputs/'alarm-display-only.ksp')
environment=os.environ.copy();environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'+environment.get('PATH','')
temporary=root/'task-temp/alarm-display-gates';temporary.mkdir(parents=True,exist_ok=True);environment.update(TEMP=str(temporary),TMP=str(temporary),TMPDIR=str(temporary))
executable=work/'build/KinSealStudioLite.exe';calls=[]
for label,argv in [('capabilities',['--project-capabilities',str(inputs/'alarm-display-only.ksp')]),('compile',['--compile-project',str(inputs/'alarm-display-only.ksp'),str(inputs/'unpublished-package')]),('export-source',['--export-native-project',str(inputs/'alarm-display-only.ksp'),str(inputs/'unpublished.sm')])]:
    r=subprocess.run([str(executable),*argv],capture_output=True,env=environment,timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
    stdout=r.stdout.decode('utf-8-sig',errors='replace');calls.append(dict(label=label,exitCode=r.returncode,stdout=stdout,stderr=r.stderr.decode('utf-8-sig',errors='replace'),result=json.loads(stdout)))
assert calls[0]['exitCode']==0 and calls[0]['result']['modelPersistence']['allowed']
assert calls[1]['exitCode']!=0 and calls[2]['exitCode']!=0 and not (inputs/'unpublished.sm').exists() and not (inputs/'unpublished-package').exists()
write(base/'backend-gates.json',dict(executableSha256=sha(executable),inputPath=str((inputs/'alarm-display-only.ksp').relative_to(work)),inputSha256=sha(inputs/'alarm-display-only.ksp'),calls=calls,usbAccessed=False,sourceAndDeviceRefusalsRetained=True))
scope='独立AlarmDisplayProperties/Dialog，两页和原厂标题/边框/文本/日期/时间分组；共用五颜色、宋体、7字号12..192与1..20行，只有消息/日期/时间三列名按语言保存，日期/时间开关联动禁用并保留列名；仅显现权限、schema12、全页报警前置/引用/依赖复制、语言事务、侧栏文本/地址禁用、创建/取消/编辑/撤销、空白示意表格和原厂基线实物下载'
native='原厂空工程门槛已独立实测，标题报警显示器、提示“位报警和条件报警都不存在”；行数Qt范围1..20，默认5，字体仅宋体，字号完整为12/16/24/48/64/96/192。五颜色、字号、行数、日期时间开关跨语言共用；只有三个列名分语言保存。默认与双语言自定义变体均保存/编译/重开；五颜色绿/红/蓝/灰/紫、字号24/行20共用，两语言ZH/EN三个列名各自保留，两列关闭后字段禁用且文字仍保存。'
rows=[]
scope += '；跨工程复制遇目标未启用的列名语言时整批拒绝，补齐工程语言后可粘贴'
for name,label in [('alarm-display-default.sm','alarm-display-default-r2'),('alarm-display-custom.sm','alarm-display-custom-r5')]:
    source=base/'native'/name;folder=base/'native/compiled'/label;assert folder.is_dir(),folder
    rows.append(f'| {name} | {source.stat().st_size} | `{sha(source)}` | {(folder/"ControlPara.dat").stat().st_size} | {(folder/"ControlPixel.dat").stat().st_size} |')
report=f'''# 报警显示：手册实现与验收

2026-11-13，阶段4第28/41项，累计23类新增编辑/KSP切片；完整源/设备和现场仍阻塞。

## 本轮交付与原厂订正

{scope}。

{native}

标题语言选择仅切换消息/日期/时间三个列名，切换过程阻断字段信号，避免旧语言写入新语言；五颜色和字号保持同一份共用状态。日期/时间关闭时标签和字段禁用，保留原有列名。工程增删语言同步所有页面的列名映射，新增语言沿用原厂观察到的中文默认列名。

沿用C27真实报警登录及全页消费者检查，不新增虚构的显示地址、报警类别筛选、确认按钮或历史查询条件。删除最后非空登录、复制到冲突登录、导航目标不一致时取消整个事务。列名语言必须在目标工程启用；若目标缺少源控件的某种列名语言，粘贴原子拒绝，补齐工程语言后可成功粘贴，避免下次属性编辑归一时丢弃未启用语言。报警输入和跳转依赖仍在全局定义，显示器只拥有活动显现位/字地址；通用侧栏文本与操作地址禁用，模型也拒绝这些字段。

## 软件与复核

[实际后台私有桌面测试](stage4-evidence/alarm-display/{args.run}/run.json)：控件156、原厂源59、SDK宏10，共225/0/0，六项新增方法全部PASS。覆盖实测默认/边界和未知/分数/嵌套类型、三列名语言隔离与共用样式、隐藏列名保留、活动显现地址角色、全页前置、依赖复制和语言事务、schema12及源/设备拒绝的输出不可变性、四种日期时间列组合×七字号的剪裁、真实菜单门槛/取消/放置/编辑/撤销。

[基本页](stage4-evidence/alarm-display/{args.run}/alarm-display-basic.png)和[样例](../samples/stage4/alarm-display.ksp)来自实际测试。样例为C27/C28两类混合，未声称28类混合覆盖。只读复核检查字段类型/JSON键、Qt信号和语言归属、前置/复制、所有页语言更新与剪裁；随后补上了模型拒绝通用文本的校验。整个运行均未切换桌面或注入系统鼠标键盘。

PC显示空白示意行及序号，不伪造报警时间、消息、分页或实际确认状态。原厂小预览包含绿色示意行；Lite按所配置背景绘制空白行，此项PC图示差异保持记录，未声称逐像素完全一致或设备运行一致。

## 原厂证据与实物

| 原源 | 字节数 | SHA-256 | ControlPara.dat | ControlPixel.dat |
|---|---:|---|---:|---:|
{chr(10).join(rows)}

[原厂证据清单](stage4-evidence/alarm-display/native/native-manifest.json)保留五轮完整gzip/SHA、原源和关键DAT。r3失败原因是探针向Frida bool参数传值导致expected an integer；不是应用崩溃。r4的setChecked只改变勾选状态，不能代替原厂clicked联动；列禁用/保存结论仅采用r5实际按钮click的成功结果。本轮读取minimum=1，没有执行旧手册所述0值异常的越界崩溃测试。

[默认基线实物下载](stage4-evidence/alarm-display/physical/alarm-display-baseline-r1/run.json)成功，四清除/固件更新false，输入与待传源SHA一致；实屏和触控未确认。原厂基线传输通过不等于Lite独立下载。实际CLI的独立源导出/设备编译继续拒绝，见[输出门槛](stage4-evidence/alarm-display/backend-gates.json)。调查126文件和物理46文件已按身份清理；测试重复截图/样例另有清理记录，所有临时目录独占Z盘。

## 未决

- Q-S04-C28-001：独立SM/设备编码、全局报警段和独立下载仍阻塞。
- Q-S04-C28-002：当前报警发生/确认/恢复/清除、分页/翻页函数及PLC运行语义待验。
- Q-S04-C28-003：设备实际列宽/顺序、长文本/大字号剪裁、原厂绿色示意行与实际内容区颜色、旧手册0值异常待补证。
- Q-S04-C28-004：实屏、触控、PLC、多语言实际报警显示和权限组合运行未验。

下一项C29历史报警显示，继续独立核对入口、基线和实物。
'''
(work/'docs/报警显示手册实现与验收.md').write_text(report,encoding='utf-8',newline='\n')
spec=dict(id='C28',ordinal=28,name='报警显示',slug='alarm-display',task='S04-10',next='C29',usbAccessed=True,hardwareRun='docs/stage4-evidence/alarm-display/physical/alarm-display-baseline-r1/run.json',expectedPassed=225,testRun='docs/stage4-evidence/alarm-display/windows-final/run.json',nativeManifest='../docs/stage4-evidence/alarm-display/native/native-manifest.json',report='docs/报警显示手册实现与验收.md',manual='design_plan/报警显示功能与交互复刻手册.md',scope=scope,unresolved=[f'Q-S04-C28-{i:03}' for i in range(1,5)],nativeSummary=native,protocol='报警显示器空工程门槛和两套源的保存/重开/编译已核对；默认原厂基线实物下载完成，四清除/更新关闭、输入/待传SHA一致。三列名分语言，五颜色/字号/行数/列开关共用为原厂UI实测，不推导USB布局。Lite独立SM/设备编码继续真实拒绝；没有新增实屏/触控/PLC或分页确认结论。',codeFiles='src/alarm_display_properties.*、alarm_display_dialog.*、alarm_registration_properties.cpp、project_model.*、stage3_project_edits.cpp、stage4_control_actions.cpp、static_text_actions.cpp、main_window.cpp、native_shell_ui.cpp、hmi_canvas.cpp、CMakeLists.txt',codeMarker='//20261113-2',reviewFindings=['只读复核覆盖三列名分语言与全局样式所有权、QSignalBlocker防串写、隐藏字段保留、严格JSON键/类型、全页前置和引用/复制；随后补齐通用文本拒绝。','六项新增方法实际PASS，156控件+59原厂源+10SDK宏=225/0/0；示意空白行不等于实际报警数据。','原厂绿色小预览与Lite配置背景示意图差异、旧0值崩溃观察与本轮minimum=1边界、r3/r4探针局限均明确分层记录。'])
write(root/'alarm-display-finalize.json',spec);subprocess.run([sys.executable,'-B',str(root/'finalize_xy.py'),str(root/'alarm-display-finalize.json')],check=True)
repro=base/'reproduction';repro.mkdir(exist_ok=True)
for name in ['prepare_alarm_display_probe.py','prepare_alarm_display_language.py','prepare_alarm_display_custom.py','inspect_alarm_display_fields.py','inspect_alarm_display_language.py','make_alarm_display_model.py','make_alarm_display_dialog.py','integrate_alarm_display.py','add_alarm_display_tests.py','review_alarm_display_before_build.py','archive_native_incremental.py','execute_native_cleanup.ps1','archive_physical_run.py','run_stage4_native_physical.py','deliver_alarm_display_docs.py','alarm-display-finalize.json','finalize_xy.py','prepare_control_cleanup.py','execute_control_cleanup.ps1']:
    shutil.copy2(root/name,repro/name)
shutil.copy2(root/'native-alarm-display/alarm-display-language-r4.json',repro/'alarm-display-language-r4.json')
for log in root.glob('alarm-display-build-*.log'):shutil.copy2(log,base/log.name)
shutil.copy2(root/'refine_alarm_display_review.py',repro/'refine_alarm_display_review.py')
print('C28 documentation prepared from 225 actual passes and completed reference transfer.')
