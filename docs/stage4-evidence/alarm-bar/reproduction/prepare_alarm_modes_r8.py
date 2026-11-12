from pathlib import Path
import json
p=Path(__file__).resolve().parent/'native-alarm-bar'
probe=p/'alarm_probe.py'
s=probe.read_text(encoding='utf-8')
s=s.replace("ws=[w for r in rows_now() for w in r['widgets'] if w.get('qt')]", "ws=[w for r in rows_now() if 'window' not in command or r['title']==command['window'] for w in r['widgets'] if w.get('qt')]")
probe.write_text(s,encoding='utf-8',newline='\n')
s=[dict(op='tree-double',entry='数位地址报警')]
for mode in [1,2]:
    s += [dict(op='combo-queued',kind='QComboBox',name='nAlarmType',value=mode),dict(op='invoke',kind='QPushButton',text='确定',window='提示'),dict(op='combo',kind='QComboBox',name='nReadLen',value=2),dict(op='dump',label=f'bit-mode{mode}-r8'),dict(op='table-state',kind='QTableWidget',name='BitAlarmTable',label=f'bit-mode{mode}-r8')]
s += [dict(op='invoke',kind='QPushButton',text='取消')]
(p/'alarm-modes-r8.json').write_text(json.dumps(s,ensure_ascii=False),encoding='utf-8')
