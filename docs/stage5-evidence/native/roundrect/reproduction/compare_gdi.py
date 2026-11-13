"""Render an observed path into an offscreen DIB without touching any desktop/window."""
from pathlib import Path
import ctypes as c
from ctypes import wintypes as w
import json
import math
import os
import struct
import subprocess
import numpy as np

root=Path(__file__).resolve().parent
work=root.parent/'kinseal-stage4-20261112/worktree'
base=work/'docs/stage5-evidence/native/audit'
gdi=c.WinDLL('gdi32',use_last_error=True)
class BitmapHeader(c.Structure):
    _fields_=[('size',w.DWORD),('width',w.LONG),('height',w.LONG),('planes',w.WORD),('bitCount',w.WORD),('compression',w.DWORD),('sizeImage',w.DWORD),('xPelsPerMeter',w.LONG),('yPelsPerMeter',w.LONG),('clrUsed',w.DWORD),('clrImportant',w.DWORD)]
class BitmapInfo(c.Structure):
    _fields_=[('header',BitmapHeader),('colors',w.DWORD*1)]
gdi.CreateCompatibleDC.argtypes=[w.HDC];gdi.CreateCompatibleDC.restype=w.HDC
gdi.CreateDIBSection.argtypes=[w.HDC,c.POINTER(BitmapInfo),w.UINT,c.POINTER(c.c_void_p),w.HANDLE,w.DWORD];gdi.CreateDIBSection.restype=w.HBITMAP
gdi.SelectObject.argtypes=[w.HDC,w.HANDLE];gdi.SelectObject.restype=w.HANDLE
gdi.CreatePen.argtypes=[c.c_int,c.c_int,w.COLORREF];gdi.CreatePen.restype=w.HPEN
gdi.CreateSolidBrush.argtypes=[w.COLORREF];gdi.CreateSolidBrush.restype=w.HBRUSH
gdi.MoveToEx.argtypes=[w.HDC,c.c_int,c.c_int,c.c_void_p]
gdi.LineTo.argtypes=[w.HDC,c.c_int,c.c_int]
gdi.PolyBezierTo.argtypes=[w.HDC,c.POINTER(w.POINT),w.DWORD]
gdi.DeleteObject.argtypes=[w.HANDLE]
gdi.DeleteDC.argtypes=[w.HDC]
for name in ('BeginPath','EndPath','FillPath','StrokePath','StrokeAndFillPath','CloseFigure'):
    getattr(gdi,name).argtypes=[w.HDC]
gdi.SetPolyFillMode.argtypes=[w.HDC,c.c_int]
records=[]
for family in ('roundrect','sector'):
    trace=json.loads((base/'path-replay-candidates-r1.json').read_text())['cases'][0 if family=='roundrect' else 1]
    elements=trace['elements']
    folder=next((base/('paint-'+family+'-r1/compiled')).iterdir())
    native=root/('temp/gdi-'+family+'.rgb565')
    decoder=root.parents[1]/'KinSealStudio/docs/stage3-evidence/native/tools/lzo_decode.exe'
    subprocess.run([str(decoder),str(folder/'SheetPixel.dat'),str(native),'768000'],check=True,capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW)
    expected=np.frombuffer(native.read_bytes(),dtype='<u2').reshape(480,800)
    item=dict(family=family,candidates=[])
    for rounding in ('nearest','truncate'):
      for operation in ('joint','separate'):
        dc=gdi.CreateCompatibleDC(None);assert dc
        info=BitmapInfo();info.header=BitmapHeader(c.sizeof(BitmapHeader),800,-480,1,32,0,800*480*4,0,0,0,0)
        bits=c.c_void_p();bitmap=gdi.CreateDIBSection(dc,c.byref(info),0,c.byref(bits),None,0);assert bitmap and bits.value
        oldBitmap=gdi.SelectObject(dc,bitmap)
        pen=gdi.CreatePen(0,0,0);brush=gdi.CreateSolidBrush(0xffffff)
        oldPen=gdi.SelectObject(dc,pen);oldBrush=gdi.SelectObject(dc,brush)
        array=np.ctypeslib.as_array((c.c_uint32*(800*480)).from_address(bits.value)).reshape(480,800)
        array[:]=0;array[1:,1:]=0xffc5dfff
        roundingFn=lambda x:math.floor(x+.5) if rounding=='nearest' else int(x)
        def makePath():
            assert gdi.BeginPath(dc)
            index=0
            while index<len(elements):
                element=elements[index];x=roundingFn(element['x']+100);y=roundingFn(element['y']+100)
                if element['type']==0:gdi.MoveToEx(dc,x,y,None)
                elif element['type']==1:gdi.LineTo(dc,x,y)
                elif element['type']==2:
                    points=(w.POINT*3)(*[w.POINT(roundingFn(e['x']+100),roundingFn(e['y']+100)) for e in elements[index:index+3]])
                    assert gdi.PolyBezierTo(dc,points,3);index+=2
                else:raise AssertionError(element)
                index+=1
            assert gdi.EndPath(dc)
        makePath()
        if family=='sector':assert gdi.FillPath(dc)
        elif operation=='joint':assert gdi.StrokeAndFillPath(dc)
        else:
            assert gdi.FillPath(dc);makePath();assert gdi.StrokePath(dc)
        gdi.GdiFlush()
        values=array.copy()
        actual=(((values>>19)&31)<<11)|(((values>>10)&63)<<5)|((values>>3)&31)
        indices=np.flatnonzero(actual!=expected)
        item['candidates'].append(dict(rounding=rounding,operation=operation,differentPixels=len(indices),first=[dict(x=int(i%800),y=int(i//800),native=int(expected.flat[i]),candidate=int(actual.flat[i])) for i in indices[:10]]))
        gdi.SelectObject(dc,oldBrush);gdi.SelectObject(dc,oldPen);gdi.SelectObject(dc,oldBitmap)
        gdi.DeleteObject(brush);gdi.DeleteObject(pen);gdi.DeleteObject(bitmap);gdi.DeleteDC(dc)
    native.unlink();records.append(item)
    print(family,[(v['rounding'],v['operation'],v['differentPixels']) for v in item['candidates']],flush=True)
destination=base/'gdi-path-candidates-r1.json';assert not destination.exists()
destination.write_text(json.dumps(dict(cases=records,offscreenOnly=True,usbAccessed=False,productCompilerOpened=False,E=False),indent=2)+'\n',encoding='utf-8')
