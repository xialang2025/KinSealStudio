"""Publish table evidence only after actual compiler and affected editor checks succeed."""
from pathlib import Path
import copy
import hashlib
import json
import re
import shutil

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence';native=base/'native/table'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def append(path,text):
    p=work/path;p.write_text(p.read_text(encoding='utf-8-sig').rstrip()+'\n\n'+text.strip()+'\n',encoding='utf-8',newline='\n')
compiler=read(base/'table-windows-r1/run.json');editor=read(base/'table-editor-windows-r1/run.json');comparison=read(native/'B-S-M-flat-candidate.json');cleanup=read(native/'cleanup.json')
assert compiler['passed'] and editor['passed'] and comparison['allRasterComparisonsEqual'] and len(comparison['cases'])==18
assert comparison['repeatedInputFreshProcessCount']==3 and len(read(native/'raw-archives.json')['archives'])==18
totals={key:sum(r['totals'][key] for result in (compiler,editor) for r in result['runs']) for key in ('passed','failed','skipped')};assert totals['failed']==totals['skipped']==0
report=work/'docs/阶段5_静态表格设备背景编译与差分验收.md'
report.write_text(f'''# 阶段5：纯静态表格设备背景编译与差分验收

第二批增加G10纯图像表格的受限独立C编译。它没有单元格数据、PLC地址、配方或历史数据语义；完整阶段5仍未完成。本轮不调用下载入口或USB通信，T/E分别交阶段6/10。

## 已开放范围

- 型号AMT070W124-L800X480R、800×480普通单页，保留既有1..100对象、1..8语言及配置守卫。
- 1..30行列，宽高分别整除列/行数，整数像素单元格；本切片宽高至少40，外框最右/最下端点不超过799/479。
- 背景不透明#e0e0e0、内部线及外框黑色、线宽1；线型0或1，水平/垂直/外框8种开关全部支持。
- 不与任何其它对象相交或边缘接触；只写SheetPixel，没有ControlPara/ControlPixel，也不新增扫描读取。
- 灰色背景以外颜色、Alpha、虚线、非整除尺寸、重叠、完整SM、动态数据表格和活动配置继续在输出前明确拒绝。

## 原厂分层对照

[B/S/M明细](stage5-evidence/native/table/B-S-M-flat-candidate.json)记录18次有效原厂编译，包含默认同源3次新进程、8种网格开关、无线但外框开启、1×1行列、偏移3×2及屏边30×30。所有源来自原厂UI创建或保留的原厂有效源，几何变体均另进程重开。共12组独立设备编译测试场景；重复过程与创建态没有重复计入独立场景数。

每幅完整RGB565为768000字节，18次共享设备栅格比较均0像素差异；正式C测试再次通过真实buildProjectPayload对12组保存重开原厂夹具全帧比较，未使用ROI或忽略掩码。PicLenPara均2400B，无控制/动态图段。

默认源SHA为`f2e1f7e14a72773c0a337df74b36bf23033ebccdea6644a7d210669d2cd9d729`。原件SHA为`305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c`；私有运行副本SHA为`38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89`。仍为此前登记的三个字节许可检查修改，仅私有副本运行，安装原件未改。

原厂action_USBdownload只用于其编译阶段，未调用传输按钮；产品只用纯离线编译/包验证API。私有桌面没有切换或激活，未发系统键鼠输入。

## 两个实际发现

关闭外框且内部网格开启时，原厂保存重开的内部线端帽为FlatCap。旧共享SquareCap在H/V/H+V三种组合分别多3/3/6个像素，[失败结果](stage5-evidence/native/table/B-S-M-comparison.json)保留。设备专用入口复用表格几何，仅选择FlatCap；原有编辑器预览保持默认SquareCap，其全部开关/Alpha草稿和取消行为另经真实Windows检查。不能把设备切片通过扩写为所有R变体通过。

尝试从100,100拖到101,101时，原厂实际保存的是40×40，而不是1×1像素表格；1×1指行列。测试按实际几何构建，没有将请求尺寸当作观察事实。

原厂同源三次的AlarmPara.dat、SysPara.dat、Usbload.bin、load.kns、upLoad.knr仍含非确定差异。未授予任何忽略范围，也未关闭Q-S05-004整项。

## 实际验证及只读复核

[编译关联回归](stage5-evidence/table-windows-r1/run.json)与[受影响编辑器检查](stage5-evidence/table-editor-windows-r1/run.json)合计{totals['passed']}/0/0；逐目标数量见真实日志，不借用先前253项或107项作为本轮新通过。包含旧五类+折线+表格的7对象索引闭合、表格10段包实际发布复读、SM/旧单文本门槛、未知字段/越界/非整除/接触拒绝及原输出保留。

只读复核检查行列校验在除法前完成、整数单元格、含右/下边界的像素范围、静态零记录/零动态图、旧预览默认参数、源快照完整性及既有能力门槛。未发现需要放宽未知格式的理由；专有SM与所有未恢复活动组合继续拒绝。修正了旧阶段4测试中“所有表格设备编译必然失败”的过期断言，仍要求SM导出失败。

## 清理与后续

18份完整原始probe和checkpoint已gzip归档并回解核SHA。[归档清单](stage5-evidence/native/table/raw-archives.json)与[清理回执](stage5-evidence/native/table/cleanup.json)记录删除{cleanup['deletedFiles']}个自有文件、{cleanup['deletedLogicalBytes']}逻辑字节（硬链接导致其不等于实际释放空间）。唯一输入/段、失败证据、脚本与当前构建保留；该构建仍用于后续切片。没有清理用户主目录build、既有发布、安装原件或SDK。

下一项继续矩形的受限C编译；多页/窗口/活动配置、新动态控件、原厂介质容器和非确定字节分类仍按S05-02/04/07等任务逐项恢复。
''',encoding='utf-8',newline='\n')
summary=f'G10纯静态表格已增加独立背景C切片：1..30行列、整数单元格、不透明灰底黑线、无线/实线及8开关；18次原厂对照和12组真实编译场景全帧相等，本批关联{totals["passed"]}/0/0。未知颜色/Alpha/虚线/非整除/重叠、SM和数据表格仍拒绝，未访问USB。'
append('docs/纯图像表格实现与验收.md','## 2026-11-13阶段5后续状态\n\n'+summary+'\n\n[阶段5专项报告](阶段5_静态表格设备背景编译与差分验收.md)是本切片C的当前入口。前文独立编码阻塞描述保留其阶段4历史身份；新产物T/E仍未执行。')
append('design_plan/06_阶段5_设备编译与格式差分.md','## 8. 第二批静态表格实际执行\n\n'+summary+'\n\n[表格验收](../docs/阶段5_静态表格设备背景编译与差分验收.md)保留3/3/6端点失败、FlatCap修正及原厂小拖框实际40×40的证据。S05-07与其它类型尚未闭合，整阶段未完成。')
append('design_plan/A05_源格式与设备编译契约.md','## 9. 阶段5静态表格编译补充（2026-11-13）\n\n'+summary+'\n\n复用SheetPixel背景和2400B PicLenPara，不伪造设备type36或新ControlPara；源顶层type36与设备记录类型不是同一映射。保存重开内部网格使用FlatCap，编辑预览仍保留其新建态默认。参见[完整证据](../docs/阶段5_静态表格设备背景编译与差分验收.md)。')
protocol='docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md';text=(work/protocol).read_text(encoding='utf-8-sig');section=max(map(int,re.findall(r'^## (\d+)\.',text,re.M)))+1
append(protocol,f'## {section}. 阶段5纯静态表格离线编译（2026-11-13）\n\n'+summary+'\n\n表格烘背景、不新增记录/扫描/动态图，复用10段与既有混合12/14段结构。新产物deviceAcknowledged=false；历史原厂基线下载和人工显示确认保持原输入身份，未冒充Lite新产物T/E通过。完整支持边界、端帽差异、非确定区和清理见[专项报告](阶段5_静态表格设备背景编译与差分验收.md)。')
matrix=work/'design_plan/控件文档与实施覆盖矩阵.md';lines=matrix.read_text(encoding='utf-8-sig').splitlines()
for index,line in enumerate(lines):
    if line.startswith('| 表格 |'):
        fields=line.split('|');fields[4]+=' [阶段5C](../docs/阶段5_静态表格设备背景编译与差分验收.md)';fields[9]=' 受限整数灰底静态背景C切片 ';fields[10]=' 原厂基线已验；新产物T待阶段6 ';fields[12]=' SM/其它变体与新产物T/E待验 ';lines[index]='|'.join(fields)
    if line.startswith('| 折线 |'):
        fields=line.split('|');fields[10]=' 原厂基线已验；新产物T待阶段6 ';lines[index]='|'.join(fields)
