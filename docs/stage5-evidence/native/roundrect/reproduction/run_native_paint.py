"""One native process at a time; compilation only, never transmission."""
from pathlib import Path
import json
import os
import subprocess
import sys

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
environment=os.environ.copy()
environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
for family,relative in [('roundrect','roundrect/native/roundrect-default.sm')]:
    name='paint-'+family+'-stroke-r3'
    result=subprocess.run([sys.executable,'-B',str(root/'native_paint_probe.py'),'--family','audit','--run',name,'--input',str(work/'docs/stage4-evidence'/relative),'--no-clear','--reopen-only','--scenario',str(root/'repeat-one.json')],capture_output=True,env=environment,timeout=100,creationflags=subprocess.CREATE_NO_WINDOW)
    (root/(name+'.log')).write_bytes(result.stdout+result.stderr)
    assert result.returncode==0,result.stderr.decode('utf-8',errors='replace')
    record=json.loads((work/'docs/stage5-evidence/native/audit'/name/'native-probe.json').read_text(encoding='utf-8'))
    messages=[x.get('payload',x) for x in record.get('instrumentationMessages',[]) if x.get('payload',{}).get('tag','').startswith('stage5-native-paint')]
    print(family,[(m.get('tag'),len(m.get('nativeStrokeStreamHex','')),m.get('error')) for m in messages],flush=True)
