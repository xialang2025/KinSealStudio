"""Generate the S00-04 reconciliation view without editing the maintained ledger."""

import argparse
from collections import Counter, defaultdict
from functools import lru_cache
from hashlib import sha256
import json
from pathlib import Path
import re
import subprocess


ROOT = Path(__file__).resolve().parents[1]
DESIGN = ROOT / "design_plan"
LEDGER = DESIGN / "未完成任务总台账.json"
ADOPTIONS = DESIGN / "未完成任务证据/history-atomic-adoptions-20261119.json"
REPORT = DESIGN / "S00-04历史原子逐项对账.md"
LABELS = {
    "historical_closed": "沿用历史已闭",
    "evidence_closed": "按既有同层证据核销",
    "partial": "具体证据部分覆盖",
    "open": "待单项证据",
}


def require(condition, message):  # //20261119-0 add
    """Input: an integrity condition and diagnostic. Return: None or fail."""
    if not condition:
        raise ValueError(message)


def by_id(records, key="id"):
    """Input: ledger records and key name. Return: unique records by ID."""
    result = {record[key]: record for record in records}
    require(len(result) == len(records), f"Duplicate {key}")
    return result


def table(headers, rows):
    """Input: headings and cells. Return: Markdown table lines."""
    yield "| " + " | ".join(headers) + " |"
    yield "|" + "|".join("---" for _ in headers) + "|"
    for row in rows:
        cells = (str(value).replace("|", "&#124;").replace("\r", "").replace("\n", "<br>") for value in row)
        yield "| " + " | ".join(cells) + " |"
    yield ""


@lru_cache(maxsize=None)
def committed_source(commit, path):
    """Input: a frozen commit and repository-relative path. Return: raw source bytes."""
    require(re.fullmatch(r"[0-9a-f]{40}", commit) is not None, f"Unpinned commit {commit}")
    require(path.startswith(("docs/", "design_plan/")) and ".." not in Path(path).parts, f"Invalid source {path}")
    return subprocess.run(
        ["git", "show", f"{commit}:{path}"], cwd=ROOT, check=True, stdout=subprocess.PIPE
    ).stdout


def adopted_evidence(evidence, atoms):
    """Input: reviewed evidence and known atoms. Return: validated evidence by atom."""
    commit = evidence["evidenceCommit"]
    sources = evidence["sources"]
    reviewed = by_id(evidence["adoptions"])
    for atomic_id, item in reviewed.items():
        require(atomic_id in atoms, f"Unknown atomic ID {atomic_id}")
        atom = atoms[atomic_id]
        require(atom["currentStatus"] == "current_review_required", f"{atomic_id} was already closed")
        source = sources[item["source"]]
        layers = set(item["layers"])
        require(layers and layers <= set(atom["blockingLayers"]), f"Wrong evidence layer: {atomic_id}")
        require(item["conclusion"] and source["identity"], f"Missing scope: {atomic_id}")
        if item["status"] == "evidence_closed":
            require(
                layers == set(atom["blockingLayers"]) and source["kind"] == "native"
                and all(layer in "DUPRC" for layer in layers) and not item["remaining"],
                f"Incomplete basis for closure: {atomic_id}",
            )
        else:
            require(item["status"] == "partial" and item["remaining"], f"Missing residual gap: {atomic_id}")
        raw = committed_source(commit, source["path"])
        lines = raw.decode("utf-8-sig").splitlines()
        matched = [i for i, line in enumerate(lines, 1) if item["anchor"] in line]
        require(len(matched) == 1, f"Evidence quote missing or ambiguous: {atomic_id}")
        item.update(path=source["path"], identity=source["identity"], line=matched[0], digest=sha256(raw).hexdigest())
    return reviewed


