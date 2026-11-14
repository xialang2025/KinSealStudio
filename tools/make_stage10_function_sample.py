"""Create a new internal-only language/visibility/notice acceptance page."""
#20261114-9 add: Existing typed sample fields are reused; no new device encoding is introduced.
import copy
import hashlib
import json
from pathlib import Path

import make_stage10_samples as fixtures

ROOT = Path(__file__).resolve().parents[1]
MARKER = "S10-FUNC-LOCAL-1114"
LANGUAGES = ["中文", "English"]


def style_state(state, label, colour):
    """Input: existing appearance, ASCII label, colour. Return: None after disabling image/flash paths."""
    fixtures.appearance(state, label, colour)
    state["picture"] = ""
    state["text"].update(texts={language: label for language in LANGUAGES},
                         textColor="#ff000000", style=1, opacity=255)


def plain_shared(obj, label):
    """Input: a typed switch copy and label. Return: None after removing unrelated active features."""
    shared = obj["bitButton"]
    shared.update(macroEnabled=False, macroName="", notice=False, noticeBit=False,
                  noticeWord=False, pressTimeEnabled=False)
    for field in ("visibility", "touch"):
        shared[field].update(visibilityEnabled=False, bitEnabled=False,
                             levelEnabled=False, wordEnabled=False)
    for state in shared["states"]:
        style_state(state, label, "#ffffffff")
    obj["text"] = label


def static_label(seed, identity, x, y, width, height, labels, size=20):
    """Input: real text seed, identity, rectangle, translations, size. Return: opaque text copy."""
    obj = fixtures.position(seed, identity, x, y, width, height)
    obj["text"] = labels["中文"]
    obj["staticText"].update(texts=labels, fontSize=size, spacing=0, style=1,
                              opacity=255, textColor="#ff000000", backgroundColor="#ffffffff",
                              visibilityEnabled=False, bitEnabled=False, levelEnabled=False,
                              wordEnabled=False)
    return obj


def bit_action(seed, identity, address, label, x, y):
    """Input: verified SET/RESET seed and internal address. Return: independent opaque action copy."""
    obj = fixtures.position(seed, identity, x, y, 215, 70)
    obj["address"] = address
    plain_shared(obj, label)
    obj["bitButton"].update(monitorBitAddress=address, monitorSame=True, monitorType=0)
    return obj


def lamp(seed, identity, address, x, y, width):
    """Input: verified two-state bit lamp and local address. Return: independent green/white observer."""
    obj = fixtures.position(seed, identity, x, y, width, 70)
    plain_shared(obj, address)
    obj["address"] = ""
    obj["bitButton"].update(monitorBitAddress=address, monitorSame=False, monitorType=0)
    for index, state in enumerate(obj["bitButton"]["states"]):
        style_state(state, f"{address} = {index}", ("#ffffffff", "#ff80ff80")[index])
    return obj


