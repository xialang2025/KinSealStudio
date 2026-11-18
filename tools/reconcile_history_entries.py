"""Reconcile the original Stage 0 entry IDs with current bounded evidence."""
# 20261119-1 add: Trace historical entries without upgrading help or family links to closure.
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re
import subprocess


ROOT = Path(__file__).resolve().parents[1]
LEDGER_PATH = "design_plan/未完成任务总台账.json"
INVENTORY_PATH = "docs/stage0-evidence/entry-inventory.json"
HELP_PATH = "help/command-map.json"
MAIN_WINDOW_PATH = "src/main_window.cpp"
OUTPUT_PATH = "design_plan/未完成任务证据/S00-02历史入口对账.md"

# Explicit renames confirmed by the current MainWindow action labels and slots.
ACTION_RENAMES = {
    "action_sengtoback": "action_sendtoback",
    "action_bringback": "action_sendbackward",
    "action_split": "action_ungroup",
}

# Structural membership is an evidence lookup, never a behavior or closure match.
FAMILY_TASK_RANGES = (
    (1, 5, "S04-01"), (6, 8, "S04-02"), (9, 10, "S04-03"),
    (11, 13, "S04-04"), (14, 15, "S04-05"), (16, 17, "S04-06"),
    (18, 21, "S04-07"), (22, 22, "S04-08"), (23, 26, "S04-09"),
    (27, 31, "S04-10"),
)