def reconcile(ledger, evidence):
    """Input: ledger and reviewed evidence. Return: atom results, parent rollups and U routing."""
    history = ledger["history"]
    atoms = by_id(history["historyAtomicIssues"])
    parents = by_id(history["historyParentIssues"])
    tasks = by_id(ledger["stageTasks"])
    families = by_id(ledger["families"], "familyId")
    questions = by_id(ledger["questions"])
    candidates = {item["row"]["线索ID"]: item for item in ledger["candidates"]}
    require(len(candidates) == len(ledger["candidates"]), "Duplicate U ID")
    issue_index = atoms | parents
    require(len(issue_index) == len(atoms) + len(parents), "Atomic and parent IDs overlap")
    require(len(atoms) == ledger["counts"]["historyAtomicIssues"] == 613 and len(parents) == 186, "History count drift")
    require(len(families) == 41 and len(tasks) == 90, "Family or stage task count drift")
    require(all(f"U-{i:03}" in candidates for i in range(1, 309)), "Missing frozen U ID")
    current_links = defaultdict(list)
    for item in questions.values():
        for related_id in item["relatedIds"]:
            current_links[related_id].append(item["id"])

    reviewed = adopted_evidence(evidence, atoms)
    prior_closed = by_id(ledger["closedFacts"], "qId")
    outcomes = {}
    used_candidates = defaultdict(list)
    for identifier, node in issue_index.items():
        for candidate in node["sourceCandidateIds"]:
            user_id = candidate.removeprefix("CAND-20261110/")
            require(user_id in candidates, f"Unknown U ID in {identifier}")
            used_candidates[user_id].append(identifier)
        for parent_id in node["parentIds"]:
            require(parent_id in parents and identifier in parents[parent_id]["children"], f"Nonreciprocal parent link: {identifier}")
        for child_id in node["children"]:
            require(child_id in issue_index and identifier in issue_index[child_id]["parentIds"], f"Nonreciprocal child link: {identifier}")

    for atom in atoms.values():
        identifier = atom["id"]
        family = families[atom["familyMapping"]["id"]]
        require(not atom["children"], f"Atomic issue has children: {identifier}")
        require(atom["currentStatus"] in {"current_review_required", "historical_fact_closed"}, f"Unknown atom status: {identifier}")
        require(set(atom["blockingLayers"]) <= set(family["layers"]), f"Unknown layer for {identifier}")
        require(set(atom["relatedTaskIds"]) <= set(tasks), f"Unknown task in {identifier}")
        candidate_q = [
            related for source in atom["sourceCandidateIds"]
            for related in candidates[source.split("/")[-1]]["relatedIds"]
        ]
        proposed_q = [*current_links[identifier], *atom["legacyQuestionIds"], *candidate_q, *family["openIssueIds"]]
        linked = list(dict.fromkeys(item for item in proposed_q if item in questions))
        # Family Q links are routing clues only. They never change a historical status.
        status = "open"
        detail = atom["historicalStimulus"].rsplit(" 对象：", 1)[0]
        if identifier in reviewed:
            item = reviewed[identifier]
            status = item["status"]
            detail = item["remaining"] or "见本轮采用证据。"
        if atom["currentStatus"] == "historical_fact_closed":
            require(len(atom["legacyQuestionIds"]) == 1 and atom["legacyQuestionIds"][0] in prior_closed, f"Unbound legacy fact: {identifier}")
            status = "historical_closed"
            detail = atom["historicalClosureScope"]["not_covered"]
        outcomes[identifier] = (status, detail, linked)
    require(sum(item[0] == "historical_closed" for item in outcomes.values()) == len(prior_closed) == 3, "Legacy closures changed")
    require(all(used_candidates[f"U-{i:03}"] for i in range(1, 308)), "Frozen issue U IDs lost")
    require(not used_candidates.get("U-308") and "非问题" in candidates["U-308"]["disposition"], "U-308 changed")

    @lru_cache(maxsize=None)
    def leaves(identifier, stack=()):
        """Input: issue ID and ancestor path. Return: unique leaf atomic IDs."""
        require(identifier not in stack, f"Cycle found in historical tree: {identifier}")
        if identifier in atoms:
            return frozenset({identifier})
        children = parents[identifier]["children"]
        require(children and len(children) == len(set(children)), f"Missing or duplicate child: {identifier}")
        return frozenset().union(*(leaves(child, (*stack, identifier)) for child in children))

    parent_results = {}
    for parent in parents.values():
        descendants = leaves(parent["id"])
        counts = Counter(outcomes[item][0] for item in descendants)
        total_closed = counts["historical_closed"] + counts["evidence_closed"]
        state = "各原子均已核销" if total_closed == len(descendants) else "仍开放"
        parent_results[parent["id"]] = (state, counts)
    return atoms, parents, outcomes, parent_results, reviewed, used_candidates


