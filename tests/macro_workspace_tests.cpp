#include <QtTest>
#include <QFile>
#include <QJsonDocument>
#include "macro_workspace.h"
#include "native_project_codec.h"

//20261114-7 add: Pure project and UTF-8 transactions; no window, compiler SDK or device is started.
class MacroWorkspaceTests : public QObject
{
    Q_OBJECT
private slots:
    void listDoesNotMigrateLegacyIds();
    void createUpdateCopyPreserveIdentity();
    void renameAllPages_data();
    void renameAllPages();
    void deleteAllCarriers_data();
    void deleteAllCarriers();
    void genericReferenceBlocksRemoval();
    void invalidNames_data();
    void invalidNames();
    void nameAndSourceAreIndependent();
    void mutationFailuresAreAtomic();
    void countLimitAllowsLegacyRecovery();
    void sourceAndAggregateBudgets();
    void invalidUtf8_data();
    void invalidUtf8();
    void bomAndExactSourceRoundTrip();
    void importRequiresExplicitReplacement();
    void refusedSourceKinds_data();
    void refusedSourceKinds();
    void invalidSourceKeepsProjectAndOutput();
    void bindingGuard_data();
    void bindingGuard();
    void bindingMatrixMatchesGuard();
    void copyNameIsBoundedAndCaseSensitive();
    void opaqueSnapshotIsNeverEdited();
    void invalidModelAndNullOutputs();
};

/******************************************************************************
 * Function Name   : workspaceObject
 * Description     : Construct each independently serialized carrier with valid non-macro defaults.
 * Input Parameters: kind, id
 * Return Value    : Valid model object, independent of rendering or native runtime.
 * Date            : 2026.11.14
 ******************************************************************************/
static HmiObject workspaceObject(int kind, const QString &id) //20261114-7 add
{
    HmiObject object;
    QStringList types;

    types = {QStringLiteral("switch"), QStringLiteral("switch"), QStringLiteral("switch"), QStringLiteral("switch"),
             QStringLiteral("screen"), QStringLiteral("multistate"), QStringLiteral("multifunction"), QStringLiteral("number"),
             QStringLiteral("ascii"), QStringLiteral("slider"), QStringLiteral("dropdown"), QStringLiteral("text")};
    object.id = id; object.name = id; object.type = types.at(kind); object.geometry = QRect(20, 20, 120, 50);
    if (kind < 4) { object.hasBitButtonProperties = true; object.address = QStringLiteral("LB0"); }
    if (kind == 1) { object.hasWordButtonProperties = true; object.bitButton.indicator = false; object.bitButton.dualMonitor = false; object.address = QStringLiteral("LW0"); }
    if (kind == 2) { object.hasLampProperties = true; object.bitButton.monitorSame = false; object.bitButton.bitIndex = 0; object.address.clear(); }
    if (kind == 3) { object.hasFunctionButtonProperties = true; object.bitButton.indicator = false; object.bitButton.dualMonitor = false; object.address.clear(); }
    if (kind == 4) { object.screenButton.screenPageId = QStringLiteral("page-1"); }
    if (kind == 7) { object.hasNumberDisplayProperties = true; object.address = QStringLiteral("LW0"); }
    if (kind == 8) { object.address = QStringLiteral("LW0"); }
    if (kind == 11) { object.hasStaticTextProperties = true; object.text = QStringLiteral("unrelated"); }
    return object;
}

/******************************************************************************
 * Function Name   : storedName
 * Description     : Select the actual serialized name carrier rather than the shared display type.
 * Input Parameters: object, kind
 * Return Value    : Macro name stored in the requested independent property family.
 * Date            : 2026.11.14
 ******************************************************************************/
static QString storedName(const HmiObject &object, int kind) //20261114-7 add
{
    if (kind < 4) { return object.bitButton.macroName; }
    if (kind == 4) { return object.screenButton.common.macroName; }
    if (kind == 5) { return object.multiStateButton.common.macroName; }
    if (kind == 6) { return object.multiFunctionButton.common.macroName; }
    if (kind == 7) { return object.numberDisplay.macroName; }
    if (kind == 8) { return object.asciiDisplay.common.macroName; }
    if (kind == 9) { return object.slider.macroName; }
    return object.dropdown.macroName;
}

