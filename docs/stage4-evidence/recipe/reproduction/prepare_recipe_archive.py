from pathlib import Path
root=Path(__file__).resolve().parent;p=root/'archive_native_incremental.py';s=p.read_text(encoding='utf-8')
before=next(line for line in s.splitlines() if 'assert args.family in' in line);s=s.replace(before,before[:-1]+", 'recipe')")
before="'history-data': '历史数据显示器'}";assert s.count(before)==1;s=s.replace(before,"'history-data': '历史数据显示器', 'recipe': '配方显示器'}")
before="        compact['snapshots'] =";assert s.count(before)==1;s=s.replace(before,"        if args.family == 'recipe':\n            titles = {'配方显示器', '配方', '提示', 'KinSealStudio', '选择颜色', '设置错误'}\n"+before)
p.write_text(s,encoding='utf-8',newline='\n')