matrix.write_text('\n'.join(lines)+'\n',encoding='utf-8',newline='\n')
progress=read(base/'progress.json');progress['active']='G08矩形：保存重开已采样，待C实现';progress['selectedSlice']='G04正交折线 + G10受限整数静态表格';progress['softwareTests']=totals;progress['nativeCanonicalPixelCases']=18
progress['tableSlice']=dict(nativeCompilations=18,independentCanonicalFrameCases=12,softwareTests=totals,cleanupFiles=cleanup['deletedFiles'],cleanupLogicalBytes=cleanup['deletedLogicalBytes'],report='docs/阶段5_静态表格设备背景编译与差分验收.md',complete=False,usbAccessed=False)
progress['complete']=False;write(base/'progress.json',progress)
append('docs/阶段5逐项进度.md','## 第二批：静态表格\n\n'+summary+'\n\n本批已沿S05-02/03/05/08/09/10完成有界证据、实现和定向清理；S05-04没有新增控制记录。S05-07仍待恢复。下一项为G08矩形，不把18份原厂重复过程当18个独立控件。')
ledger_path=work/'design_plan/分阶段文档证据/阶段交付与清理记录.json';ledger=read(ledger_path);ledger['stage5Slice2']=dict(completeStage5=False,softwareTests=totals,nativeCompilations=18,canonicalRgb565Cases=12,usbAccessed=False,cleanupFiles=cleanup['deletedFiles'],cleanupLogicalBytes=cleanup['deletedLogicalBytes'],report='../docs/阶段5_静态表格设备背景编译与差分验收.md')
for task in ledger['taskStates']:
    if task['taskId'].startswith('S05-'):
        task.setdefault('executionHistory',[]).append(dict(executionId='stage5-table-20261113',scope='G10 bounded static raster only',wholeStageComplete=False,usbCommunicationExecuted=False))
