#!/usr/bin/env python3
"""Publish one detached commit while retaining a frozen user's working-tree delta."""
#20261114-10 add: This local publication tool never pushes or resets a user's tree.
import argparse
from datetime import datetime, timezone
import difflib
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import stat
import subprocess
import sys
import uuid


class PublicationError(RuntimeError):
    """Represent a failed publication guard, without implying rollback succeeded."""


def digest(data):
    """Return the SHA256 identity of exact bytes."""
    return hashlib.sha256(data).hexdigest()


def json_bytes(value):
    """Encode a deterministic readable artifact without platform newline changes."""
    return (json.dumps(value, ensure_ascii=False, indent=2) + '\n').encode('utf-8')


def require(value, message):
    """Reject a failed guard even when Python optimization is enabled."""
    if not value:
        raise PublicationError(message)


def safe_path(path, boundary, allow_missing=False):
    """Reject reparse points and lexical/resolved escapes before touching a path."""
    path = Path(os.path.abspath(path))
    boundary = Path(os.path.abspath(boundary))
    require(path.is_relative_to(boundary), 'Path escapes declared boundary: ' + str(path))
    for node in (path, *path.parents):
        if node.exists() or node.is_symlink():
            info = node.lstat()
            require(not stat.S_ISLNK(info.st_mode) and not getattr(info, 'st_file_attributes', 0) & 0x400,
                    'Reparse/symlink path is not permitted: ' + str(node))
        elif not allow_missing:
            raise PublicationError('Required path does not exist: ' + str(node))
        if node == boundary:
            break
    require(path.resolve(strict=not allow_missing).is_relative_to(boundary.resolve()), 'Resolved path escapes boundary')
    return path


def relative_name(value):
    """Accept an exact portable Git path, with no Windows aliases or wildcards."""
    require(isinstance(value, str) and value and '\\' not in value, 'Invalid relative Git path')
    path = PurePosixPath(value)
    require(not path.is_absolute() and path.as_posix() == value, 'Noncanonical relative Git path: ' + value)
    for part in path.parts:
        require(part not in ('.', '..') and part.lower() != '.git' and not part.endswith((' ', '.')),
                'Unsafe Git path component: ' + value)
        require(not any(c in part for c in ':*?<>|"') and not any(ord(c) < 32 for c in part), 'Unsafe Windows Git path: ' + value)
        require(part.split('.')[0].upper() not in {'CON', 'PRN', 'AUX', 'NUL', *('COM' + str(x) for x in range(1, 10)), *('LPT' + str(x) for x in range(1, 10))},
                'Windows reserved path: ' + value)
    return value


def git(repo, *args, accepted=(0,), quote_paths=False, timeout_seconds=120):
    """Run a fixed-argument Git process without a shell or optional index refresh."""
    env = os.environ.copy()
    env['GIT_OPTIONAL_LOCKS'] = '0'
    require(type(quote_paths) is bool, 'Git quotepath must be an explicit boolean')
    require(type(timeout_seconds) is int and 1 <= timeout_seconds <= 600, 'Git timeout must remain bounded')
    result = subprocess.run(['git', '-c', 'core.quotepath=' + ('true' if quote_paths else 'false'), '-C', str(repo), *args],
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env, timeout=timeout_seconds)
    if result.returncode not in accepted:
        raise PublicationError('git ' + args[0] + ' failed (' + str(result.returncode) + '): ' + result.stderr.decode('utf-8', errors='replace')[:3000])
    return result.stdout


def status(repo):
    """Read every changed/untracked path without command-line path expansion."""
    # A freshly read tree has no stat cache: the real 33k-entry repository needs a complete bounded scan.
    raw = git(repo, 'status', '--porcelain=v1', '-z', '--untracked-files=all', timeout_seconds=600)
    rows = {}
    for record in raw.split(b'\0'):
        if not record:
            continue
        code = record[:2].decode('ascii')
        require('R' not in code and 'C' not in code and code[0] in (' ', '?'), 'Renames or staged changes are not allowed')
        name = relative_name(record[3:].decode('utf-8'))
        rows[name] = code
    return rows


