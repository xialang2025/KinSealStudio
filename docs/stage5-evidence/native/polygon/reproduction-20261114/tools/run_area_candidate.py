"""Compare independent straight-cell integration, diagnostic library coverage and the real native frame."""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import numpy as np
from area_candidate import coverage

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
env=os.environ.copy();env['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;'+env.get('PATH','');env.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
def call(a):subprocess.run(list(map(str,a)),check=True,capture_output=True,env=env,creationflags=subprocess.CREATE_NO_WINDOW)
masks={};checks=[]
for layer in ('fill','stroke'):
 p=root/'temp'/('area-polygon-'+layer+'.mask');call([root/'legacy_coverage_probe.exe',p,'polygon',layer]);actual=coverage(p.with_suffix('.mask.pathstream').read_bytes());reference=np.frombuffer(p.read_bytes(),dtype=np.uint8).reshape(480,800)
 ys,xs=np.where(actual!=reference);checks.append(dict(layer=layer,differentCoveragePixels=len(xs),first=[dict(x=int(x),y=int(y),actual=int(actual[y,x]),reference=int(reference[y,x])) for y,x in zip(ys[:8],xs[:8])]))
 masks[layer]=actual.astype(np.int64)
image=np.zeros((480,800,3),dtype=np.int64);image[1:,1:]=[197,222,255]
for layer in ('fill','stroke'):
 value=image*(255-masks[layer][...,None]);image=(value+(value>>8)+128)>>8
raw=((image[...,0]>>3)<<11)|((image[...,1]>>2)<<5)|(image[...,2]>>3)
native=root/'temp/area-polygon-native.rgb565';call([root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe',work/'docs/stage5-evidence/native/polygon/polygon-paint-r1/compiled/fresh-process-3/SheetPixel.dat',native,768000]);expected=np.frombuffer(native.read_bytes(),dtype='<u2').reshape(480,800);ys,xs=np.where(raw!=expected)
result=dict(independentlyIntegratedStraightEdges=True,coverageChecks=checks,differentNativePixels=len(xs),nativeSha256=hashlib.sha256(native.read_bytes()).hexdigest(),candidateSha256=hashlib.sha256(raw.astype('<u2').tobytes()).hexdigest(),first=[dict(x=int(x),y=int(y),native=int(expected[y,x]),candidate=int(raw[y,x])) for y,x in zip(ys[:8],xs[:8])],backgroundRgb=[197,222,255],usbAccessed=False,productionUnchanged=True)
p=work/'docs/stage5-evidence/native/polygon/independent-cell-area-r1.json';assert not p.exists();p.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');print(json.dumps(result))
