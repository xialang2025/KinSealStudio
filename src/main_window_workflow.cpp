#include "main_window.h"
#include "hmi_canvas.h"
#include "stage2_project_dialogs.h"
#include "stage2_global_tools.h"
#include "stage2_system_settings.h"
#include "stage3_resources.h" //20261110-8 add

#include <QAction>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QSettings>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QTabBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <algorithm>

namespace {
/******************************************************************************
 * Function Name   : projectContent
 * Description     : Compare document content independently of transient editor navigation.
 * Input Parameters: project
 * Return Value    : Canonical persisted content excluding current editor page/language selection.
 * Date            : 2026.11.10
 ******************************************************************************/
QJsonObject projectContent(const HmiProject &project) //20261110-7 add
{
    QJsonObject content;
    HmiProject normalized;
    QList<HmiPage>::iterator page;

    normalized = project;
    //20261110-7 add: Scene extraction follows stacking order; visiting a previously hidden page is not an edit.
    std::stable_sort(normalized.objects.begin(), normalized.objects.end(), [](const HmiObject &left, const HmiObject &right) {
        return left.layer < right.layer;
    });
    for (page = normalized.inactivePages.begin(); page != normalized.inactivePages.end(); ++page) {
        std::stable_sort(page->objects.begin(), page->objects.end(), [](const HmiObject &left, const HmiObject &right) {
            return left.layer < right.layer;
        });
    }
    content = normalized.toJson();
    content.remove(QStringLiteral("activePageId"));
    content.remove(QStringLiteral("activeLanguage"));
    if (content.value(QStringLiteral("schemaVersion")).toInt() >= 9) { content.remove(QStringLiteral("resolution")); }
    return content;
}
}

/******************************************************************************
 * Function Name   : MainWindow::currentEditorProject
 * Description     : Capture active canvas records together with every hidden page and project resource.
 * Input Parameters: None
 * Return Value    : A detached whole-project candidate.
 * Date            : 2026.11.10
 ******************************************************************************/
HmiProject MainWindow::currentEditorProject() const //20261110-7 add
{
    HmiProject project;

    project = m_project;
    if (m_hasProject && !m_canvas->hasActiveInteraction()) { project.objects = m_canvas->objects(); } //20261110-8 change: Read-only tools never persist a live geometry preview.
    return project;
}

/******************************************************************************
 * Function Name   : MainWindow::resetProjectHistory
 * Description     : Establish a clean whole-project undo root when a document is adopted.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::resetProjectHistory() //20261110-7 add
{
    m_projectHistory.clear();
    m_projectHistory.append(currentEditorProject());
    m_projectHistoryIndex = 0;
    m_savedContent = projectContent(currentEditorProject());
}

/******************************************************************************
 * Function Name   : MainWindow::recordProjectHistory
 * Description     : Commit one validated complete edit, preserving resources and dropping stale redo branches.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::recordProjectHistory() //20261110-7 add
{
    HmiProject candidate;

    if (m_restoringProject || !m_hasProject) { return; }
    candidate = currentEditorProject();
    if (!candidate.validateModel()) { return; }
    if (m_projectHistoryIndex >= 0 &&
        projectContent(m_projectHistory.at(m_projectHistoryIndex)) == projectContent(candidate)) {
        m_projectHistory[m_projectHistoryIndex] = candidate;
        return;
    }
    while (m_projectHistory.size() > m_projectHistoryIndex + 1) { m_projectHistory.removeLast(); }
    m_projectHistory.append(candidate);
    if (m_projectHistory.size() > 100) { m_projectHistory.removeFirst(); }
    m_projectHistoryIndex = m_projectHistory.size() - 1;
}

/******************************************************************************
 * Function Name   : MainWindow::projectIsModified
 * Description     : Resolve the saved checkpoint after undo, configuration changes or canvas edits.
 * Input Parameters: None
 * Return Value    : True only when persisted document content differs from the last successful save.
 * Date            : 2026.11.10
 ******************************************************************************/
bool MainWindow::projectIsModified() const //20261110-7 add
{
    return projectContent(currentEditorProject()) != m_savedContent;
}

