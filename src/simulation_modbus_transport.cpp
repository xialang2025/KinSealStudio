#include "simulation_modbus_transport.h"

#include <QElapsedTimer>
#include <QJsonArray>

//20261114-6 add: Only standard Modbus RTU frames are implemented; native HMI routing is separate.
namespace
{
/******************************************************************************
 * Function Name   : reject
 * Description     : Set a caller diagnostic without changing a previously constructed frame.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.14
 ******************************************************************************/
bool reject(QString *error, const QString &message)
{
    if (error != nullptr)
    {
        *error = message;
    }
    return false;
}

/******************************************************************************
 * Function Name   : isWrite
 * Description     : Identify operations that may change a remote slave before confirmation.
 * Input Parameters: operation
 * Return Value    : True for functions 05 and 06.
 * Date            : 2026.11.14
 ******************************************************************************/
bool isWrite(SimulationModbusOperation operation)
{
    return operation == SimulationModbusOperation::WriteSingleCoil ||
           operation == SimulationModbusOperation::WriteSingleRegister;
}

/******************************************************************************
 * Function Name   : appendWord
 * Description     : Append an unsigned 16-bit PDU field in network byte order.
 * Input Parameters: bytes, value
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void appendWord(QByteArray *bytes, u32 value)
{
    bytes->append(char((value >> 8) & 0xffU));
    bytes->append(char(value & 0xffU));
}

/******************************************************************************
 * Function Name   : validException
 * Description     : Accept the assigned standard Modbus exception codes in this protocol slice.
 * Input Parameters: code
 * Return Value    : True for 01..06, 08, 0a or 0b.
 * Date            : 2026.11.14
 ******************************************************************************/
bool validException(u8 code)
{
    return (code >= 1 && code <= 6) || code == 8 || code == 10 || code == 11;
}
}

/******************************************************************************
 * Function Name   : simulationModbusCrc16
 * Description     : Apply the reflected 0xa001 polynomial to every byte and bit.
 * Input Parameters: bytes
 * Return Value    : RTU CRC16, low byte first on the wire.
 * Date            : 2026.11.14
 ******************************************************************************/
u16 simulationModbusCrc16(const QByteArray &bytes) //20261114-6 add
{
    u16 crc;
    int index;
    int bit;

    crc = 0xffffU;
    for (index = 0; index < bytes.size(); ++index)
    {
        crc = u16(crc ^ u8(bytes[index]));
        for (bit = 0; bit < 8; ++bit)
        {
            crc = (crc & 1U) != 0U ? u16((crc >> 1) ^ 0xa001U) : u16(crc >> 1);
        }
    }
    return crc;
}

/******************************************************************************
 * Function Name   : buildSimulationModbusRequest
 * Description     : Reject broadcasts, wrapping ranges and unsupported operations before I/O.
 * Input Parameters: station, request, frame, error
 * Return Value    : True with the complete request; outputs are unchanged on rejection.
 * Date            : 2026.11.14
 ******************************************************************************/
bool buildSimulationModbusRequest(int station, const SimulationModbusRequest &request,
                                  QByteArray *frame, QString *error) //20261114-6 add
{
    QByteArray built;
    u32 payload;
    u16 crc;

    if (frame == nullptr || station < 1 || station > 247)
    {
        return reject(error, QStringLiteral("必须提供输出和明确的Modbus单播站号1..247。"));
    }
    if (request.id.trimmed().isEmpty() || request.id.size() > 256 || request.address > 65535U ||
        request.count == 0 || request.count > 2000U || request.address + request.count > 65536U)
    {
        return reject(error, QStringLiteral("请求标识、零起始地址或访问数量无效。"));
    }
    payload = request.count;
    if (request.operation == SimulationModbusOperation::ReadCoils)
    {
        if (request.count > 2000U || request.value != 0)
        {
            return reject(error, QStringLiteral("读线圈限1..2000位，且不得携带写入值。"));
        }
    }
    else if (request.operation == SimulationModbusOperation::ReadHoldingRegisters)
    {
        if (request.count > 125U || request.value != 0)
        {
            return reject(error, QStringLiteral("读保持寄存器限1..125字，且不得携带写入值。"));
        }
    }
    else if (isWrite(request.operation))
    {
        if (request.count != 1 || request.value > 65535U ||
            (request.operation == SimulationModbusOperation::WriteSingleCoil && request.value > 1))
        {
            return reject(error, QStringLiteral("单写数量必须为1，线圈仅支持0/1，寄存器仅支持16位无符号值。"));
        }
        payload = request.operation == SimulationModbusOperation::WriteSingleCoil ?
                  (request.value == 0 ? 0U : 0xff00U) : request.value;
    }
    else
    {
        return reject(error, QStringLiteral("仅支持Modbus RTU功能01、03、05、06。"));
    }
    built.append(char(station));
    built.append(char(request.operation));
    appendWord(&built, request.address);
    appendWord(&built, payload);
    crc = simulationModbusCrc16(built);
    built.append(char(crc & 0xffU));
    built.append(char(crc >> 8));
    *frame = built;
    if (error != nullptr)
    {
        error->clear();
    }
    return true;
}