# Read these groups only for commands that have no reviewed creation target.
COMMAND_SCOPES = (
    (("action_new_pro",), "S02-02", ("EARLY-S02-WIZARD-OPTIONS",),
     "运行新建向导的接受与取消，核目标、方向、连接和新工程是否真正落盘。"),
    (("action_open_pro", "action_close", "action_save", "action_save_as", "action_quit"),
     "S02-02", ("EARLY-A02-FILE-EDGE-AND-V31", "EARLY-S00-ENTRY-STATES"),
     "分别走新旧路径、未保存状态和取消分支，记录退出/开闭后工程与持久化字节。"),
    (("actionCompress",), "S02-07", ("EARLY-A02-FILE-EDGE-AND-V31",),
     "取得授权最小V31样本，记录窗口接受项、真实容器输入输出及失败保全。"),
    (("action_undo", "action_redo"), "S03-05", ("EARLY-S00-ENTRY-STATES", "Q-A02-CMD-001"),
     "分别在画布和文本草稿焦点执行，核工程历史、dirty及重新打开后的对象。"),
    (("action_cut", "action_paste", "action_copy", "action_copym"),
     "S03-04", ("Q-A02-CMD-001",),
     "分别在对象选中、框选及局部输入焦点执行，对比实际剪贴板/对象引用及持久化。"),
    (("action_delete", "action_checkall", "action_bringtop", "action_bringforward",
      "action_sengtoback", "action_bringback", "action_top", "action_under", "action_vcenter",
      "action_left", "action_right", "action_hcenter", "action_horizontal", "action_vertcal",
      "action_widthsame", "action_heightsame", "action_widthheightsame", "action_group",
      "action_split", "action_lock", "action_delock", "action_h", "action_v",
      "action_reverse_selection"),
     "S03-03", ("Q-S03-001", "EARLY-S00-ENTRY-STATES"),
     "实验固定两个或三个对象，单执行本入口；记录实际选择、几何或层次及撤销/保存差分。"),
    (("action_zoonbig", "action_zoonfit", "action_zoonsmall", "action_grid", "action_panview",
      "action_40", "action_36", "action_tool", "action_37", "action_41", "action_42"),
     "S02-01", ("EARLY-S02-MAINWINDOW-PARITY", "EARLY-S00-ENTRY-STATES"),
     "记录同DPI下视图、焦点、状态切换和工程字节；无写入须实测后标明。"),
    (("action_upscreen", "action_nextscreen", "action_new_screen"),
     "S02-03", ("EARLY-S02-PAGE-WINDOW-PROPERTIES", "Q-S01-001"),
     "用有顺序的两页一窗执行分页/创建，核活动页、启动页和保存 ID；若适用再比设备索引。"),
    (("action_address_find", "action_items_table", "action_TextView", "action_address_table"),
     "S02-07", ("EARLY-S00-ENTRY-CONSUMERS",),
     "交叉验证列出的真实对象/地址和点击定位；存在替换提交时再核保存、取消与扫描影响。"),
    (("action_openpropertiset",), "S03-02", ("Q-A02-CMD-001",),
     "从实际焦点触发属性窗口，分取消、确定和重开；与右键/双击分别记录。"),
    (("action_Size",), "S02-07", ("EARLY-S02-OPTIMIZE-SIZE", "Q-S03-001"),
     "用不对称对象触发，记录几何、脚本未触发的失效条件、撤销和保存重开。"),
    (("action_macro_find", "action_macro_admin", "action_script"),
     "S08-01", ("Q-S08-002",),
     "记录查找/编辑/管理的窗口、目标宏、修改及取消；另跟踪保存引用与实际编译索引。"),
    (("action_InitMacro", "action_GlobalMacro"), "S08-05", ("Q-S08-002",),
     "用单个普通宏配置一种调度，记录真实项、源字段、保存重开，再离线核编译flag。"),
    (("action_rep",), "S02-05", ("EARLY-A02-RECIPE-LIMITS", "Q-S01-003"),
     "用最小配方区分菜单入口和树入口，对比保存字段并核资源跨度。"),
    (("action_datasamp",), "S02-05", ("EARLY-A02-DATA-EMPTY-NAME",
      "EARLY-A02-DATA-INITIAL-ITEMS", "EARLY-A02-DATA-CAPACITY", "Q-S01-003"),
     "验证空名/非空名、首开零行与首次建立数据行，核保存重开及采集段。"),
    (("action_waring", "action_conalarm"), "S02-05", ("Q-S01-003",),
     "分别从菜单添加数位/条件报警，核已存在配置、事件真实窗口、保存重开及报警段。"),
    (("action_ProLib",), "S03-07", ("EARLY-S00-ENTRY-CONSUMERS",),
     "分别从原厂图库入口导入、替换或取消，核资源 ID、源图与编译后的引用。"),
    (("action_upload",), "S06-04", ("Q-S06-001", "Q-S06-002"),
     "后台无设备先观察窗口/合法输入和输出扩展名；读取或写出另需已恢复上载协议。"),
    (("action_USBdownload",), "S06-02", (),
     "先测假worker/UI取消及原厂无设备窗口状态；工程传输行为需独立T，实际显示另记E。"),
    (("action_offline_simulation", "action_online_simulation"),
     "S07-01", ("Q-S07-001", "Q-S07-003"),
     "分别从两个入口记录原厂启动进程、快照和退出因果；在线物理端点另验。"),
    (("action_upgrade", "action_upgrade2", "action_usbdriver"),
     "S11-02", ("Q-S11-001",),
     "先只观察旧菜单实际类型与保护条件；涉及安装/固件的分支需授权及隔离恢复环境。"),
    (("action_version",), "S10-01", ("EARLY-S00-HARDWARE-IDENTITY",),
     "先区分产品版本页与底层设备版本，保留实际固件身份的独立观察。"),
    (("action_H", "action_KinSeal"), "S11-05", ("EARLY-S02-CONTEXT-HELP",),
     "记录精确菜单点击、焦点和显示内容；若无工程或设备副作用，先核再标不适用。"),
    (("action_chinese", "action_english"), "S02-01", ("EARLY-S02-MAINWINDOW-PARITY",),
     "在同输入下切编辑器语言，区分界面翻译、持久化及设备启动语言。"),
)

