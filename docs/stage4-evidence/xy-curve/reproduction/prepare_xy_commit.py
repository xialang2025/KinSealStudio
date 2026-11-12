"""Keep C25 commit paths explicit and reconcile archive dates without rewriting original raw evidence."""
from pathlib import Path
import json
root=Path(__file__).resolve().parent
s=(root/'commit_trend.py').read_text(encoding='utf-8').replace('C23','C25').replace('trend_dialog','xy_curve_dialog').replace('trend_properties','xy_curve_properties').replace('samples/stage4/trend.ksp','samples/stage4/xy-curve.ksp').replace('stage4-evidence/trend/','stage4-evidence/xy-curve/').replace('趋势图','X-Y曲线')
s=s.replace("'src/stage3_project_edits.h', ",'')
s=s.replace('复刻X-Y曲线独立通道与坐标轴并验证原厂基线实物下载','复刻X-Y曲线双数值轴与成对跨度并验证原厂基线实物下载')
(root/'commit_xy.py').write_text(s,encoding='utf-8',newline='\n')
p=root/'worktree/docs/stage4-evidence/xy-curve/native/native-manifest.json';data=json.loads(p.read_text(encoding='utf-8'));data['date']='2026-11-13';data['archiveDate']='2026-11-13';data['captureDates']=['2026-11-12','2026-11-13'];p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
print('Prepared explicit C25 commit list and truthful capture/archive date scopes.')
