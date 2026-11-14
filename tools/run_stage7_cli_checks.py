"""Exercise the frozen offline CLI on a private desktop, without physical I/O."""
#20261114-6 add: Retain actual process results and file identities for both build and portable runs.
import argparse
import base64
import copy
import hashlib
import importlib.util
import json
import os
from pathlib import Path


def main():
    """Input: Explicit EXE, dependency root and new output directory. Return: real acceptance results."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--dependencies', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    spec = importlib.util.spec_from_file_location('stage7_private', root / 'docs/stage1-evidence/gui-portable/probe.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    environment = module.clean_environment(args.dependencies.resolve())
    temporary = output / 'temp'
    temporary.mkdir()
    for key in ('TEMP', 'TMP', 'TMPDIR'):
        environment[key] = str(temporary)
    executable = args.executable.resolve()
    record = dict(executable=str(executable), executableSha256=hashlib.sha256(executable.read_bytes()).hexdigest(),
                  privateDesktopOnly=True, hardwareIo=False, hmiBehaviorConfirmed=False, runs=[], checks=[])
    desktop = module.PrivateDesktop(output)

    def save(name, value):
        """Input: fixture name and JSON. Return: a bounded task-owned file."""
        path = output / name
        path.write_text(json.dumps(value, ensure_ascii=False, indent=2), encoding='utf-8')
        return path

    def check(name, passed, observed=None):
        """Input: independently computed assertion. Return: append-only evidence."""
        record['checks'].append(dict(name=name, passed=bool(passed), observed=observed))

    def run(name, arguments, expected=0):
        """Input: explicit argv and expected status. Return: parsed small stdout report."""
        result = desktop.run(name, executable, [str(item) for item in arguments], environment, expected, timeout=150)
        record['runs'].append(result)
        raw = (output / result['stdout']).read_bytes()
        return json.loads(raw.decode('utf-8-sig'))

    def replay(name, source, events, expected=0, destination=None):
        """Input: source and events. Return: process summary and successful full result."""
        event_path = save(name + '-events.json', events)
        target = destination or output / (name + '-result.json')
        report = run(name, ['--simulate-offline', source, event_path, target], expected)
        result = json.loads(target.read_text(encoding='utf-8-sig')) if expected == 0 else None
        return report, result

    try:
        original = json.loads((root / 'samples/stage7/internal-five-family.ksp').read_text(encoding='utf-8-sig'))
        source = save('internal.ksp', original)
        source_bytes = source.read_bytes()
        events = json.loads((root / 'samples/stage7/internal-events.json').read_text(encoding='utf-8-sig'))
        cap = run('capabilities', ['--simulation-capabilities', source])
        check('supported-internal-five-family', cap.get('supported') is True and cap.get('hardwareIo') is False)
        report, trace = replay('internal', source, events)
        final = trace['finalState']
        check('toggle-input-language-and-final-stop', final['LB'].get('LB0') is True and not final['LB'].get('LB1', False)
              and final['LW'].get('LW10') == 42 and final['language'] == 'English' and final['running'] is False, final)
        (output / 'internal-final.png').write_bytes(base64.b64decode(trace['finalFramePngBase64']))
        _, initial = replay('initial', source, [])
        (output / 'initial-frame.png').write_bytes(base64.b64decode(initial['finalFramePngBase64']))
        _, held = replay('held-at-exit', source, [dict(op='press', id='momentary')])
        check('process-completion-releases-momentary', held['stateBeforeStop']['LB'].get('LB1') is True
              and not held['finalState']['LB'].get('LB1', False) and not held['finalState']['running'])
        macro = root / 'samples/stage7/macro-not-executed.ksp'
        _, trace = replay('macro-skipped', macro, events)
        check('macro-does-not-write-LB10', not trace['finalState']['LB'].get('LB10', False)
              and any(row['kind'] == 'macro-skipped' for row in trace['finalState']['events']))
        password = copy.deepcopy(original)
        next(item for item in password['pages'][0]['objects'] if item['id'] == 'input-word')['wordButton']['operation'] = 2
        password_path = save('password.ksp', password)
        secret = '54321'
        password_events = [dict(op='press', id='input-word'), dict(op='release', inside=True), dict(op='input', value=secret)]
        report, trace = replay('password', password_path, password_events)
        exported = json.dumps([report, trace])
        check('password-all-exported-memory-and-event-values-omitted', secret not in exported and 'eventsSha256' not in trace
              and all('LB' not in state and 'LW' not in state for state in
                      [trace['stateBeforeStop'], trace['finalState']] + [row['state'] for row in trace['trace']]))
        sentinel = output / 'preserve-existing.json'
        for name, bad_events in [('invalid-op', [dict(op='unsupported')]), ('unknown-object', [dict(op='press', id='missing')]),
                                 ('wrong-type', [dict(op='write-word', address='LW0', type=0, value=5)]),
                                 ('too-many-events', [dict(op='cancel')] * 1025)]:
            sentinel.write_bytes(b'KEEP-OLD-OUTPUT')
            result, _ = replay(name, source, bad_events, 25, sentinel)
            check(name + '-atomic-rejection', result.get('ok') is False and sentinel.read_bytes() == b'KEEP-OLD-OUTPUT')
        for name, variant in [('external-address', copy.deepcopy(original)), ('multi-page', copy.deepcopy(original))]:
            if name == 'external-address':
                variant['pages'][0]['objects'][0]['address'] = 'D0'
            else:
                page = copy.deepcopy(variant['pages'][0]); page['id'] = 'page-2'; page['objects'] = []
                variant['pages'].append(page); variant['pageOrder'].append('page-2')
            path = save(name + '.ksp', variant)
            sentinel.write_bytes(b'KEEP-OLD-OUTPUT')
            result, _ = replay(name, path, [], 25, sentinel)
            check(name + '-no-output', result.get('ok') is False and sentinel.read_bytes() == b'KEEP-OLD-OUTPUT')
        event_path = save('alias-input.json', [])
        report = run('case-alias', ['--simulate-offline', source, event_path, str(event_path).upper()], 25)
        check('case-alias-input-kept', json.loads(event_path.read_text()) == [] and report.get('ok') is False)
        hardlink = output / 'hardlink-output.json'
        os.link(event_path, hardlink)
        report = run('hardlink-alias', ['--simulate-offline', source, event_path, hardlink], 25)
        check('hardlink-alias-input-kept', json.loads(event_path.read_text()) == [] and report.get('ok') is False)
        hardlink.unlink()
        report = run('missing-event-input', ['--simulate-offline', source, output / 'missing.json', sentinel], 25)
        check('missing-input-no-output', report.get('ok') is False and sentinel.read_bytes() == b'KEEP-OLD-OUTPUT')
        # A growing two-word sparse store exceeds the trace budget before accumulating an oversized report.
        sentinel.write_bytes(b'KEEP-OLD-OUTPUT')
        large = [dict(op='write-word', address='LW' + str(2 * i), type=1, value='4294967295') for i in range(1024)]
        report, _ = replay('bounded-trace', source, large, 25, sentinel)
        raw = (output / 'bounded-trace-stdout.txt').read_bytes()
        check('trace-budget-small-error-and-preserved-output', report.get('ok') is False and len(raw) < 4096
              and sentinel.read_bytes() == b'KEEP-OLD-OUTPUT', dict(stdoutBytes=len(raw), message=report.get('message')))
        check('source-bytes-unchanged', source.read_bytes() == source_bytes)
        dependencies = args.dependencies.resolve()
        qt_paths = sorted({path for run_record in record['runs'] for path in run_record.get('modulePathsObserved', [])
                           if Path(path).name.lower().startswith('qt6')})
        check('Qt-loaded-only-from-explicit-dependencies', bool(qt_paths) and all(Path(path).is_relative_to(dependencies) for path in qt_paths), qt_paths)
    finally:
        desktop.close()
        record['privateDesktopHandleClosed'] = True
        record['passed'] = bool(record['runs']) and all(item['passed'] for item in record['runs']) and all(item['passed'] for item in record['checks'])
        save('run.json', record)
    print(json.dumps(dict(passed=record['passed'], runs=len(record['runs']), checks=len(record['checks']),
                         failedChecks=[item['name'] for item in record['checks'] if not item['passed']]), ensure_ascii=False))
    return 0 if record['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
