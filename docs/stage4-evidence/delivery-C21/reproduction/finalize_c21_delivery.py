"""Record the verified isolated release and the explicitly unaccepted main-checkout stage3 cases."""
from package_c21 import *
import re

main_runs = read(OUT / 'main-windows-r1/run.json')
backend = read(OUT / 'main-backend-r1/run.json')
portable = read(OUT / 'portable-r1/run.json')
refined = read(OUT / 'portable-r2/run.json')
restoration = read(OUT / 'diagnose-restoration.json')
assert main_runs['runs'][0]['totals'] == dict(passed=109, failed=0, skipped=0)
assert [r['totals'] for r in main_runs['runs'][1:]] == [dict(passed=47, failed=1, skipped=0), dict(passed=16, failed=1, skipped=0)]
assert backend['passed'] and [r['totals'] for r in backend['runs']] == [dict(passed=62, failed=0, skipped=0), dict(passed=56, failed=0, skipped=0)]
assert portable['passed'] and refined['passed'] and portable['onlyBundledQtDllsObserved']
assert restoration['restoredByteIdentical'] and not restoration['temporaryWorkaroundsRetained']
manifest = read(OUT / 'package-manifest.json')
for row in manifest['files']:
    assert sha(PACKAGE / row['path']) == row['sha256']
for row in restoration['files']:
    assert sha(MAIN / row['relative']) == row['sha256']
original = read(OUT / 'diagnose-focus-r1/run.json')
experiment = read(OUT / 'diagnose-focus-r3/run.json')
assert all(not row['passed'] for row in original['records']) and all(row['passed'] for row in experiment['records'])
build = OUT / 'build'
build.mkdir(exist_ok=True)
for path in ROOT.glob('main-*.log'):
    shutil.copy2(path, build / path.name)
for name in ('run_main_focus.py', 'instrument_main_diagnose.py', 'probe_main_workarounds.py', 'restore_main_diagnose.py'):
    shutil.copy2(ROOT / name, OUT / 'reproduction' / name)
summary = dict(package=str(PACKAGE), packageExeSha256=sha(PACKAGE / 'KinSealStudioLite.exe'), sourceCommit=manifest['sourceCommit'], sourceTree='isolated_verified_committed_checkout',
    isolatedTests=dict(controls=109, nativeSource=59, sdkMacro=10, failed=0, skipped=0), nativeReferenceHardwareCompleted=True,
    independentNewEncoderVerified=False, newDeviceDisplayOrTouchPasses=0, historyNativeReferenceBackfill=dict(completed=14, total=14),
    mainCompatibility=dict(buildPassed=True, controls=dict(passed=109, failed=0, skipped=0), nativeSource=dict(passed=62, failed=0, skipped=0), projectModel=dict(passed=56, failed=0, skipped=0),
                          canvas=dict(passed=47, failed=1, skipped=0), integration=dict(passed=16, failed=1, skipped=0), accepted=False),
    diagnosticExperimentsPassed=True, temporaryWorkaroundsRetained=False, originalSourcesRestoredByteIdentical=True, preExistingUncommittedStage3ChangesExcludedFromPackage=True,
    portableChecks=dict(initial=6, layoutValidationAndCapture=2, allPassed=True, onlyBundledQtDllsObserved=True), stage4Processed=21, stage4Total=41, completeStage4=False, next='C22')
