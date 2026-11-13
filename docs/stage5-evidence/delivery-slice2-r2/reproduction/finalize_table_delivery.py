"""Freeze actual portable results and exact retained-peer cleanup for the task-owned failed attempt."""
from pathlib import Path
import hashlib
import json
import shutil

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';main=root.parents[1]/'KinSealStudio';base=work/'docs/stage5-evidence';out=base/'delivery-slice2-r2'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
manifest=read(out/'package-manifest.json');assert manifest['checksPassed']==47 and read(out/'run.json')['passed']
package=Path(manifest['package']).resolve();failed=(main/'dist/KinSealStudioLite-Stage5-compile-slice2-r1').resolve()
assert failed.parent==(main/'dist').resolve() and failed!=package
peers={sha(p):p for p in package.rglob('*') if p.is_file()}
files=[]
retained=base/'delivery-slice2-r1/retained-inputs'
for p in failed.rglob('*'):
    if not p.is_file():continue
    digest=sha(p);peer=peers.get(digest)
    if peer is None:
        assert p.suffix=='.ksp',p
        peer=retained/p.relative_to(failed);peer.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,peer)
    files.append(dict(path=str(p.resolve()),bytes=p.stat().st_size,sha256=digest,retained=str(peer.resolve())))
write(out/'cleanup-plan.json',dict(root=str(failed),files=files,logicalBytes=sum(f['bytes'] for f in files),preserved='Successful r2 package, failed input samples and diagnostics; every removed file has a retained SHA-identical peer.'))
shutil.copy2(root/'finalize_table_delivery.py',out/'reproduction/finalize_table_delivery.py')
print(json.dumps(dict(files=len(files),logicalBytes=sum(f['bytes'] for f in files),sourceCommit=manifest['sourceCommit']),ensure_ascii=False))
