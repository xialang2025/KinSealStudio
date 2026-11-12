"""Start the C25 native baseline on a separately owned desktop without modifying pending push provenance."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-xy';owner.mkdir(exist_ok=True);(owner/'temp').mkdir(exist_ok=True)
base=root/'worktree/docs/stage4-evidence/xy-curve/native';base.mkdir(parents=True,exist_ok=True)
s=(root/'native-historytrend/history_probe.py').read_text(encoding='utf-8').replace('stage4-evidence/history-trend/native','stage4-evidence/xy-curve/native').replace('native-historytrend','native-xy').replace('KinSealStage4HistoryTrend20261112','KinSealStage4XY20261112').replace('历史趋势图','X-Y曲线')
(owner/'xy_probe.py').write_text(s,encoding='utf-8',newline='\n')
scenario=json.loads((root/'native-trend/trend-r1.json').read_text(encoding='utf-8'))
for item in scenario:
 for k,v in item.items():
  if isinstance(v,str):item[k]=v.replace('trend-','xy-').replace('action_trendchart','action_X_Y')
(owner/'xy-default-r1.json').write_text(json.dumps(scenario,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
(root/'pending-push.json').write_text(json.dumps(dict(status='pending',head='a96c20b6ec03d4d91e57c2be8bc6da80b62a39ca',detail='C23 and C24 local commits retained; github.com:443 connection failed after 21069 ms on the C24 push attempt.'),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print('Prepared isolated C25 baseline and retained exact pending-push state.')
