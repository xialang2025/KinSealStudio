"""Idempotently retain complete raw probes before planning only this family's runtime/decoded-file cleanup."""
from pathlib import Path
import argparse
import gzip
import hashlib
import json
import shutil

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
parser=argparse.ArgumentParser();parser.add_argument('--family',choices=['table','rect','audit','line','freeline','roundrect'],required=True);parser.add_argument('--bootstrap-run');args=parser.parse_args()
base=work/'docs/stage5-evidence/native'/args.family
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
write=lambda p,d:p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
archives=[];roots=[]
for folder in sorted(base.iterdir()):
    probe=folder/'native-probe.json'
    if not probe.is_file():continue
    data=read(probe);assert data['ownedProcessTerminated'] and not data['usbOrPlc']
    archive=probe.with_suffix('.json.gz')
    if 'rawEvidence' in data:
        raw=gzip.decompress(archive.read_bytes());assert hashlib.sha256(raw).hexdigest()==data['rawEvidence']['uncompressedSha256']
        original=json.loads(raw)
    else:
        raw=probe.read_bytes();original=data
        if archive.exists():assert gzip.decompress(archive.read_bytes())==raw
        else:archive.write_bytes(gzip.compress(raw,mtime=0))
        assert gzip.decompress(archive.read_bytes())==raw
        summary={k:v for k,v in data.items() if k in ('run','date','startedAtUtc','status','error','originalSha256','runtimeSha256','patch','desktop','input','outputs','nativeCompilations','ownedProcessTerminated','exitCodeBeforeCleanup','systemInput','usbOrPlc','ownMruCleanup')}
        summary['rawEvidence']=dict(file=archive.name,uncompressedBytes=len(raw),uncompressedSha256=hashlib.sha256(raw).hexdigest(),archiveSha256=sha(archive))
        write(probe,summary);data=summary
    duplicate=folder/('native-probe-'+original['run']+'.json')
    if duplicate.exists():assert duplicate.read_bytes()==raw;duplicate.unlink()
    checkpoint=folder/'native-probe-checkpoint.json'
    if checkpoint.exists():
        check_raw=checkpoint.read_bytes();check_archive=checkpoint.with_suffix('.json.gz')
        if check_archive.exists():assert gzip.decompress(check_archive.read_bytes())==check_raw
        else:check_archive.write_bytes(gzip.compress(check_raw,mtime=0))
        assert gzip.decompress(check_archive.read_bytes())==check_raw;checkpoint.unlink()
    archives.append(dict(run=folder.name,raw=data['rawEvidence'],status=data['status']))
    runtime=root/'native'/('runtime-'+original['run'])
    if runtime.exists():roots.append(runtime.resolve())
if args.bootstrap_run:
    assert args.family=='rect' and args.bootstrap_run=='rect-resized-small-reopen-r1'
    runtime=root/'native'/('runtime-'+args.bootstrap_run)
    assert runtime.is_dir() and not (base/args.bootstrap_run/'native-probe.json').exists()
    trace=root/(args.bootstrap_run+'.log');raw_trace=trace.read_bytes();trace_text=raw_trace.decode('utf-8',errors='replace')
    assert 'FileNotFoundError' in trace_text and 'shutil.copy2(args.input, source)' in trace_text
    saved=base/'bootstrap-failure';saved.mkdir(exist_ok=True);shutil.copy2(trace,saved/'trace.log')
    write(saved/'failure.json',dict(run=args.bootstrap_run,classification='input_missing_before_CreateProcess',evidenceOrigin='retained_actual_stderr_and_source_order',traceSha256=sha(trace),runtime=str(runtime),processCreated=False,usbAccessed=False,scriptVersionBeforeGuardFix='docs/stage5-evidence/native/table/reproduction/native_probe.py'))
    roots.append(runtime.resolve())
peers={}
for directory in (base,root/'native/shared'):
    for p in directory.rglob('*'):
        if p.is_file():peers.setdefault(sha(p),p)
vendor=Path('G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed')
for p in vendor.rglob('*'):
    if p.is_file() and (p.suffix.lower()=='.dll' or p.name in ('model.dat','modelInfo.dat','language.ini')):peers.setdefault(sha(p),p)
files=[]
for directory in roots:
    #20261113-16 change: Only the six recorded passive paint probes extend the audit runtime allowlist.
    paint_runs={'paint-roundrect-r1','paint-ellipse-r1','paint-polygon-r1','paint-sector-r1','paint-roundrect-engine-r2','paint-roundrect-stroke-r3'}
    assert directory.is_relative_to((root/'native').resolve()) and (directory.name.startswith('runtime-'+args.family+'-') or (args.family=='audit' and directory.name in {'runtime-'+run for run in paint_runs}))
    for p in directory.rglob('*'):
        if not p.is_file():continue
        digest=sha(p);peer=peers.get(digest)
        if peer is None:
            peer=base/'retained-runtime-files'/directory.name/p.relative_to(directory);peer.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,peer);peers[digest]=peer
        files.append(dict(path=str(p.resolve()),bytes=p.stat().st_size,sha256=digest,retained=str(peer.resolve()),category='owned_native_runtime'))
roots.append((root/'temp').resolve())
for p in (root/'temp').glob(args.family+'*.rgb565'):
    files.append(dict(path=str(p.resolve()),bytes=p.stat().st_size,sha256=sha(p),retained=None,category='regenerable_decoded_frame'))
write(base/'raw-archives.json',dict(archives=archives,fullRawPreserved=True,idempotenceChecks=True))
write(base/'cleanup-plan.json',dict(family=args.family,roots=list(map(str,roots)),files=files,logicalBytes=sum(f['bytes'] for f in files),preserved='Unique source and segments, raw gzip and checkpoints, all builds still needed for the next slices, packages, main user work, native installation and SDK.'))
print(json.dumps(dict(family=args.family,rawArchives=len(archives),files=len(files),logicalBytes=sum(f['bytes'] for f in files)),ensure_ascii=False))
