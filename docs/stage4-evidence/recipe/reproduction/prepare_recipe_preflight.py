"""Prepare read-only native feasibility observations outside the C30 delivery tree."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-recipe';owner.mkdir(exist_ok=True);(owner/'temp').mkdir(exist_ok=True);out=root/'recipe-preflight/native';out.mkdir(parents=True,exist_ok=True)
s=(root/'native-history-data/history_data_probe.py').read_text(encoding='utf-8').replace('worktree/docs/stage4-evidence/history-data/native','recipe-preflight/native').replace('native-history-data','native-recipe').replace('KinSealStage4HistoryData20261113','KinSealStage4Recipe20261113')
(owner/'recipe_probe.py').write_text(s,encoding='utf-8',newline='\n')
gate=[dict(op='action',name='action_recipetable'),dict(op='dump',label='recipe-no-config-r1'),dict(op='screenshot',label='recipe-no-config-r1')]
(owner/'recipe-gate-r1.json').write_text(json.dumps(gate,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
data=json.loads((root/'worktree/docs/main-window-evidence/native-shell.json').read_text(encoding='utf-8'))
def walk(v):
    if isinstance(v,dict):
        if ('配方' in v.get('text','') or 'recipe' in v.get('id','').lower()) and 'id' in v:
            print(json.dumps({k:v[k] for k in ['id','text'] if k in v},ensure_ascii=False))
        for x in v.values():walk(x)
    elif isinstance(v,list):
        for x in v:walk(x)
walk(data)
