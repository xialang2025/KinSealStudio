"""Package the verified isolated checkout, excluding unaccepted local stage3 work, and test its own DLLs privately."""
from pathlib import Path
import copy
import datetime
import hashlib
import importlib.util
import json
import os
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
WORK = ROOT / 'worktree'
MAIN = ROOT.parents[1] / 'KinSealStudio'
OUT = WORK / 'docs/stage4-evidence/delivery-C21'
PACKAGE = MAIN / 'dist/KinSealStudioLite-Stage4-C21-0.10.1'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def write(path, data):
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')


def main():
    assert MAIN.resolve() == Path('Z:/Project/vscode_prj_temp/REA/KinSealStudio').resolve()
    compatible = read(OUT / 'main-windows-r1/run.json')
    assert len(compatible['runs']) == 3 and compatible['runs'][0]['totals'] == dict(passed=109, failed=0, skipped=0)
    isolated = read(WORK / 'docs/stage4-evidence/move-track/windows-final/run.json')
    assert isolated['passed'] and all(row['totals']['failed'] == 0 and row['totals']['skipped'] == 0 for row in isolated['runs'])
    assert read(OUT / 'diagnose-restoration.json')['restoredByteIdentical'] is True
    assert not PACKAGE.exists(), 'Do not overwrite an existing release'
    PACKAGE.mkdir(parents=True)
    source = WORK / 'build/KinSealStudioLite.exe'
    executable = PACKAGE / source.name
    shutil.copy2(source, executable)
    samples = PACKAGE / 'samples/stage4'
    samples.mkdir(parents=True)
    for path in (WORK / 'samples/stage4').glob('*.ksp'):
        shutil.copy2(path, samples / path.name)
    project = copy.deepcopy(read(samples / 'move-track.ksp'))
    project['projectName'] = '阶段4 C01～C21 编辑切片'
    counter = 0
    for page in project['pages']:
        for obj in page['objects']:
            obj.update(x=5 + counter % 6 * 118, y=10 + counter // 6 * 105, width=105, height=54 if obj['type'] == 'slider' else 85, layer=counter + 1)
            counter += 1
    assert counter == 21
    showcase = samples / 'stage4-controls-C21.ksp'
    write(showcase, project)
    for directory, name, target in [('third_party/lzokay', 'LICENSE', 'LZO-MIT-LICENSE.txt'), ('third_party/qrcodegen', 'LICENSE', 'QR-MIT-LICENSE.txt')]:
        path = WORK / directory / name
        if path.is_file():
            shutil.copy2(path, PACKAGE / target)
    environment = os.environ.copy()
    environment['PATH'] = 'G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;' + environment.get('PATH', '')
    tool = Path('G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin/windeployqt.exe')
    deployed = subprocess.run([str(tool), '--release', '--compiler-runtime', '--no-translations', '--dir', str(PACKAGE), str(executable)], capture_output=True, env=environment, timeout=120, creationflags=subprocess.CREATE_NO_WINDOW)
    (OUT / 'windeployqt-stdout.txt').write_bytes(deployed.stdout)
    (OUT / 'windeployqt-stderr.txt').write_bytes(deployed.stderr)
    assert deployed.returncode == 0, deployed.stderr.decode('utf-8', errors='replace')
    recipe = WORK / 'docs/stage1-evidence/gui-portable/probe.py'
    spec = importlib.util.spec_from_file_location('c21_private_package', recipe)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    smoke = OUT / 'portable-r1'
    smoke.mkdir(exist_ok=False)
    temporary = smoke / 'temp'
    temporary.mkdir()
    desktop = module.PrivateDesktop(smoke)
    environment = module.clean_environment(PACKAGE)
    environment.update(TEMP=str(temporary), TMP=str(temporary), TMPDIR=str(temporary))
    result = dict(startedAtUtc=datetime.datetime.now(datetime.timezone.utc).isoformat(), package=str(PACKAGE), executableSha256=sha(executable), buildExecutableSha256=sha(source),
                  gitCommit=subprocess.check_output(['git', '-C', str(MAIN), 'rev-parse', 'HEAD']).decode().strip(),
                  mainDirtyNames=subprocess.check_output(['git', '-C', str(MAIN), 'diff', '--name-only']).decode('utf-8').splitlines(),
                  buildSource='isolated_verified_committed_checkout', userStage3WorkingChangesPresentInBinary=False, userStage3ChangesCommitted=False,
                  mainCheckoutHasTwoUnacceptedStage3TestFailures=True, usbAccessed=False, desktopSwitched=False, systemInput=False, runs=[])
    try:
        for name, arguments, expected in [('self-test', ['--self-test'], 0), ('embedded-payload', ['--verify-independent-payload'], 0),
                ('showcase-model', ['--validate-project', str(showcase)], 0), ('capture-main-window', ['--capture-preview', str(smoke / 'main-window.png'), str(showcase)], 0),
                ('track-source-gate', ['--export-native-project', str(WORK / 'docs/stage4-evidence/move-track/inputs/move-track-only.ksp'), str(smoke / 'unpublished.sm')], 20),
                ('track-device-gate', ['--compile-project', str(WORK / 'docs/stage4-evidence/move-track/inputs/move-track-only.ksp'), str(smoke / 'unpublished-package')], 19)]:
            run = desktop.run(name, executable, arguments, environment, timeout=60, expected_exit=expected)
            result['runs'].append(run)
            assert run['passed'], run
        assert (smoke / 'main-window.png').is_file()
        assert not (smoke / 'unpublished.sm').exists() and not (smoke / 'unpublished-package').exists()
        observed_qt = sorted({path for row in result['runs'] for path in row['modulePathsObserved'] if Path(path).name.lower().startswith('qt6') and path.lower().endswith('.dll')})
        assert observed_qt and all(Path(path).resolve().is_relative_to(PACKAGE.resolve()) for path in observed_qt), observed_qt
        result.update(passed=True, qtDllsObserved=observed_qt, onlyBundledQtDllsObserved=True, previewSha256=sha(smoke / 'main-window.png'))
    finally:
        desktop.close()
        result['privateDesktopClosed'] = True
        result['temporaryFilesRemaining'] = [str(p.relative_to(temporary)) for p in temporary.rglob('*') if p.is_file()]
        write(smoke / 'run.json', result)
    text = ('KinSealStudio Lite 阶段4 C21增量包（0.10.1主窗口基础）\n\n'
            '运行 KinSealStudioLite.exe；文件→打开 samples/stage4/stage4-controls-C21.ksp 可查看21类编辑示例。\n'
            '当前阶段4共41项，已处理到C21移动轨迹，整阶段尚未完成。新增控件交付编辑/KSP切片，独立SM/设备编码仍受门槛限制。\n'
            'C01～C05沿用独立传输证据；C06～C21只确认原厂基线下载，不能据此认定本包可独立生成并下载这些控件。实屏显示、触控及PLC行为另验。\n'
            '14项历史原厂基线已逐项补验；每项完成后同步协议、清理并单独提交。完整证据和限制位于源工程docs/。\n'
            '此包来自已通过回归的独立已提交源码，不含主目录尚未提交验收的阶段3续作；原有主目录文件完整保留，两项阶段3兼容失败另有报告。\n')
    (PACKAGE / '使用说明.txt').write_text(text, encoding='utf-8-sig')
    manifest = dict(executableSha256=sha(executable), sourceCommit=result['gitCommit'], sourceTree='isolated_verified_committed_checkout', uncommittedStage3ChangesIncluded=False, completeStage4=False,
                    files=[dict(path=str(path.relative_to(PACKAGE)).replace('\\', '/'), bytes=path.stat().st_size, sha256=sha(path)) for path in sorted(PACKAGE.rglob('*')) if path.is_file()])
    write(OUT / 'package-manifest.json', manifest)
    (OUT / 'reproduction').mkdir(exist_ok=True)
    shutil.copy2(ROOT / 'package_c21.py', OUT / 'reproduction/package_c21.py')
    print(json.dumps(dict(package=str(PACKAGE), files=len(manifest['files']), smokePassed=True, qtOnlyFromPackage=True), ensure_ascii=False))


if __name__ == '__main__':
    main()
