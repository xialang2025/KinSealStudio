"""Collect saved/reopened canonical rounded rectangles, serially and without USB."""
from pathlib import Path
import json
import os
import subprocess
import sys

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
base=work/'docs/stage5-evidence/native/roundrect'
source=work/'docs/stage4-evidence/roundrect/native/roundrect-default.sm'
environment=os.environ.copy();environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))

def run(name,input,scenario):
    result=subprocess.run([sys.executable,'-B',str(root/'native_probe.py'),'--family','roundrect','--run',name,'--input',str(input),'--no-clear','--reopen-only','--scenario',str(scenario)],capture_output=True,env=environment,timeout=100,creationflags=subprocess.CREATE_NO_WINDOW)
    (root/(name+'.log')).write_bytes(result.stdout+result.stderr)
    print(name,result.returncode,flush=True)
    assert result.returncode==0,result.stderr.decode('utf-8',errors='replace')
    record=json.loads((base/name/'native-probe.json').read_text(encoding='utf-8'))
    assert record['status']=='native-stage4-graphics-scenario-completed' and record['ownedProcessTerminated'] and not record['usbOrPlc']

for index in range(3):
    run(f'roundrect-default-repeat-r{index+1}',source,root/'repeat-one.json')

def pair(name,commands):
    scenario=[*commands,dict(op='scene-read',label=name),dict(op='save',output=name+'.sm'),dict(op='native-compile',label='created')]
    path=root/(name+'.json');path.write_text(json.dumps(scenario,ensure_ascii=False,indent=2),encoding='utf-8')
    run(name+'-create-r1',source,path)
    run(name+'-reopen-r1',base/(name+'-create-r1')/(name+'.sm'),root/'repeat-one.json')

for name,start,end in [('shifted',[75,65],[235,185]),('wide',[100,100],[400,250]),('tall',[100,100],[220,350]),('near-edge',[1,1],[201,101]),('far-edge',[598,378],[798,478]),('odd',[100,100],[301,201])]:
    pair('roundrect-'+name,[dict(op='scene-select',rects=[[100,100,200,100]]),dict(op='action',name='action_delete'),dict(op='action',name='action_RoundedRect'),{'op':'scene-drag','from':start,'to':end}])
for line,fill in ((0,0),(0,1),(1,0)):
    pair(f'roundrect-options-{line}-{fill}',[dict(op='scene-double',x=200,y=150),dict(op='combo',name='m_LineType',value=line),dict(op='combo',name='m_Pattern',value=fill),dict(op='dump',label='line-fill-switches'),dict(op='invoke',text='确定',kind='QPushButton')])
pair('roundrect-two-objects',[dict(op='action',name='action_RoundedRect'),{'op':'scene-drag','from':[400,100],'to':[600,200]}])
