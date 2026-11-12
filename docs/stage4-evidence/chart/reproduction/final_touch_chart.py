"""Record all three hardware baselines and the unique C22 modification marker before committing."""
from pathlib import Path
import json
import shutil
root=Path(__file__).resolve().parent
work=root/'worktree'
p=work/'程序修改记录.md'
s=p.read_text(encoding='utf-8')
a=s.rfind('：阶段4图表独立编辑与事务切片')
assert a>0
s=s[:a]+s[a:].replace('//20261112-0 add/change','//20261112-1 add/change')
p.write_text(s,encoding='utf-8',newline='\n')
p=work/'docs/图表手册实现与验收.md'
s=p.read_text(encoding='utf-8').replace('非法输入与图库/颜色交互','非法输入与颜色交互')
p.write_text(s,encoding='utf-8',newline='\n')
p=work/'docs/stage4-evidence/progress.json'
d=json.loads(p.read_text(encoding='utf-8'))
row=next(x for x in d['controls'] if x['id']=='C22')
row['hardwareEvidenceAdditional']=['docs/stage4-evidence/chart/physical/chart-meter-baseline-r1/run.json','docs/stage4-evidence/chart/physical/chart-donut-baseline-r1/run.json']
row['hardwareBaselinesCompleted']=3
p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
out=work/'docs/stage4-evidence/chart/reproduction'
shutil.copy2(root/'chart-finalize.json',out/'chart-finalize.json')
for name in ['finalize_control.py','final_touch_chart.py','prepare_control_cleanup.py','execute_control_cleanup.ps1']:
 shutil.copy2(root/name,out/name)
