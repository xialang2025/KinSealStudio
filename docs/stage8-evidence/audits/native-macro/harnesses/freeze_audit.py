"""Freeze stage 8 native macro UI evidence; do not delete runtime files."""
#20261114-7 add
from pathlib import Path
import gzip, hashlib, json, shutil, time

BASE=Path(__file__).resolve().parent
ROOT=BASE.parent

def sha(path):
    """Identify exact retained source bytes."""
    return hashlib.sha256(path.read_bytes()).hexdigest()

def save(path, value):
    """Save a new audit projection without changing historical evidence."""
    path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")

def load(path):
    """Read one completed immutable run record."""
    return json.loads(path.read_text(encoding="utf-8"))

def normalize(value):
    """Decode a separate display view of old Qt local-8-bit strings."""
    if isinstance(value,str):
        try:return value.encode("latin1").decode("gbk")
        except UnicodeError:return value
    if isinstance(value,list):return [normalize(x) for x in value]
    if isinstance(value,dict):return {k:normalize(v) for k,v in value.items()}
    return value

def tree_order(run, label):
    """Extract one explicitly recorded macro tree without inferring package order."""
    data=normalize(load(BASE/run/"run.json"))
    read=next(x for x in data["reads"] if x["label"]==label)
    rows=next(x["rows"] for x in read["widgets"] if x["kind"]=="QTreeWidget" and x["rows"] and x["rows"][0]["text"]=="宏")
    return [x["text"] for x in rows[1:]]

