#include "main_window.h"
#include "hmi_canvas.h"
#include "static_text_dialog.h"
#include "bit_button_dialog.h" //20261108-15 add
#include "plc_address_dialog.h" //20261109-0 add
#include <QMessageBox>
#include <QJsonArray> //20261109-0 add
#include <QComboBox>
#include <QInputDialog>
#include <QRegularExpression>
#include <QStatusBar>

/******************************************************************************
 * Function Name   : MainWindow::applyControlDraft
 * Description     : Validate resource and object drafts together before replacing live editor state.
 * Input Parameters: object, macros, pictures, editing, error, animations
 * Return Value    : True after committing a complete valid draft; failure preserves the editor.
 * Date            : 2026.11.10
 ******************************************************************************/
bool MainWindow::applyControlDraft(const HmiObject &object, const QMap<QString, QString> &macros,
                                  const QMap<QString, QByteArray> &pictures, bool editing,
                                  QString *error, const QMap<QString, QByteArray> *animations) //20261112-0 change
{
    HmiProject candidate;
    int index;
    bool found;

    candidate = m_project;
    candidate.objects = m_canvas->objects();
    candidate.macros = macros;
    candidate.pictures = pictures;
    if (animations != nullptr) { candidate.animations = *animations; } //20261112-0 add
    found = false;
    if (editing)
    {
        for (index = 0; index < candidate.objects.size(); ++index)
        {
            if (candidate.objects.at(index).id == object.id)
            {
                candidate.objects[index] = object;
                found = true;
                break;
            }
        }
        if (!found)
        {
            if (error != nullptr) { *error = QStringLiteral("待编辑控件已不存在，修改未提交。"); }
            return false;
        }
    }
    if (!candidate.validateModel(error)) { return false; }
    //20261110-8 change: A creation draft is validated here, then retained outside the document until placement completes.
    if (!editing) { return true; }
    return commitProjectDraft(candidate, error);
}

