#include <QtTest>
#include <QBuffer>
#include <QCheckBox>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QImage>
#include <QJsonDocument>
#include <QLabel>
#include <QPushButton>
#include <QSet>
#include <QSpinBox>
#include <QTableWidget>
#include <QTemporaryDir>
#include "stage3_project_edits.h"

//20261110-8 add: Exercise complete project transactions without USB, global clipboard or foreground windows.
class Stage3ProjectEditsTests : public QObject
{
    Q_OBJECT
private slots:
    void completeFiveFamilyPaste();
    void imageActualBytesAndExplicitPoolReferences();
    void exactImageReuseAndCorruptResourceRollback();
    void macroConflictAndIdentityMapping();
    void languageAndConnectionConflicts();
    void internalAndExternalReferences();
    void cutAndInboundReferenceRollback();
    void duplicateGridRolesAndDormantDrafts();
    void wideAddressSpanRollback();
    void invalidGridAndSelection();
    void compositeExternalIncrementRejected();
    void dialogPreviewCancelAcceptAndReopen();
    void dialogNativeIncrementDirections_data();
    void dialogNativeIncrementDirections();
    void dialogNativeU32LiteralSteps_data();
    void dialogNativeU32LiteralSteps();
};

/******************************************************************************
 * Function Name   : bytesOf
 * Description     : Capture complete persistence for atomicity and exact untouched-object comparisons.
 * Input Parameters: project
 * Return Value    : Compact JSON bytes.
 * Date            : 2026.11.10
 ******************************************************************************/
static QByteArray bytesOf(const HmiProject &project) //20261110-8 add
{
    return QJsonDocument(project.toJson()).toJson(QJsonDocument::Compact);
}

/******************************************************************************
 * Function Name   : familyObject
 * Description     : Construct one valid representative from each existing five-family model path.
 * Input Parameters: family
 * Return Value    : Complete editable object with stable identity and independent geometry.
 * Date            : 2026.11.10
 ******************************************************************************/
static HmiObject familyObject(int family) //20261110-8 add
{
    HmiObject object;

    object.id = QStringLiteral("source-object-%1").arg(family);
    object.name = QStringLiteral("Family %1").arg(family);
    object.type = family == 0 ? QStringLiteral("text") : QStringLiteral("switch");
    object.text = QStringLiteral("Source text");
    object.geometry = QRect(10 + family * 90, 20, 70, 40);
    object.group = QStringLiteral("source-group");
    object.layer = family + 0.5;
    if (family == 0)
    {
        object.hasStaticTextProperties = true;
        object.staticText.fontSize = 17.5;
        object.staticText.spacing = 0.25;
        object.staticText.languageTexts.insert(QStringLiteral("中文"), QStringLiteral("中文内容"));
        object.staticText.languageTexts.insert(QStringLiteral("English"), QStringLiteral("English content"));
    }
    else
    {
        object.hasBitButtonProperties = true;
        object.address = QStringLiteral("LB0");
        object.bitButton.states[0].text.languageTexts.insert(QStringLiteral("English"), QStringLiteral("OFF draft"));
        object.bitButton.states[1].text.languageTexts.insert(QStringLiteral("English"), QStringLiteral("ON draft"));
        object.bitButton.noticeWordAddress = QStringLiteral("LW123");
        if (family == 2)
        {
            object.hasWordButtonProperties = true;
            object.address = QStringLiteral("LW20");
            object.bitButton.indicator = false;
            object.bitButton.dualMonitor = false;
        }
        if (family == 3)
        {
            object.hasLampProperties = true;
            object.address.clear();
            object.bitButton.monitorSame = false;
            object.bitButton.bitIndex = 0;
            object.bitButton.monitorBitAddress = QStringLiteral("LB40");
        }
        if (family == 4)
        {
            object.hasFunctionButtonProperties = true;
            object.address.clear();
            object.bitButton.indicator = false;
            object.bitButton.dualMonitor = false;
        }
    }
    return object;
}

/******************************************************************************
 * Function Name   : imageBytes
 * Description     : Generate a genuine decodable PNG for exact-byte image-identity tests.
 * Input Parameters: color
 * Return Value    : PNG bytes.
 * Date            : 2026.11.10
 ******************************************************************************/
static QByteArray imageBytes(const QColor &color) //20261110-8 add
{
    QImage image(5, 3, QImage::Format_ARGB32);
    QBuffer buffer;

    image.fill(color);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return buffer.data();
}

/******************************************************************************
 * Function Name   : widget
 * Description     : Locate a named real dialog widget for deterministic offscreen interaction.
 * Input Parameters: dialog, name
 * Return Value    : Typed child pointer.
 * Date            : 2026.11.10
 ******************************************************************************/
