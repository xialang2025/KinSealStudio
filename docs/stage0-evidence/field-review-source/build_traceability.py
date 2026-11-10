"""#20261110-4: Extract cited handbook records; never infer native support from a shared template."""
from pathlib import Path
import collections
import hashlib
import json
import re
import sys

sys.stdout.reconfigure(encoding="utf-8")
ROOT = next(parent for parent in Path(__file__).resolve().parents if (parent / "KinSealStudio/design_plan/A04_31控件实施索引.md").is_file())
DESIGN = ROOT / "KinSealStudio/design_plan"
OWN = Path(__file__).resolve().parent
OUT = ROOT / "KinSealStudio/docs/stage0-evidence"
SOURCES = {}
QUESTIONS = {}
MODEL_REFERENCES = {}


def digest(data):  #20261110-4 add
    """Hash exact source bytes or normalized identifier material."""
    return hashlib.sha256(data).hexdigest()


def sid(prefix, control, anchor):  #20261110-4 add
    """Keep identifiers independent of source line shifts and discovery order."""
    normalized = re.sub(r"\s+", " ", anchor).strip()
    return f"{prefix}-{control}-{digest(normalized.encode('utf-8'))[:12].upper()}"


def register(path, kind):  #20261110-4 add
    """Read each input completely once and retain its precise identity and lines."""
    path = path.resolve()
    rel = path.relative_to(ROOT).as_posix()
    if rel not in SOURCES:
        data = path.read_bytes()
        lines = data.decode("utf-8-sig").splitlines()
        SOURCES[rel] = dict(path=rel, sha256=digest(data), bytes=len(data), line_count=len(lines), kind=kind, lines=lines)
    return SOURCES[rel]


def ref(source, first, last=None):  #20261110-4 add
    """Cite literal complete source lines using one-based positions."""
    last = first if last is None else last
    return dict(path=source["path"], sha256=source["sha256"], line_start=first, line_end=last,
                quote="\n".join(source["lines"][first - 1:last]))


def evidence(raw, heading="", header=""):  #20261110-4 add
    """Separate literal V/H/B/I markers without promoting documentation to a fresh runtime observation."""
    groups = {key: [] for key in ("V", "H", "B", "I")}
    for key in groups:
        if re.search(r"\[[^\]]*\b" + key + r"\b[^\]]*\]", raw):
            groups[key].append(dict(scope="原文显式标记", claim=raw))
    if not any(groups.values()):
        for key in groups:
            if re.search(r"\[[^\]]*\b" + key + r"\b[^\]]*\]", heading + " " + header):
                groups[key].append(dict(scope="章节/表头级标记，未推定该行每一语义均属此级", claim=heading + " | " + header))
    return dict(groups=groups, unmarked=not any(groups.values()),
                qualification="手册中的历史观察/帮助/静态/推断转述；本轮没有重跑原厂UI，也没有自动核销旧候选")


def question(control, anchor, gaps, source, next_probe, layers=None):  #20261110-4 add
    """Keep an explicit unresolved question with its exact source and next bounded action."""
    key = sid("Q-S00F", control, anchor)
    if key not in QUESTIONS:
        QUESTIONS[key] = dict(id=key, control_id=control, status="未解决", gaps=gaps, source=source,
                              affected_layers=layers or ["D", "U", "P", "C", "E"], next_probe=next_probe,
                              candidate_closure="不关闭或重编号原CAND-20261110/U候选；由S00-04独立处理")
    return key


def split_row(line):  #20261110-4 add
    """Preserve pipes inside inline code while splitting real Markdown cells."""
    result, part, ticks = [], [], False
    for index, char in enumerate(line.strip().strip("|")):
        if char == "`":
            ticks = not ticks
        if char == "|" and not ticks and (index == 0 or not part or part[-1] != "\\"):
            result.append("".join(part).strip())
            part = []
        else:
            part.append(char)
    result.append("".join(part).strip())
    return result


def code_reference(filename, token):  #20261110-4 add
    """Find an actual source declaration/consumer instead of inventing a model binding."""
    path = ROOT / "KinSealStudio/src" / filename
    if not path.exists():
        return None
    source = register(path, "复刻实现源代码；不能替代原厂证据")
    for index, line in enumerate(source["lines"], 1):
        if token in line:
            return ref(source, index)
    return None


