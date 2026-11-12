"""Probe C25 type lifetime, independent channels and both numeric axis mark blocks."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-xy'
baseline=json.loads((owner/'xy-default-r1.json').read_text(encoding='utf-8'))
s=baseline[:4]
for t in range(7):s += [dict(op='combo',name='comboBox_dataType',kind='QComboBox',value=t),dict(op='dump',label=f'xy-type-{t}-r2')]
s += [dict(op='combo',name='comboBox_dataType',kind='QComboBox',value=0)]
channel=json.loads((root/'native-trend/trend-channel-r3.json').read_text(encoding='utf-8'))
end=next(i for i,x in enumerate(channel[4:],4) if x.get('op')=='invoke' and x.get('text')=='确定')
for x in channel[4:end]:
 s.append({k:(v.replace('trend-','xy-').replace('r3','r2') if isinstance(v,str) else v) for k,v in x.items()})
s += [dict(op='tab',name='tabWidget',kind='QTabWidget',value=1)]
for n in ['X_Show_Ticks','X_Show_Y_Axis','X_If_Show_Marks','Y_If_Show_Marks']:
 s += [dict(op='invoke',name=n,kind='QCheckBox'),dict(op='dump',label=n+'-off-r2'),dict(op='invoke',name=n,kind='QCheckBox')]
for n,v in [('X_Min','-100'),('X_Max','4000'),('Y_Min','-20'),('Y_Max','5000')]:
 s += [dict(op='text',name=n,kind='QLineEdit',value=v),dict(op='invoke',name=n,kind='QLineEdit',slot='editingFinished')]
s += [dict(op='spin',name='X_TotalDigits',kind='QSpinBox',value=6),dict(op='spin',name='X_Fractional',kind='QSpinBox',value=2),dict(op='dump',label='xy-axis-custom-r2'),dict(op='screenshot',label='xy-axis-custom-r2')]
start=next(i for i,x in enumerate(baseline) if x.get('op')=='invoke' and x.get('text')=='确定')
for x in baseline[start:]:
 s.append({k:(v.replace('xy-default','xy-custom').replace('r1','r2') if isinstance(v,str) else v) for k,v in x.items()})
(owner/'xy-survey-r2.json').write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
p=owner/'xy_probe.py';p.write_text(p.read_text(encoding='utf-8').replace('2026-11-12','2026-11-13'),encoding='utf-8',newline='\n')
