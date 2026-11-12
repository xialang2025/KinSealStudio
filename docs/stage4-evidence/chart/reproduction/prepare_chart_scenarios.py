"""Capture separate native shape baselines and measured chart field transitions."""
from pathlib import Path
import json
root = Path(__file__).resolve().parent / 'native-chart'
begin = [dict(op='action', name='action_checkall'), dict(op='action', name='action_delete'), dict(op='scene-assert', minSelectableRoots=0), dict(op='action', name='graphicsitem')]
for shape, radio in [('bar', None), ('meter', 'm_MeterButton_2'), ('donut', 'm_DonutBtn')]:
    steps = begin.copy()
    if radio:
        steps.append(dict(op='invoke', kind='QRadioButton', name=radio))
    steps += [dict(op='dump', label=shape+'-basic-r4'), dict(op='screenshot', label=shape+'-basic-r4')]
    if shape != 'donut':
        steps += [dict(op='tab', name='tabWidget', kind='QTabWidget', value=1), dict(op='dump', label=shape+'-appearance-r4'), dict(op='screenshot', label=shape+'-appearance-r4')]
    steps += [dict(op='invoke', text='确定', kind='QPushButton'), dict(op='scene-drag', **{'from':[160,120], 'to':[360,280]}), dict(op='scene-assert', minSelectableRoots=1), dict(op='save', output=shape+'-default.sm'), dict(op='native-compile', label=shape+'-default-r4'), dict(op='scene-select', soleRoot=True), dict(op='action', name='action_openpropertiset'), dict(op='dump', label=shape+'-reopened-r4'), dict(op='invoke', text='取消', kind='QPushButton')]
    # Use the retained, verified native property action spelling.
    # Action is copied literally from the prior successful track scenario.
    previous = json.loads((root.parent / 'native-track/track-default-r1.json').read_text(encoding='utf-8'))
    steps[-3]['name'] = next(s['name'] for s in previous if s.get('op')=='action' and 'openpro' in s.get('name',''))
    (root / (shape+'-r4.json')).write_text(json.dumps(steps,ensure_ascii=False,indent=2),encoding='utf-8')
