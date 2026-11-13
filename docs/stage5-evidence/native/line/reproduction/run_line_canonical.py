"""Sample repeatability, slopes, endpoint direction and axial carriers with the native line tool."""
from pathlib import Path
import json
import os
import subprocess
import sys

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence/native/line'
source=work/'docs/stage4-evidence/line/physical/line-baseline-r1/compiled/physical-native-reference/input-native.sm';environment=os.environ.copy();environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
record=dict(usbAccessed=False,runs=[])
def run(name,input,scenario):
    result=subprocess.run([sys.executable,'-B',str(root/'native_probe.py'),'--family','line','--run',name,'--input',str(input),'--no-clear','--reopen-only','--scenario',str(scenario)],capture_output=True,env=environment,timeout=100,creationflags=subprocess.CREATE_NO_WINDOW)
    log=root/(name+'.log');log.write_bytes(result.stdout+result.stderr);probe=base/name/'native-probe.json';data=json.loads(probe.read_text(encoding='utf-8')) if probe.exists() else {}
    passed=result.returncode==0 and data.get('status')=='native-stage4-graphics-scenario-completed' and len(data.get('nativeCompilations',[]))==1
    record['runs'].append(dict(name=name,passed=passed,exitCode=result.returncode,error=data.get('error'),log=str(log)));(root/'line-canonical-run.json').write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(name,passed,flush=True)
    return passed
for index in range(1,4):run(f'line-default-repeat-r{index}',source,root/'repeat-one.json')
for name,start,end in [('shifted',[75,65],[235,185]),('width',[100,100],[350,200]),('one-pixel',[100,100],[101,101]),('screen-edge',[0,0],[200,100]),('reversed',[300,200],[100,100]),('horizontal',[100,100],[300,100]),('vertical',[100,100],[100,300])]:
    scenario=[dict(op='scene-select',rects=[[100,100,200,100]]),dict(op='action',name='action_delete'),dict(op='action',name='action_line'),{'op':'scene-drag','from':start,'to':end},dict(op='scene-read',label='line-'+name),dict(op='save',output='line-'+name+'.sm'),dict(op='native-compile',label='created')]
    path=root/('line-'+name+'.json');path.write_text(json.dumps(scenario,ensure_ascii=False,indent=2),encoding='utf-8')
    if run('line-'+name+'-create-r1',source,path):run('line-'+name+'-reopen-r1',base/('line-'+name+'-create-r1')/('line-'+name+'.sm'),root/'repeat-one.json')
