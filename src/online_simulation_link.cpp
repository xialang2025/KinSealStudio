#include "online_simulation_link.h"

#include <QQueue> //20261124-14 add
#include <QThread>
#include <QTimer>
#include <optional>

namespace
{
/******************************************************************************
 * Function Name   : linkError
 * Description     : Provide a connection or mapping refusal without modifying output values.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.24
 ******************************************************************************/
bool linkError(QString *error, const QString &message) //20261124-13 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : bindingKey
 * Description     : Distinguish an address's bit and word-span uses in the confirmed cache.
 * Input Parameters: text, bit, span
 * Return Value    : Map key.
 * Date            : 2026.11.24
 ******************************************************************************/
QString bindingKey(const QString &text, bool bit, int span) //20261124-13 add
{
    return QStringLiteral("%1:%2:%3").arg(bit).arg(span).arg(text.trimmed());
}

/******************************************************************************
 * Function Name   : writeOverlapsScan
 * Description     : Match an acknowledged write to the same RTU address space and overlapping words or coils.
 * Input Parameters: write, scan
 * Return Value    : True when the scan's cached response can contain a changed value.
 * Date            : 2026.11.24
 ******************************************************************************/
bool writeOverlapsScan(const SimulationModbusRequest &write, const SimulationModbusRequest &scan) //20261124-14 add
{
    const SimulationModbusOperation read = write.operation == SimulationModbusOperation::WriteSingleCoil
        ? SimulationModbusOperation::ReadCoils : SimulationModbusOperation::ReadHoldingRegisters;

    return scan.operation == read && scan.address < write.address + write.count &&
        write.address < scan.address + scan.count;
}
}

//20261124-13 add: All RTU I/O and the serial object live on this worker thread.
class OnlineSimulationWorker : public QObject
{
public:
    OnlineSimulationWorker(OnlineSimulationLink *link, const QString &port)
        : m_link(link), m_settings(link->m_settings), m_factory(link->m_factory),
          m_cancel(link->m_cancel), m_scans(link->m_scans.values()), m_station(link->m_station),
          m_interval(link->m_frameInterval)
    {
        m_settings.portName = port;
    }

    /******************************************************************************
     * Function Name   : start
     * Description     : Open one persistent session and begin polling on the serial thread.
     * Input Parameters: None
     * Return Value    : None
     * Date            : 2026.11.24
     ******************************************************************************/
    void start()
    {
        SimulationModbusOptions options;
        SimulationModbusIo io;
        QString error;

        options.cancellationRequested = [cancel = m_cancel]() { return cancel->load(); };
        options.timeoutMs = m_settings.timeoutMs;
        options.maxPendingRequests = 1;
        if (m_cancel->load()) { return; }
        io = m_factory ? m_factory(m_settings, options.cancellationRequested)
                       : makePlcSerialIo(m_settings, options.cancellationRequested);
        if (!m_transport.connectToStation(m_station, io, options, &error))
        {
            stop(error);
            return;
        }
        m_timer = new QTimer(this);
        m_timer->setSingleShot(true);
        connect(m_timer, &QTimer::timeout, this, &OnlineSimulationWorker::step);
        m_timer->start(0);
    }

    /******************************************************************************
     * Function Name   : queueWrite
     * Description     : Give one user command priority over the following scan transaction.
     * Input Parameters: request, toggle
     * Return Value    : None
     * Date            : 2026.11.24
     ******************************************************************************/
    void queueWrite(const SimulationModbusRequest &request, bool toggle)
    {
        if (m_cancel->load() || !m_transport.isConnected()) { return; }
        m_write = request;
        m_toggle = toggle;
    }

private:
    /******************************************************************************
     * Function Name   : stop
     * Description     : Close on the first failure and clear unsent commands.
     * Input Parameters: error
     * Return Value    : None
     * Date            : 2026.11.24
     ******************************************************************************/
    void stop(const QString &error)
    {
        if (m_timer != nullptr) { m_timer->stop(); }
        m_write.reset();
        m_readback.clear(); //20261124-14 add: Drop pending readback on the same fail-closed path.
        m_transport.disconnectFromStation();
        QMetaObject::invokeMethod(m_link, [link = m_link, cancel = m_cancel, error]() {
            if (!cancel->load()) { link->failed(error); }
        }, Qt::QueuedConnection);
    }

