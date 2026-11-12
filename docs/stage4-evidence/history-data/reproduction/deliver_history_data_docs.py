"""Deliver C30 only from actual tests, retained native output and completed hardware evidence."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
root=Path(__file__).resolve().parent;work=root/'worktree';base=work/'docs/stage4-evidence/history-data'
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def write(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
parser=argparse.ArgumentParser();parser.add_argument('--run',default='windows-r1');args=parser.parse_args()
run=read(base/args.run/'run.json');assert run['passed']
assert [r['totals'] for r in run['runs']]==[dict(passed=168,failed=0,skipped=0),dict(passed=59,failed=0,skipped=0),dict(passed=10,failed=0,skipped=0)]
names=['historyDataMeasuredDefaultsAndZeroRows','historyDataAllColumnLanguageSynchronization','historyDataVisibleColumnGuard','historyDataStrictRoundTripAndBackendGates','historyDataCollectorReconciliationAcrossPages','historyDataClipboardAndFullVisibilitySpans','historyDataRendererGridAndZeroRowClipping','historyDataShellPlacementCollectionEditAndUndo']
log=(base/args.run/'KinSealStage4ControlTests.txt').read_text(encoding='utf-8-sig');assert all('PASS   : Stage4ControlTests::'+name+'()' in log for name in names)
(base/'windows-final').mkdir(exist_ok=False);write(base/'windows-final/run.json',dict(run,evidenceSource=args.run+'/run.json',aggregationOnlyNoTestsInvented=True))
hardware=read(base/'physical/history-data-baseline-r1/run.json');assert all(hardware[k] for k in ['completed','inputUnchanged','sourceAtTransferMatchesInput','ownedProcessTerminated'])
assert hardware['sourceProvenance']=='retained_native_reference_source' and hardware['liteIndependentEncoderVerified'] is False
assert sha(base/'native/history-data-default.sm')==hardware['sourceSha256']
assert all(not x['afterChecked'] for x in hardware['nativeHardware']['transferOptions'])
for source,destination,count in [(root/'native-history-data/native-cleanup-history-data-no-columns-r9.json.result.json',base/'native/cleanup.json',238),(root/'native-physical/physical-cleanup-history-data-baseline-r1.json.result.json',base/'physical/history-data-baseline-r1/cleanup.json',46)]:
    assert read(source)['deletedFiles']==count;shutil.copy2(source,destination)
manifest=read(base/'native/native-manifest.json');manifest['date']='2026-11-13';write(base/'native/native-manifest.json',manifest)
shutil.copy2(base/args.run/'history-data-mixed.ksp',work/'samples/stage4/history-data.ksp')
inputs=base/'inputs';inputs.mkdir(exist_ok=True);shutil.copy2(base/args.run/'history-data-only.ksp',inputs/'history-data-only.ksp')
environment=os.environ.copy();environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'+environment.get('PATH','')
temporary=root/'task-temp/history-data-gates';temporary.mkdir(parents=True,exist_ok=True);environment.update(TEMP=str(temporary),TMP=str(temporary),TMPDIR=str(temporary))
executable=work/'build/KinSealStudioLite.exe';calls=[]
for label,argv in [('capabilities',['--project-capabilities',str(inputs/'history-data-only.ksp')]),('compile',['--compile-project',str(inputs/'history-data-only.ksp'),str(inputs/'unpublished-package')]),('export-source',['--export-native-project',str(inputs/'history-data-only.ksp'),str(inputs/'unpublished.sm')])]:
    r=subprocess.run([str(executable),*argv],capture_output=True,env=environment,timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
    stdout=r.stdout.decode('utf-8-sig',errors='replace');calls.append(dict(label=label,exitCode=r.returncode,stdout=stdout,stderr=r.stderr.decode('utf-8-sig',errors='replace'),result=json.loads(stdout)))
assert calls[0]['exitCode']==0 and calls[0]['result']['modelPersistence']['allowed']
assert calls[1]['exitCode']!=0 and calls[2]['exitCode']!=0 and not (inputs/'unpublished.sm').exists() and not (inputs/'unpublished-package').exists()
write(base/'backend-gates.json',dict(executableSha256=sha(executable),inputPath=str((inputs/'history-data-only.ksp').relative_to(work)),inputSha256=sha(inputs/'history-data-only.ksp'),calls=calls,usbAccessed=False,sourceAndDeviceRefusalsRetained=True))
scope='独立historydata类型与真实采集前置；一般/数据/显现权限三页、六共用颜色、四字号、0..20行、双向格线、日期时间关闭仍可编辑；稳定采集列ID/顺序、分语言列名、共用显示标志、两页语言同步及首语言全部数据列名同步；所有页采集通道增删原子协调与撤销、复制依赖/语言冲突保护、schema12保存、时间在日期前的空白示意表格及原厂基线实物下载'
native='原厂九轮调查已归档。无采集提示没有建立数据采集；行数0可保存/编译/重开，字号仅12/16/24/48；六颜色和显示标志跨语言共用。首语言名称同步作用于全部数据列，保留日期时间名和隐藏标志；采集长度3→2允许且保留前两项列名/显示。隐藏全部数据列而保留日期时间时允许；日期时间和数据列全隐藏时实测设置错误：未选择任何显示数据。'
rows=[]
for compilation in manifest['nativeCompilations']:
    folder=base/'native/compiled'/compilation['label'];rows.append(f'| {folder.name} | {(folder/"ControlPara.dat").stat().st_size} | {(folder/"ControlPixel.dat").stat().st_size} |')
report=f'''# 历史数据显示器：手册实现与验收

2026-11-13，阶段4第30/41项，累计25类新增编辑/KSP切片。完整独立源/设备与现场运行仍有阻塞。

## 本轮实现与原厂依据

{scope}。

{native}

C30有独立模型和三页窗口，未套用报警表格的1..20行、七字号和禁用日期时间名称行为。默认格线#00c000，六色/字号/行数/格线与列开关共用，只有日期时间名和各数据列名分语言。同步按钮从工程第一语言复制全部数据列名，不复制日期时间名，不修改共用标志。属性语言只在当前草稿中切换，不改工程活动语言。

数据列严格匹配真实采集的稳定item ID与顺序；采集配置改名或修改源名称保留用户列名；删通道移除对应列，新通道按源项名称初始化。配置窗口确定和主窗口提交都先协调所有活动/非活动页，再整体验证并形成一次撤销。历史趋势图已有的被引用源项删除保护仍生效。如果缩减后一个日期/时间/数据列也不可见，则整个更新原子拒绝；没有擅自打开隐藏列。

PC表格只显示列头和空白示意行，时间在日期前。0行安全显示表头；没有据此推断固件0行的运行语义。原厂小预览中的绿色数据底色与保存的#e0e0e0字段存在差异，PC按已存样式绘制；不宣称完整像素一致，没有伪造采集值、日期时间、查询或翻页行为。

## 软件验收和只读复核

[真实Windows私有桌面测试](stage4-evidence/history-data/{args.run}/run.json)：控件168、原厂源59、SDK宏10，合计237/0/0。8个新增方法覆盖原厂边界、全列隐藏错误、三语言所有列名同步、隐藏旗标、不可变严格JSON、schema/type门槛、采集ID顺序与全页引用、跨页3→2→3协调、语言增删、无显示列时原子拒绝、复制依赖/语言冲突、32位显现完整跨度/递增、双向格线和0行剪裁、真实菜单取消/放置/编辑/采集缩减/撤销。

[一般页](stage4-evidence/history-data/{args.run}/history-data-general.png)、[数据页](stage4-evidence/history-data/{args.run}/history-data-columns.png)、[7类型混合样例](../samples/stage4/history-data.ksp)来自测试。样例含已有5类、历史趋势和历史数据；未冒称30类混合。

只读复核检查了Qt6容器遍历、专有属性匹配/白名单/schema、语言信号阻塞、两页语言同步、全表同步作用域、零行除数、格线/边框分离、采集窗口提前验证路径、全页源引用与候选提交、复制角色和源/设备门槛。复核发现采集窗口先用旧列验证新采集的拦截问题，随后修复为统一原子协调；未开放未知源或设备编码。

## 原厂源、编译与实物

| 原厂编译标签 | ControlPara.dat | ControlPixel.dat |
|---|---:|---:|
{chr(10).join(rows)}

[原厂清单](stage4-evidence/history-data/native/native-manifest.json)保留九轮完整gzip/SHA与关键DAT、原SM和复现配方。r4场景准备失败，仅留失败证据；r7确定已接受全部数据列隐藏，之后截图器要求对话框而失败；r9全隐藏按预期出现设置错误，探针assert-main-only因此记录partial。没有把这些状态改写为完整成功。r8的0行实际经过保存、编译、重开，r5三语言同步和r6六色/采集缩减亦有原厂保存编译证据。

[原厂默认基线实物下载](stage4-evidence/history-data/physical/history-data-baseline-r1/run.json)完成。输入源SHA `{hardware['sourceSha256']}`，输入/待传一致；四清除/固件更新选项false。只支持原厂基线传输，实屏/触控/PLC尚未确认；[实际Lite CLI](stage4-evidence/history-data/backend-gates.json)仍拒绝独立SM/设备输出。

调查238文件、物理46文件已完成按路径/SHA清理；软件重复截图/样例另有清理记录。宿主和构建临时目录均放本任务Z盘目录，没有切换桌面或注入系统键鼠。

## 未决

- Q-S04-C30-001：历史数据SM字段、采集及显示器设备记录、独立生成和下载未恢复。
- Q-S04-C30-002：设备历史采样、查询/翻页、空/损坏历史、掉电保存与容量覆盖运行待验。
- Q-S04-C30-003：原厂预览绿色示意底色、真实设备列宽、大字号/长列名及0行运行语义待验。
- Q-S04-C30-004：实屏、触控、PLC、多语言历史记录和权限组合运行尚未验证。

下一项C31配方显示器，需先复核真实配方配置及编辑配方展开行为。
'''
(work/'docs/历史数据显示器手册实现与验收.md').write_text(report,encoding='utf-8',newline='\n')
spec=dict(id='C30',ordinal=30,name='历史数据显示器',slug='history-data',task='S04-10',next='C31',usbAccessed=True,hardwareRun='docs/stage4-evidence/history-data/physical/history-data-baseline-r1/run.json',expectedPassed=237,testRun='docs/stage4-evidence/history-data/windows-final/run.json',nativeManifest='../docs/stage4-evidence/history-data/native/native-manifest.json',report='docs/历史数据显示器手册实现与验收.md',manual='design_plan/历史数据显示器功能与交互复刻手册.md',scope=scope,unresolved=[f'Q-S04-C30-{i:03}' for i in range(1,5)],nativeSummary=native,protocol='历史数据控件绑定真实采集；已实测六共用色、四字号、0..20行、多语言全列同步、3→2采集缩减，真正无可见列时原厂拒绝。默认原厂源已实物下载，四清除/更新关闭，输入/待传SHA一致；未恢复独立采集/显示器设备记录，独立输出仍实际拒绝。显示/触控、采样查询翻页与0行固件语义未通过。',codeFiles='src/history_data_properties.*、history_data_dialog.*、data_collection_dialog.cpp、project_model.*、stage3_project_edits.cpp、stage4_control_actions.cpp、static_text_actions.cpp、main_window.cpp、native_shell_ui.cpp、hmi_canvas.cpp、CMakeLists.txt',codeMarker='//20261113-2',reviewFindings=['只读复核检查独立C30字段范围、JSON/type/schema、语言/同步作用域、零行剪裁和采集全页协调；发现提前验证路径后已修复。','8项新增方法实际PASS，共用回归168+59+10=237/0/0；真实采集缩减与整工程撤销通过。','九轮原厂证据含三轮partial原样保留，真正全隐藏错误和0行保存编译重开均按观测记录。实物仅原厂默认基线传输，E和独立编码仍阻塞。'])
write(root/'history-data-finalize.json',spec);subprocess.run([sys.executable,'-B',str(root/'finalize_xy.py'),str(root/'history-data-finalize.json')],check=True)
repro=base/'reproduction';repro.mkdir(exist_ok=True)
for name in ['prepare_history_data_probe.py','prepare_history_data_survey.py','prepare_history_data_edges.py','prepare_history_data_no_columns.py','inspect_history_data_fields.py','inspect_history_data_survey.py','inspect_history_data_edges.py','prepare_history_data_archive.py','make_history_data_model.py','make_history_data_dialog.py','integrate_history_data.py','add_history_data_tests.py','review_history_data_before_build.py','refine_history_data_fixture.py','fix_history_data_test_comparison.py','archive_native_incremental.py','execute_native_cleanup.ps1','archive_physical_run.py','run_stage4_native_physical.py','deliver_history_data_docs.py','history-data-finalize.json','finalize_xy.py','prepare_control_cleanup.py','execute_control_cleanup.ps1']:
    shutil.copy2(root/name,repro/name)
for log in root.glob('history-data-build-*.log'):shutil.copy2(log,base/log.name)
print('C30 docs prepared from 237 actual passes and completed native-reference hardware transfer.')
