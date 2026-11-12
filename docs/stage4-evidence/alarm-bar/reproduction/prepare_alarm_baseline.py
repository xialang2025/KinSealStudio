"""Measure registry sizes/word modes, create two real native bit alarms and compile a unique C27 control baseline."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-alarm-bar';p=owner/'alarm_probe.py';s=p.read_text(encoding='utf-8')
js="""if(p.op==='table-state'){const w=find(ptr(p.hwnd)),rows=Number(prop(w,'rowCount')),cols=Number(prop(w,'columnCount')),out=[];for(let r=0;r<Math.min(rows,128);++r){let row=[];for(let c=0;c<Math.min(cols,12);++c){const item=tableItem(w,r,c);if(item.isNull()){row.push(null);continue;}let values=[];for(const role of [0,10]){const v=Memory.alloc(32);tableData(v,item,role);const st=Memory.alloc(4);vs(st,v);values.push(string(st));sd(st);vd(v);}row.push({text:values[0],checkState:values[1]});}out.push(row);}return {rows,columns:cols,cells:out};} """
assert "if(p.op==='table-state')" not in s;s=s.replace("function execute(p){",'function execute(p){'+js,1)
marker="            elif name=='tree-list':"
assert s.count(marker)==1
s=s.replace(marker,"""            elif name=='table-state':
                w=widget(command)
                result.setdefault('nativeTableStates',[]).append({'label':command['label'],'table':w['qt'],'state':op({'op':'table-state','hwnd':w['hwnd']})})
"""+marker)
p.write_text(s,encoding='utf-8',newline='\n')
scenario=[dict(op='action',name='action_checkall'),dict(op='action',name='action_delete'),dict(op='scene-assert',minSelectableRoots=0),dict(op='tree-double',entry='条件报警'),dict(op='combo',kind='QComboBox',name='nReadLen',value=2),dict(op='table-state',kind='QTableWidget',name='CondAlarmTable',label='condition-two-registers-r3'),dict(op='table-click',kind='QTableWidget',name='CondAlarmTable',row=0,column=1),dict(op='dump',label='condition-row0-r3'),dict(op='screenshot',label='condition-two-registers-r3'),dict(op='combo',kind='QComboBox',name='nReadLen',value=31),dict(op='dump',label='condition-size31-r3'),dict(op='invoke',kind='QPushButton',text='取消'),dict(op='tree-double',entry='数位地址报警')]
for mode in range(3):
 scenario += [dict(op='combo',kind='QComboBox',name='nAlarmType',value=mode),dict(op='combo',kind='QComboBox',name='nReadLen',value=511),dict(op='dump',label=f'bit-mode-{mode}-max-r3'),dict(op='combo',kind='QComboBox',name='nReadLen',value=2),dict(op='table-state',kind='QTableWidget',name='BitAlarmTable',label=f'bit-mode-{mode}-two-r3')]
scenario += [dict(op='combo',kind='QComboBox',name='nAlarmType',value=0),dict(op='combo',kind='QComboBox',name='nReadLen',value=2)]
for row,message in [(0,'ALM1'),(1,'ALM2')]:scenario += [dict(op='table-click',kind='QTableWidget',name='BitAlarmTable',row=row,column=2),dict(op='text-edit',kind='QLineEdit',name='sMessage',value=message)]
scenario += [dict(op='dump',label='bit-registered-messages-r3'),dict(op='table-state',kind='QTableWidget',name='BitAlarmTable',label='bit-registered-messages-r3'),dict(op='invoke',kind='QPushButton',text='确定'),dict(op='save',output='alarm-bit-registration.sm'),dict(op='action',name='action_dynamicarlarm'),dict(op='dump',label='alarm-basic-r3'),dict(op='screenshot',label='alarm-basic-r3'),dict(op='tab',kind='QTabWidget',name='tabWidget',value=1),dict(op='dump',label='alarm-access-r3'),dict(op='tab',kind='QTabWidget',name='tabWidget',value=0),dict(op='invoke',kind='QPushButton',text='确定'),dict(op='scene-drag',**{'from':[160,120],'to':[480,180]}),dict(op='scene-assert',minSelectableRoots=1),dict(op='save',output='alarm-default.sm'),dict(op='native-compile',label='alarm-default-r3'),dict(op='scene-select',soleRoot=True),dict(op='action',name='action_openpropertiset'),dict(op='dump',label='alarm-reopened-r3'),dict(op='invoke',kind='QPushButton',text='取消')]
(owner/'alarm-default-r3.json').write_text(json.dumps(scenario,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
