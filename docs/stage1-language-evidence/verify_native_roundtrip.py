"""#20261110-6 add: Verify actual CLI persistence, source preservation and name-based remapping."""
from pathlib import Path
from datetime import datetime, timezone
import hashlib
import json
import os
import subprocess
ROOT = Path(__file__).resolve().parents[2]
OUT = Path(__file__).resolve().parent / 'roundtrip-final'
OUT.mkdir(exist_ok=False)
env = os.environ.copy()
env['PATH'] = r'G:\01_Professional\QT6\Installed\6.12.0\mingw_64\bin;G:\01_Professional\QT6\Installed\Tools\mingw1310_64\bin;' + env['PATH']
env['QT_QPA_PLATFORM'] = 'windows'
records = []
def run(label, *args):
    """Input: Label and CLI arguments. Return: Successful invocation; preserve every stdout/stderr."""
    p = subprocess.run([str(ROOT/'build/KinSealStudioLite.exe'), *map(str,args)],env=env,capture_output=True,
                       creationflags=subprocess.CREATE_NO_WINDOW,timeout=120)
    (OUT/(label+'-stdout.txt')).write_bytes(p.stdout)
    (OUT/(label+'-stderr.txt')).write_bytes(p.stderr)
    assert p.returncode == 0, (label,p.returncode,p.stdout,p.stderr)
for name in ['settings1-editor0','settings0-editor1','settings2-editor1-three']:
    source = ROOT/'tests/fixtures/native-source/language'/(name+'-native.sm')
    restored = OUT/(name+'-unchanged.sm')
    model = OUT/(name+'.ksp')
    run(name+'-source-preservation','--export-native-project',source,restored)
    assert source.read_bytes() == restored.read_bytes()
    run(name+'-migrate','--migrate-project',source,model)
    data = json.loads(model.read_text(encoding='utf-8-sig'))
    original = list(data['languages'])
    original_editor = data['activeLanguage']
    data['languages'].reverse()
    data['activeLanguage'] = original[(original.index(original_editor)+1)%len(original)]
    reordered = OUT/(name+'-reordered.ksp')
    reordered.write_text(json.dumps(data,ensure_ascii=False,indent=2),encoding='utf-8')
    exported = OUT/(name+'-reordered.sm')
    compiled = OUT/(name+'-compiled')
    run(name+'-reorder-export','--export-native-project',reordered,exported)
    run(name+'-reorder-compile','--compile-project',exported,compiled)
    system = (compiled/'SysPara.dat').read_bytes()
    actual = int.from_bytes(system[2172:2174],'little')
    assert actual == data['languages'].index(original_editor)
    records.append(dict(name=name,sourceExact=True,originalSystemLanguage=original_editor,
        liteEditorLanguage=data['activeLanguage'],reorderedLanguages=data['languages'],
        compiledSystemIndex=actual,sourceSha256=hashlib.sha256(exported.read_bytes()).hexdigest()))
(OUT/'result.json').write_text(json.dumps(dict(completedAtUtc=datetime.now(timezone.utc).isoformat(),
    records=records,usbAccessed=False),ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(records,ensure_ascii=False))
