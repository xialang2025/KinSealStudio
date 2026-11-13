"""Test one declared native gesture change and reject empty or incorrect endpoint sources before reopening."""
from pathlib import Path
import argparse
import json
import os
import struct
import subprocess
import sys

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence/native/freeline';source=work/'docs/stage4-evidence/freeline/native/freeline-default.sm'
parser=argparse.ArgumentParser();parser.add_argument('--only');parser.add_argument('--skip',default='');parser.add_argument('--tiny-two',action='store_true');parser.add_argument('--boundary-forty',action='store_true');parser.add_argument('--near-edge',action='store_true');args=parser.parse_args()
environment=os.environ.copy();environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
variants=[('shifted',[75,65],[235,185]),('width',[100,100],[350,200]),('one-pixel',[100,100],[101,101]),('screen-edge',[0,0],[200,100]),('reversed',[280,200],[100,100]),('horizontal',[100,100],[300,100]),('vertical',[100,100],[100,300]),('steep',[100,100],[200,350]),('descending',[100,300],[350,100]),('one-horizontal',[100,100],[101,100]),('one-vertical',[100,100],[100,101])]
revision='r2'
if args.tiny_two:
    variants=[('two-pixel',[100,100],[102,102]),('two-horizontal',[100,100],[102,100]),('two-vertical',[100,100],[100,102])];revision='r3'
if args.boundary_forty:
    variants=[('boundary-diagonal',[100,100],[140,140]),('boundary-horizontal',[100,100],[140,100]),('boundary-vertical',[100,100],[100,140])];revision='r4'
if args.near_edge:
    variants=[('near-edge',[1,1],[201,101])];revision='r5'
records=[]
for name,start,end in variants:
    if args.only and name!=args.only:continue
    if name in args.skip.split(','):continue
    points=[start,*[[round(start[0]+(end[0]-start[0])*t/5),round(start[1]+(end[1]-start[1])*t/5)] for t in range(1,5)],end]
    scenario=[dict(op='scene-select',rects=[[100,100,180,100]]),dict(op='action',name='action_delete'),dict(op='action',name='action_freeline'),dict(op='scene-drag-path',points=points),dict(op='scene-read',label='freeline-'+name),dict(op='save',output='freeline-'+name+'.sm'),dict(op='native-compile',label='created')]
    recipe=root/('freeline-'+name+'-'+revision+'.json');recipe.write_text(json.dumps(scenario,ensure_ascii=False,indent=2),encoding='utf-8')
    for suffix,input,path in [('create-'+revision,source,recipe),('reopen-'+revision,base/('freeline-'+name+'-create-'+revision)/('freeline-'+name+'.sm'),root/'repeat-one.json')]:
        run='freeline-'+name+'-'+suffix
        result=subprocess.run([sys.executable,'-B',str(root/'native_probe.py'),'--family','freeline','--run',run,'--input',str(input),'--no-clear','--reopen-only','--scenario',str(path)],capture_output=True,env=environment,timeout=100,creationflags=subprocess.CREATE_NO_WINDOW)
        (root/(run+'.log')).write_bytes(result.stdout+result.stderr)
        data=json.loads((base/run/'native-probe.json').read_text(encoding='utf-8'));assert result.returncode==0 and data['status']=='native-stage4-graphics-scenario-completed' and len(data['nativeCompilations'])==1
        c=data['nativeCompilations'][0];raw=(base/run/'compiled'/c['label']/'input-native.sm').read_bytes();offset=raw.find(struct.pack('>I',65576));assert offset>=0
        count=struct.unpack_from('>I',raw,offset+4)[0];actual=[struct.unpack_from('>Idd',raw,offset+8+20*i) for i in range(count)] if count<=16 else []
        matched=count==2 and actual[0][0]==0 and actual[1][0]==1 and list(actual[0][1:])==start and list(actual[1][1:])==end
        record=dict(run=run,expected=[start,end],count=count,actual=actual,endpointsMatched=matched,usbAccessed=False)
        (base/run/'endpoint-assertion.json').write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');records.append(record)
        print(json.dumps(record,ensure_ascii=False),flush=True)
        if not matched:break
(root/('freeline-gesture-'+revision+'-'+(args.only or 'all')+'.json')).write_text(json.dumps(records,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
raise SystemExit(0 if all(r['endpointsMatched'] for r in records) else 1)
