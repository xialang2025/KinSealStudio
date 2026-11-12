"""Prepare the owned private chart probe without touching prior native evidence."""
from pathlib import Path
import json

root = Path(__file__).resolve().parent
owner = root / 'native-chart'
owner.mkdir(exist_ok=True)
(owner / 'temp').mkdir(exist_ok=True)
(root / 'worktree/docs/stage4-evidence/chart/native').mkdir(parents=True, exist_ok=True)
source = (root / 'native-track/track_probe.py').read_text(encoding='utf-8')
source = source.replace('stage4-evidence/move-track/native', 'stage4-evidence/chart/native').replace('native-track', 'native-chart').replace('KinSealStage4Track20261112', 'KinSealStage4Chart20261112')
(owner / 'chart_probe.py').write_text(source, encoding='utf-8', newline='\n')
scenario = [dict(op='action', name='action_checkall'), dict(op='action', name='action_delete'), dict(op='scene-assert', minSelectableRoots=0), dict(op='action', name='action_chart'), dict(op='dump', label='chart-default-r1'), dict(op='screenshot', label='chart-default-r1'), dict(op='tab', kind='QTabWidget', name='tabWidget', value=1), dict(op='dump', label='chart-appearance-r1'), dict(op='screenshot', label='chart-appearance-r1'), dict(op='tab', kind='QTabWidget', name='tabWidget', value=2), dict(op='dump', label='chart-access-r1'), dict(op='invoke', text='取消', kind='QPushButton')]
(owner / 'chart-default-r1.json').write_text(json.dumps(scenario, ensure_ascii=False, indent=2), encoding='utf-8')
