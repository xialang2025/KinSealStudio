"""Retain compact observed source-mode fields without changing original native capture bytes."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;base=root/'worktree/docs/stage4-evidence/data-group/native';data=json.loads((base/'native-probe-group-survey-r2.json').read_text(encoding='utf-8'))
names={'m_SampleAddrLineEdit','m_SampleNum_SpinBox','comboBox_Sample','comboBox_dataType','m_LimitSourcecbBox','Y_Min','Y_Max','Y_Max_lineEdit','Y_TotalDigits','Y_Fractional','Y_Font_Size','X_Font_Size','CB_lineStyle'}
def fields(v):
 out=[]
 if isinstance(v,dict):
  if v.get('objectName') in names:out.append({k:value for k,value in v.items() if k in ['objectName','kind','text','visible','enabled','value','currentIndex','currentText']})
  for value in v.values():out+=fields(value)
 elif isinstance(v,list):
  for value in v:out+=fields(value)
 return out
rows=[dict(label=s['label'],fields=fields(s['windows'])) for s in data['snapshots'] if s['label'] in ['group-address-sampling-r2','group-range-address-restored-r2','group-constant-sampling-restored-r2','Y_If_Show_Marks-off-r2','group-reopened-r2']]
(base/'observed-fields.json').write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
for row in rows:
 print(row['label'],json.dumps([v for v in row['fields'] if v['objectName'] in {'m_SampleAddrLineEdit','m_SampleNum_SpinBox','Y_Min','Y_Max','Y_Max_lineEdit','m_LimitSourcecbBox'}],ensure_ascii=False))
p=base/'native-manifest.json';d=json.loads(p.read_text(encoding='utf-8'));d['date']='2026-11-13';d['archiveDate']='2026-11-13';p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
