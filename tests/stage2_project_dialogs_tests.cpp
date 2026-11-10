#include <QtTest>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTemporaryDir>
#include "stage2_project_dialogs.h"
#include "stage2_system_settings.h"

//20261110-7 add: Exercise actual detached dialogs without opening a device or the user's desktop.
class Stage2ProjectDialogsTests : public QObject
{
    Q_OBJECT
private slots:
    void wizardCancelCreatesNothing();
    void wizardBackKeepsInputs();
    void wizardAcceptDefersFileWrite();
    void wizardValidation_data();
    void wizardValidation();
    void wizardCannotReplaceExistingFile();
    void wizardNativeDefaultCommunicationAndLogoBoundary();
    void projectCancelKeepsOriginal();
    void pageAddRenameSelectAndReopen();
    void pageErrorsAreAtomic();
    void copyRemapsObjectsAndGroups();
    void referencedPageCannotBeDeleted();
    void startupIsIndependentAndRejectsWindow();
    void selectionWidgetsPreserveStartupAcrossCrud();
    void acceptedIdentityAndSelections();
    void invalidAcceptKeepsDialogOpen();
    void pageButtonsUseSameTransactions();
    void unsupportedRolesRemainPreserved();
    void existingLongNamesRemainLossless();
    void systemDefaultsRemainImplicit();
    void systemStrictProperties_data();
    void systemStrictProperties();
    void systemBoundaryValues();
    void systemUpdatePreservesOtherNodesAndExtensions();
    void systemConflictsAreAtomic();
    void systemDialogCancelAndReopen();
    void systemNativeSnapshotIsReadOnly();
};

/******************************************************************************
 * Function Name   : child
 * Description     : Locate a named real dialog control for deterministic offscreen interaction.
 * Input Parameters: parent, name
 * Return Value    : Typed child widget or null.
 * Date            : 2026.11.10
 ******************************************************************************/
template <typename T>
static T *child(QObject *parent, const char *name) //20261110-7 add
{
    return parent->findChild<T *>(QString::fromLatin1(name));
}

/******************************************************************************
 * Function Name   : snapshot
 * Description     : Capture exact editor persistence before a failed or canceled transaction.
 * Input Parameters: project
 * Return Value    : Compact serialized model.
 * Date            : 2026.11.10
 ******************************************************************************/
static QByteArray snapshot(const HmiProject &project) //20261110-7 add
{
    return QJsonDocument(project.toJson()).toJson(QJsonDocument::Compact);
}

/******************************************************************************
 * Function Name   : secondPage
 * Description     : Construct an independent ordinary page for management tests.
 * Input Parameters: None
 * Return Value    : Page with a deterministic non-startup identity.
 * Date            : 2026.11.10
 ******************************************************************************/
static HmiPage secondPage() //20261110-7 add
{
    HmiPage page;

    page.id = QStringLiteral("page-b");
    page.name = QStringLiteral("画面 B");
    return page;
}

