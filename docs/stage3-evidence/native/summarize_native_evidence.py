"""#20261110-8 add: Preserve browseable observations and lossless raw native probe records."""
from pathlib import Path
import gzip
import hashlib
import json


def main():
    """Summarize only captured facts and verify every archived raw byte."""
    output = Path(__file__).parent
    summary = {'runs': [], 'sceneObservations': [], 'rawArchives': []}
    for path in sorted(output.glob('native-probe-*.json')):
        if path.name == 'native-probe-checkpoint.json':
            continue
        data = json.loads(path.read_text(encoding='utf8'))
        summary['runs'].append({'file': path.name + '.gz', 'run': data.get('run'), 'input': data.get('input'), 'status': data['status'], 'error': data.get('error'), 'pid': data.get('pid'), 'ownedProcessTerminated': data.get('ownedProcessTerminated'), 'outputSources': data.get('outputs'), 'nativeCompilations': data.get('nativeCompilations')})
        for observation in data.get('sceneReads', []):
            scene = observation['data'].get('after', observation['data'])
            summary['sceneObservations'].append({'run': data.get('run'), 'label': observation.get('label'), 'selected': scene.get('selected', []), 'selectableRoots': [v for v in scene.get('items', []) if v['parent'] == '0x0' and v['flags'] & 2]})
    for path in sorted(output.glob('*.json')):
        if path.name not in ('native-probe.json', 'native-probe-checkpoint.json') and not path.name.startswith('native-probe-'):
            continue
        raw = path.read_bytes()
        compressed = gzip.compress(raw, compresslevel=9, mtime=0)
        assert gzip.decompress(compressed) == raw
        archive = path.with_suffix(path.suffix + '.gz')
        assert not archive.exists()
        archive.write_bytes(compressed)
        summary['rawArchives'].append({'originalFile': path.name, 'archive': archive.name, 'originalBytes': len(raw), 'originalSha256': hashlib.sha256(raw).hexdigest(), 'archiveBytes': len(compressed), 'archiveSha256': hashlib.sha256(compressed).hexdigest(), 'verifiedLossless': True})
        path.unlink()
    summary['counts'] = {'runs': len(summary['runs']), 'partialOrError': sum(r['status'] == 'partial-or-error' for r in summary['runs']), 'allOwnedProcessesEnded': all(r['ownedProcessTerminated'] for r in summary['runs']), 'nativeCompilations': sum(len(r.get('nativeCompilations') or []) for r in summary['runs'])}
    (output / 'native-evidence-summary.json').write_text(json.dumps(summary, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print(json.dumps(summary['counts']))


if __name__ == '__main__':
    main()
