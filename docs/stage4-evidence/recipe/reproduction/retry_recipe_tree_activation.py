"""Activate an off-viewport recipe item through its public Qt double-click signal, once."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-recipe';p=owner/'recipe_probe.py';s=p.read_text(encoding='utf-8')
s=s.replace('function execute(p){',"""function execute(p){if(p.op==='tree-double-signal'){const item=Memory.alloc(4),column=Memory.alloc(4);item.writePointer(ptr(p.item));column.writeInt(0);return {queuedMetaInvoke:invokeFn(find(ptr(p.hwnd)),Memory.allocUtf8String('itemDoubleClicked'),2,ptr(0),ptr(0),Memory.allocUtf8String('QTreeWidgetItem*'),item,Memory.allocUtf8String('int'),column,...Array(16).fill(ptr(0)))};}""",1)
before="            viewport=next(h for h in children if win32gui.GetParent(h)==tree and win32gui.GetWindowText(h)=='qt_scrollarea_viewport')"
after="            if command['op']=='tree-activate':\n                result['steps'].append({'request':command,'row':entry,'result':op({'op':'tree-double-signal','hwnd':tree,'item':entry['pointer']}),'source':'owned private QTreeWidget public itemDoubleClicked signal; no foreground or system input'})\n                time.sleep(.5)\n                return\n"+before
assert s.count(before)==1;s=s.replace(before,after).replace("elif name in ('tree-single','tree-double','tree-context'):","elif name in ('tree-single','tree-double','tree-context','tree-activate'):")
p.write_text(s,encoding='utf-8',newline='\n')
steps=json.loads((owner/'recipe-survey-r5.json').read_text(encoding='utf-8'));steps[0]['op']='tree-activate'
for step in steps:
    for key in ('label','output'):
        if key in step:step[key]=step[key].replace('-r5','-r6')
(owner/'recipe-survey-r6.json').write_text(json.dumps(steps,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
