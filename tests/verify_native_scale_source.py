"""Verify snapshot-backed scale editing against the V5.1 scale graphs, including the paired direction change."""
# 20261128-28 add
import argparse
from copy import deepcopy
import json
from pathlib import Path
import struct
from tempfile import TemporaryDirectory

from verify_amz_local_page_io import edit, layout, sha
from verify_native_drawing_drafts import ROOT, REVISION, git_blob, run


BASE = "docs/stage4-evidence/scale/native"
CASES = ("default", "direction-down", "position-top", "equal-zero", "main6", "sub4")


def source(exe, work, name, raw):
    """Bind each field's raw bytes to an exact source, including dormant metadata and padding."""
    path = work / name
    path.write_bytes(raw)
    spans = layout(exe, work, path)
    return {key: raw[field["start"]:field["end"]] for key, field in spans.items()}


def check(exe, work, name):
    """Use the normal import/save routes, and compare patches against raw typed fields as well as the reread model."""
    original = git_blob(BASE, f"scale-{name}.sm")
    fields = source(exe, work, "input.sm", original)
    ksp = work / "project.ksp"
    saved = work / "saved.sm"
    run(exe, work, "--migrate-project", work/"input.sm", ksp)
    model = json.loads(ksp.read_bytes())
    obj, = model["pages"][0]["objects"]
    expected = dict(minimum=0, maximum=0 if name == "equal-zero" else 100, fontSize=16,
                    majorCount=6 if name == "main6" else 4, minorCount=4 if name == "sub4" else 1,
                    direction=int(name == "direction-down"), position=0 if name == "position-top" else 3,
                    showValues=True, decimals=0)
    if obj["type"] != "scale" or any(obj["scale"][k] != v for k, v in expected.items()):
        raise ValueError("the canvas scale does not match its source settings")
    if tuple(obj[k] for k in ("x","y","width","height")) != (160,120,200,100):
        raise ValueError("the cached child bounds differ from the 200x100 factory placement")
    run(exe,work,"--export-native-project",ksp,saved)
    if saved.read_bytes() != original:
        raise ValueError("unedited SM changed via KSP")
    row = dict(reference=f"{BASE}/scale-{name}.sm", sourceSha256=sha(original), settings=expected, directRoundTrip=True)
    # Counterfactual extent/range changes that would regenerate the native font metrics must not be accepted.
    negative = []
    for prop, value in (("majorCount", 7), ("minorCount", 0), ("fontSize", 18), ("maximum", 12345)):
        bad = deepcopy(model)
        bad["pages"][0]["objects"][0]["scale"][prop] = value
        ksp.write_text(json.dumps(bad,ensure_ascii=False),encoding="utf8")
        invalid = work/("invalid-"+prop+".sm")
        run(exe,work,"--export-native-project",ksp,invalid,success=False)
        if invalid.exists():
            raise ValueError("rejected edit published a native file")
        negative.append(prop)
    row["refusedProperties"] = negative
    # Changing direction permutes the cached caption widths and their strings. Two separate original saves exist.
    if name in ("default","direction-down"):
        variant = deepcopy(model)
        variant["pages"][0]["objects"][0]["scale"]["direction"] ^= 1
        ksp.write_text(json.dumps(variant,ensure_ascii=False),encoding="utf8")
        path = work/"reversed.sm"
        run(exe,work,"--export-native-project",ksp,path)
        actual = source(exe,work,"reversed-inspect.sm",path.read_bytes())
        other = "direction-down" if name == "default" else "default"
        reference = source(exe,work,"factory-other.sm",git_blob(BASE,f"scale-{other}.sm"))
        drift = sorted(k for k,v in reference.items() if actual[k] != v)
        if drift != ["object[0].common.58.3", "trailer.settings.0"] or any(actual[k]!=fields[k] for k in drift):
            raise ValueError("direction transition differs from the vendor beyond its two session fields")
        run(exe,work,"--migrate-project",path,work/"reverse.ksp")
        if json.loads((work/"reverse.ksp").read_bytes())["pages"][0]["objects"][0]["scale"] != variant["pages"][0]["objects"][0]["scale"]:
            raise ValueError("direction edit was lost on reimport")
        row["oppositeOriginalSessionFieldsRetained"] = drift
    obj.update(x=167,y=129,name="刻度复验")
    if name != "equal-zero":
        obj["scale"].update(minimum=1,maximum=101)
    ksp.write_text(json.dumps(model,ensure_ascii=False),encoding="utf8")
    path = work/"changed.sm"
    run(exe,work,"--export-native-project",ksp,path)
    actual = source(exe,work,"changed-inspect.sm",path.read_bytes())
    if fields.keys() != actual.keys():
        raise ValueError("the edit changed native control topology")
    differences = {k for k,v in actual.items() if fields[k] != v}
    for key in differences:
        if key.endswith((".text.tail.10",".stroke.3",".text.tail.11",".stroke.4")):
            delta = 7 if key.endswith((".text.tail.10",".stroke.3")) else 9
            if struct.unpack(">d",actual[key])[0] != struct.unpack(">d",fields[key])[0]+delta:
                raise ValueError("incorrect scene-space child translation")
        elif not (key.endswith(".text.head.4") or key in
                  ("object[0].group.metadata-name","object[0].private.9","object[0].private.10")):
            raise ValueError("unrelated field changed: " + key)
    run(exe,work,"--migrate-project",path,work/"changed.ksp")
    again = json.loads((work/"changed.ksp").read_bytes())["pages"][0]["objects"][0]
    if any(again[k] != obj[k] for k in ("name","x","y","width","height","scale")):
        raise ValueError("edited scale settings not preserved")
    row.update(editedSmSha256=sha(path.read_bytes()), changedFields=len(differences), editedReopen=True)
    return row


