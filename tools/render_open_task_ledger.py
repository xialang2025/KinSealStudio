"""Render the maintained open-task JSON as one searchable Markdown ledger.

Input: design_plan/未完成任务总台账.json. Output: the adjacent Markdown file.
The frozen audit inputs remain evidence; subsequent work updates the JSON records.
"""

from pathlib import Path
import argparse
import csv
import gzip
import hashlib
import json
import re


ROOT = Path(__file__).resolve().parents[1]
JSON_PATH = ROOT / "design_plan/未完成任务总台账.json"
MD_PATH = JSON_PATH.with_suffix(".md")


def cell(value):  # //20261114-4 add
    """Input: a JSON value. Return: safe, complete Markdown table text."""
    if value is None:
        return "—"
    if isinstance(value, (list, tuple)):
        return "；".join(cell(item) for item in value) or "—"
    if isinstance(value, dict):
        return "；".join(str(key) + "：" + cell(item) for key, item in value.items())
    return str(value).replace("|", "&#124;").replace("\r", "").replace("\n", "<br>")


def source_link(source):
    """Input: a normalized source record. Return: its file and line locator."""
    if isinstance(source, str):
        source = {"path": source}
    path = source.get("path", source.get("file", ""))
    if not path:
        return cell(source)
    line = source.get("line", source.get("startLine", ""))
    label = Path(path).name + (":" + str(line) if line else "")
    target = path if re.match(r"^[A-Za-z]:/", path) else "../" + path
    label = ("本地未提交：" if source.get("sourceScope") == "local_uncommitted" else "") + label
    return "[" + label + "](<" + target + ">)"


def sources(records):
    """Input: source records. Return: every distinct file/line reference."""
    if isinstance(records, dict):
        records = [records]
    return "；".join(dict.fromkeys(source_link(record) for record in records or [])) or "—"


