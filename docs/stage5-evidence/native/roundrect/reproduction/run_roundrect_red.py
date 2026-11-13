from pathlib import Path
import importlib.util
import json
import os

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
spec=importlib.util.spec_from_file_location('private_launcher',work/'docs/stage1-evidence/gui-portable/probe.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
out=work/'docs/stage5-evidence/roundrect-red-r1';out.mkdir(exist_ok=False)
desktop=module.PrivateDesktop(out);environment=module.clean_environment()
environment['PATH']='G:/01_Professional/QT6/Installed/6.12.0/mingw_64/bin;G:/01_Professional/QT6/Installed/Tools/mingw1310_64/bin;'+environment['PATH']
(out/'temp').mkdir();environment.update(TEMP=str(out/'temp'),TMP=str(out/'temp'),TMPDIR=str(out/'temp'))
try:
    result=desktop.run('roundrect-red',work/'build/KinSealStage5CompilerTests.exe',['roundrectCanonicalFrames','-o',str(out/'actual-tests.txt')+',txt'],environment,timeout=100,expected_exit=13)
    (out/'run.json').write_text(json.dumps(dict(result=result,expectation='13 native-backed rows reject unsupported roundrect before gate integration',usbAccessed=False),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print((out/'actual-tests.txt').read_text(encoding='utf-8',errors='replace')[-2400:])
finally:desktop.close()