/******************************************************************************
 * Function Name   : simulationModbusResultToJson
 * Description     : Preserve frame and result attribution separately from physical verification.
 * Input Parameters: result
 * Return Value    : Controlled-replay evidence object; physical transfer and HMI flags are false.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject simulationModbusResultToJson(const SimulationModbusResult &result) //20261114-6 add
{
    QJsonArray values;
    QJsonObject object;
    int index;

    for (index = 0; index < result.values.size(); ++index)
    {
        values.append(int(result.values[index]));
    }
    object = QJsonObject{{QStringLiteral("scope"), QStringLiteral("controlled-modbus-rtu-replay")},
                         {QStringLiteral("requestId"), result.request.id},
                         {QStringLiteral("station"), result.station},
                         {QStringLiteral("function"), int(result.request.operation)},
                         {QStringLiteral("addressOffsetZeroBased"), double(result.request.address)},
                         {QStringLiteral("count"), double(result.request.count)},
                         {QStringLiteral("requestedValue"), double(result.request.value)},
                         {QStringLiteral("ok"), result.ok},
                         {QStringLiteral("errorCode"), result.errorCode},
                         {QStringLiteral("error"), result.error},
                         {QStringLiteral("requestHex"), QString::fromLatin1(result.requestFrame.toHex())},
                         {QStringLiteral("responseHex"), QString::fromLatin1(result.responseFrame.toHex())},
                         {QStringLiteral("values"), values},
                         {QStringLiteral("writeAcknowledged"), result.writeAcknowledged},
                         {QStringLiteral("requestMayHaveReachedSlave"), result.requestMayHaveReachedSlave},
                         {QStringLiteral("remoteStateUnconfirmed"), result.remoteStateUnconfirmed},
                         {QStringLiteral("exceptionCode"), int(result.exceptionCode)},
                         {QStringLiteral("discardedQueuedRequests"), result.discardedQueuedRequests},
                         {QStringLiteral("physicalEndpointUsed"), false},
                         {QStringLiteral("hardwareTransferVerified"), false},
                         {QStringLiteral("hmiBehaviorVerified"), false}};
    return object;
}

/******************************************************************************
 * Function Name   : SimulationModbusTransport::~SimulationModbusTransport
 * Description     : Release a live injected endpoint and discard all unsent requests.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
SimulationModbusTransport::~SimulationModbusTransport() //20261114-6 add
{
    disconnectFromStation();
}

/******************************************************************************
 * Function Name   : SimulationModbusTransport::connectToStation
 * Description     : Validate all bounds and callbacks before opening a new controlled session.
 * Input Parameters: station, io, options, error
 * Return Value    : True when the endpoint opens successfully.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationModbusTransport::connectToStation(int station, const SimulationModbusIo &io,
                                                const SimulationModbusOptions &options,
                                                QString *error) //20261114-6 add
{
    QString openError;

    if (m_connected || m_processing || m_transitioning)
    {
        return reject(error, QStringLiteral("已连接或正在处理事务，不能替换会话。"));
    }
    if (station < 1 || station > 247 || options.maxPendingRequests < 1 ||
        options.maxPendingRequests > 1024 || options.timeoutMs < 1 || options.timeoutMs > 60000 ||
        !io.open || !io.write || !io.read || !io.close)
    {
        return reject(error, QStringLiteral("站号、队列上限、超时或注入I/O回调无效。"));
    }
    m_transitioning = true;
    m_disconnectRequested = false;
    if (!io.open(&openError))
    {
        m_transitioning = false;
        m_disconnectRequested = false;
        openError = openError.isEmpty() ? QStringLiteral("注入端点打开失败。") : openError;
        if (m_firstError.isEmpty())
        {
            m_firstError = openError;
        }
        return reject(error, openError);
    }
    m_io = io;
    m_options = options;
    m_station = station;
    m_connected = true;
    m_transitioning = false;
    m_cancelledQueuedRequests = 0;
    m_firstError.clear();
    m_queue.clear();
    if (m_disconnectRequested)
    {
        m_firstError = QStringLiteral("打开端点期间已请求退出会话。");
        closeEndpoint();
        return reject(error, m_firstError);
    }
    if (error != nullptr)
    {
        error->clear();
    }
    return true;
}

/******************************************************************************
 * Function Name   : SimulationModbusTransport::enqueue
 * Description     : Copy validated requests without sending or changing the remote process image.
 * Input Parameters: request, error
 * Return Value    : True when enqueued; reject disconnection, reentry, duplicates and overflow.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationModbusTransport::enqueue(const SimulationModbusRequest &request, QString *error)
{
    QByteArray frame;
    int index;

    if (!m_connected || m_processing || m_transitioning || m_disconnectRequested)
    {
        return reject(error, QStringLiteral("未连接或事务回调正在执行，不能加入新请求。"));
    }
    if (m_queue.size() >= m_options.maxPendingRequests)
    {
        return reject(error, QStringLiteral("未发送请求队列已满。"));
    }
    for (index = 0; index < m_queue.size(); ++index)
    {
        if (m_queue[index].id == request.id)
        {
            return reject(error, QStringLiteral("队列中已有相同请求标识。"));
        }
    }
    if (!buildSimulationModbusRequest(m_station, request, &frame, error))
    {
        return false;
    }
    m_queue.enqueue(request);
    return true;
}

/******************************************************************************
 * Function Name   : SimulationModbusTransport::fail
 * Description     : Preserve first failure, suppress partial values and drop unsent requests.
 * Input Parameters: result, code, message
 * Return Value    : Failure result with the remote write uncertainty explicitly retained.
 * Date            : 2026.11.14
 ******************************************************************************/
