#include "main_window.h"
#include "hmi_canvas.h"
#include "message_display_dialog.h"
#include "scale_dialog.h"
#include "qr_code_dialog.h" //20261112-0 add
#include <QMessageBox>
#include <QStatusBar>

/******************************************************************************
 * Function Name   : MainWindow::createStage4Control
 * Description     : Open a typed control dialog immediately and begin placement only after a valid accepted draft.
 * Input Parameters: type
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MainWindow::createStage4Control(const QString &type) //20261112-0 add
{
    HmiObject object;
    QDialog *dialog;
    QString error;
    object.type = type;
    if (type == QStringLiteral("message"))
    {
        object.name = QStringLiteral("消息显示器"); object.text = QStringLiteral("0"); object.address = QStringLiteral("LW0"); object.geometry = QRect(0, 0, 200, 110);
        dialog = new MessageDisplayDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("qrcode")) //20261112-0 add
    {
        object.name = QStringLiteral("二维码"); object.address = QStringLiteral("LW0"); object.geometry = QRect(0, 0, 160, 110);
        dialog = new QrCodeDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("scale"))
    {
        object.name = QStringLiteral("刻度"); object.geometry = QRect(0, 0, 160, 110);
        dialog = new ScaleDialog(object, this);
    }
    else { return; }
    if (dialog->exec() == QDialog::Accepted)
    {
        if (type == QStringLiteral("message")) { object = qobject_cast<MessageDisplayDialog *>(dialog)->object(); }
        else if (type == QStringLiteral("qrcode")) { object = qobject_cast<QrCodeDialog *>(dialog)->object(); }
        else { object = qobject_cast<ScaleDialog *>(dialog)->object(); } //20261112-0 change
        if (!applyControlDraft(object, m_project.macros, m_project.pictures, false, &error))
        { QMessageBox::warning(this, QStringLiteral("控件未创建"), error); delete dialog; return; }
        beginControlDraftPlacement(object, m_project.macros, m_project.pictures);
        statusBar()->showMessage(QStringLiteral("在画布拖出%1；按 Esc 取消。").arg(object.name), 10000);
    }
    else { m_canvas->setActiveTool(QStringLiteral("select")); }
    delete dialog;
}

/******************************************************************************
 * Function Name   : MainWindow::editStage4Control
 * Description     : Apply a complete detached control draft as one project transaction after property acceptance.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MainWindow::editStage4Control() //20261112-0 add
{
    HmiObject object;
    QDialog *dialog;
    QString error;
    bool selected;
    object = m_canvas->selectedObject(&selected);
    if (!selected) { return; }
    if (object.type == QStringLiteral("message")) { dialog = new MessageDisplayDialog(object, currentEditorProject(), this); }
    else if (object.type == QStringLiteral("qrcode")) { dialog = new QrCodeDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("scale")) { dialog = new ScaleDialog(object, this); }
    else { return; }
    if (dialog->exec() == QDialog::Accepted)
    {
        if (object.type == QStringLiteral("message")) { object = qobject_cast<MessageDisplayDialog *>(dialog)->object(); }
        else if (object.type == QStringLiteral("qrcode")) { object = qobject_cast<QrCodeDialog *>(dialog)->object(); }
        else { object = qobject_cast<ScaleDialog *>(dialog)->object(); } //20261112-0 change
        if (!applyControlDraft(object, m_project.macros, m_project.pictures, true, &error)) { QMessageBox::warning(this, QStringLiteral("控件修改未提交"), error); }
    }
    delete dialog;
}
