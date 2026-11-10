"""#20261110-8 add: Decode exact typed spans without treating a source as a device oracle."""
from pathlib import Path
import argparse
import hashlib
import json
import re
import struct


def main():
    """Decode fields and emit compact per-object address and geometry inventories."""
    parser = argparse.ArgumentParser()
    parser.add_argument('source', type=Path)
    parser.add_argument('fields', type=Path)
    args = parser.parse_args()
    data = args.source.read_bytes()
    spans = json.loads(args.fields.read_text(encoding='utf8'))
    assert spans['bodyValidated'] and spans['sha256'] == hashlib.sha256(data).hexdigest()
    values = {}
    for field in spans['fields']:
        raw = data[field['start']:field['end']]
        kind = field['kind']
        if kind == 'q':
            value = None if raw[:4] == b'\xff\xff\xff\xff' else raw[4:].decode('utf-16-be')
        elif kind == 'd':
            value = struct.unpack('>d', raw)[0]
        elif kind in ('i', 's'):
            value = int.from_bytes(raw, 'big', signed=True)
        elif kind in ('b', 'a'):
            value = int.from_bytes(raw, 'big')
        else:
            value = raw.hex()
        values[field['path']] = {'value': value, **field}
    objects = {}
    for path, value in values.items():
        match = re.match(r'object\[(\d+)\]\.(.*)', path)
        if not match:
            continue
        index, suffix = match.groups()
        obj = objects.setdefault(index, {'index': int(index), 'addresses': {}, 'geometry': {}})
        v = value['value']
        if value['kind'] == 'q' and isinstance(v, str) and re.fullmatch(r'(LB|LW|RB|RW|M|D|X|Y)\d+', v):
            obj['addresses'][suffix] = v
        if suffix.startswith(('rectangle.', 'text.head.')) or suffix in ('z', 'type', 'movable'):
            obj['geometry'][suffix] = v
    for obj in objects.values():
        g = obj['geometry']
        obj['logicalRect'] = [g.get('rectangle.0', 0) + g.get('rectangle.9', 0), g.get('rectangle.1', 0) + g.get('rectangle.10', 0), g.get('text.head.2'), g.get('text.head.3')]
    output = args.fields.with_name(args.fields.name.replace('-fields.json', '-values.json'))
    assert output != args.fields
    output.write_text(json.dumps(values, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    inventory = args.fields.with_name(args.fields.name.replace('-fields.json', '-objects.json'))
    inventory.write_text(json.dumps(list(objects.values()), ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print(json.dumps({'source': args.source.name, 'objects': list(objects.values())}, ensure_ascii=False))


if __name__ == '__main__':
    main()
