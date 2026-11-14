"""Add source-bound G06 ellipse fixtures while the production gate remains closed."""
from pathlib import Path
root=Path(__file__).resolve().parent;work=root.parent/'kinseal-stage4-20261112/worktree'
p=work/'tests/stage5_compiler_tests.cpp';text=p.read_text(encoding='utf-8');assert 'ellipseCanonicalFrames' not in text
text=text.replace('    void polygonSourceAndPackageGates();','    void polygonSourceAndPackageGates();\n    void ellipseCanonicalFrames_data(); //20261114-2 add\n    void ellipseCanonicalFrames();\n    void ellipseUnprovedFieldsPreserveOutput_data();\n    void ellipseUnprovedFieldsPreserveOutput();\n    void ellipseSourceAndPackageGates();')
marker='/******************************************************************************\n * Function Name   : lineProject'
factory='''/******************************************************************************
 * Function Name   : ellipseProject
 * Description     : Build typed ellipse defaults without retaining a native source or original pixels.
 * Input Parameters: None
 * Return Value    : One independently defined black-outline, white-fill ellipse.
 * Date            : 2026.11.14
 ******************************************************************************/
static HmiProject ellipseProject() //20261114-2 add
{
    HmiProject project;
    HmiObject object;

    object.type=QStringLiteral("ellipse");object.id=QStringLiteral("stage5-ellipse");object.name=QStringLiteral("圆/椭圆");
    object.text.clear();object.address.clear();object.geometry=QRect(100,100,200,100);
    object.graphic.foreground=Qt::black;object.graphic.background=Qt::white;project.objects={object};return project;
}

'''
assert marker in text;text=text.replace(marker,factory+marker,1)
tests='''
/******************************************************************************
 * Function Name   : Stage5CompilerTests::ellipseCanonicalFrames_data
 * Description     : Require every native default or reopened fixture, including both formerly failing families.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::ellipseCanonicalFrames_data() //20261114-2 add
{
    QString root=QFINDTESTDATA("../docs/stage5-evidence/native/ellipse");
    QJsonObject corpus=QJsonDocument::fromJson(testFileBytes(QDir(root).filePath(QStringLiteral("B-S-M-ellipse-cell-area-r2.json")))).object();
    QJsonArray cases=corpus.value(QStringLiteral("cases")).toArray();
    QJsonValue value;
    QString run;
    int count=0;

    QVERIFY(corpus.value(QStringLiteral("allRasterComparisonsEqual")).toBool());QCOMPARE(cases.size(),27);
    QTest::addColumn<QJsonObject>("sample");
    Q_FOREACH (value,cases)
    {
        run=value.toObject().value(QStringLiteral("run")).toString();
        if (run.contains(QStringLiteral("reopen")) || run.contains(QStringLiteral("default-repeat")))
        { QTest::newRow(qPrintable(run))<<value.toObject();++count; }
    }
    QCOMPARE(count,15);
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::ellipseCanonicalFrames
 * Description     : Compile fresh typed geometry and compare the complete native RGB565 background and source identity.
 * Input Parameters: Qt sample column
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::ellipseCanonicalFrames() //20261114-2 add
{
    QFETCH(QJsonObject,sample);
    HmiProject project=ellipseProject();
    HmiObject object=project.objects.first();
    DeviceCompilePlan plan;
    QJsonValue value;
    QJsonArray rect;
    QString root=QFINDTESTDATA("..");
    QString folder=QDir(root).filePath(sample.value(QStringLiteral("folder")).toString());
    QByteArray source=testFileBytes(QDir(folder).filePath(QStringLiteral("input-native.sm")));
    QByteArray native=testFileBytes(QDir(folder).filePath(QStringLiteral("SheetPixel.dat")));
    QVector<QByteArray> payloads;
    QString error;

    object.graphic.lineStyle=sample.value(QStringLiteral("lineStyle")).toInt();object.graphic.fillStyle=sample.value(QStringLiteral("fillStyle")).toInt();project.objects.clear();
    Q_FOREACH (value,sample.value(QStringLiteral("rects")).toArray())
    {
        rect=value.toArray();QCOMPARE(rect.size(),4);object.geometry=QRect(rect[0].toInt(),rect[1].toInt(),rect[2].toInt(),rect[3].toInt());
        object.id=QStringLiteral("ellipse-%1").arg(project.objects.size());project.objects.append(object);
    }
    QVERIFY(!source.isEmpty());QCOMPARE(QString::fromLatin1(QCryptographicHash::hash(source,QCryptographicHash::Sha256).toHex()),sample.value(QStringLiteral("sourceSha256")).toString());
    QCOMPARE(decodeTestFrame(native).size(),768000);
    QVERIFY2(prepareDeviceCompilePlan(project,&plan,&error),qPrintable(error));QCOMPARE(plan.staticEllipseCount,project.objects.size());
    QVERIFY2(buildProjectPayload(project,&payloads,&error),qPrintable(error));QCOMPARE(payloads.size(),10);QCOMPARE(decodeTestFrame(payloads[8]),decodeTestFrame(native));
    QCOMPARE(payloads[5].size(),2400);QCOMPARE(int(static_cast<unsigned char>(payloads[1][2])),0);QVERIFY2(validateDevicePayloadStructure(payloads,&error),qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::ellipseUnprovedFieldsPreserveOutput_data
 * Description     : Enumerate geometry, appearance, dormant drafts and composition guard boundaries.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::ellipseUnprovedFieldsPreserveOutput_data() //20261114-2 add
{
    int variant;
    QTest::addColumn<int>("variant");
    for (variant=0;variant<27;++variant) { QTest::newRow(qPrintable(QString::number(variant)))<<variant; }
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::ellipseUnprovedFieldsPreserveOutput
 * Description     : Preserve caller plans and payloads for each unsupported ellipse configuration.
 * Input Parameters: Qt variant column
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::ellipseUnprovedFieldsPreserveOutput() //20261114-2 add
{
    QFETCH(int,variant);
    HmiProject project=ellipseProject();
    HmiObject peer;
    DeviceCompilePlan plan;
    QJsonObject issue;
    QVector<QByteArray> payloads={QByteArrayLiteral("keep")};
    QString error;

    plan.staticEllipseCount=77;
    if (variant==0) { project.objects[0].graphic.lineStyle=2; }
    if (variant==1) { project.objects[0].graphic.lineWidth=2; }
    if (variant==2) { project.objects[0].graphic.alpha=128; }
    if (variant==3) { project.objects[0].graphic.lineColor=Qt::red; }
    if (variant==4) { project.objects[0].graphic.fillStyle=2; }
    if (variant==5) { project.objects[0].graphic.background=Qt::black; }
    if (variant==6) { project.objects[0].graphic.foreground=Qt::white; }
    if (variant==7) { project.objects[0].geometry.setWidth(99); }
    if (variant==8) { project.objects[0].geometry.setHeight(99); }
    if (variant==9) { project.objects[0].geometry.moveLeft(1); }
    if (variant==10) { project.objects[0].geometry.moveTop(1); }
    if (variant==11) { project.objects[0].geometry.moveLeft(499); }
    if (variant==12) { project.objects[0].geometry.moveTop(279); }
    if (variant==13) { project.objects[0].graphic.endArrow=1; }
    if (variant==14) { project.objects[0].graphic.endCap=1; }
    if (variant==15) { peer=project.objects[0];peer.id=QStringLiteral("overlap");project.objects.append(peer); }
    if (variant==16) { peer=project.objects[0];peer.id=QStringLiteral("touching");peer.geometry.moveLeft(300);project.objects.append(peer); }
    if (variant==17) { project.objects[0].graphic.lineStyle=0;project.objects[0].graphic.fillStyle=0;project.objects[0].graphic.background=Qt::red; }
    if (variant==18) { project.objects[0].graphic.lineStyle=0;project.objects[0].graphic.lineColor=Qt::red; }
    if (variant==19) { project.objects[0].graphic.fillStyle=0;project.objects[0].graphic.foreground=Qt::white; }
    if (variant==20) { project.objects[0].geometry.moveLeft(700); }
    if (variant==21) { project.objects[0].geometry.moveTop(400); }
    if (variant==22) { project.targetModel=QStringLiteral("unknown"); }
    if (variant==23) { project.pageWidth=801; }
    if (variant==24) { peer=project.objects[0];peer.type=QStringLiteral("rect");peer.id=QStringLiteral("rectangle-peer");project.objects.append(peer); }
    if (variant==25) { project.objects[0].graphic.lineStyle=0;project.objects[0].graphic.fillStyle=0;project.objects[0].graphic.lineWidth=2; }
    if (variant==26) { project.objects[0].graphic.lineStyle=0;project.objects[0].graphic.fillStyle=0;project.objects[0].graphic.alpha=0; }
    QVERIFY(!prepareDeviceCompilePlan(project,&plan,&error,&issue));QCOMPARE(plan.staticEllipseCount,77);QVERIFY(!issue.value(QStringLiteral("property")).toString().isEmpty());
    QVERIFY(!buildProjectPayload(project,&payloads,&error));QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::ellipseSourceAndPackageGates
 * Description     : Keep complete native source and the legacy compiler separate from verified device publication.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::ellipseSourceAndPackageGates() //20261114-2 add
{
    HmiProject project=ellipseProject();
    QTemporaryDir temporary;
    QString target=temporary.filePath(QStringLiteral("ellipse"));
    QByteArray source=QByteArrayLiteral("keep");
    QVector<QByteArray> payloads={QByteArrayLiteral("keep")};
    QJsonObject manifest;
    QString error;

    QVERIFY(!exportNativeProjectSource(project,&source,&error));QCOMPARE(source,QByteArrayLiteral("keep"));
    QVERIFY(!buildSingleTextPayload(project,&payloads,&error));QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
    QVERIFY2(exportProjectPayload(project,target,&error),qPrintable(error));QVERIFY2(verifyDevicePackage(target,&manifest,&error),qPrintable(error));
    QCOMPARE(manifest.value(QStringLiteral("files")).toObject().size(),10);QCOMPARE(manifest.value(QStringLiteral("deviceAcknowledged")).toBool(true),false);
}

'''
text=text.replace('QTEST_MAIN(Stage5CompilerTests)',tests+'QTEST_MAIN(Stage5CompilerTests)');p.write_text(text,encoding='utf-8')
p=work/'src/device_compile_plan.h';text=p.read_text(encoding='utf-8');text=text.replace('    int staticPolygonCount=0;','    int staticEllipseCount=0; //20261114-2 add: Count only source-verified ellipse background semantics.\n    int staticPolygonCount=0;');p.write_text(text,encoding='utf-8')
p=work/'CMakeLists.txt';text=p.read_text(encoding='utf-8');text=text.replace('    src/device_polygon_raster.h','    src/device_polygon_raster.h\n    src/device_ellipse_raster.cpp #20261114-2 add: Independent fixed-point ellipse geometry.\n    src/device_ellipse_raster.h');p.write_text(text,encoding='utf-8')
print('Added15 canonical,27 negative and source/package tests; ellipse production gate is still closed.')
