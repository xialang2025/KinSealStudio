"""Archive only the 2026-11-14 sector investigation; deletion is a separate opt-in step."""
from pathlib import Path
import argparse
import ctypes
import gzip
import hashlib
import json
import os
import re
import shutil
import urllib.request

ROOT = Path(__file__).resolve().parent
EXPECTED_ROOT = Path('Z:/Project/vscode_prj_temp/REA/.scratch/kinseal-stage5-20261114').resolve()
WORK = ROOT.parent / 'kinseal-stage4-20261112/worktree'
BASE = WORK / 'docs/stage5-evidence/native/sector'
NATIVE = ROOT / 'native'
VENDOR = Path('G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed')
SHARED_EXE = ROOT.parent / 'kinseal-stage5-20261113/native/shared/KinSealStudio5.1-private.exe'
RUN_PATTERN = re.compile(r'sector-[A-Za-z0-9][A-Za-z0-9-]*\Z')
PLAN_NAME = 'cleanup-plan-sector-20261114.json'
REPRO = BASE / 'reproduction-20261114'


def sha(path):
    """Hash a file without loading large runtime artifacts into memory."""
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(chunk)
    return digest.hexdigest()


def write_json(path, value):
    """Replace an owned manifest atomically after writing all bytes."""
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + '.sector-archive-writing')
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    os.replace(temporary, path)


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def reject_redirects(path, boundary):
    """Refuse junctions and symlinks in an existing owned path before resolving it."""
    absolute = Path(os.path.abspath(path))
    boundary = Path(os.path.abspath(boundary))
    if not absolute.is_relative_to(boundary):
        raise ValueError('Lexical path outside boundary: ' + str(absolute))
    current = absolute
    while True:
        if current.exists():
            attributes = ctypes.windll.kernel32.GetFileAttributesW(str(current))
            if attributes == 0xFFFFFFFF or attributes & 0x400:
                raise ValueError('Redirected or unreadable path: ' + str(current))
        if current == boundary:
            break
        current = current.parent
    resolved = absolute.resolve()
    if not resolved.is_relative_to(boundary.resolve()):
        raise ValueError('Resolved path outside boundary: ' + str(resolved))
    return resolved


def gzip_bytes(raw, destination):
    """Write deterministic gzip bytes and verify their full decompressed identity."""
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.exists():
        if gzip.decompress(destination.read_bytes()) != raw:
            raise ValueError('Existing gzip contains different bytes: ' + str(destination))
    else:
        temporary = destination.with_name(destination.name + '.sector-archive-writing')
        temporary.write_bytes(gzip.compress(raw, compresslevel=9, mtime=0))
        if gzip.decompress(temporary.read_bytes()) != raw:
            raise ValueError('Gzip round-trip failed: ' + str(destination))
        os.replace(temporary, destination)
    return dict(file=destination.name, uncompressedBytes=len(raw),
                uncompressedSha256=hashlib.sha256(raw).hexdigest(), archiveSha256=sha(destination))


def original_probe(path):
    """Read either a complete raw probe or a previously archived probe wrapper."""
    summary = read_json(path)
    if 'rawEvidence' not in summary:
        return summary, path.read_bytes()
    reference = summary['rawEvidence']
    archive = path.parent / reference['file']
    if archive.parent.resolve() != path.parent.resolve() or sha(archive) != reference['archiveSha256']:
        raise ValueError('Invalid raw archive reference: ' + str(path))
    raw = gzip.decompress(archive.read_bytes())
    if hashlib.sha256(raw).hexdigest() != reference['uncompressedSha256'] or len(raw) != reference['uncompressedBytes']:
        raise ValueError('Archived raw probe identity changed: ' + str(path))
    return json.loads(raw), raw


def summarize_probe(data, reference):
    names = ('date', 'startedAtUtc', 'run', 'status', 'error', 'originalSha256',
             'runtimeSha256', 'patch', 'desktop', 'input', 'outputs', 'createdSources',
             'saveAs', 'workingCopyAfter', 'nativeCompilations', 'ownedProcessTerminated',
             'exitCodeBeforeCleanup', 'systemInput', 'usbOrPlc', 'ownMruCleanup')
    summary = {key: data[key] for key in names if key in data}
    summary['rawEvidence'] = reference
    return summary


def collect_runs():
    """Skip every unproven runtime rather than broadening the deletion allowlist."""
    approved, skipped = [], []
    for folder in sorted(BASE.iterdir()):
        if not folder.is_dir() or not RUN_PATTERN.fullmatch(folder.name):
            continue
        probe = folder / 'native-probe.json'
        if not probe.is_file():
            skipped.append(dict(run=folder.name, reason='final native-probe.json absent'))
            continue
        try:
            data, raw = original_probe(probe)
            if data.get('run') != folder.name:
                raise ValueError('run name differs from evidence directory')
            if data.get('ownedProcessTerminated') is not True:
                raise ValueError('ownedProcessTerminated is not exactly true')
            if data.get('usbOrPlc') is not False:
                raise ValueError('usbOrPlc is not exactly false')
            runtime = ROOT / 'native' / ('runtime-' + folder.name)
            reject_redirects(runtime, ROOT)
            if runtime.parent.resolve() != NATIVE.resolve() or not runtime.name.startswith('runtime-sector-'):
                raise ValueError('runtime outside exact new-date sector roots')
            approved.append(dict(folder=folder, probe=probe, data=data, raw=raw, runtime=runtime))
        except (OSError, ValueError, KeyError, json.JSONDecodeError) as error:
            skipped.append(dict(run=folder.name, reason=str(error)))
    return approved, skipped


