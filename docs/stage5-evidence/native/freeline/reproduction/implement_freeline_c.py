"""Admit the independently verified two-endpoint freeline subset only after all canonical fixture comparisons pass."""
from pathlib import Path
import json

root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
evidence=json.loads((work/'docs/stage5-evidence/native/freeline/B-S-M-bresenham-r2.json').read_text(encoding='utf-8'))
canonical=[c for c in evidence['cases'] if 'reopen' in c['run'] or 'default-repeat' in c['run']]
assert len(canonical)==14 and all(c['M']['differentPixels']==0 for c in canonical)
assert len({tuple(tuple(p) for p in c['points']) for c in canonical})==12
assert all(max(abs(c['points'][1][i]-c['points'][0][i]) for i in range(2))>=40 for c in canonical)
def replace(relative,old,new):
    path=work/relative;text=path.read_text(encoding='utf-8-sig');assert text.count(old)==1,(relative,old[:100],text.count(old));path.write_text(text.replace(old,new),encoding='utf-8')
replace('src/device_compile_plan.h','    int staticLineCount = 0; //20261113-14 add: Canonical integer straight lines allocate background pixels only.','    int staticLineCount = 0; //20261113-14 add: Canonical integer straight lines allocate background pixels only.\n    int staticFreelineCount = 0; //20261113-15 add: Only the verified two-endpoint freeline subset is baked into the background.')
replace('src/device_compile_plan.cpp','if (object.type != QStringLiteral("polyline") && object.type != QStringLiteral("line")) //20261113-14 change','if (object.type != QStringLiteral("polyline") && object.type != QStringLiteral("line") && object.type != QStringLiteral("freeline")) //20261113-15 change')
replace('src/device_compile_plan.cpp','if (object.type == QStringLiteral("line") && !first && qRound(absolute.x()) == qRound(previous.x()) && qRound(absolute.y()) == qRound(previous.y())) //20261113-14 add: Distinct normalized doubles may collapse to one device pixel.','if (object.type != QStringLiteral("polyline") && !first && qRound(absolute.x()) == qRound(previous.x()) && qRound(absolute.y()) == qRound(previous.y())) //20261113-15 change: Both admitted two-point families require distinct device pixels.')
replace('src/device_compile_plan.cpp','            previous = absolute; first = false;','''            if (object.type == QStringLiteral("freeline") && !first && qMax(qAbs(qRound(absolute.x())-qRound(previous.x())),qAbs(qRound(absolute.y())-qRound(previous.y()))) < 40) //20261113-15 add
            { return compilationIssue(project,object,QStringLiteral("graphic.sampledSpan"),QJsonArray{absolute.x(),absolute.y()},QStringLiteral("自由直线本切片保守开放主轴跨度至少40像素；小尺寸有效原厂路径未核。"),error,issue); }
            if (object.type == QStringLiteral("freeline") && (absolute.x() < 1 || absolute.y() < 1)) //20261113-15 add: The native boundary source retains a fractional item translation outside this identity-transform slice.
            { return compilationIssue(project,object,QStringLiteral("graphic.sceneBoundary"),QJsonArray{absolute.x(),absolute.y()},QStringLiteral("自由直线本切片仅开放端点X/Y至少1像素；贴上/左边界的原厂小数场景变换待恢复。"),error,issue); }
            previous = absolute; first = false;''')
