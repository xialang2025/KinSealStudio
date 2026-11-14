"""Collect bounded polygons serially through the owned original UI and passive paint trace."""
from pathlib import Path
import argparse
import json
import os
import subprocess
import sys

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
parser=argparse.ArgumentParser();parser.add_argument('--run',default='r1');parser.add_argument('--reuse-defaults',action='store_true');parser.add_argument('--minimum-only',action='store_true');parser.add_argument('--resume-from');args=parser.parse_args()
base=work/'docs/stage5-evidence/native/polygon';source=work/'docs/stage4-evidence/polygon/native/polygon-right-three-point.sm'
env=os.environ.copy();env.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
record=dict(plannedCompilations=37,runs=[],complete=False,usbAccessed=False)
default=[[100,100],[300,100],[250,220]]
cases=[('shifted',[[75,65],[235,65],[195,185]]),('wide',[[100,100],[400,100],[350,250]]),('tall',[[100,100],[220,100],[190,350]]),('near-edge',[[2,2],[202,2],[152,122]]),('far-edge',[[598,358],[798,358],[748,478]]),('reversed',list(reversed(default))),('quadrilateral',[[100,100],[300,100],[270,220],[130,230]]),('concave-five',[[100,100],[300,100],[210,150],[280,230],[120,230]]),('hexagon',[[130,100],[270,100],[300,150],[270,230],[130,230],[100,150]]),('heptagon',[[140,100],[260,100],[300,140],[280,210],[200,240],[120,210],[100,140]]),('octagon',[[120,100],[280,100],[300,130],[300,200],[270,230],[130,230],[100,200],[100,130]]),('odd',[[101,101],[302,101],[252,202]])]
cases.append(('minimum',[[100,100],[200,100],[150,200]]))
cases=[(name,[[498,278],[698,278],[648,398]] if name=='far-edge' else points) for name,points in cases]
def run(name,input,scenario,polygons,line=1,fill=1):
 p=base/name/'native-probe.json';assert not p.exists(),name
 result=subprocess.run([sys.executable,'-B',str(root/'native_paint_probe.py'),'--family','polygon','--run',name,'--input',str(input),'--no-clear','--reopen-only','--scenario',str(scenario)],capture_output=True,env=env,timeout=120,creationflags=subprocess.CREATE_NO_WINDOW)
 (root/(name+'.log')).write_bytes(result.stdout+result.stderr)
 data=json.loads(p.read_text(encoding='utf-8')) if p.is_file() else {}
 traces=[m['payload'] for m in data.get('instrumentationMessages',[]) if m.get('payload',{}).get('polygon')]
 semantic=all(any(t['polygon']['points']==points and t['polygon']['fillRule']==0 and t['pen']['style']==line and t['transform']==[1,0,0,1,0,0,0,0,1] for t in traces) for points in polygons)
 passed=result.returncode==0 and data.get('status')=='native-stage4-graphics-scenario-completed' and len(data.get('nativeCompilations',[]))==1 and data.get('ownedProcessTerminated') and semantic
 record['runs'].append(dict(name=name,passed=bool(passed),targetVerticesBound=semantic,exitCode=result.returncode,error=data.get('error'),polygons=polygons,lineStyle=line,fillStyle=fill))
 (root/('polygon-canonical-run-'+args.run+'.json')).write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(name,bool(passed),flush=True)
 if not passed:raise RuntimeError('Native target or execution failed; stop before sampling more variants: '+name)
 return passed
def draw(points):
 return [dict(op='action',name='action_polygon'),*[command for x,y in points for command in (dict(op='scene-move',x=x,y=y),dict(op='scene-click',x=x,y=y))],dict(op='scene-click',x=points[-1][0],y=points[-1][1],button='right'),dict(op='scene-move',x=700,y=400)]
def pair(name,commands,polygons,line=1,fill=1):
 if args.resume_from and any(r['passed'] and r['name'].startswith('polygon-'+name+'-reopen-') for r in record['runs']):return
 label='polygon-'+name;scenario=root/(label+'-'+args.run+'.json');scenario.write_text(json.dumps(commands+[dict(op='scene-count-assert',count=len(polygons),label=label),dict(op='scene-read',label=label),dict(op='save',output=label+'.sm'),dict(op='native-compile',label='created')],ensure_ascii=False,indent=2),encoding='utf-8')
 if run(label+'-create-'+args.run,source,scenario,polygons,line,fill):run(label+'-reopen-'+args.run,base/(label+'-create-'+args.run)/(label+'.sm'),root/'repeat-one.json',polygons,line,fill)
if args.reuse_defaults:
 previous=json.loads((root/'polygon-canonical-run.json').read_text(encoding='utf-8'))
 record['runs']=[r for r in previous['runs'] if r['name'].startswith('polygon-default-repeat-')]
 assert len(record['runs'])==3 and all(r['passed'] for r in record['runs'])
else:
 for i in range(1,4):run('polygon-default-repeat-r'+str(i),source,root/'repeat-one.json',[default])
if args.resume_from:
 record['runs']=[r for r in json.loads((root/('polygon-canonical-run-'+args.resume_from+'.json')).read_text(encoding='utf-8'))['runs'] if r['passed']]
if args.minimum_only:
 cases=[('minimum',[[100,100],[200,100],[150,200]])];record['plannedCompilations']=5
for name,points in cases:
 gesture=draw(points)
 if name=='far-edge':
  pair(name,[dict(op='owned-editor-size',width=1800,height=1100),dict(op='scene-assert',rects=[[100,100,200,120]]),dict(op='scene-select',rects=[[100,100,200,120]]),dict(op='action',name='action_delete'),*gesture],[points])
 else:
  pair(name,[dict(op='scene-assert',rects=[[100,100,200,120]]),dict(op='scene-select',rects=[[100,100,200,120]]),dict(op='action',name='action_delete'),*gesture],[points])
for line,fill in (() if args.minimum_only else ((0,0),(0,1),(1,0))):
 pair(f'options-{line}-{fill}',[dict(op='scene-double',x=220,y=150),dict(op='combo',name='Line_Type',value=line),dict(op='combo',name='Pattern',value=fill),dict(op='dump',label='line-fill-switches'),dict(op='invoke',text='确定',kind='QPushButton')],[default],line,fill)
other=[[400,100],[600,100],[550,220]]
if not args.minimum_only:pair('two-objects',draw(other),[default,other])
record['complete']=True;record['passed']=len(record['runs'])==record['plannedCompilations'] and all(r['passed'] for r in record['runs']);(root/('polygon-canonical-run-'+args.run+'.json')).write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
raise SystemExit(0 if record['passed'] else 1)
