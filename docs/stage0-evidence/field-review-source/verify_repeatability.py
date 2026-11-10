"""#20261110-4: Rebuild twice and compare identities, bytes and report links; no product actions."""
from pathlib import Path
import hashlib
import json
import re
import subprocess
import sys

sys.stdout.reconfigure(encoding="utf-8")
ROOT = next(parent for parent in Path(__file__).resolve().parents if (parent / "KinSealStudio/design_plan/A04_31控件实施索引.md").is_file())
OWN = Path(__file__).resolve().parent
JSON_PATH = OWN.parent / "field-traceability.json"
REPORT_PATH = OWN.parent / "field-traceability.md"


def run_recipe():  #20261110-4 add
    """Regenerate the traceability, audit its exact bytes, then write the bounded report."""
    for name in ("build_traceability.py", "audit_traceability.py", "write_report.py"):
        result = subprocess.run([sys.executable, str(OWN / name)], cwd=ROOT, capture_output=True)
        if result.returncode:
            sys.stdout.write(result.stdout.decode("utf-8", errors="replace"))
            sys.stderr.write(result.stderr.decode("utf-8", errors="replace"))
            raise SystemExit(result.returncode)


def missing_local_links():  #20261110-4 add
    """Check generated report and recipe navigation without opening image viewers."""
    missing = []
    for path in (REPORT_PATH, OWN / "README.md"):
        for target in re.findall(r"\]\(([^)]+)\)", path.read_text(encoding="utf-8")):
            if target.startswith(("http:", "https:", "#")):
                continue
            if not (path.parent / target.split("#")[0]).exists():
                missing.append([path.relative_to(ROOT).as_posix(), target])
    return missing


run_recipe()
first_bytes = JSON_PATH.read_bytes()
first_report = REPORT_PATH.read_bytes()
first = json.loads(first_bytes)
run_recipe()
second_bytes = JSON_PATH.read_bytes()
second = json.loads(second_bytes)
ids = {kind: {item["id"] for item in first[kind]} == {item["id"] for item in second[kind]} for kind in ("controls", "fields", "events", "acceptances", "questions")}
result = dict(marker="#20261110-4", date="2026-11-10", scope="相同输入的提取/结构审计/导航报告两次重生成；不含业务或设备测试",
              source_registry_unchanged=first["source_registry"] == second["source_registry"], id_sets_equal=ids,
              json_bytes_equal=first_bytes == second_bytes, report_bytes_equal=first_report == REPORT_PATH.read_bytes(),
              first_json_sha256=hashlib.sha256(first_bytes).hexdigest(), second_json_sha256=hashlib.sha256(second_bytes).hexdigest(),
              missing_local_markdown_targets=missing_local_links(),
              recipe_sha256={path.name: hashlib.sha256(path.read_bytes()).hexdigest() for path in sorted(OWN.glob("*.py"))})
result["passed"] = result["source_registry_unchanged"] and all(ids.values()) and result["json_bytes_equal"] and result["report_bytes_equal"] and not result["missing_local_markdown_targets"]
(OWN / "repeatability-result.json").write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
print(json.dumps(result, ensure_ascii=False, indent=2))
sys.exit(not result["passed"])
