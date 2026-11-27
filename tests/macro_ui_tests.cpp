#include "main_window.h"
#include "macro_manager_dialog.h"
#include "macro_workspace.h"
#include <QAction>
#include <QApplication>
#include <QCheckBox> //20261127-1 add
#include <QCryptographicHash> //20261127-1 add
#include <QDir>
#include <QEventLoop> //20261127-1 add
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QInputDialog>
#include <QListWidget>
#include <QLineEdit> //20261127-1 add
#include <QMessageBox> //20261127-1 add
#include <QPlainTextEdit>
#include <QPushButton> //20261127-1 add
#include <QSettings>
#include <QTemporaryDir>
#include <QThread>
#include <QTextCursor>
#include <QTabWidget>
#include <QTimer>
#include <QtTest>
#ifdef Q_OS_WIN
#include <qt_windows.h> //20261127-1 add: Exercise the system close command shared by Alt+F4 and the title-bar X.
#endif

class MacroUiTests : public QObject //20261114-7 add
{
    Q_OBJECT
private:
    QTemporaryDir m_settings;
private slots:
    void initTestCase();
    void cancelledDraftAndAcceptedSource();
    void switchingSourcesAndInvalidRecovery();
    void renameReferencesAndRefuseDelete();
    void utf8FilesAndAtomicFailure();
    void acceptedWorkspaceIsOneEditorTransaction();
    void cancelledWorkspaceKeepsDirtyAndHistory();
    void missingSdkLeavesEditableSource();
    void responsiveCancellationAndOwnerDeletion();
    void nativeLibraryExamples_data(); //20261114-7 add
    void nativeLibraryExamples();
    void insertionSelectionUndoAndBudget();
    void deletionDuringNestedNameInput();
    void findAndReplaceFromToolbar(); //20261127-1 add
    void closingModifiedMacro_data(); //20261127-1 add
    void closingModifiedMacro();
};

/******************************************************************************
 * Function Name   : macroUiProject
 * Description     : Build two ordinary source records without any SDK or device dependency.
 * Input Parameters: None
 * Return Value    : Valid detached model.
 * Date            : 2026.11.14
 ******************************************************************************/
static HmiProject macroUiProject() //20261114-7 add
{
    HmiProject project;
    QString error;

    MacroWorkspace::create(&project, QStringLiteral("Alpha"), MacroWorkspace::defaultSource(), &error);
    MacroWorkspace::create(&project, QStringLiteral("Zulu"), QStringLiteral("void Macro_Entry(void) { }\n"), &error);
    return project;
}

