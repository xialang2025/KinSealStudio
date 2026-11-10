"""#20261110-4: Write the bounded Chinese field inventory report from audited traceability records."""
from pathlib import Path
import collections
import hashlib
import json
import os
import re
import sys

sys.stdout.reconfigure(encoding="utf-8")
ROOT = next(parent for parent in Path(__file__).resolve().parents if (parent / "KinSealStudio/design_plan/A04_31控件实施索引.md").is_file())
OUT = ROOT / "KinSealStudio/docs/stage0-evidence"
OWN = Path(__file__).resolve().parent
data = json.loads((OUT / "field-traceability.json").read_text(encoding="utf-8"))
structure_audit = json.loads((OWN / "audit-result.json").read_text(encoding="utf-8"))
current_sha = hashlib.sha256((OUT / "field-traceability.json").read_bytes()).hexdigest()
if structure_audit.get("errors") or structure_audit.get("document_sha256") != current_sha:
    raise SystemExit("请先对当前JSON运行audit_traceability.py；结构错误或旧审计不能生成通过措辞。")
sources = data["source_registry"]
fields = {field["id"]: field for field in data["fields"]}
questions = {item["id"]: item for item in data["questions"]}
sections = {section["id"]: section for control in data["controls"] for section in control["sections"]}


def citation(reference):
    """Resolve a normalized source reference to a readable relative file link and exact lines."""
    source = sources[reference["source_id"]]
    rel = "../../" + source["path"].removeprefix("KinSealStudio/")
    label = Path(source["path"]).name + f":{reference['start']}"
    return f"[{label}]({rel})（L{reference['start']}–{reference['end']}；源SHA见JSON `{reference['source_id']}`）"


def original(reference):
    """Read only the exact referenced line span already bound to a source SHA."""
    source = sources[reference["source_id"]]
    return "\n".join((ROOT / source["path"]).read_text(encoding="utf-8-sig").splitlines()[reference["start"] - 1:reference["end"]])


def display_quote(raw, reference):  #20261110-4 add
    """Preserve quoted labels while resolving source-relative links from the report directory."""
    origin = (ROOT / sources[reference["source_id"]]["path"]).parent

    def replace_link(match):
        target = match[2]
        if target.startswith(("http:", "https:", "#", "mailto:")):
            return match[0]
        relative, separator, anchor = target.partition("#")
        rebased = os.path.relpath(origin / relative, OUT).replace("\\", "/")
        return match[1] + rebased + (separator + anchor if separator else "") + match[3]

    return re.sub(r"(!?\[[^\]]*\]\()([^)]+)(\))", replace_link, raw)


