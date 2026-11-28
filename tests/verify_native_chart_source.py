"""Compare the type-11 canvas reader/writer to three factory V122 sources and independent Qt4 field encodings."""
# 20261128-32 add
import argparse
from copy import deepcopy
import json
from pathlib import Path
import struct
from tempfile import TemporaryDirectory

from verify_amz_local_page_io import layout, sha
from verify_native_drawing_drafts import ROOT, run


MODES = ("bar", "meter", "donut")
ORIGINALS = ("1cc6b375830e6fea36f1c74110f38314e9c96e929a31f660ff80bbda9bc1d23e",
             "3b7809752260cb2cd775cf800031b6c06262d0609237e80e7269b12d8d78ede1",
             "f3e3294fafe3f77b80ca10bc7aa86b151f335e0fe07e812e5ef6c5cb2c5bd399")
POSITION = ((".rectangle.9", ".ellipse.pos.0", ".text.tail.10", ".stroke.3"),
            (".rectangle.10", ".ellipse.pos.1", ".text.tail.11", ".stroke.4"))


def put(path, model):
    """Persist a KSP candidate in the same UTF-8 form accepted by the editor."""
    path.write_text(json.dumps(model, ensure_ascii=False), encoding="utf-8")


def qcolor(value):
    """Encode one opaque RGB color as Qt4 spec/alpha/16-bit channels/padding."""
    return struct.pack(">BHHHHH", 1, 65535, *(int(value[i:i+2],16)*257 for i in (1,3,5)), 0)


def make_patch(raw, fields, values):
    """Replace only independently predicted native byte spans without using any product encoder."""
    for k, value in sorted(values.items(), key=lambda item: -fields[item[0]]["start"]):
        f = fields[k]
        if f["kind"] == "q":
            b = value.encode("utf-16-be")
            encoded = struct.pack(">I", len(b)) + b
        elif f["kind"] == "d":
            encoded = struct.pack(">d", value)
        elif f["kind"] == "c":
            encoded = qcolor(value)
        else:
            encoded = int(value).to_bytes(f["end"]-f["start"], "big")
        raw = raw[:f["start"]] + encoded + raw[f["end"]:]
    return raw


def check_edited(exe, work, mode, ordinal):
    """Verify untouched SM, then compare a combination of never-seen edit values against an independent byte oracle."""
    path = ROOT / "resources" / f"native-chart-{mode}.sm"
    raw = path.read_bytes()
    if sha(raw) != ORIGINALS[ordinal]:
        raise ValueError(f"unexpected factory {mode} fixture")
    fields = layout(exe,work,path)
    ksp = work / "model.ksp"
    source = work / "output.sm"
    run(exe,work,"--migrate-project",path,ksp)
    model = json.loads(ksp.read_bytes())
    obj, = model["pages"][0]["objects"]
    initial = deepcopy(model)
    if (obj["type"],obj["chart"]["shape"],obj["chart"][mode]["address"]) != ("chart",ordinal,"LW0"):
        raise ValueError("native monitor/shape was not exposed")
    if tuple(obj[k] for k in ("x","y","width","height")) != (160,120,200,160 if ordinal==0 else 200):
        raise ValueError("factory chart bounds were not restored")
    run(exe,work,"--export-native-project",ksp,source)
    if source.read_bytes() != raw:
        raise ValueError("unmodified factory source changed")
    fresh = deepcopy(model)
    for k in ("nativeSourceSnapshot","nativeSourceWarnings"):
        fresh.pop(k,None)
    put(ksp,fresh)
    run(exe,work,"--export-native-project",ksp,source)
    run(exe,work,"--migrate-project",source,ksp)
    authored, = json.loads(ksp.read_bytes())["pages"][0]["objects"]
    if any(obj[k] != authored[k] for k in ("chart","x","y","width","height","name")):
        raise ValueError("fresh chart failed to reopen")
    title = "图表原始字节 " + mode
    target = "LW" + str(127+ordinal)
    obj.update(x=173, y=140, subpixelX=0.5, subpixelY=0.5, name=title)
    obj["chart"][mode]["address"] = target
    mirror, register = (("2","3"),("2","34"),("1","10"))[ordinal]
    updates = {"group.metadata-name": title, f"private.{mirror}": target, f"private.{register}.0": target,
               f"private.{register}.4": target[2:]}
    for name, f in fields.items():
        if not name.startswith("object[0].child["):
            continue
        for suffixes, delta in zip(POSITION,(13.5,20.5)):
            if name.endswith(suffixes):
                updates[name.removeprefix("object[0].")] = struct.unpack(">d",raw[f["start"]:f["end"]])[0]+delta
    if mode == "bar":
        obj.update(width=184,height=142)
        obj["chart"]["bar"].update(foreground="#123456",background="#89abcd")
        updates.update({"child[0].rectangle.2":184,"child[0].rectangle.3":142,"child[1].rectangle.1":71,
                        "child[1].rectangle.2":184,"child[1].rectangle.3":142,"private.9":"18+52+86",
                        "private.10":"137+171+205","child[0].rectangle.6":"#89abcd","child[1].rectangle.6":"#123456"})
    if mode == "donut":
        obj.update(width=176,height=176)
        obj["chart"]["donut"].update(lineColor="#abcd12", minimum="9", maximum="345")
        updates.update({"child[0].ellipse.4":176,"child[0].ellipse.5":176,"child[0].ellipse.0":"#abcd12",
                        "private.9":"#abcd12","private.12":9.0,"private.13":345.0})
    updates = {"object[0]."+k:v for k,v in updates.items()}
    expected = make_patch(raw,fields,updates)
    put(ksp,model)
    run(exe,work,"--export-native-project",ksp,source)
    if source.read_bytes() != expected:
        raise ValueError(f"{mode} whole edited source differs from independent Qt4 byte patch")
    run(exe,work,"--migrate-project",source,ksp)
    found, = json.loads(ksp.read_bytes())["pages"][0]["objects"]
    if any(found[k] != obj[k] for k in ("chart","name","x","y","width","height","subpixelX","subpixelY")):
        raise ValueError("edited chart did not restore on the canvas")
    # A contradictory address copy must not be silently interpreted as the outer monitor.
    bad = work / "inconsistent.sm"
    bad.write_bytes(make_patch(raw,fields,{f"object[0].private.{register}.4":"12"}))
    untouched = work / "untouched.ksp"
    untouched.write_bytes(b"sentinel")
    run(exe,work,"--migrate-project",bad,untouched,success=False)
    if untouched.read_bytes() != b"sentinel":
        raise ValueError("failed import replaced the target")
    initial["pages"][0]["objects"][0]["chart"][mode]["dataType"] = 5
    put(ksp,initial)
    badout = work / "refused.sm"
    badout.write_bytes(b"sentinel")
    run(exe,work,"--export-native-project",ksp,badout,success=False)
    if badout.read_bytes() != b"sentinel":
        raise ValueError("unsupported export replaced the target")
    record = dict(mode=mode, nativeMode=(0,2,3)[ordinal], factorySha256=sha(raw), factoryBytes=len(raw),
                  templateDefaultExact=True, authoredDefaultReopened=True, fieldExpectations=len(updates),
                  editedExactSha256=sha(expected), editedBytes=len(expected), editedReopened=True,
                  unknownTypeAndConflictingAddressRejected=True, newTestValues=updates)
    return record, obj


