"""Rebuild verified V122 chart DAT members from original SM fields and check two independent V5.1 compile contexts."""
# 20261128-32 add
import argparse
import json
import os
from pathlib import Path
import struct
import subprocess
from tempfile import TemporaryDirectory

from verify_amz_local_page_io import layout, sha
from verify_native_chart_source import MODES, ORIGINALS
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"tools"))
from export_native_chart_members import export
from verify_native_drawing_drafts import ROOT, REVISION, git_blob, run

BASE = "docs/stage4-evidence/chart"
ROUTES = ("--compile-project", "--compile-native-media-parts")
MEMBERS = ("AddrPara", "AlarmPara", "Collect", "ControlPara", "ControlPixel", "Curve", "PicLenPara",
           "SheetPara", "SheetPixel", "SysPara")


def product_entry(exe, work, name, sm, route):
    """Measure normal compile behavior separately from the research reconstruction."""
    out = work / (name+route)
    proc = subprocess.run([str(exe),route,str(sm),str(out)],capture_output=True,cwd=work,timeout=90,
                          env=dict(os.environ,QT_QPA_PLATFORM="offscreen",TMP=str(work),TEMP=str(work)),
                          creationflags=getattr(subprocess,"CREATE_NO_WINDOW",0))
    return dict(status=proc.returncode, response=json.loads(proc.stdout.decode()),
                dat={p.name:sha(p.read_bytes()) for p in sorted(out.glob("*.dat"))})


def compare(exe, work, mode, index):
    """Predict complete file bodies and selected control record fields before reading any original compiled DAT."""
    path = ROOT / "resources" / f"native-chart-{mode}.sm"
    source = path.read_bytes()
    if sha(source) != ORIGINALS[index]:
        raise ValueError("unsupported original chart revision")
    fields = layout(exe,work,path)
    def field(name):
        f=fields[name]
        b=source[f["start"]:f["end"]]
        if f["kind"]=="q": return b[4:].decode("utf-16-be")
        if f["kind"]=="d": return struct.unpack(">d",b)[0]
        if f["kind"]=="c":
            spec,alpha,r,g,blue,pad=struct.unpack(">BHHHHH",b)
            if (spec,alpha,pad)!=(1,65535,0): raise ValueError("unexpected Qt4 QColor")
            return f"#{r>>8:02x}{g>>8:02x}{blue>>8:02x}"
        return int.from_bytes(b,"big")
    ksp=work/"chart.ksp"
    run(exe,work,"--migrate-project",path,ksp)
    model=json.loads(ksp.read_bytes())
    obj,=model["pages"][0]["objects"]
    prop=obj["chart"][mode]
    if not prop["address"].startswith("LW") or model.get("configurationNodes") or model.get("connections"):
        raise ValueError("input is not a single local chart")
    sw,sh=int(field("page.head.2")),int(field("page.head.3"))
    px,py,w,h=[obj[key] for key in ("x","y","width","height")]
    if mode=="donut":
        # The visible pen expands the graphics bounds by one pixel on each edge in the original 640-byte record.
        px-=1; py-=1; w+=2; h+=2
    pages=field("page.objects.count")
    frames=2 if mode=="bar" else 1
    raw_pixels=w*h*2*frames
    out=work/"implemented-parts"
    export(exe,path,out)
    predicted={p.stem:p.read_bytes() for p in out.glob("*.dat")}
    expected_names={"AddrPara","Collect","Curve","SheetPara","SheetPixel"}
    if mode=="bar": expected_names.update(("ControlPixel","PicLenPara"))
    if set(predicted)!=expected_names: raise ValueError("unexpected published members")
    private=lambda i:field(f"object[0].private.{i}")
    checks={0:struct.pack("<H",field("object[0].type")),4:struct.pack("<HHHH",px,py,w,h),16:struct.pack("<I",raw_pixels),
            256:struct.pack("<I",field("object[0].private.mode"))}
    colors=lambda v:bytes.fromhex(v.lstrip("#"))+bytes(1)
    if mode=="bar":
        checks.update({0x106:struct.pack("<H",private(6)),0x10a:struct.pack("<H",private(8)),
                       0x114:struct.pack("<dd",float(prop["minimum"]),float(prop["maximum"]))})
    elif mode=="meter":
        checks.update({0x104:struct.pack("<H",private(3)),0x108:struct.pack("<I",private(1)),
                       0x10c:b"".join(colors(prop[k]) for k in ("foreground","background","scaleColor")),
                       0x118:struct.pack("<II",prop["pointer"],prop["shape"]),
                       0x120:struct.pack("<dd",float(prop["minimum"]),float(prop["maximum"]))})
    else:
        checks.update({0x108:struct.pack("<I",int(prop["opacity"])),0x118:struct.pack("<I",int(prop["spanAngle"])),
                       0x11c:colors(prop["lineColor"]),0x120:struct.pack("<I",int(prop["startAngle"])),
                       0x128:struct.pack("<dd",float(prop["minimum"]),float(prop["maximum"]))})
    routes={route:product_entry(exe,work,mode,path,route) for route in ROUTES}
    # Only now load the paired V5.1 captures. The physical build refers to the identical original source bytes.
    physical=f"{BASE}/physical/chart-{mode}-baseline-r1/compiled/physical-native-reference"
    if git_blob(physical,"input-native.sm")!=source: raise ValueError("physical run is not from this source")
    original=f"{BASE}/native/compiled/{mode}-default-"+("r7" if mode=="meter" else "r4")
    if git_blob(f"{BASE}/native",mode+"-default.sm")!=source: raise ValueError("creation source hash mismatch")
    refs={}
    for context in (original,physical):
        chunks={k:git_blob(context,k+".dat") for k in MEMBERS}
        for k,data in predicted.items():
            if data!=chunks[k]: raise ValueError(f"{mode} {context} {k}: {len(data)} predicted, {len(chunks[k])} observed")
        record=chunks["ControlPara"]
        if len(record)!=640: raise ValueError("unexpected device record size")
        for off,val in checks.items():
            if record[off:off+len(val)]!=val: raise ValueError(f"{mode} ControlPara +{off:#x}")
        refs[context]={key:sha(value) for key,value in chunks.items()}
    return dict(shape=mode, smSha256=sha(source), productEntries=routes,
                generatedWholeMembers={k+".dat":dict(bytes=len(v),sha256=sha(v)) for k,v in predicted.items()},
                controlRecordChecks={f"0x{k:03x}":v.hex() for k,v in checks.items()},
                controlRecordBytesChecked=sum(map(len,checks.values())), originalArchives=refs)


