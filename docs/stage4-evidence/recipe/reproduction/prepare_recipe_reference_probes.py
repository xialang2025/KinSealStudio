"""Check multiple definitions and bring the off-viewport original tree item into view without moving user windows."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-recipe';p=owner/'recipe_probe.py';s=p.read_text(encoding='utf-8')
s=s.replace('function execute(p){',"""function execute(p){if(p.op==='tree-scroll-item'){const scroll=new NativeFunction(gui.getExportByName('_ZN11QTreeWidget12scrollToItemEPK15QTreeWidgetItemN17QAbstractItemView10ScrollHintE'),'void',['pointer','pointer','int']);scroll(find(ptr(p.hwnd)),ptr(p.item),0);return true;}""",1)
before="            entry=next(x for x in tree_rows if x['text']==command['entry'])"
after=before+"\n            if command.get('scroll'):\n                op({'op':'tree-scroll-item','hwnd':tree,'item':entry['pointer']})\n                time.sleep(.2)\n                tree_rows=op({'op':'tree','hwnd':tree})\n                entry=next(x for x in tree_rows if x['text']==command['entry'])"
assert s.count(before)==1;s=s.replace(before,after);p.write_text(s,encoding='utf-8',newline='\n')
steps=[dict(op='action',name='action_rep'),dict(op='text',kind='QLineEdit',name='sRecipeName',value='C31Second'),dict(op='dump',label='recipe-second-definition-r15'),dict(op='invoke',kind='QPushButton',text='确定'),dict(op='assert-main-only'),dict(op='save',output='recipe-two-definitions.sm'),dict(op='action',name='action_recipetable'),dict(op='dump',label='recipe-two-definition-combo-r15'),dict(op='invoke',kind='QPushButton',text='取消')]
(owner/'recipe-multiple-r15.json').write_text(json.dumps(steps,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
steps=[dict(op='tree-double',entry='C31Multi',scroll=True),dict(op='dump',label='recipe-custom-config-reopened-r16'),dict(op='tab',kind='QTabWidget',name='tabWidget',value=1),dict(op='table-headers',kind='QTableWidget',name='RecipeDataTable',label='recipe-custom-config-reopened-r16'),dict(op='table-state',kind='QTableWidget',name='RecipeDataTable',label='recipe-custom-config-reopened-r16'),dict(op='invoke',kind='QPushButton',text='取消')]
(owner/'recipe-reopen-r16.json').write_text(json.dumps(steps,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
