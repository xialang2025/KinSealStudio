"""Archive and remove only completed sector-corpus candidate intermediates after formal full-frame success."""
from pathlib import Path
import ctypes
import gzip
import hashlib
import json
import shutil

HERE=Path(__file__).resolve().parent
ROOT=HERE.parent
WORK=ROOT.parent/'kinseal-stage4-20261112/worktree'
BASE=WORK/'docs/stage5-evidence/native/sector'
TEMP=HERE/'temp'


def sha(raw):
    """Identify retained and removable bytes exactly."""
    return hashlib.sha256(raw).hexdigest()


def main():
    """Complete all archive roundtrips and path checks before deleting the first temporary file."""
    assert ROOT==Path('Z:/Project/vscode_prj_temp/REA/.scratch/kinseal-stage5-20261114')
    bsm=json.loads((BASE/'B-S-M-sector-cell-area-r1.json').read_text(encoding='utf-8'))
    assert len(bsm['cases'])==27 and bsm['allRasterComparisonsEqual'] and bsm['allStructuresValid']
    files=sorted(TEMP.iterdir())
    assert files and all(path.is_file() and path.suffix in ('.pathstream','.rgb565','.mask','.json') for path in files)
    archive=BASE/'supplementary-temp-archives-r1'
    archive.mkdir(exist_ok=True)
    entries=[]
    for path in files:
        assert path.resolve().parent==TEMP.resolve()
        current=path
        while current!=ROOT.parent:
            attributes=ctypes.windll.kernel32.GetFileAttributesW(str(current))
            assert attributes!=0xFFFFFFFF and not attributes&0x400
            current=current.parent
        raw=path.read_bytes()
        digest=sha(raw)
        retained=archive/(digest+'.gz')
        if retained.exists():
            assert gzip.decompress(retained.read_bytes())==raw
        else:
            retained.write_bytes(gzip.compress(raw,mtime=0))
        assert gzip.decompress(retained.read_bytes())==raw
        entries.append(dict(path=str(path),bytes=len(raw),sha256=digest,archive=str(retained.relative_to(BASE)),archiveSha256=sha(retained.read_bytes())))
    for entry in entries:
        path=Path(entry['path'])
        assert path.resolve().parent==TEMP.resolve() and len(path.read_bytes())==entry['bytes'] and sha(path.read_bytes())==entry['sha256']
        compressed=(BASE/entry['archive']).read_bytes()
        assert sha(compressed)==entry['archiveSha256'] and sha(gzip.decompress(compressed))==entry['sha256']
    for entry in entries:
        Path(entry['path']).unlink()
    result=dict(scope=str(TEMP),files=entries,removedFiles=len(entries),removedLogicalBytes=sum(entry['bytes'] for entry in entries),retainedUniqueGzipFiles=len({entry['archive'] for entry in entries}),allArchivesRoundtripVerified=True,allFilesPreflightBeforeFirstDeletion=True,tempDirectoryEmpty=not any(TEMP.iterdir()),nativeRuntimeTouched=False,cDriveFilesDeleted=0)
    (BASE/'supplementary-temp-cleanup-r1.json').write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    shutil.copy2(Path(__file__),BASE/'source-driver'/Path(__file__).name)
    print(json.dumps({key:value for key,value in result.items() if key!='files'},ensure_ascii=False,indent=2))


if __name__=='__main__':
    main()