    /******************************************************************************
     * Function Name   : step
     * Description     : Process one scan or write stage with the configured interval before the next frame.
     * Input Parameters: None
     * Return Value    : None
     * Date            : 2026.11.24
     ******************************************************************************/
    void step()
    {
        SimulationModbusRequest request;
        SimulationModbusRequest scan;
        SimulationModbusResult result;
        bool writing;
        bool readForToggle;
        QString error;

        if (m_cancel->load()) { return; }
        writing = m_readback.isEmpty() && m_write.has_value(); //20261124-14 change
        readForToggle = writing && m_toggle;
        if (writing)
        {
            request = *m_write;
            if (readForToggle) { request.operation = SimulationModbusOperation::ReadCoils; }
        }
        else if (!m_readback.isEmpty())
        {
            request = m_readback.dequeue(); //20261124-14 add
        }
        else
        {
            request = m_scans.at(m_next);
            m_next = (m_next + 1) % m_scans.size();
        }
        if (!m_transport.enqueue(request, &error)) { stop(error); return; }
        result = m_transport.processNext();
        if (!result.ok)
        {
            if (result.remoteStateUnconfirmed)
            { result.error += QStringLiteral("。写入结果未确认，请核对PLC状态。"); }
            stop(result.error);
            return;
        }
        if (readForToggle)
        {
            m_write->value = result.values.first() == 0;
            m_toggle = false;
        }
        else
        {
            if (writing) //20261124-14 change: Re-read affected ranges before the next normal poll or write.
            {
                Q_FOREACH (scan, m_scans)
                {
                    if (writeOverlapsScan(result.request, scan)) { m_readback.enqueue(scan); }
                }
                m_write.reset();
            }
            QMetaObject::invokeMethod(m_link, [link = m_link, cancel = m_cancel, result, writing]() {
                if (!cancel->load()) { link->receive(result, writing); }
            }, Qt::QueuedConnection);
        }
        m_timer->start(m_interval);
    }

    OnlineSimulationLink *m_link;
    PlcSerialSettings m_settings;
    PlcSerialIoFactory m_factory;
    std::shared_ptr<std::atomic_bool> m_cancel;
    QList<SimulationModbusRequest> m_scans;
    QQueue<SimulationModbusRequest> m_readback; //20261124-14 add
    SimulationModbusTransport m_transport;
    std::optional<SimulationModbusRequest> m_write;
    QTimer *m_timer = nullptr;
    qsizetype m_next = 0;
    const int m_station;
    const int m_interval;
    bool m_toggle = false;
};

/******************************************************************************
 * Function Name   : OnlineSimulationLink::OnlineSimulationLink
 * Description     : Capture project connection settings without opening any endpoint.
 * Input Parameters: project, factory, parent
 * Return Value    : None
 * Date            : 2026.11.24
 ******************************************************************************/
OnlineSimulationLink::OnlineSimulationLink(const HmiProject &project, const PlcSerialIoFactory &factory, QObject *parent) //20261124-13 add
    : QObject(parent), m_configuration(project.configurationNodes), m_factory(factory)
{
    m_context.connections = project.connections;
    m_catalog.load(QStringLiteral(":/plc/plc-catalog.json"), &m_catalogError);
}

OnlineSimulationLink::~OnlineSimulationLink()
{
    disconnectPort();
}

/******************************************************************************
 * Function Name   : OnlineSimulationLink::access
 * Description     : Bind the existing runtime to mapping, confirmed reads and queued writes.
 * Input Parameters: None
 * Return Value    : Callbacks owned by the simulation window.
 * Date            : 2026.11.24
 ******************************************************************************/