def mapping_for(control, label, section):  #20261110-4 add
    """Attach only specifically recovered mappings for the five implemented variants."""
    if control not in {"C01", "C02", "C03", "C04", "C05"}:
        return dict(status="未恢复", model=None, ksp=None, sm=None, device=None,
                    reason="该控件未核定独立实现；不按同名字段套用已有控件的模型/union或设备type")
    model = native = consumer = filename = token = None
    base = "staticText" if control == "C01" else "bitButton"
    permission = bool(re.search("权限|显现|触控", section))
    monitor = bool(re.search("指示灯设置|监视", section))
    appearance = bool(re.search("外观|显示", section))
    if re.search("预览|选择状态|当前语言|语言选择", label):
        return dict(status="实现中的编辑器瞬态/预览", model=None, ksp=None, sm=None, device=None,
                    reason="选择器本身不等于状态/译文内容的保存字段；其具体回填及取消仍看原文与Q-ID")
    if control == "C05" and re.fullmatch(r"功能", label.strip(" **")):
        model, native, consumer, filename, token = "functionButton.function", "function.1c8.0 (i32)", "native_function_button_control.cpp:0x118", "function_button_properties.h", "int function"
    elif control == "C05" and label.strip(" **") == "类型":
        model, native, consumer, filename, token = "functionButton.type", "function.1c8.1 (i32；无类型=-1)", "native_function_button_control.cpp:0x11c", "function_button_properties.h", "int type"
    elif control == "C05" and re.search("键盘|ASCII|Unicode", label):
        model, native, consumer, filename, token = "functionButton.keyboardKey + keyboardText", "function.1c8.5 i32 / .6 QString", "native_function_button_control.cpp:0x120 / 0x121；本地代码页首字节", "function_button_properties.h", "int keyboardKey"
    elif re.search("按下效果", label) and control in {"C03", "C05"}:
        family = "wordButton" if control == "C03" else "functionButton"
        model, native, consumer, filename, token = family + ".pressedEffect", "switch.tail.8；启用时源保存2态", "独立subtype1/3；隐藏ON图及标志", family.replace("Button", "_button") + "_properties.h", "bool pressedEffect"
    elif control == "C03" and not permission and not appearance and re.search("操作类型|数据类型|常数|最小值|最大值|总位|小数", label):
        choices = [("操作", "operation", "word.16c.1"), ("数据类型", "dataType", "word.16c.0"), ("常数", "constantValue", "word.16c.2"), ("最小", "minimumValue", "word.16c.4"), ("最大", "maximumValue", "word.16c.5"), ("总位", "totalDigits", "word.18c.0"), ("小数", "decimalDigits", "word.18c.1")]
        key, member, source_key = next(x for x in choices if x[0] in label)
        model, native, consumer, filename, token = "wordButton." + member, source_key, "native_word_button_control.cpp subtype1 union", "word_button_properties.h", member
    elif control == "C04" and not permission and re.search("状态数|状态数量", label):
        model, native, consumer, filename, token = "lamp.multiStateCount", "switch.248.3 / switch.278值表", "native_lamp_control.cpp:0x190；2..32", "lamp_properties.h", "int multiStateCount"
    elif control == "C04" and not permission and re.search("多态|双态", label):
        model, native, consumer, filename, token = "lamp.monitorMode（复合行还含子字段，待细分）", "switch.248.5；不能套多态按钮布局", "native_lamp_control.cpp:0x184", "lamp_properties.h", "int monitorMode"
    elif permission:
        role = "显现" if control == "C01" else "显现/触控按列分别映射"
        prefix = base if control == "C01" else "bitButton.visibility / bitButton.touch"
        candidates = [("有效最低", "minimumLevel"), ("等级", "levelEnabled"), ("误差", "tolerance"), ("比较值", "compareValue"), ("比较", "comparison"), ("数据类型", "dataType"), ("字地址", "wordAddress"), ("位地址", "bitAddress"), ("受位", "bitEnabled"), ("受字", "wordEnabled"), ("有效", "visibilityEnabled")]
        hit = next((x for x in candidates if x[0] in label), None)
        if hit:
            model, native, consumer, filename, token = prefix + "." + hit[1], "sourceCondition/patchCondition common.*，按触控/显现列区分", role + "条件记录；活动读取进入device_address_compiler，通知写入除外", "static_text_properties.h", hit[1]
    if model is None and re.search("通知位|通知字|通知$|通知功能", label):
        model, native, consumer, filename, token = "bitButton.notice / noticeBit / noticeWord及其值（复合行）", "common.58 / common.c8 / common.e4 / common.fc / common.118", "公共通知写入槽；不加入读取扫描；指示灯禁用", "bit_button_properties.h", "bool notice"
    if model is None and re.search("按压", label):
        model, native, consumer, filename, token = "bitButton.pressTimeEnabled / pressTime", "common.118.1/.2", "公共按压编码；功能0..255，旧控件范围单独校验", "bit_button_properties.h", "pressTime"
    if model is None and re.search("宏指令|使用宏", label):
        model, native, consumer, filename, token = "bitButton.macroEnabled / macroName", "switch.22c.1/.2；休眠元数据保留", "native_device_macro / subtype绑定；指示灯及功能按钮宏禁用", "bit_button_properties.h", "macroEnabled"
    if model is None and not permission and not appearance and re.search("操作地址|控制地址", label):
        if control in {"C02", "C03"}:
            model, native, consumer, filename, token = "HmiObject.address", "switch.12c" if control == "C02" else "word.150", "操作union和provider；读写职责分开", "project_model.h", "QString address"
    if model is None and monitor and re.search("地址|位号|位序|位数|与操作", label):
        model, native, consumer, filename, token = "bitButton.monitorBitAddress / monitorWordAddress / bitIndex / monitorSame（依行细分）", "switch.254 / switch.248.0/.1/.2", "独立监视读取与对应subtype union", "bit_button_properties.h", "monitorBitAddress"
    if model is None and control == "C02" and "操作类型" in label:
        model, native, consumer, filename, token = "bitButton.operation", "switch.144.0", "native_bit_button_control.cpp subtype0", "bit_button_properties.h", "int operation"
    if model is None and (appearance or control == "C01"):
        state_base = "staticText" if control == "C01" else "bitButton.states[state].text"
        style_members = [("字体", "fontFamily"), ("字号", "fontSize"), ("字距", "spacing"), ("对齐", "alignment"), ("斜体", "italic"), ("粗体", "bold"), ("下划线", "underline"), ("自适应", "adaptive"), ("文字颜色", "textColor"), ("文本颜色", "textColor")]
        hit = next((x for x in style_members if x[0] in label), None)
        if hit:
            model, native, consumer, filename, token = state_base + "." + hit[1], "static.194 / switch.290[state][language] / 缓存字体，具体可编辑源字段按codec守卫", "static_text_renderer / bit_button_renderer -> RGB565/LZO像素；不假定直接数值字段", "static_text_properties.h", hit[1]
        elif re.search("同一文字|同一语言|同文", label) and control != "C01":
            model, native, consumer, filename, token = "bitButton.states[state].sameLanguage + sharedText", "Lamp/Function=switch.tail.4；旧Bit/Word源编辑限制独立", "状态/语言图帧数量与文字选择；Function/Lamp保留全局flag", "bit_button_properties.h", "sameLanguage"
        elif re.search("闪烁", label) and control != "C01":
            model, native, consumer, filename, token = "bitButton.states[state].flash", "switch.294[state].2", "正常/隐字帧；不同subtype图序分别处理", "bit_button_properties.h", "bool flash"
        elif re.search("图片|图库|无图|3D|透明效果", label) and control != "C01":
            model, native, consumer, filename, token = "bitButton.states[state]的useImage/picture/picturePng/effect3d/transparent（按原行细分）", "switch.294 + switch.tail.0/.7；Lamp/Function无图kind3", "kvg_library / bit_button_renderer / 位图编译；原始资源字节与渲染验证分开", "bit_button_properties.h", "bool useImage"
        elif re.search("文本|文字|内容", label):
            model, native, consumer, filename, token = state_base + ".languageTexts（共享文字另存）", "static.194[lang].10 / switch.290[state][lang].4", "按语言与状态渲染像素", "static_text_properties.h", "languageTexts"
    if model is None:
        return dict(status="本行未逐字段核定映射", model=None, ksp=None, sm=None, device=None,
                    reason="五类已有实现不代表该表行的每一隐藏/禁用或复合字段都已核定；保留Q-ID")
    implementation_ref = code_reference(filename, token)
    native_key = re.search(r"(?:function|word|switch)\.[0-9a-f]+\.\d+", native or "")
    native_reference = code_reference("native_project_codec.cpp", native_key.group(0)) if native_key else None
    consumer_file = next((item for item in re.findall(r"[a-z_]+\.cpp", consumer or "") if (ROOT / "KinSealStudio/src" / item).exists()), None)
    consumer_reference = code_reference(consumer_file, "0x") if consumer_file else None
    return dict(status="已定位实现属性/消费模块；部分复合行与精确源跨度仍需细分", model=model,
                ksp=dict(scope="pages[0].objects[*]", property=model, qualification="实现序列化路径；不是原厂.sm布局"),
                sm=native, device=consumer, implementation_reference=implementation_ref, native_codec_reference=native_reference, device_module_reference=consumer_reference,
                qualifier="实现映射独立于V/H/B/I；设备原始数值/运行行为仍须相应补证，不能从字段名推导E通过")


