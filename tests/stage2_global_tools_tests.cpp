#include "stage2_global_tools.h"
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScopedPointer>
#include <QTableWidget>
#include <QtTest>

using namespace Stage2GlobalTools;

//20261110-7 add: These tests operate on value drafts and local widgets, never native apps or devices.
class Stage2GlobalToolsTests : public QObject
{
    Q_OBJECT
private slots:
    void crossPageIndexIsReadOnly();
    void navigationReturnsStableIdentity();
    void fiveFamiliesAndCompleteSpans();
    void lateFailureIsAtomic();
    void sameAddressMonitorKeepsIndependentDraft();
    void scopesAndStalePreviews();
    void protocolFamilyAndExternalSpan();
    void unknownFieldsAreExplicitlyRejected();
    void macroSearchNeverRewritesSource();
    void dialogPreviewCancelAndConfirm();
    void tamperedCandidatesAreRejected();
    void identicalReplacementDoesNotCreateTransaction();
    void stableIdsWithSeparatorsRemainDistinct();
};

/******************************************************************************
 * Function Name   : toolObject
 * Description     : Build established subtype records with distinct IDs and valid constraints.
 * Input Parameters: kind, id
 * Return Value    : One supported editor control
 * Date            : 2026.11.10
 ******************************************************************************/
static HmiObject toolObject(int kind, const QString &id) //20261110-7 add
{
    HmiObject object;

    object.id = id; object.name = id; object.type = kind == 0 ? QStringLiteral("text") : QStringLiteral("switch");
    object.geometry = QRect(10 + kind * 90, 20, 80, 45); object.text = QStringLiteral("Draft");
    if (kind == 0)
    {
        object.hasStaticTextProperties = true;
        object.staticText.languageTexts.insert(QStringLiteral("中文"), QStringLiteral("运行"));
        object.staticText.languageTexts.insert(QStringLiteral("English"), QStringLiteral("Running"));
    }
    else
    {
        object.hasBitButtonProperties = true; object.address = QStringLiteral("LB7");
        if (kind == 2)
        {
            object.hasWordButtonProperties = true; object.address = QStringLiteral("LW100");
            object.bitButton.indicator = false; object.bitButton.dualMonitor = false;
        }
        else if (kind == 3)
        {
            object.hasLampProperties = true; object.address.clear(); object.bitButton.monitorSame = false; object.bitButton.bitIndex = 0;
        }
        else if (kind == 4)
        {
            object.hasFunctionButtonProperties = true; object.address.clear();
            object.bitButton.indicator = false; object.bitButton.dualMonitor = false;
        }
    }
    return object;
}

/******************************************************************************
 * Function Name   : addSecondPage
 * Description     : Add another normal page while retaining active and startup page identities.
 * Input Parameters: project, object
 * Return Value    : True if the page is valid
 * Date            : 2026.11.10
 ******************************************************************************/
static bool addSecondPage(HmiProject *project, const HmiObject &object) //20261110-7 add
{
    HmiPage page;

    page.id = QStringLiteral("page-other"); page.name = QStringLiteral("另一画面"); page.objects.append(object);
    return project->addPage(page);
}

