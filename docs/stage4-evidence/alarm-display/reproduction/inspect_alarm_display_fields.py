from pathlib import Path
import json
p=Path(__file__).resolve().parent/'worktree/docs/stage4-evidence/alarm-display/native/native-probe-alarm-display-default-r2.json'
d=json.loads(p.read_text(encoding='utf-8'))
def walk(v):
    if isinstance(v,dict):
        if v.get('visible') and (v.get('kind') in ['QComboBox','QSpinBox','QLineEdit','QCheckBox'] or v.get('objectName') in ['m_tbackcolorbtn','m_ttextcolorbtn','m_fframcolorbtn','m_fbackcolorbtn','m_fontcolorbtn']):
            print(json.dumps({k:x for k,x in v.items() if k in ['objectName','text','kind','enabled','minimum','maximum','value','currentIndex','currentText','items','styleSheet','checked','maxLength']},ensure_ascii=False))
        for x in v.values():walk(x)
    elif isinstance(v,list):
        for x in v:walk(x)
for s in d['snapshots']:
    if s['label']=='alarm-display-basic-r2':walk([w for w in s['windows'] if w['title']=='报警显示器'])
