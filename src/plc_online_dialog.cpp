#include "plc_online_dialog.h"
#include "plc_protocol_capabilities.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QSerialPortInfo>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QThread>
#include <QVBoxLayout>

struct PlcOnlineJob //20261114-8 add: Worker results and cancellation outlive a deleted widget.
{
    std::shared_ptr<std::atomic_bool> cancelled = std::make_shared<std::atomic_bool>(false);
    QJsonObject report;
};

/******************************************************************************
 * Function Name   : PlcOnlineDialog::PlcOnlineDialog
 * Description     : Build an explicit PC-port RTU tool and a separate observed protocol capability table.
 * Input Parameters: factory, parent
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
PlcOnlineDialog::PlcOnlineDialog(const PlcSerialIoFactory &factory, QWidget *parent) //20261114-8 add
    : QDialog(parent), m_factory(factory)
{
    QVBoxLayout *layout;
    QVBoxLayout *serialLayout;
    QFormLayout *form;
    QTabWidget *tabs;
    QWidget *serial;
    QLabel *scope;
    QDialogButtonBox *buttons;
    QTableWidget *table;
    QJsonArray profiles = plcProtocolCapabilities().value(QStringLiteral("profiles")).toArray();
    QJsonObject profile;
    QStringList cells;
    int index;
    int column;

    setObjectName(QStringLiteral("PlcOnlineDialog")); setWindowTitle(QStringLiteral("PLC通信与协议能力")); resize(1000, 720);
    layout = new QVBoxLayout(this); tabs = new QTabWidget(this); layout->addWidget(tabs);
    serial = new QWidget(tabs); serialLayout = new QVBoxLayout(serial);
    scope = new QLabel(QStringLiteral("PC串口调试：地址按0起始PDU输入。选择端口，将参数设为与从站一致，再执行一次请求。工程画面联动尚未开放。"), serial);
    scope->setWordWrap(true); serialLayout->addWidget(scope); form = new QFormLayout(); serialLayout->addLayout(form);
    m_port = new QComboBox(serial); m_port->setObjectName(QStringLiteral("PlcPcPort")); m_port->setEditable(true);
    for (const QSerialPortInfo &port : QSerialPortInfo::availablePorts()) { m_port->addItem(port.portName()); }
    m_port->setCurrentIndex(-1); m_port->setPlaceholderText(QStringLiteral("选择或输入PC串口")); form->addRow(QStringLiteral("PC串口"), m_port);
    m_baud = new QSpinBox(serial); m_baud->setObjectName(QStringLiteral("PlcBaud")); m_baud->setRange(50, 4000000); m_baud->setValue(9600); form->addRow(QStringLiteral("波特率（8数据位）"), m_baud);
    m_parity = new QComboBox(serial); m_parity->setObjectName(QStringLiteral("PlcParity"));
    m_parity->addItem(QStringLiteral("无校验"), 0); m_parity->addItem(QStringLiteral("偶校验"), 2); m_parity->addItem(QStringLiteral("奇校验"), 3); form->addRow(QStringLiteral("校验"), m_parity);
    m_stop = new QComboBox(serial); m_stop->addItem(QStringLiteral("1"), 1); m_stop->addItem(QStringLiteral("2"), 2); m_stop->setObjectName(QStringLiteral("PlcStopBits")); form->addRow(QStringLiteral("停止位"), m_stop);
    m_station = new QSpinBox(serial); m_station->setObjectName(QStringLiteral("PlcStation")); m_station->setRange(1, 247); form->addRow(QStringLiteral("从站号"), m_station);
    m_operation = new QComboBox(serial); m_operation->setObjectName(QStringLiteral("PlcOperation"));
    m_operation->addItem(QStringLiteral("01 读线圈"), 1); m_operation->addItem(QStringLiteral("02 读离散输入"), 2);
    m_operation->addItem(QStringLiteral("03 读保持寄存器"), 3); m_operation->addItem(QStringLiteral("04 读输入寄存器"), 4);
    m_operation->addItem(QStringLiteral("05 写单线圈"), 5); m_operation->addItem(QStringLiteral("06 写单寄存器"), 6);
    m_operation->addItem(QStringLiteral("15 写多线圈"), 15); m_operation->addItem(QStringLiteral("16 写多寄存器"), 16); m_operation->setCurrentIndex(2);
    form->addRow(QStringLiteral("操作"), m_operation);
    m_address = new QSpinBox(serial); m_address->setObjectName(QStringLiteral("PlcPduAddress")); m_address->setRange(0, 65535); form->addRow(QStringLiteral("PDU起始地址"), m_address);
    m_count = new QSpinBox(serial); m_count->setObjectName(QStringLiteral("PlcCount")); m_count->setRange(1, 125); form->addRow(QStringLiteral("数量"), m_count);
    m_values = new QLineEdit(serial); m_values->setObjectName(QStringLiteral("PlcValues")); m_values->setMaxLength(8192); m_values->setText(QStringLiteral("0")); form->addRow(QStringLiteral("写入值（十进制、逗号分隔）"), m_values);
    m_timeout = new QSpinBox(serial); m_timeout->setObjectName(QStringLiteral("PlcTimeout")); m_timeout->setRange(1, 60000); m_timeout->setValue(1000); form->addRow(QStringLiteral("单次超时（ms）"), m_timeout);
    m_output = new QPlainTextEdit(serial); m_output->setObjectName(QStringLiteral("PlcOutput")); m_output->setReadOnly(true); m_output->setMaximumBlockCount(2000); serialLayout->addWidget(m_output, 1);
    buttons = new QDialogButtonBox(serial); m_execute = buttons->addButton(QStringLiteral("执行一次"), QDialogButtonBox::ActionRole);
    m_execute->setObjectName(QStringLiteral("PlcExecute")); m_cancel = buttons->addButton(QStringLiteral("停止"), QDialogButtonBox::ActionRole); m_cancel->setObjectName(QStringLiteral("PlcCancel")); m_cancel->setEnabled(false);
    serialLayout->addWidget(buttons); tabs->addTab(serial, QStringLiteral("PC串口"));
    table = new QTableWidget(profiles.size(), 6, tabs); table->setObjectName(QStringLiteral("PlcCapabilities"));
    table->setHorizontalHeaderLabels({QStringLiteral("配置"), QStringLiteral("模块"), QStringLiteral("已核地址族"), QStringLiteral("设备编译"), QStringLiteral("PC工程映射"), QStringLiteral("HMI联动")});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers); table->setSelectionBehavior(QAbstractItemView::SelectRows);
    for (index = 0; index < profiles.size(); ++index)
    {
        profile = profiles.at(index).toObject();
        cells = {profile.value(QStringLiteral("profileId")).toString(), profile.value(QStringLiteral("driverName")).toString(),
            QStringLiteral("%1/%2").arg(profile.value(QStringLiteral("grammarFamilyCount")).toInt()).arg(profile.value(QStringLiteral("familyCount")).toInt()),
            profile.value(QStringLiteral("deviceCompileEligible")).toBool() ? QStringLiteral("受限COM1/站1") : QStringLiteral("未开放"), QStringLiteral("未接入"), QStringLiteral("未验证")};
        for (column = 0; column < cells.size(); ++column) { table->setItem(index, column, new QTableWidgetItem(cells.at(column))); }
    }
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); tabs->addTab(table, QStringLiteral("协议能力"));
    buttons = new QDialogButtonBox(QDialogButtonBox::Close, this); layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, this, &PlcOnlineDialog::reject);
    connect(m_execute, &QPushButton::clicked, this, &PlcOnlineDialog::executeRequest); connect(m_cancel, &QPushButton::clicked, this, &PlcOnlineDialog::cancelRequest);
    connect(m_operation, &QComboBox::currentIndexChanged, this, &PlcOnlineDialog::updateOperation);
    m_inputs = {m_port, m_baud, m_parity, m_stop, m_station, m_operation, m_address, m_count, m_values, m_timeout}; updateOperation();
}

/******************************************************************************
 * Function Name   : PlcOnlineDialog::~PlcOnlineDialog
 * Description     : Cancel the owned worker without destroying a live QThread or capturing widgets in it.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
PlcOnlineDialog::~PlcOnlineDialog() //20261114-8 add
{
    cancelRequest();
}

/******************************************************************************
 * Function Name   : PlcOnlineDialog::running
 * Description     : Report whether the worker completion has been handled by the GUI owner.
 * Input Parameters: None
 * Return Value    : Running state.
 * Date            : 2026.11.14
 ******************************************************************************/