def field_record(control, source, line_no, label, raw, section, headers, cells, kind):  #20261110-4 add
    """Create one literal field row/paragraph candidate with explicit unresolved boundaries."""
    field_id = sid("F", control, section + "|" + label)
    source_ref = ref(source, line_no)
    type_terms = re.findall(r"Q[A-Za-z]*(?:Box|Edit|Button|Widget)|复选框|单选(?:组|框)?|下拉(?:框|列表)?|输入框|文本框|颜色(?:按钮|框)|SpinBox|数值框|按钮", raw)
    explicit_values = [dict(column=headers[index] if index < len(headers) else str(index), raw=value)
                       for index, value in enumerate(cells) if re.search(r"默认|范围|最大|最小|上限|下限|[0-9]+[–—~～至]|[0-9]+\.\.[0-9]+", value) or (index < len(headers) and re.search("默认|范围|最大|最小", headers[index]))]
    dependency_claims = [value for value in cells if re.search("勾选|选中|切换|启用|禁用|灰显|隐藏|显示|只有|仅|当|如果|必须|互斥", value)]
    lifecycle_claims = [value for value in cells if re.search("保存|重开|取消|草稿|切换语言|缩减|删除状态|恢复|关闭", value)]
    record_mapping = mapping_for(control, label, section)
    composite = bool(re.search("[/／、]|与|和", label)) or kind != "table_field_row"
    gaps = []
    if not type_terms:
        gaps.append("该行没有可可靠分配到单一字段的控件类型；未从标签猜测")
    if not explicit_values:
        gaps.append("该行无独立默认/范围来源；不从同类控件填入")
    if not lifecycle_claims:
        gaps.append("该行未单独闭合原厂草稿/取消/保存/缩减寿命")
    if composite:
        gaps.append("真实原文为复合行或正文/树候选，尚未逐个子字段原子化")
    if record_mapping["model"] is None:
        gaps.append("模型/.ksp/.sm/设备消费者尚未核定")
    next_probe = f"按{source['path']}:{line_no}打开{section}，定位原文『{label[:100]}』；读取真实Qt class/objectName/enabled/visible/checked/min/max，逐项改值→确定→保存→重开→取消对照，再差分相关源/设备字段；不得借共享窗口字段猜测。"
    question_id = question(control, section + "|" + label, gaps or ["历史资料需在声明范围内逐字段复核，当前不升级D通过"], source_ref, next_probe)
    ac_id = sid("AC", control, "field|" + section + "|" + label)
    return dict(id=field_id, control_id=control, record_kind=kind, label_raw=label, location=dict(section=section, source_line=line_no),
                source=source_ref, columns=dict(zip(headers, cells)) if len(headers) == len(cells) else None,
                raw_cells=cells, table_headers=headers, row_parse_status="列数一致" if len(headers) == len(cells) else "未结构化/列数不同；保留原文",
                ui_type=dict(explicit_terms=list(dict.fromkeys(type_terms)), status="原文词项，复合行不自动分配子字段"),
                default_range_claims=explicit_values, dependency_claims=dependency_claims, lifecycle_claims=lifecycle_claims,
                lifetime_status="仅登记原文；无陈述部分未知", evidence=evidence(raw, section, " | ".join(headers)),
                mapping=record_mapping, decomposition_required=composite, specification_status="待逐字段闭合；不以手册存在判D通过",
                event_ids=[], acceptance_ids=[ac_id], question_ids=[question_id],
                acceptance=dict(id=ac_id, status="设计/待执行", source=source_ref,
                                stimulus=f"定位『{label}』，按其原文依赖执行边界/切换/接受/取消及保存重开；仅验证该字段声明范围",
                                expected_from_source=raw, observations_required=["Qt真实状态和值", "父子草稿事务", "注册模型/源/设备差分（有映射时）"],
                                unresolved_question=question_id))


