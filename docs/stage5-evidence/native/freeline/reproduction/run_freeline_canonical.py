"""Create and reopen the native two-endpoint freeline slope and boundary corpus, without USB or foreground input."""
from pathlib import Path
import json
import os
import subprocess
import sys

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence/native/freeline'
source=work/'docs/stage4-evidence/freeline/native/freeline-default.sm'
environment=os.environ.copy();environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
record=dict(usbAccessed=False,runs=[],complete=False)
def run(name,input,scenario):
    result=subprocess.run([sys.executable,'-B',str(root/'native_probe.py'),'--family','freeline','--run',name,'--input',str(input),'--no-clear','--reopen-only','--scenario',str(scenario)],capture_output=True,env=environment,timeout=100,creationflags=subprocess.CREATE_NO_WINDOW)
    log=root/(name+'.log');log.write_bytes(result.stdout+result.stderr);probe=base/name/'native-probe.json';data=json.loads(probe.read_text(encoding='utf-8')) if probe.exists() else {}
    passed=result.returncode==0 and data.get('status')=='native-stage4-graphics-scenario-completed' and len(data.get('nativeCompilations',[]))==1
    record['runs'].append(dict(name=name,passed=passed,exitCode=result.returncode,error=data.get('error'),log=str(log)))
    (root/'freeline-canonical-run.json').write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(name,passed,flush=True)
    return passed
for index in range(1,4):run(f'freeline-default-repeat-r{index}',source,root/'repeat-one.json')
for name,start,end in [('shifted',[75,65],[235,185]),('width',[100,100],[350,200]),('one-pixel',[100,100],[101,101]),('screen-edge',[0,0],[200,100]),('reversed',[280,200],[100,100]),('horizontal',[100,100],[300,100]),('vertical',[100,100],[100,300]),('steep',[100,100],[200,350]),('descending',[100,300],[350,100]),('one-horizontal',[100,100],[101,100]),('one-vertical',[100,100],[100,101])]:
    scenario=[dict(op='scene-select',rects=[[100,100,180,100]]),dict(op='action',name='action_delete'),dict(op='action',name='action_freeline'),{'op':'scene-drag','from':start,'to':end},dict(op='scene-read',label='freeline-'+name),dict(op='save',output='freeline-'+name+'.sm'),dict(op='native-compile',label='created')]
    path=root/('freeline-'+name+'.json');path.write_text(json.dumps(scenario,ensure_ascii=False,indent=2),encoding='utf-8')
    if run('freeline-'+name+'-create-r1',source,path):run('freeline-'+name+'-reopen-r1',base/('freeline-'+name+'-create-r1')/('freeline-'+name+'.sm'),root/'repeat-one.json')
record['complete']=True;record['passed']=all(r['passed'] for r in record['runs'])
(root/'freeline-canonical-run.json').write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
raise SystemExit(0 if record['passed'] else 1)