replace('src/device_compile_plan.cpp','        if (object.type == QStringLiteral("line")) { ++prepared.staticLineCount; } //20261113-14 change\n        else { ++prepared.staticPolylineCount; }','        if (object.type == QStringLiteral("line")) { ++prepared.staticLineCount; } //20261113-15 change\n        else if (object.type == QStringLiteral("freeline")) { ++prepared.staticFreelineCount; }\n        else { ++prepared.staticPolylineCount; }')
replace('src/independent_downloader.cpp','else if (object.type == QStringLiteral("line")) //20261113-14 add: Device pixels use the sampled integer raster rather than the Qt6 preview\'s line ties.','else if (object.type == QStringLiteral("line") || object.type == QStringLiteral("freeline")) //20261113-15 change: The source-bound two-endpoint families share the proven integer raster, without admitting sampled freehand paths.')
path=work/'tests/stage5_compiler_tests.cpp';text=path.read_text(encoding='utf-8-sig')
anchor='    void lineSourceAndPackageGates();'
assert text.count(anchor)==1;text=text.replace(anchor,anchor+'\n    void freelineCanonicalFrames_data(); //20261113-15 add\n    void freelineCanonicalFrames();\n    void freelineUnprovedFieldsPreserveOutput_data();\n    void freelineUnprovedFieldsPreserveOutput();\n    void freelineSourceAndPackageGates();')
helper='''/******************************************************************************
 * Function Name   : freelineProject
 * Description     : Construct only the source-verified two-endpoint freeline subset, without native binary seeds.
 * Input Parameters: None
 * Return Value    : A bounded typed project with the canonical 180-by-100 stroke.
 * Date            : 2026.11.13
 ******************************************************************************/
static HmiProject freelineProject() //20261113-15 add
{
    HmiProject project = lineProject();

    project.objects[0].type = QStringLiteral("freeline"); project.objects[0].id = QStringLiteral("stage5-freeline");
    project.objects[0].name = QStringLiteral("自由直线"); project.objects[0].geometry = QRect(100,100,180,100);
    return project;
}

'''
anchor='/******************************************************************************\n * Function Name   : polylineProject';assert text.count(anchor)==1;text=text.replace(anchor,helper+anchor)
anchor=next(line for line in text.splitlines() if line.startswith('    HmiObject line; //20261113-14'))
text=text.replace(anchor,anchor+'\n    HmiObject freeline; //20261113-15 add: The fifth background family contributes no dynamic records.')
anchor='    QCOMPARE(project.objects.size(),9);';assert text.count(anchor)==1;text=text.replace(anchor,'    freeline = freelineProject().objects.first(); freeline.geometry = QRect(600,350,100,60); project.objects.append(freeline); //20261113-15 add\n    QCOMPARE(project.objects.size(),10);')
start=text.index('/******************************************************************************\n * Function Name   : Stage5CompilerTests::lineCanonicalFrames_data')
end=text.index('QTEST_MAIN(Stage5CompilerTests)',start)
copy=text[start:end].replace('::line','::freeline').replace('lineProject()','freelineProject()').replace('staticLineCount','staticFreelineCount').replace('native/line','native/freeline').replace('B-S-M-bresenham-expanded.json','B-S-M-bresenham-r2.json').replace('//20261113-14 add','//20261113-15 add').replace('variant<14','variant<20')
copy=copy.replace('geometry.moveLeft(600)','geometry.moveLeft(620)').replace('QStringLiteral("line")','QStringLiteral("freeline")')
anchor='    QVERIFY(!prepareDeviceCompilePlan(project,&plan,&error,&issue)); QCOMPARE(plan.staticFreelineCount,77);'
assert copy.count(anchor)==1;copy=copy.replace(anchor,'''    if (variant==14) { project.objects[0].geometry=QRect(100,100,39,39); }
    if (variant==15) { project.objects[0].geometry=QRect(100,100,39,1); project.objects[0].graphic.points={QPointF(0,0),QPointF(1,0)}; }
    if (variant==16) { project.objects[0].geometry=QRect(100,100,1,39); project.objects[0].graphic.points={QPointF(0,0),QPointF(0,1)}; }
    if (variant==17) { project.objects[0].graphic.points={QPointF(0,0),QPointF(.5,.3),QPointF(1,1)}; }
    if (variant==18) { project.objects[0].geometry.moveLeft(0); }
    if (variant==19) { project.objects[0].geometry.moveTop(0); }
'''+anchor)
text=text[:end]+copy+text[end:];path.write_text(text,encoding='utf-8')
print(json.dumps(dict(canonicalFixtures=14,independentCases=12,sampledMinimumMajorSpan=40,usbAccessed=False)))
