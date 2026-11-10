"""#20261110-6 add: Decode only spans produced by the complete typed C++ parser."""
from pathlib import Path
import hashlib
import json
import struct
ROOT = Path(__file__).resolve().parent
records = []
for report in sorted(ROOT.glob('*-fields.json')):
    parsed = json.loads(report.read_text(encoding='utf-8-sig'))
    source = report.with_name(report.name.replace('-fields.json', '-native.sm'))
    data = source.read_bytes()
    assert hashlib.sha256(data).hexdigest() == parsed['sha256']
    values = {}
    for span in parsed['fields']:
        path = span['path']
        if path.startswith(('system.identity.', 'system.164', 'config.288', 'config.290', 'config.298')):
            raw = data[int(span['start']):int(span['end'])]
            kind = span['kind']
            if kind == 'q':
                n = int.from_bytes(raw[:4], 'big')
                value = None if n == 0xffffffff else raw[4:].decode('utf-16-be')
            elif len(raw) in (1, 2, 4):
                value = int.from_bytes(raw, 'big', signed=True)
            else:
                value = raw.hex()
            values[path] = dict(value=value, start=span['start'], end=span['end'], kind=kind)
    records.append(dict(source=source.name, sha256=parsed['sha256'], values=values))
(ROOT / 'language-field-observations.json').write_text(json.dumps(records, ensure_ascii=False, indent=2), encoding='utf-8')
for r in records:
    print(r['source'], {k:v['value'] for k,v in r['values'].items() if k in ['system.identity.15','system.identity.16','config.290.0','config.288.0','config.288.1','config.290.1','config.298.count']})