TREE_SCOPES = {
    "TREE.COM": ("S09-02", ("Q-S09-001",), "连接/协议选择及未知node保护"),
    "TREE.PAGE": ("S02-03", ("Q-S01-001", "EARLY-S02-PAGE-WINDOW-PROPERTIES"), "两普通页、活动/启动页和引用"),
    "TREE.WINDOW": ("S02-03", ("Q-S01-001", "EARLY-S02-PAGE-WINDOW-PROPERTIES"), "窗属性、普通页与启动/关闭关系"),
    "TREE.DATA": ("S02-05", ("EARLY-A02-DATA-EMPTY-NAME", "EARLY-A02-DATA-INITIAL-ITEMS"), "采集定义、空名和表行"),
    "TREE.TRANSFER": ("S02-06", ("EARLY-S02-DATA-TRANSFER", "Q-S09-003"), "资料传输端点、长度与周期"),
    "TREE.RECIPE": ("S02-05", ("EARLY-A02-RECIPE-LIMITS", "Q-S01-003"), "配方定义与持久化跨度"),
    "TREE.LANGUAGE": ("S01-05", ("Q-S01-004",), "工程语言、显示/启动及缩减恢复"),
    "TREE.FILE_ACCESS": ("S02-04", ("EARLY-S02-SYSTEM-BLOCKS",), "工程文件和限制保护"),
    "TREE.SYS": ("S02-04", ("EARLY-S02-SYSTEM-FIVE-BACKEND", "EARLY-S02-SYSTEM-BLOCKS"), "该系统配置块与SysPara"),
    "TREE.ALARM": ("S02-05", ("Q-S01-003",), "报警配置数位/条件与实际源段"),
    "TREE.KEYBOARD": ("S02-06", ("EARLY-S02-CUSTOM-KEYBOARD", "Q-S01-001"), "键盘实体和引用"),
    "TREE.LOGO": ("S02-06", ("EARLY-S02-WIZARD-OPTIONS", "Q-S01-001"), "Logo实体与上电引用"),
    "GEO.CONNECTION_LINE": ("S04-11", ("Q-S01-002",), "候选连接线是否有正常创建命令和实体"),
    "CTX.OBJECT": ("S03-02", ("EARLY-S02-CONTEXT-HELP", "Q-A02-CMD-001"), "画布右键属性实际菜单和对象类型"),
    "DBL.OBJECT": ("S03-02", ("EARLY-S02-CONTEXT-HELP", "Q-A02-CMD-001"), "实际双击属性路由和对象类型"),
}

# Explicit dialog routes supplement the creation entry. They never prove it.
DIALOG_ROUTES = {
    "C01": ("static_text", "StaticText"),
    "C02": ("bit_button", "BitButton"),
    "C06": ("message_display", "MessageDisplay"),
    "C07": ("scale", "Scale"),
    "C08": ("qr_code", "QrCode"),
    "C09": ("image_display", "ImageDisplay"),
    "C10": ("gif_display", "GifDisplay"),
    "C11": ("number_display", "NumberDisplay"),
    "C12": ("number_display", "NumberDisplay"),
    "C13": ("number_display", "NumberDisplay"),
    "C17": ("dropdown", "Dropdown"),
    "C18": ("flow_block", "FlowBlock"),
    "C19": ("slider", "Slider"),
    "C20": ("wheel", "Wheel"),
    "C21": ("move_track", "MoveTrack"),
    "C22": ("chart", "Chart"),
    "C23": ("trend", "Trend"),
    "C24": ("history_trend", "HistoryTrend"),
    "C25": ("xy_curve", "XyCurve"),
    "C26": ("data_group", "DataGroup"),
    "C27": ("alarm_bar", "AlarmBar"),
    "C28": ("alarm_display", "AlarmDisplay"),
    "C30": ("history_data", "HistoryData"),
    "C31": ("recipe_display", "RecipeDisplay"),
    "G10": ("graphic_table", "GraphicTable"),
}
for shape_id in ("G01", "G02", "G04", "G05", "G06", "G07", "G08", "G09"):
    DIALOG_ROUTES[shape_id] = ("graphic", "Graphic")


def digest(contents):
    return hashlib.sha256(contents).hexdigest()


def canonical_sha(contents):
    """Hash UTF-8 text independently of Git checkout line endings."""
    canonical = contents.decode("utf-8-sig").replace("\r\n", "\n")
    return digest(canonical.encode("utf-8"))


def frozen_inventory(root, commit):
    """Read the exact historical inventory without restoring a worktree deletion."""
    if re.fullmatch(r"[0-9a-f]{40}", commit) is None:
        raise ValueError("台账sourceCommit必须是完整Git提交ID")
    return subprocess.check_output(
        ["git", "show", f"{commit}:{INVENTORY_PATH}"], cwd=root,
    )


def read_json(contents):
    return json.loads(contents.decode("utf-8-sig"))


def keyed(rows):
    result = {row["id"]: row for row in rows}
    if len(result) != len(rows):
        raise ValueError("证据存在重复ID")
    return result


def name_key(name):
    # Geometry menu mnemonics are not part of the family identity.
    return re.sub(r"\s*\(&[A-Za-z]\)", "", name).strip()


