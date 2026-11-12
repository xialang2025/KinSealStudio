"""Commit exactly one graphics slice in the isolated worktree and preserve per-day sequence numbering."""
import argparse
import datetime
import json
import re
from commit_physical_backfill import git
parser=argparse.ArgumentParser();parser.add_argument('spec');args=parser.parse_args()
spec=json.load(open(args.spec,encoding='utf-8'))
paths=['CMakeLists.txt','src/graphic_properties.h','src/graphic_properties.cpp','src/graphic_dialog.h','src/graphic_dialog.cpp','src/graphic_canvas.cpp','src/project_model.cpp','src/project_model.h','src/hmi_canvas.cpp','src/hmi_canvas.h','src/canvas_editing.cpp','src/main_window.cpp','src/native_shell_ui.cpp','src/static_text_actions.cpp','src/stage4_control_actions.cpp','tests/stage4_controls_tests.cpp','samples/stage4/'+spec['slug']+'.ksp','design_plan/A03_基础图形与表格规范.md','design_plan/A04_31控件实施索引.md','design_plan/分阶段文档证据/阶段交付与清理记录.json','design_plan/控件文档与实施覆盖矩阵.md','docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','docs/阶段4逐控件进度.md',spec['report'],'docs/stage4-evidence/progress.json','docs/stage4-evidence/'+spec['slug']+'/','程序修改记录.md']
entries=git('status','--porcelain=v1','-z','--untracked-files=all').stdout.decode('utf-8').split('\0')
assert all(any(entry[3:]==p or (p.endswith('/') and entry[3:].startswith(p)) for p in paths) for entry in entries if entry)
assert not git('diff','--cached','--name-only').stdout.strip();git('diff','--check')
now=datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8),'Asia/Shanghai'));day=now.strftime('%Y年%m月%d日')
subjects=git('log','--all','--format=%s','--since='+now.strftime('%Y-%m-%dT00:00:00+08:00')).stdout.decode('utf-8')
number=max([int(m.group(1)) for m in re.finditer(re.escape(day)+r' \d\d:\d\d:\d\d，修改(\d+)：',subjects)],default=0)+1
title=now.strftime('%Y年%m月%d日 %H:%M:%S')+f'，修改{number}：复刻{spec["name"]}图形编辑并完成原厂基线实物下载'
git('add','--',*paths);git('commit','-m',title)
print(json.dumps(dict(commit=git('rev-parse','HEAD').stdout.decode().strip(),title=title),ensure_ascii=False))
