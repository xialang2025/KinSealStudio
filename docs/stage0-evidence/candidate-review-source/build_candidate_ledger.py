"""#20261110-4: Render an explicit semantic review without changing the frozen CSV."""
from __future__ import annotations
import csv
import hashlib
import io
import json
import sys
from collections import Counter
from pathlib import Path

WORKSPACE = Path(r"Z:\Project\vscode_prj_temp\REA")
PROJECT = WORKSPACE / "KinSealStudio"
HERE = Path(__file__).resolve().parent
OUT = PROJECT / "docs/stage0-evidence"
CSV_PATH = PROJECT / "design_plan/复刻待验线索台账.csv"
EXPECTED_SHA = "27f423c4b35d387b243d0e4ca69485ff2a892b41e8b3cd81e19cfc81b532b87c"

# Every classification and question below originates in review.tsv; no keyword classifier is used.
MODULES = [
    ("静态文本", "C01", "TEXT"), ("位按钮", "C02", "BIT"), ("字按钮", "C03", "WORD"),
    ("指示灯", "C04", "LAMP"), ("功能按钮", "C05", "FUNC"), ("消息显示", "C06", "MESSAGE"),
    ("刻度", "C07", "SCALE"), ("二维码", "C08", "QR"), ("图片显示", "C09", "PICTURE"),
    ("GIF显示", "C10", "GIF"), ("数值显示", "C11", "NUMBER"), ("ASCII显示", "C12", "ASCII"),
    ("时间显示", "C13", "TIME"), ("画面按钮", "C14", "SCREEN"), ("多态按钮", "C15", "MULTISTATE"),
    ("多功能按钮", "C16", "MULTIFUNC"), ("下拉框", "C17", "DROPLIST"), ("流动块", "C18", "FLOW"),
    ("滑动开关", "C19", "SLIDE"), ("滚轮", "C20", "WHEEL"), ("移动轨迹", "C21", "TRACK"),
    ("图表", "C22", "GRAPH"), ("趋势图", "C23", "TREND"), ("历史趋势图", "C24", "HISTORYTREND"),
    ("X-Y曲线", "C25", "XY"), ("数据群组", "C26", "DATAGROUP"), ("动态报警条", "C27", "ALARMBAR"),
    ("报警显示", "C28", "ALARM"), ("历史报警显示", "C29", "HISTORYALARM"),
    ("历史数据显示器", "C30", "HISTORYDATA"), ("配方显示器", "C31", "RECIPE"),
]
MODULE_MAP = {name: dict(control_id=cid, slug=slug) for name, cid, slug in MODULES}
CLASS_NAMES = {"S": "单一问题", "C": "复合父项", "E": "已有证据", "N": "说明文字", "D": "重复"}