/******************************************************************************
 * Function Name   : macroUiFixtureSdk
 * Description     : Copy a declared synthetic process fixture only for UI lifecycle tests.
 * Input Parameters: directory, fixture
 * Return Value    : True after a private synthetic SDK tree is ready.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool macroUiFixtureSdk(const QString &directory, const QString &fixture) //20261114-7 add
{
    QString name;
    QStringList tools = {QStringLiteral("armcc.exe"), QStringLiteral("armlink.exe"), QStringLiteral("fromelf.exe")};
    QStringList data = {QStringLiteral("armcc.dll"), QStringLiteral("armlink.dll"), QStringLiteral("fromelf.dll"),
        QStringLiteral("Include/macrolib.h"), QStringLiteral("Include/glib.h"), QStringLiteral("Include/math.h"),
        QStringLiteral("Include/stdio.h"), QStringLiteral("Include/stdlib.h"), QStringLiteral("script_amzlib.a"),
        QStringLiteral("guilib70_amz.a"), QStringLiteral("licenses/LICENSE.DAT"), QStringLiteral("Lib/armlib/c_a__un.l")};
    QFile file;

    if (!QDir().mkpath(directory)) { return false; }
    Q_FOREACH (name, tools) { if (!QFile::copy(fixture, QDir(directory).filePath(name))) { return false; } }
    Q_FOREACH (name, data)
    {
        file.setFileName(QDir(directory).filePath(name));
        if (!QDir().mkpath(QFileInfo(file).absolutePath()) || !file.open(QIODevice::WriteOnly) || file.write("SYNTHETIC; NOT ADS") < 0) { return false; }
        file.close();
    }
    return true;
}

/******************************************************************************
 * Function Name   : MacroUiTests::initTestCase
 * Description     : Isolate every editor preference and recent-file operation in the task's temporary root.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroUiTests::initTestCase() //20261114-7 add
{
    QVERIFY(m_settings.isValid());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settings.path());
}

/******************************************************************************
 * Function Name   : MacroUiTests::cancelledDraftAndAcceptedSource
 * Description     : Prove local edits remain detached on cancel and publish valid text only on acceptance.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroUiTests::cancelledDraftAndAcceptedSource() //20261114-7 add
{
    HmiProject project = macroUiProject();
    QJsonObject original = project.toJson();
    MacroManagerDialog cancelled(project);
    MacroManagerDialog accepted(project);
    QString code = QStringLiteral("void Macro_Entry(void) { /* changed */ }\n");
    QString captureRoot = qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR");

    if (!captureRoot.isEmpty()) //20261114-7 add: Capture only this owned test widget on the launcher's private desktop.
    {
        accepted.show();
        QApplication::processEvents();
        QVERIFY(accepted.grab().save(QDir(captureRoot).filePath(QStringLiteral("macro-editor.png"))));
        accepted.findChild<QTabWidget *>(QStringLiteral("MacroLibraryTabs"))->setCurrentIndex(1);
        QApplication::processEvents();
        QVERIFY(accepted.grab().save(QDir(captureRoot).filePath(QStringLiteral("macro-library.png"))));
        accepted.hide();
    }

    cancelled.findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"))->setPlainText(code);
    cancelled.reject();
    QCOMPARE(project.toJson(), original);
    QCOMPARE(cancelled.result(), int(QDialog::Rejected));
    accepted.findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"))->setPlainText(code);
    accepted.accept();
    QCOMPARE(accepted.result(), int(QDialog::Accepted));
    QCOMPARE(accepted.draftProject().macros.value(QStringLiteral("Alpha")), code);
    QCOMPARE(accepted.draftProject().macroIds.value(QStringLiteral("Alpha")), project.macroIds.value(QStringLiteral("Alpha")));
}

