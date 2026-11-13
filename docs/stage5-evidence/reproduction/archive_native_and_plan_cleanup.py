"""Archive raw probes and plan exact task-owned regenerable cleanup after verifying retained peers."""
from pathlib import Path
import gzip
import hashlib
import json
import shutil

root=Path(__file__).resolve().parent
old=root.parent/'kinseal-stage4-20261112'
work=old/'worktree'
base=work/'docs/stage5-evidence'
native=base/'native/polyline'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
tests=read(base/'windows-r5/run.json');assert tests['passed'] and sum(x['totals']['passed'] for x in tests['runs'])==107
reproduction=base/'reproduction';reproduction.mkdir(exist_ok=True)
for p in root.iterdir():
    if p.is_file() and p.suffix in ('.py','.cpp','.json','.log'):
        shutil.copy2(p,reproduction/p.name)
archives=[]
for folder in sorted(native.iterdir()):
    probe=folder/'native-probe.json'
    if not probe.is_file():continue
    raw=probe.read_bytes();data=json.loads(raw);assert data['ownedProcessTerminated']
    archive=probe.with_suffix('.json.gz');archive.write_bytes(gzip.compress(raw,mtime=0));assert gzip.decompress(archive.read_bytes())==raw
    summary={k:v for k,v in data.items() if k in ('run','date','status','error','originalSha256','runtimeSha256','patch','desktop','input','outputs','nativeCompilations','ownedProcessTerminated','exitCodeBeforeCleanup','systemInput','usbOrPlc','ownMruCleanup')}
    summary['rawEvidence']=dict(file=archive.name,uncompressedBytes=len(raw),uncompressedSha256=hashlib.sha256(raw).hexdigest(),archiveSha256=sha(archive))
    duplicate=folder/('native-probe-'+data['run']+'.json')
    if duplicate.exists():assert duplicate.read_bytes()==raw;duplicate.unlink()
    write(probe,summary);archives.append(dict(run=folder.name,raw=summary['rawEvidence'],status=data['status'],error=data.get('error')))
write(native/'raw-archives.json',dict(archives=archives,fullRawPreserved=True))
retained={}
for p in base.rglob('*'):
    if p.is_file():retained.setdefault(sha(p),p)
vendor=Path('G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed')
for p in vendor.rglob('*'):
    # Only small copied runtime dependencies need a peer; do not enumerate image libraries into a cleanup manifest.
    if p.is_file() and (p.suffix.lower()=='.dll' or p.name in ('model.dat','modelInfo.dat','language.ini')):
        retained.setdefault(sha(p),p)
files=[]
runtimes=sorted((root/'native').glob('runtime-*'))+[old/'native-graphics/runtime-repeat-r1']
for directory in runtimes:
    assert directory.resolve().is_relative_to(root.resolve()) or directory.resolve()==(old/'native-graphics/runtime-repeat-r1').resolve()
    run=directory.name.removeprefix('runtime-')
    for p in directory.rglob('*'):
        if not p.is_file():continue
        digest=sha(p);peer=retained.get(digest)
        if peer is None and p.suffix.lower() in ('.sm','.dat','.kns','.bin'):
            peer=base/'retained-runtime-files'/run/p.relative_to(directory);peer.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,peer);retained[digest]=peer
        if peer is None and p.suffix.lower()=='.exe':
            shared=root/'native/shared'/p.name
            if shared.exists() and sha(shared)==digest:peer=shared
            else:
                shared=old/'native-graphics/shared'/p.name
                if shared.exists() and sha(shared)==digest:peer=shared
        files.append(dict(path=str(p),bytes=p.stat().st_size,sha256=digest,category='owned_native_runtime',retained=str(peer) if peer else None))
for directory,kind in [(root/'temp','owned_temporary'),(work/'build','regenerable_build_object')]:
    for p in directory.rglob('*'):
        if p.is_file() and (kind!='regenerable_build_object' or p.suffix in ('.obj','.o','.d')):
            files.append(dict(path=str(p),bytes=p.stat().st_size,sha256=sha(p),category=kind,retained=None))
plan=dict(roots=[str(x) for x in runtimes]+[str(root/'temp'),str(work/'build')],files=files,logicalBytes=sum(x['bytes'] for x in files),
    preserved='Sources, installed application/SDK, final executables, raw gzip, original unique sources/segments, user main build and all other tasks.')
write(base/'cleanup-plan.json',plan)
print(json.dumps(dict(rawArchives=len(archives),files=len(files),logicalBytes=plan['logicalBytes']),ensure_ascii=False))
