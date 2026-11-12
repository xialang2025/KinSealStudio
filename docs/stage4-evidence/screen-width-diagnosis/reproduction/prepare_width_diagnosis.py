"""Prepare an immutable schema2 marker variant using the retained verified static-text properties."""
from pathlib import Path
import argparse
import copy
import json

root = Path(__file__).resolve().parent
base = root / 'worktree/docs/stage4-evidence/screen-width-diagnosis'
parser = argparse.ArgumentParser()
parser.add_argument('--marker', choices=('a', 'b'), default='b')
args = parser.parse_args()
project = json.loads((base / 'marker-a.ksp').read_text(encoding='utf-8'))
assert project['schemaVersion'] == 2
assert len(project['pages']) == 1 and len(project['pages'][0]['objects']) == 1
obj = project['pages'][0]['objects'][0]
assert obj['type'] == 'text' and obj['staticText']['version'] == 1
if args.marker == 'b':
    project = copy.deepcopy(project)
    project['projectName'] = '实屏诊断B'
    obj = project['pages'][0]['objects'][0]
    obj['id'] = 'width-diag-marker-b'
    obj['name'] = '诊断标识B'
    obj['text'] = 'KS-DIAG-B-1113\nSTATIC TEXT ONLY\n800x480'
    obj['staticText']['texts']['中文'] = obj['text']
destination = base / ('marker-' + args.marker + '.ksp')
if destination.exists():
    assert json.loads(destination.read_text(encoding='utf-8')) == project, 'Do not overwrite a retained diagnosis input'
else:
    destination.write_text(json.dumps(project, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
print(str(destination))