def render(data):
    """Input: the maintained ledger. Return: a complete human-readable view."""
    out = []

    def line(text=""):
        out.append(text)

    def heading(level, text):
        line(); line(("#" * level + " " + text).rstrip()); line()

    def table(headers, rows):
        line("| " + " | ".join(headers) + " |")
        line("|" + "|".join("---" for _ in headers) + "|")
        for row in rows:
            line("| " + " | ".join(cell(value) for value in row) + " |")
        line()

    def details(label, value):
        line("- " + label + "：" + cell(value))

    line("# KinSealStudio 未完成任务总台账")
    line()
    line("更新：" + data["updatedAt"] + "。已发布源码基线：`" + data["sourceCommit"] + "`。本次为文档核对，未执行构建、原厂UI、USB、PLC或实屏测试。")
    line()
    line("本文件是未完成事项的统一查看入口；同名JSON为逐条维护源。旧报告、原CSV和机器执行历史继续作为证据保留。任务编号、问题编号、原始候选和历史视图分别计数，不能相加计算任务总数或完成率。")
    line()
    line("导航：[当前范围](#当前范围与计数) · [90项主任务](#各阶段主任务全量) · [41族](#控件与图形的分层边界) · [当前问题](#当前正式问题与聚合缺口) · [历史未核销记录](#历史记录全量保留) · [318条候选](#原始候选与后续追加行全量) · [扩展与维护](#本地续作之外的扩展与维护) · [维护规则](#后续更新与核销规则)")
    heading(2, "当前范围与计数")
    for rule in data["interpretationRules"]:
        line("- " + rule)
    line()
    stats = data["counts"]
    table(["层次", "数量", "含义"], [
        ["阶段主任务", stats["stageTasks"], "阶段0～11共12阶段；下表逐项保留已过范围及剩余，不按控件数推算"],
        ["主任务仍有欠项/待证", stats["activeOpenTasks"], "按S-ID去重；不是全新未实现功能数"],
        ["既定切片已过主任务", stats["closedDefinedScopeTasks"], "仅相应定义范围通过；关联Q及后续切片仍可能开放"],
        ["控件/图形家族", stats["families"], "31控件＋9基础图形＋1纯图像表格"],
        ["当前正式及复核补充Q", stats["currentQuestions"], "Q按ID去重，包含父项；闭合事实另表保留"],
        ["跨阶段聚合事项", stats["aggregateItems"], "关联当前S/Q的恢复视图；与主任务和Q不累加"],
        ["原始U候选", stats["originalCandidates"], "U-001～U-308，U-308为说明文字；候选不是已确认缺失功能"],
        ["当前CSV全部行", stats["candidateRows"], "原308＋后续10；不改变旧快照身份"],
        ["阶段0历史原子", stats["historyAtomicIssues"], "历史开放待对账与旧关闭事实分别标识"],
        ["其它历史视图", cell(stats["historicalViews"]), "父问题/入口/字段为不同追踪视图，与原子或候选重叠；不累加"],
        ["独立扩展与维护", stats["extensions"], "LT779及文档/清理维护；不计入90项AMT主任务"],
    ])
    heading(3, "当前恢复顺序")
    for item in data["resumeOrder"]:
        line("- " + item)
    heading(2, "各阶段主任务全量")
    line("每项包含已完成边界、剩余内容、原因/依赖、下一步及可判断验收条件。标为切片已过的任务仍保留，便于确认不会重复实施。")
    for stage in range(12):
        tasks = [task for task in data["stageTasks"] if task["stage"] == stage]
        heading(3, "阶段" + str(stage) + "（" + str(len(tasks)) + "项）")
        for task in tasks:
            heading(4, task["id"] + " " + task["title"].removeprefix(task["id"] + " "))
            details("状态", task["status"])
            details("已完成范围", task["completedScope"])
            details("剩余内容", task["remaining"])
            details("原因/依赖", task["blockers"])
            details("下一步", task["nextAction"])
            details("验收条件", task["acceptance"])
            details("关联问题/明细", task["itemIds"])
            line("- 依据：" + sources(task["sources"]))
    heading(2, "控件与图形的分层边界")
    line("D=调查/文档，U=编辑交互，P=持久化（KSP与SM须区分），R=预览/像素，C=设备编译，T=实物传输，E=实屏显示/触控/PLC运行。部分通过仅限所写范围。G03是真曲线/圆弧，G05圆角矩形，G06圆/椭圆，G07扇形。")
    for family in data["families"]:
        heading(3, family["familyId"] + " " + family["name"])
        details("已交付编辑范围", family["editorProvenScope"])
        table(["层", "已证范围与剩余边界"], [[key, layer["boundary"]] for key, layer in family["layers"].items()])
        details("剩余Q", family["openIssueIds"] + family.get("sharedIssueIds", []))
        line("依据：" + sources(family["sources"]))
    heading(2, "当前正式问题与聚合缺口")
    line("这里逐条登记当前Q和跨阶段聚合项。带“待对账”的旧问题不自动变为确认未实现。多项共用依赖按原编号关联，未新增重复的整控件待做项。")
    for question in data["questions"]:
        heading(3, question["id"] + " " + question.get("title", ""))
        details("状态/范围", [question["status"], question.get("scope", "")])
        if question.get("completedScope"):
            details("已完成边界", question["completedScope"])
        details("剩余内容", question["remaining"])
        details("原因/依赖", question["blockers"])
        details("下一最小任务", question["nextAction"])
        details("验收条件", question["acceptance"])
        details("关联", question.get("relatedIds", []))
        line("- 依据：" + sources(question["sources"]))
    heading(3, "保留关闭结论与被后续成果取代的分支")
    closed_boundaries = {"Q-FUNC-MACRO-001": "V5.1功能按钮宏组真实禁用", "Q-LAMP-MACRO-001": "V5.1指示灯宏组真实禁用", "Q-LAMP-COUNT-001": "多态数2..32，状态索引0..N−1"}
    table(["ID", "状态/边界", "依据"], [[item["qId"], closed_boundaries[item["qId"]] + "；已补证关闭，保留范围", sources(item["sources"])] for item in data["closedFacts"]])
    table(["ID/分支", "旧说法", "现行范围", "父项是否关闭", "依据"], [[item["componentId"], item["oldStatement"], item["replacementBoundary"], "否", sources(item["sources"])] for item in data["supersededComponents"]])
    table(["A03父问题", "已完成范围", "仍需完成", "当前Q", "核销标准"], [[item["qId"], item["provenNow"] + "；受限C：" + item["backgroundC"], item["remaining"], item["currentIssueIds"], item["closureRule"]] for item in data["legacyA03Mappings"]])
    heading(2, "历史记录全量保留")
    line("下列是阶段0历史记录的逐条保留视图，当前统一状态为待与后续实施/补证对账（历史关闭事实另标）。历史开放不等于本版本仍未实现。处理时先查本台账当前S/Q和控件最新边界，只关闭获得新证据覆盖的分支；不能用文档规格通过核销软件实现。")
    for key, title in [("historyAtomicIssues", "历史原子问题"), ("historyParentIssues", "历史父问题"), ("historyEntryIssues", "历史入口问题"), ("historyFieldIssues", "历史字段问题")]:
        records = data["history"].get(key, [])
        heading(3, title + "（" + str(len(records)) + "条）")
        if not records:
            line("无独立记录；按源审计所保留的关联视图处理。")
            continue
        table(["历史ID", "原始问题/欠项", "历史状态 → 当前处理", "家族/S映射", "下一步/验收", "来源"], [
            [item["id"], [item.get("question", item.get("title", item.get("remaining", ""))), item.get("gapDescriptions", [])],
             str(item.get("historicalStatus", "")) + " → " + ("历史事实已关闭" if item.get("currentStatus") == "historical_fact_closed" else "待与最新证据对账"),
             [item.get("familyMapping", {}).get("id", ""), item.get("familyMapping", {}).get("name", ""), item.get("relatedTaskIds", [])],
             [item.get("nextExecutableStep", "先核同范围最新证据，再补缺失分支。"), "验收：" + str(item.get("historicalExpectedObservation", "原问题所声明字段/联动/寿命/消费者有对应层新证据；部分覆盖逐分支核销。"))], sources(item.get("sources", []))]
            for item in records])
    heading(2, "原始候选与后续追加行全量")
    line("冻结CAND-20261110原308条继续保留；现行CSV共318行，审计快照SHA及主目录等价字节身份见JSON。下列原文不截断，保留原始状态与来源，并给出本次分流。来源SHA按每行旧快照解释，不能冒充现文件哈希。")
    table(["候选ID/模块", "原始候选全文", "原记录状态", "统一处理/关联", "下一步/核销", "来源"], [
        [item["row"]["线索ID"] + " / " + item["row"]["模块"], item["row"]["候选内容"], item["row"]["状态"],
         item["disposition"] + "；" + cell(item["relatedIds"]), item["nextAction"] + "；验收：" + item["acceptance"],
         sources(item["sources"])] for item in data["candidates"]])
    heading(2, "本地续作之外的扩展与维护")
    line("主目录阶段3未合并续作已在EARLY/Q-S03-LOCAL相关项登记，不能归入已发布b73a3cf。以下LT779是独立扩展，分析建议尚无完整实施及板上验收证据；本地未提交报告只作来源，没有纳入本次Git提交。")
    for item in data["extensions"]:
        heading(3, item["id"] + " " + item["title"])
        details("状态", item["status"])
        details("已完成范围", item["completedScope"])
        details("剩余内容", item["remaining"])
        details("原因/依赖", item["blockers"])
        details("下一步", item["nextAction"])
        details("验收条件", item["acceptance"])
        line("- 依据：" + sources(item["sources"]))
    heading(2, "后续更新与核销规则")
    for rule in data["maintenanceRules"]:
        line("- " + rule)
    heading(3, "复现与证据")
    line("维护源：[同名JSON](未完成任务总台账.json)。只更新JSON中的对应S/Q/家族/候选处理，执行`python -B tools/render_open_task_ledger.py`重生成本文；该工具不运行业务程序或硬件测试。原始审计以确定性gzip去重保存于[证据目录](未完成任务证据/README.md)，输入文件名、SHA256及读取基线均在JSON中。")
    line()
    line("本轮校验只覆盖文档ID完整性、数量、关联、来源及主目录改动隔离；不新增任何D/U/P/R/C/T/E通过结论。")
    return "\n".join(out).rstrip() + "\n"


