"""Finalize the measured G06 ellipse slice and the verified A03 identity correction."""
from pathlib import Path
from datetime import datetime,timezone
import csv,hashlib,json,re,shutil
root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def append(relative,text):
 p=work/relative;p.write_text(p.read_text(encoding='utf-8-sig').rstrip()+'\n\n'+text.strip()+'\n',encoding='utf-8')
core=read(work/'docs/stage5-evidence/ellipse-windows-r2/run.json');editor=read(work/'docs/stage5-evidence/ellipse-editor-windows-r2/run.json');assert core['passed'] and editor['passed']
assert sum(r['totals']['passed'] for r in core['runs'])==340 and sum(r['totals']['passed'] for r in editor['runs'])==234
base=work/'docs/stage5-evidence/native/ellipse';corpus=read(base/'B-S-M-ellipse-cell-area-r2.json');assert len(corpus['cases'])==27 and corpus['allRasterComparisonsEqual']
cleanup=read(base/'cleanup-ellipse-20261114.json');assert cleanup['deletedFiles']==873 and cleanup['rawArchivesAndUniqueEvidencePreserved']
red=read(work/'docs/stage5-evidence/ellipse-red-r1/run.json');assert red['exitCode']==15 and red['totals']==dict(passed=2,failed=15,skipped=0)
write(work/'docs/stage5-evidence/ellipse-red-r1/analysis.json',dict(expectedFailureVerified=True,actualTotals=red['totals'],actualExit=15,rawRun='run.json',sourceFixtureCount=15,usbAccessed=False))
review=dict(date=datetime.now(timezone.utc).isoformat(),mode='read_only_review_then_reported_fixes_and_final_regression',reviewed=['all control points bounded before26.6 quantization','depth0..8,input512,generated4096 with complete cubic prebudget','historical b.x+c.y cross-term retained','Exact8Bit preserves flattened1/256 grid; polygon keeps26.6','separate source/destination D(v)=(v+(v>>8)+128)>>8','all fill/stroke masks complete before patch creation or painting','plan and direct renderer match sampled bounds and endpoint policy','peer patch contact exclusion and payload transaction','13 objects preserve5 dynamic records and scan/raw offset closure'],findings=[dict(priority='P2',problem='Old unknown-type test still used newly supported ellipse.',resolution='Use unimplemented sector to retain the original rejection purpose.'),dict(priority='P3',problem='Direct raster guard was wider than compile plan.',resolution='Align geometry and endpoint guard with supported slice.'),dict(priority='P3',problem='Shared precision branch needed a durable product regression.',resolution='Add Exact8Bit vs26.6 distinction and refusal/output preservation tests.')],blockingFindingsRemaining=[],finalRegression=dict(core='ellipse-windows-r2/run.json',editor='ellipse-editor-windows-r2/run.json',passed=574,failed=0,skipped=0),sourceFiles={n:sha(work/n) for n in ('src/device_ellipse_raster.cpp','src/device_ellipse_raster.h','src/device_polygon_raster.cpp','src/device_polygon_raster.h','src/device_compile_plan.cpp','src/device_compile_plan.h','src/independent_downloader.cpp','tests/stage5_compiler_tests.cpp','CMakeLists.txt')},remaining=['full native source body','other colors/transparency/line widths/small or touching geometry','sector and real curves require separate native variation evidence','26 dynamic families,multipage,active configuration','new product T/E stages6/10'],usbAccessed=False)
write(work/'docs/stage5-evidence/ellipse-windows-r2/readonly-review.json',review)
for label in ('r1','r2'):shutil.copy2(root/('ellipse-green-build-'+label+'.log'),work/'docs/stage5-evidence/ellipse-windows-r2'/('build-'+label+'.log'))
algorithm=root/'ellipse_algorithm';retained=base/'reproduction-20261114'/'independent-candidate';retained.mkdir(exist_ok=True)
for name in ('ellipse-algorithm-reproduction-r1.tar.gz','reproduction-manifest-r1.json','cleanup-r1.json','cpp-ellipse-coverage-r1.json','guard-api-r1.json','README.md'):
 shutil.copy2(algorithm/name,retained/name)
for name in ('module-crosscheck-r1.tar.gz','module-mask-crosscheck-r1.json','cleanup-r1.json'):
 target=retained/'module-crosscheck';target.mkdir(exist_ok=True);shutil.copy2(algorithm/'module_crosscheck'/name,target/name)
