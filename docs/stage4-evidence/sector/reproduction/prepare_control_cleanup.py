"""Plan only exact duplicate control test output removal with a retained SHA256 peer."""
from pathlib import Path
import argparse
import hashlib
import json

root = Path(__file__).resolve().parent / 'worktree'
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('slug')
parser.add_argument('--final', default='windows-final')
args = parser.parse_args()
base = root / 'docs/stage4-evidence' / args.slug
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
retained = {}
for parent in (root / 'samples/stage4', root / 'docs/stage4-evidence'):
    for path in parent.rglob('*'):
        if path.is_file() and path.suffix.lower() in ('.png', '.ksp') and not path.is_relative_to(base):
            retained.setdefault(sha(path), path)
directories = sorted([p for p in base.glob('windows*') if p.is_dir()], key=lambda p: p.name != args.final)
plan = []
for directory in directories:
    for path in directory.glob('*'):
        if not path.is_file() or path.suffix.lower() not in ('.png', '.ksp'):
            continue
        digest = sha(path)
        peer = retained.get(digest)
        if peer is not None:
            plan.append(dict(path=str(path), bytes=path.stat().st_size, sha256=digest, retained=str(peer)))
        else:
            retained[digest] = path
(base / 'cleanup-plan.json').write_text(json.dumps(dict(scope=str(base), files=plan), ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
print(json.dumps(dict(files=len(plan), bytes=sum(p['bytes'] for p in plan))))