write(OUT / 'delivery.json', summary)
issue = '''# 阶段3主目录兼容残留（2026-11-12）

状态：待后续阶段3续作修复，不混入阶段4C21控件切片。主目录18个原有已跟踪脏文件和无关未跟踪资料均保留；临时诊断涉及的3份源码已按原始字节SHA恢复，未暂存这些原有修改。

主目录实际构建成功；控件109/0/0、原厂源62/0/0、工程模型56/0/0。画布47/1/0、集成16/1/0，不能声明主目录整体回归通过。[原始运行](stage4-evidence/delivery-C21/main-windows-r1/run.json)、[后端](stage4-evidence/delivery-C21/main-backend-r1/run.json)。

| 问题ID | 实际故障、最小实验及接续 |
|---|---|
| Q-S03-LOCAL-001 | nestedGroupsRetainIndependentLocks：锁定嵌套组解组后，外层组已拆、成员/锁状态正确，Qt selectedItems仅1，预期2；group和独立对象isSelected均true。独立运行两次稳定失败，processEvents没有改变；临时对选中根先false再true后，原场景两次通过。定位为重新分组/父子变化后的Qt选择缓存与标志不一致；后续在重建选择的真实调用位置处理，不能仅放宽测试断言。 |
| Q-S03-LOCAL-002 | pixelFontsIgnorePaintDeviceDpi：23px部分通过，17.5px在96/192 DPI图像不同，两次独立运行稳定失败。paintNativeText已用明确device构造QTextLayout，却再次layout.setFont(font)，丢失构造时的设备绑定。临时只去掉冗余setFont后，原场景两次通过；后续保留小数字号及原厂基线/渐变的完整回归，不应把17.5强制改成整数。 |

[首次单独复现](stage4-evidence/delivery-C21/diagnose-focus-r1/run.json)、[诊断与事件循环反证](stage4-evidence/delivery-C21/diagnose-focus-r2/run.json)、[临时最小处理实验](stage4-evidence/delivery-C21/diagnose-focus-r3/run.json)。实验通过只证明上述定位，不是正式生产修复；[字节恢复记录](stage4-evidence/delivery-C21/diagnose-restoration.json)明确temporaryWorkaroundsRetained=false。临时诊断标签已从src/tests清除。

本轮发布包采用已提交、独立验证的C21源码构建，沿用主窗口外观版隔离未验阶段3续作的发布边界。用户原有续作没有回退、stash或混入Git提交。新包可加载其自带schema12编辑示例，不承诺加载尚未接入该已提交基线的本地扩展组合/锁数据。完整阶段3验收与上述正式修复仍需后续处理。
'''
issue_path = WORK / 'docs/阶段3主目录兼容残留_20261112.md'
assert not issue_path.exists()
issue_path.write_text(issue, encoding='utf-8')
pending = ROOT.parents[1] / '待定问题'
pending.mkdir(exist_ok=True)
(pending / 'KinSealStudio阶段3兼容残留_20261112.md').write_text(issue.replace('(stage4-evidence/', '(../KinSealStudio/docs/stage4-evidence/'), encoding='utf-8')
report = f'''# 阶段4 C21增量包交付

日期：2026-11-12。阶段4进度21/41；下一项C22图表，整阶段没有完成。历史14项原厂基线下载补验已闭环；本项移动轨迹也完成原厂基线下载。16类新增控件的新版独立源/设备编码仍阻塞，显示、触控与PLC行为没有新增通过。

便携程序：`dist/KinSealStudioLite-Stage4-C21-0.10.1/KinSealStudioLite.exe`。自带`samples/stage4/stage4-controls-C21.ksp`为六列四行21类编辑示例，另保留逐控件样例。包内使用说明标明编辑切片和下载边界。

程序SHA256：`{summary['packageExeSha256']}`，来自已提交C21源码`{summary['sourceCommit']}`的独立构建；未包含主目录原有18文件的未提交阶段3续作。原有续作完整保留，主目录另有两项已定位的[兼容残留](阶段3主目录兼容残留_20261112.md)，不能混作整体通过。

| 验证 | 真实结果 |
|---|---|
| 独立构建控件/源/SDK宏 | 109/0/0、59/0/0、10/0/0，合计178/0/0；见[C21汇总](stage4-evidence/move-track/windows-final/run.json)。 |
| 原厂基线实物 | C06～C19历史14项、C20及C21逐项完成；源/传输SHA、唯一7365:616C与四项关闭已记录；不替代Lite独立编码。 |
| 主目录兼容构建 | 成功；控件109/0/0、源62/0/0、工程模型56/0/0；画布47/1/0、集成16/1/0，两个失败保留。 |
| 便携包初次6项 | 自测、内嵌负载校验、打开示例、实际主窗口截图、未知源/设备编码按预期拒绝均通过；无USB写入。 |
| 示例布局后2项 | 重新验证和实际截图通过，21类均在当前视图宽度内可见。 |
| Qt依赖 | 清理安装路径的环境中运行，实际观察到的Qt DLL全部来自包内。 |

[完整交付元数据](stage4-evidence/delivery-C21/delivery.json)、[最终文件哈希清单](stage4-evidence/delivery-C21/package-manifest.json)、[初次便携验证](stage4-evidence/delivery-C21/portable-r1/run.json)、[最终布局验证](stage4-evidence/delivery-C21/portable-r2/run.json)、[最终主窗口图](stage4-evidence/delivery-C21/portable-r2/main-window.png)。已实际查看图片，未声明全窗口或全部控件像素一致。

清理：历史14轮644个文件；C21调查164、物理46、重复软件图/KSP119个文件已经逐项清理。额外兼容运行的重复产物及诊断原件在交付清理记录中统计。所有临时路径使用Z盘；没有再次删除曾被拒绝的空目录，没有触碰其他任务或安装目录。编译缓存保留供下一项增量构建。
'''
(WORK / 'docs/阶段4_C21增量包交付.md').write_text(report, encoding='utf-8')
readme_path = WORK / 'README.md'
text = readme_path.read_text(encoding='utf-8-sig')
assert '阶段4 C21增量包' not in text
text = text.replace('# KinSealStudio Lite\n', '# KinSealStudio Lite\n\n当前可运行的**阶段4 C21增量包**为 `dist/KinSealStudioLite-Stage4-C21-0.10.1/KinSealStudioLite.exe`，可打开包内 `samples/stage4/stage4-controls-C21.ksp` 查看21类编辑示例。阶段4进度21/41，历史14项原厂基线已补验；新增控件的独立源/设备编码仍阻塞，原厂下载成功不能替代新版独立下载或显示/触控通过。[交付记录](docs/阶段4_C21增量包交付.md)与[逐控件进度](docs/阶段4逐控件进度.md)说明实际范围。\n\n该包来自已提交源码的独立验证构建，主目录尚未提交的阶段3续作完整保留；两项[主目录兼容残留](docs/阶段3主目录兼容残留_20261112.md)单独记录。\n', 1)
text = text.replace('当前主窗口外观版为 **0.10.1**', '此前主窗口外观基线为 **0.10.1**', 1)
readme_path.write_text(text, encoding='utf-8', newline='\n')

