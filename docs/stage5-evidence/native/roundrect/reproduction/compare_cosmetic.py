"""Test actual observed zero-width pen against canonical full backgrounds."""
from pathlib import Path
import hashlib
import json
import os
import subprocess
import numpy as np

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
base=work/'docs/stage5-evidence/native/audit'
environment=os.environ.copy()
environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+environment.get('PATH','')
decoder=root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'
records=[]
for family,geometry in [('ellipse',[100,100,200,100]),('roundrect',[100,100,200,100]),('polygon',[100,100,200,120]),('sector',[100,100,200,200])]:
    folder=base/('audit-'+family+'-default-r1/compiled/canonical-default')
    native=root/('temp/cosmetic-'+family+'-native.rgb565')
    subprocess.run([str(decoder),str(folder/'SheetPixel.dat'),str(native),'768000'],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW)
    expected=native.read_bytes()
    item=dict(family=family,nativeSha256=hashlib.sha256(expected).hexdigest(),candidates=[])
    for mode in ('stroker','stroker-rgb32','stroker-aa','stroker-aa-rgb32'):
        candidate=root/('temp/cosmetic-'+family+'-candidate.rgb565')
        subprocess.run([str(root/'graphic_candidate_probe.exe'),str(candidate),family,*map(str,geometry),mode],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW)
        actual=candidate.read_bytes()
        diff=np.frombuffer(expected,dtype='<u2')!=np.frombuffer(actual,dtype='<u2')
        indices=np.flatnonzero(diff)
        item['candidates'].append(dict(mode=mode,differentPixels=int(diff.sum()),sha256=hashlib.sha256(actual).hexdigest(),first=[dict(x=int(i%800),y=int(i//800),native=expected[i*2:i*2+2].hex(),candidate=actual[i*2:i*2+2].hex()) for i in indices[:12]]))
        candidate.unlink()
    native.unlink()
    records.append(item)
    print(family,[(c['mode'],c['differentPixels']) for c in item['candidates']],flush=True)
destination=base/'curved-stroker-candidates-r1.json'
assert not destination.exists()
destination.write_text(json.dumps(dict(cases=records,usbAccessed=False,productCompilerOpened=False,E=False),indent=2)+'\n',encoding='utf-8')
