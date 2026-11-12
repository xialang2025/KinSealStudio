"""Repeat the two observed main-checkout failures on the same private desktop without changing application logic."""
from pathlib import Path
import importlib.util
import json
import os
import sys
root = Path(__file__).resolve().parent
work = root / 'worktree'
out = work / 'docs/stage4-evidence/delivery-C21' / (sys.argv[1] if len(sys.argv) > 1 else 'diagnose-focus-r1')
out.mkdir(exist_ok=False)
recipe = work / 'docs/stage1-evidence/gui-portable/probe.py'
spec = importlib.util.spec_from_file_location('diagnose_private', recipe)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
environment = module.clean_environment()
environment['PATH'] = 'G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;' + environment['PATH']
temporary = out / 'temp'
temporary.mkdir()
environment.update(TEMP=str(temporary), TMP=str(temporary), TMPDIR=str(temporary))
desktop = module.PrivateDesktop(out)
records = []
try:
    for index in range(2):
        for target, function in [('KinSealStage3CanvasTests', 'nestedGroupsRetainIndependentLocks'), ('KinSealStage3IntegrationTests', 'pixelFontsIgnorePaintDeviceDpi')]:
            name = function + '-' + str(index + 1)
            result = desktop.run(name, root.parents[1] / 'KinSealStudio/build' / (target + '.exe'), [function, '-o', str(out / (name + '.txt')) + ',txt'], environment, timeout=45)
            records.append(result)
            print(json.dumps(dict(case=name, exitCode=result.get('exitCode'), passed=result['passed'])), flush=True)
finally:
    desktop.close()
    (out / 'run.json').write_text(json.dumps(dict(records=records, privateDesktopClosed=True, usbAccessed=False), ensure_ascii=False, indent=2), encoding='utf-8')
