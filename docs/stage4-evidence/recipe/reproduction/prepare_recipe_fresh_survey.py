from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-recipe'
steps=json.loads((owner/'recipe-survey-r6.json').read_text(encoding='utf-8'));steps[:1]=[dict(op='action',name='action_rep'),dict(op='text',kind='QLineEdit',name='sRecipeName',value='C31Multi')]
for step in steps:
    for key in ('label','output'):
        if key in step:step[key]=step[key].replace('-r6','-r7')
(owner/'recipe-survey-r7.json').write_text(json.dumps(steps,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