SimulationModbusResult SimulationModbusTransport::fail(SimulationModbusResult result,
                                                      const QString &code, const QString &message)
{
    result.ok = false;
    result.errorCode = code;
    result.error = message;
    result.values.clear();
    result.writeAcknowledged = false;
    result.remoteStateUnconfirmed = isWrite(result.request.operation) && result.requestMayHaveReachedSlave;
    result.discardedQueuedRequests = int(m_queue.size()) + m_cancelledQueuedRequests;
    if (m_firstError.isEmpty())
    {
        m_firstError = message;
    }
    m_queue.clear();
    m_processing = false;
    closeEndpoint();
    return result;
}

/******************************************************************************
 * Function Name   : SimulationModbusTransport::closeEndpoint
 * Description     : Close at most once and retain cleanup failure without masking prior errors.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransport::closeEndpoint()
{
    QString closeError;
    SimulationModbusIo io;

    if (!m_connected)
    {
        return;
    }
    io = m_io;
    m_transitioning = true;
    m_connected = false;                       // Invalidate before close to prevent callback reentry.
    m_io = SimulationModbusIo();
    m_disconnectRequested = false;
    m_cancelledQueuedRequests = 0;
    if (!io.close(&closeError) && m_firstError.isEmpty())
    {
        m_firstError = closeError.isEmpty() ? QStringLiteral("注入端点关闭失败。") : closeError;
    }
    m_transitioning = false;
    m_disconnectRequested = false;
}

/******************************************************************************
 * Function Name   : SimulationModbusTransport::processNext
 * Description     : Execute a single bounded RTU transaction with a complete frame boundary.
 * Input Parameters: None
 * Return Value    : Values or write acknowledgement only after station, length, CRC and PDU checks.
 * Date            : 2026.11.14
 ******************************************************************************/
