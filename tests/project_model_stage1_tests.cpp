#include <QtTest>
#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTemporaryDir>
#include "project_model.h"

//20261110-5 add: These tests exercise editor-format transactions without windows or device access.
class ProjectModelStage1Tests : public QObject
{
    Q_OBJECT
private slots:
    void legacySchemas_data();
    void legacySchemas();
    void actualStage0EmptyMacroArrayMigrates();
    void pageSelectionAndSerialization();
    void copyRenameDeleteReferences();
    void invalidPageTransactions();
    void macroIdentityRename();
    void languageTargetsAndDrafts();
    void malformedProject_data();
    void malformedProject();
    void schemaCapabilityFloors_data();
    void schemaCapabilityFloors();
    void objectFailureIsAtomic();
    void inactiveMetadataAndConfigurations();
    void invalidReferences_data();
    void invalidReferences();
    void saveAndLoadFailuresAreAtomic();
    void snapshotAndImageRoundTrip();
    void backendCapabilityBoundary();
};

/******************************************************************************
 * Function Name   : modelObject
 * Description     : Create one of the five established model records without native seeds.
 * Input Parameters: kind
 * Return Value    : Editable object with distinct identity and valid subtype constraints.
 * Date            : 2026.11.10
 ******************************************************************************/
static HmiObject modelObject(int kind)
{
    HmiObject object;

    object.id = QStringLiteral("object-%1").arg(kind); object.name = object.id;
    object.type = kind == 0 ? QStringLiteral("text") : QStringLiteral("switch");
    object.geometry = QRect(10 + kind * 100, 20, 80, 50); object.text = QStringLiteral("Draft");
    if (kind == 0)
    {
        object.hasStaticTextProperties = true; object.staticText.fontSize = 16.5; object.staticText.spacing = 0.25;
        object.staticText.languageTexts.insert(QStringLiteral("中文"), QStringLiteral("文本"));
        object.staticText.languageTexts.insert(QStringLiteral("English"), QStringLiteral("Text"));
        object.layer = 2.5; object.group = QStringLiteral("group-source");
    }
    else
    {
        object.hasBitButtonProperties = true; object.address = kind == 2 ? QStringLiteral("LW0") : QStringLiteral("LB0");
        if (kind == 2) { object.hasWordButtonProperties = true; object.bitButton.indicator = false; object.bitButton.dualMonitor = false; }
        if (kind == 3) { object.hasLampProperties = true; object.address.clear(); object.bitButton.monitorSame = false; object.bitButton.bitIndex = 0; }
        if (kind == 4) { object.hasFunctionButtonProperties = true; object.address.clear(); object.bitButton.indicator = false; object.bitButton.dualMonitor = false; }
    }
    return object;
}

/******************************************************************************
 * Function Name   : snapshotFixture
 * Description     : Read the retained native source positive fixture by test-source-relative path.
 * Input Parameters: None
 * Return Value    : Exact fixture bytes or empty on an actual missing fixture.
 * Date            : 2026.11.10
 ******************************************************************************/
static QByteArray snapshotFixture()
{
    QFile file(QFINDTESTDATA("../samples/native-device-test/CodexProbeTest.sm"));

    if (!file.open(QIODevice::ReadOnly)) { return {}; }
    return file.readAll();
}

