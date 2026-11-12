"""Inspect the original alarm-login editor before implementing the required C27 definition lifecycle."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-alarm-bar'
s=[dict(op='action',name='action_checkall'),dict(op='action',name='action_delete'),dict(op='scene-assert',minSelectableRoots=0),dict(op='tree-double',entry='数位地址报警'),dict(op='dump',label='bit-alarm-default-r2'),dict(op='screenshot',label='bit-alarm-default-r2'),dict(op='invoke',kind='QPushButton',text='取消'),dict(op='tree-double',entry='条件报警'),dict(op='dump',label='condition-alarm-default-r2'),dict(op='screenshot',label='condition-alarm-default-r2'),dict(op='invoke',kind='QPushButton',text='取消')]
(owner/'alarm-login-r2.json').write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
