"""Prepare C29's evidence-driven delivery script without executing a premature finalization."""
from pathlib import Path
root=Path(__file__).resolve().parent
s=(root/'deliver_alarm_display_docs.py').read_text(encoding='utf-8');prefix=s[:s.index("scope='")].replace('alarm-display','history-alarm').replace('passed=156','passed=160').replace('Finalize C28','Finalize C29')
names=['historyAlarmSharedDialogKeepsDistinctIdentity','historyAlarmStrictTypeLanguagesAndBackendGates','historyAlarmWordVisibilityAndCopyDependencies','historyAlarmShellEntryPlacementEditAndUndo']
prefix='\n'.join('names='+repr(names) if line.startswith('names=') else line for line in prefix.splitlines())+'\n'
tail='''scope='历史报警独立historyalarm类型及菜单，复用已实测一致的报警表格属性类；五共用颜色/7字号/1..20行、三列名按语言保存、仅显现权限与完整字跨度、全页报警前置/导航/复制/语言冲突保护、schema12独立类型保存、创建/取消/编辑/撤销、空白示意表格及原厂历史报警基线实物下载'
native='独立历史报警入口门槛已实测，标题历史报警显示器；默认与当前报警显示14个基本交互字段逐项一致，差异字典为空，仅支持复用编辑器，不证明设备数据来源相同。默认与双语言自定义源均保存/重开/编译；自定义五共用色绿/红/蓝/灰/紫、字号24/行20、ZH/EN三列名各自保存、日期时间关闭后字段禁用且文字保留均已在r5重开验证。'
rows=[]
for name,label in [('history-alarm-default.sm','history-history-alarm-default-r2'),('history-alarm-custom.sm','history-alarm-custom-r5')]:
    source=base/'native'/name;folder=base/'native/compiled'/label;assert folder.is_dir(),folder
    rows.append(f'| {name} | {source.stat().st_size} | `{sha(source)}` | {(folder/"ControlPara.dat").stat().st_size} | {(folder/"ControlPixel.dat").stat().st_size} |')
report=f"""# 历史报警显示：手册实现与验收

2026-11-13，阶段4第29/41项，累计24类新增编辑/KSP切片。独立源/设备和现场仍有阻塞。

## 本轮实现与原厂依据

{scope}。

{native}

当前报警与历史报警共享列属性，但保留不同的对象类型、菜单、选择状态栏名称和窗口标题；属性确认、撤销和KSP重开不能把historyalarm改成alarmdisplay。表格样式共用，只有消息/日期/时间三个列名分语言；新增/删除工程语言同步各页列名，目标缺少源列名语言时原子拒绝粘贴，补齐后成功。报警登录输入/导航/复制依赖复用真实全局定义，最后非空定义不能在还有任一报警消费者时删除。

本轮只交付编辑身份和属性模型，没有实现历史记录引擎、时间查询、确认/恢复状态、存储清除或U盘/SD卡导出。原厂属性页也未出现这些编辑字段，未根据字符串或帮助补出虚构控件。PC仍显示空白示意行，不伪造历史时间或实际报警消息；与原厂绿色小图示的色差沿用C28记录，不宣称完整像素一致。

## 软件验收与复核

[实际私有桌面测试](stage4-evidence/history-alarm/{args.run}/run.json)：控件160、原厂源59、SDK宏10，共229/0/0；4项新增方法实际PASS，共用字段的C28测试继续在同一套回归中运行。新增覆盖历史窗口/类型保持、3类报警混合及单类schema12、旧schema/错类型拒绝、语言事务、不可变源/设备拒绝、32位显现完整跨度和独立复制、缺目标语言拒绝/补齐恢复、真实历史菜单门槛/取消/放置/编辑/撤销。

[历史属性页](stage4-evidence/history-alarm/{args.run}/history-alarm-basic.png)、[三类报警样例](../samples/stage4/history-alarm.ksp)来自实际测试；样例未声称29类混合。只读复核覆盖工厂原始type传递、共享对话框返回type、模型专有属性匹配与白名单、schema选择、全页前置和列名语言校验、复制角色、侧栏/画布/菜单以及取消/撤销。没有改变原厂源/设备后端允许范围。

## 原厂证据与实物

| 原源 | 字节数 | SHA-256 | ControlPara.dat | ControlPixel.dat |
|---|---:|---|---:|---:|
{chr(10).join(rows)}

[默认字段对比](stage4-evidence/history-alarm/native/default-ui-comparison.json)及[双语言重开字段](stage4-evidence/history-alarm/native/observed-fields.json)保留观测边界。[完整清单](stage4-evidence/history-alarm/native/native-manifest.json)保留五轮gzip/SHA。r3颜色框排队后尚未可见、r4快照遇已销毁子句柄而中止；均保留partial-or-error，没有改写成功。之后仅加强等待观测和跳过已销毁句柄，不重复动作；r5全流程完成并实际重开确认。default-r2编译标签含重复history前缀，按原始目录保留，可由源SHA/标题和清单追溯。

[原厂历史报警基线下载](stage4-evidence/history-alarm/physical/history-alarm-baseline-r1/run.json)完成，四清除/固件更新false，输入/待传SHA一致，实屏和触控仍未确认。该记录仅支持原厂基线传输；Lite独立生成仍由[实际CLI](stage4-evidence/history-alarm/backend-gates.json)拒绝。调查126文件与物理46文件已清，软件重复产物另有清理记录；所有运行均未切换桌面或注入系统键鼠。

## 未决

- Q-S04-C29-001：历史报警独立SM/设备记录、全局报警段与独立下载仍阻塞。
- Q-S04-C29-002：历史记录发生/恢复/确认、时间排序、掉电保存、记录容量/覆盖、分页和清除运行待验。
- Q-S04-C29-003：SD卡/U盘导出及设备端列宽、长消息/大字号、原厂示意行与真实历史表格的区别待验。
- Q-S04-C29-004：实屏、触控、PLC、多语言实际历史记录和权限组合运行尚未验证。

下一项C30历史数据显示器，须区分数据采集记录与历史报警。
"""
(work/'docs/历史报警显示手册实现与验收.md').write_text(report,encoding='utf-8',newline='\\n')
spec=dict(id='C29',ordinal=29,name='历史报警显示',slug='history-alarm',task='S04-10',next='C30',usbAccessed=True,hardwareRun='docs/stage4-evidence/history-alarm/physical/history-alarm-baseline-r1/run.json',expectedPassed=229,testRun='docs/stage4-evidence/history-alarm/windows-final/run.json',nativeManifest='../docs/stage4-evidence/history-alarm/native/native-manifest.json',report='docs/历史报警显示手册实现与验收.md',manual='design_plan/历史报警显示功能与交互复刻手册.md',scope=scope,unresolved=[f'Q-S04-C29-{i:03}' for i in range(1,5)],nativeSummary=native,protocol='历史报警默认字段14项与当前报警一致，但对象类型、原厂源和实物基线独立。两套源已保存/重开/编译，默认历史报警原厂基线实物下载完成，四清除/更新关闭，输入/待传SHA一致；未推断历史设备记录布局，独立生成继续实际拒绝，没有新增历史存储/查询/清除/导出或实屏/触控结论。',codeFiles='src/alarm_display_dialog.cpp、alarm_registration_properties.cpp、project_model.cpp、stage3_project_edits.cpp、stage4_control_actions.cpp、static_text_actions.cpp、main_window.cpp、native_shell_ui.cpp、hmi_canvas.cpp',codeMarker='//20261113-2',reviewFindings=['只读复核完整检查历史工厂类型到共享属性返回、JSON专有匹配/白名单/schema、全页前置/语言/复制和编辑/取消/撤销；共用UI没有合并当前和历史运行身份。','4项历史专属方法实际PASS，共用字段回归未省略，160+59+10=229/0/0。','原厂14字段对比/双语言保存重开/源编译与实际历史基线传输分别记录；r3/r4探针失败保留，历史记录引擎、E层和独立编码仍阻塞。'])
write(root/'history-alarm-finalize.json',spec);subprocess.run([sys.executable,'-B',str(root/'finalize_xy.py'),str(root/'history-alarm-finalize.json')],check=True)
repro=base/'reproduction';repro.mkdir(exist_ok=True)
for name in ['prepare_history_alarm_probe.py','prepare_history_alarm_retry.py','prepare_history_alarm_stable_snapshot.py','compare_history_alarm_fields.py','summarize_history_alarm_native.py','integrate_history_alarm.py','add_history_alarm_tests.py','archive_native_incremental.py','execute_native_cleanup.ps1','archive_physical_run.py','run_stage4_native_physical.py','prepare_history_alarm_delivery.py','deliver_history_alarm_docs.py','history-alarm-finalize.json','finalize_xy.py','prepare_control_cleanup.py','execute_control_cleanup.ps1']:
    shutil.copy2(root/name,repro/name)
for log in root.glob('history-alarm-build-*.log'):shutil.copy2(log,base/log.name)
print('C29 documentation prepared from 229 actual passes and completed independent native-reference transfer.')
'''
(root/'deliver_history_alarm_docs.py').write_text(prefix+tail,encoding='utf-8',newline='\n')
print('Prepared C29 finalization with real 229-test, physical-download and cleanup guards.')
