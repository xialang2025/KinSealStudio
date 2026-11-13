"""Publish only measured line-slice evidence and retain the bounded rejection list."""
from pathlib import Path
import csv
import datetime
import hashlib
import io
import json
import shutil
import subprocess

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
base=work/'docs/stage5-evidence/native/line'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
core=read(work/'docs/stage5-evidence/line-windows-r1/run.json')
editor=read(work/'docs/stage5-evidence/line-editor-windows-r1/run.json')
assert core['passed'] and editor['passed']
assert sum(r['totals']['passed'] for r in core['runs'])==183
assert editor['runs'][0]['totals']==dict(passed=8,failed=0,skipped=0)
corpus=read(base/'B-S-M-bresenham-expanded.json')
canonical=[c for c in corpus['cases'] if 'reopen' in c['run'] or 'default-repeat' in c['run']]
assert len(canonical)==14 and all(c['M']['differentPixels']==0 for c in canonical)
assert len({tuple(tuple(p) for p in c['points']) for c in canonical})==12
cleanup=read(base/'cleanup.json');audit_cleanup=read(base.parent/'audit/cleanup.json')
assert cleanup['deletedFiles']==750 and audit_cleanup['deletedFiles']==180
assert len(read(base/'raw-archives.json')['archives'])==25
repro=base/'reproduction';repro.mkdir(exist_ok=False)
for name in ('native_probe.py','run_line_canonical.py','run_line_additional.py','analyze_line_corpus.py','line_raster_probe.cpp','archive_family.py','execute_family_cleanup.ps1','run_table_editor_checks.py'):
    shutil.copy2(root/name,repro/name)
for pattern in ('line-*.json','line-*.log'):
    for path in root.glob(pattern):shutil.copy2(path,repro/path.name)
for name in ('run_graphic_candidates.py','graphic_candidate_probe.cpp','analyze_graphic_candidates.py','check_freeline_integer.py'):
    destination=base.parent/'audit/reproduction'/name
    destination.parent.mkdir(exist_ok=True)
    assert not destination.exists()
    shutil.copy2(root/name,destination)
shutil.copy2(root/'line-build.log',work/'docs/stage5-evidence/line-windows-r1/build.log')
patch=subprocess.check_output(['git','-C',str(work),'diff','--','src/device_compile_plan.h','src/device_compile_plan.cpp','src/independent_downloader.cpp','tests/stage5_compiler_tests.cpp'])
write(work/'docs/stage5-evidence/line-windows-r1/readonly-review.json',dict(date=datetime.datetime.now(datetime.timezone.utc).isoformat(),mode='read_only_no_business_logic_modified',patchSha256=hashlib.sha256(patch).hexdigest(),reviewed=['typed two-point validation before raster access','integer tolerance and rounded-zero rejection','major-axis endpoint sorting preserves minor coordinates','strict-negative error tie and excluded last endpoint','native empty 1x1 diagonal and one-pixel axes','QPainter pixel fill avoids modifying an active QImage','oblique straight line does not relax polyline guard','no new dynamic records or scan allocations','source and device eligibility remain independent','failed plan/payload unchanged','nine-object mixed package retains five 640-byte records'],blockingFindings=[],remaining=['SM body and source interoperability','unobserved line styles and overlap','whole phase multi-page/configuration and 26 dynamic classes','new product T/E deferred to stages6/10'],usbAccessed=False))

