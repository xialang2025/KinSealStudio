"""Sample one-field native rectangle line/fill switches and resize/second-object boundaries serially."""
from pathlib import Path
import json
import os
import subprocess
import sys
import argparse

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence/native/rect'
source=work/'docs/stage4-evidence/rect/native/rect-default.sm';environment=os.environ.copy();environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
parser=argparse.ArgumentParser();parser.add_argument('--only',choices=['all','two-objects'],default='all');args=parser.parse_args()
def run(name,commands):
    scenario=[*commands,dict(op='scene-read',label=name),dict(op='save',output=name+'.sm'),dict(op='native-compile',label='created')]
    path=root/(name+'.json');path.write_text(json.dumps(scenario,ensure_ascii=False,indent=2),encoding='utf-8')
    for suffix,input,recipe in [('create-r1',source,path),('reopen-r1',base/(name+'-create-r1')/(name+'.sm'),root/'repeat-one.json')]:
        label=name+'-'+suffix
        result=subprocess.run([sys.executable,'-B',str(root/'native_probe.py'),'--family','rect','--run',label,'--input',str(input),'--no-clear','--reopen-only','--scenario',str(recipe)],capture_output=True,env=environment,timeout=100,creationflags=subprocess.CREATE_NO_WINDOW)
        (root/(label+'.log')).write_bytes(result.stdout+result.stderr);print(label,result.returncode,flush=True);assert result.returncode==0,result.stderr.decode('utf-8',errors='replace')
        evidence=json.loads((base/label/'native-probe.json').read_text(encoding='utf-8'));assert evidence['status']=='native-stage4-graphics-scenario-completed',evidence.get('error')
if args.only=='all':
    for line,fill in [(0,0),(0,1),(1,0)]:
        run(f'rect-options-{line}-{fill}',[dict(op='scene-double',x=200,y=150),dict(op='combo',name='Line_Type',value=line),dict(op='combo',name='Pattern',value=fill),dict(op='dump',label='rectangle-line-fill-options'),dict(op='invoke',text='确定',kind='QPushButton')])
    run('rect-resized-small',[dict(op='scene-select',rects=[[100,100,200,100]]),{'op':'scene-drag','from':[300,200],'to':[101,101]}])
run('rect-two-objects',[dict(op='action',name='action_rect'),{'op':'scene-drag','from':[400,100],'to':[500,200]}])
