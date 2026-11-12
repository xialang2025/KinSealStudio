"""Ground C22 delivery documents in actual test runs and three native hardware baselines."""
from pathlib import Path
import copy
import hashlib
import json
import os
import shutil
import subprocess
root=Path(__file__).resolve().parent
work=root/'worktree'
base=work/'docs/stage4-evidence/chart'
def read(p): return json.loads(p.read_text(encoding='utf-8-sig'))
def write(p,d): p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
final=read(base/'windows-r3/run.json')
prior=read(base/'windows-r2/run.json')
assert final['passed'] and final['runs'][0]['totals']==dict(passed=115,failed=0,skipped=0)
assert prior['passed'] and prior['runs'][1]['totals']==dict(passed=59,failed=0,skipped=0) and prior['runs'][2]['totals']==dict(passed=10,failed=0,skipped=0)
final['runs']=[dict(final['runs'][0],evidenceSource='windows-r3/run.json'),dict(prior['runs'][1],evidenceSource='windows-r2/run.json'),dict(prior['runs'][2],evidenceSource='windows-r2/run.json')]
final['actualSourceRecords']=['windows-r3/run.json','windows-r2/run.json']
final['aggregationOnlyNoTestsInvented']=True
(base/'windows-final').mkdir(exist_ok=False)
write(base/'windows-final/run.json',final)
hardware=[]
for shape in ['bar','meter','donut']:
 run=read(base/f'physical/chart-{shape}-baseline-r1/run.json')
 assert run['completed'] and run['inputUnchanged'] and run['sourceAtTransferMatchesInput'] and run['ownedProcessTerminated'] and run['sourceProvenance']=='retained_native_reference_source' and run['liteIndependentEncoderVerified'] is False
 assert all(not x['afterChecked'] for x in run['nativeHardware']['transferOptions'])
 assert (base/f'physical/chart-{shape}-baseline-r1/cleanup.json').exists()
 hardware.append(dict(shape=shape,run=f'physical/chart-{shape}-baseline-r1/run.json',sourceSha256=run['sourceSha256'],nativeDownloadCompleted=True,liteIndependentEncoderVerified=False))
write(base/'hardware-summary.json',dict(baselines=hardware,completed=3,total=3,sourceProvenance='retained_native_reference_source',independentEncoderVerified=False,displayOrPlcVerified=False))
sample=read(base/'windows-r3/chart-mixed.ksp')
sample['projectName']='阶段4 C01～C22 编辑切片'
shutil.copy2(base/'windows-r3/chart-mixed.ksp',work/'samples/stage4/chart.ksp')
inputs=base/'inputs'; inputs.mkdir(exist_ok=True)
only=copy.deepcopy(sample)
for page in only['pages']: page['objects']=[o for o in page['objects'] if o['type']=='chart']
assert sum(len(p['objects']) for p in only['pages'])==1
write(inputs/'chart-only.ksp',only)
for shape,label in enumerate(['bar','meter','donut']):
 branch=copy.deepcopy(only)
 for page in branch['pages']:
  for obj in page['objects']: obj['chart']['shape']=shape
 write(work/f'samples/stage4/chart-{label}.ksp',branch)