/******************************************************************************
 * Function Name   : setStoredName
 * Description     : Seed live or disabled macro-name drafts without enabling forbidden control features.
 * Input Parameters: object, kind, name, live
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
static void setStoredName(HmiObject *object, int kind, const QString &name, bool live = false) //20261114-7 add
{
    if (kind < 4) { object->bitButton.macroName = name; object->bitButton.macroEnabled = live; }
    if (kind == 4) { object->screenButton.common.macroName = name; }
    if (kind == 5) { object->multiStateButton.common.macroName = name; }
    if (kind == 6) { object->multiFunctionButton.common.macroName = name; }
    if (kind == 7) { object->numberDisplay.macroName = name; }
    if (kind == 8) { object->asciiDisplay.common.macroName = name; }
    if (kind == 9) { object->slider.macroName = name; object->slider.macroEnabled = live; }
    if (kind == 10) { object->dropdown.macroName = name; object->dropdown.macroEnabled = live; }
}

/******************************************************************************
 * Function Name   : listDoesNotMigrateLegacyIds
 * Description     : Prove inspection neither invents stable identities nor rewrites sources or dormant dependencies.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::listDoesNotMigrateLegacyIds() //20261114-7 add
{
    HmiProject project;
    QList<MacroWorkspace::Entry> entries;
    QJsonObject before;
    QString error;

    project.macros.insert(QStringLiteral("Legacy"), QStringLiteral("/* 中文 */\r\n"));
    project.objects = {workspaceObject(0, QStringLiteral("bit"))}; setStoredName(&project.objects[0], 0, QStringLiteral("Legacy"));
    before = project.toJson();
    QVERIFY2(MacroWorkspace::list(project, &entries, &error), qPrintable(error));
    QCOMPARE(project.toJson(), before); QCOMPARE(entries.size(), 1); QVERIFY(entries.first().id.isEmpty());
    QCOMPARE(entries.first().source, project.macros.value(QStringLiteral("Legacy")));
    QCOMPARE(entries.first().utf8Bytes, project.macros.value(QStringLiteral("Legacy")).toUtf8().size());
    QCOMPARE(entries.first().references.size(), 1); QVERIFY(entries.first().references.first().contains(QStringLiteral("bitButton.macroName")));
}

/******************************************************************************
 * Function Name   : createUpdateCopyPreserveIdentity
 * Description     : Distinguish source replacement, copy identity and unrelated resources using a real model round trip.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::createUpdateCopyPreserveIdentity() //20261114-7 add
{
    HmiProject project;
    HmiProject restored;
    QString id;
    QString error;
    QJsonObject before;

    project.objects = {workspaceObject(11, QStringLiteral("unrelated-text"))};
    QVERIFY2(MacroWorkspace::create(&project, QStringLiteral("A"), MacroWorkspace::defaultSource(), &error), qPrintable(error));
    id = project.macroIds.value(QStringLiteral("A")); QVERIFY(!id.isEmpty());
    QVERIFY(MacroWorkspace::update(&project, QStringLiteral("A"), QStringLiteral("/* edited */\n"), &error));
    QCOMPARE(project.macroIds.value(QStringLiteral("A")), id);
    QVERIFY(MacroWorkspace::copy(&project, QStringLiteral("A"), QStringLiteral("A_copy"), &error));
    QCOMPARE(project.macros.value(QStringLiteral("A_copy")), project.macros.value(QStringLiteral("A")));
    QVERIFY(!project.macroIds.value(QStringLiteral("A_copy")).isEmpty()); QVERIFY(project.macroIds.value(QStringLiteral("A_copy")) != id);
    QCOMPARE(project.objects.first().text, QStringLiteral("unrelated"));
    QVERIFY2(HmiProject::fromJson(project.toJson(), &restored, &error), qPrintable(error)); QCOMPARE(restored.toJson(), project.toJson());
    before = project.toJson(); QVERIFY(!MacroWorkspace::copy(&project, QStringLiteral("A"), QStringLiteral("A_copy"), &error)); QCOMPARE(project.toJson(), before);
}

