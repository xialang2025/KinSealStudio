"""Normalize retained native shell evidence into a reproducible Qt resource manifest."""
#20261111-1 add: This tool reads retained evidence only; it never launches the original editor or accesses USB.
import hashlib
import json
from pathlib import Path
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]


def main():
    """Input: Retained native JSON and PNG files. Return: Stable layout and resource file."""
    evidence = ROOT / "docs/main-window-evidence"
    native = json.loads((evidence / "native-shell.json").read_text(encoding="utf-8"))
    aliases = {"action_split": "action_ungroup", "action_sengtoback": "action_sendtoback",
               "action_bringback": "action_sendbackward", "action_ProLib": "action_project_pictures"}
    actions = {}

    def icon(entry):
        path = entry.get("icon", {}).get("path", "")
        return path.removeprefix("resources/native-shell/")

    def action(entry):
        original = entry.get("id", "")
        identifier = aliases.get(original, original)
        if identifier and not entry.get("separator"):
            previous = actions.get(identifier, {})
            actions[identifier] = dict(text=previous.get("text", entry.get("text", "")),
                                       icon=icon(entry) or previous.get("icon", ""),
                                       nativeId=original, checkable=bool(entry.get("checkable")),
                                       checked=bool(entry.get("checked")))
        return identifier

    def menu(entry):
        if entry.get("separator"):
            return dict(separator=True)
        if "children" in entry:
            if "最近" in entry["text"]: return dict(text=entry["text"], items=[])
            return dict(text=entry["text"], items=[menu(child) for child in entry["children"]])
        return dict(id=action(entry))

    menus = [menu(entry) for entry in native["menus"]]
    menus[5]["tools"] = True
    rows = []
    bars = sorted(native["toolbars"], key=lambda item: (item["geometry"]["y"], item["geometry"]["x"]))
    for index, bar in enumerate(bars):
        geometry = bar["geometry"]
        row = dict(id=bar["id"], text=bar.get("title", ""), row=geometry["y"], height=geometry["height"], items=[])
        if index + 1 < len(bars) and bars[index + 1]["geometry"]["y"] == geometry["y"]:
            row["width"] = geometry["width"]
        for entry in bar["items"]:
            if entry.get("separator"):
                row["items"].append(dict(kind="separator"))
            elif bar["id"] == "toolBar_Combox":
                row["items"].extend([
                    dict(kind="label", text="状态", x=20, width=24), dict(kind="state", id="NativeStatePreview", width=68, x=45),
                    dict(kind="label", text="语言", x=125, width=24), dict(kind="language", id="ProjectLanguage", width=50, x=155),
                    dict(kind="page", id="ProjectPage", width=150, x=211)])
            else:
                row["items"].append(dict(id=action(entry)))
        rows.append(row)
    pairs = {
        "btn_switch_bit": "action_bitBtn", "btn_switch_word": "action_wordbutton", "btn_switch_Lamp": "action_lamp",
        "btn_switch_Fun": "action_funbutton", "btn_switch_Screen": "action_screenbutton", "btn_switch_MultiState": "action_polyBtn",
        "btn_dataDisplay": "action_dataDisplay", "btn_asciiDisplay": "action_asciiDisplay", "btn_timeDisplay": "action_timeDisplay",
        "btn_MF": "action_MFbtn", "btn_gif": "action_GIFoutput", "btn_picture": "pictruedisplay",
        "btn_alarmbar": "action_dynamicarlarm", "btn_hisalarmDisplay": "action_histotyarlarm", "btn_alarmDisplay": "action_arlamcontrol",
        "btn_text": "action_text", "btn_messageDisplay": "action_messagedisplay", "btn_XYtrend": "action_X_Y",
        "btn_trend": "action_trendchart", "btn_HisTrend": "action_DatagroupDisplay", "btn_dataGroup": "action_dataGroup",
        "btn_hisdataDisplay": "action_historyrecord", "btn__recipetable": "action_recipetable", "btn_metre": "graphicsitem",
        "btn_droplist": "action_droplist", "btn_flow": "action_flow_block", "btn_slider": "action_Slider",
        "btn_moveTrack": "action_moveTrack", "btn_wheelview": "action_wheelView", "btn_qrcode": "action_qrcode", "btn_scale": "action_mark"}
    controls = []
    for entry in sorted(native["toolbox"], key=lambda item: (item["geometry"]["y"], item["geometry"]["x"])):
        identifier = pairs[entry["id"]]
        controls.append(dict(widget=entry["id"], id=identifier, text=entry["text"], icon=icon(entry).replace(".png", "-32.png"),
                             buttonRect=entry["geometry"], captionRect=entry["captionLabels"][0]["geometry"]))
    basic_pairs = dict(btn_line="action_line", btn_foldline="action_foldline", btn_freeline="action_freeline",
                       btn_linecycle="action_blight_circle", btn_polygon="action_polygon", btn_roundrect="action_RoundedRect",
                       btn_rect="action_rect", btn_elipse="action_ellipse", btn_arc="action_ArcItem", btn_table="action_table")
    basic = [dict(widget=entry["id"], id=basic_pairs[entry["id"]], text=entry["text"], icon=icon(entry).replace(".png", "-32.png"),
                  buttonRect=entry["geometry"], captionRect=entry["captionLabels"][0]["geometry"])
             for entry in sorted(native["toolboxBasic"], key=lambda item: (item["geometry"]["y"], item["geometry"]["x"]))]
    tree = json.loads((evidence / "native-tree-status.json").read_text(encoding="utf-8"))
    field_ids = iter(["NativeCompressedSize", "NativePageMemory", "NativeControlCount", "NativeResolution",
                      "NativeObjectX", "NativeObjectY", "NativeObjectWidth", "NativeObjectHeight"])
    status = []
    for index, entry in enumerate(tree["statusBar"]["fieldsLeftToRight"]):
        if entry["kind"] == "QLineEdit":
            status.append(dict(kind="field", id=next(field_ids), width=entry["geometry"][2]))
        else:
            status.append(dict(kind="label", text=entry["text"] if index != 8 else "",
                               id="NativeSelectionType" if index == 8 else "", width=entry["geometry"][2]))
    tree_icons = {node["text"]: node["icons"][0]["path"].removeprefix("resources/native-shell/")
                  for node in tree["tree"]["nodes"] if node["icons"]}
    result = dict(schemaVersion=1, originalSha256=native["originalSha256"], actions=actions, menus=menus,
                  toolbars=rows, controls=controls, basic=basic, status=status, treeIcons=tree_icons)
    folder = ROOT / "resources/native-shell"
    (folder / "layout.json").write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    resource = ET.Element("RCC")
    group = ET.SubElement(resource, "qresource", prefix="/native-shell")
    for path in sorted(folder.rglob("*")):
        if path.is_file() and path.suffix in (".png", ".json", ".ico", ".svg"):
            ET.SubElement(group, "file", alias=path.relative_to(folder).as_posix()).text = path.relative_to(ROOT).as_posix()
    aliases_present = {element.attrib["alias"] for element in group}
    required = {entry["icon"] for entry in actions.values() if entry["icon"]}
    required.update(entry["icon"] for entry in controls + basic if entry["icon"])
    required.update(tree_icons.values())
    for alias in sorted(required - aliases_present):
        path = folder / alias
        if not path.exists(): raise RuntimeError("Missing retained icon: " + alias)
        ET.SubElement(group, "file", alias=alias).text = path.relative_to(ROOT).as_posix()
    ET.indent(resource)
    ET.ElementTree(resource).write(ROOT / "native_shell.qrc", encoding="utf-8", xml_declaration=True)
    print(json.dumps(dict(actions=len(actions), controls=len(controls), toolbars=len(rows), icons=len(list(folder.rglob('*.png'))))))


if __name__ == "__main__":
    main()
