"""Freeze the bounded rectangle result; preserve failed small-resize evidence and broader Stage5 limits."""
from pathlib import Path
import hashlib
import json
import re
import shutil

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence';native=base/'native/rect'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def append(path,text):
    p=work/path;p.write_text(p.read_text(encoding='utf-8-sig').rstrip()+'\n\n'+text.strip()+'\n',encoding='utf-8',newline='\n')
compiler=read(base/'rect-windows-r1/run.json');editor=read(base/'rect-editor-windows-r1/run.json');comparison=read(native/'B-S-M-comparison.json');cleanup=read(native/'cleanup.json')
assert compiler['passed'] and editor['passed'] and comparison['allRasterComparisonsEqual'] and len(comparison['cases'])==17
totals={key:sum(r['totals'][key] for result in (compiler,editor) for r in result['runs']) for key in ('passed','failed','skipped')};assert totals==dict(passed=161,failed=0,skipped=0)
report=work/'docs/阶段5_矩形设备背景编译与差分验收.md'
report.write_text(f'''# 阶段5：矩形设备背景编译与差分验收

第三个C切片G08增加独立静态矩形背景。与既有五类、折线、表格构成8类受限设备编译范围；完整阶段5仍未完成，本轮不访问USB通信。

## 开放范围

AMT070W124-L800X480R的800×480普通单页，1..100对象、1..8语言及原有配置守卫继续有效。矩形限定宽高至少100、整数像素、黑边/黑前景/白背景、线宽1、Alpha255；线型0/1与填充0/1的四组合已采样，包括可见内容为空但模型对象仍保留的组合。最右/下边框端点不超过799/479，不允许重叠或边缘接触。其它尺寸、颜色、粗线、虚线、Alpha、渐变、SM和活动配置继续拒绝。

使用共用闭合矩形渲染器写SheetPixel，不产生ControlPara、ControlPixel或新增PLC扫描。矩形不借用源顶层type65571作为设备记录类型。

## 原厂对照

[B/S/M明细](stage5-evidence/native/rect/B-S-M-comparison.json)包含17次有效编译：默认同源3次新进程、偏移/改尺寸、屏边、小拖动生成100×100、其余三个线/填充组合及两个非重叠矩形。变体都实际创建保存并另进程重开；8组独立C场景再次由真实buildProjectPayload与原厂夹具逐字节对比，每幅768000B RGB565完全相等。没有ROI、静态图拷贝或忽略掩码。

默认源SHA为`5ddcdcffe9825f129645565b517aba043f370306d13d49583d85a5c2f5c06e61`；原件及私有副本分别为`305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c`、`38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89`，修改范围仍为前次登记的私有3字节检查，安装原件未动。

所有原厂有效产物的PicLenPara为2400B，没有控制/动态图段。重复默认源的系统/报警/容器差异逐文件保留，Q-S05-004未整项关闭；压缩字节差异与解码语义分开。

## 小尺寸失败与探针改进

文件名中的minimum仅是原始试验标签。100,100→101,101的小拖动实际上创建100×100，不能称1×1像素成功，更不能推出原厂全局最小尺寸为100。

后续尝试拖动选择框角点到101,101并没有形成有效修改，action_save仍禁用；该probe为partial-or-error，没有输出新源。旧调用脚本只看进程退出0，随后重开不存在输入，产生FileNotFoundError；[实际错误链](stage5-evidence/native/rect/bootstrap-failure/failure.json)保留。这两次均未计入17个有效编译或8组C场景。

调查脚本现先检查输入文件存在，partial-or-error返回非零，调用者也读取最终状态；没有放宽产品校验。更小矩形继续拒绝，留待能确认实际缩放结果的新探针。

## 验证、只读复核与清理

[核心回归](stage5-evidence/rect-windows-r1/run.json)为154/0/0（新编译器87、既有后端14、扫描37、纯传输6、真实SDK宏10）；[矩形编辑器7项](stage5-evidence/rect-editor-windows-r1/run.json)另计，合计161/0/0。包含8类混合且仅5个动态控制记录的索引闭合、10段矩形包实际发布/复读、旧入口与源门槛、未支持样式/尺寸/接触及输出不变。

只读复核检查闭合路径、四种线/填充分支、零动态图索引、包含右/下端点的范围、帧/清单/源身份及旧五类分支；未改动独立动态记录、旧R绘制默认值或拓扑守卫。未知type仍拒绝，不以shape相似替代圆角/椭圆等证据。

18份原始probe（17有效、1失败）与checkpoint已gzip回解核SHA，bootstrap错误另存；删除{cleanup['deletedFiles']}个本任务拥有的运行副本/解码中间物，{cleanup['deletedLogicalBytes']}逻辑字节。[清理回执](stage5-evidence/native/rect/cleanup.json)保留。正在用于后续切片的构建、唯一源/段、脚本、用户主目录build、SDK和原件均保留。

多页/窗口/配置、其余动态类型、其它图形变体、完整源及原厂介质容器仍按阶段5任务和问题清单推进；T交阶段6、E交阶段10，没有继承历史下载ACK或人工显示确认。
''',encoding='utf-8',newline='\n')
summary='G08矩形已增加受限背景C：宽高至少100、黑白/宽1/Alpha255、线型0/1×填充0/1、无重叠/接触；17次有效原厂编译和8组真实C场景整帧相等，本批161/0/0。小尺寸缩放失败已保留，SM/未知样式和新产物T/E未通过。'
append('docs/矩形图形实现与验收.md','## 2026-11-13阶段5后续状态\n\n'+summary+'\n\n[当前C报告](阶段5_矩形设备背景编译与差分验收.md)更新前文阶段4历史状态，不改写原厂基线T/E身份。')
append('design_plan/06_阶段5_设备编译与格式差分.md','## 9. 第三批矩形实际执行\n\n'+summary+'\n\n[专项报告](../docs/阶段5_矩形设备背景编译与差分验收.md)明确实际源、失败输入链和保守拒绝范围；阶段5整体仍未完成。')
append('design_plan/A05_源格式与设备编译契约.md','## 10. 阶段5矩形背景补充（2026-11-13）\n\n'+summary+'\n\n没有新ControlPara联合体；源type65571不等于设备type。SheetPixel与2400B索引复用既有格式，参见[差分明细](../docs/阶段5_矩形设备背景编译与差分验收.md)。')
protocol='docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md';text=(work/protocol).read_text(encoding='utf-8-sig');section=max(map(int,re.findall(r'^## (\d+)\.',text,re.M)))+1
append(protocol,f'## {section}. 阶段5静态矩形独立背景（2026-11-13）\n\n'+summary+'\n\n本批只编译/核验10/12/14数据段和纯计划，没有访问USB。历史原厂基线传输不转作新产物通过。见[专项报告](阶段5_矩形设备背景编译与差分验收.md)。')
matrix=work/'design_plan/控件文档与实施覆盖矩阵.md';lines=matrix.read_text(encoding='utf-8-sig').splitlines()
for i,line in enumerate(lines):
    if line.startswith('| 矩形 |'):
        fields=line.split('|');fields[4]+=' [阶段5C](../docs/阶段5_矩形设备背景编译与差分验收.md)';fields[9]=' 受限黑白线框/纯色背景C切片 ';fields[10]=' 原厂基线已验；新产物T待阶段6 ';fields[12]=' 小尺寸/变体/SM和新产物T/E待验 ';lines[i]='|'.join(fields)
