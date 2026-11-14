"""Build and verify a restricted portable release using explicit file allowlists."""
#20261114-10 add: Freeze exact inputs and refuse links, overwrite and unlisted output.
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import stat
import subprocess
import sys


FORMAT = "KinSealLite-stage11-package-v1"
RUNTIME_FILES = (
    "D3Dcompiler_47.dll", "Qt6Core.dll", "Qt6Gui.dll", "Qt6Network.dll",
    "Qt6SerialPort.dll", "Qt6Svg.dll", "Qt6Widgets.dll",
    "generic/qtuiotouchplugin.dll", "iconengines/qsvgicon.dll",
    "imageformats/qgif.dll", "imageformats/qico.dll", "imageformats/qjpeg.dll",
    "imageformats/qsvg.dll", "libgcc_s_seh-1.dll", "libstdc++-6.dll",
    "libwinpthread-1.dll", "networkinformation/qnetworklistmanager.dll",
    "opengl32sw.dll", "platforms/qwindows.dll", "styles/qmodernwindowsstyle.dll",
    "tls/qcertonlybackend.dll", "tls/qschannelbackend.dll",
    "LZO-MIT-LICENSE.txt", "QR-MIT-LICENSE.txt",
)
DEFAULT_SAMPLES = (
    "samples/stage10/bit-lamp-local.ksp", "samples/stage10/word-lamp-local.ksp",
    "samples/stage10/function-local.ksp", "samples/stage1/stage1-single-page.ksp",
    "samples/stage5/ellipse-default.ksp", "samples/stage7/internal-five-family.ksp",
    "samples/stage8/ordinary-two-macros.ksp",
)
HELP_SUFFIXES = {".html", ".htm", ".md", ".txt", ".css", ".json", ".svg", ".png"}
LICENSE_SUFFIXES = {".txt", ".md", ".html", ".htm"}
TEXT_SUFFIXES = {".cpp", ".c", ".h", ".qrc", ".qss", ".svg", ".json", ".txt",
                 ".ui", ".kvg", ".xml", ".md", ".html", ".css", ".ps1", ".rc", ".py"}
SHA_RE = re.compile(r"[0-9a-f]{64}\Z")
MAX_MANIFEST = 16 * 1024 * 1024


def checked_path(value: str | Path, *, must_exist: bool = True) -> Path:  #20261114-10 add
    """Resolve an ordinary path after rejecting links in every existing ancestor."""
    path = Path(os.path.abspath(value))
    for part in path.parts[1:]:
        relative_path(part)
    for entry in reversed((path, *path.parents)):
        if entry.exists() or entry.is_symlink():
            details = entry.lstat()
            if stat.S_ISLNK(details.st_mode) or getattr(details, "st_file_attributes", 0) & 0x400:
                raise ValueError("Reparse/symlink path is forbidden: " + str(entry))
    if must_exist and not path.exists():
        raise ValueError("Input does not exist: " + str(path))
    return path


def regular_file(value: str | Path) -> Path:  #20261114-10 add
    """Require a regular file with one physical link before reading its bytes."""
    path = checked_path(value)
    details = path.stat()
    if not stat.S_ISREG(details.st_mode) or details.st_nlink != 1:
        raise ValueError("Input must be an ordinary unlinked file: " + str(path))
    return path


def sha_file(value: str | Path) -> str:  #20261114-10 add
    """Hash complete ordinary file bytes without newline or encoding conversion."""
    path = regular_file(value)
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def relative_path(value: str) -> str:  #20261114-10 add
    """Accept portable relative manifest paths without Windows alias components."""
    if not isinstance(value, str) or not value or "\\" in value or ":" in value:
        raise ValueError("Invalid portable relative path")
    parts = value.split("/")
    reserved = {"CON", "PRN", "AUX", "NUL", *("COM" + str(x) for x in range(1, 10)),
                *("LPT" + str(x) for x in range(1, 10))}
    if PurePosixPath(value).is_absolute() or any(
        not part or part in (".", "..") or part.endswith((" ", ".")) or
        any(ord(char) < 32 or char in '<>"|?*' for char in part) or
        part.split(".")[0].upper() in reserved for part in parts
    ):
        raise ValueError("Noncanonical manifest path: " + value)
    return value


