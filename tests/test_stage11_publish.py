"""Exercise publication against small owned Git repositories, never the user's repo."""
#20261114-10 add: Preserve conflict, rollback and two-overlap proof cases as real Git tests.
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import stat
import subprocess
import tempfile
import unittest
from unittest import mock

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('stage11_publish_under_test', ROOT / 'tools/publish_stage11.py')
PUBLISH = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PUBLISH)


class PublicationTests(unittest.TestCase):
    """Create independent repositories with known user edits and a detached candidate."""
    @classmethod
    def setUpClass(cls):
        supplied = os.environ.get('KINSEAL_PUBLICATION_TEST_ROOT')
        if not supplied:
            raise unittest.SkipTest('Set KINSEAL_PUBLICATION_TEST_ROOT to a dedicated Z-drive test directory')
        cls.owned_root = Path(supplied).resolve()
        if os.name == 'nt' and cls.owned_root.drive.upper() == 'C:':
            raise RuntimeError('Publication tests must not consume the C drive')
        cls.owned_root.mkdir(parents=True, exist_ok=True)
        label = os.environ.get('KINSEAL_PUBLICATION_TEST_LABEL', 'r2')
        if not label.isalnum():
            raise RuntimeError('Use a simple exclusive test receipt label')
        cls.evidence = ROOT / 'docs/stage11-evidence/publication-owned/test-receipts' / label
        cls.evidence.mkdir(parents=True, exist_ok=True)

    def git(self, repo, *args):
        result = subprocess.run(['git', '-c', 'core.quotepath=false', '-C', str(repo), *args], stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=30)
        self.assertEqual(result.returncode, 0, result.stderr.decode('utf-8', errors='replace'))
        return result.stdout

    def setUp(self):
        self.fixture = Path(tempfile.mkdtemp(prefix='case-', dir=self.owned_root)).resolve()
        self.assertTrue(self.fixture.is_relative_to(self.owned_root) and self.fixture != self.owned_root)
        self.main_repo = self.fixture / 'main'
        self.work = self.fixture / 'candidate'
        self.run = self.fixture / 'runs'
        self.main_repo.mkdir()
        self.git(self.main_repo, 'init', '-b', 'main')
        self.git(self.main_repo, 'config', 'user.name', 'Stage11 Synthetic Test')
        self.git(self.main_repo, 'config', 'user.email', 'stage11-test@example.invalid')
        self.git(self.main_repo, 'config', 'core.autocrlf', 'false')
        self.lines = [f'base-line-{i:02}'.encode() for i in range(16)]
        self.b_name = 'b-中文.txt' if self._testMethodName.startswith('test_unicode') else 'b.txt'
        self.base_a = b'\xef\xbb\xbf' + b'\r\n'.join(self.lines) + b'\r\n'
        self.base_b = b'\n'.join(self.lines) + b'\n'
        (self.main_repo / 'a.txt').write_bytes(self.base_a)
        (self.main_repo / self.b_name).write_bytes(self.base_b)
        self.git(self.main_repo, 'add', '--', 'a.txt', self.b_name)
        self.git(self.main_repo, 'commit', '-m', 'synthetic baseline')
        self.base = self.git(self.main_repo, 'rev-parse', 'HEAD').decode().strip()
        self.git(self.main_repo, 'worktree', 'add', '--detach', str(self.work), self.base)
        self.user_a = self.base_a.replace(b'base-line-01', b'user-line-A1')
        self.user_b = self.base_b.replace(b'base-line-02', b'user-line-B2')
        (self.main_repo / 'a.txt').write_bytes(self.user_a)
        (self.main_repo / self.b_name).write_bytes(self.user_b)
        (self.main_repo / 'user-only.bin').write_bytes(b'\x00original-user-data\xff')
        self.freeze_snapshot()
        self.make_candidate()

    def freeze_snapshot(self):
        statuses = PUBLISH.status(self.main_repo)
        files = {}
        for name, code in statuses.items():
            raw = (self.main_repo / name).read_bytes()
            files[name] = {'status': code, 'bytes': len(raw), 'sha256': hashlib.sha256(raw).hexdigest()}
        self.snapshot = self.fixture / 'protection.json'
        self.snapshot.write_bytes(PUBLISH.json_bytes({'head': self.base, 'files': files, 'mainIndexEmpty': True}))
        self.diff = self.fixture / 'protection.diff'
        self.diff.write_bytes(self.git(self.main_repo, 'diff', '--binary', self.base, '--'))

    def make_candidate(self, conflict=False, include_user=False):
        task_a = self.base_a.replace(b'base-line-01' if conflict else b'base-line-11', b'task-line-A')
        task_b = self.base_b.replace(b'base-line-12', b'task-line-B')
        if include_user:
            task_a = task_a.replace(b'base-line-01', b'user-line-A1')
        (self.work / 'a.txt').write_bytes(task_a)
        (self.work / self.b_name).write_bytes(task_b)
        (self.work / 'docs').mkdir(exist_ok=True)
        (self.work / 'docs/stage11-new.txt').write_bytes(b'owned new release document\n')
        self.git(self.work, 'add', '--', 'a.txt', self.b_name, 'docs/stage11-new.txt')
        self.git(self.work, 'commit', '--amend' if getattr(self, 'candidate', None) else '-m',
                 '--no-edit' if getattr(self, 'candidate', None) else 'synthetic task candidate')
        self.candidate = self.git(self.work, 'rev-parse', 'HEAD').decode().strip()
        self.allow = self.fixture / 'allow.json'
        self.allow.write_bytes(PUBLISH.json_bytes({'paths': ['a.txt', self.b_name, 'docs/stage11-new.txt']}))

    def arguments(self, mode='publish'):
        return argparse.Namespace(workspace_root=self.fixture, main=self.main_repo, worktree=self.work, run_root=self.run,
                                  baseline=self.base, candidate=self.candidate, snapshot=self.snapshot, snapshot_diff=self.diff,
                                  allowlist=self.allow, snapshot_sha256=hashlib.sha256(self.snapshot.read_bytes()).hexdigest(),
                                  snapshot_diff_sha256=hashlib.sha256(self.diff.read_bytes()).hexdigest(),
                                  allowlist_sha256=hashlib.sha256(self.allow.read_bytes()).hexdigest(), expected_user_count=3, mode=mode)

    def perform(self, mode='publish', fault=None, args=None):
        code, receipt = PUBLISH.execute(args or self.arguments(mode), fault=fault)
        self.assertIsNotNone(receipt)
        record = json.loads(receipt.read_text(encoding='utf-8'))
        return code, record, receipt

    def assert_original(self):
        self.assertEqual(self.git(self.main_repo, 'rev-parse', 'HEAD').decode().strip(), self.base)
        self.assertEqual((self.main_repo / 'a.txt').read_bytes(), self.user_a)
        self.assertEqual((self.main_repo / self.b_name).read_bytes(), self.user_b)
        self.assertEqual((self.main_repo / 'user-only.bin').read_bytes(), b'\x00original-user-data\xff')
        self.assertFalse((self.main_repo / 'docs/stage11-new.txt').exists())
        self.assertFalse(self.git(self.main_repo, 'diff', '--cached', '--name-only', '--'))

    def tearDown(self):
        receipts = []
        if self.run.exists():
            for source in sorted(self.run.glob('publication-*/receipt.json')):
                receipts.append(json.loads(source.read_text(encoding='utf-8')))
        target = self.evidence / (self._testMethodName + '.json')
        captured = {'test': self._testMethodName, 'synthetic': True,
                    'fixturePath': str(self.fixture), 'fixtureRemovedAfterCapture': False,
                    'receipts': receipts}
        target.write_bytes(PUBLISH.json_bytes(captured))
        resolved = self.fixture.resolve(strict=True)
        self.assertTrue(resolved.is_relative_to(self.owned_root) and resolved != self.owned_root)
        def clear_owned_readonly(function, path, error_info):
            owned = Path(path).resolve(strict=True)
            self.assertTrue(owned.is_relative_to(resolved) and owned != resolved)
            self.assertFalse(Path(path).is_symlink())
            os.chmod(owned, stat.S_IWRITE | stat.S_IREAD)
            function(owned)
        shutil.rmtree(resolved, onerror=clear_owned_readonly)
        captured['fixtureRemovedAfterCapture'] = not resolved.exists()
        target.write_bytes(PUBLISH.json_bytes(captured))

    def test_preflight_two_disjoint_overlaps_changes_nothing(self):
        code, record, _ = self.perform('preflight')
        self.assertEqual(code, 0)
        self.assertEqual(record['outcome'], 'preflight_passed')
        self.assertEqual(len(record['overlaps']), 2)
        self.assertTrue(all(x['reverseTaskRestoresUserRawExactly'] and x['orderedUserDeltaUnchanged'] for x in record['overlaps']))
        self.assert_original()

    def test_publish_retains_user_delta_and_bom_crlf(self):
        code, record, receipt = self.perform()
        self.assertEqual(code, 0, record.get('error'))
        self.assertEqual(record['outcome'], 'published')
        self.assertEqual(self.git(self.main_repo, 'rev-parse', 'HEAD').decode().strip(), self.candidate)
        self.assertIn(b'user-line-A1', (self.main_repo / 'a.txt').read_bytes())
        self.assertIn(b'task-line-A', (self.main_repo / 'a.txt').read_bytes())
        self.assertTrue((self.main_repo / 'a.txt').read_bytes().startswith(b'\xef\xbb\xbf'))
        self.assertNotIn(b'\n', (self.main_repo / 'a.txt').read_bytes().replace(b'\r\n', b''))
        self.assertFalse(self.git(self.main_repo, 'diff', '--cached', '--name-only', '--'))
        after = json.loads((receipt.parent / 'after-protection-snapshot.json').read_text(encoding='utf-8'))
        self.assertEqual(after['head'], self.candidate)
        self.assertEqual(len(after['files']), 3)
        self.assertNotEqual(after['files']['a.txt']['sha256'], json.loads(self.snapshot.read_text())['files']['a.txt']['sha256'])
        self.assertEqual(after['files']['user-only.bin'], json.loads(self.snapshot.read_text())['files']['user-only.bin'])

    def test_conflict_rejected_before_main_write(self):
        self.make_candidate(conflict=True)
        code, record, _ = self.perform()
        self.assertEqual(code, 2)
        self.assertIn('merge conflict', record['error'])
        self.assertFalse(record['mainMutated'])
        self.assert_original()

    def test_candidate_containing_user_edit_rejected(self):
        self.make_candidate(include_user=True)
        code, record, _ = self.perform()
        self.assertEqual(code, 2)
        self.assertIn('exact original user bytes', record['error'])
        self.assert_original()

    def test_main_head_change_rejected(self):
        self.git(self.main_repo, 'commit', '--allow-empty', '-m', 'external head moved')
        moved = self.git(self.main_repo, 'rev-parse', 'HEAD')
        code, record, _ = self.perform()
        self.assertEqual(code, 2)
        self.assertIn('HEAD moved', record['error'])
        self.assertEqual(self.git(self.main_repo, 'rev-parse', 'HEAD'), moved)
        self.assertEqual((self.main_repo / 'a.txt').read_bytes(), self.user_a)

    def test_main_index_change_rejected(self):
        self.git(self.main_repo, 'add', '--', 'a.txt')
        staged_before = self.git(self.main_repo, 'diff', '--cached', '--binary')
        code, record, _ = self.perform()
        self.assertEqual(code, 2)
        self.assertIn('index is not empty', record['error'])
        self.assertEqual(self.git(self.main_repo, 'diff', '--cached', '--binary'), staged_before)

    def test_protected_user_bytes_change_rejected(self):
        (self.main_repo / 'a.txt').write_bytes(self.user_a + b'later user edit\r\n')
        later = (self.main_repo / 'a.txt').read_bytes()
        code, record, _ = self.perform()
        self.assertEqual(code, 2)
        self.assertIn('Protected user bytes changed', record['error'])
        self.assertEqual((self.main_repo / 'a.txt').read_bytes(), later)

    def test_failure_after_ref_update_restores_index_ref_and_files(self):
        def fault(phase):
            if phase == 'after_ref':
                raise OSError('synthetic failure after atomic ref update')
        code, record, _ = self.perform(fault=fault)
        self.assertEqual(code, 2)
        self.assertEqual(record['outcome'], 'rolled_back', record)
        self.assertTrue(record['rollback']['confirmed'])
        self.assert_original()

    def test_foreign_edit_during_failure_is_preserved_and_marked_unknown(self):
        def fault(phase):
            if phase == 'after_file:a.txt':
                (self.main_repo / 'a.txt').write_bytes(b'foreign edit during transaction\n')
                raise OSError('synthetic concurrent user write')
        code, record, _ = self.perform(fault=fault)
        self.assertEqual(code, 2)
        self.assertEqual(record['outcome'], 'state_unknown_manual_recovery_required')
        self.assertFalse(record['rollback']['confirmed'])
        self.assertEqual((self.main_repo / 'a.txt').read_bytes(), b'foreign edit during transaction\n')
        self.assertEqual((self.main_repo / self.b_name).read_bytes(), self.user_b)

    def test_snapshot_and_allowlist_identity_guards(self):
        args = self.arguments()
        args.snapshot_sha256 = '0' * 64
        code, record, _ = self.perform(args=args)
        self.assertEqual(code, 2)
        self.assertIn('Input hash mismatch', record['error'])
        self.assert_original()
        self.allow.write_bytes(PUBLISH.json_bytes({'paths': ['a.txt']}))
        code, record, _ = self.perform()
        self.assertEqual(code, 2)
        self.assertIn('changed-path set differs', record['error'])
        self.assert_original()

    def test_protection_snapshot_can_publish_later_metadata_commit(self):
        code, record, receipt = self.perform()
        self.assertEqual(code, 0, record.get('error'))
        retained_user_a = (self.main_repo / 'a.txt').read_bytes()
        old_candidate = self.candidate
        (self.work / 'docs/stage11-metadata.txt').write_bytes(b'next metadata only\n')
        self.git(self.work, 'add', '--', 'docs/stage11-metadata.txt')
        self.git(self.work, 'commit', '-m', 'synthetic followup metadata')
        self.candidate = self.git(self.work, 'rev-parse', 'HEAD').decode().strip()
        self.base = old_candidate
        self.snapshot = receipt.parent / 'after-protection-snapshot.json'
        self.diff = receipt.parent / 'after-protection-snapshot.diff'
        self.allow.write_bytes(PUBLISH.json_bytes({'paths': ['docs/stage11-metadata.txt']}))
        code, second, _ = self.perform()
        self.assertEqual(code, 0, second.get('error'))
        self.assertEqual(second['outcome'], 'published')
        self.assertEqual((self.main_repo / 'a.txt').read_bytes(), retained_user_a)
        self.assertEqual(len(second['overlaps']), 0)

    def test_unicode_quoted_frozen_diff_keeps_full_headers_and_chain(self):
        self.diff.write_bytes(self.git(self.main_repo, '-c', 'core.quotepath=true', 'diff', '--binary', self.base, '--'))
        self.assertNotIn(self.b_name.encode(), self.diff.read_bytes())
        code, record, receipt = self.perform()
        self.assertEqual(code, 0, record.get('error'))
        self.assertEqual(record['protectedDiffRepresentation']['selectedQuotepath'], 'true')
        self.assertEqual(record['protectedDiffRepresentation']['matchingQuotepathValues'], ['true'])
        after = json.loads((receipt.parent / 'after-protection-snapshot.json').read_text(encoding='utf-8'))
        self.assertEqual(after['snapshotDiffQuotepath'], 'true')
        self.assertEqual((receipt.parent / 'after-protection-snapshot.diff').read_bytes(),
                         self.git(self.main_repo, '-c', 'core.quotepath=true', 'diff', '--binary', self.candidate, '--'))

    def test_unicode_unquoted_frozen_diff_keeps_utf8_headers(self):
        self.assertIn(self.b_name.encode(), self.diff.read_bytes())
        code, record, receipt = self.perform()
        self.assertEqual(code, 0, record.get('error'))
        self.assertEqual(record['protectedDiffRepresentation']['selectedQuotepath'], 'false')
        self.assertEqual(record['protectedDiffRepresentation']['matchingQuotepathValues'], ['false'])
        after = json.loads((receipt.parent / 'after-protection-snapshot.json').read_text(encoding='utf-8'))
        self.assertEqual(after['snapshotDiffQuotepath'], 'false')
        self.assertIn(self.b_name.encode(), (receipt.parent / 'after-protection-snapshot.diff').read_bytes())

    def test_unicode_diff_content_tamper_rejected_despite_valid_input_hash(self):
        frozen = self.diff.read_bytes()
        self.assertIn(b'+user-line-B2', frozen)
        self.diff.write_bytes(frozen.replace(b'+user-line-B2', b'+forged-line-B2'))
        code, record, _ = self.perform()
        self.assertEqual(code, 2)
        self.assertIn('diff differs from frozen input', record['error'])
        self.assertFalse(record['mainMutated'])
        self.assert_original()

    def test_unicode_prior_declared_quotepath_is_not_silently_changed(self):
        prior = json.loads(self.snapshot.read_text(encoding='utf-8'))
        prior['snapshotDiffQuotepath'] = 'true'
        self.snapshot.write_bytes(PUBLISH.json_bytes(prior))
        code, record, _ = self.perform()
        self.assertEqual(code, 2)
        self.assertIn('declared quotepath representation', record['error'])
        self.assert_original()

    def test_scan_budgets_cover_status_working_diff_and_cached_diff(self):
        completed = subprocess.CompletedProcess([], 0, b' M a.txt\0?? extra.txt\0', b'')
        with mock.patch.object(PUBLISH.subprocess, 'run', return_value=completed) as process:
            observed = PUBLISH.status(self.main_repo)
            self.assertEqual(observed, {'a.txt': ' M', 'extra.txt': '??'})
            self.assertEqual(process.call_count, 1)
            self.assertEqual(process.call_args.kwargs['timeout'], 600)
            self.assertEqual(process.call_args.args[0][-4:], ['status', '--porcelain=v1', '-z', '--untracked-files=all'])
            PUBLISH.git(self.main_repo, 'rev-parse', 'HEAD')
            self.assertEqual(process.call_count, 2)
            self.assertEqual(process.call_args.kwargs['timeout'], 120)
            PUBLISH.working_tree_diff(self.main_repo, self.base, quote_paths=True)
            self.assertEqual(process.call_count, 3)
            self.assertEqual(process.call_args.kwargs['timeout'], 600)
            self.assertEqual(process.call_args.args[0][-4:], ['diff', '--binary', self.base, '--'])
            PUBLISH.git(self.main_repo, 'diff', '--cached', '--name-only', '-z', '--')
            self.assertEqual(process.call_count, 4)
            self.assertEqual(process.call_args.kwargs['timeout'], 120)
            PUBLISH.git(self.main_repo, 'diff', '--name-status', '--no-renames', '-z', self.base, self.candidate, '--')
            self.assertEqual(process.call_args.kwargs['timeout'], 120)


if __name__ == '__main__':
    unittest.main()
