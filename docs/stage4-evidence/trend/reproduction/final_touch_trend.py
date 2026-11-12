"""Align C23 modification markers and the implementation index with newly verified native channel ownership."""
from pathlib import Path
import shutil
root=Path(__file__).resolve().parent; work=root/'worktree'
p=work/'程序修改记录.md'; s=p.read_text(encoding='utf-8'); a=s.rfind('：阶段4趋势图独立编辑与事务切片'); assert a>0
s=s[:a]+s[a:].replace('//20261112-0 add/change','//20261112-1 add/change'); p.write_text(s,encoding='utf-8',newline='\n')
for relative in ['程序修改记录.md','docs/趋势图手册实现与验收.md','docs/阶段4逐控件进度.md','docs/AMT070W124-L800X480R_USB下载协议_已验证部分.md','docs/stage4-evidence/progress.json']:
 p=work/relative; s=p.read_text(encoding='utf-8'); p.write_text(s.replace('签名优先7类型','有符号优先7类型'),encoding='utf-8',newline='\n')
p=work/'design_plan/A04_31控件实施索引.md'; s=p.read_text(encoding='utf-8')
s=s.replace('每通道独立地址/类型、取样、缓存、XY轴/显示/权限及时间。','共用读取基地址/类型、逐通道独立范围/线型/颜色、取样、缓存、XY轴/显示/权限及时间（本轮原厂补证纠正）。')
p.write_text(s,encoding='utf-8',newline='\n')
repro=work/'docs/stage4-evidence/trend/reproduction'
for name in ['trend-finalize.json','final_touch_trend.py','finalize_control.py','prepare_control_cleanup.py','execute_control_cleanup.ps1']:
 shutil.copy2(root/name,repro/name)
