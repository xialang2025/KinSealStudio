#include "project_model.h"
#include "project_capabilities.h"
#include "independent_downloader.h"
#include "native_project_codec.h"
#include "native_test_project.h"
#include "text_payload_compiler.h"
#include "device_address_compiler.h"
#include "main_window.h"
#include "hmi_canvas.h"

#include <QFile>
#include <QJsonArray>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QtEndian>
#include <QtTest>

//20261110-5 add: All backend checks are offline; no test calls a real downloader.
class Stage1BackendTests : public QObject
{
    Q_OBJECT
private slots:
    void singlePageCompilationRemainsAvailable();
    void multiPageNeverCompilesOnlyTheSelectedPage();
    void specialPageNeverUsesTheOrdinaryPageBackend();
    void startupAndEditorLanguagesAreIndependent();
    void capabilityQueriesSeparateModelAndBackend();
    void editorSwitchesPagesWithoutDiscardingHiddenObjects();
    void resourceDraftFailurePreservesEditorState();
    void legacyEntriesRejectUnrepresentedConfiguration();
    void capabilitiesExplainKnownBlockedRoutes();
    void independentNativeLanguageSettingsCompile_data(); //20261110-6 add
    void independentNativeLanguageSettingsCompile();
};

/******************************************************************************
 * Function Name   : independentNativeLanguageSettingsCompile_data
 * Description     : Associate saved native GUI sources with actual native compiler system segments.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage1BackendTests::independentNativeLanguageSettingsCompile_data() //20261110-6 add
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<int>("expectedIndex");
    QTest::addColumn<int>("expectedCount");
    QTest::newRow("settings-English-editor-Chinese") << QStringLiteral("settings1-editor0") << 0 << 2;
    QTest::newRow("settings-Chinese-editor-English") << QStringLiteral("settings0-editor1") << 1 << 2;
    QTest::newRow("three-languages") << QStringLiteral("settings2-editor1-three") << 1 << 3;
}

/******************************************************************************
 * Function Name   : independentNativeLanguageSettingsCompile
 * Description     : Compare the recovered language carrier with independently compiled native bytes.
 * Input Parameters: Qt fixture data.
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage1BackendTests::independentNativeLanguageSettingsCompile() //20261110-6 add
{
    QFETCH(QString, name);
    QFETCH(int, expectedIndex);
    QFETCH(int, expectedCount);
    QString directory;
    QString error;
    QFile file;
    HmiProject project;
    QVector<QByteArray> payloads;
    QByteArray expected;

    directory = QFINDTESTDATA("fixtures/native-source/language");
    QVERIFY(!directory.isEmpty());
    file.setFileName(directory + QLatin1Char('/') + name + QStringLiteral("-native.sm"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QVERIFY2(importNativeProjectSource(file.readAll(), &project, &error), qPrintable(error));
    file.close();
    project.activeLanguage = project.languages.at((expectedIndex + 1) % expectedCount);
    QVERIFY2(buildProjectPayload(project, &payloads, &error), qPrintable(error));
    QVERIFY(!payloads.isEmpty());
    QCOMPARE(qFromLittleEndian<u16>(payloads.at(0).constData() + 2172), static_cast<u16>(expectedIndex));
    QCOMPARE(qFromLittleEndian<u16>(payloads.at(0).constData() + 2174), static_cast<u16>(expectedCount));
    file.setFileName(directory + QLatin1Char('/') + name + QStringLiteral("-SysPara.dat"));
    QVERIFY(file.open(QIODevice::ReadOnly));
    expected = file.readAll();
    QCOMPARE(payloads.at(0).size(), expected.size()); //20261110-6 change: The original system segment contains documented non-deterministic opaque fields.
    QCOMPARE(payloads.at(0).mid(2172, 4), expected.mid(2172, 4));
}

/******************************************************************************
 * Function Name   : singleTextProject
 * Description     : Construct a minimal valid legacy project for backend regression.
 * Input Parameters: None
 * Return Value    : One ordinary page with one editable text object.
 * Date            : 2026.11.10
 ******************************************************************************/
static HmiProject singleTextProject() //20261110-5 add
{
    HmiProject project;
    HmiObject object;

    object.id = QStringLiteral("stage1-text");
    object.name = QStringLiteral("测试文字");
    object.type = QStringLiteral("text");
    object.text = QStringLiteral("TEST");
    object.address = QStringLiteral("LW0");
    object.geometry = QRect(244, 112, 150, 50);
    project.objects.append(object);
    return project;
}

/******************************************************************************
 * Function Name   : secondPage
 * Description     : Build a distinct second page containing its own persistent object.
 * Input Parameters: None
 * Return Value    : A page suitable for exercising whole-project boundaries.
 * Date            : 2026.11.10
 ******************************************************************************/
