"""Plan only ended portable-run empty diagnostics; retain actual logs, images, sample history and manifests."""
from pathlib import Path
import hashlib
import json
root=Path(__file__).resolve().parent;base=root/'worktree/docs/stage4-evidence/delivery-C26';owner=root/'task-temp/c26-deploy'
for run in ['portable-r1','portable-r2']:
 data=json.loads((base/run/'run.json').read_text(encoding='utf-8'));assert data['passed'] and data['privateDesktopClosed'] and not data['temporaryFilesRemaining']
files=[]
for path in base.rglob('*.txt'):
 if path.stat().st_size==0 and (path.name.endswith(('-stdout.txt','-stderr.txt')) or path.name.startswith('deploy-')):
  files.append(dict(path=str(path),bytes=0,sha256=hashlib.sha256(path.read_bytes()).hexdigest(),reason='Empty diagnostics from a completed owned portable process.'))
plan=root/'delivery-C26-cleanup-plan.json';plan.write_text(json.dumps(dict(ownerScope=str(owner),evidenceScope=str(base),files=files),ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
print(json.dumps(dict(files=len(files),plan=str(plan))))
