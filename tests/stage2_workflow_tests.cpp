#include "main_window.h"
#include "hmi_canvas.h"
#include "stage2_project_dialogs.h"
#include "stage2_global_tools.h"
#include "text_payload_compiler.h"
#include "independent_downloader.h"
#include <QAction>
#include <QAbstractButton>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QJsonDocument>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTabBar>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QToolBar>
#include <QToolButton> //20261111-1 add
#include <QDockWidget>
#include <QMenuBar>
#include <QMenu>
#include <QScrollArea>
#include <QStatusBar>
#include <QDir>
#include <QTreeWidget>
#include <functional>

//20261110-7 add: Exercise the real main-window transactions without using physical input or USB.
class Stage2WorkflowTests : public QObject
{
    Q_OBJECT
private:
    QTemporaryDir m_settings;
private slots:
    void initTestCase();
    void sharedCommandsAndAvailability();
    void canvasKeysUseWholeProjectHistory();
    void textClipboardDoesNotModifyCanvas();
    void numericDraftCommandsStayLocal();
    void focusedWindowDestructionIsSafe();
    void macroSearchLocationOpensSource();
    void cancelledWizardPreservesDirtyDocument();
    void invalidOpenPreservesDocument();
    void openingSamePathReloadsAfterSave();
    void saveDiscardCancelClose();
    void cancelledSaveAsBlocksClose();
    void failedSaveBlocksClose();
    void actualTabsComboAndTreeNavigateById();
    void visitingUnsortedLayersIsNavigationOnly();
    void crossPageReplacementUndoAndSavedCheckpoint();
    void invalidDraftAndNativeOverwriteAreAtomic();
    void reopenAfterCloseRestoresEditor();
    void recentFilesAreBoundedAndIsolated();
    void openedProjectShowsProtocolsUnderTheirOwnPorts(); //20261122-6 add
    void serialPortSettingsAreIndependentAndPersisted(); //20261122-6 add
    void protocolTreeEditsAreScopedToTheirInstance(); //20261122-7 add
    void addingProtocolKeepsExistingProtocolsOnThePort(); //20261122-7 add
    void deletingProtocolProtectsProjectReferences(); //20261122-7 add
    void newPageFromSettingsTreeCancelsOrReopens(); //20261122-9 add
    void nativeShellResourcesAndNavigation(); //20261111-1 add
    void nativeShellQuickPropertiesPreserveEditing();
    void nativeShellPendingCommandsAreReadOnly();
};

/******************************************************************************
 * Function Name   : workflowProject
 * Description     : Build a minimal valid document containing a stable editable control.
 * Input Parameters: None
 * Return Value    : A one-page project.
 * Date            : 2026.11.10
 ******************************************************************************/
static HmiProject workflowProject() //20261110-7 add
{
    HmiProject project;
    HmiObject object;
    object.id = QStringLiteral("workflow-text");
    object.name = QStringLiteral("测试文字");
    object.type = QStringLiteral("text");
    object.text = QStringLiteral("TEST");
    object.address = QStringLiteral("LW0");
    object.geometry = QRect(244, 112, 150, 50);
    project.objects.append(object);
    return project;
}

