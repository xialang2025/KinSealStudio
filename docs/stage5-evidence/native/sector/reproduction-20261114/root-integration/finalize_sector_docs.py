"""Publish only actual G07 compiler, test and cleanup evidence into the active contract."""
from pathlib import Path
import csv
import hashlib
import json

root = Path(__file__).resolve().parent
work = root.parent / 'kinseal-stage4-20261112/worktree'
read = lambda p: json.loads(p.read_text(encoding='utf-8-sig'))
write = lambda p, d: p.write_text(json.dumps(d, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()

def append(relative, text):
    p = work / relative
    p.write_text(p.read_text(encoding='utf-8-sig').rstrip() + '\n\n' + text.strip() + '\n', encoding='utf-8')

base = work / 'docs/stage5-evidence/native/sector'
bsm = read(base / 'B-S-M-sector-cell-area-r1.json')
assert bsm['allRasterComparisonsEqual'] and len(bsm['cases']) == 27
core = read(work / 'docs/stage5-evidence/sector-windows-r1/run.json')
editor = read(work / 'docs/stage5-evidence/sector-editor-windows-r2/run.json')
assert core['passed'] and editor['passed']
core_count = sum(r['totals']['passed'] for r in core['runs'])
editor_count = sum(r['totals']['passed'] for r in editor['runs'])
assert all(r['totals']['failed'] == r['totals']['skipped'] == 0 for d in (core, editor) for r in d['runs'])
cleanup = read(base / 'cleanup-sector-20261114.json')
review = read(work / 'docs/stage5-evidence/sector-windows-r1/readonly-review.json')
assert review['finalBlockingFindings'] == 0
assert '### 修改88：' not in (work / '程序修改记录.md').read_text(encoding='utf-8')
passed = core_count + editor_count
summary = (f'G07扇形已接入第九个受限静态背景C：固定45°起角/90°扫角，X/Y至少2、正方形边长100..240、右端≤698/下端≤378；'
           f'黑边/黑前景/白背景、宽1/Alpha255、线填0/1及端点0，拒绝双方补片相交或接触。'
           f'27次有效原厂编译全帧相等，15份canonical/13独立场景；最终核心{core_count}+完整编辑器{editor_count}共{passed}/0/0。'
           '完整角度/SM、其它样式边界、多页活动配置及新产物T/E仍未完成，本批没有USB通信。')
append('docs/阶段5逐项进度.md', '## 第九批：G07扇形默认角度背景\n\n' + summary + '\n\n[专项报告](阶段5_扇形默认角度背景编译与差分验收.md) · [详细UI规则](UI编译规则与阶段交付规范.md)。10项阶段主任务尚未全部闭合，九个背景家族不能计为九项主任务整体完成。')
append('design_plan/06_阶段5_设备编译与格式差分.md', '## 15. 第九批扇形默认角度实际执行\n\n' + summary + '\n\n原始local8元素及painter整数平移分别保留，显式映射后独立重建；填充隐式闭合、描边不补第九边。默认旧D函数的1像素失败及奇数框浮点绑定失败链均保留。[专项报告](../docs/阶段5_扇形默认角度背景编译与差分验收.md)及[UI规则](../docs/UI编译规则与阶段交付规范.md)。')
append('docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md', '## 92. 阶段5扇形默认角度背景编译（2026-11-14）\n\n' + summary + '\n\n只写SheetPixel，PicLenPara2400B，无新增控制记录/动态图/扫描或宏。十四对象混合仍5条640B活动控制记录；静态10段及12/14段包结构独立复读。先局部moveTo(center)/arcTo(45,90)，保留开放8元素再按bbox整数平移；26.6控制点、受限历史细分、Exact8Bit积分、RGB32正确D混色后转LE565。[正式B/S/M](stage5-evidence/native/sector/B-S-M-sector-cell-area-r1.json)，不把原厂基线下载或旧ACK计为本次T/E。')
append('design_plan/A05_源格式与设备编译契约.md', '## 16. G07扇形默认角度受限C（2026-11-14）\n\n' + summary + '\n\n原厂local path与painter transform分开保留，仅线性identity且平移等bbox的输入符合本切片；scene bbox浮点回算容差显式≤1e-8，raw保留，像素/路径量化/源SHA没有容差。严格graphic七键，不增加angles、points、radius或端点JSON字段。[B/S/M](../docs/stage5-evidence/native/sector/B-S-M-sector-cell-area-r1.json)。')
append('docs/阶段4逐控件进度.md', '## G07后续设备C更新（阶段5，2026-11-14）\n\n' + summary + '\n\n阶段4保留的原厂传输身份不转作新编译器T/E。[专项报告](阶段5_扇形默认角度背景编译与差分验收.md)。')
append('design_plan/00_复刻执行总入口.md', '## 阶段5第九批进展（2026-11-14）\n\n' + summary + '\n\n当前五类控制记录加九类受限静态背景。继续真实曲率、26新动态类型、多页及活动配置；阶段5整体未完成。')
p = work / 'design_plan/A03_基础图形与表格规范.md'
t = p.read_text(encoding='utf-8-sig')
lines = t.splitlines()
for i, line in enumerate(lines):
    if line.startswith('| G07 |'):
        fields = line.split('|')
        fields[3] = ' 默认角度编辑/KSP与受限背景C；完整SM/角度/T/E待验 '
        fields[4] = fields[4].rstrip() + ' [阶段5C](../docs/阶段5_扇形默认角度背景编译与差分验收.md) '
        lines[i] = '|'.join(fields)
p.write_text('\n'.join(lines) + '\n', encoding='utf-8')
p = work / 'design_plan/控件文档与实施覆盖矩阵.md'
lines = p.read_text(encoding='utf-8-sig').splitlines()
for i, line in enumerate(lines):
    if line.startswith('| 扇形 |'):
        lines[i] = line.replace('[本轮报告](../docs/扇形默认角度切片实现与验收.md)', '[本轮报告](../docs/扇形默认角度切片实现与验收.md) [阶段5C](../docs/阶段5_扇形默认角度背景编译与差分验收.md)').replace('| 阻塞 |', '| 固定默认角度/黑白宽1背景C |').replace('独立编码阻塞', '新编码受限C通过；新产物T/E待验')
p.write_text('\n'.join(lines) + '\n', encoding='utf-8')
with (work / 'design_plan/复刻待验线索台账.csv').open('a', encoding='utf-8', newline='') as f:
    csv.writer(f, lineterminator='\n').writerow(['Q-S05-G07-C', '阶段5扇形', 'design_plan/06_阶段5_设备编译与格式差分.md', 1, sha(work / 'design_plan/06_阶段5_设备编译与格式差分.md'), '默认45/90正方形与已采样黑白宽1线填0或1背景C通过；完整角度/SM和新产物T-E待验', '编译切片', '受限C通过，整体未闭合', 'docs/阶段5_扇形默认角度背景编译与差分验收.md', '保留舍入及奇数框绑定失败链；继续真实曲率与动态类型'])
p = work / 'docs/stage5-evidence/progress.json'
progress = read(p)
assert progress['nativeCanonicalPixelCases'] == 92
progress.update(active='G07扇形默认角度受限C完成；准备第九批便携，继续其它未决', selectedSlice=progress['selectedSlice'] + ' + G07扇形默认角度', nativeCanonicalPixelCases=105, softwareTests=dict(passed=passed, failed=0, skipped=0), complete=False)
changes = {'S05-02': '既有92场景加扇形13独立场景；27次有效编译/15canonical；local路径/平移及失败链保留。', 'S05-03': '不可变单页计划、九类受限背景守卫；完整多页IR未恢复。', 'S05-04': '五类控制记录与九类受限静态背景；26新动态类型未恢复。', 'S05-05': '105独立类型/场景；扇形27次正式全帧相等，不能推广可变角度或真实曲率。', 'S05-09': f'最终{passed}/0/0：核心{core_count}+完整编辑器{editor_count}，含新增扇形完整帧、30负向、包门禁和十四对象混合；独立只读复核无剩余阻断。', 'S05-10': f'扇形{cleanup["deletedFiles"]}文件/{cleanup["deletedLogicalBytes"]}逻辑字节定向清理，原始探针gzip回解及唯一源段/失败证据保留；build/cache保留。'}
for task in progress['tasks']:
    if task['id'] in changes:
        task['scope'] = changes[task['id']]
    if task['id'] == 'S05-05':
        task['state'] = 'passed_nine_canonical_background_families'
progress['sectorSlice'] = dict(nativeCompilations=27, canonicalFixtures=15, independentCanonicalFrameCases=13, negativeTests=30, softwareTests=dict(passed=passed, failed=0, skipped=0), cleanupFiles=cleanup['deletedFiles'], cleanupLogicalBytes=cleanup['deletedLogicalBytes'], report='docs/阶段5_扇形默认角度背景编译与差分验收.md', B_S_M='docs/stage5-evidence/native/sector/B-S-M-sector-cell-area-r1.json', usbAccessed=False, E=False)
write(p, progress)
append('程序修改记录.md', f'''### 修改88：阶段5扇形默认角度背景C与局部路径平移
- 修改内容：
  1. 新增G07扇形固定45/90默认角度受限背景C，保留局部8元素开放路径加bbox整数平移；填充隐式闭合，描边不补第九边。
  2. 与椭圆共享独立定点细分/Exact8Bit面积积分/正确D混色和隔离RGB32补片；椭圆绝对路径与既有边界继续保持，生产不加载原厂绘图库或读原厂像素。
  3. 正式27次全帧相等、15canonical/13独立场景，30条负向/静态包/十四对象混合，核心{core_count}+完整编辑器{editor_count}共{passed}/0/0；先只读复核并记录发现后处理。
  4. 同步UI规则17/扇形专项/协议92/A05/进度/台账；原始local/transform及奇数框浮点绑定失败、默认旧D失败链完整保留。
  5. 定向清理{cleanup['deletedFiles']}文件/{cleanup['deletedLogicalBytes']}逻辑字节，自有原厂进程已结束，raw gzip回解、唯一源段/失败帧/许可证保留，用户工程与增量build不清扫。
- 修改文件：CMakeLists.txt、src/device_sector_raster.h/.cpp、device_ellipse_raster.h/.cpp、device_compile_plan.h/.cpp、independent_downloader.cpp、tests/stage5_compiler_tests.cpp、UI规则/扇形专项及证据、协议/计划/CSV和本记录。
- 影响范围：AMT单普通页第九类受限背景C；完整角度/SM、其它边界、多页配置及新产物T/E仍未开放，无USB通信。
- 原因：继续阶段5并按每控件完成差分、规则文档、清理及独立提交的要求交付可复验实现。
- 修改标记：//20261114-3 add/change''')
print(json.dumps(dict(passed=passed, canonical=15, nativeCompilations=27, totalIndependentScenes=105, stage5Complete=False)))
