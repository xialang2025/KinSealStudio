"""Bounded mathematical replay of observed non-antialiased paths, for diagnosis only.

The candidate tests 26.6 curve subdivision and 16.16 intersections against the
retained native output. Qt 4.6.3 rasterizer source is a separately retained input.
It is not wired to any product compiler.
"""
from pathlib import Path
import hashlib
import json
import math
import struct
import subprocess
import numpy as np

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
base=work/'docs/stage5-evidence/native/audit'

def split_cubic(points):
    p0,p1,p2,p3=points
    mid=lambda a,b:tuple(int((a[i]+b[i])/2) for i in (0,1))
    a,b,d=mid(p0,p1),mid(p1,p2),mid(p2,p3)
    e,f=mid(a,b),mid(b,d)
    center=mid(e,f)
    return (p0,a,e,center),(center,f,d,p3)

def flatten(points,depth=0):
    a,b,d,e=points
    dx,dy=e[0]-a[0],e[1]-a[1]
    length=abs(dx)+abs(dy)
    if length>64:
        distance=sum(abs((p[0]-a[0])*dy-(p[1]-a[1])*dx) for p in (b,d))
        flat=distance<=16*length
    else:
        flat=sum(abs(p[0]-a[0])+abs(p[1]-a[1]) for p in (b,d))<=16
    if flat or depth==32:return [(a,e)]
    left,right=split_cubic(points)
    return flatten(left,depth+1)+flatten(right,depth+1)

def decode(serialized):
    count=struct.unpack_from('>I',serialized)[0]
    return [struct.unpack_from('>Idd',serialized,4+i*20) for i in range(count)],struct.unpack_from('>II',serialized,4+count*20)

def fill(image,serialized,color,rounding='truncate',offset=(100,100)):
    elements,tail=decode(serialized)
    lines=[]
    previous=None
    start=None
    i=0
    transform=lambda p:tuple(int((v+offset[i])*64) if rounding=='truncate' else math.floor((v+offset[i])*64+.5) for i,v in enumerate(p))
    while i<len(elements):
        t,x,y=elements[i]
        target=transform((x,y))
        if t==0:
            if previous is not None and previous!=start:lines.append((previous,start))
            previous=start=target
        elif t==1:lines.append((previous,target));previous=target
        elif t==2:
            controls=[previous]+[transform(e[1:]) for e in elements[i:i+3]]
            lines+=flatten(controls);previous=controls[-1];i+=2
        else:raise AssertionError(t)
        i+=1
    if previous!=start:lines.append((previous,start))
    events=[[] for _ in range(480)]
    for a,b in lines:
        winding=1
        if a[1]>b[1]:a,b=b,a;winding=-1
        ax,ay=a[0]+32,a[1]+32
        bx,by=b[0]+32,b[1]+32
        top=max(0,(ay+31)>>6);bottom=min(479,(by-33)>>6)
        if top>bottom:continue
        slope=int((bx-ax)/(by-ay)*65536) if by!=ay else 0
        initial=32768+(ax<<10)-1+(slope*((top<<16)+32768-(ay<<10))>>16)
        for y in range(top,bottom+1):
            events[y].append(((initial+(y-top)*slope)>>16,winding))
    for y,row in enumerate(events):
        winding=0;last=0
        for x,delta in sorted(row):
            if winding if tail[1] else winding&1:
                image[y,max(0,last):min(800,x)]=color
            winding+=delta;last=x
    return dict(elements=len(elements),fillRule=tail[1],lines=len(lines))

def main():
    folder=base/'paint-roundrect-stroke-r3'
    record=json.loads((folder/'native-probe.json').read_text(encoding='utf-8'))
    trace=next(x['payload'] for x in record['instrumentationMessages'] if x.get('payload',{}).get('nativeStrokeStreamHex'))
    decoder=root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'
    native=root/'temp/scanline-roundrect.rgb565'
    subprocess.run([str(decoder),str(next((folder/'compiled').iterdir())/'SheetPixel.dat'),str(native),'768000'],check=True,capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW)
    expected_bytes=native.read_bytes();expected=np.frombuffer(expected_bytes,dtype='<u2').reshape(480,800)
    candidates=[]
    for rounding in ('truncate','nearest'):
        image=np.zeros((480,800),dtype='<u2');image[1:,1:]=0xc6ff
        details=[fill(image,bytes.fromhex(trace['pathStreamHex']),0xffff,rounding),fill(image,bytes.fromhex(trace['nativeStrokeStreamHex']),0,rounding)]
        indices=np.flatnonzero(image!=expected)
        candidates.append(dict(rounding=rounding,differentPixels=len(indices),paths=details,rawSha256=hashlib.sha256(image.tobytes()).hexdigest(),first=[dict(x=int(i%800),y=int(i//800),native=int(expected.flat[i]),candidate=int(image.flat[i])) for i in indices[:50]]))
    destination=base/'fixed-scanline-candidates-r1.json';assert not destination.exists()
    destination.write_text(json.dumps(dict(candidates=candidates,usbAccessed=False,productCompilerOpened=False,E=False),indent=2)+'\n',encoding='utf-8');native.unlink()
    print([(v['rounding'],v['differentPixels']) for v in candidates])

if __name__=='__main__':main()
