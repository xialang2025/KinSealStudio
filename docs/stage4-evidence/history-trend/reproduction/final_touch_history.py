"""Reconcile the old minute-limit claim and publish the exact C24 review and cleanup totals."""
from pathlib import Path
import json
import shutil
root=Path(__file__).resolve().parent;work=root/'worktree';base=work/'docs/stage4-evidence/history-trend'
p=work/'design_plan/历史趋势图功能与交互复刻手册.md';s=p.read_text(encoding='utf-8')
s=s.replace('分钟实测上限 **10**（步进钳制）[V]；小时 20 步进到 21 未达顶、分钟/秒向下到 0/1，精确上下限 [待验]','2026-11-12读取真实微调框范围确认小时 **0..48**、分钟/秒 **0..59**；旧轮仅步进到10，不能据此断言上限10')
s=s.replace('分钟0(≤10)','分钟0(0..59)').replace('分钟≤10','小时0..48、分秒0..59')
s=s.replace('Y 侧未测出钳制值 [待验]','Y侧2026-11-12补证同为 **3..10**')
p.write_text(s,encoding='utf-8',newline='\n')
p=base/'review.json';d=json.loads(p.read_text(encoding='utf-8'))
d['findings'].append('只读复核还发现仅含数据采集的schema12容器需在输出容器之前定版；已提前设置schema并最终重建，九项新测试包含仅采集往返通过。')
d['findings'][1]='首轮构建发现画布错误传入ChartProperties，另在只读复核修正新测试QPoint粘贴接口；修正后完整重建，最终9项新增用例全部PASS，130+59+10=199/0/0。'
p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
p=work/'docs/历史趋势图手册实现与验收.md';s=p.read_text(encoding='utf-8');s=s.replace('正式测试与构建日志保留，','另在只读复核补齐仅采集工程的schema12必需容器，并在最终重建后验证仅采集往返。正式测试与构建日志保留，')
s=s.replace('软件重复PNG/KSP按SHA保留同内容副本后清理。','软件重复PNG/KSP按SHA保留同内容副本后清理71文件，共清理384文件，未把硬链接逻辑大小当作实际释放量。')
p.write_text(s,encoding='utf-8',newline='\n')
for name in ['commit_history.py','final_touch_history.py']:
 shutil.copy2(root/name,base/'reproduction'/name)
print('Reconciled measured bounds, final build review and all 384 scoped cleanup entries.')
