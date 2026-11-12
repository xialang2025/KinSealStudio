"""Archive ended native runs before producing a strictly scoped cleanup plan."""
from pathlib import Path
import argparse
import gzip
import hashlib
import json
import shutil

root = Path(__file__).resolve().parent
sha = lambda data: hashlib.sha256(data).hexdigest()


def write(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('family')
    parser.add_argument('runs', nargs='+')
    args = parser.parse_args()
    assert args.family in ('slider', 'flow', 'wheel', 'move-track')
    owner = root / ('native-' + ('track' if args.family == 'move-track' else args.family))
    base = root / 'worktree/docs/stage4-evidence' / args.family / 'native'
    rawdir = base / 'raw'
    rawdir.mkdir(exist_ok=True)
    reproduction = base / 'reproduction'
    reproduction.mkdir(exist_ok=True)
    manifest_path = base / 'native-manifest.json'
    manifest = json.loads(manifest_path.read_text(encoding='utf-8')) if manifest_path.exists() else dict(date='2026-11-12', family=args.family, records=[], nativeCompilations=[])
    records = manifest['records']
    plan = []
    for run in args.runs:
        path = base / ('native-probe-' + run + '.json')
        assert not any(item['path'] == path.name for item in records), 'Already archived: ' + run
        original = path.read_bytes()
        data = json.loads(original)
        assert data['ownedProcessTerminated'] and not data['systemInput'] and not data['usbOrPlc']
        archive = rawdir / (path.name + '.gz')
        assert not archive.exists()
        archive.write_bytes(gzip.compress(original, mtime=0))
        assert gzip.decompress(archive.read_bytes()) == original
        compact = {k: v for k, v in data.items() if k not in ('snapshots', 'menuInventory', 'steps', 'instrumentationMessages')}
        title = {'slider': '滑动开关', 'flow': '流动块', 'wheel': '滚轮控件', 'move-track': '移动轨迹'}[args.family]
        compact['snapshots'] = [dict(label=s['label'], windows=[w for w in s['windows'] if w['title'] == title]) for s in data['snapshots'] if not s['label'].startswith(('lookup', 'initial'))]
        compact['steps'] = [dict(request=s.get('request'), source=s.get('source')) for s in data['steps']]
        compact['compaction'] = dict(rawArchive=str(archive.relative_to(base)), rawSha256=sha(original), omitted='Complete original bytes are preserved in the verified gzip archive.')
        write(path, compact)
        records.append(dict(path=path.name, sha256=sha(path.read_bytes()), rawSha256=sha(original), status=data['status'], error=data.get('error'), rawArchive=str(archive.relative_to(base))))
        for capture in data.get('screenshots', []):
            assert sha((base / capture['path']).read_bytes()) == capture['sha256']
        for compilation in data.get('nativeCompilations', []):
            folder = base / 'compiled' / compilation['label']
            files = []
            for path in folder.iterdir():
                if not path.is_file():
                    continue
                item = dict(name=path.name, bytes=path.stat().st_size, sha256=sha(path.read_bytes()))
                # Preserve all nonempty DAT files, including previously unseen proprietary output.
                if path.name in {'AddrPara.dat', 'ControlPara.dat', 'ControlPixel.dat', 'PicLenPara.dat', 'SheetPara.dat', 'SheetPixel.dat', 'SysPara.dat'} or (path.suffix.lower() == '.dat' and item['bytes']):
                    item['retainedPath'] = str(path.relative_to(base))
                else:
                    item['omittedReproducible'] = True
                    plan.append(dict(path=str(path), bytes=item['bytes'], sha256=item['sha256'], reason='Reproducible compiler side output; exact identity retained.'))
                files.append(item)
            manifest['nativeCompilations'].append(dict(label=folder.name, files=files))
    for path in owner.iterdir():
        if path.is_file() and path.suffix in ('.py', '.json') and not path.name.startswith('native-cleanup'):
            shutil.copy2(path, reproduction / path.name)
    retained = {sha(p.read_bytes()): str(p) for p in base.rglob('*') if p.is_file() and p.suffix not in ('.gz',) and str(p) not in {x['path'] for x in plan}}
    for folder in owner.iterdir():
        if not folder.is_dir() or not (folder.name.startswith(('runtime-', 'host-temp-')) or (args.family == 'move-track' and folder.name == 'temp')):
            continue
        for path in folder.rglob('*'):
            if not path.is_file():
                continue
            digest = sha(path.read_bytes())
            if path.suffix.lower() == '.sm' and digest not in retained:
                sources = base / 'retained-sources'
                sources.mkdir(exist_ok=True)
                peer = sources / (digest[:12] + '-' + path.name)
                if not peer.exists():
                    shutil.copy2(path, peer)
                assert sha(peer.read_bytes()) == digest
                retained[digest] = str(peer)
            plan.append(dict(path=str(path), bytes=path.stat().st_size, sha256=digest, reason='Owned terminated runtime or Frida temporary file.', retained=retained.get(digest)))
    for name in ('native-probe.json', 'native-probe-checkpoint.json'):
        path = base / name
        if path.exists():
            original = path.read_bytes()
            digest = sha(original)
            archive = rawdir / (name + '.' + digest[:12] + '.gz')
            archive.write_bytes(gzip.compress(original, mtime=0))
            assert gzip.decompress(archive.read_bytes()) == original
            plan.append(dict(path=str(path), bytes=len(original), sha256=digest, reason='Redundant latest/checkpoint; exact bytes archived.', rawArchive=str(archive)))
    manifest.update(usbAccessed=False, systemInput=False, ownedProcessTerminated=True)
    write(manifest_path, manifest)
    destination = owner / ('native-cleanup-' + args.runs[-1] + '.json')
    write(destination, dict(ownerScope=str(owner), evidenceScope=str(base), files=plan))
    print(json.dumps(dict(plan=str(destination), files=len(plan), logicalBytes=sum(p['bytes'] for p in plan))))


if __name__ == '__main__':
    main()