def tree(repo, commit):
    """Read one complete tree instead of passing hundreds of paths in argv."""
    result = {}
    for row in git(repo, 'ls-tree', '-r', '-z', commit).split(b'\0'):
        if not row:
            continue
        meta, name = row.split(b'\t', 1)
        mode, kind, oid = meta.decode('ascii').split(' ')
        result[name.decode('utf-8')] = (mode, kind, oid)
    return result


def changed_paths(repo, base, candidate):
    """Return exactly added/modified regular-file paths from a complete commit diff."""
    fields = git(repo, 'diff', '--name-status', '--no-renames', '-z', base, candidate, '--').split(b'\0')
    result = {}
    for offset in range(0, len(fields) - 1, 2):
        require(offset + 1 < len(fields), 'Malformed Git diff')
        kind = fields[offset].decode('ascii')
        require(kind in ('A', 'M'), 'Only add/modify publication is supported; found ' + kind)
        result[relative_name(fields[offset + 1].decode('utf-8'))] = kind
    require(result, 'Candidate has no changes')
    return result


def checked_input(path, expected):
    """Bind an explicitly named input to its human-supplied exact SHA256."""
    data = path.read_bytes()
    require(len(expected) == 64 and digest(data) == expected.lower(), 'Input hash mismatch: ' + str(path))
    return data


def working_tree_diff(repo, baseline, quote_paths=False):
    """Give complete working-tree content scans their own bounded cold-cache budget."""
    return git(repo, 'diff', '--binary', baseline, '--', quote_paths=quote_paths, timeout_seconds=600)


def frozen_diff_representation(repo, baseline, frozen, declared=None):
    """Match a complete real Git diff in either explicit path-quoting representation."""
    require(declared is None or declared in ('true', 'false'), 'Invalid snapshot diff quotepath declaration')
    normalize = lambda value: value.decode('utf-8-sig').replace('\r\n', '\n')
    variants = {mode: working_tree_diff(repo, baseline, quote_paths=(mode == 'true'))
                for mode in ('false', 'true')}
    matches = [mode for mode, raw in variants.items() if normalize(raw) == normalize(frozen)]
    require(bool(matches) and (declared is None or declared in matches),
            'Protected tracked user diff differs from frozen input or declared quotepath representation')
    selected = declared if declared is not None else matches[0]
    return selected, {'selectedQuotepath': selected, 'matchingQuotepathValues': matches,
                      'declaredByPriorSnapshot': declared,
                      'comparison': 'entire actual git diff; headers and content retained; only BOM/CRLF normalized',
                      'variantSha256': {mode: digest(raw) for mode, raw in variants.items()}}


def text_shape(data):
    """Normalize UTF-8 text only, recording the exact BOM and newline envelope."""
    require(b'\0' not in data, 'Binary overlap cannot be merged')
    text = data.decode('utf-8-sig')
    require('\r' not in text.replace('\r\n', ''), 'Lone CR cannot be merged safely')
    crlf = text.count('\r\n')
    lf = text.count('\n') - crlf
    require(not (crlf and lf), 'Mixed newlines in overlap require explicit review')
    shape = {'bom': data.startswith(b'\xef\xbb\xbf'), 'eol': 'CRLF' if crlf else 'LF',
             'lineCount': len(text.splitlines(keepends=True)), 'endsWithNewline': text.endswith('\n')}
    return text.replace('\r\n', '\n').encode('utf-8'), shape


def restore_shape(normalized, shape):
    """Restore the user's original text envelope after verified three-way merging."""
    return (b'\xef\xbb\xbf' if shape['bom'] else b'') + (normalized.replace(b'\n', b'\r\n') if shape['eol'] == 'CRLF' else normalized)


def content_delta(base, changed):
    """Describe ordered removed/added text chunks without shifted hunk line numbers."""
    before = base.splitlines(keepends=True)
    after = changed.splitlines(keepends=True)
    result = []
    for tag, i, j, a, b in difflib.SequenceMatcher(None, before, after, autojunk=False).get_opcodes():
        if tag != 'equal':
            result.append((tag, digest(b''.join(before[i:j])), digest(b''.join(after[a:b])), j - i, b - a))
    return result


