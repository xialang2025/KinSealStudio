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
        statusBar()->showMessage(QStringLiteral("当前模拟会话仍在运行；关闭后可按新工程重新启动。"), 8000);
        return;
    }
    m_canvas->cancelInteraction();
    snapshot = currentEditorProject();
    simulation = new SimulationWindow(this);
    if (!simulation->initialize(snapshot, &error))
    {
        delete simulation;
        beginInformation(QStringLiteral("模拟工程：%1").arg(snapshot.name)); //20261125-84 add
        appendInformation(error, OutputKind::Error);
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
 * Description     : Launch the current engineering page as a detached online simulation.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MainWindow::showOnlineSimulationOptions() //20261124-13 change
{
    HmiProject snapshot;
    SimulationWindow *simulation;
    QString error;

    if (!m_hasProject || downloadInProgress()) { return; }
    if (!m_simulationWindow.isNull())
    {
        m_simulationWindow->show();
        statusBar()->showMessage(QStringLiteral("当前模拟会话仍在运行；关闭后可切换模式。"), 8000);
        return;
    }
    m_canvas->cancelInteraction();
    snapshot = currentEditorProject();
    simulation = new SimulationWindow(this);
    if (!simulation->initializeOnline(snapshot, &error))
    {
        delete simulation;
        beginInformation(QStringLiteral("模拟工程：%1").arg(snapshot.name)); //20261125-84 add
        appendInformation(error, OutputKind::Error);
        QMessageBox::warning(this, QStringLiteral("工程暂不支持在线模拟"), error);
        return;
    }
    simulation->setAttribute(Qt::WA_DeleteOnClose);
    m_simulationWindow = simulation;
    simulation->show();
}

/******************************************************************************
 * Function Name   : MainWindow::showPlcSerialDiagnostics
 * Description     : Open the pre-existing manual RTU transaction tool.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.24
 ******************************************************************************/
void MainWindow::showPlcSerialDiagnostics() //20261124-13 add
{
    QPointer<PlcOnlineDialog> dialog;

    if (!m_simulationWindow.isNull() && m_simulationWindow->runtime().online())
    {
        statusBar()->showMessage(QStringLiteral("关闭在线模拟后可运行串口诊断。"), 5000);
        return;
    }
    dialog = new PlcOnlineDialog({}, this, m_hasProject ? m_project.targetModel : QString()); //20261127-0 change

    dialog->exec();
    if (!dialog.isNull()) { delete dialog.data(); }
}