/******************************************************************************
 * Function Name   : initTestCase
 * Description     : Redirect recent-file preferences to an owned disposable directory.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::initTestCase() //20261110-7 add
{
    QVERIFY(m_settings.isValid());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settings.path());
}

/******************************************************************************
 * Function Name   : sharedCommandsAndAvailability
 * Description     : Check common QAction identity, native property shortcut and document state transitions.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::sharedCommandsAndAvailability() //20261110-7 add
{
    MainWindow window;
    QAction *save;
    QToolBar *toolbar;
    bool found = false;
    save = window.m_commands.value(QStringLiteral("action_save"));
    QVERIFY(save != nullptr);
    QVERIFY(!save->isEnabled());
    Q_FOREACH (toolbar, window.findChildren<QToolBar *>()) {
        if (toolbar->actions().contains(save)) { found = true; }
    }
    QVERIFY(found);
    QCOMPARE(window.m_commands.value(QStringLiteral("action_openpropertiset"))->shortcut(),
             QKeySequence(QStringLiteral("Ctrl+P")));
    window.setProject(workflowProject(), QString());
    QVERIFY(!window.m_commands.value(QStringLiteral("action_delete"))->isEnabled());
    window.m_canvas->selectObject(QStringLiteral("workflow-text"));
    QVERIFY(window.m_commands.value(QStringLiteral("action_delete"))->isEnabled());
    window.dispatchEditCommand(QStringLiteral("delete"));
    QVERIFY(save->isEnabled());
    QVERIFY(window.m_dirty);
    window.dispatchEditCommand(QStringLiteral("undo"));
    QVERIFY(!window.m_dirty);
    QVERIFY(!save->isEnabled());
    window.closeProject();
    QVERIFY(!window.m_commands.value(QStringLiteral("action_USBdownload"))->isEnabled());
    QVERIFY(!window.m_commands.value(QStringLiteral("action_project_settings"))->isEnabled());
    QVERIFY(window.m_commands.value(QStringLiteral("action_open_pro"))->isEnabled());
}

/******************************************************************************
 * Function Name   : canvasKeysUseWholeProjectHistory
 * Description     : Route hosted canvas edit requests through one shared undo and redo history.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::canvasKeysUseWholeProjectHistory() //20261110-7 add
{
    MainWindow window;
    window.setProject(workflowProject(), QString());
    window.m_canvas->selectObject(QStringLiteral("workflow-text"));
    window.m_canvas->requestEditCommand(QStringLiteral("delete"));
    QCOMPARE(window.m_canvas->objects().size(), 0);
    QCOMPARE(window.m_projectHistoryIndex, 1);
    window.m_canvas->requestEditCommand(QStringLiteral("undo"));
    QCOMPARE(window.m_canvas->objects().size(), 1);
    QVERIFY(!window.m_dirty);
    window.m_canvas->requestEditCommand(QStringLiteral("redo"));
    QCOMPARE(window.m_canvas->objects().size(), 0);
    QVERIFY(window.m_dirty);
}

/******************************************************************************
 * Function Name   : textClipboardDoesNotModifyCanvas
 * Description     : Keep focused text edit operations separate from selected canvas controls.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::textClipboardDoesNotModifyCanvas() //20261110-7 add
{
    MainWindow window;
    QLineEdit editor(&window);
    QByteArray before;
    window.setProject(workflowProject(), QString());
    window.m_canvas->selectObject(QStringLiteral("workflow-text"));
    window.show();
    editor.show();
    editor.setFocus();
    QTRY_COMPARE(QApplication::focusWidget(), &editor);
    before = QJsonDocument(window.currentEditorProject().toJson()).toJson();
    editor.setText(QStringLiteral("copy text"));
    editor.selectAll();
    window.dispatchEditCommand(QStringLiteral("copy"));
    QCOMPARE(QApplication::clipboard()->text(), QStringLiteral("copy text"));
    window.dispatchEditCommand(QStringLiteral("cut"));
    QVERIFY(editor.text().isEmpty());
    window.dispatchEditCommand(QStringLiteral("undo"));
    QCOMPARE(editor.text(), QStringLiteral("copy text"));
    QCOMPARE(QJsonDocument(window.currentEditorProject().toJson()).toJson(), before);
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : numericDraftCommandsStayLocal
 * Description     : Preserve project history when numeric property text receives menu clipboard and undo operations.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::numericDraftCommandsStayLocal() //20261110-7 add
{
    MainWindow window;
    HmiProject changed;
    QLineEdit *line;
    int history;
    window.setProject(workflowProject(), QString());
    changed = window.currentEditorProject();
    changed.name = QStringLiteral("retain whole history");
    QVERIFY(window.commitProjectDraft(changed));
    history = window.m_projectHistoryIndex;
    window.show();
    window.m_canvas->selectObject(QStringLiteral("workflow-text"));
    window.m_rightTabs->setCurrentIndex(1);
    window.m_xSpin->setFocus();
    QTest::qWait(20);
    line = window.m_xSpin->findChild<QLineEdit *>();
    QVERIFY(line != nullptr);
    line->selectAll();
    window.dispatchEditCommand(QStringLiteral("copy"));
    QCOMPARE(QApplication::clipboard()->text(), line->selectedText());
    window.dispatchEditCommand(QStringLiteral("undo"));
    QCOMPARE(window.m_projectHistoryIndex, history);
    window.dispatchEditCommand(QStringLiteral("delete"));
    QCOMPARE(window.m_canvas->objects().size(), 1);
    QCOMPARE(window.m_project.name, changed.name);
}

/******************************************************************************
 * Function Name   : focusedWindowDestructionIsSafe
 * Description     : Repeatedly release focused editor windows without calling slots on destroyed derived state.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::focusedWindowDestructionIsSafe() //20261110-7 add
{
    MainWindow *window;
    int index;
    for (index = 0; index < 8; ++index) {
        window = new MainWindow();
        window->setProject(workflowProject(), QString());
        window->show();
        window->m_canvas->selectObject(QStringLiteral("workflow-text"));
        window->m_rightTabs->setCurrentIndex(1);
        window->m_xSpin->setFocus();
        QApplication::processEvents();
        delete window;
        QApplication::processEvents();
    }
}

/******************************************************************************
 * Function Name   : macroSearchLocationOpensSource
 * Description     : Navigate a macro result to a read-only source line without changing its definition.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::macroSearchLocationOpensSource() //20261110-7 add
{
    MainWindow window;
    HmiProject project = workflowProject();
    bool checked = false;
    project.macros.insert(QStringLiteral("macro"), QStringLiteral("line1\nline2\nline3"));
    window.setProject(project, QString());
    QTimer::singleShot(0, [&checked, &window]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QPlainTextEdit *source = dialog->findChild<QPlainTextEdit *>(QStringLiteral("Stage2MacroSource"));
        QString before;
        checked = source != nullptr && source->isReadOnly() && source->textCursor().blockNumber() == 1;
        if (source != nullptr) {
            before = source->toPlainText();
            source->setFocus();
            QApplication::processEvents();
            window.dispatchEditCommand(QStringLiteral("delete"));
            checked = checked && source->toPlainText() == before;
        }
        dialog->reject();
    });
    window.showMacroSource(QStringLiteral("macro"), 2);
    QVERIFY(checked);
    QCOMPARE(window.m_project.macros, project.macros);
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : cancelledWizardPreservesDirtyDocument
 * Description     : Reject the real new-project wizard without replacing dirty data or the current path.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::cancelledWizardPreservesDirtyDocument() //20261110-7 add
{
    MainWindow window;
    HmiProject changed;
    QByteArray before;
    window.setProject(workflowProject(), QStringLiteral("unchanged.ksp"));
    changed = window.currentEditorProject();
    changed.name = QStringLiteral("unsaved");
    QVERIFY(window.commitProjectDraft(changed));
    before = QJsonDocument(window.currentEditorProject().toJson()).toJson();
    QTimer::singleShot(0, []() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (dialog != nullptr) { dialog->reject(); }
    });
    window.createProject();
    QCOMPARE(QJsonDocument(window.currentEditorProject().toJson()).toJson(), before);
    QCOMPARE(window.m_currentPath, QStringLiteral("unchanged.ksp"));
    QVERIFY(window.m_dirty);
}

/******************************************************************************
 * Function Name   : invalidOpenPreservesDocument
 * Description     : Reject malformed input before asking to discard or changing the live document.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::invalidOpenPreservesDocument() //20261110-7 add
{
    MainWindow window;
    QTemporaryDir directory;
    QFile file(directory.filePath(QStringLiteral("broken.ksp")));
    HmiProject changed;
    QString error;
    QByteArray before;
    window.setProject(workflowProject(), QStringLiteral("old.ksp"));
    changed = window.currentEditorProject();
    changed.name = QStringLiteral("unsaved");
    QVERIFY(window.commitProjectDraft(changed));
    before = QJsonDocument(window.currentEditorProject().toJson()).toJson();
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("{invalid");
    file.close();
    QVERIFY(!window.openProjectPath(file.fileName(), &error));
    QVERIFY(!error.isEmpty());
    QCOMPARE(QJsonDocument(window.currentEditorProject().toJson()).toJson(), before);
    QCOMPARE(window.m_currentPath, QStringLiteral("old.ksp"));
    QVERIFY(window.m_dirty);
}

/******************************************************************************
 * Function Name   : openingSamePathReloadsAfterSave
 * Description     : Reload a recent-file target after the unsaved-work prompt saves into that same file.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::openingSamePathReloadsAfterSave() //20261110-7 add
{
    MainWindow window;
    QTemporaryDir directory;
    HmiProject changed;
    QString error;
    QString path = directory.filePath(QStringLiteral("same.ksp"));
    window.setProject(workflowProject(), path);
    QVERIFY(window.writeProject(path));
    changed = window.currentEditorProject();
    changed.name = QStringLiteral("saved during open");
    QVERIFY(window.commitProjectDraft(changed));
    QTimer::singleShot(0, []() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (box != nullptr) { box->button(QMessageBox::Save)->click(); }
    });
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    QCOMPARE(window.m_project.name, changed.name);
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : saveDiscardCancelClose
 * Description     : Exercise all three unsaved-work outcomes through the real close command.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::saveDiscardCancelClose() //20261110-7 add
{
    MainWindow window;
    QTemporaryDir directory;
    HmiProject changed;
    HmiProject loaded;
    QString error;
    QMessageBox::StandardButton response;
    window.setProject(workflowProject(), directory.filePath(QStringLiteral("saved.ksp")));
    changed = window.currentEditorProject();
    changed.name = QStringLiteral("saved change");
    QVERIFY(window.commitProjectDraft(changed));
    Q_FOREACH (response, (QList<QMessageBox::StandardButton>{QMessageBox::Cancel, QMessageBox::Save})) {
        QTimer::singleShot(0, [response]() {
            QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            if (box != nullptr) { box->button(response)->click(); }
        });
        window.closeProject();
        QCOMPARE(window.m_hasProject, response == QMessageBox::Cancel);
    }
    QVERIFY2(HmiProject::load(directory.filePath(QStringLiteral("saved.ksp")), &loaded, &error), qPrintable(error));
    QCOMPARE(loaded.name, QStringLiteral("saved change"));
    window.setProject(workflowProject(), QString());
    QVERIFY(window.commitProjectDraft(changed));
    QTimer::singleShot(0, []() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (box != nullptr) { box->button(QMessageBox::Discard)->click(); }
    });
    window.closeProject();
    QVERIFY(!window.m_hasProject);
}

/******************************************************************************
 * Function Name   : cancelledSaveAsBlocksClose
 * Description     : Preserve an unsaved document when Save is chosen but its destination dialog is cancelled.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::cancelledSaveAsBlocksClose() //20261110-7 add
{
    MainWindow window;
    HmiProject changed;
    QTimer timer;
    window.setProject(workflowProject(), QString());
    changed = window.currentEditorProject();
    changed.name = QStringLiteral("keep me");
    QVERIFY(window.commitProjectDraft(changed));
    connect(&timer, &QTimer::timeout, []() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        QFileDialog *dialog = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
        if (box != nullptr) { box->button(QMessageBox::Save)->click(); }
        if (dialog != nullptr) { dialog->reject(); }
    });
    timer.start(10);
    window.closeProject();
    QVERIFY(window.m_hasProject);
    QVERIFY(window.m_dirty);
    QVERIFY(window.m_currentPath.isEmpty());
}

/******************************************************************************
 * Function Name   : failedSaveBlocksClose
 * Description     : Keep live work and history when the atomic file save fails.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::failedSaveBlocksClose() //20261110-7 add
{
    MainWindow window;
    HmiProject changed;
    QTemporaryDir directory;
    QTimer timer;
    window.setProject(workflowProject(), directory.filePath(QStringLiteral("missing/fail.ksp")));
    changed = window.currentEditorProject();
    changed.name = QStringLiteral("keep failed save");
    QVERIFY(window.commitProjectDraft(changed));
    connect(&timer, &QTimer::timeout, []() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (box != nullptr) {
            box->button(box->standardButtons().testFlag(QMessageBox::Save) ? QMessageBox::Save : QMessageBox::Ok)->click();
        }
    });
    timer.start(10);
    window.closeProject();
    QVERIFY(window.m_hasProject);
    QVERIFY(window.m_dirty);
    QCOMPARE(window.m_project.name, changed.name);
}

/******************************************************************************
 * Function Name   : actualTabsComboAndTreeNavigateById
 * Description     : Switch two pages and a window with identical labels through stable UI identities.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::actualTabsComboAndTreeNavigateById() //20261110-7 add
{
    MainWindow window;
    HmiProject project = workflowProject();
    HmiPage second;
    HmiPage popup;
    QString error;
    QTreeWidgetItem *item;
    int index;
    second.id = QStringLiteral("second");
    second.name = QStringLiteral("second page");
    second.objects = project.objects;
    second.objects[0].id = QStringLiteral("second-object");
    second.objects[0].text = QStringLiteral("SECOND");
    popup.id = QStringLiteral("popup");
    popup.name = QStringLiteral("popup page");
    popup.kind = QStringLiteral("window");
    popup.width = 240;
    popup.height = 180;
    QVERIFY2(project.addPage(second, &error), qPrintable(error));
    QVERIFY2(project.addPage(popup, &error), qPrintable(error));
    window.setProject(project, QString());
    QCOMPARE(window.m_pageTabs->count(), 3);
    index = window.m_pageCombo->findData(second.id);
    window.m_pageCombo->setCurrentIndex(index);
    QCOMPARE(window.m_project.activePageId, second.id);
    QCOMPARE(window.m_canvas->objects().first().text, QStringLiteral("SECOND"));
    window.m_pageTabs->setCurrentIndex(2);
    QCOMPARE(window.m_project.activePageId, popup.id);
    QVERIFY(window.m_canvas->objects().isEmpty());
    QCOMPARE(window.m_pageTabs->currentWidget(), static_cast<QWidget *>(window.m_canvas));
    item = window.m_projectTree->topLevelItem(0)->child(0);
    QVERIFY(item != nullptr);
    window.m_projectTree->setCurrentItem(item);
    QCOMPARE(window.m_project.activePageId, project.activePageId);
    QCOMPARE(window.m_canvas->objects().first().text, QStringLiteral("TEST"));
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : crossPageReplacementUndoAndSavedCheckpoint
 * Description     : Undo and redo a whole-project address batch across pages and retain the saved checkpoint.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::visitingUnsortedLayersIsNavigationOnly() //20261110-7 add
{
    MainWindow window;
    HmiProject project = workflowProject();
    HmiPage second;
    HmiObject lower = project.objects.first();
    QString error;
    second.id = QStringLiteral("unsorted");
    second.name = QStringLiteral("unsorted page");
    second.objects = project.objects;
    second.objects[0].id = QStringLiteral("upper");
    second.objects[0].layer = 2;
    lower.id = QStringLiteral("lower");
    lower.layer = 1;
    second.objects.append(lower);
    QVERIFY(project.addPage(second, &error));
    window.setProject(project, QString());
    QVERIFY(window.activatePageId(second.id));
    QVERIFY(!window.projectIsModified());
    window.setDirty(true);
    QVERIFY(!window.m_dirty);
    QCOMPARE(window.m_projectHistoryIndex, 0);
}

/******************************************************************************
 * Function Name   : crossPageReplacementUndoAndSavedCheckpoint
 * Description     : Restore a cross-page batch as one transaction and preserve its independent save checkpoint.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::crossPageReplacementUndoAndSavedCheckpoint() //20261110-7 add
{
    MainWindow window;
    HmiProject project = workflowProject();
    HmiProject candidate;
    HmiProject loaded;
    HmiPage second;
    Stage2GlobalTools::AddressCandidate preview;
    Stage2GlobalTools::Scope scope;
    QTemporaryDir directory;
    QString error;
    QVector<QByteArray> payloads;
    project.objects[0].type = QStringLiteral("switch");
    project.objects[0].hasBitButtonProperties = true;
    project.objects[0].hasWordButtonProperties = true;
    project.objects[0].bitButton.indicator = false;
    project.objects[0].bitButton.dualMonitor = false;
    second.id = QStringLiteral("second");
    second.name = QStringLiteral("second page");
    second.objects = project.objects;
    second.objects[0].id = QStringLiteral("second-object");
    QVERIFY(project.addPage(second, &error));
    window.setProject(project, QString());
    QVERIFY2(Stage2GlobalTools::previewAddressReplacement(project, QStringLiteral("LW0"), QStringLiteral("LW2"), scope, &preview, &error), qPrintable(error));
    QVERIFY2(Stage2GlobalTools::applyCandidate(project, preview, &candidate, &error), qPrintable(error));
    QVERIFY2(window.commitProjectDraft(candidate, &error), qPrintable(error));
    QVERIFY(window.m_dirty);
    QCOMPARE(window.currentEditorProject().allPages().at(0).objects.first().address, QStringLiteral("LW2"));
    QCOMPARE(window.currentEditorProject().allPages().at(1).objects.first().address, QStringLiteral("LW2"));
    QVERIFY(window.writeProject(directory.filePath(QStringLiteral("batch.ksp"))));
    QVERIFY(!window.m_dirty);
    window.dispatchEditCommand(QStringLiteral("undo"));
    QVERIFY(window.m_dirty);
    QCOMPARE(window.currentEditorProject().allPages().at(1).objects.first().address, QStringLiteral("LW0"));
    window.dispatchEditCommand(QStringLiteral("redo"));
    QVERIFY(!window.m_dirty);
    QVERIFY(HmiProject::load(directory.filePath(QStringLiteral("batch.ksp")), &loaded, &error));
    QCOMPARE(loaded.allPages().at(1).objects.first().address, QStringLiteral("LW2"));
    QVERIFY(!buildProjectPayload(loaded, &payloads, &error));
    QVERIFY(payloads.isEmpty());
}

/******************************************************************************
 * Function Name   : invalidDraftAndNativeOverwriteAreAtomic
 * Description     : Reject invalid configuration and native-source overwrite without publishing partial changes.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::invalidDraftAndNativeOverwriteAreAtomic() //20261110-7 add
{
    MainWindow window;
    HmiProject invalid;
    QTemporaryDir directory;
    QFile native(directory.filePath(QStringLiteral("existing.sm")));
    QString error;
    QByteArray before;
    window.setProject(workflowProject(), QStringLiteral("unchanged.ksp"));
    before = QJsonDocument(window.currentEditorProject().toJson()).toJson();
    invalid = window.currentEditorProject();
    invalid.objects.append(invalid.objects.first());
    QVERIFY(!window.commitProjectDraft(invalid, &error));
    QCOMPARE(QJsonDocument(window.currentEditorProject().toJson()).toJson(), before);
    QVERIFY(native.open(QIODevice::WriteOnly));
    native.write("preserve native");
    native.close();
    QTimer::singleShot(0, []() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (box != nullptr) { box->button(QMessageBox::Ok)->click(); }
    });
    QVERIFY(!window.writeProject(native.fileName()));
    QVERIFY(native.open(QIODevice::ReadOnly));
    QCOMPARE(native.readAll(), QByteArray("preserve native"));
    QCOMPARE(window.m_currentPath, QStringLiteral("unchanged.ksp"));
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : reopenAfterCloseRestoresEditor
 * Description     : Restore all command and navigation surfaces after closing then reopening a document.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::reopenAfterCloseRestoresEditor() //20261110-7 add
{
    MainWindow window;
    window.closeProject();
    QVERIFY(!window.m_systemSettingsTree->isEnabled());
    window.setProject(workflowProject(), QString());
    QVERIFY(window.m_hasProject);
    QVERIFY(window.m_canvas->isEnabled());
    QVERIFY(window.m_pageTabs->isEnabled());
    QVERIFY(window.m_systemSettingsTree->isEnabled());
    QVERIFY(window.m_commands.value(QStringLiteral("action_USBdownload"))->isEnabled());
}

/******************************************************************************
 * Function Name   : openedProjectShowsProtocolsUnderTheirOwnPorts
 * Description     : Reopen a saved project and check protocol ownership in the visible tree.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void Stage2WorkflowTests::openedProjectShowsProtocolsUnderTheirOwnPorts() //20261122-6 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    PlcConnection connection;
    QString error;
    QString path;
    MainWindow window;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> ports;

    QVERIFY(directory.isValid());
    project.targetModel = QStringLiteral("AMZ070W01");
    connection.id = QStringLiteral("com2");
    connection.channel = PlcChannel::Com2;
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.station = QStringLiteral("1");
    project.connections.append(connection);
    path = directory.filePath(QStringLiteral("port-com2.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    ports = tree->findItems(QStringLiteral("COM1"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(ports.size(), 1);
    QCOMPARE(ports.first()->childCount(), 0);
    ports = tree->findItems(QStringLiteral("COM2"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(ports.size(), 1);
    QCOMPARE(ports.first()->childCount(), 1);
    QCOMPARE(ports.first()->child(0)->text(0), QStringLiteral("Modbus_RTU"));
}

/******************************************************************************
 * Function Name   : serialPortSettingsAreIndependentAndPersisted
 * Description     : Open either port from the tree and exercise cancel, save, reopen and reset.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void Stage2WorkflowTests::serialPortSettingsAreIndependentAndPersisted() //20261122-6 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    MainWindow window;
    MainWindow reopened;
    QString error;
    QString path;
    QTreeWidget *tree;
    auto editPort = [](QTreeWidget *settings, const QString &port, const QString &baud,
                       const QString &online, bool confirm, bool reset) {
        QStringList observed;
        const QList<QTreeWidgetItem *> items = settings->findItems(port, Qt::MatchExactly | Qt::MatchRecursive);
        if (items.size() != 1)
        {
            return observed;
        }
        QTimer::singleShot(0, settings, [&]() {
            QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (dialog == nullptr)
            {
                return;
            }
            QLineEdit *name = dialog->findChild<QLineEdit *>(QStringLiteral("edit_name"));
            QComboBox *speed = dialog->findChild<QComboBox *>(QStringLiteral("PlcPortBaud"));
            QComboBox *pcPort = dialog->findChild<QComboBox *>(QStringLiteral("PlcPortOnline"));
            QComboBox *mode = dialog->findChild<QComboBox *>(QStringLiteral("PlcPortMode"));
            QDialogButtonBox *buttons = dialog->findChild<QDialogButtonBox *>();
            QPushButton *defaults = dialog->findChild<QPushButton *>(QStringLiteral("PlcPortDefaults"));
            if (dialog->windowTitle() != QStringLiteral("连接设置") || name == nullptr || !name->isReadOnly() ||
                speed == nullptr || pcPort == nullptr || mode == nullptr || buttons == nullptr || defaults == nullptr)
            {
                observed.append(dialog->windowTitle());
                dialog->reject();
                return;
            }
            observed = {name->text(), speed->currentText(), pcPort->currentText(), mode->currentText()};
            if (!baud.isEmpty()) { speed->setCurrentText(baud); }
            if (!online.isEmpty()) { pcPort->setCurrentText(online); }
            if (reset) { defaults->click(); }
            buttons->button(confirm ? QDialogButtonBox::Ok : QDialogButtonBox::Cancel)->click();
        });
        settings->itemDoubleClicked(items.first(), 0);
        return observed;
    };

    QVERIFY(directory.isValid());
    project.targetModel = QStringLiteral("AMZ070W01");
    path = directory.filePath(QStringLiteral("serial-settings.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    QCOMPARE(editPort(tree, QStringLiteral("COM2"), QStringLiteral("19200"), QStringLiteral("COM11"), false, false),
             (QStringList{QStringLiteral("COM2"), QStringLiteral("9600"), QStringLiteral("COM2"), QStringLiteral("232")}));
    QVERIFY(!window.m_dirty);
    QCOMPARE(editPort(tree, QStringLiteral("COM2"), QStringLiteral("19200"), QStringLiteral("COM11"), true, false),
             (QStringList{QStringLiteral("COM2"), QStringLiteral("9600"), QStringLiteral("COM2"), QStringLiteral("232")}));
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    tree = reopened.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    QCOMPARE(editPort(tree, QStringLiteral("COM1"), {}, {}, false, false),
             (QStringList{QStringLiteral("COM1"), QStringLiteral("9600"), QStringLiteral("COM1"), QStringLiteral("485")}));
    QCOMPARE(editPort(tree, QStringLiteral("COM2"), {}, {}, true, true),
             (QStringList{QStringLiteral("COM2"), QStringLiteral("19200"), QStringLiteral("COM11"), QStringLiteral("232")}));
    QCOMPARE(editPort(tree, QStringLiteral("COM2"), {}, {}, false, false),
             (QStringList{QStringLiteral("COM2"), QStringLiteral("9600"), QStringLiteral("COM2"), QStringLiteral("232")}));
}

/******************************************************************************
 * Function Name   : protocolTreeEditsAreScopedToTheirInstance
 * Description     : Rename and change timing on COM2 while retaining COM1 and the original stable ID.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void Stage2WorkflowTests::protocolTreeEditsAreScopedToTheirInstance() //20261122-7 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    PlcConnection connection;
    MainWindow window;
    MainWindow reopened;
    QString error;
    QString path;
    QTreeWidget *tree;
    auto editProtocol = [](QTreeWidget *settings, const QString &port, bool edit, bool confirm) {
        QStringList fields;
        const QList<QTreeWidgetItem *> ports = settings->findItems(port, Qt::MatchExactly | Qt::MatchRecursive);
        if (ports.size() != 1 || ports.first()->childCount() != 1)
        {
            return fields;
        }
        QTimer::singleShot(0, settings, [&]() {
            QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (dialog == nullptr)
            {
                return;
            }
            QLineEdit *name = dialog->findChild<QLineEdit *>(QStringLiteral("edit_name"));
            QComboBox *series = dialog->findChild<QComboBox *>(QStringLiteral("combo_protocol"));
            QSpinBox *station = dialog->findChild<QSpinBox *>(QStringLiteral("spin_stationIndex"));
            QSpinBox *timeout = dialog->findChild<QSpinBox *>(QStringLiteral("spin_time"));
            QDialogButtonBox *buttons = dialog->findChild<QDialogButtonBox *>();
            if (dialog->windowTitle() != QStringLiteral("协议") || name == nullptr || series == nullptr ||
                station == nullptr || timeout == nullptr || buttons == nullptr)
            {
                fields.append(dialog->windowTitle());
                dialog->reject();
                return;
            }
            fields = {series->currentText(), name->text(), QString::number(station->value()), QString::number(timeout->value())};
            if (edit)
            {
                name->setText(QStringLiteral("PLC-B"));
                timeout->setValue(2200);
            }
            buttons->button(confirm ? QDialogButtonBox::Ok : QDialogButtonBox::Cancel)->click();
        });
        settings->itemDoubleClicked(ports.first()->child(0), 0);
        return fields;
    };

    QVERIFY(directory.isValid());
    project.targetModel = QStringLiteral("AMZ070W01R");
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.id = QStringLiteral("com1");
    connection.channel = PlcChannel::Com1;
    connection.station = QStringLiteral("1");
    project.connections.append(connection);
    connection.id = QStringLiteral("com2");
    connection.channel = PlcChannel::Com2;
    connection.station = QStringLiteral("2");
    project.connections.append(connection);
    path = directory.filePath(QStringLiteral("protocol-settings.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    QCOMPARE(editProtocol(tree, QStringLiteral("COM2"), true, false),
             (QStringList{QStringLiteral("Modbus_RTU"), QStringLiteral("Modbus_RTU"), QStringLiteral("2"), QStringLiteral("1000")}));
    QVERIFY(!window.m_dirty);
    QCOMPARE(editProtocol(tree, QStringLiteral("COM2"), true, true),
             (QStringList{QStringLiteral("Modbus_RTU"), QStringLiteral("Modbus_RTU"), QStringLiteral("2"), QStringLiteral("1000")}));
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    tree = reopened.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    QCOMPARE(editProtocol(tree, QStringLiteral("COM1"), false, false),
             (QStringList{QStringLiteral("Modbus_RTU"), QStringLiteral("Modbus_RTU"), QStringLiteral("1"), QStringLiteral("1000")}));
    QCOMPARE(editProtocol(tree, QStringLiteral("COM2"), false, false),
             (QStringList{QStringLiteral("Modbus_RTU"), QStringLiteral("PLC-B"), QStringLiteral("2"), QStringLiteral("2200")}));
    const QList<QTreeWidgetItem *> named = tree->findItems(QStringLiteral("PLC-B"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(named.size(), 1);
    QCOMPARE(named.first()->parent()->text(0), QStringLiteral("COM2"));
    QCOMPARE(named.first()->data(0, Qt::UserRole + 1).toString(), QStringLiteral("com2"));
}

/******************************************************************************
 * Function Name   : triggerSettingsTreeAction
 * Description     : Select an action from the actual settings tree context menu.
 * Input Parameters: tree, item, actionText, whileDialogOpen.
 * Return Value    : True if the menu exposes the requested action.
 * Date            : 2026.11.22
 ******************************************************************************/