def compact_traceability(data):  #20261110-4 add
    """Normalize shared sources, mappings and probes; retain literal field cells once."""
    source_ids = {source["path"]: sid("SRC", "S00F", source["path"]) for source in data["sources"]}

    def compact_ref(value):
        if isinstance(value, list):
            return [compact_ref(item) for item in value]
        if isinstance(value, dict):
            if {"path", "sha256", "line_start", "line_end"}.issubset(value):
                return dict(source_id=source_ids[value["path"]], start=value["line_start"], end=value["line_end"])
            return {key: compact_ref(item) for key, item in value.items()}
        return value

    def compact_evidence(value):
        return dict(grades=[key for key, claims in value["groups"].items() if claims],
                    unmarked=value["unmarked"], policy_id="EVIDENCE-DOC-CLAIM")

    data["source_registry"] = {source_ids[source["path"]]: source for source in data.pop("sources")}
    mapping_registry = {}
    header_registry = {}
    gap_registry = {}
    field_refs = {}
    section_lookup = {(control["id"], section["path"]): section["id"] for control in data["controls"] for section in control["sections"]}
    for field in data["fields"]:
        field_refs[field["id"]] = field
        headers = field.pop("table_headers")
        header_id = sid("HEAD", "S00F", json.dumps(headers, ensure_ascii=False))
        header_registry[header_id] = headers
        field["header_id"] = header_id
        field["location"] = section_lookup.get((field["control_id"], field["location"]["section"]), field["location"]["section"])
        field.pop("label_raw")
        field["label_column"] = 0
        field["structured_cells"] = len(headers) == len(field["raw_cells"])
        field.pop("row_parse_status")
        field.pop("columns", None)
        field["default_range_claim_columns"] = [headers.index(item["column"]) if item["column"] in headers else -1 for item in field.pop("default_range_claims")]
        for key in ("dependency_claims", "lifecycle_claims"):
            values = field.pop(key)
            field[key + "_columns"] = [index for index, cell in enumerate(field["raw_cells"]) if cell in values]
        field["ui_type_terms"] = field.pop("ui_type")["explicit_terms"]
        field.pop("lifetime_status", None)
        field.pop("specification_status", None)
        field.pop("acceptance", None)
        field["evidence"] = compact_evidence(field["evidence"])
        field["cell_evidence"] = []
        for index, cell in enumerate(field["raw_cells"]):
            explicit = [key for key in ("V", "H", "B", "I") if re.search(r"\[[^\]]*\b" + key + r"\b[^\]]*\]", cell)]
            inherited = [key for key in ("V", "H", "B", "I") if index < len(headers) and re.search(r"\[[^\]]*\b" + key + r"\b[^\]]*\]", headers[index])]
            field["cell_evidence"].append(dict(column=index, explicit=explicit, column_header=inherited))
        mapping = compact_ref(field.pop("mapping"))
        mapping_id = sid("MAP", "S00F", json.dumps(mapping, ensure_ascii=False, sort_keys=True))
        mapping_registry[mapping_id] = mapping
        field["mapping_id"] = mapping_id
        if field["record_kind"] == "table_field_row":
            header = " ".join(headers)
            if re.search("操作类型|单选|默认类型|最小值.*最大值|字段与默认值|差异点", header):
                field["record_kind"] = "enum_condition_or_difference_row"
            else:
                field["record_kind"] = "handbook_field_table_row"
        else:
            field["record_kind"] = "prose_tree_candidate_not_atomic_field"
    for event in data["events"]:
        event.pop("trigger_and_result_raw")
        event["evidence"] = compact_evidence(event["evidence"])
        event["location"] = section_lookup.get((event["control_id"], event["location"]), event["location"])
        event["status_id"] = "EVENT-DOC-CANDIDATE"
        event.pop("status")
        event["record_kind"] = "document_event_statement_candidate"
    for acceptance in data["acceptances"]:
        acceptance.pop("expected_from_source", None)
        acceptance.pop("stimulus", None)
        acceptance.pop("observations_required", None)
        acceptance["procedure_id"] = "P-FIELD-INSPECT" if "unresolved_question" in acceptance else "P-MANUAL-AC"
        acceptance["record_kind"] = "proposed_field_acceptance" if "unresolved_question" in acceptance else "manual_acceptance_statement"
        if acceptance["record_kind"] == "proposed_field_acceptance":
            original_question = acceptance["unresolved_question"]
            field_id = next(field["id"] for field in data["fields"] if original_question in field["question_ids"])
            acceptance.pop("source")
            acceptance["field_id"] = field_id
    for item in data["questions"]:
        item.pop("next_probe")
        item.pop("candidate_closure")
        item["procedure_id"] = "P-GAP-NATIVE-ROUNDTRIP"
        codes = []
        for gap in item.pop("gaps"):
            code = sid("GAP", "S00F", gap)
            gap_registry[code] = gap
            codes.append(code)
        item["gap_codes"] = codes
    for control in data["controls"]:
        control["manual_source"] = source_ids[control["manual_source"]["path"]]
        control["read_coverage"] = dict(entire_file_decoded=True, all_lines_read=len(SOURCES[data["source_registry"][control["manual_source"]]["path"]]["lines"]), semantic_completeness="部分；没有由行覆盖推定原子字段完整")
        control["entry_examples"] = []
        entry_candidates = []
        for item in control["entries"]:
            quote = item["source"]["quote"]
            if not quote.lstrip().startswith("#") and re.search("点击|工具箱|工具栏|双击|右键|属性窗口", quote):
                score = (3 if "点击" in quote else 0) + (3 if control["name"] in quote else 0) + (2 if re.search("工具箱|工具栏", quote) else 0)
                if re.match(r"\s*1[.、]", quote) and re.search("工具箱|工具栏", quote):
                    score += 10
                entry_candidates.append((score, item["source"]["line_start"], dict(source=item["source"], raw=quote)))
            item["evidence"] = compact_evidence(item["evidence"])
        control["entry_examples"] = [item[2] for item in sorted(entry_candidates, key=lambda item: (-item[0], item[1]))[:4]]
        for item in control["differences"]:
            item["evidence"] = compact_evidence(item["evidence"])
        control.pop("retained_blocks")
        for table in control["tables"]:
            for row in table["rows"]:
                if "field_id" in row:
                    row.pop("cells")
    for supplement in data["implementation_supplements"]:
        supplement["source"] = source_ids[supplement["source"]["path"]]
        supplement["complete_file_read"] = True
        supplement["nonempty_lines"] = [item["line_start"] for item in supplement.pop("paragraphs")]
    data["mapping_registry"] = mapping_registry
    data["header_registry"] = header_registry
    data["gap_registry"] = gap_registry
    data["evidence_policy"] = dict(id="EVIDENCE-DOC-CLAIM", grades=dict(V="手册转述原厂历史运行观察，不是本轮重跑", H="帮助/帮助图片或其转述", B="原厂字符串/对象名/反编译静态证据", I="推断/建议，不能代替V"),
                                   mixed_rows="看cell_evidence逐列标记；一列H动作语义不因同一行另有V默认而升级V", unmarked="保持未标级，不自动当V", implementation="模型代码/测试记录单独放mapping_registry和补证；不是原厂行为证据")
    data["procedure_registry"] = {
        "P-FIELD-INSPECT": dict(status="待执行验收设计", action="根据记录control/location/raw_cells定位真实字段；原文默认/范围/依赖构成待检查期望；获取Qt真实状态、编辑/取消/保存/重开及对应模型/源/设备差分。未知预期先回答所链Q-ID。", prohibitions="不以模板补同类默认、不从PC预览推出E、不把历史测试当新执行"),
        "P-MANUAL-AC": dict(status="手册验收原文，未执行", action="读取所引原文单行及其上下文，拆分刺激/预期/观察量；按D/U/P/R/C/T/E分层登记新运行身份"),
        "P-GAP-NATIVE-ROUNDTRIP": dict(status="下一最小原厂探针，尚未执行", action="通过source和control的完整调用入口打开location，定位该F原标签；先读取Qt class/objectName/enabled/visible/checked/min/max，再单字段改值→确定→保存→重开/取消；需要源或设备映射时做独立差分。复合字段先拆分，不猜未见默认和寿命。")}
    data["audit"]["record_kinds"] = dict(collections.Counter(field["record_kind"] for field in data["fields"]))
    data["audit"]["field_records_without_same_line_event"] = sum(not field["event_ids"] for field in data["fields"])
    data["audit"]["fully_verified_atomic_native_fields"] = 0
    return compact_ref(data)


