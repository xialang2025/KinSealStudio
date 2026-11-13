"""Compile retained UI-created static tables serially on the owned private desktop."""
from pathlib import Path
import datetime
import json
import os
import subprocess
import sys

root = Path(__file__).resolve().parent
work = root.parent / 'kinseal-stage4-20261112/worktree'
source = work / 'docs/stage4-evidence/table/native'
environment = os.environ.copy()
environment.update(TEMP=str(root/'temp'), TMP=str(root/'temp'), TMPDIR=str(root/'temp'))
runs = [(f'table-flags-{mask}-reopen-r1', source/f'flags-investigation/table-flags-{mask}.sm') for mask in range(8)]
runs += [('table-line-zero-reopen-r1', source/'line-zero-investigation/table-line-zero.sm'),
         ('table-default-repeat-r2', source/'table-default.sm')]
record = {'startedAt': datetime.datetime.now(datetime.timezone.utc).isoformat(), 'usbAccessed': False, 'runs': []}
for name, path in runs:
    output = work/'docs/stage5-evidence/native/table'/name/'native-probe.json'
    if output.exists():
        raise RuntimeError('Refusing to replace evidence: '+str(output))
    result = subprocess.run([sys.executable, '-B', str(root/'native_probe.py'), '--family', 'table', '--run', name,
                             '--input', str(path), '--no-clear', '--reopen-only', '--scenario', str(root/'repeat-one.json')],
                            capture_output=True, env=environment, timeout=100, creationflags=subprocess.CREATE_NO_WINDOW)
    record['runs'].append({'name': name, 'input': str(path), 'exitCode': result.returncode,
                          'stdout': result.stdout.decode('utf-8', errors='replace'),
                          'stderr': result.stderr.decode('utf-8', errors='replace')})
    (root/'table-canonical-run.json').write_text(json.dumps(record, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print(name, result.returncode, flush=True)
    if result.returncode:
        raise RuntimeError(record['runs'][-1])