SimulationExternalAccess OnlineSimulationLink::access() //20261124-13 add
{
    return {
        [this](const QString &text, bool bit, int span, bool write, QString *error) {
            return bind(text, bit, span, write, error);
        },
        [this](const QString &text, bool bit, int span, QVector<u16> *values, QString *error) {
            return read(text, bit, span, values, error);
        },
        [this](const QString &text, bool bit, const QVector<u16> &values, bool toggle, QString *error) {
            return queueWrite(text, bit, values, toggle, error);
        }
    };
}

/******************************************************************************
 * Function Name   : OnlineSimulationLink::bind
 * Description     : Translate only the adopted zero-based standard RTU families on one configured route.
 * Input Parameters: text, bit, span, write, error
 * Return Value    : True when the runtime can scan or write this address.
 * Date            : 2026.11.24
 ******************************************************************************/
bool OnlineSimulationLink::bind(const QString &text, bool bit, int span, bool write, QString *error) //20261124-13 add
{
    PlcAddressContext context = m_context;
    PlcAddress plc;
    const PlcConnection *connection;
    const PlcProfile *profile;
    Binding binding;
    QString family;
    QString channel;
    QStringList parts;
    QJsonObject serial;
    HmiConfigurationNode node;
    PlcSerialSettings settings;
    QByteArray frame;
    bool stationOk;
    int station;

    if (!m_catalogError.isEmpty()) { return linkError(error, m_catalogError); }
    if (span < 1 || span > 2 || (bit && span != 1)) { return linkError(error, QStringLiteral("在线地址字长无效。")); }
    context.role = bit ? PlcAddressRole::Bit : span == 2 ? PlcAddressRole::Dword : PlcAddressRole::Word;
    context.transferCount = u32(span);
    if (!parseQualifiedPlcAddressText(text, context, m_catalog, &plc, error)) { return false; }
    connection = plcConnection(context, plc.connectionId);
    profile = connection == nullptr ? nullptr : m_catalog.profile(connection->profileId);
    if (profile == nullptr || !plcProfileHasModbusCodec(*profile) || connection->channel == PlcChannel::Unspecified)
    { return linkError(error, QStringLiteral("在线模拟仅支持已配置设备串口的标准 Modbus RTU 连接。")); }
    if (!m_connectionId.isEmpty() && m_connectionId != connection->id)
    { return linkError(error, QStringLiteral("当前画面使用多个 PLC 连接，在线模拟每次支持一个连接和站号。")); }

    family = plc.family;
    parts = plc.value.split(QChar(u'.'));
    binding.scan.address = parts.first().toUInt();
    binding.scan.count = u32(span);
    if (family == QStringLiteral("0x") || family == QStringLiteral("1x"))
    {
        binding.writable = family == QStringLiteral("0x");
        binding.scan.operation = binding.writable ? SimulationModbusOperation::ReadCoils
                                                 : SimulationModbusOperation::ReadDiscreteInputs;
    }
    else if (family == QStringLiteral("3x") || family == QStringLiteral("4x") ||
             family == QStringLiteral("3x_dw") || family == QStringLiteral("4x_dw") ||
             family == QStringLiteral("3x_bit") || family == QStringLiteral("4x_bit"))
    {
        binding.scan.operation = family.startsWith(QChar(u'4')) ? SimulationModbusOperation::ReadHoldingRegisters
                                                                : SimulationModbusOperation::ReadInputRegisters;
        binding.writable = !bit && binding.scan.operation == SimulationModbusOperation::ReadHoldingRegisters;
        if (parts.size() == 2) { binding.registerBit = parts.last().toInt(); }
    }
    else { return linkError(error, QStringLiteral("在线模拟未支持地址族 %1。").arg(family)); }
    if (!bit && connection->maximumWords < span)
    { return linkError(error, QStringLiteral("此跨字控件超过该连接的单次读写字数。")); }
    if (write && !binding.writable)
    { return linkError(error, QStringLiteral("在线写入仅支持 0x 线圈和 4x 保持寄存器。")); }
    binding.scan.id = QStringLiteral("%1/%2/%3").arg(int(binding.scan.operation)).arg(binding.scan.address).arg(span);
    station = connection->station.toInt(&stationOk);
    if (!stationOk || station < 1 || station > 247)
    { return linkError(error, QStringLiteral("在线模拟需要 1 至 247 的 Modbus 从站号。")); }
    if (!buildSimulationModbusRequest(station, binding.scan, &frame, error)) { return false; }

    if (m_connectionId.isEmpty())
    {
        channel = connection->channel == PlcChannel::Com1 ? QStringLiteral("COM1") : QStringLiteral("COM2");
        serial = PlcSerialPortSettings::defaults(channel);
        Q_FOREACH (node, m_configuration)
        {
            if (node.active && node.kind == QStringLiteral("serialPort") && node.name == channel)
            { serial = node.properties; break; }
        }
        settings.portName = serial.value(QStringLiteral("onlinePort")).toString();
        settings.baudRate = serial.value(QStringLiteral("baudRate")).toString().toInt();
        settings.dataBits = serial.value(QStringLiteral("dataBits")).toString().toInt();
        settings.parity = serial.value(QStringLiteral("parity")).toString() == QStringLiteral("ODD") ? 3 :
                          serial.value(QStringLiteral("parity")).toString() == QStringLiteral("EVEN") ? 2 : 0;
        settings.stopBits = serial.value(QStringLiteral("stopBits")).toString().toInt();
        settings.timeoutMs = connection->timeoutMs;
        if (!validatePlcOnlineSerialSettings(settings, error)) { return false; } //20261125-7 change
        m_connectionId = connection->id;
        m_station = station;
        m_frameInterval = qMax(20, connection->frameIntervalMs);
        m_settings = settings;
    }
    m_bindings.insert(bindingKey(text, bit, span), binding);
    m_scans.insert(binding.scan.id, binding.scan);
    return true;
}

