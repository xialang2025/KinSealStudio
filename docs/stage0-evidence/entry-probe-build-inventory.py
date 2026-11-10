"""#20261110-4 add: Derive a bounded inventory without promoting unknown behavior."""
from pathlib import Path
import hashlib
import json
import re

ROOT = Path(r"Z:\Project\vscode_prj_temp\REA")
OUT = ROOT / "KinSealStudio/docs/stage0-evidence"
PLAN = ROOT / "KinSealStudio/design_plan"
PROBE_PATH = OUT / "entry-probe-readonly.json"
OLD_PATH = PLAN / "分阶段文档证据/V51后台入口采集_20261110.json"
HELP_PATH = PLAN / "分阶段文档证据/原厂帮助正文_20261110.json"
probe = json.loads(PROBE_PATH.read_text(encoding="utf-8-sig"))
old = json.loads(OLD_PATH.read_text(encoding="utf-8-sig"))
help_doc = json.loads(HELP_PATH.read_text(encoding="utf-8-sig"))
topics = {topic["path"]: topic for topic in help_doc["topics"]}


def sha(path):
    """Return a digest of exactly the file referenced by an entry."""
    return hashlib.sha256(path.read_bytes()).hexdigest()


def ref(path):
    """Keep references relative to the workspace without losing input identity."""
    return {"path": path.relative_to(ROOT).as_posix(), "sha256": sha(path)}


STATES = {
    "noProject": "无工程",
    "emptyProject": "空工程",
    "singleSelection": "有选中对象/单选",
    "multipleSelection": "多个对象/多选",
    "withoutPrerequisiteConfiguration": "无前置配置",
    "withPrerequisiteConfiguration": "有配置",
    "dirtyProject": "工程已修改/脏工程",
    "readOnlyOrRunning": "只读或运行中",
}


def state_template():
    """Represent all eight requested states explicitly; absence is not disabled."""
    return {key: {"status": "not-observed", "enabled": None, "nextAction": "在本任务私有桌面的独立同型号测试工程建立该状态，读取Qt真实状态并触发入口；记录前后状态。"} for key in STATES}


def walk(rows, trail, prefix):
    """Preserve menu and toolbar provenance instead of deduplicating by label."""
    for index, row in enumerate(rows):
        pointer = prefix + "/" + str(index)
        path = trail + [row.get("text", "")]
        if row.get("objectName"):
            yield row, path, pointer
        yield from walk(row.get("children", []), path, pointer + "/children")


def collect(run_index):
    """Build a named-action map for only the returned surfaces of this run."""
    result = {}
    for surface_index, surface in enumerate(probe["runs"][run_index].get("surfaces", [])):
        surface_name = surface["qt"]["objectName"]
        for row, path, pointer in walk(surface["reading"]["actions"], [], "/runs/%d/surfaces/%d/reading/actions" % (run_index, surface_index)):
            result.setdefault(row["objectName"], []).append({"row": row, "surface": surface_name, "path": path, "jsonPointer": pointer})
    return result


by_run = [collect(i) for i in range(len(probe["runs"]))]
opened = by_run[1]
no_project = by_run[0]
widget_runs = [{w["qt"]["objectName"]: w for w in r["widgets"] if w.get("qt") and w["qt"]["objectName"]} for r in probe["runs"]]
entries = []
issues = []


def issue(qid, question, next_action, layers):
    """Retain precise unresolved work with an actionable next probe."""
    existing = next((q for q in issues if q["id"] == qid), None)
    if existing:
        return qid
    issues.append({"id": qid, "question": question, "nextAction": next_action, "layers": layers, "status": "open"})
    return qid


for number, (name, observations) in enumerate(opened.items(), 1):
    best = observations[0]["row"]
    states = state_template()
    if name in no_project:
        prior = no_project[name][0]
        states["noProject"] = {"status": "observed-property", "enabled": bool(prior["row"]["enabled"]), "evidence": "entry-probe-readonly.json#" + prior["jsonPointer"], "behaviorExecuted": False}
    qid = issue("Q-S00-ENTRY-CMD-%03d" % number, "%s（%s）的触发行为、焦点上下文、源修改与设备影响尚未逐项验证。" % (best["text"].replace("\n", " "), name), "在已记录原件/副本身份与同型号测试工程中，从%s触发%s；记录窗口/对象/草稿/脏标变化，涉及持久化的再保存重开；涉及设备段的仅离线编译差分后映射消费者。" % (observations[0]["surface"], name), ["U", "P", "C"])
    routes = [{"kind": "menu" if o["surface"] == "menuBar" else "toolbar", "surface": o["surface"], "exactLabel": o["row"]["text"], "path": o["path"], "evidence": "entry-probe-readonly.json#" + o["jsonPointer"]} for o in observations]
    if best.get("shortcutPortable"):
        routes.append({"kind": "shortcut-property", "value": best["shortcutPortable"], "getter": "QAction::shortcut→QKeySequence::toString(PortableText)", "behaviorExecuted": False, "focusContexts": ["canvas:not-observed", "text-input:not-observed", "modal:not-observed"]})
    entries.append({"id": "CMD." + name, "category": "named-action", "objectName": name, "exactLabel": best["text"], "window": "KinSealStudio main window", "evidenceKind": ["V:read-only-Qt-property"], "entryStatus": "observed-current-run", "routes": routes, "states": states, "supplementalState": {"openedUnmodifiedProbe": {"enabled": bool(best["enabled"]), "checkable": bool(best["checkable"]), "checked": bool(best["checked"]), "selectionCount": "not-observed", "configurationInventory": "not-observed", "evidence": "entry-probe-readonly.json#" + observations[0]["jsonPointer"]}}, "shortcut": {"portableText": best["shortcutPortable"], "emptyMeans": "QAction shortcut getter returned an empty sequence in this state; not proof no other key/event route exists"}, "sourceObject": {"status": "not-established-by-entry-snapshot", "questionId": qid}, "deviceConsumer": {"status": "not-established-by-entry-snapshot", "questionId": qid}, "questionIds": [qid]})

