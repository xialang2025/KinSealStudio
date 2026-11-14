#include "plc_serial_session.h"
#include <QElapsedTimer>
#include <QJsonArray>
#include <QSerialPort>
#include <QSet>

namespace
{
struct OwnedSerialPort //20261114-8 add: Construct and destroy the device in the transaction's worker thread.
{
    QSerialPort port;
    QElapsedTimer lastReceive;
    bool received = false;
    int gapMs = 2;
    std::function<bool()> cancelled;
};

/******************************************************************************
 * Function Name   : qtSerialIo
 * Description     : Bind bounded synchronous serial callbacks with explicit RTU silence boundaries.
 * Input Parameters: settings, cancelled
 * Return Value    : Owned callbacks; construction alone does not open a port.
 * Date            : 2026.11.14
 ******************************************************************************/
SimulationModbusIo qtSerialIo(const PlcSerialSettings &settings, const std::function<bool()> &cancelled) //20261114-8 add
{
    std::shared_ptr<OwnedSerialPort> state = std::make_shared<OwnedSerialPort>();
    SimulationModbusIo io;
    int characterBits = 1 + settings.dataBits + settings.stopBits + (settings.parity == 0 ? 0 : 1);

    state->cancelled = cancelled;
    state->gapMs = settings.baudRate > 19200 ? 2 : qMax(2, (3500 * characterBits + settings.baudRate - 1) / settings.baudRate);
    io.open = [state, settings](QString *error) {
        if (state->cancelled()) { if (error != nullptr) { *error = QStringLiteral("串口打开前已取消。"); } return false; }
        state->port.setPortName(settings.portName);
        state->port.setReadBufferSize(512);
        if (!state->port.setBaudRate(settings.baudRate) || !state->port.setDataBits(QSerialPort::Data8) ||
            !state->port.setParity(QSerialPort::Parity(settings.parity)) ||
            !state->port.setStopBits(QSerialPort::StopBits(settings.stopBits)) ||
            !state->port.setFlowControl(QSerialPort::NoFlowControl) || !state->port.open(QIODevice::ReadWrite))
        {
            if (error != nullptr) { *error = state->port.errorString(); }
            state->port.close(); return false;
        }
        if (!state->port.clear(QSerialPort::Input))
        {
            if (error != nullptr) { *error = state->port.errorString(); }
            state->port.close(); return false;
        }
        return true;
    };
    io.write = [state](const QByteArray &bytes, int timeoutMs) {
        QElapsedTimer elapsed;
        qint64 accepted;
        int remaining;

        state->received = false; state->lastReceive.invalidate();
        if (state->cancelled()) { return -1; }
        elapsed.start(); accepted = state->port.write(bytes);
        if (accepted != bytes.size()) { return int(accepted); }
        while (state->port.bytesToWrite() > 0)
        {
            remaining = timeoutMs - int(elapsed.elapsed());
            if (state->cancelled() || remaining <= 0) { return -1; }
            if (!state->port.waitForBytesWritten(qMin(20, remaining)) &&
                state->port.error() != QSerialPort::TimeoutError && state->port.error() != QSerialPort::NoError) { return -1; }
        }
        return int(accepted);
    };
    io.read = [state](int maximum, int timeoutMs) {
        SimulationModbusReadChunk chunk;
        QElapsedTimer elapsed;
        int waitMs;
        int remaining;

        elapsed.start();
        while (true) //20261114-8 change: A polling slice is not a protocol timeout or an empty-progress chunk.
        {
            if (maximum < 1 || state->cancelled()) { chunk.error = QStringLiteral("串口接收已取消或预算无效。"); return chunk; }
            remaining = timeoutMs - int(elapsed.elapsed());
            if (remaining <= 0) { chunk.timedOut = true; return chunk; }
            if (state->port.bytesAvailable() > 0)
            {
                chunk.bytes = state->port.read(maximum);
                if (chunk.bytes.isEmpty()) { chunk.error = state->port.errorString(); return chunk; }
                state->received = true; state->lastReceive.start(); return chunk;
            }
            if (state->received && state->lastReceive.elapsed() >= state->gapMs) { chunk.endOfFrame = true; return chunk; }
            waitMs = qMin(20, remaining);
            if (state->received) { waitMs = qMin(waitMs, qMax(1, state->gapMs - int(state->lastReceive.elapsed()))); }
            if (!state->port.waitForReadyRead(qMax(1, waitMs)) &&
                state->port.error() != QSerialPort::TimeoutError && state->port.error() != QSerialPort::NoError)
            { chunk.error = state->port.errorString(); return chunk; }
        }
    };
    io.close = [state](QString *) { state->port.close(); return !state->port.isOpen(); };
    return io;
}
}

