#include "device_compile_plan.h"
#include "device_package.h"
#include "independent_downloader.h"
#include "project_capabilities.h"
#include "native_project_codec.h"
#include "lzokay.hpp"
#include <QDir>
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
};

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
    if (variant == 10) { project.objects[0].type = QStringLiteral("rect"); }
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
    QCOMPARE(found.size(),5); QVERIFY2(buildProjectPayload(project,&payloads,&error),qPrintable(error));
    QCOMPARE(payloads.size(),12); QVERIFY2(validateDevicePayloadStructure(payloads,&error),qPrintable(error));
    QCOMPARE(payloads[10].size(),5*640);
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

QTEST_MAIN(Stage5CompilerTests)
#include "stage5_compiler_tests.moc"