/******************************************************************************
 * Function Name   : acceptSettings
 * Description     : Trigger the actual dialog button path used by the main-window integration.
 * Input Parameters: dialog
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
static void acceptSettings(Stage2ProjectDialog *dialog) //20261110-7 add
{
    QDialogButtonBox *buttons;

    buttons = child<QDialogButtonBox>(dialog, "buttonBox");
    Q_ASSERT(buttons != nullptr);
    buttons->button(QDialogButtonBox::Ok)->click();
}

/******************************************************************************
 * Function Name   : advanceWizard
 * Description     : Traverse all observed wizard steps through the visible Next command.
 * Input Parameters: dialog
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
static void advanceWizard(Stage2NewProjectDialog *dialog) //20261110-7 add
{
    QStackedWidget *steps;
    int previous;

    steps = child<QStackedWidget>(dialog, "wizardSteps");
    while (steps->currentIndex() < steps->count() - 1)
    {
        previous = steps->currentIndex();
        child<QPushButton>(dialog, "wizardNextButton")->click();
        if (steps->currentIndex() == previous) { return; }
    }
}

/******************************************************************************
 * Function Name   : wizardCancelCreatesNothing
 * Description     : Verify cancel preserves caller data and never creates the proposed destination.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::wizardCancelCreatesNothing() //20261110-7 add
{
    QTemporaryDir directory;
    HmiProject project;
    QByteArray before;
    QString path;
    Stage2NewProjectDialog *dialog;

    QVERIFY(directory.isValid());
    before = snapshot(project);
    path = directory.filePath(QStringLiteral("取消 草稿.ksp"));
    dialog = new Stage2NewProjectDialog(project, path);
    QCOMPARE(dialog->projectPath(), QString());
    child<QLineEdit>(dialog, "projectNameEdit")->setText(QStringLiteral("临时名称"));
    child<QPushButton>(dialog, "wizardCancelButton")->click();
    QCOMPARE(dialog->result(), int(QDialog::Rejected));
    QCOMPARE(dialog->projectPath(), QString());
    QCOMPARE(snapshot(project), before);
    QVERIFY(!QFile::exists(path));
    QCOMPARE(QDir(directory.path()).entryList(QDir::AllEntries | QDir::NoDotAndDotDot).size(), 0);
    delete dialog;
}

/******************************************************************************
 * Function Name   : wizardBackKeepsInputs
 * Description     : Preserve independent identity/page inputs when moving backward and forward.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::wizardBackKeepsInputs() //20261110-7 add
{
    QTemporaryDir directory;
    HmiProject project;
    Stage2NewProjectDialog dialog(project, directory.filePath(QStringLiteral("next.ksp")));
    QStackedWidget *steps;

    child<QLineEdit>(&dialog, "projectNameEdit")->setText(QStringLiteral("阶段2 中文工程"));
    child<QPushButton>(&dialog, "wizardNextButton")->click();
    steps = child<QStackedWidget>(&dialog, "wizardSteps");
    QCOMPARE(steps->currentIndex(), 1);
    QCOMPARE(dialog.windowTitle(), QStringLiteral("通信连接"));
    child<QPushButton>(&dialog, "wizardNextButton")->click();
    QCOMPARE(steps->currentIndex(), 2);
    child<QLineEdit>(&dialog, "pageNameEdit")->setText(QStringLiteral("首页 草稿"));
    child<QPushButton>(&dialog, "wizardBackButton")->click();
    QCOMPARE(steps->currentIndex(), 1);
    child<QPushButton>(&dialog, "wizardBackButton")->click();
    QCOMPARE(steps->currentIndex(), 0);
    QCOMPARE(child<QLineEdit>(&dialog, "projectNameEdit")->text(), QStringLiteral("阶段2 中文工程"));
    advanceWizard(&dialog);
    QCOMPARE(child<QLineEdit>(&dialog, "pageNameEdit")->text(), QStringLiteral("首页 草稿"));
    QVERIFY(!QFile::exists(directory.filePath(QStringLiteral("next.ksp"))));
}

/******************************************************************************
 * Function Name   : wizardAcceptDefersFileWrite
 * Description     : Accept a validated Unicode-path project but leave atomic saving to the caller.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::wizardAcceptDefersFileWrite() //20261110-7 add
{
    QTemporaryDir directory;
    HmiProject project;
    QString error;
    QString path;
    Stage2NewProjectDialog *dialog;

    project.languages.append(QStringLiteral("English"));
    path = directory.filePath(QStringLiteral("中文 新工程.ksp"));
    dialog = new Stage2NewProjectDialog(project, path);
    child<QLineEdit>(dialog, "projectNameEdit")->setText(QStringLiteral("中文 工程"));
    advanceWizard(dialog);
    child<QLineEdit>(dialog, "pageNameEdit")->setText(QStringLiteral("初始页"));
    child<QComboBox>(dialog, "startupLanguageCombo")->setCurrentIndex(2);
    child<QPushButton>(dialog, "wizardFinishButton")->click();
    QCOMPARE(dialog->result(), int(QDialog::Accepted));
    QCOMPARE(dialog->projectPath(), QDir::cleanPath(path));
    QCOMPARE(dialog->draftProject().name, QStringLiteral("中文 工程"));
    QCOMPARE(dialog->draftProject().pageName, QStringLiteral("初始页"));
    QCOMPARE(dialog->draftProject().startupLanguage, QStringLiteral("English"));
    QCOMPARE(dialog->draftProject().activeLanguage, QStringLiteral("中文"));
    QVERIFY(dialog->draftProject().validateModel(&error));
    QVERIFY(!QFile::exists(path));
    delete dialog;
}

/******************************************************************************
 * Function Name   : wizardValidation_data
 * Description     : Cover empty identity, relative destinations, wrong suffixes and missing directories.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::wizardValidation_data() //20261110-7 add
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("pathKind");
    QTest::newRow("empty-name") << QString() << QStringLiteral("valid");
    QTest::newRow("blank-name") << QStringLiteral("   ") << QStringLiteral("valid");
    QTest::newRow("empty-path") << QStringLiteral("name") << QStringLiteral("empty");
    QTest::newRow("relative-path") << QStringLiteral("name") << QStringLiteral("relative");
    QTest::newRow("native-suffix") << QStringLiteral("name") << QStringLiteral("native");
    QTest::newRow("missing-directory") << QStringLiteral("name") << QStringLiteral("missing");
    QTest::newRow("suffix-only") << QStringLiteral("name") << QStringLiteral("suffix-only");
}

/******************************************************************************
 * Function Name   : wizardValidation
 * Description     : Keep invalid input on its wizard step without creating any file.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::wizardValidation() //20261110-7 add
{
    QFETCH(QString, name);
    QFETCH(QString, pathKind);
    QTemporaryDir directory;
    HmiProject project;
    QString path;
    Stage2NewProjectDialog *dialog;

    path = directory.filePath(QStringLiteral("valid.ksp"));
    if (pathKind == QStringLiteral("empty")) { path.clear(); }
    if (pathKind == QStringLiteral("relative")) { path = QStringLiteral("relative.ksp"); }
    if (pathKind == QStringLiteral("native")) { path = directory.filePath(QStringLiteral("original.sm")); }
    if (pathKind == QStringLiteral("missing")) { path = directory.filePath(QStringLiteral("no-such-directory/new.ksp")); }
    if (pathKind == QStringLiteral("suffix-only")) { path = directory.filePath(QStringLiteral(".ksp")); }
    dialog = new Stage2NewProjectDialog(project, path);
    child<QLineEdit>(dialog, "projectNameEdit")->setText(name);
    child<QPushButton>(dialog, "wizardNextButton")->click();
    QCOMPARE(child<QStackedWidget>(dialog, "wizardSteps")->currentIndex(), 0);
    QVERIFY(!child<QLabel>(dialog, "validationErrorLabel")->text().isEmpty());
    QVERIFY(dialog->result() != QDialog::Accepted);
    QCOMPARE(QDir(directory.path()).entryList(QDir::AllEntries | QDir::NoDotAndDotDot).size(), 0);
    delete dialog;
}

/******************************************************************************
 * Function Name   : wizardCannotReplaceExistingFile
 * Description     : Preserve an existing destination even when it has a valid project suffix.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::wizardCannotReplaceExistingFile() //20261110-7 add
{
    QTemporaryDir directory;
    QFile existing(directory.filePath(QStringLiteral("existing.ksp")));
    HmiProject project;
    Stage2NewProjectDialog *dialog;

    QVERIFY(existing.open(QIODevice::WriteOnly));
    QCOMPARE(existing.write("keep", 4), qint64(4));
    existing.close();
    dialog = new Stage2NewProjectDialog(project, existing.fileName());
    child<QLineEdit>(dialog, "projectNameEdit")->setText(QStringLiteral("new"));
    child<QPushButton>(dialog, "wizardNextButton")->click();
    QCOMPARE(child<QStackedWidget>(dialog, "wizardSteps")->currentIndex(), 0);
    QVERIFY(existing.open(QIODevice::ReadOnly));
    QCOMPARE(existing.readAll(), QByteArray("keep"));
    delete dialog;
}

/******************************************************************************
 * Function Name   : wizardNativeDefaultCommunicationAndLogoBoundary
 * Description     : Preserve observed three-step labels and explicitly disable unencoded startup Logo.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::wizardNativeDefaultCommunicationAndLogoBoundary() //20261110-7 add
{
    QTemporaryDir directory;
    HmiProject project;
    Stage2NewProjectDialog dialog(project, directory.filePath(QStringLiteral("wizard.ksp")));
    QSpinBox *time;

    QCOMPARE(child<QStackedWidget>(&dialog, "wizardSteps")->count(), 3);
    QVERIFY(child<QLineEdit>(&dialog, "projectNameEdit")->text().isEmpty());
    QCOMPARE(child<QLineEdit>(&dialog, "connectionManufacturerEdit")->text(), QStringLiteral("MODBUS"));
    QCOMPARE(child<QLineEdit>(&dialog, "connectionSeriesEdit")->text(), QStringLiteral("Modbus_RTU"));
    QVERIFY(child<QLineEdit>(&dialog, "connectionNameEdit")->isReadOnly());
    QVERIFY(!child<QRadioButton>(&dialog, "startupLogoRadio")->isEnabled());
    QVERIFY(!child<QRadioButton>(&dialog, "noStartupLogoRadio")->isEnabled());
    QVERIFY(child<QRadioButton>(&dialog, "noStartupLogoRadio")->isChecked());
    QVERIFY(child<QLabel>(&dialog, "startupLogoBoundaryLabel")->text().contains(QStringLiteral("沿用已验证基线")));
    time = child<QSpinBox>(&dialog, "startupLogoTimeSpin");
    QCOMPARE(time->minimum(), 1);
    QCOMPARE(time->maximum(), 60);
    QCOMPARE(time->value(), 1);
    QVERIFY(!time->isEnabled());
}

/******************************************************************************
 * Function Name   : projectCancelKeepsOriginal
 * Description     : Discard page mutations and edited identity through the real outer Cancel button.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::projectCancelKeepsOriginal() //20261110-7 add
{
    HmiProject project;
    QByteArray before;
    QString error;
    Stage2ProjectDialog dialog(project);

    before = snapshot(project);
    QVERIFY(dialog.addPageDraft(secondPage(), &error));
    QVERIFY(dialog.renamePageDraft(project.activePageId, QStringLiteral("更名草稿"), &error));
    child<QLineEdit>(&dialog, "projectNameEdit")->setText(QStringLiteral("取消这个名称"));
    child<QDialogButtonBox>(&dialog, "buttonBox")->button(QDialogButtonBox::Cancel)->click();
    QCOMPARE(dialog.result(), int(QDialog::Rejected));
    QCOMPARE(snapshot(project), before);
}

/******************************************************************************
 * Function Name   : pageAddRenameSelectAndReopen
 * Description     : Keep two ordinary pages and independent window geometry across save and reopen.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::pageAddRenameSelectAndReopen() //20261110-7 add
{
    HmiProject project;
    HmiProject reopened;
    HmiPage window;
    QString error;
    QTemporaryDir directory;
    Stage2ProjectDialog dialog(project);

    QVERIFY(dialog.addPageDraft(secondPage(), &error));
    window.id = QStringLiteral("window-w");
    window.kind = QStringLiteral("window");
    window.name = QStringLiteral("窗口 W");
    window.width = 320;
    window.height = 180;
    QVERIFY(dialog.addPageDraft(window, &error));
    QVERIFY(dialog.renamePageDraft(QStringLiteral("page-b"), QStringLiteral("第二画面"), &error));
    QVERIFY(dialog.selectPageDraft(QStringLiteral("page-b"), &error));
    acceptSettings(&dialog);
    QCOMPARE(dialog.result(), int(QDialog::Accepted));
    QCOMPARE(dialog.draftProject().activePageId, QStringLiteral("page-b"));
    QCOMPARE(dialog.draftProject().startupPageId, QStringLiteral("page-1"));
    QCOMPARE(dialog.draftProject().pageCount(), 3);
    QVERIFY(dialog.draftProject().save(directory.filePath(QStringLiteral("pages.ksp")), &error));
    QVERIFY(HmiProject::load(directory.filePath(QStringLiteral("pages.ksp")), &reopened, &error));
    QCOMPARE(snapshot(reopened), snapshot(dialog.draftProject()));
    QVERIFY(!reopened.validateSinglePageBackend(&error));
    QVERIFY(reopened.selectPage(QStringLiteral("window-w"), &error));
    QCOMPARE(reopened.pageWidth, 320);
    QCOMPARE(reopened.pageHeight, 180);
}

/******************************************************************************
 * Function Name   : pageErrorsAreAtomic
 * Description     : Reject invalid names, IDs, unsupported roles and nonexistent references atomically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::pageErrorsAreAtomic() //20261110-7 add
{
    HmiProject project;
    HmiPage page;
    QByteArray before;
    QString error;
    Stage2ProjectDialog dialog(project);

    before = snapshot(dialog.draftProject());
    page = secondPage();
    page.id = project.activePageId;
    QVERIFY(!dialog.addPageDraft(page, &error));
    page = secondPage();
    page.id.clear();
    QVERIFY(!dialog.addPageDraft(page, &error));
    page = secondPage();
    page.name = QStringLiteral("  ");
    QVERIFY(!dialog.addPageDraft(page, &error));
    page.name = project.pageName;
    QVERIFY(!dialog.addPageDraft(page, &error));
    page = secondPage();
    page.width = 320;
    QVERIFY(!dialog.addPageDraft(page, &error));
    page = secondPage();
    page.kind = QStringLiteral("keyboard");
    QVERIFY(!dialog.addPageDraft(page, &error));
    QVERIFY(!dialog.renamePageDraft(QStringLiteral("missing"), QStringLiteral("new"), &error));
    QVERIFY(!dialog.selectPageDraft(QStringLiteral("missing"), &error));
    QVERIFY(!dialog.removePageDraft(project.startupPageId, &error));
    QCOMPARE(snapshot(dialog.draftProject()), before);
}

/******************************************************************************
 * Function Name   : copyRemapsObjectsAndGroups
 * Description     : Verify page duplication does not reuse an object or group identity from its source.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::copyRemapsObjectsAndGroups() //20261110-7 add
{
    HmiProject project;
    HmiObject object;
    HmiReference reference;
    QString copiedId;
    QString error;
    Stage2ProjectDialog *dialog;

    object.id = QStringLiteral("object-original");
    object.type = QStringLiteral("text");
    object.name = QStringLiteral("Text");
    object.text = QStringLiteral("original text");
    object.geometry = QRect(10, 10, 80, 30);
    object.group = QStringLiteral("group-original");
    project.objects.append(object);
    reference.id = QStringLiteral("reference-original");
    reference.ownerKind = QStringLiteral("object");
    reference.ownerId = object.id;
    reference.field = QStringLiteral("page-target");
    reference.targetKind = QStringLiteral("page");
    reference.targetId = project.activePageId;
    project.references.append(reference);
    QVERIFY(project.validateModel(&error));
    dialog = new Stage2ProjectDialog(project);
    QVERIFY(dialog->copyPageDraft(project.activePageId, QStringLiteral("副本"), &copiedId, &error));
    QVERIFY(copiedId != project.activePageId);
    QVERIFY(dialog->selectPageDraft(copiedId, &error));
    QCOMPARE(dialog->draftProject().objects.size(), 1);
    QVERIFY(dialog->draftProject().objects.first().id != object.id);
    QVERIFY(dialog->draftProject().objects.first().group != object.group);
    QCOMPARE(dialog->draftProject().objects.first().text, object.text);
    QCOMPARE(dialog->draftProject().references.size(), 2);
    QCOMPARE(dialog->draftProject().references.last().targetId, copiedId);
    QVERIFY(dialog->removePageDraft(copiedId, &error));
    QCOMPARE(dialog->draftProject().pageCount(), 1);
    QCOMPARE(dialog->draftProject().references.size(), 1);
    QCOMPARE(snapshot(project), snapshot(dialog->draftProject()));
    delete dialog;
}

/******************************************************************************
 * Function Name   : referencedPageCannotBeDeleted
 * Description     : Keep an inbound project reference valid when deleting its target is refused.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::referencedPageCannotBeDeleted() //20261110-7 add
{
    HmiProject project;
    HmiReference reference;
    QByteArray before;
    QString error;
    Stage2ProjectDialog *dialog;

    QVERIFY(project.addPage(secondPage(), &error));
    reference.id = QStringLiteral("inbound-reference");
    reference.ownerKind = QStringLiteral("project");
    reference.ownerId = project.projectId;
    reference.field = QStringLiteral("target");
    reference.targetKind = QStringLiteral("page");
    reference.targetId = QStringLiteral("page-b");
    project.references.append(reference);
    QVERIFY(project.validateModel(&error));
    dialog = new Stage2ProjectDialog(project);
    before = snapshot(dialog->draftProject());
    QVERIFY(!dialog->removePageDraft(QStringLiteral("page-b"), &error));
    QVERIFY(error.contains(reference.id));
    QCOMPARE(snapshot(dialog->draftProject()), before);
    delete dialog;
}

/******************************************************************************
 * Function Name   : startupIsIndependentAndRejectsWindow
 * Description     : Separate device startup choices from editor page/language and reject invalid roles.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::startupIsIndependentAndRejectsWindow() //20261110-7 add
{
    HmiProject project;
    HmiPage window;
    QByteArray before;
    QString error;
    Stage2ProjectDialog *dialog;

    project.languages.append(QStringLiteral("English"));
    QVERIFY(project.addPage(secondPage(), &error));
    dialog = new Stage2ProjectDialog(project);
    QVERIFY(dialog->selectPageDraft(QStringLiteral("page-b"), &error));
    QVERIFY(dialog->setStartupDraft(project.activePageId, QStringLiteral("English"), &error));
    QCOMPARE(dialog->draftProject().activePageId, QStringLiteral("page-b"));
    QCOMPARE(dialog->draftProject().activeLanguage, QStringLiteral("中文"));
    QCOMPARE(dialog->draftProject().startupLanguage, QStringLiteral("English"));
    window = secondPage();
    window.id = QStringLiteral("window-w");
    window.name = QStringLiteral("窗口W");
    window.kind = QStringLiteral("window");
    QVERIFY(dialog->addPageDraft(window, &error));
    before = snapshot(dialog->draftProject());
    QVERIFY(!dialog->setStartupDraft(window.id, QStringLiteral("中文"), &error));
    QVERIFY(!dialog->setStartupDraft(QStringLiteral("missing"), QStringLiteral("中文"), &error));
    QVERIFY(!dialog->setStartupDraft(project.activePageId, QStringLiteral("missing"), &error));
    QVERIFY(!dialog->setEditorLanguageDraft(QStringLiteral("missing"), &error));
    QCOMPARE(snapshot(dialog->draftProject()), before);
    delete dialog;
}

/******************************************************************************
 * Function Name   : selectionWidgetsPreserveStartupAcrossCrud
 * Description     : Prevent a list refresh from silently losing earlier startup/page language selections.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::selectionWidgetsPreserveStartupAcrossCrud() //20261110-7 add
{
    HmiProject project;
    HmiPage page;
    QString error;
    Stage2ProjectDialog *dialog;
    QComboBox *startupPage;
    QComboBox *startupLanguage;
    QComboBox *editorPage;

    project.languages.append(QStringLiteral("English"));
    QVERIFY(project.addPage(secondPage(), &error));
    dialog = new Stage2ProjectDialog(project);
    startupPage = child<QComboBox>(dialog, "startupPageCombo");
    startupLanguage = child<QComboBox>(dialog, "startupLanguageCombo");
    editorPage = child<QComboBox>(dialog, "editorPageCombo");
    startupPage->setCurrentIndex(startupPage->findData(QStringLiteral("page-b")));
    startupLanguage->setCurrentIndex(startupLanguage->findData(QStringLiteral("English")));
    editorPage->setCurrentIndex(editorPage->findData(QStringLiteral("page-b")));
    page = secondPage();
    page.id = QStringLiteral("page-c");
    page.name = QStringLiteral("画面C");
    QVERIFY(dialog->addPageDraft(page, &error));
    QCOMPARE(startupPage->currentData().toString(), QStringLiteral("page-b"));
    QCOMPARE(startupLanguage->currentData().toString(), QStringLiteral("English"));
    QCOMPARE(editorPage->currentData().toString(), QStringLiteral("page-b"));
    QVERIFY(dialog->renamePageDraft(page.id, QStringLiteral("C改名"), &error));
    QCOMPARE(dialog->draftProject().startupPageId, QStringLiteral("page-b"));
    QCOMPARE(dialog->draftProject().startupLanguage, QStringLiteral("English"));
    delete dialog;
}

/******************************************************************************
 * Function Name   : acceptedIdentityAndSelections
 * Description     : Commit actual widget edits to a validated returned project while preserving the input.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::acceptedIdentityAndSelections() //20261110-7 add
{
    HmiProject project;
    QByteArray before;
    QString error;
    Stage2ProjectDialog *dialog;
    QComboBox *editorPage;
    QComboBox *editorLanguage;

    project.languages.append(QStringLiteral("English"));
    QVERIFY(project.addPage(secondPage(), &error));
    before = snapshot(project);
    dialog = new Stage2ProjectDialog(project);
    child<QLineEdit>(dialog, "projectNameEdit")->setText(QStringLiteral("新工程名"));
    editorPage = child<QComboBox>(dialog, "editorPageCombo");
    editorLanguage = child<QComboBox>(dialog, "editorLanguageCombo");
    editorPage->setCurrentIndex(editorPage->findData(QStringLiteral("page-b")));
    editorLanguage->setCurrentIndex(editorLanguage->findData(QStringLiteral("English")));
    acceptSettings(dialog);
    QCOMPARE(dialog->result(), int(QDialog::Accepted));
    QCOMPARE(dialog->draftProject().name, QStringLiteral("新工程名"));
    QCOMPARE(dialog->draftProject().activePageId, QStringLiteral("page-b"));
    QCOMPARE(dialog->draftProject().activeLanguage, QStringLiteral("English"));
    QCOMPARE(dialog->draftProject().startupPageId, QStringLiteral("page-1"));
    QCOMPARE(dialog->draftProject().startupLanguage, QString());
    QCOMPARE(snapshot(project), before);
    delete dialog;
}

/******************************************************************************
 * Function Name   : invalidAcceptKeepsDialogOpen
 * Description     : Leave invalid project names uncommitted with an inline error and a reusable dialog.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::invalidAcceptKeepsDialogOpen() //20261110-7 add
{
    HmiProject project;
    QByteArray before;
    Stage2ProjectDialog dialog(project);

    before = snapshot(dialog.draftProject());
    child<QLineEdit>(&dialog, "projectNameEdit")->setText(QStringLiteral("   "));
    acceptSettings(&dialog);
    QVERIFY(dialog.result() != QDialog::Accepted);
    QVERIFY(!child<QLabel>(&dialog, "validationErrorLabel")->text().isEmpty());
    QCOMPARE(snapshot(dialog.draftProject()), before);
    child<QLineEdit>(&dialog, "projectNameEdit")->setText(QStringLiteral("有效名称"));
    acceptSettings(&dialog);
    QCOMPARE(dialog.result(), int(QDialog::Accepted));
}

/******************************************************************************
 * Function Name   : pageButtonsUseSameTransactions
 * Description     : Verify the visible page actions share API reference checks and stable-ID behavior.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::pageButtonsUseSameTransactions() //20261110-7 add
{
    HmiProject project;
    Stage2ProjectDialog dialog(project);
    QLineEdit *name;
    QString addedId;

    name = child<QLineEdit>(&dialog, "pageNameEdit");
    name->setText(QStringLiteral("按钮新画面"));
    child<QPushButton>(&dialog, "addPageButton")->click();
    QCOMPARE(dialog.draftProject().pageCount(), 2);
    addedId = child<QListWidget>(&dialog, "pageList")->currentItem()->data(Qt::UserRole).toString();
    QVERIFY(addedId != project.activePageId);
    name->setText(QStringLiteral("按钮改名"));
    child<QPushButton>(&dialog, "renamePageButton")->click();
    QCOMPARE(dialog.draftProject().allPages().last().name, QStringLiteral("按钮改名"));
    name->setText(QStringLiteral("按钮副本"));
    child<QPushButton>(&dialog, "copyPageButton")->click();
    QCOMPARE(dialog.draftProject().pageCount(), 3);
    child<QPushButton>(&dialog, "removePageButton")->click();
    QCOMPARE(dialog.draftProject().pageCount(), 2);
    QCOMPARE(dialog.draftProject().allPages().last().id, addedId);
}

/******************************************************************************
 * Function Name   : unsupportedRolesRemainPreserved
 * Description     : Preserve existing keyboard/Logo entities while keeping unverified creation unavailable.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::unsupportedRolesRemainPreserved() //20261110-7 add
{
    HmiProject project;
    HmiPage page;
    QString error;
    Stage2ProjectDialog *dialog;

    page = secondPage();
    page.kind = QStringLiteral("keyboard");
    QVERIFY(project.addPage(page, &error));
    dialog = new Stage2ProjectDialog(project);
    QCOMPARE(child<QComboBox>(dialog, "pageKindCombo")->count(), 2);
    QVERIFY(dialog->selectPageDraft(page.id, &error));
    acceptSettings(dialog);
    QCOMPARE(dialog->result(), int(QDialog::Accepted));
    QCOMPARE(dialog->draftProject().activePageKind, QStringLiteral("keyboard"));
    QVERIFY(!dialog->draftProject().validateSinglePageBackend(&error));
    delete dialog;
}

/******************************************************************************
 * Function Name   : existingLongNamesRemainLossless
 * Description     : Do not truncate valid preexisting model names when merely opening and accepting settings.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::existingLongNamesRemainLossless() //20261110-7 add
{
    HmiProject project;
    QString error;
    Stage2ProjectDialog *dialog;

    project.name = QString(40000, QLatin1Char('P'));
    project.pageName = QString(40000, QLatin1Char('S'));
    QVERIFY(project.validateModel(&error));
    dialog = new Stage2ProjectDialog(project);
    QCOMPARE(child<QLineEdit>(dialog, "projectNameEdit")->text(), project.name);
    QCOMPARE(child<QLineEdit>(dialog, "pageNameEdit")->text(), project.pageName);
    acceptSettings(dialog);
    QCOMPARE(dialog->result(), int(QDialog::Accepted));
    QCOMPARE(snapshot(dialog->draftProject()), snapshot(project));
    delete dialog;
}

/******************************************************************************
 * Function Name   : systemDefaultsRemainImplicit
 * Description     : Read observed defaults without creating an active configuration or changing the input.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::systemDefaultsRemainImplicit() //20261110-7 add
{
    HmiProject project;
    HmiProject draft;
    Stage2SystemSettings settings;
    QString error;
    Stage2SystemSettingsDialog dialog(project);

    settings.screensaverMinutes = 40;
    QVERIFY(readStage2SystemSettings(project, &settings, &error));
    QCOMPARE(settings.screensaverMinutes, 1);
    QCOMPARE(settings.flickerTenths, 10);
    QVERIFY(settings.touchSound);
    QCOMPARE(settings.defaultUserLevel, 1);
    QVERIFY(!settings.hexWordAddress);
    QVERIFY(applyStage2SystemSettings(project, settings, &draft, &error));
    QCOMPARE(snapshot(draft), snapshot(project));
    QVERIFY(draft.configurationNodes.isEmpty());
    child<QDialogButtonBox>(&dialog, "buttonBox")->button(QDialogButtonBox::Ok)->click();
    QCOMPARE(dialog.result(), int(QDialog::Accepted));
    QCOMPARE(snapshot(dialog.draftProject()), snapshot(project));
    QVERIFY(!readStage2SystemSettings(project, nullptr, &error));
    QVERIFY(!applyStage2SystemSettings(project, settings, nullptr, &error));
}

/******************************************************************************
 * Function Name   : systemStrictProperties_data
 * Description     : Build schema violations that must not be coerced into plausible system settings.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::systemStrictProperties_data() //20261110-7 add
{
    QTest::addColumn<QString>("field");
    QTest::addColumn<QJsonValue>("replacement");
    QTest::addColumn<bool>("remove");
    QTest::newRow("missing") << QStringLiteral("touchSound") << QJsonValue() << true;
    QTest::newRow("extra") << QStringLiteral("unrecoveredAlarmSound") << QJsonValue(false) << false;
    QTest::newRow("screen-bool") << QStringLiteral("screensaverMinutes") << QJsonValue(true) << false;
    QTest::newRow("screen-string") << QStringLiteral("screensaverMinutes") << QJsonValue(QStringLiteral("1")) << false;
    QTest::newRow("screen-fraction") << QStringLiteral("screensaverMinutes") << QJsonValue(1.25) << false;
    QTest::newRow("screen-below") << QStringLiteral("screensaverMinutes") << QJsonValue(-1) << false;
    QTest::newRow("screen-above") << QStringLiteral("screensaverMinutes") << QJsonValue(61) << false;
    QTest::newRow("flicker-below") << QStringLiteral("flickerTenths") << QJsonValue(0) << false;
    QTest::newRow("flicker-above") << QStringLiteral("flickerTenths") << QJsonValue(51) << false;
    QTest::newRow("flicker-fraction") << QStringLiteral("flickerTenths") << QJsonValue(2.5) << false;
    QTest::newRow("level-below") << QStringLiteral("defaultUserLevel") << QJsonValue(0) << false;
    QTest::newRow("level-above") << QStringLiteral("defaultUserLevel") << QJsonValue(9) << false;
    QTest::newRow("level-fraction") << QStringLiteral("defaultUserLevel") << QJsonValue(1.5) << false;
    QTest::newRow("sound-number") << QStringLiteral("touchSound") << QJsonValue(1) << false;
    QTest::newRow("sound-string") << QStringLiteral("touchSound") << QJsonValue(QStringLiteral("true")) << false;
    QTest::newRow("sound-null") << QStringLiteral("touchSound") << QJsonValue(QJsonValue::Null) << false;
    QTest::newRow("hex-number") << QStringLiteral("hexWordAddress") << QJsonValue(0) << false;
    QTest::newRow("hex-string") << QStringLiteral("hexWordAddress") << QJsonValue(QStringLiteral("false")) << false;
}

/******************************************************************************
 * Function Name   : systemStrictProperties
 * Description     : Reject malformed properties in both the parser-facing validator and the model reader.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::systemStrictProperties() //20261110-7 add
{
    QFETCH(QString, field);
    QFETCH(QJsonValue, replacement);
    QFETCH(bool, remove);
    Stage2SystemSettings settings;
    QJsonObject properties;
    HmiProject project;
    HmiConfigurationNode node;
    QString error;

    properties = settings.toJson();
    if (remove) { properties.remove(field); }
    else { properties.insert(field, replacement); }
    QVERIFY(!validateStage2SystemSettingsProperties(properties, &error));
    QVERIFY(!error.isEmpty());
    node.id = QStringLiteral("malformed-system");
    node.kind = QStringLiteral("system");
    node.name = QStringLiteral("System");
    node.implemented = true;
    node.active = true;
    node.properties = properties;
    project.configurationNodes.append(node);
    settings.defaultUserLevel = 7;
    QVERIFY(!readStage2SystemSettings(project, &settings, &error));
    QCOMPARE(settings.defaultUserLevel, 7);
    QVERIFY(!project.validateModel(&error));
}

/******************************************************************************
 * Function Name   : systemBoundaryValues
 * Description     : Accept observed integer endpoints without inventing timing or runtime behavior for zero.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::systemBoundaryValues() //20261110-7 add
{
    Stage2SystemSettings settings;
    QString error;

    settings.screensaverMinutes = 0;
    settings.flickerTenths = 1;
    settings.defaultUserLevel = 1;
    QVERIFY(validateStage2SystemSettingsProperties(settings.toJson(), &error));
    settings.screensaverMinutes = 60;
    settings.flickerTenths = 50;
    settings.defaultUserLevel = 8;
    settings.touchSound = false;
    settings.hexWordAddress = true;
    QVERIFY(validateStage2SystemSettingsProperties(settings.toJson(), &error));
}

/******************************************************************************
 * Function Name   : systemUpdatePreservesOtherNodesAndExtensions
 * Description     : Update one stable system record without losing unrelated nodes or inactive extension data.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::systemUpdatePreservesOtherNodesAndExtensions() //20261110-7 add
{
    HmiProject source;
    HmiProject first;
    HmiProject second;
    HmiProject reopened;
    HmiConfigurationNode alarm;
    Stage2SystemSettings settings;
    Stage2SystemSettings restored;
    QJsonObject metadata;
    QJsonObject envelope;
    QString id;
    QString error;
    QTemporaryDir directory;

    envelope = {{QStringLiteral("version"), 1}, {QStringLiteral("inactive"), true},
                {QStringLiteral("data"), QJsonObject{{QStringLiteral("future-value"), 17}}}};
    metadata.insert(QStringLiteral("editor.future"), envelope);
    alarm.id = QStringLiteral("unrelated-alarm");
    alarm.kind = QStringLiteral("alarm");
    alarm.name = QStringLiteral("Alarm placeholder");
    alarm.extensions = metadata;
    source.configurationNodes.append(alarm);
    source.extensions = metadata;
    settings.screensaverMinutes = 3;
    QVERIFY2(applyStage2SystemSettings(source, settings, &first, &error), qPrintable(error));
    QCOMPARE(first.configurationNodes.size(), 2);
    QCOMPARE(first.configurationNodes.first().toJson(), alarm.toJson());
    QCOMPARE(first.extensions, metadata);
    id = first.configurationNodes.last().id;
    first.configurationNodes.last().extensions = metadata;
    settings.flickerTenths = 11;
    settings.defaultUserLevel = 4;
    QVERIFY2(applyStage2SystemSettings(first, settings, &second, &error), qPrintable(error));
    QCOMPARE(second.configurationNodes.size(), 2);
    QCOMPARE(second.configurationNodes.last().id, id);
    QCOMPARE(second.configurationNodes.last().extensions, metadata);
    QVERIFY(readStage2SystemSettings(second, &restored, &error));
    QCOMPARE(restored.toJson(), settings.toJson());
    QVERIFY(second.save(directory.filePath(QStringLiteral("system.ksp")), &error));
    QVERIFY(HmiProject::load(directory.filePath(QStringLiteral("system.ksp")), &reopened, &error));
    QCOMPARE(snapshot(reopened), snapshot(second));
    QVERIFY(!second.validateSinglePageBackend(&error));
    QCOMPARE(source.configurationNodes.size(), 1);
}

/******************************************************************************
 * Function Name   : systemConflictsAreAtomic
 * Description     : Refuse placeholders, duplicate nodes, page-owned records and invalid values without output mutation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::systemConflictsAreAtomic() //20261110-7 add
{
    HmiProject source;
    HmiProject output;
    HmiProject duplicate;
    Stage2SystemSettings settings;
    HmiConfigurationNode node;
    QByteArray before;
    QString error;
    Stage2SystemSettingsDialog *dialog;

    output.name = QStringLiteral("preserved output");
    before = snapshot(output);
    node.id = QStringLiteral("system-placeholder");
    node.kind = QStringLiteral("system");
    node.name = QStringLiteral("Unimplemented system");
    source.configurationNodes.append(node);
    QVERIFY(!applyStage2SystemSettings(source, settings, &output, &error));
    QVERIFY(error.contains(QStringLiteral("占位")));
    QCOMPARE(snapshot(output), before);
    dialog = new Stage2SystemSettingsDialog(source);
    QVERIFY(!child<QDialogButtonBox>(dialog, "buttonBox")->button(QDialogButtonBox::Ok)->isEnabled());
    delete dialog;
    node.implemented = true;
    node.active = true;
    node.properties = settings.toJson();
    node.ownerPageId = source.activePageId;
    source.configurationNodes[0] = node;
    QVERIFY(!applyStage2SystemSettings(source, settings, &output, &error));
    QCOMPARE(snapshot(output), before);
    node.ownerPageId.clear();
    source.configurationNodes[0] = node;
    node.id = QStringLiteral("second-system");
    source.configurationNodes.append(node);
    QVERIFY(!readStage2SystemSettings(source, &settings, &error));
    QVERIFY(error.contains(QStringLiteral("多个")));
    QVERIFY(!applyStage2SystemSettings(source, settings, &output, &error));
    QCOMPARE(snapshot(output), before);
    settings.screensaverMinutes = 61;
    QVERIFY(!applyStage2SystemSettings(HmiProject(), settings, &output, &error));
    QCOMPARE(snapshot(output), before);
}

/******************************************************************************
 * Function Name   : systemDialogCancelAndReopen
 * Description     : Test real widget ranges, outer cancel and complete five-value persistence through reopen.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::systemDialogCancelAndReopen() //20261110-7 add
{
    HmiProject project;
    HmiProject reopened;
    Stage2SystemSettings settings;
    Stage2SystemSettingsDialog canceled(project);
    Stage2SystemSettingsDialog accepted(project);
    Stage2SystemSettingsDialog *again;
    QTemporaryDir directory;
    QString error;

    child<QSpinBox>(&canceled, "screensaverMinutesSpin")->setValue(12);
    child<QDialogButtonBox>(&canceled, "buttonBox")->button(QDialogButtonBox::Cancel)->click();
    QCOMPARE(canceled.result(), int(QDialog::Rejected));
    QCOMPARE(snapshot(canceled.draftProject()), snapshot(project));
    QCOMPARE(child<QSpinBox>(&accepted, "screensaverMinutesSpin")->minimum(), 0);
    QCOMPARE(child<QSpinBox>(&accepted, "screensaverMinutesSpin")->maximum(), 60);
    QCOMPARE(child<QSpinBox>(&accepted, "flickerTenthsSpin")->minimum(), 1);
    QCOMPARE(child<QSpinBox>(&accepted, "flickerTenthsSpin")->maximum(), 50);
    QCOMPARE(child<QSpinBox>(&accepted, "defaultUserLevelSpin")->minimum(), 1);
    QCOMPARE(child<QSpinBox>(&accepted, "defaultUserLevelSpin")->maximum(), 8);
    child<QSpinBox>(&accepted, "screensaverMinutesSpin")->setValue(60);
    child<QSpinBox>(&accepted, "flickerTenthsSpin")->setValue(50);
    child<QCheckBox>(&accepted, "touchSoundCheck")->setChecked(false);
    child<QSpinBox>(&accepted, "defaultUserLevelSpin")->setValue(8);
    child<QCheckBox>(&accepted, "hexWordAddressCheck")->setChecked(true);
    child<QDialogButtonBox>(&accepted, "buttonBox")->button(QDialogButtonBox::Ok)->click();
    QCOMPARE(accepted.result(), int(QDialog::Accepted));
    QVERIFY(readStage2SystemSettings(accepted.draftProject(), &settings, &error));
    QCOMPARE(settings.screensaverMinutes, 60);
    QCOMPARE(settings.flickerTenths, 50);
    QVERIFY(!settings.touchSound);
    QCOMPARE(settings.defaultUserLevel, 8);
    QVERIFY(settings.hexWordAddress);
    QVERIFY(accepted.draftProject().save(directory.filePath(QStringLiteral("settings.ksp")), &error));
    QVERIFY(HmiProject::load(directory.filePath(QStringLiteral("settings.ksp")), &reopened, &error));
    again = new Stage2SystemSettingsDialog(reopened);
    QCOMPARE(child<QSpinBox>(again, "screensaverMinutesSpin")->value(), 60);
    QVERIFY(!child<QCheckBox>(again, "touchSoundCheck")->isChecked());
    QVERIFY(child<QCheckBox>(again, "hexWordAddressCheck")->isChecked());
    delete again;
}

/******************************************************************************
 * Function Name   : systemNativeSnapshotIsReadOnly
 * Description     : Refuse value edits on native snapshots even if a disabled widget is changed programmatically.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialogsTests::systemNativeSnapshotIsReadOnly() //20261110-7 add
{
    HmiProject project;
    HmiProject output;
    Stage2SystemSettings settings;
    QFile source(QFINDTESTDATA("../samples/native-device-test/CodexProbeTest.sm"));
    QByteArray before;
    QString error;
    Stage2SystemSettingsDialog *dialog;

    QVERIFY(source.open(QIODevice::ReadOnly));
    project.nativeSourceSnapshot = source.readAll();
    QVERIFY(project.validateModel(&error));
    output.name = QStringLiteral("untouched output");
    before = snapshot(output);
    settings.touchSound = false;
    QVERIFY(!applyStage2SystemSettings(project, settings, &output, &error));
    QVERIFY(error.contains(QStringLiteral("只读")));
    QCOMPARE(snapshot(output), before);
    dialog = new Stage2SystemSettingsDialog(project);
    QVERIFY(!child<QSpinBox>(dialog, "screensaverMinutesSpin")->isEnabled());
    QVERIFY(!child<QCheckBox>(dialog, "touchSoundCheck")->isEnabled());
    QVERIFY(!child<QLabel>(dialog, "systemSnapshotReadOnlyLabel")->text().isEmpty());
    child<QSpinBox>(dialog, "screensaverMinutesSpin")->setValue(2);
    child<QDialogButtonBox>(dialog, "buttonBox")->button(QDialogButtonBox::Ok)->click();
    QVERIFY(dialog->result() != QDialog::Accepted);
    QCOMPARE(snapshot(dialog->draftProject()), snapshot(project));
    child<QSpinBox>(dialog, "screensaverMinutesSpin")->setValue(1);
    child<QDialogButtonBox>(dialog, "buttonBox")->button(QDialogButtonBox::Ok)->click();
    QCOMPARE(dialog->result(), int(QDialog::Accepted));
    QCOMPARE(snapshot(dialog->draftProject()), snapshot(project));
    delete dialog;
}

QTEST_MAIN(Stage2ProjectDialogsTests)
#include "stage2_project_dialogs_tests.moc"
