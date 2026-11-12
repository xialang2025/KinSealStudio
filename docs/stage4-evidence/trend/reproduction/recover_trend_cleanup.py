"""Recover a fail-closed cleanup after a shared compiler temp disappeared, without claiming natural deletions."""
from pathlib import Path
import hashlib
import json
root=Path(__file__).resolve().parent
source=root/'native-trend/native-cleanup-trend-reopen-r4.json'
d=json.loads(source.read_text(encoding='utf-8'))
stop=next(i for i,x in enumerate(d['files']) if Path(x['path']).name=='ccCTJWz3.s')
assert stop==133 and all(not Path(x['path']).exists() for x in d['files'][:stop])
remaining=[]; naturally_missing=[]
for x in d['files'][stop:]:
 p=Path(x['path'])
 if not p.exists(): naturally_missing.append(x); continue
 assert p.is_file() and p.stat().st_size==x['bytes'] and hashlib.sha256(p.read_bytes()).hexdigest()==x['sha256']
 remaining.append(x)
out=root/'native-trend/native-cleanup-recovery.json'
out.write_text(json.dumps(dict(ownerScope=d['ownerScope'],evidenceScope=d['evidenceScope'],files=remaining),ensure_ascii=False,indent=2),encoding='utf-8')
record=dict(priorVerifiedDeletions=d['files'][:stop],priorVerifiedDeletedFiles=stop,naturallyMissingNotClaimedDeleted=naturally_missing,recoveryPlan=str(out),reason='Resolve-Path stopped on a compiler temporary that disappeared; the initial build used the same Z temp root. Build completed with exit zero before recovery. Future build and native temporary directories are separate.',originalPlan=str(source))
(root/'worktree/docs/stage4-evidence/trend/native/cleanup-recovery.json').write_text(json.dumps(record,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(dict(priorDeleted=stop,remaining=len(remaining),naturallyMissing=len(naturally_missing))))