def canonical_digest(document):
    """Input: JSON document. Return: a digest independent of checkout newlines and key order."""
    encoded = json.dumps(document, ensure_ascii=False, sort_keys=True, separators=(",", ":"))
    return sha256(encoded.encode("utf-8")).hexdigest()


def load_ledger_reconciliation(ledger):  # //20261119-0 add
    """Input: the maintained ledger. Return: the validated current overlay or None."""
    reference = ledger.get("historyAtomicReconciliation")
    if reference is None:
        return None
    require(reference["evidencePath"] == ADOPTIONS.relative_to(ROOT).as_posix(), "Unknown reconciliation evidence path")
    require(reference["reportPath"] == REPORT.relative_to(ROOT).as_posix(), "Unknown reconciliation report path")
    evidence = json.loads(ADOPTIONS.read_text(encoding="utf-8-sig"))
    require(reference["evidenceCanonicalSha256"] == canonical_digest(evidence), "Reconciliation evidence hash mismatch")
    result = reconcile(ledger, evidence)
    counts = Counter(outcome[0] for outcome in result[2].values())
    require(dict(counts) == reference["atomicCounts"], "Current reconciliation counts do not match evidence")
    return result


def render(ledger, evidence, ledger_digest, result):
    """Input: source identities and reconciliation. Return: reproducible Markdown."""
    atoms, parents, outcomes, parent_results, reviewed, used_candidates = result
    counts = Counter(item[0] for item in outcomes.values())
    unresolved = counts["partial"] + counts["open"]
    open_parents = sum(state == "仍开放" for state, _ in parent_results.values())
    lines = [
        "# S00-04 历史原子逐项对账", "",
        "整理日期 2026-11-19（北京时间）。本轮逐 ID 确定证据索引、采用结论或后续验证路径。本表为总台账当前核销状态的逐条证据视图。", "",
        f"- 台账源码基线 `{ledger['sourceCommit']}`，台账规范化 JSON SHA-256 `{ledger_digest}`。",
        f"- 冻结候选 `{ledger['csvIdentity']['frozenOriginalIdentity']}` 的源 CSV SHA-256 `{ledger['csvIdentity']['frozen308Sha256']}`。",
        f"- 复用证据的固定 Git 提交 `{evidence['evidenceCommit']}`。签入的报告用提交、仓库路径、行号定位，逐项核对原句。工作目录本地删改不影响引用。",
        f"- 613 原子中沿用历史关闭 {counts['historical_closed']}，本轮按既有证据核销 {counts['evidence_closed']}，部分覆盖 {counts['partial']}，待单项证据 {counts['open']}。仍有 {unresolved} 个原子分支。",
        f"- 186 父项按递归去重后的叶子汇总，{open_parents} 个仍开放。U-001～U-307 的历史去向均保留；U-308 继续作为说明文字排除。图形 10 族留在当前台账，历史 613 原子均属于 31 控件族。",
        "- 原厂证据只用于对应 V5.1 范围与所在层。阶段 10 实屏观测保留固件身份、刺激和覆盖限制。KSP、原厂 SM、离线 C、USB T 和实物 E 分别核定。",
        "- `待单项证据` 表示未在本轮采用命中该分支的证据。该项的家族或当前 Q 仅为后续入口，不能据此判定功能缺失。",
        "- 未重新运行原厂、编译器或实物。本轮没有新增测试通过结论。详细证据输入见[采用清单](未完成任务证据/history-atomic-adoptions-20261119.json)。", "",
        "## 分族进度", "",
    ]
    family_counts = defaultdict(Counter)
    for identifier, atom in atoms.items():
        family_counts[atom["familyMapping"]["id"]][outcomes[identifier][0]] += 1
    lines.extend(table(
        ["家族", "原子总数", *LABELS.values()],
        ([item["familyId"] + " " + item["name"], sum(count.values()), *(count[key] for key in LABELS)]
         for item in ledger["families"] if (count := family_counts.get(item["familyId"]))),
    ))
    lines.extend(["## 采用的具体证据", ""])
    lines.extend(table(
        ["历史ID", "覆盖层", "采用结论", "范围身份", "签入证据", "源 SHA-256"],
        ([identifier, "/".join(item["layers"]), item["conclusion"], item["identity"],
          f"`{item['path']}:{item['line']}`", f"`{item['digest']}`"]
         for identifier, item in reviewed.items()),
    ))
    lines.extend(["## 613 原子逐条结果", ""])
    for family in ledger["families"]:
        family_id = family["familyId"]
        if family_id not in family_counts:
            continue
        lines.extend([f"### {family_id} {family['name']}", "",
                      f"家族证据入口（索引） `{family['report']}`。", ""])
        rows = []
        for atom in atoms.values():
            if atom["familyMapping"]["id"] != family_id:
                continue
            identifier = atom["id"]
            status, detail, linked = outcomes[identifier]
            if status == "historical_closed":
                detail = "/".join(atom["legacyQuestionIds"]) + "；未覆盖 " + detail
            rows.append([
                identifier, "/".join(item.split("/")[-1] for item in atom["sourceCandidateIds"]),
                atom["question"], "/".join(atom["blockingLayers"]), LABELS[status], detail,
                "、".join([*atom["relatedTaskIds"], *linked]),
            ])
        lines.extend(table(["历史ID", "冻结候选", "原子问题", "要求层", "当前处理", "未覆盖或核对动作", "关联S／当前Q索引"], rows))
    lines.extend(["## 186 父项按子项汇总", ""])
    lines.extend(table(
        ["父ID", "冻结候选", "直接子ID", "状态", *LABELS.values()],
        ([identifier, "/".join(p.split("/")[-1] for p in parent["sourceCandidateIds"]), "、".join(parent["children"]),
          parent_results[identifier][0], *(parent_results[identifier][1][key] for key in LABELS)]
         for identifier, parent in parents.items()),
    ))
    lines.extend([
        "## 复现", "",
        "`python -B tools/reconcile_history_atomic.py --check` 可验证当前文档与输入相符。",
        "改动采用范围需更新采用清单并重新核对原句。命令默认仅生成本页，不写总台账或冻结候选。",
        f"本次验证固定候选 308 条，其中 {len(used_candidates)} 条问题来源保留历史 ID，另有 U-308 说明文字。",
    ])
    return "\n".join(lines) + "\n"


def main():
    """Input: optional --check. Return: CLI status and record counts."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Verify the generated report without modifying it.")
    args = parser.parse_args()
    raw_ledger = LEDGER.read_bytes()
    ledger = json.loads(raw_ledger.decode("utf-8-sig"))
    evidence = json.loads(ADOPTIONS.read_text(encoding="utf-8-sig"))
    result = load_ledger_reconciliation(ledger)
    require(result is not None, "Add the reviewed reconciliation pointer to the maintained ledger")
    report = render(ledger, evidence, canonical_digest(ledger), result)
    if args.check:
        require(REPORT.read_text(encoding="utf-8") == report, "The S00-04 report is stale")
    else:
        REPORT.write_bytes(report.encode("utf-8"))
    print(json.dumps(Counter(item[0] for item in result[2].values()), sort_keys=True))


if __name__ == "__main__":
    main()
