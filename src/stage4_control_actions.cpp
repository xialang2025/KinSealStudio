#include "main_window.h"
#include "hmi_canvas.h"
#include "message_display_dialog.h"
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
    MessageDisplayDialog *dialog;
    QString error;
    if (type != QStringLiteral("message")) { return; }
    object.type = type; object.name = QStringLiteral("消息显示器"); object.text = QStringLiteral("0");
    object.address = QStringLiteral("LW0"); object.geometry = QRect(0, 0, 200, 110);
    dialog = new MessageDisplayDialog(object, currentEditorProject(), this);
    if (dialog->exec() == QDialog::Accepted)
    {
        if (!applyControlDraft(dialog->object(), m_project.macros, m_project.pictures, false, &error))
        { QMessageBox::warning(this, QStringLiteral("控件未创建"), error); delete dialog; return; }
        beginControlDraftPlacement(dialog->object(), m_project.macros, m_project.pictures);
        statusBar()->showMessage(QStringLiteral("在画布拖出消息显示器；按 Esc 取消。"), 10000);
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
    MessageDisplayDialog *dialog;
    QString error;
    bool selected;
    object = m_canvas->selectedObject(&selected);
    if (!selected || object.type != QStringLiteral("message")) { return; }
    dialog = new MessageDisplayDialog(object, currentEditorProject(), this);
    if (dialog->exec() == QDialog::Accepted && !applyControlDraft(dialog->object(), m_project.macros, m_project.pictures, true, &error))
    { QMessageBox::warning(this, QStringLiteral("控件修改未提交"), error); }
    delete dialog;
}
