from pathlib import Path
import json
root=Path(__file__).resolve().parent;base=root/'worktree/docs/stage4-evidence/recipe/native';d=json.loads((base/'native-probe-recipe-types-csv-r10.json').read_text(encoding='utf-8'))
out={'status':d['status'],'error':d.get('error'),'validators':d.get('validators'),'tableHeaders':d.get('tableHeaders'),'bounds':[],'saveAsDialogs':d.get('saveAsDialogs')}
for snap in d['snapshots']:
    if not snap['label'].startswith('recipe-type-'):continue
    fields=[x['qt'] for w in snap['windows'] if w['title']=='配方' for x in w['widgets'] if x['qt']['objectName'] in ['nRecipeLen','sCurrRecAddrRange','sRecMemAddrRange']]
    out['bounds'].append(dict(label=snap['label'],fields=[{k:v for k,v in q.items() if k in ['objectName','text','minimum','maximum','value']} for q in fields]))
(base/'observed-types-r10.json').write_text(json.dumps(out,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps({k:v for k,v in out.items() if k!='saveAsDialogs'},ensure_ascii=True,indent=2))
print(json.dumps([dict(destination=x['destination'],prefills=x['prefills'],postState=x.get('postState')) for x in out.get('saveAsDialogs',[])],ensure_ascii=True,indent=2))
for folder in [root/'native-recipe',base]:
    for p in folder.rglob('*'):
        if p.is_file() and (p.suffix.lower() in ['.csv','.cvs'] or 'export-r10' in p.name): print(str(p),p.stat().st_size)