def readonly_checks(project):
    """Input: generated JSON model. Return: independent structural scope checks or raise before save."""
    objects = project["pages"][0]["objects"]
    ids = {obj["id"]: obj for obj in objects}
    switches = [obj for obj in objects if obj["type"] == "switch"]
    functions = [obj for obj in switches if "functionButton" in obj]
    overlap = []
    for index, left in enumerate(objects):
        for right in objects[index + 1:]:
            if (left["x"] < right["x"] + right["width"] and right["x"] < left["x"] + left["width"] and
                    left["y"] < right["y"] + right["height"] and right["y"] < left["y"] + left["height"]):
                overlap.append([left["id"], right["id"]])
    checks = {
        "exactModelAndResolution": project["targetModel"] == "AMT070W124-L800X480R" and project["resolution"] == {"width": 800, "height": 480},
        "twoLanguagesInDeclaredOrder": project["languages"] == LANGUAGES and project["activeLanguage"] == "中文",
        "elevenDistinctObjects": len(objects) == 11 and len(ids) == 11,
        "inside800x480": all(0 <= obj["x"] and 0 <= obj["y"] and obj["width"] > 0 and obj["height"] > 0 and
                              obj["x"] + obj["width"] <= 800 and obj["y"] + obj["height"] <= 480 for obj in objects),
        "noOverlap": not overlap,
        "onlyLanguageSystemActions": len(functions) == 3 and all(obj["functionButton"]["function"] == 13 for obj in functions),
        "languageTargetsExplicit": ids["language-cn"]["functionButton"]["type"] == 0 and ids["language-en"]["functionButton"]["type"] == 1 and ids["language-gated-cn"]["functionButton"]["type"] == 0,
        "independentLanguageMarker": ids["language-marker"]["staticText"]["texts"] == {"中文": "LANG = CN", "English": "LANG = EN"},
        "onlyLB41SetResetAndLB40Reset": {(obj["id"], obj["address"], obj["bitButton"]["operation"]) for obj in switches if "functionButton" not in obj and "lamp" not in obj} == {("set-LB41", "LB41", 0), ("reset-LB41", "LB41", 1), ("reset-LB40", "LB40", 1)},
        "independentLB41LB40Lamps": {(obj["id"], obj["bitButton"]["monitorBitAddress"]) for obj in switches if "lamp" in obj} == {("lamp-LB41", "LB41"), ("lamp-LB40", "LB40")},
        "onlyEnglishNotice": [obj["id"] for obj in switches if obj["bitButton"]["notice"]] == ["language-en"],
        "noticeWritesLB40True": ids["language-en"]["bitButton"]["noticeBit"] is True and ids["language-en"]["bitButton"]["noticeBitAddress"] == "LB40" and ids["language-en"]["bitButton"]["noticeBitValue"] is True and ids["language-en"]["bitButton"]["noticeWord"] is False,
        "onlyGatedVisibility": [obj["id"] for obj in switches if obj["bitButton"]["visibility"]["visibilityEnabled"]] == ["language-gated-cn"],
        "gateIsLB41TrueOnly": ids["language-gated-cn"]["bitButton"]["visibility"]["bitEnabled"] is True and ids["language-gated-cn"]["bitButton"]["visibility"]["bitAddress"] == "LB41" and ids["language-gated-cn"]["bitButton"]["visibility"]["bitState"] is True and not ids["language-gated-cn"]["bitButton"]["visibility"]["levelEnabled"] and not ids["language-gated-cn"]["bitButton"]["visibility"]["wordEnabled"],
        "noPicturesFlashOrTransparency": all(not state["useImage"] and not state["picture"] and not state["picturePng"] and not state["flash"] and not state["transparent"] and not state["effect3d"] and state["opacity"] == 255 for obj in switches for state in obj["bitButton"]["states"]),
        "noMacroOrPressureGate": all(not obj["bitButton"]["macroEnabled"] and not obj["bitButton"]["macroName"] and not obj["bitButton"]["pressTimeEnabled"] for obj in switches),
        "noTouchGateOrPressedEffect": all(not obj["bitButton"]["touch"]["visibilityEnabled"] for obj in switches) and all(not obj["functionButton"]["pressedEffect"] for obj in functions),
        "noExternalConnectionsOrResources": not project.get("connections") and not project.get("macros") and not project.get("pictures") and not project.get("nativeSourceSnapshot"),
        "allVisibleLabelsASCII": all(obj["text"].isascii() and all(value.isascii() for value in obj.get("staticText", {}).get("texts", {}).values()) and all(state["sharedText"].isascii() and all(value.isascii() for value in state["text"]["texts"].values()) for state in obj.get("bitButton", {}).get("states", [])) for obj in objects)
    }
    if not all(checks.values()):
        raise ValueError("Generated field check failed: " + repr([name for name, passed in checks.items() if not passed]))
    return checks


