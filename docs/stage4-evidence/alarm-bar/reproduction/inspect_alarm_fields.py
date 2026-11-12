from pathlib import Path
import json
base=Path(__file__).resolve().parent/'worktree/docs/stage4-evidence/alarm-bar/native'
def walk(v):
    if isinstance(v,dict):
        if v.get('visible') and v.get('kind') in ['QComboBox','QSpinBox','QLineEdit','QRadioButton','QCheckBox','QTableWidget']:
            row={k:x for k,x in v.items() if k in ['kind','objectName','text','enabled','minimum','maximum','value','currentText','currentIndex','count','rowCount','columnCount','checked','readOnly']}
            if 'items' in v: row['items']=v['items'] if len(v['items'])<20 else [v['items'][0],v['items'][-1]]
            print(json.dumps(row,ensure_ascii=False))
        for x in v.values():walk(x)
    elif isinstance(v,list):
        for x in v:walk(x)
for run,labels in [('alarm-lifetime-r6',['condition-row1-r6','condition-row4-r6','condition-row0-r6','condition-reopened-r6'])]:
    d=json.loads((base/f'native-probe-{run}.json').read_text(encoding='utf-8'))
    for x in d['snapshots']:
        if x['label'] in labels:
            print(x['label']);walk([w for w in x['windows'] if w['title'] in ['动态报警条','数位地址报警','条件报警']])
