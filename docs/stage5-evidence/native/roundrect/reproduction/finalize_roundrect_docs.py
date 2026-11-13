"""Record measured roundrect results and backfill rules from existing UI contracts."""
from pathlib import Path
from datetime import datetime, timezone
import hashlib
import json
import re

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
append=lambda p,s:p.write_text(p.read_text(encoding='utf-8-sig').rstrip()+'\n\n'+s.strip()+'\n',encoding='utf-8')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
core=read(work/'docs/stage5-evidence/roundrect-windows-r2/run.json')
editor=read(work/'docs/stage5-evidence/roundrect-editor-windows-r2/run.json')
assert core['passed'] and editor['passed']
count=sum(r['totals']['passed'] for r in core['runs'])+sum(r['totals']['passed'] for r in editor['runs'])
assert count==482
cleanup=read(work/'docs/stage5-evidence/native/roundrect/cleanup.json')
audit=read(work/'docs/stage5-evidence/native/audit/cleanup.json')
assert cleanup['deletedFiles']==690 and audit['deletedFiles']==180
rules=work/'docs/UI编译规则与阶段交付规范.md'
text=rules.read_text(encoding='utf-8')
assert '### 6.1' not in text
progress4=(work/'docs/阶段4逐控件进度.md').read_text(encoding='utf-8-sig')
types=[('message','messageDisplay'),('scale','scale'),('qrcode','qrCode'),('image','imageDisplay'),('gif','gifDisplay'),('number','numberDisplay'),('ascii','asciiDisplay'),('time','timeDisplay'),('screen','screenButton'),('multistate','multiStateButton'),('multifunction','multiFunctionButton'),('dropdown','dropdown'),('flow','flowBlock'),('slider','slider'),('wheel','wheel'),('track','moveTrack'),('chart','chart'),('trend','trend'),('historytrend','historytrend'),('xycurve','xycurve'),('datagroup','datagroup'),('alarmbar','alarmBar'),('alarmdisplay','alarmDisplay'),('historyalarm','alarmDisplay'),('historydata','historyData'),('recipe','recipeDisplay')]
rows=[]
for n,(kind,key) in enumerate(types,6):
    section=re.search(r'^## C'+f'{n:02d}'+r'：[^\n]+\n\s*([^\n]+)',progress4,re.M).group(1)
    link=re.search(r'\[专项报告\]\(([^)]+)\)',section).group(1)
    assert (work/'docs'/link).is_file()
    details=section.split('本次')[0].strip().rstrip('。；')
    rows.append(f'| C{n:02d} [{link.removesuffix("手册实现与验收.md")}]({link}) | `{kind}/{key}` | {details} |')
addition='''### 6.1 新增26类UI→模型规则补录

下表从各控件已经交付的实施记录逐项补录，JSON键核对`HmiObject::toJson`。详细字段默认/范围和活动联动仍以链接报告、对应严格解析器为准。每行目前完整`.sm`与设备C均拒绝；PC预览、KSP重开和原厂基线传输不能替代独立设备记录实现。

| ID及规则来源 | type / 专有属性键 | 已交付的界面、模型和渲染规则 |
|---|---|---|
'''+ '\n'.join(rows)+'''

C24/C30依赖采集定义，C27～C29依赖报警定义，C31依赖配方定义；先有有效配置才能创建对应控件。配置/UI已保存仍不等于活动设备段已恢复，含这些未恢复活动节点的工程在C入口整体拒绝。没有前置配置与设备编码未实现是两个独立原因，必须分别报告。

'''
text=text.replace('## 7. 五类既有设备控制记录规则',addition+'## 7. 五类既有设备控制记录规则')
text=text.replace('（见实际程序帮助核参数）','')
rules.write_text(text,encoding='utf-8')