/******************************************************************************
 * Function Name   : crossPageIndexIsReadOnly
 * Description     : Verify independent-language indexing and cross-page IDs without changing view state.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2GlobalToolsTests::crossPageIndexIsReadOnly() //20261110-7 add
{
    HmiProject project;
    QJsonObject before;
    QList<Row> rows;
    Scope scope;
    QString error;

    project.languages.append(QStringLiteral("English")); project.objects.append(toolObject(1, QStringLiteral("first")));
    QVERIFY(addSecondPage(&project, toolObject(0, QStringLiteral("other-text"))));
    QVERIFY2(project.validateModel(&error), qPrintable(error)); before = project.toJson();
    rows = enumerate(project, Kind::Controls); QCOMPARE(rows.size(), 2);
    rows = search(project, Kind::Texts, QStringLiteral("Running")); QCOMPARE(rows.size(), 1);
    QCOMPARE(rows.first().location.pageId, QStringLiteral("page-other"));
    QCOMPARE(rows.first().location.objectId, QStringLiteral("other-text"));
    QCOMPARE(rows.first().language, QStringLiteral("English"));
    scope.range = Scope::Range::CurrentPage; scope.pageId = project.activePageId;
    QVERIFY(search(project, Kind::Texts, QStringLiteral("Running"), scope).isEmpty());
    QCOMPARE(project.toJson(), before);
}

/******************************************************************************
 * Function Name   : navigationReturnsStableIdentity
 * Description     : A table result navigates through stable IDs rather than mutating or reordering pages.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2GlobalToolsTests::navigationReturnsStableIdentity() //20261110-7 add
{
    HmiProject project;
    QJsonObject before;
    QTableWidget *table;
    QPushButton *locate;
    Location location;
    QScopedPointer<GlobalToolsDialog> dialog;

    project.objects.append(toolObject(1, QStringLiteral("first")));
    QVERIFY(addSecondPage(&project, toolObject(0, QStringLiteral("other-text")))); before = project.toJson();
    dialog.reset(new GlobalToolsDialog(project, Kind::Controls));
    table = dialog->findChild<QTableWidget *>(QStringLiteral("Stage2GlobalResults"));
    locate = dialog->findChild<QPushButton *>(QStringLiteral("Stage2GlobalLocate"));
    QVERIFY(table != nullptr); QVERIFY(locate != nullptr); QCOMPARE(table->rowCount(), 2);
    table->setCurrentCell(1, 0); locate->click(); location = dialog->selectedLocation();
    QCOMPARE(dialog->result(), int(QDialog::Accepted)); QCOMPARE(location.pageId, QStringLiteral("page-other"));
    QCOMPARE(location.objectId, QStringLiteral("other-text")); QCOMPARE(project.toJson(), before);
}

/******************************************************************************
 * Function Name   : fiveFamiliesAndCompleteSpans
 * Description     : Validate active operations, monitors and access/notice fields as a single transaction.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2GlobalToolsTests::fiveFamiliesAndCompleteSpans() //20261110-7 add
{
    HmiProject project;
    HmiProject replaced;
    HmiObject object;
    AddressCandidate candidate;
    QList<Row> rows;
    Row row;
    QJsonObject before;
    QString error;
    int kind;
    int wideRows;

    for (kind = 0; kind < 5; ++kind)
    {
        object = toolObject(kind, QStringLiteral("kind-%1").arg(kind));
        if (kind == 0)
        {
            object.staticText.visibilityEnabled = true; object.staticText.wordEnabled = true;
            object.staticText.dataType = 4; object.staticText.wordAddress = QStringLiteral("LW100");
        }
        else if (kind == 1)
        {
            object.bitButton.monitorSame = false; object.bitButton.monitorType = 1;
            object.bitButton.monitorWordAddress = QStringLiteral("LW100");
            object.bitButton.notice = true; object.bitButton.noticeWord = true;
            object.bitButton.noticeWordType = 4; object.bitButton.noticeWordAddress = QStringLiteral("LW100");
        }
        else if (kind == 2) { object.wordButton.resetDefaultsForDataType(1); }
        else if (kind == 3)
        {
            object.lamp.monitorMode = 2; object.bitButton.dualMonitor = false;
            object.lamp.multiDataType = 1; object.lamp.multiMonitorAddress = QStringLiteral("LW100");
        }
        else
        {
            object.bitButton.notice = true; object.bitButton.noticeWord = true;
            object.bitButton.noticeWordType = 1; object.bitButton.noticeWordAddress = QStringLiteral("LW100");
        }
        project.objects.append(object);
    }
    QVERIFY2(project.validateModel(&error), qPrintable(error)); before = project.toJson();
    rows = enumerate(project, Kind::Addresses); wideRows = 0;
    Q_FOREACH (row, rows)
    {
        if (row.value == QStringLiteral("LW100") && row.transferCount == 2) { ++wideRows; QCOMPARE(row.role, PlcAddressRole::Word); }
        if (row.location.objectId == QStringLiteral("kind-3") || row.location.objectId == QStringLiteral("kind-4"))
        {
            QVERIFY(row.fieldPath != QStringLiteral("address"));
        }
    }
    QCOMPARE(wideRows, 5);
    QVERIFY2(previewAddressReplacement(project, QStringLiteral("LW100"), QStringLiteral("LW4095"), Scope(), &candidate, &error), qPrintable(error));
    QCOMPARE(candidate.changes.size(), 6); QVERIFY2(applyCandidate(project, candidate, &replaced, &error), qPrintable(error));
    QCOMPARE(replaced.objects[0].staticText.wordAddress, QStringLiteral("LW4095"));
    QCOMPARE(replaced.objects[1].bitButton.monitorWordAddress, QStringLiteral("LW4095"));
    QCOMPARE(replaced.objects[1].bitButton.noticeWordAddress, QStringLiteral("LW4095"));
    QCOMPARE(replaced.objects[2].address, QStringLiteral("LW4095"));
    QCOMPARE(replaced.objects[3].lamp.multiMonitorAddress, QStringLiteral("LW4095"));
    QCOMPARE(replaced.objects[4].bitButton.noticeWordAddress, QStringLiteral("LW4095"));
    QVERIFY(!previewAddressReplacement(project, QStringLiteral("LW100"), QStringLiteral("LW4096"), Scope(), &candidate, &error));
    QVERIFY(error.contains(QStringLiteral("字段"))); QCOMPARE(project.toJson(), before);
}

/******************************************************************************
 * Function Name   : lateFailureIsAtomic
 * Description     : One invalid typed condition blocks earlier valid matches without publishing a partial draft.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2GlobalToolsTests::lateFailureIsAtomic() //20261110-7 add
{
    HmiProject project;
    HmiObject invalid;
    AddressCandidate candidate;
    QJsonObject before;
    QString error;

    project.objects.append(toolObject(1, QStringLiteral("valid-bit")));
    invalid = toolObject(0, QStringLiteral("bad-word-condition"));
    invalid.staticText.visibilityEnabled = true; invalid.staticText.wordEnabled = true;
    invalid.staticText.wordAddress = QStringLiteral("LB7"); project.objects.append(invalid);
    before = project.toJson(); candidate.projectFingerprint = QByteArrayLiteral("sentinel");
    QVERIFY(!previewAddressReplacement(project, QStringLiteral("LB7"), QStringLiteral("LB8"), Scope(), &candidate, &error));
    QVERIFY(error.contains(QStringLiteral("bad-word-condition"))); QVERIFY(error.contains(QStringLiteral("staticText.wordAddress")));
    QCOMPARE(candidate.projectFingerprint, QByteArrayLiteral("sentinel")); QVERIFY(candidate.changes.isEmpty());
    QCOMPARE(project.toJson(), before);
}

/******************************************************************************
 * Function Name   : sameAddressMonitorKeepsIndependentDraft
 * Description     : Replacing an operation changes its derived monitor once while preserving the independent draft.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2GlobalToolsTests::sameAddressMonitorKeepsIndependentDraft() //20261110-7 add
{
    HmiProject project;
    HmiProject output;
    AddressCandidate candidate;
    QList<Row> rows;
    Row row;
    QString error;
    int aliasRows;

    project.objects.append(toolObject(1, QStringLiteral("same-monitor")));
    project.objects[0].bitButton.monitorBitAddress = QStringLiteral("LB99");
    QVERIFY2(previewAddressReplacement(project, QStringLiteral("LB7"), QStringLiteral("lb0008"), Scope(), &candidate, &error), qPrintable(error));
    QCOMPARE(candidate.changes.size(), 1); QVERIFY2(applyCandidate(project, candidate, &output, &error), qPrintable(error));
    QCOMPARE(output.objects[0].address, QStringLiteral("LB8")); QVERIFY(output.objects[0].bitButton.monitorSame);
    QCOMPARE(output.objects[0].bitButton.monitorBitAddress, QStringLiteral("LB99"));
    rows = enumerate(output, Kind::Addresses); aliasRows = 0;
    Q_FOREACH (row, rows)
    {
        if (row.fieldPath == QStringLiteral("address")) { QCOMPARE(row.value, QStringLiteral("LB8")); ++aliasRows; }
    }
    QCOMPARE(aliasRows, 2); QCOMPARE(project.objects[0].address, QStringLiteral("LB7"));
}

/******************************************************************************
 * Function Name   : scopesAndStalePreviews
 * Description     : Scope changes preserve active page identity and stale previews cannot overwrite edits.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2GlobalToolsTests::scopesAndStalePreviews() //20261110-7 add
{
    HmiProject project;
    HmiProject output;
    HmiProject changed;
    AddressCandidate candidate;
    Scope scope;
    QJsonObject sentinel;
    QString error;

    project.objects.append(toolObject(1, QStringLiteral("first")));
    QVERIFY(addSecondPage(&project, toolObject(1, QStringLiteral("second"))));
    scope.range = Scope::Range::CurrentPage; scope.pageId = project.activePageId;
    QVERIFY(previewAddressReplacement(project, QStringLiteral("LB7"), QStringLiteral("LB8"), scope, &candidate, &error));
    QCOMPARE(candidate.changes.size(), 1);
    scope.range = Scope::Range::SelectedObjects; scope.objectIds.insert(QStringLiteral("first"));
    QVERIFY(previewAddressReplacement(project, QStringLiteral("LB7"), QStringLiteral("LB9"), scope, &candidate, &error));
    QCOMPARE(candidate.changes.size(), 1);
    scope.objectIds.clear(); QVERIFY(!previewAddressReplacement(project, QStringLiteral("LB7"), QStringLiteral("LB9"), scope, &candidate, &error));
    QVERIFY(previewAddressReplacement(project, QStringLiteral("LB7"), QStringLiteral("LB10"), Scope(), &candidate, &error));
    QCOMPARE(candidate.changes.size(), 2); QVERIFY2(applyCandidate(project, candidate, &output, &error), qPrintable(error));
    QCOMPARE(output.activePageId, project.activePageId); QCOMPARE(output.startupPageId, project.startupPageId);
    QCOMPARE(output.objects[0].address, QStringLiteral("LB10")); QCOMPARE(output.inactivePages[0].objects[0].address, QStringLiteral("LB10"));
    changed = project; changed.name = QStringLiteral("并发编辑"); output.name = QStringLiteral("保持原输出"); sentinel = output.toJson();
    QVERIFY(!applyCandidate(changed, candidate, &output, &error)); QCOMPARE(output.toJson(), sentinel);
}

/******************************************************************************
 * Function Name   : protocolFamilyAndExternalSpan
 * Description     : Keep external connections explicit and reject incompatible families and overflowing transfers.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2GlobalToolsTests::protocolFamilyAndExternalSpan() //20261110-7 add
{
    HmiProject project;
    PlcConnection connection;
    AddressCandidate candidate;
    HmiObject object;
    QString error;

    connection.id = QStringLiteral("com1"); connection.channel = PlcChannel::Com1;
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini"); connection.station = QStringLiteral("1");
    project.connections.append(connection); object = toolObject(1, QStringLiteral("external-bit"));
    object.address = QStringLiteral("@com1:0x1"); project.objects.append(object);
    QVERIFY(!previewAddressReplacement(project, object.address, QStringLiteral("@com1:4x1"), Scope(), &candidate, &error));
    QVERIFY(error.contains(QStringLiteral("external-bit")));
    QVERIFY(!previewAddressReplacement(project, object.address, QStringLiteral("@missing:0x1"), Scope(), &candidate, &error));
    QVERIFY(!previewAddressReplacement(project, QStringLiteral("@COM1:0x1"), QStringLiteral("@com1:0x2"), Scope(), &candidate, &error));
    QVERIFY(previewAddressReplacement(project, object.address, QStringLiteral("@com1:0x2"), Scope(), &candidate, &error));
    object = toolObject(2, QStringLiteral("external-wide")); object.wordButton.resetDefaultsForDataType(1);
    object.address = QStringLiteral("@com1:4x1"); project.objects = {object};
    QVERIFY(!previewAddressReplacement(project, object.address, QStringLiteral("@com1:4x65535"), Scope(), &candidate, &error));
    QVERIFY(error.contains(QStringLiteral("external-wide")));
}

/******************************************************************************
 * Function Name   : unknownFieldsAreExplicitlyRejected
 * Description     : A complete address in an untyped field blocks replacement with an actionable location.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2GlobalToolsTests::unknownFieldsAreExplicitlyRejected() //20261110-7 add
{
    HmiProject project;
    HmiObject object;
    AddressCandidate candidate;
    Scope scope;
    QJsonObject data;
    QJsonObject envelope;
    QString error;

    object = toolObject(0, QStringLiteral("untyped-address")); object.address = QStringLiteral("LW88"); project.objects.append(object);
    QVERIFY(!previewAddressReplacement(project, QStringLiteral("LW88"), QStringLiteral("LW89"), Scope(), &candidate, &error));
    QVERIFY(error.contains(QStringLiteral("untyped-address"))); QVERIFY(error.contains(QStringLiteral("address")));
    project.objects[0].address.clear(); data.insert(QStringLiteral("futureAddress"), QStringLiteral("LW88"));
    envelope.insert(QStringLiteral("version"), 1); envelope.insert(QStringLiteral("inactive"), true); envelope.insert(QStringLiteral("data"), data);
    project.objects[0].extensions.insert(QStringLiteral("test.metadata"), envelope);
    QVERIFY2(project.validateModel(&error), qPrintable(error));
    scope.includeDormant = true;
    QVERIFY(!previewAddressReplacement(project, QStringLiteral("LW88"), QStringLiteral("LW89"), scope, &candidate, &error));
    QVERIFY(error.contains(QStringLiteral("extensions.test.metadata.data.futureAddress")));
}

/******************************************************************************
 * Function Name   : macroSearchNeverRewritesSource
 * Description     : Search returns exact source line locations, while address edits leave macro code byte-identical.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2GlobalToolsTests::macroSearchNeverRewritesSource() //20261110-7 add
{
    HmiProject project;
    HmiProject output;
    AddressCandidate candidate;
    QList<Row> rows;
    QString source;
    QString error;
    Scope scope;

    source = QStringLiteral("MACRO_MAIN()\n// LB7 is a comment\nSetData(LB7);\n");
    project.objects.append(toolObject(1, QStringLiteral("button")));
    project.macros.insert(QStringLiteral("Drive"), source); project.macroIds.insert(QStringLiteral("Drive"), QStringLiteral("macro-drive"));
    rows = search(project, Kind::Macros, QStringLiteral("SetData")); QCOMPARE(rows.size(), 1);
    QCOMPARE(rows[0].location.macroId, QStringLiteral("macro-drive")); QCOMPARE(rows[0].location.macroName, QStringLiteral("Drive"));
    QCOMPARE(rows[0].location.line, 3); QVERIFY(!rows[0].replaceable);
    project.objects[0].bitButton.macroEnabled = true; project.objects[0].bitButton.macroName = QStringLiteral("Drive");
    scope.range = Scope::Range::CurrentPage; scope.pageId = project.activePageId;
    rows = search(project, Kind::Macros, QStringLiteral("SetData"), scope); QCOMPARE(rows.size(), 1);
    QCOMPARE(rows[0].location.line, 3);
    QVERIFY(previewAddressReplacement(project, QStringLiteral("LB7"), QStringLiteral("LB8"), Scope(), &candidate, &error));
    QVERIFY2(applyCandidate(project, candidate, &output, &error), qPrintable(error));
    QCOMPARE(output.macros.value(QStringLiteral("Drive")), source); QCOMPARE(project.macros.value(QStringLiteral("Drive")), source);
}

/******************************************************************************
 * Function Name   : dialogPreviewCancelAndConfirm
 * Description     : Preview and cancellation preserve caller state; only valid explicit acceptance exposes a draft.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2GlobalToolsTests::dialogPreviewCancelAndConfirm() //20261110-7 add
{
    HmiProject project;
    HmiProject output;
    QJsonObject original;
    QJsonObject sentinel;
    QLineEdit *find;
    QLineEdit *replacement;
    QPushButton *preview;
    QPushButton *confirm;
    QLabel *status;
    QScopedPointer<AddressReplaceDialog> canceled;
    QScopedPointer<AddressReplaceDialog> accepted;

    project.objects.append(toolObject(1, QStringLiteral("button"))); original = project.toJson();
    output.name = QStringLiteral("unchanged output"); sentinel = output.toJson();
    canceled.reset(new AddressReplaceDialog(project));
    canceled->findChild<QLineEdit *>(QStringLiteral("Stage2AddressFind"))->setText(QStringLiteral("LB7"));
    canceled->findChild<QLineEdit *>(QStringLiteral("Stage2AddressReplacement"))->setText(QStringLiteral("LB8"));
    canceled->findChild<QPushButton *>(QStringLiteral("Stage2AddressPreview"))->click();
    QVERIFY(canceled->findChild<QPushButton *>(QStringLiteral("Stage2AddressConfirm"))->isEnabled());
    QVERIFY(!canceled->candidateProject(&output)); QCOMPARE(output.toJson(), sentinel);
    canceled->findChild<QPushButton *>(QStringLiteral("Stage2AddressCancel"))->click();
    QVERIFY(!canceled->candidateProject(&output)); QVERIFY(canceled->candidate().changes.isEmpty()); QCOMPARE(project.toJson(), original);

    accepted.reset(new AddressReplaceDialog(project));
    find = accepted->findChild<QLineEdit *>(QStringLiteral("Stage2AddressFind"));
    replacement = accepted->findChild<QLineEdit *>(QStringLiteral("Stage2AddressReplacement"));
    preview = accepted->findChild<QPushButton *>(QStringLiteral("Stage2AddressPreview"));
    confirm = accepted->findChild<QPushButton *>(QStringLiteral("Stage2AddressConfirm"));
    status = accepted->findChild<QLabel *>(QStringLiteral("Stage2AddressStatus"));
    QVERIFY(find != nullptr && replacement != nullptr && preview != nullptr && confirm != nullptr && status != nullptr);
    find->setText(QStringLiteral("LB7")); replacement->setText(QStringLiteral("LB8")); preview->click(); QVERIFY(confirm->isEnabled());
    replacement->setText(QStringLiteral("LW0")); QVERIFY(!confirm->isEnabled()); preview->click(); QVERIFY(!confirm->isEnabled());
    QVERIFY(status->text().contains(QStringLiteral("button"))); accepted->accept(); QVERIFY(!accepted->candidateProject(&output));
    replacement->setText(QStringLiteral("LB8")); preview->click(); confirm->click();
    QVERIFY(accepted->candidateProject(&output)); QCOMPARE(output.objects[0].address, QStringLiteral("LB8")); QCOMPARE(project.toJson(), original);
}

/******************************************************************************
 * Function Name   : tamperedCandidatesAreRejected
 * Description     : A client cannot repurpose a preview to edit macro or arbitrary object fields.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2GlobalToolsTests::tamperedCandidatesAreRejected() //20261110-7 add
{
    HmiProject project;
    HmiProject output;
    AddressCandidate candidate;
    AddressCandidate tampered;
    QJsonObject sentinel;
    QString error;

    project.objects.append(toolObject(1, QStringLiteral("button")));
    QVERIFY(previewAddressReplacement(project, QStringLiteral("LB7"), QStringLiteral("LB8"), Scope(), &candidate, &error));
    output.name = QStringLiteral("preserve"); sentinel = output.toJson();
    tampered = candidate; tampered.changes[0].fieldPath = QStringLiteral("text");
    QVERIFY(!applyCandidate(project, tampered, &output, &error)); QCOMPARE(output.toJson(), sentinel);
    tampered = candidate; tampered.changes.append(tampered.changes.first());
    QVERIFY(!applyCandidate(project, tampered, &output, &error)); QCOMPARE(output.toJson(), sentinel);
    tampered = candidate; tampered.changes[0].after = QStringLiteral("LW1");
    QVERIFY(!applyCandidate(project, tampered, &output, &error)); QCOMPARE(output.toJson(), sentinel);
}

/******************************************************************************
 * Function Name   : identicalReplacementDoesNotCreateTransaction
 * Description     : Canonically identical addresses must not produce a pointless undo entry.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2GlobalToolsTests::identicalReplacementDoesNotCreateTransaction() //20261110-7 add
{
    HmiProject project;
    AddressCandidate candidate;
    QString error;

    project.objects.append(toolObject(1, QStringLiteral("button")));
    QVERIFY(!previewAddressReplacement(project, QStringLiteral("LB7"), QStringLiteral("lb0007"), Scope(), &candidate, &error));
    QVERIFY(candidate.changes.isEmpty()); QCOMPARE(project.objects[0].address, QStringLiteral("LB7"));
}

/******************************************************************************
 * Function Name   : stableIdsWithSeparatorsRemainDistinct
 * Description     : Distinct compound identities cannot collapse into one replacement target.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2GlobalToolsTests::stableIdsWithSeparatorsRemainDistinct() //20261110-7 add
{
    HmiProject project;
    HmiProject output;
    HmiPage page;
    AddressCandidate candidate;
    QString error;

    project.activePageId = QStringLiteral("a/b"); project.startupPageId = project.activePageId;
    project.pageOrder = {project.activePageId}; project.objects.append(toolObject(1, QStringLiteral("c")));
    page.id = QStringLiteral("a"); page.name = QStringLiteral("Second"); page.objects.append(toolObject(1, QStringLiteral("b/c")));
    QVERIFY2(project.addPage(page, &error), qPrintable(error));
    QVERIFY2(previewAddressReplacement(project, QStringLiteral("LB7"), QStringLiteral("LB8"), Scope(), &candidate, &error), qPrintable(error));
    QCOMPARE(candidate.changes.size(), 2); QVERIFY2(applyCandidate(project, candidate, &output, &error), qPrintable(error));
    QCOMPARE(output.objects[0].address, QStringLiteral("LB8")); QCOMPARE(output.inactivePages[0].objects[0].address, QStringLiteral("LB8"));
}

QTEST_MAIN(Stage2GlobalToolsTests)
#include "stage2_global_tools_tests.moc"
