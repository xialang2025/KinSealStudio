"""Measure data-group sampling and range-source visibility instead of copying neighboring chart semantics."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-data-group';baseline=json.loads((owner/'group-default-r1.json').read_text(encoding='utf-8'));s=baseline[:4]
for t in range(7):s += [dict(op='combo',name='comboBox_dataType',kind='QComboBox',value=t),dict(op='dump',label=f'group-type-{t}-r2')]
s += [dict(op='combo',name='comboBox_dataType',kind='QComboBox',value=1),dict(op='spin',name='m_SampleNum_SpinBox',kind='QSpinBox',value=250),dict(op='combo',name='comboBox_Sample',kind='QComboBox',value=1),dict(op='dump',label='group-address-sampling-r2'),dict(op='screenshot',label='group-address-sampling-r2'),dict(op='text',name='m_SampleAddrLineEdit',kind='QLineEdit',value='LW10'),dict(op='invoke',name='m_SampleAddrLineEdit',kind='QLineEdit',slot='editingFinished'),dict(op='combo',name='CB_lineStyle',kind='QComboBox',value=3),dict(op='tab',name='tabWidget',kind='QTabWidget',value=1),dict(op='combo',name='m_LimitSourcecbBox',kind='QComboBox',value=1),dict(op='dump',label='group-range-address-r2'),dict(op='screenshot',label='group-range-address-r2')]
s += [dict(op='text',name='Y_Max_lineEdit',kind='QLineEdit',value='LW20'),dict(op='invoke',name='Y_Max_lineEdit',kind='QLineEdit',slot='editingFinished'),dict(op='combo',name='m_LimitSourcecbBox',kind='QComboBox',value=0),dict(op='dump',label='group-range-constant-restored-r2')]
for n,v in [('Y_Min','-20'),('Y_Max','5000')]:s += [dict(op='text',name=n,kind='QLineEdit',value=v),dict(op='invoke',name=n,kind='QLineEdit',slot='editingFinished')]
s += [dict(op='combo',name='m_LimitSourcecbBox',kind='QComboBox',value=1),dict(op='dump',label='group-range-address-restored-r2')]
for n in ['X_Show_Ticks','X_Show_Y_Axis','X_If_Show_Marks','Y_If_Show_Marks']:
 s += [dict(op='invoke',name=n,kind='QCheckBox'),dict(op='dump',label=n+'-off-r2'),dict(op='invoke',name=n,kind='QCheckBox')]
s += [dict(op='tab',name='tabWidget',kind='QTabWidget',value=0),dict(op='combo',name='comboBox_Sample',kind='QComboBox',value=0),dict(op='dump',label='group-constant-sampling-restored-r2'),dict(op='combo',name='comboBox_Sample',kind='QComboBox',value=1)]
start=next(i for i,x in enumerate(baseline) if x.get('op')=='invoke' and x.get('text')=='确定')
for x in baseline[start:]:s.append({k:(v.replace('group-default','group-address').replace('r1','r2') if isinstance(v,str) else v) for k,v in x.items()})
(owner/'group-survey-r2.json').write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