report='''# 阶段5：圆角矩形定点背景编译与差分验收

第六个静态背景C切片G09已完成受限实现，当前为五类控制记录+六类静态背景。阶段5共10项主任务仍未全部完成；其它动态记录、多页、配置和未核样式继续推进。本轮严格纯离线，没有USB通信，新产物deviceAcknowledged=false。

## 支持范围及编码

AMT070W124-L800X480R、800×480普通单页；圆角半径10/10，X/Y至少1、宽高至少100、x+w≤799且y+h≤479；黑边/黑前景/白背景草稿、线宽1、Alpha255、线型0/1×填充0/1；不与其它对象相交或边缘接触。尺寸及边距是本切片保守范围，不是已证明的原厂最小值。休眠颜色和宽度草稿也保留本次限制，未知属性不偷偷通过。

`prepareDeviceCompilePlan`冻结并计入staticRoundrectCount；`paintDeviceRoundrectGraphic`独立产生背景跨度，只烘SheetPixel，不新增ControlPara/ControlPixel或扫描读取。十一对象混合仍只有5条640B动态控制记录，rawOffset/rawBytes与12段结构保持闭合。完整SM、旧单文本入口、其它半径/尺寸/颜色/透明及重叠继续拒绝。

## 原厂输入和B/S/M证据

[完整B/S/M](stage5-evidence/native/roundrect/B-S-M-fixed-scanline-r1.json)记录23次原厂有效编译：默认同源3次新进程；偏移75,65/160×120、宽300×150、高120×250、近边1,1、远边598,378、奇数201×101各创建+重开；线/填0/0、0/1、1/0各创建+重开；双对象各创建+重开。全部768000B RGB565逐字节相等，没有像素或ROI忽略区。13份canonical对应11个独立场景，默认3份重复不当作3个独立几何。

默认源SHA为`8846ae61ad43d67c82c8242d25da49a8970290f3acaef42ec91e9aebee71fc0e`。实际UI线型/填充控件m_LineType/m_Pattern经过set/get；每份场景bbox与sceneTransform=[1,0,0,1,x,y,0,0,1]绑定，确认对象存在才计目标成功。默认完整帧SHA为`1b3ae9051931ab0a5e6a80f8326636b4e8d65a2e360206e6773ca788691b9a82`。所有图形产物PicLenPara2400B且无控制/动态图段。

原件及运行副本身份沿用[规则文档](UI编译规则与阶段交付规范.md)第2节；原厂QtGui.dll实际FileVersion为4.6.3。重复同源Sys/Alarm/Usbload/load/upLoad差异逐文件保留为未知；Q-S05-004没有整体关闭，不设置整段忽略掩码。

## 37像素差异的定位与独立算法

被动记录原厂QPixmap800×480、黑色cosmetic画笔宽0、SquareCap16、BevelJoin64、无路径抗锯齿及实际Raster后端。GDI路径候选虽仅差7像素，但实际绘图后端不是GDI，因此排除。Qt6重放相同路径、cosmetic/格式/局部绝对坐标候选均仍差37；描边路径填充差49，不能用半径单位猜测或抹除差异通过。

恢复26.6定点三次曲线细分（flatness16、深度≤32）与16.16有向扫描交点、半像素偏置/向下平局后，原厂记录的路径先0差异；再独立由Qt6生成同语义路径/宽1描边并使用该扫描法，默认及正式23次语料仍整帧0差异。生产模块`device_roundrect_raster.cpp`独立计算路径与跨度，不依赖原厂DLL、路径字节或原厂图像。

公共Qt4.6.3绘图库源码只提供调查线索，来源URL/SHA及原始版权头保存在[public-qt-sources](stage5-evidence/native/roundrect/reproduction/public-qt-sources/sources.json)。没有把原库源码直接引入生产模块。定点规则、UI属性映射和复验步骤详见[UI编译规则](UI编译规则与阶段交付规范.md)第8节。椭圆/扇形仍有差异，多边形绘制路径未形成充分证据，没有顺带放开其它图形。

## 真实回归及只读复核

[red原始结果](stage5-evidence/roundrect-red-r1/run.json)：开闸前实际2通过/13失败/0跳过，13份canonical均被未支持type拒绝，失败原因符合预期。不是编译失败或没有执行的测试。

[最终核心回归](stage5-evidence/roundrect-windows-r2/run.json)248/0/0：阶段5编译181、后端14、扫描37、纯传输计划6、真实ARM SDK宏10；[完整控件编辑器](stage5-evidence/roundrect-editor-windows-r2/run.json)234/0/0，共482/0/0。本次编辑器执行完整目标，旧批次的8项是筛选测试，不能把两者数字当同一测试集合。r1原始通过结果也保留，样式规范整理后再跑r2绑定最终EXE。

新增13份真实原厂整帧夹具、16个不支持样式/半径/尺寸/边界/相交及接触负向、真实10段发布复读、十一对象混合。负向验证计划和payload保持旧输出，源/设备门槛独立。历史226/191/161等按原身份保留，不累计冒充本批。

[只读复核](stage5-evidence/roundrect-windows-r2/readonly-review.json)检查定点中间量边界、符号移位、曲线终止、闭合/填充规则、同X交点、QPainter活跃写入、守卫与索引/事务。没有阻塞发现。复核发现新测试两处声明位置及新循环声明需按项目风格整理，已在复核反馈后仅作声明/遍历语法整理，无业务语义改变，并通过r2回归。

## 文档补齐与定向清理

按用户新增要求补齐[UI编译规则与阶段交付规范](UI编译规则与阶段交付规范.md)：阶段0～4模型/界面/资源、26类新增控件逐项属性键和依赖、五类控制记录及六类背景算法、段/索引/扫描/宏/事务包、构建复验和每批更新要求。该文档与专项报告/协议/阶段进度同步，后续每个切片都要维护。

原始23次圆角及6次被动绘图观测、checkpoint均压缩回解核SHA；唯一源/设备段、失败候选、后端证据及复现脚本保留。[圆角清理](stage5-evidence/native/roundrect/cleanup.json)690文件/482052342逻辑字节，[新增audit清理](stage5-evidence/native/audit/cleanup.json)180文件/125761533逻辑字节，共870文件/607813875逻辑字节。包含硬链接，不等于物理释放量。旧audit180文件的回执另存cleanup-before-roundrect.json，没有覆盖抹掉历史。

清理仅针对已核SHA、无存活进程的本任务runtime及可再生解码帧；构建缓存、用户运行的阶段4程序、用户build/dist、原厂安装与SDK保留。TEMP/TMP/TMPDIR持续指向Z盘自建目录；本轮不清用户全局临时文件。

未决：其它半径/颜色/透明/小尺寸、完整SM/其它图形和26类动态记录、多页/活动配置及未知容器区。新产物T交阶段6、E交阶段10，历史原厂屏显确认不转作本次新编译器E通过。
'''
(work/'docs/阶段5_圆角矩形定点背景编译与差分验收.md').write_text(report,encoding='utf-8')
summary='G09圆角矩形已接入受限定点背景C：半径10/10、X/Y≥1、宽高≥100、黑白/宽1/Alpha255、线型0/1×填充0/1、无相交或接触。23次原厂编译全帧相等，13份canonical/11独立场景；最终核心248+完整编辑器234共482/0/0。完整SM及未知样式/多页配置和新产物T/E未开放，没有USB通信。'
append(work/'docs/阶段5逐项进度.md','## 第六批：圆角矩形定点背景\n\n'+summary+'\n\n[专项报告](阶段5_圆角矩形定点背景编译与差分验收.md)。规则已集中补齐于[UI编译规则](UI编译规则与阶段交付规范.md)，后续每个阶段性任务同步维护。阶段5共10项主任务仍未整体完成。')
append(work/'design_plan/06_阶段5_设备编译与格式差分.md','## 12. 第六批圆角矩形实际执行\n\n'+summary+'\n\n[专项报告](../docs/阶段5_圆角矩形定点背景编译与差分验收.md)。已补齐[UI编译规则与阶段交付规范](../docs/UI编译规则与阶段交付规范.md)，每批必须更新字段/编码/证据/未支持范围。')
append(work/'docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','## 89. 阶段5圆角矩形定点背景编译（2026-11-13）\n\n'+summary+'\n\n只写SheetPixel，不新增控制/动态图/扫描。十一对象混合仍5条640B记录；10/12/14段既有流程保持。定点26.6细分/16.16交点替代Qt6直接曲线扫描的37像素差异，未使用像素忽略区。详见[专项报告](阶段5_圆角矩形定点背景编译与差分验收.md)及[详细UI编译规则](UI编译规则与阶段交付规范.md)。deviceAcknowledged=false。')
append(work/'design_plan/A05_源格式与设备编译契约.md','## 13. 圆角矩形静态C与规则补录（2026-11-13）\n\n'+summary+'\n\n23次输入均核sceneTransform与实际UI属性，源类型不作为设备type。生产独立生成曲线路径及定点跨度，2400B PicLenPara无动态图；原厂非确定容器区继续未知。[专项报告](../docs/阶段5_圆角矩形定点背景编译与差分验收.md)和[规则](../docs/UI编译规则与阶段交付规范.md)。')
p=work/'design_plan/控件文档与实施覆盖矩阵.md';t=p.read_text(encoding='utf-8-sig')
lines=t.splitlines()
for i,line in enumerate(lines):
    if line.startswith('| 圆角矩形 |'):
        line=line.replace('[本轮报告](../docs/圆角矩形图形实现与验收.md)','[本轮报告](../docs/圆角矩形图形实现与验收.md) [阶段5C](../docs/阶段5_圆角矩形定点背景编译与差分验收.md)')
        line=line.replace('| 阻塞 | 原厂基线下载通过；独立编码阻塞 |','| 半径10黑白受限定点背景C切片 | 原厂基线已验；新产物T待阶段6 |')
        line=line.replace('接续专项报告未决项','其它半径/样式/尺寸/SM及新产物T/E待验');lines[i]=line