SimulationModbusResult SimulationModbusTransport::processNext() //20261114-6 add
{
    SimulationModbusResult result;
    SimulationModbusReadChunk chunk;
    QElapsedTimer timer;
    QByteArray body;
    u16 receivedCrc;
    u16 computedCrc;
    u16 value;
    u8 function;
    u8 expectedFunction;
    u8 unusedMask;
    int written;
    int remaining;
    int expectedLength;
    int expectedByteCount;
    int index;
    int maxBytes;
    bool complete;

    if (m_processing || m_transitioning || !m_connected || m_queue.isEmpty())
    {
        result.errorCode = (m_processing || m_transitioning) ? QStringLiteral("reentrant_call") :
                           (!m_connected ? QStringLiteral("not_connected") : QStringLiteral("empty_queue"));
        result.error = QStringLiteral("当前没有可执行的已连接事务。十六进制地址均为零起始PDU偏移。");
        return result;
    }
    m_processing = true;
    m_disconnectRequested = false;
    m_cancelledQueuedRequests = 0;
    result.request = m_queue.dequeue();
    result.station = m_station;
    if (!buildSimulationModbusRequest(m_station, result.request, &result.requestFrame, &result.error))
    {
        return fail(result, QStringLiteral("invalid_request"), result.error);
    }
    timer.start();
    result.requestMayHaveReachedSlave = true;   // A failed driver return cannot prove zero wire activity.
    written = m_io.write(result.requestFrame, m_options.timeoutMs);
    if (m_disconnectRequested)
    {
        return fail(result, QStringLiteral("disconnected_during_transaction"), QStringLiteral("事务写入后会话退出，远端执行状态未确认。"));
    }
    if (written != result.requestFrame.size())
    {
        return fail(result, QStringLiteral("short_or_failed_write"), QStringLiteral("请求未完整写入；未自动重试，远端状态未确认。"));
    }
    complete = false;
    expectedFunction = u8(result.request.operation);
    expectedByteCount = result.request.operation == SimulationModbusOperation::ReadCoils ?
                        int((result.request.count + 7U) / 8U) : int(result.request.count * 2U);
    expectedLength = isWrite(result.request.operation) ? 8 : expectedByteCount + 5;
    while (!complete)
    {
        remaining = m_options.timeoutMs - int(timer.elapsed());
        if (remaining <= 0)
        {
            return fail(result, QStringLiteral("timeout"), QStringLiteral("Modbus事务超时，未发布部分读值。"));
        }
        maxBytes = 257 - int(result.responseFrame.size());
        chunk = m_io.read(maxBytes, remaining);
        if (chunk.bytes.size() > maxBytes)
        {
            return fail(result, QStringLiteral("read_contract_violation"), QStringLiteral("注入读取回调超过所请求的字节上限。"));
        }
        result.responseFrame.append(chunk.bytes);
        if (m_disconnectRequested)
        {
            return fail(result, QStringLiteral("disconnected_during_transaction"), QStringLiteral("事务接收期间会话退出，未发布数据。"));
        }
        if (!chunk.error.isEmpty())
        {
            return fail(result, QStringLiteral("read_error"), chunk.error);
        }
        if (chunk.timedOut || timer.elapsed() >= m_options.timeoutMs)
        {
            return fail(result, QStringLiteral("timeout"), QStringLiteral("Modbus事务超时，未发布部分读值。"));
        }
        if (chunk.bytes.isEmpty() && !chunk.endOfFrame)
        {
            return fail(result, QStringLiteral("read_no_progress"), QStringLiteral("读取回调未提供数据、帧结束或超时状态。"));
        }
        if (result.responseFrame.size() >= 1 && u8(result.responseFrame[0]) != u8(m_station))
        {
            return fail(result, QStringLiteral("wrong_station"), QStringLiteral("应答站号与显式请求站号不符。"));
        }
        if (result.responseFrame.size() >= 2)
        {
            function = u8(result.responseFrame[1]);
            if (function != expectedFunction && function != u8(expectedFunction | 0x80U))
            {
                return fail(result, QStringLiteral("wrong_function"), QStringLiteral("应答功能码与请求不符。"));
            }
            if ((function & 0x80U) != 0)
            {
                expectedLength = 5;
            }
            else if (!isWrite(result.request.operation) && result.responseFrame.size() >= 3 &&
                     u8(result.responseFrame[2]) != expectedByteCount)
            {
                return fail(result, QStringLiteral("wrong_byte_count"), QStringLiteral("应答数据长度字段与访问数量不符。"));
            }
        }
        if (result.responseFrame.size() > expectedLength)
        {
            return fail(result, QStringLiteral("extra_response_bytes"), QStringLiteral("应答帧包含超出预期的字节。"));
        }
        complete = chunk.endOfFrame;
    }
    if (result.responseFrame.size() != expectedLength)
    {
        return fail(result, QStringLiteral("truncated_response"), QStringLiteral("完整帧边界前的应答长度不足。"));
    }
    body = result.responseFrame.left(expectedLength - 2);
    receivedCrc = u16(u8(result.responseFrame[expectedLength - 2]) |
                      (u16(u8(result.responseFrame[expectedLength - 1])) << 8));
    computedCrc = simulationModbusCrc16(body);
    if (receivedCrc != computedCrc)
    {
        return fail(result, QStringLiteral("crc_mismatch"), QStringLiteral("应答CRC16不符，未发布数据。"));
    }
    if ((u8(result.responseFrame[1]) & 0x80U) != 0)
    {
        result.exceptionCode = u8(result.responseFrame[2]);
        if (!validException(result.exceptionCode))
        {
            return fail(result, QStringLiteral("invalid_exception_code"), QStringLiteral("应答包含未支持的Modbus异常码。"));
        }
        return fail(result, QStringLiteral("slave_exception"), QStringLiteral("从站返回Modbus异常码%1。未自动重试。").arg(result.exceptionCode));
    }
    if (isWrite(result.request.operation))
    {
        if (result.responseFrame != result.requestFrame)
        {
            return fail(result, QStringLiteral("write_echo_mismatch"), QStringLiteral("单写应答的地址或写入值与请求不符。"));
        }
        result.writeAcknowledged = true;
    }
    else if (result.request.operation == SimulationModbusOperation::ReadCoils)
    {
        if ((result.request.count % 8U) != 0)
        {
            unusedMask = u8(0xffU << (result.request.count % 8U));
            if ((u8(result.responseFrame[expectedLength - 3]) & unusedMask) != 0)
            {
                return fail(result, QStringLiteral("nonzero_coil_padding"), QStringLiteral("线圈应答最后一字节的未使用高位必须为零。"));
            }
        }
        for (index = 0; index < int(result.request.count); ++index)
        {
            value = u16((u8(result.responseFrame[3 + index / 8]) >> (index % 8)) & 1U);
            result.values.append(value);
        }
    }
    else
    {
        for (index = 0; index < int(result.request.count); ++index)
        {
            value = u16((u16(u8(result.responseFrame[3 + index * 2])) << 8) |
                        u8(result.responseFrame[4 + index * 2]));
            result.values.append(value);
        }
    }
    result.ok = true;
    result.remoteStateUnconfirmed = false;
    m_processing = false;
    return result;
}

