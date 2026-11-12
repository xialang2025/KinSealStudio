"""Measure alarm type switching and per-register condition type ownership."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent
owner=root/'native-alarm-bar'
s=[dict(op='tree-double',entry='数位地址报警')]
for mode in [0,1,2]:
    s += [dict(op='combo',kind='QComboBox',name='nAlarmType',value=mode),dict(op='combo',kind='QComboBox',name='nReadLen',value=2),dict(op='dump',label=f'bit-mode{mode}-r5'),dict(op='table-state',kind='QTableWidget',name='BitAlarmTable',label=f'bit-mode{mode}-r5')]
s += [dict(op='invoke',kind='QPushButton',text='取消'),dict(op='tree-double',entry='条件报警'),dict(op='combo',kind='QComboBox',name='nReadLen',value=2),dict(op='table-click',kind='QTableWidget',name='CondAlarmTable',row=0,column=1),dict(op='combo',kind='QComboBox',name='nDataType',value=3),dict(op='text-edit',kind='QLineEdit',name='sLimit',value='-123'),dict(op='text-edit',kind='QLineEdit',name='sMessage',value='COND-LOW')]
for row in [1,4,0]:
    s += [dict(op='table-click',kind='QTableWidget',name='CondAlarmTable',row=row,column=1),dict(op='dump',label=f'condition-row{row}-r5')]
s += [dict(op='table-state',kind='QTableWidget',name='CondAlarmTable',label='condition-edited-r5'),dict(op='invoke',kind='QPushButton',text='确定'),dict(op='save',output='alarm-condition.sm'),dict(op='native-compile',label='alarm-condition-r5'),dict(op='tree-double',entry='条件报警'),dict(op='dump',label='condition-reopened-r5'),dict(op='invoke',kind='QPushButton',text='取消')]
(owner/'alarm-lifetime-r5.json').write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
