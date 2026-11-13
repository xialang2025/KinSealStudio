"""Compare exact native path serialization separately from its raster engine."""
from pathlib import Path
import hashlib
import json
import os
import struct
import subprocess
import numpy as np

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
base=work/'docs/stage5-evidence/native/audit'
environment=os.environ.copy()
environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+environment.get('PATH','')
decoder=root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'
records=[]
for family in ('roundrect','sector'):
    record=json.loads((base/('paint-'+family+'-r1/native-probe.json')).read_text(encoding='utf-8'))
    trace=next(x['payload'] for x in record['instrumentationMessages'] if x.get('payload',{}).get('pathStreamHex'))
    serialized=bytes.fromhex(trace['pathStreamHex'])
    path=root/('temp/native-'+family+'.pathstream');path.write_bytes(serialized)
    count=struct.unpack_from('>I',serialized)[0]
    elements=[dict(type=t,x=x,y=y) for t,x,y in (struct.unpack_from('>Idd',serialized,4+i*20) for i in range(count))]
    folder=base/('paint-'+family+'-r1/compiled/canonical')
    if not folder.exists():
        folder=next((base/('paint-'+family+'-r1/compiled')).iterdir())
    native=root/('temp/native-'+family+'.rgb565')
    subprocess.run([str(decoder),str(folder/'SheetPixel.dat'),str(native),'768000'],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW)
    expected=native.read_bytes()
    item=dict(family=family,elements=elements,nativeRawSha256=hashlib.sha256(expected).hexdigest(),trace=trace,candidates=[])
    for mode in ('local','absolute','local-rgb16','absolute-rgb16','generated-local','generated-local-rgb16'):
        if family=='sector' and 'generated' in mode:continue
        mode=family+'-'+mode
        candidate=root/('temp/native-'+family+'-candidate.rgb565')
        subprocess.run([str(root/'path_trace_replay.exe'),str(candidate),str(path),mode],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW)
        actual=candidate.read_bytes()
        indices=np.flatnonzero(np.frombuffer(actual,dtype='<u2')!=np.frombuffer(expected,dtype='<u2'))
        item['candidates'].append(dict(mode=mode,differentPixels=len(indices),first=[dict(x=int(i%800),y=int(i//800),native=expected[i*2:i*2+2].hex(),candidate=actual[i*2:i*2+2].hex()) for i in indices[:40]]))
        candidate.unlink()
    print(family,[(c['mode'],c['differentPixels']) for c in item['candidates']],flush=True)
    records.append(item);path.unlink();native.unlink()
destination=base/'path-replay-candidates-r1.json'
assert not destination.exists()
destination.write_text(json.dumps(dict(cases=records,usbAccessed=False,productCompilerOpened=False,E=False),indent=2)+'\n',encoding='utf-8')
