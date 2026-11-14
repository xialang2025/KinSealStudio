"""Publish only measured polygon results after real regression and owned cleanup receipts exist."""
from pathlib import Path
from datetime import datetime,timezone
import csv,hashlib,json,re,shutil
root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def append(relative,text):
 p=work/relative;p.write_text(p.read_text(encoding='utf-8-sig').rstrip()+'\n\n'+text.strip()+'\n',encoding='utf-8')
core=read(work/'docs/stage5-evidence/polygon-windows-r1/run.json');editor=read(work/'docs/stage5-evidence/polygon-editor-windows-r1/run.json')
assert core['passed'] and editor['passed']
core_count=sum(r['totals']['passed'] for r in core['runs']);editor_count=sum(r['totals']['passed'] for r in editor['runs']);assert (core_count,editor_count)==(296,234)
cleanup=read(work/'docs/stage5-evidence/native/polygon/cleanup-polygon-20261114.json');assert cleanup['rawArchivesAndUniqueEvidencePreserved'] and cleanup['incrementalBuildPreserved']
corpus=read(work/'docs/stage5-evidence/native/polygon/B-S-M-cell-area-r1.json');assert len(corpus['cases'])==37 and corpus['allRasterComparisonsEqual']
assert all(c['M']['differentPixels']==0 for c in corpus['cases'])
red=read(work/'docs/stage5-evidence/polygon-red-r1/run.json');log=work/'docs/stage5-evidence/polygon-red-r1/polygon-red.txt';totals=re.search(rb'Totals: (\d+) passed, (\d+) failed, (\d+) skipped',log.read_bytes());assert tuple(map(int,totals.groups()))==(2,20,0) and red['exitCode']==20
write(work/'docs/stage5-evidence/polygon-red-r1/analysis.json',dict(actualTotals=dict(passed=2,failed=20,skipped=0),exitCode=20,expectedFailureVerified=True,rawRun='run.json',logSha256=sha(log),note='The original launcher expects exit0 and correctly records passed=false. All20 data rows were rejected before the polygon gate. The later console UTF8 decode error affected only displaying the already-written GBK Qt log.',usbAccessed=False))
review=dict(date=datetime.now(timezone.utc).isoformat(),mode='read_only_review',reviewed=['qint64 rational floor and positive denominator bounds','bounded3..8 points and finite screen coordinates before integer conversion','closed simple paths, reversed orientation and concave fixtures','odd-even cover folding and final RGB32 blending before RGB565','straight-only stroker output rejects curves before drawing','both peer patches exclude physical boundary contact; no active QPainter image-memory mutation','source identity and20 rows derived from37 native compilations','static polygon count has no control record or scan allocation','failure closes painter and preserves caller payload; source/legacy gates stay rejected'],blockingFindings=[],nonBlockingFindings=[],sourceFiles={n:sha(work/n) for n in ('src/device_polygon_raster.cpp','src/device_polygon_raster.h','src/device_compile_plan.cpp','src/device_compile_plan.h','src/independent_downloader.cpp','tests/stage5_compiler_tests.cpp','CMakeLists.txt')},finalRegression=dict(core='polygon-windows-r1/run.json',editor='polygon-editor-windows-r1/run.json',passed=530,failed=0,skipped=0),remaining=['far-screen edge native creation not bound; admitted max698398 only','ellipse has default candidate only; sector still1 pixel','other styles, small polygons, intersecting paths and overlapping patches','completeSM,26 dynamic families,multipage and active configurations','new product T/E remain separate stages6/10'],usbAccessed=False)
write(work/'docs/stage5-evidence/polygon-windows-r1/readonly-review.json',review)
shutil.copy2(root/'polygon-green-build-r1.log',work/'docs/stage5-evidence/polygon-windows-r1/build-r1.log')
summary='G09多边形已接入第七个受限静态背景C：3..8点简单整数闭合形，X为2..698、Y为2..398、宽高至少100；黑边/白前景/黑背景、宽1、Alpha255、线填0/1，拒绝重复/连续共线/非邻边交叉接触及补片相交接触。37次有效原厂编译全帧相等，20份canonical/18独立场景；最终核心296+完整编辑器234共530/0/0。完整SM及未知样式/边界/多页配置和新产物T/E未开放，本轮没有USB通信。'
append('docs/阶段5逐项进度.md','## 第七批：G09多边形单元格面积背景\n\n'+summary+'\n\n[专项报告](阶段5_多边形单元格背景编译与差分验收.md) · [详细UI编译规则](UI编译规则与阶段交付规范.md)。阶段5仍为10项主任务，受限范围内持续扩展，不能把7个背景家族当作已完成7项主任务。')
append('design_plan/06_阶段5_设备编译与格式差分.md','## 13. 第七批多边形实际执行\n\n'+summary+'\n\n[专项报告](../docs/阶段5_多边形单元格背景编译与差分验收.md)及[UI编译规则](../docs/UI编译规则与阶段交付规范.md)。B原始字节/S段结构/M完整像素分别保存；无忽略掩码，同源容器非确定区继续未知。后续椭圆需正式变体，扇形1像素差仍阻塞。')
append('docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','## 90. 阶段5多边形受限背景编译（2026-11-14）\n\n'+summary+'\n\n独立26.6坐标量化/256子像素单元格面积积分和RGB32先混色再转565，只写SheetPixel；PicLenPara2400B、无动态图控制记录或PLC扫描。十二对象混合仍5条640B控制记录，10/12/14段包流程保持。原厂三角形历史屏显确认没有升级为新产物E。[专项报告](阶段5_多边形单元格背景编译与差分验收.md) · [规则](UI编译规则与阶段交付规范.md)。')
append('design_plan/A05_源格式与设备编译契约.md','## 14. G09多边形受限C（2026-11-14）\n\n'+summary+'\n\n生产独立生成有序路径及宽1描边，用自写直边积分合成已核页面补片，不加载原厂DLL/路径字节/保留图像。每次原厂输入绑定顶点/笔型/单位变换/源SHA，失败链单列。[B/S/M](../docs/stage5-evidence/native/polygon/B-S-M-cell-area-r1.json)和[规则](../docs/UI编译规则与阶段交付规范.md)。')
append('docs/阶段4逐控件进度.md','## G09后续设备C更新（阶段5，2026-11-14）\n\n'+summary+'\n\n以上阶段4原记录保留其当时范围；多边形设备C阻塞已在本节限定范围内解除，源出口和其它组合仍拒绝。[阶段5专项报告](阶段5_多边形单元格背景编译与差分验收.md)。')
append('design_plan/00_复刻执行总入口.md','## 阶段5第七批进展（2026-11-14）\n\n'+summary+'\n\n当前五类控制记录加七类受限背景。下一步补G05椭圆的正式原厂变体；G07扇形保留1像素失败链，动态类型、多页及活动配置继续未决。[规则入口](../docs/UI编译规则与阶段交付规范.md)。')
p=work/'design_plan/控件文档与实施覆盖矩阵.md';lines=p.read_text(encoding='utf-8-sig').splitlines()
for i,line in enumerate(lines):
 if line.startswith('| 多边形 |'):
  lines[i]=line.replace('[本轮报告](../docs/多边形图形实现与验收.md)','[本轮报告](../docs/多边形图形实现与验收.md) [阶段5C](../docs/阶段5_多边形单元格背景编译与差分验收.md)').replace('| 阻塞 | 原厂基线下载通过；独立编码阻塞 |','| 3..8点黑色简单形受限背景C | 原厂基线已验；新产物T待阶段6 |').replace('接续专项报告未决项','完整SM/边缘/其它样式和新产物T/E待验')