static bool triggerSettingsTreeAction(QTreeWidget *tree, QTreeWidgetItem *item, const QString &actionText,
                                      const std::function<void()> &whileDialogOpen = {}) //20261122-7 add
{
    bool triggered = false;
    QTimer menuSelection;

    tree->scrollToItem(item);
    menuSelection.setSingleShot(true);
    QObject::connect(&menuSelection, &QTimer::timeout, tree, [&]() {
        QMenu *menu = qobject_cast<QMenu *>(QApplication::activePopupWidget());
        QAction *action;
        QList<QAction *> actions;
        if (menu == nullptr)
        {
            return;
        }
        actions = menu->actions();
        menu->close();
        Q_FOREACH (action, actions)
        {
            if (action->text() == actionText && action->isEnabled())
            {
                if (whileDialogOpen)
                {
                    QTimer::singleShot(0, tree, whileDialogOpen);
                }
                triggered = true;
                action->trigger();
                return;
            }
        }
    });
    menuSelection.start(0);
    tree->customContextMenuRequested(tree->visualItemRect(item).center());
    return triggered;
}

/******************************************************************************
 * Function Name   : newPageFromSettingsTreeCancelsOrReopens
 * Description     : Open the ordinary page creation action and verify cancellation, activation and persistence.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void Stage2WorkflowTests::newPageFromSettingsTreeCancelsOrReopens() //20261122-9 add
{
    QTemporaryDir directory;
    HmiProject original = workflowProject();
    HmiProject created;
    MainWindow window;
    MainWindow reopened;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> groups;
    QStringList displayedDefaults;
    QString error;
    QString path;

    QVERIFY(directory.isValid());
    original.targetModel = QStringLiteral("AMZ070W01");
    path = directory.filePath(QStringLiteral("new-page.ksp"));
    QVERIFY2(original.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    groups = tree->findItems(QStringLiteral("画面"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(groups.size(), 1);
    auto finishDraft = [&displayedDefaults](bool confirm) {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QLineEdit *name;
        QSpinBox *number;
        QDialogButtonBox *buttons;

        if (dialog == nullptr)
        {
            return;
        }
        name = dialog->findChild<QLineEdit *>(QStringLiteral("newPageNameEdit"));
        number = dialog->findChild<QSpinBox *>(QStringLiteral("newPageNumberSpin"));
        buttons = dialog->findChild<QDialogButtonBox *>();
        if (name == nullptr || number == nullptr || buttons == nullptr)
        {
            dialog->reject();
            return;
        }
        displayedDefaults = {dialog->windowTitle(), name->text(), QString::number(number->value())};
        if (!confirm)
        {
            name->setText(QStringLiteral("取消的新画面"));
        }
        buttons->button(confirm ? QDialogButtonBox::Ok : QDialogButtonBox::Cancel)->click();
    };
    QVERIFY2(triggerSettingsTreeAction(tree, groups.first(), QStringLiteral("新建画面"), [&]() { finishDraft(false); }),
             "The ordinary page group must offer New Page.");
    QCOMPARE(displayedDefaults, (QStringList{QStringLiteral("新建画面"), QStringLiteral("画面2"), QStringLiteral("1")}));
    QCOMPARE(window.currentEditorProject().toJson(), original.toJson());
    QVERIFY(!window.m_dirty);
    displayedDefaults.clear();
    QVERIFY(triggerSettingsTreeAction(tree, groups.first(), QStringLiteral("新建画面"), [&]() { finishDraft(true); }));
    QCOMPARE(displayedDefaults, (QStringList{QStringLiteral("新建画面"), QStringLiteral("画面2"), QStringLiteral("1")}));
    created = window.currentEditorProject();
    QCOMPARE(created.pageCount(), 2);
    QCOMPARE(created.activePageKind, QStringLiteral("normal"));
    QCOMPARE(created.pageName, QStringLiteral("画面2"));
    QCOMPARE(created.startupPageId, original.startupPageId);
    QVERIFY(created.activePageId != original.activePageId);
    QVERIFY(window.m_canvas->objects().isEmpty());
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    QCOMPARE(reopened.currentEditorProject().toJson(), created.toJson());
    tree = reopened.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    groups = tree->findItems(QStringLiteral("画面"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(groups.size(), 1);
    QCOMPARE(groups.first()->childCount(), 2);
    QCOMPARE(groups.first()->child(1)->text(0), QStringLiteral("001:画面2"));
}

/******************************************************************************
 * Function Name   : addingProtocolKeepsExistingProtocolsOnThePort
 * Description     : Add a second COM2 instance from the context menu and reload both entries.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void Stage2WorkflowTests::addingProtocolKeepsExistingProtocolsOnThePort() //20261122-7 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    PlcConnection connection;
    MainWindow window;
    MainWindow reopened;
    QString error;
    QString path;
    QStringList fields;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> ports;

    QVERIFY(directory.isValid());
    project.targetModel = QStringLiteral("AMZ070W01R");
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.id = QStringLiteral("com2");
    connection.channel = PlcChannel::Com2;
    connection.station = QStringLiteral("1");
    project.connections.append(connection);
    path = directory.filePath(QStringLiteral("new-protocol.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    ports = tree->findItems(QStringLiteral("COM2"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(ports.size(), 1);
    QVERIFY(triggerSettingsTreeAction(tree, ports.first(), QStringLiteral("增加协议"), [&]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QComboBox *series;
        QSpinBox *station;
        QDialogButtonBox *buttons;
        if (dialog == nullptr)
        {
            return;
        }
        series = dialog->findChild<QComboBox *>(QStringLiteral("combo_protocol"));
        station = dialog->findChild<QSpinBox *>(QStringLiteral("spin_stationIndex"));
        buttons = dialog->findChild<QDialogButtonBox *>();
        if (series == nullptr || station == nullptr || buttons == nullptr)
        {
            dialog->reject();
            return;
        }
        fields = {series->currentText(), QString::number(station->value())};
        buttons->button(QDialogButtonBox::Ok)->click();
    }));
    QCOMPARE(fields, (QStringList{QStringLiteral("Mitsubishi_Fx3U"), QStringLiteral("0")}));
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    tree = reopened.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    ports = tree->findItems(QStringLiteral("COM2"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(ports.size(), 1);
    QCOMPARE(ports.first()->childCount(), 2);
    QCOMPARE(ports.first()->child(0)->text(0), QStringLiteral("Modbus_RTU"));
    QCOMPARE(ports.first()->child(1)->text(0), QStringLiteral("Mitsubishi_Fx3U"));
    QVERIFY(ports.first()->child(0)->data(0, Qt::UserRole + 1) != ports.first()->child(1)->data(0, Qt::UserRole + 1));
}

/******************************************************************************
 * Function Name   : deletingProtocolProtectsProjectReferences
 * Description     : Reject deletion of a referenced protocol and persist removal of its unreferenced neighbor.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void Stage2WorkflowTests::deletingProtocolProtectsProjectReferences() //20261122-7 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    HmiProject reloaded;
    HmiReference reference;
    PlcConnection connection;
    MainWindow window;
    QString error;
    QString path;
    QString warning;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> ports;

    QVERIFY(directory.isValid());
    project.targetModel = QStringLiteral("AMZ070W01R");
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.id = QStringLiteral("com2");
    connection.channel = PlcChannel::Com2;
    connection.station = QStringLiteral("1");
    project.connections.append(connection);
    connection.id = QStringLiteral("spare-com2");
    connection.displayName = QStringLiteral("PLC-SPARE");
    project.connections.append(connection);
    reference.id = QStringLiteral("system-route");
    reference.ownerKind = QStringLiteral("project");
    reference.ownerId = project.projectId;
    reference.field = QStringLiteral("route");
    reference.targetKind = QStringLiteral("connection");
    reference.targetId = QStringLiteral("com2");
    project.references.append(reference);
    path = directory.filePath(QStringLiteral("delete-protocol.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    ports = tree->findItems(QStringLiteral("COM2"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(ports.size(), 1);
    QCOMPARE(ports.first()->childCount(), 2);
    QVERIFY(triggerSettingsTreeAction(tree, ports.first()->child(0), QStringLiteral("删除"), [&]() {
        QMessageBox *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (message != nullptr)
        {
            warning = message->windowTitle();
            message->accept();
        }
    }));
    QCOMPARE(warning, QStringLiteral("协议仍被使用"));
    QCOMPARE(ports.first()->childCount(), 2);
    QVERIFY(!window.m_dirty);
    QVERIFY(triggerSettingsTreeAction(tree, ports.first()->child(1), QStringLiteral("删除")));
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(HmiProject::load(path, &reloaded, &error), qPrintable(error));
    QCOMPARE(reloaded.connections.size(), 1);
    QCOMPARE(reloaded.connections.first().id, QStringLiteral("com2"));
    QCOMPARE(reloaded.references.first().targetId, QStringLiteral("com2"));
}

/******************************************************************************
 * Function Name   : recentFilesAreBoundedAndIsolated
 * Description     : Keep ten exact recent paths while moving an existing entry to the front without duplication.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::recentFilesAreBoundedAndIsolated() //20261110-7 add
{
    MainWindow window;
    QTemporaryDir directory;
    QSettings settings(QStringLiteral("KinSealStudioLite"), QStringLiteral("Editor"));
    QString path;
    QStringList paths;
    int index;
    settings.clear();
    for (index = 0; index < 12; ++index) {
        window.updateRecentProjects(directory.filePath(QStringLiteral("project%1.ksp").arg(index)));
    }
    path = directory.filePath(QStringLiteral("project5.ksp"));
    window.updateRecentProjects(path);
    paths = settings.value(QStringLiteral("recentProjects")).toStringList();
    QCOMPARE(paths.size(), 10);
    QCOMPARE(paths.first(), path);
    QCOMPARE(paths.count(path), 1);
}

/******************************************************************************
 * Function Name   : nativeShellResourcesAndNavigation
 * Description     : Check retained icons, native shell geometry, recent-file routing and live status clearing.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.11
 ******************************************************************************/
