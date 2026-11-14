#include "device_compile_plan.h"
#include "device_polygon_raster.h" //20261114-2 add: Freeze the two independent path precision contracts.
#include "device_package.h"
#include "independent_downloader.h"
#include "project_capabilities.h"
#include "native_project_codec.h"
#include "lzokay.hpp"
#include <QDir>
#include <QCryptographicHash> //20261113-14 add
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QtEndian>
#include <QtTest>

//20261113-11 add: Only pure compiler and filesystem APIs are invoked by this target.
class Stage5CompilerTests : public QObject
{
    Q_OBJECT
private slots:
    void polylineFullNativeRgb565Equality_data();
    void polylineFullNativeRgb565Equality();
    void unprovedFieldsPreservePlanAndPayload_data();
    void unprovedFieldsPreservePlanAndPayload();
    void sourceAndDeviceGatesRemainIndependent();
    void layerOrderAndStartupLanguageStayIndependent();
    void mixedExistingFiveAndPolylineCloseIndexes();
    void twoPolylinesMatchReopenedNativeFrame();
    void publicationVerifiesAndRefusesExistingDestination();
    void failedWritesNeverPublish_data();
    void failedWritesNeverPublish();
    void occupiedDestinationDuringPublishIsPreserved();
    void verifierRejectsStaleFilesHashAndPlanTampering();
    void validatorRejectsRehashedIndexCorruption();
    void validatorRejectsUnknownRecordsAndBrokenFrames_data();
    void validatorRejectsUnknownRecordsAndBrokenFrames();
    void realMacroFourteenSegmentPackage();
    void snapshotCorruptionAndFalseIdentityAreRejected();
    void legacyTextCompilerRejectsPolyline();
    void tableFullNativeRgb565Equality_data(); //20261113-12 add
    void tableFullNativeRgb565Equality();
    void tableUnprovedFieldsPreserveOutput_data();
    void tableUnprovedFieldsPreserveOutput();
    void tablePackageAndSourceGates();
    void rectangleCanonicalFrames_data(); //20261113-13 add
    void rectangleCanonicalFrames();
    void rectangleUnprovedFieldsPreserveOutput_data();
    void rectangleUnprovedFieldsPreserveOutput();
    void rectangleSourceAndPackageGates();
    void lineCanonicalFrames_data(); //20261113-14 add
    void lineCanonicalFrames();
    void lineUnprovedFieldsPreserveOutput_data();
    void lineUnprovedFieldsPreserveOutput();
    void lineSourceAndPackageGates();
    void polygonCanonicalFrames_data(); //20261114-1 add
    void polygonCanonicalFrames();
    void polygonUnprovedFieldsPreserveOutput_data();
    void polygonUnprovedFieldsPreserveOutput();
    void polygonSourceAndPackageGates();
    void ellipseCanonicalFrames_data(); //20261114-2 add
    void ellipseCanonicalFrames();
    void ellipseUnprovedFieldsPreserveOutput_data();
    void ellipseUnprovedFieldsPreserveOutput();
    void ellipseSourceAndPackageGates();
    void ellipseSharedPrecisionGuards();
    void sectorCanonicalFrames_data(); //20261114-3 add
    void sectorCanonicalFrames();
    void sectorUnprovedFieldsPreserveOutput_data();
    void sectorUnprovedFieldsPreserveOutput();
    void sectorSourceAndPackageGates();

    void roundrectCanonicalFrames_data(); //20261113-16 add
    void roundrectCanonicalFrames();
    void roundrectUnprovedFieldsPreserveOutput_data();
    void roundrectUnprovedFieldsPreserveOutput();
    void roundrectSourceAndPackageGates();
    void freelineCanonicalFrames_data(); //20261113-15 add
    void freelineCanonicalFrames();
    void freelineUnprovedFieldsPreserveOutput_data();
    void freelineUnprovedFieldsPreserveOutput();
    void freelineSourceAndPackageGates();
};

/******************************************************************************
 * Function Name   : polygonProject
 * Description     : Construct typed default vertices without a native source or baked image.
 * Input Parameters: None
 * Return Value    : One black simple triangle on the admitted target.
 * Date            : 2026.11.14
 ******************************************************************************/
static HmiProject polygonProject() //20261114-1 add
{
    HmiProject project;
    HmiObject object;

    object.type=QStringLiteral("polygon");object.id=QStringLiteral("stage5-polygon");object.name=QStringLiteral("多边形");
    object.text.clear();object.address.clear();object.geometry=QRect(100,100,200,120);
    object.graphic.foreground=Qt::white;object.graphic.background=Qt::black;
    object.graphic.points={QPointF(0,0),QPointF(1,0),QPointF(.75,1)};project.objects={object};return project;
}

/******************************************************************************
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

/******************************************************************************
 * Function Name   : sectorProject
 * Description     : Build typed sector defaults without retaining a native source or original pixels.
 * Input Parameters: None
 * Return Value    : One independently defined open default-angle, white-fill sector.
 * Date            : 2026.11.14
 ******************************************************************************/
static HmiProject sectorProject() //20261114-3 add
{
    HmiProject project;
    HmiObject object;

    object.type=QStringLiteral("sector");object.id=QStringLiteral("stage5-sector");object.name=QStringLiteral("扇形");
    object.text.clear();object.address.clear();object.geometry=QRect(100,100,200,200);
    object.graphic.lineStyle=0;object.graphic.foreground=Qt::black;object.graphic.background=Qt::white;project.objects={object};return project;
}

/******************************************************************************
 * Function Name   : lineProject
 * Description     : Construct the independently sampled typed straight-line default without native source bytes.
 * Input Parameters: None
 * Return Value    : One page with one black integer diagonal stroke.
 * Date            : 2026.11.13
 ******************************************************************************/
static HmiProject lineProject() //20261113-14 add
{
    HmiProject project;
    HmiObject object;

    object.type = QStringLiteral("line"); object.id = QStringLiteral("stage5-line"); object.name = QStringLiteral("直线");
    object.text.clear(); object.address.clear(); object.geometry = QRect(100,100,200,100);
    object.graphic.points = {QPointF(0,0),QPointF(1,1)}; project.objects = {object};
    return project;
}

/******************************************************************************
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

/******************************************************************************
 * Function Name   : polylineProject
 * Description     : Reconstruct the independently observed three-point native default in a fresh Lite model.
 * Input Parameters: None
 * Return Value    : One ordinary page with one opaque black orthogonal polyline.
 * Date            : 2026.11.13
 ******************************************************************************/
static HmiProject polylineProject() //20261113-11 add
{
    HmiProject project;
    HmiObject object;

    object.type = QStringLiteral("polyline"); object.id = QStringLiteral("stage5-polyline"); object.name = QStringLiteral("折线");
    object.text.clear(); object.address.clear(); object.geometry = QRect(100,100,150,100);
    object.graphic.points = {QPointF(0,0),QPointF(1,0),QPointF(1,1)};
    project.objects = {object};
    return project;
}

/******************************************************************************
 * Function Name   : tableProject
 * Description     : Construct a fresh pure-image table without importing opaque native bytes.
 * Input Parameters: None
 * Return Value    : One ordinary page with the independently observed default table.
 * Date            : 2026.11.13
 ******************************************************************************/
static HmiProject tableProject() //20261113-12 add
{
    HmiProject project;
    HmiObject object;

    object.type = QStringLiteral("table"); object.id = QStringLiteral("stage5-table"); object.name = QStringLiteral("表格");
    object.text.clear(); object.address.clear(); object.geometry = QRect(100,100,200,100);
    object.graphic.background = QColor(QStringLiteral("#e0e0e0"));
    project.objects = {object};
    return project;
}

/******************************************************************************
 * Function Name   : rectangleProject
 * Description     : Construct a typed closed rectangle without native seeds or retained binary fields.
 * Input Parameters: None
 * Return Value    : One ordinary page with a black outline and opaque white fill.
 * Date            : 2026.11.13
 ******************************************************************************/
static HmiProject rectangleProject() //20261113-13 add
{
    HmiProject project;
    HmiObject object;

    object.type = QStringLiteral("rect"); object.id = QStringLiteral("stage5-rectangle"); object.name = QStringLiteral("矩形");
    object.text.clear(); object.address.clear(); object.geometry = QRect(100,100,200,100); project.objects = {object};
    return project;
}

/******************************************************************************
 * Function Name   : testFileBytes
 * Description     : Read a retained fixture or an actual published package file for comparisons.
 * Input Parameters: path
 * Return Value    : Exact bytes, or an empty array on read failure.
 * Date            : 2026.11.13
 ******************************************************************************/
static QByteArray testFileBytes(const QString &path) //20261113-11 add
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) { return QByteArray(); }
    return file.readAll();
}

/******************************************************************************
 * Function Name   : testWriteBytes
 * Description     : Use the actual Qt file transaction in failure-seam test writers.
 * Input Parameters: path, bytes, error
 * Return Value    : True after a complete file commit.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool testWriteBytes(const QString &path, const QByteArray &bytes, QString *error) //20261113-11 add
{
    QSaveFile file(path);
    Q_UNUSED(error)
    return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size() && file.commit();
}

/******************************************************************************
 * Function Name   : decodeTestFrame
 * Description     : Independently decode complete native and product background bytes for pixel equality.
 * Input Parameters: bytes
 * Return Value    : Packed 800 by 480 RGB565, or an empty array on failure.
 * Date            : 2026.11.13
 ******************************************************************************/