# Probe recipes specify observations, not assumed native answers. Targets come from the explicit row review.
PROFILES = {
    "u": ("UI", ["D", "U"], "在原生V5.1最小工程打开该控件，保留真实Qt enabled/visible/checked及原字段快照。",
          "对指定字段或入口执行真实等价点击；记录前后Qt属性、可见子项及确认、重开结果。",
          "取得该字段或入口的直接状态与联动记录；输入无效、真正禁用和缺前置配置分别判断。"),
    "o": ("OPTIONS", ["D", "U"], "打开指定下拉或列表，记录当前工程配置及真实Qt count。",
          "逐索引读取文本、enabled和数据，并滚动首尾；确认首项、末项及是否仍可滚动。",
          "形成有索引的完整项集；帮助范围及可见弹层高度不能替代完整枚举。"),
    "b": ("BOUND", ["D", "U", "P"], "记录指定字段真实Qt minimum/maximum/validator、默认值及数据类型。",
          "清空后键入空、负数、小数及边界内外值；若有Qt边界，测min-1/min/min+1/max-1/max/max+1，再确定和重开。",
          "记录拒绝、钳制、回退、原值保留或弹窗的精确结果；实现保护范围不当作原生范围。"),
    "p": ("PERSIST", ["U", "P"], "保留可辨识原值和工程副本，固定当前语言、状态、类型及对象ID。",
          "按问题所述路径改变单个字段或模式，分别确定与取消；保存、关闭、重开并比对字段及源文件。",
          "取得提交、取消和休眠草稿的保存寿命；区分ksp草稿与原厂sm实际保存字段。"),
    "r": ("RENDER", ["R"], "固定原生版本、DPI、字体、对象几何、颜色和背景，选择可辨识的非默认文本或图像。",
          "对指定绘制参数作单变量改变，分别记录属性预览、画布和可取得的原厂编译解压像素。",
          "给出逐像素或可定位几何差分；PC截图、编译像素与实屏显示分层，不由外观名称推算法。"),
    "s": ("SOURCE", ["P"], "准备原厂空白基线与仅含该对象的最小sm，保留源文件SHA及原件/副本身份。",
          "逐字段改一个值后保存重开，解析typed流并比对；复制、删增或改变状态数时另留独立样本。",
          "定位稳定字段、长度、引用及休眠数据；只能在明确支持边界内声明可编辑互通。"),
    "c": ("COMPILE", ["C"], "使用最小原厂sm及已固定默认AMT目标；隔离各次编译输出并记录输入SHA。",
          "单变量原厂离线编译，比较对应控制、图片、文本或配置段；需要图像时独立解压并核对索引。",
          "形成活动字段、字节序、段结构及图片数量的证据；随机残留与未初始化区域单列。"),
    "e": ("RUNTIME", ["E"], "先固定AMT型号、固件与工程/程序哈希，取得原生期望；具备可记录的屏幕及LB/LW或PLC观察。",
          "针对问题中的单一动作施加可辨识激励，分别记录刺激前、按下、保持、松开、取消及必要的重启后状态。",
          "记录屏端或寄存器的实际结果、时间与顺序；没有原生观察不预设答案，USB ACK不算运行结果。"),
    "w": ("PERMISSION", ["E"], "选择该控件的显现或触控列，单独建立位、等级、字条件，且对象无重叠遮挡。",
          "分别使各条件全真、仅一真、仅一假和全假，保持其他参数不变，记录显现与触控写值结果。",
          "取得显现与触控各自的真值表、组合方式及优先级；不由共用容器推断所有控件相同。"),
    "t": ("THRESHOLD", ["E"], "固定该控件的最低等级阈值，位和字条件关闭或恒定满足。",
          "分别以阈值-1、阈值、阈值+1等级观察显现或触控；显现和触控列分开记录。",
          "取得最低等级的等值边界与实际权限结果，不由字段名称预设闭开关系。"),
    "f": ("TOLERANCE", ["E"], "固定字比较值和误差，位和等级条件恒定满足；整数和浮点分别留样本。",
          "测比较值±容差及邻界，分别测试等于、不等于及0误差，保持其他字段不变。",
          "取得容差闭开区间、绝对/相对误差及浮点边界，不提前假设具体公式。"),
    "h": ("HELP", ["D", "U"], "确认原CHM路径/哈希及成员存在，记录完整安装依赖和私有副本环境差异。",
          "仅在自有原生私有桌面点击该对话框帮助，记录新窗口、目标URL或错误、等待时间及进程状态。",
          "确认目标成员或有界失败；CHM成员存在不能替代按钮直达事实。"),
    "a": ("ADDRESS", ["D", "U", "C"], "固定所用地址角色、位宽、连接/型号和通道；保留无连接及已配置工程两份。",
          "从指定控件字段打开拾取器，枚举相应家族并选首末/宽地址边界，确认与取消后核对回填和跨度。",
          "取得此调用点的地址角色与可达族；别的控件同款窗口仅作线索，不能代替本入口。"),
    "g": ("GALLERY", ["U", "P", "R"], "建立独立工程图库，加入有唯一内容的两张图并设置一个对象引用，保留资源及工程SHA。",
          "按问题中的系统/工程图库路径选择、添加、删除或导出，确认及取消后保存重开并检查引用与像素。",
          "确定列表、路径、引用、删除恢复及落盘时机；共用图库实现不证明每个控件调用点相同。"),
    "v": ("VISIBILITY", ["D", "U"], "固定当前AMT和V5.1；B/H隐藏字段只作线索，另列型号或版本扩展范围。",
          "在明确的工程前置与控件模式矩阵中读取真实Qt visible/enabled，搜索已指明的入口或调用点。",
          "只对搜索边界报告可达、隐藏或未找到；不因字符串存在增加字段，也不推断所有型号不存在。"),
    "d": ("SCOPE", ["D"], "并列原始候选、原帮助及当前V5.1直接证据，保留版本/型号和文档章节。",
          "核对问题中的版本、能力或归属差异，列出适用和不适用范围；缺失运行事实另建E分支。",
          "形成可追溯的来源与采用边界；说明句或旧图不自动成为待实现功能或运行算法。"),
    "l": ("LANGUAGE", ["D", "U", "P"], "建立中文单语及中文/English/第三种语言工程，给各状态各语言写不同文本。",
          "切换工程语言、状态及同文开关，记录下拉顺序；确定、保存重开，检查独立文本与共享文本。",
          "确认语言列表来源、索引及草稿规则；运行切换是否正确仍需独立E观察。"),
    "x": ("DIAGNOSTIC", ["D", "U"], "保留发生问题的源工程、原生身份、私有桌面PID、事件类型和退出码。",
          "仅在自有实例串行最小复现，区分直接点击、Qt等价事件及重试；核对已引用截图来自哪次成功会话。",
          "确定采集可复现性或留下有界异常；未证实原因不得写成试用状态或其他程序冲突，更不要求复刻崩溃。"),
    "decision": ("DECISION", ["D", "U"], "保留已观察到的原生瑕疵及受影响字段，不改变其事实记录。",
          "比较保留原生表现与采用明确差异的用户影响，登记采用方案及相关验收例。",
          "产生显式产品差异记录；不把原生瑕疵改述为正确业务行为。"),
    "closed": ("FACT", ["D", "U"], "复用旧核销中相同V5.1身份及AMT范围。",
          "核对原核销引用与现存证据身份；本任务不重跑原生UI或硬件。",
          "只沿用已关闭事实及其版本/型号/层，不扩大为整个控件或E通过。"),
}

REPORTS = {
    "静态文本": ("EV-S00-CAND-TEXT", "docs/静态文本手册实现与验收.md", "历史0.3实现；开头明确0.4已扩展多语言及显现编译，旧拒绝表不可恢复为当前边界。仅一个指定外观/位置样例有用户现场确认。"),
    "位按钮": ("EV-S00-CAND-BIT", "docs/位按钮手册实现与验收.md", "0.4设备编译及后续图库版：宏工具链、权限记录、通知、语言图帧及sm互通有历史局部证据；四操作和权限现场待确认。"),
    "字按钮": ("EV-S00-CAND-WORD", "docs/字按钮手册实现与验收.md", "0.5.0：49组合活动字段、源往返及OFF/ON帧已有历史对照；键盘、寄存器运算、通知和权限实屏尚待反馈。"),
    "指示灯": ("EV-S00-CAND-LAMP", "docs/指示灯手册实现与验收.md", "0.6.0：真实Qt禁用、2..32范围、图序、默认/删除及无图同文已有补证；现场状态、闪烁与权限仍未确认。"),
    "功能按钮": ("EV-S00-CAND-FUNC", "docs/功能按钮手册实现与验收.md", "0.7.0：18功能/类型、Qt禁用、隐藏ON及源/编译已有补证；USB只证明T，回执明确screenDisplayAndTouchConfirmed=false。"),
}

