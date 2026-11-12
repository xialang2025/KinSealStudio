from pathlib import Path
import json
base=Path(__file__).resolve().parent/'worktree/docs/stage4-evidence/history-data/native'
def walk(v,out):
    if isinstance(v,dict):
        if v.get('visible') and (v.get('objectName') in ['m_dateedit','m_timeedit','lanCombox','m_tfontsizecmb','m_rowspin','m_fontcolorbtn','m_linecolorbtn','BG_Color','Text_Color','FG_Color','Frame_Color'] or v.get('kind')=='QMessageBox'):
            out.append({k:x for k,x in v.items() if k in ['kind','objectName','text','enabled','value','currentText','styleSheet']})
        for x in v.values():walk(x,out)
    elif isinstance(v,list):
        for x in v:walk(x,out)
for run in ['history-data-colors-r6','history-data-empty-r7','history-data-zero-r8']:
    d=json.loads((base/f'native-probe-{run}.json').read_text(encoding='utf-8'));print(run,d['status'],d.get('error'))
    for t in d.get('nativeTableStates',[]):print(t['label'],json.dumps(t['state']['cells'],ensure_ascii=False))
    for s in d['snapshots']:
        if s['label'].startswith(('history-data-colors-lang','history-data-shrunk','history-data-all-hidden-result','history-data-zero-rows-reopened')):
            out=[];walk(s['windows'],out);print(s['label'],json.dumps(out,ensure_ascii=False))