manual_index_text = (PLAN / "工具箱控件复刻总览.md").read_text(encoding="utf-8-sig")
dialogs = {}
for line in manual_index_text.splitlines():
    cols = [c.strip() for c in line.split("|")]
    if len(cols) == 7 and cols[1].isdigit() and cols[3].endswith(".md"):
        dialogs[cols[2]] = cols[5]

a02 = (PLAN / "A02_全局入口与工程配置规范.md").read_text(encoding="utf-8-sig")
toolbox_rows = []
for line in a02.splitlines():
    cols = [c.strip() for c in line.split("|")]
    if len(cols) == 6 and cols[1].isdigit() and cols[3].startswith("btn_"):
        toolbox_rows.append((int(cols[1]), cols[2], cols[3], cols[4]))
assert len(toolbox_rows) == 31
extra_actions = {"btn_alarmbar": "action_dynamicarlarm", "btn_hisalarmDisplay": "action_histotyarlarm", "btn_alarmDisplay": "action_arlamcontrol", "btn_dataGroup": "action_dataGroup"}
known = {
    "btn_switch_bit": ("type10/switch.mode=0", "ControlPara type10/subtype0; ControlPixel and AddrPara as active fields require"),
    "btn_switch_word": ("type10/switch.mode=1; word.150 and word.16c", "ControlPara type10/subtype1 and associated pixels/address scan"),
    "btn_switch_Lamp": ("type10/switch.mode=4; switch monitor/exterior structures", "ControlPara type10/subtype4 and associated pixels/address scan"),
    "btn_switch_Fun": ("type10/switch.mode=3; function.1c8 iiqbbiq", "ControlPara type10/subtype3 and associated pixels/condition reads; no operation-address read"),
    "btn_text": ("type28/static.194 plus static.head/tail", "SheetPixel background for ordinary single-language static text; type28 dynamic record under supported language/visibility conditions"),
}


def creation_mapping(index, label, key):
    """Reuse A05's bounded five mappings, leaving every other format explicit."""
    if key in known:
        return ({"status": "known-with-existing-support-constraints", "value": known[key][0], "evidence": "KinSealStudio/design_plan/A05_源格式与设备编译契约.md#4-sm当前可定位文法", "newExecution": False}, {"status": "known-with-existing-support-constraints", "value": known[key][1], "evidence": "KinSealStudio/design_plan/A05_源格式与设备编译契约.md#5-设备段与记录", "newExecution": False}, [])
    qid = issue("Q-S00-ENTRY-MAP-%02d" % index, "%s的完整原厂typed源边界及设备消费者尚未恢复。" % label, "在AMT070W124-L800X480R独立工程仅创建一个%s，保存A、重复保存A2、单字段修改B并原厂重开；获取原厂离线编译产物，对比typed对象边界、图像段/记录/扫描/引用；未闭合前保持导出/下载拒绝。" % label, ["P", "C", "E"])
    return ({"status": "unknown", "questionId": qid}, {"status": "unknown", "questionId": qid}, [qid])


creation_by_action = {}
for index, (row, label, button, old_action) in enumerate(toolbox_rows, 1):
    action_match = re.search(r"(?:action_[A-Za-z0-9_]+|pictruedisplay|graphicsitem)", old_action)
    action = extra_actions.get(button) or (action_match.group() if action_match else None)
    assert action in opened, (button, action)
    states = state_template()
    qt1 = widget_runs[1][button]["qt"]
    if button in widget_runs[0]:
        qt0 = widget_runs[0][button]["qt"]
        states["noProject"] = {"status": "observed-property", "enabled": bool(qt0["enabled"]), "evidence": "entry-probe-readonly.json#/runs/0/widgets;objectName=" + button, "behaviorExecuted": False}
    else:
        states["noProject"] = {"status": "toolbox-widget-not-returned-in-native-window-enumeration", "enabled": None, "associatedActionEnabled": bool(no_project[action][0]["row"]["enabled"]), "associatedActionId": "CMD." + action, "boundary": "工具栏QAction已读到disabled；未返回工具箱btn HWND不能当btn本体禁用证据。", "behaviorExecuted": False}
    source, consumer, qids = creation_mapping(index, label, button)
    manual = "KinSealStudio/design_plan/" + label + "功能与交互复刻手册.md"
    assert (ROOT / manual).is_file()
    entry_id = "TBX." + button
    creation_by_action[action] = entry_id
    entries.append({"id": entry_id, "category": "toolbox-control", "toolboxGridRowFromManual": row, "labelFromManual": label, "objectName": button, "exactLabel": qt1["text"], "exactLabelNote": "图标按钮text为空；旁侧标签/手册名称另列，不向原始text注入中文。", "window": dialogs.get(label, "见该控件手册"), "entryStatus": "observed-current-run", "evidenceKind": ["V:read-only-Qt-property", "V/H/B:existing-manual-separated"], "routes": [{"kind": "toolbox", "objectName": button, "evidence": "entry-probe-readonly.json#/runs/1/widgets;objectName=" + button}, {"kind": "menu-and-toolbar", "commandId": "CMD." + action}], "states": states, "supplementalState": {"openedUnmodifiedProbe": {"enabled": bool(qt1["enabled"]), "visible": bool(qt1["visible"]), "selectionCount": "not-observed", "configurationInventory": "not-observed"}}, "sourceObject": source, "deviceConsumer": consumer, "questionIds": qids, "manual": ref(ROOT / manual)})

