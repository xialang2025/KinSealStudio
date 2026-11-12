from pathlib import Path
import json
base=Path(__file__).resolve().parent/'worktree/docs/stage4-evidence/history-data/native'
d=json.loads((base/'native-probe-history-data-survey-r5.json').read_text(encoding='utf-8'))
def walk(v,out):
    if isinstance(v,dict):
        if v.get('visible') and v.get('objectName') in ['lanCombox','m_rowspin','m_tfontsizecmb','m_dateedit','m_timeedit','m_datecheck','m_timecheck','m_verticalcheck','m_horizontalcheck']:
            out[v['objectName']]={k:x for k,x in v.items() if k in ['text','enabled','value','currentIndex','currentText','checked']}
        for x in v.values():walk(x,out)
    elif isinstance(v,list):
        for x in v:walk(x,out)
records=[]
for s in d['snapshots']:
    if s['label'] in ['history-data-general-lang1-before-r5','history-data-general-lang2-before-r5','history-data-general-lang0-returned-r5','history-data-general-after-column-language-r5','history-data-custom-general-reopened-r5']:
        out={};walk([w for w in s['windows'] if w['title']=='历史数据显示器'],out);records.append(dict(label=s['label'],fields=out));print(s['label'],json.dumps(out,ensure_ascii=False))
tables=[]
for t in d.get('nativeTableStates',[]):
    state=t['state'];row=dict(label=t['label'],names=[c[0]['text'] for c in state['cells']],shown=[c[1]['checkState'] for c in state['cells']]);tables.append(row);print(json.dumps(row,ensure_ascii=False))
(base/'survey-fields-r5.json').write_text(json.dumps(dict(status=d['status'],general=records,tables=tables),ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
