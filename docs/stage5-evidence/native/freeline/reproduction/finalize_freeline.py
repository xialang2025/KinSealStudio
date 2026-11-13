"""Freeze measured freeline admission, failed source binding, regression and cleanup without widening production logic."""
from pathlib import Path
from collections import Counter
import csv
import datetime
import gzip
import hashlib
import io
import json
import shutil
import subprocess

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence/native/freeline'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
core=read(work/'docs/stage5-evidence/freeline-windows-r1/run.json');editor=read(work/'docs/stage5-evidence/freeline-editor-windows-r1/run.json')
assert core['passed'] and sum(r['totals']['passed'] for r in core['runs'])==218
assert editor['passed'] and editor['runs'][0]['totals']==dict(passed=8,failed=0,skipped=0)
corpus=read(base/'B-S-M-bresenham-r2.json');canonical=[c for c in corpus['cases'] if 'reopen' in c['run'] or 'default-repeat' in c['run']]
assert len(corpus['cases'])==25 and len(canonical)==14 and all(c['M']['differentPixels']==0 for c in canonical)
assert len({tuple(tuple(p) for p in c['points']) for c in canonical})==12
excluded=Counter(c['classification'] for c in corpus['excludedInvalidTargetSamples']);assert sorted(excluded.values())==[2,28]
archives=read(base/'raw-archives.json')['archives'];assert len(archives)==56
compiles=sum(len(read(base/a['run']/'native-probe.json').get('nativeCompilations',[])) for a in archives);assert compiles==55
cleanup=read(base/'cleanup.json');assert cleanup['deletedFiles']==1666
mapping_summary=read(base/'freeline-screenedge-map-r6/native-probe.json')
mapping=json.loads(gzip.decompress((base/'freeline-screenedge-map-r6'/mapping_summary['rawEvidence']['file']).read_bytes()))
mapped=next(s['mappedScenePoints'] for s in mapping['steps'] if 'mappedScenePoints' in s)
assert mapped['points'][0][0]>0.67 and mapped['points'][0][0]<0.68 and mapped['points'][1][0]>200.67
write(base/'admission-scope.json',dict(containerCompilations=55,passiveMappingProcesses=1,rawProcessArchives=56,emptyTargetCompilationsExcluded=28,nonIdentityTransformCompilationsExcluded=2,admittedCompilations=25,canonicalFixtures=14,independentCanonicalGeometries=12,geometryBinding='two >Idd local points plus asserted identity native scene transform',nonidentityApiProof=dict(sourceSha256=mapping['input']['sha256'],mappedLocalPoints=mapped['points'],evidence='freeline-screenedge-map-r6/native-probe.json.gz'),sampledMinimumMajorSpan=40,minimumIsConservativeNotNativeGlobal=True,endpointMinXY=1,ignoredByteRanges=[],usbAccessed=False,E=False))
repro=base/'reproduction';repro.mkdir(exist_ok=False)
for name in ('native_probe.py','run_freeline_canonical.py','run_freeline_gesture.py','analyze_freeline_corpus.py','analyze_freeline_corpus-initial-strict.py','inspect_freeline_prefix.py','implement_freeline_c.py','line_raster_probe.cpp','archive_family.py','execute_family_cleanup.ps1','run_table_editor_checks.py','finalize_freeline.py'):
    shutil.copy2(root/name,repro/name)
for pattern in ('freeline-*.json','freeline-*.log'):
    for p in root.glob(pattern):shutil.copy2(p,repro/p.name)
