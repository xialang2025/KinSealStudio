"""Record delayed human observations against their exact original pages, preserving active table changes."""
from pathlib import Path
import datetime
import json
import re
import subprocess
import sys
from commit_physical_backfill import git, ROOT, WORK

items = [
    ('G02', 'freeline', '自由直线首尾切片实现与验收.md', '已显示黑色斜线，没有宽度提示', '黑色斜线'),
    ('G03', 'curve', '曲线圆弧调查与阻塞记录.md', '倒 V 形两段直线，没有宽度提示', '倒V形两段直线'),
    ('G09', 'polygon', '多边形图形实现与验收.md', '已显示黑色三角形，没有宽度提示', '黑色实心三角形'),
]
paths = []
sentences = []
progress_path = WORK / 'docs/stage4-evidence/progress.json'
progress = json.loads(progress_path.read_text(encoding='utf-8'))
matrix_path = WORK / 'design_plan/控件文档与实施覆盖矩阵.md'
matrix_lines = matrix_path.read_text(encoding='utf-8-sig').splitlines()
for identifier, slug, report, answer, expected in items:
    relative = f'docs/stage4-evidence/{slug}/physical/{slug}-baseline-r1/screen-result.json'
    destination = WORK / relative
    assert not destination.exists(), relative
    run = json.loads(destination.with_name('run.json').read_text(encoding='utf-8-sig'))
    assert run['completed'] and run['nativeHardware']['usbCompletion']['ready']
    observation = dict(source='explicit_user_reply', answer=answer, sourceSha256=run['sourceSha256'],
        nativeBaselineDefaultDisplayConfirmed=True, widthWarningAbsent=True, screenTouchConfirmed=False,
        fullRenderingNotClaimed=True, liteIndependentEncoderVerified=False,
        scope='Reply to the exact earlier question and downloaded baseline; not evidence for a later page or other variants.')
    if slug == 'curve':
        observation.update(observedGeometry='two_straight_segments', trueCurvatureConfirmed=False)
    destination.write_text(json.dumps(observation, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    sentence = f'{identifier}用户明确回复“{answer}”，仅确认此前对应原厂基线页显示{expected}且没有宽度提示；完整变体、触控和Lite独立编码仍未验收。'
    if slug == 'curve':
        sentence += '此结果支持原厂三点诊断页是折段，真实曲率阻塞未解除。'
    report_path = WORK / 'docs' / report
    content = report_path.read_text(encoding='utf-8-sig')
    note = sentence + f'\n\n[原始人工确认](stage4-evidence/{slug}/physical/{slug}-baseline-r1/screen-result.json)。'
    content = content.replace('本轮没有新增实屏显示确认。', note) if '本轮没有新增实屏显示确认。' in content else content + '\n\n## 对应诊断页实屏补验\n\n' + note + '\n'
    report_path.write_text(content, encoding='utf-8', newline='\n')
    current = next(item for item in progress['controls'] if item['id'] == identifier)
    current.update(screenObservation=relative, screenObservationScope='native_reference_default_diagnostic_only')
    current.setdefault('layers', {})['E'] = 'native_default_diagnostic_display_confirmed_full_E_unverified'
    for index, line in enumerate(matrix_lines):
        fields = line.split('|')
        if len(fields) >= 13 and fields[1].strip() == current['name']:
            fields[11] = ' 原厂默认诊断页显示确认；完整E未验 '
            matrix_lines[index] = '|'.join(fields)
    paths.extend([relative, str(report_path.relative_to(WORK)).replace('\\', '/')])
    sentences.append(sentence)
progress_path.write_text(json.dumps(progress, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
matrix_path.write_text('\n'.join(matrix_lines) + '\n', encoding='utf-8', newline='\n')
protocol = 'docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md'
with (WORK / protocol).open('a', encoding='utf-8', newline='\n') as stream:
    stream.write('\n\n§78—80对应原厂基线实屏补验：\n\n' + '\n\n'.join(sentences) + '\n')
log = '程序修改记录.md'
number = max(map(int, re.findall(r'^### 修改(\d+)：', (WORK / log).read_text(encoding='utf-8-sig'), re.M))) + 1
with (WORK / log).open('a', encoding='utf-8', newline='\n') as stream:
    stream.write(f'\n\n### 修改{number}：补记自由直线、曲线诊断和多边形对应实屏确认\n- 修改内容：\n' + '\n'.join(f'  {i+1}. {s}' for i, s in enumerate(sentences)) + '\n- 修改文件：各原厂基线screen-result.json、对应报告、进度、覆盖矩阵、USB协议、本记录。\n- 影响范围：仅补充对应基线默认显示证据；曲率阻塞和完整E边界保留。\n- 原因：用户对先前各自下载问题给出明确实屏回复。\n- 修改标记：//20261113-9 add（文档）\n')
paths += [str(progress_path.relative_to(WORK)).replace('\\', '/'), str(matrix_path.relative_to(WORK)).replace('\\', '/'), protocol, log]
assert not git('diff', '--cached', '--name-only').stdout.strip()
git('diff', '--check')
previous = git('rev-parse', 'HEAD').stdout.decode().strip()
now = datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8)))
day = now.strftime('%Y年%m月%d日')
subjects = git('log', '--all', '--format=%s', '--since=' + now.strftime('%Y-%m-%dT00:00:00+08:00')).stdout.decode('utf-8')
sequence = max([int(m.group(1)) for m in re.finditer(re.escape(day) + r' \d\d:\d\d:\d\d，修改(\d+)：', subjects)], default=0) + 1
title = now.strftime('%Y年%m月%d日 %H:%M:%S') + f'，修改{sequence}：补记三个图形对应原厂诊断页实屏确认'
git('add', '--', *paths)
git('commit', '-m', title)
publication = subprocess.run([sys.executable, '-B', str(ROOT / 'publish_stage4.py'), '--previous', previous], capture_output=True, check=True)
print(publication.stdout.decode('utf-8'))
print(title)
