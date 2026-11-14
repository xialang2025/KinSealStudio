"""Verify ellipse evidence remains complete after exact-whitelist runtime cleanup."""
from pathlib import Path
import gzip
import hashlib
import json

HERE=Path(__file__).resolve().parent
ROOT=HERE.parent
WORK=ROOT.parent/'kinseal-stage4-20261112/worktree'
BASE=WORK/'docs/stage5-evidence/native/ellipse'


def read(path):
    """Load one retained UTF-8 manifest."""
    return json.loads(path.read_text(encoding='utf-8-sig'))


def sha(raw):
    """Verify the full identity of retained binary evidence."""
    return hashlib.sha256(raw).hexdigest()


def main():
    """Audit raw observations, native B files, failed frames and independent path archives."""
    archive=read(BASE/'raw-archives-20261114.json')
    plan=read(BASE/'cleanup-plan-ellipse-20261114.json')
    receipt=read(BASE/'cleanup-ellipse-20261114.json')
    assert len(archive['archives'])==30 and not plan['skippedRuns']
    assert receipt['deletedFiles']==len(plan['files'])==873 and receipt['deletedLogicalBytes']==628781756
    assert all(not Path(path).exists() for path in plan['roots'])
    for entry in archive['archives']:
        reference=entry['raw']
        compressed=(BASE/entry['run']/reference['file']).read_bytes()
        assert sha(compressed)==reference['archiveSha256']
        raw=gzip.decompress(compressed)
        assert len(raw)==reference['uncompressedBytes'] and sha(raw)==reference['uncompressedSha256']
    formal=read(BASE/'B-S-M-ellipse-cell-area-r2.json')
    verified_b=0
    for case in formal['cases']:
        for file in case['B']:
            raw=(WORK/case['folder']/file['name']).read_bytes()
            assert len(raw)==file['bytes'] and sha(raw)==file['sha256']
            verified_b+=1
    previous=read(BASE/'B-S-M-ellipse-cell-area-r1.json')
    failure_frames=0
    for case in previous['cases']:
        if case['M']['differentPixels']:
            for kind,expected in (('native',case['M']['nativeSha256']),('candidate',case['M']['candidateSha256'])):
                raw=gzip.decompress((BASE/'independent-failures-r1'/(case['run']+'-'+kind+'.rgb565.gz')).read_bytes())
                assert len(raw)==768000 and sha(raw)==expected
                failure_frames+=1
    paths=0
    for version in ('r1','r2'):
        for entry in read(BASE/('independent-temp-cleanup-'+version+'.json'))['files']:
            compressed=(BASE/entry['archive']).read_bytes()
            raw=gzip.decompress(compressed)
            assert sha(compressed)==entry['archiveSha256'] and len(raw)==entry['bytes'] and sha(raw)==entry['sha256']
            paths+=1
    scripts=[]
    for name in ('analyze_ellipse-r1.py','analyze_ellipse-r2.py','analyze_ellipse-r2-archiveaware.py','explain_rounding.py','remove_empty_runtimes.ps1'):
        source=BASE/'source-driver'/name
        scripts.append(dict(path=str(source.relative_to(BASE)),sha256=sha(source.read_bytes()),bytes=source.stat().st_size))
    result=dict(rawProbeGzipRoundtrips=30,nativeBFilesVerified=verified_b,failedFramesPreserved=failure_frames,independentPathsAndDetailsRoundtrips=paths,allFormalR2FramesEqual=formal['allRasterComparisonsEqual'],r1FailurePreserved=not previous['allRasterComparisonsEqual'],allThirtyRuntimeRootsAbsent=True,deletedFiles=receipt['deletedFiles'],deletedLogicalBytes=receipt['deletedLogicalBytes'],cDriveFilesDeleted=receipt['cDriveFilesDeleted'],skippedRuns=plan['skippedRuns'],extraReproductionScripts=scripts,verified=True)
    (BASE/'final-preservation-audit-r2.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(result,ensure_ascii=False,indent=2))


if __name__=='__main__':
    main()
