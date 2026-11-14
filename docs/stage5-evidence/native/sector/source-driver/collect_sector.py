"""Collect source-bound default-angle sectors through the owned original offline editor."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import struct
import subprocess
import sys

HERE=Path(__file__).resolve().parent
ROOT=HERE.parent
WORK=ROOT.parent/'kinseal-stage4-20261112/worktree'
BASE=WORK/'docs/stage5-evidence/native/sector'
SOURCE=WORK/'docs/stage4-evidence/sector/native/sector-default.sm'
ENV=os.environ.copy()
ENV.update(TEMP=str(ROOT/'temp'),TMP=str(ROOT/'temp'),TMPDIR=str(ROOT/'temp'),PYTHONIOENCODING='utf-8')
IDENTITY=[1,0,0,1,0,0,0,0,1]
CLI=ROOT/'sector_followup/sector_curve_cli.exe'
TEMP=HERE/'temp'
ENV['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+ENV.get('PATH','')
PATH_CACHE={}


def write_json(path,value):
    """Retain UTF-8 observations entirely inside the owned Z task/evidence roots."""
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')


def parse_path(raw):
    """Read the versioned native path stream without inventing a closing element."""
    count=struct.unpack_from('>I',raw)[0]
    assert len(raw)==4+count*20+8
    return [list(struct.unpack_from('>Idd',raw,4+index*20)) for index in range(count)],list(struct.unpack_from('>II',raw,4+count*20))


def observe(name,source,commands,compilations=1):
    """Launch only one owned native helper and refuse replaced evidence or incomplete process cleanup."""
    scenario=HERE/(name+'.scenario.json')
    output=BASE/name
    if output.exists():
        raise RuntimeError('Refuse to replace existing evidence: '+str(output))
    write_json(scenario,commands)
    result=subprocess.run([sys.executable,'-B',str(ROOT/'native_paint_probe.py'),'--family','sector','--run',name,'--input',str(source),'--no-clear','--reopen-only','--scenario',str(scenario)],capture_output=True,env=ENV,timeout=120,creationflags=subprocess.CREATE_NO_WINDOW)
    (HERE/(name+'.log')).write_bytes(result.stdout+result.stderr)
    record=json.loads((output/'native-probe.json').read_text(encoding='utf-8'))
    if result.returncode or record.get('status')!='native-stage4-graphics-scenario-completed' or not record.get('ownedProcessTerminated') or record.get('usbOrPlc') or record.get('systemInput'):
        raise RuntimeError('Owned process or target execution failed: '+name+' '+str(record.get('error')))
    assert len(record.get('nativeCompilations',[]))==compilations
    roots=[item for item in record['sceneReads'][-1]['data']['items'] if item['parent']=='0x0' and item['flags']&2]
    paints=[message['payload'] for message in record.get('instrumentationMessages',[]) if message.get('payload',{}).get('tag')=='stage5-native-paint']
    return record,roots,paints


def candidate(rect):
    """Construct the independent local eight-element arc and map its explicit integer translation."""
    key=tuple(rect)
    if key not in PATH_CACHE:
        TEMP.mkdir(exist_ok=True)
        prefix=TEMP/('scene-'+'-'.join(map(str,rect)))
        if not Path(str(prefix)+'.json').exists():
            result=subprocess.run([str(CLI),str(prefix),*map(str,rect),'0','1'],capture_output=True,env=ENV,creationflags=subprocess.CREATE_NO_WINDOW)
            if result.returncode:
                raise RuntimeError('Independent local-path CLI failed: '+str((rect,result.returncode,result.stderr)))
        details=json.loads(Path(str(prefix)+'.json').read_text(encoding='utf-8'))
        path=Path(str(prefix)+'-path.pathstream').read_bytes()
        elements,tail=parse_path(path)
        assert details['geometry']==rect and details['transform']==[1,0,0,1,rect[0],rect[1],0,0,1]
        assert len(details['localElements'])==8 and details['pathExplicitlyClosed'] is False
        PATH_CACHE[key]=dict(details=details,path=path,elements=elements,tail=tail,prefix=str(prefix))
    return PATH_CACHE[key]


def assert_target(name,record,roots,paints,rects,line,fill,property_count=0):
    """Bind unchanged raw local paths and transforms to separately serialized scene semantics."""
    if len(roots)!=len(rects) or not all(any(all(abs(root['rect'][index]-value)<=1e-8 for index,value in enumerate(rect)) for root in roots) for rect in rects):
        raise RuntimeError('Actual sector root geometry/count mismatch: '+name)
    observed=[]
    for rect in rects:
        x,y,width,height=rect
        expected_transform=[1,0,0,1,x,y,0,0,1]
        root=next(root for root in roots if all(abs(root['rect'][index]-value)<=1e-8 for index,value in enumerate(rect)))
        if root['transform']!=expected_transform:
            raise RuntimeError('Actual item transform is not exact bbox translation: '+name)
        expected=candidate(rect)
        bound=[]
        for paint in paints:
            if 'drawPath' not in paint['symbol'] or paint.get('pen')!=dict(style=line,width=0,rgba=4278190080,cap=16,join=64) or paint.get('hints')!=3:
                continue
            if paint['transform']!=expected_transform:
                continue
            raw=bytes.fromhex(paint['pathStreamHex'])
            local,tail=parse_path(raw)
            if len(local)!=8 or [element[0] for element in local]!=[0,1,2,3,3,2,3,3] or tail!=[0,0]:
                continue
            scene=[[tag,px+x,py+y] for tag,px,py in local]
            if [element[0] for element in scene]!=[element[0] for element in expected['elements']]:
                continue
            float_difference=max(abs(scene[index][axis]-expected['elements'][index][axis]) for index in range(8) for axis in (1,2))
            quantized_equal=all(int(scene[index][axis]*64)==int(expected['elements'][index][axis]*64) for index in range(8) for axis in (1,2))
            if float_difference>1e-7 or not quantized_equal:
                continue
            mapped=struct.pack('>I',8)+b''.join(struct.pack('>Idd',*element) for element in scene)+struct.pack('>II',*tail)
            bound.append(dict(rect=rect,rawSceneBoundingRect=root['rect'],sceneBoundingRectDelta=[root['rect'][index]-value for index,value in enumerate(rect)],sceneBoundingRectIntegerTolerance=1e-8,itemTransform=root['transform'],painterTransform=paint['transform'],localPathStreamHex=paint['pathStreamHex'],localPathElements=local,localPathSha256=hashlib.sha256(raw).hexdigest(),mappedScenePathStreamHex=mapped.hex(),mappedScenePathElements=scene,mappedScenePathSha256=hashlib.sha256(mapped).hexdigest(),independentScenePathSha256=hashlib.sha256(expected['path']).hexdigest(),independentLocalConstruction=expected['details'],maximumFloatingCoordinateDifference=float_difference,quantized26_6ElementsEqual=quantized_equal,pathExplicitlyClosed=False,actualPaint=paint))
        if len(bound)!=1:
            raise RuntimeError('Sector path/translation/style not bound to exactly one independent eight-element target: '+name+' '+str(rect)+' traces='+str(paints))
        observed.append(bound[0])
    if property_count:
        snapshots=[snapshot for snapshot in record['snapshots'] if snapshot['label'].startswith('sector-properties-')]
        if len(snapshots)!=property_count:
            raise RuntimeError('Missing exact property observations: '+name)
        for snapshot in snapshots:
            widgets=[widget['qt'] for window in snapshot['windows'] for widget in window['widgets'] if widget.get('qt')]
            combos={widget['objectName']:int(widget['currentIndex']) for widget in widgets if widget['kind']=='QComboBox' and widget.get('objectName') in ('m_LineType','m_Pattern')}
            if combos!=dict(m_LineType=line,m_Pattern=fill):
                raise RuntimeError('Loaded sector line/fill properties differ: '+name+' '+str(combos))
    compilation=record['nativeCompilations'][0]
    source=BASE/name/'compiled'/compilation['label']/'input-native.sm'
    if hashlib.sha256(source.read_bytes()).hexdigest()!=compilation['sourceSha256']:
        raise RuntimeError('Actual native input source digest differs: '+name)
    return dict(name=name,passed=True,targetGeometryBound=True,rects=rects,lineStyle=line,fillStyle=fill,inputSourceSha256=record['input']['sha256'],sourceSha256=compilation['sourceSha256'],compilationLabel=compilation['label'],propertiesObserved=property_count,rootCount=len(roots),sectorPaths=observed,ownedProcessTerminated=record['ownedProcessTerminated'],usbOrPlc=record['usbOrPlc'])


def properties(rect,line,fill,edit,index):
    """Observe the fixed-angle sector dialog at a point inside the actual upper wedge."""
    x,y,width,height=rect
    commands=[dict(op='scene-double',x=x+width*3//5,y=y+height//4)]
    if edit:
        commands.extend([dict(op='combo',name='m_LineType',value=line),dict(op='combo',name='m_Pattern',value=fill)])
    commands.extend([dict(op='dump',label='sector-properties-'+str(index)),dict(op='invoke',text='确定',kind='QPushButton')])
    return commands


def collect(priority=False):
    """Collect exactly 27 source-bound fixed-angle square sectors without any native transport."""
    manifest=dict(plannedCompilations=27,canonicalCases=15,independentScenes=13,runs=[],complete=False,usbOrPlc=False,sourcePreparation='not_required_source_has_one_root',additionalReadOnlyInspection='sector-source-inspect-r1',admittedTransform='linear_identity_with_exact_bbox_integer_translation')
    prior=HERE/'sector-canonical-run-r2.json'
    if not prior.exists():
        prior=HERE/'sector-canonical-run-r1.json'
    if prior.exists():
        previous=json.loads(prior.read_text(encoding='utf-8'))
        manifest['runs']=[row for row in previous['runs'] if row['passed']]
        manifest['failedBindingSamples']=['sector-odd-create-r1']
    manifest_path=HERE/'sector-canonical-run-r2.json'
    index_path=BASE/'valid-target-index-r1.json'

    def run(name,source,commands,rects,line=0,fill=1,property_count=0,reuse=False):
        try:
            previous=[row for row in manifest['runs'] if row['name']==name]
            if previous:
                assert previous[0]['rects']==rects and previous[0]['lineStyle']==line and previous[0]['fillStyle']==fill
                return
            if reuse:
                record=json.loads((BASE/name/'native-probe.json').read_text(encoding='utf-8'))
                roots=[item for item in record['sceneReads'][-1]['data']['items'] if item['parent']=='0x0' and item['flags']&2]
                paints=[message['payload'] for message in record.get('instrumentationMessages',[]) if message.get('payload',{}).get('tag')=='stage5-native-paint']
            else:
                record,roots,paints=observe(name,source,commands)
            sample=assert_target(name,record,roots,paints,rects,line,fill,property_count)
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

    default=[100,100,200,200]
    run('sector-default-repeat-r1',SOURCE,[],[default],reuse=True)
    for index in (2,3):
        run('sector-default-repeat-r'+str(index),SOURCE,[dict(op='scene-count-assert',count=1,label='default-count'),dict(op='scene-assert',rects=[default]),dict(op='scene-read',label='default-target'),dict(op='native-compile',label='fresh-process-3')],[default])

    def draw(rect):
        x,y,width,height=rect
        return [dict(op='action',name='action_ArcItem'),dict(op='scene-drag',**{'from':[x,y],'to':[x+width,y+height]})]

    def pair(label,commands,rects,line=0,fill=1):
        revision='r2' if label=='odd' else 'r1'
        created='sector-'+label+'-create-'+revision
        reopened='sector-'+label+'-reopen-'+revision
        filename='sector-'+label+'.sm'
        observed=[command for index,rect in enumerate(rects) for command in properties(rect,line,fill,True,index)]
        commands=[*commands,dict(op='scene-count-assert',count=len(rects),label='created-count'),dict(op='scene-assert',rects=rects),*observed,dict(op='scene-read',label='created-target'),dict(op='save',output=filename),dict(op='native-compile',label='created')]
        run(created,SOURCE,commands,rects,line,fill,len(rects))
        observed=[command for index,rect in enumerate(rects) for command in properties(rect,line,fill,False,index)]
        commands=[dict(op='scene-count-assert',count=len(rects),label='reopened-count'),dict(op='scene-assert',rects=rects),*observed,dict(op='save-current'),dict(op='scene-read',label='reopened-target'),dict(op='native-compile',label='fresh-process-3')]
        run(reopened,BASE/created/filename,commands,rects,line,fill,len(rects))

    if priority:
        for line,fill in ((1,0),(1,1)):
            pair('options-'+str(line)+'-'+str(fill),[dict(op='scene-count-assert',count=1,label='source-count'),dict(op='scene-assert',rects=[default])],[default],line,fill)
        return

    for label,rect in [('shifted',[75,65,160,160]),('large',[100,100,240,240]),('near-edge',[2,2,200,200]),('far-edge',[498,178,200,200]),('odd',[101,101,201,201]),('minimum',[100,100,100,100]),('threshold-168',[100,100,168,168]),('threshold-170',[100,100,170,170])]:
        commands=[dict(op='owned-editor-size',width=1800,height=1100),dict(op='scene-count-assert',count=1,label='source-count'),dict(op='scene-assert',rects=[default]),dict(op='scene-select',rects=[default]),dict(op='action',name='action_delete'),dict(op='scene-count-assert',count=0,label='deleted-count'),*draw(rect)]
        pair(label,commands,[rect])
    for line,fill in ((0,0),(1,0),(1,1)):
        pair('options-'+str(line)+'-'+str(fill),[dict(op='scene-count-assert',count=1,label='source-count'),dict(op='scene-assert',rects=[default])],[default],line,fill)
    other=[400,100,200,200]
    pair('two-objects',[dict(op='owned-editor-size',width=1800,height=1100),*draw(other)],[default,other])
    manifest['complete']=True
    manifest['passed']=len(manifest['runs'])==27 and all(row['passed'] for row in manifest['runs'])
    write_json(manifest_path,manifest)
    write_json(index_path,manifest)
    if not manifest['passed']:
        raise RuntimeError('Sector formal compilation count differs from declared coverage')


def main():
    """Read the actual source roots before preparing a single-sector canonical source."""
    parser=argparse.ArgumentParser()
    parser.add_argument('--phase',choices=['inspect','default','collect','priority'],default='inspect')
    args=parser.parse_args()
    if args.phase in ('collect','priority'):
        collect(args.phase=='priority')
        return
    if args.phase=='default':
        record,roots,paints=observe('sector-default-repeat-r1',SOURCE,[dict(op='scene-count-assert',count=1,label='default-count'),dict(op='scene-assert',rects=[[100,100,200,200]]),dict(op='scene-read',label='default-target'),dict(op='native-compile',label='fresh-process-3')])
    else:
        record,roots,paints=observe('sector-source-inspect-r1',SOURCE,[dict(op='scene-read',label='actual-source-roots')],0)
    for paint in paints:
        if 'pathStreamHex' in paint:
            paint['decodedPathElements'],paint['decodedPathTail']=parse_path(bytes.fromhex(paint['pathStreamHex']))
    summary=dict(input=record['input'],roots=roots,paints=paints,ownedProcessTerminated=record['ownedProcessTerminated'],usbOrPlc=record['usbOrPlc'])
    write_json(HERE/'source-inspection-r1.json',summary)
    print(json.dumps(summary,ensure_ascii=False,indent=2),flush=True)


if __name__=='__main__':
    main()