/******************************************************************************
 * Function Name   : legacySchemas_data
 * Description     : Cover each shipped schema, including its original minimum version trigger.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::legacySchemas_data()
{
    int schema;

    QTest::addColumn<int>("schema");
    for (schema = 1; schema <= 8; ++schema) { QTest::newRow(qPrintable(QString::number(schema))) << schema; }
}

/******************************************************************************
 * Function Name   : legacySchemas
 * Description     : Preserve old serialization semantics while assigning the initial stable page identity.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::legacySchemas()
{
    QFETCH(int, schema);
    HmiProject original;
    HmiProject loaded;
    PlcConnection connection;
    QJsonObject json;
    QString error;

    if (schema == 2) { original.objects = {modelObject(0)}; }
    if (schema == 3) { original.objects = {modelObject(1)}; }
    if (schema == 4)
    {
        connection.id = QStringLiteral("com1"); connection.channel = PlcChannel::Com1;
        connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
        connection.station = QStringLiteral("1"); connection.nodeKnown = true; connection.node = 1;
        original.connections.append(connection);
    }
    if (schema == 5) { original.nativeSourceSnapshot = snapshotFixture(); QVERIFY(!original.nativeSourceSnapshot.isEmpty()); }
    if (schema == 6) { original.objects = {modelObject(2)}; }
    if (schema == 7) { original.objects = {modelObject(3)}; }
    if (schema == 8) { original.objects = {modelObject(4)}; }
    json = original.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), schema);
    QVERIFY2(HmiProject::fromJson(json, &loaded, &error), qPrintable(error));
    QCOMPARE(loaded.toJson(), json); QCOMPARE(loaded.activePageId, QStringLiteral("page-1"));
    QCOMPARE(loaded.startupPageId, loaded.activePageId); QCOMPARE(loaded.pageCount(), 1);
    QVERIFY(loaded.startupLanguage.isEmpty()); QVERIFY(loaded.validateSinglePageBackend(&error));
}

/******************************************************************************
 * Function Name   : pageSelectionAndSerialization
 * Description     : Verify active-page edits, explicit order and startup state survive repeated switches and save.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::pageSelectionAndSerialization()
{
    HmiProject project;
    HmiProject loaded;
    HmiPage page;
    QString error;

    project.objects = {modelObject(0)}; page.id = QStringLiteral("page-2"); page.name = QStringLiteral("Second");
    page.objects = {modelObject(4)}; QVERIFY2(project.addPage(page, &error), qPrintable(error));
    project.pageOrder = {page.id, project.activePageId}; project.languages.append(QStringLiteral("English"));
    project.startupLanguage = QStringLiteral("English");
    QVERIFY(project.selectPage(page.id, &error)); project.objects[0].name = QStringLiteral("Edited on second");
    project.pageName = QStringLiteral("Renamed legacy field");
    QCOMPARE(project.startupPageId, QStringLiteral("page-1"));
    QVERIFY(project.selectPage(QStringLiteral("page-1"), &error));
    QCOMPARE(project.objects.first().id, QStringLiteral("object-0"));
    QVERIFY(project.selectPage(page.id, &error)); QCOMPARE(project.objects.first().name, QStringLiteral("Edited on second"));
    QCOMPARE(project.pageName, QStringLiteral("Renamed legacy field")); QCOMPARE(project.inactivePages.size(), 1);
    QCOMPARE(project.toJson().value(QStringLiteral("schemaVersion")).toInt(), 9);
    QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error));
    QCOMPARE(loaded.toJson(), project.toJson()); QCOMPARE(loaded.pageOrder.first(), page.id);
    QCOMPARE(loaded.startupLanguage, QStringLiteral("English"));
}

/******************************************************************************
 * Function Name   : copyRenameDeleteReferences
 * Description     : Remap cloned identities and internal links, and refuse deletion while external references exist.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::copyRenameDeleteReferences()
{
    HmiProject project;
    HmiReference reference;
    HmiReference inbound;
    HmiPage copied;
    QJsonObject before;
    QString error;
    QString copiedId;

    project.objects = {modelObject(0)};
    reference.id = QStringLiteral("self-link"); reference.ownerKind = QStringLiteral("object"); reference.ownerId = project.objects[0].id;
    reference.field = QStringLiteral("editor.navigation"); reference.targetKind = QStringLiteral("page"); reference.targetId = project.activePageId;
    project.references.append(reference); QVERIFY(project.validateModel(&error));
    QVERIFY2(project.copyPage(project.activePageId, QStringLiteral("Copy"), &copiedId, &error), qPrintable(error));
    copied = project.inactivePages.first(); QCOMPARE(copied.id, copiedId); QVERIFY(copied.objects.first().id != project.objects.first().id);
    QVERIFY(copied.objects.first().group != project.objects.first().group); QCOMPARE(copied.objects.first().staticText.toJson(), project.objects.first().staticText.toJson());
    QCOMPARE(project.references.size(), 2); QCOMPARE(project.references.last().ownerId, copied.objects.first().id);
    QCOMPARE(project.references.last().targetId, copiedId); QVERIFY(project.references.last().id != reference.id);
    QVERIFY(project.renamePage(copiedId, QStringLiteral("Display name only"), &error)); QCOMPARE(project.references.last().targetId, copiedId);
    inbound = reference; inbound.id = QStringLiteral("inbound"); inbound.ownerKind = QStringLiteral("project"); inbound.ownerId = project.projectId;
    inbound.field = QStringLiteral("editor.bookmark"); inbound.targetId = copiedId; project.references.append(inbound);
    before = project.toJson(); QVERIFY(!project.removePage(copiedId, &error)); QCOMPARE(project.toJson(), before);
    project.references.removeLast(); QVERIFY(project.selectPage(copiedId, &error));
    QVERIFY2(project.removePage(copiedId, &error), qPrintable(error)); QCOMPARE(project.pageCount(), 1);
    QCOMPARE(project.activePageId, project.startupPageId); QCOMPARE(project.references.size(), 1);
}

/******************************************************************************
 * Function Name   : invalidPageTransactions
 * Description     : Reject duplicate page/object IDs and bad startup/order without changing live state or output IDs.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::invalidPageTransactions()
{
    HmiProject project;
    HmiProject output;
    HmiPage page;
    QJsonObject before;
    QJsonObject invalid;
    QJsonArray pages;
    QString error;
    QString newId = QStringLiteral("sentinel");

    project.objects = {modelObject(0)}; before = project.toJson();
    page.id = project.activePageId; page.name = QStringLiteral("Duplicate");
    QVERIFY(!project.addPage(page, &error)); QCOMPARE(project.toJson(), before);
    page.id = QStringLiteral("other"); page.objects = project.objects;
    QVERIFY(!project.addPage(page, &error)); QCOMPARE(project.toJson(), before);
    QVERIFY(!project.copyPage(project.activePageId, QString(), &newId, &error)); QCOMPARE(newId, QStringLiteral("sentinel")); QCOMPARE(project.toJson(), before);
    QVERIFY(!project.renamePage(project.activePageId, QString(), &error)); QCOMPARE(project.toJson(), before);
    QVERIFY(!project.selectPage(QStringLiteral("missing"), &error)); QCOMPARE(project.toJson(), before);
    QVERIFY(!project.removePage(project.startupPageId, &error)); QCOMPARE(project.toJson(), before);
    project.startupLanguage = QStringLiteral("中文"); invalid = project.toJson(); pages = invalid.value(QStringLiteral("pages")).toArray();
    pages.append(pages.first()); invalid.insert(QStringLiteral("pages"), pages); output.name = QStringLiteral("Do not replace");
    QVERIFY(!HmiProject::fromJson(invalid, &output, &error)); QCOMPARE(output.name, QStringLiteral("Do not replace"));
}

/******************************************************************************
 * Function Name   : macroIdentityRename
 * Description     : Keep IDs and dormant bindings stable across rename while refusing overwrites and name-as-ID references.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::macroIdentityRename()
{
    HmiProject project;
    HmiPage page;
    HmiReference reference;
    QJsonObject before;
    QString macroId;
    QString error;

    project.macros.insert(QStringLiteral("Old"), QStringLiteral("void Old(void) {}"));
    project.objects = {modelObject(1)}; project.objects[0].bitButton.macroEnabled = true; project.objects[0].bitButton.macroName = QStringLiteral("Old");
    page.id = QStringLiteral("other"); page.name = page.id; page.objects = {modelObject(4)}; page.objects[0].bitButton.macroName = QStringLiteral("Old");
    QVERIFY(project.addPage(page, &error));
    QVERIFY2(project.renameMacro(QStringLiteral("Old"), QStringLiteral("New"), &error), qPrintable(error));
    macroId = project.macroIds.value(QStringLiteral("New")); QVERIFY(!macroId.isEmpty());
    QCOMPARE(project.objects[0].bitButton.macroName, QStringLiteral("New")); QCOMPARE(project.inactivePages[0].objects[0].bitButton.macroName, QStringLiteral("New"));
    reference.id = QStringLiteral("macro-binding"); reference.ownerKind = QStringLiteral("project"); reference.ownerId = project.projectId;
    reference.field = QStringLiteral("editor.macro"); reference.targetKind = QStringLiteral("macro"); reference.targetId = macroId; project.references.append(reference);
    QVERIFY(project.renameMacro(QStringLiteral("New"), QStringLiteral("Again"), &error)); QCOMPARE(project.macroIds.value(QStringLiteral("Again")), macroId);
    QCOMPARE(project.references[0].targetId, macroId); project.macros.insert(QStringLiteral("Occupied"), QStringLiteral("source")); before = project.toJson();
    QVERIFY(!project.renameMacro(QStringLiteral("Again"), QStringLiteral("Occupied"), &error)); QCOMPARE(project.toJson(), before);
    project.references[0].targetId = QStringLiteral("Again"); QVERIFY(!project.validateModel(&error));
}

/******************************************************************************
 * Function Name   : languageTargetsAndDrafts
 * Description     : Reorder indexed function actions by language name on every page while preserving dormant translations.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::languageTargetsAndDrafts()
{
    HmiProject project;
    HmiPage page;
    QJsonObject before;
    QString error;

    project.languages = {QStringLiteral("中文"), QStringLiteral("English"), QStringLiteral("Deutsch")};
    project.objects = {modelObject(4), modelObject(0)}; project.objects[0].functionButton.function = 13; project.objects[0].functionButton.type = 1;
    project.startupLanguage = QStringLiteral("中文"); page.id = QStringLiteral("second"); page.name = page.id;
    page.objects = {modelObject(4)}; page.objects[0].id = QStringLiteral("other-function"); page.objects[0].functionButton.function = 13; page.objects[0].functionButton.type = 0;
    QVERIFY(project.addPage(page, &error));
    QVERIFY2(project.setLanguages({QStringLiteral("English"), QStringLiteral("中文"), QStringLiteral("Deutsch")}, &error), qPrintable(error));
    QCOMPARE(project.objects[0].functionButton.type, 0); QCOMPARE(project.inactivePages[0].objects[0].functionButton.type, 1);
    QCOMPARE(project.startupLanguage, QStringLiteral("中文")); QCOMPARE(project.activeLanguage, QStringLiteral("中文"));
    before = project.toJson(); QVERIFY(!project.setLanguages({QStringLiteral("中文"), QStringLiteral("Deutsch")}, &error)); QCOMPARE(project.toJson(), before);
    QVERIFY(!project.setLanguages({QStringLiteral("English"), QStringLiteral("Deutsch")}, &error)); QCOMPARE(project.toJson(), before);
    project.objects[1].staticText.languageTexts.insert(QStringLiteral("Deutsch"), QStringLiteral("Dormant draft"));
    QVERIFY(project.setLanguages({QStringLiteral("English"), QStringLiteral("中文")}, &error));
    QCOMPARE(project.objects[1].staticText.languageTexts.value(QStringLiteral("Deutsch")), QStringLiteral("Dormant draft"));
}

/******************************************************************************
 * Function Name   : malformedProject_data
 * Description     : Name independent malformed carriers and semantic faults rather than implementation branches.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::malformedProject_data()
{
    QString fault;

    QTest::addColumn<QString>("fault");
    Q_FOREACH (fault, (QStringList{QStringLiteral("future"), QStringLiteral("schema-null"), QStringLiteral("unknown-top"), QStringLiteral("unknown-page"),
        QStringLiteral("unknown-object"), QStringLiteral("object-null"), QStringLiteral("geometry-fraction"), QStringLiteral("property-null"),
        QStringLiteral("property-unknown"), QStringLiteral("bool-as-int"), QStringLiteral("duplicate-object"), QStringLiteral("resource-null"),
        QStringLiteral("pseudo-base64"), QStringLiteral("invalid-image"), QStringLiteral("snapshot-garbage"), QStringLiteral("languages-duplicate"),
        QStringLiteral("languages-null"), QStringLiteral("property-conflict"), QStringLiteral("layer-string")}))
    { QTest::newRow(qPrintable(fault)) << fault; }
}

/******************************************************************************
 * Function Name   : malformedProject
 * Description     : Assert every malformed load leaves a distinct existing project unchanged.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::malformedProject()
{
    QFETCH(QString, fault);
    HmiProject source;
    HmiProject output;
    QJsonObject json;
    QJsonObject page;
    QJsonObject object;
    QJsonObject properties;
    QJsonArray objects;
    QJsonObject before;
    QString error;

    source.objects = {modelObject(0), modelObject(1)}; json = source.toJson();
    page = json.value(QStringLiteral("pages")).toArray().first().toObject(); objects = page.value(QStringLiteral("objects")).toArray(); object = objects.first().toObject();
    if (fault == QStringLiteral("future")) { json.insert(QStringLiteral("schemaVersion"), 10); }
    if (fault == QStringLiteral("schema-null")) { json.insert(QStringLiteral("schemaVersion"), QJsonValue(QJsonValue::Null)); }
    if (fault == QStringLiteral("unknown-top")) { json.insert(QStringLiteral("futureActiveAction"), true); }
    if (fault == QStringLiteral("unknown-page")) { page.insert(QStringLiteral("futureKind"), 9); }
    if (fault == QStringLiteral("unknown-object")) { object.insert(QStringLiteral("futureAction"), 1); }
    if (fault == QStringLiteral("object-null")) { object.insert(QStringLiteral("name"), QJsonValue(QJsonValue::Null)); }
    if (fault == QStringLiteral("geometry-fraction")) { object.insert(QStringLiteral("width"), 1.5); }
    if (fault == QStringLiteral("layer-string")) { object.insert(QStringLiteral("layer"), QStringLiteral("1")); }
    properties = object.value(QStringLiteral("staticText")).toObject();
    if (fault == QStringLiteral("property-unknown")) { properties.insert(QStringLiteral("futureAction"), 1); }
    if (fault == QStringLiteral("bool-as-int")) { properties.insert(QStringLiteral("bold"), 1); }
    object.insert(QStringLiteral("staticText"), properties);
    if (fault == QStringLiteral("property-null")) { object.insert(QStringLiteral("staticText"), QJsonValue(QJsonValue::Null)); }
    if (fault == QStringLiteral("property-conflict")) { object.insert(QStringLiteral("bitButton"), BitButtonProperties().toJson()); }
    objects[0] = object;
    if (fault == QStringLiteral("duplicate-object")) { objects.append(object); }
    page.insert(QStringLiteral("objects"), objects); json.insert(QStringLiteral("pages"), QJsonArray{page});
    if (fault == QStringLiteral("resource-null")) { json.insert(QStringLiteral("pictures"), QJsonValue(QJsonValue::Null)); }
    if (fault == QStringLiteral("pseudo-base64")) { json.insert(QStringLiteral("pictures"), QJsonObject{{QStringLiteral("asset"), QStringLiteral("%%%AAAA")}}); }
    if (fault == QStringLiteral("invalid-image")) { json.insert(QStringLiteral("pictures"), QJsonObject{{QStringLiteral("asset"), QString::fromLatin1(QByteArrayLiteral("not an image").toBase64())}}); }
    if (fault == QStringLiteral("snapshot-garbage")) { json.insert(QStringLiteral("schemaVersion"), 5); json.insert(QStringLiteral("nativeSourceSnapshot"), QString::fromLatin1(QByteArrayLiteral("bad source").toBase64())); }
    if (fault == QStringLiteral("languages-duplicate")) { json.insert(QStringLiteral("languages"), QJsonArray{QStringLiteral("中文"), QStringLiteral("中文")}); }
    if (fault == QStringLiteral("languages-null")) { json.insert(QStringLiteral("languages"), QJsonValue(QJsonValue::Null)); }
    output.name = QStringLiteral("Existing sentinel"); before = output.toJson();
    QVERIFY2(!HmiProject::fromJson(json, &output, &error), qPrintable(fault)); QCOMPARE(output.toJson(), before); QVERIFY(!error.isEmpty());
}

/******************************************************************************
 * Function Name   : schemaCapabilityFloors_data
 * Description     : Prevent hand-lowered schema numbers from accepting and later losing higher-version data.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::schemaCapabilityFloors_data()
{
    int minimum;

    QTest::addColumn<int>("minimum");
    for (minimum = 2; minimum <= 9; ++minimum) { QTest::newRow(qPrintable(QString::number(minimum))) << minimum; }
}

/******************************************************************************
 * Function Name   : schemaCapabilityFloors
 * Description     : Refuse low-version controls, resources, connections, snapshots and page metadata atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::schemaCapabilityFloors()
{
    QFETCH(int, minimum);
    HmiProject project;
    HmiProject output;
    QJsonObject json;
    QString error;

    if (minimum == 2) { project.objects = {modelObject(0)}; }
    if (minimum == 3) { project.macros.insert(QStringLiteral("source"), QStringLiteral("void main(void) {}")); }
    if (minimum == 4) { json = project.toJson(); json.insert(QStringLiteral("connections"), QJsonArray()); }
    if (minimum == 5) { project.nativeSourceSnapshot = snapshotFixture(); QVERIFY(!project.nativeSourceSnapshot.isEmpty()); }
    if (minimum == 6) { project.objects = {modelObject(2)}; }
    if (minimum == 7) { project.objects = {modelObject(3)}; }
    if (minimum == 8) { project.objects = {modelObject(4)}; }
    if (minimum == 9) { project.startupLanguage = QStringLiteral("中文"); }
    if (json.isEmpty()) { json = project.toJson(); }
    json.insert(QStringLiteral("schemaVersion"), minimum - 1); output.name = QStringLiteral("unchanged");
    QVERIFY(!HmiProject::fromJson(json, &output, &error)); QCOMPARE(output.name, QStringLiteral("unchanged"));
}

/******************************************************************************
 * Function Name   : objectFailureIsAtomic
 * Description     : Keep a previously populated object untouched after a late subtype conflict or image failure.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::objectFailureIsAtomic()
{
    HmiObject output;
    QJsonObject before;
    QJsonObject invalid;
    QString error;

    output = modelObject(4); before = output.toJson(); invalid = modelObject(0).toJson(); invalid.insert(QStringLiteral("wordButton"), WordButtonProperties().toJson());
    QVERIFY(!HmiObject::fromJson(invalid, &output, &error)); QCOMPARE(output.toJson(), before);
    invalid = modelObject(1).toJson(); invalid.insert(QStringLiteral("width"), 2147483647); invalid.insert(QStringLiteral("x"), 100);
    QVERIFY(!HmiObject::fromJson(invalid, &output, &error)); QCOMPARE(output.toJson(), before);
}

/******************************************************************************
 * Function Name   : inactiveMetadataAndConfigurations
 * Description     : Preserve bounded inert extensions while refusing unknown kinds, active flags and uninterpreted properties.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::inactiveMetadataAndConfigurations()
{
    HmiProject project;
    HmiProject loaded;
    HmiConfigurationNode node;
    QJsonObject metadata;
    QJsonObject envelope;
    QString error;

    envelope = {{QStringLiteral("version"), 1}, {QStringLiteral("inactive"), true},
        {QStringLiteral("data"), QJsonObject{{QStringLiteral("note"), QStringLiteral("Unimplemented configuration")}}}};
    metadata.insert(QStringLiteral("editor.notes"), envelope);
    node.id = QStringLiteral("alarm-config"); node.kind = QStringLiteral("alarm"); node.name = QStringLiteral("Alarm placeholder"); node.extensions = metadata;
    project.configurationNodes.append(node); project.extensions = metadata;
    QVERIFY2(HmiProject::fromJson(project.toJson(), &loaded, &error), qPrintable(error)); QCOMPARE(loaded.toJson(), project.toJson());
    QVERIFY(!loaded.validateSinglePageBackend(&error));
    project.configurationNodes[0].kind = QStringLiteral("unknown"); QVERIFY(!project.validateModel(&error));
    project.configurationNodes[0] = node; project.configurationNodes[0].active = true; QVERIFY(!project.validateModel(&error));
    project.configurationNodes[0] = node; project.configurationNodes[0].properties.insert(QStringLiteral("unmappedSetting"), 7); QVERIFY(!project.validateModel(&error));
    project.configurationNodes[0] = node; metadata[QStringLiteral("editor.notes")] = QJsonObject{{QStringLiteral("version"), 1}, {QStringLiteral("inactive"), false}, {QStringLiteral("data"), QJsonObject()}};
    project.extensions = metadata; QVERIFY(!project.validateModel(&error));
}

/******************************************************************************
 * Function Name   : invalidReferences_data
 * Description     : Cover unresolved and duplicate identities in the model graph.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::invalidReferences_data()
{
    QTest::addColumn<int>("fault");
    QTest::newRow("dangling-target") << 0; QTest::newRow("dangling-owner") << 1;
    QTest::newRow("duplicate-reference-id") << 2; QTest::newRow("missing-startup-page") << 3;
    QTest::newRow("duplicate-order") << 4; QTest::newRow("macro-name-is-not-id") << 5;
}

/******************************************************************************
 * Function Name   : invalidReferences
 * Description     : Fail reference resolution before replacing the caller's project.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::invalidReferences()
{
    QFETCH(int, fault);
    HmiProject project;
    HmiProject loaded;
    HmiReference reference;
    QJsonObject before;
    QString error;

    reference.id = QStringLiteral("ref"); reference.ownerKind = QStringLiteral("project"); reference.ownerId = project.projectId;
    reference.field = QStringLiteral("editor.bookmark"); reference.targetKind = QStringLiteral("page"); reference.targetId = project.activePageId;
    if (fault == 0) { reference.targetId = QStringLiteral("missing"); }
    if (fault == 1) { reference.ownerId = QStringLiteral("missing"); }
    if (fault == 3) { project.startupPageId = QStringLiteral("missing"); }
    if (fault == 4) { project.pageOrder.append(project.activePageId); }
    if (fault == 5) { project.macros.insert(QStringLiteral("legacy-name"), QStringLiteral("source")); reference.targetKind = QStringLiteral("macro"); reference.targetId = QStringLiteral("legacy-name"); }
    project.references.append(reference); if (fault == 2) { project.references.append(reference); }
    before = loaded.toJson(); QVERIFY(!HmiProject::fromJson(project.toJson(), &loaded, &error)); QCOMPARE(loaded.toJson(), before);
}

/******************************************************************************
 * Function Name   : saveAndLoadFailuresAreAtomic
 * Description     : Preserve an existing disk file after model validation failure and preserve output after truncated reads.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::saveAndLoadFailuresAreAtomic()
{
    QTemporaryDir directory;
    HmiProject project;
    HmiProject output;
    PlcConnection connection;
    QFile file;
    QByteArray before;
    QJsonObject outputBefore;
    QString path;
    QString error;

    QVERIFY(directory.isValid()); path = directory.filePath(QStringLiteral("valid.ksp")); project.objects = {modelObject(0)};
    QVERIFY2(project.save(path, &error), qPrintable(error)); file.setFileName(path); QVERIFY(file.open(QIODevice::ReadOnly)); before = file.readAll(); file.close();
    project.objects.append(project.objects.first()); QVERIFY(!project.save(path, &error));
    QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(), before); file.close(); project.objects.removeLast();
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini"); connection.channel = PlcChannel::Com1;
    connection.station = QStringLiteral("1"); project.connections.append(connection);
    QVERIFY(connection.id.isEmpty()); QVERIFY(!project.save(path, &error));
    QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(), before); file.close(); project.connections.clear();
    QVERIFY(!project.save(directory.path(), &error)); QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(), before); file.close();
    output.name = QStringLiteral("Current unsaved work"); outputBefore = output.toJson();
    file.setFileName(directory.filePath(QStringLiteral("broken.ksp"))); QVERIFY(file.open(QIODevice::WriteOnly)); file.write("{\"format\":"); file.close();
    QVERIFY(!HmiProject::load(file.fileName(), &output, &error)); QCOMPARE(output.toJson(), outputBefore);
}

/******************************************************************************
 * Function Name   : snapshotAndImageRoundTrip
 * Description     : Preserve real PNG bytes, native source bytes and all five controls through a multi-page save/load cycle.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::snapshotAndImageRoundTrip()
{
    HmiProject project;
    HmiProject loaded;
    HmiPage page;
    QImage image(3, 2, QImage::Format_ARGB32);
    QBuffer buffer;
    QTemporaryDir directory;
    QString error;
    int kind;

    for (kind = 0; kind < 5; ++kind) { project.objects.append(modelObject(kind)); }
    project.nativeSourceSnapshot = snapshotFixture(); QVERIFY(!project.nativeSourceSnapshot.isEmpty());
    image.fill(Qt::transparent); QVERIFY(buffer.open(QIODevice::WriteOnly)); QVERIFY(image.save(&buffer, "PNG"));
    project.pictures.insert(QStringLiteral("stable-image"), buffer.data()); project.macros.insert(QStringLiteral("draft"), QStringLiteral("/* 中文 source */"));
    page.id = QStringLiteral("empty-window"); page.kind = QStringLiteral("window"); page.name = page.id; page.width = 100; page.height = 60;
    QVERIFY(project.addPage(page, &error)); QVERIFY2(project.save(directory.filePath(QStringLiteral("all.ksp")), &error), qPrintable(error));
    QVERIFY2(HmiProject::load(directory.filePath(QStringLiteral("all.ksp")), &loaded, &error), qPrintable(error));
    QCOMPARE(loaded.toJson(), project.toJson()); QCOMPARE(loaded.pictures, project.pictures); QCOMPARE(loaded.nativeSourceSnapshot, project.nativeSourceSnapshot);
    project.nativeSourceSnapshot.chop(1); QVERIFY(!project.save(directory.filePath(QStringLiteral("all.ksp")), &error));
}

