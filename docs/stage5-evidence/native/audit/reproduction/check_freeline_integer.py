"""Compare the retained native two-endpoint freeline against the independently established integer-line candidate."""
from pathlib import Path
import hashlib
import json
import os
import subprocess

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
base=work/'docs/stage5-evidence/native/audit'
case=next(c for c in json.loads((base/'candidate-comparison-r2.json').read_text(encoding='utf-8'))['cases'] if c['family']=='freeline')
folder=work/case['folder']
environment=os.environ.copy()
environment.update(TEMP=str(root/'temp'),TMP=str(root/'temp'),TMPDIR=str(root/'temp'))
environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+environment.get('PATH','')
decoder=root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'
native=root/'temp/audit-freeline-integer-native.rgb565'
candidate=root/'temp/audit-freeline-integer-candidate.rgb565'
subprocess.run([str(decoder),str(folder/'SheetPixel.dat'),str(native),'768000'],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW)
subprocess.run([str(root/'line_raster_probe.exe'),str(candidate),'100','100','280','200','bresenham'],check=True,capture_output=True,env=environment,creationflags=subprocess.CREATE_NO_WINDOW)
expected=native.read_bytes();actual=candidate.read_bytes()
assert len(expected)==len(actual)==768000
result=dict(sourceSha256=case['sourceSha256'],geometryBinding='previous verified >Idd endpoint prefix: (100,100) to (280,200)',candidate='integer_major_axis_strict_negative_ties_excluding_last_endpoint',nativeSha256=hashlib.sha256(expected).hexdigest(),candidateSha256=hashlib.sha256(actual).hexdigest(),differentPixels=sum(expected[i:i+2]!=actual[i:i+2] for i in range(0,len(expected),2)),productCompilerOpened=False,usbAccessed=False,E=False)
destination=base/'freeline-integer-candidate.json'
assert not destination.exists()
destination.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
native.unlink();candidate.unlink()
print(json.dumps(result,ensure_ascii=False))
