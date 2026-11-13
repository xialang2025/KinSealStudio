"""Retain exact type/count observations when a new freeline source does not satisfy the two-point contract."""
from pathlib import Path
import hashlib
import json
import struct

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree';base=work/'docs/stage5-evidence/native/freeline'
rows=[]
for folder in sorted(base.iterdir()):
    probe=folder/'native-probe.json'
    if not probe.is_file():continue
    data=json.loads(probe.read_text(encoding='utf-8'))
    for c in data['nativeCompilations']:
        source=folder/'compiled'/c['label']/'input-native.sm';raw=source.read_bytes();offset=raw.find(struct.pack('>I',65576))
        assert hashlib.sha256(raw).hexdigest()==c['sourceSha256']
        count=struct.unpack_from('>I',raw,offset+4)[0] if offset>=0 else None
        rows.append(dict(run=folder.name,source=str(source.relative_to(work)),sourceSha256=c['sourceSha256'],typeOffset=offset,count=count,prefixHex=raw[offset:offset+56].hex() if offset>=0 else None,points=[struct.unpack_from('>Idd',raw,offset+8+20*i) for i in range(count)] if count is not None and count<=16 else None))
destination=base/'prefix-inspection-r1.json';assert not destination.exists();destination.write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps([dict(run=r['run'],count=r['count'],points=r['points']) for r in rows],ensure_ascii=False))
