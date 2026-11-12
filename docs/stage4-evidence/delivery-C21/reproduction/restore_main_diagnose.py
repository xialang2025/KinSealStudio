"""Restore all diagnostic edits byte-for-byte without reverting any pre-existing stage3 work."""
from pathlib import Path
import hashlib
import json
root = Path(__file__).resolve().parent
main = root.parents[1] / 'KinSealStudio'
backup = root / 'diagnose-test-originals'
records = json.loads((backup / 'manifest.json').read_text(encoding='utf-8'))
for row in records:
    path = main / row['relative']
    original = (backup / path.name).read_bytes()
    assert hashlib.sha256(path.read_bytes()).hexdigest() == row['instrumentedSha256'], 'Concurrent source change must be preserved'
    assert hashlib.sha256(original).hexdigest() == row['sha256']
for row in records:
    path = main / row['relative']
    path.write_bytes((backup / path.name).read_bytes())
    assert hashlib.sha256(path.read_bytes()).hexdigest() == row['sha256']
out = root / 'worktree/docs/stage4-evidence/delivery-C21/diagnose-restoration.json'
out.write_text(json.dumps(dict(restoredByteIdentical=True, preExistingStage3ChangesPreserved=True, temporaryWorkaroundsRetained=False, files=records), ensure_ascii=False, indent=2), encoding='utf-8')
print('All three diagnostic sources restored byte-for-byte; prior user changes preserved.')