/******************************************************************************
 * Function Name   : backendCapabilityBoundary
 * Description     : Separate editable K topology from the established one-page native/device capability.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::backendCapabilityBoundary()
{
    HmiProject project;
    HmiPage page;
    QString error;

    project.objects = {modelObject(4)}; project.languages.append(QStringLiteral("English")); project.activeLanguage = QStringLiteral("English");
    project.startupLanguage = QStringLiteral("English"); QVERIFY2(project.validateSinglePageBackend(&error), qPrintable(error));
    page.id = QStringLiteral("page-2"); page.name = page.id; QVERIFY(project.addPage(page, &error));
    QVERIFY(project.validateModel(&error)); QVERIFY(!project.validateSinglePageBackend(&error));
    QVERIFY(project.removePage(page.id, &error)); QVERIFY(project.validateSinglePageBackend(&error));
    project.activePageKind = QStringLiteral("keyboard"); QVERIFY(!project.validateSinglePageBackend(&error));
}

/******************************************************************************
 * Function Name   : actualStage0EmptyMacroArrayMigrates
 * Description     : Migrate the released baseline's empty-map alias while rejecting nonempty and schema9 arrays.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void ProjectModelStage1Tests::actualStage0EmptyMacroArrayMigrates() //20261110-5 add
{
    QFile file(QFINDTESTDATA("../samples/stage0/stage0-baseline.ksp"));
    QJsonObject json;
    QJsonObject invalid;
    HmiProject project;
    HmiProject output;
    QJsonObject before;
    QString error;

    QVERIFY(file.open(QIODevice::ReadOnly));
    json = QJsonDocument::fromJson(file.readAll()).object();
    QVERIFY(json.value(QStringLiteral("macros")).isArray());
    QVERIFY(json.value(QStringLiteral("macros")).toArray().isEmpty());
    QVERIFY2(HmiProject::fromJson(json, &project, &error), qPrintable(error));
    QCOMPARE(project.objects.size(), 12);
    QVERIFY(project.macros.isEmpty());
    QVERIFY(project.toJson().value(QStringLiteral("macros")).isObject());
    before = output.toJson(); invalid = json;
    invalid.insert(QStringLiteral("macros"), QJsonArray{QStringLiteral("discarded source")});
    QVERIFY(!HmiProject::fromJson(invalid, &output, &error)); QCOMPARE(output.toJson(), before);
    project.projectId = QStringLiteral("schema9-project"); invalid = project.toJson();
    QCOMPARE(invalid.value(QStringLiteral("schemaVersion")).toInt(), 9);
    invalid.insert(QStringLiteral("macros"), QJsonArray());
    QVERIFY(!HmiProject::fromJson(invalid, &output, &error)); QCOMPARE(output.toJson(), before);
}

QTEST_MAIN(ProjectModelStage1Tests)
#include "project_model_stage1_tests.moc"
