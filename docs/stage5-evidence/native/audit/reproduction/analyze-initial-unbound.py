"""Compare observed static-family defaults without labeling a candidate as a supported compiler type."""
from pathlib import Path
import hashlib
import json
import os
import subprocess

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence/native/audit'
decoder=root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
environment=os.environ.copy();environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'));environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+environment.get('PATH','')
cases=[]
for folder in sorted(base.iterdir()):
    probe=folder/'native-probe.json'
    if not probe.is_file():continue
    d=json.loads(probe.read_text(encoding='utf-8'));assert d['status']=='native-stage4-graphics-scenario-completed' and d['ownedProcessTerminated'] and not d['usbOrPlc']
    family=folder.name.split('-')[1];compiled=folder/'compiled/canonical-default';assert sha(compiled/'input-native.sm')==d['input']['sha256']
    inventories=[s['result']['value'] for s in d['steps'] if s['request']['op']=='scene-read'];rectangles=[x['rect'] for x in inventories[-1]['items'] if x['flags']&2]
    item=dict(family=family,sourceSha256=d['input']['sha256'],geometryCandidates=rectangles,folder=str(compiled.relative_to(work)).replace('\\','/'),candidateOnly=True,productCompilerOpened=False,usbAccessed=False,E=False,
              B=[dict(name=p.name,bytes=p.stat().st_size,sha256=sha(p)) for p in sorted(compiled.iterdir()) if p.is_file()],
              S=dict(controlParaPresent=(compiled/'ControlPara.dat').exists(),controlPixelPresent=(compiled/'ControlPixel.dat').exists(),picLenBytes=(compiled/'PicLenPara.dat').stat().st_size))
    if len(rectangles)!=1 or not all(v==int(v) for v in rectangles[0]):
        item['status']='semantic_geometry_not_yet_bound';cases.append(item);continue
    raw=root/'temp'/('audit-'+family+'-native.rgb565');subprocess.run([str(decoder),str(compiled/'SheetPixel.dat'),str(raw),'768000'],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW);expected=raw.read_bytes();assert len(expected)==768000
    item['M']=dict(nativeRawSha256=sha(raw),rgb565Bytes=len(expected),candidates=[])
    for cap in (['square','flat'] if family in ('line','freeline') else ['square']):
        actual=root/'temp'/('audit-'+family+'-'+cap+'.rgb565');subprocess.run([str(root/'graphic_candidate_probe.exe'),str(actual),family,*map(lambda v:str(int(v)),rectangles[0]),cap],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW)
        pixels=actual.read_bytes();assert len(pixels)==768000;different=[i//2 for i in range(0,len(pixels),2) if pixels[i:i+2]!=expected[i:i+2]]
        item['M']['candidates'].append(dict(cap=cap,differentPixels=len(different),rawSha256=sha(actual),firstDifferences=[dict(x=i%800,y=i//800,native=expected[i*2:i*2+2].hex(),candidate=pixels[i*2:i*2+2].hex()) for i in different[:12]],bounds=[min(i%800 for i in different),min(i//800 for i in different),max(i%800 for i in different),max(i//800 for i in different)] if different else None))
        actual.unlink()
    raw.unlink();item['status']='default_candidate_equal_needs_repeat_and_boundaries' if any(c['differentPixels']==0 for c in item['M']['candidates']) else 'raster_contract_unresolved';cases.append(item)
record=dict(cases=cases,probeSha256=sha(root/'graphic_candidate_probe.exe'),productCompilerOpened=False,usbAccessed=False,completeStage5=False,ignoredByteRanges=[])
(base/'candidate-comparison.json').write_text(json.dumps(record,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps([dict(family=c['family'],status=c['status'],geometry=c['geometryCandidates'],differences=[(v['cap'],v['differentPixels']) for v in c.get('M',{}).get('candidates',[])]) for c in cases],ensure_ascii=False))