def changed_values(exe,work,mode,index):
    """Verify SM and KSP candidates, changed page geometry, refused settings, and clip/collision boundaries."""
    source=ROOT/"resources"/f"native-chart-{mode}.sm"
    ksp=work/"candidate.ksp"
    sm=work/"candidate.sm"
    run(exe,work,"--migrate-project",source,ksp)
    model=json.loads(ksp.read_bytes())
    obj=model["pages"][0]["objects"][0]
    obj.update(x=146,y=127,name="变更后图表")
    obj["chart"][mode]["address"]="LW"+str(700+index)
    if mode=="bar":
        obj.update(width=215,height=147)
        obj["chart"][mode].update(background="#ddccbb",foreground="#123abc")
    elif mode=="donut":
        obj.update(width=168,height=168)
        obj["chart"][mode].update(lineColor="#2981bc",minimum="12",maximum="399")
    ksp.write_text(json.dumps(model,ensure_ascii=False),encoding="utf8")
    run(exe,work,"--export-native-project",ksp,sm)
    one,two=work/"from-ksp",work/"from-sm"
    a=export(exe,ksp,one);b=export(exe,sm,two)
    if a!=b: raise ValueError("SM and KSP member bytes differ")
    raw_count,=struct.unpack_from("<I",(one/"SheetPara.dat").read_bytes(),68)
    width,height=obj["width"],obj["height"]
    expected=(width+2)*(height+2)*2 if mode=="donut" else width*height*(4 if mode=="bar" else 2)
    if raw_count!=expected: raise ValueError("edited sheet does not encode the new raw size")
    checks=[]
    if mode=="bar":
        # The production value-display path supplies the same compressor independently for arbitrary sprite sizes.
        num=work/"number.ksp"
        run(exe,work,"--migrate-project",ROOT/"resources/native-data-number.sm",num)
        test=json.loads(num.read_bytes())
        test.pop("nativeSourceSnapshot",None);test.pop("nativeSourceWarnings",None)
        item=test["pages"][0]["objects"][0]
        item.update(x=0,y=0,width=width,height=height)
        item["numberDisplay"].update(backgroundColor="#ddccbb", highColor="#123abc")
        num.write_text(json.dumps(test,ensure_ascii=False),encoding="utf8")
        pack=work/"cross-codec"
        run(exe,work,"--compile-native-value-display-parts",num,pack)
        def prefixes(path):
            lens=(path/"PicLenPara.dat").read_bytes();data=(path/"ControlPixel.dat").read_bytes()
            start=0;result=[]
            for tag,align,size in struct.iter_unpack("<III",lens[2400:]):
                if tag: raise ValueError("unknown frame compression")
                result.append(data[start:start+size]);start+=align
            return result
        sprite=prefixes(one)
        if sprite!=prefixes(pack)[:2]: raise ValueError("both edited sprites must match the independent C++ compressor")
        checks=[sha(b) for b in sprite]
    denied=False
    try: export(exe,ksp,one)
    except ValueError: denied=True
    if not denied or a["members"]!={f.name:sha(f.read_bytes()) for f in one.iterdir()}:
        raise ValueError("existing destination was overwritten")
    unknown=work/"invalid.sm"
    raw=sm.read_bytes();spans=layout(exe,work,sm)
    key="page.tail.2"
    entry=spans[key]
    unknown.write_bytes(raw[:entry["start"]]+struct.pack(">I",71)+raw[entry["end"]:])
    guard=work/"unknown"
    try: export(exe,unknown,guard)
    except ValueError: pass
    else: raise ValueError("unknown source configuration was accepted")
    if guard.exists(): raise ValueError("failed source published partial members")
    return dict(sourceSha256=sha(sm.read_bytes()),smAndKspExact=True,rawBytes=raw_count,
                completeFiles=a["members"],barColorFramesAgainstIndependentCpp=checks,
                existingDirectoryPreserved=True,unverifiedPageAttributeRejected=True)