def validate(data):
    """Input: the maintained JSON. Return: checked counts or fail on lost IDs."""
    tasks = data["stageTasks"]
    expected = {f"S{stage:02}-{number:02}" for stage, total in enumerate([6, 9, 7, 8, 12, 10, 6, 6, 6, 6, 8, 6]) for number in range(1, total + 1)}
    assert len(tasks) == 90 and {item["id"] for item in tasks} == expected
    assert sum(bool(item["activeOpen"]) for item in tasks) == data["counts"]["activeOpenTasks"]
    assert sum(not item["activeOpen"] for item in tasks) == data["counts"]["closedDefinedScopeTasks"]
    families = data["families"]
    assert len(families) == 41
    assert {item["familyId"] for item in families} == {f"C{i:02}" for i in range(1, 32)} | {f"G{i:02}" for i in range(1, 11)}
    questions = data["questions"]
    question_ids = {item["id"] for item in questions}
    assert len(question_ids) == len(questions)
    assert sum(item.startswith("Q-") for item in question_ids) == data["counts"]["currentQuestions"]
    assert sum(item.startswith("EARLY-") for item in question_ids) == data["counts"]["aggregateItems"]
    closed = {item["qId"] for item in data["closedFacts"]}
    assert closed == {"Q-FUNC-MACRO-001", "Q-LAMP-MACRO-001", "Q-LAMP-COUNT-001"}
    assert not closed & question_ids
    assert len(data["supersededComponents"]) == 13
    assert all(not item["wholeIssueClosed"] for item in data["supersededComponents"])
    for family in families:
        assert set(family["layers"]) == set("DUPRCTE")
        assert set(family["openIssueIds"] + family.get("sharedIssueIds", [])) <= question_ids
    for task in tasks:
        assert set(task["itemIds"]) <= question_ids
        for key in ["completedScope", "remaining", "blockers", "nextAction", "acceptance", "sources"]:
            assert task[key], (task["id"], key)
    for item in questions:
        for key in ["remaining", "blockers", "nextAction", "acceptance", "sources"]:
            assert item[key], (item["id"], key)
    history_counts = {"historyAtomicIssues": 613, "historyParentIssues": 186, "historyEntryIssues": 195, "historyFieldIssues": 1521}
    history_ids = set()
    for key, total in history_counts.items():
        records = data["history"][key]
        assert len(records) == total
        assert len({item["id"] for item in records}) == total
        for item in records:
            assert item["id"] not in history_ids
            history_ids.add(item["id"])
            assert set(item.get("relatedTaskIds", [])) <= expected
            assert not item["countAsConfirmedUnimplemented"]
    atomic = data["history"]["historyAtomicIssues"]
    assert sum(item["currentStatus"] == "current_review_required" for item in atomic) == 610
    assert sum(item["currentStatus"] == "historical_fact_closed" for item in atomic) == 3
    candidates = data["candidates"]
    assert len(candidates) == 318
    ids = [item["row"]["线索ID"] for item in candidates]
    assert len(set(ids)) == len(ids)
    assert {item for item in ids if item.startswith("U-")} == {f"U-{i:03}" for i in range(1, 309)}
    allowed = question_ids | history_ids | closed | expected | set(data["aliases"])
    for item in candidates:
        assert set(item["relatedIds"]) <= allowed, item["row"]["线索ID"]
    assert "非问题" in next(item for item in candidates if item["row"]["线索ID"] == "U-308")["disposition"]
    current_csv = ROOT / "design_plan/复刻待验线索台账.csv"
    # //20261114-4 change: preserve the frozen byte identity while accepting
    # an equivalent checkout whose BOM or line endings Git has normalized.
    if hashlib.sha256(current_csv.read_bytes()).hexdigest() != data["csvIdentity"]["sha256"]:
        rows = list(csv.DictReader(current_csv.read_text(encoding="utf-8-sig").splitlines()))
        assert rows == [item["row"] for item in candidates], "CSV records changed; reconcile the ledger before rendering"
    for item in data["auditInputManifest"]:
        packed = (ROOT / item["path"]).read_bytes()
        raw = gzip.decompress(packed)
        assert len(raw) == item["rawBytes"] and hashlib.sha256(raw).hexdigest() == item["rawSha256"]
        assert len(packed) == item["archiveBytes"] and hashlib.sha256(packed).hexdigest() == item["archiveSha256"]
    return {"stageTasks": len(tasks), "families": len(families), "currentRecords": len(questions), "historicalRecords": len(history_ids), "candidateRows": len(candidates), "archives": len(data["auditInputManifest"])}


def main():
    """Input: optional --check. Return: render or verify the maintained view."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    data = json.loads(JSON_PATH.read_text(encoding="utf-8-sig"))
    checks = validate(data)
    rendered = render(data).encode("utf-8")
    if args.check:
        if MD_PATH.read_bytes() != rendered:
            raise SystemExit("Markdown is stale; render it from the maintained JSON")
    else:
        MD_PATH.write_bytes(rendered)
    print(json.dumps({"checked": args.check, "bytes": len(rendered), "sha256": hashlib.sha256(rendered).hexdigest(), "coverage": checks}, ensure_ascii=False))


if __name__ == "__main__":
    main()