/******************************************************************************
 * Function Name   : renameAllPages_data
 * Description     : Cover every persisted carrier on the visible and hidden page, including allowed live variants.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::renameAllPages_data() //20261114-7 add
{
    int kind;
    int live;
    QByteArray label;

    QTest::addColumn<int>("kind"); QTest::addColumn<bool>("live");
    for (kind = 0; kind <= 10; ++kind)
    {
        for (live = 0; live <= 1; ++live)
        {
            if (live != 0 && kind != 0 && kind != 1 && kind != 9 && kind != 10) { continue; }
            label = QByteArray::number(kind) + (live != 0 ? "-live" : "-dormant");
            QTest::newRow(label.constData()) << kind << (live != 0);
        }
    }
}

/******************************************************************************
 * Function Name   : renameAllPages
 * Description     : Verify stable identities and generic targets while remapping both active and inactive page carriers.
 * Input Parameters: kind, live from test data
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::renameAllPages() //20261114-7 add
{
    QFETCH(int, kind);
    QFETCH(bool, live);
    HmiProject project;
    HmiProject restored;
    HmiPage page;
    HmiReference reference;
    QString error;
    QString originalSource;
    QList<MacroWorkspace::Entry> entries;

    originalSource = QStringLiteral("/* Display name Old is deliberately not a C symbol. */\n");
    project.macros.insert(QStringLiteral("Old"), originalSource); project.macroIds.insert(QStringLiteral("Old"), QStringLiteral("macro-stable"));
    project.objects = {workspaceObject(kind, QStringLiteral("front"))}; setStoredName(&project.objects[0], kind, QStringLiteral("Old"), live);
    page.id = QStringLiteral("hidden"); page.name = page.id; page.objects = {workspaceObject(kind, QStringLiteral("back"))};
    setStoredName(&page.objects[0], kind, QStringLiteral("Old"), live); QVERIFY2(project.addPage(page, &error), qPrintable(error));
    reference.id = QStringLiteral("stable-ref"); reference.ownerKind = QStringLiteral("project"); reference.ownerId = project.projectId;
    reference.field = QStringLiteral("editor.dormant"); reference.targetKind = QStringLiteral("macro"); reference.targetId = QStringLiteral("macro-stable"); project.references.append(reference);
    QVERIFY2(project.validateModel(&error), qPrintable(error));
    QVERIFY2(MacroWorkspace::rename(&project, QStringLiteral("Old"), QStringLiteral("Renamed"), &error), qPrintable(error));
    QCOMPARE(storedName(project.objects.first(), kind), QStringLiteral("Renamed"));
    QCOMPARE(storedName(project.inactivePages.first().objects.first(), kind), QStringLiteral("Renamed"));
    QCOMPARE(project.macros.value(QStringLiteral("Renamed")), originalSource); QVERIFY(!project.macros.contains(QStringLiteral("Old")));
    QCOMPARE(project.macroIds.value(QStringLiteral("Renamed")), QStringLiteral("macro-stable")); QCOMPARE(project.references.first().targetId, QStringLiteral("macro-stable"));
    QVERIFY2(HmiProject::fromJson(project.toJson(), &restored, &error), qPrintable(error));
    QCOMPARE(storedName(restored.inactivePages.first().objects.first(), kind), QStringLiteral("Renamed"));
    QVERIFY(MacroWorkspace::list(restored, &entries, &error)); QCOMPARE(entries.first().references.size(), 3);
}