/******************************************************************************
 * Function Name   : MainWindow::beginControlDraftPlacement
 * Description     : Hold accepted picture and macro drafts outside project history until a complete object rectangle exists.
 * Input Parameters: object, macros, pictures, animations
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::beginControlDraftPlacement(const HmiObject &object, const QMap<QString, QString> &macros,
                                           const QMap<QString, QByteArray> &pictures, const QMap<QString, QByteArray> *animations) //20261112-0 change
{
    m_canvas->beginControlPlacement(object);
    if (!m_canvas->hasPendingPlacement()) { //20261110-8 add: A rejected placement must never attach resources to a later unrelated edit.
        m_hasPendingControlResources = false;
        m_pendingControlResources = HmiProject();
        return;
    }
    // Starting a placement cancels its predecessor synchronously; register the new resources afterwards.
    m_pendingControlResources = m_project;
    m_pendingControlResources.macros = macros;
    m_pendingControlResources.pictures = pictures;
    if (animations != nullptr) { m_pendingControlResources.animations = *animations; } //20261112-0 add
    m_hasPendingControlResources = true;
}

/******************************************************************************
 * Function Name   : MainWindow::createStaticText
 * Description     : Open properties immediately and retain an accepted draft for rectangle drawing.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void MainWindow::createStaticText() //20261108-14 add
{
    HmiObject object;
    StaticTextDialog *dialog;

    object.type = QStringLiteral("text");
    object.name = QStringLiteral("静态文本");
    object.text = QStringLiteral("TEXT");
    object.geometry = QRect(0, 0, 150, 50);
    object.hasStaticTextProperties = true;
    dialog = new StaticTextDialog(object, m_project.languages, this);
    if (dialog->exec() == QDialog::Accepted)
    {
        m_canvas->beginStaticTextPlacement(dialog->object());
        statusBar()->showMessage(QStringLiteral("在画布按住左键拖出文本框；按 Esc 取消。"), 10000);
    }
    else
    {
        m_canvas->setActiveTool(QStringLiteral("select"));
    }
    delete dialog;
}

/******************************************************************************
 * Function Name   : MainWindow::editStaticText
 * Description     : Edit a detached copy and apply it only after the user confirms the dialog.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void MainWindow::editStaticText() //20261108-14 add
{
    HmiObject object;
    bool selected;
    StaticTextDialog *dialog;

    object = m_canvas->selectedObject(&selected);
    if (selected && (object.type == QStringLiteral("message") || object.type == QStringLiteral("scale") || object.type == QStringLiteral("qrcode") || object.type == QStringLiteral("image") || object.type == QStringLiteral("gif") || object.type == QStringLiteral("number") || object.type == QStringLiteral("ascii") || object.type == QStringLiteral("time") || object.type == QStringLiteral("screen") || object.type == QStringLiteral("multistate") || object.type == QStringLiteral("multifunction") || object.type == QStringLiteral("dropdown") || object.type == QStringLiteral("flow"))) { editStage4Control(); return; } //20261112-0 add
    if (selected && object.type == QStringLiteral("switch")) //20261108-15 add: Every property entry dispatches by control type.
    {
        editBitButton(); return;
    }
    if (!selected || object.type != QStringLiteral("text")) { return; }
    dialog = new StaticTextDialog(object, m_project.languages, this);
    if (dialog->exec() == QDialog::Accepted)
    {
        m_canvas->updateSelectedObject(dialog->object());
    }
    delete dialog;
}

/******************************************************************************
 * Function Name   : MainWindow::createBitButton
 * Description     : Open all button pages immediately and begin drag placement only after acceptance.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void MainWindow::createBitButton() //20261108-15 add
{
    HmiObject object;
    BitButtonDialog *dialog;
    QString error; //20261110-5 add
    object.type = QStringLiteral("switch"); object.name = QStringLiteral("位按钮");
    object.text = QStringLiteral("OFF"); object.address = QStringLiteral("LB0");
    object.geometry = QRect(0, 0, 120, 60); object.hasBitButtonProperties = true;
    dialog = new BitButtonDialog(object, m_project, this);
    if (dialog->exec() == QDialog::Accepted)
    {
        if (!applyControlDraft(dialog->object(), dialog->macros(), dialog->pictures(), false, &error)) //20261110-5 change
        {
            QMessageBox::warning(this, QStringLiteral("控件资源修改未提交"), error);
            delete dialog;
            return;
        }
        beginControlDraftPlacement(dialog->object(), dialog->macros(), dialog->pictures()); //20261110-8 change
        statusBar()->showMessage(QStringLiteral("在画布拖出位按钮；按 Esc 取消。"), 10000);
    }
    else { m_canvas->setActiveTool(QStringLiteral("select")); }
    delete dialog;
}

/******************************************************************************
 * Function Name   : MainWindow::createWordButton
 * Description     : Create a word-operation switch using the shared artwork and access-rule dialog.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void MainWindow::createWordButton() //20261109-6 add
{
    HmiObject object;
    BitButtonDialog *dialog;
    QString error; //20261110-5 add

    object.type = QStringLiteral("switch"); object.name = QStringLiteral("字按钮");
    object.text = QStringLiteral("OFF"); object.address = QStringLiteral("LW0");
    object.geometry = QRect(0, 0, 120, 60);
    object.hasBitButtonProperties = true; object.hasWordButtonProperties = true;
    object.bitButton.indicator = false; object.bitButton.dualMonitor = false;
    dialog = new BitButtonDialog(object, m_project, this);
    if (dialog->exec() == QDialog::Accepted)
    {
        if (!applyControlDraft(dialog->object(), dialog->macros(), dialog->pictures(), false, &error)) //20261110-5 change
        {
            QMessageBox::warning(this, QStringLiteral("控件资源修改未提交"), error);
            delete dialog;
            return;
        }
        beginControlDraftPlacement(dialog->object(), dialog->macros(), dialog->pictures()); //20261110-8 change
        statusBar()->showMessage(QStringLiteral("在画布拖出字按钮；按 Esc 取消。"), 10000);
    }
    else { m_canvas->setActiveTool(QStringLiteral("select")); }
    delete dialog;
}

/******************************************************************************
 * Function Name   : MainWindow::createLamp
 * Description     : Configure a read-only lamp before entering the shared rectangle placement transaction.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void MainWindow::createLamp() //20261109-8 add
{
    HmiObject object;
    BitButtonDialog *dialog;
    QString error; //20261110-5 add

    object.type = QStringLiteral("switch"); object.name = QStringLiteral("指示灯");
    object.text = QStringLiteral("OFF"); object.geometry = QRect(0, 0, 120, 60);
    object.hasBitButtonProperties = true; object.hasLampProperties = true;
    object.bitButton.monitorSame = false; object.bitButton.bitIndex = 0;
    dialog = new BitButtonDialog(object, m_project, this);
    if (dialog->exec() == QDialog::Accepted)
    {
        if (!applyControlDraft(dialog->object(), dialog->macros(), dialog->pictures(), false, &error)) //20261110-5 change
        {
            QMessageBox::warning(this, QStringLiteral("控件资源修改未提交"), error);
            delete dialog;
            return;
        }
        beginControlDraftPlacement(dialog->object(), dialog->macros(), dialog->pictures()); //20261110-8 change
        statusBar()->showMessage(QStringLiteral("在画布拖出指示灯；按 Esc 取消。"), 10000);
    }
    else { m_canvas->setActiveTool(QStringLiteral("select")); }
    delete dialog;
}

/******************************************************************************
 * Function Name   : MainWindow::createFunctionButton
 * Description     : Configure a system-function switch before starting rectangle placement.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::createFunctionButton() //20261110-0 add
{
    HmiObject object;
    BitButtonDialog *dialog;
    QString error; //20261110-5 add

    object.type = QStringLiteral("switch"); object.name = QStringLiteral("功能按钮");
    object.text = QStringLiteral("OFF"); object.geometry = QRect(0, 0, 120, 60);
    object.hasBitButtonProperties = true; object.hasFunctionButtonProperties = true;
    object.bitButton.indicator = false; object.bitButton.dualMonitor = false;
    dialog = new BitButtonDialog(object, m_project, this);
    if (dialog->exec() == QDialog::Accepted)
    {
        if (!applyControlDraft(dialog->object(), dialog->macros(), dialog->pictures(), false, &error)) //20261110-5 change
        {
            QMessageBox::warning(this, QStringLiteral("控件资源修改未提交"), error);
            delete dialog;
            return;
        }
        beginControlDraftPlacement(dialog->object(), dialog->macros(), dialog->pictures()); //20261110-8 change
        statusBar()->showMessage(QStringLiteral("在画布拖出功能按钮；按 Esc 取消。"), 10000);
    }
    else { m_canvas->setActiveTool(QStringLiteral("select")); }
    delete dialog;
}

/******************************************************************************
 * Function Name   : MainWindow::editBitButton
 * Description     : Commit complete object and resource drafts only when the button dialog is accepted.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void MainWindow::editBitButton() //20261108-15 add
{
    HmiObject object;
    BitButtonDialog *dialog;
    QString error; //20261110-5 add
    bool selected;
    object = m_canvas->selectedObject(&selected);
    if (!selected || object.type != QStringLiteral("switch")) { return; }
    dialog = new BitButtonDialog(object, m_project, this);
    if (dialog->exec() == QDialog::Accepted)
    {
        if (!applyControlDraft(dialog->object(), dialog->macros(), dialog->pictures(), true, &error)) //20261110-5 change
        {
            QMessageBox::warning(this, QStringLiteral("控件资源修改未提交"), error);
            delete dialog;
            return;
        }
        //20261110-8 change: applyControlDraft has already published the complete object/resource transaction once.
    }
    delete dialog;
}

/******************************************************************************
 * Function Name   : MainWindow::configureLanguages
 * Description     : Let project languages drive the text dialog and canvas language selector.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void MainWindow::configureLanguages() //20261110-5 change
{
    bool accepted;
    QString input;
    QStringList languages;
    QString language;
    QString error;
    QString selectedId;
    HmiProject candidate;

    input = QInputDialog::getText(this, QStringLiteral("工程语言设置"),
                                  QStringLiteral("语言名称，以逗号分隔（例如：中文,English）"),
                                  QLineEdit::Normal, m_project.languages.join(QStringLiteral(",")), &accepted);
    if (!accepted) { return; }
    Q_FOREACH (language, input.split(QRegularExpression(QStringLiteral("[,，]")), Qt::SkipEmptyParts))
    {
        language = language.trimmed();
        if (!language.isEmpty() && !languages.contains(language)) { languages.append(language); }
    }
    if (languages.isEmpty()) { return; }
    candidate = m_project;
    candidate.objects = m_canvas->objects();
    if (!candidate.setLanguages(languages, &error))
    {
        QMessageBox::warning(this, QStringLiteral("语言设置未提交"), error);
        return;
    }
    m_project = candidate;
    m_languageCombo->blockSignals(true);
    m_languageCombo->clear(); m_languageCombo->addItems(languages);
    m_languageCombo->setCurrentText(m_project.activeLanguage);
    m_languageCombo->blockSignals(false);
    // Rebuild from remapped function-language targets, not stale canvas property copies.
    selectedId = m_selectedId; //20261110-5 add: Rebuilding remapped object copies must retain the user's selected property target.
    m_canvas->setProject(m_project);
    if (!selectedId.isEmpty()) { m_canvas->selectObject(selectedId); }
    rebuildProjectTree();
    setDirty(true);
}

/******************************************************************************
 * Function Name   : MainWindow::configureConnections
 * Description     : Save explicit PLC connection drafts without starting device communication.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void MainWindow::configureConnections() //20261109-0 add
{
    QVector<PlcConnection> connections;
    HmiProject candidate;
    QJsonArray connectionsJson; //20261109-0 add
    PlcConnection connection;
    QString error;

    connections = m_project.connections;
    if (editPlcConnections(this, m_project.targetModel, &connections, &error))
    {
        candidate = m_project;
        candidate.objects = m_canvas->objects();
        candidate.connections = connections;
        if (candidate.validateModel(&error)) //20261110-5 change: Explicit schema9 connection references also participate in this transaction.
        {
            m_project.connections = connections;
            Q_FOREACH (connection, connections) { connectionsJson.append(connection.toJson()); }
            setProperty("plcConnections", connectionsJson.toVariantList());
            setDirty(true);
        }
        else
        {
            QMessageBox::warning(this, QStringLiteral("连接仍被使用"), error);
        }
    }
    else if (!error.isEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("连接设置不可用"), error);
    }
}