def tree_files(value: str | Path) -> list[Path]:  #20261114-10 add
    """Enumerate sorted ordinary tree files while refusing links and case aliases."""
    root = checked_path(value)
    if not root.is_dir():
        raise ValueError("Expected a directory: " + str(root))
    result = []
    seen = set()
    for current, directories, names in os.walk(root, followlinks=False):
        for name in directories + names:
            entry = checked_path(Path(current) / name)
            relative = relative_path(entry.relative_to(root).as_posix())
            if relative.casefold() in seen:
                raise ValueError("Case-insensitive path collision: " + relative)
            seen.add(relative.casefold())
        result.extend(regular_file(Path(current) / name) for name in names)
    return sorted(result, key=lambda entry: entry.relative_to(root).as_posix())


def identity(path: Path, relative: str, kind: str | None = None) -> dict:  #20261114-10 add
    """Record a normalized relative identity and optional package-role label."""
    row = {"path": relative_path(relative), "bytes": regular_file(path).stat().st_size,
           "sha256": sha_file(path)}
    if kind is not None:
        row["kind"] = kind
    return row


def content_digest(rows: list[dict]) -> str:  #20261114-10 add
    """Bind ordered paths, lengths and byte digests with unambiguous JSON encoding."""
    payload = [{key: row[key] for key in ("path", "bytes", "sha256")} for row in rows]
    return hashlib.sha256(json.dumps(payload, ensure_ascii=False, sort_keys=True,
                                    separators=(",", ":")).encode("utf-8")).hexdigest()


def json_bytes(value: object) -> bytes:  #20261114-10 add
    """Encode stable UTF-8 JSON for manifests and evidence reports."""
    return (json.dumps(value, ensure_ascii=False, sort_keys=True, indent=2,
                       allow_nan=False) + "\n").encode("utf-8")


def load_json(path: Path) -> object:  #20261114-10 add
    """Reject oversized, ambiguous or nonfinite JSON before validating its schema."""
    def unique_pairs(pairs):
        """Build one object while refusing repeated property names."""
        result = {}
        for key, value in pairs:
            if key in result:
                raise ValueError("Duplicate JSON property: " + key)
            result[key] = value
        return result

    def reject_constant(value):
        """Reject nonfinite JSON tokens instead of accepting Python extensions."""
        raise ValueError("Nonfinite JSON token: " + value)

    def finite_float(value):
        """Accept finite JSON decimals while rejecting overflowing exponents."""
        number = float(value)
        if not math.isfinite(number):
            raise ValueError("Nonfinite JSON number")
        return number

    if regular_file(path).stat().st_size > MAX_MANIFEST:
        raise ValueError("JSON manifest exceeds input size limit")
    return json.loads(path.read_text(encoding="utf-8-sig"), object_pairs_hook=unique_pairs,
                      parse_constant=reject_constant, parse_float=finite_float)


def new_output(value: str | Path, *, require_z: bool = True) -> Path:  #20261114-10 add
    """Validate a fresh output without replacing any existing object or link."""
    path = checked_path(value, must_exist=False)
    if path.exists() or path.is_symlink():
        raise ValueError("Output already exists; nothing overwritten: " + str(path))
    if require_z and os.name == "nt" and path.drive.casefold() != "z:":
        raise ValueError("Generated package/evidence must be on Z drive")
    return path


def overlaps(left: Path, right: Path) -> bool:  #20261114-10 add
    """Compare canonical paths using Windows case-insensitive containment rules."""
    first = str(left).replace("\\", "/").rstrip("/").casefold()
    second = str(right).replace("\\", "/").rstrip("/").casefold()
    return first == second or first.startswith(second + "/") or second.startswith(first + "/")


def write_new(path: Path, data: bytes) -> None:  #20261114-10 add
    """Write exclusively to a new unlinked file under ordinary parent directories."""
    new_output(path, require_z=False)
    path.parent.mkdir(parents=True, exist_ok=True)
    checked_path(path.parent)
    with path.open("xb") as stream:
        stream.write(data)
    regular_file(path)


