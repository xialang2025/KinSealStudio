"""Recheck native file digests and retain the exact ellipse corpus acquisition scripts."""
from pathlib import Path
import hashlib
import json
import shutil

HERE=Path(__file__).resolve().parent
ROOT=HERE.parent
WORK=ROOT.parent/'kinseal-stage4-20261112/worktree'
BASE=WORK/'docs/stage5-evidence/native/ellipse'


def digest(path):
    """Compute evidence identity from the complete retained file bytes."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    """Verify each process and fresh compiler artifact without reopening any application."""
    manifest=json.loads((HERE/'ellipse-canonical-run-r1.json').read_text(encoding='utf-8'))
    assert manifest['complete'] and manifest['passed'] and len(manifest['runs'])==27
    audit=dict(processes=[],verifiedCompiledFiles=0,verifiedNativeInputs=0,formalCompilations=27,preparationCompilations=1,failedDiagnosticProcesses=2,allOwnedProcessesTerminated=True,usbOrPlc=False,productCompilerAccepted=False)
    names=[sample['name'] for sample in manifest['runs']]+['ellipse-isolate-default-r1','ellipse-default-repeat-r1','ellipse-default-repeat-r2']
    for name in names:
        folder=BASE/name
        record=json.loads((folder/'native-probe.json').read_text(encoding='utf-8'))
        assert record['ownedProcessTerminated'] and not record['usbOrPlc'] and not record['systemInput']
        for compilation in record.get('nativeCompilations',[]):
            compiled=folder/'compiled'/compilation['label']
            assert digest(compiled/'input-native.sm')==compilation['sourceSha256']
            audit['verifiedNativeInputs']+=1
            for file in compilation['files']:
                retained=compiled/file['path']
                assert retained.stat().st_size==file['bytes'] and digest(retained)==file['sha256']
                audit['verifiedCompiledFiles']+=1
        audit['processes'].append(dict(name=name,status=record['status'],nativeCompilations=len(record.get('nativeCompilations',[])),pid=record['pid'],ownedProcessTerminated=record['ownedProcessTerminated'],exitCodeBeforeCleanup=record.get('exitCodeBeforeCleanup'),probeSha256=digest(folder/'native-probe.json'),runtime=str(ROOT/'native'/('runtime-'+name))))
    defaults=[sample for sample in manifest['runs'] if 'default-repeat' in sample['name']]
    assert len({sample['inputSourceSha256'] for sample in defaults})==1
    assert len({sample['sourceSha256'] for sample in defaults})==1
    audit['defaultInputSourceSha256']=defaults[0]['inputSourceSha256']
    audit['defaultCompiledSourceSha256']=defaults[0]['sourceSha256']
    scripts=BASE/'source-driver'
    scripts.mkdir(exist_ok=True)
    audit['driverCopies']=[]
    for source in sorted(HERE.iterdir()):
        if source.suffix in ('.py','.json','.log'):
            target=scripts/source.name
            if target.exists():
                assert digest(target)==digest(source)
            else:
                shutil.copy2(source,target)
            audit['driverCopies'].append(dict(path=str(target.relative_to(BASE)),bytes=target.stat().st_size,sha256=digest(target)))
    (BASE/'corpus-audit-r1.json').write_text(json.dumps(audit,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({key:value for key,value in audit.items() if key not in ('processes','driverCopies')},ensure_ascii=False,indent=2))


if __name__=='__main__':
    main()