write(retained/'identity-and-rounding-superseding-note.json',dict(authority='A03 + stage4 family reports',G05='roundrect',G06='ellipse',candidateArchiveHistoricalG05LabelWasWrong=True,candidateOldComposeWasOnlyDefaultValidated=True,correctedFormula='(v+(v>>8)+128)>>8',formalReport='../../B-S-M-ellipse-cell-area-r2.json',productionCodeIsIndependent=True))
summary='G06圆/椭圆已接入第八个受限静态背景C：X/Y至少2、宽高至少100、右端不超过698、下端不超过378；黑边/黑前景/白背景、宽1、Alpha255、线填0/1，拒绝补片相交或接触。27次有效原厂编译全帧相等，15份canonical/13独立场景；最终核心340+完整编辑器234共574/0/0。完整SM、其它样式/边界/多页活动配置和新产物T/E未开放，没有USB通信。'
append('docs/阶段5逐项进度.md','## 第八批：G06圆/椭圆定点细分背景\n\n'+summary+'\n\n[专项报告](阶段5_椭圆定点细分背景编译与差分验收.md) · [详细规则](UI编译规则与阶段交付规范.md)。阶段5仍有10项主任务，八个背景家族不是八项主任务整体完成。')
append('design_plan/06_阶段5_设备编译与格式差分.md','## 14. 第八批圆/椭圆实际执行\n\n'+summary+'\n\nr1宽椭圆差1像素、无描边白填差5像素；修正div255中先右移再加128后r2全部相等，失败帧及12条像素诊断保留，没有忽略区。[专项报告](../docs/阶段5_椭圆定点细分背景编译与差分验收.md)和[规则](../docs/UI编译规则与阶段交付规范.md)。')
append('docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','## 91. 阶段5圆/椭圆背景编译（2026-11-14）\n\n'+summary+'\n\n独立26.6控制点量化、历史自适应曲线细分、8位子像素格点面积积分和RGB32先混色后转565，只写SheetPixel；PicLenPara2400B，无新增控制记录/动态图/扫描。十三对象混合仍5条640B动态记录，10/12/14段结构保持。精确D(v)=(v+(v>>8)+128)>>8，source/dest各D后相加。[专项报告](阶段5_椭圆定点细分背景编译与差分验收.md)及[UI规则](UI编译规则与阶段交付规范.md)。')
append('design_plan/A05_源格式与设备编译契约.md','## 15. G06椭圆受限C与编号核正（2026-11-14）\n\n'+summary+'\n\n按A03当前表及阶段4原报告，G05为圆角矩形、G06为圆/椭圆；此前修改83仍写错圆角编号，现修正当前规则/报告/协议/计划/机器进度/CSV，原始提交消息、raw及历史重现脚本保持原样。编号不等于原厂sourceType/action或设备type。[B/S/M r2](../docs/stage5-evidence/native/ellipse/B-S-M-ellipse-cell-area-r2.json)。')
append('docs/阶段4逐控件进度.md','## G06后续设备C更新（阶段5，2026-11-14）\n\n'+summary+'\n\n阶段4历史原厂屏显仅对应其原厂页，不转作本次新编码T/E。[专项报告](阶段5_椭圆定点细分背景编译与差分验收.md)。')
append('design_plan/00_复刻执行总入口.md','## 阶段5第八批进展（2026-11-14）\n\n'+summary+'\n\n当前五类控制记录加八类受限背景。继续扇形、真实曲率、26新动态类型、多页及活动配置；阶段5整体未完成。G05圆角/G06圆与椭圆以A03原文为准。')
p=work/'design_plan/A03_基础图形与表格规范.md';text=p.read_text(encoding='utf-8');text=text.replace('| G06 | 圆/椭圆 | 编辑/KSP切片；源/设备阻塞 |','| G06 | 圆/椭圆 | 编辑/KSP切片；黑白受限背景C；完整源阻塞 |');p.write_text(text,encoding='utf-8')
p=work/'design_plan/控件文档与实施覆盖矩阵.md';lines=p.read_text(encoding='utf-8-sig').splitlines()
for i,line in enumerate(lines):
 if line.startswith('| 圆/椭圆 |'):
  lines[i]=line.replace('[本轮报告](../docs/圆与椭圆图形实现与验收.md)','[本轮报告](../docs/圆与椭圆图形实现与验收.md) [阶段5C](../docs/阶段5_椭圆定点细分背景编译与差分验收.md)').replace('| 阻塞 | 原厂基线下载通过；独立编码阻塞 |','| 黑白/宽1定点背景C切片 | 原厂基线已验；新产物T待阶段6 |').replace('接续专项报告未决项','完整SM/其它样式边界和新产物T/E待验')
