#include "main_window.h"
#include "hmi_canvas.h"
#include "simulation_window.h"
#include "plc_online_dialog.h" //20261114-8 add
#include <QPointer>

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : MainWindow::startOfflineSimulation
 * Description     : Run one independently owned editor snapshot without saving or mutating the editable project.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MainWindow::startOfflineSimulation() //20261114-6 add
{
    HmiProject snapshot;
    SimulationWindow *simulation;
    QString error;

    if (!m_hasProject || downloadInProgress()) { return; }
    if (!m_simulationWindow.isNull())
    {
        m_simulationWindow->show();
        statusBar()->showMessage(QStringLiteral("当前离线会话仍在运行；关闭后可按新工程重新启动。"), 8000);
        return;
    }
    m_canvas->cancelInteraction();
    snapshot = currentEditorProject();
    simulation = new SimulationWindow(this);
    if (!simulation->initialize(snapshot, &error))
    {
        delete simulation;
        QMessageBox::warning(this, QStringLiteral("工程暂不支持离线模拟"), error);
        return;
    }
    simulation->setAttribute(Qt::WA_DeleteOnClose);
    m_simulationWindow = simulation;
    simulation->show();
    statusBar()->showMessage(QStringLiteral("离线模拟使用当前工程快照；运行数据不会写回工程。"), 8000);
}

/******************************************************************************
 * Function Name   : MainWindow::showOnlineSimulationOptions
 * Description     : Open the explicit PC RTU tool without mapping or mutating the editor project.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MainWindow::showOnlineSimulationOptions() //20261114-8 change
{
    QPointer<PlcOnlineDialog> dialog = new PlcOnlineDialog({}, this);

    dialog->exec();
    if (!dialog.isNull()) { delete dialog.data(); }
}
