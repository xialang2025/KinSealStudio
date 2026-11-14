#include "main_window.h"
#include "hmi_canvas.h"
#include "simulation_window.h"

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
 * Description     : Explain the verified replay boundary without inventing a physical port or connecting to a PLC.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void MainWindow::showOnlineSimulationOptions() //20261114-6 add
{
    QDialog dialog(this);
    QVBoxLayout *layout;
    QLabel *scope;
    QDialogButtonBox *buttons;

    dialog.setObjectName(QStringLiteral("onlineSimulationScope"));
    dialog.setWindowTitle(QStringLiteral("在线模拟"));
    layout = new QVBoxLayout(&dialog);
    scope = new QLabel(QStringLiteral("在线模拟的实际PLC端口尚未接入。\n\n"
        "当前已有受控Modbus RTU事务测试：内部测试从站、读线圈/保持寄存器、单点写入、CRC与异常应答、超时和退出清理。\n\n"
        "原厂端口配置、设备地址映射、在线时限和真实PLC通信仍待验证。此窗口不会连接PLC。"), &dialog);
    scope->setWordWrap(true);
    scope->setMinimumWidth(420);
    layout->addWidget(scope);
    buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.exec();
}