def entry_family(entry, old_family, families, historical_entries):
    """Resolve only a ledger family, exact family label or a frozen alias target."""
    if old_family["id"] is not None:
        matches = [f for f in families if f["familyId"] == old_family["id"]]
        if len(matches) != 1:
            raise ValueError(f"已列入口族无法解析：{old_family['id']}")
        return matches[0], "台账稳定族"
    if not entry:
        return None, ""
    target = historical_entries.get(entry.get("creationTargetId"), entry)
    if target["category"] not in ("toolbox-control", "basic-geometry", "table"):
        return None, ""
    label = target.get("labelFromManual") or target.get("exactLabel") or ""
    matches = [f for f in families if name_key(f["name"]) == name_key(label)]
    if len(matches) != 1:
        return None, ""
    basis = "创建别名→精确族名" if target is not entry else "精确族名"
    return matches[0], basis


def action_routes(entry, commands, main_source):
    """Keep direct keys, reviewed renames and recorded sibling routes separate."""
    if not entry:
        return "无单独帮助键，状态问题", "missing"
    category = entry["category"]
    if category not in ("named-action", "toolbox-control", "basic-geometry", "table"):
        return "无逐路径证明，通用F1不计", "missing"
    if category == "named-action":
        action = entry["objectName"]
        mapped = ACTION_RENAMES.get(action, action)
        if mapped != action and f'QStringLiteral("{mapped}")' not in main_source:
            mapped = action
        method = "同键" if mapped == action else "已核改名"
    else:
        siblings = [route["commandId"].removeprefix("CMD.") for route in entry["routes"]
                    if route.get("commandId", "").startswith("CMD.")]
        if len(siblings) != 1:
            return "无唯一实际同业务命令键", "missing"
        mapped = siblings[0]
        method = "别名同业务键" if category == "toolbox-control" else "原表同一QAction"
    command = commands.get(mapped)
    if not command:
        return "无独立命令帮助键", "missing"
    bound = (f'showCommand(QStringLiteral("{mapped}")' in main_source)
    kind = "dialog-or-menu-dispatch" if bound else {
        "同键": "exact-key", "已核改名": "renamed-key",
        "别名同业务键": "sibling-key", "原表同一QAction": "same-action-alias",
    }[method]
    summary = f"{method} {mapped} → {command['topic']}（{command['coverage']}）"
    if bound:
        summary += "，源码有菜单帮助派发"
    return summary, kind


def dialog_route(root, family, commands, source_fingerprints):
    """Only record explicit showCommand source and the matching topic."""
    if not family:
        return ""
    route = DIALOG_ROUTES.get(family["familyId"])
    if not route:
        return ""
    filename, class_name = route
    command_id = f"help_{class_name}Dialog"
    command = commands.get(command_id)
    if not command:
        return ""
    source_name = f"src/{filename}_dialog.cpp"
    if source_name not in source_fingerprints:
        contents = (root / source_name).read_bytes()
        source_fingerprints[source_name] = (
            canonical_sha(contents), contents.decode("utf-8-sig"),
        )
    source = source_fingerprints[source_name][1]
    if f'showCommand(QStringLiteral("{command_id}")' not in source:
        return ""
    return f"；独立属性帮助 {command_id} → {command['topic']}"