p.write_text('\n'.join(lines)+'\n',encoding='utf-8')
p=work/'design_plan/A03_基础图形与表格规范.md';t=p.read_text(encoding='utf-8-sig');t=t.replace('| G09 | 多边形 | 编辑/KSP切片；源/设备阻塞 |','| G09 | 多边形 | 编辑/KSP切片；3..8点受限背景C；完整源阻塞 |');p.write_text(t,encoding='utf-8')
with (work/'design_plan/复刻待验线索台账.csv').open('a',encoding='utf-8',newline='') as f:
 csv.writer(f,lineterminator='\n').writerow(['Q-S05-G09-C','阶段5多边形','design_plan/06_阶段5_设备编译与格式差分.md',1,sha(work/'design_plan/06_阶段5_设备编译与格式差分.md'),'3至8点黑色简单整数闭合背景C通过；完整SM与屏右下边缘/其它样式及新产物T-E待验','编译切片','受限C通过，整体未闭合','docs/阶段5_多边形单元格背景编译与差分验收.md','保留全部无效采样失败链；椭圆正式变体和扇形1像素差继续调查'])
p=work/'docs/stage5-evidence/progress.json';d=read(p);d.update(active='G09多边形受限C完成；准备第七批便携交付，继续椭圆正式变体',selectedSlice=d['selectedSlice']+' + G09多边形',nativeCanonicalPixelCases=79,softwareTests=dict(passed=530,failed=0,skipped=0),complete=False)
changes={'S05-02':'既有61场景加多边形18独立场景；37次有效编译/20canonical，无效采样单列，非确定区无忽略掩码。','S05-03':'共享不可变单页计划、七类受限背景守卫；完整多页IR未恢复。','S05-04':'五类控制记录与七类静态背景；26个新动态类型未恢复。','S05-05':'79个独立类型/场景；多边形20份原厂整帧夹具，椭圆默认候选不当正式支持。','S05-09':'最终530/0/0：核心296与完整控件编辑器234；red2/20/0与旧批次各自保留。','S05-10':f'本批定向清理{cleanup["deletedFiles"]}文件/{cleanup["deletedLogicalBytes"]}逻辑字节；原始gzip回解核SHA、全部独有证据及许可证保留。'}
for task in d['tasks']:
 if task['id'] in changes:task['scope']=changes[task['id']]
 if task['id']=='S05-05':task['state']='passed_seven_canonical_background_families'