p.write_text('\n'.join(lines)+'\n',encoding='utf-8')
for file in ('design_plan/00_复刻执行总入口.md','docs/阶段4逐控件进度.md'):
    p=work/file;t=p.read_text(encoding='utf-8-sig');first,rest=t.split('\n',1)
    link='../docs/UI编译规则与阶段交付规范.md' if file.startswith('design_plan') else 'UI编译规则与阶段交付规范.md'
    p.write_text(first+'\n\n> 新增维护入口：[UI编译规则与阶段交付规范]('+link+')；已补齐前序阶段规则，后续每批同步更新。\n'+rest,encoding='utf-8')
append(work/'design_plan/复刻待验线索台账.csv',f'Q-S05-G09-C,阶段5圆角矩形,design_plan/06_阶段5_设备编译与格式差分.md,1,{sha(work/"design_plan/06_阶段5_设备编译与格式差分.md")},半径10黑白和已采样尺寸的定点背景C已恢复，SM/其它半径样式尺寸及新产物T-E待验,编译切片,受限C通过，整体未闭合,docs/阶段5_圆角矩形定点背景编译与差分验收.md,继续未知字段原厂探针；T-E按阶段6和10单独验证')
p=work/'docs/stage5-evidence/progress.json';d=read(p)
d.update(active='G09圆角矩形受限C及前序UI编译规则补齐；准备第六批便携交付',selectedSlice=d['selectedSlice']+' + G09圆角矩形',softwareTests=dict(passed=482,failed=0,skipped=0),nativeCanonicalPixelCases=61,uiCompilationRules='docs/UI编译规则与阶段交付规范.md')
changes={
'S05-02':'既有50场景加圆角11独立场景；圆角23次有效编译/13canonical，非确定区无忽略掩码。',
'S05-03':'共享不可变单页计划、六类静态图形守卫；完整多页IR未恢复。',
'S05-04':'五类控制记录与六类静态背景；26个新动态类型未恢复。',
'S05-05':'61个独立类型/场景；圆角13份实际编译全帧相等，不推广其它曲线。',
'S05-09':'最终482/0/0：核心248与完整控件编辑器234；r1及历史结果分别保留。',
'S05-10':'本批圆角690文件与新增audit180文件共607813875逻辑字节定向清理；原始观测gzip回解核SHA。'}
for task in d['tasks']:
    if task['id'] in changes:task['scope']=changes[task['id']]
