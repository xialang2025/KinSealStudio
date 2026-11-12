from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-recipe';base=root/'recipe-preflight/native'
d=json.loads((base/'native-probe-recipe-gate-r1.json').read_text(encoding='utf-8'))
fields=[]
for snap in d['snapshots']:
    if snap['label']=='recipe-no-config-r1':
        for w in snap['windows']:
            if w['title']=='提示':
                fields.extend(x['qt'] for x in w['widgets'] if x['qt']['kind'] in ('QMessageBox','QLabel'))
print(json.dumps(fields,ensure_ascii=False))
steps=[dict(op='action',name='action_rep'),dict(op='dump',label='recipe-config-default-r2'),dict(op='screenshot',label='recipe-config-default-r2')]
(owner/'recipe-config-r2.json').write_text(json.dumps(steps,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
