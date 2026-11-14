"""Search quantization-equivalent opaque source/background channels without native execution."""
from pathlib import Path
import json,numpy as np
here=Path(__file__).resolve().parent
a=np.frombuffer((here/'temp/qt6.mask').read_bytes(),np.uint8).reshape(480,800).astype(np.int64)
e=np.frombuffer((here/'temp/native.rgb565').read_bytes(),'<u2').reshape(480,800).astype(np.int64)
result=[]
for channel,shift,bits,bs,ss in [('red',11,3,range(192,200),range(248,256)),('green',5,2,range(220,224),range(252,256)),('blue',0,3,range(248,256),range(248,256))]:
 expected=(e>>shift)&(63 if bits==2 else 31);cases=[]
 for bg in bs:
  for source in ss:
   t=bg*(255-a);s=source*a
   values=((s+128+((s+128)>>8))>>8)+((t+128+((t+128)>>8))>>8)
   candidate=values>>bits;candidate[0,:]=0;candidate[:,0]=0
   differences=np.count_nonzero(candidate!=expected)
   if differences<=1:cases.append(dict(background=bg,source=source,difference=int(differences)))
 result.append(dict(channel=channel,candidates=cases))
p=here.parents[1]/'kinseal-stage4-20261112/worktree/docs/stage5-evidence/native/sector-investigation-20261114/source-channel-sweep-r1.json'
p.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');print(json.dumps(result))
