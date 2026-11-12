"""Measure actual language ownership and inspect the original color-dialog controls."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-alarm-display';probe=owner/'alarm_display_probe.py'
s=probe.read_text(encoding='utf-8');marker='function execute(p){';addition="if(p.op==='invoke-queued'){return {queuedMetaInvoke:invokeFn(find(ptr(p.hwnd)),Memory.allocUtf8String(p.slot||'click'),2,...Array(22).fill(ptr(0)))};}"
assert addition not in s;s=s.replace(marker,marker+addition,1).replace("elif name=='invoke':", "elif name in ('invoke','invoke-queued'):").replace("op({'op':'invoke','hwnd':w['hwnd'],'slot':command.get('slot','click')})", "op({'op':name,'hwnd':w['hwnd'],'slot':command.get('slot','click')})")
probe.write_text(s,encoding='utf-8',newline='\n')
registration=json.loads((root/'native-alarm-bar/alarm-default-r4.json').read_text(encoding='utf-8'))
end=next(i for i,c in enumerate(registration) if c.get('op')=='save')
scenario=registration[:end]+[dict(op='action',name='action_arlamcontrol'),dict(op='dump',label='alarm-bilingual-default-r3')]
for name,value in [('m_msgedit','ZH Message'),('m_dateedit','ZH Date'),('m_timeedit','ZH Time')]:scenario += [dict(op='text-edit',kind='QLineEdit',name=name,value=value)]
scenario += [dict(op='combo',kind='QComboBox',name='fontSize',value=3),dict(op='spin',kind='QSpinBox',name='m_rowspin',value=20),dict(op='check',kind='QCheckBox',name='m_datecheck',value=False),dict(op='dump',label='alarm-language0-edited-r3'),dict(op='combo',kind='QComboBox',name='lanCombox',value=1),dict(op='dump',label='alarm-language1-before-edit-r3')]
for name,value in [('m_msgedit','EN Message'),('m_dateedit','EN Date'),('m_timeedit','EN Time')]:scenario += [dict(op='text-edit',kind='QLineEdit',name=name,value=value)]
scenario += [dict(op='combo',kind='QComboBox',name='fontSize',value=2),dict(op='combo',kind='QComboBox',name='lanCombox',value=0),dict(op='dump',label='alarm-language0-returned-r3'),dict(op='invoke-queued',kind='QPushButton',name='m_tbackcolorbtn'),dict(op='dump',label='alarm-title-color-dialog-r3')]
(owner/'alarm-display-language-r3.json').write_text(json.dumps(scenario,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