geometries = [
    ("LINE", "直线", "action_line", "BaseItem/line.htm"),
    ("POLYLINE", "折线", "action_foldline", "BaseItem/flodline.htm"),
    ("FREE", "自由直线", "action_freeline", "BaseItem/freeline.htm"),
    ("ARC", "曲线/圆弧", "action_blight_circle", "BaseItem/curline.htm"),
    ("RECT", "矩形", "action_rect", "BaseItem/rect.htm"),
    ("ROUND", "圆角矩形", "action_RoundedRect", "BaseItem/roundrect.htm"),
    ("ELLIPSE", "圆/椭圆", "action_ellipse", "BaseItem/elli.htm"),
    ("POLYGON", "多边形", "action_polygon", "BaseItem/poly.htm"),
    ("SECTOR", "扇形", "action_ArcItem", "BaseItem/sector.htm"),
    ("TABLE", "表格", "action_table", "Table/Table.htm"),
]
for index, (gid, label, action, topic) in enumerate(geometries, 32):
    source, consumer, qids = creation_mapping(index, label, gid)
    command = next(e for e in entries if e["id"] == "CMD." + action)
    creation_by_action[action] = "GEO." + gid
    entries.append({"id": "GEO." + gid, "category": "table" if gid == "TABLE" else "basic-geometry", "labelFromManual": label, "objectName": action, "exactLabel": command["exactLabel"], "window": "待真实属性/绘制探针；既有动作排队未实际激活，不能推导没有属性框", "entryStatus": "observed-current-run", "evidenceKind": ["V:read-only-Qt-property", "H:help-body"], "routes": [{"kind": "menu-and-toolbar", "commandId": command["id"]}], "states": command["states"], "supplementalState": command["supplementalState"], "sourceObject": source, "deviceConsumer": consumer, "questionIds": qids, "helpTopic": {"path": topic, "htmlSha256": topics[topic]["html_sha256"]}})

for entry in entries:
    if entry["category"] == "named-action" and entry["objectName"] in creation_by_action:
        creation = next(e for e in entries if e["id"] == creation_by_action[entry["objectName"]])
        entry["creationTargetId"] = creation["id"]
        entry["sourceObject"] = {"status": "see-creation-target", "entryId": creation["id"]}
        entry["deviceConsumer"] = {"status": "see-creation-target", "entryId": creation["id"]}

historical = [
    ("btn_HisTrend", "withoutPrerequisiteConfiguration", "historically-observed-message", "尚未建立数据采集！", "历史趋势图功能与交互复刻手册.md", "§2及assets/01；仅工具箱入口，非菜单独立复测"),
    ("btn_HisTrend", "withPrerequisiteConfiguration", "historically-observed-dialog", "历史趋势图", "历史趋势图功能与交互复刻手册.md", "§3；已建立ddcc11/data_0的特定测试工程"),
    ("btn_hisdataDisplay", "withoutPrerequisiteConfiguration", "historically-observed-message", "没有建立数据采集", "历史数据显示器功能与交互复刻手册.md", "§2及assets/01；不与历史趋势图文案混同"),
    ("btn_hisdataDisplay", "withPrerequisiteConfiguration", "historically-observed-dialog", "历史数据显示器", "历史数据显示器功能与交互复刻手册.md", "§3/4；已建立CJ1最小采集"),
    ("btn_alarmbar", "withoutPrerequisiteConfiguration", "historically-observed-message", "位报警和条件报警都不存在", "动态报警条功能与交互复刻手册.md", "§3/asset02；标题动态报警条；不外推另两报警控件"),
    ("btn_alarmbar", "withPrerequisiteConfiguration", "historically-observed-dialog", "动态报警条", "动态报警条功能与交互复刻手册.md", "§3/4；已有最小数位地址报警"),
    ("btn__recipetable", "withPrerequisiteConfiguration", "historically-observed-dialog", "配方显示器", "配方显示器功能与交互复刻手册.md", "§3/4；工程中RCP1；无配方时未单独观察，不能补成禁用"),
]
for button, state, status, observation, filename, bound in historical:
    entry = next(e for e in entries if e["id"] == "TBX." + button)
    entry["states"][state] = {"status": status, "enabled": None, "behavior": observation, "evidence": "KinSealStudio/design_plan/" + filename, "boundary": bound, "currentRun": False}

# Use only the states actually confirmed by the bounded action probe.
state_r2 = json.loads((OUT / "entry-probe-states-r2.json").read_text(encoding="utf-8-sig"))
state_final = json.loads((OUT / "entry-probe-states.json").read_text(encoding="utf-8-sig"))
multi_snapshot = next(s for s in state_r2["snapshots"] if s["label"] == "after:action_checkall")
edit_snapshot = next(s for s in state_r2["snapshots"] if s["label"] == "after-private-right-key")
assert multi_snapshot["selectedCount"] == 12

def snapshot_action_map(snapshot):
    """Index the same actual QAction collection in one observed state."""
    mapped = {}
    for s in snapshot["surfaces"]:
        for row, path, pointer in walk(s.get("actions", []), [], ""):
            mapped[row["objectName"]] = row
    return mapped

multi_actions = snapshot_action_map(multi_snapshot)
edit_actions = snapshot_action_map(edit_snapshot)
for e in entries:
    if e["category"] == "named-action" and e["objectName"] in multi_actions:
        e["states"]["multipleSelection"] = {"status": "observed-property-after-real-QAction-select-all", "enabled": bool(multi_actions[e["objectName"]]["enabled"]), "selectedItemsCount": 12, "stateConfirmed": True, "currentRun": True, "pid": state_r2["pid"], "evidence": "entry-probe-states-r2.json#snapshots[label=after:action_checkall]", "boundary": "12 logical items selected in one retained five-class bilingual sample; not all multiselect combinations"}
        e["states"]["dirtyProject"] = {"status": "partial-scene-edit-and-native-save-indicator; exact-modified-state-unconfirmed", "enabled": bool(edit_actions[e["objectName"]]["enabled"]), "stateConfirmed": False, "nativeSaveEnabled": True, "nativeUndoEnabled": True, "qtWindowModified": False, "selectedGeometryXShift": 1, "sourceWasSaved": False, "pid": state_r2["pid"], "evidence": "entry-probe-states-r2.json#snapshots[label=after-private-right-key]", "boundary": "Scene x moved and native Save/Undo enabled, but no internal dirty flag or save/reopen transition verified; do not label complete dirty-state acceptance"}
    elif e["category"] in {"basic-geometry", "table"}:
        # These entry records reference exactly the same QAction, not toolbox QWidget state.
        cmd = next(x for x in entries if x["id"] == "CMD." + e["objectName"])
        e["states"] = cmd["states"]
    elif e["category"] == "toolbox-control":
        action = e["routes"][1]["commandId"].removeprefix("CMD.")
        e["states"]["multipleSelection"] = {"status": "associated-action-observed; toolbox-widget-not-reread", "enabled": None, "associatedActionEnabled": bool(multi_actions[action]["enabled"]), "selectedItemsCount": 12, "evidence": "entry-probe-states-r2.json#snapshots[label=after:action_checkall]", "stateConfirmed": True, "boundary": "Only associated QAction property is established; no extrapolation to the toolbox button itself"}