audit = data["audit"]
lines = ["# S00-03｜31控件字段、事件与验收追踪", "", "日期：2026-11-10。修改标记：`#20261110-4`。本切片只写本报告、同目录JSON及自用提取/审计脚本，没有修改程序、现有手册、协议、总日志或阶段状态。", "",
         "本次完成的是 **31份手册完整结构接入与实际原文记录基线**，不是全部原子字段规格完成，也不宣告D/U/P/R/C/T/E通过。未确证类型、默认、范围、依赖和保存寿命继续挂Q-ID；其余26类的源/设备字段保持未知。主代理本轮硬件验收单独记录，见[USB协议第23节](../AMT070W124-L800X480R_USB下载协议_已验证部分.md)；本报告的未运行程序/硬件仅指本字段分工。", "",
         "## 1. 记录数量与解释", "", "| 项目 | 本次结果 | 边界 |", "|---|---:|---|",
         f"| 手册 | {audit['manual_count']}份 / {audit['manual_lines']}行 | 完整读取，不只读A04摘要 |",
         f"| 标题、Markdown表 | {audit['headings']} / {audit['tables']} | 章节和非字段表都保留类型，不算字段 |",
         f"| 真实表数据行 | {audit['table_data_rows']} | 包含入口、身份、枚举等非字段行 |",
         f"| 字段表行记录 | {audit['record_kinds']['handbook_field_table_row']} | 实际原手册表行；尚不等于原子字段数 |",
         f"| 条件/差异枚举行 | {audit['record_kinds']['enum_condition_or_difference_row']} | 条件变体与选项，不能当独立可编辑字段 |",
         f"| 正文/树字段候选 | {audit['record_kinds']['prose_tree_candidate_not_atomic_field']} | 保留隐藏、禁用、空页、联动等原句；可能仍含描述 |",
         f"| F追踪记录合计 | {audit['field_records']} | 上三项合计；不是规格通过数 |",
         f"| 需继续原子化 | {audit['decomposition_required']} | 复合表行、正文/树候选未细分完 |",
         f"| EV事件陈述候选 | {audit['events']} | 实际原句/表行；复合刺激未全部拆分，未重跑 |",
         f"| AC设计/手册验收原文 | {audit['acceptances']} | 全部待执行；不借用历史测试计数 |",
         f"| Q追踪占位 | {audit['questions']} | 每F一个含缺口集合的追踪Q；不是独立问题总量，也不是原308候选数 |",
         f"| 已定位实现映射的记录 | {audit['mapped_records']} | 仅五已有类，含明确标识的复合映射；不是原厂D通过 |",
         f"| 未链接同原文行EV的记录 | {audit['field_records_without_same_line_event']} | 没有编造启用事件，用Q继续补证 |", "",
         "完整数据为 [field-traceability.json](field-traceability.json)。为避免重复大段原文，来源、表头、模型映射、缺口和验收探针各只保存一份，记录按ID引用；手册原文就地保留，以文件SHA和一基行号读取。JSON压缩书写，Markdown负责导航。", "",
         "## 2. 来源、等级和身份", "",
         f"已完整读取总入口、阶段0、A01/A04/A05、31份手册、五类实施报告和位按钮设备编译后续台账。`source_registry`登记{len(sources)}个本次实际引用文件（含模型声明和消费者定位）的路径、大小、SHA-256和行数；以实际JSON为准。", "",
         "V/H/B/I分栏表示手册中的历史证据声明，不表示本轮重新观察：V为历史运行，H为帮助，B为字符串/对象名/反编译静态资料，I为推断。每条字段的`cell_evidence`保留原单元格和表头标记；例如功能类型表中H动作语义不能因为同一行有V默认就升级V。未标级的原句仍未标级。实现代码和实施报告另列，不充当原厂行为本身。", "",
         f"旧报告和后续补证冲突按具体范围处理，保留原行，不按日期把整个旧文档覆盖。已登记{len(data['known_implementation_deltas'])}条`known_implementation_deltas`，包括：", "",
         "- 静态文本旧单语/无显现下载拒绝与后续type28多语言/显现编译分层；指定实屏样例不扩展成所有字体或权限公式通过。",
         "- 位按钮KSP两态草稿与单态SM保存寿命不同；互斥通知及u16毫秒保护不外推原厂UI上限。",
         "- 字按钮七类型/七操作、默认I16/LW0/常数1；空指示灯页和常态/按下态独立。",
         "- 指示灯宏/通知/与操作地址相同禁用、2..32状态、禁用显示15与位监视存储0、显式5→2→5删草稿及全局同文。",
         "- 功能按钮宏禁用、字符框始终可编辑且1 UTF16单位、设备本地首字节、字体1..128、按压0..255、无图颜色启用、状态0锁定/隐藏ON图和`iiqbbiq`源字段。", "",
         "上述是引用已有补证的关联，不在本切片关闭原U候选，也不把历史构建、USB或屏端结果记成本轮执行。", "",
         "## 3. 编号、结构与使用方法", "",
         "C01..C31沿A04。F/EV/AC及`Q-S00F`由控件、章节路径和原标签/原句的规范化SHA前12位生成，不含行号、扫描序号或文件SHA。插入空行不会重排ID；正文整句候选发生语义改写或后续拆分时需建立新ID和显式去向，不能声称已具有最终原子字段身份。", "",
         "一个字段沿`fields[id] → source.source_id/start/end → source_registry`定位原文。`raw_cells`是该记录的实际原文字段/正文，`header_id`解码列义；`default_range_claim_columns`、`dependency_claims_columns`、`lifecycle_claims_columns`指向原文列，不猜缺少的默认值。位置用SEC-ID指向控件的完整章节树。", "",
         "`mapping_id`指向模型/.ksp/.sm/设备消费者；未知为null并有理由。代码引用只证明对应实现属性或模块存在；`device_module_reference`是消费者模块锚点，不冒充该表行每个子字段的精确设备偏移。复合映射需按其Q继续拆分。", "",
         "每个F都连AC设计和Q缺口；确有原文事件时连EV。原文没有单独刺激时EV为空且报告缺口，不用通用模板捏造原厂事件。共用验收/探针只在`procedure_registry`存一次，期望必须从该F原文和已引用补证提炼；未知期望先补Q。", "",
         "## 4. 全31控件接入", "",
         "表中F记录是字段表行、条件行及正文候选的合计。入口列定位真实原手册的调用陈述；全部窗口/页签、表格、正文和差异索引见对应控件JSON，未展开的公共权限引用继续是Q，未复制一套假定相同的字段。", "",
         "| 控件 | F记录 | EV候选 | 窗口/章节 | 入口原文位置 |", "|---|---:|---:|---:|---|"]