/******************************************************************************
 * Function Name   : deleteAllCarriers_data
 * Description     : Reuse every carrier case so disabled drafts cannot bypass dependency checks.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::deleteAllCarriers_data() //20261114-7 add
{
    renameAllPages_data();
}

/******************************************************************************
 * Function Name   : deleteAllCarriers
 * Description     : Reject a macro referenced only from an inactive page, without removing its ID or source.
 * Input Parameters: kind, live from test data
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::deleteAllCarriers() //20261114-7 add
{
    QFETCH(int, kind);
    QFETCH(bool, live);
    HmiProject project;
    HmiPage page;
    QJsonObject before;
    QString error;

    project.macros.insert(QStringLiteral("Bound"), QStringLiteral("source")); project.macroIds.insert(QStringLiteral("Bound"), QStringLiteral("bound-id"));
    page.id = QStringLiteral("hidden-only"); page.name = page.id; page.objects = {workspaceObject(kind, QStringLiteral("hidden-control"))};
    setStoredName(&page.objects[0], kind, QStringLiteral("Bound"), live); QVERIFY2(project.addPage(page, &error), qPrintable(error));
    before = project.toJson(); QVERIFY(!MacroWorkspace::remove(&project, QStringLiteral("Bound"), &error));
    QVERIFY2(error.contains(QStringLiteral("hidden-control")), qPrintable(error)); QCOMPARE(project.toJson(), before);
}

/******************************************************************************
 * Function Name   : genericReferenceBlocksRemoval
 * Description     : Refuse a stable generic reference even when no control has a name binding, then allow detached deletion.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::genericReferenceBlocksRemoval() //20261114-7 add
{
    HmiProject project;
    HmiReference reference;
    QJsonObject before;
    QString error;

    QVERIFY(MacroWorkspace::create(&project, QStringLiteral("A"), QStringLiteral("source"), &error));
    reference.id = QStringLiteral("generic-dependency"); reference.ownerKind = QStringLiteral("project"); reference.ownerId = project.projectId;
    reference.field = QStringLiteral("editor.future"); reference.targetKind = QStringLiteral("macro"); reference.targetId = project.macroIds.value(QStringLiteral("A"));
    project.references.append(reference); before = project.toJson();
    QVERIFY(!MacroWorkspace::remove(&project, QStringLiteral("A"), &error)); QVERIFY(error.contains(reference.id)); QCOMPARE(project.toJson(), before);
    project.references.clear(); QVERIFY(MacroWorkspace::remove(&project, QStringLiteral("A"), &error)); QVERIFY(project.macros.isEmpty()); QVERIFY(project.macroIds.isEmpty());
}

/******************************************************************************
 * Function Name   : invalidNames_data
 * Description     : Exercise empty, whitespace, control and Unicode boundary failures independently of C syntax.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::invalidNames_data() //20261114-7 add
{
    QTest::addColumn<QString>("name");
    QTest::newRow("empty") << QString(); QTest::newRow("space") << QStringLiteral(" ");
    QTest::newRow("leading") << QStringLiteral(" A"); QTest::newRow("trailing") << QStringLiteral("A ");
    QTest::newRow("newline") << QStringLiteral("A\nB"); QTest::newRow("nul") << (QStringLiteral("A") + QChar(0));
    QTest::newRow("oversize") << QString(257, QLatin1Char('A')); QTest::newRow("surrogate") << QString(QChar(0xd800));
}

/******************************************************************************
 * Function Name   : invalidNames
 * Description     : Keep model and IDs unchanged for all invalid create and rename keys.
 * Input Parameters: name from test data
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::invalidNames() //20261114-7 add
{
    QFETCH(QString, name);
    HmiProject project;
    QJsonObject before;
    QString error;

    project.macros.insert(QStringLiteral("A"), QStringLiteral("source")); before = project.toJson();
    QVERIFY(!MacroWorkspace::checkName(name, &error)); QVERIFY(!MacroWorkspace::create(&project, name, QStringLiteral("source"), &error)); QCOMPARE(project.toJson(), before);
    QVERIFY(!MacroWorkspace::rename(&project, QStringLiteral("A"), name, &error)); QCOMPARE(project.toJson(), before);
}

/******************************************************************************
 * Function Name   : nameAndSourceAreIndependent
 * Description     : Preserve arbitrary Unicode display keys and source bytes rather than rewriting function identifiers.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::nameAndSourceAreIndependent() //20261114-7 add
{
    HmiProject project;
    QString error;
    QString source;
    QString id;

    source = QStringLiteral("void Macro_Entry(void) { /* 中文 */ }\r\n");
    QVERIFY(MacroWorkspace::create(&project, QStringLiteral("中文 宏1"), source, &error)); id = project.macroIds.first();
    QVERIFY(MacroWorkspace::rename(&project, QStringLiteral("中文 宏1"), QStringLiteral("1-display-key"), &error));
    QCOMPARE(project.macros.value(QStringLiteral("1-display-key")), source); QCOMPARE(project.macroIds.first(), id);
}