for qid, question, next_action in [
    ("Q-S00-ENTRY-STATE-SINGLE", "私有Win32点击/Qt事件分发均未取得selectedItems=1；单选动作状态矩阵仍未闭合。", "先定位myGraphicsView实际选择事件入口与逻辑对象/选择辅助框关系；在同一样本仅点击单个逻辑对象，记录真实selectedItems=1及同一121动作；不可直接setSelected伪装用户等价动作。"),
    ("Q-S00-ENTRY-STATE-DIRTY", "局部右键码消息使12对象x+1、保存/撤销enabled，但windowModified=false；正常Qt事件最终未产生保存转换。", "定位原生项目真实dirty成员或保存信号消费者，固定输入后用正常场景编辑确认dirty→保存→clean，并保存/重开证明持久化；保留Qt windowModified不参与的观测。"),
    ("Q-S00-ENTRY-STATE-EMPTY", "当前Probe和12控件基线均不能用作空工程状态；新建向导没有在本切片执行。", "按A02 P0在同型号新建工程，确认场景中逻辑对象计数0及源对象列表为空后采动作状态。"),
    ("Q-S00-ENTRY-STATE-CONFIG", "无配置/有配置只有旧手册特定报警/采集/配方样例，缺当前完整动作矩阵。", "分别建立无前置配置和仅一项最小报警/采集/配方的同基线工程；保存重开确认计数，分别触发4类依赖入口并采原始提示与动作状态。"),
    ("Q-S00-ENTRY-STATE-READONLY", "未确认V5.1何种原生模式构成只读工程，不能等同文件只读属性。", "先定位原生只读工程/保护打开的真实入口与mode getter，取得真实只读标志和禁止/允许编辑的用户操作，再采矩阵。"),
    ("Q-S00-ENTRY-STATE-RUNNING", "未执行编译/模拟运行，无法确认运行中动作可用性。", "在阶段7/5授权切片取得真实模拟/编译运行状态与生命周期后，对同一动作集合读取状态；不得用模态窗口禁用冒充运行中。"),
]:
    issue(qid, question, next_action, ["D", "U", "P"])

# Entries below remain explicit candidates when QObject names or runtime paths are unknown.
tree_specs = [
    ("TREE.COM.PROPERTIES", "连接→COM1/COM2→双击串口", "连接——串口", "Connection/Connection-detail.htm"),
    ("TREE.COM.ADD_PROTOCOL", "连接→COM1/COM2→右键→增加协议", "连接——协议", "Connection/Connection-detail.htm"),
    ("TREE.PAGE.NEW", "画面→右键→新建画面", "新建画面", "SceneAndWindow/SceneAndWindow.htm"),
    ("TREE.PAGE.OPEN", "画面名称→右键→打开", "画面编辑", "SceneAndWindow/SceneAndWindow.htm"),
    ("TREE.PAGE.DOUBLE_CLICK", "工程管理器→画面名称→双击", "画面编辑", "SceneAndWindow/SceneAndWindow.htm"),
    ("TREE.PAGE.PROPERTIES", "画面名称→右键→属性", "画面属性", "SceneAndWindow/SceneInfo.htm"),
    ("TREE.PAGE.COPY", "画面名称→右键→复制", "画面复制", "SceneAndWindow/copyScene.htm"),
    ("TREE.PAGE.DELETE", "画面名称→右键→删除", "删除确认", "SceneAndWindow/deleteScene.htm"),
    ("TREE.WINDOW.NEW", "窗口→右键→新建窗口（实际标签待验）", "新建窗口", "SceneAndWindow/SceneAndWindow.htm"),
    ("TREE.WINDOW.OPEN", "窗口名称→右键→打开", "窗口编辑", "SceneAndWindow/SceneAndWindow.htm"),
    ("TREE.WINDOW.DOUBLE_CLICK", "窗口名称→双击", "窗口编辑", "SceneAndWindow/SceneAndWindow.htm"),
    ("TREE.WINDOW.PROPERTIES", "窗口名称→右键→属性", "窗口属性", "SceneAndWindow/WindowInfo.htm"),
    ("TREE.WINDOW.COPY", "窗口名称→右键→复制", "窗口复制", "SceneAndWindow/copyWindow.htm"),
    ("TREE.WINDOW.DELETE", "窗口名称→右键→删除", "删除确认", "SceneAndWindow/deleteWindow.htm"),
    ("TREE.DATA.NEW", "系统参数→数据采集→右键→新建数据采集", "数据采集", "Data/Data.htm"),
    ("TREE.TRANSFER.NEW", "系统参数→资料传输→右键→添加资料传输", "资料传输", "Data-Transfer/Data-Transfer.htm"),
    ("TREE.RECIPE.NEW", "系统参数→配方→右键→新建配方/添加配方（帮助用词不同）", "配方", "Recipe/ChuangJian.html"),
    ("TREE.LANGUAGE", "系统参数→语言设置→右键（具体项待验）", "语言设置", "SystemSetUp/mainframeProject/languageSet.htm"),
    ("TREE.FILE_ACCESS", "系统参数→访问加密→右键（具体项待验）", "访问加密", "SystemSetUp/mainframeProject/visitPass.htm"),
    ("TREE.SYS.PARAMETERS", "系统设置→参数设置（真实入口手势待验）", "参数设置", "SystemSetUp/mainframeManager/modelType.htm"),
    ("TREE.SYS.HMI_STATUS", "系统设置→HMI状态（真实入口手势待验）", "HMI状态", "SystemSetUp/mainframeManager/showSetUp.htm"),
    ("TREE.SYS.PLC_CONTROL", "系统设置→PLC控制（真实入口手势待验）", "PLC控制", "SystemSetUp/mainframeManager/controlSetUp.htm"),
    ("TREE.SYS.FILE_PROTECTION", "系统设置→文件保护（真实入口手势待验）", "文件保护", "SystemSetUp/mainframeManager/dataControl.htm"),
    ("TREE.SYS.HMI_PROTECTION", "系统设置→HMI保护（真实入口手势待验）", "HMI保护", "SystemSetUp/mainframeManager/userPur.htm"),
    ("TREE.SYS.CLOCK", "系统设置→时钟设置（真实入口手势待验）", "时钟设置", "SystemSetUp/mainframeProject/clock.htm"),
]
for number, (eid, path, window, topic) in enumerate(tree_specs, 1):
    qid = issue("Q-S00-ENTRY-TREE-%02d" % number, "%s的V5.1精确QObject、路径、事务及源/设备影响待核。" % path, "在同型号独立测试工程定位%s；先保留树模型/菜单对象与状态，再执行用户等价操作；确定/取消/保存重开并比较唯一字段源及设备段。" % path, ["D", "U", "P", "C"])
    entry = {"id": eid, "category": "project-tree-or-configuration", "labelFromHelp": path, "objectName": None, "exactLabel": None, "window": window, "entryStatus": "help-candidate-not-currently-triggered", "evidenceKind": ["H:help-body"], "routes": [{"kind": "project-tree/context-or-double-click", "candidatePath": path}], "states": state_template(), "sourceObject": {"status": "unknown", "questionId": qid}, "deviceConsumer": {"status": "unknown", "questionId": qid}, "questionIds": [qid], "helpTopic": {"path": topic, "htmlSha256": topics[topic]["html_sha256"]}}
    if eid == "TREE.DATA.NEW":
        entry["entryStatus"] = "historically-observed-menu-and-dialog"
        entry["evidenceKind"].append("V:existing-manual")
        entry["historicalEvidence"] = {"path": "KinSealStudio/design_plan/历史趋势图功能与交互复刻手册.md", "section": "§2.1", "actualMenuText": "新建数据采集", "dialogTitle": "数据采集", "sourceAndDeviceMappingStillUnknown": True}
    entries.append(entry)

