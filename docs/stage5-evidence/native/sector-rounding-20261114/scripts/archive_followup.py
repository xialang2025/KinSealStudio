"""Retain the complete follow-up evidence before deleting its seven regenerated temporary files."""
from pathlib import Path
import gzip
import hashlib
import io
import json
import tarfile

HERE = Path(__file__).resolve().parent
EVIDENCE = HERE.parent.parent / 'kinseal-stage4-20261112/worktree/docs/stage5-evidence/native/sector-rounding-20261114'


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def main():
    manifest_path = EVIDENCE / 'reproduction-manifest-r1.json'
    archive_path = EVIDENCE / 'sector-rounding-reproduction-r1.tar.gz'
    receipt_path = EVIDENCE / 'cleanup-r1.json'
    if any(path.exists() for path in (manifest_path, archive_path, receipt_path)):
        raise ValueError('Do not overwrite this archive/cleanup version.')
    scripts = EVIDENCE / 'scripts'
    scripts.mkdir(exist_ok=True)
    for name in ('verify_sector_rounding.py', 'historical_curve_reference.py', 'area_candidate_256.py', 'archive_followup.py'):
        source = HERE / name
        target = scripts / name
        if target.exists() and target.read_bytes() != source.read_bytes():
            raise ValueError('Script snapshot collision.')
        target.write_bytes(source.read_bytes())
    content = {}
    for path in sorted(EVIDENCE.rglob('*')):
        if path.is_file():
            if path.is_symlink() or not path.resolve().is_relative_to(EVIDENCE.resolve()):
                raise ValueError('Redirected follow-up evidence.')
            content[path.relative_to(EVIDENCE).as_posix()] = path.read_bytes()
    manifest = dict(files=[dict(path=name, bytes=len(raw), sha256=sha(raw)) for name, raw in content.items()],
                    scope='Default sector only; corrected integer blending; no production support claim.',
                    originalOnePixelFailureRetained=True, nativeProcessLaunched=False,
                    originalDllLoaded=False, usbAccessed=False, productionChanged=False)
    manifest_raw = (json.dumps(manifest, indent=2) + '\n').encode('utf-8')
    manifest_path.write_bytes(manifest_raw)
    content[manifest_path.name] = manifest_raw
    buffer = io.BytesIO()
    with tarfile.open(fileobj=buffer, mode='w', format=tarfile.PAX_FORMAT) as archive:
        for name, raw in sorted(content.items()):
            info = tarfile.TarInfo(name)
            info.size = len(raw)
            info.mtime = 0
            info.mode = 0o644
            archive.addfile(info, io.BytesIO(raw))
    compressed = gzip.compress(buffer.getvalue(), compresslevel=9, mtime=0)
    archive_path.write_bytes(compressed)
    with tarfile.open(fileobj=io.BytesIO(gzip.decompress(compressed)), mode='r:') as archive:
        if set(archive.getnames()) != set(content):
            raise ValueError('Archive member set differs.')
        for name, raw in content.items():
            if sha(archive.extractfile(name).read()) != sha(raw):
                raise ValueError('Archive member round-trip hash differs: ' + name)
    temporary = (HERE / 'temp').resolve()
    if temporary.parent != HERE.resolve():
        raise ValueError('Owned temporary directory resolves outside the follow-up root.')
    names = {'native.rgb565', 'independent.mask', 'independent.mask.pathstream'}
    for label in ('independent-qt6-arcTo45-span90', 'retained-native-eight-elements'):
        names.add(label + '.mask')
        names.add(label + '.mask.pathstream')
    if any(path.name not in names for path in temporary.iterdir()):
        raise ValueError('Unexpected temporary entry is not authorized for deletion.')
    entries = []
    for name in sorted(names):
        path = temporary / name
        if path.is_symlink() or path.resolve().parent != temporary or not path.is_file():
            raise ValueError('Missing or redirected expected temporary file: ' + str(path))
        raw = path.read_bytes()
        entries.append(dict(path=str(path), bytes=len(raw), sha256=sha(raw)))
    for item in entries:
        path = Path(item['path'])
        if sha(path.read_bytes()) != item['sha256']:
            raise ValueError('Temporary identity changed after preflight.')
        path.unlink()
    receipt = dict(archive=archive_path.name, archiveBytes=len(compressed), archiveSha256=sha(compressed),
                   archivedMembers=len(content), everyArchiveMemberRoundTripVerified=True,
                   deletedFiles=len(entries), deletedBytes=sum(item['bytes'] for item in entries),
                   remainingTemporaryFiles=[str(path) for path in temporary.iterdir()],
                   cDriveFilesDeleted=0, nativeProcessLaunched=False, originalDllLoaded=False,
                   usbAccessed=False, productionChanged=False)
    receipt_path.write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(receipt))


if __name__ == '__main__':
    main()