shutil.copy2(root/'freeline-build.log',work/'docs/stage5-evidence/freeline-windows-r1/build.log')
patch=subprocess.check_output(['git','-C',str(work),'diff','--','src/device_compile_plan.h','src/device_compile_plan.cpp','src/independent_downloader.cpp','tests/stage5_compiler_tests.cpp'])
write(work/'docs/stage5-evidence/freeline-windows-r1/readonly-review.json',dict(date=datetime.datetime.now(datetime.timezone.utc).isoformat(),mode='read_only_no_business_logic_modified',patchSha256=hashlib.sha256(patch).hexdigest(),reviewed=['model validates exactly two freeline endpoints before shared raster access','integer and rounded-zero checks retained','freeline span40 and minXY1 are independent of line and orthogonal polyline','local native points require identity scene transform; mapToScene proves fractional rejected source','no sampled multi-point path is flattened','no new device record or scan demand','ten objects retain five dynamic records','source/legacy and atomic package gates remain independent','negative plan and payload outputs unchanged'],blockingFindings=[],remaining=['SM body and transformed native paths','smaller freeline sources and complete gesture threshold','unobserved styles and overlaps','remaining dynamic and multipage/configuration phase5 tasks','new product T/E stages6/10'],usbAccessed=False))
report='''# 阶段5：自由直线两端点背景编译与差分验收

第五个静态C切片G02已接入：既有五类控制记录+五类受限背景，整个阶段5仍未完成。这里的freeline仅是已证的两端点直线切片，不宣称恢复自由手绘、多点采样或平滑。严格纯离线，没有USB通信。

## 支持范围

AMT070W124-L800X480R、800×480普通单页；黑实线、宽1、Alpha255、无箭头端点0、两个不同的屏内整数端点，X/Y均至少1、主轴跨度至少40，不与其它对象重叠。跨度40是本批已证的保守支持范围，不是已证明的原厂/固件最小值。普通line的1像素及原点规则保持其既有独立范围，斜polyline仍拒绝。

生产只复用已经验证的整数主轴栅格写SheetPixel，没有新ControlPara、ControlPixel或PLC读取需求。完整SM、其它样式/Alpha/箭头/端帽、较小尺寸、边界变换、重叠和多点输入继续拒绝。编辑器原绘制没有改动，本报告证明C的保存重开背景目标。

## 原厂输入有效性与坐标绑定

[支持范围清单](stage5-evidence/native/freeline/admission-scope.json)和[最终B/S/M](stage5-evidence/native/freeline/B-S-M-bresenham-r2.json)共核55次原厂容器编译及1次被动坐标映射。27次源实际有两个端点；其中2次带小数场景变换，不进入本切片。最终25次支持输入对应14份canonical夹具、12个独立场景：默认、偏移、斜率0.4、反向、水平、垂直、陡线、反斜率、40像素对角/水平/垂直及(1,1)近屏边。每幅768000B RGB565全部逐字节相等，没有ROI/像素忽略区，也没有把原厂图静态拷入产品。

默认源SHA为`20b0d323f46eca9cfebaa371d96274c85d121b5b6953476d362d575f8c463a8d`，重复3次新进程。源type65576/count2及两条大端`>Idd`只是局部路径前缀；每个支持样本额外断言原厂sceneTransform为单位矩阵，才能把路径点绑定到typed KSP绝对几何。原件/私有副本仍分别为`305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c`、`38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89`，安装原件未改。

所有支持产物只有2400B PicLenPara，没有控制/动态图记录。重复同源的Sys/Alarm/Usbload/load/upLoad非确定字节逐文件保留为未知，没有取得忽略资格，Q-S05-004未整项关闭。

## 空路径与初版200像素差异

首次单move拖动的22个新建/重开变体虽然容器编译成功，实际path count=0。严格两点解析断言先发现此问题；没有算目标成功。改用已有成功记录中的六点连续拖动，正常尺寸恢复两端点并经过保存重开。1像素和2像素的对角/轴向仍为空路径，累计28份空路径编译均明确排除并保留。频数、阈值和时序的更细原因没有伪称完全定位；小尺寸继续拒绝。

初版[B/S/M r1](stage5-evidence/native/freeline/B-S-M-bresenham-r1.json)把屏边源局部点(0,0)→(200,100)当成绝对点，出现200像素差异。原厂现场sceneTransform含约0.67082平移；另进程通过真实Qt `QGraphicsItem::mapToScene(QPointF)`被动核验，实际点为(0.670820393249938,0.670820393249937)→(200.67082039324993,100.67082039324994)。因此初版比较没有绑定同一输入，不能归因于Qt栅格算法或“削掉差异像素”。原始200差异完整保留；这两个带变换输入按不支持条件排除，产品也拒绝贴上/左边界的freeline。(1,1)近边样例则身份变换且整帧相等。

## 验证、只读复核、清理

[核心回归](stage5-evidence/freeline-windows-r1/run.json)218/0/0：阶段5编译151、后端14、扫描37、纯计划6、真实SDK宏10；[编辑器8项](stage5-evidence/freeline-editor-windows-r1/run.json)另计，合计226/0/0。包含14份真实原厂整帧、20个负向、源/旧单文本拒绝与10段包发布复读；十对象混合只有5个640B动态控制记录，12段结构闭合。负向包括未核样式、非法/小数/取整重合/多点端点、跨度39、上/左边界、越界与重叠，失败不改调用方输出。

[只读复核](stage5-evidence/freeline-windows-r1/readonly-review.json)没有阻塞发现，检查范围和残留项已列明；复核时不改业务逻辑。历史253/107/143/161/191分别保留原身份，不相加冒充本轮。

56份原始probe及checkpoint已gzip回解核SHA，保存唯一源、段、错误绑定/空路径、端点断言及复现脚本。[清理回执](stage5-evidence/native/freeline/cleanup.json)删除1666个自有运行副本/解码中间文件，共1173592830逻辑字节；含硬链接，不等于物理释放。构建仍用于后续切片，用户build/dist、SDK及原厂安装均保留。TEMP/TMP/TMPDIR持续指向本任务Z盘目录。

剩余图形/样式、26个新动态类、多页/窗口/活动配置及原厂容器继续按阶段5推进；新产物T交阶段6、E交阶段10。deviceAcknowledged=false，历史原厂下载及人工显示确认不转作本轮新产物通过。
'''
(work/'docs/阶段5_自由直线两端点背景编译与差分验收.md').write_text(report,encoding='utf-8')
summary='G02自由直线两端点受限背景C已接入：黑实线/宽1/Alpha255、无箭头端点0、屏内整数且X/Y至少1、主轴跨度至少40、不重叠。55次容器编译中25次符合本切片，14份canonical/12独立场景整帧相等；28份空路径及2份小数场景变换明确排除，失败输入和初版错误绑定保留。本批226/0/0，SM/多点/未知样式/小尺寸及新产物T/E未开放。'
def append(relative,text):
    path=work/relative;path.write_text(path.read_text(encoding='utf-8-sig').rstrip()+'\n\n'+text+'\n',encoding='utf-8')
