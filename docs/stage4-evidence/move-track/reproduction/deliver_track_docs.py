"""Prepare C21 reports only from actual software runs, isolated product gates and native-reference hardware evidence."""
from pathlib import Path
import copy
import hashlib
import json
import os
import shutil
import subprocess
root = Path(__file__).resolve().parent
work = root / 'worktree'
base = work / 'docs/stage4-evidence/move-track'


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, data):
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


assert not (work / 'docs/移动轨迹手册实现与验收.md').exists()
software = read(base / 'windows-r2/run.json')
initial = read(base / 'windows-r1/run.json')
macro = read(base / 'macro-r2/run.json')
assert software['passed'] and software['runs'][0]['totals'] == dict(passed=109, failed=0, skipped=0)
assert initial['runs'][1]['totals'] == dict(passed=59, failed=0, skipped=0)
assert macro['passed'] and macro['runs'][0]['totals'] == dict(passed=10, failed=0, skipped=0)
final = copy.deepcopy(software)
final['runs'] = [dict(software['runs'][0], evidenceSource='windows-r2/run.json'), dict(initial['runs'][1], evidenceSource='windows-r1/run.json'), dict(macro['runs'][0], evidenceSource='macro-r2/run.json')]
final['actualSourceRecords'] = ['windows-r2/run.json', 'windows-r1/run.json', 'macro-r2/run.json']
final['aggregationOnlyNoTestsInvented'] = True
(base / 'windows-final').mkdir(exist_ok=False)
write(base / 'windows-final/run.json', final)
hardware = read(base / 'physical/track-native-baseline-r1/run.json')
assert hardware['completed'] and hardware['inputUnchanged'] and hardware['sourceAtTransferMatchesInput'] and hardware['ownedProcessTerminated']
assert hardware['sourceProvenance'] == 'retained_native_reference_source' and hardware['liteIndependentEncoderVerified'] is False
assert all(not row['afterChecked'] for row in hardware['nativeHardware']['transferOptions'])
shutil.copy2(base / 'windows-r2/track-mixed.ksp', work / 'samples/stage4/move-track.ksp')
product = read(work / 'samples/stage4/move-track.ksp')
for page in product['pages']:
    page['objects'] = [obj for obj in page['objects'] if obj['type'] == 'track']
assert sum(len(page['objects']) for page in product['pages']) == 1
inputs = base / 'inputs'
inputs.mkdir(exist_ok=True)
product_path = inputs / 'move-track-only.ksp'
write(product_path, product)
environment = os.environ.copy()
environment['PATH'] = 'G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;' + environment.get('PATH', '')
executable = work / 'build/KinSealStudioLite.exe'
calls = []
for label, arguments in [('capabilities', ['--project-capabilities', str(product_path)]), ('compile', ['--compile-project', str(product_path), str(inputs / 'unpublished-compile')]), ('export-source', ['--export-native-project', str(product_path), str(inputs / 'unpublished-source.sm')])]:
    result = subprocess.run([str(executable), *arguments], env=environment, capture_output=True, timeout=30, creationflags=subprocess.CREATE_NO_WINDOW)
    stdout = result.stdout.decode('utf-8-sig', errors='replace')
    calls.append(dict(label=label, exitCode=result.returncode, stdout=stdout, stderr=result.stderr.decode('utf-8-sig', errors='replace'), result=json.loads(stdout)))
assert calls[0]['exitCode'] == 0 and calls[0]['result']['modelPersistence']['allowed'] is True
assert calls[1]['exitCode'] != 0 and calls[2]['exitCode'] != 0
assert not (inputs / 'unpublished-source.sm').exists() and not (inputs / 'unpublished-compile').exists()
write(base / 'backend-gates.json', dict(executableSha256=hashlib.sha256(executable.read_bytes()).hexdigest(), inputPath=str(product_path.relative_to(work)), inputSha256=hashlib.sha256(product_path.read_bytes()).hexdigest(), calls=calls, usbAccessed=False, sourceAndDeviceRefusalsRetained=True))
(base / 'build').mkdir(exist_ok=True)
for path in root.glob('track-build-r*.log'):
    shutil.copy2(path, base / 'build' / path.name)