def source_rows(worktree: Path, help_root: Path) -> list[dict]:  #20261114-10 add
    """Freeze production/build/help sources without copying them into the user package."""
    paths = [worktree / "CMakeLists.txt", worktree / "Build.ps1",
             worktree / "tools/BuildStage11.ps1", worktree / "tools/package_stage11.py"]
    paths.extend(worktree.glob("*.qrc"))
    for directory in (worktree / "src", worktree / "resources", help_root):
        paths.extend(tree_files(directory))
    result = []
    for path in sorted(set(paths), key=lambda item: item.relative_to(worktree).as_posix()):
        result.append(identity(path, path.relative_to(worktree).as_posix()))
    return result


def validate_rows(rows: object) -> list[dict]:  #20261114-10 add
    """Validate a bounded, sorted, alias-free manifest identity list."""
    if not isinstance(rows, list) or not rows or len(rows) > 25000:
        raise ValueError("Invalid manifest identity list")
    seen = set()
    paths = []
    for row in rows:
        if not isinstance(row, dict):
            raise ValueError("Invalid manifest row")
        relative = relative_path(row.get("path"))
        if relative.casefold() in seen:
            raise ValueError("Duplicate/case-colliding manifest path")
        if type(row.get("bytes")) is not int or not 0 <= row["bytes"] <= 512 * 1024 * 1024:
            raise ValueError("Invalid byte count")
        if not isinstance(row.get("sha256"), str) or not SHA_RE.fullmatch(row["sha256"]):
            raise ValueError("Invalid SHA256")
        seen.add(relative.casefold())
        paths.append(relative)
    if paths != sorted(paths):
        raise ValueError("Manifest rows must have deterministic path order")
    return rows


def allowed_content(row: dict) -> bool:  #20261114-10 add
    """Ensure that even an edited manifest cannot expand the package file allowlist."""
    path = row["path"]
    kind = row.get("kind")
    if kind == "runtime":
        return path in RUNTIME_FILES
    if kind == "current-executable":
        return path == "KinSealStudioLite.exe"
    if kind == "verifier":
        return path == "tools/package_stage11.py"
    if kind == "user-guide":
        return path == "使用说明.md"
    if kind == "dependency-policy":
        return path == "release/stage11-dependencies.json"
    if kind == "sample":
        return path.startswith("samples/") and Path(path).suffix == ".ksp"
    if kind == "help":
        return path.startswith("help/") and Path(path).suffix.lower() in HELP_SUFFIXES
    if kind == "license":
        return path.startswith("licenses/") and Path(path).suffix.lower() in LICENSE_SUFFIXES
    return False


def check_dependency_policy(policy: object, rows: list[dict], runtime_manifest_sha: str) -> None:  #20261114-10 add
    """Bind conservative dependency policy and its complete declared license identities."""
    if (not isinstance(policy, dict) or policy.get("format") != "KinSealLite-stage11-dependency-policy-v1" or
            policy.get("releaseChannel") != "restricted_internal_validation" or
            policy.get("publicRedistribution") is not False or
            policy.get("baselineManifestSha256") != runtime_manifest_sha):
        raise ValueError("Dependency policy must bind restricted distribution and frozen runtime manifest")
    declared = policy.get("licenseFiles")
    if not isinstance(declared, list):
        raise ValueError("Dependency policy lacks complete license identities")
    license_rows = []
    for row in declared:
        if not isinstance(row, dict):
            raise ValueError("Invalid dependency license row")
        license_rows.append({key: row.get(key) for key in ("path", "bytes", "sha256")})
    license_rows.sort(key=lambda row: str(row["path"]))
    validate_rows(license_rows)
    actual = [{key: row[key] for key in ("path", "bytes", "sha256")}
              for row in rows if row.get("kind") == "license"]
    actual.sort(key=lambda row: row["path"])
    if license_rows != actual:
        raise ValueError("License content differs from dependency policy's exact complete inventory")