report='''# 阶段5：普通直线独立背景编译与差分验收

第四个静态C切片G01已接入。当前为既有五类控制记录加四类受限背景，阶段5共10项任务仍按切片推进，整个阶段尚未完成。本批严格纯离线，没有USB通信或实物下载。

## 本次开放范围

AMT070W124-L800X480R、800×480普通单页；黑实线、宽1、Alpha255、无箭头、端点0、两端点均为屏内整数像素，取整后不重合，无对象重叠。有序端点支持反向、水平、垂直及倾斜。只写SheetPixel，不增加ControlPara、ControlPixel或PLC读取需求。

整数栅格按主轴递增排序，不画最后主轴端点；误差严格小于0才进副轴。保存重开原厂的1×1对角线不着色，1像素水平/垂直线各着色1像素。这些是原厂已观测行为，不修成Qt6预览的端点规则。现有编辑器绘制没有改动；本切片只闭合C的保存重开目标。斜折线仍拒绝，未开放样式/透明/重叠及完整SM仍拒绝。

## 有效原厂语料与失败链

[B/S/M完整对照](stage5-evidence/native/line/B-S-M-bresenham-expanded.json)记录25次有效编译：同一默认源3次新进程，11个几何变体分别创建编译及另进程重开。14份canonical对应12个独立几何场景：默认、偏移、斜率0.4、1×1对角、屏边、反向、水平、垂直、陡线、反斜率、1像素水平与垂直。

每份实际源核SHA后读取已确证的type65573、count=2及两条大端`>Idd`记录（tag0起点、tag1终点），把坐标绑定到typed KSP；这仅恢复两点前缀，没有假定完整SM body。水平/垂直使用最小1像素carrier，对应归一轴分量为0。默认源SHA为`eb70868aae07460009ad12aba3dc2e0da3df56ed61d0a7772c04c8c8da4f215c`；原件/私有运行副本身份仍分别为`305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c`、`38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89`。安装原件未修改。

Qt6 FlatCap默认场景偶然相等，但斜率0.4差200像素、1×1对角差1像素；cosmetic、宽0、整数API和半像素平移均未整体闭合。失败JSON完整保留。新建缓存与保存重开端帽差异也保留，不能把创建态全称为相等。最终候选算法14份canonical全768000B逐字节相等，生产编译器又对这14份夹具全部真实重验，无ROI、忽略掩码或静态图片拷贝。

各产物均为2400B PicLenPara、无动态段/控制记录。3次同源Sys/Alarm/Usbload/load/upLoad变化按文件记录为未知；Q-S05-004未关闭，未据此设忽略区。

## 实际验证与只读复核

[核心回归](stage5-evidence/line-windows-r1/run.json)183/0/0：阶段5编译116、既有后端14、扫描37、纯计划6、真实SDK宏10。[直线编辑器](stage5-evidence/line-editor-windows-r1/run.json)8/0/0，合计191/0/0。历史253/107/143/161保留原身份，没有相加冒充本轮。

新增14份原厂整帧、14个负向、10段包发布及复读；九对象混合仍只有5个640B控制记录，12段结构与既有索引校验通过。负向包含样式、粗线、颜色、Alpha、箭头/端帽、小数端点、屏界、零长/取整后零长、重叠及斜折线；失败不修改计划和payload。

[只读复核](stage5-evidence/line-windows-r1/readonly-review.json)检查排序/误差/边界、活动QPainter写法、结构索引和源/设备独立门槛，未发现阻塞问题；不在复核阶段改业务逻辑。

## 其余图形初探与清理

另6次保存重开候选的[修正输入对照](stage5-evidence/native/audit/candidate-comparison-r2.json)与[r3](stage5-evidence/native/audit/candidate-comparison-r3.json)显示椭圆、圆角、多边形、扇形仍有像素差异。初版polygon/sector/freeline输入绑定不完整，保留为初探，不能当同模型正式结论。圆角相对半径候选更差，未采用。

自由直线既有两点前缀绑定后，整数算法在单默认页为0差异，见[候选](stage5-evidence/native/audit/freeline-integer-candidate.json)。这只是下一切片线索，尚未开放freeline生产编译，必须补重复与变体。

25份直线及6份初探的原始probe/checkpoint已gzip回解核SHA；唯一源/段、失败链和复现脚本保留。清理[直线750文件/523976833逻辑字节](stage5-evidence/native/line/cleanup.json)与[初探180文件/125761933逻辑字节](stage5-evidence/native/audit/cleanup.json)，逻辑量含硬链接，不等于物理释放。后续仍需构建缓存、用户build/dist、SDK和原厂安装均保留。C盘约12.0GiB、Z盘约16.2GiB空闲（清理回执时）。

阶段5多页/窗口/配置、26个新动态类型、其余图形/样式和原厂容器仍待恢复；新产物T交阶段6、E交阶段10。deviceAcknowledged=false，历史下载回执与用户显示确认没有转作本轮新产物通过。
'''
(work/'docs/阶段5_普通直线独立背景编译与差分验收.md').write_text(report,encoding='utf-8')
summary='G01普通直线已增加受限独立背景C：黑实线/宽1/Alpha255、无箭头端点0、屏内整数两端点且不重叠；25次有效原厂编译、12个独立场景的14份保存重开夹具整帧相等，本批191/0/0。整数主轴算法保留原厂端点和1×1对角行为，Qt6失败候选保留。SM/未知样式/斜折线与新产物T/E未开放。'
append=lambda p,t:p.write_text(p.read_text(encoding='utf-8-sig').rstrip()+'\n\n'+t+'\n',encoding='utf-8')
append(work/'docs/阶段5逐项进度.md','## 第四批：普通直线\n\n'+summary+'\n\n[专项报告](阶段5_普通直线独立背景编译与差分验收.md)。当前5类控制记录+4类静态背景；S05-07及其它未恢复范围继续推进。下一探针为两点自由直线的重复/变体，不以单默认0差异直接放开。')
progress=read(work/'docs/stage5-evidence/progress.json')
progress.update(active='完成G01提交/便携交付后，继续G02两点自由直线重复与变体探针',selectedSlice='既有五类 + G04正交折线 + G10静态表格 + G08矩形 + G01普通直线',softwareTests=dict(passed=191,failed=0,skipped=0),nativeCanonicalPixelCases=38)
scopes={'S05-02':'G04六组、G10十二组、G08八组、G01十二个独立场景/十四份canonical夹具；非确定区未知且无忽略掩码。','S05-03':'共享不可变单页计划、四类静态图形守卫；完整多页IR未恢复。','S05-04':'复用五类控制记录，四类图形只烘背景；26个新动态类型未恢复。','S05-05':'38个独立原厂整帧场景；本批直线14份夹具实际编译全相等，未扩大为全部样式/重叠。','S05-09':'最新191/0/0，核心183与直线编辑器8；历史基线及回归分别保留。','S05-10':'本批直线750文件/523976833逻辑字节，初探180文件/125761933逻辑字节；构建保留供后续。'}
for task in progress['tasks']:
    if task['id'] in scopes:task['scope']=scopes[task['id']]
