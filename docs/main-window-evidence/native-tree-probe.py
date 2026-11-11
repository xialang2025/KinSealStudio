"""#20261111-0 add: Read the original main-window tree, status fields and geometry on an owned private desktop."""
from pathlib import Path
import argparse
import ctypes
import hashlib
import json
import os
import shutil
import sys
import time
import winreg
import pywintypes
import win32con
import win32api
import win32gui
import win32process
import win32ui
from PIL import Image

sys.dont_write_bytecode = True
sys.stdout.reconfigure(encoding="utf-8")
sys.path.insert(0, r"G:\01_Professional\Frida\Installed")
import frida

ROOT = Path(r"Z:\Project\vscode_prj_temp\REA")
VENDOR = Path(r"G:\01_Professional\触摸屏\金玺\KinSealStudio_Setup_V5.1\Installed")
OUT = ROOT / "KinSealStudio/docs/main-window-evidence"
WORK = ROOT / ".scratch/kinseal-stage1-20261110/native/runtime-r4"
DESKTOP = "KinSealNativeShellTree20261111"

JS = r"""
//20261110-5 add: Every Qt call below runs from the owned GUI-thread event hook.
const gui=Process.getModuleByName('QtGui4.dll');const core=Process.getModuleByName('QtCore4.dll');
const find=new NativeFunction(gui.getExportByName('_ZN7QWidget4findEP6HWND__'),'pointer',['pointer']);
const actionsFn=new NativeFunction(gui.getExportByName('_ZNK7QWidget7actionsEv'),'void',['pointer','pointer']);
const textFn=new NativeFunction(gui.getExportByName('_ZNK7QAction4textEv'),'void',['pointer','pointer']);
const menuFn=new NativeFunction(gui.getExportByName('_ZNK7QAction4menuEv'),'pointer',['pointer']);
const enabledFn=new NativeFunction(gui.getExportByName('_ZNK7QAction9isEnabledEv'),'bool',['pointer']);
const propFn=new NativeFunction(core.getExportByName('_ZNK7QObject8propertyEPKc'),'void',['pointer','pointer','pointer']);
const vs=new NativeFunction(core.getExportByName('_ZNK8QVariant8toStringEv'),'void',['pointer','pointer']);
const vb=new NativeFunction(core.getExportByName('_ZNK8QVariant6toBoolEv'),'bool',['pointer']);
const vd=new NativeFunction(core.getExportByName('_ZN8QVariantD1Ev'),'void',['pointer']);
const sd=new NativeFunction(core.getExportByName('_ZN7QStringD1Ev'),'void',['pointer']);
const sc=new NativeFunction(core.getExportByName('_ZN7QStringC1EPK5QChari'),'void',['pointer','pointer','int']);
const invokeFn=new NativeFunction(core.getExportByName('_ZN11QMetaObject12invokeMethodEP7QObjectPKcN2Qt14ConnectionTypeE22QGenericReturnArgument16QGenericArgumentS7_S7_S7_S7_S7_S7_S7_S7_S7_'),'bool',['pointer','pointer','int',...Array(22).fill('pointer')]);
const setTextFn=new NativeFunction(gui.getExportByName('_ZN9QLineEdit7setTextERK7QString'),'void',['pointer','pointer']);
const spinFn=new NativeFunction(gui.getExportByName('_ZN8QSpinBox8setValueEi'),'void',['pointer','int']);
const comboSetFn=new NativeFunction(gui.getExportByName('_ZN9QComboBox15setCurrentIndexEi'),'void',['pointer','int']);
const comboSignalFn=new NativeFunction(gui.getExportByName('_ZN9QComboBox9activatedEi'),'void',['pointer','int']);
const comboTextFn=new NativeFunction(gui.getExportByName('_ZNK9QComboBox8itemTextEi'),'void',['pointer','pointer','int']);
const tabSetFn=new NativeFunction(gui.getExportByName('_ZN10QTabWidget15setCurrentIndexEi'),'void',['pointer','int']);
const groupSetFn=new NativeFunction(gui.getExportByName('_ZN9QGroupBox10setCheckedEb'),'void',['pointer','bool']);
const groupClickedFn=new NativeFunction(gui.getExportByName('_ZN9QGroupBox7clickedEb'),'void',['pointer','bool']);
const tableCurrentFn=new NativeFunction(gui.getExportByName('_ZN12QTableWidget14setCurrentCellEii'),'void',['pointer','int','int']);
const tableRowYFn=new NativeFunction(gui.getExportByName('_ZNK10QTableView19rowViewportPositionEi'),'int',['pointer','int']);
const tableRowHeightFn=new NativeFunction(gui.getExportByName('_ZNK10QTableView9rowHeightEi'),'int',['pointer','int']);
const tableColumnXFn=new NativeFunction(gui.getExportByName('_ZNK10QTableView22columnViewportPositionEi'),'int',['pointer','int']);
const tableColumnWidthFn=new NativeFunction(gui.getExportByName('_ZNK10QTableView11columnWidthEi'),'int',['pointer','int']);
const tableItem=new NativeFunction(gui.getExportByName('_ZNK12QTableWidget4itemEii'),'pointer',['pointer','int','int']);
const tableData=new NativeFunction(gui.getExportByName('_ZNK16QTableWidgetItem4dataEi'),'void',['pointer','pointer','int']);
const treeCount=new NativeFunction(gui.getExportByName('_ZNK11QTreeWidget17topLevelItemCountEv'),'int',['pointer']);
const treeTop=new NativeFunction(gui.getExportByName('_ZNK11QTreeWidget12topLevelItemEi'),'pointer',['pointer','int']);
const treeBelow=new NativeFunction(gui.getExportByName('_ZNK11QTreeWidget9itemBelowEPK15QTreeWidgetItem'),'pointer',['pointer','pointer']);
const treeData=new NativeFunction(gui.getExportByName('_ZNK15QTreeWidgetItem4dataEii'),'void',['pointer','pointer','int','int']);
const treeRect=new NativeFunction(gui.getExportByName('_ZNK11QTreeWidget14visualItemRectEPK15QTreeWidgetItem'),'void',['pointer','pointer','pointer']);
function treeRows(w){let item=treeCount(w)?treeTop(w,0):ptr(0);const out=[];while(!item.isNull()&&out.length<256){const v=Memory.alloc(32);treeData(v,item,0,0);const text=Memory.alloc(4);vs(text,v);const rect=Memory.alloc(16);treeRect(rect,w,item);out.push({pointer:item.toString(),text:string(text),rect:[0,4,8,12].map(i=>rect.add(i).readInt())});sd(text);vd(v);item=treeBelow(w,item);}return out;}
const fileApiCalls=[];for(const e of gui.enumerateExports().filter(e=>e.name.includes('getOpenFile'))){Interceptor.attach(e.address,{onEnter(args){this.out=args[0];this.index=fileApiCalls.length;fileApiCalls.push({name:e.name,thread:Process.getCurrentThreadId(),args:Array.from({length:7},(_,i)=>args[i].toString())});},onLeave(ret){try{fileApiCalls[this.index].returnString=string(this.out);}catch(ex){fileApiCalls[this.index].error=String(ex);}}});}
let pending=null;let result=null;let guiThread=0;let serial=0;
let saveAsDestination=null;const saveAsPrefills=[];const hookedShow=new Set();
Interceptor.attach(Process.getModuleByName('ole32.dll').getExportByName('CoCreateInstance'),{onEnter(args){this.selected=false;try{const b=Array.from(new Uint8Array(args[0].readByteArray(16))).map(x=>x.toString(16).padStart(2,'0')).join('');this.selected=b==='f3e2b4c021ba73478dba335ec946eb8b';this.ppv=args[4];}catch(e){}},onLeave(ret){if(!this.selected||ret.toInt32()!==0){return;}const o=this.ppv.readPointer();const show=o.readPointer().add(3*4).readPointer();if(hookedShow.has(show.toString())){return;}hookedShow.add(show.toString());Interceptor.attach(show,{onEnter(args){if(saveAsDestination===null){return;}const self=args[0];const f=new NativeFunction(self.readPointer().add(15*4).readPointer(),'int',['pointer','pointer'],'stdcall');const name=saveAsDestination;saveAsDestination=null;try{const hr=f(self,Memory.allocUtf16String(name));saveAsPrefills.push({destination:name,hresult:hr,thread:Process.getCurrentThreadId(),expectedGuiThread:guiThread,method:'IFileDialog::SetFileName immediately before native Show'});}catch(e){saveAsPrefills.push({destination:name,error:String(e)});}}});}});
function string(s){const d=s.readPointer();const n=d.add(8).readInt();if(n<0||n>200000){throw Error('QString bound');}return n?d.add(12).readPointer().readUtf16String(n):'';}
function prop(w,n,b=false){const v=Memory.alloc(32);propFn(v,w,Memory.allocUtf8String(n));if(b){const a=vb(v);vd(v);return a;}const s=Memory.alloc(4);vs(s,v);const a=string(s);sd(s);vd(v);return a;}
function kind(w){const f=new NativeFunction(w.readPointer().readPointer(),'pointer',['pointer']);return f(w).add(4).readPointer().readCString();}
function props(w){const r={pointer:w.toString(),kind:kind(w),objectName:prop(w,'objectName'),text:prop(w,'text'),title:prop(w,'windowTitle'),enabled:prop(w,'enabled',true),visible:prop(w,'visible',true)};for(const n of ['minimum','maximum','value','currentIndex','currentText','count','rowCount','columnCount','maxLength','checked','checkable','decimals','singleStep','suffix','prefix']){const v=prop(w,n);if(v!==''){r[n]=v;}}if(r.kind==='QComboBox'){r.items=[];for(let i=0;i<Math.min(Number(r.count),256);++i){const s=Memory.alloc(4);comboTextFn(s,w,i);r.items.push(string(s));sd(s);}}return r;}
function actions(w,depth=0){if(depth>6){return [];}const l=Memory.alloc(4);actionsFn(l,w);const d=l.readPointer();const b=d.add(8).readInt(),e=d.add(12).readInt();if(b<0||e<b||e-b>500){throw Error('QList bound');}const out=[];for(let i=b;i<e;++i){const a=d.add(20+i*4).readPointer();const s=Memory.alloc(4);textFn(s,a);const m=menuFn(a);out.push({pointer:a.toString(),objectName:prop(a,'objectName'),text:string(s),enabled:enabledFn(a),children:m.isNull()?[]:actions(m,depth+1)});sd(s);}return out;}
function invoke(w,slot){return invokeFn(w,Memory.allocUtf8String(slot),2,...Array(22).fill(ptr(0)));}

const sceneOf=new NativeFunction(gui.getExportByName('_ZNK13QGraphicsView5sceneEv'),'pointer',['pointer']);
const itemsOf=new NativeFunction(gui.getExportByName('_ZNK14QGraphicsScene5itemsEv'),'void',['pointer','pointer']);
const selectedOf=new NativeFunction(gui.getExportByName('_ZNK14QGraphicsScene13selectedItemsEv'),'void',['pointer','pointer']);
const rectOf=new NativeFunction(gui.getExportByName('_ZNK13QGraphicsItem17sceneBoundingRectEv'),'void',['pointer','pointer']);
const parentOf=new NativeFunction(gui.getExportByName('_ZNK13QGraphicsItem10parentItemEv'),'pointer',['pointer']);
const flagsOf=new NativeFunction(gui.getExportByName('_ZNK13QGraphicsItem5flagsEv'),'void',['pointer','pointer']);
function itemFlags(item){const out=Memory.alloc(4);flagsOf(out,item);return out.readInt();}
const selectedState=new NativeFunction(gui.getExportByName('_ZNK13QGraphicsItem10isSelectedEv'),'bool',['pointer']);
const visibleItem=new NativeFunction(gui.getExportByName('_ZNK13QGraphicsItem9isVisibleEv'),'bool',['pointer']);
const zOf=new NativeFunction(gui.getExportByName('_ZNK13QGraphicsItem6zValueEv'),'double',['pointer']);
const clearScene=new NativeFunction(gui.getExportByName('_ZN14QGraphicsScene14clearSelectionEv'),'void',['pointer']);
const selectItem=new NativeFunction(gui.getExportByName('_ZN13QGraphicsItem11setSelectedEb'),'void',['pointer','bool']);
function pointers(s,fn){const out=Memory.alloc(4);fn(out,s);const d=out.readPointer();const begin=d.add(8).readInt(),end=d.add(12).readInt();if(begin<0||end<begin||end-begin>4096){throw Error('graphics item list bound');}const a=[];for(let i=begin;i<end;++i){a.push(d.add(20+i*4).readPointer());}return a;}
function itemInfo(item){const r=Memory.alloc(32);rectOf(r,item);return {pointer:item.toString(),parent:parentOf(item).toString(),rect:[0,8,16,24].map(i=>r.add(i).readDouble()),flags:itemFlags(item),selected:selectedState(item),visible:visibleItem(item),z:zOf(item)};}
function sceneInventory(view){const scene=sceneOf(view);return {scene:scene.toString(),items:pointers(scene,itemsOf).map(itemInfo),selected:pointers(scene,selectedOf).map(itemInfo)};}
function selectRects(view,rects){const scene=sceneOf(view);const all=pointers(scene,itemsOf).map(itemInfo);const chosen=rects.map(rect=>{const matching=all.filter(i=>(i.flags&2)!==0&&i.rect.every((v,k)=>Math.abs(v-rect[k])<=2));if(matching.length!==1){throw Error('Selectable geometry not unique '+JSON.stringify({rect,matching}));}return matching[0];});clearScene(scene);chosen.forEach(i=>selectItem(ptr(i.pointer),1));return {chosen,after:sceneInventory(view)};}

const getViewport=new NativeFunction(gui.getExportByName('_ZNK19QAbstractScrollArea8viewportEv'),'pointer',['pointer']);const getWinId=new NativeFunction(gui.getExportByName('_ZNK7QWidget5winIdEv'),'pointer',['pointer']);
const mapFromScene=new NativeFunction(gui.getExportByName('_ZNK13QGraphicsView12mapFromSceneERK7QPointF'),'int64',['pointer','pointer']);

//20261111-0 add: Read only owned GUI-thread widget metrics and tree-decoration resources.
const resizeClient=new NativeFunction(gui.getExportByName('_ZN7QWidget6resizeERK5QSize'),'void',['pointer','pointer']);const grabWidget=new NativeFunction(gui.getExportByName('_ZN7QPixmap10grabWidgetEP7QWidgetRK5QRect'),'void',['pointer','pointer','pointer']);
const vconst=new NativeFunction(core.getExportByName('_ZNK8QVariant9constDataEv'),'pointer',['pointer']);
const vtype=new NativeFunction(core.getExportByName('_ZNK8QVariant8userTypeEv'),'int',['pointer']);
const vrect=new NativeFunction(core.getExportByName('_ZNK8QVariant6toRectEv'),'void',['pointer','pointer']);
const fontString=new NativeFunction(gui.getExportByName('_ZNK5QFont8toStringEv'),'void',['pointer','pointer']);
const colorRgba=new NativeFunction(gui.getExportByName('_ZNK6QColor4rgbaEv'),'uint',['pointer']);
const paletteBrush=new NativeFunction(gui.getExportByName('_ZNK8QPalette5brushENS_10ColorGroupENS_9ColorRoleE'),'pointer',['pointer','int','int']);const penCtor=new NativeFunction(gui.getExportByName('_ZN4QPenC1ERK6QBrushdN2Qt8PenStyleENS3_11PenCapStyleENS3_12PenJoinStyleE'),'void',['pointer','pointer','double','int','int','int']);const penColor=new NativeFunction(gui.getExportByName('_ZNK4QPen5colorEv'),'void',['pointer','pointer']);const penDtor=new NativeFunction(gui.getExportByName('_ZN4QPenD1Ev'),'void',['pointer']);function paletteRgba(p,g,r){const pen=Memory.alloc(16);penCtor(pen,paletteBrush(p,g,r),1.0,1,0,0);try{const c=Memory.alloc(32);penColor(c,pen);return colorRgba(c);}finally{penDtor(pen);}}
const marginsFn=new NativeFunction(gui.getExportByName('_ZNK7QWidget18getContentsMarginsEPiS0_S0_S0_'),'void',['pointer','pointer','pointer','pointer','pointer']);
const layoutFn=new NativeFunction(gui.getExportByName('_ZNK7QWidget6layoutEv'),'pointer',['pointer']);
const layoutMargins=new NativeFunction(gui.getExportByName('_ZNK7QLayout18getContentsMarginsEPiS0_S0_S0_'),'void',['pointer','pointer','pointer','pointer','pointer']);
const layoutSpacing=new NativeFunction(gui.getExportByName('_ZNK7QLayout7spacingEv'),'int',['pointer']);


const treeIndex=new NativeFunction(gui.getExportByName('_ZNK11QTreeWidget13indexFromItemEP15QTreeWidgetItemi'),'void',['pointer','pointer','pointer','int']);const treeExpanded=new NativeFunction(gui.getExportByName('_ZNK9QTreeView10isExpandedERK11QModelIndex'),'bool',['pointer','pointer']);
const iconNull=new NativeFunction(gui.getExportByName('_ZNK5QIcon6isNullEv'),'bool',['pointer']);
const iconPixmap=new NativeFunction(gui.getExportByName('_ZNK5QIcon6pixmapERK5QSizeNS_4ModeENS_5StateE'),'void',['pointer','pointer','pointer','int','int']);
const pixmapSave=new NativeFunction(gui.getExportByName('_ZNK7QPixmap4saveERK7QStringPKci'),'bool',['pointer','pointer','pointer','int']);
const pixmapDtor=new NativeFunction(gui.getExportByName('_ZN7QPixmapD1Ev'),'void',['pointer']);
function variant(w,n,fn){const v=Memory.alloc(32);propFn(v,w,Memory.allocUtf8String(n));try{return fn(v,vtype(v),vconst(v));}finally{vd(v);}}
function rectProp(w,n){return variant(w,n,(v,t,d)=>{if(t!==19){return {variantType:t};}const r=Memory.alloc(16);vrect(r,v);return [r.readInt(),r.add(4).readInt(),r.add(8).readInt()-r.readInt()+1,r.add(12).readInt()-r.add(4).readInt()+1];});}
function sizeProp(w,n){return variant(w,n,(v,t,d)=>t===21?[d.readInt(),d.add(4).readInt()]:{variantType:t});}
function marginValues(w,fn){const r=Memory.alloc(16);fn(w,r,r.add(4),r.add(8),r.add(12));return [0,4,8,12].map(i=>r.add(i).readInt());}
function widgetDetails(w){const q=props(w);q.geometry=rectProp(w,'geometry');q.contentsRect=rectProp(w,'contentsRect');q.contentsMargins=marginValues(w,marginsFn);q.minimumSize=sizeProp(w,'minimumSize');q.maximumSize=sizeProp(w,'maximumSize');q.font=variant(w,'font',(v,t,d)=>{if(t!==64){return {variantType:t};}const st=Memory.alloc(4);fontString(st,d);const out=string(st);sd(st);return out;});q.palette=variant(w,'palette',(v,t,d)=>{if(t!==68){return {variantType:t};}const roles=['WindowText','Button','Light','Midlight','Dark','Mid','Text','BrightText','ButtonText','Base','Window','Shadow','Highlight','HighlightedText','Link','LinkVisited','AlternateBase','NoRole','ToolTipBase','ToolTipText'];const out={};for(let g=0;g<3;g++){const colors={};for(let role=0;role<roles.length;role++){colors[roles[role]]='0x'+paletteRgba(d,g,role).toString(16).padStart(8,'0');}out[['active','disabled','inactive'][g]]=colors;}return out;});const l=layoutFn(w);if(!l.isNull()){q.layout={kind:kind(l),margins:marginValues(l,layoutMargins),spacing:layoutSpacing(l)};}for(const n of ['readOnly','focusPolicy','frameShape','textInteractionFlags','styleSheet','frameWidth','lineWidth','midLineWidth','indentation','rootIsDecorated','itemsExpandable','headerHidden','alternatingRowColors','uniformRowHeights','expandsOnDoubleClick','horizontalScrollBarPolicy','verticalScrollBarPolicy','dockNestingEnabled','dockOptions','iconSize','toolButtonStyle','orientation','features','allowedAreas','floating','tabPosition','documentMode','tabsClosable','movable','currentMessage','sizeGripEnabled','alignment','wordWrap','margin','spacing']){if(n==='iconSize'){q[n]=sizeProp(w,n);}else{const v=prop(w,n);if(v!==''){q[n]=v;}}}return q;}
function saveTreeIcon(item,path){const v=Memory.alloc(32);treeData(v,item,0,1);try{const t=vtype(v);if(t!==69){return {variantType:t,null:true};}const icon=vconst(v);if(iconNull(icon)){return {variantType:t,null:true};}const out=[];for(const size of [16,24]){const p=Memory.alloc(64);const dim=Memory.alloc(8);dim.writeInt(size);dim.add(4).writeInt(size);iconPixmap(p,icon,dim,0,1);const st=Memory.alloc(4);const dest=path+'-'+size+'.png';sc(st,Memory.allocUtf16String(dest),dest.length);const saved=pixmapSave(p,st,Memory.allocUtf8String('PNG'),-1);sd(st);pixmapDtor(p);out.push({size,path:dest,saved});}return {variantType:t,null:false,exports:out};}finally{vd(v);}}
function treeDetailed(w,directory){const nodes=[];const tops=[];for(let i=0;i<treeCount(w);i++){tops.push(treeTop(w,i).toString());}let item=tops.length?ptr(tops[0]):ptr(0);while(!item.isNull()&&nodes.length<256){const v=Memory.alloc(32);treeData(v,item,0,0);const st=Memory.alloc(4);vs(st,v);const label=string(st);sd(st);vd(v);const r=Memory.alloc(16);treeRect(r,w,item);nodes.push({ordinal:nodes.length,pointer:item.toString(),topLevelIndex:tops.indexOf(item.toString()),text:label,rect:[r.readInt(),r.add(4).readInt(),r.add(8).readInt()-r.readInt()+1,r.add(12).readInt()-r.add(4).readInt()+1],icon:saveTreeIcon(item,directory+'/'+String(nodes.length).padStart(3,'0'))});item=treeBelow(w,item);}return {widget:widgetDetails(w),hierarchyEvidence:'Top-level pointers and expanded visualItemRect indentation, in actual itemBelow order',nodes};}

function execute(p){if(p.op==='tree-expansion'){const w=find(ptr(p.hwnd));return treeRows(w).map(row=>{const index=Memory.alloc(16);treeIndex(index,w,ptr(row.pointer),0);return {...row,expanded:treeExpanded(w,index)};});}if(p.op==='resize-client'){const size=Memory.alloc(8);size.writeInt(p.width);size.add(4).writeInt(p.height);resizeClient(find(ptr(p.hwnd)),size);return props(find(ptr(p.hwnd)));}if(p.op==='client-capture'){const w=find(ptr(p.hwnd));const width=Number(prop(w,'width')),height=Number(prop(w,'height'));const rect=Memory.alloc(16);rect.writeInt(0);rect.add(4).writeInt(0);rect.add(8).writeInt(width-1);rect.add(12).writeInt(height-1);const pic=Memory.alloc(64);grabWidget(pic,w,rect);const st=Memory.alloc(4);sc(st,Memory.allocUtf16String(p.path),p.path.length);try{return {size:[width,height],saved:pixmapSave(pic,st,Memory.allocUtf8String('PNG'),-1),path:p.path,method:'original Qt4 QPixmap::grabWidget, client content including child widgets'};}finally{sd(st);pixmapDtor(pic);}}if(p.op==='widget-details'){return p.hwnds.map(h=>({hwnd:h,qt:widgetDetails(find(ptr(h)))}));}if(p.op==='tree-detailed'){return treeDetailed(find(ptr(p.hwnd)),p.directory);}if(p.op==='scene-point'){const xy=Memory.alloc(16);xy.writeDouble(p.x);xy.add(8).writeDouble(p.y);const value=BigInt(mapFromScene(find(ptr(p.hwnd)),xy).toString());return {x:Number(BigInt.asIntN(32,value)),y:Number(BigInt.asIntN(32,value>>32n)),viewport:getWinId(getViewport(find(ptr(p.hwnd)))).toUInt32()};}if(p.op==='scene-read'){return sceneInventory(find(ptr(p.hwnd)));}if(p.op==='scene-select'){return selectRects(find(ptr(p.hwnd)),p.rects);}if(p.op==='table-read'){const w=find(ptr(p.hwnd));const rows=Number(prop(w,'rowCount')),cols=Number(prop(w,'columnCount')),out=[];for(let r=0;r<Math.min(rows,300);++r){let row=[];for(let c=0;c<Math.min(cols,12);++c){const item=tableItem(w,r,c);if(item.isNull()){row.push(null);continue;}const v=Memory.alloc(32);tableData(v,item,0);const st=Memory.alloc(4);vs(st,v);row.push(string(st));sd(st);vd(v);}out.push(row);}return out;}if(p.op==='tree-item-clicked'){const ip=Memory.alloc(4);ip.writePointer(ptr(p.item));const col=Memory.alloc(4);col.writeInt(0);return {queuedMetaInvoke:invokeFn(find(ptr(p.hwnd)),Memory.allocUtf8String('SlotItemClicked'),2,ptr(0),ptr(0),Memory.allocUtf8String('QTreeWidgetItem*'),ip,Memory.allocUtf8String('int'),col,...Array(16).fill(ptr(0)))};}if(p.op==='point-invoke'){const xy=Memory.alloc(8);xy.writeInt(p.x);xy.add(4).writeInt(p.y);return {queuedMetaInvoke:invokeFn(find(ptr(p.hwnd)),Memory.allocUtf8String('customContextMenuRequested'),2,ptr(0),ptr(0),Memory.allocUtf8String('QPoint'),xy,...Array(18).fill(ptr(0)))};}if(p.op==='batch'){return p.hwnds.map(h=>{try{return {hwnd:h,qt:props(find(ptr(h)))};}catch(e){return {hwnd:h,error:String(e)};}});}if(p.op==='menus'){return actions(find(ptr(p.hwnd)));}if(p.op==='invoke'){const w=p.pointer?ptr(p.pointer):find(ptr(p.hwnd));return {queuedMetaInvoke:invoke(w,p.slot),slot:p.slot,pointer:w.toString()};}const w=find(ptr(p.hwnd));if(p.op==='tree'){return treeRows(w);}if(p.op==='table-select'){tableCurrentFn(w,p.row,p.column);return props(w);}if(p.op==='table-cell'){return {x:tableColumnXFn(w,p.column)+Math.floor(tableColumnWidthFn(w,p.column)/2),y:tableRowYFn(w,p.row)+Math.floor(tableRowHeightFn(w,p.row)/2),row:p.row,column:p.column};}if(p.op==='table-open'){tableCurrentFn(w,p.row,p.column);const a=Memory.alloc(4);a.writeInt(p.row);const b=Memory.alloc(4);b.writeInt(p.column);const ty=Memory.allocUtf8String('int');return {queuedMetaInvoke:invokeFn(w,Memory.allocUtf8String('cellDoubleClicked'),2,ptr(0),ptr(0),ty,a,ty,b,...Array(16).fill(ptr(0))),row:p.row,column:p.column,selection:'normal control-list current cell and queued cellDoubleClicked signal'};}if(p.op==='text'){const s=Memory.alloc(4);sc(s,Memory.allocUtf16String(p.value),p.value.length);setTextFn(w,s);sd(s);return props(w);}if(p.op==='spin'){spinFn(w,p.value);return props(w);}if(p.op==='tab'){tabSetFn(w,p.value);return props(w);}if(p.op==='group'){groupSetFn(w,p.value?1:0);groupClickedFn(w,p.value?1:0);return props(w);}if(p.op==='combo'){comboSetFn(w,p.value);comboSignalFn(w,p.value);return props(w);}throw Error('unsupported operation');}
for(const symbol of ['DispatchMessageW','DispatchMessageA']){Interceptor.attach(Process.getModuleByName('user32.dll').getExportByName(symbol),{onEnter(){if(Process.getCurrentThreadId()!==guiThread||pending===null){return;}const p=pending;pending=null;try{result={serial:p.serial,status:'completed',value:execute(p),guiThread:Process.getCurrentThreadId(),dispatchHook:symbol};}catch(e){result={serial:p.serial,status:'error',error:String(e)};}}});}
rpc.exports={fileApiCalls(){return fileApiCalls;},setThread(t){guiThread=t;},armSaveAs(value){saveAsDestination=value;return 'armed-standard-native-SetFileName';},saveAsPrefills(){return saveAsPrefills;},queue(p){if(pending!==null){throw Error('operation pending');}p.serial=++serial;pending=p;result=null;return serial;},poll(){return result;}};
"""


