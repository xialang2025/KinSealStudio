"""Start C26's dependency-free native data-group baseline on its own desktop and temp root."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-data-group';owner.mkdir(exist_ok=True);(owner/'temp').mkdir(exist_ok=True)
(root/'worktree/docs/stage4-evidence/data-group/native').mkdir(parents=True,exist_ok=True)
s=(root/'native-xy/xy_probe.py').read_text(encoding='utf-8').replace('stage4-evidence/xy-curve/native','stage4-evidence/data-group/native').replace('native-xy','native-data-group').replace('KinSealStage4XY20261112','KinSealStage4DataGroup20261113').replace('X-Y曲线','数据群组')
(owner/'group_probe.py').write_text(s,encoding='utf-8',newline='\n')
scenario=json.loads((root/'native-xy/xy-default-r1.json').read_text(encoding='utf-8'))
for item in scenario:
 for k,v in item.items():
  if isinstance(v,str):item[k]=v.replace('xy-','group-').replace('action_X_Y','action_dataGroup')
(owner/'group-default-r1.json').write_text(json.dumps(scenario,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