def action_scope(entry, row, family):
    """Map an entry to the exact recovery slot; never infer an implementation."""
    if family:
        family_id = family["familyId"]
        if family_id.startswith("C"):
            number = int(family_id[1:])
            stage = next(task for low, high, task in FAMILY_TASK_RANGES if low <= number <= high)
        else:
            stage = "S04-11"
        proof = {
            "named-action": "从记录的菜单和工具条逐入口触发，核草稿、焦点/取消与对象创建；同工程再比保存与设备消费者。",
            "toolbox-control": "真正点击工具箱控件并在画布落点；与同族菜单/工具条分开记录，核保存重开及设备输出/实屏。",
            "basic-geometry": "从本命令创建该图元，只改一个几何参数；核原厂保存路径、整帧和同包实屏。",
            "table": "从当前表格入口建立最小格数并只改一值；核纯图像表格保存、整帧和同包实屏。",
        }[entry["category"]]
        return stage, family["openIssueIds"], proof
    if not entry:
        return "S02-01", ["EARLY-S00-ENTRY-STATES", row["id"]], row["nextExecutableStep"]
    if entry["category"] == "named-action":
        for actions, stage, questions, proof in COMMAND_SCOPES:
            if entry["objectName"] in actions:
                return stage, list(questions), proof
        return "S02-01", ["EARLY-S00-ENTRY-CONSUMERS"], row["nextExecutableStep"]
    for prefix, (stage, questions, subject) in TREE_SCOPES.items():
        if entry["id"] == prefix or entry["id"].startswith(prefix + "."):
            detail = entry["routes"][0].get("candidatePath") or entry["routes"][0].get("labelFromHelp")
            proof = f"沿该入口单独核{subject}；记录是否真正到达、确定/取消、保存重开和对应消费者。"
            if detail:
                proof = f"按{detail}触发，" + proof
            return stage, list(questions), proof
    return "S00-02", ["EARLY-S00-ENTRY-CONSUMERS"], row["nextExecutableStep"]


def native_route(entry):
    if not entry:
        return "状态定义，无独立入口"
    parts = []
    for route in entry["routes"]:
        detail = route.get("surface") or route.get("candidatePath") or route.get("objectName")
        if not detail:
            detail = route.get("commandId") or route["kind"]
        parts.append(detail)
    return " / ".join(dict.fromkeys(parts))


def cell(value):
    return str(value).replace("|", r"\|").replace("\r", " ").replace("\n", " ").strip()


