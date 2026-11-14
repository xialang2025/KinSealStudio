"""Test the observed legacy RGB565 multiply independently from coverage generation."""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import numpy as np

root=Path(__file__).resolve().parent;old=root.parent/'kinseal-stage5-20261113';work=root.parent/'kinseal-stage4-20261112/worktree'
env=os.environ.copy();env['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+env.get('PATH','');env.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
def call(args):subprocess.run(list(map(str,args)),check=True,capture_output=True,env=env,creationflags=subprocess.CREATE_NO_WINDOW)
def mul16(pixel,a):
 a=np.asarray(a,dtype=np.int64)+1
 return ((((pixel&0x7e0)*a)>>8)&0x7e0)|((((pixel&0xf81f)*(a>>2))>>6)&0xf81f)
def blend(dst,src,alpha):return np.where(alpha==255,src,mul16(src,alpha)+mul16(dst,255-alpha))
records=[]
for family in ('polygon','sector','ellipse'):
 mask={}
 for layer in ('fill','stroke','combined'):
  p=root/'temp'/('multiply-'+family+'-'+layer+'.argb32');call([old/'aa_mask_probe.exe',p,family,layer,'aa']);mask[layer]=np.frombuffer(p.read_bytes(),dtype=np.uint8).reshape(480,800,4)[...,3].astype(np.int64);p.unlink()
 native=root/'temp'/('multiply-'+family+'.rgb565');folder=work/'docs/stage5-evidence/native/audit'/('audit-'+family+'-default-r1/compiled/canonical-default')
 call([root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe',folder/'SheetPixel.dat',native,768000]);raw=native.read_bytes();expected=np.frombuffer(raw,dtype='<u2').reshape(480,800);native.unlink()
 background=np.zeros((480,800),dtype=np.int64);background[1:,1:]=0xc6ff
 actual=blend(background,0 if family=='polygon' else 0xffff,mask['fill'])
 if family!='sector':actual=blend(actual,0,mask['stroke'])
 ys,xs=np.where(actual!=expected)
 records.append(dict(family=family,coverageSource='Qt6_independent_masks',composition='Qt4_rgb565_source_component_multiply',differentPixels=len(xs),nativeSha256=hashlib.sha256(raw).hexdigest(),first=[dict(x=int(x),y=int(y),native=int(expected[y,x]),candidate=int(actual[y,x]),fill=int(mask['fill'][y,x]),stroke=int(mask['stroke'][y,x])) for y,x in zip(ys[:8],xs[:8])]))
destination=work/'docs/stage5-evidence/native/polygon/rgb16-multiply-r1.json';assert not destination.exists();destination.write_text(json.dumps(dict(cases=records,usbAccessed=False,productGateChanged=False),indent=2)+'\n',encoding='utf-8')
print(json.dumps(records))
