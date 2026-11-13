"""Record only measured first-slice results and retain outstanding Stage5 dependencies."""
from pathlib import Path
import copy
import datetime
import json
import re
import shutil

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
base=work/'docs/stage5-evidence'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
def append(relative,text):
    with (work/relative).open('a',encoding='utf-8',newline='\n') as f:f.write('\n\n'+text.strip()+'\n')

report=work/'docs/阶段5_正交折线编译与离线包验收.md'
assert not report.exists(), 'Do not duplicate first-slice records.'
run=read(base/'windows-r5/run.json');assert run['passed']
totals={key:sum(item['totals'][key] for item in run['runs']) for key in ['passed','failed','skipped']};assert totals==dict(passed=107,failed=0,skipped=0)
cleanup=read(base/'cleanup.json');assert cleanup['deletedFiles']==1124
corpus=read(base/'native/polyline/B-S-M-comparison.json')
assert corpus['repeatedInputFreshProcessCount']==3 and corpus['ignoredByteRanges']==[]
shutil.copy2(root/'execute_cleanup.ps1',base/'reproduction/execute_cleanup.ps1')
shutil.copy2(root/'document_slice1.py',base/'reproduction/document_slice1.py')
progress=read(base/'progress.json')
states=[
    ('S05-01','passed_current_baseline','五类253项记录检查、10个正向CLI、TEST ZIP14项和10内置段身份已冻结。'),
    ('S05-02','passed_selected_corpus_with_open_nondeterminism','三次同源新进程、空白基线、6组保存重开栅格；原厂非确定区保持未知，未设忽略掩码。'),
    ('S05-03','passed_bounded_single_page_projection','共享不可变单页计划、稳定顺序及页/对象/属性诊断；没有完整多页IR。'),
    ('S05-04','existing_five_revalidated_static_slice_has_no_new_record','复用五类编码器；折线烘SheetPixel，无新增ControlPara类型。其他26类未恢复。'),
    ('S05-05','passed_six_canonical_background_cases','默认、改宽、改位置尺寸、1像素、屏边及双对象整帧RGB565逐字节相等。'),
    ('S05-06','passed_existing_modbus_and_sdk_slice','默认Modbus扫描37项、真实SDK宏10项及实际14段包通过；未新增驱动/变量/全局宏。'),
    ('S05-07','blocked_outside_selected_slice','多页/窗口/键盘/Logo与采集/报警/配方等活动配置尚未恢复，拒绝编译。'),
    ('S05-08','passed_new_directory_transaction','新目录暂存/复读/rename发布；10/12/14段及源身份/计划核对，旧目录拒绝覆盖。'),
    ('S05-09','passed_selected_regression_and_review','107项关联回归；253项基线分开；初始失败及修复链保留，未访问USB通信。'),
    ('S05-10','passed_owned_cleanup','原始gzip与唯一源/段已保存；清理1124文件，主目录用户build及原件/SDK未动。')]
progress.update(active='首批程序提交与便携包交付',complete=False,selectedSlice='G04 opaque black solid width-1 integer orthogonal polyline; no overlaps',
    tasks=[dict(id=i,state=s,scope=t) for i,s,t in states],softwareTests=totals,
    baselineRecordTests=dict(passed=253,failed=0,skipped=0),baselineCliCases=10,
    nativeCanonicalPixelCases=6,hardwareTransferExecuted=False,completeIndependentCompatibility=False,
    unresolved=['Q-S05-001','Q-S05-002','Q-S05-003','Q-S05-004','Q-S04-G04-001','Q-S04-G04-002','Q-S04-G04-003'])
