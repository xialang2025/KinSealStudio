"""Read existing graphics evidence without editing it or repeating original-app investigations."""
from pathlib import Path
import hashlib
import json
import sys
root=Path(__file__).resolve().parent;native=root.parents[1]/'KinSealStudio/docs/stage4-evidence/graphics/native'
family=sys.argv[1] if len(sys.argv)>1 else 'line'
p=native/(family+'-observations.json');data=json.loads(p.read_text(encoding='utf-8'))
out={'family':family,'observationSha256':hashlib.sha256(p.read_bytes()).hexdigest(),'runs':[],'fields':{},'sources':[]}
for run in data['runs']:
    observed={'run':run['run'],'status':run.get('status'),'error':run.get('error'),'ownedProcessTerminated':run.get('ownedProcessTerminated'),'input':run.get('input'),'snapshots':[],'scenes':[]}
    for snap in run.get('snapshots',[]):
        observed['snapshots'].append({'label':snap['label'],'titles':[w.get('title') for w in snap.get('windows',[])]})
        for w in snap.get('windows',[]):
            for item in w.get('widgets',[]):
                q=item.get('qt',{});name=q.get('objectName')
                if not name or not q.get('visible') or q.get('kind') not in ['QComboBox','QSpinBox','QDoubleSpinBox','QCheckBox','QPushButton']:continue
                if name=='qt_spinbox_lineedit':continue
                field=out['fields'].setdefault(name,{'kind':q['kind'],'observations':[]})
                state={k:v for k,v in q.items() if k in ['text','currentIndex','currentText','items','minimum','maximum','value','checked','enabled','styleSheet','checkable']}
                if state not in field['observations']:field['observations'].append(state)
    for scene in run.get('sceneReads',[]):
        roots=[{k:v for k,v in item.items() if k not in ['pointer','parent','selected']} for item in scene['data'].get('items',[]) if item.get('parent')=='0x0' and item.get('flags',0)&2]
        observed['scenes'].append({'label':scene.get('label'),'roots':roots})
    out['runs'].append(observed);out['sources'].extend(run.get('outputs',[]))
destination=root/('graphics-'+family+'-preflight.json');destination.write_text(json.dumps(out,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(out,ensure_ascii=True,indent=2))
