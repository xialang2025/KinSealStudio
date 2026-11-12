#include "main_window.h"
#include "hmi_canvas.h"
#include "message_display_dialog.h"
#include "scale_dialog.h"
#include "qr_code_dialog.h" //20261112-0 add
#include "image_display_dialog.h" //20261112-0 add
#include "gif_display_dialog.h" //20261112-0 add
#include "number_display_dialog.h" //20261112-0 add
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
    QMap<QString, QByteArray> pictures = m_project.pictures; //20261112-0 add: Child resource edits publish together with the object.
    QMap<QString, QByteArray> animations = m_project.animations; //20261112-0 add
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
    else if (type == QStringLiteral("number")) //20261112-0 add
    {
        object.name = QStringLiteral("数值显示器"); object.geometry = QRect(0, 0, 120, 60); object.address = QStringLiteral("LW0"); object.hasNumberDisplayProperties = true;
        dialog = new NumberDisplayDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("gif")) //20261112-0 add
    {
        object.name = QStringLiteral("GIF显示器"); object.geometry = QRect(0, 0, 100, 100);
        dialog = new GifDisplayDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("image")) //20261112-0 add
    {
        object.name = QStringLiteral("图片显示器"); object.hasImageDisplayProperties = true; object.geometry = QRect(0, 0, 200, 100);
        dialog = new ImageDisplayDialog(object, currentEditorProject(), this);
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
        else if (type == QStringLiteral("gif")) { animations = qobject_cast<GifDisplayDialog *>(dialog)->animations(); object = qobject_cast<GifDisplayDialog *>(dialog)->object(); }
        else if (type == QStringLiteral("number")) { pictures = qobject_cast<NumberDisplayDialog *>(dialog)->pictures(); object = qobject_cast<NumberDisplayDialog *>(dialog)->object(); }
        else if (type == QStringLiteral("image")) { pictures = qobject_cast<ImageDisplayDialog *>(dialog)->pictures(); object = qobject_cast<ImageDisplayDialog *>(dialog)->object(); }
        else { object = qobject_cast<ScaleDialog *>(dialog)->object(); } //20261112-0 change
        if (!applyControlDraft(object, m_project.macros, pictures, false, &error, &animations)) //20261112-0 change
        { QMessageBox::warning(this, QStringLiteral("控件未创建"), error); delete dialog; return; }
        beginControlDraftPlacement(object, m_project.macros, pictures, &animations); //20261112-0 change
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
    QMap<QString, QByteArray> pictures = m_project.pictures; //20261112-0 add
    QMap<QString, QByteArray> animations = m_project.animations; //20261112-0 add
    QString error;
    bool selected;
    object = m_canvas->selectedObject(&selected);
    if (!selected) { return; }
    if (object.type == QStringLiteral("message")) { dialog = new MessageDisplayDialog(object, currentEditorProject(), this); }
    else if (object.type == QStringLiteral("qrcode")) { dialog = new QrCodeDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("gif")) { dialog = new GifDisplayDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("number")) { dialog = new NumberDisplayDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("image")) { dialog = new ImageDisplayDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("scale")) { dialog = new ScaleDialog(object, this); }
    else { return; }
    if (dialog->exec() == QDialog::Accepted)
    {
        if (object.type == QStringLiteral("message")) { object = qobject_cast<MessageDisplayDialog *>(dialog)->object(); }
        else if (object.type == QStringLiteral("qrcode")) { object = qobject_cast<QrCodeDialog *>(dialog)->object(); }
        else if (object.type == QStringLiteral("gif")) { animations = qobject_cast<GifDisplayDialog *>(dialog)->animations(); object = qobject_cast<GifDisplayDialog *>(dialog)->object(); }
        else if (object.type == QStringLiteral("number")) { pictures = qobject_cast<NumberDisplayDialog *>(dialog)->pictures(); object = qobject_cast<NumberDisplayDialog *>(dialog)->object(); }
        else if (object.type == QStringLiteral("image")) { pictures = qobject_cast<ImageDisplayDialog *>(dialog)->pictures(); object = qobject_cast<ImageDisplayDialog *>(dialog)->object(); }
        else { object = qobject_cast<ScaleDialog *>(dialog)->object(); } //20261112-0 change
        if (!applyControlDraft(object, m_project.macros, pictures, true, &error, &animations)) { QMessageBox::warning(this, QStringLiteral("控件修改未提交"), error); } //20261112-0 change
    }
    delete dialog;
}