/******************************************************************************
 * Function Name   : MacroUiTests::switchingSourcesAndInvalidRecovery
 * Description     : Keep each source under its own identity and preserve invalid input for correction during selection changes.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroUiTests::switchingSourcesAndInvalidRecovery() //20261114-7 add
{
    MacroManagerDialog dialog(macroUiProject());
    QPlainTextEdit *source = dialog.findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"));
    QListWidget *list = dialog.findChild<QListWidget *>(QStringLiteral("MacroList"));
    QString error;
    QString invalid(1024 * 1024 + 1, QLatin1Char('x'));

    source->setPlainText(QStringLiteral("alpha text"));
    QVERIFY(dialog.selectMacro(QStringLiteral("Zulu"), &error));
    QCOMPARE(dialog.draftProject().macros.value(QStringLiteral("Alpha")), QStringLiteral("alpha text"));
    QCOMPARE(source->toPlainText(), QStringLiteral("void Macro_Entry(void) { }\n"));
    source->setPlainText(invalid);
    list->setCurrentRow(0);
    QCOMPARE(list->currentItem()->text(), QStringLiteral("Zulu"));
    QCOMPARE(source->toPlainText(), invalid);
    source->setPlainText(QStringLiteral("recovered"));
    QVERIFY(dialog.selectMacro(QStringLiteral("Alpha"), &error));
    QCOMPARE(dialog.draftProject().macros.value(QStringLiteral("Zulu")), QStringLiteral("recovered"));
}

/******************************************************************************
 * Function Name   : MacroUiTests::renameReferencesAndRefuseDelete
 * Description     : Exercise actual workspace rename and deletion feedback against an active button reference.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroUiTests::renameReferencesAndRefuseDelete() //20261114-7 add
{
    HmiProject project = macroUiProject();
    HmiObject button;
    QString error;

    button.id = QStringLiteral("bound-button"); button.name = button.id; button.type = QStringLiteral("switch");
    button.address = QStringLiteral("LB0"); button.geometry = QRect(10, 10, 100, 50); button.hasBitButtonProperties = true;
    button.bitButton.macroEnabled = true; button.bitButton.macroName = QStringLiteral("Alpha");
    project.objects.append(button);
    QVERIFY2(project.validateModel(&error), qPrintable(error));
    MacroManagerDialog dialog(project);
    QVERIFY2(dialog.renameSource(QStringLiteral("Alpha"), QStringLiteral("Renamed"), &error), qPrintable(error));
    QCOMPARE(dialog.draftProject().objects.first().bitButton.macroName, QStringLiteral("Renamed"));
    QVERIFY(!dialog.removeSource(QStringLiteral("Renamed"), &error));
    QVERIFY(dialog.draftProject().macros.contains(QStringLiteral("Renamed")));
    QCOMPARE(project.objects.first().bitButton.macroName, QStringLiteral("Alpha"));
}

/******************************************************************************
 * Function Name   : MacroUiTests::utf8FilesAndAtomicFailure
 * Description     : Observe strict source import, successful UTF-8 output and failed writes preserving existing files.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroUiTests::utf8FilesAndAtomicFailure() //20261114-7 add
{
    MacroManagerDialog dialog(macroUiProject());
    QTemporaryDir directory;
    QFile file;
    QString error;
    QJsonObject before;

    QVERIFY(directory.isValid());
    QVERIFY(dialog.exportSourceFile(directory.filePath(QStringLiteral("source.c")), QStringLiteral("Alpha"), &error));
    QVERIFY(dialog.importSourceFile(directory.filePath(QStringLiteral("source.c")), QStringLiteral("Imported"), false, &error));
    QCOMPARE(dialog.draftProject().macros.value(QStringLiteral("Imported")), MacroWorkspace::defaultSource());
    before = dialog.draftProject().toJson();
    file.setFileName(directory.filePath(QStringLiteral("bad.c"))); QVERIFY(file.open(QIODevice::WriteOnly)); file.write(QByteArray::fromHex("fffe00")); file.close();
    QVERIFY(!dialog.importSourceFile(file.fileName(), QStringLiteral("Bad"), false, &error));
    QCOMPARE(dialog.draftProject().toJson(), before);
    QVERIFY(!dialog.exportSourceFile(directory.filePath(QStringLiteral("fake.sm")), QStringLiteral("Alpha"), &error));
    QVERIFY(!QFileInfo::exists(directory.filePath(QStringLiteral("fake.sm"))));
    QVERIFY(QDir().mkdir(directory.filePath(QStringLiteral("folder.c"))));
    QVERIFY(!dialog.exportSourceFile(directory.filePath(QStringLiteral("folder.c")), QStringLiteral("Alpha"), &error));
    file.setFileName(directory.filePath(QStringLiteral("source.c"))); QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8(file.readAll()), MacroWorkspace::defaultSource());
}

/******************************************************************************
 * Function Name   : MacroUiTests::acceptedWorkspaceIsOneEditorTransaction
 * Description     : Commit macro edits through the real shell action and verify a single whole-project undo entry.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroUiTests::acceptedWorkspaceIsOneEditorTransaction() //20261114-7 add
{
    MainWindow window;
    QString code = QStringLiteral("void Macro_Entry(void) { /* accepted */ }\n");
    int previous;
    bool accepted = false;

    window.setProject(macroUiProject(), QString()); previous = window.m_projectHistoryIndex;
    QTimer::singleShot(0, &window, [&accepted, code]() {
        MacroManagerDialog *dialog = qobject_cast<MacroManagerDialog *>(QApplication::activeModalWidget());

        if (dialog != nullptr) { dialog->findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"))->setPlainText(code); dialog->accept(); accepted = true; }
    });
    window.m_commands.value(QStringLiteral("action_script"))->trigger();
    QVERIFY(accepted);
    QCOMPARE(window.currentEditorProject().macros.value(QStringLiteral("Alpha")), code);
    QCOMPARE(window.m_projectHistoryIndex, previous + 1);
    QVERIFY(window.m_dirty);
}

