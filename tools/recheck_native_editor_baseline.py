"""Recheck retained KinSeal V5.1 editor evidence against the supplied PE and a Lite release."""
# 20261119-11 add: Bind static editor evidence and offline Lite outcomes to exact inputs.
from __future__ import annotations

import argparse
import base64
from collections import Counter
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess


REPO = Path(__file__).resolve().parents[1]
FROZEN = 'adbaa60c230bc45c705a15b001f8fe26e3da9716'
EXPECTED_NATIVE = '305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c'
EXPECTED_LITE = '1bcd29cf1b799096def3d3a820e836b5d683de27ef5a96b23ace4b7ab8783b97'
STATE_ROOT = 'docs/stage0-evidence/'
PAGE_ROOT = 'docs/stage1-evidence/native-analysis/'
SETTINGS_ROOT = 'docs/stage2-evidence/native/'
RECIPE_ROOT = 'docs/stage4-evidence/recipe/native/'
PAGE_SAMPLES = (
    'A-empty-one-page', 'B-two-empty-pages', 'C-third-page-number9', 'D-third-page-name',
)
WINDOW_SAMPLES = (
    'window-sameprocess-baseline-native-ui', 'window-320x200-native-ui', 'window-center-native-ui',
    'window-close-off-native-ui', 'window-position-55-66-native-ui',
)
SYSTEM_VARIANTS = {
    'screensaver2': ('screensaverMinutes', 2, 'config.head.5', 1, ((3, 1, 2),)),
    'flick11': ('flickerTenths', 11, 'config.head.6', 10, ((4, 10, 11),)),
    'user2': ('defaultUserLevel', 2, 'config.head.14', 1, ((22, 1, 2),)),
    'touch-off': ('touchSound', False, 'config.head.8', 1, ((8, 0, 1),)),
    'hex-on': ('hexWordAddress', True, 'config.flags.1', 0, ()),
}
RECIPE_SAMPLES = {'recipe-custom-configuration': True, 'recipe-configuration-only': True,
                  'recipe-custom': False, 'recipe-default-wide': False}


def sha(data: bytes) -> str:
    """Input: exact file bytes. Return: lowercase SHA-256."""
    return hashlib.sha256(data).hexdigest()


def git(args: list[str], environment: dict[str, str]) -> bytes:
    """Input: read-only Git arguments and temp-scoped environment. Return: raw stdout."""
    return subprocess.run(['git', *args], cwd=REPO, env=environment, check=True, capture_output=True,
                          timeout=30, creationflags=subprocess.CREATE_NO_WINDOW).stdout


def pe_regions(data: bytes) -> list[tuple[str, int, int]]:
    """Input: pinned unmodified Windows PE. Return: its section names and bounded file spans."""
    header = struct.unpack_from('<I', data, 0x3C)[0]
    if data[:2] != b'MZ' or data[header:header + 4] != b'PE\x00\x00':
        raise ValueError('Expected a Windows PE')
    count = struct.unpack_from('<H', data, header + 6)[0]
    optional = struct.unpack_from('<H', data, header + 20)[0]
    table = header + 24 + optional
    result = []
    for index in range(count):
        offset = table + 40 * index
        name = data[offset:offset + 8].rstrip(b'\x00').decode('ascii')
        size, start = struct.unpack_from('<II', data, offset + 16)
        if start + size > len(data):
            raise ValueError(f'PE section {name} exceeds the file')
        result.append((name, start, start + size))
    return result


def typed_field(data: bytes, field: dict) -> dict:
    """Input: source bytes and a validated typed field span. Return: raw hex and decoded value."""
    start, end, kind = field['start'], field['end'], field['kind']
    if not 0 <= start < end <= len(data):
        raise ValueError('Invalid typed source span')
    part = data[start:end]
    if kind in {'i', 'a', 'b'}:
        value = int.from_bytes(part, 'big', signed=kind == 'i')
    elif kind == 'd':
        value = struct.unpack('>d', part)[0]
    elif kind == 'q':
        if len(part) < 4:
            raise ValueError('Truncated QString length')
        size = int.from_bytes(part[:4], 'big')
        if size == 0xFFFFFFFF and len(part) == 4:
            value = None
        elif size == len(part) - 4:
            value = part[4:].decode('utf-16-be')
        else:
            raise ValueError('Invalid QString payload width')
    else:
        raise ValueError(f'Unexpected typed field kind: {kind}')
    return dict(field, hex=part.hex(), value=value)