void Stage2WorkflowTests::nativeShellResourcesAndNavigation() //20261111-1 add
{
    MainWindow window;
    QToolBar *bar;
    QScrollArea *palette;
    QLineEdit *field;
    QTemporaryDir files;
    QMenu *fileMenu;
    QAction *action;
    bool recentFound;
    QString captures;

    window.setProject(workflowProject(), QString()); window.resize(1353, 708); window.show();
    QTest::qWait(30);
    QCOMPARE(window.menuBar()->actions().size(), 8);
    QCOMPARE(window.findChildren<QToolBar *>().size(), 7);
    bar = window.findChild<QToolBar *>(QStringLiteral("standardToolBar"));
    QVERIFY(bar != nullptr); QCOMPARE(bar->geometry(), QRect(0, 19, 198, 34));
    bar = window.findChild<QToolBar *>(QStringLiteral("toolBar_Control"));
    QCOMPARE(bar->geometry().y(), 95); QCOMPARE(bar->height(), 34);
    QCOMPARE(window.findChild<QDockWidget *>(QStringLiteral("ProjectTreeDock"))->width(), 242);
    QCOMPARE(window.findChild<QDockWidget *>(QStringLiteral("ToolPropertyDock"))->width(), 337);
    QCOMPARE(window.statusBar()->height(), 25);
    QCOMPARE(window.m_systemSettingsTree->mapTo(&window, QPoint()), QPoint(20, 188));
    QCOMPARE(window.m_rightTabs->mapTo(&window, QPoint()).y(), 149);
    QCOMPARE(window.m_rightTabs->tabText(2), QStringLiteral("基本控件"));
    QVERIFY(!window.m_rightTabs->isTabVisible(1));
    palette = window.findChild<QScrollArea *>(QStringLiteral("NativeControlPalette"));
    QCOMPARE(palette->findChildren<QToolButton *>().size(), 31);
    palette = window.findChild<QScrollArea *>(QStringLiteral("NativeBasicPalette"));
    QCOMPARE(palette->findChildren<QToolButton *>().size(), 10);
    QVERIFY(!window.m_commands.value(QStringLiteral("action_H"))->icon().isNull());
    QVERIFY(!window.m_commands.value(QStringLiteral("action_h"))->icon().isNull());
    QVERIFY(window.m_commands.value(QStringLiteral("action_H"))->icon().pixmap(24).toImage() !=
            window.m_commands.value(QStringLiteral("action_h"))->icon().pixmap(24).toImage());
    QVERIFY(!window.windowIcon().isNull());
    window.m_commands.value(QStringLiteral("action_zoonbig"))->trigger();
    QCOMPARE(window.m_canvas->transform().m11(), 1.25);
    window.m_commands.value(QStringLiteral("action_zoonfit"))->trigger();
    QCOMPARE(window.m_canvas->transform().m11(), 1.0); QVERIFY(!window.m_dirty);
    QVERIFY(!window.m_commands.value(QStringLiteral("action_dataDisplay"))->property("nativeShellPending").toBool());
    QVERIFY(!window.m_commands.value(QStringLiteral("pictruedisplay"))->property("nativeShellPending").toBool());
    QVERIFY(files.isValid()); window.updateRecentProjects(files.filePath(QStringLiteral("visible-recent.ksp")));
    fileMenu = window.menuBar()->actions().first()->menu(); recentFound = false;
    Q_FOREACH (action, fileMenu->actions()) { if (action->menu() == window.m_recentProjectsMenu) { recentFound = true; } }
    QVERIFY(recentFound); QVERIFY(!window.m_recentProjectsMenu->actions().isEmpty());
    window.m_canvas->selectObject(QStringLiteral("workflow-text"));
    field = window.findChild<QLineEdit *>(QStringLiteral("NativeObjectX"));
    QVERIFY(field != nullptr); QCOMPARE(field->text(), QStringLiteral("244"));
    captures = qEnvironmentVariable("KINSEAL_SHELL_CAPTURE_DIR");
    if (!captures.isEmpty())
    {
        QVERIFY(window.grab().save(QDir(captures).filePath(QStringLiteral("selected-control.png"))));
        window.m_rightTabs->setCurrentIndex(2); QTest::qWait(20);
        QVERIFY(window.grab().save(QDir(captures).filePath(QStringLiteral("basic-controls.png"))));
    }
    window.closeProject();
    QCOMPARE(window.findChild<QLineEdit *>(QStringLiteral("NativeControlCount"))->text(), QString());
    QCOMPARE(window.findChild<QLineEdit *>(QStringLiteral("NativeResolution"))->text(), QString());
    QCOMPARE(field->text(), QString());
}

