"""Commit only the reviewed C25 slice in the isolated checkout."""
import datetime
import json
import re
from commit_physical_backfill import git

paths = ['CMakeLists.txt', 'src/xy_curve_dialog.cpp', 'src/xy_curve_dialog.h', 'src/xy_curve_properties.cpp', 'src/xy_curve_properties.h', 'src/project_model.cpp', 'src/project_model.h', 'src/hmi_canvas.cpp', 'src/main_window.cpp', 'src/native_shell_ui.cpp', 'src/stage3_project_edits.cpp', 'src/stage4_control_actions.cpp', 'src/static_text_actions.cpp', 'tests/stage4_controls_tests.cpp', 'samples/stage4/xy-curve.ksp', 'design_plan/A04_31控件实施索引.md', 'design_plan/分阶段文档证据/阶段交付与清理记录.json', 'design_plan/控件文档与实施覆盖矩阵.md', 'design_plan/X-Y曲线功能与交互复刻手册.md', 'docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md', 'docs/阶段4逐控件进度.md', 'docs/X-Y曲线手册实现与验收.md', 'docs/stage4-evidence/progress.json', 'docs/stage4-evidence/xy-curve/', '程序修改记录.md']
entries = git('status', '--porcelain=v1', '-z', '--untracked-files=all').stdout.decode('utf-8').split('\0')
assert all(any(entry[3:] == path or (path.endswith('/') and entry[3:].startswith(path)) for path in paths) for entry in entries if entry)
assert not git('diff', '--cached', '--name-only').stdout.strip()
git('diff', '--check')
now = datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8), 'Asia/Shanghai'))
day = now.strftime('%Y年%m月%d日')
subjects = git('log', '--all', '--format=%s', '--since=' + now.strftime('%Y-%m-%dT00:00:00+08:00')).stdout.decode('utf-8')
number = max([int(m.group(1)) for m in re.finditer(re.escape(day) + r' \d\d:\d\d:\d\d，修改(\d+)：', subjects)], default=0) + 1
title = now.strftime('%Y年%m月%d日 %H:%M:%S') + f'，修改{number}：复刻X-Y曲线双数值轴与成对跨度并验证原厂基线实物下载'
git('add', '--', *paths)
git('commit', '-m', title)
print(json.dumps(dict(commit=git('rev-parse', 'HEAD').stdout.decode().strip(), title=title), ensure_ascii=False))
