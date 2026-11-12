"""Resolve only the reviewed C27 known-key insertion against fresh user-stage3 identities."""
from pathlib import Path
import hashlib
import json
import re
root=Path(__file__).resolve().parent;main=root.parents[1]/'KinSealStudio';p=root/'conflict-src__project_model.cpp'
identities=json.loads(p.with_suffix('.cpp.inputs.json').read_text(encoding='utf-8'))
assert hashlib.sha256((main/'src/project_model.cpp').read_bytes()).hexdigest()==identities['current']
s=p.read_text(encoding='utf-8');blocks=list(re.finditer(r'^<<<<<<<[^\n]*\n(.*?)^=======\n(.*?)^>>>>>>>[^\n]*\n',s,re.M|re.S));assert len(blocks)==1
def resolve(m):
    current,proposed=m.group(1),m.group(2)
    assert 'QStringLiteral("locked")' in current and 'QStringLiteral("alarmBar")' in proposed
    before='QStringLiteral("datagroup"), QStringLiteral("extensions")'
    assert current.count(before)==1
    return current.replace(before,'QStringLiteral("datagroup"), QStringLiteral("alarmBar"), QStringLiteral("extensions")')
s=re.sub(r'^<<<<<<<[^\n]*\n(.*?)^=======\n(.*?)^>>>>>>>[^\n]*\n',resolve,s,flags=re.M|re.S)
assert all(x in s for x in ['remapObjectGroupIds','lockedGroups','groupAncestors','subpixelX','subpixelY','AlarmBarProperties::fromJson','validateAlarmRegistrations'])
assert not any(x in s for x in ['<<<<<<<','=======','>>>>>>>'])
out=root/'resolved-src__project_model.cpp';out.write_text(s,encoding='utf-8',newline='\n');out.with_suffix('.cpp.inputs.json').write_text(json.dumps(identities),encoding='utf-8')