/******************************************************************************
 * Function Name   : MacroUiTests::cancelledWorkspaceKeepsDirtyAndHistory
 * Description     : Reject the actual shell editor after changing text without mutating editor content or history.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroUiTests::cancelledWorkspaceKeepsDirtyAndHistory() //20261114-7 add
{
    MainWindow window;
    QJsonObject before;
    int history;

    window.setProject(macroUiProject(), QString()); before = window.currentEditorProject().toJson(); history = window.m_projectHistoryIndex;
    QTimer::singleShot(0, &window, []() {
        MacroManagerDialog *dialog = qobject_cast<MacroManagerDialog *>(QApplication::activeModalWidget());

        if (dialog != nullptr) { dialog->findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"))->setPlainText(QStringLiteral("discard me")); dialog->reject(); }
    });
    window.m_commands.value(QStringLiteral("action_script"))->trigger();
    QCOMPARE(window.currentEditorProject().toJson(), before);
    QCOMPARE(window.m_projectHistoryIndex, history);
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : MacroUiTests::missingSdkLeavesEditableSource
 * Description     : Keep editable source and the live project intact when the background compiler reports a missing SDK.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroUiTests::missingSdkLeavesEditableSource() //20261114-7 add
{
    MacroManagerDialog dialog(macroUiProject());
    QTemporaryDir directory;
    DeviceMacroCompileOptions options;
    QJsonObject before = dialog.draftProject().toJson();

    QVERIFY(directory.isValid()); options.temporaryRoot = directory.path();
    dialog.startCompilation(directory.filePath(QStringLiteral("missing-sdk")), options);
    QVERIFY(dialog.compiling());
    QTRY_VERIFY_WITH_TIMEOUT(!dialog.compiling(), 10000);
    QVERIFY(!dialog.compilationReport().value(QStringLiteral("ok")).toBool());
    QVERIFY(dialog.findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"))->isEnabled());
    QCOMPARE(dialog.draftProject().toJson(), before);
}

/******************************************************************************
 * Function Name   : MacroUiTests::responsiveCancellationAndOwnerDeletion
 * Description     : Cancel a synthetic owned process through a responsive UI and safely destroy a second live owner.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroUiTests::responsiveCancellationAndOwnerDeletion() //20261114-7 add
{
    QString fixture = qEnvironmentVariable("KINSEAL_MACRO_PROCESS_FIXTURE");
    QTemporaryDir directory;
    DeviceMacroCompileOptions options;
    QPointer<MacroManagerDialog> dialog;
    QPointer<QThread> thread;
    QByteArray oldMode = qgetenv("KINSEAL_MACRO_FIXTURE_MODE");
    QByteArray oldStage = qgetenv("KINSEAL_MACRO_FIXTURE_STAGE");
    QTimer pulse;
    int pulses = 0;

    QVERIFY2(!fixture.isEmpty(), "Explicit synthetic process fixture must be configured for this UI lifecycle case.");
    QVERIFY(directory.isValid());
    QVERIFY(macroUiFixtureSdk(directory.filePath(QStringLiteral("sdk")), fixture));
    options.temporaryRoot = directory.path(); options.processTimeoutMs = 5000; options.totalTimeoutMs = 15000;
    qputenv("KINSEAL_MACRO_FIXTURE_MODE", QByteArray("sleep")); qputenv("KINSEAL_MACRO_FIXTURE_STAGE", QByteArray("armcc"));
    pulse.setInterval(10); connect(&pulse, &QTimer::timeout, this, [&pulses]() { ++pulses; }); pulse.start();
    dialog = new MacroManagerDialog(macroUiProject());
    dialog->startCompilation(directory.filePath(QStringLiteral("sdk")), options);
    QTest::qWait(150); QVERIFY(pulses > 1); QVERIFY(dialog->compiling());
    dialog->reject();
    QTRY_VERIFY_WITH_TIMEOUT(!dialog->compiling(), 10000);
    QVERIFY(dialog->compilationReport().value(QStringLiteral("cancelled")).toBool());
    delete dialog.data();
    dialog = new MacroManagerDialog(macroUiProject()); dialog->startCompilation(directory.filePath(QStringLiteral("sdk")), options);
    thread = dialog->m_thread;
    QTest::qWait(100); delete dialog.data(); QVERIFY(dialog.isNull());
    QTRY_VERIFY_WITH_TIMEOUT(thread.isNull(), 10000);
    if (oldMode.isNull()) { qunsetenv("KINSEAL_MACRO_FIXTURE_MODE"); } else { qputenv("KINSEAL_MACRO_FIXTURE_MODE", oldMode); }
    if (oldStage.isNull()) { qunsetenv("KINSEAL_MACRO_FIXTURE_STAGE"); } else { qputenv("KINSEAL_MACRO_FIXTURE_STAGE", oldStage); }
}


/******************************************************************************
 * Function Name   : MacroUiTests::nativeLibraryExamples_data
 * Description     : Enumerate all observed native snippets without treating their comments as execution oracles.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroUiTests::nativeLibraryExamples_data() //20261114-7 add
{
    QFile file(QStringLiteral(":/macro-library/examples.json"));
    QJsonArray examples;

    QTest::addColumn<QString>("name"); QTest::addColumn<QString>("fragment");
    QVERIFY(file.open(QIODevice::ReadOnly));
    examples = QJsonDocument::fromJson(file.readAll()).object().value(QStringLiteral("examples")).toArray();
    QCOMPARE(examples.size(), 36);
    for (const QJsonValue &entry : examples)
    {
        QJsonObject row = entry.toObject();
        QTest::newRow(qPrintable(row.value(QStringLiteral("name")).toString())) << row.value(QStringLiteral("name")).toString() << row.value(QStringLiteral("text")).toString();
    }
}

/******************************************************************************
 * Function Name   : MacroUiTests::nativeLibraryExamples
 * Description     : Match native cursor-zero/no-selection insertion exactly, including missing trailing newlines.
 * Input Parameters: name, fragment
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroUiTests::nativeLibraryExamples() //20261114-7 add
{
    QFETCH(QString, name);
    QFETCH(QString, fragment);
    MacroManagerDialog dialog(macroUiProject());
    QPlainTextEdit *source = dialog.findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"));
    QString error;
    QString seed = QStringLiteral("/*KS-STAGE8-EXAMPLE-SENTINEL*/\n");

    source->setPlainText(seed); source->moveCursor(QTextCursor::Start);
    QVERIFY2(dialog.insertLibraryExample(name, &error), qPrintable(error));
    QCOMPARE(source->toPlainText(), fragment + seed);
    source->undo(); QCOMPARE(source->toPlainText(), seed);
    source->redo(); QCOMPARE(source->toPlainText(), fragment + seed);
}