matrix.write_text('\n'.join(lines)+'\n',encoding='utf-8',newline='\n')
progress=read(base/'progress.json');progress.update(active='继续其它图形/动态记录、重叠与多页配置的最小原厂探针',selectedSlice='既有五类 + G04正交折线 + G10静态表格 + G08矩形',softwareTests=totals,nativeCanonicalPixelCases=26,complete=False);progress['rectangleSlice']=dict(nativeCompilations=17,independentCanonicalFrameCases=8,softwareTests=totals,cleanupFiles=cleanup['deletedFiles'],cleanupLogicalBytes=cleanup['deletedLogicalBytes'],report='docs/阶段5_矩形设备背景编译与差分验收.md',usbAccessed=False,smallGeometryVerified=False)
scopes={'S05-02':'G04六组、G10十二组、G08八组独立保存重开背景；同源新进程及非确定差异保留，无忽略掩码。','S05-03':'共享不可变单页计划、稳定顺序及三类静态图形属性拒绝；完整多页IR仍未恢复。','S05-04':'复用已有五类控制记录；三类图形只有背景、无新ControlPara；其余26动态类型尚未恢复。','S05-05':'26组独立原厂整帧RGB565编译场景通过；不代表所有样式或重叠组合。','S05-09':'最新161/0/0（核心154与受影响编辑器7）；旧253基线及107/143回归保留历史，不重复相加。','S05-10':'每切片分别归档和定向清理；当前矩形541文件/398054767逻辑字节，构建仍供后续使用。'}
for task in progress['tasks']:
    if task['id'] in scopes:task['scope']=scopes[task['id']]
