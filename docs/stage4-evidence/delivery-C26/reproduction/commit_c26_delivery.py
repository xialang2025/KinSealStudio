"""Commit the verified portable C26 artifact records and retry one GitHub push without including binary files."""
from pathlib import Path
import datetime
import json
import re
import subprocess
from commit_physical_backfill import git
root=Path(__file__).resolve().parent;work=root/'worktree';base=work/'docs/stage4-evidence/delivery-C26'
assert all(json.loads((base/run/'run.json').read_text(encoding='utf-8'))['passed'] for run in ['portable-r1','portable-r2'])
with (work/'程序修改记录.md').open('a',encoding='utf-8',newline='\n') as stream:
 stream.write('\n### 修改23：C26独立增量包与后台验证\n- 修改内容：\n  1. 从6a3c1eb已验证独立工作树生成26类两页便携包，新增曲线页完整保留采集定义与历史/XY/数据群组引用。\n  2. 10项包内Qt运行/模型/拒绝门槛通过；三控件适配默认视口后追加模型/截图2项通过；未扩展未实现的独立编码或现场结论。\n  3. 自动清理空诊断文件，更新当前便携交付索引。\n- 修改文件：docs/阶段4_C26增量包交付.md、docs/stage4-evidence/delivery-C26/、docs/stage4-evidence/progress.json、本记录；dist为本地交付，不提交二进制。\n- 影响范围：C26便携交付和两页示例，不含主目录阶段3既有18个改动。\n- 原因：使新增历史采集/历史趋势、X-Y与数据群组编辑切片可直接打开审阅。\n- 修改标记：//20261113-1 add\n')
paths=['程序修改记录.md','docs/阶段4_C26增量包交付.md','docs/stage4-evidence/delivery-C26/','docs/stage4-evidence/progress.json']
entries=git('status','--porcelain=v1','-z','--untracked-files=all').stdout.decode('utf-8').split('\0');assert all(any(x[3:]==p or (p.endswith('/') and x[3:].startswith(p)) for p in paths) for x in entries if x)
assert not git('diff','--cached','--name-only').stdout.strip();git('diff','--check')
now=datetime.datetime.now(datetime.timezone(datetime.timedelta(hours=8),'Asia/Shanghai'));day=now.strftime('%Y年%m月%d日');subjects=git('log','--all','--format=%s','--since='+now.strftime('%Y-%m-%dT00:00:00+08:00')).stdout.decode('utf-8')
number=max([int(m.group(1)) for m in re.finditer(re.escape(day)+r' \d\d:\d\d:\d\d，修改(\d+)：',subjects)],default=0)+1
title=now.strftime('%Y年%m月%d日 %H:%M:%S')+f'，修改{number}：发布C26独立增量包并验证包内运行与输出边界'
git('add','--',*paths);git('commit','-m',title);head=git('rev-parse','HEAD').stdout.decode().strip()
try:
 push=subprocess.run(['git','-C',str(work),'push','origin','HEAD:refs/heads/main'],capture_output=True,timeout=35);success=push.returncode==0;detail=push.stderr.decode('utf-8',errors='replace')[-1500:]
except subprocess.TimeoutExpired:
 success=False;detail='Push exceeded 35 seconds; local commit preserved.'
(root/'pending-push.json').write_text(json.dumps(dict(status='pushed' if success else 'pending',head=head,detail=detail),ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
print(json.dumps(dict(commit=head,title=title,pushed=success),ensure_ascii=False))
