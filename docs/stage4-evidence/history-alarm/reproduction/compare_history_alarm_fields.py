"""Compare observed default interactive fields, excluding process identities and dialog title."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent/'worktree/docs/stage4-evidence'
def fields(value,out):
    if isinstance(value,dict):
        if value.get('visible') and value.get('kind') in ['QComboBox','QSpinBox','QLineEdit','QCheckBox','QPushButton']:
            name=value.get('objectName','')
            if name and name!='qt_spinbox_lineedit':out[name]={k:v for k,v in value.items() if k in ['kind','objectName','text','enabled','minimum','maximum','value','currentIndex','currentText','items','checked','maxLength','styleSheet']}
        for v in value.values():fields(v,out)
    elif isinstance(value,list):
        for v in value:fields(v,out)
allfields=[]
for family,run,title in [('alarm-display','alarm-display-default-r2','报警显示器'),('history-alarm','history-alarm-default-r2','历史报警显示器')]:
    d=json.loads((root/family/'native'/f'native-probe-{run}.json').read_text(encoding='utf-8'));out={}
    snapshot=next(s for s in d['snapshots'] if 'basic' in s['label']);fields([w for w in snapshot['windows'] if w['title']==title],out);allfields.append(out)
diff={k:[allfields[0].get(k),allfields[1].get(k)] for k in allfields[0].keys()|allfields[1].keys() if allfields[0].get(k)!=allfields[1].get(k)}
report=dict(basicInteractiveFieldCounts=list(map(len,allfields)),differences=diff,currentAndHistoryRuntimeSourcesEquivalentClaimed=False)
print(json.dumps(report,ensure_ascii=False,indent=2))
(root/'history-alarm/native/default-ui-comparison.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
