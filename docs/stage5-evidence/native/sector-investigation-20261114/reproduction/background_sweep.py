"""Independently test whether RGB565 background quantization hides RGB32 source-channel ambiguity."""
from pathlib import Path
import json, numpy as np
here=Path(__file__).resolve().parent
a=np.frombuffer((here/'temp/qt6.mask').read_bytes(),np.uint8).reshape(480,800).astype(np.int64)
expected=np.frombuffer((here/'temp/native.rgb565').read_bytes(),'<u2').reshape(480,800).astype(np.int64)
records=[]
for red in range(192,200):
 for green in range(220,224):
  for blue in range(248,256):
   channels=[]
   for value in (red,green,blue):
    t=value*(255-a);channels.append(a+((t+128+((t+128)>>8))>>8))
   r,g,b=channels;candidate=((r>>3)<<11)|((g>>2)<<5)|(b>>3)
   candidate[0,:]=0;candidate[:,0]=0
   different=int(np.count_nonzero(candidate!=expected))
   if different<=1:records.append(dict(rgb=[red,green,blue],differentPixels=different))
out=here.parents[1]/'kinseal-stage4-20261112/worktree/docs/stage5-evidence/native/sector-investigation-20261114/background-sweep-r1.json'
out.write_text(json.dumps(dict(candidates=records,scope='sector default only',productionChanged=False),indent=2)+'\n',encoding='utf-8')
print(json.dumps(records))