/******************************************************************************
 * Function Name   : nativeShellQuickPropertiesPreserveEditing
 * Description     : Exercise the relocated exact-geometry editor and restore the native toolbox after modal closing.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.11
 ******************************************************************************/
void Stage2WorkflowTests::nativeShellQuickPropertiesPreserveEditing() //20261111-1 add
{
    MainWindow window;
    QAction *quick;
    bool edited;

    window.setProject(workflowProject(), QString()); window.show();
    window.m_canvas->selectObject(QStringLiteral("workflow-text"));
    quick = window.findChild<QAction *>(QStringLiteral("ShellQuickProperties"));
    QVERIFY(quick != nullptr); edited = false;
    QTimer::singleShot(0, &window, [&window, &edited]() {
        QDialog *dialog;
        QPushButton *apply;
        dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (dialog == nullptr) { return; }
        QVERIFY(window.m_xSpin->isVisible());
        QVERIFY(window.m_nameEdit->isVisible());
        window.m_xSpin->setValue(123);
        apply = dialog->findChild<QPushButton *>(QStringLiteral("ApplyPropertiesButton"));
        if (apply != nullptr) { apply->click(); edited = true; }
        dialog->reject();
    });
    quick->trigger(); QVERIFY(edited);
    QCOMPARE(window.m_canvas->objects().first().geometry.x(), 123);
    QCOMPARE(window.m_rightTabs->count(), 3); QVERIFY(!window.m_rightTabs->isTabVisible(1));
    QCOMPARE(window.m_rightTabs->tabText(2), QStringLiteral("基本控件"));
}

/******************************************************************************
 * Function Name   : nativeShellPendingCommandsAreReadOnly
 * Description     : Confirm future-stage icons give an explicit message and cannot mutate the current project.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.11
 ******************************************************************************/
void Stage2WorkflowTests::nativeShellPendingCommandsAreReadOnly() //20261111-1 add
{
    MainWindow window;
    QJsonObject before;
    bool explained;

    window.setProject(workflowProject(), QString()); window.show();
    before = window.currentEditorProject().toJson(); explained = false;
    QTimer::singleShot(0, &window, [&explained]() {
        QMessageBox *message;
        message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (message != nullptr) { explained = message->text().contains(QStringLiteral("后续阶段")); message->accept(); }
    });
    window.m_commands.value(QStringLiteral("action_upload"))->trigger(); //20261114-6 change: Online simulation now has a specific capability explanation; upload remains pending.
    QVERIFY(explained); QCOMPARE(window.currentEditorProject().toJson(), before);
    QVERIFY(!window.m_dirty);
}

QTEST_MAIN(Stage2WorkflowTests)
#include "stage2_workflow_tests.moc"
