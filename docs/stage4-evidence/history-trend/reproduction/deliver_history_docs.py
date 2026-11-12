"""Publish C24 documentation only from completed test, native-reference transfer and cleanup records."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
root=Path(__file__).resolve().parent; work=root/'worktree'; base=work/'docs/stage4-evidence/history-trend'
def read(p): return json.loads(p.read_text(encoding='utf-8-sig'))
def write(p,d): p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
parser=argparse.ArgumentParser();parser.add_argument('--run',default='windows-r2');args=parser.parse_args()
run=read(base/args.run/'run.json');assert run['passed']
assert [r['totals'] for r in run['runs']]==[dict(passed=130,failed=0,skipped=0),dict(passed=59,failed=0,skipped=0),dict(passed=10,failed=0,skipped=0)]
names=['collectionSharedTypeMemoryAndFullAddressSpan','collectionDialogItemsAndInactiveDrafts','historyIndependentCurvesAndStableNames','historyCheckableAxesAndTimeBounds','historyReferenceLifetimeAndSingleCollectorLimit','historyCrossProjectPasteAndVisibilityRoles','historyStrictPersistenceAndBackendGates','historyStaticRasterAndTimeFormatClipping','historyShellGateCollectionPlacementAndUndo']
log=(base/args.run/'KinSealStage4ControlTests.txt').read_text(encoding='utf-8-sig');assert all('PASS   : Stage4ControlTests::'+name+'()' in log for name in names)
(base/'windows-final').mkdir(exist_ok=False);write(base/'windows-final/run.json',dict(run,evidenceSource=args.run+'/run.json',aggregationOnlyNoTestsInvented=True))
hardware=read(base/'physical/history-baseline-r1/run.json');assert all(hardware[k] for k in ['completed','inputUnchanged','sourceAtTransferMatchesInput','ownedProcessTerminated'])
assert hardware['sourceProvenance']=='retained_native_reference_source' and hardware['liteIndependentEncoderVerified'] is False
assert all(not x['afterChecked'] for x in hardware['nativeHardware']['transferOptions'])
assert (base/'physical/history-baseline-r1/cleanup.json').is_file() and read(base/'native/cleanup.json')['deletedFiles']==267
shutil.copy2(base/args.run/'history-mixed.ksp',work/'samples/stage4/history-trend.ksp')
inputs=base/'inputs';inputs.mkdir(exist_ok=True);shutil.copy2(base/args.run/'history-only.ksp',inputs/'history-only.ksp')
environment=os.environ.copy();environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'+environment.get('PATH','')
temporary=root/'task-temp/history-gates';temporary.mkdir(parents=True,exist_ok=True);environment.update(TEMP=str(temporary),TMP=str(temporary),TMPDIR=str(temporary))
executable=work/'build/KinSealStudioLite.exe';calls=[]
for label,argv in [('capabilities',['--project-capabilities',str(inputs/'history-only.ksp')]),('compile',['--compile-project',str(inputs/'history-only.ksp'),str(inputs/'unpublished-package')]),('export-source',['--export-native-project',str(inputs/'history-only.ksp'),str(inputs/'unpublished.sm')])]:
 result=subprocess.run([str(executable),*argv],capture_output=True,env=environment,timeout=30,creationflags=subprocess.CREATE_NO_WINDOW)
 stdout=result.stdout.decode('utf-8-sig',errors='replace');calls.append(dict(label=label,exitCode=result.returncode,stdout=stdout,stderr=result.stderr.decode('utf-8-sig',errors='replace'),result=json.loads(stdout)))
assert calls[0]['exitCode']==0 and calls[0]['result']['modelPersistence']['allowed']
assert calls[1]['exitCode']!=0 and calls[2]['exitCode']!=0 and not (inputs/'unpublished.sm').exists() and not (inputs/'unpublished-package').exists()
write(base/'backend-gates.json',dict(executableSha256=sha(executable),inputPath=str((inputs/'history-only.ksp').relative_to(work)),inputSha256=sha(inputs/'history-only.ksp'),calls=calls,usbAccessed=False,sourceAndDeviceRefusalsRetained=True))
scope='独立数据采集定义与历史趋势图编辑器；单采集组、共用5数据类型、固定LH项目/稳定ID、完整地址跨度和内存计算、取样/通知与缩放草稿；历史三页、1..min(8,项目数)条独立来源/范围/线型/颜色、X时间轴与Y刻度组、跨页引用保护、同工程复用和跨工程采集冲突拒绝、树新增/编辑/删除与撤销、24类schema12、固定PC示意图及原厂基线实物下载'
native='原厂确认无采集时提示“尚未建立数据采集！”，当前测试机型第二组采集被“已经达到最大数据采集数量，请删除后再新建。”拒绝。采集U16/U32/I16/I32/F32是整组共用类型；LH序号只读；3项×1000点U16占25 kbyte、U32占31 kbyte，公式为(20+取样长度×类型字节数)×取样总数。项目名称需真实textEdited信号后更新数据源；First/Second/Third命名样本在r7保存、重开并编译成功。曲线数默认min(8,项目数)，所有曲线默认来源均为第一个项目；曲线范围独立。X小时0..48、分秒0..59，关闭时间/刻度组只隐藏组内字段，Y最小/最大仍可见。'
rows=[]
for name,label in [('history-default.sm','history-default-r2'),('history-multi.sm','history-default-r4'),('history-renamed.sm','history-default-r7')]:
 source=base/'native'/name;folder=base/'native/compiled'/label
 assert source.is_file() and folder.is_dir(),(name,label)
 rows.append(f'| {name} | {source.stat().st_size} | `{sha(source)}` | {(folder/"ControlPara.dat").stat().st_size} | {(folder/"ControlPixel.dat").stat().st_size} |')
report=f'''# 历史趋势图与数据采集：手册实现与验收

2026-11-12，阶段4第24/41项。累计19类新增编辑/KSP切片；整控件设备运行与全阶段仍未完成。

## 实现与证据

{scope}。

{native}

先前阶段2仅有未实施空配置容器，本轮补上必要的实际采集定义；这不是已恢复设备采集引擎。每个采集项目保留稳定编辑器ID与固定LH序号，改名不改变引用；缩短长度会保护被曲线使用的项目及曲线数量，删除采集要通过全工程校验。跨工程粘贴可将完整采集复制到空目标；已有不同采集时整笔拒绝。相同定义可复用，采集寄存器不会随控件多重复制重复递增。

原厂缩放启用后增益/偏移为空，本版要求填写有限数值后提交，没有假设1/0。63 kbyte使用严格63×1024字节保护，内存显示向下取整；拒绝零时长、非递增活动显示范围、重复/空项目名，以及缩减曲线重置移除草稿，均属于明确的Lite保护或编辑策略，不外推为已证明的原厂运行语义。云上传开关只保存编辑设置。

## 验证

[后台Windows真实测试]({"stage4-evidence/history-trend/"+args.run+"/run.json"})：控件130/0/0、源59/0/0、配置SDK宏10/0/0，合计199/0/0。9项新增方法均在实际日志出现PASS，覆盖共享类型/精确内存上限、完整64字地址跨度、独立命名/缩放、逐曲线来源/范围、checkable时间/刻度组、跨页引用保护、采集复制冲突、24类/仅采集schema12、固定曲线像素剪裁以及真实菜单/树/创建/取消/撤销。

只读复核发现新测试误用粘贴接口参数，首轮构建另在历史图画布误传图表属性处报错，原始日志保留；修复QPoint调用与专有属性路由后重建。正式测试与构建日志保留，未将旧二进制当作新用例通过。对采集字段不伪造历史检索、文件恢复或PLC取样；预览固定示意数据，未宣称逐像素等同原厂。

[采集一般页]({"stage4-evidence/history-trend/"+args.run+"/collection-general.png"})、[数据项页]({"stage4-evidence/history-trend/"+args.run+"/collection-items.png"})、[历史一般页]({"stage4-evidence/history-trend/"+args.run+"/history-general.png"})、[XY轴页]({"stage4-evidence/history-trend/"+args.run+"/history-axes.png"})来自实际私有桌面测试。示例为[24类混合工程](../samples/stage4/history-trend.ksp)。

## 原厂源与实物下载

| 原源 | 字节数 | SHA-256 | ControlPara.dat | ControlPixel.dat |
|---|---:|---|---:|---:|
{chr(10).join(rows)}

完整清单见[原厂调查清单](stage4-evidence/history-trend/native/native-manifest.json)。7轮调查含2轮失败均保留原始gzip和SHA：r5复用已有采集触发数量上限，r6模板残留静态对象导致唯一根断言拒绝保存/编译；r7明确清空模板并断言0根后取得纯历史样本。没有把失败或混合模板当作成功兼容证据。

[原厂基线实物下载](stage4-evidence/history-trend/physical/history-baseline-r1/run.json)完成：输入与待传源SHA一致、四项清除/固件更新false、原厂下载日志确认完成、测试进程结束。来源为retained_native_reference_source；Lite独立编码仍阻塞。[实际输出门槛](stage4-evidence/history-trend/backend-gates.json)确认模型允许保存、独立SM和设备编译继续拒绝且未产生输出。下载完成不替代屏显、历史运行或PLC验证。

## 清理与未决

调查归档后清理267文件，物理下载后另清46文件；软件重复PNG/KSP按SHA保留同内容副本后清理。临时目录均在本任务Z盘，C盘余量见各清理记录。用户原有阶段3改动保持隔离。

- Q-S04-C24-001：采集/历史控件的独立SM及DAT布局、重序列化与Lite独立实物下载尚未恢复。
- Q-S04-C24-002：设备触发/计时/整点、取满通知、云上传、缩放与PLC数据解释未验。
- Q-S04-C24-003：历史文件、时间检索/滚动/缩放、断电恢复及空/异常文件运行未验。
- Q-S04-C24-004：实屏与触控结果待用户观察；界面完整逐像素对照及进一步异常输入策略待后续补证。

下一项C25：X-Y曲线。上述未决保持阻塞，不冒充完整控件完成。
'''
(work/'docs/历史趋势图手册实现与验收.md').write_text(report,encoding='utf-8',newline='\n')
spec=dict(id='C24',ordinal=24,name='历史趋势图',slug='history-trend',task='S04-09',next='C25',usbAccessed=True,hardwareRun='docs/stage4-evidence/history-trend/physical/history-baseline-r1/run.json',expectedPassed=199,testRun='docs/stage4-evidence/history-trend/windows-final/run.json',nativeManifest='../docs/stage4-evidence/history-trend/native/native-manifest.json',report='docs/历史趋势图手册实现与验收.md',manual='design_plan/历史趋势图功能与交互复刻手册.md',scope=scope,unresolved=[f'Q-S04-C24-{i:03}' for i in range(1,5)],nativeSummary=native,protocol='历史曲线通过稳定采集/项目ID绑定；原厂默认、三项目及命名项目源/编译均已保留，来源SHA和关键DAT清单见专项报告。已执行原厂默认历史图实物下载，四项清除/更新关闭，输入/待传SHA一致。未依据跨进程多变量样本猜测Collect.dat、Curve.dat或固件布局；Lite保存schema12而独立SM/设备编译继续拒绝。当前无历史文件读取、PLC采样和逐字节USB应答结论。',codeFiles='src/data_collection_properties.*、data_collection_dialog.*、history_trend_properties.*、history_trend_dialog.*、project_model.*、main_window.*、main_window_workflow.cpp、stage3_project_edits.cpp、stage4_control_actions.cpp、static_text_actions.cpp、native_shell_ui.cpp、hmi_canvas.cpp、CMakeLists.txt',reviewFindings=['只读复核覆盖构造期信号、共享类型与整段字跨度、采集单组上限、跨页稳定项目引用、复制复用/冲突与删除保护、仅采集schema12和后台输出拒绝。','新测试的粘贴接口参数编译错误修复后重建，最终确认9项新增用例全部PASS；130+59+10=199/0/0。','固定PC曲线不代表历史采样或文件读取；实物下载仅原厂基线，显示/触控/PLC不据日志宣称通过。'])
write(root/'history-finalize.json',spec)
subprocess.run([sys.executable,'-B',str(root/'finalize_control.py'),str(root/'history-finalize.json')],check=True)
p=work/'程序修改记录.md';s=p.read_text(encoding='utf-8');i=s.rfind('：阶段4历史趋势图独立编辑与事务切片');assert i>=0;s=s[:i]+s[i:].replace('//20261112-0 add/change','//20261112-1 add/change');p.write_text(s,encoding='utf-8',newline='\n')
repro=base/'reproduction';repro.mkdir(exist_ok=True)
for name in ['complete_history_ui.py','integrate_history.py','add_history_tests.py','refine_history_review.py','archive_native_incremental.py','execute_native_cleanup.ps1','archive_physical_run.py','run_stage4_native_physical.py','deliver_history_docs.py','history-finalize.json','finalize_control.py','prepare_control_cleanup.py','execute_control_cleanup.ps1']:
 shutil.copy2(root/name,repro/name)
for name in ['history-build-r1.log','history-build-r2.log','history-build-r3.log']:
 if (root/name).exists():shutil.copy2(root/name,base/name)
print('C24 documentation prepared from 199 actual passing tests and completed reference download.')
