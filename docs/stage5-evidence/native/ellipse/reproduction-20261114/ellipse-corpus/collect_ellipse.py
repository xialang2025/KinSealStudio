"""Collect source-bound native ellipse observations on the owned private desktop."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import subprocess
import sys

HERE=Path(__file__).resolve().parent
ROOT=HERE.parent
WORK=ROOT.parent/'kinseal-stage4-20261112/worktree'
BASE=WORK/'docs/stage5-evidence/native/ellipse'
SOURCE=WORK/'docs/stage4-evidence/ellipse/native/ellipse-default.sm'
ENV=os.environ.copy()
ENV.update(TEMP=str(ROOT/'temp'),TMP=str(ROOT/'temp'),TMPDIR=str(ROOT/'temp'),PYTHONIOENCODING='utf-8')
IDENTITY=[1,0,0,1,0,0,0,0,1]
ISOLATED=BASE/'ellipse-isolate-default-r1/ellipse-default-isolated.sm'


def write_json(path,value):
    """Preserve UTF-8 diagnostics without writing intermediate files on C."""
    path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')


def observe(name,source,commands):
    """Run one owned compiler phase and stop before recording any invalid target."""
    scenario=HERE/(name+'.scenario.json')
    output=BASE/name
    if output.exists():
        raise RuntimeError('Refuse to replace retained evidence: '+str(output))
    write_json(scenario,commands)
    result=subprocess.run([sys.executable,'-B',str(ROOT/'native_paint_probe.py'),'--family','ellipse','--run',name,'--input',str(source),'--no-clear','--reopen-only','--scenario',str(scenario)],capture_output=True,env=ENV,timeout=120,creationflags=subprocess.CREATE_NO_WINDOW)
    (HERE/(name+'.log')).write_bytes(result.stdout+result.stderr)
    probe=output/'native-probe.json'
    record=json.loads(probe.read_text(encoding='utf-8')) if probe.is_file() else {}
    if result.returncode!=0 or record.get('status')!='native-stage4-graphics-scenario-completed' or not record.get('ownedProcessTerminated') or record.get('usbOrPlc') or record.get('systemInput'):
        raise RuntimeError('Owned native process did not complete safely: '+name+' '+str(record.get('error')))
    if len(record.get('nativeCompilations',[]))!=1:
        raise RuntimeError('Expected one fresh native compilation: '+name)
    roots=[item for item in record['sceneReads'][-1]['data']['items'] if item['parent']=='0x0' and item['flags']&2]
    paints=[message['payload'] for message in record.get('instrumentationMessages',[]) if message.get('payload',{}).get('tag')=='stage5-native-paint']
    return record,roots,paints


def assert_target(name,record,roots,paints,rects,line,fill,properties=False):
    """Bind fresh native output to the complete expected roots, painter state and saved source."""
    actual=sorted(item['rect'] for item in roots)
    if len(roots)!=len(rects) or actual!=sorted(rects) or any(item['transform']!=IDENTITY for item in roots):
        raise RuntimeError('Actual root geometry mismatch: '+name+' '+str(actual))
    valid=[paint for paint in paints if 'drawEllipse' in paint['symbol'] and paint.get('transform')==IDENTITY and paint['pen']==dict(style=line,width=0,rgba=4278190080,cap=16,join=64) and paint.get('hints')==11]
    if not all(any(paint.get('rect')==rect for paint in valid) for rect in rects):
        raise RuntimeError('Actual native ellipse rectangle, pen or transform mismatch: '+name+' '+str(paints))
    if properties:
        snapshots=[snapshot for snapshot in record['snapshots'] if snapshot['label']=='ellipse-properties']
        widgets=[widget['qt'] for snapshot in snapshots for window in snapshot['windows'] for widget in window['widgets'] if widget.get('qt')]
        combos={widget['objectName']:int(widget['currentIndex']) for widget in widgets if widget['kind']=='QComboBox' and widget.get('objectName') in ('Line_Type','Pattern')}
        if combos!=dict(Line_Type=line,Pattern=fill):
            raise RuntimeError('Loaded line/fill property persistence mismatch: '+name+' '+str(combos))
    compilation=record['nativeCompilations'][0]
    source=BASE/name/'compiled'/compilation['label']/'input-native.sm'
    if hashlib.sha256(source.read_bytes()).hexdigest()!=compilation['sourceSha256']:
        raise RuntimeError('Copied native source digest mismatch: '+name)
    return dict(name=name,passed=True,targetGeometryBound=True,lineStyle=line,fillStyle=fill,rects=rects,sourceSha256=compilation['sourceSha256'],inputSourceSha256=record['input']['sha256'],compilationLabel=compilation['label'],propertiesObserved=properties,ownedProcessTerminated=record['ownedProcessTerminated'],usbOrPlc=record['usbOrPlc'],actualEllipsePaints=valid,rootCount=len(roots))


def properties(rect,line,fill,edit):
    """Observe the actual native dialog and save after confirmation marks its source dirty."""
    x,y,width,height=rect
    commands=[dict(op='scene-double',x=x+width//2,y=y+height//2)]
    if edit:
        commands.extend([dict(op='combo',name='Line_Type',value=line),dict(op='combo',name='Pattern',value=fill)])
    commands.extend([dict(op='dump',label='ellipse-properties'),dict(op='invoke',text='确定',kind='QPushButton')])
    return commands


def collect():
    """Collect eight geometry groups, three style groups, repeatability and two-object composition serially."""
    manifest=dict(plannedCompilations=27,canonicalCases=15,independentScenes=13,runs=[],complete=False,usbOrPlc=False,sourcePreparation='ellipse-isolate-default-r1',failedDiagnostics=['ellipse-default-repeat-r1','ellipse-default-repeat-r2'])
    manifest_path=HERE/'ellipse-canonical-run-r1.json'
    index_path=BASE/'valid-target-index-r1.json'

    def run(name,source,commands,rects,line=1,fill=1,observe_properties=False):
        try:
            record,roots,paints=observe(name,source,commands)
            sample=assert_target(name,record,roots,paints,rects,line,fill,observe_properties)
            manifest['runs'].append(sample)
            write_json(manifest_path,manifest)
            write_json(index_path,manifest)
            print(name,'VALID',len(manifest['runs']),flush=True)
        except Exception as error:
            manifest['error']=repr(error)
            manifest['failedRun']=name
            write_json(manifest_path,manifest)
            write_json(index_path,manifest)
            raise

    default=[100,100,200,100]
    for index in range(3,6):
        run('ellipse-default-repeat-r'+str(index),ISOLATED,[dict(op='scene-count-assert',count=1,label='default-count'),dict(op='scene-assert',rects=[default]),dict(op='scene-read',label='default-target'),dict(op='native-compile',label='fresh-process-3')],[default])

    def draw(rect):
        x,y,width,height=rect
        return [dict(op='action',name='action_ellipse'),dict(op='scene-drag',**{'from':[x,y],'to':[x+width,y+height]})]

    def pair(label,commands,rects,line=1,fill=1):
        created='ellipse-'+label+'-create-r1'
        reopened='ellipse-'+label+'-reopen-r1'
        filename='ellipse-'+label+'.sm'
        commands=[*commands,dict(op='scene-count-assert',count=len(rects),label='created-count'),dict(op='scene-assert',rects=rects),*properties(rects[0],line,fill,True),dict(op='scene-read',label='created-target'),dict(op='save',output=filename),dict(op='native-compile',label='created')]
        run(created,ISOLATED,commands,rects,line,fill,True)
        commands=[dict(op='scene-count-assert',count=len(rects),label='reopened-count'),dict(op='scene-assert',rects=rects),*properties(rects[0],line,fill,False),dict(op='save-current'),dict(op='scene-read',label='reopened-target'),dict(op='native-compile',label='fresh-process-3')]
        run(reopened,BASE/created/filename,commands,rects,line,fill,True)

    for label,rect in [('shifted',[75,65,160,120]),('wide',[100,100,300,150]),('tall',[100,100,120,250]),('near-edge',[2,2,200,100]),('far-edge',[498,278,200,100]),('odd',[101,101,201,101]),('minimum',[100,100,100,100]),('circle',[100,100,200,200])]:
        commands=[dict(op='owned-editor-size',width=1800,height=1100),dict(op='scene-count-assert',count=1,label='source-count'),dict(op='scene-assert',rects=[default]),dict(op='scene-select',rects=[default]),dict(op='action',name='action_delete'),dict(op='scene-count-assert',count=0,label='deleted-count'),*draw(rect)]
        pair(label,commands,[rect])
    for line,fill in ((0,0),(0,1),(1,0)):
        pair('options-'+str(line)+'-'+str(fill),[dict(op='scene-count-assert',count=1,label='source-count'),dict(op='scene-assert',rects=[default])],[default],line,fill)
    other=[400,100,200,100]
    pair('two-objects',[dict(op='owned-editor-size',width=1800,height=1100),*draw(other)],[default,other])
    manifest['complete']=True
    manifest['passed']=len(manifest['runs'])==manifest['plannedCompilations'] and all(sample['passed'] for sample in manifest['runs'])
    write_json(manifest_path,manifest)
    write_json(index_path,manifest)
    if not manifest['passed']:
        raise RuntimeError('Completed run count does not match declared coverage')


def main():
    """Inspect the actual source scene before selecting geometry edit commands."""
    parser=argparse.ArgumentParser()
    parser.add_argument('--phase',choices=['inspect','isolate','properties','collect'],default='inspect')
    args=parser.parse_args()
    if args.phase=='collect':
        collect()
        return
    if args.phase=='properties':
        isolated=BASE/'ellipse-isolate-default-r1/ellipse-default-isolated.sm'
        record,roots,paints=observe('ellipse-default-repeat-r2',isolated,[dict(op='scene-count-assert',count=1,label='default-count'),dict(op='scene-assert',rects=[[100,100,200,100]]),dict(op='scene-double',x=200,y=150),dict(op='dump',label='ellipse-default-properties'),dict(op='invoke',text='确定',kind='QPushButton'),dict(op='scene-read',label='actual-default-bbox'),dict(op='native-compile',label='fresh-process-3')])
    elif args.phase=='isolate':
        record,roots,paints=observe('ellipse-isolate-default-r1',SOURCE,[dict(op='scene-count-assert',count=2,label='source-count'),dict(op='scene-assert',rects=[[100,100,200,100],[244,112,150,50]]),dict(op='scene-select',rects=[[244,112,150,50]]),dict(op='action',name='action_delete'),dict(op='scene-count-assert',count=1,label='isolated-count'),dict(op='scene-assert',rects=[[100,100,200,100]]),dict(op='scene-read',label='isolated-ellipse'),dict(op='save',output='ellipse-default-isolated.sm'),dict(op='native-compile',label='created')])
    else:
        record,roots,paints=observe('ellipse-default-repeat-r1',SOURCE,[dict(op='scene-count-assert',count=1,label='default-count'),dict(op='scene-read',label='actual-default-bbox'),dict(op='native-compile',label='fresh-process-3')])
    summary=dict(source=str(SOURCE),sourceSha256=hashlib.sha256(SOURCE.read_bytes()).hexdigest(),roots=roots,paints=paints,ownedProcessTerminated=record['ownedProcessTerminated'],usbOrPlc=record['usbOrPlc'])
    write_json(HERE/'default-inspection.json',summary)
    print(json.dumps(summary,ensure_ascii=False,indent=2),flush=True)


if __name__=='__main__':
    main()
