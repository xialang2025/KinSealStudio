"""#20261111-0 add: Summarize original main-window evidence without changing product code."""
from pathlib import Path
import hashlib
import gzip
import json
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs/main-window-evidence"

def digest(data):
    """Return SHA-256 of an immutable artifact."""
    return hashlib.sha256(data).hexdigest()

def read_record(name):
    """Read a complete raw record or its verified gzip archive."""
    file = OUT / name
    data = file.read_bytes() if file.exists() else gzip.decompress(file.with_suffix(file.suffix+".gz").read_bytes())
    return json.loads(data.decode("utf8"))

def main():
    """Publish measured geometry, original hierarchy, icon identities and status fields."""
    raw = read_record("native-tree-probe-tree-status-r8.json")
    client = read_record("native-tree-probe-tree-status-r7.json")
    outer = read_record("native-tree-probe-tree-status-r4.json")
    assert raw["status"] == client["status"] == outer["status"] == "native-stage3-scenario-completed"
    metrics = raw["shellMetrics"]
    native = {item["hwnd"]: item for item in metrics["native"]}
    palettes = {}
    widgets = []
    for item in metrics["qt"]:
        widget = dict(item["qt"])
        palette = widget.pop("palette")
        palette_id = digest(json.dumps(palette, sort_keys=True).encode("utf8"))[:12]
        palettes[palette_id] = palette
        record = native[item["hwnd"]]
        widget.update(hwnd=item["hwnd"], parentHwnd=record["parent"], paletteId=palette_id,
                      mainClientRect=record["mainClientOrigin"]+[record["clientRect"][2], record["clientRect"][3]])
        widgets.append(widget)
    default_expansion = {item["text"]: bool(item["expanded"]) for item in raw["treeDefaultExpansion"][0]["rows"]}
    tree = raw["treeDetails"][0]
    nodes = []
    stack = []
    counts = {}
    for item in tree["nodes"]:
        depth = (item["rect"][0] - tree["nodes"][0]["rect"][0]) // 20
        assert item["rect"][0] == 20 + depth * 20
        stack = stack[:depth]
        parent = stack[-1] if stack else None
        ordinal = counts.get(parent, 0)
        counts[parent] = ordinal + 1
        path = ([] if parent is None else nodes[parent]["path"]) + [ordinal]
        node = {"ordinal": len(nodes), "path": path, "parentOrdinal": parent,
                "depth": depth, "text": item["text"], "expandedByDefault": default_expansion.get(item["text"], False),
                "visibleByDefault": item["text"] in default_expansion, "expandedVisualRect": item["rect"], "icons": []}
        for icon in item["icon"].get("exports", []):
            assert icon["saved"]
            file = Path(icon["path"])
            with Image.open(file) as image:
                assert 0 < image.width <= icon["size"] and 0 < image.height <= icon["size"]
                assert image.convert("RGBA").getchannel("A").getbbox() is not None
                width, height = image.size
            node["icons"].append({"path": file.relative_to(ROOT).as_posix(), "requestedSize": icon["size"],
                                  "size": [width, height], "bytes": file.stat().st_size, "sha256": digest(file.read_bytes())})
        nodes.append(node)
        stack.append(len(nodes)-1)
    status = next(widget for widget in widgets if widget["kind"] == "QStatusBar")
    status_fields = []
    for widget in widgets:
        if widget["kind"] in ("QStatusBar", "QSizeGrip"):
            continue
        parent = widget["parentHwnd"]
        while parent in native and parent != status["hwnd"]:
            parent = native[parent]["parent"]
        if parent == status["hwnd"]:
            status_fields.append(widget)
    status_fields.sort(key=lambda widget: widget["mainClientRect"][0])
    plugin_identities = []
    vendor = Path(r"G:/01_Professional/触摸屏/金玺/KinSealStudio_Setup_V5.1/Installed")
    for directory in ("codecs", "imageformats", "iconengines"):
        for file in (vendor/directory).glob("*4.dll"):
            if not file.name.endswith("d4.dll"):
                plugin_identities.append({"directory": directory, "file": file.name, "sha256": digest(file.read_bytes())})
    captures = []
    for record, frame in ((outer["mainCapture"], "full-window"), (client["clientCapture"], "client"), (client["mainCapture"], "full-window-after-client-resize")):
        record = dict(record)
        record["frame"] = frame
        record["path"] = "docs/main-window-evidence/" + record["path"]
        captures.append(record)
    output = {"date": "2026-11-11", "marker": "//20261111-0", "status": "native-tree-status-captured",
              "provenance": {"originalSha256": raw["originalSha256"], "runtimeSha256": raw["runtimeSha256"],
                             "startupPatch": raw["patch"], "desktop": raw["desktop"], "input": raw["input"],
                             "ownedPid": raw["pid"], "ownedProcessTerminated": raw["ownedProcessTerminated"],
                             "qtOperationsGuiThreadOnly": True, "systemInput": False, "usbOrPlc": False,
                             "releasePlugins": plugin_identities,
                             "actions": "私有进程打开原厂位按钮seed后全选/删除，仅清空自有进程画布内存；未保存输入源。所有读取在自有GUI线程执行。"},
              "coordinates": {"dpi": metrics["windowDpi"], "mainClientSize": metrics["clientRect"][2:],
                              "mainClientScreenOrigin": metrics["clientScreenOrigin"], "outerRect": metrics["outerRect"],
                              "widgetGeometry": "Qt geometry 相对直接父项；顶层为屏幕客户区坐标。",
                              "mainClientRect": "所有widgets.mainClientRect统一为主窗口客户区坐标，基于Win32 ClientToScreen，无重采样。"},
              "captures": captures, "tree": {"rootCount": 1, "defaultVisibleCount": len(default_expansion), "expandedNodeCount": len(nodes),
                                            "indentation": 20, "rowHeight": 17, "iconCount": sum(bool(node["icons"]) for node in nodes),
                                            "hierarchyEvidence": tree["hierarchyEvidence"], "nodes": nodes},
              "statusBar": {"widget": status, "fieldsLeftToRight": status_fields},
              "palettes": palettes, "widgets": widgets,
              "notes": ["原厂Qt4的codecs/imageformats/iconengines发布插件必须随私有EXE提供，否则GBK中文或SVG图标不完整；失败初轮保留。",
                        "full-window为1353×731时客户区实际1337×692；client为1353×708时系统完整外框1369×747。图像frame字段明确区分，未把Windows边框差异混作客户端布局。",
                        "主客户区工具栏到底y129；左dock宽242；右dock固定最小宽350；状态栏高25。statusBar实际最小内容宽略超过窄客户区，未篡改其测量值。",
                        "28个展开节点中27个带原生QIcon；画面1叶子无图标。默认COM1折叠，Modbus_RTU仅在展开后可见。",
                        "画布蓝色来自该原厂seed页底色；属于工程内容，并不定义新工程固定底色。状态栏几何字段保留原厂删除选择后的旧值，控件总数已为0。",
                        "QPixmap::grabWidget初次失败记录保留；最终client图使用PrintWindow后精确客户区裁切，没有缩放。"]}
    (OUT / "native-tree-status.json").write_text(json.dumps(output, ensure_ascii=False, indent=2)+"\n", encoding="utf8")
    print(json.dumps({"widgets": len(widgets), "palettes": len(palettes), "nodes": len(nodes), "icons": sum(len(node["icons"]) for node in nodes), "statusFields": len(status_fields), "path": str(OUT / "native-tree-status.json")}, ensure_ascii=False))

if __name__ == "__main__":
    main()