write(ledger_path,ledger)
append('程序修改记录.md','''### 修改73：阶段5纯静态表格设备背景编译
- 修改内容：
  1. 开放受限整数网格、8开关及无线/实线的纯静态表格背景；不新增控制、图帧或PLC扫描记录。
  2. 保存重开的网格端帽用FlatCap，复用几何并保留既有编辑预览；拒绝未核样式、非整除、边界及接触组合。
  3. 原厂18次全帧差分、设备包/混合工程/编辑器回归、证据归档和自有中间文件清理。
- 修改文件：src/device_compile_plan.h/.cpp、graphic_properties.h/.cpp、independent_downloader.cpp、tests/stage5_compiler_tests.cpp、stage4_controls_tests.cpp、阶段5文档/协议/证据与本记录。
- 影响范围：AMT070W124-L800X480R的单页受限G10静态C切片；SM、数据表格、多页/配置和新产物T/E继续待验。
- 原因：按用户阶段5指令逐项恢复独立编译，不能把原厂历史下载或PC绘制当作Lite新产物通过。
- 修改标记：//20261113-12 add/change''')
repro=native/'reproduction';repro.mkdir(exist_ok=True)
names=['native_probe.py','run_table_canonical.py','run_table_boundaries.py','analyze_table_corpus.py','table_raster_probe.cpp','table_boundary_raster_probe.cpp','archive_family.py','execute_family_cleanup.ps1','run_table_editor_checks.py','document_table_slice.py','repeat-one.json','table-canonical-run.json','table-build-r1.log','table-build-r2.log','table-editor-build-r1.log','table-boundary-probe-build.log']
for name in names:
    p=root/name
    if p.exists():shutil.copy2(p,repro/name)
for p in root.glob('table-*.json'):
    if p.name not in names:shutil.copy2(p,repro/p.name)
review=dict(mode='read_only_review_after_implementation',newBusinessLogicChangedDuringReview=False,checks=['validated model before division','row and column ranges','right/bottom raster pixels and touching edges','static no records and no sprites','old preview default cap retained','independent native-source gate','atomic package and legacy text gate','existing five-family mixed indexes'],remaining=['unsupported color/alpha/style and noninteger cells','overlap composition','SM body','all stage5 tasks beyond this slice'],files=[dict(path=p,sha256=sha(work/p)) for p in ['src/device_compile_plan.h','src/device_compile_plan.cpp','src/graphic_properties.h','src/graphic_properties.cpp','src/independent_downloader.cpp','tests/stage5_compiler_tests.cpp','tests/stage4_controls_tests.cpp']],softwareTests=totals,usbAccessed=False)
write(native/'review.json',review)
print(json.dumps(dict(softwareTests=totals,protocolSection=section,report=str(report),completeStage5=False),ensure_ascii=False))
