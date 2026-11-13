"""Bind native >Idd endpoint records to actual full-frame raster comparisons, including axial carriers."""
from pathlib import Path
import hashlib
import json
import os
import struct
import subprocess
import argparse

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence/native/freeline';decoder=root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
environment=os.environ.copy();environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'));environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+environment.get('PATH','')
cases=[];excluded=[]
parser=argparse.ArgumentParser();parser.add_argument('--mode',default='bresenham');parser.add_argument('--run');args=parser.parse_args()
for run in sorted(base.iterdir()):
    probe=run/'native-probe.json'
    if not probe.exists():continue
    data=json.loads(probe.read_text(encoding='utf-8'));assert data['ownedProcessTerminated'] and not data['usbOrPlc']
    if data['status']!='native-stage4-graphics-scenario-completed':continue
    if 'nativeCompilations' not in data:
        assert any(s.get('request',{}).get('op')=='scene-map-points' for s in data['steps']);continue
    for c in data['nativeCompilations']:
        folder=run/'compiled'/c['label'];source=(folder/'input-native.sm').read_bytes();assert hashlib.sha256(source).hexdigest()==c['sourceSha256'];offset=source.find(struct.pack('>I',65576));assert offset>=0
        count=struct.unpack_from('>I',source,offset+4)[0]
        if count!=2:
            excluded.append(dict(run=run.name,folder=str(folder.relative_to(work)).replace('\\','/'),sourceSha256=c['sourceSha256'],pathElementCount=count,classification='compiled_container_has_no_valid_two_endpoint_target',countedAsTargetSuccess=False));continue
        first=struct.unpack_from('>Idd',source,offset+8);last=struct.unpack_from('>Idd',source,offset+28);assert first[0]==0 and last[0]==1
        inventories=[s['result']['value'] for s in data['steps'] if s.get('request',{}).get('op')=='scene-read']
        selected=[item for item in inventories[-1]['items'] if item['flags']&2];assert len(selected)==1
        if selected[0]['transform']!=[1,0,0,1,0,0,0,0,1]:
            excluded.append(dict(run=run.name,folder=str(folder.relative_to(work)).replace('\\','/'),sourceSha256=c['sourceSha256'],pathElementCount=2,localPrefixPoints=[first[1:],last[1:]],sceneTransformRaw=selected[0]['transform'],classification='local_prefix_is_not_absolute_due_to_nonidentity_scene_transform',mappingEvidence='freeline-screenedge-map-r6/native-probe.json',countedAsTargetSuccess=False));continue
        points=[first[1:],last[1:]];assert all(v==int(v) for p in points for v in p) and points[0]!=points[1]
        left=min(p[0] for p in points);top=min(p[1] for p in points);width=max(1,abs(last[1]-first[1]));height=max(1,abs(last[2]-first[2]));geometry=[left,top,width,height];normalized=[[(p[0]-left)/width,(p[1]-top)/height] for p in points]
        actual=root/'temp'/(run.name+'-candidate.rgb565');raw=root/'temp'/(run.name+'-native.rgb565');subprocess.run([str(decoder),str(folder/'SheetPixel.dat'),str(raw),'768000'],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW)
        subprocess.run([str(root/'line_raster_probe.exe'),str(actual),*map(str,(*first[1:],*last[1:])),args.mode],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW);pixels=actual.read_bytes();expected=raw.read_bytes();assert len(pixels)==len(expected)==768000
        lengths=(folder/'PicLenPara.dat').read_bytes();assert len(lengths)==2400 and not (folder/'ControlPara.dat').exists() and not (folder/'ControlPixel.dat').exists()
        cases.append(dict(run=run.name,sourceSha256=c['sourceSha256'],folder=str(folder.relative_to(work)).replace('\\','/'),geometry=geometry,points=points,normalizedPoints=normalized,
                          B=[dict(name=p.name,bytes=p.stat().st_size,sha256=sha(p)) for p in sorted(folder.iterdir()) if p.is_file()],S=dict(picLenBytes=2400,controlParaPresent=False,controlPixelPresent=False,dynamicImageCount=int.from_bytes(lengths[1600:1604],'little')),
                          M=dict(rgb565Bytes=768000,nativeSha256=sha(raw),candidateSha256=sha(actual),differentPixels=sum(pixels[i:i+2]!=expected[i:i+2] for i in range(0,len(pixels),2))),E=False,usbAccessed=False));actual.unlink();raw.unlink()
repeated=[c for c in cases if c['run'].startswith('freeline-default-repeat-')];assert len(repeated)==3
differences=[]
for name in sorted({f['name'] for c in repeated for f in c['B']}):
    buffers=[(work/c['folder']/name).read_bytes() for c in repeated];ranges=[];start=None
    for i in range(max(map(len,buffers))):
        changed=len({b[i] if i<len(b) else None for b in buffers})!=1
        if changed and start is None:start=i
        if start is not None and (not changed or i==max(map(len,buffers))-1):ranges.append([start,i+1 if changed else i]);start=None
    differences.append(dict(name=name,lengths=list(map(len,buffers)),identical=not ranges,changedRangesExclusiveEnd=ranges,classification='unknown_no_ignore_mask' if ranges else 'stable_in_observed_fresh_processes'))
canonical=[c for c in cases if 'reopen' in c['run'] or 'default-repeat' in c['run']]
result=dict(mode=args.mode,cases=cases,excludedInvalidTargetSamples=excluded,allRasterComparisonsEqual=all(c['M']['differentPixels']==0 for c in cases),allCanonicalRasterComparisonsEqual=all(c['M']['differentPixels']==0 for c in canonical),repeatedInputFreshProcessCount=3,B_repeatDifferences=differences,ignoredByteRanges=[],usbAccessed=False,E=False)
destination=base/('B-S-M-'+args.mode+('-'+args.run if args.run else '')+'.json');assert not destination.exists();destination.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(json.dumps(dict(mode=args.mode,cases=len(cases),allCanonicalEqual=result['allCanonicalRasterComparisonsEqual'],examples=[(c['run'],c['M']['differentPixels']) for c in canonical]),ensure_ascii=False))