write(base/'progress.json',progress)
report.write_text('''# 阶段5首批：正交折线与离线包

本批在既有五类设备编译器上增加有边界的G04背景编译，并把离线输出改为核验后发布。阶段5整体未完成；其它新控件、复杂图形样式、多页及活动配置仍拒绝。没有进行本批实物下载，T/E由阶段6/10接续，历史ACK没有转用于本批。

## 实际支持范围

- 目标AMT070W124-L800X480R、800×480、单普通启动页；固件版本未确认。1..100对象、1..8语言及既有默认Modbus边界保留。
- 新增折线：实线、宽1、不透明黑色、无箭头、端点0；顶点落整数像素、每段水平或垂直，且不与其它对象包围框重叠。未证样式/小数点/斜线/重叠明确拒绝。
- 折线独立渲染到背景，不产生动态控制记录或图帧。其它26个新增动态家族仍未开放设备编码；其它基础图形也未因共享UI开放。
- `.ksp`可保存；G04完整`.sm`导出仍拒绝。能力查询将SM和设备资格分别列出，candidate仍须实际编译。

## 原厂字节、结构与语义

三次全新进程对同一原厂源SHA `2fac27d72ba777de8d357536e334160dbb1478eb8be5822b1d6108d73fe9ec68`重新编译，解压背景一致。原厂运行副本身份、输入和逐文件SHA见[B/S/M对照](stage5-evidence/native/polyline/B-S-M-comparison.json)。空白原厂基线另存，失败手势产生的空path没有当作图形正例。

默认、仅改宽度、改位置尺寸、1像素、屏幕边界、双折线6组保存重开工程，与产品独立生成的整幅768000字节RGB565逐字节相等。LZO压缩字节允许不同，没有把压缩SHA不同直接判像素错误。

**端点差异**：原厂新建立即编译使用的缓存笔，与保存后重新打开不同。默认源重开后FlatCap与产品完全相符；SquareCap/RoundCap均多出(250,200)一个像素。见[三个实际候选](stage5-evidence/native/polyline/cap-comparison.json)。本次仅修正设备canonical栅格；阶段4编辑预览对应新建时的笔，完整R没有整体升级。旧创建时栅格测试曾通过，改用新重开夹具后实际失败，再修正，记录保留。

同输入的SysPara、AlarmPara、upLoad.knr、load.kns和Usbload.bin存在非确定字节。本批保留准确区间，**未设置任何忽略区**；其语义仍未知，Q-S05-004未关闭。背景切片通过不等于全部系统/报警语义已恢复。

## 纯预检与包契约

`prepareDeviceCompilePlan`保留现有单页/配置/源默认值守卫，从同一工程快照稳定排序并定位错误至页ID、对象ID、属性路径和值。`buildProjectPayload`、设备能力查询共用该检查，旧单文本入口不会把折线当文本。

新包必须写入尚不存在的输出目录。全部段先写到同父目录暂存位置，复读长度/SHA、段集合、完整源快照、控制记录、图块LZO/32B补齐、页数组、rawOffset/rawBytes和传输计划后才rename发布。既有目录（含空目录）拒绝覆盖；未实现覆盖旧目录事务或跨卷切换。暂存失败自动回收，不会留下同名有效包。

清单格式为`kinseal-device-package-v1`，包含complete、目标、输入工程/程序SHA、源快照身份及纯传输计划。`deviceAcknowledged=false`、`usbAccessed=false`。源快照分别核Lite模型或已支持SM完整typed结构，解压保护256MiB是Lite边界。它仍不能作为原厂上传互通容器。

新增CLI：`--verify-compiled-package <目录>`，只验证本格式包，失败返回23。`--compile-project`仍为纯离线；本阶段未调用任何`--download-*`入口。

## 实际验证与复核

既有五类记录[253项](stage5-evidence/baseline-windows-r2/run.json)、各默认/边界[10个真实CLI编译](stage5-evidence/baseline-cli-r1/run.json)分别记录。TEST ZIP14项、10内置段和三个ARM SDK程序身份见[冻结记录](stage5-evidence/baseline-identity.json)。

最终[107/0/0](stage5-evidence/windows-r5/run.json)：新增编译器40、既有后端14、扫描37、纯计划6、真实SDK宏10。包括6组原厂整帧、五类+折线混合索引、真实SDK14段包、短写/假成功损坏/清单失败/目标抢占、旧目录/旧段/坏SHA/伪造计划/错误源快照及未知控制类型拒绝。它不是其它26类控件的测试通过数。

只读复核后纠正了端帽、type首字节与下一字节macro flag的区分、PicLen有界读取和测试夹具中文窄字符串路径。首次失败、红色夹具和中间回归均保留，第三方lzokay初次编译已有告警单独记录；最终受影响构建没有新告警。

## 清理与后续

17个完整原始probe已gzip并回解核SHA；[原始索引](stage5-evidence/native/polyline/raw-archives.json)。清理1124文件、1419271271逻辑字节，包括自有原厂运行副本/TEMP和构建对象。逻辑尺寸含硬链接，不当成物理释放量。原始源/段、失败输入、可执行文件和用户main/build保留，见[清理回执](stage5-evidence/cleanup.json)。

G04完整SM、全部样式/斜线/重叠、独立新产物T/E继续按原Q-ID推进。S05-07多页/配置、其它动态类型和Q-S05-001..004保持未闭合；下一批从已有原厂证据明确的纯静态表格切片继续。
''',encoding='utf-8',newline='\n')
progress_doc=work/'docs/阶段5逐项进度.md'
progress_doc.write_text('# 阶段5逐项进度\n\n本阶段共10项。本批为G04受限静态背景和安全离线包闭环；阶段5整体仍未完成，下一批继续静态表格/其它已证映射。\n\n| 任务 | 本批结果 |\n|---|---|\n'+'\n'.join('| '+i+' | '+t+' |' for i,s,t in states)+'\n\n[首批报告](阶段5_正交折线编译与离线包验收.md) · [机器进度](stage5-evidence/progress.json)\n',encoding='utf-8',newline='\n')
summary='阶段5首批已增加受限正交折线的独立SheetPixel背景编译、共享单页语义预检及新目录事务包发布/验证。6组保存重开原厂整帧RGB565逐字节相等；本批关联107/0/0，五类253项基线及10个CLI正例另计。SM导出、多页/活动配置、其它新类型及完整T/E未通过。'
append('design_plan/06_阶段5_设备编译与格式差分.md','## 7. 2026-11-13首批实际执行\n\n'+summary+'\n\n[逐项进度](../docs/阶段5逐项进度.md)与[首批验收](../docs/阶段5_正交折线编译与离线包验收.md)记录已执行范围、原厂新建/重开端点差异、非确定字节和拒绝边界。S05-07及全类型映射继续保留，不将本批处理等同完整阶段5。')
append('design_plan/A05_源格式与设备编译契约.md','## 8. 2026-11-13阶段5首批设备编译补充\n\n'+summary+'\n\n当前exportProjectPayload已改为同父目录暂存、复读校验后发布到新目录，前文“逐文件可能留部分结果”保留为历史。清单格式kinseal-device-package-v1；新--verify-compiled-package核段、源身份和计划。现有目录拒绝覆盖，无跨卷或覆盖事务声明。折线仅实线/宽1/不透明黑色/整数正交/无箭头端点0/不重叠，使用保存重开的FlatCap；其完整SM仍拒绝。Q-S05-004只取得同源3新进程差分，Sys/Alarm等未知区未获忽略资格。具体边界见[首批报告](../docs/阶段5_正交折线编译与离线包验收.md)。')
protocol='docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md'
text=(work/protocol).read_text(encoding='utf-8-sig');number=max(map(int,re.findall(r'^## (\d+)\.',text,re.M)))+1
append(protocol,f'## {number}. 阶段5首批纯离线编译与交接边界（2026-11-13）\n\n'+summary+'\n\n本批只核12/14/16步纯计划及10/12/14数据段，不调用USB通信；历史原厂基线T、屏端人工确认均未转作本批独立新产物通过。待阶段6验证新产物传输与完整提交/重启/read，E另按阶段10执行。包中deviceAcknowledged固定false，源快照仍区分Lite JSON和原厂SM，不能当作原厂上传互通通过。')
matrix=work/'design_plan/控件文档与实施覆盖矩阵.md';lines=matrix.read_text(encoding='utf-8-sig').splitlines()
for index,line in enumerate(lines):
    if line.startswith('| 折线 |'):
        fields=line.split('|');fields[4]+='[阶段5首批](../docs/阶段5_正交折线编译与离线包验收.md) ';fields[9]=' 正交默认黑实线背景C切片；其余阻塞 ';fields[12]=' 完整源/变体及新产物T/E待验 ';lines[index]='|'.join(fields)