/******************************************************************************
 * Function Name   : validatePlcSerialSettings
 * Description     : Validate explicit port, RTU framing and adopted transaction budgets without touching a device.
 * Input Parameters: settings, error
 * Return Value    : True for valid settings, false with a diagnostic otherwise.
 * Date            : 2026.11.14
 ******************************************************************************/
bool validatePlcSerialSettings(const PlcSerialSettings &settings, QString *error) //20261114-8 add
{
    bool valid = !settings.portName.isEmpty() && settings.portName == settings.portName.trimmed() && settings.portName.size() <= 256;

    for (const QChar ch : settings.portName) { if (ch.unicode() < 32 || ch.unicode() == 127) { valid = false; } }
    valid = valid && settings.baudRate >= 50 && settings.baudRate <= 4000000 && settings.dataBits == 8 &&
        (settings.parity == 0 || settings.parity == 2 || settings.parity == 3) &&
        (settings.stopBits == 1 || settings.stopBits == 2) && settings.timeoutMs >= 1 && settings.timeoutMs <= 60000 &&
        settings.totalTimeoutMs >= settings.timeoutMs && settings.totalTimeoutMs <= 120000;
    if (!valid && error != nullptr) { *error = QStringLiteral("请选择PC串口；RTU使用8数据位、无/偶/奇校验、1/2停止位及有效超时。"); }
    return valid;
}