/******************************************************************************
 * Function Name   : MainWindow::storeSavedCheckpoint
 * Description     : Advance the saved document checkpoint without discarding undo history.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::storeSavedCheckpoint() //20261110-7 add
{
    m_savedContent = projectContent(currentEditorProject());
}

/******************************************************************************
 * Function Name   : MainWindow::restoreProjectHistory
 * Description     : Restore page topology, object fields and shared resources as one undo transaction.
 * Input Parameters: delta
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::restoreProjectHistory(int delta) //20261110-7 add
{
    int index;
    QString path;

    index = m_projectHistoryIndex + delta;
    if (!m_hasProject || index < 0 || index >= m_projectHistory.size()) { return; }
    path = m_currentPath;
    m_restoringProject = true;
    m_projectHistoryIndex = index;
    setProject(m_projectHistory.at(index), path);
    m_restoringProject = false;
    setDirty(projectContent(m_project) != m_savedContent);
}

/******************************************************************************
 * Function Name   : MainWindow::commitProjectDraft
 * Description     : Validate a detached configuration or batch edit before replacing the live project once.
 * Input Parameters: project, error
 * Return Value    : True after an atomic in-memory commit.
 * Date            : 2026.11.10
 ******************************************************************************/
bool MainWindow::commitProjectDraft(const HmiProject &project, QString *error) //20261110-7 add
{
    QString path;
    QString selected;

    if (!m_hasProject || !project.validateModel(error)) { return false; }
    if (projectContent(currentEditorProject()) == projectContent(project) &&
        m_project.activePageId == project.activePageId && m_project.activeLanguage == project.activeLanguage) { return true; }
    path = m_currentPath;
    selected = m_selectedId;
    m_restoringProject = true;
    setProject(project, path);
    m_restoringProject = false;
    if (!selected.isEmpty()) { m_canvas->selectObject(selected); }
    setDirty(true);
    return true;
}

/******************************************************************************
 * Function Name   : MainWindow::openProjectPath
 * Description     : Parse a candidate before asking about unsaved work, then adopt it only after approval.
 * Input Parameters: path, error
 * Return Value    : True after replacing the current document; cancellation or failure leaves it intact.
 * Date            : 2026.11.10
 ******************************************************************************/
bool MainWindow::openProjectPath(const QString &path, QString *error) //20261110-7 add
{
    HmiProject candidate;
    bool native;

    if (!HmiProject::load(path, &candidate, error)) { return false; }
    if (!confirmDiscardChanges()) { return false; }
    //20261110-7 add: Saving unsaved work may replace this exact path; never adopt the stale pre-prompt candidate.
    if (!HmiProject::load(path, &candidate, error)) { return false; }
    native = QFileInfo(path).suffix().compare(QStringLiteral("sm"), Qt::CaseInsensitive) == 0;
    setProject(candidate, native ? QString() : path);
    updateRecentProjects(path);
    return true;
}

/******************************************************************************
 * Function Name   : MainWindow::closeProject
 * Description     : Close a document through the save/discard/cancel transaction and disable editing commands.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::closeProject() //20261110-7 add
{
    if (!m_hasProject || !confirmDiscardChanges()) { return; }
    m_restoringProject = true;
    setProject(HmiProject(), QString());
    m_restoringProject = false;
    m_hasProject = false;
    m_projectHistory.clear();
    m_projectHistoryIndex = -1;
    m_projectTree->clear();
    m_canvas->setEnabled(false);
    m_rightTabs->setEnabled(false);
    m_pageTabs->setEnabled(false);
    m_systemSettingsTree->setEnabled(false);
    setDirty(false);
}

/******************************************************************************
 * Function Name   : MainWindow::configureProject
 * Description     : Edit page topology and startup choices in a detached project dialog.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::configureProject() //20261110-7 add
{
    Stage2ProjectDialog dialog(currentEditorProject(), this);
    QString error;

    if (!m_hasProject) { return; }
    if (dialog.exec() == QDialog::Accepted && !commitProjectDraft(dialog.draftProject(), &error)) {
        QMessageBox::warning(this, QStringLiteral("工程设置未提交"), error);
    }
}

/******************************************************************************
 * Function Name   : MainWindow::activatePageId
 * Description     : Synchronize the active canvas before navigating by a stable page and optional object identity.
 * Input Parameters: pageId, objectId
 * Return Value    : True when the requested page is active.
 * Date            : 2026.11.10
 ******************************************************************************/