def append(path, value):
    with path.open('a', encoding='utf-8', newline='\n') as stream:
        stream.write('\n\n' + value + '\n')

protocol = WORK / 'docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md'
number = max(map(int, re.findall(r'^## (\d+)\.', protocol.read_text(encoding='utf-8-sig'), re.M))) + 1
append(protocol, f'## {number}. C21增量包的下载门槛与主目录验证边界（2026-11-12）\n\n历史14项原厂基线与C20/C21原厂基线已实际完成；C21独立单类型源/设备编码仍拒绝。便携包自测、内嵌负载、打开21类示例、实际截图及两条拒绝门槛按预期通过，未新增USB写入、屏显/触控或PLC通过。程序SHA256 `{summary["packageExeSha256"]}`，来自已提交C21独立源码；未把原有未提交阶段3续作混入包或Git。主目录构建成功，控件109/0/0、源62/0/0、模型56/0/0；画布及集成各1失败，诊断试验定位但已恢复源码原字节，不冒充正式修复或整体通过。见[交付](阶段4_C21增量包交付.md)与[残留](阶段3主目录兼容残留_20261112.md)。')
append(WORK / 'docs/阶段4逐控件进度.md', f'## C21便携增量交付与兼容边界\n\n阶段4已处理21/41，下一项C22图表。历史14项原厂基线补验及C21原厂基线完成；新版独立源/设备编码仍阻塞。便携包来自已提交独立源码，178/0/0与8项便携验证通过。主目录两项原有阶段3兼容失败已定位并记录，诊断源码按原字节恢复，未混入本轮控件提交。协议§{number}与[交付记录](阶段4_C21增量包交付.md)已同步。')
append(WORK / '程序修改记录.md', '### C21增量包、主目录兼容检查与诊断恢复\n- 修改内容：\n  1. 从已提交的C21独立源码发布带21类编辑示例的便携增量，验证8项自测/打开/截图/拒绝及包内Qt依赖。\n  2. 主目录构建及控件/源/模型通过，记录画布/集成各1个原有阶段3失败；最小临时实验定位选择缓存和QTextLayout重复setFont，3份诊断源码按SHA原字节恢复，原有用户改动保留。\n  3. 更新README、交付/残留报告、协议、进度及哈希证据，清理重复产物和诊断原件；未改变生产业务逻辑。\n- 修改文件：README.md、docs/阶段4_C21增量包交付.md、阶段3主目录兼容残留_20261112.md、下载协议、进度、delivery-C21证据及本记录；工作区待定问题另存一份残留副本。\n- 影响范围：阶段4C21独立便携包及验收记录，不包含未验阶段3续作。\n- 原因：交付当前真实可运行版本，同时如实区分主目录未验变更及原厂基线下载与独立输出门槛。\n- 验证：独立109/0/0+59/0/0+10/0/0；主目录109/0/0+62/0/0+56/0/0、另2项失败保留；8项便携检查通过，源码恢复SHA一致。\n- 修改标记：//20261112-0 change')
progress_path = WORK / 'docs/stage4-evidence/progress.json'
progress = read(progress_path)
progress['latestDelivery'] = dict(evidence='docs/stage4-evidence/delivery-C21/delivery.json', package=str(PACKAGE), sourceTree='isolated_verified_committed_checkout', processed=21, total=41, completeStage4=False, mainUnacceptedStage3Failures=2)
write(progress_path, progress)
shutil.copy2(ROOT / 'finalize_c21_delivery.py', OUT / 'reproduction/finalize_c21_delivery.py')
print(json.dumps(dict(protocolSection=number, packageVerified=True, mainStage3FailuresRecorded=2, next='C22'), ensure_ascii=False))
