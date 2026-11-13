from pathlib import Path
import hashlib
import json
import os
import subprocess
import numpy as np
from scanline_candidate import fill, decode

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
base=work/'docs/stage5-evidence/native/audit'
environment=os.environ.copy();environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+environment.get('PATH','')
fillPath=root/'temp/independent-roundrect-fill.pathstream';strokePath=root/'temp/independent-roundrect-stroke.pathstream'
subprocess.run([str(root/'roundrect_path_probe.exe'),str(fillPath),str(strokePath),'200','100','10','10'],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW)
native=root/'temp/independent-roundrect.rgb565';decoder=root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'
subprocess.run([str(decoder),str(next((base/'paint-roundrect-stroke-r3/compiled').iterdir())/'SheetPixel.dat'),str(native),'768000'],check=True,capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW)
expected=native.read_bytes();image=np.zeros((480,800),dtype='<u2');image[1:,1:]=0xc6ff
fill(image,fillPath.read_bytes(),0xffff);fill(image,strokePath.read_bytes(),0)
actual=image.tobytes();different=int(np.count_nonzero(np.frombuffer(expected,dtype='<u2')!=np.frombuffer(actual,dtype='<u2')))
result=dict(independentGeneratedGeometry=True,differentPixels=different,nativeSha256=hashlib.sha256(expected).hexdigest(),candidateSha256=hashlib.sha256(actual).hexdigest(),usbAccessed=False,productCompilerOpened=False,E=False)
destination=base/'roundrect-independent-fixed-candidate-r1.json';assert not destination.exists();destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
for p in (fillPath,strokePath,native):p.unlink()
print(result)
