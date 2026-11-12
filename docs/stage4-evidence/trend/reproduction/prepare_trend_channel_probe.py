"""Complete native text-edit signals before judging per-channel persistence."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent/'native-trend'
old=json.loads((root/'trend-r1.json').read_text(encoding='utf-8'))
s=old[:4]+[dict(op='spin',name='spinCount',kind='QSpinBox',value=3),dict(op='combo',name='CB_CurrentValue',kind='QComboBox',value=2)]
for name,value in [('LE_Min','-10'),('LE_Max','2000')]:
 s += [dict(op='text',name=name,kind='QLineEdit',value=value),dict(op='invoke',name=name,kind='QLineEdit',slot='editingFinished')]
s += [dict(op='combo',name='CB_lineStyle',kind='QComboBox',value=2),dict(op='dump',label='trend-channel3-edited-r3'),dict(op='combo',name='CB_CurrentValue',kind='QComboBox',value=0),dict(op='dump',label='trend-channel1-r3'),dict(op='combo',name='CB_CurrentValue',kind='QComboBox',value=2),dict(op='dump',label='trend-channel3-restored-r3')]
for x in old[12:]:
 x={k:(v.replace('r1','r3') if isinstance(v,str) else v) for k,v in x.items()}
 if x.get('op')=='save': x['output']='trend-channel-custom.sm'
 s.append(x)
(root/'trend-channel-r3.json').write_text(json.dumps(s,ensure_ascii=False,indent=2),encoding='utf-8')
