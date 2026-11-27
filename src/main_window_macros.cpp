#include "main_window.h"
#include "hmi_canvas.h"
#include "macro_manager_dialog.h"
#include "macro_schedule_dialogs.h" //20261123-20 add
#include <QMessageBox>
#include <QPointer>

/******************************************************************************
 * Function Name   : MainWindow::manageMacros
 * Description     : Commit an accepted ordinary-macro workspace as one complete editor-history transaction.
 * Input Parameters: addSource, to request a macro name before opening the editor.
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MainWindow::manageMacros(bool addSource) //20261127-1 change
{
    QPointer<MainWindow> lifetime(this);
    QPointer<MacroManagerDialog> dialog;
    HmiProject draft;
    QString error;
    int result;

    if (!m_hasProject || downloadInProgress()) { return; }
    m_canvas->cancelInteraction();
    dialog = new MacroManagerDialog(currentEditorProject(), this);
    if (addSource && !dialog->promptNew(this)) //20261127-1 add
    {
        delete dialog.data();
        return;
    }
    result = dialog->exec();
    if (lifetime.isNull() || dialog.isNull()) { return; }
    if (result == QDialog::Accepted)
    {
        draft = dialog->draftProject();
        delete dialog.data();
        if (!commitProjectDraft(draft, &error)) { QMessageBox::warning(this, QStringLiteral("宏未提交"), error); }
    }
    else { delete dialog.data(); }
}

/******************************************************************************
 * Function Name   : MainWindow::manageMacroSchedule
 * Description     : Edit a project invocation list and publish it as one history transaction.
 * Input Parameters: initialization
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void MainWindow::manageMacroSchedule(bool initialization) //20261123-20 add
{
    QPointer<MainWindow> lifetime(this);
    HmiProject draft;
    QString error;

    if (!m_hasProject || downloadInProgress())
    {
        return;
    }
    m_canvas->cancelInteraction();
    draft = currentEditorProject();
    if (editMacroSchedules(this, &draft, initialization) && !lifetime.isNull() && !commitProjectDraft(draft, &error))
    {
        QMessageBox::warning(this, initialization ? QStringLiteral("初始化宏未提交") :
            QStringLiteral("全局宏未提交"), error);
    }
}