append('docs/阶段5逐项进度.md','## 第五批：自由直线的两端点切片\n\n'+summary+'\n\n[专项报告](阶段5_自由直线两端点背景编译与差分验收.md)。当前5类记录+5类受限静态背景；S05-07及其余未恢复项继续推进，不将10个受限类型算作完整阶段5。')
progress_path=work/'docs/阶段5逐项进度.md';text=progress_path.read_text(encoding='utf-8');text=text.replace('本阶段共10项。本批为G04受限静态背景和安全离线包闭环；阶段5整体仍未完成，下一批继续静态表格/其它已证映射。','本阶段共10项主要任务。当前已交付五类控制记录及五类受限静态背景切片；完整多页/配置、其余类型及样式仍未完成。下面首批表保留当时验收，各批最新结果见后文。');progress_path.write_text(text,encoding='utf-8')
progress=read(work/'docs/stage5-evidence/progress.json');progress.update(active='完成G02便携交付，继续其它图形像素、新动态记录及多页配置最小探针',selectedSlice='既有五类 + G04正交折线 + G10静态表格 + G08矩形 + G01普通直线 + G02两端点自由直线',softwareTests=dict(passed=226,failed=0,skipped=0),nativeCanonicalPixelCases=50)
scopes={'S05-02':'G04六组、G10十二组、G08八组、G01十二组、G02十二组；G02另保留28空路径和2场景变换拒绝样本，非确定区无忽略掩码。','S05-03':'共享不可变单页计划、五类静态图形守卫；完整多页IR未恢复。','S05-04':'五类控制记录和五类静态背景；26个新动态类型未恢复。','S05-05':'50个独立类型/场景整帧对照；G02十四份canonical实际编译全等，不扩大为全部样式/变换/重叠。','S05-09':'最新226/0/0，核心218与自由直线编辑器8；历史回归分别保留。','S05-10':'本批56原始进程记录归档回解核SHA，1666文件/1173592830逻辑字节定向清理；构建供后续保留。'}
for task in progress['tasks']:
    if task['id'] in scopes:task['scope']=scopes[task['id']]
progress['freelineSlice']=dict(containerCompilations=55,passiveMappingProcesses=1,admittedCompilations=25,excludedEmptyPaths=28,excludedNonidentityTransforms=2,canonicalFixtures=14,independentCanonicalFrameCases=12,softwareTests=dict(passed=226,failed=0,skipped=0),cleanupFiles=1666,cleanupLogicalBytes=1173592830,sampledMinimumMajorSpan=40,report='docs/阶段5_自由直线两端点背景编译与差分验收.md',usbAccessed=False,E=False)
for id in ('Q-S04-G02-001','Q-S04-G02-002','Q-S04-G02-003'):
    if id not in progress['unresolved']:progress['unresolved'].append(id)
