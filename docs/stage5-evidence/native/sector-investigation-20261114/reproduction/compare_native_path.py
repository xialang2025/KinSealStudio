"""Compare retained native sector paths with independently produced paths without launching native software."""
from pathlib import Path
import gzip,hashlib,json,os,struct,subprocess
import numpy as np
here=Path(__file__).resolve().parent
root=here.parent
work=root.parent/'kinseal-stage4-20261112/worktree'
evidence=work/'docs/stage5-evidence/native/sector-investigation-20261114'
env=os.environ.copy();env['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+env.get('PATH','')
for key in ('TEMP','TMP','TMPDIR'):env[key]=str(here/'temp')
def call(args):
 r=subprocess.run(list(map(str,args)),capture_output=True,env=env,creationflags=subprocess.CREATE_NO_WINDOW)
 if r.returncode:raise RuntimeError((args,r.returncode,r.stdout,r.stderr))
def parse(raw):
 n=struct.unpack_from('>I',raw)[0]
 return [struct.unpack_from('>Idd',raw,4+20*i) for i in range(n)],struct.unpack_from('>II',raw,4+20*n)
def serialize(elements,tail):return struct.pack('>I',len(elements))+b''.join(struct.pack('>Idd',*p) for p in elements)+struct.pack('>II',*tail)
probe_path=work/'docs/stage5-evidence/native/audit/paint-sector-r1/native-probe.json.gz'
probe=json.loads(gzip.decompress(probe_path.read_bytes()))
payload=next(m['payload'] for m in probe['instrumentationMessages'] if 'pathStreamHex' in m.get('payload',{}))
local=bytes.fromhex(payload['pathStreamHex']);points,tail=parse(local)
native=serialize([(t,x+100,y+100) for t,x,y in points],tail)
(evidence/'native-local.pathstream').write_bytes(local);(evidence/'native-absolute.pathstream').write_bytes(native)
mask_a=here/'temp/qt6.mask';mask_b=here/'temp/native.mask';mask_c=here/'temp/native-local.mask'
call([root/'legacy_coverage_probe.exe',mask_a,'sector','fill'])
call([root/'legacy_coverage_probe.exe',mask_b,'sector','fill',evidence/'native-absolute.pathstream'])
call([root/'legacy_coverage_probe.exe',mask_c,'sector','fill',evidence/'native-local.pathstream'])
qt6=(here/'temp/qt6.mask.pathstream').read_bytes()
(evidence/'qt6-absolute.pathstream').write_bytes(qt6)
expected_path=work/'docs/stage5-evidence/native/audit/paint-sector-r1/compiled/fresh-process-3/SheetPixel.dat'
raw_path=here/'temp/native.rgb565'
call([root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe',expected_path,raw_path,768000])
expected_raw=raw_path.read_bytes();expected=np.frombuffer(expected_raw,dtype='<u2').reshape(480,800)
records=[]
for name,path in [('qt6',mask_a),('native',mask_b),('native-local-then-shift',mask_c)]:
 mask=np.frombuffer(path.read_bytes(),dtype=np.uint8).reshape(480,800).astype(np.int64)
 if name=='native-local-then-shift':
  shifted=np.zeros_like(mask);shifted[100:,100:]=mask[:-100,:-100];mask=shifted
 image=np.zeros((480,800,3),dtype=np.int64);image[1:,1:]=[197,222,255]
 a=mask[...,None];value=image*(255-a)
 image=255*a//255+((value+128+((value+128)>>8))>>8)
 frame=((image[...,0]>>3)<<11)|((image[...,1]>>2)<<5)|(image[...,2]>>3)
 ys,xs=np.where(frame!=expected)
 records.append(dict(path=name,differentPixels=int(len(xs)),maskSha256=hashlib.sha256(path.read_bytes()).hexdigest(),frameSha256=hashlib.sha256(frame.astype('<u2').tobytes()).hexdigest(),first=[dict(x=int(x),y=int(y),native=int(expected[y,x]),candidate=int(frame[y,x]),coverage=int(mask[y,x])) for y,x in zip(ys[:20],xs[:20])]))
qpoints,qtail=parse(qt6)
result=dict(nativeProbeSha256=hashlib.sha256(probe_path.read_bytes()).hexdigest(),nativePathPayload=payload,nativeLocalElements=points,qt6AbsoluteElements=qpoints,nativeTail=tail,qt6Tail=qtail,nativeFrameSha256=hashlib.sha256(expected_raw).hexdigest(),candidates=records,usbAccessed=False,nativeProcessLaunched=False,productionChanged=False)
(evidence/'native-path-replay-r2.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
print(json.dumps(result,indent=2))
