"""Begin C27 with the actual missing-registration gate and a read-only inventory of the native settings tree."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent;owner=root/'native-alarm-bar';owner.mkdir(exist_ok=True);(owner/'temp').mkdir(exist_ok=True)
(root/'worktree/docs/stage4-evidence/alarm-bar/native').mkdir(parents=True,exist_ok=True)
s=(root/'native-data-group/group_probe.py').read_text(encoding='utf-8').replace('stage4-evidence/data-group/native','stage4-evidence/alarm-bar/native').replace('native-data-group','native-alarm-bar').replace('KinSealStage4DataGroup20261113','KinSealStage4AlarmBar20261113').replace('数据群组','动态报警条')
marker="            elif name in ('tree-single','tree-double','tree-context'):"
assert s.count(marker)==1
insert="""            elif name=='tree-list':
                candidates=op({'op':'batch','hwnds':children})
                trees=[w for w in candidates if w.get('qt',{}).get('kind')=='QTreeWidget']
                for tree in trees:
                    op({'op':'invoke','hwnd':tree['hwnd'],'slot':'expandAll'})
                    result.setdefault('nativeTreeItems',[]).append({'label':command['label'],'tree':tree['qt'],'items':op({'op':'tree','hwnd':tree['hwnd']})})
"""
s=s.replace(marker,insert+marker)
(owner/'alarm_probe.py').write_text(s,encoding='utf-8',newline='\n')
scenario=[dict(op='action',name='action_checkall'),dict(op='action',name='action_delete'),dict(op='scene-assert',minSelectableRoots=0),dict(op='action',name='action_dynamicarlarm'),dict(op='dump',label='alarm-missing-registration-r1'),dict(op='screenshot',label='alarm-missing-registration-r1'),dict(op='invoke',kind='QPushButton',text='确定'),dict(op='tree-list',label='alarm-system-tree-r1')]
(owner/'alarm-gate-r1.json').write_text(json.dumps(scenario,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
