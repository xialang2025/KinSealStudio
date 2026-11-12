"""Independently observe the historical-alarm entry, source identity and matching table fields."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-history-alarm';owner.mkdir(exist_ok=True);(owner/'temp').mkdir(exist_ok=True)
(root/'worktree/docs/stage4-evidence/history-alarm/native').mkdir(parents=True,exist_ok=True)
s=(root/'native-alarm-display/alarm_display_probe.py').read_text(encoding='utf-8').replace('stage4-evidence/alarm-display/native','stage4-evidence/history-alarm/native').replace('native-alarm-display','native-history-alarm').replace('KinSealStage4AlarmDisplay20261113','KinSealStage4HistoryAlarm20261113')
(owner/'history_alarm_probe.py').write_text(s,encoding='utf-8',newline='\n')
for previous,name in [('alarm-display-gate-r1','history-alarm-gate-r1'),('alarm-display-default-r2','history-alarm-default-r2'),('alarm-display-custom-r5','history-alarm-custom-r3')]:
    data=json.loads((root/'native-alarm-display'/(previous+'.json')).read_text(encoding='utf-8'))
    for c in data:
        if c.get('name')=='action_arlamcontrol':c['name']='action_histotyarlarm'
        if 'label' in c:c['label']=c['label'].replace('alarm-display','history-alarm').replace('alarm-','history-alarm-',1).replace('-r5','-r3')
        if 'output' in c:c['output']=c['output'].replace('alarm-display','history-alarm')
    (owner/(name+'.json')).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
