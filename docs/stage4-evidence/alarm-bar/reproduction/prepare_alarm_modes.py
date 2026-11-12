from pathlib import Path
import json
p=Path(__file__).resolve().parent/'native-alarm-bar'
s=[dict(op='tree-double',entry='数位地址报警'),dict(op='combo-queued',kind='QComboBox',name='nAlarmType',value=1),dict(op='dump',label='bit-switch1-r7')]
(p/'alarm-modes-r7.json').write_text(json.dumps(s,ensure_ascii=False),encoding='utf-8')
