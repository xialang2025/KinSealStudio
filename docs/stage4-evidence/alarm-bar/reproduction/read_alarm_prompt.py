import json
from pathlib import Path
p=Path(__file__).resolve().parent/'worktree/docs/stage4-evidence/alarm-bar/native/native-probe-alarm-modes-r7.json'
d=json.loads(p.read_text(encoding='utf-8'))
def walk(v):
    if isinstance(v,dict):
        if v.get('text'): print(v.get('kind'),v.get('objectName'),v['text'])
        for x in v.values():walk(x)
    elif isinstance(v,list):
        for x in v:walk(x)
walk([w for w in d['snapshots'][-1]['windows'] if w['title']=='提示'])
