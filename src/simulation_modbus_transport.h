#ifndef SIMULATION_MODBUS_TRANSPORT_H
#define SIMULATION_MODBUS_TRANSPORT_H

#include <QByteArray>
#include <QJsonObject>
#include <QQueue>
#include <QString>
#include <QVector>
#include <functional>

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

//20261114-6 add: Standard RTU offsets are explicit; no native address-family mapping is inferred.
enum class SimulationModbusOperation { ReadCoils = 1, ReadHoldingRegisters = 3,
                                       WriteSingleCoil = 5, WriteSingleRegister = 6 };

struct SimulationModbusRequest
{
    QString id;
    SimulationModbusOperation operation = SimulationModbusOperation::ReadHoldingRegisters;
    u32 address = 0;                        // Zero-based PDU offset, never a 40001-style label.
    u32 count = 1;
    u32 value = 0;                          // Logical 0/1 for coils; unsigned 16-bit for registers.
};

struct SimulationModbusReadChunk
{
    QByteArray bytes;
    bool endOfFrame = false;                // Caller confirms an RTU gap or an explicit replay boundary.
    bool timedOut = false;
    QString error;
};

struct SimulationModbusIo
{
    std::function<bool(QString *)> open;     // Failed open must release resources it acquired.
    std::function<int(const QByteArray &, int)> write;
    std::function<SimulationModbusReadChunk(int, int)> read;
    std::function<bool(QString *)> close;
};

struct SimulationModbusOptions
{
    int maxPendingRequests = 32;             // Bounded queue: 1..1024.
    int timeoutMs = 1000;                    // One transaction deadline: 1..60000 ms, no retries.
};

struct SimulationModbusResult
{
    SimulationModbusRequest request;
    int station = 0;
    bool ok = false;
    QString errorCode;
    QString error;
    QByteArray requestFrame;
    QByteArray responseFrame;
    QVector<u16> values;                     // Published together only after complete validation.
    bool writeAcknowledged = false;
    bool requestMayHaveReachedSlave = false;
    bool remoteStateUnconfirmed = false;     // An attempted write cannot be promised to roll back.
    u8 exceptionCode = 0;
    int discardedQueuedRequests = 0;
};

/******************************************************************************
 * Function Name   : simulationModbusCrc16
 * Description     : Compute standard Modbus RTU CRC16 with initial value 0xffff.
 * Input Parameters: bytes
 * Return Value    : CRC value; the low byte is transmitted first.
 * Date            : 2026.11.14
 ******************************************************************************/
u16 simulationModbusCrc16(const QByteArray &bytes); //20261114-6 add

/******************************************************************************
 * Function Name   : buildSimulationModbusRequest
 * Description     : Validate an explicit station and construct an eight-byte RTU request.
 * Input Parameters: station, request, frame, error
 * Return Value    : True with a frame; the output is unchanged after invalid input.
 * Date            : 2026.11.14
 ******************************************************************************/
bool buildSimulationModbusRequest(int station, const SimulationModbusRequest &request,
                                  QByteArray *frame, QString *error = nullptr);

/******************************************************************************
 * Function Name   : simulationModbusResultToJson
 * Description     : Serialize controlled protocol evidence without claiming physical endpoint use.
 * Input Parameters: result
 * Return Value    : JSON result with explicit replay scope and hardware evidence flags.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject simulationModbusResultToJson(const SimulationModbusResult &result);

//20261114-6 add: Single-owner synchronous adapter. Callbacks obey deadlines and do not throw.
// This module supplies no serial, USB, socket, PLC-profile or native-driver implementation.
class SimulationModbusTransport
{
public:
    /******************************************************************************
     * Function Name   : SimulationModbusTransport::~SimulationModbusTransport
     * Description     : Close the injected endpoint and discard all unsent requests.
     * Input Parameters: None
     * Return Value    : None
     * Date            : 2026.11.14
     ******************************************************************************/
    ~SimulationModbusTransport();

    SimulationModbusTransport() = default;
    SimulationModbusTransport(const SimulationModbusTransport &) = delete;
    SimulationModbusTransport &operator=(const SimulationModbusTransport &) = delete;

    /******************************************************************************
     * Function Name   : SimulationModbusTransport::connectToStation
     * Description     : Open a fully injected endpoint for an explicit unicast station.
     * Input Parameters: station, io, options, error
     * Return Value    : True after open; an already connected session is preserved on rejection.
     * Date            : 2026.11.14
     ******************************************************************************/
    bool connectToStation(int station, const SimulationModbusIo &io,
                           const SimulationModbusOptions &options = {}, QString *error = nullptr);

    /******************************************************************************
     * Function Name   : SimulationModbusTransport::enqueue
     * Description     : Validate and copy one request into the bounded unsent queue.
     * Input Parameters: request, error
     * Return Value    : True after acceptance; invalid requests leave the queue unchanged.
     * Date            : 2026.11.14
     ******************************************************************************/
    bool enqueue(const SimulationModbusRequest &request, QString *error = nullptr);

    /******************************************************************************
     * Function Name   : SimulationModbusTransport::processNext
     * Description     : Execute one request without retries and validate a complete response.
     * Input Parameters: None
     * Return Value    : Atomic response values or a failure that disconnects and clears the queue.
     * Date            : 2026.11.14
     ******************************************************************************/
    SimulationModbusResult processNext();

    /******************************************************************************
     * Function Name   : SimulationModbusTransport::disconnectFromStation
     * Description     : Clear unsent work and close after any current callback has returned.
     * Input Parameters: None
     * Return Value    : None; already attempted remote writes are not rolled back.
     * Date            : 2026.11.14
     ******************************************************************************/
    void disconnectFromStation();

    /******************************************************************************
     * Function Name   : SimulationModbusTransport::isConnected
     * Description     : Report whether the injected session remains usable.
     * Input Parameters: None
     * Return Value    : Connected flag.
     * Date            : 2026.11.14
     ******************************************************************************/
    bool isConnected() const;

    /******************************************************************************
     * Function Name   : SimulationModbusTransport::pendingCount
     * Description     : Return the number of queued requests that have not been sent.
     * Input Parameters: None
     * Return Value    : Pending request count.
     * Date            : 2026.11.14
     ******************************************************************************/
    int pendingCount() const;

    /******************************************************************************
     * Function Name   : SimulationModbusTransport::firstError
     * Description     : Preserve the first failure until a new endpoint opens successfully.
     * Input Parameters: None
     * Return Value    : First session failure text, including a lone cleanup failure.
     * Date            : 2026.11.14
     ******************************************************************************/
    QString firstError() const;

private:
    SimulationModbusResult fail(SimulationModbusResult result, const QString &code,
                                 const QString &message);
    void closeEndpoint();

    SimulationModbusIo m_io;
    SimulationModbusOptions m_options;
    QQueue<SimulationModbusRequest> m_queue;
    QString m_firstError;
    int m_station = 0;
    bool m_connected = false;
    bool m_processing = false;
    bool m_transitioning = false;
    bool m_disconnectRequested = false;
    int m_cancelledQueuedRequests = 0;
};

#endif