/******************************************************************************
 * Function Name   : MacroUiTests::insertionSelectionUndoAndBudget
 * Description     : Test the declared Lite selection rule and reject oversized insertion without changing text.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroUiTests::insertionSelectionUndoAndBudget() //20261114-7 add
{
    MacroManagerDialog dialog(macroUiProject());
    QPlainTextEdit *source = dialog.findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"));
    QTextCursor cursor;
    QString error;
    QString before;

    source->setPlainText(QStringLiteral("beforeSELECTafter"));
    cursor = source->textCursor(); cursor.setPosition(6); cursor.setPosition(12, QTextCursor::KeepAnchor); source->setTextCursor(cursor);
    QVERIFY(dialog.insertLibraryExample(QStringLiteral("ADDSUM"), &error));
    QVERIFY(source->toPlainText().startsWith(QStringLiteral("beforechar data[5]")));
    QVERIFY(source->toPlainText().endsWith(QStringLiteral("after")));
    source->undo(); QCOMPARE(source->toPlainText(), QStringLiteral("beforeSELECTafter"));
    before = QString(1024 * 1024, QLatin1Char('x')); source->setPlainText(before); source->moveCursor(QTextCursor::End);
    QVERIFY(!dialog.insertLibraryExample(QStringLiteral("ADDSUM"), &error)); QCOMPARE(source->toPlainText(), before);
}

/******************************************************************************
 * Function Name   : MacroUiTests::findAndReplaceFromToolbar
 * Description     : Verify matching options, direction, wrapping, replacement and undo in an actual editor draft.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.27
 ******************************************************************************/