/******************************************************************************
 * Function Name   : runPlcSerialBatch
 * Description     : Validate every request before opening an endpoint and publish acknowledged results without speculative write rollback.
 * Input Parameters: settings, station, requests, cancellation, factory
 * Return Value    : Bounded receipt; default factory uses QSerialPort, injected factories remain explicitly nonphysical.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject runPlcSerialBatch(const PlcSerialSettings &settings, int station,
                              const QVector<SimulationModbusRequest> &requests,
                              const std::shared_ptr<std::atomic_bool> &cancellation,
                              const PlcSerialIoFactory &factory) //20261114-8 add
{
    QElapsedTimer elapsed;
    SimulationModbusTransport transport;
    SimulationModbusOptions options;
    SimulationModbusIo io;
    SimulationModbusResult result;
    QJsonObject report;
    QJsonObject value;
    QJsonArray results;
    QSet<QString> ids;
    QByteArray frame;
    QString error;
    std::function<bool()> cancelled;
    bool opened = false;
    bool closeAttempted = false;
    bool closeConfirmed = false;
    bool physical = !bool(factory);
    bool allSucceeded = true;
    std::function<bool(QString *)> closeOwned;

    report = QJsonObject{{QStringLiteral("format"), QStringLiteral("KinSealLite-pc-rtu-batch-v1")},
        {QStringLiteral("port"), settings.portName}, {QStringLiteral("station"), station},
        {QStringLiteral("endpointKind"), physical ? QStringLiteral("pc-serial") : QStringLiteral("injected-controlled-endpoint")},
        {QStringLiteral("physicalEndpointUsed"), false}, {QStringLiteral("hmiExecutionConfirmed"), false},
        {QStringLiteral("projectAddressMapping"), false}, {QStringLiteral("automaticRetries"), 0}, {QStringLiteral("ok"), false},
        {QStringLiteral("opened"), false}, {QStringLiteral("sessionInactive"), true}, {QStringLiteral("closed"), true},
        {QStringLiteral("closeAttempted"), false}, {QStringLiteral("closeConfirmed"), false},
        {QStringLiteral("cancelled"), cancellation && cancellation->load()},
        {QStringLiteral("totalDeadlineReached"), false}, {QStringLiteral("results"), QJsonArray()},
        {QStringLiteral("requestedCount"), requests.size()}, {QStringLiteral("completedCount"), 0}};
    if (!validatePlcSerialSettings(settings, &error) || requests.isEmpty() || requests.size() > 256)
    { report.insert(QStringLiteral("error"), error.isEmpty() ? QStringLiteral("手动批次须有1..256项请求。") : error); return report; }
    for (const SimulationModbusRequest &request : requests)
    {
        if (!buildSimulationModbusRequest(station, request, &frame, &error) || ids.contains(request.id))
        { report.insert(QStringLiteral("error"), error.isEmpty() ? QStringLiteral("批次请求标识重复。") : error); return report; }
        ids.insert(request.id);
    }
    elapsed.start();
    cancelled = [&elapsed, cancellation, settings]() {
        return (cancellation && cancellation->load()) || elapsed.elapsed() >= settings.totalTimeoutMs;
    };
    if (cancelled())
    {
        report.insert(QStringLiteral("cancelled"), cancellation && cancellation->load());
        report.insert(QStringLiteral("totalDeadlineReached"), elapsed.elapsed() >= settings.totalTimeoutMs);
        report.insert(QStringLiteral("elapsedMs"), elapsed.elapsed());
        report.insert(QStringLiteral("error"), QStringLiteral("批次打开端口前已取消或超过总时限。")); return report;
    }
    io = factory ? factory(settings, cancelled) : qtSerialIo(settings, cancelled);
    //20261114-8 add: Factory work consumes the same total budget; cancellation cannot open a new endpoint afterwards.
    if (cancelled())
    {
        report.insert(QStringLiteral("cancelled"), cancellation && cancellation->load());
        report.insert(QStringLiteral("totalDeadlineReached"), elapsed.elapsed() >= settings.totalTimeoutMs);
        report.insert(QStringLiteral("elapsedMs"), elapsed.elapsed());
        report.insert(QStringLiteral("error"), QStringLiteral("批次创建端点期间已取消或超过总时限。"));
        return report;
    }
    closeOwned = io.close;
    if (closeOwned)
    {
        io.close = [&closeAttempted, &closeConfirmed, closeOwned](QString *detail) {
            closeAttempted = true; closeConfirmed = closeOwned(detail); return closeConfirmed;
        };
    }
    options.timeoutMs = settings.timeoutMs; options.maxPendingRequests = 256; options.cancellationRequested = cancelled;
    opened = transport.connectToStation(station, io, options, &error);
    report.insert(QStringLiteral("physicalEndpointUsed"), physical && opened);
    report.insert(QStringLiteral("opened"), opened);
    if (opened)
    {
        for (const SimulationModbusRequest &request : requests)
        {
            if (!transport.enqueue(request, &error)) { allSucceeded = false; break; }
        }
        while (allSucceeded && transport.pendingCount() > 0)
        {
            result = transport.processNext(); value = simulationModbusResultToJson(result);
            value.insert(QStringLiteral("scope"), physical ? QStringLiteral("pc-serial-response") : QStringLiteral("controlled-rtu-session"));
            value.insert(QStringLiteral("physicalEndpointUsed"), physical);
            value.insert(QStringLiteral("hmiExecutionConfirmed"), false);
            results.append(value);
            if (!result.ok) { allSucceeded = false; error = result.error; }
        }
    }
    else { allSucceeded = false; }
    transport.disconnectFromStation();
    if (!transport.firstError().isEmpty()) { allSucceeded = false; if (error.isEmpty()) { error = transport.firstError(); } }
    report.insert(QStringLiteral("ok"), allSucceeded && results.size() == requests.size());
    report.insert(QStringLiteral("error"), error); report.insert(QStringLiteral("results"), results);
    report.insert(QStringLiteral("requestedCount"), requests.size()); report.insert(QStringLiteral("completedCount"), results.size());
    report.insert(QStringLiteral("elapsedMs"), elapsed.elapsed()); report.insert(QStringLiteral("cancelled"), cancellation && cancellation->load());
    report.insert(QStringLiteral("totalDeadlineReached"), elapsed.elapsed() >= settings.totalTimeoutMs);
    report.insert(QStringLiteral("sessionInactive"), !transport.isConnected());
    report.insert(QStringLiteral("closeAttempted"), closeAttempted); report.insert(QStringLiteral("closeConfirmed"), closeConfirmed);
    report.insert(QStringLiteral("closed"), !opened || closeConfirmed); //20261114-8 change: Inactive state alone cannot prove a failed close released its endpoint.
    report.insert(QStringLiteral("timeoutMs"), settings.timeoutMs); report.insert(QStringLiteral("totalTimeoutMs"), settings.totalTimeoutMs);
    report.insert(QStringLiteral("serialParameters"), QJsonObject{{QStringLiteral("baud"), settings.baudRate}, {QStringLiteral("dataBits"), settings.dataBits},
        {QStringLiteral("parity"), settings.parity}, {QStringLiteral("stopBits"), settings.stopBits}});
    return report;
}
