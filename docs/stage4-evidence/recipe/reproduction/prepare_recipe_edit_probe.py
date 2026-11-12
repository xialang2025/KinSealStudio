from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-recipe'
steps=[dict(op='action',name='action_recipetable'),dict(op='group',kind='QGroupBox',name='groupBox_edit',value=True),dict(op='dump',label='recipe-edit-expanded-r4'),dict(op='screenshot',label='recipe-edit-expanded-r4'),dict(op='invoke',kind='QPushButton',text='取消'),dict(op='assert-main-only')]
(owner/'recipe-edit-r4.json').write_text(json.dumps(steps,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
