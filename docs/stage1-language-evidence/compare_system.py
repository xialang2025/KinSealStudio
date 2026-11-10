"""#20261110-6 add: Compare actual Lite outputs with the retained native compiler segments."""
from pathlib import Path
import hashlib
import json
import os
import subprocess
ROOT = Path(__file__).resolve().parents[2]
OUT = Path(__file__).resolve().parent / 'lite-comparison-r1'
OUT.mkdir(exist_ok=False)
env = os.environ.copy()
env['PATH'] = r'G:\01_Professional\QT6\Installed\6.12.0\mingw_64\bin;G:\01_Professional\QT6\Installed\Tools\mingw1310_64\bin;' + env['PATH']
env['QT_QPA_PLATFORM'] = 'windows'
records = []
for name in ['settings1-editor0', 'settings0-editor1', 'settings2-editor1-three']:
    source = ROOT / 'tests/fixtures/native-source/language' / (name+'-native.sm')
    folder = OUT / name
    result = subprocess.run([str(ROOT/'build/KinSealStudioLite.exe'),'--compile-project',str(source),str(folder)],
        env=env,capture_output=True,timeout=120,creationflags=subprocess.CREATE_NO_WINDOW)
    row = dict(name=name,exitCode=result.returncode,stdout=result.stdout.decode('utf-8',errors='replace'),stderr=result.stderr.decode('utf-8',errors='replace'))
    if not result.returncode:
        native = (ROOT/'tests/fixtures/native-source/language'/(name+'-SysPara.dat')).read_bytes()
        lite = (folder/'SysPara.dat').read_bytes()
        row.update(nativeBytes=len(native), liteBytes=len(lite), differences=[dict(offset=i,native=a,lite=b) for i,(a,b) in enumerate(zip(native,lite)) if a!=b])
    records.append(row)
(OUT/'comparison.json').write_text(json.dumps(records,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(records,ensure_ascii=False,indent=2))