environment=os.environ.copy()
environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'+environment.get('PATH','')
environment.update(TEMP=str(root/'native-chart/temp'),TMP=str(root/'native-chart/temp'),TMPDIR=str(root/'native-chart/temp'))
executable=work/'build/KinSealStudioLite.exe'
calls=[]
for label,args in [('capabilities',['--project-capabilities',str(inputs/'chart-only.ksp')]),('compile',['--compile-project',str(inputs/'chart-only.ksp'),str(inputs/'unpublished-package')]),('export-source',['--export-native-project',str(inputs/'chart-only.ksp'),str(inputs/'unpublished.sm')])]:
 result=subprocess.run([str(executable),*args],capture_output=True,env=environment,timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
 stdout=result.stdout.decode('utf-8-sig',errors='replace')
 calls.append(dict(label=label,exitCode=result.returncode,stdout=stdout,stderr=result.stderr.decode('utf-8-sig',errors='replace'),result=json.loads(stdout)))
assert calls[0]['exitCode']==0 and calls[0]['result']['modelPersistence']['allowed']
assert calls[1]['exitCode']!=0 and calls[2]['exitCode']!=0 and not (inputs/'unpublished-package').exists() and not (inputs/'unpublished.sm').exists()
write(base/'backend-gates.json',dict(executableSha256=sha(executable),inputPath=str((inputs/'chart-only.ksp').relative_to(work)),inputSha256=sha(inputs/'chart-only.ksp'),calls=calls,usbAccessed=False,sourceAndDeviceRefusalsRetained=True))
reproduction=base/'reproduction'; reproduction.mkdir(exist_ok=True)
for name in ['prepare_chart_probe.py','prepare_chart_scenarios.py','prepare_chart_ui.py','integrate_chart.py','refine_chart_fields.py','refine_chart_layout.py','refine_chart_preview.py','deliver_chart_docs.py','archive_native_incremental.py','run_stage4_native_physical.py','archive_physical_run.py','execute_native_cleanup.ps1']:
 shutil.copy2(root/name,reproduction/name)
rows=[]
for shape in ['bar','meter','donut']:
 source=base/f'native/{shape}-default.sm'
 label=f'{shape}-default-'+('r7' if shape=='meter' else 'r4')
 folder=base/'native/compiled'/label
 rows.append(f'| {shape} | {source.stat().st_size} | `{sha(source)}` | {(folder/"ControlPara.dat").stat().st_size} | {(folder/"ControlPixel.dat").stat().st_size} |')
table='\n'.join(rows)
scope='独立ChartProperties/Dialog，柱状图/仪表/圆环三套草稿、基本/外形/仅显现权限页；7数据类型及柱状范围复位、两极柱中点、常量/地址草稿、13图案、32线宽、仪表刻度实测边界与圆环/圆弧字段切换；22类混合/单类schema12、活动地址全跨度及复制、侧栏/创建/取消/撤销，静态PC预览和三形态原厂基线实物下载'
native='本轮实测入口graphicsitem与btn_metre（QPushButton），旧action_chart未在当前菜单找到；柱状/仪表默认I16、圆环U16。柱状7类型满范围及中点复位、13图案完整列表、圆环两处线宽1..32已枚举；仪表形状实际顺序半圆向上/全圆向上/全圆向下/四分之三圆向上，默认第二项；主刻度2..20、次刻度2..5、字号5..100；显示和源范围字段实际为QLineEdit。三份单根原源保存、重开、原厂编译和独立物理下载完成，未核验新版生成。'
limitations=['Q-S04-C22-001','Q-S04-C22-002','Q-S04-C22-003','Q-S04-C22-004']
report=f'''# 图表：手册实现与验收

2026-11-12，阶段4第22/41项。交付有边界的编辑与KSP切片；17类新增编辑切片的完整独立源/设备编码仍阻塞。

## 已实现及实测纠正

{scope}。

{native}

原厂反复调查中的失败仍保留：错误动作名、按钮类型、仪表直接页签调用系统错误和错误范围的页签定位均归档；最终使用图表自有页签的后台消息完成仪表样本，没有调用系统键鼠或切换用户桌面。原始JSON以gzip保留，缩减文件可追溯到原始SHA。

## 实际软件验证

最终[控件115/0/0](stage4-evidence/chart/windows-r3/run.json)、[源59/0/0、SDK宏10/0/0](stage4-evidence/chart/windows-r2/run.json)，[分来源汇总184/0/0](stage4-evidence/chart/windows-final/run.json)。新增6项覆盖三套非活动草稿、全部类型复位与联动、全跨度地址/复制、严格持久化/拒绝门槛、实际像素与剪裁、真实命令创建/取消/侧栏/撤销。

首次控件114/1/0保留：原厂按钮启用状态的纠正在构建过程中完成，首次对象文件仍保留旧逻辑；重新构建后115/0/0。之后按截图调整源范围左右布局、同排中点、字段宽度和默认零值仪表预览，并加强非默认非活动草稿往返覆盖，最终控件再次115/0/0。本次没有把用户主目录的阶段3既有改动并入提交，主目录既有画布/集成残留另见历史报告。

原厂对照：[柱状](stage4-evidence/chart/native/bar-basic-r4.png)、[仪表](stage4-evidence/chart/native/meter-appearance-r7.png)、[圆环](stage4-evidence/chart/native/donut-basic-r4.png)。新版：[柱状](stage4-evidence/chart/windows-r3/chart-bar-basic.png)、[仪表](stage4-evidence/chart/windows-r3/chart-meter-appearance.png)、[圆环](stage4-evidence/chart/windows-r3/chart-donut-basic.png)。尚未声明所有布局、绘制与原厂逐像素一致。

[混合样例](../samples/stage4/chart.ksp)含22类；另有chart-bar/meter/donut三个单类形态样例。Chart字段必须与类型匹配，缺失/未知字段、小数枚举或非有限活动标量被拒绝，失败保留调用者输出；仅活动形态与动态源范围地址参与复制和全跨度校验，非活动地址保留。

## 原厂基线实物下载

[三份汇总](stage4-evidence/chart/hardware-summary.json)：bar、meter、donut各1次实际下载完成，唯一健康7365:616C、libusb0，四项清除/底层更新均关闭。输入未改、传输源SHA一致、DownLoadDlg/m_table确认为下载完成、自有进程已终止，编译下载包保留。

| 原厂形态 | 源字节 | 源SHA256 | ControlPara字节 | ControlPixel字节 |
|---|---:|---|---:|---:|
{table}

不同形态样本及进程不是单变量差分，不能按总长或二进制差异推导设备记录布局。以上来源均为retained_native_reference_source，liteIndependentEncoderVerified=false。

[真实单类门槛检查](stage4-evidence/chart/backend-gates.json)确认KSP可保存，源导出/独立编译继续拒绝且没有部分输出。原厂基线成功不替代新版独立编码或下载；没有新增屏显、PLC刺激或权限运行通过结论。

## 未决项及明确实现边界

| 编号 | 后续证据与本版边界 |
|---|---|
| Q-S04-C22-001 | P_SM：独立三形态源编码、权限与几何字段、修改写回均未恢复。 |
| Q-S04-C22-002 | C/独立T：设备type、参数/绘图/地址布局未恢复，Lite保持拒绝；三份原厂下载不能替代。 |
| Q-S04-C22-003 | E：PLC值驱动柱高/方向/指针/弧度、越界、多个显现条件公式、透明混合及设备表现未验。图表没有触控功能。 |
| Q-S04-C22-004 | D/U/R：非法输入与图库/颜色交互未穷尽；本版活动数值确认校验、动态源只校验地址及中点格式；圆环半径/填充宽度、两极柱的精确预览、渐变/刻度/角度基准仍待原厂单变量证据。当前圆环用静态轮廓、仪表用零值样例、柱状用静态半柱；各字段保存不等于精确绘制已验证。 |

只读复核检查了类型绑定、schema12、三草稿寿命、活动地址宽度、全源/设备拒绝、初始化及绘制剪裁；发现并修正按钮启用推测，补动态源中点格式校验。即刻输入验证和弹窗全量文案未声明一致。

## 清理

9次原厂调查已归档后删除323个自有运行/宿主临时/可再生文件；三份物理下载共删除138个自有文件。均按绝对路径、SHA和文件大小核验，不删除其他任务内容、源样本、关键DAT、原始压缩证据或手册assets。重复软件图/KSP清理另见cleanup.json；临时路径位于Z盘，逻辑字节不冒充物理释放量。
'''
(work/'docs/图表手册实现与验收.md').write_text(report,encoding='utf-8')
spec=dict(id='C22',ordinal=22,name='图表',slug='chart',task='S04-08',next='C23',usbAccessed=True,hardwareRun='docs/stage4-evidence/chart/physical/chart-bar-baseline-r1/run.json',expectedPassed=184,testRun='docs/stage4-evidence/chart/windows-final/run.json',nativeManifest='../docs/stage4-evidence/chart/native/native-manifest.json',report='docs/图表手册实现与验收.md',manual='design_plan/图表功能与交互复刻手册.md',scope=scope,unresolved=limitations,nativeSummary=native,protocol='三形态单根原厂源、编译关键DAT及三份实际下载见[汇总](stage4-evidence/chart/hardware-summary.json)，源SHA和字节表见[专项报告](图表手册实现与验收.md)。四项清除/底层更新均false，源SHA一致，DownLoadDlg/m_table确认完成，进程终止。没有按跨形态或跨进程多变量差异推断记录布局；来源retained_native_reference_source，Lite独立源/设备编码与下载仍阻塞。实际模型能力和拒绝测试见[门槛](stage4-evidence/chart/backend-gates.json)。未抓逐字节USB应答或设备读回，屏显/PLC与权限语义保持未验。',codeFiles='src/chart_properties.*、chart_dialog.*、project_model.*、stage3_project_edits.cpp、stage4_control_actions.cpp、static_text_actions.cpp、main_window.cpp、native_shell_ui.cpp、hmi_canvas.cpp、CMakeLists.txt',reviewFindings=['只读复核后修正图案色启用推测、活动动态源中点格式及地址选择器全跨度；确认三草稿结构、类型绑定、schema12和未知源/设备拒绝。','首次114/1/0保留，重新构建后115/0/0；布局和默认预览调整及非活动非默认草稿往返强化后，最终115/0/0，源59/0/0、SDK宏10/0/0，汇总184/0/0。','三种原厂基线实物下载成功不代表Lite独立编码、动态渲染或设备现场通过；精确绘制及源码格式未决明确记录。'])
write(root/'chart-finalize.json',spec)
print(json.dumps(dict(softwarePassed=184,nativeBaselineDownloads=3,independentEncoding='blocked'),ensure_ascii=False))
