"""Write the verified DAT members for one ordinary local V122 chart; leaves incomplete device packages unassembled."""
# 20261128-36 add
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
from tempfile import TemporaryDirectory

ROOT=Path(__file__).resolve().parents[1]
MODES=("bar","meter","donut")
SOURCE_HASHES=("1cc6b375830e6fea36f1c74110f38314e9c96e929a31f660ff80bbda9bc1d23e",
               "3b7809752260cb2cd775cf800031b6c06262d0609237e80e7269b12d8d78ede1",
               "f3e3294fafe3f77b80ca10bc7aa86b151f335e0fe07e812e5ef6c5cb2c5bd399")


def run(exe,work,option,*args):
    """Run the normal offscreen file operation with all scratch paths under the caller's private directory."""
    result=subprocess.run([str(exe),option,*map(str,args)],cwd=work,env=dict(os.environ,QT_QPA_PLATFORM="offscreen",
                          TEMP=str(work),TMP=str(work),TMPDIR=str(work)),capture_output=True,timeout=90,
                          creationflags=getattr(subprocess,"CREATE_NO_WINDOW",0))
    if result.returncode:
        raise ValueError(f"{option} returned {result.returncode}: {result.stdout.decode('utf-8','replace').strip()}")


def rgb565(color):
    """Convert opaque HTML sRGB to the little-endian RGB565 word used on the panel."""
    r,g,b=bytes.fromhex(color.lstrip("#"))
    return struct.pack("<H",(r>>3)<<11|(g>>2)<<5|b>>3)