/******************************************************************************
 * Function Name   : OnlineSimulationLink::read
 * Description     : Return only a complete validated response for the requested range.
 * Input Parameters: text, bit, span, values, error
 * Return Value    : True for a confirmed value.
 * Date            : 2026.11.24
 ******************************************************************************/
bool OnlineSimulationLink::read(const QString &text, bool bit, int span, QVector<u16> *values, QString *error) const //20261124-13 add
{
    const auto binding = m_bindings.constFind(bindingKey(text, bit, span));
    if (binding == m_bindings.cend() || !m_values.contains(binding->scan.id))
    { return linkError(error, QStringLiteral("PLC 数据未确认。")); }
    *values = m_values.value(binding->scan.id);
    if (binding->registerBit >= 0) { (*values)[0] = (values->first() >> binding->registerBit) & 1u; }
    return true;
}

/******************************************************************************
 * Function Name   : OnlineSimulationLink::queueWrite
 * Description     : Queue one explicitly triggered coil or holding register write.
 * Input Parameters: text, bit, values, toggle, error
 * Return Value    : True when the worker has been given the command.
 * Date            : 2026.11.24
 ******************************************************************************/
bool OnlineSimulationLink::queueWrite(const QString &text, bool bit, const QVector<u16> &values, bool toggle, QString *error) //20261124-13 add
{
    OnlineSimulationWorker *worker = m_worker;
    SimulationModbusRequest request;
    const auto binding = m_bindings.constFind(bindingKey(text, bit, int(values.size())));
    u16 value;

    if (binding == m_bindings.cend() || !binding->writable || !m_active ||
        !m_values.contains(binding->scan.id) || m_writePending)
    { return linkError(error, QStringLiteral("PLC 尚未确认该地址，或上一次写入仍待确认。")); }
    request.id = binding->scan.id;
    request.address = binding->scan.address;
    request.count = binding->scan.count;
    request.value = values.first();
    request.operation = bit ? SimulationModbusOperation::WriteSingleCoil : SimulationModbusOperation::WriteSingleRegister;
    if (values.size() > 1)
    {
        request.operation = SimulationModbusOperation::WriteMultipleRegisters;
        request.value = 0; // Multiwrite carries its operands only in the vector.
        Q_FOREACH (value, values) { request.values.append(value); }
    }
    m_writePending = true;
    m_status = QStringLiteral("写入待 PLC 确认…");
    QMetaObject::invokeMethod(worker, [worker, request, toggle]() { worker->queueWrite(request, toggle); }, Qt::QueuedConnection);
    emit updated();
    return true;
}

