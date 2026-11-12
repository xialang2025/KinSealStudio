"""Prepare C31's delivery recipe; it cannot run successfully before actual tests finish."""
from pathlib import Path
import re
root=Path(__file__).resolve().parent
s=(root/'deliver_history_data_docs.py').read_text(encoding='utf-8').split("scope=",1)[0]
s=s.replace('C30','C31').replace('history-data','recipe').replace('passed=168','passed=178').replace('native-cleanup-recipe-no-columns-r9','native-cleanup-recipe-reopen-r16').replace(',238)',',498)').replace("native/recipe-default.sm","native/recipe-default-wide.sm")
names=['recipeDefinitionNativeBoundsAndRetainedMatrix','recipeDefinitionDialogTableCopyAndCommonNames','recipeCsvExactNativeFixtureAndAtomicRefusals','recipeDefinitionsSingleLimitAndAllPageReferences','recipeDisplayNativeDefaultsAndKeyboardDrafts','recipeDisplayNoticePermissionSpansAndCopies','recipeStrictPersistenceAndBackendGates','recipeClipboardDefinitionAndKeyboardConflicts','recipeCanvasResolvedContextAndRasterClipping','recipeShellDefinitionCreationPlacementEditAndUndo']
s=re.sub(r'^names=\[.*\]$', 'names='+repr(names),s,flags=re.M)
s+='''scope='真实单份配方定义及最多200份数据列；16位长度1..200、32位1..99、五类型、公共名称/元素名称、可见尺寸与保留矩阵分离、零起点只复制数值、完整读写跨度及RW/RWI只读范围；原厂GBK CSV基线字节对等/严格原子导入导出；独立recipe显示类型、五色/四字号/三对齐、网格、编辑/自定义键盘/位置、显现和触控权限、互斥通知、稳定引用/全页删除/复制保护、临时画布表头、schema12和创建/取消/编辑/撤销及原厂基线实物下载'
native='已归档16轮原厂调查。未建配方提示配方不存在，请先创建配方；已有一份再添加提示已经达到最大配方数量，请删除后再新建。实际类型顺序为16位正整数/32位正整数/16位整数/32位整数/32位浮点数，32位长度上限实测99。名称和元素名称切换语言仍共用；复制编号从0开始，只复制数值，不复制表头。类型切换及长度/总数缩减后再放大保留已有数值；32位当前区范围RWI2起，重新展开的自动元素名按完整字跨度生成。编辑配方可稳定开启，自定义键盘空时原厂拒绝，X/Y均0..800；网格关闭隐藏颜色项，通知位/字互斥。默认和字号48/右对齐/编辑开启/网格关闭样例已保存编译并重开。'
rows=[]
for compilation in manifest['nativeCompilations']:
    folder=base/'native/compiled'/compilation['label'];rows.append(f'| {folder.name} | {(folder/"ControlPara.dat").stat().st_size} | {(folder/"ControlPixel.dat").stat().st_size} |')
report=f"""# 配方显示器：手册实现与验收

2026-11-13，阶段4第31/41项，累计26类新增编辑/KSP切片。后续为G01～G10；完整SM/设备与现场运行尚未恢复。

## 本轮实现与原厂依据

{scope}。

{native}

配方定义是单份全局配置，表格列代表这份定义内的各份配方，行代表数据项。显示器只保存稳定recipeId，配置改名、复制与整工程撤销不改变引用。当前/非活动页任何显示器仍引用时拒绝删除。所选自定义键盘必须是真实keyboard页；仅在编辑和自定义键盘都启用时激活引用，原厂Y上限800按实测保留，没有照抄数值显示器480。

定义与显示器采用独立模型。长度/总数是当前可见尺寸；KSP同时保留最大已编辑矩阵及表头，缩小后再放大恢复数值。切换类型保留值，超出新类型范围时在确定处拒绝，不做静默截断。新展开的自动RWI名称按字跨度重建；自定义名称保留。源配方编号与目标编号均0起，只复制当前可见行的数值，不复制任何表头；隐藏缓存的原厂复制范围没有升级为已验证。

配置窗口的语言选择保留，但按r7/r8观测名称共用，没有擅自添加分语言表头映射。显示器只有实测出现的编辑/键盘字段；静态字符串中的行列数、使用标题、固定列宽及边框色仍隐藏，未据此新增控件。通知位/字互斥，父通知关闭后提交清除两个选择标志，但保留地址/值草稿；键盘位置等关闭分支保留值，符合r13重开观测。

## CSV边界

[原厂默认CSV](stage4-evidence/recipe/native/C31Golden-native-r14.csv)由实际ExportButton和原生保存框默认文件名产生；本轮编码器对该CP936/GBK、CRLF、十行中文元信息、末尾逗号、默认名称和值逐字节相等，且实际读入同一文件。只有当前可见矩阵和配方列名进入CSV；元素名称与隐藏缓存不在原厂格式内，完整保存仍用KSP。

r10原厂成功生成了[C31Types CSV](stage4-evidence/recipe/native/C31Types-native-r10.csv)，其中名称含逗号时没有转义，产生歧义。本实现明确拒绝含逗号/换行的配方列名和不能无损表示的GBK字符，不猜引号扩展。原厂CSV数据类型标为32位时使用完整32位数值边界，浮点值采用有限可表示float32保护；与原厂正则验证器的全部词法差异仍待验。

导入先完整解析候选，编码/元信息/尺寸/数据范围错误不修改当前草稿；只有父窗口确定才提交工程。导出先完整编码再原子写文件。Windows外没有假冒GBK转换；不支持编码时明确拒绝。当前只验证默认原厂字节及PC往返，非默认五类型的原厂反向导入、文件路径/覆盖细节与设备U盘交换仍未全部验证。

## PC渲染与真实测试

画布通过临时角色解析配置名称和元素名，JSON只保留引用，避免配置改名后残留旧表头。所有重建、放置、选中编辑和本地历史的item创建都走同一绑定路径。标题为序号、配置名称、元素名称；数据区使用空白示意行，不伪造PLC读值。PC最多示意20份配方，并按本地最小列宽给出原厂相同的宽度不足文案。

该最小列宽公式没有宣称为原厂阈值；原厂230宽不足、620宽可显示已保留截图，但原厂行高/列宽、滚动分页和逐像素外观仍是未决项。原厂基本页空白区实际是编辑/键盘组，本轮没有擅自加表格预览。

[实际私有桌面测试](stage4-evidence/recipe/{args.run}/run.json)：控件178、原厂源59、SDK宏10，合计247/0/0。10项新增方法覆盖五类型/地址跨度/保留矩阵、真实配置表格和复制、原厂CSV字节对等与不可变错误路径、单定义上限/全页依赖/语言、真实编辑和键盘分支、互斥通知与完整地址递增、schema12/严格JSON与源设备拒绝、复制定义/键盘冲突、临时画布引用和剪裁、真实配置菜单/显示创建/改名/受保护删除及撤销。

[配置数据页](stage4-evidence/recipe/{args.run}/recipe-definition-data.png)、[显示器属性页](stage4-evidence/recipe/{args.run}/recipe-display-basic.png)、[6类型样例](../samples/stage4/recipe.ksp)来自实际测试，未冒称31类混合。只读复核检查了配置/显示类型匹配、schema、CSV边界和原子替换、保留矩阵、双权限/通知、全页键盘引用、临时画布上下文及菜单/树事务；发现数值窄化、跨度测试差一、声明位置和错误文案后已修复，再进行实际构建测试。

## 原厂编译、下载与清理

| 原厂编译标签 | ControlPara.dat | ControlPixel.dat |
|---|---:|---:|
{chr(10).join(rows)}

[完整原厂清单](stage4-evidence/recipe/native/native-manifest.json)保留16轮gzip/SHA、默认/自定义源、关键DAT、原CSV及复现脚本。r5/r6是树节点位于可视区外而未打开属性，r16仅增加后台scrollToItem后重开成功；r9单击与r11未滚动的上下文探针没有冒充配置编辑通过。r10实际导出了CSV，但探针预期的改名路径没有产生，因此仍记partial；r14按原生默认文件名导出成功。r15试图添加第二定义时原厂按上限阻止，探针随后找不到名称字段，保持partial而未伪造第二份定义。

[原厂默认基线实物下载](stage4-evidence/recipe/physical/recipe-baseline-r1/run.json)完成，源SHA `{hardware['sourceSha256']}`，输入/待传一致，四清除/更新选项false。只支持原厂基线T层，不证明Lite独立输出或触控、PLC运行。[实际CLI门槛](stage4-evidence/recipe/backend-gates.json)继续拒绝未知SM/设备记录。

原厂调查498文件与物理46文件已按路径/SHA清理；软件重复截图和样例另有清理记录。宿主与构建临时文件均放独占Z盘目录，没有切换桌面或注入系统键鼠。

## 未决

- Q-S04-C31-001：原厂配方全局段、显示器SM/设备记录、独立生成和下载仍阻塞。
- Q-S04-C31-002：设备实际显示/编辑、PLC读写、当前配方编号、存储与通知/权限执行未通过。
- Q-S04-C31-003：原厂精确宽度阈值、行高/列宽、分页滚动和隐藏的候选字段仍待验。
- Q-S04-C31-004：非默认五类型CSV原厂反向导入、扩展字符、原厂导出改名路径及设备U盘交换未全部验证。
- Q-S04-C31-005：真实自定义键盘选择后的设备输入与位/等级/字条件组合、最终SM对隐藏缓存的保存范围仍待验。

下一项G01直线，先复用已有图形原厂调查，再逐项实现/下载/更新协议/清理/提交。
"""
(work/'docs/配方显示器手册实现与验收.md').write_text(report,encoding='utf-8',newline='\\n')
spec=dict(id='C31',ordinal=31,name='配方显示器',slug='recipe',task='S04-10',next='G01',usbAccessed=True,hardwareRun='docs/stage4-evidence/recipe/physical/recipe-baseline-r1/run.json',expectedPassed=247,testRun='docs/stage4-evidence/recipe/windows-final/run.json',nativeManifest='../docs/stage4-evidence/recipe/native/native-manifest.json',report='docs/配方显示器手册实现与验收.md',manual='design_plan/配方显示器功能与交互复刻手册.md',scope=scope,unresolved=[f'Q-S04-C31-{i:03}' for i in range(1,6)],nativeSummary=native,protocol='原厂只允许一份配方定义，200列数据；32位长度上限实测99，配置名称/元素名共用，缩减再放大保留值，通知位/字互斥，编辑/自定义键盘空校验和双坐标0..800已恢复。原厂默认GBK CSV实际字节对等；默认配方显示器原厂源已实物下载，四清除/更新关闭，输入/待传SHA一致。独立SM/设备编码、显示/触控/PLC执行仍未验证，门槛继续实际拒绝。',codeFiles='src/recipe_properties.*、recipe_csv.*、recipe_dialog.*、recipe_display_properties.*、recipe_display_dialog.*、project_model.*、stage3_project_edits.cpp、stage4_control_actions.cpp、main_window*、native_shell_ui.cpp、hmi_canvas.*、static_text_actions.cpp、CMakeLists.txt',codeMarker='//20261113-2',reviewFindings=['完成只读复核后修复有限float32检查、地址边界测试、声明顺序、CSV错误提示和schema12活动配置门槛；源/设备白名单未扩大。','10项新增方法实际PASS，178+59+10=247/0/0；原厂GBK默认CSV逐字节一致、保留矩阵及真实菜单/树生命周期/全页引用/复制/撤销通过。','16轮原厂证据及其partial状态保留，原厂基线传输完成；PC表格明确为示意，未宣称设备运行、完整像素或全部CSV反向互通。'])
write(root/'recipe-finalize.json',spec);subprocess.run([sys.executable,'-B',str(root/'finalize_xy.py'),str(root/'recipe-finalize.json')],check=True)
repro=base/'reproduction';repro.mkdir(exist_ok=True)
for name in ['prepare_recipe_preflight.py','prepare_recipe_config_probe.py','prepare_recipe_baseline_probe.py','prepare_recipe_edit_probe.py','prepare_recipe_survey.py','retry_recipe_tree_activation.py','prepare_recipe_fresh_survey.py','prepare_recipe_language_copy_probe.py','prepare_recipe_types_csv.py','prepare_recipe_lifetimes.py','prepare_recipe_golden_export.py','prepare_recipe_reference_probes.py','inspect_recipe_preflight.py','summarize_recipe_survey.py','summarize_recipe_language_copy.py','summarize_recipe_types.py','inspect_recipe_lifetimes.py','summarize_recipe_display.py','prepare_recipe_archive.py','make_recipe_models.py','make_recipe_display_model.py','make_recipe_csv.py','make_recipe_definition_dialog.py','make_recipe_display_dialog.py','integrate_recipe.py','integrate_recipe_tree_canvas.py','add_recipe_tests.py','review_recipe_before_build.py','deliver_recipe_docs.py','prepare_recipe_delivery_script.py','recipe-finalize.json','archive_native_incremental.py','execute_native_cleanup.ps1','archive_physical_run.py','run_stage4_native_physical.py','finalize_xy.py','prepare_control_cleanup.py','execute_control_cleanup.ps1']:
    shutil.copy2(root/name,repro/name)
for path in root.glob('recipe-build-*.log'):shutil.copy2(path,base/path.name)
print('Prepared C31 docs from 247 actual passes and completed native-reference download.')
'''
(root/'deliver_recipe_docs.py').write_text(s,encoding='utf-8',newline='\n')
compile(s,str(root/'deliver_recipe_docs.py'),'exec')
print('Prepared guarded C31 delivery documentation recipe.')
