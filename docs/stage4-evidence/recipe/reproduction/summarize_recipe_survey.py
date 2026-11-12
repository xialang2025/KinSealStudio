from pathlib import Path
import json
root=Path(__file__).resolve().parent;base=root/'worktree/docs/stage4-evidence/recipe/native'
d=json.loads((base/'native-probe-recipe-survey-r7.json').read_text(encoding='utf-8'))
summary={key:d.get(key) for key in ['tableHeaders','tableStates','validators']}
summary['snapshots']=[]
for snap in d['snapshots']:
    if snap['label'] not in ['recipe-config-cell-r7','recipe-config-english-r7','recipe-custom-keyboard-empty-r7','recipe-custom-keyboard-rejection-r7']:continue
    windows=[]
    for w in snap['windows']:
        widgets=[]
        for x in w['widgets']:
            q=x['qt']
            if q.get('visible') and (q.get('objectName') in ['sRowHeadName','sColumnHeadName','sRecipeData','groupBox_edit','ckx_keyboardCus','cbx_keyboardCus','checkBox_useKeyPos','spinBox_X','spinBox_Y','spinBox_rowCount','spinBox_columnCount','checkBox_ColumnWidth','ckx_useTitle'] or q['kind']=='QMessageBox'):
                widgets.append(q)
        if widgets:windows.append(dict(title=w['title'],widgets=widgets))
    summary['snapshots'].append(dict(label=snap['label'],windows=windows))
(base/'observed-survey-r7.json').write_text(json.dumps(summary,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(summary,ensure_ascii=True,indent=2))
