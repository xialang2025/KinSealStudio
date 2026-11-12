"""Publish C26 from actual passing tests and a completed native-reference transfer; do not infer firmware semantics."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
root=Path(__file__).resolve().parent;work=root/'worktree';base=work/'docs/stage4-evidence/data-group'
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def write(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
parser=argparse.ArgumentParser();parser.add_argument('--run',default='windows-r1');args=parser.parse_args()
run=read(base/args.run/'run.json');assert run['passed']
assert [r['totals'] for r in run['runs']]==[dict(passed=142,failed=0,skipped=0),dict(passed=59,failed=0,skipped=0),dict(passed=10,failed=0,skipped=0)]
names=['groupNativeDefaultsAndSingleChannelLimits','groupSampleAndMaximumDraftLifetime','groupPointBufferAndIndependentAddressRoles','groupStrictPersistenceAndBackendGates','groupStaticFillOwnershipAndClipping','groupShellPlacementEditAndUndo']
log=(base/args.run/'KinSealStage4ControlTests.txt').read_text(encoding='utf-8-sig');assert all('PASS   : Stage4ControlTests::'+name+'()' in log for name in names)
(base/'windows-final').mkdir(exist_ok=False);write(base/'windows-final/run.json',dict(run,evidenceSource=args.run+'/run.json',aggregationOnlyNoTestsInvented=True))
hardware=read(base/'physical/group-baseline-r1/run.json');assert all(hardware[k] for k in ['completed','inputUnchanged','sourceAtTransferMatchesInput','ownedProcessTerminated'])
assert hardware['sourceProvenance']=='retained_native_reference_source' and hardware['liteIndependentEncoderVerified'] is False
assert all(not x['afterChecked'] for x in hardware['nativeHardware']['transferOptions'])
assert read(base/'native/cleanup.json')['deletedFiles']==104 and read(base/'physical/group-baseline-r1/cleanup.json')['deletedFiles']==46
shutil.copy2(base/args.run/'group-mixed.ksp',work/'samples/stage4/data-group.ksp')
inputs=base/'inputs';inputs.mkdir(exist_ok=True);shutil.copy2(base/args.run/'group-only.ksp',inputs/'group-only.ksp')
environment=os.environ.copy();environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'+environment.get('PATH','')
temporary=root/'task-temp/group-gates';temporary.mkdir(parents=True,exist_ok=True);environment.update(TEMP=str(temporary),TMP=str(temporary),TMPDIR=str(temporary))
executable=work/'build/KinSealStudioLite.exe';calls=[]
for label,argv in [('capabilities',['--project-capabilities',str(inputs/'group-only.ksp')]),('compile',['--compile-project',str(inputs/'group-only.ksp'),str(inputs/'unpublished-package')]),('export-source',['--export-native-project',str(inputs/'group-only.ksp'),str(inputs/'unpublished.sm')])]:
 r=subprocess.run([str(executable),*argv],capture_output=True,env=environment,timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
 stdout=r.stdout.decode('utf-8-sig',errors='replace');calls.append(dict(label=label,exitCode=r.returncode,stdout=stdout,stderr=r.stderr.decode('utf-8-sig',errors='replace'),result=json.loads(stdout)))
assert calls[0]['exitCode']==0 and calls[0]['result']['modelPersistence']['allowed']
assert calls[1]['exitCode']!=0 and calls[2]['exitCode']!=0 and not (inputs/'unpublished.sm').exists() and not (inputs/'unpublished-package').exists()
write(base/'backend-gates.json',dict(executableSha256=sha(executable),inputPath=str((inputs/'group-only.ksp').relative_to(work)),inputSha256=sha(inputs/'group-only.ksp'),calls=calls,usbAccessed=False,sourceAndDeviceRefusalsRetained=True))
scope='独立DataGroupProperties/Dialog，通道/显示/仅显现权限三页、固定单通道、7类型和10..800声明采样点、常量/地址采样草稿、Y轴常量最小值与常量/地址最大值、X仅字号和Y完整刻度、完整声明缓冲区跨度及独立采样/最大值复制角色、26类/单类schema12、创建/取消/侧栏/撤销、固定PC示意图及原厂基线实物下载'
native='原厂数据类型实为I16/I32/U16/U32/BCD16/BCD32/F32有符号优先7项，纠正旧手册顺序；通道固定1..1，采样点10..800、两种采样模式均保留点数编辑，采样地址仅地址模式启用；线型编号0..4。Y范围来源为常量/地址，地址模式仍显示常量最小值且只将最大值改为地址；X只有字号，X数值范围/位数字段隐藏。两轴主3..10/次1..3，Y总位1..10、小数0..3。r2地址采样LW10/250点、动态最大值LW20及线型3在重开保留；地址范围的Y_Min/Y_Max重开回0/1000，不能声称-20/5000原生持久化通过。'
rows=[]
for name,label in [('group-default.sm','group-default-r1'),('group-address.sm','group-default-r2')]:
 source=base/'native'/name;folder=base/'native/compiled'/label
 if not folder.is_dir() and label=='group-default-r2':folder=base/'native/compiled/group-address-r2'
 assert folder.is_dir(),folder
 rows.append(f'| {name} | {source.stat().st_size} | `{sha(source)}` | {(folder/"ControlPara.dat").stat().st_size} | {(folder/"ControlPixel.dat").stat().st_size} |')
report=f'''# 数据群组：手册实现与验收

2026-11-13，阶段4第26/41项，累计21类新增编辑/KSP切片。完整源/设备及现场仍有未决，不代表整控件或阶段4完成。

## 已实现与原厂订正

{scope}。

{native}

当前目标仅一条曲线，不沿用趋势图/XY的8通道、清除位或采样时间字段；严格JSON拒绝这些外来字段。Y地址范围只有最大值地址，最小值输入仍可见，没有编造最小值地址。采样地址运行含义、指针变化、缓冲区读取/绘制时序尚未恢复；地址校验按声明点数×数值字宽保护整个缓冲区，动态最大值按所选字宽保护，采样字按一字保护，均是明确的编辑保护，不代表固件格式/读取顺序已证实。32位800点共1600字，LW2497通过、2498拒绝。

地址/常量草稿在同一属性窗口切换时保留；地址范围确认或重新打开属性时，Y最小/常量最大值归0/1000，最大值地址保留，已按本轮原厂观察实现并测试。KSP模型可显式记录有界草稿，编辑器执行上述确认/重开归一；这不代表SM/设备格式已兼容，独立输出仍拒绝。范围有限数值和递增保护、无效非活动最大值草稿的有界保留为明确Lite策略。固定示意曲线不读取PLC，也不解析采样地址运行值。

## 软件验收

[实际Windows私有桌面测试](stage4-evidence/data-group/{args.run}/run.json)：控件142/0/0、原厂源59/0/0、SDK宏10/0/0，共211/0/0。6项新增方法全部实际PASS，覆盖固定单通道/实测字段边界、常量/地址切换草稿、采样点保持可编辑及地址范围确认/重开归一、Y最大值独立地址、1600字完整跨度与采样/触发/最大值独立复制及失败原子性、26类/单类严格schema12和未知编码拒绝、蓝外底/白绘图区的固定预览剪裁、真实菜单创建/取消/侧栏/编辑/撤销。

[通道页](stage4-evidence/data-group/{args.run}/group-channel.png)、[显示页](stage4-evidence/data-group/{args.run}/group-display.png)来自实际后台测试；[26类样例](../samples/stage4/data-group.ksp)保留地址采样/最大值来源草稿。只读复核检查构造期信号、常量/地址互斥容器、固定单曲线、采样角色枚举边界、完整点缓冲区、旧字段拒绝及不可变输出。未声称逐像素等同原厂图标预览、固件曲线或SM中数值范围的二进制布局。

## 原厂编译与实物下载

| 原源 | 字节数 | SHA-256 | ControlPara.dat | ControlPixel.dat |
|---|---:|---|---:|---:|
{chr(10).join(rows)}

[原厂清单](stage4-evidence/data-group/native/native-manifest.json)保留两轮完整原始gzip/SHA、原源和关键DAT；[观测字段](stage4-evidence/data-group/native/observed-fields.json)区分切换草稿与保存重开结果。

[原厂基线实物下载](stage4-evidence/data-group/physical/group-baseline-r1/run.json)完成：四项清除/固件更新false，输入/待传源SHA一致，原厂日志完成、进程终止。来源retained_native_reference_source，Lite独立编码未通过；[实际CLI门槛](stage4-evidence/data-group/backend-gates.json)确认模型可保存，独立SM/设备编译继续拒绝且无输出。屏显/触控/PLC未据日志声明通过。

## 清理与未决

调查归档清理104文件，物理下载后清46文件；软件重复PNG/KSP按SHA保留对照副本后清理，见[记录](stage4-evidence/data-group/cleanup.json)。全部新临时目录独占Z盘，未新增C盘Frida缓存。主目录既有阶段3修改保持隔离。

- Q-S04-C26-001：独立SM、数据群组设备/Curve.dat记录与Lite独立下载、地址范围数值草稿的原生保存映射未恢复。
- Q-S04-C26-002：采样地址运行含义、点缓冲区PLC排列、32位字序、触发/周期/取满时序待刺激验证。
- Q-S04-C26-003：原厂默认Y轴1000与旧画布100显示关系、动态最大值运行/边界、页面重入和异常数据待验。
- Q-S04-C26-004：实屏、触控、PLC和完整像素对照待补证；未将PC固定曲线计为实际数据运行。

下一项C27：动态报警条。未决层保持阻塞。
'''
(work/'docs/数据群组手册实现与验收.md').write_text(report,encoding='utf-8',newline='\n')
spec=dict(id='C26',ordinal=26,name='数据群组',slug='data-group',task='S04-09',next='C27',usbAccessed=True,hardwareRun='docs/stage4-evidence/data-group/physical/group-baseline-r1/run.json',expectedPassed=211,testRun='docs/stage4-evidence/data-group/windows-final/run.json',nativeManifest='../docs/stage4-evidence/data-group/native/native-manifest.json',report='docs/数据群组手册实现与验收.md',manual='design_plan/数据群组功能与交互复刻手册.md',scope=scope,unresolved=[f'Q-S04-C26-{i:03}' for i in range(1,5)],nativeSummary=native,protocol='数据群组固定单曲线，完整声明点缓冲区和独立采样/最大值地址分别保护；原厂默认/地址变体已保存重开编译，关键DAT和源SHA见报告/清单。地址范围的数值最小/常量最大值重开归0/1000，编辑确认/重开按原厂归0/1000，独立SM保存映射仍未恢复。原厂默认已实物下载，四清除/更新false，输入/待传SHA一致；Lite独立源/设备编译仍实际拒绝，没有新增USB逐字节、显示/触控或PLC结论。',codeFiles='src/data_group_properties.*、data_group_dialog.*、project_model.*、stage3_project_edits.*、stage4_control_actions.cpp、static_text_actions.cpp、main_window.cpp、native_shell_ui.cpp、hmi_canvas.cpp、CMakeLists.txt',reviewFindings=['只读复核覆盖固定单曲线、源模式切换草稿/可见性、声明完整缓冲区、采样枚举上界及独立采样/最大值角色、严格26类JSON和拒绝输出。','六项新增方法实际PASS；控件142+源59+SDK宏10=211/0/0，未将邻近趋势/XY字段冒充数据群组。','原厂地址范围重开数值回0/1000，编辑确认/重开归一已覆盖，KSP显式字段与原生格式仍分层；原厂基线下载不等于独立编码或运行验收。'])
write(root/'group-finalize.json',spec)
subprocess.run([sys.executable,'-B',str(root/'finalize_xy.py'),str(root/'group-finalize.json')],check=True)
repro=base/'reproduction';repro.mkdir(exist_ok=True)
for name in ['prepare_group_probe.py','prepare_group_survey.py','make_group_model.py','make_group_dialog.py','prepare_group_integration.py','integrate_group.py','add_group_tests.py','refine_group_native_lifetime.py','inspect_group_survey.py','archive_native_incremental.py','execute_native_cleanup.ps1','archive_physical_run.py','run_stage4_native_physical.py','prepare_group_delivery.py','deliver_group_docs.py','group-finalize.json','finalize_xy.py','prepare_control_cleanup.py','execute_control_cleanup.ps1']:
 shutil.copy2(root/name,repro/name)
for log in root.glob('group-build-*.log'):shutil.copy2(log,base/log.name)
print('C26 documentation prepared from 211 actual passing tests and completed reference transfer.')
