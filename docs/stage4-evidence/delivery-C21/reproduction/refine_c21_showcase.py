"""Keep the package example visible at the verified native-shell viewport width, then revalidate its actual capture."""
from package_c21 import *
old_manifest = read(OUT / 'package-manifest.json')
shutil.copy2(OUT / 'package-manifest.json', OUT / 'package-manifest-r1.json')
showcase = PACKAGE / 'samples/stage4/stage4-controls-C21.ksp'
assert sha(showcase) == next(row['sha256'] for row in old_manifest['files'] if row['path'] == 'samples/stage4/stage4-controls-C21.ksp')
shutil.copy2(showcase, OUT / 'portable-r1/showcase-input.ksp')
project = read(showcase)
counter = 0
for page in project['pages']:
    for obj in page['objects']:
        obj.update(x=5 + counter % 6 * 118, y=10 + counter // 6 * 105)
        counter += 1
write(showcase, project)
smoke = OUT / 'portable-r2'
smoke.mkdir(exist_ok=False)
temporary = smoke / 'temp'
temporary.mkdir()
spec = importlib.util.spec_from_file_location('c21_refined_private', WORK / 'docs/stage1-evidence/gui-portable/probe.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
desktop = module.PrivateDesktop(smoke)
environment = module.clean_environment(PACKAGE)
environment.update(TEMP=str(temporary), TMP=str(temporary), TMPDIR=str(temporary))
record = dict(executableSha256=sha(PACKAGE / 'KinSealStudioLite.exe'), inputSha256=sha(showcase), runs=[], usbAccessed=False, desktopSwitched=False, systemInput=False)
try:
    for name, arguments in [('showcase-model', ['--validate-project', str(showcase)]), ('capture-main-window', ['--capture-preview', str(smoke / 'main-window.png'), str(showcase)])]:
        result = desktop.run(name, PACKAGE / 'KinSealStudioLite.exe', arguments, environment, timeout=60)
        record['runs'].append(result)
        assert result['passed']
    record['passed'] = True
finally:
    desktop.close()
    record['privateDesktopClosed'] = True
    record['temporaryFilesRemaining'] = [str(p.relative_to(temporary)) for p in temporary.rglob('*') if p.is_file()]
    write(smoke / 'run.json', record)
old_manifest['files'] = [dict(path=str(p.relative_to(PACKAGE)).replace('\\', '/'), bytes=p.stat().st_size, sha256=sha(p)) for p in sorted(PACKAGE.rglob('*')) if p.is_file()]
write(OUT / 'package-manifest.json', old_manifest)
shutil.copy2(ROOT / 'package_c21.py', OUT / 'reproduction/package_c21.py')
shutil.copy2(ROOT / 'refine_c21_showcase.py', OUT / 'reproduction/refine_c21_showcase.py')
print(json.dumps(dict(controls=counter, validationAndCapturePassed=True), ensure_ascii=False))