matrix.write_text('\n'.join(lines)+'\n',encoding='utf-8',newline='\n')
ledger_path=work/'design_plan/分阶段文档证据/阶段交付与清理记录.json';ledger=read(ledger_path)
for task in ledger['taskStates']:
    if task['taskId'].startswith('S05-'):
        task.setdefault('executionHistory',[]).append(dict(executionId='stage5-slice1-20261113',previousState=copy.deepcopy({k:task.get(k) for k in ['documentation','investigation','implementation','resumePoint']})))
        state=next(x for x in states if x[0]==task['taskId'])
        task.update(documentation='updated_selected_slice',implementation=dict(state=state[1],scope=state[2]),resumePoint=dict(document='06_阶段5_设备编译与格式差分.md',nextAction='首批编译器提交和便携包交付后，继续有原厂证据的下一静态C切片；全类型/多页/配置仍未恢复。'))
ledger['stage5Slice1']=dict(completeStage5=False,softwareTests=totals,canonicalRgb565Cases=6,usbAccessed=False,report='../docs/阶段5_正交折线编译与离线包验收.md')
write(ledger_path,ledger)
log=work/'程序修改记录.md';n=max(map(int,re.findall(r'^### 修改(\d+)：',log.read_text(encoding='utf-8-sig'),re.M)))+1
append('程序修改记录.md',f'### 修改{n}：阶段5正交折线独立背景及离线包事务\n- 修改内容：\n  1. {summary}\n  2. 原厂同源三次新进程与六组保存重开栅格对照；纠正canonical端帽，不忽略Sys/Alarm非确定区。\n  3. 包结构/源身份/传输计划校验、失败发布保护及纯CLI验证；归档与定向清理自有中间物。\n- 修改文件：CMakeLists.txt、src/device_compile_plan.h/.cpp、src/device_package.h/.cpp、independent_downloader.cpp、project_capabilities.cpp、main.cpp、tests/stage5_compiler_tests.cpp、阶段5文档/证据/台账和本记录。\n- 影响范围：已有单页设备后端的共享预检与离线包，增加受限G04背景；不开放未知SM、多页、活动配置或其它新设备类型。\n- 原因：用户要求按阶段5设备编译与格式差分文档继续实施。\n- 修改标记：//20261113-11 add/change')
write(base/'review-final.json',dict(mode='read_only_review_after_repairs',businessLogicChangedDuringReview=False,tests=totals,
    actualFinalRun='docs/stage5-evidence/windows-r5/run.json',canonicalNativeFramesEqual=6,
    findings=['type仅首字节，下一字节macro flag保持；真实14段宏包和帧校验通过。','整帧对照针对保存重开后的canonical笔；旧创建时与新结果一像素差异已分别保存。','GUI下载worker直接buildProjectPayload，不受离线输出目录拒绝覆盖策略影响；共用预检仍在任何USB前。','旧主目录用户修改未参与验证；阶段5全类型、多页/配置和原厂非确定掩码未宣称完成。'],usbCommunicationExecuted=False))
print(json.dumps(dict(softwareTests=totals,protocolSection=number,report=str(report.relative_to(work)),completeStage5=False),ensure_ascii=False))
