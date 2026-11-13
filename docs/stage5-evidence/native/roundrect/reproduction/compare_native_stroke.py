from pathlib import Path
import hashlib
import json
import os
import subprocess
import numpy as np

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
base=work/'docs/stage5-evidence/native/audit';folder=base/'paint-roundrect-stroke-r3'
record=json.loads((folder/'native-probe.json').read_text(encoding='utf-8'))
trace=next(x['payload'] for x in record['instrumentationMessages'] if x.get('payload',{}).get('nativeStrokeStreamHex'))
path=root/'temp/native-roundrect-r3.pathstream';stroke=root/'temp/native-roundrect-r3-stroke.pathstream'
path.write_bytes(bytes.fromhex(trace['pathStreamHex']));stroke.write_bytes(bytes.fromhex(trace['nativeStrokeStreamHex']))
decoder=root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'
native=root/'temp/native-roundrect-r3.rgb565'
subprocess.run([str(decoder),str(next((folder/'compiled').iterdir())/'SheetPixel.dat'),str(native),'768000'],check=True,capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW)
expected=native.read_bytes();environment=os.environ.copy()
environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+environment.get('PATH','')
records=[]
for mode in ('roundrect-local','roundrect-local-rgb16'):
    output=root/'temp/native-stroke-r3.rgb565'
    subprocess.run([str(root/'path_trace_replay.exe'),str(output),str(path),mode,str(stroke)],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW)
    actual=output.read_bytes();indices=np.flatnonzero(np.frombuffer(actual,dtype='<u2')!=np.frombuffer(expected,dtype='<u2'))
    records.append(dict(mode=mode,differentPixels=len(indices),first=[dict(x=int(i%800),y=int(i//800),native=expected[i*2:i*2+2].hex(),candidate=actual[i*2:i*2+2].hex()) for i in indices[:60]]))
    output.unlink()
for p in (path,stroke,native):p.unlink()
destination=base/'native-stroke-replay-r1.json';assert not destination.exists()
destination.write_text(json.dumps(dict(candidates=records,usbAccessed=False,productCompilerOpened=False,E=False),indent=2)+'\n',encoding='utf-8')
print([(v['mode'],v['differentPixels']) for v in records])
