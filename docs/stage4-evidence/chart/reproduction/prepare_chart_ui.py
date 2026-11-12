"""Build focused native chart field enumeration and shape baseline retries."""
from pathlib import Path
import json
owner=Path(__file__).resolve().parent/'native-chart'
s=json.loads((owner/'meter-r4.json').read_text(encoding='utf-8'))
s=[dict(x,op='tab-click',x=75) if x.get('op')=='tab' else x for x in s]
for x in s:
 for k,v in list(x.items()):
  if isinstance(v,str): x[k]=v.replace('r4','r5')
(owner/'meter-r5.json').write_text(json.dumps(s,ensure_ascii=False,indent=2),encoding='utf-8')
start=json.loads((owner/'bar-r4.json').read_text(encoding='utf-8'))[:4]
s=start.copy()
for i in range(7):
 s += [dict(op='combo',kind='QComboBox',name='Date_type',value=i),dict(op='dump',label='bar-type-'+str(i)+'-r6')]
s += [dict(op='invoke',kind='QCheckBox',name='twopoles_checkBox'),dict(op='dump',label='bar-midpoint-r6'),dict(op='combo',kind='QComboBox',name='range_Combox',value=1),dict(op='dump',label='bar-address-range-r6'),dict(op='invoke',text='取消',kind='QPushButton')]
(owner/'bar-ui-r6.json').write_text(json.dumps(s,ensure_ascii=False,indent=2),encoding='utf-8')