def main():
    """Keep one receipt, and remove all per-source working files on exit."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe", type=Path)
    parser.add_argument("--scratch", type=Path,default=ROOT/".local")
    parser.add_argument("--receipt", type=Path)
    args=parser.parse_args()
    exe=args.exe.resolve()
    rows=[]
    for name in CASES:
        with TemporaryDirectory(dir=args.scratch) as temp:
            rows.append(check(exe,Path(temp).resolve(),name))
    with TemporaryDirectory(dir=args.scratch) as temp:
        work=Path(temp).resolve()
        sm=ROOT/"docs/文件规则/样例/刻度二维码与消息-重建.sm"
        ksp=work/"mixed.ksp"
        run(exe,work,"--migrate-project",sm,ksp)
        mixed=json.loads(ksp.read_bytes())
        if [obj["type"] for obj in mixed["pages"][0]["objects"]] != ["scale","qrcode","message"]:
            raise ValueError("mixed source no longer opens in the ordinary canvas")
        run(exe,work,"--export-native-project",ksp,work/"mixed-exact.sm")
        if (work/"mixed-exact.sm").read_bytes() != sm.read_bytes():
            raise ValueError("mixed source was not fully restored")
        mixed["pages"][0]["objects"][0]["name"]="新刻度"
        ksp.write_text(json.dumps(mixed,ensure_ascii=False),encoding="utf8")
        run(exe,work,"--export-native-project",ksp,work/"mixed-edited.sm")
        actual=source(exe,work,"inspect-edited.sm",(work/"mixed-edited.sm").read_bytes())
        before=source(exe,work,"inspect-base.sm",sm.read_bytes())
        if {key for key in before if before[key]!=actual[key]}!={"object[0].group.metadata-name"}:
            raise ValueError("editing a scale changed its neighbour or a project field")
        raw=git_blob(BASE,"scale-default.sm")
        original=work/"input.sm"
        original.write_bytes(raw)
        spans=layout(exe,work,original)
        negative=[]
        for index,(key,value) in enumerate((("object[0].common.touch.0",1),("object[0].common.visibility.0",1),
                                   ("object[0].common.58.2",1),("object[0].child[2].font.family","Arial"))):
            changed=work/f"unsupported{index}.sm"
            output=work/f"unsupported{index}.ksp"
            changed.write_bytes(edit(raw,spans,{key:value}))
            run(exe,work,"--migrate-project",changed,output,success=False)
            if output.exists():
                raise ValueError("unsupported native scale was presented as an editable KSP")
            negative.append(key)
    receipt=dict(revision=REVISION,exeSha256=sha(exe.read_bytes()),cases=rows,
                 mixedSmSha256=sha(sm.read_bytes()), mixedRenamedField="object[0].group.metadata-name",
                 unsupportedInputFields=negative)
    if args.receipt:
        args.receipt.write_text(json.dumps(receipt,ensure_ascii=False,indent=1)+"\n",encoding="utf8")
    print(json.dumps(dict(originals=len(rows),editedReopens=len(rows),mixedExact=True,
                          rejections=sum(len(r["refusedProperties"]) for r in rows)+len(negative))))


if __name__ == "__main__":
    main()
