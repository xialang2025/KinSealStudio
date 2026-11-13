"""Test counter-slopes and short axial lines before admitting a general integer-line raster."""
from pathlib import Path
import json
import os
import subprocess
import sys

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence/native/line';source=work/'docs/stage4-evidence/line/physical/line-baseline-r1/compiled/physical-native-reference/input-native.sm'
environment=os.environ.copy();environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
for name,start,end in [('steep',[100,100],[200,350]),('descending',[100,300],[350,100]),('one-horizontal',[100,100],[101,100]),('one-vertical',[100,100],[100,101])]:
    scenario=[dict(op='scene-select',rects=[[100,100,200,100]]),dict(op='action',name='action_delete'),dict(op='action',name='action_line'),{'op':'scene-drag','from':start,'to':end},dict(op='scene-read',label=name),dict(op='save',output='line-'+name+'.sm'),dict(op='native-compile',label='created')]
    path=root/('line-'+name+'.json');path.write_text(json.dumps(scenario,ensure_ascii=False,indent=2),encoding='utf-8')
    for suffix,input,recipe in [('create-r1',source,path),('reopen-r1',base/('line-'+name+'-create-r1')/('line-'+name+'.sm'),root/'repeat-one.json')]:
        run='line-'+name+'-'+suffix;result=subprocess.run([sys.executable,'-B',str(root/'native_probe.py'),'--family','line','--run',run,'--input',str(input),'--no-clear','--reopen-only','--scenario',str(recipe)],capture_output=True,env=environment,timeout=100,creationflags=subprocess.CREATE_NO_WINDOW);(root/(run+'.log')).write_bytes(result.stdout+result.stderr)
        d=json.loads((base/run/'native-probe.json').read_text(encoding='utf-8'));print(run,result.returncode,d['status'],flush=True);assert result.returncode==0 and d['status']=='native-stage4-graphics-scenario-completed'
