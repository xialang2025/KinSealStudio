"""Passively trace Qt painter calls in the owned native offline compile process."""
import native_probe

extension = r'''
//20261113-16 add: Serialize actual native paths instead of inferring them from bounding boxes.
const paintGetDevice=new NativeFunction(gui.getExportByName('_ZNK8QPainter6deviceEv'),'pointer',['pointer']);
const paintGetEngine=new NativeFunction(gui.getExportByName('_ZNK8QPainter11paintEngineEv'),'pointer',['pointer']);
const paintGetPen=new NativeFunction(gui.getExportByName('_ZNK8QPainter3penEv'),'pointer',['pointer']);
const paintGetTransform=new NativeFunction(gui.getExportByName('_ZNK8QPainter9transformEv'),'pointer',['pointer']);
const paintGetHints=new NativeFunction(gui.getExportByName('_ZNK8QPainter11renderHintsEv'),'void',['pointer','pointer']);
const paintPenWidth=new NativeFunction(gui.getExportByName('_ZNK4QPen6widthFEv'),'double',['pointer']);
const paintPenStyle=new NativeFunction(gui.getExportByName('_ZNK4QPen5styleEv'),'int',['pointer']);
const paintPenCap=new NativeFunction(gui.getExportByName('_ZNK4QPen8capStyleEv'),'int',['pointer']);
const paintPenJoin=new NativeFunction(gui.getExportByName('_ZNK4QPen9joinStyleEv'),'int',['pointer']);
const paintPenColor=new NativeFunction(gui.getExportByName('_ZNK4QPen5colorEv'),'void',['pointer','pointer']);
const paintRgba=new NativeFunction(gui.getExportByName('_ZNK6QColor4rgbaEv'),'uint',['pointer']);
const paintStreamCtor=new NativeFunction(core.getExportByName('_ZN11QDataStreamC1EP10QByteArrayi'),'void',['pointer','pointer','int']);
const paintStreamDtor=new NativeFunction(core.getExportByName('_ZN11QDataStreamD1Ev'),'void',['pointer']);
const paintArrayDtor=new NativeFunction(core.getExportByName('_ZN10QByteArrayD1Ev'),'void',['pointer']);
const paintPathWrite=new NativeFunction(gui.getExportByName('_ZlsR11QDataStreamRK12QPainterPath'),'pointer',['pointer','pointer']);
const paintStrokerCtor=new NativeFunction(gui.getExportByName('_ZN19QPainterPathStrokerC1Ev'),'void',['pointer']);
const paintStrokerDtor=new NativeFunction(gui.getExportByName('_ZN19QPainterPathStrokerD1Ev'),'void',['pointer']);
const paintStrokerWidth=new NativeFunction(gui.getExportByName('_ZN19QPainterPathStroker8setWidthEd'),'void',['pointer','double']);
const paintStrokerCap=new NativeFunction(gui.getExportByName('_ZN19QPainterPathStroker11setCapStyleEN2Qt11PenCapStyleE'),'void',['pointer','int']);
const paintStrokerJoin=new NativeFunction(gui.getExportByName('_ZN19QPainterPathStroker12setJoinStyleEN2Qt12PenJoinStyleE'),'void',['pointer','int']);
const paintStrokerCreate=new NativeFunction(gui.getExportByName('_ZNK19QPainterPathStroker12createStrokeERK12QPainterPath'),'void',['pointer','pointer','pointer']);
const paintPathDtor=new NativeFunction(gui.getExportByName('_ZN12QPainterPathD1Ev'),'void',['pointer']);
const paintSeen=new Set(); let paintCount=0;let engineLogged=false;
function paintPathHex(path){
    const a=Memory.alloc(4); a.writePointer(core.getExportByName('_ZN10QByteArray11shared_nullE'));
    const s=Memory.alloc(128);paintStreamCtor(s,a,2);
    try{paintPathWrite(s,path);const d=a.readPointer();const n=d.add(8).readInt();if(n<0||n>200000){throw Error('path stream bound '+n);}return Array.from(new Uint8Array(d.add(12).readPointer().readByteArray(n))).map(b=>b.toString(16).padStart(2,'0')).join('');}
    finally{paintStreamDtor(s);paintArrayDtor(a);}
}
function tracePaint(args,symbol){
    if(paintCount>=200){return;}
    try{
        const painter=args[0], device=paintGetDevice(painter);if(device.isNull()){return;}
        const virtualType=device.readPointer().add(8).readPointer();
        const deviceType=new NativeFunction(virtualType,'int',['pointer'])(device);
        if(deviceType!==2&&deviceType!==3){return;}
        const cls=deviceType===3?'QImage':'QPixmap';
        const width=new NativeFunction(gui.getExportByName('_ZNK'+(deviceType===3?'6QImage':'7QPixmap')+'5widthEv'),'int',['pointer'])(device);
        const height=new NativeFunction(gui.getExportByName('_ZNK'+(deviceType===3?'6QImage':'7QPixmap')+'6heightEv'),'int',['pointer'])(device);
        if(width!==800||height!==480){return;}
        if(!engineLogged){engineLogged=true;const engine=paintGetEngine(painter),vtable=engine.readPointer();send({tag:'stage5-native-engine',pointer:engine.toString(),vtable:vtable.toString(),virtuals:Array.from({length:26},(_,i)=>{const address=vtable.add(i*4).readPointer();let p=address;const instructions=[];try{for(let n=0;n<4;n++){const instruction=Instruction.parse(p);instructions.push(instruction.toString());p=instruction.next;if(instruction.mnemonic==='ret'){break;}}}catch(e){instructions.push(String(e));}return {index:i,address:address.toString(),moduleOffset:address.sub(gui.base).toString(),instructions};})});}
        const pen=paintGetPen(painter), color=Memory.alloc(32);paintPenColor(color,pen);
        const hints=Memory.alloc(4);paintGetHints(hints,painter);
        const t=paintGetTransform(painter);
        const result={tag:'stage5-native-paint',symbol,deviceType,deviceClass:cls,width,height,pen:{style:paintPenStyle(pen),width:paintPenWidth(pen),rgba:paintRgba(color),cap:paintPenCap(pen),join:paintPenJoin(pen)},hints:hints.readInt(),transform:[0,8,16,24,32,40,48,56,64].map(i=>t.add(i).readDouble())};
        if(deviceType===3){result.imageFormat=new NativeFunction(gui.getExportByName('_ZNK6QImage6formatEv'),'int',['pointer'])(device);}
        if(symbol.includes('drawPath')){result.pathStreamHex=paintPathHex(args[1]);
            if(result.pen.style===1){const stroker=Memory.alloc(64),strokePath=Memory.alloc(16);paintStrokerCtor(stroker);try{paintStrokerWidth(stroker,1);paintStrokerCap(stroker,result.pen.cap);paintStrokerJoin(stroker,result.pen.join);paintStrokerCreate(strokePath,stroker,args[1]);try{result.nativeStrokeStreamHex=paintPathHex(strokePath);}finally{paintPathDtor(strokePath);}}finally{paintStrokerDtor(stroker);}}
        }
        else if(symbol.includes('drawPolygon')){const count=args[2].toInt32();if(count<0||count>4096){throw Error('polygon bound '+count);}const floating=symbol.includes('QPointF');result.polygon={count,fillRule:args[3].toInt32(),points:Array.from({length:count},(_,i)=>floating?[args[1].add(i*16).readDouble(),args[1].add(i*16+8).readDouble()]:[args[1].add(i*8).readInt(),args[1].add(i*8+4).readInt()])};}
        else if(symbol.includes('QRectF')){result.rect=[0,8,16,24].map(i=>args[1].add(i).readDouble());}
        if(symbol.includes('drawRoundRect')){result.radii=[args[2].toInt32(),args[3].toInt32()];}
        const key=JSON.stringify(result);if(!paintSeen.has(key)){paintSeen.add(key);paintCount++;send(result);}
    }catch(error){if(paintCount++<4){send({tag:'stage5-native-paint-error',symbol,error:String(error)});}}
}
for(const symbol of ['_ZN8QPainter11drawPolygonEPK6QPointiN2Qt8FillRuleE','_ZN8QPainter11drawPolygonEPK7QPointFiN2Qt8FillRuleE','_ZN8QPainter8drawPathERK12QPainterPath','_ZN8QPainter11drawEllipseERK6QRectF','_ZN8QPainter13drawRoundRectERK6QRectFii','_ZN8QPainter15drawRoundedRectERK6QRectFddN2Qt8SizeModeE']){
    const address=gui.findExportByName(symbol);if(address!==null){Interceptor.attach(address,{onEnter(args){tracePaint(args,symbol);}});}
}
'''
native_probe.JS = native_probe.JS.replace('let pending=null;', extension + '\nlet pending=null;', 1)
native_probe.main()