write(base/'progress.json',progress)
append('docs/阶段5逐项进度.md','## 第三批：矩形\n\n'+summary+'\n\n当前5类控制记录与3类受限静态背景独立编译。S05-07、多页IR、活动配置、其余26动态类型和所有新产物T/E仍未闭合；不能把3个图形切片算作完整阶段5。')
ledger_path=work/'design_plan/分阶段文档证据/阶段交付与清理记录.json';ledger=read(ledger_path);ledger['stage5Slice3']=dict(completeStage5=False,softwareTests=totals,nativeCompilations=17,canonicalRgb565Cases=8,usbAccessed=False,cleanupFiles=cleanup['deletedFiles'],cleanupLogicalBytes=cleanup['deletedLogicalBytes'],report='../docs/阶段5_矩形设备背景编译与差分验收.md');write(ledger_path,ledger)
append('程序修改记录.md','### 修改75：阶段5受限矩形独立背景编译\n- 修改内容：\n  1. 独立矩形背景支持黑白、宽1、不透明、线型0/1和填充0/1；保守拒绝小尺寸/样式及边界/接触。\n  2. 原厂17个有效编译、8个真实编译场景全帧等同及8类混合索引；源/旧入口/包事务守卫继续有效。\n  3. 保留小尺寸未修改及缺输入失败链，修正调查脚本退出状态，并归档/定向清理运行副本。\n- 修改文件：src/device_compile_plan.h/.cpp、independent_downloader.cpp、tests/stage5_compiler_tests.cpp及阶段5文档/协议/证据/台账、本记录。\n- 影响范围：受限G08静态C；不开放SM、其它图形/动态记录、多页配置或新产物T/E。\n- 原因：继续用户授权的阶段5设备编译与差分任务，避免使用原厂现成图替代独立输出。\n- 修改标记：//20261113-13 add/change')
repro=native/'reproduction';repro.mkdir(exist_ok=True)
for p in root.iterdir():
    if p.is_file() and (p.name.startswith(('rect-','run_rect','analyze_rect')) or p.name in ('native_probe.py','rect_raster_probe.cpp','run_table_editor_checks.py','archive_family.py','execute_family_cleanup.ps1','document_rect_slice.py','repeat-one.json')):
        if p.suffix in ('.py','.cpp','.json','.log','.ps1'):shutil.copy2(p,repro/p.name)
write(native/'review.json',dict(mode='read_only_after_implementation',businessLogicChangedDuringReview=False,softwareTests=totals,checks=['closed path and 0/1 line/fill','static zero-record and zero-sprite layout','stroke extent and touching edges','source and legacy gates','old five plus three static mixed indexes','no USB calls'],remaining=['sub100 geometry','other colors/styles/alpha','overlaps','SM and multi-page/configuration'],files=[dict(path=p,sha256=sha(work/p)) for p in ['src/device_compile_plan.h','src/device_compile_plan.cpp','src/independent_downloader.cpp','tests/stage5_compiler_tests.cpp']],usbAccessed=False))
print(json.dumps(dict(softwareTests=totals,protocolSection=section,completeStage5=False),ensure_ascii=False))