for eid, label, window, source in [
    ("TREE.ALARM.BIT.DOUBLE_CLICK", "系统参数→报警登录→数位地址报警→双击", "数位地址报警", "动态报警条功能与交互复刻手册.md"),
    ("TREE.ALARM.CONDITION", "系统参数→报警登录→条件报警（手势待验）", "条件报警", "A02_全局入口与工程配置规范.md"),
    ("TREE.KEYBOARD.NEW", "自定义键盘→右键→添加（实际标签待验）", "自定义键盘", "A02_全局入口与工程配置规范.md"),
    ("TREE.LOGO", "Logo候选节点/向导启动页关联待核", "待验", "A02_全局入口与工程配置规范.md"),
    ("GEO.CONNECTION_LINE.CANDIDATE", "连接线候选（静态线索，不计入9图形）", "待验", "原生软件复刻补充分析_20261110.md"),
    ("CTX.OBJECT.PROPERTIES", "画布对象→右键→属性", "当前选中对象属性", "历史趋势图功能与交互复刻手册.md"),
    ("DBL.OBJECT.PROPERTIES", "画布对象→双击", "当前对象属性", "配方显示器功能与交互复刻手册.md"),
]:
    qid = issue("Q-S00-ENTRY-ROUTE-" + eid.replace(".", "-"), "%s的全范围适用状态/源和设备影响待验。" % label, "先按%s记录已证范围，在独立测试工程补实际QObject/窗口、单选/多选/无配置/有配置/脏工程/只读或运行状态；涉及未知格式时取得保存重开与原厂离线差分，不用同族推断替代。" % label, ["D", "U", "P", "C"])
    historically_observed = eid in {"TREE.ALARM.BIT.DOUBLE_CLICK", "CTX.OBJECT.PROPERTIES", "DBL.OBJECT.PROPERTIES"}
    entries.append({"id": eid, "category": "other-route-or-candidate", "labelFromSource": label, "objectName": None, "exactLabel": None, "window": window, "entryStatus": "historically-observed-bounded" if historically_observed else "candidate-not-observed", "evidenceKind": ["V:existing-manual" if historically_observed else "H/B:plan-candidate"], "routes": [{"kind": "context-or-double-click-or-candidate", "path": label}], "states": state_template(), "sourceObject": {"status": "unknown-or-selected-object-dependent", "questionId": qid}, "deviceConsumer": {"status": "unknown-or-selected-object-dependent", "questionId": qid}, "questionIds": [qid], "evidence": ref(PLAN / source), "boundary": "历史趋势图右键属性、配方显示器双击各仅具体样例；不可扩为全部31控件均已观察。" if eid.startswith(("CTX", "DBL")) else "仅所引来源声明范围；未自动关闭字段、格式或设备运行。"})

root_menus = next(s for s in probe["runs"][1]["surfaces"] if s["qt"]["objectName"] == "menuBar")["reading"]["actions"]
assert len(root_menus) == 8
assert len({entry["id"] for entry in entries}) == len(entries)
assert sum(entry["category"] == "toolbox-control" for entry in entries) == 31
assert sum(entry["category"] == "basic-geometry" for entry in entries) == 9
assert sum(entry["category"] == "table" for entry in entries) == 1
for entry in entries:
    assert set(entry["states"]) == set(STATES)
for run in probe["runs"]:
    assert run["status"] == "bounded-read-only-captured"
    assert run["ownedProcessTerminated"]
    for surface in run["surfaces"]:
        def no_error(rows):
            for row in rows:
                assert not row.get("error"), row
                no_error(row.get("children", []))
        no_error(surface["reading"]["actions"])

