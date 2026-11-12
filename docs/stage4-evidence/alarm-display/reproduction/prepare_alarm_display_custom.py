"""Use actual button clicks for native enablement and verify every color plus multilingual caption persistence."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-alarm-display'
s=json.loads((owner/'alarm-display-language-r4.json').read_text(encoding='utf-8'))
for c in s:
    if c['op']=='check': c.pop('value');c['op']='invoke'
    if 'label' in c:c['label']=c['label'].replace('-r4','-r5')
# The last prefix action opens title background color; select and accept it explicitly.
s += [dict(op='color',kind='QColorDialog',value=0xff008000),dict(op='invoke',kind='QPushButton',text='确定',window='选择颜色')]
for name,color in [('m_ttextcolorbtn',0xffff0000),('m_fframcolorbtn',0xff0000ff),('m_fbackcolorbtn',0xffc0c0c0),('m_fontcolorbtn',0xff800080)]:
    s += [dict(op='invoke-queued',kind='QPushButton',name=name),dict(op='color',kind='QColorDialog',value=color),dict(op='invoke',kind='QPushButton',text='确定',window='选择颜色')]
s += [dict(op='dump',label='alarm-colors-language0-r5'),dict(op='combo',kind='QComboBox',name='lanCombox',value=1),dict(op='dump',label='alarm-colors-language1-r5'),dict(op='invoke',kind='QCheckBox',name='m_datecheck'),dict(op='invoke',kind='QCheckBox',name='m_timecheck'),dict(op='dump',label='alarm-columns-disabled-r5'),dict(op='invoke',kind='QPushButton',text='确定'),dict(op='scene-drag',**{'from':[150,120],'to':[470,280]}),dict(op='scene-assert',minSelectableRoots=1),dict(op='save',output='alarm-display-custom.sm'),dict(op='native-compile',label='alarm-display-custom-r5'),dict(op='scene-select',soleRoot=True),dict(op='action',name='action_openpropertiset'),dict(op='dump',label='alarm-custom-reopened-r5'),dict(op='combo',kind='QComboBox',name='lanCombox',value=1),dict(op='dump',label='alarm-custom-English-reopened-r5'),dict(op='invoke',kind='QPushButton',text='取消')]
(owner/'alarm-display-custom-r5.json').write_text(json.dumps(s,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
