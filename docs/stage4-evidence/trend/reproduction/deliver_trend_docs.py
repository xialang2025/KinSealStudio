"""Finalize C23 only after six registered trend tests, baseline transport and exact cleanup recovery evidence."""
from pathlib import Path
import copy
import hashlib
import json
import os
import shutil
import subprocess
root=Path(__file__).resolve().parent
work=root/'worktree'; base=work/'docs/stage4-evidence/trend'
def read(p): return json.loads(p.read_text(encoding='utf-8-sig'))
def write(p,d): p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
final=read(base/'windows-r3/run.json'); prior=read(base/'windows-r1/run.json')
assert final['passed'] and final['runs'][0]['totals']==dict(passed=121,failed=0,skipped=0)
text=(base/'windows-r3/KinSealStage4ControlTests.txt').read_text(encoding='utf-8')
names=['trendNativeBoundsAndDataTypeLifetime','trendIndependentChannelsAndRemovalReset','trendAxisHiddenFieldsAndAccessSpans','trendStrictPersistenceAndBackendGates','trendStaticRasterAndClipping','trendShellPlacementEditAndUndo']
assert all('PASS   : Stage4ControlTests::'+name+'()' in text for name in names)
assert prior['runs'][1]['totals']==dict(passed=59,failed=0,skipped=0) and prior['runs'][2]['totals']==dict(passed=10,failed=0,skipped=0)
final['runs']=[dict(final['runs'][0],evidenceSource='windows-r3/run.json'),dict(prior['runs'][1],evidenceSource='windows-r1/run.json'),dict(prior['runs'][2],evidenceSource='windows-r1/run.json')]
final['actualSourceRecords']=['windows-r3/run.json','windows-r1/run.json']; final['aggregationOnlyNoTestsInvented']=True
(base/'windows-final').mkdir(exist_ok=False); write(base/'windows-final/run.json',final)
hardware=read(base/'physical/trend-baseline-r1/run.json')
assert all(hardware[k] for k in ['completed','inputUnchanged','sourceAtTransferMatchesInput','ownedProcessTerminated']) and hardware['sourceProvenance']=='retained_native_reference_source' and hardware['liteIndependentEncoderVerified'] is False
assert all(not x['afterChecked'] for x in hardware['nativeHardware']['transferOptions']) and (base/'physical/trend-baseline-r1/cleanup.json').exists()
assert read(base/'native/cleanup-recovery.json')['priorVerifiedDeletedFiles']==133 and read(base/'native/cleanup.json')['deletedFiles']==50
shutil.copy2(base/'windows-r3/trend-mixed.ksp',work/'samples/stage4/trend.ksp')
inputs=base/'inputs'; inputs.mkdir(exist_ok=True)
only=read(base/'windows-r3/trend-only.ksp'); write(inputs/'trend-only.ksp',only)
environment=os.environ.copy(); environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'+environment.get('PATH','')
temporary=root/'task-temp/trend-gates'; temporary.mkdir(parents=True,exist_ok=True); environment.update(TEMP=str(temporary),TMP=str(temporary),TMPDIR=str(temporary))
executable=work/'build/KinSealStudioLite.exe'; calls=[]
for label,args in [('capabilities',['--project-capabilities',str(inputs/'trend-only.ksp')]),('compile',['--compile-project',str(inputs/'trend-only.ksp'),str(inputs/'unpublished-package')]),('export-source',['--export-native-project',str(inputs/'trend-only.ksp'),str(inputs/'unpublished.sm')])]:
 r=subprocess.run([str(executable),*args],capture_output=True,env=environment,timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
 stdout=r.stdout.decode('utf-8-sig',errors='replace'); calls.append(dict(label=label,exitCode=r.returncode,stdout=stdout,stderr=r.stderr.decode('utf-8-sig',errors='replace'),result=json.loads(stdout)))
assert calls[0]['exitCode']==0 and calls[0]['result']['modelPersistence']['allowed']
assert calls[1]['exitCode']!=0 and calls[2]['exitCode']!=0 and not (inputs/'unpublished.sm').exists() and not (inputs/'unpublished-package').exists()
write(base/'backend-gates.json',dict(executableSha256=sha(executable),inputPath=str((inputs/'trend-only.ksp').relative_to(work)),inputSha256=sha(inputs/'trend-only.ksp'),calls=calls,usbAccessed=False,sourceAndDeviceRefusalsRetained=True))
repro=base/'reproduction'; repro.mkdir(exist_ok=True)
for name in ['prepare_trend_probe.py','prepare_trend_ui.py','prepare_trend_channel_probe.py','prepare_trend_reopen.py','integrate_trend.py','add_trend_helpers.py','refine_trend_review.py','refine_trend_final.py','archive_native_incremental.py','execute_native_cleanup.ps1','recover_trend_cleanup.py','run_stage4_native_physical.py','archive_physical_run.py','deliver_trend_docs.py']:
 shutil.copy2(root/name,repro/name)
compat=base/'prior-C22-main-compatibility'; compat.mkdir(exist_ok=True)
for name in ['run.json','KinSealStage4ControlTests.txt']: shutil.copy2(root/'chart-main-windows-r1'/name,compat/name)
shutil.copy2(root/'chart-main-compatibility-build.log',compat/'build.log')
rows=[]
for name,label in [('trend-default.sm','trend-default-r1'),('trend-custom.sm','trend-default-r2'),('trend-channel-custom.sm','trend-default-r3')]:
 source=base/'native'/name; folder=base/'native/compiled'/label
 rows.append(f'| {name} | {source.stat().st_size} | `{sha(source)}` | {(folder/"ControlPara.dat").stat().st_size} | {(folder/"ControlPixel.dat").stat().st_size} |')
table='\n'.join(rows)
scope='独立TrendProperties/Dialog，通道/显示/仅显现权限三页、共用触发/读取/清除地址、签名优先7类型、采样1..100点与1..255秒、1..8通道独立范围/线型/颜色及删除重建复位、X/Y轴隐藏字段保留；23类混合/单类schema12、读取整段跨度、独立触发/清除复制角色、侧栏/创建/取消/撤销，固定PC示意曲线和原厂基线实物下载'
native='本轮原厂观察确认采样时间1..255、X/Y次分隔数1..3、Y总位数1..10；范围字段实际为LE_Min/LE_Max与Y_Min/Y_Max文本框。七类型切换不复位通道0/1000范围；第三通道编辑完成信号后-10/2000和线型2在切换及新原厂进程重开后保留；缩减再增加通道会重置被移除通道，属性窗重开回到第1通道。Y显示刻度标识关闭隐藏整组字号/最小/最大/总位数/小数位字段，X只隐藏字号行。只有一组读取基地址和数据类型，未观察到每通道独立地址/类型。'
report=f'''# 趋势图：手册实现与验收

2026-11-12，阶段4第23/41项。新增18类编辑/KSP切片的独立源/设备格式仍阻塞，当前不是整阶段完成。

## 已实现及原厂补证

{scope}。

{native}

通道配置必须以真实编辑完成为比较前提：r2只设置文本而未发editingFinished，切换后恢复旧值，不能据此认定原厂不支持范围编辑；r3补完整信号，r4在新进程读取保存源，完成独立范围持久化闭环。原始JSON压缩保留并记录SHA，三份原源和必要DAT均保留。

## 软件验证

[最终控件121/0/0](stage4-evidence/trend/windows-r3/run.json)、[原厂源59/0/0及配置SDK宏10/0/0](stage4-evidence/trend/windows-r1/run.json)，[按真实来源汇总190/0/0](stage4-evidence/trend/windows-final/run.json)。新增6项核验采样与类型寿命、独立通道与移除重置、X/Y隐藏字段和完整读取跨度/独立位角色复制、严格23类/单类嵌套KSP、实际剪裁像素、工具箱创建/取消/侧栏/撤销。

首轮Qt元对象注册缓存只运行旧115项，日志原样保留；核对moc/对象时间后重新构建，第二轮新增用例实际运行120/1/0，失败源于测试把本地LW范围误当作0..65535。已按既有0..4096边界用4081/4082检验16字跨度，并加强C22两字跨度边界，最终121/0/0明确包含6个趋势图方法的PASS行。没有把旧115项当作新增测试通过。只读复核还发现极小尺寸绘图区可为负，已补剪裁保护；构造期预览信号有空指针保护，各字段持有稳定草稿。

[通道页](stage4-evidence/trend/windows-r3/trend-channel.png)、[显示页](stage4-evidence/trend/windows-r3/trend-display.png)为实际后台测试截图，通道图在类型切换后恢复默认I16再拍摄；不声明全部布局/渲染与原厂逐像素一致。[23类样例](../samples/stage4/trend.ksp)及单类门槛样例保留非默认第三通道与隐藏轴草稿。

用户主目录C22同步后的[单独兼容检查](stage4-evidence/trend/prior-C22-main-compatibility/run.json)为115/0/0，主程序和控件目标构建成功，保留其18个阶段3既有改动。此检查只覆盖C22版本，没有声称阶段3全部残留已修复，也不冒充C23主目录测试。

## 真实下载与格式边界

[原厂基线实物记录](stage4-evidence/trend/physical/trend-baseline-r1/run.json)：唯一健康7365:616C、libusb0，原厂默认单控件源→原厂编译→原厂USB下载成功，DownLoadDlg/m_table确认“下载完成”；四项清除/底层更新全部false，源未改、传输源SHA一致，进程已终止，下载编译包保留。

| 原厂源 | 字节 | SHA256 | ControlPara字节 | ControlPixel字节 |
|---|---:|---|---:|---:|
{table}

三份样本跨进程、多字段变化，不能按总长差异或多变量差分推断设备记录。r4是保存源重开，不是额外独立编译。下载来源retained_native_reference_source、liteIndependentEncoderVerified=false；原厂成功不能替代新版生成下载。

[实际门槛](stage4-evidence/trend/backend-gates.json)确认单类KSP允许保存，但源导出/独立编译继续拒绝，未留下部分SM/下载包。本轮没有新增实屏观察、触控或PLC采样通过。

## 未决项与当前保护

| 编号 | 仍需完成的证据与实现 |
|---|---|
| Q-S04-C23-001 | P_SM：趋势图独立源、通道/轴/权限和修改写回格式未恢复。 |
| Q-S04-C23-002 | C/独立T：设备记录、地址/取样/曲线资源布局未恢复；保持源和设备输出拒绝。 |
| Q-S04-C23-003 | E/运行：PLC实时采样缓存、取满滚动、触发0→1、清除停止、页面生命周期、并发通道时序及权限组合未实现/验证；当前固定示意曲线不代表真实数据。 |
| Q-S04-C23-004 | D/U/R：非法/相等范围、全部颜色对话框、轴总位数/次刻度/每通道数值与线宽精确绘制、方向的设备语义未穷尽；本版明确要求活动范围min<max和有界有限标量，属于Lite保护，不冒充原厂弹窗/边界。总位数和次分隔值持久化，精确格式/绘制待证。 |

## 清理

原厂调查计划开始后发现首轮构建与调查共享Z临时目录，Resolve-Path在第133项后的编译临时.s自然消失时中止。保留133项此前逐项SHA核对删除记录，等待构建退出0后重新核验剩余50项并清理；3项编译临时自然消失不计入删除量，见[恢复记录](stage4-evidence/trend/native/cleanup-recovery.json)及[后续结果](stage4-evidence/trend/native/cleanup.json)。源、压缩证据及关键DAT均完整。已分离task-temp/build-trend与调查/物理下载临时根，避免重现。

物理下载另清46个自有文件；重复软件PNG/KSP清理见cleanup.json。不删除其他任务资料、手册assets或主目录既有改动；逻辑文件字节不等于物理释放量。
'''
(work/'docs/趋势图手册实现与验收.md').write_text(report,encoding='utf-8')
spec=dict(id='C23',ordinal=23,name='趋势图',slug='trend',task='S04-09',next='C24',usbAccessed=True,hardwareRun='docs/stage4-evidence/trend/physical/trend-baseline-r1/run.json',expectedPassed=190,testRun='docs/stage4-evidence/trend/windows-final/run.json',nativeManifest='../docs/stage4-evidence/trend/native/native-manifest.json',report='docs/趋势图手册实现与验收.md',manual='design_plan/趋势图功能与交互复刻手册.md',scope=scope,unresolved=[f'Q-S04-C23-00{i}' for i in range(1,5)],nativeSummary=native,protocol='原厂默认、Y刻度变体、三通道范围变体源与关键DAT均保留，具体SHA/总长见[专项报告](趋势图手册实现与验收.md)与[原厂清单](stage4-evidence/trend/native/native-manifest.json)。新进程重开证实第三通道-10/2000和线型2保存；未按跨进程多变量样本推断固件布局。原厂基线实物下载日志确认完成，四项清除/更新false，源和传输SHA一致且进程终止。来源retained_native_reference_source、Lite独立编码未通过，模型可保存而独立源/编译实际继续拒绝，见[门槛](stage4-evidence/trend/backend-gates.json)。没有逐字节USB应答、实屏读回或PLC触发/清除/缓存运行证据，相关层保持未验。',codeFiles='src/trend_properties.*、trend_dialog.*、project_model.*、stage3_project_edits.*、stage4_control_actions.cpp、static_text_actions.cpp、main_window.cpp、native_shell_ui.cpp、hmi_canvas.cpp、CMakeLists.txt',reviewFindings=['只读复核覆盖通道寿命、类型改变不复位、缩减重置、轴隐藏草稿、构造期信号、完整读取跨度与独立位角色、schema12和未知编码拒绝；补极小绘图区保护。','首轮moc缓存只运行旧115项；第二轮120/1/0的地址测试使用错误边界，改为本地LW4081/4082覆盖16字跨度，并加强C22两字边界；最终六个趋势图方法全部PASS；控件121/0/0、源59/0/0、SDK宏10/0/0，共190/0/0。','调查清理在133项后遇共享编译临时消失而中止；构建退出0后重新SHA核验清剩余50项，不计3项自然消失；已分离临时根。原厂基线下载不替代独立编码或运行层。'])
write(root/'trend-finalize.json',spec)
print(json.dumps(dict(tests=190,nativeDownload=True,independentEncoder='blocked'),ensure_ascii=False))