def solid_boundaries(exe, work):
    """Compare the narrow sprite encoder with the product C++ compression routine across its native block boundary."""
    from export_native_chart_members import solid_sprite, rgb565
    source = work / "number.ksp"
    run(exe, work, "--migrate-project", ROOT/"resources/native-data-number.sm", source)
    model = json.loads(source.read_bytes())
    model.pop("nativeSourceSnapshot", None)
    model.pop("nativeSourceWarnings", None)
    item = model["pages"][0]["objects"][0]
    colors = ("#000000", "#139ac1", "#ffffff")
    item["numberDisplay"].update(backgroundColor=colors[0], highColor=colors[1], lowColor=colors[2])
    rows = []
    for index,(width,height) in enumerate(((5,3), (17,7), (192,127), (192,128), (192,129), (460,302))):
        out = work / f"cpp-{index}"
        item.update(x=0, y=0, width=width, height=height)
        source.write_text(json.dumps(model, ensure_ascii=False), encoding="utf8")
        run(exe, work, "--compile-native-value-display-parts", source, out)
        lengths = (out/"PicLenPara.dat").read_bytes()
        data = (out/"ControlPixel.dat").read_bytes()
        offset = 0
        records = list(struct.iter_unpack("<III", lengths[2400:]))
        if len(records)!=len(colors): raise ValueError("unsupported numeric test fixture")
        for color,(kind,stored,actual) in zip(colors,records):
            if kind or solid_sprite(rgb565(color),width*height)!=data[offset:offset+actual]:
                raise ValueError(f"solid sprite differs at {width}x{height}: {color}")
            offset += stored
        rows.append(dict(width=width, height=height, rgb565Bytes=width*height*2, confirmedColors=list(colors)))
    return rows


def main():
    """Generate deterministic members on a temporary volume, compare independent archives and retain only the receipt."""
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("exe",type=Path);p.add_argument("--scratch",type=Path,required=True);p.add_argument("--receipt",type=Path)
    args=p.parse_args();exe=args.exe.resolve()
    rows=[]
    for i,name in enumerate(MODES):
        with TemporaryDirectory(dir=args.scratch,prefix="chart-members-") as tmp:
            row=compare(exe,Path(tmp).resolve(),name,i)
            row["newValues"]=changed_values(exe,Path(tmp).resolve(),name,i)
            rows.append(row)
    with TemporaryDirectory(dir=args.scratch,prefix="chart-codec-") as tmp:
        boundaries=solid_boundaries(exe,Path(tmp).resolve())
    report=dict(solidSpriteCppBoundaryCases=boundaries,revision=REVISION, exeSha256=sha(exe.read_bytes()), tool="independent reconstructed member comparisons",
                sameSourceReferenceContexts=2, shapes=rows)
    if args.receipt:
        args.receipt.write_text(json.dumps(report,ensure_ascii=False,indent=2)+"\n",encoding="utf8")
    print(json.dumps({r["shape"]:{"whole":len(r["generatedWholeMembers"]),"control640":r["controlRecordBytesChecked"],
                                "normalOutputFiles":sum(len(v["dat"]) for v in r["productEntries"].values())} for r in rows}))


if __name__=="__main__":
    main()