d['polygonSlice']=dict(nativeCompilations=37,canonicalFixtures=20,independentCanonicalFrameCases=18,negativeTests=27,softwareTests=dict(passed=530,failed=0,skipped=0),cleanupFiles=cleanup['deletedFiles'],cleanupLogicalBytes=cleanup['deletedLogicalBytes'],report='docs/阶段5_多边形单元格背景编译与差分验收.md',usbAccessed=False,E=False);write(p,d)
append('程序修改记录.md',f'''### 修改84：阶段5多边形单元格面积背景编译
- 修改内容：
  1. 新增G09多边形受限设备背景C；独立单元格面积积分恢复抗锯齿覆盖，RGB32精度混色后再转RGB565，生产不依赖原厂DLL或原厂帧。
  2. 37次有效编译/20canonical/18场景完整帧相等；20真实正例、27边界负例、十二对象混合、源/legacy拒绝及10段事务包复读，核心296+完整编辑器234共530/0/0。
  3. 保留r1至r7的选框/顶点/边缘手势失败链，严格绑定实际顶点和源SHA；坐标门槛收窄至X2..698/Y2..398，未知范围保持拒绝。
  4. 同步详细UI编译规则、协议90、A03/A05、阶段5进度/矩阵/台账及只读复核；定向清理{cleanup['deletedFiles']}文件/{cleanup['deletedLogicalBytes']}逻辑字节，公共源码和许可证、源/设备段及全部独有字节保留。
- 修改文件：CMakeLists.txt、src/device_polygon_raster.h/.cpp、src/device_compile_plan.h/.cpp、src/independent_downloader.cpp、tests/stage5_compiler_tests.cpp、UI规则/多边形报告、协议/进度/证据及design_plan、本记录。
- 影响范围：AMT单普通页第七个受限背景C；其它样式/边缘/完整源/多页配置及新产物T/E未开放，本轮没有USB通信。
- 原因：继续阶段5逐项恢复原厂设备编译，并按每批文档、清理和独立提交要求交付可复验结果。
- 修改标记：//20261114-1 add/change''')
print(json.dumps(dict(passed=530,nativeCompilations=37,canonical=20,independentScenes=18,cleanupFiles=cleanup['deletedFiles'],stage5Complete=False),ensure_ascii=False))
