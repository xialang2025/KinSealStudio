"""Verify the final real-commit binding and freeze package metadata without application changes."""
#20261114-7 add: Keep historical acceptance identities and the two release-helper versions distinct.
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import shutil
import subprocess


def sha(path):
    """Input: file path. Return: exact SHA256."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def save(path, value):
    """Input: own metadata path/value. Return: None after JSON publication."""
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def main():
    """Input: None. Return: zero after final package identity and metadata are frozen."""
    worktree = Path(__file__).resolve().parents[3]
    evidence = Path(__file__).resolve().parent
    package = Path(r"Z:\Project\vscode_prj_temp\REA\KinSealStudio\dist\KinSealStudioLite-Stage8-macro-r1")
    manifest = json.loads((package / "manifest.json").read_text(encoding="utf-8"))
    expected = "da8fccee0e615270dfcdf638cd74133fc36a1181dec48da73f0d6a7d2c12ce91"
    commit = "fcbc196f9779718fc8ea314cec9d0a5f4e6d62e8"
    assert manifest["codeCommit"] == commit and len(manifest["sourceFiles"]) == 6331
    actual = sorted(path.relative_to(package).as_posix() for path in package.rglob("*") if path.is_file())
    assert actual == sorted([row["path"] for row in manifest["files"]] + ["manifest.json"])
    assert len(actual) == 191
    assert all(sha(package / row["path"]) == row["sha256"]
               and (package / row["path"]).stat().st_size == row["bytes"]
               and (package / row["path"]).stat().st_nlink == 1 for row in manifest["files"])
    assert (package / "manifest.json").stat().st_nlink == 1
    assert sha(package / "KinSealStudioLite.exe") == sha(worktree / "build/KinSealStudioLite.exe") == expected
    assert all((package / name).read_bytes() == (worktree / name).read_bytes() for name in
               ("samples/stage8/ClearLB1.c", "samples/stage8/SetLB1.c", "samples/stage8/ordinary-two-macros.ksp"))
    original = subprocess.check_output(["git", "-C", str(worktree), "cat-file", "blob",
                                        commit + ":tools/package_stage8.py"])
    current = (worktree / "tools/package_stage8.py").read_bytes()
    old_token = b'".ui")'
    new_token = b'".ui", ".kvg", ".xml")'
    assert original.count(old_token) == 1 and original.replace(old_token, new_token) == current
    new_helper = sha(worktree / "tools/package_stage8.py")
    old_helper = hashlib.sha256(original).hexdigest()
    receipt_path = evidence / "commit-binding.json"
    receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
    receipt.update(
        applicationCodeCommit=commit, packageFilesVerified=191, allPackageFilesHaveOneLink=True,
        executableSha256=expected, sourceContentSha256=manifest["sourceContentSha256"],
        sourceFilesWithGitEolNormalization=sum(row["sha256"] != row["gitBlobSha256"] for row in manifest["sourceFiles"]),
        sourceOrBinaryOrSampleContentModified=False,
        helperRevision=dict(originalCommittedHelperSha256=old_helper, newHelperSha256=new_helper,
                            onlyAddedTextSuffixes=[".kvg", ".xml"],
                            reason="All relevant source differences independently verified UTF8 without NUL and CRLF/LF-only; release helper omitted these textual resource extensions.",
                            affectsApplicationBinaryOrInput=False,
                            intendedMetadataCommit="parent-managed subsequent commit"),
        failedFirstAttempt="binding-helper-attempt-r1.json", eolAnalysis="binding-eol-analysis.json")
    save(receipt_path, receipt)
    inputs_path = evidence / "inputs.json"
    inputs = json.loads(inputs_path.read_text(encoding="utf-8"))
    inputs["initialCodeCommit"] = inputs["codeCommit"]
    inputs["initialCodeCommitState"] = inputs["codeCommitState"]
    inputs["codeCommit"] = commit
    inputs["codeCommitState"] = manifest["codeCommitState"]
    inputs["bindingReceipt"] = "commit-binding.json"
    save(inputs_path, inputs)
    history = evidence / "readonly-review-before-binding.json"
    assert not history.exists()
    shutil.copyfile(evidence / "readonly-review.json", history)
    review = json.loads((evidence / "readonly-review.json").read_text(encoding="utf-8"))
    package_record = json.loads((evidence / "package.json").read_text(encoding="utf-8"))
    review["verdict"] = "便携包内容、私有桌面启动与普通宏外部SDK编译通过；6331源/资源及191包文件已绑定真实程序提交"
    review["codeCommitState"] = manifest["codeCommitState"]
    review["codeCommit"] = commit
    review["remaining"] = []
    review["finalPackage"] = package_record
    review["bindingReceipt"] = "commit-binding.json"
    review["builder"]["originalCommittedSha256"] = old_helper
    review["builder"]["sha256"] = new_helper
    review["builder"]["revisionScope"] = "仅发布工具文本EOL白名单新增.kvg/.xml；由父代理后续metadata提交，不改变App binary/input"
    review["reviewRevision"] = "final-real-commit-bound"
    review["failureHistory"].append(dict(
        kind="发布工具EOL白名单遗漏", evidence="binding-helper-attempt-r1.json",
        detail="首轮保守拒绝.kvg，未写manifest；全量分析确认6331源无冻结后变化，5791个Git差异全部UTF8无NUL且仅CRLF/LF。工具仅增加.kvg/.xml白名单后全量绑定成功。原工具版本和新工具SHA分别保留，应用生产提交仍fcbc196f。"))
    for check in review["checks"]:
        if check["topic"] == "真实提交绑定":
            check.update(result="通过",
                         detail="真实应用程序提交fcbc196f9779718fc8ea314cec9d0a5f4e6d62e8；冻结源6331项逐项核验通过，仅5791项Git EOL差异；包191文件/主EXE/当前样例再验均通过。发布工具仅修正.kvg/.xml文本EOL白名单，不改变App binary/input，回执同时保留原/新helper SHA。")
    save(evidence / "readonly-review.json", review)
    freeze = dict(
        schemaVersion=1, codeCommit=commit, sourceFilesVerified=6331, packageFilesVerified=191,
        samples=164, packageBytes=package_record["packageBytes"],
        manifestSha256=sha(package / "manifest.json"), executableSha256=expected,
        sourceContentSha256=manifest["sourceContentSha256"], fileContentSha256=manifest["fileContentSha256"],
        helperSha256=new_helper, originalCommittedHelperSha256=old_helper,
        bindingReceiptSha256=sha(receipt_path), readonlyReviewSha256=sha(evidence / "readonly-review.json"),
        inputsSha256=sha(inputs_path), smokeR3Passed=True, smokeR3Processes=2, smokeR3SemanticChecks=4,
        fullCliHistoricalPassed=True, fullCliProcesses=24, fullCliSemanticChecks=31,
        productionOrSamplesChanged=False, gitMutated=False)
    save(evidence / "final-freeze.json", freeze)
    print(json.dumps(freeze, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

