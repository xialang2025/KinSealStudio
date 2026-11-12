from pathlib import Path
import json
root = Path(__file__).resolve().parent
p = root / 'archive_native_incremental.py'
s = p.read_text(encoding='utf-8')
s = s.replace("'alarm-display', 'history-alarm')", "'alarm-display', 'history-alarm', 'history-data')")
s = s.replace("'history-alarm': '历史报警显示器'}", "'history-alarm': '历史报警显示器', 'history-data': '历史数据显示器'}")
s = s.replace("if args.family == 'history-trend' else", "if args.family in ('history-trend', 'history-data') else")
s = s.replace("{title, '数据采集', 'KinSealStudio'}", "{title, '数据采集', 'KinSealStudio', '设置错误', '选择颜色'}")
p.write_text(s, encoding='utf-8', newline='\n')
base = root / 'worktree/docs/stage4-evidence/history-data/native'
for p in sorted(base.glob('native-probe-*.json')):
    d = json.loads(p.read_text(encoding='utf-8'))
    if 'status' not in d:
        continue
    print(p.name, d['status'], d.get('ownedProcessTerminated'))
    if 'no-columns' in p.name:
        for snap in d['snapshots']:
            for w in snap['windows']:
                if w['title'] == '设置错误':
                    print(json.dumps(w, ensure_ascii=False)[:4000])