categories = {key: sum(e["category"] == key for e in entries) for key in sorted({e["category"] for e in entries})}
result = {"schemaVersion": 1, "date": "2026-11-10", "taskIds": ["S00-02", "S00-05"], "scope": "AMT070W124-L800X480R native entry identities, known sources, explicit eight-state gaps; no feature implementation or device-runtime pass", "coverageStatus": "bounded-inventory-with-explicit-candidates-and-gaps", "evidencePolicy": {"V-current": "本轮独立PID只读Qt属性", "V-historical": "已有手册所记具体输入/行为；未当本轮重跑", "H": "帮助正文静态信息", "B": "静态线索，仅候选", "unknown": "未观察不写禁用；无属性窗不能由排队未激活推导"}, "sources": [ref(PROBE_PATH), ref(OUT / "entry-probe-readonly.py"), ref(OUT / "entry-probe-readonly-r1.json"), ref(OUT / "entry-probe-readonly-r1.py"), ref(OLD_PATH), ref(PLAN / "分阶段文档证据/后台入口采集.py"), ref(HELP_PATH), ref(PLAN / "A02_全局入口与工程配置规范.md"), ref(PLAN / "A03_基础图形与表格规范.md"), ref(PLAN / "A05_源格式与设备编译契约.md")], "identity": {"originalPath": probe["originalPath"], "originalSha256": probe["originalSha256"], "runtimeSha256": probe["runtimeSha256"], "patch": probe["patch"], "desktop": probe["desktop"], "pids": [r["pid"] for r in probe["runs"]], "input": probe["runs"][1]["input"], "processesTerminated": True}, "rootMenus": [{"exactLabel": r["text"], "menuPointerForThisRunOnly": r["menuPointer"]} for r in root_menus], "stateDefinitions": STATES, "globalStateCoverage": {"noProject": "observed-current-run; exact QAction/QWidget enabled, no activation", "openedUnmodifiedProbe": "observed-current-run supplementary state; no verified selection/config inventory", "emptyProject": "not-established-from-opened-sample", "singleSelection": "not-observed-as-complete-action-state-matrix", "multipleSelection": "not-observed", "withoutPrerequisiteConfiguration": "historical bounded behavior only for dynamic alarm/history trend/history data; remaining unknown", "withPrerequisiteConfiguration": "historical bounded behavior for dynamic alarm/history trend/history data/recipe; remaining unknown", "dirtyProject": "not-observed", "readOnlyOrRunning": "neither state observed; no inference of disabled"}, "counts": {"entryRecords": len(entries), "categories": categories, "namedActions": len(opened), "rootMenus": len(root_menus), "toolboxControls": 31, "basicGeometries": 9, "tables": 1, "questions": len(issues), "countNote": "命令、工具箱、图形是入口记录/别名层，不能相加当原生独立功能数；表格不计31。"}, "probeCorrection": {"previousArrayOffset": 16, "adoptedArrayOffset": 20, "basis": "本轮r1对每个列表首项读到0/垃圾且末项缺失；r2保留20B原始QList头并按20读取，两状态所有已枚举菜单/工具栏0读异常、31+表格/9图形对应完整。仅该Qt4构建适用，不推广为文件协议或其他Qt版本ABI。", "failedProbeRetained": "entry-probe-readonly-r1.json", "oldSnapshotUnaffected": True, "oldDrawingActionProbe": "10 actions queue_status=queued but activation_result=null; not successful activation"}, "entries": entries, "questions": issues, "acceptance": {"AC-S00-02": {"status": "bounded-ledger-populated", "passed": ["every listed entry has observed source or explicit candidate", "stable IDs and precise returned QAction names", "31/9/1 separate", "8 required states explicit", "unknown mappings have Q-ID and next action"], "notPassed": ["full runtime entry behavior", "all project-tree/context routes", "all eight-state activation matrix", "all source/device consumers"]}, "AC-S00-05": {"status": "evidence-method-applied-to-this-bounded-probe", "passed": ["original/runtime distinct hash", "two owned private-desktop PIDs recorded and terminated", "input sample digest and before/after windows", "failed r1 retained; corrected read errors not called original disablement", "historical/source/current evidence separated"], "notPassed": ["full UI field/gesture/save/reopen behavior", "device execution semantics"]}}, "newDeviceTransfer": False, "newSourceCompile": False}
for filename in ("entry-probe-readonly-r2.json", "entry-probe-readonly-r2.py", "entry-probe-states.json", "entry-probe-states.py", "entry-probe-states-r1.json", "entry-probe-states-r1.py", "entry-probe-states-r2.json", "entry-probe-states-r2.py", "entry-probe-states-r3.json", "entry-probe-states-r3.py", "entry-probe-states-r4.json", "entry-probe-states-r4.py", "entry-probe-close-prompt-win32.json"):
    result["sources"].append(ref(OUT / filename))
result["stateProbeSummary"] = {"final": {"pid": state_final["pid"], "status": state_final["status"], "singleSelectionStatus": state_final.get("singleSelectionStatus"), "dirtyStateStatus": state_final.get("dirtyStateStatus"), "multipleSelectionStatus": state_final.get("multipleSelectionStatus"), "ownedProcessTerminated": state_final["ownedProcessTerminated"]}, "adoptedMultipleSelection": {"pid": state_r2["pid"], "selectedCount": multi_snapshot["selectedCount"], "evidence": "entry-probe-states-r2.json", "action": "action_checkall", "normalActivationConfirmed": True}, "dirtyPartialOnly": {"pid": state_r2["pid"], "selectedGeometryXShift": 1, "saveAndUndoTransition": "disabled→enabled", "windowModified": False, "sourceChanged": False, "notConfirmedAsFullDirtyState": True}, "failuresRetained": {"r1": "mapFromScene used wrong struct-return ABI; failed before single selection", "r2": "Win32 click selected helper/group state; only multiselect and partial edit indicators adopted", "r3": "Win32 routed clicks failed to establish single selection and an unexpected trend dialog appeared; modal cancelled only in owned PID; not a save-prompt claim", "r4": "default QString constructor export absent; instrumentation did not expose RPC; no state inferred", "r5": "corrected Qt constructors dispatched normal events but single/dirty not established; final bounded attempt"}}
result["globalStateCoverage"]["multipleSelection"] = "observed-current-r2: real action_checkall activated, selectedItems=12; same 121 QAction collection read"
result["globalStateCoverage"]["dirtyProject"] = "partial-current-r2: 12 selected geometry x+1, Save/Undo enabled, windowModified=false; no confirmed persistence/internal modified transition"
result["globalStateCoverage"]["singleSelection"] = "attempted-current-r1..r5; no selectedItems=1 confirmed; failure retained"
result["acceptance"]["AC-S00-02"]["passed"].append("normal select-all activation and exactly 12 selected items with same action-state collection")
result["acceptance"]["AC-S00-05"]["passed"].append("bounded single/dirty attempts, unexpected modal and ABI/export failures retained without upgrading acceptance")
(OUT / "entry-inventory.json").write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def cell(value):
    """Escape multiline evidence labels for a Markdown cell without changing JSON."""
    return str(value).replace("|", "\\|").replace("\n", "<br>")


