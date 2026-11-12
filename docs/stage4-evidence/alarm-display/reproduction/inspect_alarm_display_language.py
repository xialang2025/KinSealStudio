from pathlib import Path
import json
p=Path(__file__).resolve().parent/'worktree/docs/stage4-evidence/alarm-display/native/native-probe-alarm-display-custom-r5.json'
d=json.loads(p.read_text(encoding='utf-8'))
names=['m_dateedit','m_timeedit','m_msgedit','fontSize','m_rowspin','m_datecheck','m_timecheck','m_tbackcolorbtn','m_ttextcolorbtn','m_fframcolorbtn','m_fbackcolorbtn','m_fontcolorbtn']
def walk(v,color=False):
    if isinstance(v,dict):
        if v.get('visible') and (v.get('objectName') in names or color and v.get('kind') in ['QPushButton','QColorDialog']):
            print(json.dumps({k:x for k,x in v.items() if k in ['kind','objectName','text','enabled','value','currentText','currentColor','checked','styleSheet']},ensure_ascii=False))
        for x in v.values():walk(x,color)
    elif isinstance(v,list):
        for x in v:walk(x,color)
for s in d['snapshots']:
    if s['label'] in ['alarm-colors-language1-r5','alarm-custom-reopened-r5','alarm-custom-English-reopened-r5']:
        print(s['label']);walk([w for w in s['windows'] if w['title'] in ['报警显示器','选择颜色']],s['label']=='alarm-title-color-dialog-r4')
