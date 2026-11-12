"""Start C28 with independent gate and native minimal table source observations."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent
owner=root/'native-alarm-display';owner.mkdir(exist_ok=True);(owner/'temp').mkdir(exist_ok=True)
(root/'worktree/docs/stage4-evidence/alarm-display/native').mkdir(parents=True,exist_ok=True)
s=(root/'native-alarm-bar/alarm_probe.py').read_text(encoding='utf-8').replace('stage4-evidence/alarm-bar/native','stage4-evidence/alarm-display/native').replace('native-alarm-bar','native-alarm-display').replace('KinSealStage4AlarmBar20261113','KinSealStage4AlarmDisplay20261113')
(owner/'alarm_display_probe.py').write_text(s,encoding='utf-8',newline='\n')
gate=[dict(op='action',name='action_checkall'),dict(op='action',name='action_delete'),dict(op='scene-assert',minSelectableRoots=0),dict(op='action',name='action_arlamcontrol'),dict(op='dump',label='alarm-display-gate-r1'),dict(op='screenshot',label='alarm-display-gate-r1'),dict(op='invoke',kind='QPushButton',text='确定')]
default=[dict(op='scene-assert',minSelectableRoots=0),dict(op='action',name='action_arlamcontrol'),dict(op='dump',label='alarm-display-basic-r2'),dict(op='screenshot',label='alarm-display-basic-r2'),dict(op='tab',kind='QTabWidget',name='tabWidget',value=1),dict(op='dump',label='alarm-display-access-r2'),dict(op='tab',kind='QTabWidget',name='tabWidget',value=0),dict(op='invoke',kind='QPushButton',text='确定'),dict(op='scene-drag',**{'from':[150,120],'to':[470,280]}),dict(op='scene-assert',minSelectableRoots=1),dict(op='save',output='alarm-display-default.sm'),dict(op='native-compile',label='alarm-display-default-r2'),dict(op='scene-select',soleRoot=True),dict(op='action',name='action_openpropertiset'),dict(op='dump',label='alarm-display-reopened-r2'),dict(op='invoke',kind='QPushButton',text='取消')]
for name,data in [('alarm-display-gate-r1',gate),('alarm-display-default-r2',default)]:
    (owner/(name+'.json')).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