def verify_package(package: Path, expected_manifest_sha: str | None = None) -> dict:  #20261114-10 add
    """Check package bytes, complete file inventory, role allowlist and frozen digests."""
    package = checked_path(package)
    manifest_path = regular_file(package / "manifest.json")
    manifest_sha = sha_file(manifest_path)
    if expected_manifest_sha and manifest_sha != expected_manifest_sha.lower():
        raise ValueError("Manifest SHA256 differs from independently frozen identity")
    manifest = load_json(manifest_path)
    if (not isinstance(manifest, dict) or manifest.get("format") != FORMAT or
            manifest.get("stage") != 11 or manifest.get("distributionStatus") != "restricted_internal_validation" or
            manifest.get("publicRedistribution") is not False):
        raise ValueError("Unsupported or unsafe package policy manifest")
    rows = validate_rows(manifest.get("files"))
    sources = validate_rows(manifest.get("sourceFiles"))
    if any(not allowed_content(row) for row in rows):
        raise ValueError("Manifest contains a file outside package-role allowlists")
    if content_digest(rows) != manifest.get("fileContentSha256"):
        raise ValueError("Manifest content-list digest mismatch")
    if content_digest(sources) != manifest.get("sourceContentSha256"):
        raise ValueError("Manifest source-list digest mismatch")
    expected = {row["path"] for row in rows} | {"manifest.json"}
    actual = {path.relative_to(package).as_posix() for path in tree_files(package)}
    errors = []
    for path in sorted(actual - expected):
        errors.append({"path": path, "problem": "unexpected-file"})
    for row in rows:
        path = package / row["path"]
        if row["path"] not in actual:
            errors.append({"path": row["path"], "problem": "missing-file"})
        elif path.stat().st_size != row["bytes"] or sha_file(path) != row["sha256"]:
            errors.append({"path": row["path"], "problem": "byte-identity-mismatch"})
    executable = next((row for row in rows if row["path"] == "KinSealStudioLite.exe"), None)
    runtime_names = {row["path"] for row in rows if row.get("kind") == "runtime"}
    if executable is None or executable["sha256"] != manifest.get("executableSha256"):
        raise ValueError("Executable identity is absent or inconsistent")
    if runtime_names != set(RUNTIME_FILES):
        raise ValueError("Complete frozen runtime allowlist is required")
    kinds = [row.get("kind") for row in rows]
    if any(kinds.count(kind) != 1 for kind in ("current-executable", "verifier", "user-guide", "dependency-policy")):
        raise ValueError("Executable, verifier, user guide and dependency policy are required once")
    if not all(kind in kinds for kind in ("help", "license", "sample")):
        raise ValueError("Nonempty help, license and sample sets are required")
    if not errors:
        check_dependency_policy(load_json(package / "release/stage11-dependencies.json"), rows,
                                manifest.get("runtimeManifestSha256"))
    return {"format": FORMAT, "verified": not errors, "errors": errors,
            "manifestSha256": manifest_sha, "fileCountIncludingManifest": len(actual),
            "fileContentSha256": manifest["fileContentSha256"],
            "sourceContentSha256": manifest["sourceContentSha256"],
            "executableSha256": manifest["executableSha256"],
            "codeCommit": manifest.get("codeCommit"),
            "distributionStatus": manifest["distributionStatus"]}