/******************************************************************************
 * Function Name   : OnlineSimulationLink::connectPort
 * Description     : Open the explicitly chosen PC port on its own worker thread.
 * Input Parameters: port, error
 * Return Value    : True once the asynchronous attempt is scheduled.
 * Date            : 2026.11.24
 ******************************************************************************/
bool OnlineSimulationLink::connectPort(const QString &port, QString *error) //20261124-13 add
{
    PlcSerialSettings configured = m_settings;

    if (!available() || m_active) { return linkError(error, QStringLiteral("当前模拟无可连接的PLC地址或会话已启动。")); }
    configured.portName = port.trimmed();
    if (!validatePlcOnlineSerialSettings(configured, error)) { return false; } //20261125-7 change
    disconnectPort();
    m_cancel = std::make_shared<std::atomic_bool>(false);
    m_thread = new QThread(this);
    m_worker = new OnlineSimulationWorker(this, configured.portName);
    m_worker->moveToThread(m_thread);
    connect(m_thread, &QThread::started, m_worker, &OnlineSimulationWorker::start);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    m_active = true;
    m_status = QStringLiteral("正在连接 %1，等待读取 PLC…").arg(configured.portName);
    m_thread->start();
    emit updated();
    return true;
}

/******************************************************************************
 * Function Name   : OnlineSimulationLink::disconnectPort
 * Description     : Cancel queued I/O, close the worker-owned endpoint and invalidate remote data.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.24
 ******************************************************************************/
void OnlineSimulationLink::disconnectPort() //20261124-13 add
{
    const bool unconfirmed = m_writePending;

    if (m_thread != nullptr)
    {
        m_cancel->store(true);
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        m_thread = nullptr;
        m_worker = nullptr;
    }
    m_values.clear();
    m_active = false;
    m_writePending = false;
    m_status = available() ? QStringLiteral("PLC 未连接。") : QStringLiteral("当前画面没有 PLC 地址。");
    if (unconfirmed) { m_status += QStringLiteral(" 上一次写入未确认，请核对 PLC 状态。"); }
    emit updated();
}

/******************************************************************************
 * Function Name   : OnlineSimulationLink::receive
 * Description     : Publish a validated scan separately from a write acknowledgement.
 * Input Parameters: result, write
 * Return Value    : None
 * Date            : 2026.11.24
 ******************************************************************************/
void OnlineSimulationLink::receive(const SimulationModbusResult &result, bool write) //20261124-14 change
{
    SimulationModbusRequest scan;

    if (write)
    {
        // An echo confirms the write, but the affected bits and words are unknown until read back.
        Q_FOREACH (scan, m_scans)
        {
            if (writeOverlapsScan(result.request, scan)) { m_values.remove(scan.id); }
        }
        m_writePending = false;
        m_status = QStringLiteral("PLC 已确认写入，画面值随读取刷新。");
    }
    else
    {
        if (m_values.value(result.request.id) == result.values) { return; } // Avoid repainting a stable PLC value at each poll.
        if (m_values.isEmpty() && !m_writePending) { m_status = QStringLiteral("PLC 已连接，正在刷新地址。"); }
        m_values.insert(result.request.id, result.values);
    }
    emit updated();
}

/******************************************************************************
 * Function Name   : OnlineSimulationLink::failed
 * Description     : Clear stale monitor values and surface a failure until explicit reconnection.
 * Input Parameters: error
 * Return Value    : None
 * Date            : 2026.11.24
 ******************************************************************************/
void OnlineSimulationLink::failed(const QString &error) //20261124-13 add
{
    m_values.clear();
    m_active = false;
    m_writePending = false;
    m_status = QStringLiteral("PLC 通讯已停止。%1").arg(error);
    emit updated();
}
