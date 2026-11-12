"""Probe trend range validators, independent channel drafts and axis field lifetime."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent/'native-trend'
old=json.loads((root/'trend-r1.json').read_text(encoding='utf-8'))
s=old[:4]
s += [dict(op='validator',name='LE_Min',kind='QLineEdit',label='trend-channel-minimum'),dict(op='validator',name='LE_Max',kind='QLineEdit',label='trend-channel-maximum')]
for t in range(7):
 s += [dict(op='combo',name='comboBox_dataType',kind='QComboBox',value=t),dict(op='dump',label=f'trend-type-{t}-r2')]
s += [dict(op='combo',name='comboBox_dataType',kind='QComboBox',value=0),dict(op='spin',name='spinCount',kind='QSpinBox',value=3),dict(op='combo',name='CB_CurrentValue',kind='QComboBox',value=2),dict(op='text',name='LE_Min',kind='QLineEdit',value='-10'),dict(op='text',name='LE_Max',kind='QLineEdit',value='2000'),dict(op='combo',name='CB_lineStyle',kind='QComboBox',value=2),dict(op='combo',name='CB_CurrentValue',kind='QComboBox',value=0),dict(op='dump',label='trend-channel1-r2'),dict(op='combo',name='CB_CurrentValue',kind='QComboBox',value=2),dict(op='dump',label='trend-channel3-r2'),dict(op='spin',name='spinCount',kind='QSpinBox',value=1),dict(op='spin',name='spinCount',kind='QSpinBox',value=3),dict(op='combo',name='CB_CurrentValue',kind='QComboBox',value=2),dict(op='dump',label='trend-regrown-channel3-r2'),dict(op='tab',name='tabWidget',kind='QTabWidget',value=1),dict(op='validator',name='Y_Min',kind='QLineEdit',label='trend-y-minimum'),dict(op='validator',name='Y_Max',kind='QLineEdit',label='trend-y-maximum')]
for name in ['X_Show_Ticks','Y_Show_Ticks','X_If_Show_Marks','Y_If_Show_Marks']:
 s += [dict(op='invoke',name=name,kind='QCheckBox'),dict(op='dump',label=name+'-off-r2'),dict(op='invoke',name=name,kind='QCheckBox')]
s += [dict(op='text',name='Y_Min',kind='QLineEdit',value='-20'),dict(op='text',name='Y_Max',kind='QLineEdit',value='5000'),dict(op='spin',name='Y_TotalDigits',kind='QSpinBox',value=6),dict(op='spin',name='Y_Fractional',kind='QSpinBox',value=2),dict(op='dump',label='trend-y-custom-r2')]
s += [dict(x,label=x.get('label','').replace('r1','r2'),output='trend-custom.sm') if x.get('op')=='save' else {k:(v.replace('r1','r2') if isinstance(v,str) else v) for k,v in x.items()} for x in old[12:]]
(root/'trend-ui-r2.json').write_text(json.dumps(s,ensure_ascii=False,indent=2),encoding='utf-8')