write(work/'docs/stage5-evidence/progress.json',progress)
append('docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','## 88. 阶段5两端点自由直线受限背景（2026-11-13）\n\n'+summary+'\n\n仅复用SheetPixel背景，不新增控制/动态图/扫描记录，没有USB通信。deviceAcknowledged=false；历史原厂基线T及人工显示不转作新产物通过。局部前缀与实际场景变换分开核验，详见[专项报告](阶段5_自由直线两端点背景编译与差分验收.md)。')
append('design_plan/06_阶段5_设备编译与格式差分.md','## 11. 第五批两端点自由直线实际执行\n\n'+summary+'\n\n[专项报告](../docs/阶段5_自由直线两端点背景编译与差分验收.md)。整个阶段5仍未完成。')
append('design_plan/A05_源格式与设备编译契约.md','## 12. 阶段5自由直线前缀与场景绑定（2026-11-13）\n\n'+summary+'\n\n源type65576的两条`>Idd`是局部路径，只有已断言单位sceneTransform的样本可直接绑定绝对KSP端点。屏边约0.67082平移由真实Qt mapToScene另进程核实；不能把局部点当屏幕点，再将200差异算作算法错误。完整body/变换和小尺寸尚未恢复。见[报告](../docs/阶段5_自由直线两端点背景编译与差分验收.md)。')
append('docs/自由直线首尾切片实现与验收.md','## 阶段5独立C补验（2026-11-13）\n\n'+summary+'\n\nQ-S04-G02-001仅受限背景C分支恢复，完整SM继续阻塞；002多点/样式和003新产物T/E未闭合。[专项报告](阶段5_自由直线两端点背景编译与差分验收.md)。')
matrix=work/'design_plan/控件文档与实施覆盖矩阵.md';lines=matrix.read_text(encoding='utf-8-sig').splitlines()
for i,line in enumerate(lines):
    if line.startswith('| 自由直线 |'):
        lines[i]=line.replace('[本轮报告](../docs/自由直线首尾切片实现与验收.md)','[本轮报告](../docs/自由直线首尾切片实现与验收.md) [阶段5C](../docs/阶段5_自由直线两端点背景编译与差分验收.md)').replace('| 阻塞 | 原厂基线下载通过；独立编码阻塞 |','| 两端点整数黑实线背景C切片 | 原厂基线已验；新产物T待阶段6 |').replace('接续专项报告未决项','SM/小尺寸/变换/其它样式及新产物T/E待验')
matrix.write_text('\n'.join(lines)+'\n',encoding='utf-8')
append('程序修改记录.md','''### 修改79：阶段5自由直线两端点受限背景编译
- 修改内容：
  1. 新增staticFreelineCount及两端点整数背景复用，保守限制主轴至少40、X/Y至少1；普通line和正交polyline保持各自范围。
  2. 55次原厂容器编译中28空路径、2非单位变换明确拒绝；单位变换25次/14canonical整帧相等，真实Qt mapToScene纠正初版200像素错误绑定。
  3. 十对象混合、20负向和包复读，218核心+8编辑器共226/0/0；只读复核、协议同步、56原始观测归档及1666中间文件定向清理。
- 修改文件：src/device_compile_plan.h/.cpp、src/independent_downloader.cpp、tests/stage5_compiler_tests.cpp、阶段5报告/进度/协议/格式契约/覆盖矩阵与证据、本记录。
- 影响范围：AMT070W124-L800X480R单页第五个静态C切片；不开放SM、多点采样、其它样式、多页/配置或新产物T/E，没有USB通信。
- 原因：继续阶段5独立编译与格式差分，必须核实际目标对象和场景坐标，不能把容器编译成功或错误输入绑定当作功能通过。
- 修改标记：//20261113-15 add/change''')
table=work/'design_plan/复刻待验线索台账.csv';buffer=io.StringIO(newline='');writer=csv.writer(buffer,lineterminator='\n');writer.writerow(['Q-S05-G02-C','阶段5自由直线','design_plan/06_阶段5_设备编译与格式差分.md','1',sha(work/'design_plan/06_阶段5_设备编译与格式差分.md'),'两端点单位变换/跨度至少40/X-Y至少1的黑实线C切片通过，SM/小尺寸/场景变换/其它样式待恢复','编译切片','受限C通过，整体未闭合','docs/阶段5_自由直线两端点背景编译与差分验收.md','先核有效路径和sceneTransform；新产物T/E按阶段6/10验证']);text=table.read_text(encoding='utf-8-sig');assert 'Q-S05-G02-C,' not in text;table.write_text(text.rstrip()+'\n'+buffer.getvalue(),encoding='utf-8')
print(json.dumps(dict(passed=226,containerCompilations=55,admittedCompilations=25,canonicalFixtures=14,independentCases=12,cleanupFiles=1666,completeStage5=False)))
