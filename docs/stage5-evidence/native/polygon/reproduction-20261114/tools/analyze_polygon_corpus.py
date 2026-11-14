"""Bind every admitted native frame to observed absolute vertices and independent cell-area output."""
from pathlib import Path
import hashlib
import json
import os
import struct
import subprocess
import numpy as np
from area_candidate import coverage

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence/native/polygon'
read=lambda p:json.loads(p.read_text(encoding='utf-8-sig'))
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
env=os.environ.copy();env['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'+env.get('PATH','');env.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
def call(a):subprocess.run(list(map(str,a)),check=True,capture_output=True,env=env,creationflags=subprocess.CREATE_NO_WINDOW)
def stream(points):return struct.pack('>I',len(points)+1)+b''.join(struct.pack('>Idd',0 if i==0 else 1,*p) for i,p in enumerate(points+[points[0]]))+struct.pack('>II',0,0)
admitted=read(root/'polygon-canonical-run-r8.json');assert admitted['passed'];rows=admitted['runs']
results=[]
for row in rows:
 assert row['passed'];folder=base/row['name'];probe=read(folder/'native-probe.json');assert probe['ownedProcessTerminated'] and not probe['usbOrPlc']
 traces=[m['payload'] for m in probe['instrumentationMessages'] if m.get('payload',{}).get('polygon')]
 for points in row['polygons']:
  bound=[t for t in traces if t['polygon']['points']==points and t['pen']['style']==row['lineStyle']]
  assert bound,(row['name'],'unbound target vertices or pen style')
  assert all(t['transform']==[1,0,0,1,0,0,0,0,1] and t['polygon']['fillRule']==0 for t in bound)
 for comp in probe['nativeCompilations']:
  target=folder/'compiled'/comp['label'];assert sha(target/'input-native.sm')==comp['sourceSha256']
  image=np.zeros((480,800,3),dtype=np.int64);image[1:,1:]=[197,222,255]
  coverageChecks=[]
  for index,points in enumerate(row['polygons']):
   path=root/'temp'/(row['name']+f'-{index}.pathstream');path.write_bytes(stream(points))
   for layer,enabled in (('fill',row['fillStyle']),('stroke',row['lineStyle'])):
    if not enabled:continue
    mask=root/'temp'/(row['name']+f'-{index}-{layer}.mask');call([root/'legacy_coverage_probe.exe',mask,'polygon',layer,path])
    own=coverage(Path(str(mask)+'.pathstream').read_bytes());public=np.frombuffer(mask.read_bytes(),dtype=np.uint8).reshape(480,800);assert np.array_equal(own,public),(row['name'],layer,'diagnostic coverage mismatch')
    coverageChecks.append(dict(object=index,layer=layer,publicDiagnosticCoverageDifferentPixels=0))
    value=image*(255-own.astype(np.int64)[...,None]);image=(value+(value>>8)+128)>>8
    mask.unlink();Path(str(mask)+'.pathstream').unlink()
   path.unlink()
  actual=(((image[...,0]>>3)<<11)|((image[...,1]>>2)<<5)|(image[...,2]>>3)).astype('<u2').tobytes()
  native=root/'temp'/(row['name']+'.rgb565');call([root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe',target/'SheetPixel.dat',native,768000]);expected=native.read_bytes();different=int(np.count_nonzero(np.frombuffer(expected,dtype='<u2')!=np.frombuffer(actual,dtype='<u2')))
  lengths=(target/'PicLenPara.dat').read_bytes();assert len(lengths)==2400 and not (target/'ControlPara.dat').exists() and not (target/'ControlPixel.dat').exists()
  results.append(dict(run=row['name'],polygons=row['polygons'],lineStyle=row['lineStyle'],fillStyle=row['fillStyle'],sourceSha256=comp['sourceSha256'],folder=str(target.relative_to(work)).replace('\\','/'),B=[dict(name=p.name,bytes=p.stat().st_size,sha256=sha(p)) for p in sorted(target.iterdir()) if p.is_file()],S=dict(controlParaPresent=False,controlPixelPresent=False,picLenBytes=2400,dynamicImageCount=int.from_bytes(lengths[1600:1604],'little')),M=dict(rgb565Bytes=768000,nativeSha256=sha(native),candidateSha256=hashlib.sha256(actual).hexdigest(),differentPixels=different,coverageChecks=coverageChecks),usbAccessed=False,E=False));native.unlink();print(row['name'],different,flush=True)
repeated=[c for c in results if 'default-repeat' in c['run']];assert len(repeated)==3
changes=[]
for name in sorted({x['name'] for c in repeated for x in c['B']}):
 buffers=[(work/c['folder']/name).read_bytes() for c in repeated];ranges=[];start=None;length=max(map(len,buffers))
 for i in range(length):
  changed=len({b[i] if i<len(b) else None for b in buffers})!=1
  if changed and start is None:start=i
  if start is not None and (not changed or i==length-1):ranges.append([start,i+1 if changed else i]);start=None
 changes.append(dict(name=name,lengths=list(map(len,buffers)),changedRangesExclusiveEnd=ranges,classification='unknown_no_ignore_mask' if ranges else 'stable_in_three_observed_fresh_processes'))
result=dict(cases=results,B_repeatDifferences=changes,ignoredByteRanges=[],allRasterComparisonsEqual=all(c['M']['differentPixels']==0 for c in results),usbAccessed=False,E=False)
destination=base/'B-S-M-cell-area-r1.json';assert not destination.exists();destination.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(cases=len(results),allEqual=result['allRasterComparisonsEqual'])))