lines = ["# 阶段0入口与后台证据台账", "", "日期：2026-11-10；任务：S00-02、S00-05。机器可读完整字段见[entry-inventory.json](entry-inventory.json)，原始快照见[entry-probe-readonly.json](entry-probe-readonly.json)，可复现配方见[entry-probe-readonly.py](entry-probe-readonly.py)。", "", "本轮补齐了已返回入口的身份、菜单/工具栏归属和无工程状态；没有执行入口业务动作、保存重开、原厂编译或USB操作。全入口行为与全部8状态仍未闭合，不宣称阶段0已使全部功能可下载。", "", "## 身份和补采范围", "", "- 原件SHA256：`%s`。" % probe["originalSha256"], "- 私有运行副本SHA256：`%s`；仅0x7c4..0x7c6启动检查三字节`5589e5→b001c3`，不称未修改原件运行。" % probe["runtimeSha256"], "- 桌面`%s`；无工程PID %s、已打开未改Probe PID %s，均已终止。没有切换桌面或调用系统键鼠/前置窗口。" % (probe["desktop"], probe["runs"][0]["pid"], probe["runs"][1]["pid"]), "- 输入`KinSealStudio/samples/native-device-test/CodexProbeTest.sm`，SHA256 `%s`。本轮没有确认该样本对象选择数与配置全集，因此不把“已打开Probe”改称空工程或有配置状态。" % probe["runs"][1]["input"]["sha256"], "", "## 旧采集器缺项的纠正", "", "旧采集器按Qt QList数组偏移16读取；本轮r1重现每列表首项垃圾、末项漏采。纠正为本构建的20后，两状态全部已枚举菜单/工具栏0读取异常。r2每行保留20字节头供复核；[失败r1](entry-probe-readonly-r1.json)与[当时配方](entry-probe-readonly-r1.py)均保留。该ABI事实只适用已记录的Qt4构建，不能当.sm或设备协议字段。旧JSON保持原样。", "", "根菜单实际为8个：" + " → ".join("`" + r["text"] + "`" for r in root_menus) + "。旧快照漏掉最后“语言(&R)”及多个子菜单最后项；补得关于、安装usb驱动、辅助功能、工程图库、宏管理器、下移/取消组合/解锁/垂直翻转/缩小/同宽高及报警/数据群组入口。", "", "旧10个图形探针只有`queue_status=queued`而`activation_result=null`；动作未证实激活，不能把无对话框判断为图形没有属性。", "", "## 计数与8状态边界", "", "| 类别 | 入口记录数 | 说明 |", "|---|---:|---|", "| 实际命名QAction | %d | 菜单与工具栏按对象名去重，保留每处路径 |" % len(opened), "| 工具箱 | 31 | 行序来自既有手册；btn原文多数空text，实际中文标签不写回按钮text |", "| 基础图形 | 9 | 独立于31 |", "| 表格 | 1 | 控件工具栏第32项，独立于31 |", "| 工程树/配置与其他候选 | %d | 来源与实际观察状态逐条明确 |" % (categories["project-tree-or-configuration"] + categories["other-route-or-candidate"]), "", "这些是入口层记录而不是独立功能数量。`toolBar_Control`有31控件+表格32项；`toolBar_Graphs`有9图形。", "", "| 要求状态 | 现有证据 |", "|---|---|", "| 无工程 | 本轮真实Qt QAction/QWidget enabled；没有触发行为 |", "| 空工程 | 未由本轮样本确认，不外推 |", "| 单选 | 未建立完整命令矩阵；具体旧手册回填不等于全部action状态 |", "| 多选 | 未观察 |", "| 无前置配置 | 旧手册独立观察动态报警条/历史趋势图/历史数据显示器门槛；配方显示器无配方状态仍待验 |", "| 有配置 | 旧手册独立观察最小报警/采集/配方具体样例；不外推所有配置组合 |", "| 脏工程 | 未观察 |", "| 只读或运行中 | 两类均未观察，不写禁用 |", "", "补充状态“已打开未改Probe”的全部Qt属性保存在JSON。该状态保存、撤销、恢复、取消组合disabled；无工程上载数据enabled，控件/下载/大多数编辑动作disabled。属性是当时状态，未验证点击结果。", "", "## 31个工具箱入口", "", "| ID | 名称 | btn对象 | 实际菜单/工具栏action | 属性窗口（既有手册） | 格式/消费者 |", "|---|---|---|---|---|---|"]
for e in entries:
    if e["category"] != "toolbox-control":
        continue
    action = e["routes"][1]["commandId"].removeprefix("CMD.")
    mapping = "A05既有受限映射" if e["sourceObject"]["status"].startswith("known") else e["sourceObject"]["questionId"]
    lines.append("| %s | %s | `%s` | `%s` | %s | %s |" % (e["id"], cell(e["labelFromManual"]), e["objectName"], action, cell(e["window"]), mapping))
