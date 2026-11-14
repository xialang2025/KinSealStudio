"""Validate frozen stage6 inputs using owned Windows desktops; USB requires --hardware."""
#20261114-5 add: Preserve exact input/executable/wire identities and never use foreground input.
import argparse
import copy
import hashlib
import importlib.util
import json
import os
import time
from pathlib import Path
from datetime import datetime, timezone

WORK = Path(__file__).resolve().parents[1] / 'kinseal-stage4-20261112/worktree'

def sha(path):
    """Input: file path. Return: exact SHA256."""
    return hashlib.sha256(path.read_bytes()).hexdigest()

def write(path, value):
    """Input: path and JSON value. Return: None after writing evidence."""
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')

def main():
    """Input: explicit binary/output and optional hardware switch. Return: actual check result."""
    global WORK
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--work-root', type=Path, default=WORK)
    parser.add_argument('--exe', type=Path, required=True)
    parser.add_argument('--expected-sha', required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--hardware', action='store_true')
    parser.add_argument('--plans', nargs='+', choices=('static-10','dynamic-12','macro-14'), default=['static-10','dynamic-12','macro-14'])
    args = parser.parse_args()
    WORK = args.work_root.resolve()
    exe = args.exe.resolve()
    assert sha(exe) == args.expected_sha
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    spec = importlib.util.spec_from_file_location('stage6_private', WORK/'docs/stage1-evidence/gui-portable/probe.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    environment = module.clean_environment(exe.parent)
    environment['KINSEAL_ARM_SDK'] = 'G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed/script'
    if exe.parent.name == 'build':
        environment['PATH'] = ('G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'
            'G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;' + environment['PATH'])
    temporary = output / 'temp'
    temporary.mkdir()
    for key in ('TEMP', 'TMP', 'TMPDIR'):
        environment[key] = str(temporary)
    desktop = module.PrivateDesktop(output)
    record = dict(startedAtUtc=datetime.now(timezone.utc).isoformat(), executable=str(exe), executableSha256=sha(exe),
        usbAccessed=args.hardware, screenDisplayAndTouchConfirmed=False, firmwareVersion='not-observed',
        model='AMT070W124-L800X480R', desktopSwitched=False, systemInput=False, runs=[], passed=False)

    def run(name, argv, expected=0):
        """Run one explicit operation and retain its actual process result."""
        result = desktop.run(name, exe, argv, environment, expected_exit=expected, timeout=240)
        record['runs'].append(result)
        assert result['passed'], result
        return result

    try:
        for name, count in (('static-10', 10), ('dynamic-12', 12), ('macro-14', 14)):
            if name not in args.plans:
                continue
            source = WORK/'samples/stage6'/(name+'.ksp')
            identity = sha(source)
            contract = output/(name+'-contract.json')
            run(name+'-contract', ['--transfer-contract', str(source), str(contract)])
            document = json.loads(contract.read_text(encoding='utf-8'))
            assert document['payloadCount'] == count
            assert document['identity']['sourceSha256'] == identity
            assert len(document['operations']) == 3*count+11
            assert not document['usbAccessed']
            compiled = output/(name+'-compiled')
            run(name+'-compile', ['--compile-project', str(source), str(compiled)])
            run(name+'-verify', ['--verify-compiled-package', str(compiled)])
            if args.hardware:
                result = run(name+'-download', ['--download-project', str(source)])
                lines = [json.loads(line) for line in (output/result['stdout']).read_text(encoding='utf-8-sig').splitlines() if line.strip()]
                final = lines[-1]
                assert final['kind'] == 'transfer-result'
                assert all(final[key] for key in ('ok', 'compiled', 'transferAccepted', 'commitAccepted', 'resetWritten',
                    'postResetReadCompleted', 'interfaceReleased', 'closed'))
                assert final['payloadBlocks'] == count and final['postResetReadResult'] == -5
                assert not final['screenDisplayAndTouchConfirmed']
                expected = {item['name']: item['wireSha256'] for item in document['segments']}
                actual = {item['segment']: item['sha256'] for item in final['events'] if item['operation']=='payload-write'}
                assert actual == expected, (actual, expected)
                result['milestones'] = {key: final[key] for key in ('compiled', 'transferAccepted', 'commitAccepted',
                    'resetWritten', 'postResetReadCompleted', 'postResetReadResult', 'interfaceReleased', 'closed', 'outcome')}
                result['wirePayloadHashesMatchContract'] = True
                # Wait for USB re-enumeration before a separate planned project; never retry a failed transfer.
                time.sleep(6)
            assert sha(source) == identity
        if not args.hardware:
            original = json.loads((WORK/'samples/stage6/static-10.ksp').read_text(encoding='utf-8'))
            bad = copy.deepcopy(original)
            bad['targetModel'] = 'unknown-model'
            source = output/'unknown.ksp'
            write(source, bad)
            old = output/'old.json'
            old.write_bytes(b'preserve-existing-output')
            run('reject-target', ['--transfer-contract',str(source),str(old)],24)
            assert old.read_bytes() == b'preserve-existing-output'
            alias_source = output/'Source.JSON'
            write(alias_source, original)
            before = sha(alias_source)
            run('reject-case-alias', ['--transfer-contract',str(alias_source),str(output/'source.json')],24)
            assert sha(alias_source) == before
            hardlink = output/'hardlink.json'
            os.link(alias_source, hardlink)
            run('reject-hardlink', ['--transfer-contract',str(alias_source),str(hardlink)],24)
            assert sha(hardlink) == before
            bad = copy.deepcopy(original)
            bad['pages'][0]['objects'][0]['graphic']['alpha']=128
            source = output/'unsupported.ksp'
            write(source,bad)
            run('reject-unsupported-before-usb', ['--download-project',str(source)],14)
            final=json.loads((output/'reject-unsupported-before-usb-stdout.txt').read_text(encoding='utf-8-sig').splitlines()[-1])
            assert not final['opened'] and not final['compiled'] and not final['anyWriteAttempted']
        qt = sorted({path for item in record['runs'] for path in item['modulePathsObserved'] if Path(path).name.lower().startswith('qt6') and path.lower().endswith('.dll')})
        record['qtDlls']=qt
        record['onlyBundledQtDllsObserved'] = bool(qt) and all(Path(path).resolve().is_relative_to(exe.parent) for path in qt)
        record['passed']=True
    finally:
        desktop.close()
        record['privateDesktopClosed']=True
        record['completedAtUtc']=datetime.now(timezone.utc).isoformat()
        record['temporaryFilesRemaining']=[str(path.relative_to(temporary)) for path in temporary.rglob('*') if path.is_file()]
        write(output/'run.json',record)
    print(json.dumps({'passed':record['passed'],'checks':len(record['runs']),'usbAccessed':args.hardware},ensure_ascii=False))

if __name__=='__main__':
    main()