void MacroUiTests::findAndReplaceFromToolbar() //20261127-1 add
{
    MacroManagerDialog editor(macroUiProject());
    QPlainTextEdit *source = editor.findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"));
    QString initial = QStringLiteral("a A aa A2 a");
    bool correct = false;

    // Original V5.1 template at file offset 0xACBB30, normalized to Qt editor newlines.
    QCOMPARE(QCryptographicHash::hash(MacroWorkspace::defaultSource().toUtf8(), QCryptographicHash::Sha256).toHex(),
             QByteArray("d3b94376d6ce4d9c1e6e5686e67191a8495751d6edf55fd1b4a07ef964d34571"));
    source->setPlainText(initial);
    source->moveCursor(QTextCursor::Start);
    QTimer::singleShot(0, &editor, [&]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QList<QLineEdit *> fields;
        QList<QCheckBox *> options;
        QList<QPushButton *> actions;
        bool matched;

        if (!dialog) { return; }
        fields = dialog->findChildren<QLineEdit *>();
        options = dialog->findChildren<QCheckBox *>();
        actions = dialog->findChildren<QPushButton *>();
        if (fields.size() != 2 || options.size() != 4 || actions.size() != 3)
        {
            dialog->reject();
            return;
        }
        matched = !actions[0]->isEnabled() && actions[1]->isEnabled();
        fields[0]->setText(QStringLiteral("a"));
        options[0]->setChecked(true);                   // Match case
        options[1]->setChecked(true);                   // Whole words
        actions[0]->click();
        matched = matched && source->textCursor().selectionStart() == 0;
        actions[0]->click();
        matched = matched && source->textCursor().selectionStart() == 10;
        options[2]->setChecked(true);                   // Backwards
        actions[0]->click();
        matched = matched && source->textCursor().selectionStart() == 0;
        actions[0]->click();
        matched = matched && source->textCursor().selectionStart() == 10;
        options[3]->setChecked(true);                   // Regular expression
        fields[0]->setText(QStringLiteral("A[0-9]"));
        actions[0]->click();
        matched = matched && source->textCursor().selectedText() == QStringLiteral("A2");
        fields[1]->setText(QStringLiteral("found"));
        actions[1]->click();
        correct = matched && source->toPlainText() == QStringLiteral("a A aa found a");
        actions[2]->click();
    });
    editor.findChild<QPushButton *>(QStringLiteral("MacroFind"))->click();
    QVERIFY(correct);
    source->undo();
    QCOMPARE(source->toPlainText(), initial);
}

/******************************************************************************
 * Function Name   : MacroUiTests::deletionDuringNestedNameInput
 * Description     : Destroy the owner of a nested macro name prompt without returning into stale state.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MacroUiTests::deletionDuringNestedNameInput() //20261114-7 add
{
    QPointer<MacroManagerDialog> dialog = new MacroManagerDialog(macroUiProject());
    bool deleted = false;

    QTimer::singleShot(0, [&dialog, &deleted]() {
        QDialog *input = qobject_cast<QDialog *>(QApplication::activeModalWidget()); //20261127-1 change

        if (input != nullptr && input->objectName() == QStringLiteral("MacroNameDialog"))
        {
            delete dialog.data();
            deleted = true;
        }
    });
    dialog->promptNew();
    QVERIFY(deleted); QVERIFY(dialog.isNull());
}

/******************************************************************************
 * Function Name   : MacroUiTests::closingModifiedMacro_data
 * Description     : Cover save, discard and return to editing for one source and for multiple dirty sources.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.27
 ******************************************************************************/
