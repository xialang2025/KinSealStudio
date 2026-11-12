"""Create a separately owned private trend investigation from the verified C22 recipe."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent
owner=root/'native-trend'; owner.mkdir(exist_ok=True)
(owner/'temp').mkdir(exist_ok=True)
(root/'worktree/docs/stage4-evidence/trend/native').mkdir(parents=True,exist_ok=True)
source=(root/'native-chart/chart_probe.py').read_text(encoding='utf-8').replace('stage4-evidence/chart/native','stage4-evidence/trend/native').replace('native-chart','native-trend').replace('KinSealStage4Chart20261112','KinSealStage4Trend20261112').replace("r['title']=='图表'","r['title']=='趋势图'")
(owner/'trend_probe.py').write_text(source,encoding='utf-8',newline='\n')
s=[dict(op='action',name='action_checkall'),dict(op='action',name='action_delete'),dict(op='scene-assert',minSelectableRoots=0),dict(op='action',name='action_trendchart'),dict(op='dump',label='trend-basic-r1'),dict(op='screenshot',label='trend-basic-r1'),dict(op='tab',kind='QTabWidget',name='tabWidget',value=1),dict(op='dump',label='trend-display-r1'),dict(op='screenshot',label='trend-display-r1'),dict(op='tab',kind='QTabWidget',name='tabWidget',value=2),dict(op='dump',label='trend-access-r1'),dict(op='tab',kind='QTabWidget',name='tabWidget',value=0),dict(op='invoke',text='确定',kind='QPushButton'),dict(op='scene-drag',**{'from':[160,120],'to':[400,280]}),dict(op='scene-assert',minSelectableRoots=1),dict(op='save',output='trend-default.sm'),dict(op='native-compile',label='trend-default-r1'),dict(op='scene-select',soleRoot=True)]
previous=json.loads((root/'native-track/track-default-r1.json').read_text(encoding='utf-8'))
s += [next(x for x in previous if x.get('op')=='action' and 'openpro' in x.get('name','')),dict(op='dump',label='trend-reopened-r1'),dict(op='invoke',text='取消',kind='QPushButton')]
(owner/'trend-r1.json').write_text(json.dumps(s,ensure_ascii=False,indent=2),encoding='utf-8')
(root/'pending-push.json').write_text(json.dumps(dict(status='pushed',head='1602b816e25c87afe7af0850ffa79c527b89b97d',detail='C22 chart commit pushed successfully; main stage3 changes preserved.'),ensure_ascii=False,indent=2),encoding='utf-8')
