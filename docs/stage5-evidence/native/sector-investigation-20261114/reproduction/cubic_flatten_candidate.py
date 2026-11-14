"""Explore fixed-point de Casteljau subdivision, then independent cell-area integration.
The public Qt4 grayraster is a diagnostic comparator only. This script is not production.
"""
from pathlib import Path
import json,hashlib,struct
import numpy as np
from area_candidate_256 import coverage
here=Path(__file__).resolve().parent
evidence=here.parents[1]/'kinseal-stage4-20261112/worktree/docs/stage5-evidence/native/sector-investigation-20261114'
raw=(evidence/'native-absolute.pathstream').read_bytes();count=struct.unpack_from('>I',raw)[0]
elements=[struct.unpack_from('>Idd',raw,4+i*20) for i in range(count)]
native=np.frombuffer((here/'temp/native.rgb565').read_bytes(),'<u2').reshape(480,800)
oracle=np.frombuffer((here/'temp/native.mask').read_bytes(),np.uint8).reshape(480,800)
def mid(a,b):return tuple((x+y)//2 for x,y in zip(a,b))
def split(c):
 a,b,c,d=c;ab=mid(a,b);bc=mid(b,c);cd=mid(c,d);abc=mid(ab,bc);bcd=mid(bc,cd);m=mid(abc,bcd)
 return (a,ab,abc,m),(m,bcd,cd,d)
def flatten(c,depth,include_midpoint):
 if depth:
  l,r=split(c);return flatten(l,depth-1,include_midpoint)+flatten(r,depth-1,include_midpoint)
 if include_midpoint:return [tuple((c[0][i]+3*c[1][i]+3*c[2][i]+c[3][i])//8 for i in (0,1)),c[3]]
 return [c[3]]
records=[]
for depth in range(2,8):
 for with_mid in (False,True):
  points=[];i=0;previous=None
  while i<len(elements):
   tag,x,y=elements[i];point=(int(x*64)*4,int(y*64)*4)
   if tag==2:
    curve=[previous]+[(int(elements[j][1]*64)*4,int(elements[j][2]*64)*4) for j in (i,i+1,i+2)]
    points.extend((1,p[0]/256,p[1]/256) for p in flatten(curve,depth,with_mid));previous=curve[-1];i+=3
   else:
    points.append((tag,point[0]/256,point[1]/256));previous=point;i+=1
  serialized=struct.pack('>I',len(points))+b''.join(struct.pack('>Idd',*p) for p in points)+struct.pack('>II',0,0)
  alpha=coverage(serialized).astype(np.int64)
  channels=[]
  for value in (197,222,255):
   v=value*(255-alpha);channels.append(alpha+((v+128+((v+128)>>8))>>8))
  red,green,blue=channels;frame=((red>>3)<<11)|((green>>2)<<5)|(blue>>3);frame[0,:]=0;frame[:,0]=0
  ys,xs=np.where(frame!=native)
  record=dict(depth=depth,midpoint=with_mid,oracleCoverageDifference=int(np.count_nonzero(alpha!=oracle)),nativeFrameDifference=int(len(xs)),first=[dict(x=int(x),y=int(y),native=int(native[y,x]),actual=int(frame[y,x]),candidateCoverage=int(alpha[y,x]),oracleCoverage=int(oracle[y,x])) for y,x in zip(ys[:10],xs[:10])])
  records.append(record)
  if not np.any(alpha!=oracle):(evidence/'fixed-curve-oracle-equal.pathstream').write_bytes(serialized)
result=dict(scope='single sector default retained native frame',candidates=records,usbAccessed=False,nativeLaunched=False,productionChanged=False)
(evidence/'cubic-flatten-r1.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
print(json.dumps([(r['depth'],r['midpoint'],r['oracleCoverageDifference'],r['nativeFrameDifference']) for r in records]))