def states_and_literals(blob, native: bytes) -> dict:
    """Input: frozen blob reader and pinned PE. Return: 121 literal anchors and retained state observations."""
    inventory = json.loads(blob(STATE_ROOT + 'entry-inventory.json'))
    probe = json.loads(blob(STATE_ROOT + 'entry-probe-states-r2.json'))
    if inventory['identity']['originalSha256'] != sha(native) or probe['originalSha256'] != sha(native):
        raise ValueError('The retained actions refer to a different V5.1 executable')
    regions = pe_regions(native)
    known_actions = [entry for entry in inventory['entries'] if entry['category'] == 'named-action']
    names = {entry['objectName']: [] for entry in known_actions}
    # Accumulate all anchors in one scan, including the three exceptional identifier spellings.
    alternatives = b'|'.join(re.escape(name.encode('ascii')) for name in sorted(names))
    pattern = re.compile(rb'(?<![A-Za-z0-9_])(?:' + alternatives + rb')\x00')
    for section, begin, end in regions:
        for match in pattern.finditer(native, begin, end):
            name = match[0][:-1].decode('ascii')
            if name in names:
                names[name].append({'section': section, 'offset': match.start()})
    rows = []
    for action in known_actions:
        name = action['objectName']
        offsets = names[name]
        if not offsets:
            raise ValueError(f'Missing action identity in original PE: {name}')
        rows.append({'id': action['id'], 'name': name, 'asciiNullTerminatedMatches': offsets,
                     'historicalShortcut': action['shortcut']['portableText'],
                     'historicalStates': {k: {'status': v['status'], 'enabled': v.get('enabled')}
                                          for k, v in action['states'].items()}})
    state_counts = {}
    for key in inventory['stateDefinitions']:
        state_counts[key] = dict(Counter(row['historicalStates'][key]['status'] for row in rows))
    snapshots = [{'label': row['label'], 'selectedCount': row['selectedCount'],
                  'windowModified': row['windowModified']} for row in probe['snapshots']]
    if len(rows) != 121 or not any(row['label'] == 'after:action_checkall' and row['selectedCount'] == 12
                                   for row in snapshots):
        raise ValueError('The expected bounded original selection record is missing')
    return {'inventorySha256': sha(blob(STATE_ROOT + 'entry-inventory.json')), 'staticActions': rows,
            'staticActionCount': len(rows), 'stateCounts': state_counts,
            'historicalStateSource': STATE_ROOT + 'entry-probe-states-r2.json',
            'historicalRuntimeSha256': probe['runtimeSha256'], 'historicalSnapshots': snapshots,
            'historicalSingleSelectionConfirmed': any(row['selectedCount'] == 1 for row in snapshots),
            'historicalSavedSourceChanged': probe['workingCopySha256After'] != probe['input']['sha256'],
            'bindingNote': 'PE literals prove names exist. State values come from the prior copied executable only.'}