(base / 'physical/reproduction').mkdir(exist_ok=True)
for name in ('run_stage4_native_physical.py', 'archive_physical_run.py', 'execute_native_cleanup.ps1'):
    shutil.copy2(root / name, base / 'physical/reproduction' / name)
(base / 'reproduction').mkdir(exist_ok=True)
for name in ('implement_track_integration.py', 'deliver_track_docs.py', 'prepare_control_cleanup.py', 'execute_control_cleanup.ps1'):
    shutil.copy2(root / name, base / 'reproduction' / name)
report = '''# 移动轨迹手册实现与验收

日期：2026-11-12。阶段4第21/41项，C21／S04-07。交付独立模型、两页编辑、静态图形、KSP及文档事务切片；原厂基线实物下载成功，Lite独立源/设备编码仍阻塞。

## 原厂补证和旧手册修订

已阅读完整《移动轨迹功能与交互复刻手册》及公共规范；在自有私有桌面直接读取Qt属性、保存单根源并调用原厂编译器。见[完整归档/输入身份](stage4-evidence/move-track/native/native-manifest.json)和[逐项事实](stage4-evidence/move-track/native/findings.json)。

- 方向实际顺序为向上、向下、向左、向右，默认向右；修订旧手册的上下顺序。移动距离0..900、默认100，-1钳0、901钳900。
- 常量默认0/100，两条地址默认LW0/LW0。地址模式禁用常量、启用地址；常量7/88与地址LW20/LW24切换两轮均保留，不能套用滑动开关重新启用时重置动态地址的规则。
- 常量65535的acceptableInput=true，65536、-1和1.5为false；原厂确认非法范围/相等范围的完整提示仍未测。Lite要求有效常量为U16且min<max，后者是明确的本版保护。
- 默认源含:/btn/images/gray.kvg，映射现有相同gray.kvg图库资源，保持银色球。图形是静态编辑外观，不按方向/距离虚构PLC位置。隐藏轨迹点、X/Y字段没有新增。
- 权限等级默认显现1、触控2；新建两列位单选均空圈，接受保存后重开均选1。五数据类型、六比较项/八等级已枚举；等值误差交互的r3原厂调用失败，不算通过。
- 通知总开关打开后两子选项确实启用，通知位→通知字会取消通知位；关闭通知保留字选项和值。旧手册“勾选总框后仍灰显”已被本轮实测修订。无按压时间或宏入口。

首轮由于未创建输出目录而未保留初始快照，不提供原厂行为结论；补建目录后r2完成。r3在权限GroupV的原厂GUI线程调用报system error，保留其此前实际范围/输入观察；r4跳过该失败调用并完成通知/数值和变体编译。失败记录没有回填成功。

## 字段、事件及交付范围

| ID | 字段/事件及实现 |
|---|---|
| F-C21-ADDR | 独立写入字地址默认LW0；地址拾取为嵌套草稿，活动地址按工程连接校验。当前PC编辑载体一字不代表完整固件存储已恢复。 |
| F-C21-GEOMETRY | 独立方向0..3、距离0..900；画布自由矩形静态图，不将距离直接改成控件宽高或添加看不见的轨迹点。 |
| F-C21-RANGE | dynamicRange、minimum/maximum、minimumAddress/maximumAddress全部同时保存；仅切enabled，两个草稿不重置。 |
| F-C21-IMAGE | appearance.picture与原始PNG绑定；系统/工程图库、子取消/父取消、全页改名/替换/删除保护和内容冲突复制。 |
| F-C21-ACCESS | visibility/touch分别保存主开关、位地址/状态、等级和字类型/地址/比较/值/误差；fresh/accepted选择标志独立保存，已选0不被覆盖。 |
| F-C21-NOTICE | notice、noticeBit/Word、位地址/状态、字地址/类型/值独立保存；位字互斥，关闭总开关不删除潜在草稿。无按压/宏。 |
| EV-C21-PLACE | 工具箱立即属性→确定待放置→拖框一次提交；属性取消或待放置Esc保持原工程和资源。 |
| EV-C21-EDIT | 双击/属性回填、侧栏独立写入地址，禁用通用文字；复制时操作、范围、权限和通知地址按独立角色递增。 |
| F-C21-PERSIST | schema12保存完整严格字段；21类混合和track-only往返，缺/多字段、错类型、小数枚举、坏图、错误类型绑定原子拒绝。 |
| EV-C21-GATE | 源和独立设备编码保持拒绝，不发布部分SM/包；原厂实物基线另记录。 |

## 实际软件验证与复核

最终[控件109/0/0](stage4-evidence/move-track/windows-r2/run.json)，包含本次新增6项；[原厂源59/0/0](stage4-evidence/move-track/windows-r1/run.json)及[已配置SDK宏10/0/0](stage4-evidence/move-track/macro-r2/run.json)。[汇总178/0/0](stage4-evidence/move-track/windows-final/run.json)明确引用三个真实运行记录，没有重造测试。

首次构建新复制测试漏传可选newIds参数，修正测试调用后构建通过；只读复核还发现常量框即时验证未接入，已补0..65535整数验证。首轮控件109通过、源59通过；宏因环境变量名配置错误5通过/5跳过，使用KINSEAL_ARM_SDK重跑10/0/0，不冒充首轮全部通过。截图复核缩窄字段和图库按钮并恢复两组间距后，控件最终再次109/0/0。

[样例](../samples/stage4/move-track.ksp)含21类；[单控件源导出/独立编译门槛](stage4-evidence/move-track/backend-gates.json)真实校验模型可保存，而源和独立编译均拒绝且未留下部分输出。[外观](stage4-evidence/move-track/windows-r2/track-appearance.png)、[权限](stage4-evidence/move-track/windows-r2/track-permissions.png)实际查看；整体布局、图库和未穷尽尺寸不声明全部像素一致。

## 实物下载

[原厂基线记录](stage4-evidence/move-track/physical/track-native-baseline-r1/run.json)实际完成：唯一健康7365:616C、libusb0设备，保留原厂单控件源→原厂编译器→原厂下载。四项清除/底层更新选项全部关闭；输入未改、传输源SHA一致，DownLoadDlg/m_table取得“下载完成”，自有进程终止，完整编译包保留。

默认源3917B、SHA256 4dcb78b01775f5f597c709f3ecdaf6f24dbf9aa7b47985d5b07e16e84e383138；多变量变体3905B、SHA256 ceb7033978274c05c4ab13b04b5acadde34954aa2827b27c5ac9b1e0d286a8d7。两套原厂ControlPara均640B、ControlPixel均11744B；多个字段及独立进程同时改变，不能把全部差异归于方向，不能按总长推导设备记录布局。

来源为retained_native_reference_source，liteIndependentEncoderVerified=false。原厂成功不能替代新版源/设备编码或独立下载；没有逐字节USB应答、屏内读回、实屏显示/触控及PLC刺激，相关层保持未验。

## 未决项

| 编号 | 层及后续证据 |
|---|---|
| Q-S04-C21-001 | P_SM：独立移动轨迹源、图片/权限/几何字段及无损写回未恢复。 |
| Q-S04-C21-002 | C/独立T：设备type、参数/图帧/扫描与地址宽度布局未恢复；原厂基线成功不替代新版生成下载。 |
| Q-S04-C21-003 | E：方向与数值映射、距离含义、越界、快速拖动/松手写入、通知和权限的实际时序及屏显/触控未验。 |
| Q-S04-C21-004 | D/U/R：非法/相等范围确认、原厂图库确认/取消回填、全方向与全尺寸几何/图片缩放、字等值误差交互及权限公式未穷尽；本版min<max保护与静态缩放分开标识。 |

## 清理

本项原厂调查逐次清理164个自有运行、Frida宿主和可再生文件；硬件下载另清46个文件，完整原始JSON压缩记录、源和必要DAT/下载包保留。重复软件PNG/KSP清理量见cleanup.json。临时目录都指向Z盘，不删除空目录或其他任务资料，不把硬链接逻辑字节当成物理释放量。
'''
(work / 'docs/移动轨迹手册实现与验收.md').write_text(report, encoding='utf-8')
spec = dict(id='C21', ordinal=21, name='移动轨迹', slug='move-track', task='S04-07', next='C22', usbAccessed=True,
    hardwareRun='docs/stage4-evidence/move-track/physical/track-native-baseline-r1/run.json', expectedPassed=178, testRun='docs/stage4-evidence/move-track/windows-final/run.json', nativeManifest='../docs/stage4-evidence/move-track/native/native-manifest.json',
    report='docs/移动轨迹手册实现与验收.md', manual='design_plan/移动轨迹功能与交互复刻手册.md',
    scope='独立MoveTrackProperties/Dialog及两页全部可见字段、方向实测顺序和距离0..900、常量/地址双草稿保留、gray银色球及图库嵌套资源事务、fresh/accepted权限选择与通知互斥、21类混合/单类型schema12、独立地址角色递增/侧栏/撤销；新增单类型门槛与原厂基线实物下载',
    unresolved=['Q-S04-C21-001', 'Q-S04-C21-002', 'Q-S04-C21-003', 'Q-S04-C21-004'],
    nativeSummary='原厂方向实际向上/向下/向左/向右，距离0..900默认100；常量7/88和动态LW20/LW24切换独立保留，通知主框启用子项且位字互斥，修订旧手册方向顺序/通知灰显结论。U16上下界及负数/小数acceptableInput实测；fresh位空圈、默认接受重开选1，原源gray.kvg。r1输出目录失败、r3原厂权限调用系统错误均保留，r2/r4实际保存单根并编译。',
    protocol='原厂移动轨迹单控件默认3917B，SHA256 4dcb78b01775f5f597c709f3ecdaf6f24dbf9aa7b47985d5b07e16e84e383138；多变量变体3905B，SHA256 ceb7033978274c05c4ab13b04b5acadde34954aa2827b27c5ac9b1e0d286a8d7。两份ControlPara总640B、ControlPixel11744B，不按总长或跨进程多变量差分推断记录。原厂基线真实下载完成，四项清除/更新均关闭并核验，源与传输SHA一致、进程结束、编译包保留。来源retained_native_reference_source、Lite独立编码未通过；新版单类型实际能力检查模型可保存，源导出和独立编译继续拒绝且无部分输出。未抓逐字节USB应答，下载日志不升级为完整协议布局或屏显/触控/PLC行为。详见[原厂清单](stage4-evidence/move-track/native/native-manifest.json)、[门槛](stage4-evidence/move-track/backend-gates.json)及专项报告。',
    codeFiles='src/move_track_properties.*、move_track_dialog.*、project_model.*、stage3_project_edits.cpp、stage3_resources.cpp、stage4_control_actions.cpp、static_text_actions.cpp、main_window.cpp、native_shell_ui.cpp、hmi_canvas.cpp、CMakeLists.txt',
    reviewFindings=['只读复核检查独立草稿、0..900距离、方向顺序、动态图范围不重置、通知互斥与关闭保留、权限默认寿命、跨页资源/复制、schema12与未知源/设备拒绝；发现并补齐常量即时验证。', '首轮测试编译漏传可选newIds，修正测试调用；控件109/0/0、源59/0/0，宏首轮SDK变量错误5通过/5跳过，已配置真实SDK重跑10/0/0。截图调整字段宽和组间距后最终控件再次109/0/0。', '原厂r3权限调用失败明确保留；原厂基线实物完成不代替新版独立编码或E，完整编码与实际写入语义继续阻塞。'])
write(root / 'track-finalize.json', spec)
print(json.dumps(dict(controls=109, source=59, macro=10, isolatedModelAllowed=True, independentCompileAndSource='blocked', nativeReferenceDownload=True), ensure_ascii=False))