def main():  #20261110-4 add
    """Build the 31-control structural baseline and verify traceability links without running product tests."""
    out = dict(schema_version=1, task="S00-03", date="2026-11-10", marker="#20261110-4", execution_mode="只读调查与文档，未执行业务构建/设备动作",
               id_policy="C01..C31沿A04；F/EV/AC/Q-S00F用控件+章节路径+原标签的规范化SHA256前12位；不含行号、扫描顺序或文件SHA；原文实质改名产生新ID，不能无声复用",
               status="31手册结构提取完成；字段原子化/逐条D核定仍为部分完成", sources=[], controls=[], fields=[], events=[], acceptances=[], questions=[], implementation_supplements=[])
    parent_docs = ["00_复刻执行总入口.md", "01_阶段0_基线范围与证据管理.md", "A01_公共属性与事务规范.md", "A04_31控件实施索引.md", "A05_源格式与设备编译契约.md"]
    for name in parent_docs:
        register(DESIGN / name, "规范/调用索引；不能代替具体原厂证据")
    index_source = register(DESIGN / "A04_31控件实施索引.md", "调用索引")
    controls = []
    for index, line in enumerate(index_source["lines"], 1):
        match = re.match(r"## (C\d\d)｜([^（]+)（(.+)）", line)
        if match:
            control_id, name, batch = match.groups()
            controls.append((control_id, name, batch, index))
    assert len(controls) == 31
    for control_id, name, batch, index_line in controls:
        source = register(DESIGN / (name + "功能与交互复刻手册.md"), "31控件原手册")
        index_end = next((number for number in range(index_line + 1, len(index_source["lines"]) + 1) if re.match(r"## C\d\d｜", index_source["lines"][number - 1])), len(index_source["lines"]) + 1)
        index_block = index_source["lines"][index_line - 1:index_end - 1]
        control = dict(id=control_id, name=name, batch=batch, manual_source={key: source[key] for key in ("path", "sha256", "bytes", "line_count")}, index_reference=ref(index_source, index_line),
                       specification_status="未判D通过", sections=[], tables=[], retained_blocks=[], entries=[], differences=[], field_ids=[], event_ids=[], acceptance_ids=[], question_ids=[])
        control["a04_specific_differences"] = [ref(index_source, index_line + offset) for offset, line in enumerate(index_block) if "本控件必须单独保留的差异" in line]
        control["original_candidate_ownership"] = dict(ids=re.findall(r"U-\d{3}", "\n".join(index_block)),
                                                     references=[ref(index_source, index_line + offset) for offset, line in enumerate(index_block) if "原始候选" in line],
                                                     status="只接入控件归属；不假定具体F与U一一对应，不执行核销")
        lines = source["lines"]
        hierarchy = []
        sections = []
        section_for_line = {}
        fenced = False
        for number, line in enumerate(lines, 1):
            if line.strip().startswith("```"):
                fenced = not fenced
            match = re.match(r"^(#{1,6})\s+(.+)", line) if not fenced else None
            if match:
                depth, title = len(match[1]), match[2]
                hierarchy = [item for item in hierarchy if item[0] < depth] + [(depth, title)]
                section = dict(id=sid("SEC", control_id, "/".join(x[1] for x in hierarchy)), title=title, path=" / ".join(x[1] for x in hierarchy), depth=depth, start=number)
                sections.append(section)
            section_for_line[number] = sections[-1] if sections else dict(title="前言", path="前言", start=1)
        for index, section in enumerate(sections):
            section["end"] = (sections[index + 1]["start"] - 1) if index + 1 < len(sections) else len(lines)
            section["source"] = ref(source, section["start"], section["end"])
            section["role"] = "字段/窗口/页签候选" if re.search("页|字段|外观|权限|属性|通用|窗口|指示灯|设置|类型", section["title"]) else "调用/交互/证据/边界正文"
        control["sections"] = sections
        table_lines = set()
        index = 0
        while index < len(lines):
            if index + 1 < len(lines) and lines[index].lstrip().startswith("|") and re.match(r"^\s*\|?\s*:?-{2,}", lines[index + 1]):
                first = index + 1
                headers = split_row(lines[index])
                end = index + 2
                while end < len(lines) and lines[end].lstrip().startswith("|"):
                    end += 1
                section = section_for_line[first]["path"]
                header_text = " | ".join(headers)
                is_field = bool(re.search("位置|控件|字段|项目|单选|操作类型|差异点|列|界面文字", header_text)) and not bool(re.search("SHA|图标|内部对象名.*属性对话框", header_text))
                table = dict(id=sid("TABLE", control_id, section + "|" + header_text), headers=headers, source=ref(source, first, end), classification="字段/依赖/枚举行，保留复合性" if is_field else "证据/入口/枚举元信息表，不计字段", rows=[])
                for row_index in range(index + 2, end):
                    cells = split_row(lines[row_index])
                    row = dict(source=ref(source, row_index + 1), cells=cells)
                    if is_field:
                        record = field_record(control_id, source, row_index + 1, cells[0], lines[row_index], section, headers, cells, "table_field_row")
                        if record["id"] not in control["field_ids"]:
                            out["fields"].append(record)
                            out["acceptances"].append(record["acceptance"])
                            control["field_ids"].append(record["id"])
                        row["field_id"] = record["id"]
                        if re.search("点击|双击|勾选|切换|取消|确定|重开|保存|删除|灰显|隐藏|启用|禁用", lines[row_index]):
                            event = dict(id=sid("EV", control_id, section + lines[row_index]), control_id=control_id,
                                         source=ref(source, row_index + 1), location=section, trigger_and_result_raw=lines[row_index],
                                         evidence=evidence(lines[row_index], section, header_text),
                                         status="原文事件候选；复合句未逐刺激拆分，未重跑", field_ids=[])
                            out["events"].append(event)
                            control["event_ids"].append(event["id"])
                    table["rows"].append(row)
                control["tables"].append(table)
                table_lines.update(range(first, end + 1))
                index = end
            else:
                index += 1
        for number, raw in enumerate(lines, 1):
            section = section_for_line[number]
            if re.search("入口|新建|修改现有|公共部分|完整操作路径|前置|硬前置", section["title"]) and raw.strip() and number not in table_lines:
                control["entries"].append(dict(id=sid("ENTRY", control_id, section["path"] + raw), source=ref(source, number), evidence=evidence(raw, section["path"])))
            if raw.strip() and re.search("冲突|差异|待验|未证|不能|禁用|灰显|隐藏|草稿|无效|删除|保留", raw):
                control["differences"].append(dict(source=ref(source, number), evidence=evidence(raw, section["path"])))
            if not raw.strip() or raw.lstrip().startswith("#") or number in table_lines:
                continue
            field_context = bool(re.search("页|字段|外观|权限|设置|类型", section["title"])) and not bool(re.search("证据|定义|约束|验收|缺口|资料", section["title"]))
            paragraph_field = field_context and bool(re.search(r"(?:默认|复选框|单选|下拉|输入框|文本框|按钮|隐藏|禁用|空页|草稿)|(?:[├└][─]+)", raw))
            if paragraph_field:
                label = re.sub(r"^\s*[-*\d.、）\s]+", "", raw).strip()
                record = field_record(control_id, source, number, label, raw, section["path"], ["正文/树原文"], [raw], "prose_or_tree_field_candidate")
                if record["id"] not in control["field_ids"]:
                    out["fields"].append(record)
                    out["acceptances"].append(record["acceptance"])
                    control["field_ids"].append(record["id"])
            if re.search("点击|双击|勾选|切换|取消|确定|重开|保存|增删|删除|灰显|隐藏|启用|禁用|拖|Esc|ESC", raw):
                event = dict(id=sid("EV", control_id, section["path"] + raw), control_id=control_id, source=ref(source, number), location=section["path"],
                             trigger_and_result_raw=raw, evidence=evidence(raw, section["path"]), status="原文事件候选；复合句未逐刺激拆分，未重跑", field_ids=[])
                out["events"].append(event)
                control["event_ids"].append(event["id"])
            if re.search("验收", section["title"]):
                ac = dict(id=sid("AC", control_id, "manual|" + section["path"] + raw), control_id=control_id, source=ref(source, number), status="手册验收原文/未执行", expected_from_source=raw)
                out["acceptances"].append(ac)
                control["acceptance_ids"].append(ac["id"])
        # Keep every original line, including code trees, footnotes and non-tabulated statements.
        control["retained_blocks"] = [ref(source, number, min(number + 49, len(lines))) for number in range(1, len(lines) + 1, 50)]
        if not control["entries"]:
            qid = question(control_id, "调用入口", ["未提取到调用入口"], ref(source, 1), "读取原手册入口与工具箱，区分未见入口和采集失败")
            control["question_ids"].append(qid)
        out["controls"].append(control)
    supplements = [("C01", "静态文本手册实现与验收.md"), ("C02", "位按钮手册实现与验收.md"), ("C03", "字按钮手册实现与验收.md"), ("C04", "指示灯手册实现与验收.md"), ("C05", "功能按钮手册实现与验收.md")]
    supplements.append(("C01+C02", "位按钮设备编译实现.md"))
    for control_id, filename in supplements:
        source = register(ROOT / "KinSealStudio/docs" / filename, "实施补证：历史实现/测试范围，需按具体段落区分")
        out["implementation_supplements"].append(dict(control_id=control_id, source={key: source[key] for key in ("path", "sha256", "bytes", "line_count")},
                                                      paragraphs=[ref(source, index + 1) for index, line in enumerate(source["lines"]) if line.strip()],
                                                      qualification="完整读取与保留；旧报告的下载拒绝由后续补证按范围覆盖，不按文档日期全量覆盖；历史测试未重跑"))
    delta_specs = [
        ("C01", "位按钮设备编译实现.md", "静态文本有两条路径", "旧静态文本报告的单语/无显现下载边界只属历史；现有type28多语言/显现与背景栅格两路径分开，不外推E权限公式。", "显现|多语言|语言|编译|下载"),
        ("C02", "位按钮手册实现与验收.md", "`.ksp` 保存完整两态草稿", "KSP保留两态/宏/语言等草稿；原厂单态SM没有独立另一态记录，不能把KSP寿命等同SM。", "状态|同一|宏|语言|保存|指示灯"),
        ("C02", "位按钮手册实现与验收.md", "互斥位/字通知", "通知互斥；启用按压655个100ms是设备u16毫秒保护，不宣称原厂UI最大输入。", "通知|按压"),
        ("C03", "字按钮手册实现与验收.md", "七个数据类型依次", "U16/U32/I16/I32/BCD16/BCD32/FLOAT，默认I16/LW0/常数1；原文帮助范围、实现保护和实测默认分别追踪。", "数据类型|常数|地址|类型"),
        ("C03", "字按钮手册实现与验收.md", "常态和按下态草稿独立保留", "字按钮指示灯页为空；常态/按下态草稿独立，按下关闭只用常态；不能套位按钮双态监视。", "指示灯|状态|按下|复制"),
        ("C04", "指示灯手册实现与验收.md", "双态与多态的“与操作地址相同”均为禁用", "指示灯操作地址耦合、宏和通知真实禁用；不沿旧合成输入失败推断是否可用。", "与操作|宏|通知"),
        ("C04", "指示灯手册实现与验收.md", "状态数范围为2..32", "真实点击状态数2..32，外观0..N-1；只设checked残留256不作为范围。", "状态数|状态数量|多态"),
        ("C04", "指示灯手册实现与验收.md", "默认位监视存储bitIndex=0", "界面禁用位号15与默认位监视存储0分开；切到字监视后可保存15。", "位号|位序|位数|字监视|位地址"),
        ("C04", "指示灯手册实现与验收.md", "显式状态数5→2→5", "显式缩减删附加状态；双/多态切换保留草稿；同文字段为全局开关，不能按每态独立flag误写。", "状态|同一|同文|删除|切换"),
        ("C05", "功能按钮手册实现与验收.md", "宏组真实Qt enabled=false", "功能按钮宏组禁用；原手册对合成鼠标无响应的未决描述已被该运行补证限定。", "宏指令|使用宏|写宏"),
        ("C05", "功能按钮手册实现与验收.md", "字符框在10个键中均保持可编辑", "字符框始终可编辑、1个UTF16单位；只有字符键影响设备，设备只本地代码页首字节，不能从ASCII/Unicode标签推完整Unicode。", "ASCII|Unicode|键盘|字符"),
        ("C05", "功能按钮手册实现与验收.md", "字体大小1..128；按压时间0..255", "功能字体1..128、按压0..255默认255；不覆盖其他控件范围。", "字体|字号|按压"),
        ("C05", "功能按钮手册实现与验收.md", "背景色与透明度只在", "无图时背景/透明度可调，3D/透明勾选不再另外禁这两项。", "背景|透明度|无图|图片|3D|透明"),
        ("C05", "功能按钮手册实现与验收.md", "两项从状态1复制按钮禁用", "状态0锁定与隐藏ON按下图并存；两项从状态1复制禁用，状态0改字/无图不自动改按下帧。", "状态|按下|复制|图片"),
        ("C05", "功能按钮手册实现与验收.md", "按Qt大端流`iiqbbiq`读取", "源iiqbbiq：func/type i32、保留QString/2byte、key i32/textQString；独立设备type10/subtype3/640B，不凭22B长度猜key u16。", "功能|类型|键盘|ASCII|Unicode")
    ]
    out["known_implementation_deltas"] = []
    for control_id, filename, needle, adopted, label_pattern in delta_specs:
        source = register(ROOT / "KinSealStudio/docs" / filename, "实施补证")
        line_number = next((index for index, line in enumerate(source["lines"], 1) if needle in line), None)
        assert line_number is not None, (filename, needle)
        delta_id = sid("DELTA", control_id, needle)
        linked_fields = [field["id"] for field in out["fields"] if field["control_id"] == control_id and re.search(label_pattern, field["label_raw"])]
        out["known_implementation_deltas"].append(dict(id=delta_id, control_id=control_id, source=ref(source, line_number), adopted_scope=adopted, related_field_ids=linked_fields,
                                                       qualification="实施报告中历史补证/实现范围，非本轮新运行；只关联相应差异，不自动关闭本切片Q或原始U候选"))
        for field in out["fields"]:
            if field["id"] in linked_fields:
                field.setdefault("implementation_delta_ids", []).append(delta_id)
    for field in out["fields"]:
        linked = [event for event in out["events"] if event["control_id"] == field["control_id"] and event["source"]["line_start"] == field["source"]["line_start"]]
        field["event_ids"] = [event["id"] for event in linked]
        for event in linked:
            event["field_ids"].append(field["id"])
    for control in out["controls"]:
        control["acceptance_ids"] += [field["acceptance_ids"][0] for field in out["fields"] if field["control_id"] == control["id"]]
        control["question_ids"] += [qid for field in out["fields"] if field["control_id"] == control["id"] for qid in field["question_ids"]]
    out["questions"] = list(QUESTIONS.values())
    out["sources"] = [{key: value for key, value in source.items() if key != "lines"} for source in SOURCES.values()]
    out["audit"] = dict(manual_count=len(controls), manual_lines=sum(control["manual_source"]["line_count"] for control in out["controls"]),
                        retained_manual_lines=sum(sum(block["line_end"] - block["line_start"] + 1 for block in control["retained_blocks"]) for control in out["controls"]),
                        headings=sum(len(control["sections"]) for control in out["controls"]), tables=sum(len(control["tables"]) for control in out["controls"]),
                        table_data_rows=sum(sum(len(table["rows"]) for table in control["tables"]) for control in out["controls"]),
                        field_records=len(out["fields"]), field_table_rows=sum(field["record_kind"] == "table_field_row" for field in out["fields"]),
                        prose_tree_candidates=sum(field["record_kind"] != "table_field_row" for field in out["fields"]),
                        decomposition_required=sum(field["decomposition_required"] for field in out["fields"]), events=len(out["events"]), acceptances=len(out["acceptances"]), questions=len(out["questions"]),
                        mapped_records=sum(field["mapping"]["model"] is not None for field in out["fields"]),
                        meanings="字段记录数包含真实复合表行/正文候选，不是原子字段数、规格通过数或测试执行数；原文保留率不等于语义覆盖率")
    for group in ("fields", "events", "acceptances", "questions"):
        ids = [record["id"] for record in out[group]]
        assert len(ids) == len(set(ids)), (group, "duplicate IDs")
    assert out["audit"]["manual_lines"] == out["audit"]["retained_manual_lines"]
    OUT.mkdir(parents=True, exist_ok=True)
    out = compact_traceability(out)
    (OUT / "field-traceability.json").write_text(json.dumps(out, ensure_ascii=False, separators=(",", ":")) + "\n", encoding="utf-8")
    (OWN / "build-audit.json").write_text(json.dumps(out["audit"], ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps(out["audit"], ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