def system_oracle(blob) -> dict:
    """Input: frozen blob reader. Return: byte differences that survive paired repeats and baseline restore."""
    base_names = ['baseline-r1-fresh', 'baseline-repeat-fresh', 'baseline-restored-fresh']
    lengths = set()

    def read(run):
        root = SETTINGS_ROOT + 'compiled/final-system-' + run + '/'
        source, output = blob(root + 'input-native.sm'), blob(root + 'SysPara.dat')
        lengths.add(len(output))
        return output, {'sourcePath': root + 'input-native.sm', 'sourceSha256': sha(source),
                        'outputPath': root + 'SysPara.dat', 'outputSha256': sha(output)}

    baseline = [read(name) for name in base_names]
    rows = []
    for variant, (property_name, property_value, source_field, source_default, expected) in SYSTEM_VARIANTS.items():
        alternatives = [read(variant + '-r1-fresh'), read(variant + '-repeat-fresh')]
        data = [row[0] for row in baseline + alternatives]
        if lengths != {4998}:
            raise ValueError('Unexpected retained SysPara width')
        stable = []
        noise = []
        for index, (base, repeat, restored, trial, trial_repeat) in enumerate(zip(*data)):
            if base != repeat or base != restored or trial != trial_repeat:
                noise.append(index)
            elif base != trial:
                stable.append((index, base, trial))
        if tuple(stable) != expected:
            raise ValueError(f'{variant} stable byte result changed: {stable}')
        before_path = SETTINGS_ROOT + 'system-baseline-native-ui.sm'
        after_path = SETTINGS_ROOT + f'system-{variant}-native-ui.sm'
        source_before, source_after = blob(before_path), blob(after_path)
        base_grammar = json.loads(blob(before_path[:-3] + '-fields.json'))
        changed_grammar = json.loads(blob(after_path[:-3] + '-fields.json'))
        if base_grammar['sha256'] != sha(source_before) or changed_grammar['sha256'] != sha(source_after):
            raise ValueError('Typed source grammar identity differs')
        if len(source_before) != len(source_after) or base_grammar['fields'] != changed_grammar['fields']:
            raise ValueError('Unexpected source span change in settings probe')
        differing_fields = [f for f in base_grammar['fields'] if source_before[f['start']:f['end']]
                            != source_after[f['start']:f['end']]]
        if len(differing_fields) != 1 or differing_fields[0]['path'] != source_field:
            raise ValueError('Setting changed more than its retained one-field source evidence')
        old_field, new_field = [typed_field(source, differing_fields[0]) for source in (source_before, source_after)]
        if old_field['value'] != source_default or new_field['value'] != property_value:
            raise ValueError('Typed source settings value differs')
        rows.append({'variant': variant, 'property': property_name, 'requestedValue': property_value,
                     'originalSavedSourceChange': {'baselineSourceSha256': sha(source_before),
                                                   'variantSourceSha256': sha(source_after),
                                                   'before': old_field, 'after': new_field},
                     'runIdentities': [row[1] for row in alternatives], 'repeatOrRestoreNoiseOffsets': noise,
                     'stableDifferences': [{'offset': i, 'default': left, 'changed': right}
                                           for i, left, right in stable]})
    return {'baselineRuns': [row[1] for row in baseline], 'variants': rows, 'outputWidth': 4998,
            'method': 'Difference must be constant across three historical controls and both variant repeats.',
            'sourceInputsSameWithinEveryComparison': False,
            'fieldWidthOrDeviceConsumerInferred': False}


class LiteCommands:
    """Bound process runner for explicitly offline CLI commands in an owned task directory."""

    def __init__(self, executable: Path, root: Path, environment: dict[str, str]):
        self.executable = executable
        self.root = root
        self.environment = environment

    def run(self, command: str, args: list[Path], expected: int) -> dict:
        """Input: offline CLI name, paths, expected exit. Return: parsed UTF-8 result."""
        if command not in {'inspect-native-source', 'project-capabilities', 'migrate-project',
                           'export-native-project', 'compile-project'}:
            raise ValueError('Only explicit offline commands are allowed')
        proc = subprocess.run([str(self.executable), '--' + command, *[str(p) for p in args]],
                              cwd=self.root, env=self.environment, capture_output=True, timeout=35,
                              creationflags=subprocess.CREATE_NO_WINDOW)
        if proc.returncode != expected:
            raise ValueError(f'{command} {args[0].name}: expected {expected}, got {proc.returncode}: '
                             f'{proc.stdout.decode("utf-8", "replace")} {proc.stderr.decode("utf-8", "replace")}')
        return {'exit': proc.returncode, 'result': json.loads(proc.stdout)}

    def compare(self, fixture: str, data: bytes, allow_import: bool, observe_structure: bool = True) -> dict:
        """Input: exact retained source path and bytes. Return: model, export and compile observations."""
        focus = self.root / (Path(fixture).stem + '.sm')
        focus.write_bytes(data)
        inspection = self.run('inspect-native-source', [focus], 0 if observe_structure else 22)
        inspected = inspection['result']
        if observe_structure:
            if (not inspected['bodyValidated'] or inspected['semanticImportPerformed'] or inspected['usbAccessed']
                    or inspected['sha256'] != sha(data)):
                raise ValueError(f'Incorrect structural result for {fixture}')
        elif '原生控件类型 30' not in inspected['message']:
            raise ValueError(f'Recipe object was rejected for an unexpected reason: {inspected}')
        out = focus.with_suffix('.ksp')
        exported = focus.with_suffix('.export.sm')
        compiled = focus.with_suffix('.compiled')
        sentinel = b'recheck-native-editor-baseline: preexisting output sentinel\n'
        if not allow_import:
            out.write_bytes(sentinel)
            exported.write_bytes(sentinel)
        caps = self.run('project-capabilities', [focus], 0 if allow_import else 13)
        migration = self.run('migrate-project', [focus, out], 0 if allow_import else 21)
        native_export = self.run('export-native-project', [focus, exported], 0 if allow_import else 20)
        result = self.run('compile-project', [focus, compiled], 19)
        if compiled.exists() or focus.read_bytes() != data:
            raise ValueError(f'Rejected device compile modified outputs or original working copy for {fixture}')
        fields = inspected.get('fields', [])
        interesting = re.compile(r'(?:project\.pages\.count|page(?:\[\d+\])?\.head\.(?:18|19)|'
                                 r'page\[1\]\.(?:head\.(?:2|3|13|15|16)|tail\.0))')
        page_fields = {f['path']: typed_field(data, f) for f in fields if interesting.fullmatch(f['path'])}
        row = {'source': fixture, 'originalSha256': sha(data), 'bytes': len(data), 'typedFields': len(fields),
               'pageCountFromTypedSource': page_fields.get('project.pages.count', {}).get('value'),
               'commands': {'inspect': inspection['exit'], 'capabilities': caps['exit'], 'migrate': migration['exit'],
                            'nativeExport': native_export['exit'], 'deviceCompile': result['exit']},
               'compileRefusal': result['result']['message'], 'compileOutputCreated': False}
        if Path(fixture).stem in PAGE_SAMPLES + WINDOW_SAMPLES:
            row['selectedTypedPageFields'] = page_fields
        if not observe_structure:
            row['typedRefusal'] = inspected['message']
        if allow_import:
            parsed = json.loads(out.read_bytes())
            opaque_equal = base64.b64decode(parsed['nativeSourceSnapshot'], validate=True) == data
            exported_equal = exported.read_bytes() == data
            if not opaque_equal or not exported_equal:
                raise ValueError(f'Opaque source preservation failed for {fixture}')
            row.update(opaqueSnapshotByteEqual=opaque_equal, exportedNativeBytesEqual=exported_equal,
                       semanticConfigurationKinds=[node['kind'] for node in parsed.get('configurationNodes', [])])
        else:
            if out.read_bytes() != sentinel or exported.read_bytes() != sentinel:
                raise ValueError(f'Rejected source transaction changed existing file for {fixture}')
            row['preexistingOutputsUnchanged'] = True
        return row


