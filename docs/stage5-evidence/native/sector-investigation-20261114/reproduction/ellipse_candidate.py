"""Test one independently generated ellipse path/stroke against a diagnostic historical raster and retained native frame.
No original executable, UI, device, or production encoder is accessed.
"""
from pathlib import Path
import hashlib,json,os,struct,subprocess
import numpy as np
from area_candidate_256 import coverage
here=Path(__file__).resolve().parent;root=here.parent
work=root.parent/'kinseal-stage4-20261112/worktree'
evidence=work/'docs/stage5-evidence/native/sector-investigation-20261114'
env=os.environ.copy();env['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+env.get('PATH','')
for key in ('TEMP','TMP','TMPDIR'):env[key]=str(here/'temp')
def call(args):
 result=subprocess.run(list(map(str,args)),capture_output=True,env=env,creationflags=subprocess.CREATE_NO_WINDOW)
 if result.returncode:raise RuntimeError((args,result.returncode,result.stdout,result.stderr))
def parse(raw):
 count=struct.unpack_from('>I',raw)[0]
 return [struct.unpack_from('>Idd',raw,4+20*i) for i in range(count)],struct.unpack_from('>II',raw,4+20*count)
def serialize(elements,tail):return struct.pack('>I',len(elements))+b''.join(struct.pack('>Idd',*p) for p in elements)+struct.pack('>II',*tail)
def midpoint(a,b):return tuple((a[i]+b[i])//2 for i in (0,1))
def halves(curve):
 a,b,c,d=curve;ab=midpoint(a,b);bc=midpoint(b,c);cd=midpoint(c,d);abc=midpoint(ab,bc);bcd=midpoint(bc,cd);m=midpoint(abc,bcd)
 return (a,ab,abc,m),(m,bcd,cd,d)
def curve_lines(curve,depth,final_midpoint=True):
 if depth:
  left,right=halves(curve)
  return curve_lines(left,depth-1,final_midpoint)+curve_lines(right,depth-1,final_midpoint)
 if final_midpoint:return [tuple((curve[0][i]+3*curve[1][i]+3*curve[2][i]+curve[3][i])//8 for i in (0,1)),curve[3]]
 return [curve[3]]
def make_flat(raw,depth_adjust=0,final_midpoint=True):
 elements,tail=parse(raw);fixed=[(tag,(int(x*64),int(y*64))) for tag,x,y in elements]
 xcoords=[p[0] for t,p in fixed];ycoords=[p[1] for t,p in fixed]
 extent_x=((max(xcoords)+63)//64)-(min(xcoords)//64);extent_y=((max(ycoords)+63)//64)-(min(ycoords)//64)
 adaptation=int(extent_x>24 or extent_y>24)+int(extent_x>120 or extent_y>120)
 cubic_threshold=16<<adaptation;conic_threshold=32<<adaptation
 points=[];depths=[];previous=None;i=0
 while i<len(fixed):
  tag,point=fixed[i]
  if tag==2:
   curve=[previous]+[fixed[j][1] for j in (i,i+1,i+2)]
   a,b,c,d=curve
   curvature=max(abs(a[j]+d[j]-2*b[j]) for j in (0,1))//cubic_threshold
   # This diagnostic heuristic intentionally records the Qt4 historical cross-coordinate term.
   third=max(abs(a[0]+d[0]-3*(b[0]+c[0])),abs(a[1]+d[1]-3*(b[0]+c[1])))//conic_threshold
   depth=0
   while curvature or third:
    curvature>>=2;third>>=3;depth+=1
   depth=max(0,depth+depth_adjust);assert depth<=32
   depths.append(depth)
   high=[tuple(v*4 for v in p) for p in curve]
   points.extend((1,p[0]/256,p[1]/256) for p in curve_lines(high,depth,final_midpoint));previous=d;i+=3
  else:
   points.append((tag,point[0]/64,point[1]/64));previous=point;i+=1
 return serialize(points,tail),dict(adaptation=adaptation,cubicThreshold=cubic_threshold,conicThreshold=conic_threshold,depths=depths,inputElements=len(elements),outputElements=len(points),extents=[extent_x,extent_y])
def compose(fill,stroke):
 rgb=np.zeros((480,800,3),dtype=np.int64);rgb[1:,1:]=[197,222,255]
 for alpha,source in ((fill,255),(stroke,0)):
  alpha=alpha.astype(np.int64)[...,None];v=rgb*(255-alpha);s=source*alpha
  rgb=((s+128+((s+128)>>8))>>8)+((v+128+((v+128)>>8))>>8)
 return (((rgb[...,0]>>3)<<11)|((rgb[...,1]>>2)<<5)|(rgb[...,2]>>3)).astype('<u2')
if __name__=='__main__':
 records=[];masks={};raws={}
 for layer in ('fill','stroke'):
  mask_path=here/'temp'/('ellipse-'+layer+'.mask');call([root/'legacy_coverage_probe.exe',mask_path,'ellipse',layer])
  raw=Path(str(mask_path)+'.pathstream').read_bytes();raws[layer]=raw
  oracle=np.frombuffer(mask_path.read_bytes(),np.uint8).reshape(480,800)
  for adjust,mid in ((0,True),(-1,True),(1,True),(0,False)):
   flat,details=make_flat(raw,adjust,mid);actual=coverage(flat)
   ys,xs=np.where(actual!=oracle)
   records.append(dict(layer=layer,depthAdjust=adjust,leafMidpoint=mid,geometry=details,differentCoveragePixels=int(len(xs)),oracleSha256=hashlib.sha256(mask_path.read_bytes()).hexdigest(),candidateSha256=hashlib.sha256(actual.tobytes()).hexdigest(),first=[dict(x=int(x),y=int(y),oracle=int(oracle[y,x]),candidate=int(actual[y,x])) for y,x in zip(ys[:10],xs[:10])]))
   if adjust==0 and mid:
    masks[layer]=actual;(evidence/('ellipse-'+layer+'-independent.pathstream')).write_bytes(raw);(evidence/('ellipse-'+layer+'-flattened.pathstream')).write_bytes(flat)
 native_path=work/'docs/stage5-evidence/native/audit/audit-ellipse-default-r1/compiled/canonical-default/SheetPixel.dat'
 decoded=here/'temp/ellipse-native.rgb565';call([root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe',native_path,decoded,768000])
 raw=decoded.read_bytes();expected=np.frombuffer(raw,'<u2').reshape(480,800);actual=compose(masks['fill'],masks['stroke'])
 ys,xs=np.where(actual!=expected)
 result=dict(scope='retained default ellipse (100,100,200,100), white fill / black width-1 SquareCap BevelJoin stroke only; not a supported type claim',candidates=records,nativeCompiledSha256=hashlib.sha256(native_path.read_bytes()).hexdigest(),nativeFrameSha256=hashlib.sha256(raw).hexdigest(),independentFrameSha256=hashlib.sha256(actual.tobytes()).hexdigest(),differentNativeFramePixels=int(len(xs)),first=[dict(x=int(x),y=int(y),native=int(expected[y,x]),candidate=int(actual[y,x])) for y,x in zip(ys[:10],xs[:10])],usbAccessed=False,nativeProcessLaunched=False,productionChanged=False)
 (evidence/'ellipse-candidate-r1.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
 print(json.dumps(dict(cases=[(r['layer'],r['depthAdjust'],r['leafMidpoint'],r['differentCoveragePixels']) for r in records],nativeDifference=len(xs),nativeSha256=result['nativeFrameSha256'])))