def mixed(exe, work, objects):
    """Compare every host shell field while adding three newly authored charts to a separately sourced project."""
    blank = ROOT / "tests/fixtures/native-source/multipage/A-empty-one-page.sm"
    ksp = work / "host.ksp"
    output = work / "mixed.sm"
    run(exe,work,"--migrate-project",blank,ksp)
    host = json.loads(ksp.read_bytes())
    host.update(schemaVersion=12, animations={}, configurationNodes=[], macroIds={}, references=[],
                activePageId="page-1", startupPageId="page-1", startupLanguage="", projectId="project-1", pageOrder=["page-1"])
    host["pages"][0].update(id="page-1", kind="normal", width=800, height=480)
    for i,obj in enumerate(objects):
        obj["id"] = f"independent-chart-{i}"
    host["pages"][0]["objects"] = objects
    put(ksp,host)
    run(exe,work,"--export-native-project",ksp,output)
    run(exe,work,"--migrate-project",output,ksp)
    restored = json.loads(ksp.read_bytes())["pages"][0]["objects"]
    if [o["chart"] for o in restored] != [o["chart"] for o in objects]:
        raise ValueError("one of the three authored forms changed in a common host")
    f0,f1 = layout(exe,work,blank),layout(exe,work,output)
    b0,b1 = blank.read_bytes(),output.read_bytes()
    count = "page.objects.count"
    if b0[:f0[count]["start"]] != b1[:f1[count]["start"]] or b0[f0[count]["end"]:] != b1[f1["object[2].z"]["end"]:]:
        raise ValueError("donor project bytes leaked into the host header/footer")
    return dict(sha256=sha(b1), bytes=len(b1), headerAndFooterExact=True, objectsReopened=len(restored))


def main():
    """Run the three natural imports and save one auditable receipt outside the temporary workspace."""
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("exe",type=Path)
    parser.add_argument("--scratch",required=True,type=Path)
    parser.add_argument("--receipt",type=Path)
    args=parser.parse_args()
    exe=args.exe.resolve()
    rows,objects=[],[]
    for i,mode in enumerate(MODES):
        with TemporaryDirectory(dir=args.scratch) as temp:
            record,obj=check_edited(exe,Path(temp).resolve(),mode,i)
            rows.append(record)
            objects.append(obj)
    with TemporaryDirectory(dir=args.scratch) as temp:
        host=mixed(exe,Path(temp).resolve(),objects)
    receipt=dict(exeSha256=sha(exe.read_bytes()), references="e3b72657:docs/stage4-evidence/chart/native/",
                 sourceMode="ordinary V122 SM import/export", factoryReferences=rows, mixedHost=host)
    if args.receipt:
        args.receipt.write_text(json.dumps(receipt,ensure_ascii=False,indent=2)+"\n",encoding="utf8")
    print(json.dumps(dict(factoryExact=len(rows), editedExact=len(rows), authoredDefaultReopened=len(rows),
                          mixedReopened=host["objectsReopened"],protectiveRejections=2*len(rows))))


if __name__ == "__main__":
    main()