def main():
    """Freeze selected evidence, all failures and an unexecuted cleanup plan."""
    preserved=BASE/"preserved-evidence";preserved.mkdir(exist_ok=True)
    source_store=preserved/"source-inputs";source_store.mkdir(exist_ok=True)
    run_store=preserved/"run-records";run_store.mkdir(exist_ok=True)
    runs=[];unique_sources={};cleanup=[]
    for path in sorted(BASE.glob("*/run.json")):
        record=load(path)
        assert record.get("cleanup",{}).get("terminated"),path
        archive=run_store/(path.parent.name+"-run.json.gz")
        archive.write_bytes(gzip.compress(path.read_bytes(),mtime=0))
        harness=path.parent/"harness-source.py"
        trace=record.get("trace",{}).get("events",[])
        launched=[x for x in trace if x.get("api","").startswith("CreateProcess")]
        assert all(x.get("blocked") for x in launched),path
        item={"name":path.parent.name,"path":str(path),"sha256":sha(path),"bytes":path.stat().st_size,"archive":str(archive),"archiveSha256":sha(archive),"archiveBytes":archive.stat().st_size,"status":record["status"],"error":record.get("error"),"harnessSha256":sha(harness),"input":record.get("input"),"cleanup":record["cleanup"],"nativeChildLaunchRequests":launched,"fridaErrorCount":len(record.get("fridaErrors",[])),"traceOverflow":record.get("trace",{}).get("overflow",0),"loadedHardwareModules":record.get("loadedHardwareModules",[])}
        runs.append(item)
        source=Path(record["input"]["path"])
        digest=sha(source)
        if digest not in unique_sources:
            destination=source_store/(digest+".sm")
            if not destination.exists():shutil.copy2(source,destination)
            unique_sources[digest]={"path":str(destination),"sha256":digest,"bytes":destination.stat().st_size,"sourceRun":path.parent.name}
        for file in (path.parent/"runtime").rglob("*"):
            if file.is_file():cleanup.append({"path":str(file),"logicalBytes":file.stat().st_size,"category":"owned runtime; EXE/plugins are hard links, so length is not physical reclaimable space","retainedSourceSha256":digest if file==source else None})
        for file in [path,path.parent/"projection-zh.json"]:
            if file.exists():cleanup.append({"path":str(file),"logicalBytes":file.stat().st_size,"category":"raw evidence duplicate; compressed exact run record retained","retainedRunArchive":str(archive)})
    examples_by_name={};order=[]
    for run in ["examples-r2","examples-r3"]:
        source=BASE/run/"library-examples.json"
        raw=load(source)
        for row in raw["examples"]:
            if row["name"] not in order:order.append(row["name"])
            if row["status"]!="inserted":continue
            assert row["seedPreserved"] and row["insertedText"]
            digest=hashlib.sha256(row["insertedText"].encode()).hexdigest()
            assert digest==row["insertedTextUtf8Sha256"]
            examples_by_name[row["name"]]={"name":row["name"],"group":row["group"],"text":row["insertedText"],"utf8Sha256":digest,"sourceRun":run,"sourcePath":str(source),"sourceSha256":sha(source),"cursor":row["cursor"],"selection":row["selection"],"operation":"owned native tree right click -> real QAction 插入例子", "compilerValidated":False,"deviceExecuted":False}
    assert len(examples_by_name)==36
    examples={"schema":"kinseal-native-library-examples-v1","createdEpoch":time.time(),"nativeExecutableSha256":"305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c","privateExecutableSha256":"38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89","count":36,"functionCount":28,"graphicsCount":8,"semantics":"Exact current V5.1 UI insertion text at cursor (0,0), no selected range. Preserve native spelling and comments; this catalogue is not a compile or correctness oracle.","examples":[examples_by_name[name] for name in order]}
    save(BASE/"native-library-examples.json",examples)
    selectors={
      "S08.editor.entry":{"action":"action_script","kind":"MacroEdit","title":"宏指令编辑器","evidence":"editor-r4"},
      "S08.editor.usageEntry":{"action":"action_macro_admin","kind":"MarcoUseDisplay","title":"宏指令使用一览表","fields":{"tableColumns":3,"allScreensDefault":True,"specifiedScreenDefault":False},"evidence":"admin-r1","isOrdinaryEditor":False},
      "S08.editor.tabs":{"left":["宏","函数库"],"bottom":["信息输出","变量设定(0/25)"],"evidence":"editor-r4"},
      "S08.editor.new":{"kind":"AddLib","title":"添加宏","nameDefault":"","qtMaxLength":32767,"blankAcceptableInput":False,"validator":"QRegExpValidator; pattern not recovered","buttons":["确定","取消"],"evidence":"editor-r4/editor-r5"},
      "S08.editor.context":{"macro":["编辑","删除","重命名"],"library":["插入例子"],"evidence":"context-r1/example-r1"},
      "S08.editor.rename":{"title":"修改宏名称","kind":"AddLib","prefill":"SetLB1","buttons":["确定","取消"],"evidence":"rename-r1"},
      "S08.editor.delete":{"title":"确认","message":"是否删除：SetLB1？","buttons":["删除","取消"],"confirmedDeletionPerformed":False,"referenceMigrationKnown":False,"evidence":"delete-r1"},
      "S08.editor.closeDirty":{"title":"未保存","message":"是否保存代码到宏：SetLB1","buttons":["保存","不保存","取消"],"cancelVerified":"Editor remains open with modified LocalBit[1]=2 draft","discardVerified":"Editor closes and reopens with original LocalBit[1]=1 source","saveBranchVerified":False,"evidence":"edit-close-r1/edit-close-r2"},
      "S08.editor.variableTable":{"headers":["变量名","数据类别","地址长度","读写属性","地址类型","地址"],"zeroVariableRow":["新建",None,None,None,None,None],"advertisedCount":"0/25","enforcedMaximumTested":False,"evidence":"editor-r4"},
      "S08.editor.variableNew":{"kind":"varSettingWidget","title":"变量设定","dataTypes":["位 ","16位整数","16位正整型","32位整数","32位正整型","16位BCD码","32位BCD码","32位浮点型数"],"dataTypeDefaultIndex":0,"addressDefault":"LB0","nameDefault":"","lengthDefault":"1","lengthDefaultEnabled":False,"readWriteItems":["读","写","读/写"],"readWriteDefaultIndex":2,"evidence":"variable-r2","entryCaveat":"onNewVar() is a verified MacroEdit slot; an earlier generic cellDoubleClicked probe did not open it"},
      "S08.scheduler.init":{"action":"action_InitMacro","kind":"initScriptDlg","title":"初始化宏管理器","header":"宏指令名称 0/16","buttons":["新增","删除","修改","退出","帮助"],"newKind":"newInitScriptDlg","newTitle":"新建初始化宏","newButtons":["写宏","确定","取消"],"newComboItems":[],"emptyComboScope":"uncompiled macro-bound seed; no compilation performed","evidence":"init-r1/init-r2","enforcedMaximumTested":False},
      "S08.scheduler.global":{"action":"action_GlobalMacro","kind":"globalScriptDlg","title":"全局宏管理器","advertisedCount":"0/64","headers":["宏指令名称","频率","执行次数","是否受控触发","受控类型","受控地址"],"newKind":"newGlobalScriptDlg","newTitle":"新建全局宏","frequency":{"default":1,"minimum":1,"maximum":65535,"unitLabel":"x100MS"},"executionCount":{"default":0,"minimum":0,"maximum":65535,"zeroLabel":"0表示一直执行"},"controlledTrigger":{"default":False,"groupEnabled":False,"address":"LB0","typeItems":["位地址"]},"newComboItems":[],"evidence":"global-r1/global-r2","runtimeTimingVerified":False,"enforcedMaximumTested":False}
    }
    save(BASE/"native-ui-fields.json",selectors)
    ordering={"creationOrder":tree_order("order-r1","created-order"),"editorReopenOrder":tree_order("order-r1","reopened-order"),"newProcessSourceReopenOrder":tree_order("order-r2","persisted-order-new-process"),"sourceSha256":load(BASE/"order-r1/run.json")["input"]["finalSha256"],"compilerOrderKnown":False,"conclusion":"For this three-macro sample, tree order persists in creation order across main save and a fresh native process; this does not establish MarcoRun/MarcoVar or binding-index order."}
    save(BASE/"native-name-order.json",ordering)
    failures=[{"run":r["name"],"error":r["error"]} for r in runs if r["status"]!="observed"]
    failures.append({"run":"examples-r2","failedItems":["LOBYTE","LOWORD"],"reason":"No popup observed after background scrolling; successful isolated retries in examples-r3","nativeDefectClaim":False})
    unresolved=[
      {"id":"S08.name-validation","unknown":"Name regexp, collision/case/whitespace rules, import format and cancellation","nextProbe":"One owned name-dialog case at a time; capture validator/message and save/reopen source."},
      {"id":"S08.references","unknown":"Committed rename/deletion of a referenced macro, source binding migration and undo","nextProbe":"Make a verified referenced macro seed; confirm rename, save/reopen and compare binding fields; keep device/SDK blocked."},
      {"id":"S08.compile-order","unknown":"Compiled package/index order, failed/disabled macro filtering","nextProbe":"Separate authorized SDK experiment with swapped names/code and explicit package parser; UI order is not package order."},
      {"id":"S08.variables","unknown":"All type-dependent length/address controls, accepted 0..25 boundary, generated 904/36 byte layout and read/write exchange","nextProbe":"One variable type/address tuple per native save/compile artifact; current capture is UI fields only."},
      {"id":"S08.schedule","unknown":"Initialization/global source representation, compiled flags, frequency runtime, reentry/queueing and page scheduling","nextProbe":"One scheduling-field source-only delta, then independently authorized compilation and later device test."},
      {"id":"S08.insertion-selection","unknown":"Selected-range replacement, nonzero cursor indentation and undo for each library example","nextProbe":"Use three focused cursor/selection/undo UI cases; catalogue currently covers cursor0 with no selection."},
      {"id":"S08.runtime","unknown":"Macro execution, hardware acceptance and drawing function effects","nextProbe":"Stage 10 separately authorized controlled hardware verification; never infer from UI or SDK machine code."}
    ]
    audit={"schema":"kinseal-stage8-native-macro-audit-v1","status":"frozen-ui-evidence","createdEpoch":time.time(),"owner":"/root/polygon_docs","scope":"ordinary native macro editor, variable and scheduling UI plus exact library examples; no production source/docs/Git edits","modificationTag":"//20261114-7","boundaries":{"privateDesktopOnly":True,"globalInput":False,"sdkExecuted":False,"hardwareTransferExecuted":False,"allNativeChildLaunchesBlocked":True,"historicalDeletionRetried":False,"filesDeleted":0,"sourceInputsOnlyOwnedCopiesMutated":True},"identity":{"originalExecutable":"G:\\01_Professional\\触摸屏\\金玺\\KinSealStudio_Setup_V5.1\\Installed\\KinSealStudio5.1.exe","originalSha256":"305592b4e4bc1b15a707c7bc832c31f27ca0958b97f9c79a5b50ac6df49e907c","privateSha256":"38100bf47294342119560dadfbd6a35d6ab51180c0eeaf4b7b21517bea20ed89","privatePatch":{"fileOffset":"0x7c4","before":"55 89 e5","after":"b0 01 c3"},"hardwareBlockersSource":"stage7 native-startup/preserved-evidence/hardware-blockers; exact module paths recorded per run"},"runs":runs,"runCount":len(runs),"allOwnedProcessesTerminated":all(r["cleanup"]["terminated"] for r in runs),"uiDictionary":{"path":str(BASE/"native-ui-fields.json"),"sha256":sha(BASE/"native-ui-fields.json")},"libraryCatalogue":{"path":str(BASE/"native-library-examples.json"),"sha256":sha(BASE/"native-library-examples.json"),"count":36},"nameOrder":{"path":str(BASE/"native-name-order.json"),"sha256":sha(BASE/"native-name-order.json"),"facts":ordering},"uniqueInputs":list(unique_sources.values()),"failures":failures,"unknowns":unresolved,"cleanupPlan":{"executed":False,"deletions":0,"reason":"Parent reports automatic policy rejection of previous cleanup. No deletion retried; candidates are informational only.","candidateCount":len(cleanup),"logicalBytes":sum(x["logicalBytes"] for x in cleanup),"physicalReclaimableBytesKnown":False,"retainedRunArchives":len(runs),"retainedUniqueSourceCount":len(unique_sources),"candidateManifest":str(BASE/"cleanup-candidates.json")}}
    save(BASE/"cleanup-candidates.json",cleanup)
    save(ROOT/"stage8-native-macro-audit.json",audit)
    print(json.dumps({"runCount":len(runs),"failures":failures,"libraryCount":36,"audit":str(ROOT/"stage8-native-macro-audit.json"),"auditSha256":sha(ROOT/"stage8-native-macro-audit.json"),"order":ordering,"cleanupCandidates":len(cleanup),"logicalCandidateBytes":sum(x["logicalBytes"] for x in cleanup),"deleted":0},ensure_ascii=False))

if __name__=="__main__":main()
