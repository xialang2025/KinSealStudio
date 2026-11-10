"""#20261110-5 add: Retain exact XML model metadata without inventing units."""
from pathlib import Path
import hashlib
import json
import re
import xml.etree.ElementTree as ET

ROOT = Path(r"Z:\Project\vscode_prj_temp\REA")
VENDOR = Path(r"G:\01_Professional\触摸屏\金玺\KinSealStudio_Setup_V5.1\Installed")
OUT = ROOT / "KinSealStudio/docs/stage1-evidence/native-analysis"
MODEL = "AMT070W124-L800X480R"
result = {"date": "2026-11-10", "targetModel": MODEL, "method": "Python hashlib + ElementTree and exact bounded XML member extraction; not a REA execution or firmware readback", "artifacts": [], "limitations": ["XML metadata is static installed configuration, not observed firmware behavior or full UI capability proof", "Keep every raw lexical value: decimal/hex interpretation and size units require consumer confirmation", "Device=76 is an internal XML field; not inferred as USB VID/PID", "fbottomFile/cbottomFile name a disk image, not the connected device firmware identity"]}
for filename in ("model.dat", "modelInfo.dat"):
    path = VENDOR / filename
    raw = path.read_bytes()
    root = ET.fromstring(raw)
    matching = [element for element in root.findall("Model") if element.attrib.get("name") == MODEL]
    assert len(matching) == 1
    pattern = rb'<Model\s+name="' + MODEL.encode() + rb'"\s*>.*?</Model>'
    hits = list(re.finditer(pattern, raw, re.DOTALL))
    assert len(hits) == 1
    hit = hits[0]
    snippet = hit.group()
    snippet_path = OUT / (filename + ".target.xml")
    snippet_path.write_bytes(snippet)
    fields = [{"tag": child.tag, "rawText": child.text or "", "attributes": child.attrib} for child in matching[0]]
    result["artifacts"].append({"path": str(path), "length": len(raw), "sha256": hashlib.sha256(raw).hexdigest(), "encoding": "UTF-8 with BOM; XML declares UTF-8", "rootTag": root.tag, "modelElementCount": len(root.findall("Model")), "matchingTargetCount": 1, "targetByteRange": {"start": hit.start(), "endExclusive": hit.end()}, "targetSnippet": {"path": snippet_path.relative_to(ROOT).as_posix(), "length": len(snippet), "sha256": hashlib.sha256(snippet).hexdigest()}, "fields": fields})
(OUT / "model-metadata.json").write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
lines = ["# AMT型号元数据的静态身份与原始字段", "", "日期：2026-11-10。来源为原安装model.dat/modelInfo.dat；两者都是带UTF-8 BOM的XML。此记录采用普通Python文件解析，不是REA运行证据，也没有读回固件。", "", "| 文件 | 字节数 | SHA256 |", "|---|---:|---|"]
for artifact in result["artifacts"]:
    lines.append("| %s | %d | `%s` |" % (Path(artifact["path"]).name, artifact["length"], artifact["sha256"]))
for artifact in result["artifacts"]:
    lines += ["", "## " + Path(artifact["path"]).name, "", "目标节点精确字节区间：[%d,%d)，摘录哈希`%s`。" % (artifact["targetByteRange"]["start"], artifact["targetByteRange"]["endExclusive"], artifact["targetSnippet"]["sha256"]), "", "| 原始标签 | 原始值 |", "|---|---|"]
    lines += ["| `%s` | `%s` |" % (field["tag"], field["rawText"]) for field in artifact["fields"]]
lines += ["", "## 可采用的范围", "", "- 型号节点名与原厂源头型号均为AMT070W124-L800X480R；model.dat声明Seris=AMT、Pixels=800*480、COM1/COM2/USB=true。它们是静态配置声明，不代替端口/方向/通信运行验证。", "- modelInfo.dat声明Device=76、USB=2、SerialPortDown=false，maxScreen=200、maxControl=100、MonitorStates/MfbStates=32等。其他容量、地址、缓冲区均保留原文，尚未统一进制或单位。", "- 新建普通页UI实际编号微调0..999，和maxScreen=200的配置字段分别记录；编号空间与页面数量上限不可混同。默认新页方向下拉有4项但当前AMT窗口中隐藏，不能由此开放旋转编译。", "- 已有AMT设备编译/USB事实仍引用A05及既有协议。XML的guilib、驱动目录、宏地址及离线/在线EXE字段只说明所配置的文件/地址，不证明所有消费者与固件ABI已恢复。", "- Q-S01-NATIVE-MODEL-001：定位原厂对这些字段的具体toInt/toUInt进制、单位及限制检查；在指定AMT UI按字段取边界，再做原厂离线差分。Q-S01-NATIVE-FIRMWARE-001：后续读取实际连接设备版本/固件身份，不以磁盘同名bin代替。", ""]
(OUT / "model-metadata.md").write_text("\n".join(lines), encoding="utf-8")
print(json.dumps({"status": "parsed", "target": MODEL, "files": len(result["artifacts"]), "fields": [len(a["fields"]) for a in result["artifacts"]]}))