def recipe_traces(blob) -> list[dict]:
    """Input: frozen blob reader. Return: original recipe trace provenance and observed table state."""
    records = []
    for name in ('recipe-config-r2', 'recipe-tree-context-r11', 'recipe-lifetimes-r12', 'recipe-reopen-r16'):
        path = RECIPE_ROOT + 'native-probe-' + name + '.json'
        raw = blob(path)
        trace = json.loads(raw)
        if trace['originalSha256'] != EXPECTED_NATIVE or trace['systemInput'] or trace['usbOrPlc']:
            raise ValueError('Recipe trace scope differs')
        table_states = [{'label': state['label'], 'rows': state['state']['rows'],
                         'columns': state['state']['columns'],
                         'cells': [[cell['text'] for cell in row] for row in state['state']['cells']]}
                        for state in trace.get('nativeTableStates', [])]
        if name == 'recipe-reopen-r16' and sha(blob(RECIPE_ROOT + 'recipe-custom-configuration.sm')) != trace['input']['sha256']:
            raise ValueError('Recipe reopen trace input is not the retained sample')
        records.append({'path': path, 'sha256': sha(raw), 'steps': len(trace['steps']), 'status': trace['status'],
                        'tableStates': table_states, 'sourceSha256': trace['input']['sha256']})
    return records


def verify_page_observations(rows: list[dict]) -> None:
    """Input: current structural reads. Return: None after checking the retained page/window transitions."""
    observed = {Path(row['source']).stem: row for row in rows}
    if [observed[name]['pageCountFromTypedSource'] for name in PAGE_SAMPLES] != [1, 2, 3, 3]:
        raise ValueError('Original A–D page counts differ')
    fields = observed['C-third-page-number9']['selectedTypedPageFields']
    renamed = observed['D-third-page-name']['selectedTypedPageFields']
    if (fields['page[2].head.19']['value'], renamed['page[2].head.19']['value'],
            renamed['page[2].head.18']['value']) != ('009', '002', 'Stage1_Third'):
        raise ValueError('Original third-page name/number evidence differs')
    window_fields = ('head.2', 'head.3', 'head.13', 'head.15', 'head.16', 'tail.0')
    expected = [(240, 180, 1, 0, 0, 1), (320, 200, 0, 0, 0, 1), (240, 180, 1, 0, 0, 1),
                (240, 180, 1, 0, 0, 0), (240, 180, 0, 55, 66, 1)]
    for name, target in zip(WINDOW_SAMPLES, expected):
        row = observed[name]
        actual = tuple(row['selectedTypedPageFields']['page[1].' + key]['value'] for key in window_fields)
        if row['pageCountFromTypedSource'] != 2 or actual != target:
            raise ValueError(f'Original window source evidence differs: {name}')


