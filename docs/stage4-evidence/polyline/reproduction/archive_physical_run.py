"""Keep full physical-run provenance and payloads, then plan deletion of only that ended runtime and host temp."""
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
    parser.add_argument('run')
    args = parser.parse_args()
    base = root / 'worktree/docs/stage4-evidence' / args.family / 'physical' / args.run
    assert base.resolve().is_relative_to(root / 'worktree/docs/stage4-evidence')
    record = json.loads((base / 'run.json').read_text(encoding='utf-8'))
    assert record['ownedProcessTerminated'] and record['inputUnchanged']
    owner = root / 'native-physical'
    rawdir = base / 'raw'
    rawdir.mkdir(exist_ok=True)
    plan = []
    records = []
    for path in base.glob('native-probe*.json'):
        original = path.read_bytes()
        data = json.loads(original)
        assert 'compaction' not in data, 'Physical run is already archived'
        archive = rawdir / ('native-probe-' + sha(original)[:16] + '.json.gz')
        archive.write_bytes(gzip.compress(original, mtime=0))
        assert gzip.decompress(archive.read_bytes()) == original
        records.append(dict(originalName=path.name, originalSha256=sha(original), archive=str(archive.relative_to(base))))
        if path.name in ('native-probe.json', 'native-probe-checkpoint.json'):
            plan.append(dict(path=str(path), bytes=len(original), sha256=sha(original), reason='Redundant original bytes retained in verified gzip archive.'))
        else:
            compact = {k: v for k, v in data.items() if k not in ('snapshots', 'menuInventory', 'steps', 'instrumentationMessages')}
            compact['compaction'] = records[-1]
            write(path, compact)
    retained = {sha(p.read_bytes()): str(p) for p in base.rglob('*') if p.is_file() and p.suffix != '.gz' and p.name not in ('native-probe.json', 'native-probe-checkpoint.json')}
    for folder in (owner / ('runtime-' + args.run), owner / ('host-temp-' + args.run)):
        assert folder.resolve().is_relative_to(owner)
        for path in folder.rglob('*'):
            if not path.is_file():
                continue
            digest = sha(path.read_bytes())
            if path.suffix.lower() == '.sm' and digest not in retained:
                sources = base / 'retained-sources'
                sources.mkdir(exist_ok=True)
                peer = sources / (digest[:12] + '-' + path.name)
                shutil.copy2(path, peer)
                assert sha(peer.read_bytes()) == digest
                retained[digest] = str(peer)
            plan.append(dict(path=str(path), bytes=path.stat().st_size, sha256=digest, retained=retained.get(digest), reason='Ended owned native physical runtime or Frida host temporary component.'))
    write(base / 'raw-manifest.json', dict(records=records, allNativeCompilationOutputsRetained=True, sourceProvenance=record['sourceProvenance']))
    destination = owner / ('physical-cleanup-' + args.run + '.json')
    write(destination, dict(ownerScope=str(owner), evidenceScope=str(base), files=plan))
    print(json.dumps(dict(plan=str(destination), files=len(plan), logicalBytes=sum(p['bytes'] for p in plan))))


if __name__ == '__main__':
    main()
