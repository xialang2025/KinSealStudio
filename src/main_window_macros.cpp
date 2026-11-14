#include "main_window.h"
#include "hmi_canvas.h"
#include "macro_manager_dialog.h"
#include <QMessageBox>
#include <QPointer>

/******************************************************************************
 * Function Name   : MainWindow::manageMacros
 * Description     : Commit an accepted ordinary-macro workspace as one complete editor-history transaction.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MainWindow::manageMacros() //20261114-7 add
{
    QPointer<MainWindow> lifetime(this);
    QPointer<MacroManagerDialog> dialog;
    HmiProject draft;
    QString error;
    int result;

    if (!m_hasProject || downloadInProgress()) { return; }
    m_canvas->cancelInteraction();
    dialog = new MacroManagerDialog(currentEditorProject(), this);
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