def create_package(args) -> dict:  #20261114-10 add
    """Preflight explicit inputs, copy to new paths and retain diagnostic evidence on failure."""
    worktree = checked_path(args.worktree)
    runtime = checked_path(args.runtime)
    executable = regular_file(args.executable)
    help_root = checked_path(args.help)
    licenses = checked_path(args.licenses)
    dependencies = regular_file(args.dependencies)
    guide = regular_file(args.guide)
    package = new_output(args.package)
    evidence = new_output(args.evidence)
    if not worktree.is_dir() or not runtime.is_dir() or not help_root.is_relative_to(worktree):
        raise ValueError("Worktree/runtime directories and help inside worktree are required")
    if not executable.is_relative_to(worktree / "build"):
        raise ValueError("New executable must come from this worktree build directory")
    if sha_file(Path(__file__).absolute()) != sha_file(worktree / "tools/package_stage11.py"):
        raise ValueError("Invoked package builder differs from frozen worktree builder")
    if not SHA_RE.fullmatch(args.expected_exe_sha.lower()) or sha_file(executable) != args.expected_exe_sha.lower():
        raise ValueError("New executable differs from expected SHA256")
    runtime_manifest = regular_file(runtime / "manifest.json")
    if (not SHA_RE.fullmatch(args.runtime_manifest_sha.lower()) or
            sha_file(runtime_manifest) != args.runtime_manifest_sha.lower()):
        raise ValueError("Frozen runtime manifest SHA256 mismatch")
    old = load_json(runtime_manifest)
    old_rows = {row["path"]: row for row in validate_rows(old.get("files"))}
    if old.get("stage") != 9:
        raise ValueError("Runtime must be frozen Stage9 package")
    policy = load_json(dependencies)
    if not isinstance(policy, dict) or policy.get("format") != "KinSealLite-stage11-dependency-policy-v1":
        raise ValueError("Unsupported dependency policy input")
    inputs = []
    roots = [runtime, help_root, licenses, worktree / "src", worktree / "resources"]
    frozen = source_rows(worktree, help_root)
    def add(path, destination, kind):
        """Register one exact input file without permitting duplicate destinations."""
        row = identity(path, destination, kind)
        if any(item["row"]["path"].casefold() == destination.casefold() for item in inputs):
            raise ValueError("Duplicate package destination: " + destination)
        inputs.append({"row": row, "inputPath": str(path)})

    for relative in RUNTIME_FILES:
        path = runtime / relative
        row = identity(path, relative)
        if relative not in old_rows or any(row[key] != old_rows[relative][key] for key in ("bytes", "sha256")):
            raise ValueError("Runtime DLL/license differs from frozen manifest: " + relative)
        add(path, relative, "runtime")
    for path in tree_files(help_root):
        if path.suffix.lower() not in HELP_SUFFIXES:
            raise ValueError("Unlisted help file extension: " + str(path))
        add(path, "help/" + path.relative_to(help_root).as_posix(), "help")
    for path in tree_files(licenses):
        if path.suffix.lower() not in LICENSE_SUFFIXES:
            raise ValueError("Unlisted license file extension: " + str(path))
        add(path, "licenses/" + path.relative_to(licenses).as_posix(), "license")
    if not any(item["row"]["kind"] == "help" for item in inputs) or not any(
            item["row"]["kind"] == "license" for item in inputs):
        raise ValueError("Nonempty help and license directories are required")
    check_dependency_policy(policy, [item["row"] for item in inputs], args.runtime_manifest_sha.lower())
    for relative in args.sample or DEFAULT_SAMPLES:
        relative = relative_path(relative)
        if not relative.startswith("samples/") or Path(relative).suffix != ".ksp":
            raise ValueError("Samples must be explicit samples/*.ksp files")
        sample = worktree / relative
        # Only Lite JSON projects are shipped; native/opaque snapshots are excluded.
        model = load_json(sample)
        if not isinstance(model, dict) or model.get("format") != "KinSealStudio-Prototype":
            raise ValueError("Selected sample is not a Lite project")
        def reject_native(value):
            """Reject sample metadata that embeds opaque native source or snapshots."""
            if isinstance(value, dict):
                for key, child in value.items():
                    if any(word in key.casefold() for word in ("opaque", "snapshot", "nativesource", "nativepayload")):
                        raise ValueError("Opaque/native sample content is excluded")
                    reject_native(child)
            elif isinstance(value, list):
                for child in value:
                    reject_native(child)
        reject_native(model)
        add(sample, relative, "sample")
    add(executable, "KinSealStudioLite.exe", "current-executable")
    add(regular_file(Path(__file__).absolute()), "tools/package_stage11.py", "verifier")
    add(dependencies, "release/stage11-dependencies.json", "dependency-policy")
    add(guide, "使用说明.md", "user-guide")
    inputs.sort(key=lambda item: item["row"]["path"])
    validate_rows([item["row"] for item in inputs])
    for output in (package, evidence):
        if overlaps(output, package if output == evidence else evidence) or any(
                overlaps(output, root) for root in roots) or any(
                overlaps(output, Path(item["inputPath"])) for item in inputs):
            raise ValueError("Input/output trees overlap")
    evidence.mkdir(parents=True, exist_ok=False)
    run = {"recordedAtUtc": datetime.now(timezone.utc).isoformat(), "outcome": "started",
           "package": str(package), "worktree": str(worktree), "inputs": inputs,
           "runtimeSource": str(runtime), "runtimeManifestSha256": sha_file(runtime_manifest),
           "hardwareIo": False, "inputsDeleted": False}
    write_new(evidence / "inputs.json", json_bytes(run))
    try:
        package.mkdir(parents=True, exist_ok=False)
        for item in inputs:
            row = item["row"]
            destination = package / row["path"]
            destination.parent.mkdir(parents=True, exist_ok=True)
            checked_path(destination.parent)
            with regular_file(item["inputPath"]).open("rb") as source, destination.open("xb") as target:
                shutil.copyfileobj(source, target, 1024 * 1024)
            if identity(destination, row["path"]) != {key: row[key] for key in ("path", "bytes", "sha256")}:
                raise ValueError("Copied input changed: " + row["path"])
        # Recheck both packaged inputs and frozen production sources before publishing identity.
        for item in inputs:
            if sha_file(item["inputPath"]) != item["row"]["sha256"]:
                raise ValueError("Input changed during packaging: " + item["row"]["path"])
        if source_rows(worktree, help_root) != frozen:
            raise ValueError("Production/build/help source changed during packaging")
        rows = [item["row"] for item in inputs]
        manifest = {"format": FORMAT, "stage": 11, "applicationVersion": "0.11.0",
                    "distributionStatus": "restricted_internal_validation", "publicRedistribution": False,
                    "thirdPartyRightsComplete": False, "hardwareIo": False,
                    "vendorExecutableRedistributed": False, "sdkRedistributed": False,
                    "embeddedVendorResourceRights": "unknown", "codeCommit": None,
                    "codeCommitState": "awaiting-verified-production-source-commit",
                    "sourceFiles": frozen, "sourceContentSha256": content_digest(frozen),
                    "runtimeManifestSha256": args.runtime_manifest_sha.lower(),
                    "executableSha256": args.expected_exe_sha.lower(), "files": rows,
                    "fileContentSha256": content_digest(rows),
                    "fileContentHashExcludes": ["manifest.json"],
                    "timestampPolicy": "UTC execution metadata is outside deterministic package in evidence/run.json"}
        write_new(package / "manifest.json", json_bytes(manifest))
        report = verify_package(package)
        if not report["verified"]:
            raise ValueError("Post-copy verification failed: " + str(report["errors"]))
        run = {**report, "recordedAtUtc": datetime.now(timezone.utc).isoformat(),
               "outcome": "package_verified", "package": str(package), "hardwareIo": False,
               "filesBytes": sum(row["bytes"] for row in rows),
               "sampleCount": sum(row["kind"] == "sample" for row in rows)}
        write_new(evidence / "run.json", json_bytes(run))
        return run
    except Exception as error:
        write_new(evidence / "failure.json", json_bytes({"outcome": "failed_preserved",
                  "error": str(error), "package": str(package), "partialOutputPreserved": True,
                  "inputsDeleted": False, "recordedAtUtc": datetime.now(timezone.utc).isoformat()}))
        raise


