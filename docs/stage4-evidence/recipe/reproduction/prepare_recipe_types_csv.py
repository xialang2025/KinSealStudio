from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-recipe';p=owner/'recipe_probe.py';s=p.read_text(encoding='utf-8')
s=s.replace('        def save_as(output):','        def save_as(output, export_button=None):')
s=s.replace("            action('action_save_as')\n            deadline = time.time() + 7", "            if export_button is None:\n                action('action_save_as')\n            else:\n                button = widget({'kind':'QPushButton','name':export_button})\n                op({'op':'invoke-queued','hwnd':button['hwnd'],'slot':'click'})\n            deadline = time.time() + 7")
s=s.replace("            elif name=='save-as':", "            elif name=='export-csv':\n                save_as(command['output'], command.get('button','ExportButton'))\n            elif name=='save-as':")
p.write_text(s,encoding='utf-8',newline='\n')
steps=[dict(op='action',name='action_rep'),dict(op='text',kind='QLineEdit',name='sRecipeName',value='C31Types')]
for value in [0,1,2,3,4,0]:
    label='recipe-type-'+str(value)+('-initial' if len(steps)==2 else '-switched')+'-r10'
    if len(steps)>2:steps.append(dict(op='combo-activate',kind='QComboBox',name='nDataType',value=value))
    steps.extend([dict(op='dump',label=label),dict(op='tab',kind='QTabWidget',name='tabWidget',value=1),dict(op='table-click',kind='QTableWidget',name='RecipeDataTable',row=0,column=0),dict(op='validator',kind='QLineEdit',name='sRecipeData',label=label),dict(op='table-headers',kind='QTableWidget',name='RecipeDataTable',label=label),dict(op='tab',kind='QTabWidget',name='tabWidget',value=0)])
steps.extend([dict(op='tab',kind='QTabWidget',name='tabWidget',value=1),dict(op='table-click',kind='QTableWidget',name='RecipeDataTable',row=0,column=0),dict(op='text-edit',kind='QLineEdit',name='sRecipeData',value='123'),dict(op='text-edit',kind='QLineEdit',name='sColumnHeadName',value='Item,"0"'),dict(op='text-edit',kind='QLineEdit',name='sRowHeadName',value='Recipe,0'),dict(op='tab',kind='QTabWidget',name='tabWidget',value=0),dict(op='export-csv',output='recipe-native-export-r10.csv'),dict(op='dump',label='recipe-after-export-r10'),dict(op='invoke',kind='QPushButton',text='取消')])
(owner/'recipe-types-csv-r10.json').write_text(json.dumps(steps,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
steps=[dict(op='tree-context',entry='C31One'),dict(op='dump',label='recipe-tree-context-r11')]
(owner/'recipe-tree-context-r11.json').write_text(json.dumps(steps,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
