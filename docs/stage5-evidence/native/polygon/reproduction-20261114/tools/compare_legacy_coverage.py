"""Use retained public-library coverage only as a diagnostic oracle, never as a production dependency."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import subprocess
import numpy as np

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
parser=argparse.ArgumentParser();parser.add_argument('--run',default='r1');parser.add_argument('--green',type=int,default=223);args=parser.parse_args()
env=os.environ.copy();env['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+env.get('PATH','');env.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
def call(args):subprocess.run(list(map(str,args)),check=True,capture_output=True,env=env,creationflags=subprocess.CREATE_NO_WINDOW)
def mul16(pixel,a):
 a=np.asarray(a,dtype=np.int64)+1
 return ((((pixel&0x7e0)*a)>>8)&0x7e0)|((((pixel&0xf81f)*(a>>2))>>6)&0xf81f)
def mix16(dst,src,a):return np.where(a==255,src,mul16(src,a)+mul16(dst,255-a))
def rgb565(rgb):return ((rgb[...,0]>>3)<<11)|((rgb[...,1]>>2)<<5)|(rgb[...,2]>>3)
records=[]
for family in ('polygon','sector','ellipse'):
 masks={}
 for layer in ('fill','stroke'):
  path=root/'temp'/('legacy-'+family+'-'+layer+'.mask');call([root/'legacy_coverage_probe.exe',path,family,layer]);masks[layer]=np.frombuffer(path.read_bytes(),dtype=np.uint8).reshape(480,800).astype(np.int64);path.unlink()
 native=root/'temp'/('legacy-'+family+'.rgb565');folder=work/'docs/stage5-evidence/native/audit'/('audit-'+family+'-default-r1/compiled/canonical-default')
 call([root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe',folder/'SheetPixel.dat',native,768000]);raw=native.read_bytes();expected=np.frombuffer(raw,dtype='<u2').reshape(480,800);native.unlink()
 candidates=[]
 for mode in ('rgb16','rgb32-qt','rgb32-floor','rgb32-expanded'):
  if mode=='rgb16':
   actual=np.zeros((480,800),dtype=np.int64);actual[1:,1:]=0xc6ff
   actual=mix16(actual,0 if family=='polygon' else 0xffff,masks['fill'])
   if family!='sector':actual=mix16(actual,0,masks['stroke'])
  else:
   image=np.zeros((480,800,3),dtype=np.int64);image[1:,1:]=[198 if mode.endswith('expanded') else 197,args.green,255]
   for layer,source in [('fill',0 if family=='polygon' else 255),('stroke',0)]:
    if family=='sector' and layer=='stroke':continue
    alpha=masks[layer][...,None];value=image*(255-alpha)
    image=source*alpha//255+(value//255 if mode.endswith('floor') else (value+128+((value+128)>>8))>>8)
   actual=rgb565(image)
  ys,xs=np.where(actual!=expected);candidates.append(dict(mode=mode,differentPixels=len(xs),first=[dict(x=int(x),y=int(y),native=int(expected[y,x]),candidate=int(actual[y,x]),fill=int(masks['fill'][y,x]),stroke=int(masks['stroke'][y,x])) for y,x in zip(ys[:8],xs[:8])]))
 records.append(dict(family=family,coverageSource='public_Qt4.6.3_gray_raster_diagnostic_only',pathSource='independently_generated_Qt6_fill_and_stroker',nativeSha256=hashlib.sha256(raw).hexdigest(),candidates=candidates))
destination=work/'docs/stage5-evidence/native/polygon'/('legacy-coverage-'+args.run+'.json');assert not destination.exists();destination.write_text(json.dumps(dict(cases=records,backgroundGreen=args.green,usbAccessed=False,productionUnchanged=True),indent=2)+'\n',encoding='utf-8')
print(json.dumps([(r['family'],[(c['mode'],c['differentPixels']) for c in r['candidates']]) for r in records]))