/******************************************************************************
 * Function Name   : mutationFailuresAreAtomic
 * Description     : Retain all resources and stable IDs after missing-source, conflict and repeated-delete failures.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::mutationFailuresAreAtomic() //20261114-7 add
{
    HmiProject project;
    QJsonObject before;
    QString error;

    QVERIFY(MacroWorkspace::create(&project, QStringLiteral("A"), QStringLiteral("A-source"), &error));
    QVERIFY(MacroWorkspace::create(&project, QStringLiteral("B"), QStringLiteral("B-source"), &error)); before = project.toJson();
    QVERIFY(!MacroWorkspace::create(&project, QStringLiteral("A"), QStringLiteral("replacement"), &error)); QCOMPARE(project.toJson(), before);
    QVERIFY(!MacroWorkspace::update(&project, QStringLiteral("Missing"), QStringLiteral("source"), &error)); QCOMPARE(project.toJson(), before);
    QVERIFY(!MacroWorkspace::copy(&project, QStringLiteral("Missing"), QStringLiteral("New"), &error)); QCOMPARE(project.toJson(), before);
    QVERIFY(!MacroWorkspace::rename(&project, QStringLiteral("A"), QStringLiteral("B"), &error)); QCOMPARE(project.toJson(), before);
    QVERIFY(!MacroWorkspace::remove(&project, QStringLiteral("Missing"), &error)); QCOMPARE(project.toJson(), before);
    QVERIFY(MacroWorkspace::rename(&project, QStringLiteral("A"), QStringLiteral("A"), &error)); QCOMPARE(project.toJson(), before);
}

/******************************************************************************
 * Function Name   : countLimitAllowsLegacyRecovery
 * Description     : Bound new macros at 100 while allowing existing model-valid 101..128 sources to be updated or removed.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::countLimitAllowsLegacyRecovery() //20261114-7 add
{
    HmiProject project;
    QList<MacroWorkspace::Entry> entries;
    QJsonObject before;
    QString error;
    int index;

    for (index = 0; index < 100; ++index) { project.macros.insert(QStringLiteral("M%1").arg(index), QStringLiteral("source")); }
    QVERIFY(project.validateModel(&error)); before = project.toJson();
    QVERIFY(!MacroWorkspace::create(&project, QStringLiteral("M100"), QStringLiteral("source"), &error)); QCOMPARE(project.toJson(), before);
    QVERIFY(!MacroWorkspace::copy(&project, QStringLiteral("M0"), QStringLiteral("Copy"), &error)); QCOMPARE(project.toJson(), before);
    for (index = 100; index < 128; ++index) { project.macros.insert(QStringLiteral("M%1").arg(index), QStringLiteral("source")); }
    QVERIFY(MacroWorkspace::list(project, &entries, &error)); QCOMPARE(entries.size(), 128);
    QVERIFY(MacroWorkspace::update(&project, QStringLiteral("M0"), QStringLiteral("edited"), &error));
    QVERIFY(MacroWorkspace::remove(&project, QStringLiteral("M127"), &error)); QCOMPARE(project.macros.size(), 127);
    QVERIFY(MacroWorkspace::rename(&project, QStringLiteral("M0"), QStringLiteral("Renamed"), &error));
}

/******************************************************************************
 * Function Name   : sourceAndAggregateBudgets
 * Description     : Enforce UTF-8 byte boundaries and independently preserve the existing model aggregate-unit budget.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::sourceAndAggregateBudgets() //20261114-7 add
{
    HmiProject project;
    QJsonObject before;
    QString error;
    QString boundary;
    QByteArray encoded;
    int index;

    boundary = QString(MacroWorkspace::kMaximumSourceBytes, QLatin1Char('x'));
    QVERIFY(MacroWorkspace::create(&project, QStringLiteral("M0"), boundary, &error));
    QVERIFY(MacroWorkspace::encodeUtf8Source(boundary, true, &encoded, &error)); QCOMPARE(encoded.size(), MacroWorkspace::kMaximumEncodedSourceBytes);
    before = project.toJson(); QVERIFY(!MacroWorkspace::update(&project, QStringLiteral("M0"), boundary + QLatin1Char('x'), &error)); QCOMPARE(project.toJson(), before);
    QVERIFY(!MacroWorkspace::update(&project, QStringLiteral("M0"), QString(400000, QChar(0x4e2d)), &error)); QCOMPARE(project.toJson(), before);
    for (index = 1; index < 8; ++index) { project.macros.insert(QStringLiteral("M%1").arg(index), boundary); }
    QVERIFY(project.validateModel(&error)); before = project.toJson();
    QVERIFY(!MacroWorkspace::create(&project, QStringLiteral("over-total"), QStringLiteral("x"), &error)); QCOMPARE(project.toJson(), before);
    QVERIFY(!MacroWorkspace::copy(&project, QStringLiteral("M0"), QStringLiteral("over-total-copy"), &error)); QCOMPARE(project.toJson(), before);
}

/******************************************************************************
 * Function Name   : invalidUtf8_data
 * Description     : Include truncated sequences, non-shortest forms, surrogate code points and out-of-range UTF-8.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::invalidUtf8_data() //20261114-7 add
{
    QTest::addColumn<QByteArray>("bytes");
    QTest::newRow("truncated-two") << QByteArray::fromHex("c2"); QTest::newRow("truncated-three") << QByteArray::fromHex("e4b8");
    QTest::newRow("truncated-four") << QByteArray::fromHex("f09f98"); QTest::newRow("continuation") << QByteArray::fromHex("80");
    QTest::newRow("overlong-nul") << QByteArray::fromHex("c080"); QTest::newRow("overlong-three") << QByteArray::fromHex("e080af");
    QTest::newRow("surrogate") << QByteArray::fromHex("eda080"); QTest::newRow("beyond-unicode") << QByteArray::fromHex("f4908080");
    QTest::newRow("invalid-leading") << QByteArray::fromHex("ff"); QTest::newRow("utf16-bom") << QByteArray::fromHex("fffe4100");
    QTest::newRow("double-bom") << QByteArray::fromHex("efbbbfefbbbf61"); QTest::newRow("embedded-nul") << QByteArray::fromHex("610062");
}

/******************************************************************************
 * Function Name   : invalidUtf8
 * Description     : Fail strict decoding atomically, retaining both the source output and complete import project.
 * Input Parameters: bytes from test data
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::invalidUtf8() //20261114-7 add
{
    QFETCH(QByteArray, bytes);
    HmiProject project;
    QJsonObject before;
    QString output;
    QString error;

    output = QStringLiteral("kept"); before = project.toJson();
    QVERIFY(!MacroWorkspace::decodeUtf8Source(bytes, MacroWorkspace::BomPolicy::Allow, &output, &error)); QCOMPARE(output, QStringLiteral("kept"));
    QVERIFY(!MacroWorkspace::importSource(&project, QStringLiteral("Imported"), bytes, MacroWorkspace::BomPolicy::Allow, false, &error)); QCOMPARE(project.toJson(), before);
}

/******************************************************************************
 * Function Name   : bomAndExactSourceRoundTrip
 * Description     : Verify explicit BOM policy, supplementary Unicode, replacement characters and exact CRLF preservation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::bomAndExactSourceRoundTrip() //20261114-7 add
{
    QString source;
    QString restored;
    QString error;
    QByteArray bytes;
    QByteArray plain;

    source = QStringLiteral("/* 中文 😀 � */\r\nvoid Macro_Entry(void) {}\n"); plain = source.toUtf8();
    QVERIFY(MacroWorkspace::encodeUtf8Source(source, true, &bytes, &error)); QCOMPARE(bytes, QByteArray::fromHex("efbbbf") + plain);
    QVERIFY(MacroWorkspace::decodeUtf8Source(bytes, MacroWorkspace::BomPolicy::Require, &restored, &error)); QCOMPARE(restored, source);
    restored = QStringLiteral("kept"); QVERIFY(!MacroWorkspace::decodeUtf8Source(bytes, MacroWorkspace::BomPolicy::Reject, &restored, &error)); QCOMPARE(restored, QStringLiteral("kept"));
    QVERIFY(!MacroWorkspace::decodeUtf8Source(plain, MacroWorkspace::BomPolicy::Require, &restored, &error)); QCOMPARE(restored, QStringLiteral("kept"));
    QVERIFY(MacroWorkspace::decodeUtf8Source(plain, MacroWorkspace::BomPolicy::Reject, &restored, &error)); QCOMPARE(restored, source);
    QVERIFY(MacroWorkspace::encodeUtf8Source(QString(), false, &bytes, &error)); QVERIFY(bytes.isEmpty());
    QVERIFY(MacroWorkspace::decodeUtf8Source(bytes, MacroWorkspace::BomPolicy::Allow, &restored, &error)); QVERIFY(restored.isEmpty());
}