bool MainWindow::activatePageId(const QString &pageId, const QString &objectId) //20261110-7 add
{
    HmiProject candidate;
    QString error;

    if (!m_hasProject || pageId.isEmpty()) { return false; }
    if (pageId != m_project.activePageId) {
        candidate = currentEditorProject();
        if (!candidate.selectPage(pageId, &error)) {
            statusBar()->showMessage(error, 6000);
            return false;
        }
        m_project = candidate;
        m_selectedId.clear();
        m_canvas->setProject(m_project);
        fillProperties(HmiObject(), false);
        rebuildProjectTree();
    }
    if (!objectId.isEmpty()) { m_canvas->selectObject(objectId); }
    if (m_projectHistoryIndex >= 0 &&
        projectContent(m_projectHistory.at(m_projectHistoryIndex)) == projectContent(m_project)) {
        m_projectHistory[m_projectHistoryIndex] = m_project;
    }
    updateCommandStates();
    return true;
}

/******************************************************************************
 * Function Name   : MainWindow::configureSystemSettings
 * Description     : Commit the five recovered system properties as an explicit editor-only project transaction.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::configureSystemSettings() //20261110-7 add
{
    Stage2SystemSettingsDialog dialog(currentEditorProject(), this);
    QString error;
    if (!m_hasProject) { return; }
    if (dialog.exec() == QDialog::Accepted && !commitProjectDraft(dialog.draftProject(), &error)) {
        QMessageBox::warning(this, QStringLiteral("系统参数未提交"), error);
    }
}

/******************************************************************************
 * Function Name   : MainWindow::refreshPageNavigation
 * Description     : Keep actual page tabs and the toolbar selector synchronized with stable model identities.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::refreshPageNavigation() //20261110-7 add
{
    QList<HmiPage> pages;
    HmiPage page;
    QWidget *widget;
    bool rebuild;
    int index;
    int active;
    QSignalBlocker blocker(m_pageTabs);

    if (m_updatingNavigation) { return; }
    m_updatingNavigation = true;
    pages = m_project.allPages();
    active = m_project.pageOrder.indexOf(m_project.activePageId);
    rebuild = m_pageTabs->count() != pages.size() || active < 0 || m_pageTabs->widget(active) != m_canvas;
    for (index = 0; !rebuild && index < pages.size(); ++index) {
        rebuild = m_pageTabs->tabBar()->tabData(index).toString() != pages.at(index).id;
    }
    if (rebuild) {
        while (m_pageTabs->count() > 0) {
            widget = m_pageTabs->widget(0);
            m_pageTabs->removeTab(0);
            if (widget != m_canvas) { delete widget; }
        }
        for (index = 0; index < pages.size(); ++index) {
            widget = index == active ? static_cast<QWidget *>(m_canvas) : new QWidget(m_pageTabs);
            m_pageTabs->addTab(widget, pages.at(index).name);
            m_pageTabs->tabBar()->setTabData(index, pages.at(index).id);
        }
    }
    for (index = 0; index < pages.size(); ++index) { m_pageTabs->setTabText(index, pages.at(index).name); }
    if (active >= 0) { m_pageTabs->setCurrentIndex(active); }
    if (m_pageCombo != nullptr) {
        QSignalBlocker comboBlocker(m_pageCombo);
        m_pageCombo->clear();
        Q_FOREACH (page, pages) { m_pageCombo->addItem(page.name, page.id); }
        m_pageCombo->setCurrentIndex(active);
    }
    m_projectLabel->setText(QStringLiteral("%1 × %2").arg(m_project.pageWidth).arg(m_project.pageHeight));
    rebuildSystemSettings();
    m_updatingNavigation = false;
}

/******************************************************************************
 * Function Name   : MainWindow::rebuildSystemSettings
 * Description     : Replace static sample nodes with current pages, roles and qualified PLC connections.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::rebuildSystemSettings() //20261110-7 add
{
    QTreeWidgetItem *settings;
    QTreeWidgetItem *connections;
    QTreeWidgetItem *normal;
    QTreeWidgetItem *windows;
    QTreeWidgetItem *keyboards;
    QTreeWidgetItem *logos;
    QTreeWidgetItem *system;
    QTreeWidgetItem *item;
    QTreeWidgetItem *parent;
    HmiPage page;
    PlcConnection connection;
    QString label;
    int number;
    QSignalBlocker blocker(m_systemSettingsTree);

    if (m_systemSettingsTree == nullptr) { return; }
    m_systemSettingsTree->clear();
    //20261111-1 change: Match the original navigation hierarchy without creating unsupported configuration records.
    settings = new QTreeWidgetItem(m_systemSettingsTree, {QStringLiteral("设置")});
    connections = new QTreeWidgetItem(settings, {QStringLiteral("连接设置")});
    connections->setData(0, Qt::UserRole, QStringLiteral("connections"));
    for (number = 1; number <= 2; ++number)
    {
        item = new QTreeWidgetItem(connections, {QStringLiteral("COM%1").arg(number)});
        item->setData(0, Qt::UserRole, number == 1 ? QStringLiteral("connections") : QStringLiteral("unsupported"));
        if (number == 1)
        {
            parent = new QTreeWidgetItem(item, {QStringLiteral("Modbus_RTU")});
            parent->setData(0, Qt::UserRole, QStringLiteral("connections"));
        }
    }
    normal = new QTreeWidgetItem(settings, {QStringLiteral("画面")});
    windows = new QTreeWidgetItem(settings, {QStringLiteral("窗口")});
    keyboards = new QTreeWidgetItem(settings, {QStringLiteral("自定义键盘")});
    logos = new QTreeWidgetItem(settings, {QStringLiteral("开机Logo")});
    number = 0;
    Q_FOREACH (page, m_project.allPages())
    {
        parent = page.kind == QStringLiteral("normal") ? normal : page.kind == QStringLiteral("window") ? windows :
                 page.kind == QStringLiteral("keyboard") ? keyboards : logos;
        item = new QTreeWidgetItem(parent, {QStringLiteral("%1:%2").arg(number++, 3, 10, QLatin1Char('0')).arg(page.name)});
        item->setData(0, Qt::UserRole, QStringLiteral("page"));
        item->setData(0, Qt::UserRole + 1, page.id);
    }
    normal->setData(0, Qt::UserRole, QStringLiteral("project"));
    windows->setData(0, Qt::UserRole, QStringLiteral("project"));
    system = new QTreeWidgetItem(settings, {QStringLiteral("系统设置")});
    Q_FOREACH (label, (QStringList{QStringLiteral("参数设置"), QStringLiteral("HMI状态"), QStringLiteral("PLC控制"),
        QStringLiteral("时钟"), QStringLiteral("文件保护"), QStringLiteral("HMI保护"), QStringLiteral("语言设置")}))
    {
        item = new QTreeWidgetItem(system, {label});
        item->setData(0, Qt::UserRole, label == QStringLiteral("参数设置") ? QStringLiteral("system") :
                      label == QStringLiteral("语言设置") ? QStringLiteral("languages") : QStringLiteral("unsupported"));
    }
    item = new QTreeWidgetItem(settings, {QStringLiteral("数据采集")});
    item->setData(0, Qt::UserRole, QStringLiteral("unsupported"));
    parent = new QTreeWidgetItem(settings, {QStringLiteral("报警登录")});
    Q_FOREACH (label, (QStringList{QStringLiteral("数位地址报警"), QStringLiteral("条件报警")}))
    {
        item = new QTreeWidgetItem(parent, {label}); item->setData(0, Qt::UserRole, QStringLiteral("unsupported"));
    }
    parent = new QTreeWidgetItem(settings, {QStringLiteral("宏指令")});
    Q_FOREACH (label, (QStringList{QStringLiteral("全局宏"), QStringLiteral("初始化宏"), QStringLiteral("宏指令编辑器")}))
    {
        item = new QTreeWidgetItem(parent, {label}); item->setData(0, Qt::UserRole, QStringLiteral("macros"));
    }
    Q_FOREACH (label, (QStringList{QStringLiteral("配方"), QStringLiteral("资料传输")}))
    {
        item = new QTreeWidgetItem(settings, {label}); item->setData(0, Qt::UserRole, QStringLiteral("unsupported"));
    }
    decorateNativeSystemTree(); //20261111-1 add
    m_systemSettingsTree->setEnabled(m_hasProject);
    m_systemSettingsTree->expandAll();
    connections->child(0)->setExpanded(false); //20261111-1 add: Preserve the native initial COM1 expansion state.
}

/******************************************************************************
 * Function Name   : MainWindow::dispatchEditCommand
 * Description     : Route text-edit commands to their focused editor and canvas commands through project history.
 * Input Parameters: command
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::dispatchEditCommand(const QString &command) //20261110-7 add
{
    QWidget *focus;
    QLineEdit *line;
    QPlainTextEdit *plain;
    QTextEdit *rich;
    QTextCursor cursor;
    QByteArray method;
    HmiProject candidate; //20261110-8 add
    Stage3Clipboard clipboard;
    QStringList newIds;
    QString error;
    bool accepted;
    QString referenceId; //20261110-8 add
    HmiObject reference;

    focus = QApplication::focusWidget();
    if (qobject_cast<QAbstractSpinBox *>(focus) != nullptr) {
        focus = focus->findChild<QLineEdit *>(); //20261110-7 add: A numeric property draft owns its own text undo and clipboard.
    }
    line = qobject_cast<QLineEdit *>(focus);
    plain = qobject_cast<QPlainTextEdit *>(focus);
    rich = qobject_cast<QTextEdit *>(focus);
    if ((line != nullptr || plain != nullptr || rich != nullptr) &&
        (command == QStringLiteral("cut") || command == QStringLiteral("copy") || command == QStringLiteral("paste") ||
         command == QStringLiteral("selectAll") || command == QStringLiteral("undo") || command == QStringLiteral("redo") ||
         command == QStringLiteral("delete"))) {
        if (command == QStringLiteral("delete")) { //20261110-7 add: Menu Delete must not delete a selected control while editing text.
            if ((line != nullptr && line->isReadOnly()) || (plain != nullptr && plain->isReadOnly()) ||
                (rich != nullptr && rich->isReadOnly())) { return; } //20261110-7 add: Cursor APIs must honor a read-only macro/source view.
            if (line != nullptr) { line->del(); }
            else {
                cursor = plain != nullptr ? plain->textCursor() : rich->textCursor();
                cursor.deleteChar();
            }
            updateCommandStates();
            return;
        }
        method = command.toLatin1();
        QMetaObject::invokeMethod(focus, method.constData(), Qt::DirectConnection);
        updateCommandStates();
        return;
    }
    if (!m_hasProject) { return; }
    m_canvas->cancelInteraction(); //20261110-8 add: Editing commands consume committed geometry, never an in-flight move or resize preview.
    if (command == QStringLiteral("undo")) { restoreProjectHistory(-1); }
    else if (command == QStringLiteral("redo")) { restoreProjectHistory(1); }
    else if (command == QStringLiteral("copy")) { //20261110-8 add: Preserve all resource identities and inactive property drafts.
        if (!captureStage3Selection(currentEditorProject(), m_canvas->selectedObjectIds(), &m_objectClipboard, &error)) {
            statusBar()->showMessage(error, 8000);
        }
    } else if (command == QStringLiteral("cut") || command == QStringLiteral("delete")) {
        accepted = command == QStringLiteral("cut") ?
            cutStage3Selection(currentEditorProject(), m_canvas->selectedObjectIds(), &clipboard, &candidate, &error) :
            deleteStage3Selection(currentEditorProject(), m_canvas->selectedObjectIds(), &candidate, &error);
        if (accepted) { accepted = commitProjectDraft(candidate, &error); }
        if (accepted && command == QStringLiteral("cut")) { m_objectClipboard = clipboard; }
        if (!accepted) { statusBar()->showMessage(error, 8000); }
    } else if (command == QStringLiteral("paste")) {
        // Sixteen page pixels is the existing editor paste offset, independent of viewport zoom or DPI.
        if (pasteStage3Selection(currentEditorProject(), m_objectClipboard, QPoint(16, 16), &candidate, &newIds, &error) &&
            commitProjectDraft(candidate, &error)) { m_canvas->selectObjects(newIds); }
        else { statusBar()->showMessage(error, 8000); }
    } else if (command == QStringLiteral("multiCopy")) {
        Stage3DuplicateDialog dialog(currentEditorProject(), m_canvas->selectedObjectIds(), this);
        if (dialog.exec() == QDialog::Accepted) {
            if (commitProjectDraft(dialog.draftProject(), &error)) { m_canvas->selectObjects(dialog.newObjectIds()); }
            else { statusBar()->showMessage(error, 8000); }
        }
    }
    else {
        referenceId = m_canvas->arrangementReferenceObjectId(command); //20261110-8 add: Make the observed scene-order anchor visible to the editor user.
        m_canvas->executeEdit(command);
        Q_FOREACH (reference, m_project.objects) {
            if (!referenceId.isEmpty() && reference.id == referenceId) {
                statusBar()->showMessage(QStringLiteral("本次对齐参考：%1").arg(reference.name), 8000);
                break;
            }
        }
    }
    updateCommandStates();
}

/******************************************************************************
 * Function Name   : MainWindow::updateCommandStates
 * Description     : Maintain a single enabled truth table for menus, toolbars, focused inputs and document lifetime.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::updateCommandStates() //20261110-7 add
{
    QMap<QString, QAction *>::const_iterator it;
    QString id;
    QString command;
    bool enabled;
    bool textEditor;
    QWidget *focus;

    focus = QApplication::focusWidget();
    textEditor = qobject_cast<QLineEdit *>(focus) != nullptr || qobject_cast<QPlainTextEdit *>(focus) != nullptr ||
                 qobject_cast<QTextEdit *>(focus) != nullptr || qobject_cast<QAbstractSpinBox *>(focus) != nullptr;
    for (it = m_commands.constBegin(); it != m_commands.constEnd(); ++it) {
        id = it.key();
        command = it.value()->property("editCommand").toString();
        enabled = m_hasProject;
        if (id == QStringLiteral("action_new_pro") || id == QStringLiteral("action_open_pro") ||
            id == QStringLiteral("action_quit") || id == QStringLiteral("action_H") || id == QStringLiteral("action_KinSeal") ||
            id == QStringLiteral("action_42") || id == QStringLiteral("action_tool") || id == QStringLiteral("action_40")) {
            enabled = true;
        } else if (id == QStringLiteral("action_save")) { enabled = m_hasProject && m_dirty; }
        else if (id == QStringLiteral("action_USBdownload")) {
            enabled = m_hasProject && m_downloadProcess == nullptr; //20261110-7 change: Cover Starting and finalization as well as Running.
        } else if (!command.isEmpty()) {
            enabled = m_hasProject && (command == QStringLiteral("undo") ? m_projectHistoryIndex > 0 :
                command == QStringLiteral("redo") ? m_projectHistoryIndex + 1 < m_projectHistory.size() :
                command == QStringLiteral("paste") ? !m_objectClipboard.objectIds.isEmpty() :
                m_canvas->canExecuteEdit(command));
            if (textEditor && (command == QStringLiteral("copy") || command == QStringLiteral("cut") ||
                command == QStringLiteral("paste") || command == QStringLiteral("selectAll") ||
                command == QStringLiteral("undo") || command == QStringLiteral("redo") || command == QStringLiteral("delete"))) { enabled = true; }
        }
        it.value()->setEnabled(enabled);
    }
    if (m_languageCombo != nullptr) { m_languageCombo->setEnabled(m_hasProject); } //20261110-7 add
    if (m_pageCombo != nullptr) { m_pageCombo->setEnabled(m_hasProject); }
}

/******************************************************************************
 * Function Name   : MainWindow::manageProjectPictures
 * Description     : Publish imported, renamed or replaced image bytes and their references as one reversible document edit.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::manageProjectPictures() //20261110-8 add
{
    Stage3PicturesDialog dialog(currentEditorProject(), this);
    QString error;
    if (!m_hasProject) { return; }
    if (dialog.exec() == QDialog::Accepted && !commitProjectDraft(dialog.draftProject(), &error)) {
        QMessageBox::warning(this, QStringLiteral("工程图片修改未提交"), error);
    }
}

/******************************************************************************
 * Function Name   : MainWindow::showGlobalTool
 * Description     : Open a read-only cross-page index and navigate only after a stable result is accepted.
 * Input Parameters: kind
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::showGlobalTool(int kind) //20261110-7 add
{
    Stage2GlobalTools::GlobalToolsDialog dialog(currentEditorProject(), static_cast<Stage2GlobalTools::Kind>(kind), this);
    Stage2GlobalTools::Location location;
    QStringList selected;
    QSet<QString> ids;
    QString id;

    if (!m_hasProject) { return; }
    selected = m_canvas->selectedObjectIds();
    Q_FOREACH (id, selected) { ids.insert(id); }
    dialog.setSelectedObjectIds(ids);
    if (dialog.exec() == QDialog::Accepted) {
        location = dialog.selectedLocation();
        if (!location.pageId.isEmpty()) { activatePageId(location.pageId, location.objectId); }
        else if (!location.macroName.isEmpty()) {
            showMacroSource(location.macroName, location.line);
        }
    }
}

/******************************************************************************
 * Function Name   : MainWindow::showMacroSource
 * Description     : Open a read-only macro definition at a global search result without changing compilation state.
 * Input Parameters: name, line
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::showMacroSource(const QString &name, int line) //20261110-7 add
{
    QDialog dialog(this);
    QVBoxLayout layout(&dialog);
    QPlainTextEdit source(&dialog);
    QDialogButtonBox buttons(QDialogButtonBox::Close, &dialog);
    QTextCursor cursor;
    QTextBlock block;
    if (!m_project.macros.contains(name)) { return; }
    dialog.setObjectName(QStringLiteral("Stage2MacroSourceDialog"));
    dialog.setWindowTitle(QStringLiteral("宏源码：%1，第%2行").arg(name).arg(line));
    dialog.resize(780, 540);
    source.setObjectName(QStringLiteral("Stage2MacroSource"));
    source.setReadOnly(true);
    source.setPlainText(m_project.macros.value(name));
    block = source.document()->findBlockByLineNumber(qBound(0, line - 1, source.document()->blockCount() - 1));
    cursor = QTextCursor(block);
    source.setTextCursor(cursor);
    source.ensureCursorVisible();
    layout.addWidget(&source);
    layout.addWidget(&buttons);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.exec();
}

/******************************************************************************
 * Function Name   : MainWindow::replaceProjectAddresses
 * Description     : Publish a validated address-preview candidate as one undoable project transaction.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::replaceProjectAddresses() //20261110-7 add
{
    Stage2GlobalTools::AddressReplaceDialog dialog(currentEditorProject(), this);
    HmiProject candidate;
    QString error;
    QStringList selected;
    QString id;
    QSet<QString> ids;

    if (!m_hasProject) { return; }
    selected = m_canvas->selectedObjectIds();
    Q_FOREACH (id, selected) { ids.insert(id); }
    dialog.setSelectedObjectIds(ids);
    if (dialog.exec() == QDialog::Accepted && dialog.candidateProject(&candidate) &&
        !commitProjectDraft(candidate, &error)) {
        QMessageBox::warning(this, QStringLiteral("地址替换未提交"), error);
    }
}

/******************************************************************************
 * Function Name   : MainWindow::updateRecentProjects
 * Description     : Persist up to ten exact project paths and rebuild actionable recent-file entries.
 * Input Parameters: path
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::updateRecentProjects(const QString &path) //20261110-7 add
{
    QSettings settings(QStringLiteral("KinSealStudioLite"), QStringLiteral("Editor"));
    QStringList paths;
    QString stored;
    QAction *action;
    int index;

    paths = settings.value(QStringLiteral("recentProjects")).toStringList();
    if (!path.isEmpty()) {
        stored = QFileInfo(path).absoluteFilePath();
        paths.removeAll(stored);
        paths.prepend(stored);
        while (paths.size() > 10) { paths.removeLast(); }
        settings.setValue(QStringLiteral("recentProjects"), paths);
    }
    if (m_recentProjectsMenu == nullptr) { return; }
    m_recentProjectsMenu->clear();
    for (index = 0; index < paths.size(); ++index) {
        stored = paths.at(index);
        action = m_recentProjectsMenu->addAction(stored);
        action->setObjectName(QStringLiteral("recentProject%1").arg(index));
        connect(action, &QAction::triggered, this, [this, stored]() {
            QString failure;
            if (!openProjectPath(stored, &failure) && !failure.isEmpty()) {
                QMessageBox::warning(this, QStringLiteral("打开最近工程失败"), failure);
            }
        });
    }
    m_recentProjectsMenu->setEnabled(!paths.isEmpty());
}