static HmiPage secondPage() //20261110-5 add
{
    HmiPage page;
    HmiObject object;

    page.id = QStringLiteral("stage1-page-two");
    page.kind = QStringLiteral("normal");
    page.name = QStringLiteral("画面二");
    page.width = 800;
    page.height = 480;
    object = singleTextProject().objects.first();
    object.id = QStringLiteral("stage1-text-two");
    object.text = QStringLiteral("PAGE TWO");
    page.objects.append(object);
    return page;
}

/******************************************************************************
 * Function Name   : singlePageCompilationRemainsAvailable
 * Description     : Check the original one-page path with the new whole-model gates.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage1BackendTests::singlePageCompilationRemainsAvailable() //20261110-5 add
{
    HmiProject project;
    QVector<QByteArray> payloads;
    QString error;

    project = singleTextProject();
    QVERIFY2(project.validateModel(&error), qPrintable(error));
    QVERIFY2(buildProjectPayload(project, &payloads, &error), qPrintable(error));
    QCOMPARE(payloads.size(), 10);
}

/******************************************************************************
 * Function Name   : multiPageNeverCompilesOnlyTheSelectedPage
 * Description     : Reject all one-page adapters transactionally when a second page exists.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage1BackendTests::multiPageNeverCompilesOnlyTheSelectedPage() //20261110-5 add
{
    HmiProject project;
    QVector<QByteArray> payloads;
    QByteArray frame;
    QByteArray scan;
    DeviceAddressCompiler compiler;
    QTemporaryDir directory;
    QFile existing;
    QString error;

    project = singleTextProject();
    QVERIFY2(project.addPage(secondPage(), &error), qPrintable(error));
    payloads.append(QByteArray("unchanged"));
    QVERIFY(!buildProjectPayload(project, &payloads, &error));
    QCOMPARE(payloads, QVector<QByteArray>{QByteArray("unchanged")});
    QVERIFY(!buildSingleTextPayload(project, &payloads, &error));
    QCOMPARE(payloads, QVector<QByteArray>{QByteArray("unchanged")});
    frame = QByteArray("old-frame");
    QVERIFY(!renderSingleTextFrame(project, QStringLiteral("Arial"), 16, false, &frame, &error));
    QCOMPARE(frame, QByteArray("old-frame"));
    scan = QByteArray("old-scan");
    QVERIFY(!compiler.prepare(project, &scan, &error));
    QCOMPARE(scan, QByteArray("old-scan"));
    QVERIFY(directory.isValid());
    existing.setFileName(directory.filePath(QStringLiteral("existing.sm")));
    QVERIFY(existing.open(QIODevice::WriteOnly));
    QCOMPARE(existing.write("previous-file"), qint64(13));
    existing.close();
    QVERIFY(!exportNativeTestProject(project, existing.fileName(), &error));
    QVERIFY(existing.open(QIODevice::ReadOnly));
    QCOMPARE(existing.readAll(), QByteArray("previous-file"));
}

/******************************************************************************
 * Function Name   : specialPageNeverUsesTheOrdinaryPageBackend
 * Description     : Prevent role metadata from being lost by the legacy raster compiler.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage1BackendTests::specialPageNeverUsesTheOrdinaryPageBackend() //20261110-5 add
{
    HmiProject project;
    HmiPage windowPage;
    QVector<QByteArray> payloads;
    QString error;

    project = singleTextProject();
    windowPage = secondPage();
    windowPage.kind = QStringLiteral("window");
    QVERIFY2(project.addPage(windowPage, &error), qPrintable(error));
    QVERIFY2(project.selectPage(windowPage.id, &error), qPrintable(error));
    QVERIFY2(project.validateModel(&error), qPrintable(error));
    QVERIFY(!buildProjectPayload(project, &payloads, &error));
    QVERIFY(payloads.isEmpty());
}

/******************************************************************************
 * Function Name   : startupAndEditorLanguagesAreIndependent
 * Description     : Resolve startup indices by registered names rather than the editor's selected language.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage1BackendTests::startupAndEditorLanguagesAreIndependent() //20261110-5 add
{
    HmiProject project;
    QString error;
    u16 index;

    project = singleTextProject();
    project.languages = {QStringLiteral("中文"), QStringLiteral("English")};
    project.activeLanguage = QStringLiteral("English");
    project.startupLanguage = QStringLiteral("中文");
    index = 7;
    QVERIFY2(nativeProjectInitialLanguageIndex(project, &index, &error), qPrintable(error));
    QCOMPARE(index, u16(0));
    project.startupLanguage = QStringLiteral("English");
    project.activeLanguage = QStringLiteral("中文");
    QVERIFY2(nativeProjectInitialLanguageIndex(project, &index, &error), qPrintable(error));
    QCOMPARE(index, u16(1));
    project.startupLanguage = QStringLiteral("missing");
    QVERIFY(!nativeProjectInitialLanguageIndex(project, &index, &error));
    QCOMPARE(index, u16(1));
}

/******************************************************************************
 * Function Name   : capabilityQueriesSeparateModelAndBackend
 * Description     : Keep editable multi-page files distinct from a verified device backend candidate.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage1BackendTests::capabilityQueriesSeparateModelAndBackend() //20261110-5 add
{
    HmiProject project;
    QJsonObject report;
    QString error;

    project = singleTextProject();
    QVERIFY2(project.addPage(secondPage(), &error), qPrintable(error));
    report = describeProjectCapabilities(project);
    QVERIFY(report.value(QStringLiteral("modelPersistence")).toObject().value(QStringLiteral("allowed")).toBool());
    QCOMPARE(report.value(QStringLiteral("nativeSource")).toObject().value(QStringLiteral("status")).toString(), QStringLiteral("blocked"));
    QCOMPARE(report.value(QStringLiteral("deviceCompile")).toObject().value(QStringLiteral("status")).toString(), QStringLiteral("blocked"));
    QVERIFY(!report.value(QStringLiteral("deviceCompile")).toObject().value(QStringLiteral("usbAccessed")).toBool());
    QVERIFY(!report.value(QStringLiteral("target")).toObject().value(QStringLiteral("firmwareVersionConfirmed")).toBool());
}

/******************************************************************************
 * Function Name   : editorSwitchesPagesWithoutDiscardingHiddenObjects
 * Description     : Exercise the actual hierarchy/canvas bridge while retaining the page not being edited.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage1BackendTests::editorSwitchesPagesWithoutDiscardingHiddenObjects() //20261110-5 add
{
    HmiProject project;
    HmiProject saved;
    MainWindow window;
    QTemporaryDir directory;
    QTreeWidgetItem *second;
    QString error;
    QString firstId;

    project = singleTextProject();
    firstId = project.activePageId;
    QVERIFY2(project.addPage(secondPage(), &error), qPrintable(error));
    window.setProject(project, QString());
    QCOMPARE(window.m_projectTree->topLevelItem(0)->childCount(), 2);
    second = window.m_projectTree->topLevelItem(0)->child(1);
    window.m_projectTree->setCurrentItem(second);
    QCOMPARE(window.m_project.activePageId, QStringLiteral("stage1-page-two"));
    QCOMPARE(window.m_canvas->objects().first().id, QStringLiteral("stage1-text-two"));
    QCOMPARE(window.m_project.startupPageId, firstId);
    QVERIFY(directory.isValid());
    QVERIFY2(window.m_project.save(directory.filePath(QStringLiteral("two-pages.ksp")), &error), qPrintable(error));
    QVERIFY2(HmiProject::load(directory.filePath(QStringLiteral("two-pages.ksp")), &saved, &error), qPrintable(error));
    QCOMPARE(saved.pageCount(), 2);
    QVERIFY2(saved.selectPage(firstId, &error), qPrintable(error));
    QCOMPARE(saved.objects.first().text, QStringLiteral("TEST"));
}

/******************************************************************************
 * Function Name   : resourceDraftFailurePreservesEditorState
 * Description     : Reject deletion of referenced resources before either model or canvas is changed.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage1BackendTests::resourceDraftFailurePreservesEditorState() //20261110-5 add
{
    HmiProject project;
    HmiReference reference;
    MainWindow window;
    QJsonObject before;
    HmiObject edited;
    QString error;

    project = singleTextProject();
    project.macros.insert(QStringLiteral("kept"), QStringLiteral("source"));
    project.macroIds.insert(QStringLiteral("kept"), QStringLiteral("macro-kept"));
    reference.id = QStringLiteral("macro-ref");
    reference.ownerKind = QStringLiteral("project"); reference.ownerId = project.projectId;
    reference.field = QStringLiteral("editor.bookmark");
    reference.targetKind = QStringLiteral("macro"); reference.targetId = QStringLiteral("macro-kept");
    project.references.append(reference);
    QVERIFY2(project.validateModel(&error), qPrintable(error));
    window.setProject(project, QString()); before = window.m_project.toJson();
    edited = project.objects.first(); edited.text = QStringLiteral("discarded draft");
    QVERIFY(!window.applyControlDraft(edited, {}, {}, true, &error));
    QCOMPARE(window.m_project.toJson(), before);
    QCOMPARE(window.m_canvas->objects().first().text, QStringLiteral("TEST"));
    QVERIFY(!window.applyControlDraft(edited, {}, {}, false, &error));
    QCOMPARE(window.m_project.toJson(), before);
}

/******************************************************************************
 * Function Name   : legacyEntriesRejectUnrepresentedConfiguration
 * Description     : Keep old fixed templates from discarding languages, connections or resources.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage1BackendTests::legacyEntriesRejectUnrepresentedConfiguration() //20261110-5 add
{
    HmiProject project;
    HmiProject restored;
    PlcConnection connection;
    QVector<QByteArray> payloads;
    QTemporaryDir directory;
    QFile old;
    QFile snapshot(QStringLiteral(":/native/bit-button-default.sm")); //20261110-5 add
    QString error;

    QVERIFY(directory.isValid()); old.setFileName(directory.filePath(QStringLiteral("old.sm")));
    QVERIFY(old.open(QIODevice::WriteOnly)); old.write("previous"); old.close();
    project = singleTextProject(); project.languages.append(QStringLiteral("English"));
    QVERIFY(!exportNativeTestProject(project, old.fileName(), &error));
    project = singleTextProject(); project.languages = {QStringLiteral("English")}; project.activeLanguage = QStringLiteral("English");
    project.objects.first().address.clear(); //20261110-5 add: Native static text has no operation-address carrier; the positive source case uses the actual editor default.
    QVERIFY(!exportNativeTestProject(project, old.fileName(), &error));
    payloads = {QByteArray("previous")};
    QVERIFY2(buildSingleTextPayload(project, &payloads, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(qUncompress(payloads.at(7)), &restored, &error), qPrintable(error));
    QCOMPARE(restored.languages, project.languages);
    QVERIFY2(buildProjectPayload(project, &payloads, &error), qPrintable(error));
    project = singleTextProject(); project.macros.insert(QStringLiteral("draft"), QStringLiteral("source"));
    QVERIFY(!exportNativeTestProject(project, old.fileName(), &error));
    payloads = {QByteArray("previous")};
    QVERIFY(!buildSingleTextPayload(project, &payloads, &error));
    QCOMPARE(payloads, QVector<QByteArray>{QByteArray("previous")});
    project = singleTextProject();
    // A complete native seed is still not the legacy text fixture and must survive refusal unchanged.
    QVERIFY(snapshot.open(QIODevice::ReadOnly)); project.nativeSourceSnapshot = snapshot.readAll();
    QVERIFY2(project.validateModel(&error), qPrintable(error));
    QVERIFY(!exportNativeTestProject(project, old.fileName(), &error));
    project = singleTextProject(); connection.id = QStringLiteral("plc");
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.channel = PlcChannel::Com2; connection.station = QStringLiteral("1");
    connection.nodeKnown = true; connection.node = 1; project.connections.append(connection);
    QVERIFY2(project.validateModel(&error), qPrintable(error));
    QVERIFY(!exportNativeTestProject(project, old.fileName(), &error));
    QVERIFY(!buildSingleTextPayload(project, &payloads, &error));
    QCOMPARE(payloads, QVector<QByteArray>{QByteArray("previous")});
    QVERIFY(old.open(QIODevice::ReadOnly)); QCOMPARE(old.readAll(), QByteArray("previous"));
}

/******************************************************************************
 * Function Name   : capabilitiesExplainKnownBlockedRoutes
 * Description     : Report definite offline blockers instead of suggesting unsupported routes are candidates.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage1BackendTests::capabilitiesExplainKnownBlockedRoutes() //20261110-5 add
{
    HmiProject project;
    PlcConnection connection;
    QJsonObject device;

    project = singleTextProject(); connection.id = QStringLiteral("plc");
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.channel = PlcChannel::Com2; connection.station = QStringLiteral("1");
    connection.nodeKnown = true; connection.node = 1; project.connections.append(connection);
    device = describeProjectCapabilities(project).value(QStringLiteral("deviceCompile")).toObject();
    QCOMPARE(device.value(QStringLiteral("status")).toString(), QStringLiteral("blocked"));
    QVERIFY(!device.value(QStringLiteral("reason")).toString().isEmpty());
    project = HmiProject(); device = describeProjectCapabilities(project).value(QStringLiteral("deviceCompile")).toObject();
    QCOMPARE(device.value(QStringLiteral("status")).toString(), QStringLiteral("blocked"));
    QVERIFY(!device.value(QStringLiteral("reason")).toString().isEmpty());
}

QTEST_MAIN(Stage1BackendTests)
#include "stage1_backend_tests.moc"
