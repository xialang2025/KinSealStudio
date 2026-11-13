"""Observe native rectangle geometry and canonical saved-source compilation, never transmission."""
from pathlib import Path
import json
import os
import subprocess
import sys

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
base=work/'docs/stage5-evidence/native/rect'
source=work/'docs/stage4-evidence/rect/native/rect-default.sm'
environment=os.environ.copy();environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
def run(name,input,scenario):
    result=subprocess.run([sys.executable,'-B',str(root/'native_probe.py'),'--family','rect','--run',name,'--input',str(input),'--no-clear','--reopen-only','--scenario',str(scenario)],capture_output=True,env=environment,timeout=100,creationflags=subprocess.CREATE_NO_WINDOW)
    (root/(name+'.log')).write_bytes(result.stdout+result.stderr)
    print(name,result.returncode,flush=True)
    assert result.returncode==0, result.stderr.decode('utf-8',errors='replace')
for index in range(3):
    run(f'rect-default-repeat-r{index+1}',source,root/'repeat-one.json')
for name,start,end in [('shifted',[75,65],[235,185]),('screen-edge',[0,0],[200,100]),('minimum',[100,100],[101,101])]:
    scenario=[dict(op='scene-select',rects=[[100,100,200,100]]),dict(op='action',name='action_delete'),dict(op='action',name='action_rect'),
              {'op':'scene-drag','from':start,'to':end},dict(op='scene-read',label='rect-'+name),
              dict(op='save',output='rect-'+name+'.sm'),dict(op='native-compile',label='created')]
    path=root/('rect-'+name+'.json');path.write_text(json.dumps(scenario,ensure_ascii=False,indent=2),encoding='utf-8')
    run('rect-'+name+'-create-r1',source,path)
    run('rect-'+name+'-reopen-r1',base/('rect-'+name+'-create-r1')/('rect-'+name+'.sm'),root/'repeat-one.json')
