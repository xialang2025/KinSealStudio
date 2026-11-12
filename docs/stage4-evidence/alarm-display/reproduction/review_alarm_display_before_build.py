"""Apply concrete read-only review findings before the first C28 build."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
p=root/'src/project_model.cpp';s=p.read_text(encoding='utf-8');before='if (object.type == QStringLiteral("alarmdisplay") && (!object.address.isEmpty() ||';after='if (object.type == QStringLiteral("alarmdisplay") && (!object.address.isEmpty() || !object.text.isEmpty() ||';assert s.count(before)==1;s=s.replace(before,after);p.write_text(s,encoding='utf-8',newline='\n')
p=root/'src/project_model.h';s=p.read_text(encoding='utf-8').replace('AlarmDisplayProperties alarmDisplay; //20261113-2 add: Separate bar, meter and donut drafts.','AlarmDisplayProperties alarmDisplay; //20261113-2 add: Common table styling and independently localized column captions.');p.write_text(s,encoding='utf-8',newline='\n')
print('Reviewed JSON identity, global style ownership, language blockers, inactive captions, shared dependencies and renderer clipping; rejected generic display text.')
