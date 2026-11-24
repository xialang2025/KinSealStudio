#include "main_window.h"
#include "dialog_button_text.h" //20261124-8 add
#include "hmi_canvas.h"
#include "local_help.h" //20261122-4 add
#include "stage2_file_protection.h" //20261123-17 add
#include "stage2_io_settings.h" //20261123-15 add
#include "stage2_clock_panel.h" //20261123-16 add
#include "plc_address_dialog.h" //20261122-6 add
#include "stage2_project_dialogs.h"
#include "stage2_keyboard_dialogs.h" //20261123-24 add
#include "stage2_global_tools.h"
#include "stage2_system_settings.h"
#include "stage3_resources.h" //20261110-8 add

#include <QAction>
#include <QAbstractButton> //20261123-17 add
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QCryptographicHash> //20261123-17 add
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
#include <QUuid>
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
    if (!project.filePassword.isEmpty()) //20261123-17 add: Detect access-policy changes for undo and dirty tracking without serializing the password.
    {
        content.insert(QStringLiteral("fileProtectionState"), QString::fromLatin1(
            QCryptographicHash::hash(project.filePassword.toUtf8(), QCryptographicHash::Sha256).toHex()));
    }
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
    QString password; //20261123-17 add

    auto loadCandidate = [&]() {
        QString failure;
        bool passwordRequired;
        bool entered = !password.isEmpty();

        while (!HmiProject::load(path, &candidate, &failure, password, &passwordRequired))
        {
            if (!passwordRequired)
            {
                if (error != nullptr)
                {
                    *error = failure;
                }
                return false;
            }
            if (entered)
            {
                QMessageBox warning(QMessageBox::Warning, QStringLiteral("KinSealStudio"), failure,
                                    QMessageBox::Ok, this);

                warning.button(QMessageBox::Ok)->setText(dialogButtonText(QStringLiteral("确定"), "OK"));
                warning.exec();
            }
            if (!requestStage2FilePassword(this, &password))
            {
                if (error != nullptr)
                {
                    error->clear();
                }
                return false;
            }
            entered = true;
        }
        return true;
    };
    if (!loadCandidate()) { return false; }
    if (!confirmDiscardChanges()) { return false; }
    //20261110-7 add: Saving unsaved work may replace this exact path; never adopt the stale pre-prompt candidate.
    if (!loadCandidate()) { return false; }
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
 * Function Name   : MainWindow::createNormalPage
 * Description     : Publish a new page as one editor transaction after the settings dialog accepts.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void MainWindow::createNormalPage() //20261122-9 add
{
    HmiProject draft;
    QString error;

    if (!m_hasProject)
    {
        return;
    }
    draft = currentEditorProject();
    if (editStage2NewNormalPage(this, draft) && !commitProjectDraft(draft, &error))
    {
        QMessageBox::warning(this, QStringLiteral("新建画面未提交"), error);
    }
}

