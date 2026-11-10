"""//20261110-7 add: Separate repeated native compiler variation from tested settings."""
from pathlib import Path
import hashlib
import json

ROOT = Path(__file__).resolve().parent
COMPILED = ROOT / 'compiled'


def differing(left, right):
    """Input: two segment byte strings. Return: all differing byte offsets."""
    return [i for i in range(max(len(left), len(right))) if i >= len(left) or i >= len(right) or left[i] != right[i]]


def main():
    """Input: retained final native outputs. Return: complete repeat and field comparison JSON."""
    names = ['baseline', 'screensaver2', 'flick11', 'user2', 'touch-off', 'hex-on']
    folders = {}
    for name in names:
        folders[name] = [COMPILED / f'final-system-{name}-{suffix}-fresh' for suffix in ('r1', 'repeat')]
        assert all(p.is_dir() for p in folders[name]), (name, folders[name])
    restored = COMPILED / 'final-system-baseline-restored-fresh'
    assert restored.is_dir()
    repeats = []
    noise = set()
    for name, pair in folders.items():
        left = (pair[0] / 'SysPara.dat').read_bytes()
        right = (pair[1] / 'SysPara.dat').read_bytes()
        diff = differing(left, right)
        noise.update(diff)
        repeats.append({'name': name, 'SysParaDifferingOffsets': diff, 'files': []})
        all_files = sorted({p.name for folder in pair for p in folder.iterdir() if p.is_file()})
        for filename in all_files:
            one = pair[0] / filename
            two = pair[1] / filename
            a = one.read_bytes() if one.exists() else b''
            b = two.read_bytes() if two.exists() else b''
            repeats[-1]['files'].append({'file': filename, 'same': a == b, 'leftBytes': len(a), 'rightBytes': len(b), 'differingOffsets': differing(a, b)})
    baseline_bytes = [(p / 'SysPara.dat').read_bytes() for p in folders['baseline']] + [(restored / 'SysPara.dat').read_bytes()]
    noise.update(differing(baseline_bytes[0], baseline_bytes[2]))
    rows = []
    for name in names[1:]:
        variant_bytes = [(p / 'SysPara.dat').read_bytes() for p in folders[name]]
        stable = []
        for offset in range(min(map(len, baseline_bytes + variant_bytes))):
            before = {x[offset] for x in baseline_bytes}
            after = {x[offset] for x in variant_bytes}
            if len(before) == 1 and len(after) == 1 and before != after and offset not in noise:
                stable.append({'offset': offset, 'baselineByte': baseline_bytes[0][offset], 'variantByte': variant_bytes[0][offset]})
        files = []
        for p in sorted(folders['baseline'][0].iterdir()):
            if not p.is_file():
                continue
            q = folders[name][0] / p.name
            if q.is_file():
                a, b = p.read_bytes(), q.read_bytes()
                files.append({'file': p.name, 'same': a == b, 'baselineBytes': len(a), 'variantBytes': len(b), 'differingOffsets': differing(a, b)})
        rows.append({'variant': name, 'stableSysParaChangedBytes': stable, 'allFilesAgainstBaseline': files})
    report = {'method': '13 fresh native compiler runs on one owned private-desktop process; original UI no-op parameter commit invalidates compiler cache before each run; no download/transfer button invoked', 'SysParaBytes': [len(x) for x in baseline_bytes], 'nondeterministicSysParaOffsets': sorted(noise), 'repeats': repeats, 'variants': rows, 'widthOrEndiannessInferred': False, 'firmwareValidated': False}
    raw_noise = {}
    for file in folders['baseline'][0].iterdir():
        if file.suffix.lower() not in ('.dat', '.bin'):
            continue
        bases = [(p / file.name).read_bytes() for p in folders['baseline']] + [(restored / file.name).read_bytes()]
        changing = set(differing(bases[0], bases[2]))
        for pair in folders.values():
            changing.update(differing((pair[0] / file.name).read_bytes(), (pair[1] / file.name).read_bytes()))
        raw_noise[file.name] = sorted(changing)
        for row in rows:
            variants = [(p / file.name).read_bytes() for p in folders[row['variant']]]
            stable_bytes = []
            for offset in range(min(map(len, bases + variants))):
                old_values = {x[offset] for x in bases}
                new_values = {x[offset] for x in variants}
                if len(old_values) == 1 and len(new_values) == 1 and old_values != new_values and offset not in changing:
                    stable_bytes.append({'offset': offset, 'baselineByte': bases[0][offset], 'variantByte': variants[0][offset]})
            if stable_bytes:
                row.setdefault('allRawSegmentsStableChangedBytes', []).append({'segment': file.name, 'bytes': stable_bytes})
    report['allRawSegmentNondeterministicOffsets'] = raw_noise
    (ROOT / 'system-compile-comparison.json').write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf8')
    for row in rows:
        print(row['variant'], row['stableSysParaChangedBytes'])
    print('nondeterministic', len(noise), 'offsets')


if __name__ == '__main__':
    main()