def solid_sprite(pixel,count):
    """Specialize the V122 49152-byte LZO blocks for one repeating RGB565 word as used by a solid bar."""
    if not 1<=count<=800*480 or len(pixel)!=2:
        raise ValueError("invalid solid RGB565 sprite")
    raw=pixel*count
    out=bytearray()
    def length(n):
        zeros,remainder=divmod(n-1,255)
        out.extend(bytes(zeros));out.append(remainder+1)
    def literals(start,end,final=False):
        size=end-start
        if not size: return
        if final and not out and size<=238: out.append(17+size)
        elif size<=3: out[-2]|=size
        elif size<=18: out.append(size-3)
        else: out.append(0);length(size-18)
        out.extend(raw[start:end])
    start=block=0
    while len(raw)-block>20:
        end=min(block+49152,len(raw))
        if end-start<32: break
        limit=end-20
        pos=max(block,start+4)+1
        table={}
        while pos<limit:
            word=int.from_bytes(raw[pos:pos+4],"little")
            slot=((word*0x1824429d)&0xffffffff)>>18
            previous=table.get(slot,block)
            table[slot]=pos
            if raw[previous:previous+4]!=raw[pos:pos+4]:
                pos+=1
                continue
            # Equal four-byte substrings in a constant 16-bit fill match through the encoder's final lookahead window.
            match=12+8*max(0,(limit-pos-12+7)//8)
            literals(start,pos)
            distance=pos-previous-1
            if not 0<=distance<16384: raise ValueError("unexpected solid-fill match distance")
            if match<=33: out.append(32|(match-2))
            else: out.append(32);length(match-33)
            out.extend(((distance<<2)&255,distance>>6))
            start=pos+match
            pos=start
        block=end
    literals(start,len(raw),True)
    return bytes(out)+b"\x11\0\0"


def inspect(exe,work,sm):
    """Read typed primitive spans from this exact source, preserving the original byte order and string encodings."""
    path=work/"inspection.json"
    run(exe,work,"--inspect-native-source",sm,path)
    data=sm.read_bytes()
    doc=json.loads(path.read_bytes())
    if hashlib.sha256(data).hexdigest()!=doc["sha256"]:
        raise ValueError("source changed during inspection")
    spans={f["path"]:f for f in doc["fields"]}
    def value(name):
        f=spans[name];b=data[f["start"]:f["end"]]
        if f["kind"]=="q": return b[4:].decode("utf-16-be")
        if f["kind"]=="d": return struct.unpack(">d",b)[0]
        if f["kind"]=="c":
            spec,alpha,r,g,blue,pad=struct.unpack(">BHHHHH",b)
            if (spec,alpha,pad)!=(1,65535,0): raise ValueError("unsupported page color encoding")
            return f"#{r>>8:02x}{g>>8:02x}{blue>>8:02x}"
        return int.from_bytes(b,"big")
    return spans,data,value


def derive(exe,work,input_file):
    """Generate only the verified members for one chart on a factory-equivalent source configuration."""
    sm=work/"source.sm"
    if input_file.suffix.lower()==".ksp":
        run(exe,work,"--export-native-project",input_file,sm)
    elif input_file.suffix.lower()==".sm":
        # The model and field inspector must see the same source even if the editor saves again during export.
        sm.write_bytes(input_file.read_bytes())
    else:
        raise ValueError("input must be a V122 .sm or .ksp")
    ksp=work/"model.ksp"
    run(exe,work,"--migrate-project",sm,ksp)
    model=json.loads(ksp.read_bytes())
    pages=model["pages"]
    if len(pages)!=1 or len(pages[0]["objects"])!=1 or model.get("configurationNodes") or model.get("connections") or model.get("macros"):
        raise ValueError("requires a single chart and no other active project definitions")
    obj=pages[0]["objects"][0]
    if obj["type"]!="chart" or obj.get("subpixelX") or obj.get("subpixelY"):
        raise ValueError("requires an integer-position chart")
    shape=obj["chart"]["shape"]
    if not 0<=shape<3: raise ValueError("unsupported chart mode")
    mode=MODES[shape]
    prop=obj["chart"][mode]
    spans,raw,field=inspect(exe,work,sm)
    ref=ROOT/"resources"/f"native-chart-{mode}.sm"
    frozen,base,_=inspect(exe,work,ref)
    if hashlib.sha256(base).hexdigest()!=SOURCE_HASHES[shape]:
        raise ValueError("chart profile belongs to a different factory source")
    # No unknown project/system/page flag may silently change one of the generated constant tables.
    def active_project(name):
        return not name.startswith("object[") and name not in ("page.head.18","page.tail.9","page.tail.10","page.tail.11","page.tail.12",
               "config.head.4","config.head.11","trailer.settings.0","trailer.settings.1","trailer.settings.11")
    if {k for k in spans if active_project(k)}!={k for k in frozen if active_project(k)}:
        raise ValueError("project has unverified page/configuration sections")
    for key,f in spans.items():
        if active_project(key):
            old=frozen[key]
            if raw[f["start"]:f["end"]]!=base[old["start"]:old["end"]]:
                raise ValueError("unsupported chart project setting: "+key)
    w,h=obj["width"],obj["height"]
    x,y=obj["x"],obj["y"]
    if mode=="donut": x-=1;y-=1;w+=2;h+=2
    sw,sh=int(field("page.head.2")),int(field("page.head.3"))
    if (sw,sh)!=(800,480) or min(x,y)<0 or x+w>sw or y+h>sh:
        raise ValueError("chart bounds are clipped")
    frame=work/"background.rgb565";packed=work/"background.lzo"
    # The native page leaves its first scanline and the first column of the remaining scanlines black.
    row=bytes(2)+rgb565(field("page.head.4"))*(sw-1)
    frame.write_bytes(bytes(2*sw)+row*(sh-1))
    run(exe,work,"--compress-rgb565-frame",frame,packed)
    bg=packed.read_bytes()
    count=2 if shape==0 else 1
    sheet=bytearray(72)
    sheet[1]=field("page.tail.13");sheet[2]=field("page.objects.count")
    title=field("page.head.18").encode("gbk")[:40]
    sheet[7:7+len(title)]=title
    struct.pack_into("<HH",sheet,48,sw,sh);struct.pack_into("<H",sheet,56,field("page.tail.2"))
    sheet[60:68]=bytes([255])*8;struct.pack_into("<I",sheet,68,w*h*2*count)
    parts={"AddrPara.dat":bytes(200),"Collect.dat":bytes(1424),"Curve.dat":bytes(501),"SheetPara.dat":bytes(sheet),"SheetPixel.dat":bg}
    if shape==0:
        workbuf=bytearray(bg);pixels=bytearray();lens=bytearray(2400)
        for key in ("background","foreground"):
            data=solid_sprite(rgb565(prop[key]),w*h)
            aligned=(len(data)+31)&~31
            if len(workbuf)<aligned: workbuf.extend(bytes(aligned-len(workbuf)))
            workbuf[:len(data)]=data
            pixels.extend(workbuf[:aligned]);lens.extend(struct.pack("<III",0,aligned,len(data)))
        for off,val in ((0,len(bg)),(800,len(pixels)),(1600,count)): struct.pack_into("<I",lens,off,val)
        parts.update({"PicLenPara.dat":bytes(lens),"ControlPixel.dat":bytes(pixels)})
    return parts


def export(exe,input_file,destination):
    """Stage the entire set beside the requested destination. Publish by rename, rejecting an existing entry."""
    destination=destination.absolute()
    if destination.exists() or destination.is_symlink() or not destination.parent.is_dir():
        raise ValueError("destination must be a new directory under an existing parent")
    with TemporaryDirectory(prefix=".chart-members-",dir=destination.parent) as tmp:
        w=Path(tmp)
        parts=derive(exe.resolve(),w,input_file.resolve())
        stage=w/"parts";stage.mkdir()
        for k,b in parts.items(): (stage/k).write_bytes(b)
        if os.name == "nt":
            # MoveFileEx without REPLACE_EXISTING fails atomically if another writer already published its folder.
            stage.rename(destination)
        else:
            # POSIX rename can replace an existing empty directory. Reserve a new one exclusively before publication.
            destination.mkdir()
            try:
                stage.rename(destination)
            except OSError:
                destination.rmdir()
                raise
    return dict(members={k:hashlib.sha256(v).hexdigest() for k,v in parts.items()},fullPackage=False)


def main():
    """Invoke from the project tree with the built Lite EXE, source and a new destination directory."""
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("exe",type=Path);p.add_argument("source",type=Path);p.add_argument("destination",type=Path)
    a=p.parse_args()
    try: print(json.dumps(export(a.exe,a.source,a.destination)))
    except (ValueError,OSError,KeyError,subprocess.TimeoutExpired) as e:
        print(str(e),file=sys.stderr);return 1
    return 0


if __name__=="__main__":
    raise SystemExit(main())