# These notes are explicit adoption decisions per reviewed candidate, not broad report-based closure.
PARTIAL = {
    53: "真实SDK编译与宏源保存已有P/C证据；原厂宏窗口全部交互不能仅凭Lite测试关闭。",
    54: "位按钮报告已支持宏编辑绑定，旧合成点击失败不再当作宏不可用；精确原生启用链仍按引用复核。",
    55: "位按钮报告已有互斥通知UI和设备记录补证；本控件旧禁用说法不能直接作为产品禁用条件。",
    56: "原厂两语言样本、图序和闪烁无字帧已有P/C证据；闪烁频率和运行切换保持E未验。",
    58: "位号及图像编码已有C证据；位值运行映射和完整合成公式保留开放。",
    62: "受支持位按钮sm和640B设备记录已有P/C；固件时序与全部组合仍未验。",
    75: "原厂双语单控件语言count=2及对应编译已补证；完整语言顺序/配置变更矩阵不由此全关。",
    76: "原厂Qt getter已确认两项复制按钮disabled；作为已有事实复用，不为不存在的可用复制路径再造窗口。",
    77: "隐藏state1橙ON、编辑state0不自动改ON及OFF/flash/ON图序已解码；E按下显示未确认。",
    81: "subtype3 typed源union与640B设备字段及全新样例往返已补证；E解析及动作不由ACK关闭。",
    156: "字按钮报告已有宏编辑绑定和真实SDK编译；旧合成点击失败不能推定宏禁用。",
    157: "字报告及Qt测试覆盖复制与state1禁用；原厂逐字段复制采用范围需核对原证据，不能把测试实现当原生事实。",
    158: "字报告已有通知字五类型和位项禁用规则；取消再开启的原生草稿恢复仍应逐路径核验。",
    159: "ksp保留两态草稿且sm单态列表不能保存休眠另一态已解释；原厂交互路径与Lite保存声明分开。",
    161: "实现采用可表示数值域和保护范围不等于原厂全域输入上限；透明和全部渐变仍有边界。",
    162: "字按钮语言和闪烁编译有历史局部证据，运行切换和周期不由C推出。",
    167: "49种原厂字记录活动字段及源往返已有P/C；休眠字段、残留及E动作未等同。",
    176: "指示灯报告原厂补证第1项：两处同地址复选框均disabled，不能沿旧手册要求开放同步。",
    177: "指示灯报告原厂补证第1项：通知整个组disabled；不继续实现不可用通知展开。",
    180: "当前AMT实际多态监视已恢复U16/U32；同SHA当前范围直接复用，不能据此说所有型号只有两项。",
    181: "旧核销的2..32/0..N-1事实关闭继续有效；报告还解释状态图序、新增默认及5→2→5删除，但E渲染分支不闭合。",
    182: "报告第3项解释新建15/存储0及切字监视可存15；真实位值取值另验。",
    183: "报告第4及9项已有图序、同文图数；屏端显示与权限不以其代替。",
    184: "报告第6、7项解释新增默认和显式删除的草稿寿命；3D实屏语义不扩展。",
    186: "双语多态原厂复开编译和状态独立文字已有补证；完整语言配置变更矩阵仍保留。",
    187: "Lamp typed源、kind1/3及subtype4图帧已补证；固件最终显示和权限仍未验。",
    301: "静态文本报告明确字号/字距等实现保护范围不是已测原生极限。",
    302: "13种默认100×100填充容差及指定粗体帧有R证据；非方形、透明全域和跨分辨率未穷尽。",
    303: "需采用0.4后type28多语言/显现编译补证；旧报告设备多语言拒绝文字仅为历史，E切换仍未验。",
    305: "Lite入口取消、属性与双击测试已覆盖；原生同链及帮助直达必须按本条分别核实。",
    306: "位按钮后续报告已有139配置与1689/1724族纯C++地址编码；PC/屏端通信、字序及浮点字节序未因此关闭。",
    307: "type28及静态背景编译、typed源已有后续P/C；特定样例现场显示不推出全固件字段兼容。",
}

# Ownership follows the actual subsystem, while the frozen source module is always retained.
OWNERS = {
    69: ("A02全局入口/仪表归属待核", ["C07", "C22"]),
    86: ("A02报警登录配置", ["C27"]), 87: ("A02报警登录配置", ["C27"]),
    89: ("A02条件报警配置", ["C27", "C28", "C29"]),
    94: ("A02历史报警清除入口调查", ["C27", "C28", "C29"]),
    104: ("A02数据采集配置", ["C30", "C24"]),
    106: ("A02采集数据项/C24历史趋势", ["C24", "C30"]),
    107: ("A02数据采集配置", ["C24", "C30"]),
    155: ("C04指示灯，原多态按钮手册范围转交", ["C04", "C15"]),
    298: ("A02配方配置/C31配方显示", ["C31"]),
}


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def rel(path: Path) -> str:
    return path.relative_to(PROJECT).as_posix()


