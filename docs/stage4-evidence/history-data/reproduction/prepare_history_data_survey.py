"""Measure C30 column-language synchronization using all three project languages."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-history-data';p=owner/'history_data_probe.py';s=p.read_text(encoding='utf-8')
marker='function execute(p){';extra="if(p.op==='combo-activate'){const w=find(ptr(p.hwnd));comboSetFn(w,p.value);const signal=new NativeFunction(gui.getExportByName('_ZN9QComboBox9activatedEi'),'void',['pointer','int']);signal(w,p.value);return props(w);}"
assert extra not in s;s=s.replace(marker,marker+extra,1).replace("'combo','combo-index'","'combo','combo-activate','combo-index'")
s=s.replace('tableColumnXFn(w,p.column)+Math.floor(tableColumnWidthFn(w,p.column)/2)','tableColumnXFn(w,p.column)+(p.checkbox?8:Math.floor(tableColumnWidthFn(w,p.column)/2))')
before="{'op':'table-click-point','hwnd':w['hwnd'],'row':command['row'],'column':command.get('column',1)}";assert s.count(before)==1;s=s.replace(before,"{'op':'table-click-point','hwnd':w['hwnd'],'row':command['row'],'column':command.get('column',1),'checkbox':command.get('checkbox',False)}")
p.write_text(s,encoding='utf-8',newline='\n')
scenario=[dict(op='action',name='action_historyrecord')]
for language in range(3):
    scenario += [dict(op='combo-activate',kind='QComboBox',name='lanCombox',value=language),dict(op='dump',label=f'history-data-general-lang{language}-before-r4'),dict(op='text-edit',kind='QLineEdit',name='m_dateedit',value=f'D{language}'),dict(op='text-edit',kind='QLineEdit',name='m_timeedit',value=f'T{language}')]
    if language==0:scenario += [dict(op='spin',kind='QSpinBox',name='m_rowspin',value=9),dict(op='combo-activate',kind='QComboBox',name='m_tfontsizecmb',value=2),dict(op='invoke',kind='QCheckBox',name='m_datecheck'),dict(op='invoke',kind='QCheckBox',name='m_timecheck'),dict(op='invoke',kind='QCheckBox',name='m_verticalcheck'),dict(op='invoke',kind='QCheckBox',name='m_horizontalcheck')]
scenario += [dict(op='combo-activate',kind='QComboBox',name='lanCombox',value=0),dict(op='dump',label='history-data-general-lang0-returned-r4'),dict(op='screenshot',label='history-data-hidden-date-fields-r4'),dict(op='tab',kind='QTabWidget',name='tabWidget',value=1),dict(op='dump',label='history-data-columns-lang-before-r4')]
for language,prefix in [(0,'C'),(1,'E'),(2,'G')]:
    scenario += [dict(op='combo-activate',kind='QComboBox',name='lanCombox',value=language)]
    for row in range(3):scenario += [dict(op='table-click',kind='QTableWidget',name='HisDataTable',row=row,column=0),dict(op='text-edit',kind='QLineEdit',name='sDataNameEdit',value=f'{prefix}{row}')]
    if language==0:scenario += [dict(op='table-click',kind='QTableWidget',name='HisDataTable',row=1,column=1,checkbox=True)]
    scenario += [dict(op='table-state',kind='QTableWidget',name='HisDataTable',label=f'history-data-columns-lang{language}-edited-r4')]
scenario += [dict(op='combo-activate',kind='QComboBox',name='lanCombox',value=1),dict(op='table-click',kind='QTableWidget',name='HisDataTable',row=2,column=0),dict(op='invoke',kind='QPushButton',name='pushButton_synCLan'),dict(op='table-state',kind='QTableWidget',name='HisDataTable',label='history-data-current-sync-lang1-r4'),dict(op='combo-activate',kind='QComboBox',name='lanCombox',value=2),dict(op='table-state',kind='QTableWidget',name='HisDataTable',label='history-data-after-current-sync-lang2-r4'),dict(op='invoke',kind='QPushButton',name='pushButton_synALan'),dict(op='table-state',kind='QTableWidget',name='HisDataTable',label='history-data-all-sync-lang2-r4'),dict(op='combo-activate',kind='QComboBox',name='lanCombox',value=1),dict(op='table-state',kind='QTableWidget',name='HisDataTable',label='history-data-all-sync-lang1-r4'),dict(op='tab',kind='QTabWidget',name='tabWidget',value=0),dict(op='dump',label='history-data-general-after-column-language-r4'),dict(op='invoke',kind='QPushButton',name='accept'),dict(op='scene-drag',**{'from':[150,100],'to':[470,280]}),dict(op='scene-assert',minSelectableRoots=1),dict(op='save',output='history-data-custom.sm'),dict(op='native-compile',label='history-data-custom-r4'),dict(op='scene-select',soleRoot=True),dict(op='action',name='action_openpropertiset'),dict(op='dump',label='history-data-custom-general-reopened-r4'),dict(op='tab',kind='QTabWidget',name='tabWidget',value=1)]
for language in range(3):scenario += [dict(op='combo-activate',kind='QComboBox',name='lanCombox',value=language),dict(op='table-state',kind='QTableWidget',name='HisDataTable',label=f'history-data-custom-columns-lang{language}-reopened-r4')]
scenario += [dict(op='invoke',kind='QPushButton',name='cancel')]
for c in scenario:
    if 'label' in c:c['label']=c['label'].replace('-r4','-r5')
(owner/'history-data-survey-r5.json').write_text(json.dumps(scenario,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
