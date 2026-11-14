"""Replace pending G07 statements with the actually completed evidence chain."""
from pathlib import Path
import json
import shutil

root = Path(__file__).resolve().parent
work = root.parent / 'kinseal-stage4-20261112/worktree'
read = lambda p: json.loads(p.read_text(encoding='utf-8-sig'))
core = read(work / 'docs/stage5-evidence/sector-windows-r1/run.json')
editor = read(work / 'docs/stage5-evidence/sector-editor-windows-r2/run.json')
assert core['passed'] and editor['passed']
core_count = sum(x['totals']['passed'] for x in core['runs'])
editor_count = sum(x['totals']['passed'] for x in editor['runs'])
passed = core_count + editor_count
review = read(work / 'docs/stage5-evidence/sector-windows-r1/readonly-review.json')
assert review['finalBlockingFindings'] == 0
base = work / 'docs/stage5-evidence/native/sector'
bsm = read(base / 'B-S-M-sector-cell-area-r1.json')
assert bsm['allRasterComparisonsEqual'] and len(bsm['cases']) == 27
cleanup = read(base / 'cleanup-sector-20261114.json')
empty = read(base / 'cleanup-empty-directories-r1.json')

p = work / 'docs/阶段5_扇形默认角度背景编译与差分验收.md'
t = p.read_text(encoding='utf-8')
t = t.replace('当前已证默认200×200、45°起角/90°扫角、白填NoPen的独立候选完整帧相等；正式27次目标采样和后续生产实现正在推进，不能把默认候选升级为全家族C/T/E通过，阶段5整体仍未完成。', f'本批已完成固定45°起角/90°扫角的受限背景C：27次正式编译、15份canonical/13独立场景完整帧相等，最终产品回归{passed}/0/0。仅本报告限定的默认角度/黑白/正方形切片开放，完整角度/SM/T/E及阶段5整体仍未完成。')
t = t.replace('计划复用已证独立定点曲线几何', '复用已证独立定点曲线几何').replace('拟用`geometry.adjusted', '使用`geometry.adjusted').replace('计划对本对象及所有peer', '对本对象及所有peer')
t = t.replace('## 5. 正式语料计划与拟开放范围', '## 5. 正式语料与已开放范围')
old = '当前计划27次目标：3次默认同源新进程、8组几何创建/保存重开、3组线/填开关创建/重开及双对象创建/重开。几何覆盖平移160、large240、near200、far`498,178/200×200`、奇数201、最小100及168/170两种尺寸。若全部有效，将形成15份canonical（3默认+12重开）、13个独立场景；当前不是通过数。'
new = '实际27次正式编译：3次默认同源新进程、8组几何创建/保存重开、3组线/填开关创建/重开及双对象创建/重开。几何覆盖平移160、large240、near200、far`498,178/200×200`、奇数201、最小100及168/170两种尺寸，形成15份canonical（3默认+12重开）、13个独立场景。全部完整768000B像素相等，无忽略区；S检查及独立Python/C++ coverage也全部相等。line1/fill0与line1/fill1的创建/重开四帧优先实测0差，确认局部路径先map到scene、再以scene路径生成描边，无需未使用的预构造strokeOutline扩展。'
assert old in t
t = t.replace(old, new).replace('拟开放单普通页', '当前开放单普通页')
t = t[:t.index('## 6. 尚待收尾的产品验证')]
t += f'''## 6. B/S/M、生产回归及只读复核

[正式B/S/M](stage5-evidence/native/sector/B-S-M-sector-cell-area-r1.json)按B原始文件长度/SHA与差异段、S语义结构、M完整RGB565逐层保留。三次同源重复的AlarmPara/SysPara/Usbload.bin/load.kns/upLoad.knr仍含非确定原始字节或长度，原因未知，不删改、不生成忽略掩码，也不把容器逐字节相等作为已证结论。S确认无新增ControlPara/ControlPixel，PicLenPara2400B，完整静态背景768000B；M的27次differentPixels均0。

原厂一共29个自有进程：27正式、1只读源检查、1奇数框绑定失败。odd-r1创建框为201×201，属性确认后sceneBoundingRect回算为200.99999999999997，原厂helper/容器实际成功但目标严格绑定拒绝；该轮不混入27。odd-r2只对scene bbox显式采用≤1e-8整数回算容差，raw小数值完整保留；源SHA、实际transform、26.6路径元素与整帧像素没有放宽。原始local8元素、item/painter平移和另存scene映射同时保留。

生产`paintDeviceSectorGraphic`独立构造局部8元素路径再平移，双方扩展补片隔离；`prepareDeviceSector`与renderer直接入口均限制正方形100..240及既定边界/样式。共享`paintDeviceWhiteCurvePath`只抽原椭圆的有界mask/补片主体，保留原绝对椭圆路径、scene stroke、正确D及旧守卫；未保留不需要的strokeOutline扩展。32份旧椭圆模块参考及28份扇形CLI参考交叉核共60帧逐像素相等，另计模块诊断，不计作27次原厂或产品测试数。

[开闸前red](stage5-evidence/sector-red-r1/run.json)实际2通过/15失败/0跳过，15份源绑定canonical全部由旧门禁拒绝。[最终核心](stage5-evidence/sector-windows-r1/run.json){core_count}/0/0，[完整编辑器](stage5-evidence/sector-editor-windows-r2/run.json){editor_count}/0/0，共{passed}/0/0。核心包括319编译、14后端、37扫描、6纯传输计划及10真实SDK宏；30条负向覆盖99/241、非正方形、精确右699/下379、X/Y1、休眠颜色/宽/Alpha、端点和补片物理接触，保持旧计划/payload。静态10段发布/复读和SM/legacy独立拒绝；十四对象混合仍5条640B动态记录，12段及索引/扫描通过。

[只读复核](stage5-evidence/sector-windows-r1/readonly-review.json)绑定最终源码身份，业务实现没有P1/P2阻断。完整编辑器r1实际233/1/0，失败为旧测试仍断言sector一律拒绝设备编译；先反馈再只更新测试，默认10段通过、非正方形与完整SM继续拒绝，生产没有改动。补充只读复核和r2完整编辑器234/0/0通过，原r1失败日志保留。真实曲率、多页/窗口/活动配置、26个新动态类型以及扇形完整角度/颜色/线型/填充/SM均未恢复，新输出ACK=false，无USB。

## 7. 证据保留与定向清理

[原厂清理回执](stage5-evidence/native/sector/cleanup-sector-20261114.json)：{cleanup['deletedFiles']}文件/{cleanup['deletedLogicalBytes']}逻辑字节，{empty['emptyDirectoriesRemoved']}个空目录及{empty['registeredRuntimeRootsRemoved']}个登记runtime根已移除，跳过0。含硬链接的逻辑字节不等于磁盘物理释放；29份raw完整gzip回解核SHA，28次实际编译输入及392份原厂段复核通过，唯一源/段、失败/属性/原始path与矩阵、许可证和复现配方保留。

采样owner另归档清理202份展开mask/frame临时（90+112，合计36998044逻辑字节），每份先gzip回解核SHA，自己的temp已空；模块候选交叉核的压缩证据/清理另见independent-candidate。没有清扫C盘无关文件、用户工程、SDK/安装、旧shared或增量build/cache。

版本提交/主目录同步/推送及第九批便携包另按实际回执追加，不能由候选、版本或便携包冒充T/E。完整角度编辑、非方框缩放的原厂语义、全部样式及阶段5整体仍未完成。规则同步至[UI编译规则与阶段交付规范](UI编译规则与阶段交付规范.md)第17节。
'''
p.write_text(t, encoding='utf-8')
p = work / 'docs/UI编译规则与阶段交付规范.md'
t = p.read_text(encoding='utf-8')
head, section = t.split('## 17. G07扇形：', 1)
section = section.replace('当前仅默认200×200、line0/fill1、45°起角/90°扫角的两种路径候选完整768000B相等；正式27次采样及生产流程仍在推进，不将默认相等提前升级C完成，阶段5整体未完成。', f'本批固定45°起角/90°扫角的受限背景C已完成，27次原厂完整768000B相等、15canonical/13独立场景，产品{passed}/0/0；其它角度/SM/T/E及阶段5整体仍未完成。')
section = section.replace('拟复用独立历史定点细分', '复用独立历史定点细分').replace('补片拟为', '补片为').replace('拟支持正方形', '当前支持正方形')
old = '正式目标27次（3默认+8几何创建重开+3样式组合创建重开+双对象创建重开），全部有效时15canonical/13场景；尚待实际结果。后续逐项补B/S/M、red/green、负向、包/混合页、只读复核及定向清理，不写预计通过数。新产物ACK=false，无USB，阶段5整体和扇形完整角度功能仍未完成。'
new = f'''[正式B/S/M](stage5-evidence/native/sector/B-S-M-sector-cell-area-r1.json)实际27次（3默认+8几何创建重开+3样式组合创建重开+双对象创建重开），15canonical/13场景；S结构和独立Python/C++覆盖也全等。line1四份完整帧已证scene map后stroke；只共享有界曲线mask/补片主体，椭圆仍保持原绝对路径，不保留未用的strokeOutline扩展。29个原厂进程均结束，odd-r1为helper实际成功、目标浮点框绑定拒绝，odd-r2显式≤1e-8 bbox回算容差；raw保留，像素/26.6路径/源身份仍严格。三次同源的AlarmPara/SysPara及容器非确定字节仍未知，不加忽略掩码。

red实际2/15/0；最终核心{core_count}与完整编辑器{editor_count}共{passed}/0/0。编辑器r1的233/1/0为旧测试把sector一律当未支持，先反馈后更新为默认C通过、非方框及SM拒绝，生产未改，r2完整234/0/0，失败日志保留。15源绑定canonical、30负向、静态10段包、SM/legacy门禁及十四对象混合（仍5×640B记录）实际通过；独立只读复核无剩余阻断。[最终回执](stage5-evidence/sector-windows-r1/run.json)。32椭圆+28扇形模块参考帧全等另计诊断，不并入原厂或产品数量。

原厂定向清理{cleanup['deletedFiles']}文件/{cleanup['deletedLogicalBytes']}逻辑字节、{empty['emptyDirectoriesRemoved']}空目录，29raw回解及唯一源段/失败/许可证保留；采样owner另清202份mask/frame展开物，模块交叉核清理单独登记，不扫用户build/cache或无关C临时。新产物ACK=false，无USB，阶段5整体和扇形完整角度功能仍未完成。每批同步本规则、专项、协议、台账和版本/清理回执。'''
assert old in section
section = section.replace(old, new)
p.write_text(head + '## 17. G07扇形：' + section, encoding='utf-8')
shutil.copy2(root / 'sector-green-build-r1.log', work / 'docs/stage5-evidence/sector-windows-r1/build-r1.log')
shutil.copy2(root / 'sector-red-build-r1.log', work / 'docs/stage5-evidence/sector-red-r1/build-r1.log')
shutil.copy2(root / 'sector-editor-build-r2.log', work / 'docs/stage5-evidence/sector-editor-windows-r2/build-r2.log')
print(json.dumps(dict(core=core_count, editor=editor_count, passed=passed, formalNative=27)))