template <typename T>
static T *widget(QObject *dialog, const char *name) //20261110-8 add
{
    return dialog->findChild<T *>(QString::fromLatin1(name));
}

/******************************************************************************
 * Function Name   : completeFiveFamilyPaste
 * Description     : Paste all five families without losing complete active or dormant properties and translations.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::completeFiveFamilyPaste() //20261110-8 add
{
    HmiProject source;
    HmiProject target;
    HmiProject draft;
    Stage3Clipboard clipboard;
    QStringList selection;
    QStringList ids;
    QByteArray before;
    QSet<QString> groups;
    QJsonObject expected;
    QJsonObject actual;
    int index;
    QString error;

    source.languages.append(QStringLiteral("English"));
    target.languages = source.languages;
    target.activeLanguage = QStringLiteral("English");
    for (index = 0; index < 5; ++index) { source.objects.append(familyObject(index)); selection.append(source.objects.last().id); }
    QVERIFY2(source.validateModel(&error), qPrintable(error));
    before = bytesOf(source);
    QVERIFY2(captureStage3Selection(source, selection, &clipboard, &error), qPrintable(error));
    QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(16, 16), &draft, &ids, &error), qPrintable(error));
    QCOMPARE(draft.objects.size(), 5);
    QCOMPARE(ids.size(), 5);
    for (index = 0; index < 5; ++index)
    {
        expected = source.objects.at(index).toJson();
        actual = draft.objects.at(index).toJson();
        QVERIFY(draft.objects.at(index).id != source.objects.at(index).id);
        QVERIFY(draft.objects.at(index).group != source.objects.at(index).group);
        groups.insert(draft.objects.at(index).group);
        expected.remove(QStringLiteral("id")); expected.remove(QStringLiteral("group"));
        actual.remove(QStringLiteral("id")); actual.remove(QStringLiteral("group"));
        expected[QStringLiteral("x")] = expected.value(QStringLiteral("x")).toInt() + 16;
        expected[QStringLiteral("y")] = expected.value(QStringLiteral("y")).toInt() + 16;
        QCOMPARE(actual, expected);
    }
    QCOMPARE(groups.size(), 1);
    QCOMPARE(bytesOf(source), before);
    QVERIFY(target.objects.isEmpty());
    QCOMPARE(draft.activeLanguage, QStringLiteral("English"));
}

/******************************************************************************
 * Function Name   : imageActualBytesAndExplicitPoolReferences
 * Description     : Distinguish state embedded bytes from a same-named but different source resource-pool slot.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::imageActualBytesAndExplicitPoolReferences() //20261110-8 add
{
    HmiProject source;
    HmiProject target;
    HmiProject draft;
    HmiObject object;
    HmiReference reference;
    Stage3Clipboard clipboard;
    QByteArray red;
    QByteArray blue;
    QByteArray green;
    QString stateName;
    QString refName;
    QString error;

    red = imageBytes(Qt::red); blue = imageBytes(Qt::blue); green = imageBytes(Qt::green);
    object = familyObject(1);
    object.bitButton.states[0].picture = QStringLiteral("Icon");
    object.bitButton.states[0].picturePng = red;
    source.objects.append(object);
    source.pictures.insert(QStringLiteral("Icon"), blue);
    target.pictures.insert(QStringLiteral("Icon"), green);
    reference.id = QStringLiteral("picture-reference");
    reference.ownerKind = QStringLiteral("object"); reference.ownerId = object.id;
    reference.field = QStringLiteral("pool-slot"); reference.targetKind = QStringLiteral("picture"); reference.targetId = QStringLiteral("Icon");
    source.references.append(reference);
    QVERIFY2(captureStage3Selection(source, {object.id}, &clipboard, &error), qPrintable(error));
    QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(0, 0), &draft, nullptr, &error), qPrintable(error));
    stateName = draft.objects.first().bitButton.states[0].picture;
    refName = draft.references.first().targetId;
    QCOMPARE(stateName, QStringLiteral("Icon~") + QString::fromLatin1(QCryptographicHash::hash(red, QCryptographicHash::Sha256).toHex()).left(12));
    QVERIFY(stateName != refName);
    QCOMPARE(draft.pictures.value(QStringLiteral("Icon")), green);
    QCOMPARE(draft.pictures.value(stateName), red);
    QCOMPARE(draft.pictures.value(refName), blue);
    QCOMPARE(draft.objects.first().bitButton.states[0].picturePng, red);
    QCOMPARE(source.objects.first().bitButton.states[0].picture, QStringLiteral("Icon"));
}

/******************************************************************************
 * Function Name   : exactImageReuseAndCorruptResourceRollback
 * Description     : Reuse exact bytes under another name and refuse corrupt image data without partial output changes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::exactImageReuseAndCorruptResourceRollback() //20261110-8 add
{
    HmiProject source;
    HmiProject target;
    HmiProject draft;
    HmiObject object;
    Stage3Clipboard clipboard;
    QStringList ids;
    QByteArray before;
    QString error;

    object = familyObject(1);
    object.bitButton.states[0].picture = QStringLiteral("Original Name");
    object.bitButton.states[0].picturePng = imageBytes(Qt::cyan);
    source.objects.append(object);
    target.pictures.insert(QStringLiteral("Existing Content"), object.bitButton.states[0].picturePng);
    QVERIFY(captureStage3Selection(source, {object.id}, &clipboard, &error));
    QVERIFY(pasteStage3Selection(target, clipboard, QPoint(0, 0), &draft, nullptr, &error));
    QCOMPARE(draft.pictures.size(), 1);
    QCOMPARE(draft.objects.first().bitButton.states[0].picture, QStringLiteral("Existing Content"));
    before = bytesOf(draft);
    ids = {QStringLiteral("unchanged-output-id")};
    clipboard.source.objects[0].bitButton.states[0].picturePng = QByteArrayLiteral("not-a-real-png");
    QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(0, 0), &draft, &ids, &error));
    QCOMPARE(bytesOf(draft), before);
    QCOMPARE(ids, QStringList{QStringLiteral("unchanged-output-id")});
}

/******************************************************************************
 * Function Name   : macroConflictAndIdentityMapping
 * Description     : Reject same-name different-source macros and map identical-source stable macro references safely.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::macroConflictAndIdentityMapping() //20261110-8 add
{
    HmiProject source;
    HmiProject target;
    HmiProject draft;
    HmiObject object;
    HmiReference reference;
    Stage3Clipboard clipboard;
    QByteArray before;
    QString error;

    object = familyObject(1);
    object.bitButton.macroEnabled = true;
    object.bitButton.macroName = QStringLiteral("MacroA");
    source.objects.append(object);
    source.macros.insert(QStringLiteral("MacroA"), QStringLiteral("void MacroA() {}"));
    source.macroIds.insert(QStringLiteral("MacroA"), QStringLiteral("source-macro-id"));
    reference.id = QStringLiteral("macro-ref"); reference.ownerKind = QStringLiteral("object"); reference.ownerId = object.id;
    reference.field = QStringLiteral("macro"); reference.targetKind = QStringLiteral("macro"); reference.targetId = QStringLiteral("source-macro-id");
    source.references.append(reference);
    QVERIFY(captureStage3Selection(source, {object.id}, &clipboard, &error));
    target.macros.insert(QStringLiteral("MacroA"), QStringLiteral("void MacroA() { different(); }"));
    draft.name = QStringLiteral("output must survive");
    before = bytesOf(draft);
    QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(), &draft, nullptr, &error));
    QVERIFY(error.contains(QStringLiteral("源码不同")));
    QCOMPARE(bytesOf(draft), before);
    target.macros[QStringLiteral("MacroA")] = source.macros.value(QStringLiteral("MacroA"));
    target.macroIds.insert(QStringLiteral("MacroA"), QStringLiteral("target-macro-id"));
    QVERIFY2(pasteStage3Selection(target, clipboard, QPoint(), &draft, nullptr, &error), qPrintable(error));
    QCOMPARE(draft.references.first().targetId, QStringLiteral("target-macro-id"));
    QCOMPARE(draft.macroIds.value(QStringLiteral("MacroA")), QStringLiteral("target-macro-id"));
    QCOMPARE(draft.macros.value(QStringLiteral("MacroA")), source.macros.value(QStringLiteral("MacroA")));
}

/******************************************************************************
 * Function Name   : languageAndConnectionConflicts
 * Description     : Refuse language-index and connection-contract changes instead of silently reinterpreting copied data.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::languageAndConnectionConflicts() //20261110-8 add
{
    HmiProject source;
    HmiProject target;
    HmiProject draft;
    Stage3Clipboard clipboard;
    PlcCatalog catalog;
    PlcConnection connection;
    PlcProfile profile;
    QByteArray before;
    QString error;

    source.objects.append(familyObject(1));
    QVERIFY(captureStage3Selection(source, {source.objects.first().id}, &clipboard, &error));
    target.languages.append(QStringLiteral("English"));
    before = bytesOf(draft);
    QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(), &draft, nullptr, &error));
    QCOMPARE(bytesOf(draft), before);
    target.languages = source.languages;
    QVERIFY(catalog.load(QStringLiteral(":/plc/plc-catalog.json"), &error));
    Q_FOREACH (profile, catalog.profiles())
    {
        if (plcProfileHasModbusCodec(profile)) { connection.profileId = profile.id; break; }
    }
    QVERIFY(!connection.profileId.isEmpty());
    connection.id = QStringLiteral("connection-new"); connection.channel = PlcChannel::Com1;
    connection.station = QStringLiteral("1"); connection.nodeKnown = true; connection.node = 1;
    target.connections.append(connection);
    QVERIFY(target.validateModel(&error));
    QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(), &draft, nullptr, &error));
    QCOMPARE(bytesOf(draft), before);
}

/******************************************************************************
 * Function Name   : internalAndExternalReferences
 * Description     : Remap selected-object targets per clone and reject absent dependencies outside the selection.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::internalAndExternalReferences() //20261110-8 add
{
    HmiProject source;
    HmiProject target;
    HmiProject draft;
    Stage3Clipboard clipboard;
    HmiReference reference;
    QStringList ids;
    QString error;
    QByteArray before;

    source.objects.append(familyObject(0)); source.objects.append(familyObject(1));
    reference.id = QStringLiteral("peer-ref"); reference.ownerKind = QStringLiteral("object"); reference.ownerId = source.objects.first().id;
    reference.field = QStringLiteral("peer"); reference.targetKind = QStringLiteral("object"); reference.targetId = source.objects.last().id;
    source.references.append(reference);
    QVERIFY(captureStage3Selection(source, {source.objects.first().id, source.objects.last().id}, &clipboard, &error));
    QVERIFY(pasteStage3Selection(target, clipboard, QPoint(), &draft, &ids, &error));
    QCOMPARE(draft.references.size(), 1);
    QCOMPARE(draft.references.first().ownerId, ids.first());
    QCOMPARE(draft.references.first().targetId, ids.last());
    QVERIFY(draft.references.first().id != reference.id);
    QVERIFY(captureStage3Selection(source, {source.objects.first().id}, &clipboard, &error));
    before = bytesOf(draft);
    QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(), &draft, nullptr, &error));
    QCOMPARE(bytesOf(draft), before);
    QVERIFY(pasteStage3Selection(source, clipboard, QPoint(0, 50), &draft, &ids, &error));
    QCOMPARE(draft.references.last().targetId, source.objects.last().id);
}

/******************************************************************************
 * Function Name   : cutAndInboundReferenceRollback
 * Description     : Cut clipboard and project outputs together, rejecting external inbound references first.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::cutAndInboundReferenceRollback() //20261110-8 add
{
    HmiProject source;
    HmiProject draft;
    HmiProject pasted;
    Stage3Clipboard clipboard;
    HmiReference reference;
    QByteArray before;
    QStringList oldClipboardIds;
    QString error;

    source.objects.append(familyObject(1));
    reference.id = QStringLiteral("inbound-project-ref"); reference.ownerKind = QStringLiteral("project"); reference.ownerId = source.projectId;
    reference.field = QStringLiteral("target"); reference.targetKind = QStringLiteral("object"); reference.targetId = source.objects.first().id;
    source.references.append(reference);
    clipboard.objectIds = {QStringLiteral("must-survive")};
    oldClipboardIds = clipboard.objectIds;
    before = bytesOf(draft);
    QVERIFY(!cutStage3Selection(source, {source.objects.first().id}, &clipboard, &draft, &error));
    QCOMPARE(bytesOf(draft), before);
    QCOMPARE(clipboard.objectIds, oldClipboardIds);
    source.references.clear();
    QVERIFY(cutStage3Selection(source, {source.objects.first().id}, &clipboard, &draft, &error));
    QVERIFY(draft.objects.isEmpty());
    QCOMPARE(source.objects.size(), 1);
    QVERIFY(pasteStage3Selection(draft, clipboard, QPoint(), &pasted, nullptr, &error));
    QCOMPARE(pasted.objects.size(), 1);
    QVERIFY(pasted.objects.first().id != source.objects.first().id);
}

/******************************************************************************
 * Function Name   : duplicateGridRolesAndDormantDrafts
 * Description     : Verify row-major geometry and independent explicitly selected address roles without dormant edits.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::duplicateGridRolesAndDormantDrafts() //20261110-8 add
{
    HmiProject source;
    HmiProject draft;
    HmiObject object;
    Stage3DuplicateOptions options;
    Stage3AddressIncrement operation;
    Stage3AddressIncrement notice;
    QStringList ids;
    QStringList expectedOperations;
    QStringList expectedNotices;
    QString error;
    int index;

    object = familyObject(1); object.geometry = QRect(10, 20, 20, 10);
    object.bitButton.notice = true; object.bitButton.noticeBit = true; object.bitButton.noticeBitAddress = QStringLiteral("LB70");
    object.bitButton.visibility.visibilityEnabled = true; object.bitButton.visibility.bitEnabled = true;
    object.bitButton.visibility.bitAddress = QStringLiteral("LB50");
    source.objects.append(object);
    options.rows = 2; options.columns = 3; options.offsetX = 40; options.offsetY = 30;
    operation.field = Stage3AddressField::Operation; operation.columnStep = 1; operation.rowStep = 10;
    notice.field = Stage3AddressField::NoticeBit; notice.columnStep = 2; notice.rowStep = 20;
    options.increments = {operation, notice};
    QVERIFY2(duplicateStage3Selection(source, {object.id}, options, &draft, &ids, &error), qPrintable(error));
    QCOMPARE(draft.objects.size(), 6); QCOMPARE(ids.size(), 5);
    QCOMPARE(draft.objects.first().toJson(), object.toJson());
    expectedOperations = {QStringLiteral("LB1"), QStringLiteral("LB2"), QStringLiteral("LB10"), QStringLiteral("LB11"), QStringLiteral("LB12")};
    expectedNotices = {QStringLiteral("LB72"), QStringLiteral("LB74"), QStringLiteral("LB90"), QStringLiteral("LB92"), QStringLiteral("LB94")};
    for (index = 0; index < ids.size(); ++index)
    {
        QCOMPARE(draft.objects.at(index + 1).address, expectedOperations.at(index));
        QCOMPARE(draft.objects.at(index + 1).bitButton.noticeBitAddress, expectedNotices.at(index));
        QCOMPARE(draft.objects.at(index + 1).bitButton.noticeWordAddress, QStringLiteral("LW123"));
        QCOMPARE(draft.objects.at(index + 1).bitButton.visibility.bitAddress, QStringLiteral("LB50"));
    }
    QCOMPARE(draft.objects.at(1).geometry.topLeft(), QPoint(50, 20));
    QCOMPARE(draft.objects.at(3).geometry.topLeft(), QPoint(10, 50));
    QCOMPARE(source.objects.size(), 1);
}

/******************************************************************************
 * Function Name   : wideAddressSpanRollback
 * Description     : Reject a U32 tail-address overflow even when its starting word alone would be legal.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::wideAddressSpanRollback() //20261110-8 add
{
    HmiProject source;
    HmiProject output;
    HmiObject object;
    Stage3DuplicateOptions options;
    Stage3AddressIncrement increment;
    QStringList ids;
    QByteArray before;
    QString error;

    object = familyObject(2);
    object.wordButton.resetDefaultsForDataType(1);
    QCOMPARE(object.wordButton.transferWordCount(), 2);
    object.address = QStringLiteral("LW4095");
    source.objects.append(object);
    options.rows = 1; options.columns = 2; options.offsetX = 100;
    increment.field = Stage3AddressField::Operation; increment.columnStep = 1; options.increments.append(increment);
    before = bytesOf(output); ids = {QStringLiteral("keep")};
    QVERIFY(!duplicateStage3Selection(source, {object.id}, options, &output, &ids, &error));
    QCOMPARE(bytesOf(output), before); QCOMPARE(ids, QStringList{QStringLiteral("keep")});
    source.objects[0].address = QStringLiteral("LW4094");
    QVERIFY2(duplicateStage3Selection(source, {object.id}, options, &output, &ids, &error), qPrintable(error));
    QCOMPARE(output.objects.last().address, QStringLiteral("LW4095"));
}

/******************************************************************************
 * Function Name   : invalidGridAndSelection
 * Description     : Keep outputs unchanged for duplicate IDs, unavailable roles, grid limits and geometry failures.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::invalidGridAndSelection() //20261110-8 add
{
    HmiProject source;
    HmiProject output;
    Stage3Clipboard clipboard;
    Stage3DuplicateOptions options;
    Stage3AddressIncrement increment;
    QByteArray before;
    QStringList ids;
    QString error;

    source.objects.append(familyObject(1));
    before = bytesOf(output);
    QVERIFY(!captureStage3Selection(source, {source.objects.first().id, source.objects.first().id}, &clipboard, &error));
    QVERIFY(!captureStage3Selection(source, {QStringLiteral("missing")}, &clipboard, &error));
    options.rows = 0;
    QVERIFY(!duplicateStage3Selection(source, {source.objects.first().id}, options, &output, &ids, &error));
    options.rows = 99; options.columns = 99;
    QVERIFY(!duplicateStage3Selection(source, {source.objects.first().id}, options, &output, &ids, &error));
    options.rows = 1; options.columns = 2; options.offsetX = 800;
    QVERIFY(!duplicateStage3Selection(source, {source.objects.first().id}, options, &output, &ids, &error));
    options.offsetX = 1;
    increment.field = Stage3AddressField::TouchWord; increment.columnStep = 1; options.increments = {increment};
    QVERIFY(!duplicateStage3Selection(source, {source.objects.first().id}, options, &output, &ids, &error));
    QCOMPARE(bytesOf(output), before);
    options.increments.clear(); options.columns = 1;
    QVERIFY(duplicateStage3Selection(source, {source.objects.first().id}, options, &output, &ids, &error));
    QCOMPARE(bytesOf(output), bytesOf(source)); QVERIFY(ids.isEmpty());
}

/******************************************************************************
 * Function Name   : compositeExternalIncrementRejected
 * Description     : Reject unverified segmented external increment arithmetic while retaining ordinary copy support.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::compositeExternalIncrementRejected() //20261110-8 add
{
    HmiProject source;
    HmiProject output;
    HmiObject object;
    PlcCatalog catalog;
    PlcProfile profile;
    PlcConnection connection;
    Stage3DuplicateOptions options;
    Stage3AddressIncrement increment;
    QString error;
    QByteArray before;

    QVERIFY(catalog.load(QStringLiteral(":/plc/plc-catalog.json"), &error));
    Q_FOREACH (profile, catalog.profiles())
    {
        if (plcProfileHasModbusCodec(profile)) { connection.profileId = profile.id; break; }
    }
    QVERIFY(!connection.profileId.isEmpty());
    connection.id = QStringLiteral("plc-1"); connection.channel = PlcChannel::Com1;
    connection.station = QStringLiteral("1"); connection.nodeKnown = true; connection.node = 1;
    source.connections.append(connection);
    object = familyObject(1); object.address = QStringLiteral("@plc-1:4x_bit0.00"); source.objects.append(object);
    QVERIFY2(source.validateModel(&error), qPrintable(error));
    options.columns = 2; options.rows = 1; options.offsetX = 100;
    increment.columnStep = 1; options.increments.append(increment);
    before = bytesOf(output);
    QVERIFY(!duplicateStage3Selection(source, {object.id}, options, &output, nullptr, &error));
    QCOMPARE(bytesOf(output), before);
    options.increments.clear();
    QVERIFY2(duplicateStage3Selection(source, {object.id}, options, &output, nullptr, &error), qPrintable(error));
    QCOMPARE(output.objects.last().address, object.address);
}

/******************************************************************************
 * Function Name   : dialogPreviewCancelAcceptAndReopen
 * Description     : Exercise real preview/cancel/accept controls, edge-gap conversion and whole-project reopen.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::dialogPreviewCancelAcceptAndReopen() //20261110-8 add
{
    HmiProject source;
    HmiProject reopened;
    HmiObject object;
    Stage3DuplicateDialog *canceled;
    Stage3DuplicateDialog *accepted;
    QTemporaryDir directory;
    QString error;

    object = familyObject(1); object.geometry = QRect(20, 20, 30, 15); source.objects.append(object);
    canceled = new Stage3DuplicateDialog(source, {object.id});
    widget<QSpinBox>(canceled, "duplicateColumnsSpin")->setValue(3);
    widget<QSpinBox>(canceled, "duplicateRowsSpin")->setValue(2);
    QCOMPARE(widget<QTableWidget>(canceled, "duplicatePreviewTable")->rowCount(), 5);
    QCOMPARE(bytesOf(canceled->draftProject()), bytesOf(source));
    QVERIFY(canceled->newObjectIds().isEmpty());
    widget<QDialogButtonBox>(canceled, "buttonBox")->button(QDialogButtonBox::Cancel)->click();
    QCOMPARE(canceled->result(), int(QDialog::Rejected));
    QCOMPARE(bytesOf(canceled->draftProject()), bytesOf(source));
    delete canceled;
    accepted = new Stage3DuplicateDialog(source, {object.id});
    widget<QSpinBox>(accepted, "duplicateColumnsSpin")->setValue(3);
    widget<QSpinBox>(accepted, "duplicateRowsSpin")->setValue(2);
    widget<QSpinBox>(accepted, "duplicateOffsetXSpin")->setValue(17);
    widget<QSpinBox>(accepted, "duplicateOffsetYSpin")->setValue(23);
    widget<QDialogButtonBox>(accepted, "buttonBox")->button(QDialogButtonBox::Ok)->click();
    QCOMPARE(accepted->result(), int(QDialog::Accepted));
    QCOMPARE(accepted->draftProject().objects.size(), 6);
    QCOMPARE(accepted->draftProject().objects.at(1).geometry.topLeft(), QPoint(67, 20));
    QCOMPARE(accepted->draftProject().objects.at(3).geometry.topLeft(), QPoint(20, 58));
    QVERIFY(accepted->draftProject().save(directory.filePath(QStringLiteral("batch.ksp")), &error));
    QVERIFY(HmiProject::load(directory.filePath(QStringLiteral("batch.ksp")), &reopened, &error));
    QCOMPARE(bytesOf(reopened), bytesOf(accepted->draftProject()));
    delete accepted;
}

/******************************************************************************
 * Function Name   : dialogNativeIncrementDirections_data
 * Description     : Register independently observed row-major/right and column-major/down native orders.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::dialogNativeIncrementDirections_data() //20261110-8 add
{
    QTest::addColumn<int>("direction");
    QTest::addColumn<QStringList>("operations");
    QTest::addColumn<QStringList>("monitors");
    QTest::newRow("native-right") << 0
        << QStringList{QStringLiteral("LB7"), QStringLiteral("LB12"), QStringLiteral("LB17"), QStringLiteral("LB22"), QStringLiteral("LB27"), QStringLiteral("LB32")}
        << QStringList{QStringLiteral("LW40"), QStringLiteral("LW45"), QStringLiteral("LW50"), QStringLiteral("LW55"), QStringLiteral("LW60"), QStringLiteral("LW65")};
    QTest::newRow("native-down") << 1
        << QStringList{QStringLiteral("LB7"), QStringLiteral("LB17"), QStringLiteral("LB27"), QStringLiteral("LB12"), QStringLiteral("LB22"), QStringLiteral("LB32")}
        << QStringList{QStringLiteral("LW40"), QStringLiteral("LW50"), QStringLiteral("LW60"), QStringLiteral("LW45"), QStringLiteral("LW55"), QStringLiteral("LW65")};
}

/******************************************************************************
 * Function Name   : dialogNativeIncrementDirections
 * Description     : Copy a real dialog's 2x3 grid with native address order while preserving all common address roles.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::dialogNativeIncrementDirections() //20261110-8 add
{
    QFETCH(int, direction);
    QFETCH(QStringList, operations);
    QFETCH(QStringList, monitors);
    HmiProject source;
    HmiProject result;
    HmiObject object;
    Stage3DuplicateDialog *dialog;
    QString error;
    int index;

    object = familyObject(1); object.geometry = QRect(20, 20, 20, 10); object.address = QStringLiteral("LB7");
    object.bitButton.monitorSame = false; object.bitButton.monitorType = 1; object.bitButton.monitorWordAddress = QStringLiteral("LW40");
    object.bitButton.notice = true; object.bitButton.noticeBit = true; object.bitButton.noticeWord = true;
    object.bitButton.noticeBitAddress = QStringLiteral("LB21"); object.bitButton.noticeWordAddress = QStringLiteral("LW44");
    object.bitButton.visibility.visibilityEnabled = true; object.bitButton.visibility.bitEnabled = true; object.bitButton.visibility.wordEnabled = true;
    object.bitButton.visibility.bitAddress = QStringLiteral("LB31"); object.bitButton.visibility.wordAddress = QStringLiteral("LW52");
    object.bitButton.touch.visibilityEnabled = true; object.bitButton.touch.bitEnabled = true; object.bitButton.touch.wordEnabled = true;
    object.bitButton.touch.bitAddress = QStringLiteral("LB41"); object.bitButton.touch.wordAddress = QStringLiteral("LW60");
    source.objects.append(object);
    QVERIFY2(source.validateModel(&error), qPrintable(error));
    dialog = new Stage3DuplicateDialog(source, {object.id});
    QVERIFY(!widget<QCheckBox>(dialog, "duplicateAddressIncrementCheck")->isChecked());
    QCOMPARE(widget<QSpinBox>(dialog, "duplicateAddressIncrementSpin")->minimum(), 0);
    QCOMPARE(widget<QSpinBox>(dialog, "duplicateAddressIncrementSpin")->maximum(), 256);
    QCOMPARE(widget<QSpinBox>(dialog, "duplicateAddressIncrementSpin")->value(), 1);
    widget<QSpinBox>(dialog, "duplicateColumnsSpin")->setValue(3);
    widget<QSpinBox>(dialog, "duplicateRowsSpin")->setValue(2);
    widget<QCheckBox>(dialog, "duplicateAddressIncrementCheck")->setChecked(true);
    widget<QSpinBox>(dialog, "duplicateAddressIncrementSpin")->setValue(5);
    widget<QComboBox>(dialog, "duplicateAddressDirectionCombo")->setCurrentIndex(direction);
    widget<QDialogButtonBox>(dialog, "buttonBox")->button(QDialogButtonBox::Ok)->click();
    QCOMPARE(dialog->result(), int(QDialog::Accepted));
    result = dialog->draftProject();
    QCOMPARE(result.objects.size(), 6);
    for (index = 0; index < result.objects.size(); ++index)
    {
        QCOMPARE(result.objects.at(index).address, operations.at(index));
        QCOMPARE(result.objects.at(index).bitButton.monitorWordAddress, monitors.at(index));
        QCOMPARE(result.objects.at(index).bitButton.noticeBitAddress, QStringLiteral("LB21"));
        QCOMPARE(result.objects.at(index).bitButton.noticeWordAddress, QStringLiteral("LW44"));
        QCOMPARE(result.objects.at(index).bitButton.visibility.bitAddress, QStringLiteral("LB31"));
        QCOMPARE(result.objects.at(index).bitButton.visibility.wordAddress, QStringLiteral("LW52"));
        QCOMPARE(result.objects.at(index).bitButton.touch.bitAddress, QStringLiteral("LB41"));
        QCOMPARE(result.objects.at(index).bitButton.touch.wordAddress, QStringLiteral("LW60"));
    }
    QCOMPARE(source.objects.first().toJson(), object.toJson());
    delete dialog;
}

/******************************************************************************
 * Function Name   : dialogNativeU32LiteralSteps_data
 * Description     : Cover native U32 word-operation and multistate-lamp address increments in literal word units.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::dialogNativeU32LiteralSteps_data() //20261110-8 add
{
    QTest::addColumn<bool>("lamp");
    QTest::newRow("native-u32-word") << false;
    QTest::newRow("native-u32-multistate-lamp") << true;
}

/******************************************************************************
 * Function Name   : dialogNativeU32LiteralSteps
 * Description     : Do not multiply a native increment interval by a two-word field's transfer width.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3ProjectEditsTests::dialogNativeU32LiteralSteps() //20261110-8 add
{
    QFETCH(bool, lamp);
    HmiProject source;
    HmiProject result;
    HmiObject object;
    Stage3DuplicateDialog *dialog;
    QString error;
    QString value;
    int index;
    int start;

    object = familyObject(lamp ? 3 : 2); object.geometry = QRect(20, 20, 20, 10);
    start = lamp ? 80 : 70;
    if (lamp)
    {
        object.lamp.monitorMode = 2; object.lamp.multiDataType = 1; object.lamp.multiMonitorSame = false;
        object.lamp.multiMonitorAddress = QStringLiteral("LW80");
        object.lamp.ensureStateCount(5, object.bitButton);
        object.bitButton.dualMonitor = false;
    }
    else { object.wordButton.resetDefaultsForDataType(1); object.address = QStringLiteral("LW70"); }
    source.objects.append(object);
    QVERIFY2(source.validateModel(&error), qPrintable(error));
    dialog = new Stage3DuplicateDialog(source, {object.id});
    widget<QSpinBox>(dialog, "duplicateColumnsSpin")->setValue(2);
    widget<QSpinBox>(dialog, "duplicateRowsSpin")->setValue(2);
    widget<QCheckBox>(dialog, "duplicateAddressIncrementCheck")->setChecked(true);
    widget<QSpinBox>(dialog, "duplicateAddressIncrementSpin")->setValue(5);
    widget<QDialogButtonBox>(dialog, "buttonBox")->button(QDialogButtonBox::Ok)->click();
    QCOMPARE(dialog->result(), int(QDialog::Accepted));
    result = dialog->draftProject();
    QCOMPARE(result.objects.size(), 4);
    for (index = 0; index < result.objects.size(); ++index)
    {
        value = lamp ? result.objects.at(index).lamp.multiMonitorAddress : result.objects.at(index).address;
        QCOMPARE(value, QStringLiteral("LW%1").arg(start + index * 5));
    }
    delete dialog;
}

QTEST_MAIN(Stage3ProjectEditsTests)
#include "stage3_project_edits_tests.moc"