/******************************************************************************
 * Function Name   : MainWindow::configureWindowPage
 * Description     : Open a popup editor and publish its accepted transaction.
 * Input Parameters: id (empty for creation).
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void MainWindow::configureWindowPage(const QString &id) //20261123-12 add
{
    HmiProject draft;
    QString error;

    if (!m_hasProject)
    {
        return;
    }
    draft = currentEditorProject();
    if (editStage2Window(this, draft, id) && !commitProjectDraft(draft, &error))
    {
        QMessageBox::warning(this, QStringLiteral("窗口未提交"), error);
    }
}

/******************************************************************************
 * Function Name   : MainWindow::configureKeyboardPage
 * Description     : Publish an accepted keyboard creation or properties edit as one history entry.
 * Input Parameters: id (empty for creation).
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void MainWindow::configureKeyboardPage(const QString &id) //20261123-24 add
{
    HmiProject draft;
    QString error;
    bool accepted;

    if (!m_hasProject)
    {
        return;
    }
    draft = currentEditorProject();
    accepted = id.isEmpty() ? addStage2Keyboards(this, draft) : editStage2KeyboardProperties(this, draft, id);
    if (accepted && !commitProjectDraft(draft, &error))
    {
        QMessageBox::warning(this, QStringLiteral("自定义键盘未提交"), error);
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
 * Description     : Publish one settings draft and open project help in the same dialog context.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::configureSystemSettings() //20261122-4 change
{
    Stage2SystemSettingsDialog dialog(currentEditorProject(), this);
    QString error;
    if (!m_hasProject) { return; }
    connect(&dialog, &Stage2SystemSettingsDialog::helpRequested, this, [this, &dialog]() {
        m_localHelp->showTopic(QStringLiteral("project"), &dialog);
    });
    if (dialog.exec() == QDialog::Accepted && !commitProjectDraft(dialog.draftProject(), &error)) {
        QMessageBox::warning(this, QStringLiteral("系统参数未提交"), error);
    }
}

/******************************************************************************
 * Function Name   : MainWindow::configureExtendedSystemSetting
 * Description     : Publish one confirmed system dialog as an atomic editor transaction.
 * Input Parameters: kind, label
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void MainWindow::configureExtendedSystemSetting(const QString &kind, const QString &label) //20261123-15 add
{
    HmiProject draft = currentEditorProject();
    HmiConfigurationNode node;
    QJsonObject defaults;
    QJsonObject properties;
    QString error;
    int index;
    bool accepted;
    const bool io = kind == QStringLiteral("hmiStatus") || kind == QStringLiteral("plcControl");

    if (!m_hasProject)
    {
        return;
    }
    if (kind == QStringLiteral("fileProtection"))
    {
        if (editStage2FileProtection(this, draft.filePassword, &draft.filePassword) && !commitProjectDraft(draft, &error))
        {
            QMessageBox::warning(this, label, error);
        }
        return;
    }
    defaults = io ? defaultStage2IoSettings(kind) : defaultStage2ClockPanel(kind);
    for (index = 0; index < draft.configurationNodes.size(); ++index)
    {
        if (draft.configurationNodes.at(index).kind == kind)
        {
            node = draft.configurationNodes.at(index);
            break;
        }
    }
    properties = node.id.isEmpty() ? defaults : node.properties;
    accepted = io ? editStage2IoSettings(this, draft, kind, properties, &properties) :
        editStage2ClockPanel(this, draft, kind, properties, &properties);
    if (!accepted)
    {
        return;
    }
    if (properties == defaults)
    {
        if (index < draft.configurationNodes.size())
        {
            draft.configurationNodes.removeAt(index);
        }
    }
    else
    {
        node.id = QStringLiteral("system-") + kind;
        node.kind = kind;
        node.name = label;
        node.implemented = true;
        node.active = io ? stage2IoSettingsActive(kind, properties) : stage2ClockPanelActive(kind, properties);
        node.properties = properties;
        if (index < draft.configurationNodes.size())
        {
            draft.configurationNodes[index] = node;
        }
        else
        {
            draft.configurationNodes.append(node);
        }
    }
    if (!commitProjectDraft(draft, &error))
    {
        QMessageBox::warning(this, label, error);
    }
}

/******************************************************************************
 * Function Name   : MainWindow::configureSerialPort
 * Description     : Commit a single device port's settings alongside independent PC port selection.
 * Input Parameters: port (COM1 or COM2).
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void MainWindow::configureSerialPort(const QString &port) //20261122-6 add
{
    HmiProject draft;
    HmiConfigurationNode node;
    QJsonObject settings;
    QJsonObject defaults;
    QString notice;
    QString id;
    QString error;
    qsizetype index;

    if (!m_hasProject || (port != QStringLiteral("COM1") && port != QStringLiteral("COM2")))
    {
        return;
    }
    draft = currentEditorProject();
    id = QStringLiteral("serial-port-") + port.toLower();
    defaults = PlcSerialPortSettings::defaults(port);
    settings = defaults;
    for (index = 0; index < draft.configurationNodes.size(); ++index)
    {
        node = draft.configurationNodes.at(index);
        if (node.id == id && node.kind == QStringLiteral("serialPort"))
        {
            settings = node.properties;
            break;
        }
    }
    if (!draft.nativeSourceSnapshot.isEmpty())
    {
        notice = QStringLiteral("原厂源串口参数尚未完整解析。此处展示 Lite 默认值，原厂配置会保留。");
    }
    if (!editPlcSerialPort(this, port, &settings, notice))
    {
        return;
    }
    if (settings == defaults)
    {
        // Removing the explicit override restores the original compile eligibility.
        if (index < draft.configurationNodes.size())
        {
            draft.configurationNodes.removeAt(index);
        }
    }
    else if (index < draft.configurationNodes.size())
    {
        draft.configurationNodes[index].properties = settings;
    }
    else
    {
        node = HmiConfigurationNode();
        node.id = id;
        node.name = port;
        node.kind = QStringLiteral("serialPort");
        node.active = true;
        node.implemented = true;
        node.properties = settings;
        draft.configurationNodes.append(node);
    }
    if (!commitProjectDraft(draft, &error))
    {
        QMessageBox::warning(this, QStringLiteral("连接设置未提交"), error);
    }
}

/******************************************************************************
 * Function Name   : MainWindow::configureProtocol
 * Description     : Edit the chosen protocol by ID and protect references from driver changes.
 * Input Parameters: id.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void MainWindow::configureProtocol(const QString &id) //20261122-7 add
{
    HmiProject draft;
    HmiProject referenceCheck;
    QVector<PlcConnection>::iterator record;
    PlcConnection candidate;
    QString error;

    if (!m_hasProject)
    {
        return;
    }
    draft = currentEditorProject();
    record = std::find_if(draft.connections.begin(), draft.connections.end(), [&id](const PlcConnection &entry) {
        return entry.id == id;
    });
    if (record == draft.connections.end())
    {
        return;
    }
    candidate = *record;
    if (!editPlcProtocol(this, draft.targetModel, &candidate, !draft.nativeSourceSnapshot.isEmpty(), &error))
    {
        if (!error.isEmpty())
        {
            QMessageBox::warning(this, QStringLiteral("协议窗口未打开"), error);
        }
        return;
    }
    if (candidate.profileId != record->profileId)
    {
        // Validate without this ID before allowing a different driver to reuse it.
        referenceCheck = draft;
        referenceCheck.connections.removeAt(record - draft.connections.begin());
        if (!referenceCheck.validateModel(&error))
        {
            QMessageBox::warning(this, QStringLiteral("协议仍被使用"), error);
            return;
        }
    }
    *record = candidate;
    if (!commitProjectDraft(draft, &error))
    {
        QMessageBox::warning(this, QStringLiteral("协议未提交"), error);
    }
}

/******************************************************************************
 * Function Name   : MainWindow::addProtocol
 * Description     : Append a separately identified protocol draft to the chosen port.
 * Input Parameters: port (COM1 or COM2).
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void MainWindow::addProtocol(const QString &port) //20261122-7 add
{
    HmiProject draft;
    PlcConnection connection;
    QString error;

    if (!m_hasProject || !m_project.nativeSourceSnapshot.isEmpty() ||
        (port != QStringLiteral("COM1") && port != QStringLiteral("COM2")))
    {
        return;
    }
    draft = currentEditorProject();
    connection.id = QStringLiteral("plc-") + QUuid::createUuid().toString(QUuid::WithoutBraces);
    connection.channel = port == QStringLiteral("COM1") ? PlcChannel::Com1 : PlcChannel::Com2;
    if (!editPlcProtocol(this, draft.targetModel, &connection, false, &error))
    {
        if (!error.isEmpty())
        {
            QMessageBox::warning(this, QStringLiteral("协议窗口未打开"), error);
        }
        return;
    }
    draft.connections.append(connection);
    if (!commitProjectDraft(draft, &error))
    {
        QMessageBox::warning(this, QStringLiteral("协议未提交"), error);
    }
}

/******************************************************************************
 * Function Name   : MainWindow::deleteProtocol
 * Description     : Remove a single protocol only after checking all project references.
 * Input Parameters: id.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void MainWindow::deleteProtocol(const QString &id) //20261122-7 add
{
    HmiProject draft;
    QVector<PlcConnection>::iterator record;
    QString error;

    if (!m_hasProject || !m_project.nativeSourceSnapshot.isEmpty())
    {
        return;
    }
    draft = currentEditorProject();
    record = std::find_if(draft.connections.begin(), draft.connections.end(), [&id](const PlcConnection &entry) {
        return entry.id == id;
    });
    if (record == draft.connections.end())
    {
        return;
    }
    draft.connections.erase(record);
    if (!commitProjectDraft(draft, &error))
    {
        QMessageBox::warning(this, QStringLiteral("协议仍被使用"), error);
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
    QMap<QString, int> keyboardNumbers; //20261123-24 add
    QTreeWidgetItem *logos;
    QTreeWidgetItem *system;
    QTreeWidgetItem *item;
    QTreeWidgetItem *parent;
    HmiPage page;
    HmiConfigurationNode collection; //20261112-1 add
    QTreeWidgetItem *collectionItem;
    PlcConnection connection;
    PlcCatalog catalog;
    const PlcProfile *profile;
    PlcChannel channel;
    QString label;
    QMap<QString, int> screenAndWindowNumbers; //20261123-13 change
    int number;
    QSignalBlocker blocker(m_systemSettingsTree);

    if (m_systemSettingsTree == nullptr) { return; }
    m_systemSettingsTree->clear();
    //20261111-1 change: Match the original navigation hierarchy without creating unsupported configuration records.
    settings = new QTreeWidgetItem(m_systemSettingsTree, {QStringLiteral("设置")});
    connections = new QTreeWidgetItem(settings, {QStringLiteral("连接设置")});
    connections->setData(0, Qt::UserRole, QStringLiteral("connections"));
    if (!m_project.connections.isEmpty())
    {
        catalog.load(); //20261122-6 add: Resolve protocol labels without altering the saved connection identity.
    }
    for (number = 1; number <= 2; ++number)
    {
        item = new QTreeWidgetItem(connections, {QStringLiteral("COM%1").arg(number)});
        item->setData(0, Qt::UserRole, QStringLiteral("serialPort")); //20261122-6 change: The device port owns its settings even without a PLC protocol.
        item->setData(0, Qt::UserRole + 1, item->text(0));
        channel = number == 1 ? PlcChannel::Com1 : PlcChannel::Com2;
        Q_FOREACH (connection, m_project.connections)
        {
            if (connection.channel != channel)
            {
                continue;
            }
            profile = catalog.profile(connection.profileId);
            label = connection.displayName; //20261122-7 change: Names can change without renaming the bound ID.
            if (label.isEmpty())
            {
                label = profile != nullptr ? profile->name : QFileInfo(connection.profileId).completeBaseName();
            }
            parent = new QTreeWidgetItem(item, {label.isEmpty() ? connection.profileId : label});
            parent->setData(0, Qt::UserRole, QStringLiteral("plcProtocol")); //20261122-7 change
            parent->setData(0, Qt::UserRole + 1, connection.id);
        }
    }
    normal = new QTreeWidgetItem(settings, {QStringLiteral("画面")});
    windows = new QTreeWidgetItem(settings, {QStringLiteral("窗口")});
    keyboards = new QTreeWidgetItem(settings, {QStringLiteral("自定义键盘")});
    logos = new QTreeWidgetItem(settings, {QStringLiteral("开机Logo")});
    screenAndWindowNumbers = m_project.screenAndWindowNumbers(); //20261123-13 change
    keyboardNumbers = m_project.keyboardNumbers(); //20261123-24 add
    number = 0;
    Q_FOREACH (page, m_project.allPages())
    {
        parent = page.kind == QStringLiteral("normal") ? normal : page.kind == QStringLiteral("window") ? windows :
                 page.kind == QStringLiteral("keyboard") ? keyboards : logos;
        item = new QTreeWidgetItem(parent, {QStringLiteral("%1:%2") //20261123-12 change
            .arg(keyboardNumbers.value(page.id, screenAndWindowNumbers.value(page.id, number)),
                3, 10, QLatin1Char('0')).arg(page.name)}); //20261123-24 change
        ++number;
        item->setData(0, Qt::UserRole, QStringLiteral("page"));
        item->setData(0, Qt::UserRole + 1, page.id);
    }
    normal->setData(0, Qt::UserRole, QStringLiteral("normalPages")); //20261122-9 change: Route the ordinary page group to its own context actions.
    windows->setData(0, Qt::UserRole, QStringLiteral("windowPages")); //20261123-12 change
    keyboards->setData(0, Qt::UserRole, QStringLiteral("keyboardPages")); //20261123-24 add
    system = new QTreeWidgetItem(settings, {QStringLiteral("系统设置")});
    item = new QTreeWidgetItem(system, {QStringLiteral("参数设置")});
    item->setData(0, Qt::UserRole, QStringLiteral("system"));
    for (const auto &entry : (QList<QPair<QString, QString>>{
        {QStringLiteral("HMI状态"), QStringLiteral("hmiStatus")},
        {QStringLiteral("PLC控制"), QStringLiteral("plcControl")},
        {QStringLiteral("时钟"), QStringLiteral("clockSettings")},
        {QStringLiteral("文件保护"), QStringLiteral("fileProtection")},
        {QStringLiteral("HMI保护"), QStringLiteral("panelProtection")}})) //20261123-15 add
    {
        item = new QTreeWidgetItem(system, {entry.first});
        item->setData(0, Qt::UserRole, QStringLiteral("extendedSystemSetting"));
        item->setData(0, Qt::UserRole + 1, entry.second);
    }
    item = new QTreeWidgetItem(system, {QStringLiteral("语言设置")});
    item->setData(0, Qt::UserRole, QStringLiteral("languages"));
    item = new QTreeWidgetItem(settings, {QStringLiteral("数据采集")});
    item->setData(0, Qt::UserRole, QStringLiteral("dataCollection")); //20261112-1 change
    Q_FOREACH (collection, m_project.configurationNodes)
    {
        if (collection.kind != QStringLiteral("dataCollection")) { continue; }
        collectionItem = new QTreeWidgetItem(item, {collection.name}); collectionItem->setData(0, Qt::UserRole, QStringLiteral("dataCollection")); collectionItem->setData(0, Qt::UserRole + 1, collection.id);
    }
    parent = new QTreeWidgetItem(settings, {QStringLiteral("报警登录")});
    Q_FOREACH (label, (QStringList{QStringLiteral("数位地址报警"), QStringLiteral("条件报警")}))
    {
        item = new QTreeWidgetItem(parent, {label}); item->setData(0, Qt::UserRole, label == QStringLiteral("数位地址报警") ? QStringLiteral("alarm-bit") : QStringLiteral("alarm-condition")); //20261113-2 change
    }
    parent = new QTreeWidgetItem(settings, {QStringLiteral("宏指令")});
    Q_FOREACH (label, (QStringList{QStringLiteral("全局宏"), QStringLiteral("初始化宏"), QStringLiteral("宏指令编辑器")}))
    {
        item = new QTreeWidgetItem(parent, {label}); item->setData(0, Qt::UserRole, QStringLiteral("macros"));
    }
    Q_FOREACH (label, (QStringList{QStringLiteral("配方"), QStringLiteral("资料传输")}))
    {
        item = new QTreeWidgetItem(settings, {label}); item->setData(0, Qt::UserRole, label == QStringLiteral("配方") ? QStringLiteral("recipe") : QStringLiteral("unsupported")); //20261113-2 change
        if (label == QStringLiteral("配方")) { Q_FOREACH (collection, m_project.configurationNodes) { if (collection.kind != QStringLiteral("recipe") || !collection.active || !collection.implemented) { continue; } collectionItem = new QTreeWidgetItem(item, {collection.name}); collectionItem->setData(0, Qt::UserRole, QStringLiteral("recipe")); collectionItem->setData(0, Qt::UserRole + 1, collection.id); } }
    }
    decorateNativeSystemTree(); //20261111-1 add
    m_systemSettingsTree->setEnabled(m_hasProject);
    m_systemSettingsTree->expandAll();
    connections->child(0)->setExpanded(false); //20261111-1 add: Preserve the native initial COM1 expansion state.
}

/******************************************************************************
 * Function Name   : MainWindow::duplicateCanvasObject
 * Description     : Copy one canvas control through the validated project editing path.
 * Input Parameters: id, offset
 * Return Value    : None
 * Date            : 2026.11.24
 ******************************************************************************/
