"""Verify type-35 editable source and disk message output against original V5.1 files."""
# 20261128-23 add
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
NATIVE = "docs/stage4-evidence/message/native/"
REV = "e3b72657"


def verify(exe, directory):
    """Compare whole native tables and archives, then change real editor properties in both directions."""
    def git_blob(path):
        return subprocess.check_output(["git", "show", REV + ":" + path], cwd=ROOT)

    def run(*args, expect=True):
        result = subprocess.run([exe, *map(str, args)], capture_output=True, timeout=60,
                                env=dict(os.environ, QT_QPA_PLATFORM="offscreen", TEMP=str(directory), TMP=str(directory)))
        if (result.returncode == 0) != expect:
            raise AssertionError((args, result.returncode, result.stdout.decode("utf-8", "replace")))

    def digest(value):
        return hashlib.sha256(value).hexdigest()

    paths = subprocess.check_output(["git", "ls-tree", "-r", "--name-only", REV, NATIVE], cwd=ROOT).decode().splitlines()
    sources = [p for p in paths if p.endswith(".sm") and "/compiled/" not in p]
    tables = [p for p in paths if p.endswith("/Message.dat")]
    assert len(sources) == 10 and len(tables) == 9
    receipts = {"revision": REV, "sourceRoundTrips": [], "compiledMessageTables": [], "simulatorTables": []}
    sessions = {}
    for log in (p for p in paths if "/native-probe-" in p and p.endswith(".json")):
        evidence = json.loads(git_blob(log))
        for ordinal, build in enumerate(evidence.get("nativeCompilations", [])):
            assert build["label"] not in sessions
            sessions[build["label"]] = (build["sourceSha256"], "download-defaults" if ordinal else "fresh", log, ordinal)
    sm, ksp, output = (directory / name for name in ("input.sm", "canvas.ksp", "output.sm"))
    for path in sources:
        original = git_blob(path)
        sm.write_bytes(original)
        run("--migrate-project", sm, ksp)
        run("--export-native-project", ksp, output)
        assert output.read_bytes() == original, path
        receipts["sourceRoundTrips"].append({"source": path, "bytes": len(original), "sha256": digest(original)})
    # Read the actual saved preview index and QString bytes independently of the KSP importer.
    def preview(source):
        spans = directory / "preview-fields.json"
        run("--inspect-native-source", source, spans)
        raw = source.read_bytes()
        fields = {v["path"]: raw[v["start"]:v["end"]] for v in json.loads(spans.read_bytes())["fields"]}
        state = int.from_bytes(fields["object[0].private.head.0"], "big", signed=True)
        count = int.from_bytes(fields["object[0].private.head.4"], "big")
        text = fields["object[0].child[2].text.head.4"]
        assert int.from_bytes(text[:4], "big") == len(text) - 4
        return count, state, text[4:].decode("utf-16-be")

    regrown = git_blob(NATIVE + "message-state3-regrown.sm")
    sm.write_bytes(regrown)
    assert preview(sm) == (3, 1, "1")
    run("--migrate-project", sm, ksp)
    original = json.loads(ksp.read_bytes())
    # The native source has one state/language matrix. Extra KSP language keys and wholly shadowed
    # fallback styles cannot be recovered after a successful native save.
    rejected_drafts = 0
    for authored in (False, True):
        for field in ("states", "languageAppearances", "languageFrameColors", "appearance", "frameColor"):
            candidate = json.loads(json.dumps(original))
            draft = candidate["pages"][0]["objects"][0]["messageDisplay"]
            if authored:
                candidate.pop("nativeSourceSnapshot", None)
                candidate.pop("nativeSourceWarnings", None)
            if field == "states":
                draft["states"][0]["未注册语言"] = "不能丢失"
            elif field in ("languageAppearances", "languageFrameColors"):
                draft[field]["未注册语言"] = draft[field]["中文"]
            elif field == "appearance":
                draft[field]["fontFamily"] = "Unrepresented fallback"
            else:
                draft[field] = "#ff123456"
            item = directory / f"unrepresentable-{int(authored)}-{field}.ksp"
            destination = item.with_suffix(".sm")
            item.write_text(json.dumps(candidate, ensure_ascii=False), encoding="utf-8")
            run("--export-native-project", item, destination, expect=False)
            assert not destination.exists()
            rejected_drafts += 1
    receipts["protectedMessageDrafts"] = rejected_drafts
    run("--export-native-project", ksp, output)
    assert output.read_bytes() == regrown
    for row, new_text, expected_preview in ((1, "已选第二行", "已选第二行"), (0, "改第一行", "1")):
        candidate = json.loads(json.dumps(original))
        obj = candidate["pages"][0]["objects"][0]
        obj["messageDisplay"]["states"][row]["中文"] = new_text
        if row == 0:
            obj["text"] = new_text
        ksp.write_text(json.dumps(candidate, ensure_ascii=False), encoding="utf8")
        run("--export-native-project", ksp, output)
        assert preview(output) == (3, 1, expected_preview)
    single = directory / "original-single.sm"
    single.write_bytes(git_blob(NATIVE + "message-state1.sm"))
    expected_single = preview(single)
    assert expected_single == (1, 0, "0")
    obj = original["pages"][0]["objects"][0]
    msg = obj["messageDisplay"]
    msg["stateCount"], msg["states"] = 1, msg["states"][:1]
    ksp.write_text(json.dumps(original, ensure_ascii=False), encoding="utf8")
    run("--export-native-project", ksp, output)
    # The regrown source has KEEP-2 in row zero. Clamping the preview must keep this independent row text.
    assert preview(output) == (1, 0, "KEEP-2")
    obj["text"] = msg["states"][0]["中文"] = "0"
    ksp.write_text(json.dumps(original, ensure_ascii=False), encoding="utf8")
    run("--export-native-project", ksp, output)
    assert preview(output) == expected_single
    run("--migrate-project", output, ksp)
    assert json.loads(ksp.read_bytes())["pages"][0]["objects"][0]["messageDisplay"]["stateCount"] == 1
    receipts["nativePreviewState"] = dict(originalState=1, selectedEdit=True, inactiveEdit=True,
                                         retainedFirstRowOnShrink=True, shrunkToNativeSingle=True, unchangedBytes=True)
    for path in tables:
        folder = path.rsplit("/", 1)[0]
        sm.write_bytes(git_blob(folder + "/input-native.sm"))
        target = directory / (folder.rsplit("/", 1)[1] + ".dat")
        run("--compile-native-message-table", sm, target)
        original = git_blob(path)
        assert target.read_bytes() == original == git_blob(folder + "/SimMessage.dat")[512:], path
        receipts["compiledMessageTables"].append({"file": path, "bytes": len(original), "sha256": digest(original)})
        source_sha, profile, log, ordinal = sessions[folder.rsplit("/", 1)[1]]
        assert source_sha == digest(sm.read_bytes())
        sim = directory / ("sim-" + target.name)
        run("--compile-native-message-table", sm, sim, "--simulator-session", profile)
        expected_sim = git_blob(folder + "/SimMessage.dat")
        assert sim.read_bytes() == expected_sim, folder
        receipts["simulatorTables"].append({"reference": folder, "profile": profile, "nativeLog": log,
                                            "compileIndex": ordinal, "bytes": len(expected_sim), "sha256": digest(expected_sim)})

    sm.write_bytes(git_blob(NATIVE + "message-bilingual-colored.sm"))
    run("--migrate-project", sm, ksp)
    project = json.loads(ksp.read_bytes())
    obj = project["pages"][0]["objects"][0]
    msg = obj["messageDisplay"]
    original = json.loads(ksp.read_bytes())
    msg.update(monitorAddress="LW27", speed=9, movement=1, dataType=1, stateCount=3)
    msg["states"][0]["English"] = "READY"
    msg["states"].append({"中文": "测试", "English": "END"})
    msg["languageAppearances"]["English"]["fontSize"] = 48
    obj.update(x=77, y=39, address="LW27")
    ksp.write_text(json.dumps(project, ensure_ascii=False), encoding="utf8")
    run("--export-native-project", ksp, output)
    back = directory / "back.ksp"
    run("--migrate-project", output, back)
    edited = json.loads(back.read_bytes())
    current = edited["pages"][0]["objects"][0]
    assert current["messageDisplay"] == msg and (current["x"], current["y"]) == (77, 39)
    # Restoring the two original states must also recover the opaque per-state cache words and original preview.
    edited["pages"][0]["objects"] = original["pages"][0]["objects"]
    back.write_text(json.dumps(edited, ensure_ascii=False), encoding="utf8")
    run("--export-native-project", back, output)
    assert output.read_bytes() == sm.read_bytes()
    # Rebuild the object in a new project with all native source metadata removed.
    for key in ("nativeSourceSnapshot", "nativeSourceWarnings"):
        project.pop(key, None)
    ksp.write_text(json.dumps(project, ensure_ascii=False), encoding="utf8")
    run("--export-native-project", ksp, output)
    run("--migrate-project", output, back)
    rebuilt = json.loads(back.read_bytes())["pages"][0]["objects"][0]["messageDisplay"]
    assert rebuilt["states"] == msg["states"] and rebuilt["languageAppearances"] == msg["languageAppearances"]
    assert rebuilt["monitorAddress"] == "LW27" and rebuilt["movement"] == 1 and rebuilt["speed"] == 9
    target = directory / "new.dat"
    def native_color(language):
        # V122 KSP stores #AARRGGBB. The V5.1 writer emits the three components then a zero byte.
        argb = bytes.fromhex(msg["languageAppearances"][language]["textColor"][1:])
        return argb[1:] + bytes(1)

    expected = b"".join(native_color(lang) + row[lang].encode("cp936").ljust(100, b"\0")
                        for lang in project["languages"] for row in msg["states"])
    run("--compile-native-message-table", output, target)
    assert target.read_bytes() == expected
    obj.update(width=121, height=57)
    ksp.write_text(json.dumps(project, ensure_ascii=False), encoding="utf8")
    run("--export-native-project", ksp, output)
    sim = directory / "resized-simulator.dat"
    run("--compile-native-message-table", output, sim, "--simulator-session", "fresh")
    assert int.from_bytes(sim.read_bytes()[436:440], "little") == 121 * 57 * 2 and sim.read_bytes()[512:] == expected
    run("--compile-native-message-table", output, target, expect=False)
    assert target.read_bytes() == expected
    for index, value in enumerate(("A" * 100, "B" * 101, "A" * 98 + "中", "C" * 99 + "中")):
        msg["states"][1]["English"] = value
        msg["languageAppearances"]["English"]["textColor"] = "#ff1234ab"
        ksp.write_text(json.dumps(project, ensure_ascii=False), encoding="utf8")
        colored = directory / f"bounded-colored-{index}.dat"
        run("--compile-native-message-table", ksp, colored)
        # The second language follows all three first-language states; the high byte of the color DWORD stays zero.
        offset = 4 * 104
        assert colored.read_bytes()[offset:offset + 104] == bytes.fromhex("12 34 ab 00") + value.encode("cp936")[:100]
    for index, invalid in enumerate(("A\0B", "\U0001f600")):
        msg["states"][1]["English"] = invalid
        ksp.write_text(json.dumps(project, ensure_ascii=False), encoding="utf8")
        target = directory / f"invalid-{index}.dat"
        run("--compile-native-message-table", ksp, target, expect=False)
        assert not target.exists()
    receipts.update(editedAndRestored=True, authoredWithoutSource=True, protectedOutputs=3,
                    nativeColorAndHundredByteSlots=4)
    return receipts


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="message-format-", dir=ROOT / "build") as temp:
        result = verify(args.exe.resolve(), Path(temp))
    report = json.dumps(result, ensure_ascii=False, indent=2)
    if args.report:
        args.report.write_text(report + "\n", encoding="utf8")
    print(report)
