"""Retain one exact gzip for each unique owned Frida extraction byte sequence; never delete files."""
from pathlib import Path
import base64
import csv
import ctypes
import gzip
import hashlib
import json
import os
import re
import shutil

ROOT=Path('Z:/Project/vscode_prj_temp/REA/.scratch/kinseal-stage5-20261114')
TEMP=ROOT/'temp'
WORK=ROOT.parent/'kinseal-stage4-20261112/worktree'
BASE=WORK/'docs/stage5-evidence/native/sector/reproduction-20261114/owned-frida-temp'
INSTALL=Path('G:/01_Professional/Frida/Installed')
PACKAGE=INSTALL/'frida-17.6.2.dist-info'
PATTERN=re.compile(r'frida-[0-9a-fA-F]{32}\Z')


def sha(path):
    """Hash complete bytes in bounded streaming reads."""
    digest=hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda:stream.read(1024*1024),b''):
            digest.update(block)
    return digest.hexdigest()


def guarded(path):
    """Reject reparse points and any lexical or resolved path outside this exact task temp."""
    absolute=Path(os.path.abspath(path))
    assert absolute.is_relative_to(TEMP)
    current=absolute
    while current!=ROOT.parent:
        attributes=ctypes.windll.kernel32.GetFileAttributesW(str(current))
        assert attributes not in (-1,0xFFFFFFFF) and not attributes&0x400
        current=current.parent
    assert absolute.resolve().is_relative_to(TEMP.resolve())
    return absolute


def main():
    """Archive full identities and installation provenance before proposing the exact file whitelist."""
    assert Path(__file__).resolve().parent.parent==ROOT
    BASE.mkdir(parents=True,exist_ok=True)
    snapshot=json.loads((BASE/'process-preflight.json').read_text(encoding='utf-8-sig'))
    assert snapshot['kinsealProcesses']==[] and snapshot['tempProcesses']==[]
    metadata=(PACKAGE/'METADATA').read_text(encoding='utf-8')
    assert '\nName: frida\n' in metadata and '\nVersion: 17.6.2\n' in metadata
    pyd=INSTALL/'frida/_frida.pyd'
    pyd_sha=sha(pyd)
    record=list(csv.reader((PACKAGE/'RECORD').read_text(encoding='utf-8').splitlines()))
    pyd_record=next(row for row in record if row[0]=='frida/_frida.pyd')
    assert pyd_record[1].startswith('sha256=')
    expected=base64.urlsafe_b64decode(pyd_record[1].split('=',1)[1]+'=').hex()
    assert pyd_sha==expected and pyd.stat().st_size==int(pyd_record[2])
    provenance=[]
    for source in (PACKAGE/'METADATA',PACKAGE/'RECORD',PACKAGE/'WHEEL',PACKAGE/'licenses/COPYING',INSTALL/'frida/__init__.py'):
        target=BASE/'installation-metadata'/source.relative_to(INSTALL)
        target.parent.mkdir(parents=True,exist_ok=True)
        if target.exists():
            assert sha(target)==sha(source)
        else:
            shutil.copy2(source,target)
        provenance.append(dict(original=str(source),retained=str(target),bytes=source.stat().st_size,sha256=sha(source)))
    roots=sorted(path for path in TEMP.iterdir() if path.is_dir() and PATTERN.fullmatch(path.name))
    assert roots
    files=[]
    uniques={}
    for root_index,root in enumerate(roots,1):
        guarded(root)
        for directory,subdirectories,names in os.walk(root,followlinks=False):
            for name in subdirectories:
                guarded(Path(directory)/name)
            for name in sorted(names):
                path=guarded(Path(directory)/name)
                assert path.is_file() and path.suffix.lower() in ('.dll','.exe')
                digest=sha(path)
                if digest not in uniques:
                    destination=BASE/'unique-bytes'/(digest+'.gz')
                    destination.parent.mkdir(parents=True,exist_ok=True)
                    raw=path.read_bytes()
                    assert hashlib.sha256(raw).hexdigest()==digest
                    if destination.exists():
                        assert gzip.decompress(destination.read_bytes())==raw
                    else:
                        destination.write_bytes(gzip.compress(raw,compresslevel=9,mtime=0))
                    decoded=gzip.decompress(destination.read_bytes())
                    assert hashlib.sha256(decoded).hexdigest()==digest and len(decoded)==path.stat().st_size
                    uniques[digest]=dict(path=str(destination),uncompressedSha256=digest,uncompressedBytes=len(raw),archiveBytes=destination.stat().st_size,archiveSha256=sha(destination))
                archive=uniques[digest]
                files.append(dict(path=str(path),root=str(root),relative=str(path.relative_to(root)),bytes=path.stat().st_size,sha256=digest,lastWriteTimeNs=path.stat().st_mtime_ns,archive=archive['path'],archiveSha256=archive['archiveSha256']))
        if root_index%20==0:
            print(json.dumps(dict(verifiedDirectories=root_index,totalDirectories=len(roots),files=len(files))),flush=True)
    plan=dict(schema=1,scope='shared_owned_frida_extractions_only_not_sector_native_runtimes',taskRoot=str(ROOT),tempRoot=str(TEMP),roots=list(map(str,roots)),rootNamePattern='^frida-[0-9a-fA-F]{32}$',files=files,fileCount=len(files),logicalBytes=sum(entry['bytes'] for entry in files),uniqueBytes=list(uniques.values()),uniqueByteSequences=len(uniques),uniqueLogicalBytes=sum(entry['uncompressedBytes'] for entry in uniques.values()),archiveBytes=sum(entry['archiveBytes'] for entry in uniques.values()),fridaVersion='17.6.2',installationPydIdentity=dict(path=str(pyd),bytes=pyd.stat().st_size,sha256=pyd_sha,wheelRecordSha256=expected,installedRecordVerified=True,modified=False),installationMetadata=provenance,processPreflight=str(BASE/'process-preflight.json'),allGzipRoundtripsVerified=True,cDriveFilesDeletionPlanned=False,preservedOtherTempEntries=[path.name for path in TEMP.iterdir() if not (path.is_dir() and PATTERN.fullmatch(path.name))],deletionPerformed=False)
    (BASE/'cleanup-plan.json').write_text(json.dumps(plan,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps({key:plan[key] for key in ('scope','fileCount','logicalBytes','uniqueByteSequences','uniqueLogicalBytes','archiveBytes','preservedOtherTempEntries','deletionPerformed')},ensure_ascii=False,indent=2))


if __name__=='__main__':
    main()
