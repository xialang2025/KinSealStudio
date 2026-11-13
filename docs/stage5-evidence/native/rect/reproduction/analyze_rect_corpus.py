"""Inspect independent native rectangle samples by raw bytes, structure and complete decoded raster."""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import gzip

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence/native/rect'
decoder=root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
environment=os.environ.copy();environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+environment.get('PATH','')
cases=[]
for run in sorted(base.iterdir()):
    probe=run/'native-probe.json'
    if not probe.is_file():continue
    data=json.loads(probe.read_text(encoding='utf-8'));assert data['ownedProcessTerminated'] and not data['usbOrPlc']
    if 'rawEvidence' in data:data=json.loads(gzip.decompress(probe.with_suffix('.json.gz').read_bytes()))
    if data['status']!='native-stage4-graphics-scenario-completed':continue
    inventories=[s['result']['value'] for s in data['steps'] if s['request']['op']=='scene-read']
    rectangles=sorted([i['rect'] for i in inventories[-1]['items'] if i['flags']&2]);assert 1<=len(rectangles)<=2
    geometry=rectangles[0];assert all(v==int(v) for v in geometry)
    line,fill=(list(map(int,run.name.split('-')[2:4])) if run.name.startswith('rect-options-') else [1,1])
    for compiled in data['nativeCompilations']:
        folder=run/'compiled'/compiled['label'];assert sha(folder/'input-native.sm')==compiled['sourceSha256']
        raw=root/'temp'/(run.name+'-native.rgb565');actual=root/'temp'/(run.name+'-product.rgb565')
        subprocess.run([str(decoder),str(folder/'SheetPixel.dat'),str(raw),'768000'],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW)
        parameters=[*geometry,line,fill,*rectangles[1]] if len(rectangles)==2 else [*geometry,line,fill]
        subprocess.run([str(root/'rect_raster_probe.exe'),str(actual),*map(lambda x:str(int(x)),parameters)],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW)
        pixels=actual.read_bytes();expected=raw.read_bytes();assert len(pixels)==len(expected)==768000
        lengths=(folder/'PicLenPara.dat').read_bytes();assert len(lengths)==2400 and not (folder/'ControlPara.dat').exists() and not (folder/'ControlPixel.dat').exists()
        cases.append(dict(run=run.name,geometry=geometry,rectangles=rectangles,lineStyle=line,fillStyle=fill,sourceSha256=compiled['sourceSha256'],folder=str(folder.relative_to(work)).replace('\\','/'),
                          B=[dict(name=p.name,bytes=p.stat().st_size,sha256=sha(p)) for p in sorted(folder.iterdir()) if p.is_file()],
                          S=dict(controlParaPresent=False,controlPixelPresent=False,picLenBytes=2400,dynamicImageCount=int.from_bytes(lengths[1600:1604],'little')),
                          M=dict(rgb565Bytes=768000,nativeSha256=sha(raw),actualSha256=sha(actual),differentPixels=sum(pixels[i:i+2]!=expected[i:i+2] for i in range(0,len(pixels),2))),E='not_executed',usbAccessed=False))
        raw.unlink();actual.unlink()
repeated=[c for c in cases if c['run'].startswith('rect-default-repeat-')];assert len(repeated)==3
differences=[]
for name in sorted({x['name'] for c in repeated for x in c['B']}):
    buffers=[(work/c['folder']/name).read_bytes() for c in repeated];ranges=[];start=None
    for index in range(max(map(len,buffers))):
        changed=len({b[index] if index<len(b) else None for b in buffers})!=1
        if changed and start is None:start=index
        if start is not None and (not changed or index==max(map(len,buffers))-1):ranges.append([start,index+1 if changed else index]);start=None
    differences.append(dict(name=name,lengths=list(map(len,buffers)),identical=not ranges,changedRangesExclusiveEnd=ranges,classification='unknown_no_ignore_mask' if ranges else 'stable_in_three_observed_fresh_processes'))
record=dict(cases=cases,allRasterComparisonsEqual=all(c['M']['differentPixels']==0 for c in cases),B_repeatDifferences=differences,repeatedInputFreshProcessCount=3,ignoredByteRanges=[],usbAccessed=False,E=False)
(base/'B-S-M-comparison.json').write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(cases=len(cases),allEqual=record['allRasterComparisonsEqual'],differences=[(c['run'],c['M']['differentPixels']) for c in cases]),ensure_ascii=False))
