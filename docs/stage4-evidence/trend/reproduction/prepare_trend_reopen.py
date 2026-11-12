"""Check saved third-channel values in a fresh native process."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent/'native-trend'
previous=json.loads((root/'trend-r1.json').read_text(encoding='utf-8'))
action=next(x for x in previous if x.get('op')=='action' and 'openpro' in x.get('name',''))
s=[dict(op='scene-assert',minSelectableRoots=1),dict(op='scene-select',soleRoot=True),action,dict(op='dump',label='trend-fresh-channel1-r4'),dict(op='combo',name='CB_CurrentValue',kind='QComboBox',value=2),dict(op='dump',label='trend-fresh-channel3-r4'),dict(op='invoke',text='取消',kind='QPushButton')]
(root/'trend-reopen-r4.json').write_text(json.dumps(s,ensure_ascii=False,indent=2),encoding='utf-8')