for control in data["controls"]:
    entry = control["entry_examples"][0]
    source = sources[entry["source"]["source_id"]]
    rel = "../../" + source["path"].removeprefix("KinSealStudio/")
    lines.append(f"| {control['id']} {control['name']} | {len(control['field_ids'])} | {len(control['event_ids'])} | {len(control['sections'])} | [L{entry['source']['start']}]({rel}) |")
lines += ["", "## 5. 各控件差异、具体F与恢复点", ""]
for control in data["controls"]:
    source = sources[control["manual_source"]]
    lines += [f"### {control['id']}｜{control['name']}", "", f"手册SHA-256：`{source['sha256']}`。完整读取{source['line_count']}行，登记{len(control['tables'])}表；A04批次：{control['batch']}。", ""]
    for difference in control["a04_specific_differences"]:
        lines.append(original(difference).removeprefix("- ") + " 来源：" + citation(difference))
    entry = control["entry_examples"][0]
    lines += ["", "调用入口原文（相对链接按本报告位置重定基址，原始内容见JSON及所引行）：", "", "> " + display_quote(entry["raw"], entry["source"]).replace("\n", "\n> "), "", citation(entry["source"]), ""]
    picked = [fields[field_id] for field_id in control["field_ids"] if fields[field_id]["record_kind"] == "handbook_field_table_row"][:3]
    if not picked:
        picked = [fields[field_id] for field_id in control["field_ids"]][:3]
    lines.append("定位示例（全部记录在JSON，不把示例当本控件全部字段）：")
    lines.append("")
    for field in picked:
        label = field["raw_cells"][0].replace("\n", " ")
        lines.append(f"- `{field['id']}`：{label}；位置`{field['location']}`；`{field['question_ids'][0]}`。来源：" + citation(field["source"]))
    lines += [""]
lines += ["## 6. 审计方法与未闭合边界", "",
          f"本字段分工执行的结构/引用审计覆盖：{audit['manual_count']}控件集合、完整文件读取/行数、{audit['tables']}表和{audit['table_data_rows']}原文数据行、全部F/EV/AC/Q唯一性、跨记录链接、SourceSpan行界限、{len(sources)}个源文件当前SHA与行数、每控件实际调用陈述、每表真实Markdown行。永久配方和审计结果位于[field-review-source](field-review-source/README.md)，本分工结构审计无错误。", "",
          "本切片仍有以下明确边界：", "",
          f"1. {audit['record_kinds']['handbook_field_table_row']}个字段表行仍含复合字段；{audit['record_kinds']['enum_condition_or_difference_row']}个条件/差异行和{audit['record_kinds']['prose_tree_candidate_not_atomic_field']}个正文/树候选需人工细分。{audit['decomposition_required']}条记录显式标记`decomposition_required`，最终原子字段数尚未确定。",
          f"2. {audit['field_records_without_same_line_event']}条F没有同原文行的独立EV，部分刺激在别章或公共引用中；尚未按全部启用/禁用/隐藏组合建立事件图。不能把{audit['events']}条原句候选当{audit['events']}个独立已验证事件。",
          f"3. {audit['mapped_records']}条映射是五类已实现代码定位，其中也有复合行；其余记录没有假设模型、SM或设备type。五类所有子字段精确源跨度、禁用休眠值和原厂寿命仍需按记录继续闭合。",
          "4. 仅在截图里的字段、手册引用其他手册但未逐项重列的公共权限、多个显示形态的动态隐藏字段，本轮没有打开全部原厂窗口穷举；其正文/章节引用保留在JSON并挂Q。",
          f"5. {audit['questions']}个Q是字段追踪占位，不等于{audit['questions']}项独立未决事实；应由S00-04按具体问题归并、拆分、连接旧U候选。A04的原候选控件归属原样接入，不改变原308计数。",
          "6. 原厂运行、帮助、静态、推断、实现源码、历史测试和本轮结构审计分别保留；没有依据‘已写手册’或‘引用存在’更新阶段状态或宣布D通过。",
          "", "后续恢复：先选择一个C控件的`decomposition_required`字段表行，沿F→Q→原文行和实施差异读取，取得单一字段、刺激、默认/边界与保存寿命后补其模型/源/设备映射和分层AC。对原手册公共引用先实查目标窗口，不复制共同模板。"]
(OUT / "field-traceability.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
print("wrote", OUT / "field-traceability.md")
