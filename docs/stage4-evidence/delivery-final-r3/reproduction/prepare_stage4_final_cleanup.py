"""Plan deletion of owned regenerable objects and superseded package copies with retained peers."""
from pathlib import Path
import hashlib
import json
import shutil

root = Path(__file__).resolve().parent
work = root / 'worktree'
main = root.parents[1] / 'KinSealStudio'
out = work / 'docs/stage4-evidence/delivery-final-r3'
read = lambda p: json.loads(p.read_text(encoding='utf-8-sig'))
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
manifest = read(out / 'package-manifest.json')
package = Path(manifest['package'])
assert package.resolve() == (main / 'dist/KinSealStudioLite-Stage4-41items-0.10.1-final-r3').resolve()
smoke = read(out / 'portable-r1/run.json')
assert smoke['passed'] and len(smoke['runs']) == 40 and smoke['privateDesktopClosed']
for entry in manifest['files']:
    peer = package / entry['path']
    assert peer.stat().st_size == entry['bytes'] and sha(peer) == entry['sha256']
files = []
build = work / 'build'
assert build.resolve().is_relative_to(work.resolve()) and not build.is_symlink()
for path in build.rglob('*'):
    if path.is_file() and path.suffix in ('.obj', '.o', '.d'):
        assert path.resolve().is_relative_to(build.resolve()) and not path.is_symlink()
        files.append(dict(path=str(path), bytes=path.stat().st_size, sha256=sha(path),
            category='regenerable_compiler_object_or_dependency', retained=None))
for run in ('final-r1', 'final-r2'):
    old = main / 'dist' / ('KinSealStudioLite-Stage4-41items-0.10.1-' + run)
    assert old.resolve().is_relative_to((main / 'dist').resolve()) and not old.is_symlink()
    evidence = work / 'docs/stage4-evidence' / ('delivery-' + run)
    retained = evidence / 'retained-package-inputs'
    retained.mkdir(exist_ok=True)
    inputs = []
    for path in old.rglob('*'):
        if not path.is_file():
            continue
        assert path.resolve().is_relative_to(old.resolve()) and not path.is_symlink()
        relative = path.relative_to(old)
        peer = package / relative
        digest = sha(path)
        if not peer.is_file() or sha(peer) != digest:
            peer = retained / relative
            peer.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, peer)
            assert sha(peer) == digest
        record = dict(path=str(path), bytes=path.stat().st_size, sha256=digest, retained=str(peer))
        inputs.append(record)
        files.append(dict(record, category='superseded_package_with_verified_peer'))
    (evidence / 'superseded-package-manifest.json').write_text(json.dumps(dict(package=str(old), files=inputs,
        supersededBy=str(package), failed=(run == 'final-r1')), ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
result = dict(allowedRoots=[str(build)] + [str(main / 'dist' / ('KinSealStudioLite-Stage4-41items-0.10.1-' + run)) for run in ('final-r1','final-r2')],
    files=files, retainedFinalPackage=str(package), logicalBytes=sum(x['bytes'] for x in files),
    exclusions='Main user checkout build and pre-existing untracked files, original installation, sources, protocol evidence, and final-r3 package are preserved.')
(out / 'cleanup-plan.json').write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
print(json.dumps(dict(files=len(files), logicalBytes=result['logicalBytes']), ensure_ascii=False))