def merge_bytes(directory, label, current, base, other):
    """Ask Git to merge private byte copies; a conflict never touches the main tree."""
    paths = []
    for number, data in enumerate((current, base, other)):
        path = directory / (label + '-' + str(number) + '.txt')
        with path.open('xb') as stream:
            stream.write(data)
        paths.append(path)
    result = subprocess.run(['git', 'merge-file', '-p', '--', *map(str, paths)], stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=120)
    require(result.returncode == 0, 'Three-way merge conflict/failure: ' + label + ': ' + result.stderr.decode('utf-8', errors='replace')[:1000])
    return result.stdout


def merge_overlap(directory, number, current, base, candidate):
    """Prove both original user delta and independent candidate delta survive merging."""
    user_norm, user_shape = text_shape(current)
    base_norm, base_shape = text_shape(base)
    task_norm, task_shape = text_shape(candidate)
    require(task_shape['bom'] == base_shape['bom'], 'Task changed BOM in an overlapping file')
    result = merge_bytes(directory, str(number) + '-forward', user_norm, base_norm, task_norm)
    user_restored = merge_bytes(directory, str(number) + '-remove-task', result, task_norm, base_norm)
    task_restored = merge_bytes(directory, str(number) + '-remove-user', result, user_norm, base_norm)
    require(user_restored == user_norm and restore_shape(user_restored, user_shape) == current,
            'Reverse task merge did not recover the exact original user bytes')
    require(task_restored == task_norm, 'Reverse user merge did not recover candidate content; candidate may contain user edits')
    before_delta = content_delta(base_norm, user_norm)
    after_delta = content_delta(task_norm, result)
    require(before_delta == after_delta, 'Ordered user edit chunks changed during merge')
    final = restore_shape(result, user_shape)
    return final, {'userRawSha256': digest(current), 'baseBlobSha256': digest(base), 'candidateRawSha256': digest(candidate),
                   'mergedRawSha256': digest(final), 'userEnvelope': user_shape, 'baseEnvelope': base_shape,
                   'candidateEnvelope': task_shape, 'normalization': 'UTF-8 BOM recorded; LF merge only; original user BOM/EOL restored',
                   'reverseTaskRestoresUserRawExactly': True, 'reverseUserRestoresCandidateNormalizedExactly': True,
                   'orderedUserDeltaUnchanged': True, 'userDeltaSha256': digest(json_bytes(before_delta)),
                   'userDeltaChunkCount': len(before_delta)}


def inspect_main(args, snapshot):
    """Recheck immutable user files, branch, base HEAD, empty index and changed paths."""
    require(git(args.main, 'symbolic-ref', '--short', 'HEAD').decode().strip() == 'main', 'Main checkout is not on main')
    require(git(args.main, 'rev-parse', 'HEAD').decode().strip() == args.baseline, 'Main HEAD moved')
    require(not git(args.main, 'diff', '--cached', '--name-only', '-z', '--'), 'Main index is not empty')
    require(snapshot['head'] == args.baseline and snapshot.get('mainIndexEmpty') is True, 'Snapshot base/index declaration differs')
    files = snapshot['files']
    require(len(files) == args.expected_user_count, 'Unexpected protected user file count')
    observed_status = status(args.main)
    require(observed_status == {relative_name(k): v['status'] for k, v in files.items()}, 'Main user path/status set changed')
    for name, recorded in files.items():
        path = safe_path(args.main / name, args.main)
        require(path.is_file(), 'Protected user path is not a file: ' + name)
        raw = path.read_bytes()
        require(len(raw) == recorded['bytes'] and digest(raw) == recorded['sha256'], 'Protected user bytes changed: ' + name)
    return {'head': args.baseline, 'protectedUserFiles': len(files), 'statusCount': len(observed_status), 'allProtectedBytesMatch': True}


def write_receipt(operation, record):
    """Atomically update this invocation's exclusive receipt, never another operation's."""
    temp = operation / ('receipt-' + uuid.uuid4().hex + '.tmp')
    with temp.open('xb') as stream:
        stream.write(json_bytes(record))
    os.replace(temp, operation / 'receipt.json')


