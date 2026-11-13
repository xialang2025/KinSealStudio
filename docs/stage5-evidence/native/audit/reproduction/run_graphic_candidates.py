"""Observe canonical defaults of the remaining static families without opening product compiler gates."""
from pathlib import Path
import json
import os
import subprocess
import sys

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence/native/audit'
cases=[('ellipse','ellipse/native/ellipse-empty-default.sm'),('roundrect','roundrect/native/roundrect-default.sm'),('polygon','polygon/native/polygon-right-three-point.sm'),('sector','sector/native/sector-default.sm'),('freeline','freeline/native/freeline-default.sm'),('line','line/physical/line-baseline-r1/compiled/physical-native-reference/input-native.sm')]
scenario=root/'audit-default.json';scenario.write_text(json.dumps([dict(op='scene-read',label='canonical-input'),dict(op='editor-screenshot',label='canonical-input'),dict(op='native-compile',label='canonical-default')],indent=2),encoding='utf-8')
environment=os.environ.copy();environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
for family,relative in cases:
    name='audit-'+family+'-default-r1';source=work/'docs/stage4-evidence'/relative
    result=subprocess.run([sys.executable,'-B',str(root/'native_probe.py'),'--family','audit','--run',name,'--input',str(source),'--no-clear','--reopen-only','--scenario',str(scenario)],capture_output=True,env=environment,timeout=100,creationflags=subprocess.CREATE_NO_WINDOW)
    (root/(name+'.log')).write_bytes(result.stdout+result.stderr);print(family,result.returncode,flush=True)
    assert result.returncode==0,result.stderr.decode('utf-8',errors='replace')
    evidence=json.loads((base/name/'native-probe.json').read_text(encoding='utf-8'));assert evidence['status']=='native-stage4-graphics-scenario-completed' and len(evidence['nativeCompilations'])==1
