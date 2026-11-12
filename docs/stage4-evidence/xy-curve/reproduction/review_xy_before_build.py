"""Apply two bounded authoring corrections before the first C25 build, then retain a small observed-field ledger."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;work=root/'worktree'
p=work/'tests/stage4_controls_tests.cpp';s=p.read_text(encoding='utf-8');i=s.index('static HmiObject xyObject()');s=s[:i]+s[i:].replace('.trend','.xyCurve');p.write_text(s,encoding='utf-8',newline='\n')
data=json.loads((work/'docs/stage4-evidence/xy-curve/native/native-probe-xy-survey-r2.json').read_text(encoding='utf-8'))
names={'LE_Min','LE_Max','CB_lineStyle','X_Min','X_Max','Y_Min','Y_Max','X_TotalDigits','X_Fractional','X_Grid_Color','X_Font_Size','Y_Font_Size'}
def fields(v):
 out=[]
 if isinstance(v,dict):
  if v.get('objectName') in names:out.append({k:value for k,value in v.items() if k in ['objectName','kind','text','visible','currentIndex','currentText','value']})
  for value in v.values():out+=fields(value)
 elif isinstance(v,list):
  for value in v:out+=fields(value)
 return out
rows=[dict(label=x['label'],fields=fields(x['windows'])) for x in data['snapshots'] if x['label'] in ['xy-channel3-restored-r2','xy-channel1-r2','xy-axis-custom-r2','X_If_Show_Marks-off-r2','X_Show_Y_Axis-off-r2','xy-reopened-r2']]
(work/'docs/stage4-evidence/xy-curve/native/observed-fields.json').write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
for row in rows:
 print(row['label'],json.dumps([x for x in row['fields'] if x['objectName'] in {'LE_Min','LE_Max','X_Min','X_Max','X_Grid_Color'}],ensure_ascii=False))