lines += ["", "历史趋势图action实际为`action_DatagroupDisplay`；数据群组为新补`action_dataGroup`。不能按英文片段交换。报警三项分别为`action_dynamicarlarm`、`action_histotyarlarm`、`action_arlamcontrol`（原文拼写保留）。", "", "## 9图形与表格", "", "| ID | 名称 | QAction | 源/设备映射待验 |", "|---|---|---|---|"]
for e in entries:
    if e["category"] in {"basic-geometry", "table"}:
        lines.append("| %s | %s | `%s` | %s |" % (e["id"], cell(e["labelFromManual"]), e["objectName"], e["sourceObject"]["questionId"]))
lines += ["", "## 主菜单和工具栏命令", "", "精确标签保留原始换行和乱码；JSON原文未改。快捷键通过QAction::shortcut和QKeySequence::toString(PortableText)直接读取；空序列不证明不存在自定义按键事件。`Ctrl+P`为属性设置；多重复制本轮标准QAction快捷键为空，画布/文本/模态焦点的实际事件仍待Q-A02-CMD-001。锁定与解锁均返回Ctrl+L；同时存在不代表两个动作在每个上下文均触发。", "", "| 稳定ID | 精确标签 | 菜单/工具栏归属 | 无工程enabled | 已开未改Probe enabled | QAction快捷键 |", "|---|---|---|---|---|---|"]
for e in entries:
    if e["category"] == "named-action":
        surfaces = "; ".join("/".join(route["path"]) if route["kind"] == "menu" else route["surface"] for route in e["routes"] if route["kind"] in {"menu", "toolbar"})
        lines.append("| %s | %s | %s | %s | %s | %s |" % (e["id"], cell(e["exactLabel"]), cell(surfaces), e["states"]["noProject"].get("enabled"), e["supplementalState"]["openedUnmodifiedProbe"]["enabled"], cell(e["shortcut"]["portableText"])))
lines += ["", "## 工程树、右键、双击及候选", "", "本轮读取主窗口和工具栏，不向树发上下文动作。帮助/旧手册已给路径的条目继续保留；QObject未知写null，不伪造对象名。每行的问题与下一动作已在JSON的questions关联。", "", "| 稳定ID | 来源路径/标签 | 入口状态 | 窗口/结果 |", "|---|---|---|---|"]
for e in entries:
    if e["category"] in {"project-tree-or-configuration", "other-route-or-candidate"}:
        lines.append("| %s | %s | %s | %s |" % (e["id"], cell(e.get("labelFromHelp", e.get("labelFromSource"))), e["entryStatus"], cell(e["window"])))
lines += ["", "## 未闭环与下一步", "", "- 每个命名动作的业务与焦点行为用`Q-S00-ENTRY-CMD-*`追踪；工具箱/图形/表格未知源与设备消费者用`Q-S00-ENTRY-MAP-*`逐项追踪。已有5类只引用A05受限范围；此次未重跑其格式/设备测试。", "- 工程树/配置项用`Q-S00-ENTRY-TREE-*`，其余路线/连接线候选用`Q-S00-ENTRY-ROUTE-*`；每个Q在JSON写出指定窗口/动作、源A/A2/B和原厂重开/离线差分的下一探针。", "- Qt读取失败、事件未激活、真实disabled分别记录。r1读取错误已保留；旧图形未激活仍待行为探针；新属性disabled只用于对应状态。", "- 本轮没有借入口台账开放未知编译/下载，也没有把USB ACK升级为屏端行为通过。", "", "恢复点：先按`Q-A02-CMD-001`建立空工程/单选/多选/脏/只读/运行的状态矩阵，再在各阶段执行对应单个命令的业务探针；未知源/消费者维持拒绝。", ""]
lines[4] = "本轮补齐已返回入口的身份、菜单/工具栏归属、无工程状态，并通过正常QAction全选确认12对象多选。单选/脏工程补采仅取得部分状态，未闭合。没有原厂编译、USB或PLC操作，不宣称阶段0已使全部功能可下载。"
text = "\n".join(lines).replace("| 多选 | 未观察 |", "| 多选 | 已真实激活action_checkall，selectedItems=12；同一121动作已读属性 |")
text = text.replace("| 脏工程 | 未观察 |", "| 脏工程 | 仅部分证据：12对象x+1、Save/Undo enabled；windowModified=false，未确认内部dirty/保存重开转换 |")
text = text.replace("| 单选 | 未建立完整命令矩阵；具体旧手册回填不等于全部action状态 |", "| 单选 | Win32/Qt事件补采均未确认selectedItems=1；失败保留，不算通过 |")
text += "\n## 有界状态补采与冻结\n\n最终配方与记录：[entry-probe-states.py](entry-probe-states.py)、[entry-probe-states.json](entry-probe-states.json)。采用的多选证据在[状态r2](entry-probe-states-r2.json)：GUI线程正常QAction::activate(action_checkall)实际执行，selectedItems从0变12；此后采同一121个QAction。单选点击与正常Qt鼠标事件最终均未达到1，不写成功。\n\nr2私有键事件曾使12对象x各+1、Save/Undo由0变1，而QWidget windowModified=false；这只证明场景变化与原生保存指示，尚未读到真实dirty字段/完成持久化转换。r3异常打开趋势图窗口已取消并留Win32窗口记录，不能把它改称保存提示；r4缺QString默认构造导出、r1返回ABI错误均保留。最终r5 PID %s已终止，Qt事件分发但单选/保存转换仍未成立。没有使用Windows共享剪贴板。\n\n空工程、配置、只读与运行的具体阻塞、单选及dirty下一探针分别为Q-S00-ENTRY-STATE-EMPTY/CONFIG/READONLY/RUNNING/SINGLE/DIRTY，完整问题和下一动作在JSON。此次探针范围已冻结，未继续试配置或模拟。\n" % state_final["pid"]
(OUT / "entry-inventory.md").write_text(text, encoding="utf-8")
print(json.dumps({"counts": result["counts"], "outputs": ["entry-inventory.json", "entry-inventory.md"], "newProbePids": result["identity"]["pids"]}, ensure_ascii=False))
