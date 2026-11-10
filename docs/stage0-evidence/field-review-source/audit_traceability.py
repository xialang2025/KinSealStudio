"""#20261110-4: Audit document structure, source identities and cross-record links only."""
from pathlib import Path
import hashlib
import json
import sys

sys.stdout.reconfigure(encoding="utf-8")
ROOT = next(parent for parent in Path(__file__).resolve().parents if (parent / "KinSealStudio/design_plan/A04_31控件实施索引.md").is_file())
OWN = Path(__file__).resolve().parent
path = ROOT / "KinSealStudio/docs/stage0-evidence/field-traceability.json"
data = json.loads(path.read_text(encoding="utf-8"))
errors = []
counts = {}
sources = data["source_registry"]
for source_id, source in sources.items():
    content = (ROOT / source["path"]).read_bytes()
    if hashlib.sha256(content).hexdigest() != source["sha256"]:
        errors.append(["source_sha_changed", source_id, source["path"]])
    if len(content.decode("utf-8-sig").splitlines()) != source["line_count"]:
        errors.append(["line_count_changed", source_id])


def visit(item):
    """Validate every normalized source span against the exact current file identity."""
    if isinstance(item, dict):
        if {"source_id", "start", "end"}.issubset(item):
            source = sources.get(item["source_id"])
            if source is None or not (1 <= item["start"] <= item["end"] <= source["line_count"]):
                errors.append(["bad_source_span", item])
        for child in item.values():
            visit(child)
    elif isinstance(item, list):
        for child in item:
            visit(child)


visit(data)
sets = {}
for name in ("fields", "events", "acceptances", "questions"):
    ids = [item["id"] for item in data[name]]
    sets[name] = set(ids)
    counts[name] = len(ids)
    if len(ids) != len(set(ids)):
        errors.append(["duplicate_ids", name])
control_ids = {control["id"] for control in data["controls"]}
if control_ids != {f"C{index:02}" for index in range(1, 32)}:
    errors.append(["control_coverage", sorted(control_ids)])
for field in data["fields"]:
    for name, ids in (("events", field["event_ids"]), ("acceptances", field["acceptance_ids"]), ("questions", field["question_ids"])):
        for target in ids:
            if target not in sets[name]:
                errors.append(["dangling_field_link", field["id"], target])
    if field["header_id"] not in data["header_registry"] or field["mapping_id"] not in data["mapping_registry"]:
        errors.append(["missing_catalogue", field["id"]])
for control in data["controls"]:
    if not control["entries"] or not control["entry_examples"]:
        errors.append(["missing_call_entry", control["id"]])
    if any(item["raw"].lstrip().startswith("#") for item in control["entry_examples"]):
        errors.append(["heading_used_as_entry", control["id"]])
    section_ids = [section["id"] for section in control["sections"]]
    if len(section_ids) != len(set(section_ids)):
        errors.append(["duplicate_section_ids", control["id"]])
    table_ids = [table["id"] for table in control["tables"]]
    if len(table_ids) != len(set(table_ids)):
        errors.append(["duplicate_table_ids", control["id"]])
    source = sources[control["manual_source"]]
    lines = (ROOT / source["path"]).read_text(encoding="utf-8-sig").splitlines()
    for table in control["tables"]:
        for row in table["rows"]:
            if not lines[row["source"]["start"] - 1].lstrip().startswith("|"):
                errors.append(["table_row_not_literal_source", control["id"], row])
    for field_id in control["field_ids"]:
        if field_id not in sets["fields"]:
            errors.append(["unknown_control_field", control["id"], field_id])
report = dict(scope="只审计文档/引用，不执行业务测试或设备动作", document_bytes=path.stat().st_size,
              document_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
              source_files=len(sources), controls=len(control_ids), record_counts=counts,
              all_current_source_sha_match=not any(error[0] == "source_sha_changed" for error in errors),
              all_links_and_spans_valid=not errors, errors=errors,
              semantic_boundary="字段候选是否完整原子化、原厂默认/寿命是否充分补证，不在机器结构检查通过声明内")
(OWN / "audit-result.json").write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
print(json.dumps(report, ensure_ascii=False, indent=2))
sys.exit(bool(errors))
