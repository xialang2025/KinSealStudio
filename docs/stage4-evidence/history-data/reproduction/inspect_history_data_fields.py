from pathlib import Path
import json
base=Path(__file__).resolve().parent/'worktree/docs/stage4-evidence/history-data/native'
def walk(v):
    if isinstance(v,dict):
        if v.get('visible') and v.get('kind') in ['QComboBox','QSpinBox','QLineEdit','QCheckBox','QTableWidget','QPushButton']:
            print(json.dumps({k:x for k,x in v.items() if k in ['objectName','kind','text','enabled','minimum','maximum','value','currentIndex','currentText','items','checked','rowCount','columnCount','styleSheet','maxLength']},ensure_ascii=False))
        for x in v.values():walk(x)
    elif isinstance(v,list):
        for x in v:walk(x)
for run,labels in [('history-data-default-r2',['history-data-general-r2','history-data-columns-r2']),('history-data-columns-r3',['history-data-three-language-columns-r3'])]:
    d=json.loads((base/f'native-probe-{run}.json').read_text(encoding='utf-8'))
    for s in d['snapshots']:
        if s['label'] in labels:print(s['label']);walk([w for w in s['windows'] if w['title']=='历史数据显示器'])
