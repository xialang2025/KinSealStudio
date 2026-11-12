from pathlib import Path
import json
base=Path(__file__).resolve().parent/'worktree/docs/stage4-evidence/recipe/native';d=json.loads((base/'native-probe-recipe-display-custom-r13.json').read_text(encoding='utf-8'))
out={}
keep=['pbtn_gridColor','ckx_useGridColor','ckx_keyboardCus','checkBox_useKeyPos','spinBox_X','spinBox_Y','checkBox_NoteB','checkBox_NoteW','radioButton_3','radioButton_4','comboBox_NoteW','edit_noticeAddrB','edit_noticeAddrW','comboBox_fontSize','comboBox_align','groupBox_edit']
for snap in d['snapshots']:
    if snap['label'].startswith(('lookup','initial')):continue
    out[snap['label']]=[{k:v for k,v in x['qt'].items() if k in ['objectName','kind','text','checked','enabled','visible','minimum','maximum','currentIndex','value']} for w in snap['windows'] if w['title']=='配方显示器' for x in w['widgets'] if x['qt']['objectName'] in keep]
(base/'observed-display-r13.json').write_text(json.dumps(out,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(json.dumps(out,ensure_ascii=True,indent=2))