def main():
    """Input: None. Return: zero after exclusive fixture/report creation; never compile or access devices."""
    sample = ROOT / "samples/stage10/function-local.ksp"
    output = ROOT / "docs/stage10-evidence/function-design-owned/derivation.json"
    if sample.exists() or output.exists():
        raise FileExistsError("New-only creation refuses an existing sample or design report")
    function_path = "samples/function-button/功能按钮实机联调.ksp"
    bit_path = "samples/bit-button/四操作实机验收.ksp"
    lamp_path = "samples/lamp/指示灯实机联调.ksp"
    function = fixtures.load(function_path)
    bits = fixtures.load(bit_path)["pages"][0]["objects"]
    lamps = fixtures.load(lamp_path)["pages"][0]["objects"]
    funcs = function["pages"][0]["objects"]
    seed_text = next(obj for obj in bits if obj["type"] == "text")
    seed_set = next(obj for obj in bits if obj.get("bitButton", {}).get("operation") == 0)
    seed_reset = next(obj for obj in bits if obj.get("bitButton", {}).get("operation") == 1)
    seed_lamp = next(obj for obj in lamps if obj.get("lamp", {}).get("monitorMode") == 1)
    project = copy.deepcopy(function)
    project.update(projectName=MARKER, languages=LANGUAGES[:], activeLanguage="中文", macros={}, pictures={})
    objects = [static_label(seed_text, "page-marker", 20, 15, 760, 30,
                            {language: MARKER for language in LANGUAGES}),
               static_label(seed_text, "language-marker", 20, 55, 760, 32,
                            {"中文": "LANG = CN", "English": "LANG = EN"}, 24)]
    for identity, type_index, label, x in (("language-cn", 0, "LANG CN", 25),
                                           ("language-en", 1, "LANG EN + N40", 290),
                                           ("language-gated-cn", 0, "GATED CN", 555)):
        seed = next(obj for obj in funcs if obj.get("functionButton", {}).get("function") == 13 and obj["functionButton"]["type"] == type_index)
        obj = fixtures.position(seed, identity, x, 110, 215 if x < 555 else 220, 70)
        plain_shared(obj, label)
        obj["functionButton"]["pressedEffect"] = False
        if identity == "language-en":
            notice = next(obj for obj in funcs if obj["bitButton"]["notice"] and obj["bitButton"]["noticeBit"])
            for key in ("notice", "noticeBit", "noticeBitAddress", "noticeBitValue", "noticeWord", "noticeWordAddress", "noticeWordType", "noticeWordValue"):
                obj["bitButton"][key] = copy.deepcopy(notice["bitButton"][key])
        if identity == "language-gated-cn":
            gated = next(obj for obj in funcs if obj["bitButton"]["visibility"]["visibilityEnabled"])
            obj["bitButton"]["visibility"] = copy.deepcopy(gated["bitButton"]["visibility"])
        objects.append(obj)
    objects.extend([bit_action(seed_set, "set-LB41", "LB41", "SET LB41", 25, 225),
                    bit_action(seed_reset, "reset-LB41", "LB41", "RESET LB41", 290, 225),
                    lamp(seed_lamp, "lamp-LB41", "LB41", 555, 225, 220),
                    bit_action(seed_reset, "reset-LB40", "LB40", "RESET LB40", 25, 340),
                    lamp(seed_lamp, "lamp-LB40", "LB40", 290, 340, 215),
                    static_label(seed_text, "initialization-guide", 20, 430, 760, 30,
                                 {language: "RESET LB41 + RESET LB40 FIRST" for language in LANGUAGES}, 18)])
    for index, obj in enumerate(objects):
        obj["layer"] = index
    project["pages"][0]["objects"] = objects
    checks = readonly_checks(project)
    written = fixtures.save("function-local.ksp", project)
    output.parent.mkdir(parents=True, exist_ok=True)
    report = {"format": "KinSealStage10-function-local-design-v1", "marker": MARKER,
              "createdSample": written, "generatorSha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
              "sourceSamples": [{"path": name, "sha256": hashlib.sha256((ROOT / name).read_bytes()).hexdigest()} for name in (function_path, bit_path, lamp_path)],
              "rootHelper": {"path": "tools/make_stage10_samples.py", "sha256": hashlib.sha256((ROOT / "tools/make_stage10_samples.py").read_bytes()).hexdigest()},
              "layout": [{"id": obj["id"], "rect": [obj[key] for key in ("x", "y", "width", "height")], "layer": obj["layer"], "label": obj["text"]} for obj in objects],
              "readOnlyFieldChecks": checks, "allFieldChecksPassed": all(checks.values()),
              "expectationSources": [
                  {"path": "design_plan/功能按钮功能与交互复刻手册.md", "sections": "function13 language order; visibility bit1; mutually exclusive notice-bit state"},
                  {"path": "docs/功能按钮手册实现与验收.md", "sections": "existing LB40 notification and LB41 visibility sample; source/compiled function13 encoding"},
                  {"path": "samples/function-button/功能按钮实机联调.ksp", "records": "function-6, function-7 language union; function-0 notice; function-14 visibility"},
                  {"path": "samples/bit-button/四操作实机验收.ksp", "records": "usb-button-1 SET operation0; usb-button-2 RESET operation1"},
                  {"path": "samples/lamp/指示灯实机联调.ksp", "records": "LB0双态 monitorMode1 mapped to LB40/LB41"}],
              "runtimeStimuliSuggested": ["Observe exact page marker and independent LANG text; do not assume initial LB values.",
                  "Press RESET LB41 and RESET LB40, observe both independent lamps=0; record whether GATED CN is hidden.",
                  "Press LANG EN + N40; observe independent marker LANG = EN and LB40 lamp=1 as separate facts.",
                  "Press RESET LB40, then LANG CN; observe marker LANG = CN and record LB40 lamp unchanged=0.",
                  "Press SET LB41; observe LB41 lamp=1 and GATED CN becomes visible.",
                  "Press LANG EN + N40, then visible GATED CN; observe marker changes EN to CN; notification ordering is not inferred.",
                  "Press RESET LB41; observe independent LB41 lamp=0 and GATED CN hidden; finish with RESET LB40."],
              "limits": ["No device initialization/retention assumption for LB40/LB41; reset explicitly before observation.",
                         "Visibility and language results do not prove notification timing or hidden-region hit testing.",
                         "No outer PLC, macro, reboot, calibration or system-setting action exists in this fixture.",
                         "Field design checks are not HmiProject parser, C compilation, T download or E runtime acceptance."],
              "compilerRun": False, "usbAccessed": False, "plcAccessed": False, "screenObserved": False}
    with output.open("x", encoding="utf-8", newline="\n") as stream:
        json.dump(report, stream, ensure_ascii=False, indent=2)
        stream.write("\n")
    print(json.dumps({"sample": written, "objects": len(objects), "checks": len(checks), "allPassed": True}, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
