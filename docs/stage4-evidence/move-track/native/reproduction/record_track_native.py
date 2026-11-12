"""Derive bounded track facts from archived native runs and retain all partial-run limitations."""
from pathlib import Path
import gzip
import hashlib
import json
import shutil
root = Path(__file__).resolve().parent
base = root / 'worktree/docs/stage4-evidence/move-track/native'


def read(name):
    return json.loads(gzip.decompress((base / 'raw' / ('native-probe-' + name + '.json.gz')).read_bytes()))


def widget(data, label, name):
    snapshot = next(s for s in data['snapshots'] if s['label'] == label)
    return next(w['qt'] for win in snapshot['windows'] for w in win['widgets'] if w.get('qt', {}).get('objectName') == name)


def write(path, data):
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


default = read('track-default-r2')
variant = read('track-ui-r4')
assert default['status'] == variant['status'] == 'native-stage3-scenario-completed'
directions = widget(default, 'track-default-r2', 'direction_comboBox')['items']
assert directions == ['向上', '向下', '向左', '向右']
spin = widget(default, 'track-default-r2', 'moveDistance_spinBox')
assert [spin[k] for k in ('minimum', 'maximum', 'value')] == ['0', '900', '100']
assert widget(variant, 'track-range-address', 'min_lineEdit')['enabled'] == 0
assert widget(variant, 'track-range-address', 'minAddr_lineEdit')['enabled'] == 1
assert widget(variant, 'track-range-address-return', 'minAddr_lineEdit')['text'] == 'LW20'
assert widget(variant, 'track-range-address-return', 'maxAddr_lineEdit')['text'] == 'LW24'
assert widget(variant, 'track-range-constant-return', 'min_lineEdit')['text'] == '7'
assert widget(variant, 'track-range-constant-return', 'max_lineEdit')['text'] == '88'
assert widget(variant, 'track-notice-enabled', 'checkBox_NoteB')['enabled'] == 1
assert widget(variant, 'track-notice-word', 'checkBox_NoteB')['checked'] == 'false'
assert widget(variant, 'track-notice-word', 'checkBox_NoteW')['checked'] == 'true'
assert widget(variant, 'track-u16-max', 'max_lineEdit')['acceptableInput'] == 'true'
assert widget(variant, 'track-u16-overflow', 'max_lineEdit')['acceptableInput'] == 'false'
facts = dict(directionOrder=directions, distance=dict(minimum=0, maximum=900, default=100, lowerClamp=0, upperClamp=900),
    range=dict(default='constant', minimum='0', maximum='100', minimumAddress='LW0', maximumAddress='LW0', addressModeDisablesConstants=True, bothDraftsRetained=True, dynamicDraft=['LW20', 'LW24'], constantDraft=['7', '88']),
    scalarInput=dict(acceptedMax='65535', refusedOverflow='65536', negativeAndFractionAcceptableInput=False, nativeAcceptInvalidBehavior='Not tested'),
    appearance=dict(nativeResourceString=':/btn/images/gray.kvg', liteMappedResource=':/system-gallery/default/gray.kvg', defaultStaticOnly=True),
    permissions=dict(visibilityDefaultLevel=1, touchDefaultLevel=2, freshBitStates='Neither radio selected', acceptedReopenedBitStates='One selected; observed in r3/r4 reopened dialog', types=widget(default, 'track-permissions-r2', 'comboBox_DataTypeV')['items'], comparisons=widget(default, 'track-permissions-r2', 'comboBox_cpV')['items'],
                     noticeChildrenEnableAfterMaster=True, noticeBitAndWordExclusive=True, pressTimeHidden=True, equalityToleranceInteraction='r3 queued native call failed before equality snapshot; not verified'),
    errors=dict(r1='Output directory was missing; probe failed before retaining initial snapshot. Owned process termination and private desktop cleanup execute in finally; this run supplies no native behavior assertion.', r3='Native GUI-thread permission group invocation reported system error; partial input/range/notice observations retained, no variant compiled in this run.'),
    limits=['Exact movement position, axis geometry under all directions, touch/write timing and actual PLC value mapping remain unobserved.', 'Lite ordered min<max and one-word edit carrier do not establish complete firmware layout.', 'Native image selection round-trip and permission formulas are not promoted by shared-widget implementation.'])
write(base / 'findings.json', facts)
manifest = json.loads((base / 'native-manifest.json').read_text(encoding='utf-8'))
manifest['sources'] = [dict(path=p.name, bytes=p.stat().st_size, sha256=hashlib.sha256(p.read_bytes()).hexdigest(), selectableRootCount=1) for p in (base / 'track-default.sm', base / 'track-variant.sm')]
manifest['observations'] = 'findings.json'
write(base / 'native-manifest.json', manifest)
cleanup = [json.loads(p.read_text(encoding='utf-8-sig')) for p in (root / 'native-track').glob('native-cleanup-*.json.result.json')]
write(base / 'native-cleanup.json', dict(deletedFiles=sum(r['deletedFiles'] for r in cleanup), logicalBytes=sum(r['logicalBytes'] for r in cleanup), runs=cleanup, physicalSpaceClaimed=False))
for name in ('prepare_track_probe.py', 'record_track_native.py', 'archive_native_incremental.py', 'execute_native_cleanup.ps1'):
    shutil.copy2(root / name, base / 'reproduction' / name)
print(json.dumps(dict(sources=manifest['sources'], cleanupFiles=sum(r['deletedFiles'] for r in cleanup)), ensure_ascii=False))