/******************************************************************************
 * Function Name   : importRequiresExplicitReplacement
 * Description     : Separate add and replacement policy while retaining the existing stable identity and references.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::importRequiresExplicitReplacement() //20261114-7 add
{
    HmiProject project;
    QString id;
    QString error;
    QJsonObject before;
    QByteArray output;

    QVERIFY(MacroWorkspace::importSource(&project, QStringLiteral("A"), QByteArray("old\r\n"), MacroWorkspace::BomPolicy::Allow, false, &error));
    id = project.macroIds.first(); before = project.toJson();
    QVERIFY(!MacroWorkspace::importSource(&project, QStringLiteral("A"), QByteArray("new\r\n"), MacroWorkspace::BomPolicy::Allow, false, &error)); QCOMPARE(project.toJson(), before);
    QVERIFY(MacroWorkspace::importSource(&project, QStringLiteral("A"), QByteArray("new\r\n"), MacroWorkspace::BomPolicy::Allow, true, &error));
    QCOMPARE(project.macroIds.value(QStringLiteral("A")), id); QCOMPARE(project.macros.value(QStringLiteral("A")), QStringLiteral("new\r\n"));
    QVERIFY(MacroWorkspace::exportSource(project, QStringLiteral("A"), false, &output, &error)); QCOMPARE(output, QByteArray("new\r\n"));
}

/******************************************************************************
 * Function Name   : refusedSourceKinds_data
 * Description     : Keep address-variable and unverified scheduling candidates refused, including unknown enum values.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::refusedSourceKinds_data() //20261114-7 add
{
    QTest::addColumn<int>("kind"); QTest::newRow("address-variables") << 1; QTest::newRow("unverified-schedule") << 2; QTest::newRow("invalid-enum") << 99;
}

/******************************************************************************
 * Function Name   : refusedSourceKinds
 * Description     : Reject unsupported import/export kinds before changing project or caller-owned output bytes.
 * Input Parameters: kind from test data
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::refusedSourceKinds() //20261114-7 add
{
    QFETCH(int, kind);
    HmiProject project;
    QJsonObject before;
    QString error;
    QByteArray bytes;
    MacroWorkspace::SourceKind sourceKind;

    sourceKind = static_cast<MacroWorkspace::SourceKind>(kind); bytes = QByteArray("kept");
    QVERIFY(MacroWorkspace::create(&project, QStringLiteral("A"), QStringLiteral("source"), &error)); before = project.toJson();
    QVERIFY(!MacroWorkspace::importSource(&project, QStringLiteral("A"), QByteArray("changed"), MacroWorkspace::BomPolicy::Allow, true, &error, sourceKind)); QCOMPARE(project.toJson(), before);
    QVERIFY(!MacroWorkspace::exportSource(project, QStringLiteral("A"), false, &bytes, &error, sourceKind)); QCOMPARE(bytes, QByteArray("kept"));
}

/******************************************************************************
 * Function Name   : invalidSourceKeepsProjectAndOutput
 * Description     : Reject NUL, unpaired Unicode and embedded BOM without silently replacing them in model or export.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::invalidSourceKeepsProjectAndOutput() //20261114-7 add
{
    HmiProject project;
    QJsonObject before;
    QStringList invalid;
    QString source;
    QString error;
    QByteArray output;

    invalid = {QStringLiteral("A") + QChar(0), QString(QChar(0xd800)), QString(QChar(0xfeff)) + QStringLiteral("source")};
    project.macros.insert(QStringLiteral("A"), QStringLiteral("original")); before = project.toJson();
    Q_FOREACH (source, invalid)
    {
        output = QByteArray("kept"); QVERIFY(!MacroWorkspace::update(&project, QStringLiteral("A"), source, &error)); QCOMPARE(project.toJson(), before);
        QVERIFY(!MacroWorkspace::encodeUtf8Source(source, false, &output, &error)); QCOMPARE(output, QByteArray("kept"));
    }
}

/******************************************************************************
 * Function Name   : bindingGuard_data
 * Description     : Exercise all ordinary binding families and every known disabled shared carrier.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::bindingGuard_data() //20261114-7 add
{
    int kind;
    QByteArray label;

    QTest::addColumn<int>("kind"); QTest::addColumn<bool>("allowed");
    for (kind = 0; kind <= 11; ++kind)
    { label = QByteArray::number(kind); QTest::newRow(label.constData()) << kind << (kind == 0 || kind == 1 || kind == 9 || kind == 10); }
}

/******************************************************************************
 * Function Name   : bindingGuard
 * Description     : Demonstrate disabled name preservation never grants an active binding capability.
 * Input Parameters: kind, allowed from test data
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::bindingGuard() //20261114-7 add
{
    QFETCH(int, kind);
    QFETCH(bool, allowed);
    HmiObject object;
    QString reason;

    object = workspaceObject(kind, QStringLiteral("object")); reason = QStringLiteral("stale");
    if (kind != 11) { setStoredName(&object, kind, QStringLiteral("dormant")); }
    QCOMPARE(MacroWorkspace::macroBindable(object, &reason), allowed); QCOMPARE(reason.isEmpty(), allowed);
}

/******************************************************************************
 * Function Name   : bindingMatrixMatchesGuard
 * Description     : Verify the public finite matrix agrees with each accepted carrier and defaults unknown types to refused.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::bindingMatrixMatchesGuard() //20261114-7 add
{
    QJsonArray matrix;
    QJsonValue value;
    int enabled;
    HmiObject object;

    matrix = MacroWorkspace::bindingCapabilities(); enabled = 0;
    Q_FOREACH (value, matrix) { if (value.toObject().value(QStringLiteral("bindable")).toBool()) { ++enabled; } }
    QCOMPARE(enabled, 4); QCOMPARE(matrix.size(), 12);
    object = workspaceObject(0, QStringLiteral("ambiguous")); object.hasLampProperties = true; QVERIFY(!MacroWorkspace::macroBindable(object));
    object = workspaceObject(9, QStringLiteral("invalid-carrier")); object.hasBitButtonProperties = true; QVERIFY(!MacroWorkspace::macroBindable(object));
    object.type = QStringLiteral("future"); QVERIFY(!MacroWorkspace::macroBindable(object));
}

/******************************************************************************
 * Function Name   : copyNameIsBoundedAndCaseSensitive
 * Description     : Check Lite copy naming, exact case collisions and surrogate-safe truncation at the model name bound.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::copyNameIsBoundedAndCaseSensitive() //20261114-7 add
{
    HmiProject project;
    QString name;
    QString suggested;
    QString error;

    project.macros.insert(QStringLiteral("A"), QStringLiteral("source"));
    QCOMPARE(MacroWorkspace::nextCopyName(project, QStringLiteral("A")), QStringLiteral("A_copy"));
    project.macros.insert(QStringLiteral("A_copy"), QStringLiteral("occupied"));
    QCOMPARE(MacroWorkspace::nextCopyName(project, QStringLiteral("A")), QStringLiteral("A_copy2"));
    QVERIFY(MacroWorkspace::create(&project, QStringLiteral("a_copy"), QStringLiteral("different case"), &error));
    name = QString(250, QLatin1Char('x')) + QStringLiteral("😀😀😀"); QCOMPARE(name.size(), 256);
    project.macros.insert(name, QStringLiteral("source")); suggested = MacroWorkspace::nextCopyName(project, name);
    QVERIFY(MacroWorkspace::checkName(suggested, &error)); QVERIFY(suggested.size() <= 256); QVERIFY(suggested.endsWith(QStringLiteral("_copy")));
    QVERIFY(MacroWorkspace::nextCopyName(project, QStringLiteral("missing")).isEmpty());
}

/******************************************************************************
 * Function Name   : opaqueSnapshotIsNeverEdited
 * Description     : Retain exact native metadata through source edits and refuse destructive deletion with unverified hidden consumers.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::opaqueSnapshotIsNeverEdited() //20261114-7 add
{
    QFile file(QFINDTESTDATA("fixtures/native-source/bit-analysis/macro-bound-product-seed.sm"));
    HmiProject project;
    QByteArray snapshot;
    QJsonObject before;
    QString error;

    QVERIFY(file.open(QIODevice::ReadOnly)); snapshot = file.readAll();
    QVERIFY2(importNativeProjectSource(snapshot, &project, &error), qPrintable(error)); QCOMPARE(project.nativeSourceSnapshot, snapshot);
    QVERIFY(MacroWorkspace::create(&project, QStringLiteral("New"), QStringLiteral("source"), &error)); QCOMPARE(project.nativeSourceSnapshot, snapshot);
    QVERIFY(MacroWorkspace::update(&project, QStringLiteral("New"), QStringLiteral("updated"), &error)); QCOMPARE(project.nativeSourceSnapshot, snapshot);
    QVERIFY(MacroWorkspace::copy(&project, QStringLiteral("New"), QStringLiteral("Copied"), &error)); QCOMPARE(project.nativeSourceSnapshot, snapshot);
    QVERIFY(MacroWorkspace::rename(&project, QStringLiteral("New"), QStringLiteral("Renamed"), &error)); QCOMPARE(project.nativeSourceSnapshot, snapshot);
    before = project.toJson(); QVERIFY(!MacroWorkspace::remove(&project, QStringLiteral("Renamed"), &error)); QCOMPARE(project.toJson(), before); QCOMPARE(project.nativeSourceSnapshot, snapshot);
}

/******************************************************************************
 * Function Name   : invalidModelAndNullOutputs
 * Description     : Reject invalid input models and null outputs before changing any caller-owned source, list or bytes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroWorkspaceTests::invalidModelAndNullOutputs() //20261114-7 add
{
    HmiProject project;
    QList<MacroWorkspace::Entry> entries;
    MacroWorkspace::Entry sentinel;
    QString error;
    QString output;
    QByteArray bytes;
    QJsonObject before;

    sentinel.name = QStringLiteral("kept"); entries.append(sentinel); project.macros.insert(QStringLiteral("A"), QStringLiteral("source")); project.startupPageId = QStringLiteral("missing"); before = project.toJson();
    QVERIFY(!MacroWorkspace::list(project, &entries, &error)); QCOMPARE(entries.first().name, QStringLiteral("kept"));
    QVERIFY(!MacroWorkspace::update(&project, QStringLiteral("A"), QStringLiteral("changed"), &error)); QCOMPARE(project.toJson(), before);
    bytes = QByteArray("kept"); QVERIFY(!MacroWorkspace::exportSource(project, QStringLiteral("A"), false, &bytes, &error)); QCOMPARE(bytes, QByteArray("kept"));
    QVERIFY(!MacroWorkspace::create(nullptr, QStringLiteral("A"), QStringLiteral("source"), &error));
    QVERIFY(!MacroWorkspace::list(HmiProject(), nullptr, &error)); QVERIFY(!MacroWorkspace::decodeUtf8Source(QByteArray("x"), MacroWorkspace::BomPolicy::Allow, nullptr, &error));
    QVERIFY(!MacroWorkspace::encodeUtf8Source(QStringLiteral("x"), false, nullptr, &error));
    output = QStringLiteral("kept"); QVERIFY(!MacroWorkspace::decodeUtf8Source(QByteArray("x"), static_cast<MacroWorkspace::BomPolicy>(99), &output, &error)); QCOMPARE(output, QStringLiteral("kept"));
}

QTEST_GUILESS_MAIN(MacroWorkspaceTests)
#include "macro_workspace_tests.moc"
