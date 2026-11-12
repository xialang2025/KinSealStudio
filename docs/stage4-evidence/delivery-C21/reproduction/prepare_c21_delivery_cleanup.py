"""Plan deletion of restored diagnostic backups, empty launch logs and SHA-identical repeated test outputs."""
from package_c21 import *
owner = ROOT / 'diagnose-test-originals'
restoration = read(OUT / 'diagnose-restoration.json')
retained = {}
for folder in (WORK / 'samples/stage4', WORK / 'docs/stage4-evidence'):
    for path in folder.rglob('*'):
        if path.is_file() and path.suffix.lower() in ('.png', '.ksp') and not path.is_relative_to(OUT):
            retained.setdefault(sha(path), str(path))
plan = []
for path in sorted(OUT.rglob('*')):
    if not path.is_file():
        continue
    digest = sha(path)
    if path.suffix.lower() in ('.png', '.ksp'):
        peer = retained.get(digest)
        if peer:
            plan.append(dict(path=str(path), bytes=path.stat().st_size, sha256=digest, retained=peer, reason='Repeated Qt image/sample with exact retained peer.'))
        else:
            retained[digest] = str(path)
    elif path.name.endswith(('-stdout.txt', '-stderr.txt')) and path.stat().st_size == 0:
        plan.append(dict(path=str(path), bytes=0, sha256=digest, reason='Empty owned launcher diagnostics.'))
for path in owner.iterdir():
    if not path.is_file():
        continue
    record = next((row for row in restoration['files'] if Path(row['relative']).name == path.name), None)
    peer = MAIN / record['relative'] if record else None
    if peer:
        assert sha(peer) == sha(path) == record['sha256']
    plan.append(dict(path=str(path), bytes=path.stat().st_size, sha256=sha(path), retained=str(peer) if peer else None, reason='Diagnostic source bytes already restored and hash-verified; original state preserved in main checkout.'))
for name in ('dpi-first.png', 'dpi-second.png'):
    path = ROOT / name
    if path.exists():
        target = OUT / 'diagnose-focus-r3' / ('workaround-' + name)
        shutil.copy2(path, target)
        assert sha(path) == sha(target)
        # Move the owned single file into the cleanup scope only after retaining an exact peer.
        scoped = owner / name
        assert not scoped.exists()
        path.rename(scoped)
        plan.append(dict(path=str(scoped), bytes=scoped.stat().st_size, sha256=sha(scoped), retained=str(target), reason='Redundant rendered diagnostic image; explicit workaround image retained.'))
write(ROOT / 'delivery-C21-cleanup-plan.json', dict(ownerScope=str(owner), evidenceScope=str(OUT), files=plan))
shutil.copy2(ROOT / 'prepare_c21_delivery_cleanup.py', OUT / 'reproduction/prepare_c21_delivery_cleanup.py')
print(json.dumps(dict(files=len(plan), logicalBytes=sum(row['bytes'] for row in plan)), ensure_ascii=False))
