"""Exercise release identity, isolation and tamper gates with disposable tiny fixtures."""
#20261114-10 add: Use only self-owned synthetic inputs; no runtime or hardware is launched.
from __future__ import annotations

import argparse
from contextlib import redirect_stdout
import importlib.util
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from unittest.mock import patch


TOOL = Path(__file__).resolve().parents[1] / "tools/package_stage11.py"
SPEC = importlib.util.spec_from_file_location("package_stage11_under_test", TOOL)
PACKAGE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PACKAGE)


class PackageTests(unittest.TestCase):  #20261114-10 add
    """Validate file contracts and negative inputs independently of production binaries."""

    def setUp(self):
        """Create an isolated source tree, frozen runtime and explicit tiny release inputs."""
        self.temporary = tempfile.TemporaryDirectory(prefix="stage11-package-test-")
        self.root = Path(self.temporary.name)
        self.worktree = self.root / "worktree"
        self.runtime = self.root / "old-runtime"
        self.help = self.worktree / "help"
        self.licenses = self.root / "licenses"
        self.write(self.worktree / "CMakeLists.txt", b"project(test)\n")
        self.write(self.worktree / "Build.ps1", b"# build\n")
        self.write(self.worktree / "tools/BuildStage11.ps1", b"# isolated build\n")
        self.write(self.worktree / "tools/package_stage11.py", TOOL.read_bytes())
        self.write(self.worktree / "src/main.cpp", b"int main() { return 0; }\n")
        self.write(self.worktree / "src/version.rc", b'FILEVERSION 0,11,0,0\n')
        self.write(self.worktree / "resources/not-for-redistribution.sm", b"opaque-source")
        self.write(self.help / "index.html", b"<html>Authored help</html>\n")
        self.write(self.licenses / "LICENSE.txt", b"Test license\n")
        self.write(self.worktree / "build/KinSealStudioLite.exe", b"NEW-EXE\0")
        self.write(self.root / "guide.md", "使用说明\n".encode("utf-8"))
        for relative in PACKAGE.DEFAULT_SAMPLES:
            self.write_json(self.worktree / relative, {
                "format": "KinSealStudio-Prototype", "x": 1.25, "objects": []})
        rows = []
        for relative in PACKAGE.RUNTIME_FILES:
            self.write(self.runtime / relative, ("runtime-" + relative).encode("utf-8"))
            rows.append(PACKAGE.identity(self.runtime / relative, relative))
        self.write(self.runtime / "KinSealStudioLite.exe", b"OLD-EXE-MUST-NOT-COPY")
        self.write(self.runtime / "unlisted-sdk.exe", b"DO-NOT-COPY")
        self.write_json(self.runtime / "manifest.json", {"stage": 9, "files": sorted(rows, key=lambda row: row["path"])})
        self.write_json(self.root / "dependencies.json", {
            "format": "KinSealLite-stage11-dependency-policy-v1",
            "releaseChannel": "restricted_internal_validation", "publicRedistribution": False,
            "baselineManifestSha256": PACKAGE.sha_file(self.runtime / "manifest.json"),
            "licenseFiles": [PACKAGE.identity(self.licenses / "LICENSE.txt", "licenses/LICENSE.txt")],
        })
        self.args = argparse.Namespace(
            worktree=self.worktree, executable=self.worktree / "build/KinSealStudioLite.exe",
            expected_exe_sha=PACKAGE.sha_file(self.worktree / "build/KinSealStudioLite.exe"),
            runtime=self.runtime, runtime_manifest_sha=PACKAGE.sha_file(self.runtime / "manifest.json"),
            help=self.help, licenses=self.licenses, dependencies=self.root / "dependencies.json",
            guide=self.root / "guide.md", package=self.root / "new-package", evidence=self.root / "evidence",
            sample=None)

    def tearDown(self):
        """Remove only this fixture's verified self-owned temporary root."""
        self.temporary.cleanup()

    def write(self, path, data):
        """Create one fixture file under its caller-owned root."""
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)

    def write_json(self, path, value):
        """Create stable JSON bytes for a fixture or deliberately changed manifest."""
        self.write(path, PACKAGE.json_bytes(value))

    def create(self):
        """Build a tiny package using the real public creation function."""
        return PACKAGE.create_package(self.args)

    def read_manifest(self):
        """Read the created manifest for targeted negative-fixture mutations."""
        return PACKAGE.load_json(self.args.package / "manifest.json")

    def test_complete_package_keeps_only_allowlisted_inputs(self):
        """Exclude native source, old executable and unlisted SDK while preserving exact new bytes."""
        report = self.create()
        self.assertTrue(report["verified"])
        self.assertEqual((self.args.package / "KinSealStudioLite.exe").read_bytes(), b"NEW-EXE\0")
        self.assertFalse((self.args.package / "resources").exists())
        self.assertFalse((self.args.package / "unlisted-sdk.exe").exists())
        self.assertEqual(report["sampleCount"], 7)
        manifest = self.read_manifest()
        self.assertFalse(manifest["publicRedistribution"])
        self.assertTrue(any(row["path"] == "resources/not-for-redistribution.sm" for row in manifest["sourceFiles"]))
        self.assertFalse(any("inputPath" in row for row in manifest["files"]))

    def test_repeated_packages_are_content_and_manifest_identical(self):
        """Keep timestamps and absolute execution paths outside the reproducible package."""
        self.create()
        first = self.args.package
        self.args.package = self.root / "second-package"
        self.args.evidence = self.root / "second-evidence"
        self.create()
        self.assertTrue(all(PACKAGE.compare_packages(first, self.args.package).values()))

    def test_existing_package_is_preserved(self):
        """Never overwrite an existing caller package directory or content."""
        self.write(self.args.package / "user.txt", b"user")
        with self.assertRaisesRegex(ValueError, "already exists"):
            self.create()
        self.assertEqual((self.args.package / "user.txt").read_bytes(), b"user")
        self.assertFalse(self.args.evidence.exists())

    def test_existing_evidence_is_preserved(self):
        """Reject a used evidence destination before creating a package."""
        self.write(self.args.evidence / "user.txt", b"user")
        with self.assertRaises(ValueError):
            self.create()
        self.assertFalse(self.args.package.exists())

    def test_wrong_executable_hash_is_preflight_failure(self):
        """Refuse stale or substituted executable inputs before writing output."""
        self.args.expected_exe_sha = "0" * 64
        with self.assertRaisesRegex(ValueError, "executable differs"):
            self.create()
        self.assertFalse(self.args.package.exists())

    def test_runtime_byte_tamper_is_preflight_failure(self):
        """Compare every selected old runtime DLL against the frozen old manifest."""
        self.write(self.runtime / PACKAGE.RUNTIME_FILES[-3], b"tampered")
        with self.assertRaisesRegex(ValueError, "differs from frozen"):
            self.create()
        self.assertFalse(self.args.package.exists())

    def test_runtime_manifest_identity_is_required(self):
        """Reject a replaced old manifest even if its DLL rows were edited too."""
        self.args.runtime_manifest_sha = "0" * 64
        with self.assertRaisesRegex(ValueError, "manifest SHA256 mismatch"):
            self.create()

    def test_missing_runtime_file_is_not_silently_omitted(self):
        """Require the full pinned runtime allowlist rather than whatever directory enumeration returns."""
        (self.runtime / PACKAGE.RUNTIME_FILES[2]).unlink()
        with self.assertRaises(ValueError):
            self.create()

    def test_sample_path_traversal_is_rejected(self):
        """Do not accept sample paths escaping the declared source root."""
        self.args.sample = ["samples/../../guide.md"]
        with self.assertRaises(ValueError):
            self.create()

    def test_opaque_sample_is_rejected(self):
        """Exclude JSON projects carrying opaque native source snapshots."""
        self.write_json(self.worktree / PACKAGE.DEFAULT_SAMPLES[0], {
            "format": "KinSealStudio-Prototype", "nativeSourceSnapshot": "base64"})
        with self.assertRaisesRegex(ValueError, "Opaque/native"):
            self.create()

    def test_non_lite_sample_is_rejected(self):
        """Reject an arbitrary JSON document renamed as a project."""
        self.write_json(self.worktree / PACKAGE.DEFAULT_SAMPLES[0], {"format": "Vendor"})
        with self.assertRaisesRegex(ValueError, "not a Lite"):
            self.create()

    def test_extra_help_executable_is_rejected(self):
        """A recursively supplied help tree cannot smuggle an executable into the package."""
        self.write(self.help / "hidden.exe", b"not-help")
        with self.assertRaisesRegex(ValueError, "help file extension"):
            self.create()

    def test_empty_licenses_are_rejected(self):
        """Do not emit a release with a missing attribution directory."""
        (self.licenses / "LICENSE.txt").unlink()
        with self.assertRaisesRegex(ValueError, "Nonempty help and license"):
            self.create()

    def test_license_identity_is_bound_to_dependency_inventory(self):
        """Reject a changed attribution file even when it is still an allowed text filename."""
        self.write(self.licenses / "LICENSE.txt", b"substituted license")
        with self.assertRaisesRegex(ValueError, "License content differs"):
            self.create()

    def test_public_distribution_policy_is_rejected(self):
        """Cannot promote uncertain third-party rights through edited machine policy metadata."""
        policy = PACKAGE.load_json(self.args.dependencies)
        policy["publicRedistribution"] = True
        self.write_json(self.args.dependencies, policy)
        with self.assertRaisesRegex(ValueError, "restricted distribution"):
            self.create()

    def test_dependency_baseline_must_match_runtime_manifest(self):
        """Tie dependency declarations to this selected frozen runtime rather than another package."""
        policy = PACKAGE.load_json(self.args.dependencies)
        policy["baselineManifestSha256"] = "0" * 64
        self.write_json(self.args.dependencies, policy)
        with self.assertRaisesRegex(ValueError, "frozen runtime manifest"):
            self.create()

    def test_input_output_subtree_overlap_is_rejected(self):
        """Prevent writing output inside input help and recursively ingesting it."""
        self.args.package = self.help / "nested-package"
        with self.assertRaisesRegex(ValueError, "overlap"):
            self.create()
        self.assertFalse(self.args.package.exists())

    def test_evidence_package_overlap_is_rejected(self):
        """Evidence cannot be hidden inside the delivered package tree."""
        self.args.evidence = self.args.package / "evidence"
        with self.assertRaisesRegex(ValueError, "overlap"):
            self.create()

    def test_external_executable_is_rejected(self):
        """A source-root name alone cannot authorize an executable outside its build tree."""
        self.args.executable = self.runtime / "KinSealStudioLite.exe"
        self.args.expected_exe_sha = PACKAGE.sha_file(self.args.executable)
        with self.assertRaisesRegex(ValueError, "this worktree build"):
            self.create()

    def test_file_tamper_and_extra_file_are_reported(self):
        """Return actionable byte mismatch and extra-file diagnostics in one verification."""
        self.create()
        self.write(self.args.package / "Qt6Core.dll", b"tampered")
        self.write(self.args.package / "unexpected.exe", b"unexpected")
        report = PACKAGE.verify_package(self.args.package)
        self.assertFalse(report["verified"])
        self.assertEqual({row["problem"] for row in report["errors"]}, {"unexpected-file", "byte-identity-mismatch"})

    def test_missing_file_is_reported(self):
        """List missing payloads without substituting their manifest declaration for evidence."""
        self.create()
        (self.args.package / "Qt6Core.dll").unlink()
        report = PACKAGE.verify_package(self.args.package)
        self.assertIn({"path": "Qt6Core.dll", "problem": "missing-file"}, report["errors"])

    def test_independently_frozen_manifest_hash_detects_manifest_edit(self):
        """Detect coordinated content and manifest replacement with the external manifest identity."""
        original = self.create()["manifestSha256"]
        manifest = self.read_manifest()
        manifest["codeCommit"] = "a" * 40
        self.write_json(self.args.package / "manifest.json", manifest)
        with self.assertRaisesRegex(ValueError, "independently frozen"):
            PACKAGE.verify_package(self.args.package, original)

    def test_manifest_duplicate_json_keys_are_rejected(self):
        """Reject ambiguous JSON rather than silently selecting the last repeated key."""
        self.create()
        self.write(self.args.package / "manifest.json", b'{"format":"one","format":"two"}')
        with self.assertRaisesRegex(ValueError, "Duplicate JSON"):
            PACKAGE.verify_package(self.args.package)

    def test_nonfinite_numbers_are_rejected(self):
        """Catch both nonstandard NaN and overflowing legal JSON exponent syntax."""
        for text in (b'{"x":NaN}', b'{"x":1e999}'):
            self.write(self.root / "bad.json", text)
            with self.assertRaisesRegex(ValueError, "Nonfinite"):
                PACKAGE.load_json(self.root / "bad.json")

    def test_manifest_path_and_case_aliases_are_rejected(self):
        """Reject Windows ADS, reserved names, path traversal and casing collisions."""
        for name in ("../x", "samples//x", "samples/x:secret", "samples/NUL.txt", "samples/x. ", "C:/x"):
            with self.subTest(name=name), self.assertRaises(ValueError):
                PACKAGE.relative_path(name)
        with self.assertRaisesRegex(ValueError, "Duplicate/case"):
            PACKAGE.validate_rows([{"path": "A", "bytes": 1, "sha256": "a" * 64},
                                   {"path": "a", "bytes": 1, "sha256": "a" * 64}])

    def test_hardlinked_input_is_rejected(self):
        """Refuse an input alias that could mutate another physical source file."""
        os.link(self.runtime / "Qt6Core.dll", self.root / "hardlink.dll")
        with self.assertRaisesRegex(ValueError, "unlinked file"):
            self.create()

    def test_hardlinked_output_is_rejected(self):
        """Verify checks physical file identity as well as matching bytes."""
        self.create()
        os.link(self.args.package / "Qt6Core.dll", self.root / "hardlink.dll")
        with self.assertRaisesRegex(ValueError, "unlinked file"):
            PACKAGE.verify_package(self.args.package)

    def test_symlink_parent_is_rejected_when_supported(self):
        """Do not resolve a caller path through a linked ancestor into another tree."""
        alias = self.root / "help-alias"
        try:
            alias.symlink_to(self.help, target_is_directory=True)
        except OSError as error:
            if os.name != "nt":
                self.skipTest("Symlink privilege unavailable: " + str(error))
            junction = subprocess.run(["cmd.exe", "/d", "/c", "mklink", "/J", str(alias), str(self.help)],
                                      stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                      creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
            if junction.returncode:
                self.skipTest("Symlink and junction creation unavailable: " + str(error))
        with self.assertRaisesRegex(ValueError, "Reparse/symlink"):
            PACKAGE.checked_path(alias / "index.html")

    def test_copy_failure_retains_diagnostic_and_inputs(self):
        """Keep partial new output and failure evidence without deleting any existing input."""
        with patch.object(PACKAGE.shutil, "copyfileobj", side_effect=OSError("injected copy failure")):
            with self.assertRaisesRegex(OSError, "injected"):
                self.create()
        self.assertTrue(self.args.package.exists())
        self.assertEqual(PACKAGE.load_json(self.args.evidence / "failure.json")["outcome"], "failed_preserved")
        self.assertEqual((self.runtime / "Qt6Core.dll").read_bytes(), b"runtime-Qt6Core.dll")

    def test_successful_git_binding_and_changed_source_rejection(self):
        """Bind only real matching Git source bytes and preserve payload content identities."""
        self.create()
        subprocess.run(["git", "init", "-q", str(self.worktree)], check=True)
        subprocess.run(["git", "-C", str(self.worktree), "add", "."], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        subprocess.run(["git", "-C", str(self.worktree), "-c", "user.name=Stage11 Test",
                        "-c", "user.email=stage11-test@example.invalid", "commit", "-qm", "fixture"], check=True)
        commit = subprocess.check_output(["git", "-C", str(self.worktree), "rev-parse", "HEAD"], text=True).strip()
        args = argparse.Namespace(package=self.args.package, worktree=self.worktree, commit=commit,
                                  evidence=self.root / "binding-evidence")
        before = PACKAGE.verify_package(self.args.package)["fileContentSha256"]
        result = PACKAGE.bind_commit(args)
        self.assertEqual(result["commit"], commit)
        self.assertEqual(PACKAGE.verify_package(self.args.package)["fileContentSha256"], before)
        self.write(self.worktree / "src/main.cpp", b"changed\n")
        args.evidence = self.root / "bad-binding-evidence"
        with self.assertRaisesRegex(ValueError, "Frozen source changed"):
            PACKAGE.bind_commit(args)
        self.assertFalse(args.evidence.exists())

    def test_cli_reports_tamper_with_exit_three(self):
        """The supported CLI separates valid tampered packages from malformed inputs."""
        self.create()
        self.write(self.args.package / "extra.txt", b"extra")
        output = io.StringIO()
        with redirect_stdout(output):
            result = PACKAGE.main(["verify", "--package", str(self.args.package)])
        self.assertEqual(result, 3)
        self.assertFalse(json.loads(output.getvalue())["verified"])


if __name__ == "__main__":
    unittest.main(verbosity=2)