def main() -> None:
    raw = CSV_PATH.read_bytes()
    assert digest(raw) == EXPECTED_SHA, "Frozen CSV identity changed; do not regenerate or overwrite it."
    rows = list(csv.DictReader(io.StringIO(raw.decode("utf-8-sig"))))
    assert len(rows) == 308
    assert [row["线索ID"] for row in rows] == [f"U-{n:03d}" for n in range(1, 309)]
    specs = {}
    for line in (HERE / "review.tsv").read_text(encoding="utf-8").splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        number, classification, items = line.split("|", 2)
        n = int(number)
        assert n not in specs
        specs[n] = (classification, [tuple(item.split("@", 1)) for item in items.split("~")])
    assert set(specs) == set(range(1, 309)), "Every source row requires an explicit semantic decision."

    evidence = {}
    def add_evidence(eid: str, path: str, scope: str) -> str:
        file = PROJECT / path
        data = file.read_bytes()
        evidence[eid] = dict(id=eid, path=path, sha256=digest(data), size=len(data), supports=scope,
                             used_as_new_execution=False)
        return eid

    add_evidence("EV-S00-CAND-CSV", rel(CSV_PATH), "冻结308候选身份、原文及原始来源；不是问题答案或测试结果。")
    add_evidence("EV-S00-CAND-PRIOR", "design_plan/复刻待验核销记录.md", "沿用3事实关闭、U181渲染开放、U079父项开放、U308非问题。")
    add_evidence("EV-S00-CAND-STAGE0", "design_plan/01_阶段0_基线范围与证据管理.md", "S00-04核销方法、范围与放行边界。")
    add_evidence("EV-S00-CAND-ENTRY", "design_plan/00_复刻执行总入口.md", "当前用户授权下的阶段方法；文档原始调查模式不替代本轮阶段0实现授权。")
    add_evidence("EV-S00-CAND-A04", "design_plan/A04_31控件实施索引.md", "31控件调用归属，不替代逐字段或运行通过。")
    add_evidence("EV-S00-CAND-MATRIX", "design_plan/控件文档与实施覆盖矩阵.md", "D/U/P/R/C/T/E起始边界与5类历史局部范围。")
    add_evidence("EV-S00-CAND-README", "README.md", "0.7.0软件受支持范围，非完整控件或E证明。")
    for eid, path, scope in REPORTS.values():
        add_evidence(eid, path, scope)
    add_evidence("EV-S00-CAND-FUNC-DEVICE", "docs/function-button-evidence/原厂设备补证.md", "真实Qt getter、原厂UI/源/编译及字符和隐藏图补证；不含现场动作通过。")
    add_evidence("EV-S00-CAND-FUNC-USB", "docs/function-button-evidence/usb-function-integration.json", "历史16控件传输/提交ACK，screenDisplayAndTouchConfirmed=false；不关闭E。")
    add_evidence("EV-S00-CAND-BIT-DEVICE", "docs/位按钮设备编译实现.md", "后续type28文本、地址及图库补证，覆盖旧文本报告的过时拒绝边界。")

    # Hash every currently used manual separately from the source SHA frozen inside each original row.
    manuals = {}
    for row in rows:
        module = row["模块"]
        if module in manuals:
            continue
        current = PROJECT / "design_plan" / Path(row["来源文件"].replace("\\", "/")).name
        eid = "EV-S00-CAND-MANUAL-" + MODULE_MAP[module]["slug"]
        add_evidence(eid, rel(current), "按冻结CSV来源定位的当前手册身份；不声称本轮已逐字段审定整份手册，旧来源SHA和原候选仍取CSV。")
        manuals[module] = eid

    lock_path = WORKSPACE / ".scratch/kinseal-stage0-20261110/candidates/id-lock.json"
    previous_path = OUT / "candidate-ledger.json"
    previous = json.loads(previous_path.read_text(encoding="utf-8")) if previous_path.exists() else {}
    lock = json.loads(lock_path.read_text(encoding="utf-8")) if lock_path.exists() else {
        q["stable_assignment_key"]: dict(id=q["id"], title=q["assignment_title"])
        for q in previous.get("questions", []) if "stable_assignment_key" in q
    }
    counter = max([int(item["id"].split("-")[3]) for item in lock.values()] or [0])
    questions = []
    by_id = {}
    candidates = []
    per_candidate = {}

    def allocate(key: str, slug: str, suffix: str, title: str) -> str:
        nonlocal counter
        if key in lock:
            assert lock[key]["title"] == title, "A stable question changed meaning; append a new key instead."
            return lock[key]["id"]
        counter += 1
        qid = f"Q-S00-CAND-{counter:03d}-{slug}-{suffix}"
        lock[key] = dict(id=qid, title=title)
        return qid

    def make_question(row: dict, key: str, profile: str, title: str, kind: str = "atomic", aliases=None) -> dict:
        n = int(row["线索ID"].split("-")[1])
        module = row["模块"]
        suffix, layers, pre, stimulus, expected = PROFILES[profile] if profile != "parent" else (
            "PARENT", [], "按各子问题分别建立前置。", "仅汇总子问题，不作为单个测试执行。", "所有声明范围子项都有证据才可关闭父项。")
        eid_list = ["EV-S00-CAND-CSV", manuals[module]]
        if module in REPORTS:
            eid_list.append(REPORTS[module][0])
        if module == "功能按钮":
            eid_list.extend(["EV-S00-CAND-FUNC-DEVICE", "EV-S00-CAND-FUNC-USB"])
        if n in (303, 306, 307):
            eid_list.append("EV-S00-CAND-BIT-DEVICE")
        qid = allocate(key, MODULE_MAP[module]["slug"], suffix, title)
        q = dict(id=qid, kind=kind, stable_assignment_key=key, assignment_title=title,
                 source_candidate_ids=["CAND-20261110/" + row["线索ID"]],
                 source_module=module, control_id=MODULE_MAP[module]["control_id"],
                 responsible_module=OWNERS.get(n, (module, []))[0], related_control_ids=OWNERS.get(n, (module, []))[1],
                 question=title + ("" if title.endswith("？") else "？"), blocking_layers=list(layers),
                 status="未闭合", legacy_q_ids=aliases or [], parent_ids=[], children=[],
                 preconditions=pre, stimulus=stimulus + " 对象：" + title + "。",
                 expected_observation=expected, actual_observation_this_run=None,
                 existing_evidence_ids=eid_list, evidence_adoption=PARTIAL.get(n, "原候选保留部分V/H/B线索；本轮未新增原生运行或硬件结果。"),
                 next_probe=stimulus + " 先核当前证据身份，已有同范围证据直接复用；缺口才新增最小原生样本。",
                 new_execution_performed=False, closure_scope=None)
        q["blocking_dependencies"] = (["先取得固定AMT固件及可记录的屏端/寄存器观察", "对应配置及原生期望不足时仅阻塞本E问项"]
                                      if profile in ("e", "w", "t", "f") else
                                      ["先核本条来源身份；复用同版本同范围直接证据，输入失败不等于禁用或不存在"])
        q["scope_limit"] = "关闭仅限当前AMT/V5.1所核范围；其他型号与版本不得继承。"
        if (profile == "v" and n in (61, 74, 153, 165, 166, 196, 271, 289, 302)) or n == 124:
            q["scope_limit"] = "当前V5.1/AMT已观察的隐藏或缺席状态保留；新增其他机型/版本出现条件转能力矩阵扩展，不阻塞当前已核AMT切片。"
        if n in PARTIAL:
            q["status"] = "部分历史补证，余项未闭合"
        if kind == "parent":
            q["status"] = "父项开放，按子项汇总"
        if n in (22, 105, 124, 155, 172, 209, 251, 273):
            q["interpretation"] = "说明/归属/差异上下文；保留可判断问题，不等同待实现的新字段或要求复刻异常。"
        questions.append(q)
        by_id[qid] = q
        return q

    # First establish all ordinary candidates so exact duplicates can reference existing questions.
    for row in rows:
        n = int(row["线索ID"].split("-")[1])
        classification, items = specs[n]
        source = dict(row)
        candidate = dict(id="CAND-20261110/" + row["线索ID"], original_id=row["线索ID"],
                         frozen_csv_sha256=EXPECTED_SHA, source=source, classification=CLASS_NAMES[classification],
                         review_method="逐条语义初审；由代理显式编写review.tsv，不是人类签字或关键词分类。",
                         status="初审开放", question_ids=[], review_note="", formal_test_ids=[], tests_executed_this_run=0)
        if n == 308:
            candidate.update(status="说明排除，保留原行", review_note="沿用旧核销：只是避免将帮助旧图当运行算法的说明，无独立行为；不建立Q，不从原始308删除。")
        elif n in (79, 85, 181):
            candidate["review_note"] = "按明确的父子或精确重复关系在下一步接入，原U-ID不改。"
        else:
            parent = None
            if len(items) > 1:
                parent = make_question(row, f"{n}:parent", "parent", row["模块"] + "候选" + row["线索ID"] + "的分层问题", "parent")
            qids = []
            for index, (profile, title) in enumerate(items, 1):
                aliases = ["Q-FUNC-MACRO-001"] if n == 71 else ["Q-LAMP-MACRO-001"] if n == 178 else []
                q = make_question(row, f"{n}:q{index}", profile, title, aliases=aliases)
                if parent:
                    q["parent_ids"].append(parent["id"])
                    parent["children"].append(q["id"])
                if n in (71, 178):
                    q.update(status="沿用事实关闭", closure_scope={
                        "basis": "旧核销原有关闭，不是本轮重新测试",
                        "version": "V5.1，同哈希私有运行副本的所核AMT范围", "layers": ["D", "U"],
                        "conclusion": "宏组真实Qt enabled=false；使用宏/写宏没有可用交互路径。",
                        "not_covered": "其他控件宏、其它机型、整控件通过及E。"})
                    q["existing_evidence_ids"].append("EV-S00-CAND-PRIOR")
                    candidate["status"] = "沿用事实关闭"
                    candidate["review_note"] = "仅沿用旧核销真实Qt宏禁用事实及其D/U范围，不代表新测试或整控件关闭。"
                qids.append(q["id"])
            candidate["question_ids"] = ([parent["id"]] if parent else []) + qids
            if n in PARTIAL:
                candidate["status"] = "已有历史补证，整体未关闭"
                candidate["review_note"] = PARTIAL[n]
            elif classification == "N":
                candidate["status"] = "说明上下文保留，关联问题开放"
                candidate["review_note"] = "不增加未观察到的可用控件；仅就来源适用性、证据稳定性或产品差异保留Q。"
            elif classification not in ("E",):
                candidate["review_note"] = "按原文可独立判断的观察结果拆问；尚无本轮新执行结果。"
        candidates.append(candidate)
        per_candidate[n] = candidate

    # U085 explicitly names the two untested gate paths also captured by U091 and U168.
    gate_ids = [per_candidate[n]["question_ids"][0] for n in (91, 168)]
    per_candidate[85].update(status="重复映射，目标问题仍开放", question_ids=gate_ids,
        duplicate_of_candidates=["CAND-20261110/U-091", "CAND-20261110/U-168"],
        review_note="原文把已实测动态条门槛与未复测报警/历史报警门槛并列；未复测的两个入口分别等同U168/U091的无配置刺激和拦截观察。不以动态条结果替代这两个入口。")
    for qid in gate_ids:
        by_id[qid]["source_candidate_ids"].append("CAND-20261110/U-085")

    # U181 retains the already-closed count fact and a separate open rendering branch.
    row = rows[180]
    p181 = make_question(row, "181:parent", "parent", "指示灯状态范围与超过两态的显示问题", "parent")
    qcount = make_question(row, "181:count", "closed", specs[181][1][0][1], aliases=["Q-LAMP-COUNT-001"])
    qcount.update(status="沿用事实关闭", closure_scope={"basis": "旧核销已有事实关闭", "version": "V5.1所核AMT",
        "layers": ["D", "U"], "conclusion": "真实进入多态后2..32；外观索引0..N-1。setChecked残留256及步进到27均非上限依据。",
        "not_covered": "多态屏端图像、闪烁、语言和E。"})
    qcount["existing_evidence_ids"].append("EV-S00-CAND-PRIOR")
    render_parent = make_question(row, "181:render-parent", "parent", "指示灯大于两态的外观及屏端显示分支", "parent", ["Q-LAMP-RENDER-001"])
    for index, (profile, title) in enumerate(specs[181][1][1:], 1):
        q = make_question(row, f"181:render-{index}", profile, title)
        q["parent_ids"].append(render_parent["id"])
        render_parent["children"].append(q["id"])
    p181["children"] = [qcount["id"], render_parent["id"]]
    qcount["parent_ids"].append(p181["id"])
    render_parent["parent_ids"].append(p181["id"])
    per_candidate[181].update(status="部分关闭：范围事实已闭，渲染分支开放", question_ids=[p181["id"], qcount["id"], render_parent["id"]] + render_parent["children"],
        review_note=PARTIAL[181])

    # U079 is deliberately not a single test. These labels are explicit native function/type orders.
    functions = ["背景灯开关", "系统设置", "系统重启", "设置屏保时间", "触摸校正", "设置系统时间", "面板保护解锁", "更改用户等级", "注销用户等级", "触摸声音开关", "历史数据操作", "配方操作", "报警操作", "语言切换", "键盘组件", "无线设置", "背光调节", "下载方式"]
    subtypes = {
        10: ["清除全部历史数据", "历史数据上移", "历史数据下移", "历史数据上翻页", "历史数据下翻页", "U盘历史数据导出", "SD卡历史数据导出"],
        11: ["当前配方写入PLC", "从PLC读当前配方", "前一个配方", "后一个配方", "当前配方保存", "配方读取及保存", "重载配方", "当前配方删除", "U盘配方导出", "U盘配方导入", "配方添加", "SD卡配方导出", "SD卡配方导入"],
        12: ["确认报警", "清除报警", "报警声音开关", "清除历史报警数据", "报警控件报警上移", "报警控件报警下移", "报警控件报警上翻页", "报警控件报警下翻页", "历史报警控件报警上移", "历史报警控件报警下移", "历史报警控件报警上翻页", "历史报警控件报警下翻页", "U盘历史报警导出", "SD卡历史报警导出"],
        14: ["Enter", "BackSpace", "Clear", "ESC", "Delete", "Caps", "Left", "Right", "Space", "ASCII/BMP字符"],
    }
    stimuli = {
        0: "记录初始背光，触碰该按钮后再触碰屏幕空白区，记录亮度和恢复。",
        1: "触碰按钮，记录系统设置弹窗、可返回路径及原画面状态。",
        2: "在已保存且可恢复的独立工程中触碰重启，记录停机、启动画面及保留值；先取得原生期望。",
        3: "触碰后输入两种可辨识屏保时长，观察确认、取消和实际进入屏保时间。",
        4: "记录按钮触发的校正界面及退出路径；是否执行校正作为后续独立现场授权范围。",
        5: "触碰后输入可辨识日期时间，分别确认与取消并记录系统时钟变化。",
        6: "配置保护锁定，在锁定前后触碰，分别输入错误及正确密码并观察保护状态。",
        7: "配置已知等级及受限对象，执行更改等级，记录登录键盘、结果等级及权限变化。",
        8: "先登录非默认等级再注销，记录实际返回等级及受限对象可用性；不得预设注销后回哪个等级。",
        9: "先记录触摸声音，交替触碰此按钮和普通按钮，记录声音开关状态。",
        13: "给中英文状态设置明显不同文本，逐目标语言触碰，记录当前语言及所有相关对象文本。",
        15: "记录原生是否无动作、提示或打开无线界面；帮助暂不支持只作H假设，未观测前不写可用结论。",
        16: "先记录亮度，触碰后观察是否有调节入口及亮度变化，保留两篇帮助的冲突。",
    }
    row = rows[78]
    p79 = make_question(row, "79:parent", "parent", "18功能及全部子类型的设备动作与前置配置", "parent", ["Q-FUNC-RUN-001"])
    all79 = [p79["id"]]
    for function, name in enumerate(functions):
        group = make_question(row, f"79:function-{function}", "parent", f"功能{function} {name}的运行观察", "parent")
        group["function_index"] = function
        p79["children"].append(group["id"])
        group["parent_ids"].append(p79["id"])
        all79.append(group["id"])
        if function == 17:
            # Exact same two function17 actions; reuse U078, rather than inventing two extra tests.
            leaves = [qid for qid in per_candidate[78]["question_ids"] if by_id[qid]["kind"] == "atomic"]
            group["children"] = leaves
            for type_index, qid in enumerate(leaves):
                by_id[qid]["parent_ids"].append(group["id"])
                by_id[qid]["source_candidate_ids"].append("CAND-20261110/U-079")
                by_id[qid]["function_index"] = 17
                by_id[qid]["type_index"] = type_index
            all79.extend(leaves)
            continue
        actions = subtypes.get(function, [name])
        for subtype, action in enumerate(actions):
            title = f"功能{function} {name}" + (f"/类型{subtype} {action}" if function in subtypes else "") + "的实际设备动作和状态变化"
            q = make_question(row, f"79:function-{function}-type-{subtype}", "e", title)
            q["parent_ids"].append(group["id"])
            q["function_index"] = function
            q["type_index"] = subtype if function in (10, 11, 12) else None
            q["keyboard_key_index"] = subtype if function == 14 else None
            if function in (10, 11, 12):
                subsystem = {10: "数据采集与历史数据显示", 11: "配方及PLC可观测区", 12: "报警登录、活动及历史报警显示"}[function]
                q["preconditions"] += " 另需完整" + subsystem + "，至少两条可区分记录及明确选中项；介质动作需空白测试U盘/SD卡。"
                q["stimulus"] = "在上述可观测数据中只触碰一次“" + action + "”，记录当前索引、记录内容、PLC区或导出文件前后差异及取消/失败结果。"
                q["blocking_dependencies"] = ["阶段2对应配置尚未完整恢复时，仅阻塞此功能的现场切片", "阶段10固定固件及现场记录"]
            elif function == 14:
                q["preconditions"] += " 另需原生数值或ASCII输入框及可观察的键盘焦点、文本和光标位置。"
                q["stimulus"] = "在文本Ab12和中间光标处触发“" + action + "”；记录输入框、光标、大小写、提交或取消。字符键分别测A和本地可编码BMP字符，不宣称完整Unicode。"
            else:
                q["stimulus"] = stimuli[function]
            q["evidence_adoption"] = "功能代码及相应类型的P/C历史证据已取得；USB回执E=false，尚无本条动作的现场实际观察。"
            q["next_probe"] = "先在已固定原生目标获取期望，再对同哈希Lite样例作等价激励：" + q["stimulus"]
            group["children"].append(q["id"])
            all79.append(q["id"])
    per_candidate[79].update(status="复合父项开放，18功能分别接续", question_ids=all79,
        review_note="保留Q-FUNC-RUN-001父项；按18功能再拆历史7、配方13、报警14和键盘10动作。下载方式2动作与U078精确复用；不把一次USB或一个原子测试覆盖整父项。")

    # Cross references are evidence of overlap, not automatic merging across wrappers or common containers.
    related = {7: [193], 16: [20], 85: [91, 168], 105: [209, 300], 137: [267, 276],
               155: [181], 176: [263], 177: [55], 209: [105, 300], 240: [33], 300: [105, 209],
               59: [80, 151, 164, 269, 304], 80: [59, 164, 304]}
    for n, neighbors in related.items():
        per_candidate[n]["related_candidate_ids"] = [f"CAND-20261110/U-{value:03d}" for value in neighbors]
        per_candidate[n]["non_merge_reason"] = "相同名词、共用窗口或相似异常不证明调用点及运行结果相同；除明确登记的U085和功能17动作外保持独立。"
    for candidate in candidates:
        qids = candidate["question_ids"]
        candidate["blocking_layers"] = sorted({layer for qid in qids for layer in by_id[qid]["blocking_layers"]})
        candidate["atomic_question_ids"] = list(dict.fromkeys(qid for qid in qids if by_id[qid]["kind"] == "atomic"))
        candidate["current_evidence_refs"] = [manuals[candidate["source"]["模块"]]]
        assert candidate["original_id"] == "U-308" or qids, "A real candidate must retain a concrete destination."
    for q in questions:
        if q["kind"] == "parent":
            q["blocking_layers"] = sorted({layer for child in q["children"] for layer in by_id[child]["blocking_layers"]})
        q["source_candidate_ids"] = list(dict.fromkeys(q["source_candidate_ids"]))
        q["parent_ids"] = list(dict.fromkeys(q["parent_ids"]))

    # Parents were appended out of order; recompute transitive layer union without changing stable IDs.
    for _ in range(3):
        for q in questions:
            if q["kind"] == "parent":
                q["blocking_layers"] = sorted({layer for child in q["children"] for layer in by_id[child]["blocking_layers"]})
    for candidate in candidates:
        candidate["blocking_layers"] = sorted({layer for qid in candidate["question_ids"] for layer in by_id[qid]["blocking_layers"]})

    atomic = [q for q in questions if q["kind"] == "atomic"]
    parents = [q for q in questions if q["kind"] == "parent"]
    closed = [q for q in atomic if q["status"] == "沿用事实关闭"]
    assert len(closed) == 3
    assert len({q["id"] for q in questions}) == len(questions)
    assert {q["id"] for q in previous.get("questions", [])}.issubset(by_id), "Do not silently remove a stable Q-ID."
    assert all(child in by_id for q in questions for child in q["children"])
    assert digest(CSV_PATH.read_bytes()) == EXPECTED_SHA
    stats = dict(original_candidate_count=308, semantic_reviewed_candidate_count=len(candidates),
                 candidate_classification_counts=dict(Counter(c["classification"] for c in candidates)),
                 candidate_status_counts=dict(Counter(c["status"] for c in candidates)),
                 candidate_fact_closed_count=2, candidate_partially_closed_count=1,
                 excluded_non_problem_candidate_count=1, duplicate_mapped_candidate_count=1,
                 formal_question_record_count=len(questions), concrete_atomic_question_count=len(atomic),
                 parent_question_count=len(parents), atomic_fact_closed_count=len(closed),
                 atomic_not_closed_count=len(atomic)-len(closed), tests_executed_this_run=0,
                 new_fact_closures_this_run=0, function_runtime_function_group_count=18,
                 function_runtime_atomic_count=len({qid for qid in all79 if by_id[qid]["kind"] == "atomic"}))
    ledger = dict(schema_version=1, task="S00-04", review_date="2026-11-10", modification_marker="#20261110-4",
                  snapshot_id="CAND-20261110", frozen_source=dict(path=rel(CSV_PATH), sha256=EXPECTED_SHA, row_count=308,
                  identity_rule="保留原行序、U-ID、来源行及旧来源SHA；当前手册SHA另存，不覆盖冻结身份。"),
                  author="Codex /root/function_ui，逐条语义初审；不代表人类审批",
                  method="显式review.tsv逐候选拆问；脚本仅生成、稳定ID分配和一致性检查，不从关键词判断类别或闭合。",
                  review_source={"explicit_decisions": {"path": "docs/stage0-evidence/candidate-review-source/review.tsv",
                                                        "sha256": digest((HERE / "review.tsv").read_bytes()), "row_count": 308},
                                 "generator": {"path": "docs/stage0-evidence/candidate-review-source/build_candidate_ledger.py",
                                               "sha256": digest(Path(__file__).read_bytes())},
                                 "stable_id_authority": "本JSON questions中的stable_assignment_key/assignment_title/id；scratch id-lock只是可再生镜像。"},
                  field_semantics={"control_id": "原CSV来源模块对应的A04控件ID；实际责任归属见responsible_module及related_control_ids。",
                                   "status": "本次问题初审/历史采用状态，不是测试状态或软件阶段通过。",
                                   "stable_assignment_key": "正式JSON保留分配键与原题，scratch锁丢失时仍可恢复原Q-ID；键不是测试编号。"},
                  scope="AMT070W124-L800X480R/800×480/V5.1；LT779及其他型号不继承当前关闭事实。",
                  evidence_boundaries=["本轮只读调查和文档生成，无原生运行、构建、USB或PLC写入。", "3个关闭事实沿用旧核销；其余补证按有限范围标注，不以报告存在关闭整体。", "P区分ksp与sm，R区分PC/解压像素/实屏，T与E分开；未知固件保持未知。", "Q-ID是问题ID，U-ID是冻结候选ID，本文件不分配或报告实际测试编号。"],
                  statistics=stats, evidence_catalog=list(evidence.values()), candidates=candidates, questions=questions,
                  integrity_checks=dict(frozen_csv_sha_unchanged=True, original_ids_exactly_preserved=True,
                  every_real_candidate_has_question_destination=True, no_missing_question_reference=True,
                  existing_three_fact_closures_preserved=True, u181_render_branch_open=True, u079_parent_open=True,
                  u308_excluded_without_deleting_source=True, all_new_execution_results_null=True))

    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "candidate-ledger.json").write_text(json.dumps(ledger, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    lock_path.write_text(json.dumps(lock, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    lines = ["# S00-04 冻结候选逐条语义初审台账", "", "日期：2026-11-10。由代理逐条读取并显式拆问；不代表人类签字或本轮新测试通过。完整机器可读内容见[candidate-ledger.json](candidate-ledger.json)。", "",
             "原始快照 `CAND-20261110` 保持308行，CSV SHA256 `" + EXPECTED_SHA + "`。本文件不修改原CSV、旧核销、程序日志或阶段状态；旧U号只作候选身份，正式问题使用独立分配并锁定的Q-S00-CAND编号。", "",
             "## 数量与方法", "", "| 口径 | 数量 |", "|---|---:|",
             f"| 原始候选 / 已逐条初审候选 | 308 / {len(candidates)} |",
             f"| 正式Q记录（包含父项） | {len(questions)} |", f"| 具体原子问题 | {len(atomic)} |",
             f"| 复合父项Q | {len(parents)} |", "| 沿用关闭事实 / 本轮新关闭 | 3 / 0 |",
             f"| 尚未闭合的原子问题 | {len(atomic)-3} |", "| 本轮实际执行测试 | 0 |", "",
             "候选分类：" + "；".join(f"{key}{value}" for key, value in stats["candidate_classification_counts"].items()) + "。候选状态与问题状态分别统计，不据此计算软件完成率。", "",
             "显式审阅表与生成器归档于[candidate-review-source/review.tsv](candidate-review-source/review.tsv)及[生成器](candidate-review-source/build_candidate_ledger.py)，工作副本在 `.scratch/kinseal-stage0-20261110/candidates/`。每一行问题标题和分类均显式编写，探针模板只规范记录前置、刺激和观察。生成器校验308个原ID/原SHA、引用完整性及3个旧事实关闭；不运行历史覆盖CSV的survey脚本。正式JSON保存stable_assignment_key与原题，scratch锁丢失仍可恢复Q号；禁止静默移除既有Q。现存手册与5类实施报告另存当前SHA，冻结CSV旧来源SHA始终保留。", "",
             "## 关键边界与复用", "",
             "- U071→Q-FUNC-MACRO-001、U178→Q-LAMP-MACRO-001的宏禁用事实沿用；U181→Q-LAMP-COUNT-001的2..32和0..N−1事实沿用。这是3个事实、仅2个候选整体事实关闭，U181还有渲染分支。",
             "- U181继续保留Q-LAMP-RENDER-001及PC外观、五态显示、闪烁、语言子项；既有源/图帧解释不重新当成未知，但不能替代E。",
             f"- U079继续保留Q-FUNC-RUN-001开放父项，按18功能分组形成{stats['function_runtime_atomic_count']}个具体动作问项；历史7、配方13、报警14和键盘10分别拆出。功能17的两动作与U078精确复用，不重复当测试。",
             "- U085原文中的两个未测报警门槛，分别映射U091历史报警和U168报警显示的同一前置/刺激/观察。没有拿动态报警条的已知门槛关闭另外两个入口。",
             "- U308仅排除为说明句，无Q且仍保留原始行和308计数。其他说明/归属/异常上下文保留可判断问题，不要求实现旧界面字段或复刻崩溃。",
             "- 未按宏/权限/通知等同名关键词跨控件合并；调用点或设备结果相同缺证时保持独立，并保留相关候选链接。", "",
             "## 逐候选去向", "", "以下308行保持冻结CSV顺序。每个Q的前置、刺激、预期观察、现有证据和下一探针在后面的逐问题表及JSON中。", "",
             "| 原候选 | 来源模块 / 实际归属 | 分类 / 候选状态 | 正式问题去向 |", "|---|---|---|---|"]
    for candidate in candidates:
        module = candidate["source"]["模块"]
        owner = OWNERS.get(int(candidate["original_id"][2:]), (module, []))[0]
        qlinks = "、".join(f"[{qid}](#{qid.lower()})" for qid in candidate["question_ids"]) or "说明排除；不建Q"
        lines.append(f"| {candidate['original_id']} | {module} / {owner} | {candidate['classification']} / {candidate['status']} | {qlinks} |")
    lines.extend(["", "## 逐问题记录", "", "本节的刺激均是下一探针设计，实际观察字段全部为空。已有报告的范围声明与其引用来源保留，未在本轮重新执行。"])
    for q in questions:
        lines.extend(["", f"<a id=\"{q['id'].lower()}\"></a>", "", "### " + q["id"], "", "- 问题：" + q["question"],
                      "- 来源/归属：" + "、".join(q["source_candidate_ids"]) + "；" + q["responsible_module"] + "；" + q["control_id"],
                      "- 层/状态：" + "/".join(q["blocking_layers"]) + "；" + q["status"] + "；" + q["kind"],
                      "- 适用范围：" + q["scope_limit"], "- 依赖：" + "；".join(q["blocking_dependencies"]),
                      "- 前置：" + q["preconditions"], "- 刺激：" + q["stimulus"], "- 预期观察：" + q["expected_observation"],
                      "- 现有证据：" + "、".join(q["existing_evidence_ids"]) + "；" + q["evidence_adoption"],
                      "- 下一探针：" + q["next_probe"]])
        if q["legacy_q_ids"]:
            lines.append("- 沿用问题身份：" + "、".join(q["legacy_q_ids"]))
        if q["parent_ids"]:
            lines.append("- 父项：" + "、".join(q["parent_ids"]))
        if q["children"]:
            lines.append("- 子项：" + "、".join(q["children"]))
        if q["closure_scope"]:
            lines.append("- 关闭范围：" + json.dumps(q["closure_scope"], ensure_ascii=False))
    lines.extend(["", "## 证据身份", "", "| 证据ID | 路径 | SHA256 | 可支持范围 |", "|---|---|---|---|"])
    for e in evidence.values():
        lines.append(f"| {e['id']} | `{e['path']}` | `{e['sha256']}` | {e['supports']} |")
    lines.extend(["", "恢复点：从具体Q选择下一最小探针；复用当前身份一致的已有证据，缺前置只阻塞相关层。后续补证在同Q追加采用范围或重开原因，新增含义另给Q，不重排原U或重用测试数。", ""])
    (OUT / "candidate-ledger.md").write_text("\n".join(lines), encoding="utf-8")
    print(json.dumps(stats, ensure_ascii=False, indent=2))
    print("frozen CSV SHA unchanged:", digest(CSV_PATH.read_bytes()))


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    main()
