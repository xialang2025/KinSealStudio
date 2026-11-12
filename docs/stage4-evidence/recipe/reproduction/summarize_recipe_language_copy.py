from pathlib import Path
import json
base=Path(__file__).resolve().parent/'worktree/docs/stage4-evidence/recipe/native'
d=json.loads((base/'native-probe-recipe-language-copy-r8.json').read_text(encoding='utf-8'))
out={k:d.get(k) for k in ['tableHeaders','tableStates','tables','validators']}
out['fields']={}
for snap in d['snapshots']:
    if snap['label'].startswith(('lookup','initial')):continue
    fields=[]
    for w in snap['windows']:
        if w['title']!='配方':continue
        for x in w['widgets']:
            q=x['qt']
            if q['objectName'] in ['nRecipeLen','sCurrRecAddrRange','sRecMemAddrRange','sRRecipeAddr','sWRecipeAddr','bRaddrSameWaddr','bRRecipeFromPlc','bWRecipeToPlc']:
                fields.append({k:v for k,v in q.items() if k in ['objectName','minimum','maximum','value','text','enabled','checked']})
    out['fields'][snap['label']]=fields
(base/'observed-language-copy-r8.json').write_text(json.dumps(out,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(json.dumps(out,ensure_ascii=True,indent=2))