def main() -> int:
    """Input: original PE, approved Lite release, and an owned new Z-drive run directory. Return: exit status."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--original', required=True, type=Path)
    parser.add_argument('--lite', required=True, type=Path)
    parser.add_argument('--run-root', required=True, type=Path)
    parser.add_argument('--evidence', required=True, type=Path)
    args = parser.parse_args()
    original, lite = args.original.resolve(strict=True), args.lite.resolve(strict=True)
    run = args.run_root.resolve()
    workspace_scratch = REPO.parent.joinpath('.scratch').resolve(strict=True)
    if not run.is_relative_to(workspace_scratch) or run == workspace_scratch or args.evidence.exists():
        raise ValueError('Use a fresh work directory within workspace .scratch and a new evidence file')
    native = original.read_bytes()
    if sha(native) != EXPECTED_NATIVE or sha(lite.read_bytes()) != EXPECTED_LITE:
        raise ValueError('Input executable identity differs from the frozen comparison baseline')
    run.mkdir(parents=True, exist_ok=False)
    temp = run / 'temp'
    temp.mkdir()
    pkg = lite.parent
    windows = Path(os.environ.get('SystemRoot', r'C:\Windows'))
    git_environment = dict(os.environ, TEMP=str(temp), TMP=str(temp), TMPDIR=str(temp))
    environment = dict(git_environment,
                       QT_QPA_PLATFORM='windows', QT_PLUGIN_PATH=str(pkg),
                       QT_QPA_PLATFORM_PLUGIN_PATH=str(pkg / 'platforms'),
                       PATH=os.pathsep.join([str(pkg), str(windows / 'System32'), str(windows)]))
    commit = git(['rev-parse', '--verify', FROZEN + '^{commit}'], git_environment).decode('ascii').strip()

    def blob(path):
        return git(['show', commit + ':' + path], git_environment)

    actions = states_and_literals(blob, native)
    settings = system_oracle(blob)
    cli = LiteCommands(lite, run, environment)
    checks = [(PAGE_ROOT + p + '.sm', i == 0) for i, p in enumerate(PAGE_SAMPLES)]
    checks.extend((SETTINGS_ROOT + p + '.sm', False) for p in WINDOW_SAMPLES)
    checks.extend((SETTINGS_ROOT + 'system-' + p + '-native-ui.sm', True)
                  for p in ['baseline', *SYSTEM_VARIANTS])
    results = [cli.compare(path, blob(path), allow) for path, allow in checks]
    verify_page_observations(results)
    results.extend(cli.compare(RECIPE_ROOT + name + '.sm', blob(RECIPE_ROOT + name + '.sm'), limited, limited)
                   for name, limited in RECIPE_SAMPLES.items())
    recipe = recipe_traces(blob)
    report = {'schemaVersion': 1, 'observedAtUtc': datetime.now(timezone.utc).isoformat(),
              'sourceCommit': commit, 'originalSha256': EXPECTED_NATIVE, 'liteSha256': EXPECTED_LITE,
              'nativeExeExecuted': False, 'nativeEvidenceCoverage': 'static PE plus frozen historical observations',
              'usbOrPlc': False, 'inputDesktopSwitched': False, 'systemInput': False,
              'actions': actions, 'systemOracle': settings, 'liteSourceTransactions': results,
              'recipeHistoricalBindings': recipe}
    if sha(original.read_bytes()) != EXPECTED_NATIVE or sha(lite.read_bytes()) != EXPECTED_LITE:
        raise ValueError('Input executable changed during the comparison')
    args.evidence.parent.mkdir(parents=True, exist_ok=True)
    with args.evidence.open('x', encoding='utf-8', newline='\n') as destination:
        json.dump(report, destination, ensure_ascii=False, indent=2)
        destination.write('\n')
    print(f'{len(actions["staticActions"])} PE action IDs, {len(results)} Lite source transactions, '
          f'{len(settings["variants"])} retained compiler variant pairs, {len(recipe)} recipe trace bindings')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
