"""Create valid boundary samples with normal native UI and reopen in fresh processes."""
from pathlib import Path
import json
import os
import subprocess
import sys

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
base=work/'docs/stage5-evidence/native/table'
environment=os.environ.copy();environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
def run(name,source,scenario):
    result=subprocess.run([sys.executable,'-B',str(root/'native_probe.py'),'--family','table','--run',name,'--input',str(source),'--no-clear','--reopen-only','--scenario',str(scenario)],capture_output=True,env=environment,timeout=100,creationflags=subprocess.CREATE_NO_WINDOW)
    (root/(name+'.log')).write_bytes(result.stdout+result.stderr)
    print(name,result.returncode,flush=True)
    assert result.returncode==0, result.stderr.decode('utf-8',errors='replace')

source=work/'docs/stage4-evidence/table/native/table-default.sm'
run('table-default-repeat-r3',source,root/'repeat-one.json')
for name,rows,cols,start,end in [('one-cell',1,1,[100,100],[101,101]),('three-by-two',3,2,[75,65],[235,185]),('thirty-boundary',30,30,[0,0],[600,300])]:
    scenario=[dict(op='scene-select',rects=[[100,100,200,100]]),dict(op='action',name='action_delete'),dict(op='action',name='action_table'),
              dict(op='spin',name='m_hnumberspin',value=rows),dict(op='spin',name='m_Vnumberspin',value=cols),
              dict(op='dump',label='table-'+name+'-properties'),dict(op='invoke',text='确定',kind='QPushButton'),
              {'op':'scene-drag','from':start,'to':end},dict(op='scene-read',label='table-'+name),
              dict(op='save',output='table-'+name+'.sm'),dict(op='native-compile',label='created')]
    path=root/('table-'+name+'.json');path.write_text(json.dumps(scenario,ensure_ascii=False,indent=2),encoding='utf-8')
    run('table-'+name+'-create-r1',source,path)
    run('table-'+name+'-reopen-r1',base/('table-'+name+'-create-r1')/('table-'+name+'.sm'),root/'repeat-one.json')
