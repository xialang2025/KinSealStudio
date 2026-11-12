from pathlib import Path
import json
p=Path(__file__).resolve().parent/'worktree/docs/stage4-evidence/alarm-bar/native/native-probe-alarm-modes-r8.json'
d=json.loads(p.read_text(encoding='utf-8'))
for t in d.get('nativeTableStates',[]):
    print(t['label'],t['state']['rows'],json.dumps(t['state']['cells'][:3],ensure_ascii=False))
def walk(v):
    if isinstance(v,dict):
        if v.get('objectName') in ['sReadAddr','nReadLen','sMessageType']: print(v.get('objectName'),v.get('text'),v.get('currentText'),v.get('count'))
        for x in v.values():walk(x)
    elif isinstance(v,list):
        for x in v:walk(x)
for s in d['snapshots']:
    if s['label'].startswith('bit-mode'): print(s['label']);walk(s['windows'])
