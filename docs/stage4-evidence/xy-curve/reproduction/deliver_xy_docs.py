"""Publish C25 from actual passing tests and a completed native-reference transfer; do not infer firmware semantics."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
root=Path(__file__).resolve().parent;work=root/'worktree';base=work/'docs/stage4-evidence/xy-curve'
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def write(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
parser=argparse.ArgumentParser();parser.add_argument('--run',default='windows-r1');args=parser.parse_args()
run=read(base/args.run/'run.json');assert run['passed']
assert [r['totals'] for r in run['runs']]==[dict(passed=136,failed=0,skipped=0),dict(passed=59,failed=0,skipped=0),dict(passed=10,failed=0,skipped=0)]
names=['xyNativeBoundsAndDataTypeLifetime','xyIndependentChannelsAndRemovalReset','xyAxisHiddenFieldsAndAccessSpans','xyStrictPersistenceAndBackendGates','xyStaticRasterAndClipping','xyShellPlacementEditAndUndo']
log=(base/args.run/'KinSealStage4ControlTests.txt').read_text(encoding='utf-8-sig');assert all('PASS   : Stage4ControlTests::'+name+'()' in log for name in names)
(base/'windows-final').mkdir(exist_ok=False);write(base/'windows-final/run.json',dict(run,evidenceSource=args.run+'/run.json',aggregationOnlyNoTestsInvented=True))
hardware=read(base/'physical/xy-baseline-r1/run.json');assert all(hardware[k] for k in ['completed','inputUnchanged','sourceAtTransferMatchesInput','ownedProcessTerminated'])
assert hardware['sourceProvenance']=='retained_native_reference_source' and hardware['liteIndependentEncoderVerified'] is False
assert all(not x['afterChecked'] for x in hardware['nativeHardware']['transferOptions'])
assert read(base/'native/cleanup.json')['deletedFiles']==104 and read(base/'physical/xy-baseline-r1/cleanup.json')['deletedFiles']==46
shutil.copy2(base/args.run/'xy-mixed.ksp',work/'samples/stage4/xy-curve.ksp')
inputs=base/'inputs';inputs.mkdir(exist_ok=True);shutil.copy2(base/args.run/'xy-only.ksp',inputs/'xy-only.ksp')
environment=os.environ.copy();environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'+environment.get('PATH','')
temporary=root/'task-temp/xy-gates';temporary.mkdir(parents=True,exist_ok=True);environment.update(TEMP=str(temporary),TMP=str(temporary),TMPDIR=str(temporary))
executable=work/'build/KinSealStudioLite.exe';calls=[]
for label,argv in [('capabilities',['--project-capabilities',str(inputs/'xy-only.ksp')]),('compile',['--compile-project',str(inputs/'xy-only.ksp'),str(inputs/'unpublished-package')]),('export-source',['--export-native-project',str(inputs/'xy-only.ksp'),str(inputs/'unpublished.sm')])]:
 r=subprocess.run([str(executable),*argv],capture_output=True,env=environment,timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
 stdout=r.stdout.decode('utf-8-sig',errors='replace');calls.append(dict(label=label,exitCode=r.returncode,stdout=stdout,stderr=r.stderr.decode('utf-8-sig',errors='replace'),result=json.loads(stdout)))
assert calls[0]['exitCode']==0 and calls[0]['result']['modelPersistence']['allowed']
assert calls[1]['exitCode']!=0 and calls[2]['exitCode']!=0 and not (inputs/'unpublished.sm').exists() and not (inputs/'unpublished-package').exists()
write(base/'backend-gates.json',dict(executableSha256=sha(executable),inputPath=str((inputs/'xy-only.ksp').relative_to(work)),inputSha256=sha(inputs/'xy-only.ksp'),calls=calls,usbAccessed=False,sourceAndDeviceRefusalsRetained=True))
scope='独立XyCurveProperties/Dialog，通道/显示/仅显现权限三页、共用触发/读取/清除地址、有符号优先7类型、1..100采样点/1..255秒/1..8独立通道、两套数值轴范围/位数/颜色/隐藏草稿、完整X/Y成对读取跨度保护与独立位复制角色、25类及单类schema12、创建/取消/侧栏/撤销、固定PC坐标示意图及原厂基线实物下载'
native='默认样本与多通道/坐标轴变体均在原厂保存、重开、编译成功。实测采样点1..100、时间1..255、通道1..8；X/Y主分隔3..10、次分隔1..3，总位数1..10、小数0..3，字号12/16/24/48。第三通道-10/2000与线型2在通道切换后保留，第一通道仍0/1000；X轴-100/4000、总位6/小数2与Y轴-20/5000保留至重开。关闭X格线时隐藏本列格线色；关闭X标识时隐藏本列整组字号、最小/最大、总位及小数；本控件没有实时趋势图的绘制方向控件。'
rows=[]
for name,label in [('xy-default.sm','xy-default-r1'),('xy-custom.sm','xy-custom-r2')]:
 source=base/'native'/name;folder=base/'native/compiled'/label
 rows.append(f'| {name} | {source.stat().st_size} | `{sha(source)}` | {(folder/"ControlPara.dat").stat().st_size} | {(folder/"ControlPixel.dat").stat().st_size} |')
report=f'''# X-Y曲线：手册实现与验收

2026-11-13，阶段4第25/41项，累计20类新增编辑/KSP切片。源/设备及现场未决仍保留，不代表全控件或阶段4已完成。

## 实现与原厂补证

{scope}。

{native}

X/Y复用已审计的纯通道/轴结构；属性载体、JSON和控件类型独立，不接受趋势图的rightToLeft字段。读地址按“每通道两个坐标×类型字数×通道数”做完整跨度保护；这是依据成对坐标定义采用的保守编辑保护，不代表已验证固件X/Y地址排序、读取一致性或设备二进制布局。32位8通道按32字处理，LW4065通过、4066拒绝。缩减再增长重置移除草稿、活动范围严格递增等为明确Lite策略。

## 软件验证

[真实Windows私有桌面测试]({"stage4-evidence/xy-curve/"+args.run+"/run.json"})：控件136/0/0、原厂源59/0/0、配置SDK宏10/0/0，共205/0/0。6项新增用例全部在实际日志中PASS，覆盖实测字段上限、7类型切换、独立通道草稿与移除策略、两套数值轴的隐藏字段、完整成对跨度及触发/清除位复制、25类混合/单类schema12和非默认X轴-100/4000/6/2往返、固定坐标图剪裁与真实菜单创建/取消/侧栏/撤销。

[通道页]({"stage4-evidence/xy-curve/"+args.run+"/xy-channel.png"})、[显示页]({"stage4-evidence/xy-curve/"+args.run+"/xy-display.png"})为实际后台截图；预览和画布使用固定示意坐标，未宣称真实PLC采样或与原厂预览逐像素一致。[25类样例](../samples/stage4/xy-curve.ksp)保留独立历史采集定义与X/Y字段。只读复核检查类型分流、构造期信号、固定数组草稿、完整字跨度、严格JSON及未知编码拒绝；构建前已纠正生成片段括号/专有属性引用并增强非默认X轴持久化核验。

## 原厂编译与实物下载

| 原源 | 字节数 | SHA-256 | ControlPara.dat | ControlPixel.dat |
|---|---:|---|---:|---:|
{chr(10).join(rows)}

[原厂清单](stage4-evidence/xy-curve/native/native-manifest.json)保留两轮完整原始gzip、SHA、原源和必要非空DAT；[字段观测表](stage4-evidence/xy-curve/native/observed-fields.json)列出独立通道、X/Y范围与隐藏变化。跨进程多变量样本不用于猜测固件布局。

[原厂默认基线真实下载](stage4-evidence/xy-curve/physical/xy-baseline-r1/run.json)完成：清配方/历史数据/历史报警/固件更新四项全部false，待传源SHA与输入一致，原厂日志确认完成、拥有进程结束。Lite独立编码仍未通过；[实际CLI门槛](stage4-evidence/xy-curve/backend-gates.json)确认模型允许保存，独立SM/设备编译继续拒绝且没有生成输出。显示、触控和PLC结果未据下载日志声明通过。

## 清理与未决

原厂调查归档后清理104文件，下载后另清46文件；软件重复PNG/KSP清理详见[清理记录](stage4-evidence/xy-curve/cleanup.json)。Qt/GCC/Frida均使用任务独占Z盘临时目录；C盘仍有余量。主目录阶段3既存改动保持隔离。

- Q-S04-C25-001：独立SM、X/Y设备记录、Curve.dat及Lite独立下载尚未恢复。
- Q-S04-C25-002：成对地址排列、32位字序、X/Y一致性和触发/周期/清除/取满时序待设备刺激验证。
- Q-S04-C25-003：页面离开重入、缓存保持/清空、空点、越界/异常数据与权限变化的固件行为未验。
- Q-S04-C25-004：实屏与PLC观测、进一步异常输入原厂验证及完整像素对照待补证。

下一项C26：数据群组；未决层保持阻塞。
'''
(work/'docs/X-Y曲线手册实现与验收.md').write_text(report,encoding='utf-8',newline='\n')
spec=dict(id='C25',ordinal=25,name='X-Y曲线',slug='xy-curve',task='S04-09',next='C26',usbAccessed=True,hardwareRun='docs/stage4-evidence/xy-curve/physical/xy-baseline-r1/run.json',expectedPassed=205,testRun='docs/stage4-evidence/xy-curve/windows-final/run.json',nativeManifest='../docs/stage4-evidence/xy-curve/native/native-manifest.json',report='docs/X-Y曲线手册实现与验收.md',manual='design_plan/X-Y曲线功能与交互复刻手册.md',scope=scope,unresolved=[f'Q-S04-C25-{i:03}' for i in range(1,5)],nativeSummary=native,protocol='原厂X-Y默认与通道/轴变体保存、重开、编译成功；关键DAT与源SHA见专项报告/清单。成对地址跨度按2×类型字数×通道数保守保护，未认定固件排列或时序。默认原厂基线已实物下载，四项清除/更新false、输入/待传SHA一致且原厂日志完成；Lite独立SM/设备编译实际仍拒绝。没有新增逐字节USB、屏显/触控或PLC结论。',codeFiles='src/xy_curve_properties.*、xy_curve_dialog.*、project_model.*、stage3_project_edits.cpp、stage4_control_actions.cpp、static_text_actions.cpp、main_window.cpp、native_shell_ui.cpp、hmi_canvas.cpp、CMakeLists.txt',reviewFindings=['只读复核覆盖独立X/Y类型分流、两轴字段生命周期、完整成对坐标跨度、触发/清除位角色、严格25类JSON和原厂/独立输出门槛。','构建前纠正代码生成括号和专有属性引用，增强非默认X轴范围/位数往返；六项新增测试全部实际PASS，136+59+10=205/0/0。','保守成对地址保护不代表固件地址排序已恢复；原厂基线下载不替代Lite独立编码、显示或PLC验证。'])
write(root/'xy-finalize.json',spec)
program=work/'程序修改记录.md';text=program.read_text(encoding='utf-8')
if '## 2026年11月13日' not in text:
 with program.open('a',encoding='utf-8',newline='\n') as stream:stream.write('\n\n## 2026年11月13日：\n')
finalizer=root/'finalize_xy.py';finalizer.write_text((root/'finalize_control.py').read_text(encoding='utf-8').replace('2026-11-12','2026-11-13').replace('20261112','20261113').replace('//20261113-0','//20261113-1'),encoding='utf-8',newline='\n')
subprocess.run([sys.executable,'-B',str(finalizer),str(root/'xy-finalize.json')],check=True)
repro=base/'reproduction';repro.mkdir(exist_ok=True)
for name in ['prepare_xy_probe.py','prepare_xy_survey.py','make_xy_model_ui.py','integrate_xy.py','add_xy_tests.py','review_xy_before_build.py','refine_xy_test_coverage.py','archive_native_incremental.py','execute_native_cleanup.ps1','archive_physical_run.py','run_stage4_native_physical.py','deliver_xy_docs.py','xy-finalize.json','finalize_xy.py','prepare_control_cleanup.py','execute_control_cleanup.ps1']:
 shutil.copy2(root/name,repro/name)
for log in root.glob('xy-build-*.log'):shutil.copy2(log,base/log.name)
print('C25 documentation prepared from 205 actual passing tests and completed reference transfer.')