/******************************************************************************
 * Function Name   : SimulationModbusTransport::disconnectFromStation
 * Description     : Clear pending work immediately and defer callback cleanup during execution.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransport::disconnectFromStation()
{
    m_cancelledQueuedRequests += int(m_queue.size());
    m_queue.clear();
    if (m_processing || m_transitioning)
    {
        m_disconnectRequested = true;
        return;
    }
    closeEndpoint();
}

/******************************************************************************
 * Function Name   : SimulationModbusTransport::isConnected
 * Description     : Expose current session usability without opening any physical endpoint.
 * Input Parameters: None
 * Return Value    : True for an open injected session without a pending disconnect.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationModbusTransport::isConnected() const
{
    return m_connected && !m_disconnectRequested;
}

/******************************************************************************
 * Function Name   : SimulationModbusTransport::pendingCount
 * Description     : Return queued unsent requests separately from the active transaction.
 * Input Parameters: None
 * Return Value    : Number of pending requests.
 * Date            : 2026.11.14
 ******************************************************************************/
int SimulationModbusTransport::pendingCount() const
{
    return int(m_queue.size());
}

/******************************************************************************
 * Function Name   : SimulationModbusTransport::firstError
 * Description     : Return the first failure even after additional cleanup failures.
 * Input Parameters: None
 * Return Value    : First failure text for the current or most recent session.
 * Date            : 2026.11.14
 ******************************************************************************/
QString SimulationModbusTransport::firstError() const
{
    return m_firstError;
}
