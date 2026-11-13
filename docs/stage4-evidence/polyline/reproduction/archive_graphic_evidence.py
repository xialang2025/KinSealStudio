"""Copy only one family's existing evidence, verifying identities and leaving the original inventory untouched."""
from pathlib import Path
import hashlib
import json
import shutil
import sys
root=Path(__file__).resolve().parent
family=sys.argv[1]
assert family in ['line','rect','ellipse','roundrect','polyline','polygon','freeline','curve','sector','table']
source=root.parents[1]/'KinSealStudio/docs/stage4-evidence/graphics/native'
destination=root/'worktree/docs/stage4-evidence'/family/'native'
destination.mkdir(parents=True,exist_ok=True)
paths=[]
for p in source.iterdir():
    if p.is_file() and (p.name.startswith(family+'-') or p.name.startswith('native-probe-'+family+'-') or p.name.startswith('sheet-'+family+'-')):
        paths.append(p)
for folder in (source/'compiled').iterdir():
    if folder.is_dir() and folder.name.startswith(family+'-'):
        paths.extend(p for p in folder.rglob('*') if p.is_file() and p.suffix.lower() in ['.dat','.json'])
records=[]
for p in paths:
    relative=p.relative_to(source);target=destination/relative
    identity=hashlib.sha256(p.read_bytes()).hexdigest()
    if target.exists():assert hashlib.sha256(target.read_bytes()).hexdigest()==identity
    else:
        target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,target)
    records.append(dict(path=relative.as_posix(),sha256=identity,bytes=p.stat().st_size))
(destination/'retained-manifest.json').write_text(json.dumps(dict(family=family,sourceRoot=str(source),originalEvidenceModified=False,files=records),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(family=family,files=len(records),bytes=sum(p['bytes'] for p in records))))