void MainWindow::duplicateCanvasObject(const QString &id, const QPoint &offset) //20261124-11 add
{
    HmiProject project;
    Stage3Clipboard selection;
    HmiProject candidate;
    QStringList newIds;
    QString error;

    if (!m_hasProject) { return; }
    project = currentEditorProject();
    // Use a private snapshot so the user's Copy and Paste commands keep their clipboard.
    if (captureStage3Selection(project, QStringList{id}, &selection, &error) &&
        pasteStage3Selection(project, selection, offset, &candidate, &newIds, &error) &&
        commitProjectDraft(candidate, &error))
    {
        m_canvas->selectObjects(newIds);
    }
    else if (!error.isEmpty())
    {
        statusBar()->showMessage(error, 5000);
    }
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
    else if (command == QStringLiteral("properties")) //20261124-0 add
    {
        if (!m_canvas->selectedObjectIds().isEmpty())
        {
            if (m_canvas->canExecuteEdit(command))
            {
                m_canvas->executeEdit(command);
            }
        }
        else if (m_project.activePageKind == QStringLiteral("normal"))
        {
            candidate = currentEditorProject();
            if (editStage2NormalPageProperties(this, candidate, candidate.activePageId) &&
                !commitProjectDraft(candidate, &error))
            {
                QMessageBox::warning(this, QStringLiteral("画面属性未提交"), error);
            }
        }
        else if (m_project.activePageKind == QStringLiteral("window"))
        {
            configureWindowPage(m_project.activePageId);
        }
        else if (m_project.activePageKind == QStringLiteral("keyboard"))
        {
            configureKeyboardPage(m_project.activePageId);
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
            id == QStringLiteral("action_chinese") || id == QStringLiteral("action_english") || //20261124-7 add
            id == QStringLiteral("action_quit") || id == QStringLiteral("action_H") || id == QStringLiteral("action_KinSeal") ||
            id == QStringLiteral("action_42") || id == QStringLiteral("action_tool") || id == QStringLiteral("action_40")) {
            enabled = true;
        } else if (id == QStringLiteral("action_save")) { enabled = m_hasProject && m_dirty; }
        else if (id == QStringLiteral("action_USBdownload")) {
            enabled = m_hasProject && !downloadInProgress(); //20261114-5 change: Cover the owned session from snapshot publication through final cleanup.
        } else if (id == QStringLiteral("action_offline_simulation") || id == QStringLiteral("action_online_simulation")) { //20261124-13 change
            enabled = m_hasProject && !downloadInProgress();
        } else if (id == QStringLiteral("action_script") || id == QStringLiteral("action_GlobalMacro") ||
                   id == QStringLiteral("action_InitMacro")) { //20261123-20 change
            enabled = m_hasProject && !downloadInProgress();
        } else if (command == QStringLiteral("properties")) { //20261124-0 add: An empty selection still has page properties.
            enabled = m_hasProject;
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
