"""Run the newly built, still closed G07 gate on an owned private desktop."""
from pathlib import Path
import hashlib
import importlib.util
import json
import re

root = Path(__file__).resolve().parent
work = root.parent / 'kinseal-stage4-20261112/worktree'
out = work / 'docs/stage5-evidence/sector-red-r1'
out.mkdir(exist_ok=False)
spec = importlib.util.spec_from_file_location('private', work / 'docs/stage1-evidence/gui-portable/probe.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
desk = module.PrivateDesktop(out)
env = module.clean_environment()
env['PATH'] = 'G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;' + env['PATH']
for name in ('TEMP', 'TMP', 'TMPDIR'):
    env[name] = str(root / 'temp')
exe = work / 'build/KinSealStage5CompilerTests.exe'
log = out / 'sector-red.txt'
try:
    result = desk.run('sectorRed', exe, ['sectorCanonicalFrames', '-o', str(log) + ',txt'], env, timeout=120)
    totals = re.search(rb'Totals: (\d+) passed, (\d+) failed, (\d+) skipped', log.read_bytes())
    assert totals
    result.update(executableSha256=hashlib.sha256(exe.read_bytes()).hexdigest(), expectedFailure=True,
                  usbAccessed=False, desktopSwitched=False, systemInput=False,
                  totals=dict(zip(('passed', 'failed', 'skipped'), map(int, totals.groups()))))
finally:
    desk.close()
(out / 'run.json').write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
print(result['totals'])
assert result['totals'] == dict(passed=2, failed=15, skipped=0)