bool PlcOnlineDialog::running() const //20261114-8 add
{
    return m_thread != nullptr;
}

/******************************************************************************
 * Function Name   : PlcOnlineDialog::lastReport
 * Description     : Return a value copy of the last completed session receipt.
 * Input Parameters: None
 * Return Value    : Last report, empty before completion.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject PlcOnlineDialog::lastReport() const //20261114-8 add
{
    return m_report;
}

/******************************************************************************
 * Function Name   : PlcOnlineDialog::updateOperation
 * Description     : Apply standard PDU count limits without translating any native project address.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcOnlineDialog::updateOperation() //20261114-8 add
{
    int operation = m_operation->currentData().toInt();
    bool single = operation == 5 || operation == 6;
    bool writing = single || operation == 15 || operation == 16;

    m_count->setRange(1, single ? 1 : (operation == 15 ? 1968 : (operation == 16 ? 123 : (operation <= 2 ? 2000 : 125))));
    m_count->setEnabled(!single && !running()); m_values->setEnabled(writing && !running());
}

/******************************************************************************
 * Function Name   : PlcOnlineDialog::setRunning
 * Description     : Freeze one immutable request while retaining the stop control.
 * Input Parameters: active
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcOnlineDialog::setRunning(bool active) //20261114-8 add
{
    for (QWidget *input : m_inputs) { input->setEnabled(!active); }
    m_execute->setEnabled(!active); m_cancel->setEnabled(active);
    if (!active) { updateOperation(); }
}

/******************************************************************************
 * Function Name   : PlcOnlineDialog::executeRequest
 * Description     : Validate and freeze a manual request, then perform serial I/O only in an owned worker.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcOnlineDialog::executeRequest() //20261114-8 add
{
    PlcSerialSettings settings;
    SimulationModbusRequest request;
    QStringList parts;
    QString error;
    QByteArray frame;
    std::shared_ptr<PlcOnlineJob> job;
    PlcSerialIoFactory factory = m_factory;
    u32 value;
    int station;
    int operation;
    bool ok;
    const QRegularExpression decimal(QStringLiteral("^[0-9]+$"));

    if (running()) { return; }
    settings.portName = m_port->currentText(); settings.baudRate = m_baud->value(); settings.parity = m_parity->currentData().toInt();
    settings.stopBits = m_stop->currentData().toInt(); settings.timeoutMs = m_timeout->value(); station = m_station->value();
    operation = m_operation->currentData().toInt(); request.id = QStringLiteral("manual-1"); request.operation = SimulationModbusOperation(operation);
    request.address = u32(m_address->value()); request.count = u32(m_count->value());
    if (operation == 5 || operation == 6 || operation == 15 || operation == 16)
    {
        //20261114-8 change: Empty fields and signed tokens cannot silently change the user's write list.
        parts = m_values->text().split(QChar(u','), Qt::KeepEmptyParts);
        for (const QString &part : parts)
        {
            value = part.trimmed().toUInt(&ok, 10);
            if (!ok || !decimal.match(part.trimmed()).hasMatch())
            { m_output->setPlainText(QStringLiteral("写入值须为逗号分隔的非负十进制整数，不能有空项或符号。")); return; }
            request.values.append(value);
        }
        if (operation == 5 || operation == 6)
        {
            if (request.values.size() != 1) { m_output->setPlainText(QStringLiteral("单点写入需要一个值。")); return; }
            request.value = request.values.first(); request.values.clear();
        }
    }
    if (!validatePlcSerialSettings(settings, &error) || !buildSimulationModbusRequest(station, request, &frame, &error))
    { m_output->setPlainText(error); return; }
    m_report = QJsonObject(); m_closeWhenFinished = false; job = std::make_shared<PlcOnlineJob>(); m_job = job;
    m_output->setPlainText(QStringLiteral("正在收发..."));
    m_thread = QThread::create([job, settings, station, request, factory]() { job->report = runPlcSerialBatch(settings, station, {request}, job->cancelled, factory); });
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, this, [this, job]() {
        QPointer<PlcOnlineDialog> lifetime(this);
        bool closeAfter = m_closeWhenFinished;
        QStringList lines;
        QStringList values;
        QJsonObject result;

        m_report = job->report; m_thread = nullptr; m_job.reset(); setRunning(false);
        lines.append(m_report.value(QStringLiteral("ok")).toBool() ? QStringLiteral("收发完成。") :
            QStringLiteral("收发失败：%1").arg(m_report.value(QStringLiteral("error")).toString()));
        for (const QJsonValue &entry : m_report.value(QStringLiteral("results")).toArray())
        {
            result = entry.toObject(); values.clear();
            lines.append(QStringLiteral("发送：%1").arg(result.value(QStringLiteral("requestHex")).toString()));
            lines.append(QStringLiteral("接收：%1").arg(result.value(QStringLiteral("responseHex")).toString()));
            for (const QJsonValue &value : result.value(QStringLiteral("values")).toArray()) { values.append(QString::number(value.toInt())); }
            if (!values.isEmpty()) { lines.append(QStringLiteral("读取值（%1项）：%2").arg(values.size()).arg(values.join(QStringLiteral(", ")))); }
            if (result.value(QStringLiteral("writeAcknowledged")).toBool()) { lines.append(QStringLiteral("写入应答已确认。")); }
            if (result.value(QStringLiteral("remoteStateUnconfirmed")).toBool()) { lines.append(QStringLiteral("写入可能已到达从站，状态未确认。")); }
        }
        if (m_report.value(QStringLiteral("opened")).toBool())
        {
            lines.append(m_report.value(QStringLiteral("closed")).toBool() ? QStringLiteral("已关闭本次PC串口会话。") : QStringLiteral("串口释放未确认，请查看错误。"));
        }
        m_output->setPlainText(lines.join(QChar(u'\n'))); //20261114-8 change: A 2000-value read stays complete without exposing a truncated JSON fragment.
        if (!lifetime.isNull() && closeAfter) { QDialog::reject(); }
    });
    setRunning(true); m_thread->start();
}

/******************************************************************************
 * Function Name   : PlcOnlineDialog::cancelRequest
 * Description     : Signal cancellation without touching the worker-owned serial object.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcOnlineDialog::cancelRequest() //20261114-8 add
{
    if (m_job) { m_job->cancelled->store(true); }
}

/******************************************************************************
 * Function Name   : PlcOnlineDialog::reject
 * Description     : Defer ordinary close until the owned port has completed cancellation and cleanup.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcOnlineDialog::reject() //20261114-8 add
{
    if (running()) { m_closeWhenFinished = true; cancelRequest(); return; }
    QDialog::reject();
}