def build_report(root):
    """Join the stable evidence without modifying or closing any canonical row."""
    ledger_bytes = (root / LEDGER_PATH).read_bytes()
    ledger = read_json(ledger_bytes)
    inventory_bytes = frozen_inventory(root, ledger["sourceCommit"])
    inventory = read_json(inventory_bytes)
    help_bytes = (root / HELP_PATH).read_bytes()
    catalog = read_json(help_bytes)
    if any(command.get("topic") not in catalog["topics"]
           for command in catalog["commands"].values()):
        raise ValueError("帮助索引存在未注册的目标主题")
    main_bytes = (root / MAIN_WINDOW_PATH).read_bytes()
    main_source = main_bytes.decode("utf-8-sig")
    old_rows = ledger["history"]["historyEntryIssues"]
    historical_questions = keyed(inventory["questions"])
    entries = keyed(inventory["entries"])
    questions = keyed(ledger["questions"])
    stages = keyed(ledger["stageTasks"])
    if set(keyed(old_rows)) != set(historical_questions):
        raise ValueError("台账与原始入口清单的问题集合不同")
    rows = []
    counts = Counter()
    source_fingerprints = {}
    for row in old_rows:
        qid = row["id"]
        frozen = historical_questions[qid]
        sources = row["sources"]
        expected_pointer = next((source["jsonPointer"] for source in sources
                                 if source["path"] == INVENTORY_PATH), "")
        if re.fullmatch(r"/questions/\d+", expected_pointer) is None:
            raise ValueError(f"{qid}缺少可复核的原始JSON指针")
        pointer = int(expected_pointer.rsplit("/", 1)[1])
        if inventory["questions"][pointer]["id"] != qid or frozen["layers"] != row["blockingLayers"]:
            raise ValueError(f"{qid}问题指针或证据层已变化")
        old_family = row["familyMapping"]
        entry_id = old_family["entryId"]
        entry = entries.get(entry_id)
        if entry_id and (not entry or qid not in entry["questionIds"]):
            raise ValueError(f"{qid}归属的历史入口ID无法反向核实")
        if entry:
            category = entry["category"]
            if old_family["entryCategory"] != category:
                raise ValueError(f"{qid}原厂入口类别与台账不同")
        else:
            category = "state"
        family, basis = entry_family(entry, old_family, ledger["families"], entries)
        stage, candidate_questions, proof = action_scope(entry, row, family)
        related = list(dict.fromkeys([stage, *row["relatedTaskIds"]]))
        if set(related) - stages.keys() or set(candidate_questions) - questions.keys():
            raise ValueError(f"{qid}现行任务或问题ID不可解析")
        help_route, help_kind = action_routes(entry, catalog["commands"], main_source)
        supplemental_help = dialog_route(root, family, catalog["commands"], source_fingerprints)
        counts[(category, "entries")] += 1
        counts[(category, help_kind)] += 1
        if supplemental_help:
            counts[(category, "dialog-supplement")] += 1
        if family:
            counts[(category, "family")] += 1
        rows.append((category, qid, expected_pointer, entry_id or "状态",
                     native_route(entry), family, basis, related, candidate_questions,
                     help_route + supplemental_help, row["blockingLayers"], proof))

    lines = [
        "# S00-02 历史入口逐ID对账", "",
        "以总台账 historyEntryIssues 对接原厂入口、现行阶段任务和产品帮助。"
        "本表共 %d 项，所有问题维持原状态；未做原厂交互、设备编译或实物测试。" % len(rows), "",
        "- 原始入口清单固定在总台账基线提交 " + ledger["sourceCommit"] + "，文件SHA-256 " + digest(inventory_bytes) + "。",
        "- 对账文本以UTF-8/LF统一后计算SHA-256。总台账 " + canonical_sha(ledger_bytes) + "，帮助命令表 " + canonical_sha(help_bytes)
        + "，主窗口源码 " + canonical_sha(main_bytes) + "。",
        "- 现行Q是可调查的专项或同族问题，主任务的既定范围通过不替代本入口。"
        "父范围和同一业务的多个入口不重复计功能。",
        "- “同键”仅证实命令存在于帮助表。“已核改名”同时检查新的主窗口命令标识。"
        "“别名同业务键”不会算工具箱入口自身有帮助。属性帮助仅在有实际 showCommand 调用与对应主题时列出，仍是另一入口。",
        "- 每一行的历史资料可用原始清单 JSON 指针复核。没有逐入口源/设备结果时，保留历史证据层和下一项最小验证。"
        "通用标题F1、同族帮助和同业务别名均不能替代实际菜单、右键或双击。",
        "", "## 查核统计", "",
        "| 历史入口类型 | 条数 | 同键帮助表 | 改名帮助表 | 菜单直接派发 | 工具箱关联命令 | 图形同一QAction | 无专用证明 | 关联41族 | 有独立属性帮助 |",
        "|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|",
    ]
    category_names = {
        "named-action": "命名QAction", "toolbox-control": "工具箱",
        "basic-geometry": "图形", "table": "表格", "state": "状态",
        "project-tree-or-configuration": "工程树/配置", "other-route-or-candidate": "其它入口/候选",
    }
    count_fields = ("entries", "exact-key", "renamed-key", "dialog-or-menu-dispatch",
                    "sibling-key", "same-action-alias", "missing", "family", "dialog-supplement")
    for category, title in category_names.items():
        stats = [counts[(category, key)] for key in count_fields]
        lines.append("| " + " | ".join([title, *map(str, stats)]) + " |")
    lines.extend(["", "## 逐项核对", ""])
    for category, title in category_names.items():
        lines.extend([
            "### " + title, "",
            "| 问题ID / 原始指针 | 原厂入口与路线 | 现行族 / 任务 / Q | 产品帮助证据 | 保留层与下一动作 |",
            "|---|---|---|---|---|",
        ])
        for cat, qid, pointer, eid, route, family, basis, tasks, qs, help_route, layers, proof in rows:
            if cat != category:
                continue
            association = "、".join(tasks)
            if family:
                association = f"{family['familyId']} {family['name']}（{basis}）；" + association
            if qs:
                association += "；Q：" + "、".join(qs)
            columns = (f"{qid} {pointer}", f"{eid} · {route}",
                       association, help_route, f"{'/'.join(layers)} · {proof}")
            lines.append("| " + " | ".join(map(cell, columns)) + " |")
        lines.append("")
    lines.extend(["## 关联属性帮助的源码身份", ""])
    for source, (sha, _) in sorted(source_fingerprints.items()):
        lines.append(f"- {source} · SHA-256 {sha}")
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="check current report without changing files")
    args = parser.parse_args()
    output = ROOT / OUTPUT_PATH
    report = build_report(ROOT)
    if args.check:
        if not output.is_file() or output.read_text(encoding="utf-8-sig") != report:
            parser.exit(1, "历史入口对账与当前输入不一致，重新运行生成器复核。\n")
        print("历史入口对账已复核。")
    else:
        output.write_bytes(report.encode("utf-8"))
        print(f"已生成：{output.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