def execute(args, fault=None):
    """Preflight or publish with complete previews, conditional writes and checked rollback."""
    record = {'format': 'KinSealLite-stage11-publication-v1', 'startedAtUtc': datetime.now(timezone.utc).isoformat(),
              'mode': args.mode, 'phase': 'initialization', 'outcome': 'not_started', 'mainMutated': False,
              'gitTimeoutPolicySeconds': {'status': 600, 'workingTreeDiff': 600, 'otherIncludingWrites': 120},
              'rollback': {'attempted': False, 'confirmed': False}, 'changedFiles': [], 'overlaps': []}
    operation = None
    lock = None
    written = []
    created_directories = []
    ref_changed = False
    index_changed = False
    index_before = None
    index_after = None
    plan = []
    try:
        args.workspace_root = safe_path(args.workspace_root, args.workspace_root)
        args.run_root = safe_path(args.run_root, args.workspace_root, allow_missing=True)
        require(args.run_root != args.workspace_root, 'Run root cannot be the workspace root')
        for checkout in (args.main, args.worktree):
            lexical = Path(os.path.abspath(checkout))
            require(not args.run_root.is_relative_to(lexical) and not lexical.is_relative_to(args.run_root), 'Run root cannot overlap a checkout')
        args.run_root.mkdir(parents=True, exist_ok=True)
        operation = args.run_root / ('publication-' + uuid.uuid4().hex)
        operation.mkdir()
        write_receipt(operation, record)
        args.main = safe_path(args.main, args.workspace_root)
        args.worktree = safe_path(args.worktree, args.workspace_root)
        require(not args.run_root.is_relative_to(args.main) and not args.run_root.is_relative_to(args.worktree), 'Run root must be separate from both checkouts')
        record.update(main=str(args.main), worktree=str(args.worktree), baseline=args.baseline, candidate=args.candidate,
                      expectedUserCount=args.expected_user_count, operationDirectory=str(operation))
        require(len(args.baseline) == 40 and len(args.candidate) == 40 and all(c in '0123456789abcdef' for c in args.baseline + args.candidate), 'Require full lowercase SHA1 commit IDs')
        for name in ('snapshot', 'snapshot_diff', 'allowlist'):
            setattr(args, name, safe_path(getattr(args, name), args.workspace_root))
        snapshot_raw = checked_input(args.snapshot, args.snapshot_sha256)
        original_diff = checked_input(args.snapshot_diff, args.snapshot_diff_sha256)
        allow_raw = checked_input(args.allowlist, args.allowlist_sha256)
        snapshot = json.loads(snapshot_raw)
        allow_doc = json.loads(allow_raw)
        allowed = allow_doc.get('paths') if isinstance(allow_doc, dict) else allow_doc
        require(isinstance(allowed, list) and len(allowed) == len(set(allowed)), 'Allowlist must contain unique exact paths')
        allowed = {relative_name(x) for x in allowed}
        record['inputs'] = {'snapshot': str(args.snapshot), 'snapshotSha256': digest(snapshot_raw),
                            'snapshotDiff': str(args.snapshot_diff), 'snapshotDiffSha256': digest(original_diff),
                            'allowlist': str(args.allowlist), 'allowlistSha256': digest(allow_raw)}
        require(git(args.worktree, 'rev-parse', '--show-toplevel').decode().strip().replace('\\', '/').lower() == args.worktree.as_posix().lower(), 'Worktree root differs')
        require(git(args.main, 'rev-parse', '--show-toplevel').decode().strip().replace('\\', '/').lower() == args.main.as_posix().lower(), 'Main root differs')
        main_common = Path(git(args.main, 'rev-parse', '--path-format=absolute', '--git-common-dir').decode().strip()).resolve()
        work_common = Path(git(args.worktree, 'rev-parse', '--path-format=absolute', '--git-common-dir').decode().strip()).resolve()
        require(main_common == work_common and main_common.is_relative_to(args.main), 'Checkouts must share main repository metadata')
        safe_path(main_common, args.main)
        require(git(args.worktree, 'symbolic-ref', '-q', 'HEAD', accepted=(0, 1)) == b'', 'Candidate worktree is not detached')
        require(git(args.worktree, 'rev-parse', 'HEAD').decode().strip() == args.candidate, 'Candidate worktree HEAD differs')
        parents = git(args.worktree, 'rev-list', '--parents', '-n', '1', args.candidate).decode().split()
        require(parents == [args.candidate, args.baseline], 'Candidate must have exactly one parent equal to baseline')
        require(not status(args.worktree), 'Candidate worktree has uncommitted files')
        record['phase'] = 'preflight-user-protection'
        record['before'] = inspect_main(args, snapshot)
        selected_quotepath, diff_proof = frozen_diff_representation(
            args.main, args.baseline, original_diff, snapshot.get('snapshotDiffQuotepath'))
        record['protectedDiffRepresentation'] = diff_proof
        changes = changed_paths(args.worktree, args.baseline, args.candidate)
        require(set(changes) == allowed, 'Candidate changed-path set differs from exact allowlist')
        candidate_tree = tree(args.worktree, args.candidate)
        base_tree = tree(args.worktree, args.baseline)
        previews = operation / 'previews'
        previews.mkdir()
        backups = operation / 'backups'
        backups.mkdir()
        for number, (name, change) in enumerate(sorted(changes.items())):
            require(candidate_tree[name][0] in ('100644', '100755') and candidate_tree[name][1] == 'blob', 'Candidate symlink/submodule unsupported: ' + name)
            source = safe_path(args.worktree / name, args.worktree)
            target = safe_path(args.main / name, args.main, allow_missing=True)
            require(source.is_file(), 'Candidate working file missing: ' + name)
            candidate_raw = source.read_bytes()
            require(git(args.worktree, 'hash-object', '--path=' + name, str(source)).decode().strip() == candidate_tree[name][2], 'Candidate raw file does not match commit: ' + name)
            overlap = name in snapshot['files']
            old = target.read_bytes() if target.is_file() else None
            require(not target.exists() or target.is_file(), 'Target is not a file: ' + name)
            require(change != 'A' or old is None, 'New target already exists: ' + name)
            require(change != 'M' or old is not None, 'Modified target is missing: ' + name)
            final = candidate_raw
            if overlap:
                require(snapshot['files'][name]['status'] == ' M' and change == 'M', 'Only unstaged tracked text overlaps can be merged')
                base_blob = git(args.worktree, 'cat-file', 'blob', base_tree[name][2])
                final, proof = merge_overlap(previews, number, old, base_blob, candidate_raw)
                record['overlaps'].append({'path': name, **proof})
            elif old is not None:
                require(git(args.main, 'hash-object', '--path=' + name, str(target)).decode().strip() == base_tree[name][2], 'Unprotected target differs from baseline: ' + name)
            staged = previews / (str(number) + '.bin')
            staged.write_bytes(final)
            backup = backups / (str(number) + '.bin')
            if old is not None:
                backup.write_bytes(old)
            item = {'path': name, 'change': change, 'overlap': overlap, 'beforeSha256': digest(old) if old is not None else None,
                    'afterSha256': digest(final), 'bytes': len(final), 'source': staged, 'backup': backup, 'target': target}
            plan.append(item)
            record['changedFiles'].append({k: v for k, v in item.items() if k not in ('source', 'backup', 'target')})
        index_path = Path(git(args.main, 'rev-parse', '--path-format=absolute', '--git-path', 'index').decode().strip())
        safe_path(index_path, main_common)
        index_before = index_path.read_bytes()
        (backups / 'index.bin').write_bytes(index_before)
        record['beforeIndexSha256'] = digest(index_before)
        record['phase'] = 'preflight-complete'
        record['outcome'] = 'preflight_passed'
        write_receipt(operation, record)
        if args.mode == 'preflight':
            return 0, operation / 'receipt.json'
        lock = main_common / 'kinseal-stage11-publication.lock'
        with lock.open('xb') as stream:
            stream.write(operation.name.encode())
        record['phase'] = 'recheck-before-write'
        inspect_main(args, snapshot)
        require(index_path.read_bytes() == index_before, 'Main index bytes changed after preflight')
        require(git(args.worktree, 'rev-parse', 'HEAD').decode().strip() == args.candidate and not status(args.worktree), 'Candidate worktree changed after preflight')
        if fault:
            fault('before_write')
        for item in plan:
            record['phase'] = 'write:' + item['path']
            write_receipt(operation, record)
            safe_path(item['target'], args.main, allow_missing=True)
            actual = item['target'].read_bytes() if item['target'].is_file() else None
            require((digest(actual) if actual is not None else None) == item['beforeSha256'], 'Target changed after preflight: ' + item['path'])
            missing = []
            parent = item['target'].parent
            while not parent.exists():
                missing.append(parent)
                parent = parent.parent
            for directory in reversed(missing):
                directory.mkdir()
                created_directories.append(directory)
            temporary = item['target'].with_name(item['target'].name + '.stage11-' + uuid.uuid4().hex + '.tmp')
            try:
                with temporary.open('xb') as stream:
                    stream.write(item['source'].read_bytes())
                    stream.flush()
                    os.fsync(stream.fileno())
                os.replace(temporary, item['target'])
            finally:
                if temporary.exists():
                    temporary.unlink()
            written.append(item)
            record['mainMutated'] = True
            write_receipt(operation, record)
            if fault:
                fault('after_file:' + item['path'])
        record['phase'] = 'index-update'
        write_receipt(operation, record)
        require(index_path.read_bytes() == index_before, 'Main index changed during controlled writes')
        git(args.main, 'read-tree', args.candidate)
        index_changed = True
        index_after = index_path.read_bytes()
        record['afterIndexSha256'] = digest(index_after)
        if fault:
            fault('after_index')
        record['phase'] = 'ref-update'
        write_receipt(operation, record)
        git(args.main, 'update-ref', '-m', 'stage11 protected publication', 'refs/heads/main', args.candidate, args.baseline)
        ref_changed = True
        if fault:
            fault('after_ref')
        record['phase'] = 'verify-after'
        require(git(args.main, 'rev-parse', 'HEAD').decode().strip() == args.candidate, 'Published HEAD differs')
        require(not git(args.main, 'diff', '--cached', '--name-only', '-z', '--'), 'Published index has staged user changes')
        expected_files = json.loads(json.dumps(snapshot['files']))
        for item in plan:
            require(item['target'].is_file() and digest(item['target'].read_bytes()) == item['afterSha256'], 'Published target verification failed: ' + item['path'])
            if item['overlap']:
                expected_files[item['path']]['bytes'] = item['bytes']
                expected_files[item['path']]['sha256'] = item['afterSha256']
        observed = status(args.main)
        require(set(observed) == set(expected_files), 'Published user path set changed')
        for name, old in expected_files.items():
            path = safe_path(args.main / name, args.main)
            raw = path.read_bytes()
            require(len(raw) == old['bytes'] and digest(raw) == old['sha256'], 'Published user protection failed: ' + name)
            require(observed[name] == old['status'], 'Published user status changed: ' + name)
        after_snapshot = {'head': args.candidate, 'files': expected_files, 'mainIndexEmpty': True,
                          'marker': '//20261114-10', 'publicationReceipt': str(operation / 'receipt.json'),
                          'snapshotDiffQuotepath': selected_quotepath,
                          'priorSnapshotSha256': digest(snapshot_raw), 'priorSnapshotPath': str(args.snapshot),
                          'originalProtectedFileCount': len(expected_files), 'overlapProofs': record['overlaps']}
        after_path = operation / 'after-protection-snapshot.json'
        with after_path.open('xb') as stream:
            stream.write(json_bytes(after_snapshot))
        after_diff = working_tree_diff(args.main, args.candidate, quote_paths=(selected_quotepath == 'true'))
        with (operation / 'after-protection-snapshot.diff').open('xb') as stream:
            stream.write(after_diff)
        record['after'] = {'head': args.candidate, 'protectedUserFiles': len(expected_files), 'overlapFiles': len(record['overlaps']),
                           'nonoverlapUserBytesUnchanged': True, 'overlapDeltaProvedPreserved': True,
                           'snapshotPath': str(after_path), 'snapshotSha256': digest(after_path.read_bytes()),
                           'diffPath': str(operation / 'after-protection-snapshot.diff'), 'diffSha256': digest(after_diff),
                           'allProtectedPathsRemainUncommitted': True}
        record['phase'] = 'complete'
        record['outcome'] = 'published'
        return 0, operation / 'receipt.json'
    except Exception as error:
        record['error'] = type(error).__name__ + ': ' + str(error)
        record['failedPhase'] = record['phase']
        record['outcome'] = 'rejected_before_main_write' if not written else 'publication_failed'
        if written or index_changed or ref_changed:
            record['rollback']['attempted'] = True
            failures = []
            try:
                if ref_changed:
                    git(args.main, 'update-ref', '-m', 'stage11 checked rollback', 'refs/heads/main', args.baseline, args.candidate)
                if index_changed:
                    require(index_path.read_bytes() == index_after, 'Index changed externally; not overwriting it')
                    temp_index = index_path.with_name('index.stage11-' + uuid.uuid4().hex + '.tmp')
                    with temp_index.open('xb') as stream:
                        stream.write(index_before)
                    os.replace(temp_index, index_path)
            except Exception as rollback_error:
                failures.append(str(rollback_error))
            for item in reversed(written):
                try:
                    safe_path(item['target'], args.main)
                    require(digest(item['target'].read_bytes()) == item['afterSha256'], 'Target changed externally; not overwriting: ' + item['path'])
                    if item['beforeSha256'] is None:
                        item['target'].unlink()
                    else:
                        require(digest(item['backup'].read_bytes()) == item['beforeSha256'], 'Backup checksum differs')
                        item['target'].write_bytes(item['backup'].read_bytes())
                except Exception as rollback_error:
                    failures.append(str(rollback_error))
            for directory in reversed(created_directories):
                try:
                    safe_path(directory, args.main)
                    directory.rmdir()
                except OSError:
                    pass
            try:
                inspect_main(args, snapshot)
                require(index_path.read_bytes() == index_before, 'Original index bytes were not restored')
                for item in plan:
                    if item['beforeSha256'] is None:
                        require(not item['target'].exists(), 'Owned new target remains after rollback')
                    else:
                        require(item['target'].is_file() and digest(item['target'].read_bytes()) == item['beforeSha256'], 'Original target not restored')
            except Exception as rollback_error:
                failures.append(str(rollback_error))
            record['rollback'].update(confirmed=not failures, failures=failures)
            record['outcome'] = 'rolled_back' if not failures else 'state_unknown_manual_recovery_required'
        return 2, operation / 'receipt.json' if operation else None
    finally:
        if lock is not None and lock.exists():
            try:
                if lock.read_bytes() == operation.name.encode():
                    lock.unlink()
            except OSError as error:
                record['lockCleanupError'] = str(error)
        if operation is not None:
            record['finishedAtUtc'] = datetime.now(timezone.utc).isoformat()
            write_receipt(operation, record)


def parser():
    """Define explicit identities, roots and immutable input hashes for publication."""
    result = argparse.ArgumentParser(description=__doc__)
    for flag in ('workspace-root', 'main', 'worktree', 'run-root', 'snapshot', 'snapshot-diff', 'allowlist'):
        result.add_argument('--' + flag, type=Path, required=True)
    for flag in ('baseline', 'candidate', 'snapshot-sha256', 'snapshot-diff-sha256', 'allowlist-sha256'):
        result.add_argument('--' + flag, required=True)
    result.add_argument('--expected-user-count', type=int, default=1891)
    result.add_argument('--mode', choices=('preflight', 'publish'), default='preflight')
    return result


def main():
    """Print only the resulting receipt path and explicit status, never user file data."""
    args = parser().parse_args()
    code, receipt = execute(args)
    print(json.dumps({'exitCode': code, 'receipt': str(receipt) if receipt else None}, ensure_ascii=False))
    return code


if __name__ == '__main__':
    sys.exit(main())
