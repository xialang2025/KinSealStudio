"""Record a verified ninth portable package and create its separate delivery commit."""
from pathlib import Path
import datetime
import json
import re
import shutil
import subprocess

root = Path(__file__).resolve().parent
work = root.parent / 'kinseal-stage4-20261112/worktree'
read = lambda p: json.loads(p.read_text(encoding='utf-8-sig'))
write = lambda p, d: p.write_text(json.dumps(d, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')

def git(*arguments):
    return subprocess.run(['git', '-C', str(work), *arguments], check=True, capture_output=True).stdout

def append(relative, text):
    p = work / relative
    p.write_text(p.read_text(encoding='utf-8-sig').rstrip() + '\n\n' + text.strip() + '\n', encoding='utf-8')

head = git('rev-parse', 'HEAD').decode().strip()
assert head == read(root / 'sector-commit.json')['commit']
out = work / 'docs/stage5-evidence/delivery-slice9-r1'
d = read(out / 'package-manifest.json')
run = read(out / 'run.json')
assert d['sourceCommit'] == head and run['passed'] and d['checksPassed'] == 136 and run['onlyBundledQtDllsObserved']
assert read(out / 'preview-reviewed.json')['reviewed']
assert '### 修改89：' not in (work / '程序修改记录.md').read_text(encoding='utf-8')
note = (f'第九批便携包`{d["package"]}`已通过{d["checksPassed"]}项实际检查，源提交`{head}`。'
        '新增扇形默认、实线无填、奇数201及168/170阈值五样例和十四对象混合；10/12/14段均实际生成并复读，Qt DLL全部来自包内，混合页截图已实际查看。'
        '源/legacy/非正方形/Alpha门禁正确拒绝且无残留输出；没有USB通信，deviceAcknowledged=false。')
append('docs/阶段5_扇形默认角度背景编译与差分验收.md', '## 第九批便携交付\n\n' + note + '\n\n[真实交付证据](stage5-evidence/delivery-slice9-r1/package-manifest.json)。版本和便携包不替代新产物T/E，完整阶段5继续按台账推进。')
append('docs/UI编译规则与阶段交付规范.md', '## 18. 第九批扇形便携规则（2026-11-14）\n\n' + note + '\n\nsector严格使用七键graphic；不写角度、半径、points或endArrow/endCap。固定45/90开放8元素先局部生成再整数平移，fill隐式闭合、stroke不补第九边；尺寸100..240正方形与X/Y≥2、右698/下378界限同生产。十四对象布局将矩形移到130,300，新增扇形10,205/100×100；91对扩展补片经计算全部不相交/接触，实际12段及五条640B控制记录复读通过。[回执](stage5-evidence/delivery-slice9-r1/run.json)。')
append('docs/阶段5逐项进度.md', '### 第九批便携交付\n\n' + note)
append('design_plan/06_阶段5_设备编译与格式差分.md', '### 15.1 第九批便携实测\n\n' + note + '\n\n[回执](../docs/stage5-evidence/delivery-slice9-r1/run.json)。真实曲率、26新动态类型、多页/活动配置及新产物T/E尚未闭合。')
append('docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md', '### 92.1 扇形便携验证\n\n' + note + '\n\n[交付清单](stage5-evidence/delivery-slice9-r1/package-manifest.json)。背景切片没有更改控制记录类型或USB传输规则。')
p = work / 'docs/stage5-evidence/progress.json'
progress = read(p)
progress['latestDelivery'] = dict(package=d['package'], sourceCommit=head, evidence='docs/stage5-evidence/delivery-slice9-r1/package-manifest.json', checksPassed=d['checksPassed'], usbAccessed=False)
progress['sectorSlice']['delivery'] = progress['latestDelivery']
progress['active'] = 'G07扇形默认角度及第九批便携已交付；继续真实曲率/动态类型/多页配置等任务'
write(p, progress)
append('程序修改记录.md', f'''### 修改89：阶段5扇形便携包与十四对象混合样例
- 修改内容：第九批便携包实际通过{d['checksPassed']}项检查，源提交`{head}`；五个扇形样例及十四对象混合，真实10/12/14段生成复读，包内Qt身份、截图及负向门禁均核验；无USB通信。
- 修改文件：samples/stage5新增5个扇形样例和mixed-fourteen.ksp、docs/stage5-evidence/delivery-slice9-r1、UI编译规则18/扇形专项/协议92/阶段5进度及本记录。
- 影响范围：第九批可运行离线交付，没有新增生产逻辑；完整阶段5、新产物T/E仍未完成。主目录与远端状态以实际发布回执为准。
- 原因：提供默认/描边/奇数/阈值及混合索引的可复验样例，并同步详细UI和设备编译规则。
- 修改标记：//20261114-3 add（交付）''')
for name in ('prepare_sector_delivery.py', 'finalize_sector_delivery.py'):
    shutil.copy2(root / name, out / 'reproduction' / name)
paths = ['samples/stage5/sector-default.ksp', 'samples/stage5/sector-outline.ksp',
         'samples/stage5/sector-odd.ksp', 'samples/stage5/sector-threshold168.ksp',
         'samples/stage5/sector-threshold170.ksp', 'samples/stage5/mixed-fourteen.ksp',
         'docs/stage5-evidence/delivery-slice9-r1', 'docs/UI编译规则与阶段交付规范.md',
         'docs/阶段5_扇形默认角度背景编译与差分验收.md', 'docs/阶段5逐项进度.md',
         'design_plan/06_阶段5_设备编译与格式差分.md', 'docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md',
         'docs/stage5-evidence/progress.json', '程序修改记录.md']
assert not git('diff', '--cached', '--name-only').strip()
print(git('status', '--short').decode(errors='replace'))
git('add', '--', *paths)
staged = list(filter(None, git('diff', '--cached', '--name-only', '-z').decode().split('\0')))
assert all(any(name == p or name.startswith(p + '/') for p in paths) for name in staged)
git('diff', '--cached', '--check')
now = datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8)))
day = now.strftime('%Y年%m月%d日')
subjects = git('log', '--all', '-400', '--format=%s').decode()
sequence = max([int(m[1]) for m in re.finditer(re.escape(day) + r' \d\d:\d\d:\d\d，修改(\d+)：', subjects)], default=0) + 1
message = now.strftime('%Y年%m月%d日 %H:%M:%S') + f'，修改{sequence}：交付扇形便携包及十四对象编译回归样例'
git('commit', '-m', message)
result = dict(commit=git('rev-parse', 'HEAD').decode().strip(), message=message, files=len(staged), sourceCommit=head, checksPassed=d['checksPassed'])
write(root / 'sector-delivery-commit.json', result)
print(json.dumps(result, ensure_ascii=False))