progress['lineSlice']=dict(nativeCompilations=25,independentCanonicalFrameCases=12,canonicalFixtures=14,softwareTests=dict(passed=191,failed=0,skipped=0),cleanupFiles=750,cleanupLogicalBytes=523976833,report='docs/阶段5_普通直线独立背景编译与差分验收.md',usbAccessed=False,E=False)
for id in ('Q-S04-G01-001','Q-S04-G01-002','Q-S04-G01-003'):
    if id not in progress['unresolved']:progress['unresolved'].append(id)
write(work/'docs/stage5-evidence/progress.json',progress)
append(work/'docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','## 87. 阶段5普通直线整数背景编译（2026-11-13）\n\n'+summary+'\n\n只烘SheetPixel，不新增控制/动态图/扫描；10/12/14段与纯计划已核。新产物deviceAcknowledged=false，未访问USB；历史原厂基线T和人工显示确认不转作新产物通过。见[专项报告](阶段5_普通直线独立背景编译与差分验收.md)。')
append(work/'design_plan/06_阶段5_设备编译与格式差分.md','## 10. 第四批普通直线实际执行\n\n'+summary+'\n\n[专项报告](../docs/阶段5_普通直线独立背景编译与差分验收.md)。整个阶段5仍未完成，继续类型/配置及多页切片。')
append(work/'design_plan/A05_源格式与设备编译契约.md','## 11. 阶段5普通直线背景补充（2026-11-13）\n\n'+summary+'\n\n源type65573/count2及两条`>Idd`只绑定已证端点，不作为设备记录。整数主轴栅格写背景，14份实际夹具768000B完全相等；SM body及非确定系统/报警/容器区未知。见[报告](../docs/阶段5_普通直线独立背景编译与差分验收.md)。')
append(work/'docs/直线图形实现与验收.md','## 阶段5独立C补验（2026-11-13）\n\n'+summary+'\n\nQ-S04-G01-001仅受限C分支已恢复，完整SM仍阻塞；002其它样式/几何及003新产物T/E保持待验。[阶段5报告](阶段5_普通直线独立背景编译与差分验收.md)。')
matrix=work/'design_plan/控件文档与实施覆盖矩阵.md'
text=matrix.read_text(encoding='utf-8-sig');lines=text.splitlines()
for i,line in enumerate(lines):
    if line.startswith('| 直线 |'):
        lines[i]=line.replace('[本轮报告](../docs/直线图形实现与验收.md)','[本轮报告](../docs/直线图形实现与验收.md) [阶段5C](../docs/阶段5_普通直线独立背景编译与差分验收.md)').replace('| 阻塞 | 原厂基线下载通过；独立编码阻塞 | 待现场 | 接续专项报告未决项 |','| 整数黑实线背景C切片 | 原厂基线已验；新产物T待阶段6 | 待现场 | SM/其它样式/重叠及新产物T/E待验 |')
