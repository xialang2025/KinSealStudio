"""Exercise ordinary-macro CLI contracts on a private desktop without device I/O."""
#20261114-7 add: Retain real process, package, alias and failure-preservation evidence.
from __future__ import annotations

import argparse
import base64
import copy
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import tempfile
from datetime import datetime, timezone


def sha(data: bytes) -> str:  #20261114-7 add
    """Function Name: sha
    Description: Hash exact bytes without newline or encoding normalization.
    Input Parameters: data
    Return Value: Lowercase SHA256.
    Date: 2026.11.14
    """
    return hashlib.sha256(data).hexdigest()


def macro_source(bit: int, value: int = 1) -> str:  #20261114-7 add
    """Function Name: macro_source
    Description: Build the existing ordinary-source oracle with one explicit constant.
    Input Parameters: bit, value
    Return Value: C source; the macro display name is independent of its entry point.
    Date: 2026.11.14
    """
    return ('#include "macro.h"\n#include "include\\macrolib.h"\n'
            '#include "include\\glib.h"\nvoid Macro_Entry( )\n'
            '{\n    LocalBit[' + str(bit) + '] = ' + str(value) + ';\n}\n')


class Stage8Checks:  #20261114-7 add
    """Own one private desktop and one explicit evidence/temp scope."""

    def __init__(self, args):  #20261114-7 add
        """Function Name: Stage8Checks.__init__
        Description: Freeze identities before any process and isolate all temporary paths on Z.
        Input Parameters: args
        Return Value: None.
        Date: 2026.11.14
        """
        self.root = Path(__file__).resolve().parents[1]
        self.executable = args.executable.resolve()
        self.output = args.output.resolve()
        self.dependencies = args.dependencies.resolve()
        self.sdk = args.sdk.resolve()
        self.expected_sha = args.expected_sha.lower()
        if sha(self.executable.read_bytes()) != self.expected_sha:
            raise ValueError('Frozen executable SHA differs; no process started.')
        temporary_root = args.temporary_root.resolve()
        if temporary_root.drive.lower() != 'z:':
            raise ValueError('This acceptance run requires a task-owned Z temporary root.')
        self.output.mkdir(parents=True, exist_ok=False)
        temporary_root.mkdir(parents=True, exist_ok=True)
        self.temporary = Path(tempfile.mkdtemp(prefix='stage8-cli-', dir=temporary_root))
        self.compiler_temp = self.temporary / 'compiler'
        self.compiler_temp.mkdir()
        spec = importlib.util.spec_from_file_location(
            'stage8_private', self.root / 'docs/stage1-evidence/gui-portable/probe.py')
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        self.environment = module.clean_environment(self.dependencies)
        for key in ('TEMP', 'TMP', 'TMPDIR'):
            self.environment[key] = str(self.temporary)
        self.environment['KINSEAL_MACRO_TEMP_ROOT'] = str(self.compiler_temp)
        self.environment['KINSEAL_ARM_SDK'] = str(self.sdk)
        for key in ('KINSEAL_MACRO_FIXTURE_MODE', 'KINSEAL_MACRO_FIXTURE_STAGE'):
            self.environment.pop(key, None)
        self.record = dict(
            schemaVersion=1, stage=8, startedAtUtc=datetime.now(timezone.utc).isoformat(),
            executable=str(self.executable), executableSha256=self.expected_sha,
            sdkDirectory=str(self.sdk), dependencies=str(self.dependencies),
            privateDesktopOnly=True, hardwareIo=False, hmiExecutionConfirmed=False,
            systemInput=False, desktopSwitched=False,
            temporaryRoot=str(self.temporary), compilerTemporaryRoot=str(self.compiler_temp),
            runs=[], checks=[], notCovered=[
                'No HMI download, display, touch or macro execution.',
                'No address variables, global/init scheduler, other models or SDK versions.',
                'SDK identity hashes are not vendor authentication or redistribution authorization.',
                'CLI tests do not replace synthetic timeout/cancellation/owner-destruction tests.'
            ])
        self.desktop = module.PrivateDesktop(self.output)

    def save(self, name: str, value) -> Path:  #20261114-7 add
        """Function Name: Stage8Checks.save
        Description: Save a small task-owned JSON fixture or report.
        Input Parameters: name, value
        Return Value: Absolute path.
        Date: 2026.11.14
        """
        path = self.output / name
        path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        return path

    def check(self, name: str, passed: bool, observed=None) -> None:  #20261114-7 add
        """Function Name: Stage8Checks.check
        Description: Retain independently computed semantic assertions separately from exit status.
        Input Parameters: name, passed, observed
        Return Value: None.
        Date: 2026.11.14
        """
        self.record['checks'].append(dict(name=name, passed=bool(passed), observed=observed))

    def run(self, name: str, arguments, expected: int = 0, sdk: Path | None = None):  #20261114-7 add
        """Function Name: Stage8Checks.run
        Description: Launch only an owned explicit CLI process on the private desktop.
        Input Parameters: name, arguments, expected, sdk override
        Return Value: Parsed stdout JSON; malformed output is retained as a failed semantic check.
        Date: 2026.11.14
        """
        if sha(self.executable.read_bytes()) != self.expected_sha:
            raise ValueError('Executable changed during acceptance; no further process started.')
        environment = self.environment.copy()
        if sdk is not None:
            environment['KINSEAL_ARM_SDK'] = str(sdk)
        process = self.desktop.run(name, self.executable, [str(x) for x in arguments],
                                   environment, expected, timeout=145)
        self.record['runs'].append(process)
        raw = (self.output / process['stdout']).read_bytes()
        try:
            result = json.loads(raw.decode('utf-8-sig'))
        except (ValueError, UnicodeError):
            self.check(name + '-stdout-json', False, dict(bytes=len(raw), prefixHex=raw[:128].hex()))
            result = {}
        self.save('run.json', self.record)
        return result

    def fixture(self, name: str, macros: dict) -> Path:  #20261114-7 add
        """Function Name: Stage8Checks.fixture
        Description: Use the real bounded Lite schema, removing unrelated controls and binding explicit IDs.
        Input Parameters: name, macros in deliberately chosen insertion order
        Return Value: Complete model fixture.
        Date: 2026.11.14
        """
        project = json.loads((self.root / 'samples/stage7/internal-five-family.ksp').read_text(encoding='utf-8-sig'))
        project['pages'][0]['objects'] = []
        project['macros'] = macros
        project['macroIds'] = {key: 'stage8-cli-id-' + str(index) for index, key in enumerate(macros)}
        return self.save(name, project)

    def compile(self, name: str, source: Path, expected: int = 0,
                destination: Path | None = None, sdk: Path | None = None):  #20261114-7 add
        """Function Name: Stage8Checks.compile
        Description: Compile through the product verb and inspect only newly successful package reports.
        Input Parameters: name, source, expected, destination, sdk
        Return Value: stdout summary and successful report object.
        Date: 2026.11.14
        """
        target = destination or self.output / (name + '-result.json')
        summary = self.run(name, ['--compile-macros', source, target], expected, sdk)
        report = json.loads(target.read_text(encoding='utf-8-sig')) if expected == 0 and summary.get('ok') else None
        return summary, report

    def package_checks(self, multi_report: dict, macros: dict) -> None:  #20261114-7 add
        """Function Name: Stage8Checks.package_checks
        Description: Compile matching KSP/SM packages and independently inspect macro bytes and control index.
        Input Parameters: multi_report, macros
        Return Value: None; no package is sent to hardware.
        Date: 2026.11.14
        """
        project = json.loads((self.root / 'samples/stage5/word-macro.ksp').read_text(encoding='utf-8-sig'))
        project['macros'] = macros
        project['pages'][0]['objects'][0]['bitButton']['macroName'] = 'Zulu'
        source = self.save('bound-word-multi.ksp', project)
        native = self.output / 'bound-word-multi.sm'
        exported = self.run('export-native-source', ['--export-native-project', source, native])
        self.check('native-source-export-for-package-check', exported.get('ok') is True and native.is_file())
        if not exported.get('ok') or not native.is_file():
            self.record['notCovered'].append('SM package unavailable because native source export failed.')
            return
        expected_run = base64.b64decode(multi_report['marcoRunBase64'], validate=True)
        expected_var = base64.b64decode(multi_report['marcoVarBase64'], validate=True)
        for label, path in [('ksp', source), ('sm', native)]:
            directory = self.output / (label + '-device-package')
            result = self.run(label + '-package-compile', ['--compile-project', path, directory])
            self.check(label + '-package-compile-success', result.get('ok') is True)
            if not result.get('ok'):
                continue
            manifest = self.run(label + '-package-verify', ['--verify-compiled-package', directory])
            control = (directory / 'ControlPara.dat').read_bytes()
            package_run = (directory / 'MarcoRun.dat').read_bytes()
            package_var = (directory / 'MarcoVar.dat').read_bytes()
            self.check(label + '-package-macro-index-and-exact-bytes',
                       manifest.get('complete') is True and len(control) == 640
                       and control[1] == 1 and control[2] == 1
                       and package_run == expected_run and package_var == expected_var,
                       dict(controlBytes=len(control), macroEnabled=control[1], encodedIndex=control[2],
                            expectedMap=multi_report['macroIndices'], marcoRunSha256=sha(package_run),
                            marcoVarSha256=sha(package_var), hardwareIo=False))

    def execute(self) -> None:  #20261114-7 add
        """Function Name: Stage8Checks.execute
        Description: Run success, reproducibility, failure, alias and package-index acceptance cases.
        Input Parameters: None
        Return Value: None.
        Date: 2026.11.14
        """
        zero = self.fixture('zero.ksp', {})
        one = self.fixture('one.ksp', {'SetLB1': macro_source(1)})
        changed = self.fixture('constant-changed.ksp', {'SetLB1': macro_source(1, 0)})
        macros = {'Zulu': macro_source(2), 'Alpha': macro_source(1)}
        multi = self.fixture('multi.ksp', macros)
        inputs = {str(path): sha(path.read_bytes()) for path in (zero, one, changed, multi)}
        _, empty = self.compile('zero-no-sdk', zero, sdk=self.temporary / 'absent-sdk')
        self.check('zero-macros-needs-no-sdk',
                   empty is not None and empty['macroIndices'] == {}
                   and empty['marcoRunBase64'] == '' and empty['marcoVarBase64'] == ''
                   and empty['compiler']['macroCount'] == 0 and empty['compiler']['stages'] == [])
        _, first = self.compile('one-first', one)
        _, repeat = self.compile('one-repeat', one)
        _, modified = self.compile('one-changed', changed)
        _, multiple = self.compile('multi', multi)
        if first and repeat and modified and multiple:
            code = base64.b64decode(first['marcoRunBase64'], validate=True)
            variable = base64.b64decode(first['marcoVarBase64'], validate=True)
            self.check('one-macro-independent-arm-oracle',
                       first['macroIndices'] == {'SetLB1': 0} and len(code) == 1048
                       and code[:2] == bytes.fromhex('01ff')
                       and struct.unpack_from('<H', code, 102)[0] == 24
                       and sha(code[1024:]) == 'd6e4f44119b03071a0dea0e8066f084054da6736f8ba5aeb74d443fff914e5da'
                       and variable == bytes(904),
                       dict(rawSha256=sha(code[1024:]), marcoRunSha256=sha(code), variableBytes=len(variable)))
            self.check('identical-source-reproducibility',
                       first['compiler']['reproducibilityKeySha256'] == repeat['compiler']['reproducibilityKeySha256']
                       and first['marcoRunBase64'] == repeat['marcoRunBase64']
                       and first['marcoVarBase64'] == repeat['marcoVarBase64'],
                       dict(key=first['compiler']['reproducibilityKeySha256']))
            self.check('changed-constant-changes-code-and-key',
                       first['compiler']['reproducibilityKeySha256'] != modified['compiler']['reproducibilityKeySha256']
                       and first['marcoRunBase64'] != modified['marcoRunBase64'])
            self.check('multiple-macros-sort-by-name-not-json-insertion',
                       multiple['macroIndices'] == {'Alpha': 0, 'Zulu': 1}
                       and [(m['macroName'], m['compiledIndex']) for m in multiple['compiler']['macros']]
                       == [('Alpha', 0), ('Zulu', 1)])
            for label, report, sources in [('one', first, {'SetLB1': macro_source(1)}), ('multi', multiple, macros)]:
                compiler = report['compiler']
                self.check(label + '-source-sdk-and-package-identity',
                           report['sourceSha256'] == inputs[str(one if label == 'one' else multi)]
                           and compiler['sdk']['fullyHashed'] is True
                           and compiler['sdk']['recheckedUnchangedAfterCompile'] is True
                           and compiler['sdk']['requiredFileCount'] == 15
                           and compiler['temporaryCleanupSucceeded'] is True
                           and all(row['sourceUtf8Sha256'] == sha(sources[row['macroName']].encode('utf-8'))
                                   and row['qstringUtf16LeSha256'] == sha(sources[row['macroName']].encode('utf-16-le'))
                                   and row['ansiRoundTripExact'] is True for row in compiler['sources'])
                           and report['hardwareIo'] is False and report['hmiExecutionConfirmed'] is False)
            self.record['sdkIdentitySha256'] = first['compiler']['sdk']['identitySha256']
            self.record['reproducibilityKeySha256'] = first['compiler']['reproducibilityKeySha256']
            self.package_checks(multiple, macros)
        else:
            self.record['notCovered'].append('Package and reproducibility comparisons require all ordinary SDK success cases.')
        sentinel = self.output / 'preserve-existing.json'
        sentinel_bytes = b'KEEP-STAGE8-OLD-OUTPUT'

        def rejected(name, source, sdk=None):  #20261114-7 add
            """Function Name: rejected
            Description: Assert expected CLI failure preserves a pre-existing destination exactly.
            Input Parameters: name, source, sdk
            Return Value: Failure summary.
            Date: 2026.11.14
            """
            sentinel.write_bytes(sentinel_bytes)
            result, _ = self.compile(name, source, 26, sentinel, sdk)
            self.check(name + '-atomic-preservation',
                       result.get('ok') is False and sentinel.read_bytes() == sentinel_bytes,
                       dict(message=result.get('message'), firstError=result.get('compiler', {}).get('firstError')))
            return result

        invalid = self.fixture('invalid-c-source.ksp', {'Broken': 'void Macro_Entry(void) { broken = ; }\n'})
        failure = rejected('invalid-c-source', invalid)
        self.check('invalid-source-keeps-structured-compiler-stage',
                   failure.get('compiler', {}).get('firstError', {}).get('phase') == 'armcc'
                   or any('armcc' in str(value) for value in failure.get('compiler', {}).get('firstError', {}).values()))
        rejected('missing-sdk', one, self.temporary / 'absent-sdk')
        missing = self.output / 'missing.ksp'
        rejected('missing-input', missing)
        bad = self.output / 'malformed-input.ksp'
        bad.write_bytes(b'{"format":')
        rejected('malformed-input', bad)
        non_object = self.save('array-input.ksp', [])
        rejected('array-input', non_object)
        oversized = self.temporary / 'oversized.ksp'
        with oversized.open('wb') as stream:
            stream.truncate(16 * 1024 * 1024 + 1)
        self.record['oversizedFixture'] = dict(path=str(oversized), bytes=oversized.stat().st_size,
                                              generator='truncate a fresh zero-filled file to 16MiB+1; input size guard rejects before read')
        rejected('oversized-input', oversized)
        alias_source = self.output / 'alias-input.json'
        alias_source.write_bytes(one.read_bytes())
        original_alias = alias_source.read_bytes()
        for label, destination in [
                ('identical-path', alias_source), ('normalized-alias', alias_source.parent / 'nonexistent' / '..' / alias_source.name),
                ('case-alias', Path(str(alias_source).upper()))]:
            result = self.run(label, ['--compile-macros', alias_source, destination], 26)
            self.check(label + '-input-kept', result.get('ok') is False and alias_source.read_bytes() == original_alias)
        hardlink = self.output / 'hardlink-output.json'
        os.link(alias_source, hardlink)
        result = self.run('hardlink-alias', ['--compile-macros', alias_source, hardlink], 26)
        self.check('hardlink-alias-input-kept',
                   result.get('ok') is False and alias_source.read_bytes() == original_alias and hardlink.read_bytes() == original_alias)
        wrong_extension = self.output / 'not-json.c'
        wrong_extension.write_bytes(sentinel_bytes)
        result = self.run('wrong-output-extension', ['--compile-macros', one, wrong_extension], 26)
        self.check('wrong-extension-output-kept', result.get('ok') is False and wrong_extension.read_bytes() == sentinel_bytes)
        result = self.run('wrong-argument-count', ['--compile-macros', one], 26)
        self.check('bad-arguments-have-small-error', result.get('ok') is False
                   and (self.output / 'wrong-argument-count-stdout.txt').stat().st_size < 4096)
        sentinel.write_bytes(sentinel_bytes)
        result = self.run('unwritable-destination', ['--compile-macros', one, self.output / 'absent-parent' / 'out.json'], 26)
        self.check('failed-publication-leaves-existing-output',
                   result.get('ok') is False and sentinel.read_bytes() == sentinel_bytes
                   and not (self.output / 'absent-parent' / 'out.json').exists())
        self.check('all-original-source-bytes-kept', all(Path(path).is_file() and sha(Path(path).read_bytes()) == value
                                                       for path, value in inputs.items()))
        self.check('compiler-owned-temporary-scope-empty', not list(self.compiler_temp.iterdir()))
        self.check('executable-kept-frozen', sha(self.executable.read_bytes()) == self.expected_sha)
        qt_paths = sorted({path for item in self.record['runs'] for path in item.get('modulePathsObserved', [])
                           if Path(path).name.lower().startswith('qt6')})
        self.check('Qt-loaded-only-from-explicit-dependencies',
                   bool(qt_paths) and all(Path(path).is_relative_to(self.dependencies) for path in qt_paths), qt_paths)
        self.record['expectedSourceInputs'] = inputs

    def finish(self) -> bool:  #20261114-7 add
        """Function Name: Stage8Checks.finish
        Description: Close only the owned desktop and freeze accurate pass/failure and remaining-file evidence.
        Input Parameters: None
        Return Value: Aggregate acceptance status.
        Date: 2026.11.14
        """
        self.desktop.close()
        self.record['privateDesktopHandleClosed'] = True
        self.record['completedAtUtc'] = datetime.now(timezone.utc).isoformat()
        self.record['temporaryFilesRemaining'] = [
            dict(path=str(path), bytes=path.stat().st_size) for path in self.temporary.rglob('*') if path.is_file()]
        self.record['cleanup'] = dict(
            status='not-attempted', note='Retained the generated size-guard fixture for parent accounting; no historical denied path retried.')
        self.record['passed'] = (not self.record.get('exception') and bool(self.record['runs'])
                                 and all(x['passed'] for x in self.record['runs'])
                                 and all(x['passed'] for x in self.record['checks']))
        self.save('run.json', self.record)
        return self.record['passed']


def main() -> int:  #20261114-7 add
    """Function Name: main
    Description: Require explicit immutable inputs and run one isolated CLI acceptance batch.
    Input Parameters: Command-line arguments.
    Return Value: Zero only when every executed process and semantic check passes.
    Date: 2026.11.14
    """
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--expected-sha', required=True)
    parser.add_argument('--dependencies', type=Path, required=True)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--temporary-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    checks = Stage8Checks(args)
    try:
        checks.execute()
    except Exception as error:
        checks.record['exception'] = repr(error)
    finally:
        passed = checks.finish()
    print(json.dumps(dict(passed=passed, runs=len(checks.record['runs']), checks=len(checks.record['checks']),
                         failedChecks=[item['name'] for item in checks.record['checks'] if not item['passed']],
                         exception=checks.record.get('exception')), ensure_ascii=False))
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
