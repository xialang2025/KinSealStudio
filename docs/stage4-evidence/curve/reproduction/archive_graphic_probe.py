"""Archive one ended private graphics investigation and plan only its own identified cleanup."""
from pathlib import Path
import argparse, gzip, hashlib, json, shutil
r=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('family');p.add_argument('folder');p.add_argument('run');p.add_argument('script');p.add_argument('scenario');a=p.parse_args()
owner=r/'native-graphics';base=r/'worktree/docs/stage4-evidence'/a.family/'native';b=base/a.folder
assert b.resolve().is_relative_to(base.resolve())
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
p=b/('native-probe-'+a.run+'.json');raw=p.read_bytes();d=json.loads(raw)
assert d['ownedProcessTerminated'] and not d['systemInput'] and not d['usbOrPlc'] and 'compaction' not in d
archive=b/(p.name+'.gz');archive.write_bytes(gzip.compress(raw,mtime=0));assert gzip.decompress(archive.read_bytes())==raw
compact={k:v for k,v in d.items() if k not in ('snapshots','menuInventory','steps','instrumentationMessages')};compact['compaction']=dict(rawArchive=archive.name,rawSha256=hashlib.sha256(raw).hexdigest());p.write_text(json.dumps(compact,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
plan=[]
for name in ['native-probe.json','native-probe-checkpoint.json']:
 peer=b/name
 if not peer.exists():continue
 data=peer.read_bytes();gzip_peer=b/(name+'.gz');gzip_peer.write_bytes(gzip.compress(data,mtime=0));assert gzip.decompress(gzip_peer.read_bytes())==data
 plan.append(dict(path=str(peer),bytes=len(data),sha256=hashlib.sha256(data).hexdigest(),reason='Full bytes preserved in verified gzip.'))
repro=b/'reproduction';repro.mkdir(exist_ok=True)
for name in [a.script,a.scenario]:shutil.copy2(owner/name,repro/name)
retained={sha(x):x for x in base.rglob('*') if x.is_file() and x.suffix!='.gz' and x.name not in ['native-probe.json','native-probe-checkpoint.json']}
runtime=owner/('runtime-'+a.run);assert runtime.resolve().is_relative_to(owner.resolve())
for folder in [runtime,owner/'temp']:
 for x in folder.rglob('*'):
  if not x.is_file():continue
  digest=sha(x)
  if x.suffix.lower() in ['.sm','.dat'] and digest not in retained:
   out=b/'retained-runtime';out.mkdir(exist_ok=True);target=out/(digest[:12]+'-'+x.name);shutil.copy2(x,target);retained[digest]=target
  plan.append(dict(path=str(x),bytes=x.stat().st_size,sha256=digest,retained=str(retained[digest]) if digest in retained else None,reason='Owned ended private graphics runtime/Frida host temporary file.'))
path=owner/('native-cleanup-'+a.run+'.json');path.write_text(json.dumps(dict(ownerScope=str(owner),evidenceScope=str(base),files=plan),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(plan=str(path),files=len(plan),logicalBytes=sum(x['bytes'] for x in plan))))
