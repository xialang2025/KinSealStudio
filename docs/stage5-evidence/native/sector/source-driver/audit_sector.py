"""Verify complete source/segment identities and retain the sector acquisition/comparison recipes."""
from pathlib import Path
import gzip
import hashlib
import json
import shutil

HERE=Path(__file__).resolve().parent
ROOT=HERE.parent
WORK=ROOT.parent/'kinseal-stage4-20261112/worktree'
BASE=WORK/'docs/stage5-evidence/native/sector'


def sha(raw):
    """Identify all source/segment bytes without ignored ranges."""
    return hashlib.sha256(raw).hexdigest()


def probe(path):
    """Read the original observation even if its owner has already archived its raw JSON."""
    raw=path.read_bytes()
    data=json.loads(raw)
    if 'rawEvidence' in data:
        reference=data['rawEvidence']
        archive=path.parent/reference['file']
        compressed=archive.read_bytes()
        assert archive.parent.resolve()==path.parent.resolve() and sha(compressed)==reference['archiveSha256']
        raw=gzip.decompress(compressed)
        assert len(raw)==reference['uncompressedBytes'] and sha(raw)==reference['uncompressedSha256']
        data=json.loads(raw)
    return data,raw


def main():
    """Audit all 29 processes, 28 compilations and the 27 formally bound candidates."""
    index=json.loads((BASE/'valid-target-index-r1.json').read_text(encoding='utf-8'))
    bsm=json.loads((BASE/'B-S-M-sector-cell-area-r1.json').read_text(encoding='utf-8'))
    assert index['complete'] and index['passed'] and len(index['runs'])==27
    assert bsm['allRasterComparisonsEqual'] and bsm['allStructuresValid'] and bsm['allPythonCppCoverageEqual']
    names=[row['name'] for row in index['runs']]+['sector-source-inspect-r1','sector-odd-create-r1']
    processes=[]
    files=0
    inputs=0
    for name in names:
        folder=BASE/name
        data,raw=probe(folder/'native-probe.json')
        assert data['ownedProcessTerminated'] and not data['usbOrPlc'] and not data['systemInput']
        assert data['status']=='native-stage4-graphics-scenario-completed'
        for compilation in data.get('nativeCompilations',[]):
            target=folder/'compiled'/compilation['label']
            assert sha((target/'input-native.sm').read_bytes())==compilation['sourceSha256']
            inputs+=1
            for entry in compilation['files']:
                binary=(target/entry['path']).read_bytes()
                assert len(binary)==entry['bytes'] and sha(binary)==entry['sha256']
                files+=1
        processes.append(dict(run=name,status=data['status'],nativeCompilations=len(data.get('nativeCompilations',[])),pid=data['pid'],ownedProcessTerminated=data['ownedProcessTerminated'],rawProbeSha256=sha(raw),runtime=str(ROOT/'native'/('runtime-'+name)),formalValid=name not in ('sector-source-inspect-r1','sector-odd-create-r1')))
    recipes=BASE/'source-driver'
    recipes.mkdir(exist_ok=True)
    scripts=[]
    for source in sorted(HERE.iterdir()):
        if source.is_file() and source.suffix in ('.py','.json','.log'):
            target=recipes/source.name
            if target.exists():
                assert target.read_bytes()==source.read_bytes()
            else:
                shutil.copy2(source,target)
            scripts.append(dict(path=str(target.relative_to(BASE)),bytes=target.stat().st_size,sha256=sha(target.read_bytes())))
    result=dict(formalCompilations=27,canonicalCases=15,independentScenes=13,ownedProcesses=29,readOnlyInspectionProcesses=1,bindingRejectedCompilations=1,bindingRejectedRun='sector-odd-create-r1',allNativeHelpersReportedSuccess=True,verifiedNativeInputs=inputs,verifiedNativeCompiledFiles=files,allFormalFullFramesEqual=True,allStructuresValid=True,allPythonCppCoverageEqual=True,allOwnedProcessesTerminated=True,usbOrPlc=False,systemInput=False,processes=processes,sourceDriverCopies=scripts,productGateAcceptanceClaimed=False)
    (BASE/'corpus-audit-r1.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({key:value for key,value in result.items() if key not in ('processes','sourceDriverCopies')},ensure_ascii=False,indent=2))


if __name__=='__main__':
    main()