def windows(desktop, pid):
    """Enumerate only this task's target process on its private desktop."""
    found = []
    win32gui.EnumDesktopWindows(pywintypes.HANDLE(desktop), lambda h, _: found.append(h) if win32process.GetWindowThreadProcessId(h)[1] == pid else None, None)
    return found


def main():
    """Create native samples through actual actions and preserve original inputs."""
    global WORK
    parser = argparse.ArgumentParser()
    parser.add_argument('--run', default='r4')
    parser.add_argument('--input', type=Path, default=ROOT / 'KinSealStudio/samples/native-device-test/CodexProbeTest.sm')
    parser.add_argument('--no-clear', action='store_true')
    parser.add_argument('--page-name')
    parser.add_argument('--page-number', type=int)
    parser.add_argument('--output-name')
    parser.add_argument('--reopen-only', action='store_true')
    parser.add_argument('--cycle-pages', type=int, default=0)
    parser.add_argument('--bit-variant', choices=['operations','notice','external','monitor-disabled','static-level','roundtrip'])
    parser.add_argument('--save-as-output')
    parser.add_argument('--inspect-control-list', action='store_true')
    parser.add_argument('--control-row', type=int)
    parser.add_argument('--language-index', type=int)
    parser.add_argument('--inspect-action')
    parser.add_argument('--menus-only', action='store_true')
    parser.add_argument('--tree-only', action='store_true')
    parser.add_argument('--tree-open')
    parser.add_argument('--settings-language-index', type=int)
    parser.add_argument('--language-count', type=int, default=2, choices=(2,3))
    parser.add_argument('--compile-native', action='store_true')
    parser.add_argument('--scenario', type=Path, required=True)
    args = parser.parse_args()
    WORK = ROOT / '.scratch/kinseal-main-window-20261111/tree' / ('runtime-' + args.run)
    WORK.mkdir(parents=True, exist_ok=False)
    original = (VENDOR / "KinSealStudio5.1.exe").read_bytes()
    assert hashlib.sha256(original).hexdigest() == "305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c"
    private = bytearray(original)
    assert private[0x7c4:0x7c7] == bytes.fromhex("5589e5")
    private[0x7c4:0x7c7] = bytes.fromhex("b001c3")
    assert hashlib.sha256(private).hexdigest() == "38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89"
    exe = WORK / "KinSealStudio5.1-private.exe"
    shared = ROOT / ".scratch/kinseal-stage3-20261110/native/private-runtime"
    shared.mkdir(exist_ok=True)
    shared_exe = shared / "KinSealStudio5.1-private.exe"
    if not shared_exe.exists(): shared_exe.write_bytes(private)
    assert hashlib.sha256(shared_exe.read_bytes()).hexdigest()==hashlib.sha256(private).hexdigest()
    os.link(shared_exe,exe)
    for name in ("model.dat", "modelInfo.dat", "language.ini"):
        shutil.copy2(VENDOR / name, WORK / name)
    #20261111-0 add: The original Qt4 shell requires its release codec/SVG/image plugins beside the private executable.
    for plugin_directory in ('codecs', 'imageformats', 'iconengines'):
        target_directory = WORK / plugin_directory
        target_directory.mkdir(exist_ok=True)
        for plugin_file in (VENDOR / plugin_directory).glob('*4.dll'):
            if not plugin_file.name.endswith('d4.dll'):
                shutil.copy2(plugin_file, target_directory / plugin_file.name)
    source = WORK / "NativePages.sm"
    shutil.copy2(args.input, source)
    result = {"date": "2026-11-11", "run":args.run, "originalSha256": hashlib.sha256(original).hexdigest(), "runtimeSha256": hashlib.sha256(private).hexdigest(), "patch": {"offset": "0x7c4", "before": "5589e5", "after": "b001c3"}, "desktop": DESKTOP, "input": {"path": str(args.input), "sha256": hashlib.sha256(source.read_bytes()).hexdigest()}, "steps": [], "snapshots": [], "systemInput": False, "usbOrPlc": False, "qtOperationsGuiThreadOnly": True}
    u = ctypes.WinDLL("user32", use_last_error=True)
    u.CreateDesktopW.argtypes = [ctypes.c_wchar_p, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_void_p]
    u.CreateDesktopW.restype = ctypes.c_void_p
    u.CloseDesktop.argtypes = [ctypes.c_void_p]
    desk = u.CreateDesktopW(DESKTOP, None, None, 0, 0x000F01FF, None)
    if not desk:
        raise OSError(ctypes.get_last_error(), "private desktop unavailable")
    process = thread = session = None
    try:
        startup = win32process.STARTUPINFO()
        startup.lpDesktop = "WinSta0\\" + DESKTOP
        startup.dwFlags = win32con.STARTF_USESHOWWINDOW
        startup.wShowWindow = win32con.SW_SHOWNORMAL
        env = os.environ.copy()
        env["PATH"] = str(VENDOR) + os.pathsep + env.get("PATH", "")
        env["__COMPAT_LAYER"] = "RunAsInvoker"
        env["TEMP"] = str(ROOT / ".scratch/kinseal-stage3-20261110/native/temp")
        env["TMP"] = env["TEMP"]
        process, thread, pid, _ = win32process.CreateProcess(str(exe), '"%s" "%s"' % (exe, source), None, None, False, win32process.CREATE_UNICODE_ENVIRONMENT, env, str(WORK), startup)
        result["pid"] = pid
        deadline = time.time() + 24
        editor = None
        while time.time() < deadline:
            for top in windows(desk, pid):
                children = []
                win32gui.EnumChildWindows(top, lambda h, _: children.append(h), None)
                if any(win32gui.GetWindowText(h) == "menuBar" for h in children):
                    editor = top
                    break
            if editor:
                break
            time.sleep(.25)
        if editor is None:
            raise RuntimeError("editor unavailable within 24 seconds")
        time.sleep(1.5)
        children = []
        win32gui.EnumChildWindows(editor, lambda h, _: children.append(h), None)
        menu = next(h for h in children if win32gui.GetWindowText(h) == "menuBar")
        session = frida.attach(pid)
        script = session.create_script(JS)
        script.on("message", lambda m, _: result.setdefault("instrumentationMessages", []).append(m))
        script.load()
        api = script.exports_sync
        api.set_thread(win32process.GetWindowThreadProcessId(menu)[0])

        def op(request):
            serial = api.queue(request)
            win32gui.PostMessage(menu, win32con.WM_NULL, 0, 0)
            limit = time.time() + 5
            while time.time() < limit:
                r = api.poll()
                if r and r["serial"] == serial:
                    result["steps"].append({"request": request, "result": r})
                    if r["status"] != "completed":
                        raise RuntimeError(str(r))
                    return r["value"]
                time.sleep(.04)
            raise RuntimeError("GUI-thread queued operation timeout: " + str(request))

        def dump(label):
            rows = []
            for top in windows(desk, pid):
                if not win32gui.IsWindowVisible(top) or win32gui.GetClassName(top) not in ('QWidget','#32770'):
                    continue
                cs = [top]
                win32gui.EnumChildWindows(top, lambda h, _: cs.append(h), None)
                rows.append({"hwnd": top, "title": win32gui.GetWindowText(top), "winClass":win32gui.GetClassName(top), "widgets": op({"op": "batch", "hwnds": cs}), "nativeChildren":[{'hwnd':h,'text':win32gui.GetWindowText(h),'class':win32gui.GetClassName(h),'controlId':win32gui.GetDlgCtrlID(h)} for h in cs]})
            result["snapshots"].append({"label": label, "windows": rows})
            (OUT / "native-tree-probe-checkpoint.json").write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8")
            print(json.dumps({"snapshot": label, "windows": [r["title"] for r in rows]}, ensure_ascii=False), flush=True)
            return rows

        def action(name):
            names = []
            batch = op({"op": "batch", "hwnds": children})
            for item in batch:
                q = item.get("qt", {})
                if q.get("objectName") == "menuBar" or "toolbar" in q.get("objectName", "").casefold():
                    def walk(rows):
                        for row in rows:
                            yield row
                            yield from walk(row.get("children", []))
                    names.extend(walk(op({"op": "menus", "hwnd": item["hwnd"]})))
            row = next(r for r in names if r["objectName"] == name)
            assert row["enabled"], (name, row)
            r = op({"op": "invoke", "pointer": row["pointer"], "slot": "trigger"})
            assert r["queuedMetaInvoke"]
            time.sleep(.35)

        def save_as(output):
            """Accept only this process's native Save As dialog after verified prefill."""
            destination = (OUT / output).resolve()
            assert destination.parent == OUT.resolve() and not destination.exists()
            api.arm_save_as(str(destination))
            action('action_save_as')
            deadline = time.time() + 7
            dialogs = []
            while time.time() < deadline:
                dialogs = [h for h in windows(desk, pid) if win32gui.IsWindowVisible(h) and win32gui.GetClassName(h) == '#32770']
                if dialogs:
                    break
                time.sleep(.1)
            assert len(dialogs) == 1, 'Owned native Save As dialog not unique'
            dialog = dialogs[0]
            time.sleep(.7)
            controls = []
            win32gui.EnumChildWindows(dialog, lambda h, _: controls.append(h), None)
            prefills = api.save_as_prefills()
            native_rows = [{'hwnd': h, 'text': win32gui.GetWindowText(h), 'class': win32gui.GetClassName(h), 'controlId': win32gui.GetDlgCtrlID(h), 'parentId': win32gui.GetDlgCtrlID(win32gui.GetParent(h))} for h in controls]
            result.setdefault('saveAsDialogs', []).append({'destination': str(destination), 'prefills': prefills, 'dialog': dialog, 'controls': native_rows})
            if not any(p.get('hresult') == 0 and p.get('destination') == str(destination) for p in prefills):
                edits = [r for r in native_rows if r['class'] == 'Edit' and (r['controlId'] == 1148 or r['parentId'] == 1148)]
                if not edits:
                    edits = [r for r in native_rows if r['class'] == 'Edit' and r['controlId'] == 1001]
                assert len(edits) == 1, 'Owned Save As filename edit not unique'
                message = ctypes.WinDLL('user32', use_last_error=True).SendMessageW
                message.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_size_t, ctypes.c_void_p]
                message.restype = ctypes.c_ssize_t
                filename = ctypes.create_unicode_buffer(str(destination))
                message(edits[0]['hwnd'], win32con.WM_SETTEXT, 0, ctypes.cast(filename, ctypes.c_void_p))
                readback = ctypes.create_unicode_buffer(4096)
                message(edits[0]['hwnd'], win32con.WM_GETTEXT, 4096, ctypes.cast(readback, ctypes.c_void_p))
                result['saveAsDialogs'][-1]['filenameSetAndReadBack'] = readback.value
                assert readback.value == str(destination), 'Native filename edit verification failed'
            buttons = [h for h in controls if win32gui.GetClassName(h) == 'Button' and win32gui.GetDlgCtrlID(h) == 1]
            assert len(buttons) == 1
            result.setdefault('saveAs', []).append({'destination': str(destination), 'prefills': prefills, 'dialog': dialog, 'button': buttons[0], 'method': 'owned private native common dialog; verified IFileDialog::SetFileName then WM_COMMAND IDOK'})
            win32gui.PostMessage(buttons[0], win32con.BM_CLICK, 0, 0)
            deadline = time.time() + 7
            while time.time() < deadline and not destination.is_file():
                time.sleep(.1)
            result['saveAsDialogs'][-1]['postState'] = [{'hwnd': h, 'title': win32gui.GetWindowText(h), 'class': win32gui.GetClassName(h)} for h in windows(desk, pid) if win32gui.IsWindowVisible(h)]
            assert destination.is_file(), 'Native Save As produced no source'
            result.setdefault('outputs', []).append({'path': destination.name, 'bytes': destination.stat().st_size, 'sha256': hashlib.sha256(destination.read_bytes()).hexdigest(), 'origin': 'actual enabled native Save As after native editing action'})


        def rows_now():
            return dump('lookup-'+str(len(result['snapshots'])))

        def widget(command):
            ws=[w for ri,r in enumerate(rows_now()) for w in r['widgets'] if w.get('qt') and ('windowIndex' not in command or ri==command['windowIndex'])]
            if 'name' in command:
                ws=[w for w in ws if w['qt'].get('objectName')==command['name']]
            if 'text' in command:
                ws=[w for w in ws if w['qt'].get('text') in (command['text'],command['text'].encode('gbk',errors='replace').decode('latin1'))]
            if 'kind' in command:
                ws=[w for w in ws if w['qt'].get('kind')==command['kind']]
            ws=[w for w in ws if w['qt'].get('visible')]
            if len(ws)!=1:
                raise RuntimeError('widget match not unique '+str(command)+' '+str([(w['hwnd'],w['qt'].get('objectName')) for w in ws]))
            return ws[0]

        def tree_event(command):
            ws=op({'op':'batch','hwnds':children})
            tree=next(w['hwnd'] for w in ws if w.get('qt',{}).get('kind')=='QTreeWidget')
            op({'op':'invoke','hwnd':tree,'slot':'expandAll'})
            time.sleep(.2)
            tree_rows=op({'op':'tree','hwnd':tree})
            entry=next(x for x in tree_rows if x['text']==command['entry'])
            viewport=next(h for h in children if win32gui.GetParent(h)==tree and win32gui.GetWindowText(h)=='qt_scrollarea_viewport')
            left,top,right,bottom=entry['rect']; lparam=win32api.MAKELONG((left+right)//2,(top+bottom)//2)
            if command['op']=='tree-context':
                messages=((win32con.WM_RBUTTONDOWN,win32con.MK_RBUTTON),(win32con.WM_RBUTTONUP,0))
            elif command['op']=='tree-single':
                messages=((win32con.WM_LBUTTONDOWN,win32con.MK_LBUTTON),(win32con.WM_LBUTTONUP,0))
            else:
                messages=((win32con.WM_LBUTTONDOWN,win32con.MK_LBUTTON),(win32con.WM_LBUTTONUP,0),(win32con.WM_LBUTTONDBLCLK,win32con.MK_LBUTTON),(win32con.WM_LBUTTONUP,0))
            for message,flags in messages:
                win32gui.PostMessage(viewport,message,flags,lparam)
            if command['op']=='tree-context':
                result['steps'].append({'request':{'op':'point-invoke','hwnd':tree,'x':(left+right)//2,'y':(top+bottom)//2},'result':op({'op':'point-invoke','hwnd':tree,'x':(left+right)//2,'y':(top+bottom)//2})})
            if command['op']=='tree-single':
                op({'op':'tree-item-clicked','hwnd':editor,'item':entry['pointer']})
            if command['op']=='tree-context':
                xy=win32gui.ClientToScreen(viewport,((left+right)//2,(top+bottom)//2))
                win32gui.PostMessage(viewport,win32con.WM_CONTEXTMENU,viewport,win32api.MAKELONG(*xy))
            result['steps'].append({'request':command,'source':'owned private tree viewport background window messages; no system input','row':entry})
            time.sleep(.5)
            if command['op']=='tree-double' and command['entry']=='²ÎÊýÉèÖÃ':
                for attempt in range(3):
                    visible_parameters=[h for h in windows(desk,pid) if h!=editor and win32gui.IsWindowVisible(h) and '参数设置' in win32gui.GetWindowText(h)]
                    if visible_parameters:break
                    for message,flags in messages:win32gui.PostMessage(viewport,message,flags,lparam)
                    time.sleep(.5)
                result['steps'].append({'request':{'op':'bounded-private-parameter-activation-check','attempts':attempt+1},'parameterDialogVisible':bool(visible_parameters)})
            if command['op']=='tree-context':
                result.setdefault('contextMenus',[]).append({'entry':entry,'menus':[{'hwnd':t,'actions':op({'op':'menus','hwnd':t})} for t in windows(desk,pid) if win32gui.IsWindowVisible(t) and win32gui.GetClassName(t)=='QWidget' and t!=editor]})

        dump('initial')
        result['menuInventory']=op({'op':'menus','hwnd':menu})
        for index,command in enumerate(json.loads(args.scenario.read_text(encoding='utf8'))):
            name=command['op']
            if name=='dump':
                dump(command.get('label','step-'+str(index)))
            elif name=='action':
                action(command['name'])
            elif name in ('scene-read','scene-select','scene-assert'):
                ws=op({'op':'batch','hwnds':children})
                view=next(w['hwnd'] for w in ws if w.get('qt',{}).get('kind')=='myGraphicsView')
                request={'op':'scene-read' if name=='scene-assert' else name,'hwnd':view}
                if name=='scene-select':request['rects']=command['rects']
                scene_data=op(request)
                result.setdefault('sceneReads',[]).append({'label':command.get('label'),'data':scene_data})
                if name=='scene-assert':
                    roots=[i for i in scene_data['items'] if i['parent']=='0x0' and i['flags']&2]
                    assert all(any(all(abs(i['rect'][k]-v)<.001 for k,v in enumerate(rect)) for i in roots) for rect in command['rects']), 'Native undo did not restore expected geometry'
            elif name=='native-compile':
                start_ns=time.time_ns()
                action('action_USBdownload')
                ws=op({'op':'batch','hwnds':children})
                tables=[w for w in ws if w.get('qt',{}).get('kind')=='QTableWidget']
                logs=[op({'op':'table-read','hwnd':w['hwnd']}) for w in tables]
                decoded_logs=[[[(cell.encode('latin1').decode('gbk') if cell and all(ord(c)<256 for c in cell) else cell) for cell in row] for row in table] for table in logs]
                system=WORK/'compile/SysPara.dat'
                compile_deadline=time.time()+25
                while time.time()<compile_deadline and (not system.is_file() or system.stat().st_mtime_ns<start_ns or not any('编译成功' in str(table) for table in decoded_logs)):
                    time.sleep(.25)
                    logs=[op({'op':'table-read','hwnd':w['hwnd']}) for w in tables]
                    decoded_logs=[[[(cell.encode('latin1').decode('gbk') if cell and all(ord(c)<256 for c in cell) else cell) for cell in row] for row in table] for table in logs]
                if not system.is_file() or system.stat().st_mtime_ns<start_ns or not any('编译成功' in str(table) for table in decoded_logs):
                    dump('native-compile-not-confirmed-'+command['label'])
                    raise RuntimeError('Fresh native compile not confirmed: '+str(decoded_logs))
                destination=OUT/'compiled'/command['label'];destination.parent.mkdir(exist_ok=True);assert not destination.exists();shutil.copytree(system.parent,destination)
                result.setdefault('nativeCompilations',[]).append({'label':command['label'],'sourceSha256':hashlib.sha256(source.read_bytes()).hexdigest(),'logsRaw':logs,'logsDecodedCandidate':decoded_logs,'files':[{'path':f.name,'bytes':f.stat().st_size,'sha256':hashlib.sha256(f.read_bytes()).hexdigest()} for f in sorted(destination.iterdir()) if f.is_file()],'usbWriteCommandsInvoked':False,'method':'actual native action_USBdownload compiler phase; captured WORK/compile after fresh SysPara mtime and success log; no download/transfer button invoked'})
                compiled_source=destination/'input-native.sm';shutil.copy2(source,compiled_source)
                current=rows_now()
                closes=[w for r in current for w in r['widgets'] if w.get('qt',{}).get('objectName')=='m_closebtn' and w['qt'].get('visible')]
                if closes:
                    op({'op':'invoke','hwnd':closes[0]['hwnd'],'slot':'click'})
                    time.sleep(.4)
            elif name=='shell-client-resize':
                assert win32process.GetWindowThreadProcessId(editor)[1]==pid
                win32gui.ShowWindow(editor,win32con.SW_SHOWNOACTIVATE)
                time.sleep(.4)
                op({'op':'resize-client','hwnd':editor,'width':command['width'],'height':command['height']});time.sleep(1.2)
            elif name=='main-client-capture':
                destination=(OUT/command['output']).resolve();assert not destination.exists()
                left,top,right,bottom=win32gui.GetWindowRect(editor);width=right-left;height=bottom-top
                origin=win32gui.ClientToScreen(editor,(0,0));client=win32gui.GetClientRect(editor)
                hdc=win32gui.GetWindowDC(editor);srcdc=win32ui.CreateDCFromHandle(hdc);memdc=srcdc.CreateCompatibleDC();bitmap=win32ui.CreateBitmap();bitmap.CreateCompatibleBitmap(srcdc,width,height);memdc.SelectObject(bitmap)
                try:
                    ok=ctypes.windll.user32.PrintWindow(editor,memdc.GetSafeHdc(),2);assert ok
                    picture=Image.frombuffer('RGB',(width,height),bitmap.GetBitmapBits(True),'raw','BGRX',0,1)
                    crop=(origin[0]-left,origin[1]-top,origin[0]-left+client[2],origin[1]-top+client[3]);picture=picture.crop(crop);picture.save(destination)
                    result['clientCapture']={'path':destination.name,'size':picture.size,'sha256':hashlib.sha256(destination.read_bytes()).hexdigest(),'fullWindowRect':[left,top,right,bottom],'clientCropInWindow':crop,'method':'PrintWindow owned private mainwindow, exact Win32 client rectangle crop; no resampling'}
                finally:
                    win32gui.DeleteObject(bitmap.GetHandle());memdc.DeleteDC();srcdc.DeleteDC();win32gui.ReleaseDC(editor,hdc)
            elif name=='shell-metrics':
                assert win32process.GetWindowThreadProcessId(editor)[1]==pid
                if command.get('resize',True):win32gui.SetWindowPos(editor,0,0,0,command.get('width',1353),command.get('height',731),win32con.SWP_NOACTIVATE|win32con.SWP_NOZORDER|win32con.SWP_NOMOVE)
                time.sleep(1.2)
                cs=[editor];win32gui.EnumChildWindows(editor,lambda h,_:cs.append(h),None)
                details=[]
                for begin in range(0,len(cs),20):details.extend(op({'op':'widget-details','hwnds':cs[begin:begin+20]}))
                native=[]
                client_origin=win32gui.ClientToScreen(editor,(0,0))
                for h in cs:
                    native.append({'hwnd':h,'parent':win32gui.GetParent(h),'windowRect':win32gui.GetWindowRect(h),'mainClientOrigin':[win32gui.ClientToScreen(h,(0,0))[axis]-client_origin[axis] for axis in range(2)],'clientRect':win32gui.GetClientRect(h),'visible':bool(win32gui.IsWindowVisible(h))})
                dpi=ctypes.WinDLL('user32').GetDpiForWindow;dpi.argtypes=[ctypes.c_void_p];dpi.restype=ctypes.c_uint
                result['shellMetrics']={'qt':details,'native':native,'windowDpi':dpi(editor),'clientScreenOrigin':client_origin,'clientRect':win32gui.GetClientRect(editor),'outerRect':win32gui.GetWindowRect(editor),'requestedOuterSize':[command.get('width',1353),command.get('height',731)],'resizeFlags':'SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE'}
            elif name=='tree-details':
                ws=op({'op':'batch','hwnds':children});trees=[w for w in ws if w.get('qt',{}).get('kind')=='QTreeWidget']
                result['treeDetails']=[]
                for index,w in enumerate(trees):
                    result.setdefault('treeDefaultExpansion',[]).append({'hwnd':w['hwnd'],'rows':op({'op':'tree-expansion','hwnd':w['hwnd']})})
                    op({'op':'invoke','hwnd':w['hwnd'],'slot':'expandAll'});time.sleep(.2)
                    directory=ROOT/'KinSealStudio/resources/native-shell/tree';directory.mkdir(parents=True,exist_ok=True)
                    result['treeDetails'].append(op({'op':'tree-detailed','hwnd':w['hwnd'],'directory':str(directory).replace('\\','/')}))
            elif name=='main-capture':
                assert win32process.GetWindowThreadProcessId(editor)[1]==pid
                left,top,right,bottom=win32gui.GetWindowRect(editor);width=right-left;height=bottom-top
                hdc=win32gui.GetWindowDC(editor);srcdc=win32ui.CreateDCFromHandle(hdc);memdc=srcdc.CreateCompatibleDC();bitmap=win32ui.CreateBitmap();bitmap.CreateCompatibleBitmap(srcdc,width,height);memdc.SelectObject(bitmap)
                try:
                    ok=ctypes.windll.user32.PrintWindow(editor,memdc.GetSafeHdc(),2);assert ok
                    picture=Image.frombuffer('RGB',(width,height),bitmap.GetBitmapBits(True),'raw','BGRX',0,1);destination=OUT/command['output'];assert not destination.exists();picture.save(destination)
                    result['mainCapture']={'path':destination.name,'size':[width,height],'sha256':hashlib.sha256(destination.read_bytes()).hexdigest(),'method':'PrintWindow owned mainwindow on private desktop; no foreground or global capture'}
                finally:
                    win32gui.DeleteObject(bitmap.GetHandle());memdc.DeleteDC();srcdc.DeleteDC();win32gui.ReleaseDC(editor,hdc)
            elif name=='scene-double':
                time.sleep(1.2)
                ws=op({'op':'batch','hwnds':children});view=next(w['hwnd'] for w in ws if w.get('qt',{}).get('kind')=='myGraphicsView')
                xy=op({'op':'scene-point','hwnd':view,'x':command['x'],'y':command['y']});viewport=xy['viewport'];assert win32process.GetWindowThreadProcessId(viewport)[1]==pid;pos=win32api.MAKELONG(xy['x'],xy['y'])
                for message,flags in [(win32con.WM_LBUTTONDOWN,win32con.MK_LBUTTON),(win32con.WM_LBUTTONUP,0),(win32con.WM_LBUTTONDBLCLK,win32con.MK_LBUTTON),(win32con.WM_LBUTTONUP,0)]:win32gui.PostMessage(viewport,message,flags,pos)
                time.sleep(1.5)
            elif name=='table-select':
                w=widget(command);op({'op':'table-select','hwnd':w['hwnd'],'row':command['row'],'column':command.get('column',0)})
                time.sleep(.3)
            elif name=='table-open':
                w=widget(command)
                op({'op':'table-open','hwnd':w['hwnd'],'row':command['row'],'column':command.get('column',0)})
                time.sleep(.7)
            elif name=='read-tables':
                ws=[w for win in rows_now() for w in win['widgets'] if w.get('qt')]
                result.setdefault('tableReads',[]).append({'label':command.get('label'),'tables':[{'hwnd':w['hwnd'],'objectName':w['qt'].get('objectName'),'cells':op({'op':'table-read','hwnd':w['hwnd']})} for w in ws if w.get('qt',{}).get('kind')=='QTableWidget']})
            elif name=='native-dump':
                time.sleep(2)
                rows=[]
                for top in windows(desk,pid):
                    if not win32gui.IsWindowVisible(top): continue
                    cs=[top]; win32gui.EnumChildWindows(top,lambda h,_:cs.append(h),None)
                    rows.append({'hwnd':top,'title':win32gui.GetWindowText(top),'children':[{'hwnd':h,'class':win32gui.GetClassName(h),'controlId':win32gui.GetDlgCtrlID(h),'text':win32gui.GetWindowText(h),'parentId':win32gui.GetDlgCtrlID(win32gui.GetParent(h))} for h in cs]})
                result['fileApiCalls']=api.file_api_calls()
                result.setdefault('nativeDumps',[]).append({'label':command.get('label'),'windows':rows})
            elif name=='native-file':
                dialogs=[h for h in windows(desk,pid) if win32gui.IsWindowVisible(h) and win32gui.GetClassName(h)=='#32770']
                assert len(dialogs)==1, dialogs
                dialog=dialogs[0]; cs=[]; win32gui.EnumChildWindows(dialog,lambda h,_:cs.append(h),None)
                edits=[h for h in cs if win32gui.GetClassName(h)=='Edit' and (win32gui.GetDlgCtrlID(h)==1148 or win32gui.GetDlgCtrlID(win32gui.GetParent(h))==1148)]
                if not edits: edits=[h for h in cs if win32gui.GetClassName(h)=='Edit' and win32gui.GetDlgCtrlID(h)==1001]
                assert len(edits)==1, edits
                message=ctypes.WinDLL('user32',use_last_error=True).SendMessageW
                message.argtypes=[ctypes.c_void_p,ctypes.c_uint,ctypes.c_size_t,ctypes.c_void_p]; message.restype=ctypes.c_ssize_t
                filename=ctypes.create_unicode_buffer(command['path']); message(edits[0],win32con.WM_SETTEXT,0,ctypes.cast(filename,ctypes.c_void_p))
                readback=ctypes.create_unicode_buffer(4096);message(edits[0],win32con.WM_GETTEXT,4096,ctypes.cast(readback,ctypes.c_void_p));assert readback.value==command['path']
                buttons=[h for h in cs if win32gui.GetClassName(h)=='Button' and win32gui.GetDlgCtrlID(h)==1];assert len(buttons)==1
                win32gui.PostMessage(buttons[0],win32con.BM_CLICK,0,0)
                result.setdefault('fileSelections',[]).append({'dialog':dialog,'path':readback.value,'method':'owned private common dialog WM_SETTEXT+BM_CLICK'})
                time.sleep(1)
            elif name=='screenshot':
                targets=[h for h in windows(desk,pid) if h!=editor and win32gui.IsWindowVisible(h) and win32gui.GetClassName(h)=='QWidget']
                assert len(targets)==1, targets
                target=targets[0]
                assert win32process.GetWindowThreadProcessId(target)[1]==pid
                left,top,right,bottom=win32gui.GetWindowRect(target); width=right-left; height=bottom-top
                hdc=win32gui.GetWindowDC(target); source_dc=win32ui.CreateDCFromHandle(hdc); memory_dc=source_dc.CreateCompatibleDC(); bitmap=win32ui.CreateBitmap(); bitmap.CreateCompatibleBitmap(source_dc,width,height); memory_dc.SelectObject(bitmap)
                try:
                    ok=ctypes.windll.user32.PrintWindow(target,memory_dc.GetSafeHdc(),2)
                    assert ok
                    pixels=bitmap.GetBitmapBits(True)
                    picture=Image.frombuffer('RGB',(width,height),pixels,'raw','BGRX',0,1)
                    destination=OUT/(command['label']+'.png');assert not destination.exists();picture.save(destination)
                    result.setdefault('screenshots',[]).append({'path':destination.name,'hwnd':target,'pid':pid,'width':width,'height':height,'sha256':hashlib.sha256(destination.read_bytes()).hexdigest(),'method':'PrintWindow owned modal on private desktop; no foreground or global capture'})
                finally:
                    win32gui.DeleteObject(bitmap.GetHandle()); memory_dc.DeleteDC(); source_dc.DeleteDC(); win32gui.ReleaseDC(target,hdc)
            elif name=='tree-read':
                ts=op({'op':'batch','hwnds':children})
                th=next(w['hwnd'] for w in ts if w.get('qt',{}).get('kind')=='QTreeWidget')
                op({'op':'invoke','hwnd':th,'slot':'expandAll'})
                result.setdefault('treeReads',[]).append({'label':command.get('label'),'rows':op({'op':'tree','hwnd':th})})
            elif name=='main-slot':
                op({'op':'invoke','hwnd':editor,'slot':command['slot']})
                time.sleep(.4)
            elif name in ('tree-single','tree-double','tree-context'):
                tree_event(command)
            elif name=='context-action':
                candidates=[]
                def walk(a):
                    for x in a:
                        yield x
                        yield from walk(x.get('children',[]))
                for t in windows(desk,pid):
                    if win32gui.IsWindowVisible(t) and win32gui.GetClassName(t)=='QWidget' and t!=editor:
                        try: candidates.extend(walk(op({'op':'menus','hwnd':t})))
                        except Exception: pass
                if 'name' in command: candidates=[c for c in candidates if c.get('objectName')==command['name']]
                if 'text' in command: candidates=[c for c in candidates if c.get('text')==command['text']]
                if len(candidates)!=1: raise RuntimeError('context action not unique '+str(candidates))
                op({'op':'invoke','pointer':candidates[0]['pointer'],'slot':'trigger'})
                time.sleep(.4)
            elif name=='invoke':
                w=widget(command)
                op({'op':'invoke','hwnd':w['hwnd'],'slot':command.get('slot','click')})
                time.sleep(.4)
            elif name in ('text','spin','combo','tab','group'):
                w=widget(command)
                op({'op':name,'hwnd':w['hwnd'],'value':command['value']})
                time.sleep(.15)
            elif name=='save-current':
                action('action_save')
            elif name=='save':
                action('action_save')
                destination=OUT/command['output']
                assert destination.parent==OUT and not destination.exists()
                shutil.copy2(source,destination)
                result.setdefault('outputs',[]).append({'path':destination.name,'bytes':destination.stat().st_size,'sha256':hashlib.sha256(destination.read_bytes()).hexdigest()})
            elif name=='save-as':
                assert index == len(json.loads(args.scenario.read_text(encoding='utf8'))) - 1, 'Save As must finish a scenario so active source tracking remains exact'
                save_as(command['output'])
            else: raise RuntimeError('Unknown command '+name)
        result['createdSources']=[]
        for created in WORK.rglob('*.sm'):
            if created==source: continue
            destination=OUT/('created-'+args.run+'-'+created.name)
            assert not destination.exists()
            shutil.copy2(created,destination)
            result['createdSources'].append({'path':destination.name,'originalRelativePath':str(created.relative_to(WORK)),'bytes':created.stat().st_size,'sha256':hashlib.sha256(created.read_bytes()).hexdigest()})
        result['workingCopyAfter']={'path':str(source),'sha256':hashlib.sha256(source.read_bytes()).hexdigest()}
        result['status']='native-stage3-scenario-completed'
    except Exception as exc:
        result["status"] = "partial-or-error"
        result["error"] = repr(exc)
    finally:
        if process and win32process.GetExitCodeProcess(process) == 259:
            win32process.TerminateProcess(process, 0)
        if session:
            try:
                session.detach()
            except Exception as exc:
                result["detachError"] = repr(exc)
        if process:
            result["exitCodeBeforeCleanup"] = win32process.GetExitCodeProcess(process)
            if result["exitCodeBeforeCleanup"] == 259:
                win32process.TerminateProcess(process, 0)
            process.Close()
            result["ownedProcessTerminated"] = True
        if thread:
            thread.Close()
        u.CloseDesktop(desk)
        try:
            with winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"Software\KinSealStudio\recent pro files", 0, winreg.KEY_QUERY_VALUE | winreg.KEY_SET_VALUE) as key:
                recent, kind = winreg.QueryValueEx(key, "recentFileList")
                own = str(WORK).replace("\\", "/").casefold()+"/"
                exact = {x['destination'].replace('\\', '/').casefold() for x in result.get('saveAs', [])}
                remaining = [x for x in recent if not x.replace("\\", "/").casefold().startswith(own) and x.replace('\\', '/').casefold() not in exact]
                if kind == winreg.REG_MULTI_SZ and len(remaining) != len(recent):
                    winreg.SetValueEx(key, "recentFileList", 0, kind, remaining)
                result["ownMruCleanup"] = {"removed": len(recent) - len(remaining), "otherEntriesPreserved": len(remaining)}
        except OSError as exc:
            result["ownMruCleanup"] = {"error": repr(exc)}
        (OUT / "native-tree-probe.json").write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        (OUT / ('native-tree-probe-' + args.run + '.json')).write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({"status": result["status"], "pid": result.get("pid"), "error": result.get("error"), "evidence": str(OUT / "native-tree-probe.json")}, ensure_ascii=False))


if __name__ == "__main__":
    main()
