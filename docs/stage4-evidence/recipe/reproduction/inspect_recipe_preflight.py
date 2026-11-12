from pathlib import Path
import json
import sys
base=Path(__file__).resolve().parent/'worktree/docs/stage4-evidence/recipe/native'
run=sys.argv[1] if len(sys.argv)>1 else 'recipe-config-r2'
d=json.loads((base/('native-probe-'+run+'.json')).read_text(encoding='utf-8'))
for snap in d['snapshots']:
    if snap['label'].startswith(('lookup','initial')):continue
    print(snap['label'])
    for w in snap['windows']:
        if w['title'].startswith('KinSealStudio--'):continue
        for x in w['widgets']:
            q=x['qt']
            if q['kind'] in ('QLineEdit','QSpinBox','QDoubleSpinBox','QCheckBox','QGroupBox','QRadioButton','QComboBox','QTabWidget','QTableWidget','QPushButton'):
                print(json.dumps({k:v for k,v in q.items() if k in ['objectName','kind','text','visible','enabled','value','minimum','maximum','items','currentIndex','currentText','checked','rowCount','columnCount','styleSheet']},ensure_ascii=True))