void MacroUiTests::closingModifiedMacro_data() //20261127-1 add
{
    QTest::addColumn<int>("choice");
    QTest::addColumn<bool>("multiple");
    QTest::newRow("save-single") << int(QMessageBox::Save) << false;
    QTest::newRow("discard-single") << int(QMessageBox::Discard) << false;
    QTest::newRow("cancel-single") << int(QMessageBox::Cancel) << false;
    QTest::newRow("save-multiple") << int(QMessageBox::Save) << true;
    QTest::newRow("discard-multiple") << int(QMessageBox::Discard) << true;
    QTest::newRow("cancel-multiple") << int(QMessageBox::Cancel) << true;
}

/******************************************************************************
 * Function Name   : MacroUiTests::closingModifiedMacro
 * Description     : Close the actual editor, verify named choices, undo history and survival of drafts on cancel.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.27
 ******************************************************************************/
void MacroUiTests::closingModifiedMacro() //20261127-1 add
{
    QFETCH(int, choice);
    QFETCH(bool, multiple);
    MainWindow window;
    HmiProject original = macroUiProject();
    bool promptMatched = false;
    bool recovered = false;
    int history;

    window.setProject(original, QString());
    original = window.currentEditorProject();
    history = window.m_projectHistoryIndex;
    QTimer::singleShot(0, &window, [&]() {
        MacroManagerDialog *editor = qobject_cast<MacroManagerDialog *>(QApplication::activeModalWidget());
        QPlainTextEdit *source = editor->findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"));
        QTimer selection;
        QEventLoop delivered;
        QString error;

        source->setPlainText(QStringLiteral("edited Alpha"));
        if (multiple)
        {
            editor->selectMacro(QStringLiteral("Zulu"), &error);
            source->setPlainText(QStringLiteral("edited Zulu"));
        }
        connect(&selection, &QTimer::timeout, editor, [&]() {
            QMessageBox *prompt = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            if (!prompt) { return; }
            selection.stop();
            promptMatched = prompt->windowTitle() == QStringLiteral("未保存") &&
                prompt->text().contains(QStringLiteral("Alpha")) && (!multiple || prompt->text().contains(QStringLiteral("Zulu"))) &&
                prompt->button(QMessageBox::Save) && prompt->button(QMessageBox::Discard) && prompt->button(QMessageBox::Cancel);
            prompt->button(QMessageBox::StandardButton(choice))->click();
            delivered.quit();
        });
        selection.start(10);
#ifdef Q_OS_WIN
        if (QApplication::platformName() == QStringLiteral("windows"))
        {
            // Windows forwards SC_CLOSE to Qt asynchronously. Keep the draft alive until the prompt has been answered.
            PostMessageW(reinterpret_cast<HWND>(editor->winId()), WM_SYSCOMMAND, SC_CLOSE, 0);
            QTimer::singleShot(5000, &delivered, &QEventLoop::quit);
            delivered.exec();
        }
        else
#endif
        {
            editor->close();
        }
        if (editor->isVisible())
        {
            recovered = source->toPlainText() == (multiple ? QStringLiteral("edited Zulu") : QStringLiteral("edited Alpha")) &&
                (!multiple || editor->draftProject().macros.value(QStringLiteral("Alpha")) == QStringLiteral("edited Alpha"));
            editor->reject(); // End the test after proving Cancel returned control to the intact editor.
        }
    });
    window.m_commands.value(QStringLiteral("action_script"))->trigger();
    QVERIFY(promptMatched);
    QCOMPARE(recovered, choice == QMessageBox::Cancel);
    if (choice == QMessageBox::Save)
    {
        QCOMPARE(window.currentEditorProject().macros.value(QStringLiteral("Alpha")), QStringLiteral("edited Alpha"));
        if (multiple) { QCOMPARE(window.currentEditorProject().macros.value(QStringLiteral("Zulu")), QStringLiteral("edited Zulu")); }
        QCOMPARE(window.m_projectHistoryIndex, history + 1);
    }
    else
    {
        QCOMPARE(window.currentEditorProject().toJson(), original.toJson());
        QCOMPARE(window.m_projectHistoryIndex, history);
    }
}

QTEST_MAIN(MacroUiTests)
#include "macro_ui_tests.moc"