matrix.write_text('\n'.join(lines)+'\n',encoding='utf-8')
append(work/'程序修改记录.md','''### 修改77：阶段5普通直线独立整数背景编译
- 修改内容：
  1. 新增G01受限背景C和staticLineCount，整数主轴算法匹配原厂误差平局、端点排序与1×1对角行为；斜折线仍拒绝。
  2. 原厂25次有效编译、12独立场景/14份canonical夹具全帧相等；14负向、九对象混合与实际包复读，核心183+编辑器8共191/0/0。
  3. 保留Qt6失败和其它图形候选输入修正链；只读复核、协议同步、原始gzip及定向清理930个自有中间文件。
- 修改文件：src/device_compile_plan.h/.cpp、src/independent_downloader.cpp、tests/stage5_compiler_tests.cpp、阶段5报告/进度/协议/格式契约/覆盖矩阵和证据、本记录。
- 影响范围：AMT070W124-L800X480R单页的第四个静态C切片；未开放SM/其它样式/多页配置或新产物T/E，没有USB通信。
- 原因：继续用户授权的阶段5设备编译与格式差分，不能以Qt6默认线偶然相等替代边界对照。
- 修改标记：//20261113-14 add/change''')
table=work/'design_plan/复刻待验线索台账.csv'
buffer=io.StringIO(newline='');writer=csv.writer(buffer,lineterminator='\n')
writer.writerow(['Q-S05-G01-C','阶段5普通直线','design_plan/06_阶段5_设备编译与格式差分.md','1',sha(work/'design_plan/06_阶段5_设备编译与格式差分.md'),'两点整数黑实线受限C已恢复，完整SM与未核样式/重叠和T/E待验','编译切片','受限C通过，整体未闭合','docs/阶段5_普通直线独立背景编译与差分验收.md','按新产物在阶段6/10验证T/E，继续其它字段探针'])
text=table.read_text(encoding='utf-8-sig');assert 'Q-S05-G01-C,' not in text;table.write_text(text.rstrip()+'\n'+buffer.getvalue(),encoding='utf-8')
print(json.dumps(dict(passed=191,nativeCompilations=25,canonicalFixtures=14,independentCases=12,cleanupFiles=930,completeStage5=False),ensure_ascii=False))