def compare_packages(left: Path, right: Path) -> dict:  #20261114-10 add
    """Compare deterministic identities after independently verifying both complete packages."""
    first = verify_package(left)
    second = verify_package(right)
    return {"leftVerified": first["verified"], "rightVerified": second["verified"],
            "sameFileContent": first["fileContentSha256"] == second["fileContentSha256"],
            "sameSourceContent": first["sourceContentSha256"] == second["sourceContentSha256"],
            "sameExecutable": first["executableSha256"] == second["executableSha256"],
            "sameManifest": first["manifestSha256"] == second["manifestSha256"]}


def bind_commit(args) -> dict:  #20261114-10 add
    """Bind an existing matching Git source commit without staging or mutating Git history."""
    package = checked_path(args.package)
    worktree = checked_path(args.worktree)
    evidence = new_output(args.evidence)
    report = verify_package(package)
    if not report["verified"]:
        raise ValueError("Cannot bind a modified or incomplete package")
    manifest_path = package / "manifest.json"
    manifest = load_json(manifest_path)
    if overlaps(evidence, package) or any(overlaps(evidence, worktree / row["path"])
                                         for row in manifest["sourceFiles"]):
        raise ValueError("Binding evidence overlaps frozen input/package")
    if not re.fullmatch(r"[0-9a-fA-F]{40}", args.commit):
        raise ValueError("Commit binding requires a full existing 40-character Git commit ID")
    commit = subprocess.check_output(["git", "-C", str(worktree), "rev-parse", "--verify",
                                      args.commit + "^{commit}"], text=True).strip()
    with subprocess.Popen(["git", "-C", str(worktree), "cat-file", "--batch"],
                          stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as process:
        try:
            for row in manifest["sourceFiles"]:
                path = regular_file(worktree / row["path"])
                current = path.read_bytes()
                if sha_file(path) != row["sha256"]:
                    raise ValueError("Frozen source changed before commit binding: " + row["path"])
                process.stdin.write((commit + ":" + row["path"] + "\n").encode("utf-8"))
                process.stdin.flush()
                header = process.stdout.readline().split()
                if len(header) != 3 or header[1] != b"blob" or int(header[2]) > 512 * 1024 * 1024:
                    raise ValueError("Commit lacks frozen source blob: " + row["path"])
                blob = process.stdout.read(int(header[2]))
                if process.stdout.read(1) != b"\n":
                    raise ValueError("Incomplete Git batch source response")
                if current != blob and (path.suffix.lower() not in TEXT_SUFFIXES and path.name != "CMakeLists.txt" or
                                         current.replace(b"\r\n", b"\n") != blob.replace(b"\r\n", b"\n")):
                    raise ValueError("Commit production bytes differ: " + row["path"])
            process.stdin.close()
            if process.wait() != 0:
                raise ValueError("Git batch source check failed")
        finally:
            if process.poll() is None:
                process.kill()
    evidence.mkdir(parents=True, exist_ok=False)
    write_new(evidence / "previous-manifest.json", manifest_path.read_bytes())
    manifest["codeCommit"] = commit
    manifest["codeCommitState"] = "verified-real-commit-source-match-after-only-Git-EOL-normalization"
    temporary = new_output(package / ("manifest-bind-" + os.urandom(8).hex() + ".tmp"), require_z=False)
    write_new(temporary, json_bytes(manifest))
    # Only the verified manifest is replaced; payloads and history remain frozen.
    if sha_file(manifest_path) != report["manifestSha256"]:
        raise ValueError("Manifest changed concurrently; temporary retained")
    os.replace(temporary, manifest_path)
    result = {"commit": commit, "previousManifestSha256": report["manifestSha256"],
              "manifestSha256": sha_file(manifest_path), "sourceFilesVerified": len(manifest["sourceFiles"]),
              "gitMutated": False, "recordedAtUtc": datetime.now(timezone.utc).isoformat()}
    write_new(evidence / "commit-binding.json", json_bytes(result))
    return result


def main(argv: list[str] | None = None) -> int:  #20261114-10 add
    """Parse explicit release operations and return structured failure without deleting evidence."""
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    create = commands.add_parser("create")
    for name in ("worktree", "executable", "runtime", "licenses", "dependencies", "guide", "package", "evidence"):
        create.add_argument("--" + name, type=Path, required=True)
    create.add_argument("--help-dir", dest="help", type=Path, required=True)
    create.add_argument("--expected-exe-sha", required=True)
    create.add_argument("--runtime-manifest-sha", required=True)
    create.add_argument("--sample", action="append")
    verify = commands.add_parser("verify")
    verify.add_argument("--package", type=Path, required=True)
    verify.add_argument("--expected-manifest-sha")
    verify.add_argument("--report", type=Path)
    compare = commands.add_parser("compare")
    compare.add_argument("--left", type=Path, required=True)
    compare.add_argument("--right", type=Path, required=True)
    binding = commands.add_parser("bind-commit")
    for name in ("package", "worktree", "evidence"):
        binding.add_argument("--" + name, type=Path, required=True)
    binding.add_argument("--commit", required=True)
    args = parser.parse_args(argv)
    try:
        if args.command == "create":
            result = create_package(args)
        elif args.command == "verify":
            report = new_output(args.report) if args.report else None
            result = verify_package(args.package, args.expected_manifest_sha)
            if report:
                write_new(report, json_bytes(result))
        elif args.command == "compare":
            result = compare_packages(args.left, args.right)
        else:
            result = bind_commit(args)
        print(json.dumps(result, ensure_ascii=False, allow_nan=False))
        if args.command == "verify" and not result["verified"]:
            return 3
        if args.command == "compare" and not all(result.values()):
            return 3
        return 0
    except (ValueError, OSError, subprocess.SubprocessError, TypeError, KeyError, RecursionError) as error:
        print(json.dumps({"outcome": "rejected", "error": str(error), "inputsDeleted": False}, ensure_ascii=False))
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