d['roundrectSlice']=dict(nativeCompilations=23,canonicalFixtures=13,independentCanonicalFrameCases=11,softwareTests=dict(passed=482,failed=0,skipped=0),cleanupFiles=870,cleanupLogicalBytes=607813875,report='docs/阶段5_圆角矩形定点背景编译与差分验收.md',usbAccessed=False,E=False)
write(p,d)
review=dict(date=datetime.now(timezone.utc).isoformat(),mode='read_only_review_then_style_only_followup_retested',reviewed=['26.6 coordinate and cross-product bounds within admitted 800x480 page','GCC/MinGW arithmetic signed shifts are part of the frozen toolchain; negative slope tested with native full frames','bounded curve subdivision depth and local radius10','closed subpaths, winding/odd-even and same-X intersections','no direct active-QPainter QImage memory mutation','radius/style/dormant drafts and geometry gates reject before output allocation','five dynamic records and raw offset/index closure unchanged','negative plan/payload preserve caller output; source and device gates independent'],blockingFindings=[],nonBlockingFindings=['new test declarations and range variable placement adjusted after reporting review; no business semantics changed'],finalRegression=dict(core='roundrect-windows-r2/run.json',editor='roundrect-editor-windows-r2/run.json',passed=482,failed=0,skipped=0),remaining=['other roundrect radii/style/small size/overlap','ellipse sector polygon curve native equivalence','complete SM and new dynamic types','multipage and active configuration','T/E stages6/10'],usbAccessed=False)
review['sourceFiles']={name:sha(work/name) for name in ('src/device_roundrect_raster.cpp','src/device_roundrect_raster.h','src/device_compile_plan.cpp','src/independent_downloader.cpp','tests/stage5_compiler_tests.cpp')}
write(work/'docs/stage5-evidence/roundrect-windows-r2/readonly-review.json',review)
append(work/'程序修改记录.md','''### 修改81：阶段5圆角矩形定点背景与前序UI编译规则补齐
- 修改内容：
  1. 新增圆角矩形半径10黑白受限背景C；独立26.6曲线细分和16.16扫描跨度解决Qt6直接绘制37像素差异，不引入原厂图像或DLL。
  2. 原厂23次有效编译、11独立场景/13canonical全帧相等；13整帧正例、16负向、十一对象混合和实际包复读，最终248核心+234完整编辑器共482/0/0。
  3. 补齐阶段0～4、26类新增控件属性键/依赖、五类记录和六类背景的详细UI编译规则及每批维护规范；同步协议89、A05、进度、矩阵和待验台账。
  4. 只读复核后仅整理声明风格并重验；原始观测及公共库来源归档回解核SHA，定向清理870文件/607813875逻辑字节，保留构建和用户产物。
- 修改文件：CMakeLists.txt、src/device_roundrect_raster.h/.cpp、src/device_compile_plan.h/.cpp、src/independent_downloader.cpp、tests/stage5_compiler_tests.cpp、docs/UI编译规则与阶段交付规范.md、阶段5报告/协议/进度/证据、design_plan契约/入口/矩阵/台账、本记录。
- 影响范围：AMT单普通页第六个静态C切片；其它半径/样式/源/多页配置和新产物T/E未开放；没有USB通信。
- 原因：继续阶段5，并落实用户每阶段性任务详细记录UI编译规则、补齐前序遗漏的新增要求。
- 修改标记：//20261113-16 add/change''')
for match in re.finditer(r'\]\(([^)]+)\)',rules.read_text(encoding='utf-8')):
    target=match[1].split('#')[0]
    assert (rules.parent/target).exists(),target
print(json.dumps(dict(regressionPassed=count,rulesBytes=rules.stat().st_size,stage4Rows=len(rows),cleanupFiles=870,wholeStage5Complete=False),ensure_ascii=False))
