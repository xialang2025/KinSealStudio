"""Bind native UI inputs, segment structure and complete independent fixed-scanline pixels."""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import numpy as np
from scanline_candidate import fill

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence/native/roundrect'
decoder=root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'
environment=os.environ.copy();environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+environment.get('PATH','')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
cases=[]
for run in sorted(base.iterdir()):
    probe=run/'native-probe.json'
    if not probe.exists():continue
    data=json.loads(probe.read_text(encoding='utf-8'))
    assert data['status']=='native-stage4-graphics-scenario-completed' and data['ownedProcessTerminated'] and not data['usbOrPlc'],run
    inventories=[s['result']['value'] for s in data['steps'] if s['request']['op']=='scene-read']
    selected=[i for i in inventories[-1]['items'] if i['flags']&2]
    rectangles=sorted(i['rect'] for i in selected)
    assert 1<=len(rectangles)<=2 and all(all(v==int(v) for v in r) for r in rectangles)
    for i in selected:
        assert i['transform'][:4]==[1,0,0,1] and i['transform'][6:]==[0,0,1]
        assert i['transform'][4:6]==i['rect'][:2]
    line,pattern=list(map(int,run.name.split('-')[2:4])) if run.name.startswith('roundrect-options-') else (1,1)
    for comp in data['nativeCompilations']:
        folder=run/'compiled'/comp['label'];assert sha(folder/'input-native.sm')==comp['sourceSha256']
        native=root/('temp/'+run.name+'.rgb565')
        subprocess.run([str(decoder),str(folder/'SheetPixel.dat'),str(native),'768000'],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW)
        expected=native.read_bytes();image=np.zeros((480,800),dtype='<u2');image[1:,1:]=0xc6ff
        for x,y,width,height in rectangles:
            fillPath=root/'temp/roundrect-fill.pathstream';strokePath=root/'temp/roundrect-stroke.pathstream'
            subprocess.run([str(root/'roundrect_path_probe.exe'),str(fillPath),str(strokePath),str(int(width)),str(int(height)),'10','10'],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW)
            if pattern:fill(image,fillPath.read_bytes(),0xffff,offset=(x,y))
            if line:fill(image,strokePath.read_bytes(),0,offset=(x,y))
            fillPath.unlink();strokePath.unlink()
        actual=image.tobytes();different=np.flatnonzero(image!=np.frombuffer(expected,dtype='<u2').reshape(480,800))
        lengths=(folder/'PicLenPara.dat').read_bytes();assert len(lengths)==2400 and not (folder/'ControlPara.dat').exists() and not (folder/'ControlPixel.dat').exists()
        cases.append(dict(run=run.name,rectangles=rectangles,lineStyle=line,fillStyle=pattern,radiusX=10,radiusY=10,sourceSha256=comp['sourceSha256'],folder=str(folder.relative_to(work)).replace('\\','/'),B=[dict(name=p.name,bytes=p.stat().st_size,sha256=sha(p)) for p in sorted(folder.iterdir()) if p.is_file()],S=dict(controlParaPresent=False,controlPixelPresent=False,picLenBytes=2400,dynamicImageCount=int.from_bytes(lengths[1600:1604],'little')),M=dict(rgb565Bytes=768000,nativeSha256=sha(native),candidateSha256=hashlib.sha256(actual).hexdigest(),differentPixels=len(different),first=[dict(x=int(i%800),y=int(i//800)) for i in different[:10]]),usbAccessed=False,E=False))
        native.unlink()
repeated=[c for c in cases if 'default-repeat' in c['run']];assert len(repeated)==3
changes=[]
for name in sorted({x['name'] for c in repeated for x in c['B']}):
    buffers=[(work/c['folder']/name).read_bytes() for c in repeated];ranges=[];start=None;length=max(map(len,buffers))
    for i in range(length):
        changed=len({b[i] if i<len(b) else None for b in buffers})!=1
        if changed and start is None:start=i
        if start is not None and (not changed or i==length-1):ranges.append([start,i+1 if changed else i]);start=None
    changes.append(dict(name=name,lengths=list(map(len,buffers)),changedRangesExclusiveEnd=ranges,classification='unknown_no_ignore_mask' if ranges else 'stable_in_three_observed_fresh_processes'))
result=dict(cases=cases,B_repeatDifferences=changes,ignoredByteRanges=[],allRasterComparisonsEqual=all(c['M']['differentPixels']==0 for c in cases),usbAccessed=False,E=False)
destination=base/'B-S-M-fixed-scanline-r1.json';assert not destination.exists();destination.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(cases=len(cases),allEqual=result['allRasterComparisonsEqual'],differences=[(c['run'],c['M']['differentPixels']) for c in cases]),ensure_ascii=False))