def archive_public_sources():
    """Verify and reference the already retained polygon diagnostic sources and licenses."""
    original_root=WORK/'docs/stage5-evidence/native/polygon/reproduction-20261114'
    original_manifest=original_root/'public-qt-sources-manifest.json'
    data=read_json(original_manifest)
    entries=[]
    licenses=set()
    for entry in data['sourcesAndLicenses']:
        retained=original_root/entry['path']
        if not retained.resolve().is_relative_to(original_root.resolve()) or sha(retained)!=entry['sha256']:
            raise ValueError('Previously retained diagnostic source/license changed: '+str(retained))
        if entry['kind']=='upstream_license':
            licenses.add(retained.name)
        entries.append(dict(**entry,retainedPath=str(retained.resolve())))
    if not {'LICENSE.LGPL','LICENSE.GPL','LGPL_EXCEPTION.txt','FTL.TXT'}.issubset(licenses):
        raise ValueError('Previously retained Qt/FreeType license set is incomplete')
    write_json(REPRO/'public-qt-sources-manifest.json',dict(
        scope='Reference-only reuse of retained polygon Qt4 diagnostic sources/licenses; no production dependency',
        reusedManifest=dict(path=str(original_manifest.resolve()),sha256=sha(original_manifest),sourceRoot=str(original_root.resolve())),
        sourcesAndLicenses=entries,originalLicenseHeadersPreserved=True,productionDependency=False))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archive', action='store_true', help='Write verified archives and a cleanup plan; never delete files.')
    args = parser.parse_args()
    if ROOT != EXPECTED_ROOT:
        raise ValueError('Archive script is not in the fixed new-date task root')
    reject_redirects(ROOT, ROOT)
    approved, skipped = collect_runs()
    if not args.archive:
        print(json.dumps(dict(mode='read_only_preview', approvedRuns=[entry['data']['run'] for entry in approved],
                              skippedRuns=skipped, deletionPerformed=False, cDriveCleanupPlanned=False), ensure_ascii=False, indent=2))
        return
    if not approved:
        raise ValueError('No completed, owned, USB-free sector run was proved')
    archive_public_sources()  # License retention must succeed before preparing any deletion plan.
    peers = {}
    for path in sorted(BASE.rglob('*')):
        if path.is_file() and not path.name.endswith('.sector-archive-writing'):
            peers.setdefault(sha(path), path)
    if SHARED_EXE.is_file():
        peers.setdefault(sha(SHARED_EXE), SHARED_EXE)  # Read-only peer; never a cleanup root.
    files, archives, roots, proofs, logs = [], [], [], [], []
    for entry in approved:
        folder, data, raw, runtime = (entry[key] for key in ('folder', 'data', 'raw', 'runtime'))
        reference = gzip_bytes(raw, folder / 'native-probe.json.gz')
        summary = summarize_probe(data, reference)
        write_json(entry['probe'], summary)
        duplicate = folder / ('native-probe-' + data['run'] + '.json')
        if duplicate.is_file():
            duplicate_data, duplicate_raw = original_probe(duplicate)
            if duplicate_raw != raw or duplicate_data.get('run') != data['run']:
                raise ValueError('Duplicate raw probe is not identical: ' + str(duplicate))
            write_json(duplicate, dict(run=data['run'], duplicateOf='native-probe.json', rawEvidence=reference))
        checkpoint = folder / 'native-probe-checkpoint.json'
        if checkpoint.is_file():
            checkpoint_data, checkpoint_raw = original_probe(checkpoint)
            checkpoint_reference = gzip_bytes(checkpoint_raw, folder / 'native-probe-checkpoint.json.gz')
            write_json(checkpoint, dict(run=data['run'], status=checkpoint_data.get('status'), rawEvidence=checkpoint_reference))
        archives.append(dict(run=data['run'], status=data.get('status'), raw=reference))
        proof = dict(run=data['run'], runtime=str(runtime.resolve()), probe=str(entry['probe'].resolve()),
                     rawArchive=str((folder / reference['file']).resolve()), rawEvidence=reference,
                     ownedProcessTerminated=True, usbOrPlc=False)
        proofs.append(proof)
        if not runtime.is_dir():
            continue
        roots.append(str(runtime.resolve()))
        for path in sorted(runtime.rglob('*')):
            if not path.is_file():
                continue
            path = reject_redirects(path, ROOT)
            digest, size = sha(path), path.stat().st_size
            peer = peers.get(digest)
            # Probe wrappers are compacted after the peer index is built; never trust a stale entry.
            if peer is not None and (not peer.is_file() or sha(peer) != digest):
                peer = None
            if peer is None:
                vendor_peer = VENDOR / path.relative_to(runtime.resolve())
                if vendor_peer.is_file() and sha(vendor_peer) == digest:
                    peer = vendor_peer
                    peers[digest] = peer
            retained_encoding = 'identity'
            if peer is None:
                if path.suffix.lower() in ('.sm', '.dat'):
                    peer = BASE / 'retained-runtime-files-20261114' / digest[:16] / path.name
                    peer.parent.mkdir(parents=True, exist_ok=True)
                    if not peer.exists():
                        shutil.copy2(path, peer)
                    if sha(peer) != digest:
                        raise ValueError('Unique source/segment retention failed: ' + str(path))
                    peers[digest] = peer
                else:
                    peer = BASE / 'retained-runtime-gzip-20261114' / (digest + '.gz')
                    gzip_bytes(path.read_bytes(), peer)
                    retained_encoding = 'gzip'
            files.append(dict(path=str(path), bytes=size, sha256=digest, retained=str(peer.resolve()),
                              retainedEncoding=retained_encoding, retainedArchiveSha256=sha(peer),
                              category='owned_sector_runtime', proofRun=data['run']))
        log = ROOT / (data['run'] + '.log')
        if log.is_file():
            log = reject_redirects(log, ROOT)
            retained = REPRO / 'logs' / (sha(log) + '.log.gz')
            log_reference = gzip_bytes(log.read_bytes(), retained)
            logs.append(dict(run=data['run'], original=str(log), archive=str(retained.resolve()), **log_reference))
            files.append(dict(path=str(log), bytes=log.stat().st_size, sha256=sha(log), retained=str(retained.resolve()),
                              retainedEncoding='gzip', retainedArchiveSha256=sha(retained), category='owned_sector_log', proofRun=data['run']))
    script_names = ('native_probe.py', 'native_paint_probe.py', 'legacy_coverage_probe.cpp',
                    'archive_sector.py', 'execute_sector_cleanup.ps1')
    copied = []
    for name in script_names:
        source = ROOT / name
        if not source.is_file():
            continue
        target = REPRO / 'tools' / name
        target.parent.mkdir(parents=True, exist_ok=True)
        if target.exists() and target.read_bytes() != source.read_bytes():
            raise ValueError('Previously archived script changed; use a new evidence version: ' + str(source))
        if not target.exists():
            shutil.copy2(source, target)
        copied.append(dict(path=str(target.relative_to(BASE)), sha256=sha(target), bytes=target.stat().st_size))
    for source in sorted(ROOT.glob('sector*.json')):
        target = REPRO / 'scenarios-and-progress' / source.name
        target.parent.mkdir(parents=True, exist_ok=True)
        if target.exists() and target.read_bytes() != source.read_bytes():
            target = target.with_name(target.stem + '-' + sha(source)[:12] + target.suffix)
        if not target.exists():
            shutil.copy2(source, target)
        copied.append(dict(path=str(target.relative_to(BASE)), sha256=sha(target), bytes=target.stat().st_size))
    for source in sorted((ROOT/'sector_corpus').iterdir()):
        if not source.is_file() or source.suffix not in ('.py','.json','.log'):
            continue
        target=REPRO/'sector-corpus'/source.name
        target.parent.mkdir(parents=True,exist_ok=True)
        if target.exists() and target.read_bytes()!=source.read_bytes():
            target=target.with_name(target.stem+'-'+sha(source)[:12]+target.suffix)
        if not target.exists():
            shutil.copy2(source,target)
        copied.append(dict(path=str(target.relative_to(BASE)),sha256=sha(target),bytes=target.stat().st_size))
    write_json(BASE / 'raw-archives-20261114.json', dict(archives=archives, scriptsAndScenarios=copied,
              logs=logs, skippedRuns=skipped, fullRawPreserved=True, gzipRoundTripVerified=True))
    plan = dict(schema=1, family='sector', taskRoot=str(ROOT), runtimeParent=str(NATIVE.resolve()),
                roots=roots, proofs=proofs, files=files, logicalBytes=sum(item['bytes'] for item in files),
                publicSourcesManifest=dict(path=str((REPRO / 'public-qt-sources-manifest.json').resolve()),
                                           sha256=sha(REPRO / 'public-qt-sources-manifest.json')),
                skippedRuns=skipped, cDriveCleanupPlanned=False, decodedFramesDeletionPlanned=False,
                preserved='Unique .sm/.dat and all unique runtime bytes retained, raw/checkpoint gzip verified; failures, scenes, JSON, public licenses, old shared, installation, SDK and incremental builds untouched.',
                deletionPerformed=False)
    write_json(BASE / PLAN_NAME, plan)
    print(json.dumps(dict(mode='archived_plan_only', approvedRuns=len(approved), skippedRuns=skipped,
                          files=len(files), logicalBytes=plan['logicalBytes'], plan=str(BASE / PLAN_NAME), deletionPerformed=False), ensure_ascii=False, indent=2))


if __name__ == '__main__':
    main()