p.write_text('\n'.join(lines)+'\n',encoding='utf-8')
with (work/'design_plan/复刻待验线索台账.csv').open('a',encoding='utf-8',newline='') as f:
 csv.writer(f,lineterminator='\n').writerow(['Q-S05-G06-C','阶段5圆与椭圆','design_plan/06_阶段5_设备编译与格式差分.md',1,sha(work/'design_plan/06_阶段5_设备编译与格式差分.md'),'黑白宽1线填0或1和已采样范围的定点背景C通过；完整SM与其它样式边界及新产物T-E待验','编译切片','受限C通过，整体未闭合','docs/阶段5_椭圆定点细分背景编译与差分验收.md','保持r1舍入失败链；继续扇形及真实曲率探针'])
p=work/'docs/stage5-evidence/progress.json';d=read(p);d.update(active='G06圆/椭圆受限C完成；准备第八批便携，继续扇形等未决',selectedSlice=d['selectedSlice']+' + G06圆/椭圆',nativeCanonicalPixelCases=92,softwareTests=dict(passed=574,failed=0,skipped=0),complete=False)
changes={'S05-02':'既有79场景加椭圆13独立场景；27次有效编译/15canonical；r1失败及r2修正完整保留。','S05-03':'共享不可变单页计划、八类受限背景守卫；完整多页IR未恢复。','S05-04':'五类控制记录与八类静态背景；26个新动态类型未恢复。','S05-05':'92个独立类型/场景；椭圆27次正式全帧相等，不推广其它曲线。','S05-09':'最终574/0/0：核心340与完整编辑器234；先只读复核并反馈后修旧负例/入口边界并增精度测试。','S05-10':'椭圆873文件/628781756逻辑字节及180空目录定向清理；候选算法临时6918432B与生产mask6144000B分别归档清理，raw/失败帧/唯一源段保留。'}
for task in d['tasks']:
 if task['id'] in changes:task['scope']=changes[task['id']]
 if task['id']=='S05-05':task['state']='passed_eight_canonical_background_families'
d['ellipseSlice']=dict(nativeCompilations=27,preparationCompilations=1,failedDiagnostics=2,canonicalFixtures=15,independentCanonicalFrameCases=13,negativeTests=27,precisionGuardGroups=1,softwareTests=dict(passed=574,failed=0,skipped=0),cleanupFiles=873,cleanupLogicalBytes=628781756,report='docs/阶段5_椭圆定点细分背景编译与差分验收.md',B_S_M='docs/stage5-evidence/native/ellipse/B-S-M-ellipse-cell-area-r2.json',usbAccessed=False,E=False);write(p,d)
append('程序修改记录.md','''### 修改86：阶段5圆/椭圆定点细分与准确混色背景C
- 修改内容：
  1. 新增G06圆/椭圆黑白受限背景C；独立控制点26.6量化、自适应整数deCasteljau、8位子像素单元格积分，保持历史XY交叉项和512/4096/深度8预算。
  2. r1宽椭圆1像素及无线白填5像素差异定位到div255先移位后加128，修正后27次原厂完整帧相等；15canonical、27负向、共享精度守卫、十三对象混合和真实包复读，核心340+完整编辑器234共574/0/0。
  3. 只读复核反馈后将旧未知类型负例从ellipse换sector、直接绘制守卫对齐计划，并新增256精度与26.6区别/失败保留检查；最终回归通过。
  4. 同步UI详细规则/协议91/A03/A05/进度/台账；依据A03原文更正修改83仍误写的编号，G05为圆角矩形、G06为圆与椭圆；历史raw/repro与原始提交保留。
  5. 30raw gzip、405原厂B文件、8失败帧及100路径归档再验；定向清理873文件628781756逻辑字节和180空目录，候选/生产mask临时另分别清理6918432B和6144000B。
- 修改文件：CMakeLists.txt、src/device_ellipse_raster.h/.cpp、device_polygon_raster.h/.cpp、device_compile_plan.h/.cpp、independent_downloader.cpp、tests/stage5_compiler_tests.cpp、UI规则/椭圆专项及阶段5证据、当前编号相关报告/协议/计划/CSV、本记录。
- 影响范围：AMT单普通页第八个受限背景C；完整源/其它样式边界/多页配置及新产物T/E未开放，无USB通信。
- 原因：继续阶段5恢复设备曲线栅格与准确混色，并按阶段性文档、清理和独立提交要求形成可复验结果。
- 修改标记：//20261114-2 add/change''')
print(json.dumps(dict(passed=574,canonical=15,nativeCompilations=27,totalIndependentScenes=92,stage5Complete=False)))