static QByteArray decodeTestFrame(const QByteArray &bytes) //20261113-11 add
{
    QByteArray frame(768000,char(0));
    std::size_t actual = 0;
    if (lzokay::decompress(reinterpret_cast<const unsigned char *>(bytes.constData()),std::size_t(bytes.size()),reinterpret_cast<unsigned char *>(frame.data()),std::size_t(frame.size()),actual) != lzokay::EResult::Success || actual != 768000) { return QByteArray(); }
    return frame;
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::polylineFullNativeRgb565Equality_data
 * Description     : Bind canonical saved-and-reopened native sources to independent product geometries.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::polylineFullNativeRgb565Equality_data() //20261113-11 add
{
    QTest::addColumn<QRect>("geometry"); QTest::addColumn<QString>("fixture");
    QTest::newRow("default-reopened") << QRect(100,100,150,100) << QStringLiteral("repeat-r2");
    QTest::newRow("shifted-sized-reopened") << QRect(75,65,130,120) << QStringLiteral("shifted-reopen-r1");
    QTest::newRow("one-pixel-reopened") << QRect(100,100,1,1) << QStringLiteral("tiny-reopen-r1");
    QTest::newRow("width-only-reopened") << QRect(100,100,200,100) << QStringLiteral("width-reopen-r1");
    QTest::newRow("screen-boundary-reopened") << QRect(0,0,130,120) << QStringLiteral("boundary-reopen-r1");
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::polylineFullNativeRgb565Equality
 * Description     : Compare complete canonical compiled pixels, including the saved-and-reopened flat endpoint.
 * Input Parameters: Qt fixture geometry, fixture
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::polylineFullNativeRgb565Equality() //20261113-11 add
{
    QFETCH(QRect,geometry);
    QFETCH(QString,fixture);
    HmiProject project = polylineProject();
    QVector<QByteArray> payloads;
    QByteArray native;
    QString error;
    QString root = QFINDTESTDATA("../docs/stage5-evidence/native/polyline");

    project.objects[0].geometry = geometry;
    native = testFileBytes(QDir(root).filePath(fixture+QStringLiteral("/compiled/fresh-process-3/SheetPixel.dat")));
    QVERIFY(!native.isEmpty()); QVERIFY2(buildProjectPayload(project,&payloads,&error),qPrintable(error)); QCOMPARE(payloads.size(),10);
    QCOMPARE(decodeTestFrame(payloads[8]),decodeTestFrame(native)); QCOMPARE(decodeTestFrame(native).size(),768000);
    QCOMPARE(int(static_cast<unsigned char>(payloads[1][2])),0); QCOMPARE(payloads[5].size(),2400);
    QVERIFY2(validateDevicePayloadStructure(payloads,&error),qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::unprovedFieldsPreservePlanAndPayload_data
 * Description     : Enumerate independently meaningful unproved rendering, topology and overlap boundaries.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::unprovedFieldsPreservePlanAndPayload_data() //20261113-11 add
{
    int index;
    QTest::addColumn<int>("variant");
    for (index=0;index<12;++index) { QTest::newRow(qPrintable(QString::number(index))) << index; }
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::unprovedFieldsPreservePlanAndPayload
 * Description     : Reject unsupported combinations with a locatable issue and unchanged caller outputs.
 * Input Parameters: Qt fixture variant
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::unprovedFieldsPreservePlanAndPayload() //20261113-11 add
{
    QFETCH(int,variant);
    HmiProject project = polylineProject();
    DeviceCompilePlan plan;
    QJsonObject issue;
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    HmiObject peer;
    QString error;

    plan.staticPolylineCount = 77;
    if (variant == 0) { project.objects[0].graphic.lineStyle = 2; }
    if (variant == 1) { project.objects[0].graphic.lineWidth = 2; }
    if (variant == 2) { project.objects[0].graphic.alpha = 128; }
    if (variant == 3) { project.objects[0].graphic.lineColor = Qt::red; }
    if (variant == 4) { project.objects[0].graphic.endArrow = 1; }
    if (variant == 5) { project.objects[0].graphic.endCap = 1; }
    if (variant == 6) { project.objects[0].graphic.points = {QPointF(0,0),QPointF(1,1)}; }
    if (variant == 7) { project.objects[0].graphic.points = {QPointF(0,0),QPointF(0.333,0),QPointF(0.333,1)}; }
    if (variant == 8) { project.objects[0].geometry.moveLeft(750); }
    if (variant == 9) { peer = project.objects[0]; peer.id = QStringLiteral("overlap"); project.objects.append(peer); }
    if (variant == 10) { project.objects[0].type = QStringLiteral("unrecovered-control"); } //20261114-3 change: Keep the unknown-type case independent of newly admitted graphical families.
    if (variant == 11) { project.objects[0].geometry = QRect(650,100,150,100); }
    QVERIFY(!prepareDeviceCompilePlan(project,&plan,&error,&issue)); QCOMPARE(plan.staticPolylineCount,77);
    QVERIFY(!issue.value(QStringLiteral("property")).toString().isEmpty()); QVERIFY(!error.isEmpty());
    QVERIFY(!buildProjectPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::sourceAndDeviceGatesRemainIndependent
 * Description     : Allow the observed static device slice while keeping its unimplemented typed SM source export blocked.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::sourceAndDeviceGatesRemainIndependent() //20261113-11 add
{
    HmiProject project = polylineProject();
    QJsonObject capabilities = describeProjectCapabilities(project);
    QByteArray source = QByteArrayLiteral("keep");
    QString error;

    QCOMPARE(capabilities.value(QStringLiteral("deviceCompile")).toObject().value(QStringLiteral("status")).toString(),QStringLiteral("candidate"));
    QCOMPARE(capabilities.value(QStringLiteral("nativeSource")).toObject().value(QStringLiteral("status")).toString(),QStringLiteral("blocked"));
    QVERIFY(!exportNativeProjectSource(project,&source,&error)); QCOMPARE(source,QByteArrayLiteral("keep"));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::layerOrderAndStartupLanguageStayIndependent
 * Description     : Freeze stable layer ties without turning the selected editor language into a startup change.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::layerOrderAndStartupLanguageStayIndependent() //20261113-11 add
{
    HmiProject project = polylineProject();
    HmiObject peer = project.objects[0];
    DeviceCompilePlan plan;
    QVector<QByteArray> first;
    QVector<QByteArray> second;
    QString error;

    project.languages = {QStringLiteral("中文"),QStringLiteral("English")}; project.startupLanguage = QStringLiteral("中文");
    peer.id = QStringLiteral("second"); peer.geometry.moveTop(250); project.objects.append(peer);
    QVERIFY2(prepareDeviceCompilePlan(project,&plan,&error),qPrintable(error)); QCOMPARE(plan.objects[0].id,QStringLiteral("stage5-polyline"));
    QVERIFY2(buildProjectPayload(project,&first,&error),qPrintable(error)); project.activeLanguage = QStringLiteral("English");
    QVERIFY2(buildProjectPayload(project,&second,&error),qPrintable(error)); QCOMPARE(first[0],second[0]); QCOMPARE(first[8],second[8]);
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::mixedExistingFiveAndPolylineCloseIndexes
 * Description     : Compile all existing record families beside a static polyline and verify actual frame and raw-offset closure.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::mixedExistingFiveAndPolylineCloseIndexes() //20261113-11 add
{
    HmiProject seed;
    HmiProject project = polylineProject();
    HmiPage page;
    HmiObject object;
    HmiObject table; //20261113-12 add: Both static families must coexist with the five existing record families.
    HmiObject rectangle; //20261113-13 add: Three static families must not disturb five dynamic raw offsets.
    HmiObject line; //20261113-14 add: A fourth background family must leave the existing dynamic record count unchanged.
    HmiObject polygon; //20261114-1 add: A seventh background family must preserve dynamic records.
    HmiObject sector; //20261114-3 add: A ninth static family must preserve all existing control records.
    HmiObject ellipse; //20261114-2 add: The eighth background family contributes no dynamic record.
    HmiObject roundrect; //20261113-16 add: A sixth static family must preserve all five dynamic records.
    HmiObject freeline; //20261113-15 add: The fifth background family contributes no dynamic records.
    QSet<QString> found;
    QString family;
    QString error;
    QVector<QByteArray> payloads;

    QVERIFY2(HmiProject::load(QFINDTESTDATA("../samples/stage4/stage4-editor-40-slices.ksp"),&seed,&error),qPrintable(error));
    project.languages = seed.languages; project.pictures = seed.pictures; project.macros = seed.macros; project.connections = seed.connections;
    Q_FOREACH (page,seed.allPages())
    {
        Q_FOREACH (object,page.objects)
        {
            if (object.type != QStringLiteral("text") && object.type != QStringLiteral("switch")) { continue; }
            family = object.type == QStringLiteral("text") ? QStringLiteral("text") : object.hasWordButtonProperties ? QStringLiteral("word") : object.hasLampProperties ? QStringLiteral("lamp") : object.hasFunctionButtonProperties ? QStringLiteral("function") : QStringLiteral("bit");
            if (found.contains(family)) { continue; }
            object.id = QStringLiteral("existing-")+family; object.geometry = QRect(20+found.size()*130,20,100,50); found.insert(family); project.objects.append(object);
        }
    }
    table = tableProject().objects.first(); table.geometry.moveTopLeft(QPoint(400,200)); project.objects.append(table); //20261113-12 add
    rectangle = rectangleProject().objects.first(); rectangle.geometry.moveTopLeft(QPoint(130,300)); project.objects.append(rectangle); //20261113-13 add
    line = lineProject().objects.first(); line.geometry = QRect(400,350,100,60); project.objects.append(line); //20261113-14 add
    freeline = freelineProject().objects.first(); freeline.geometry = QRect(600,350,100,60); project.objects.append(freeline); //20261113-15 add
    roundrect = rectangleProject().objects.first(); roundrect.type = QStringLiteral("roundrect"); roundrect.id = QStringLiteral("stage5-roundrect"); roundrect.geometry.moveTopLeft(QPoint(350,80)); project.objects.append(roundrect); //20261113-16 add
    polygon=polygonProject().objects.first();polygon.geometry=QRect(590,80,100,100);project.objects.append(polygon); //20261114-1 add
    ellipse=ellipseProject().objects.first();ellipse.geometry=QRect(260,190,100,100);project.objects.append(ellipse); //20261114-2 add
    sector=sectorProject().objects.first();sector.geometry=QRect(10,205,100,100);project.objects.append(sector); //20261114-3 add
    QCOMPARE(project.objects.size(),14);
    QCOMPARE(found.size(),5); QVERIFY2(buildProjectPayload(project,&payloads,&error),qPrintable(error));
    QCOMPARE(payloads.size(),12); QVERIFY2(validateDevicePayloadStructure(payloads,&error),qPrintable(error));
    QCOMPARE(payloads[10].size(),5*640);
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::tableFullNativeRgb565Equality_data
 * Description     : Bind every grid toggle, no-line mode and native count/geometry boundaries to saved-source frames.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::tableFullNativeRgb565Equality_data() //20261113-12 add
{
    int mask;

    QTest::addColumn<int>("mask"); QTest::addColumn<int>("lineStyle"); QTest::addColumn<int>("rows"); QTest::addColumn<int>("columns");
    QTest::addColumn<QRect>("geometry"); QTest::addColumn<QString>("fixture");
    for (mask=0;mask<8;++mask)
    {
        QTest::newRow(qPrintable(QStringLiteral("grid-flags-%1").arg(mask))) << mask << 1 << 4 << 4 << QRect(100,100,200,100) << QStringLiteral("table-flags-%1-reopen-r1").arg(mask);
    }
    QTest::newRow("no-grid-solid-outline") << 7 << 0 << 4 << 4 << QRect(100,100,200,100) << QStringLiteral("table-line-zero-reopen-r1");
    QTest::newRow("minimum-native-one-cell") << 7 << 1 << 1 << 1 << QRect(100,100,40,40) << QStringLiteral("table-one-cell-reopen-r1");
    QTest::newRow("three-by-two-shifted") << 7 << 1 << 3 << 2 << QRect(75,65,160,120) << QStringLiteral("table-three-by-two-reopen-r1");
    QTest::newRow("maximum-thirty-by-thirty-screen-edge") << 7 << 1 << 30 << 30 << QRect(0,0,600,300) << QStringLiteral("table-thirty-boundary-reopen-r1");
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::tableFullNativeRgb565Equality
 * Description     : Compare all 768000 RGB565 bytes and enforce the static table's zero-record/zero-sprite contract.
 * Input Parameters: Qt fixture mask, lineStyle, rows, columns, geometry, fixture
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::tableFullNativeRgb565Equality() //20261113-12 add
{
    QFETCH(int,mask); QFETCH(int,lineStyle); QFETCH(int,rows); QFETCH(int,columns); QFETCH(QRect,geometry); QFETCH(QString,fixture);
    HmiProject project = tableProject();
    DeviceCompilePlan plan;
    QVector<QByteArray> payloads;
    QString error;
    QString root = QFINDTESTDATA("../docs/stage5-evidence/native/table");
    QByteArray native = testFileBytes(QDir(root).filePath(fixture+QStringLiteral("/compiled/fresh-process-3/SheetPixel.dat")));

    project.objects[0].geometry = geometry; project.objects[0].graphic.rows = rows; project.objects[0].graphic.columns = columns;
    project.objects[0].graphic.lineStyle = lineStyle; project.objects[0].graphic.showHorizontal = (mask&1)!=0;
    project.objects[0].graphic.showVertical = (mask&2)!=0; project.objects[0].graphic.showOutline = (mask&4)!=0;
    QVERIFY2(prepareDeviceCompilePlan(project,&plan,&error),qPrintable(error)); QCOMPARE(plan.staticTableCount,1); QCOMPARE(plan.staticPolylineCount,0);
    QVERIFY(!native.isEmpty()); QCOMPARE(decodeTestFrame(native).size(),768000);
    QVERIFY2(buildProjectPayload(project,&payloads,&error),qPrintable(error)); QCOMPARE(payloads.size(),10);
    QCOMPARE(decodeTestFrame(payloads[8]),decodeTestFrame(native)); QCOMPARE(payloads[5].size(),2400);
    QCOMPARE(int(static_cast<unsigned char>(payloads[1][2])),0); QVERIFY2(validateDevicePayloadStructure(payloads,&error),qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::tableUnprovedFieldsPreserveOutput_data
 * Description     : Enumerate raster and boundary inputs outside the proven static-table slice.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::tableUnprovedFieldsPreserveOutput_data() //20261113-12 add
{
    int variant;

    QTest::addColumn<int>("variant");
    for (variant=0;variant<13;++variant) { QTest::newRow(qPrintable(QString::number(variant))) << variant; }
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::tableUnprovedFieldsPreserveOutput
 * Description     : Reject unsupported tables and preserve both caller output and a prepared plan, including touching edges.
 * Input Parameters: Qt fixture variant
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::tableUnprovedFieldsPreserveOutput() //20261113-12 add
{
    QFETCH(int,variant);
    HmiProject project = tableProject();
    DeviceCompilePlan plan;
    QJsonObject issue;
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    HmiObject peer;
    QString error;

    plan.staticTableCount = 77;
    if (variant == 0) { project.objects[0].graphic.lineStyle = 2; }
    if (variant == 1) { project.objects[0].graphic.alpha = 128; }
    if (variant == 2) { project.objects[0].graphic.lineColor = Qt::red; }
    if (variant == 3) { project.objects[0].graphic.frameColor = Qt::red; }
    if (variant == 4) { project.objects[0].graphic.background = Qt::white; }
    if (variant == 5) { project.objects[0].graphic.rows = 3; }
    if (variant == 6) { project.objects[0].graphic.columns = 3; }
    if (variant == 7) { project.objects[0].geometry.moveLeft(600); }
    if (variant == 8) { project.objects[0].geometry.moveTop(380); }
    if (variant == 9) { peer = project.objects[0]; peer.id = QStringLiteral("overlap"); project.objects.append(peer); }
    if (variant == 10) { peer = project.objects[0]; peer.id = QStringLiteral("touching-edge"); peer.geometry.moveLeft(300); project.objects.append(peer); }
    if (variant == 11) { project.objects[0].type = QStringLiteral("recipe-display"); }
    if (variant == 12) { project.objects[0].geometry = QRect(100,100,4,4); }
    QVERIFY(!prepareDeviceCompilePlan(project,&plan,&error,&issue)); QCOMPARE(plan.staticTableCount,77);
    QVERIFY(!issue.value(QStringLiteral("property")).toString().isEmpty()); QVERIFY(!error.isEmpty());
    QVERIFY(!buildProjectPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::tablePackageAndSourceGates
 * Description     : Publish and revalidate a real static-table package while preserving native-source and legacy-text gates.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::tablePackageAndSourceGates() //20261113-12 add
{
    HmiProject project = tableProject();
    QJsonObject capabilities = describeProjectCapabilities(project);
    QTemporaryDir temporary;
    QJsonObject manifest;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    QString target = temporary.filePath(QStringLiteral("table"));

    QCOMPARE(capabilities.value(QStringLiteral("deviceCompile")).toObject().value(QStringLiteral("status")).toString(),QStringLiteral("candidate"));
    QCOMPARE(capabilities.value(QStringLiteral("nativeSource")).toObject().value(QStringLiteral("status")).toString(),QStringLiteral("blocked"));
    QVERIFY(!exportNativeProjectSource(project,&source,&error)); QCOMPARE(source,QByteArrayLiteral("keep"));
    QVERIFY(!buildSingleTextPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
    QVERIFY2(exportProjectPayload(project,target,&error),qPrintable(error)); QVERIFY2(verifyDevicePackage(target,&manifest,&error),qPrintable(error));
    QCOMPARE(manifest.value(QStringLiteral("files")).toObject().size(),10); QCOMPARE(manifest.value(QStringLiteral("deviceAcknowledged")).toBool(true),false);
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::rectangleCanonicalFrames_data
 * Description     : Bind saved-source geometry, all binary line/fill switches and two-object composition to native full frames.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::rectangleCanonicalFrames_data() //20261113-13 add
{
    QTest::addColumn<QRect>("geometry"); QTest::addColumn<int>("lineStyle"); QTest::addColumn<int>("fillStyle"); QTest::addColumn<bool>("twoObjects"); QTest::addColumn<QString>("fixture");
    QTest::newRow("default") << QRect(100,100,200,100) << 1 << 1 << false << QStringLiteral("rect-default-repeat-r1");
    QTest::newRow("small-drag-produces-native-default-size") << QRect(100,100,100,100) << 1 << 1 << false << QStringLiteral("rect-minimum-reopen-r1");
    QTest::newRow("shifted-sized") << QRect(75,65,160,120) << 1 << 1 << false << QStringLiteral("rect-shifted-reopen-r1");
    QTest::newRow("screen-edge") << QRect(0,0,200,100) << 1 << 1 << false << QStringLiteral("rect-screen-edge-reopen-r1");
    QTest::newRow("invisible-retained-object") << QRect(100,100,200,100) << 0 << 0 << false << QStringLiteral("rect-options-0-0-reopen-r1");
    QTest::newRow("fill-without-outline") << QRect(100,100,200,100) << 0 << 1 << false << QStringLiteral("rect-options-0-1-reopen-r1");
    QTest::newRow("outline-without-fill") << QRect(100,100,200,100) << 1 << 0 << false << QStringLiteral("rect-options-1-0-reopen-r1");
    QTest::newRow("two-nonoverlapping-rectangles") << QRect(100,100,200,100) << 1 << 1 << true << QStringLiteral("rect-two-objects-reopen-r1");
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::rectangleCanonicalFrames
 * Description     : Compare every RGB565 byte through the real project compiler and verify the rectangle emits no dynamic record.
 * Input Parameters: Qt fixture geometry, lineStyle, fillStyle, twoObjects, fixture
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::rectangleCanonicalFrames() //20261113-13 add
{
    QFETCH(QRect,geometry); QFETCH(int,lineStyle); QFETCH(int,fillStyle); QFETCH(bool,twoObjects); QFETCH(QString,fixture);
    HmiProject project = rectangleProject();
    HmiObject peer;
    DeviceCompilePlan plan;
    QString root = QFINDTESTDATA("../docs/stage5-evidence/native/rect");
    QByteArray native = testFileBytes(QDir(root).filePath(fixture+QStringLiteral("/compiled/fresh-process-3/SheetPixel.dat")));
    QVector<QByteArray> payloads;
    QString error;

    project.objects[0].geometry = geometry; project.objects[0].graphic.lineStyle = lineStyle; project.objects[0].graphic.fillStyle = fillStyle;
    if (twoObjects) { peer=project.objects[0]; peer.id=QStringLiteral("rectangle-peer"); peer.geometry=QRect(400,100,100,100); project.objects.append(peer); }
    QVERIFY2(prepareDeviceCompilePlan(project,&plan,&error),qPrintable(error)); QCOMPARE(plan.staticRectCount,twoObjects?2:1);
    QCOMPARE(decodeTestFrame(native).size(),768000); QVERIFY2(buildProjectPayload(project,&payloads,&error),qPrintable(error));
    QCOMPARE(payloads.size(),10); QCOMPARE(decodeTestFrame(payloads[8]),decodeTestFrame(native)); QCOMPARE(payloads[5].size(),2400);
    QCOMPARE(int(static_cast<unsigned char>(payloads[1][2])),0); QVERIFY2(validateDevicePayloadStructure(payloads,&error),qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::rectangleUnprovedFieldsPreserveOutput_data
 * Description     : Enumerate unsupported rectangle rendering and geometric boundaries.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::rectangleUnprovedFieldsPreserveOutput_data() //20261113-13 add
{
    int variant;

    QTest::addColumn<int>("variant");
    for (variant=0;variant<12;++variant) { QTest::newRow(qPrintable(QString::number(variant))) << variant; }
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::rectangleUnprovedFieldsPreserveOutput
 * Description     : Reject unproved styles, small sizes, edge clipping and overlap before changing caller output.
 * Input Parameters: Qt fixture variant
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::rectangleUnprovedFieldsPreserveOutput() //20261113-13 add
{
    QFETCH(int,variant);
    HmiProject project = rectangleProject();
    HmiObject peer;
    DeviceCompilePlan plan;
    QJsonObject issue;
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;

    plan.staticRectCount=77;
    if (variant==0) { project.objects[0].graphic.lineStyle=2; }
    if (variant==1) { project.objects[0].graphic.lineWidth=2; }
    if (variant==2) { project.objects[0].graphic.alpha=128; }
    if (variant==3) { project.objects[0].graphic.lineColor=Qt::red; }
    if (variant==4) { project.objects[0].graphic.fillStyle=2; }
    if (variant==5) { project.objects[0].graphic.background=Qt::red; }
    if (variant==6) { project.objects[0].graphic.foreground=Qt::red; }
    if (variant==7) { project.objects[0].geometry=QRect(100,100,99,100); }
    if (variant==8) { project.objects[0].geometry.moveLeft(600); }
    if (variant==9) { project.objects[0].geometry.moveTop(380); }
    if (variant==10) { peer=project.objects[0]; peer.id=QStringLiteral("overlap"); project.objects.append(peer); }
    if (variant==11) { peer=project.objects[0]; peer.id=QStringLiteral("touching"); peer.geometry.moveLeft(300); project.objects.append(peer); }
    QVERIFY(!prepareDeviceCompilePlan(project,&plan,&error,&issue)); QCOMPARE(plan.staticRectCount,77);
    QVERIFY(!issue.value(QStringLiteral("property")).toString().isEmpty()); QVERIFY(!error.isEmpty());
    QVERIFY(!buildProjectPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::rectangleSourceAndPackageGates
 * Description     : Revalidate a rectangle device package while retaining native-SM and legacy text rejection.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::rectangleSourceAndPackageGates() //20261113-13 add
{
    HmiProject project = rectangleProject();
    QTemporaryDir temporary;
    QString target = temporary.filePath(QStringLiteral("rectangle"));
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QJsonObject manifest;
    QString error;

    QVERIFY(!exportNativeProjectSource(project,&source,&error)); QCOMPARE(source,QByteArrayLiteral("keep"));
    QVERIFY(!buildSingleTextPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
    QVERIFY2(exportProjectPayload(project,target,&error),qPrintable(error)); QVERIFY2(verifyDevicePackage(target,&manifest,&error),qPrintable(error));
    QCOMPARE(manifest.value(QStringLiteral("files")).toObject().size(),10); QCOMPARE(manifest.value(QStringLiteral("deviceAcknowledged")).toBool(true),false);
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::twoPolylinesMatchReopenedNativeFrame
 * Description     : Compare a two-object original compile and keep both static paths out of the dynamic control table.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::twoPolylinesMatchReopenedNativeFrame() //20261113-11 add
{
    HmiProject project = polylineProject();
    HmiObject peer = project.objects[0];
    QVector<QByteArray> payloads;
    QByteArray native;
    QString error;

    peer.id = QStringLiteral("second-polyline"); peer.geometry = QRect(400,100,100,100); peer.layer = 2; project.objects.append(peer);
    native = testFileBytes(QFINDTESTDATA("../docs/stage5-evidence/native/polyline/two-polylines-reopen-r1/compiled/fresh-process-3/SheetPixel.dat"));
    QVERIFY(!native.isEmpty()); QVERIFY2(buildProjectPayload(project,&payloads,&error),qPrintable(error)); QCOMPARE(payloads.size(),10);
    QCOMPARE(decodeTestFrame(payloads[8]),decodeTestFrame(native));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::publicationVerifiesAndRefusesExistingDestination
 * Description     : Exercise production export, full package verification, and preservation of a previous valid output.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::publicationVerifiesAndRefusesExistingDestination() //20261113-11 add
{
    QTemporaryDir temporary;
    QString target = temporary.filePath(QStringLiteral("valid"));
    QString error;
    QJsonObject manifest;
    QByteArray before;

    QVERIFY2(exportProjectPayload(polylineProject(),target,&error),qPrintable(error));
    QVERIFY2(verifyDevicePackage(target,&manifest,&error),qPrintable(error));
    QCOMPARE(manifest.value(QStringLiteral("files")).toObject().size(),10); QVERIFY(!manifest.value(QStringLiteral("deviceAcknowledged")).toBool());
    before = testFileBytes(target+QStringLiteral("/compiler-manifest.json"));
    QVERIFY(!exportProjectPayload(polylineProject(),target,&error)); QCOMPARE(testFileBytes(target+QStringLiteral("/compiler-manifest.json")),before);
    QCOMPARE(QDir(temporary.path()).entryList(QDir::Dirs|QDir::Hidden|QDir::NoDotAndDotDot),QStringList{QStringLiteral("valid")});
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::failedWritesNeverPublish_data
 * Description     : Fault the actual staging write seam at data, corruption and final-manifest boundaries.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::failedWritesNeverPublish_data() //20261113-11 add
{
    QTest::addColumn<int>("mode"); QTest::newRow("short-write") << 0; QTest::newRow("corrupt-success") << 1; QTest::newRow("manifest-write-failure") << 2;
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::failedWritesNeverPublish
 * Description     : Prove failed or falsely successful writes cannot leave a valid-looking published package or staging directory.
 * Input Parameters: Qt fixture mode
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::failedWritesNeverPublish() //20261113-11 add
{
    QFETCH(int,mode);
    QTemporaryDir temporary;
    QVector<QByteArray> payloads;
    QString error;
    int writes = 0;
    DevicePackageWriter writer;

    QVERIFY2(buildProjectPayload(polylineProject(),&payloads,&error),qPrintable(error));
    writer = [&writes,mode](const QString &path,const QByteArray &bytes,QString *diagnostic) {
        ++writes;
        if (mode == 0 && writes == 3) { testWriteBytes(path,bytes.left(bytes.size()/2),diagnostic); return false; }
        if (mode == 1 && writes == 1) { return testWriteBytes(path,QByteArrayLiteral("corrupted"),diagnostic); }
        if (mode == 2 && QFileInfo(path).fileName() == QStringLiteral("compiler-manifest.json")) { return false; }
        return testWriteBytes(path,bytes,diagnostic);
    };
    QVERIFY(!publishDevicePackage(polylineProject(),payloads,temporary.filePath(QStringLiteral("failed")),&error,writer));
    QVERIFY(!QFileInfo::exists(temporary.filePath(QStringLiteral("failed"))));
    QVERIFY(QDir(temporary.path()).entryList(QDir::AllEntries|QDir::Hidden|QDir::NoDotAndDotDot).isEmpty());
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::occupiedDestinationDuringPublishIsPreserved
 * Description     : Simulate destination occupation after staging and preserve the other writer's file on rename failure.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::occupiedDestinationDuringPublishIsPreserved() //20261113-11 add
{
    QTemporaryDir temporary;
    QString target = temporary.filePath(QStringLiteral("occupied"));
    QString error;
    QVector<QByteArray> payloads;
    DevicePackageWriter writer;

    QVERIFY(buildProjectPayload(polylineProject(),&payloads,&error));
    writer = [target](const QString &path,const QByteArray &bytes,QString *diagnostic) {
        if (QFileInfo(path).fileName() == QStringLiteral("compiler-manifest.json")) { QDir().mkpath(target); testWriteBytes(target+QStringLiteral("/keep.txt"),QByteArrayLiteral("other-writer"),diagnostic); }
        return testWriteBytes(path,bytes,diagnostic);
    };
    QVERIFY(!publishDevicePackage(polylineProject(),payloads,target,&error,writer));
    QCOMPARE(testFileBytes(target+QStringLiteral("/keep.txt")),QByteArrayLiteral("other-writer"));
    QVERIFY(!QFileInfo::exists(target+QStringLiteral("/compiler-manifest.json")));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::verifierRejectsStaleFilesHashAndPlanTampering
 * Description     : Detect stale segments, modified files and a forged command plan against a real valid package.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::verifierRejectsStaleFilesHashAndPlanTampering() //20261113-11 add
{
    QTemporaryDir temporary;
    QString target = temporary.filePath(QStringLiteral("valid"));
    QString error;
    QJsonObject manifest;
    QJsonArray plan;
    QJsonObject step;
    QByteArray saved;

    QVERIFY(exportProjectPayload(polylineProject(),target,&error));
    QVERIFY(testWriteBytes(target+QStringLiteral("/ControlPara.dat"),QByteArrayLiteral("stale"),&error)); QVERIFY(!verifyDevicePackage(target,nullptr,&error));
    QVERIFY(QFile::remove(target+QStringLiteral("/ControlPara.dat"))); QVERIFY(verifyDevicePackage(target,&manifest,&error));
    plan = manifest.value(QStringLiteral("transferPlan")).toArray(); step = plan[0].toObject(); step.insert(QStringLiteral("command"),255); plan[0] = step; manifest.insert(QStringLiteral("transferPlan"),plan);
    saved = testFileBytes(target+QStringLiteral("/compiler-manifest.json")); QVERIFY(testWriteBytes(target+QStringLiteral("/compiler-manifest.json"),QJsonDocument(manifest).toJson(),&error)); QVERIFY(!verifyDevicePackage(target,nullptr,&error));
    QVERIFY(testWriteBytes(target+QStringLiteral("/compiler-manifest.json"),saved,&error));
    QVERIFY(testWriteBytes(target+QStringLiteral("/SheetPixel.dat"),QByteArrayLiteral("changed"),&error)); QVERIFY(!verifyDevicePackage(target,nullptr,&error));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::validatorRejectsRehashedIndexCorruption
 * Description     : Reject an internally inconsistent pixel length even when checksum protection is outside the seam.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::validatorRejectsRehashedIndexCorruption() //20261113-11 add
{
    QVector<QByteArray> payloads;
    QString error;

    QVERIFY(buildProjectPayload(polylineProject(),&payloads,&error));
    payloads[5][0] = char(payloads[5][0]^1); QVERIFY(!validateDevicePayloadStructure(payloads,&error));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::validatorRejectsUnknownRecordsAndBrokenFrames_data
 * Description     : Name independent record identity, geometry, raw index, codec and padding corruption cases.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::validatorRejectsUnknownRecordsAndBrokenFrames_data() //20261113-11 add
{
    int variant;
    QTest::addColumn<int>("variant");
    for (variant=0;variant<7;++variant) { QTest::newRow(qPrintable(QString::number(variant))) << variant; }
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::validatorRejectsUnknownRecordsAndBrokenFrames
 * Description     : Check structural semantics after checksum checks rather than accepting a self-consistently rehashed bad record.
 * Input Parameters: Qt fixture variant
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::validatorRejectsUnknownRecordsAndBrokenFrames() //20261113-11 add
{
    QFETCH(int,variant);
    HmiProject project;
    QVector<QByteArray> payloads;
    QString error;

    QVERIFY(HmiProject::load(QDir(QFINDTESTDATA("../samples")).filePath(QStringLiteral("bit-button/默认交替位按钮.ksp")),&project,&error));
    QVERIFY2(buildProjectPayload(project,&payloads,&error),qPrintable(error)); QCOMPARE(payloads.size(),12);
    if (variant == 0) { payloads[10][0] = char(255); }
    if (variant == 1) { qToLittleEndian<unsigned short>(2,payloads[10].data()+0x106); }
    if (variant == 2) { qToLittleEndian<unsigned short>(799,payloads[10].data()+4); }
    if (variant == 3) { qToLittleEndian<unsigned int>(1,payloads[10].data()+0x0c); }
    if (variant == 4) { payloads[5][2400] = char(1); }
    if (variant == 5) { qToLittleEndian<unsigned int>(1,payloads[5].data()+2404); }
    if (variant == 6) { payloads[11][0] = char(payloads[11][0]^127); }
    QVERIFY(!validateDevicePayloadStructure(payloads,&error)); QVERIFY(!error.isEmpty());
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::realMacroFourteenSegmentPackage
 * Description     : Exercise the actual ARM SDK and verify a published fourteen-segment macro package offline.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::realMacroFourteenSegmentPackage() //20261113-11 add
{
    HmiProject project;
    QTemporaryDir temporary;
    QJsonObject manifest;
    QString error;
    QString target = temporary.filePath(QStringLiteral("macro"));

    QVERIFY(HmiProject::load(QDir(QFINDTESTDATA("../samples")).filePath(QStringLiteral("word-button/字按钮宏绑定.ksp")),&project,&error));
    QVERIFY2(exportProjectPayload(project,target,&error),qPrintable(error));
    QVERIFY2(verifyDevicePackage(target,&manifest,&error),qPrintable(error)); QCOMPARE(manifest.value(QStringLiteral("files")).toObject().size(),14);
    QVERIFY(testFileBytes(target+QStringLiteral("/MarcoRun.dat")).size()>1024);
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::snapshotCorruptionAndFalseIdentityAreRejected
 * Description     : Reject invalid compressed source, invented snapshot format and a false input-project identity.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::snapshotCorruptionAndFalseIdentityAreRejected() //20261113-11 add
{
    QVector<QByteArray> payloads;
    QString error;
    QTemporaryDir temporary;
    QString target = temporary.filePath(QStringLiteral("valid"));
    QJsonObject manifest;

    QVERIFY(buildProjectPayload(polylineProject(),&payloads,&error));
    payloads[7] = qCompress(QByteArrayLiteral("not-a-native-source")); QVERIFY(!validateDevicePayloadStructure(payloads,&error));
    payloads[7] = QByteArrayLiteral("bad-zlib"); QVERIFY(!validateDevicePayloadStructure(payloads,&error));
    QVERIFY(exportProjectPayload(polylineProject(),target,&error)); QVERIFY(verifyDevicePackage(target,&manifest,&error));
    manifest.insert(QStringLiteral("sourceSnapshot"),QStringLiteral("native-sm"));
    QVERIFY(testWriteBytes(target+QStringLiteral("/compiler-manifest.json"),QJsonDocument(manifest).toJson(),&error)); QVERIFY(!verifyDevicePackage(target,nullptr,&error));
    manifest.insert(QStringLiteral("sourceSnapshot"),QStringLiteral("lite-project-json")); manifest.insert(QStringLiteral("inputProjectSha256"),QStringLiteral("wrong-input"));
    QVERIFY(testWriteBytes(target+QStringLiteral("/compiler-manifest.json"),QJsonDocument(manifest).toJson(),&error)); QVERIFY(!verifyDevicePackage(target,nullptr,&error));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::legacyTextCompilerRejectsPolyline
 * Description     : Keep the historical one-text compiler from silently compiling a new static graphic as text.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::legacyTextCompilerRejectsPolyline() //20261113-11 add
{
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;
    QVERIFY(!buildSingleTextPayload(polylineProject(),&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::lineCanonicalFrames_data
 * Description     : Bind each saved native line source and its proven endpoint geometry to a complete-frame fixture.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::lineCanonicalFrames_data() //20261113-14 add
{
    QString root = QFINDTESTDATA("../docs/stage5-evidence/native/line");
    QJsonObject corpus = QJsonDocument::fromJson(testFileBytes(QDir(root).filePath(QStringLiteral("B-S-M-bresenham-expanded.json")))).object();
    QJsonValue entry;
    QJsonObject sample;
    QJsonArray geometry;
    QJsonArray points;
    QString run;
    int count = 0;

    QTest::addColumn<QRect>("geometry"); QTest::addColumn<QPointF>("first"); QTest::addColumn<QPointF>("last");
    QTest::addColumn<QString>("fixture"); QTest::addColumn<QString>("sourceSha");
    QVERIFY(corpus.value(QStringLiteral("allCanonicalRasterComparisonsEqual")).toBool());
    Q_FOREACH (entry,corpus.value(QStringLiteral("cases")).toArray())
    {
        sample = entry.toObject(); run = sample.value(QStringLiteral("run")).toString();
        if (!run.contains(QStringLiteral("reopen")) && !run.contains(QStringLiteral("default-repeat"))) { continue; }
        geometry = sample.value(QStringLiteral("geometry")).toArray(); points = sample.value(QStringLiteral("normalizedPoints")).toArray();
        QTest::newRow(qPrintable(run)) << QRect(geometry[0].toInt(),geometry[1].toInt(),geometry[2].toInt(),geometry[3].toInt())
            << QPointF(points[0].toArray()[0].toDouble(),points[0].toArray()[1].toDouble())
            << QPointF(points[1].toArray()[0].toDouble(),points[1].toArray()[1].toDouble())
            << sample.value(QStringLiteral("folder")).toString() << sample.value(QStringLiteral("sourceSha256")).toString();
        ++count;
    }
    QCOMPARE(count,14);
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::lineCanonicalFrames
 * Description     : Compare all 768000 product bytes with each actual source-bound native raster, including error ties and tiny strokes.
 * Input Parameters: Qt geometry, normalized endpoints, fixture and source identity
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::lineCanonicalFrames() //20261113-14 add
{
    QFETCH(QRect,geometry); QFETCH(QPointF,first); QFETCH(QPointF,last); QFETCH(QString,fixture); QFETCH(QString,sourceSha);
    HmiProject project = lineProject();
    DeviceCompilePlan plan;
    QVector<QByteArray> payloads;
    QString root = QFINDTESTDATA("..");
    QString folder = QDir(root).filePath(fixture);
    QString error;
    QByteArray native = testFileBytes(QDir(folder).filePath(QStringLiteral("SheetPixel.dat")));
    QByteArray source = testFileBytes(QDir(folder).filePath(QStringLiteral("input-native.sm")));

    project.objects[0].geometry = geometry; project.objects[0].graphic.points = {first,last};
    QVERIFY(!source.isEmpty()); QCOMPARE(QString::fromLatin1(QCryptographicHash::hash(source,QCryptographicHash::Sha256).toHex()),sourceSha);
    QVERIFY2(prepareDeviceCompilePlan(project,&plan,&error),qPrintable(error)); QCOMPARE(plan.staticLineCount,1); QCOMPARE(plan.staticPolylineCount,0);
    QVERIFY2(buildProjectPayload(project,&payloads,&error),qPrintable(error)); QCOMPARE(payloads.size(),10);
    QCOMPARE(decodeTestFrame(native).size(),768000); QCOMPARE(decodeTestFrame(payloads[8]),decodeTestFrame(native));
    QCOMPARE(int(static_cast<unsigned char>(payloads[1][2])),0); QCOMPARE(payloads[5].size(),2400);
    QVERIFY2(validateDevicePayloadStructure(payloads,&error),qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::lineUnprovedFieldsPreserveOutput_data
 * Description     : Enumerate meaningful style, coordinate, rounded-zero, clipping and overlap failures.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::lineUnprovedFieldsPreserveOutput_data() //20261113-14 add
{
    int variant;

    QTest::addColumn<int>("variant");
    for (variant=0;variant<14;++variant) { QTest::newRow(qPrintable(QString::number(variant))) << variant; }
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::lineUnprovedFieldsPreserveOutput
 * Description     : Preserve both plan and output when an unproved line or an oblique polyline is rejected.
 * Input Parameters: Qt variant
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::lineUnprovedFieldsPreserveOutput() //20261113-14 add
{
    QFETCH(int,variant);
    HmiProject project = lineProject();
    HmiObject peer;
    DeviceCompilePlan plan;
    QJsonObject issue;
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;

    plan.staticLineCount=77;
    if (variant==0) { project.objects[0].graphic.lineStyle=2; }
    if (variant==1) { project.objects[0].graphic.lineWidth=2; }
    if (variant==2) { project.objects[0].graphic.alpha=128; }
    if (variant==3) { project.objects[0].graphic.lineColor=Qt::red; }
    if (variant==4) { project.objects[0].graphic.endArrow=1; }
    if (variant==5) { project.objects[0].graphic.endCap=1; }
    if (variant==6) { project.objects[0].graphic.points={QPointF(0,0),QPointF(0.333,1)}; }
    if (variant==7) { project.objects[0].geometry.moveLeft(600); }
    if (variant==8) { project.objects[0].geometry.moveTop(380); }
    if (variant==9) { peer=project.objects[0]; peer.id=QStringLiteral("overlap"); project.objects.append(peer); }
    if (variant==10) { project.objects[0].graphic.points={QPointF(0,0),QPointF(1e-11,0)}; }
    if (variant==11) { project.objects[0].type=QStringLiteral("polyline"); }
    if (variant==12) { project.objects[0].graphic.points={QPointF(0,0),QPointF(0,0)}; }
    if (variant==13) { project.objects[0].geometry.moveLeft(-1); }
    QVERIFY(!prepareDeviceCompilePlan(project,&plan,&error,&issue)); QCOMPARE(plan.staticLineCount,77);
    QVERIFY(!issue.value(QStringLiteral("property")).toString().isEmpty()); QVERIFY(!error.isEmpty());
    QVERIFY(!buildProjectPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::lineSourceAndPackageGates
 * Description     : Publish and revalidate a real straight-line package while keeping full SM and legacy single-text export blocked.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::lineSourceAndPackageGates() //20261113-14 add
{
    HmiProject project = lineProject();
    QTemporaryDir temporary;
    QString target = temporary.filePath(QStringLiteral("line"));
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QJsonObject manifest;
    QJsonObject capabilities = describeProjectCapabilities(project);
    QString error;

    QCOMPARE(capabilities.value(QStringLiteral("deviceCompile")).toObject().value(QStringLiteral("status")).toString(),QStringLiteral("candidate"));
    QCOMPARE(capabilities.value(QStringLiteral("nativeSource")).toObject().value(QStringLiteral("status")).toString(),QStringLiteral("blocked"));
    QVERIFY(!exportNativeProjectSource(project,&source,&error)); QCOMPARE(source,QByteArrayLiteral("keep"));
    QVERIFY(!buildSingleTextPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
    QVERIFY2(exportProjectPayload(project,target,&error),qPrintable(error)); QVERIFY2(verifyDevicePackage(target,&manifest,&error),qPrintable(error));
    QCOMPARE(manifest.value(QStringLiteral("files")).toObject().size(),10); QCOMPARE(manifest.value(QStringLiteral("deviceAcknowledged")).toBool(true),false);
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::freelineCanonicalFrames_data
 * Description     : Bind each saved native line source and its proven endpoint geometry to a complete-frame fixture.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::freelineCanonicalFrames_data() //20261113-15 add
{
    QString root = QFINDTESTDATA("../docs/stage5-evidence/native/freeline");
    QJsonObject corpus = QJsonDocument::fromJson(testFileBytes(QDir(root).filePath(QStringLiteral("B-S-M-bresenham-r2.json")))).object();
    QJsonValue entry;
    QJsonObject sample;
    QJsonArray geometry;
    QJsonArray points;
    QString run;
    int count = 0;

    QTest::addColumn<QRect>("geometry"); QTest::addColumn<QPointF>("first"); QTest::addColumn<QPointF>("last");
    QTest::addColumn<QString>("fixture"); QTest::addColumn<QString>("sourceSha");
    QVERIFY(corpus.value(QStringLiteral("allCanonicalRasterComparisonsEqual")).toBool());
    Q_FOREACH (entry,corpus.value(QStringLiteral("cases")).toArray())
    {
        sample = entry.toObject(); run = sample.value(QStringLiteral("run")).toString();
        if (!run.contains(QStringLiteral("reopen")) && !run.contains(QStringLiteral("default-repeat"))) { continue; }
        geometry = sample.value(QStringLiteral("geometry")).toArray(); points = sample.value(QStringLiteral("normalizedPoints")).toArray();
        QTest::newRow(qPrintable(run)) << QRect(geometry[0].toInt(),geometry[1].toInt(),geometry[2].toInt(),geometry[3].toInt())
            << QPointF(points[0].toArray()[0].toDouble(),points[0].toArray()[1].toDouble())
            << QPointF(points[1].toArray()[0].toDouble(),points[1].toArray()[1].toDouble())
            << sample.value(QStringLiteral("folder")).toString() << sample.value(QStringLiteral("sourceSha256")).toString();
        ++count;
    }
    QCOMPARE(count,14);
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::freelineCanonicalFrames
 * Description     : Compare all 768000 product bytes with each actual source-bound native raster, including error ties and tiny strokes.
 * Input Parameters: Qt geometry, normalized endpoints, fixture and source identity
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::freelineCanonicalFrames() //20261113-15 add
{
    QFETCH(QRect,geometry); QFETCH(QPointF,first); QFETCH(QPointF,last); QFETCH(QString,fixture); QFETCH(QString,sourceSha);
    HmiProject project = freelineProject();
    DeviceCompilePlan plan;
    QVector<QByteArray> payloads;
    QString root = QFINDTESTDATA("..");
    QString folder = QDir(root).filePath(fixture);
    QString error;
    QByteArray native = testFileBytes(QDir(folder).filePath(QStringLiteral("SheetPixel.dat")));
    QByteArray source = testFileBytes(QDir(folder).filePath(QStringLiteral("input-native.sm")));

    project.objects[0].geometry = geometry; project.objects[0].graphic.points = {first,last};
    QVERIFY(!source.isEmpty()); QCOMPARE(QString::fromLatin1(QCryptographicHash::hash(source,QCryptographicHash::Sha256).toHex()),sourceSha);
    QVERIFY2(prepareDeviceCompilePlan(project,&plan,&error),qPrintable(error)); QCOMPARE(plan.staticFreelineCount,1); QCOMPARE(plan.staticPolylineCount,0);
    QVERIFY2(buildProjectPayload(project,&payloads,&error),qPrintable(error)); QCOMPARE(payloads.size(),10);
    QCOMPARE(decodeTestFrame(native).size(),768000); QCOMPARE(decodeTestFrame(payloads[8]),decodeTestFrame(native));
    QCOMPARE(int(static_cast<unsigned char>(payloads[1][2])),0); QCOMPARE(payloads[5].size(),2400);
    QVERIFY2(validateDevicePayloadStructure(payloads,&error),qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::freelineUnprovedFieldsPreserveOutput_data
 * Description     : Enumerate meaningful style, coordinate, rounded-zero, clipping and overlap failures.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::freelineUnprovedFieldsPreserveOutput_data() //20261113-15 add
{
    int variant;

    QTest::addColumn<int>("variant");
    for (variant=0;variant<20;++variant) { QTest::newRow(qPrintable(QString::number(variant))) << variant; }
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::freelineUnprovedFieldsPreserveOutput
 * Description     : Preserve both plan and output when an unproved line or an oblique polyline is rejected.
 * Input Parameters: Qt variant
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::freelineUnprovedFieldsPreserveOutput() //20261113-15 add
{
    QFETCH(int,variant);
    HmiProject project = freelineProject();
    HmiObject peer;
    DeviceCompilePlan plan;
    QJsonObject issue;
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;

    plan.staticFreelineCount=77;
    if (variant==0) { project.objects[0].graphic.lineStyle=2; }
    if (variant==1) { project.objects[0].graphic.lineWidth=2; }
    if (variant==2) { project.objects[0].graphic.alpha=128; }
    if (variant==3) { project.objects[0].graphic.lineColor=Qt::red; }
    if (variant==4) { project.objects[0].graphic.endArrow=1; }
    if (variant==5) { project.objects[0].graphic.endCap=1; }
    if (variant==6) { project.objects[0].graphic.points={QPointF(0,0),QPointF(0.333,1)}; }
    if (variant==7) { project.objects[0].geometry.moveLeft(620); }
    if (variant==8) { project.objects[0].geometry.moveTop(380); }
    if (variant==9) { peer=project.objects[0]; peer.id=QStringLiteral("overlap"); project.objects.append(peer); }
    if (variant==10) { project.objects[0].graphic.points={QPointF(0,0),QPointF(1e-11,0)}; }
    if (variant==11) { project.objects[0].type=QStringLiteral("polyline"); }
    if (variant==12) { project.objects[0].graphic.points={QPointF(0,0),QPointF(0,0)}; }
    if (variant==13) { project.objects[0].geometry.moveLeft(-1); }
    if (variant==14) { project.objects[0].geometry=QRect(100,100,39,39); }
    if (variant==15) { project.objects[0].geometry=QRect(100,100,39,1); project.objects[0].graphic.points={QPointF(0,0),QPointF(1,0)}; }
    if (variant==16) { project.objects[0].geometry=QRect(100,100,1,39); project.objects[0].graphic.points={QPointF(0,0),QPointF(0,1)}; }
    if (variant==17) { project.objects[0].graphic.points={QPointF(0,0),QPointF(.5,.3),QPointF(1,1)}; }
    if (variant==18) { project.objects[0].geometry.moveLeft(0); }
    if (variant==19) { project.objects[0].geometry.moveTop(0); }
    QVERIFY(!prepareDeviceCompilePlan(project,&plan,&error,&issue)); QCOMPARE(plan.staticFreelineCount,77);
    QVERIFY(!issue.value(QStringLiteral("property")).toString().isEmpty()); QVERIFY(!error.isEmpty());
    QVERIFY(!buildProjectPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::freelineSourceAndPackageGates
 * Description     : Publish and revalidate a real straight-line package while keeping full SM and legacy single-text export blocked.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::freelineSourceAndPackageGates() //20261113-15 add
{
    HmiProject project = freelineProject();
    QTemporaryDir temporary;
    QString target = temporary.filePath(QStringLiteral("freeline"));
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QJsonObject manifest;
    QJsonObject capabilities = describeProjectCapabilities(project);
    QString error;

    QCOMPARE(capabilities.value(QStringLiteral("deviceCompile")).toObject().value(QStringLiteral("status")).toString(),QStringLiteral("candidate"));
    QCOMPARE(capabilities.value(QStringLiteral("nativeSource")).toObject().value(QStringLiteral("status")).toString(),QStringLiteral("blocked"));
    QVERIFY(!exportNativeProjectSource(project,&source,&error)); QCOMPARE(source,QByteArrayLiteral("keep"));
    QVERIFY(!buildSingleTextPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
    QVERIFY2(exportProjectPayload(project,target,&error),qPrintable(error)); QVERIFY2(verifyDevicePackage(target,&manifest,&error),qPrintable(error));
    QCOMPARE(manifest.value(QStringLiteral("files")).toObject().size(),10); QCOMPARE(manifest.value(QStringLiteral("deviceAcknowledged")).toBool(true),false);
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::roundrectCanonicalFrames_data
 * Description     : Bind independently observed geometry and binary style modes to complete saved-source frames.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::roundrectCanonicalFrames_data() //20261113-16 add
{
    QString root = QFINDTESTDATA("../docs/stage5-evidence/native/roundrect");
    QJsonArray cases = QJsonDocument::fromJson(testFileBytes(QDir(root).filePath(QStringLiteral("B-S-M-fixed-scanline-r1.json")))).object().value(QStringLiteral("cases")).toArray();
    QJsonValue sample;
    QString run;

    QTest::addColumn<QJsonObject>("sample");
    Q_FOREACH (sample,cases)
    {
        run = sample.toObject().value(QStringLiteral("run")).toString();
        if (run.contains(QStringLiteral("reopen")) || run.contains(QStringLiteral("default-repeat"))) { QTest::newRow(qPrintable(run)) << sample.toObject(); }
    }
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::roundrectCanonicalFrames
 * Description     : Compare actual project compiler RGB565 and segment closure without injecting native geometry.
 * Input Parameters: Qt sample column.
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::roundrectCanonicalFrames() //20261113-16 add
{
    QFETCH(QJsonObject,sample);
    HmiProject project = rectangleProject();
    HmiObject object = project.objects.first();
    QJsonValue value;
    QJsonArray rect;
    QString root = QFINDTESTDATA("../docs/stage5-evidence/native/roundrect");
    QString folder = sample.value(QStringLiteral("folder")).toString();
    QByteArray native = testFileBytes(QDir(root).filePath(folder.mid(folder.indexOf(QStringLiteral("roundrect/"))+10)+QStringLiteral("/SheetPixel.dat")));
    QVector<QByteArray> payloads;
    QString error;

    object.type = QStringLiteral("roundrect"); object.graphic.lineStyle = sample.value(QStringLiteral("lineStyle")).toInt(); object.graphic.fillStyle = sample.value(QStringLiteral("fillStyle")).toInt();
    project.objects.clear();
    Q_FOREACH (value,sample.value(QStringLiteral("rectangles")).toArray())
    {
        rect = value.toArray(); object.geometry = QRect(rect[0].toInt(),rect[1].toInt(),rect[2].toInt(),rect[3].toInt());
        object.id = QStringLiteral("rounded-%1").arg(project.objects.size()); project.objects.append(object);
    }
    QCOMPARE(decodeTestFrame(native).size(),768000);
    QVERIFY2(buildProjectPayload(project,&payloads,&error),qPrintable(error));
    QCOMPARE(payloads.size(),10); QCOMPARE(decodeTestFrame(payloads[8]),decodeTestFrame(native));
    QCOMPARE(payloads[5].size(),2400); QCOMPARE(int(static_cast<unsigned char>(payloads[1][2])),0);
    QVERIFY2(validateDevicePayloadStructure(payloads,&error),qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::roundrectUnprovedFieldsPreserveOutput_data
 * Description     : Enumerate unsupported roundrect rendering and geometric boundaries.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::roundrectUnprovedFieldsPreserveOutput_data() //20261113-16 add
{
    int variant;

    QTest::addColumn<int>("variant");
    for (variant=0;variant<16;++variant) { QTest::newRow(qPrintable(QString::number(variant))) << variant; }
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::roundrectUnprovedFieldsPreserveOutput
 * Description     : Reject unproved styles, small sizes, edge clipping and overlap before changing caller output.
 * Input Parameters: Qt fixture variant
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::roundrectUnprovedFieldsPreserveOutput() //20261113-16 add
{
    QFETCH(int,variant);
    HmiProject project = rectangleProject();
    HmiObject peer;
    DeviceCompilePlan plan;
    QJsonObject issue;
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QString error;

    project.objects[0].type = QStringLiteral("roundrect");
    plan.staticRoundrectCount=77;
    if (variant==0) { project.objects[0].graphic.lineStyle=2; }
    if (variant==1) { project.objects[0].graphic.lineWidth=2; }
    if (variant==2) { project.objects[0].graphic.alpha=128; }
    if (variant==3) { project.objects[0].graphic.lineColor=Qt::red; }
    if (variant==4) { project.objects[0].graphic.fillStyle=2; }
    if (variant==5) { project.objects[0].graphic.background=Qt::red; }
    if (variant==6) { project.objects[0].graphic.foreground=Qt::red; }
    if (variant==7) { project.objects[0].geometry=QRect(100,100,99,100); }
    if (variant==8) { project.objects[0].geometry.moveLeft(600); }
    if (variant==9) { project.objects[0].geometry.moveTop(380); }
    if (variant==10) { peer=project.objects[0]; peer.id=QStringLiteral("overlap"); project.objects.append(peer); }
    if (variant==11) { peer=project.objects[0]; peer.id=QStringLiteral("touching"); peer.geometry.moveLeft(300); project.objects.append(peer); }
    if (variant==12) { project.objects[0].graphic.radiusX=0; }
    if (variant==13) { project.objects[0].graphic.radiusY=20; }
    if (variant==14) { project.objects[0].geometry.moveLeft(0); }
    if (variant==15) { project.objects[0].geometry.moveTop(0); }
    QVERIFY(!prepareDeviceCompilePlan(project,&plan,&error,&issue)); QCOMPARE(plan.staticRoundrectCount,77);
    QVERIFY(!issue.value(QStringLiteral("property")).toString().isEmpty()); QVERIFY(!error.isEmpty());
    QVERIFY(!buildProjectPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::roundrectSourceAndPackageGates
 * Description     : Revalidate a roundrect device package while retaining native-SM and legacy text rejection.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.13
 ******************************************************************************/
void Stage5CompilerTests::roundrectSourceAndPackageGates() //20261113-16 add
{
    HmiProject project = rectangleProject();
    QTemporaryDir temporary;
    QString target = temporary.filePath(QStringLiteral("roundrect"));
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    QJsonObject manifest;
    QString error;

    project.objects[0].type = QStringLiteral("roundrect");
    QVERIFY(!exportNativeProjectSource(project,&source,&error)); QCOMPARE(source,QByteArrayLiteral("keep"));
    QVERIFY(!buildSingleTextPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
    QVERIFY2(exportProjectPayload(project,target,&error),qPrintable(error)); QVERIFY2(verifyDevicePackage(target,&manifest,&error),qPrintable(error));
    QCOMPARE(manifest.value(QStringLiteral("files")).toObject().size(),10); QCOMPARE(manifest.value(QStringLiteral("deviceAcknowledged")).toBool(true),false);
}


/******************************************************************************
 * Function Name   : Stage5CompilerTests::polygonCanonicalFrames_data
 * Description     : Bind every saved/reopened scene to its complete raw native frame.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::polygonCanonicalFrames_data() //20261114-1 add
{
    QString root=QFINDTESTDATA("../docs/stage5-evidence/native/polygon");
    QJsonArray cases=QJsonDocument::fromJson(testFileBytes(QDir(root).filePath(QStringLiteral("B-S-M-cell-area-r1.json")))).object().value(QStringLiteral("cases")).toArray();
    QJsonValue sample;
    QString run;
    int count=0;

    QCOMPARE(cases.size(),37);
    QTest::addColumn<QJsonObject>("sample");
    Q_FOREACH (sample,cases)
    {
        run=sample.toObject().value(QStringLiteral("run")).toString();
        if (run.contains(QStringLiteral("reopen")) || run.contains(QStringLiteral("default-repeat"))) { QTest::newRow(qPrintable(run))<<sample.toObject();++count; }
    }
    QCOMPARE(count,20);
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::polygonCanonicalFrames
 * Description     : Compile independent typed vertices and compare all 768000 background bytes.
 * Input Parameters: Qt sample column
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::polygonCanonicalFrames() //20261114-1 add
{
    QFETCH(QJsonObject,sample);
    HmiProject project=polygonProject();
    HmiObject object=project.objects.first();
    QJsonValue polygon;
    QJsonValue value;
    QJsonArray vertices;
    QJsonArray point;
    QString root=QFINDTESTDATA("../docs/stage5-evidence/native/polygon");
    QString folder=sample.value(QStringLiteral("folder")).toString();
    QByteArray native=testFileBytes(QDir(root).filePath(folder.mid(folder.indexOf(QStringLiteral("polygon/"))+8)+QStringLiteral("/SheetPixel.dat")));
    QByteArray source=testFileBytes(QDir(root).filePath(folder.mid(folder.indexOf(QStringLiteral("polygon/"))+8)+QStringLiteral("/input-native.sm")));
    DeviceCompilePlan plan;
    QVector<QByteArray> payloads;
    QString error;
    int minX;
    int minY;
    int maxX;
    int maxY;

    object.graphic.lineStyle=sample.value(QStringLiteral("lineStyle")).toInt();object.graphic.fillStyle=sample.value(QStringLiteral("fillStyle")).toInt();project.objects.clear();
    Q_FOREACH (polygon,sample.value(QStringLiteral("polygons")).toArray())
    {
        vertices=polygon.toArray();minX=800;minY=480;maxX=0;maxY=0;
        Q_FOREACH (value,vertices)
        {
            point=value.toArray();minX=qMin(minX,point[0].toInt());minY=qMin(minY,point[1].toInt());maxX=qMax(maxX,point[0].toInt());maxY=qMax(maxY,point[1].toInt());
        }
        object.geometry=QRect(minX,minY,maxX-minX,maxY-minY);object.graphic.points.clear();
        Q_FOREACH (value,vertices)
        {
            point=value.toArray();object.graphic.points.append(QPointF(double(point[0].toInt()-minX)/(maxX-minX),double(point[1].toInt()-minY)/(maxY-minY)));
        }
        object.id=QStringLiteral("polygon-%1").arg(project.objects.size());project.objects.append(object);
    }
    QCOMPARE(decodeTestFrame(native).size(),768000);
    QVERIFY(!source.isEmpty());QCOMPARE(QString::fromLatin1(QCryptographicHash::hash(source,QCryptographicHash::Sha256).toHex()),sample.value(QStringLiteral("sourceSha256")).toString());
    QVERIFY2(prepareDeviceCompilePlan(project,&plan,&error),qPrintable(error));QCOMPARE(plan.staticPolygonCount,project.objects.size());
    QVERIFY2(buildProjectPayload(project,&payloads,&error),qPrintable(error));QCOMPARE(payloads.size(),10);QCOMPARE(decodeTestFrame(payloads[8]),decodeTestFrame(native));
    QCOMPARE(payloads[5].size(),2400);QCOMPARE(int(static_cast<unsigned char>(payloads[1][2])),0);QVERIFY2(validateDevicePayloadStructure(payloads,&error),qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::polygonUnprovedFieldsPreserveOutput_data
 * Description     : Enumerate style, topology, coordinate and composition boundaries.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::polygonUnprovedFieldsPreserveOutput_data() //20261114-1 add
{
    int variant;
    QTest::addColumn<int>("variant");
    for (variant=0;variant<27;++variant) { QTest::newRow(qPrintable(QString::number(variant)))<<variant; }
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::polygonUnprovedFieldsPreserveOutput
 * Description     : Reject unsupported polygons without replacing caller-owned plans or payloads.
 * Input Parameters: Qt variant column
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::polygonUnprovedFieldsPreserveOutput() //20261114-1 add
{
    QFETCH(int,variant);
    HmiProject project=polygonProject();
    HmiObject peer;
    DeviceCompilePlan plan;
    QJsonObject issue;
    QVector<QByteArray> payloads={QByteArrayLiteral("keep")};
    QString error;

    plan.staticPolygonCount=77;
    if (variant==0) { project.objects[0].graphic.lineStyle=2; }
    if (variant==1) { project.objects[0].graphic.lineWidth=2; }
    if (variant==2) { project.objects[0].graphic.alpha=128; }
    if (variant==3) { project.objects[0].graphic.lineColor=Qt::red; }
    if (variant==4) { project.objects[0].graphic.fillStyle=2; }
    if (variant==5) { project.objects[0].graphic.background=Qt::white; }
    if (variant==6) { project.objects[0].graphic.foreground=Qt::black; }
    if (variant==7) { project.objects[0].geometry.setWidth(99); }
    if (variant==8) { project.objects[0].graphic.points[2]=QPointF(.751,1); }
    if (variant==9) { project.objects[0].geometry.moveLeft(1); }
    if (variant==10) { project.objects[0].geometry.moveLeft(600); }
    if (variant==11) { project.objects[0].geometry.moveTop(360); }
    if (variant==12) { project.objects[0].graphic.points[2]=project.objects[0].graphic.points[1]; }
    if (variant==13) { project.objects[0].graphic.points={QPointF(0,0),QPointF(1,0),QPointF(.5,0)}; }
    if (variant==14) { project.objects[0].graphic.points={QPointF(0,0),QPointF(1,1),QPointF(0,1),QPointF(1,0)}; }
    if (variant==15) { project.objects[0].graphic.points={QPointF(0,0),QPointF(1,0),QPointF(.75,1),QPointF(0,1),QPointF(1,0)}; }
    if (variant==16) { project.objects[0].graphic.points={QPointF(0,0),QPointF(.2,0),QPointF(.4,0),QPointF(.6,0),QPointF(.8,0),QPointF(1,0),QPointF(1,1),QPointF(.5,1),QPointF(0,1)}; }
    if (variant==17) { project.objects[0].graphic.points={QPointF(0,0),QPointF(1,1)}; }
    if (variant==18) { peer=project.objects[0];peer.id=QStringLiteral("overlap");project.objects.append(peer); }
    if (variant==19) { peer=project.objects[0];peer.id=QStringLiteral("touching");peer.geometry.moveLeft(300);project.objects.append(peer); }
    if (variant==20) { project.objects[0].graphic.points={QPointF(0,0),QPointF(.5,0),QPointF(1,0),QPointF(.75,1)}; }
    if (variant==21) { project.objects[0].graphic.lineStyle=0;project.objects[0].graphic.fillStyle=0;project.objects[0].graphic.background=Qt::red; }
    if (variant==22) { project.objects[0].geometry.setHeight(99); }
    if (variant==23) { project.objects[0].graphic.endArrow=1; }
    if (variant==24) { project.objects[0].geometry.moveLeft(499); }
    if (variant==25) { project.objects[0].geometry.moveTop(279); }
    if (variant==26) { project.objects[0].graphic.endCap=1; }
    QVERIFY(!prepareDeviceCompilePlan(project,&plan,&error,&issue));QCOMPARE(plan.staticPolygonCount,77);QVERIFY(!issue.value(QStringLiteral("property")).toString().isEmpty());
    QVERIFY(!buildProjectPayload(project,&payloads,&error));QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::polygonSourceAndPackageGates
 * Description     : Verify real atomic publication while native source and legacy text remain rejected.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::polygonSourceAndPackageGates() //20261114-1 add
{
    HmiProject project=polygonProject();
    QTemporaryDir temporary;
    QString target=temporary.filePath(QStringLiteral("polygon"));
    QByteArray source=QByteArrayLiteral("keep");
    QVector<QByteArray> payloads={QByteArrayLiteral("keep")};
    QJsonObject manifest;
    QString error;

    QVERIFY(!exportNativeProjectSource(project,&source,&error));QCOMPARE(source,QByteArrayLiteral("keep"));
    QVERIFY(!buildSingleTextPayload(project,&payloads,&error));QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
    QVERIFY2(exportProjectPayload(project,target,&error),qPrintable(error));QVERIFY2(verifyDevicePackage(target,&manifest,&error),qPrintable(error));
    QCOMPARE(manifest.value(QStringLiteral("files")).toObject().size(),10);QCOMPARE(manifest.value(QStringLiteral("deviceAcknowledged")).toBool(true),false);
}


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

/******************************************************************************
 * Function Name   : Stage5CompilerTests::ellipseSharedPrecisionGuards
 * Description     : Preserve fractional lattice semantics and refuse malformed shared coverage inputs without output replacement.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::ellipseSharedPrecisionGuards() //20261114-2 add
{
    QPainterPath path;
    QPainterPath invalid;
    QPainterPath curve;
    QByteArray exact;
    QByteArray legacy;
    QByteArray preserved=QByteArrayLiteral("keep");
    QString error;

    path.moveTo(100+1.0/256,100);path.lineTo(101+1.0/256,100);path.lineTo(101+1.0/256,101);path.lineTo(100+1.0/256,101);path.closeSubpath();
    QVERIFY2(deviceStraightPathCoverage(path,DeviceStraightCoveragePrecision::Exact8Bit,&exact,&error),qPrintable(error));
    QVERIFY2(deviceStraightPathCoverage(path,DeviceStraightCoveragePrecision::Quantized26_6,&legacy,&error),qPrintable(error));
    QCOMPARE(exact.size(),800*480);QCOMPARE(legacy.size(),800*480);QVERIFY(exact!=legacy);
    invalid.moveTo(100+1.0/512,100);invalid.lineTo(101,101);invalid.lineTo(100,101);invalid.closeSubpath();
    QVERIFY(!deviceStraightPathCoverage(invalid,DeviceStraightCoveragePrecision::Exact8Bit,&preserved,&error));QCOMPARE(preserved,QByteArrayLiteral("keep"));
    curve.moveTo(100,100);curve.cubicTo(120,100,150,150,200,200);
    QVERIFY(!deviceStraightPathCoverage(curve,DeviceStraightCoveragePrecision::Exact8Bit,&preserved,&error));QCOMPARE(preserved,QByteArrayLiteral("keep"));
    QVERIFY(!deviceStraightPathCoverage(path,static_cast<DeviceStraightCoveragePrecision>(9),&preserved,&error));QCOMPARE(preserved,QByteArrayLiteral("keep"));
    QVERIFY(!deviceStraightPathCoverage(path,DeviceStraightCoveragePrecision::Exact8Bit,nullptr,&error));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::sectorCanonicalFrames_data
 * Description     : Require every native default or reopened fixture, including local paths mapped by their verified integer translations.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::sectorCanonicalFrames_data() //20261114-3 add
{
    QString root=QFINDTESTDATA("../docs/stage5-evidence/native/sector");
    QJsonObject corpus=QJsonDocument::fromJson(testFileBytes(QDir(root).filePath(QStringLiteral("B-S-M-sector-cell-area-r1.json")))).object();
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
 * Function Name   : Stage5CompilerTests::sectorCanonicalFrames
 * Description     : Compile fresh typed geometry and compare the complete native RGB565 background and source identity.
 * Input Parameters: Qt sample column
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::sectorCanonicalFrames() //20261114-3 add
{
    QFETCH(QJsonObject,sample);
    HmiProject project=sectorProject();
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
        object.id=QStringLiteral("sector-%1").arg(project.objects.size());project.objects.append(object);
    }
    QVERIFY(!source.isEmpty());QCOMPARE(QString::fromLatin1(QCryptographicHash::hash(source,QCryptographicHash::Sha256).toHex()),sample.value(QStringLiteral("sourceSha256")).toString());
    QCOMPARE(decodeTestFrame(native).size(),768000);
    QVERIFY2(prepareDeviceCompilePlan(project,&plan,&error),qPrintable(error));QCOMPARE(plan.staticSectorCount,project.objects.size());
    QVERIFY2(buildProjectPayload(project,&payloads,&error),qPrintable(error));QCOMPARE(payloads.size(),10);QCOMPARE(decodeTestFrame(payloads[8]),decodeTestFrame(native));
    QCOMPARE(payloads[5].size(),2400);QCOMPARE(int(static_cast<unsigned char>(payloads[1][2])),0);QVERIFY2(validateDevicePayloadStructure(payloads,&error),qPrintable(error));
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::sectorUnprovedFieldsPreserveOutput_data
 * Description     : Enumerate geometry, appearance, dormant drafts and composition guard boundaries.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::sectorUnprovedFieldsPreserveOutput_data() //20261114-3 add
{
    int variant;
    QTest::addColumn<int>("variant");
    for (variant=0;variant<30;++variant) { QTest::newRow(qPrintable(QString::number(variant)))<<variant; }
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::sectorUnprovedFieldsPreserveOutput
 * Description     : Preserve caller plans and payloads for each unsupported sector configuration.
 * Input Parameters: Qt variant column
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::sectorUnprovedFieldsPreserveOutput() //20261114-3 add
{
    QFETCH(int,variant);
    HmiProject project=sectorProject();
    HmiObject peer;
    DeviceCompilePlan plan;
    QJsonObject issue;
    QVector<QByteArray> payloads={QByteArrayLiteral("keep")};
    QString error;

    plan.staticSectorCount=77;
    if (variant==0) { project.objects[0].graphic.lineStyle=2; }
    if (variant==1) { project.objects[0].graphic.lineWidth=2; }
    if (variant==2) { project.objects[0].graphic.alpha=128; }
    if (variant==3) { project.objects[0].graphic.lineColor=Qt::red; }
    if (variant==4) { project.objects[0].graphic.fillStyle=2; }
    if (variant==5) { project.objects[0].graphic.background=Qt::black; }
    if (variant==6) { project.objects[0].graphic.foreground=Qt::white; }
    if (variant==7) { project.objects[0].geometry.setSize(QSize(99,99)); }
    if (variant==8) { project.objects[0].geometry.setHeight(99); }
    if (variant==9) { project.objects[0].geometry.moveLeft(1); }
    if (variant==10) { project.objects[0].geometry.moveTop(1); }
    if (variant==11) { project.objects[0].geometry.moveLeft(499); }
    if (variant==12) { project.objects[0].geometry.moveTop(179); }
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
    if (variant==27) { project.objects[0].geometry.setSize(QSize(241,241)); }
    if (variant==28) { project.objects[0].geometry.setHeight(199); }
    if (variant==29) { peer=project.objects[0];peer.id=QStringLiteral("patch-edge-contact");peer.geometry.moveLeft(303);project.objects.append(peer); }
    QVERIFY(!prepareDeviceCompilePlan(project,&plan,&error,&issue));QCOMPARE(plan.staticSectorCount,77);QVERIFY(!issue.value(QStringLiteral("property")).toString().isEmpty());
    QVERIFY(!buildProjectPayload(project,&payloads,&error));QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
}

/******************************************************************************
 * Function Name   : Stage5CompilerTests::sectorSourceAndPackageGates
 * Description     : Keep complete native source and the legacy compiler separate from verified device publication.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void Stage5CompilerTests::sectorSourceAndPackageGates() //20261114-3 add
{
    HmiProject project=sectorProject();
    QTemporaryDir temporary;
    QString target=temporary.filePath(QStringLiteral("sector"));
    QByteArray source=QByteArrayLiteral("keep");
    QVector<QByteArray> payloads={QByteArrayLiteral("keep")};
    QJsonObject manifest;
    QString error;

    QVERIFY(!exportNativeProjectSource(project,&source,&error));QCOMPARE(source,QByteArrayLiteral("keep"));
    QVERIFY(!buildSingleTextPayload(project,&payloads,&error));QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
    QVERIFY2(exportProjectPayload(project,target,&error),qPrintable(error));QVERIFY2(verifyDevicePackage(target,&manifest,&error),qPrintable(error));
    QCOMPARE(manifest.value(QStringLiteral("files")).toObject().size(),10);QCOMPARE(manifest.value(QStringLiteral("deviceAcknowledged")).toBool(true),false);
}

QTEST_MAIN(Stage5CompilerTests)
#include "stage5_compiler_tests.moc"
