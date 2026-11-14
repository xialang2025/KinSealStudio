#include "simulation_modbus_transport.h"

#include <QJsonArray>
#include <QMap>
#include <QtTest>

//20261114-6 add: The slave below implements real RTU bytes in memory; it has no device endpoint.
namespace
{
/******************************************************************************
 * Function Name   : referenceFrame
 * Description     : Seal a simulated slave PDU using an independent reference CRC loop.
 * Input Parameters: body
 * Return Value    : RTU frame with low-byte-first CRC.
 * Date            : 2026.11.14
 ******************************************************************************/
QByteArray referenceFrame(const QByteArray &body)
{
    QByteArray result;
    u32 crc;
    int index;
    int bit;

    crc = 65535;
    result = body;
    for (index = 0; index < body.size(); ++index)
    {
        crc ^= u8(body[index]);
        for (bit = 0; bit < 8; ++bit)
        {
            if ((crc % 2U) == 0)
            {
                crc /= 2;
            }
            else
            {
                crc = (crc / 2U) ^ 40961U;
            }
        }
    }
    result.append(char(crc % 256U));
    result.append(char(crc / 256U));
    return result;
}

/******************************************************************************
 * Function Name   : request
 * Description     : Construct an attributed request with explicit zero-based addressing.
 * Input Parameters: function, address, count, value, id
 * Return Value    : Request structure.
 * Date            : 2026.11.14
 ******************************************************************************/
SimulationModbusRequest request(int function = 3, u32 address = 0, u32 count = 1,
                                u32 value = 0, const QString &id = QStringLiteral("r1"))
{
    SimulationModbusRequest result;

    result.operation = SimulationModbusOperation(function);
    result.address = address;
    result.count = count;
    result.value = value;
    result.id = id;
    return result;
}

struct FakeSlave
{
    QMap<int, u16> registers;
    QMap<int, bool> coils;
    QVector<QByteArray> writes;
    QVector<int> readTimeouts;
    QVector<int> readCapacities;
    std::function<QByteArray(const QByteArray &)> transform;
    std::function<void()> onWrite;
    std::function<void()> onRead;
    QByteArray reply;
    int fragmentSize = 256;
    int openCount = 0;
    int closeCount = 0;
    int readCount = 0;
    int writeReturn = -999;
    int delayMs = 0;
    bool openOk = true;
    bool closeOk = true;
    bool timeout = false;
    bool noProgress = false;
    bool noBoundary = false;
    bool oversize = false;
    QString readError;

    /******************************************************************************
     * Function Name   : FakeSlave::callbacks
     * Description     : Bind a deterministic in-memory slave and record all caller operations.
     * Input Parameters: None
     * Return Value    : Controlled injected I/O; no serial, USB, socket or subprocess is opened.
     * Date            : 2026.11.14
     ******************************************************************************/
    SimulationModbusIo callbacks()
    {
        SimulationModbusIo io;

        io.open = [this](QString *error) {
            ++openCount;
            if (!openOk && error != nullptr)
            {
                *error = QStringLiteral("fake-open-error");
            }
            return openOk;
        };
        io.write = [this](const QByteArray &frame, int timeoutMs) {
            QByteArray body;
            int function;
            int address;
            int argument;
            int index;
            int byte;
            int bits;
            u16 word;

            writes.append(frame);
            if (onWrite)
            {
                onWrite();
            }
            if (timeoutMs < 1 || frame.size() != 8 ||
                referenceFrame(frame.left(6)) != frame)
            {
                return -90;
            }
            if (writeReturn != -999)
            {
                return writeReturn;
            }
            function = u8(frame[1]);
            address = (int(u8(frame[2])) << 8) | int(u8(frame[3]));
            argument = (int(u8(frame[4])) << 8) | int(u8(frame[5]));
            body.append(frame.left(2));
            if (function == 3)
            {
                body.append(char(argument * 2));
                for (index = 0; index < argument; ++index)
                {
                    word = registers.value(address + index, u16(address + index));
                    body.append(char(word >> 8));
                    body.append(char(word & 0xffU));
                }
            }
            else if (function == 1)
            {
                body.append(char((argument + 7) / 8));
                for (byte = 0; byte < (argument + 7) / 8; ++byte)
                {
                    bits = 0;
                    for (index = 0; index < 8 && byte * 8 + index < argument; ++index)
                    {
                        if (coils.value(address + byte * 8 + index, false))
                        {
                            bits |= (1 << index);
                        }
                    }
                    body.append(char(bits));
                }
            }
            else
            {
                if (function == 5)
                {
                    coils[address] = argument == 0xff00;
                }
                else if (function == 6)
                {
                    registers[address] = u16(argument);
                }
                body = frame.left(6);         // The simulated effect precedes acknowledgement.
            }
            reply = referenceFrame(body);
            if (transform)
            {
                reply = transform(reply);
            }
            return int(frame.size());
        };
        io.read = [this](int capacity, int timeoutMs) {
            SimulationModbusReadChunk chunk;

            ++readCount;
            readCapacities.append(capacity);
            readTimeouts.append(timeoutMs);
            if (onRead)
            {
                onRead();
            }
            if (delayMs > 0)
            {
                QTest::qWait(delayMs);
            }
            if (timeout)
            {
                chunk.timedOut = true;
            }
            else if (oversize)
            {
                chunk.bytes = QByteArray(capacity + 1, char(0));
            }
            else if (!noProgress)
            {
                chunk.bytes = reply.left(qMin(capacity, fragmentSize));
                reply.remove(0, chunk.bytes.size());
                chunk.endOfFrame = reply.isEmpty() && !noBoundary;
            }
            chunk.error = readError;
            return chunk;
        };
        io.close = [this](QString *error) {
            ++closeCount;
            if (!closeOk && error != nullptr)
            {
                *error = QStringLiteral("fake-close-error");
            }
            return closeOk;
        };
        return io;
    }
};
}

class SimulationModbusTransportTests : public QObject
{
    Q_OBJECT

private slots:
    void crcVectors_data();
    void crcVectors();
    void goldenRequests_data();
    void goldenRequests();
    void requestValidation_data();
    void requestValidation();
    void connectionValidation_data();
    void connectionValidation();
    void validResponses_data();
    void validResponses();
    void malformedResponses_data();
    void malformedResponses();
    void exceptions_data();
    void exceptions();
    void ioFaults_data();
    void ioFaults();
    void queueAndSessionLifecycle();
    void disconnectDuringCallback_data();
    void disconnectDuringCallback();
    void remoteWriteIsNotRolledBack();
    void realDeadline();
    void explicitFrameBoundaryRequired();
    void writeThenReadWorkflow();
    void physicalEvidenceRemainsFalse();
    void endpointTransitionReentry();
    void destroyDiscardsUnsentQueue();
    void frameBoundaryMayArriveSeparately();
    void partialThenTimeoutPublishesNothing();
    void writeExhaustsDeadlineBeforeRead();
    void queuedRequestIsSnapshot();
};

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::crcVectors_data
 * Description     : Supply published CRC check vectors independent of the in-memory slave.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::crcVectors_data()
{
    QTest::addColumn<QByteArray>("bytes");
    QTest::addColumn<int>("crc");
    QTest::newRow("empty") << QByteArray() << 65535;
    QTest::newRow("ascii-check") << QByteArray("123456789") << 0x4b37;
    QTest::newRow("read-holding") << QByteArray::fromHex("01030000000a") << 0xcdc5;
    QTest::newRow("complete-frame-zero") << QByteArray::fromHex("01030000000ac5cd") << 0;
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::crcVectors
 * Description     : Compare the production CRC to fixed values, not a mirrored encoder.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::crcVectors()
{
    QFETCH(QByteArray, bytes);
    QFETCH(int, crc);
    QCOMPARE(int(simulationModbusCrc16(bytes)), crc);
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::goldenRequests_data
 * Description     : Pin standard network byte order, logical-coil encoding and CRC byte order.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::goldenRequests_data()
{
    QTest::addColumn<int>("function");
    QTest::addColumn<u32>("address");
    QTest::addColumn<u32>("count");
    QTest::addColumn<u32>("value");
    QTest::addColumn<QByteArray>("expected");
    QTest::newRow("holding10") << 3 << u32(0) << u32(10) << u32(0) << QByteArray::fromHex("01030000000ac5cd");
    QTest::newRow("coil37") << 1 << u32(19) << u32(37) << u32(0) << QByteArray::fromHex("0101001300250c14");
    QTest::newRow("coil-on") << 5 << u32(0) << u32(1) << u32(1) << QByteArray::fromHex("01050000ff008c3a");
    QTest::newRow("coil-off") << 5 << u32(0) << u32(1) << u32(0) << QByteArray::fromHex("010500000000cdca");
    QTest::newRow("register") << 6 << u32(1) << u32(1) << u32(3) << QByteArray::fromHex("010600010003980b");
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::goldenRequests
 * Description     : Match complete request bytes to fixed golden RTU requests.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::goldenRequests()
{
    QByteArray frame;
    QString error;
    QFETCH(int, function);
    QFETCH(u32, address);
    QFETCH(u32, count);
    QFETCH(u32, value);
    QFETCH(QByteArray, expected);
    QVERIFY(buildSimulationModbusRequest(1, request(function, address, count, value), &frame, &error));
    QCOMPARE(frame, expected);
    QVERIFY(error.isEmpty());
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::requestValidation_data
 * Description     : Exercise legal limits and rejected broadcasts, wraps and write encodings.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::requestValidation_data()
{
    QTest::addColumn<int>("station");
    QTest::addColumn<int>("function");
    QTest::addColumn<u32>("address");
    QTest::addColumn<u32>("count");
    QTest::addColumn<u32>("value");
    QTest::addColumn<bool>("valid");
    QTest::newRow("lowest") << 1 << 3 << u32(0) << u32(1) << u32(0) << true;
    QTest::newRow("highest") << 247 << 3 << u32(65535) << u32(1) << u32(0) << true;
    QTest::newRow("coils-max") << 1 << 1 << u32(63536) << u32(2000) << u32(0) << true;
    QTest::newRow("registers-max") << 1 << 3 << u32(65411) << u32(125) << u32(0) << true;
    QTest::newRow("write-max") << 247 << 6 << u32(65535) << u32(1) << u32(65535) << true;
    QTest::newRow("broadcast") << 0 << 3 << u32(0) << u32(1) << u32(0) << false;
    QTest::newRow("negative-station") << -1 << 3 << u32(0) << u32(1) << u32(0) << false;
    QTest::newRow("reserved-station") << 248 << 3 << u32(0) << u32(1) << u32(0) << false;
    QTest::newRow("unsupported-function") << 2 << 2 << u32(0) << u32(1) << u32(0) << false;
    QTest::newRow("address-overflow") << 1 << 3 << u32(65536) << u32(1) << u32(0) << false;
    QTest::newRow("wrap") << 1 << 3 << u32(65535) << u32(2) << u32(0) << false;
    QTest::newRow("count-zero") << 1 << 3 << u32(0) << u32(0) << u32(0) << false;
    QTest::newRow("too-many-registers") << 1 << 3 << u32(0) << u32(126) << u32(0) << false;
    QTest::newRow("too-many-coils") << 1 << 1 << u32(0) << u32(2001) << u32(0) << false;
    QTest::newRow("read-has-value") << 1 << 3 << u32(0) << u32(1) << u32(1) << false;
    QTest::newRow("read-coil-has-value") << 1 << 1 << u32(0) << u32(1) << u32(1) << false;
    QTest::newRow("write-many") << 1 << 6 << u32(0) << u32(2) << u32(1) << false;
    QTest::newRow("write-value-overflow") << 1 << 6 << u32(0) << u32(1) << u32(65536) << false;
    QTest::newRow("nonlogical-coil") << 1 << 5 << u32(0) << u32(1) << u32(2) << false;
    QTest::newRow("coil-wire-value-not-api") << 1 << 5 << u32(0) << u32(1) << u32(0xff00) << false;
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::requestValidation
 * Description     : Verify all invalid requests preserve previously constructed output bytes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::requestValidation()
{
    QByteArray frame;
    QString error;
    QFETCH(int, station);
    QFETCH(int, function);
    QFETCH(u32, address);
    QFETCH(u32, count);
    QFETCH(u32, value);
    QFETCH(bool, valid);
    frame = QByteArray("preserve");
    QCOMPARE(buildSimulationModbusRequest(station, request(function, address, count, value), &frame, &error), valid);
    if (valid)
    {
        QCOMPARE(frame.size(), 8);
        QVERIFY(error.isEmpty());
    }
    else
    {
        QCOMPARE(frame, QByteArray("preserve"));
        QVERIFY(!error.isEmpty());
    }
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::connectionValidation_data
 * Description     : Check endpoint validation before open and bounded session options.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::connectionValidation_data()
{
    QTest::addColumn<int>("caseId");
    QTest::newRow("broadcast") << 0;
    QTest::newRow("station248") << 1;
    QTest::newRow("queue0") << 2;
    QTest::newRow("queue1025") << 3;
    QTest::newRow("timeout0") << 4;
    QTest::newRow("timeout60001") << 5;
    QTest::newRow("missing-open") << 6;
    QTest::newRow("missing-write") << 7;
    QTest::newRow("missing-read") << 8;
    QTest::newRow("missing-close") << 9;
    QTest::newRow("open-failure") << 10;
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::connectionValidation
 * Description     : Reject unsafe setup without any unsolicited write or retry.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::connectionValidation()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusOptions options;
    SimulationModbusIo io;
    QString error;
    int station;
    QFETCH(int, caseId);

    station = 1;
    io = slave.callbacks();
    if (caseId == 0) { station = 0; }
    if (caseId == 1) { station = 248; }
    if (caseId == 2) { options.maxPendingRequests = 0; }
    if (caseId == 3) { options.maxPendingRequests = 1025; }
    if (caseId == 4) { options.timeoutMs = 0; }
    if (caseId == 5) { options.timeoutMs = 60001; }
    if (caseId == 6) { io.open = {}; }
    if (caseId == 7) { io.write = {}; }
    if (caseId == 8) { io.read = {}; }
    if (caseId == 9) { io.close = {}; }
    if (caseId == 10) { slave.openOk = false; }
    QVERIFY(!transport.connectToStation(station, io, options, &error));
    QCOMPARE(slave.openCount, caseId == 10 ? 1 : 0);
    QCOMPARE(slave.writes.size(), 0);
    QCOMPARE(slave.closeCount, 0);
    QVERIFY(!transport.isConnected());
    QVERIFY(!error.isEmpty());
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::validResponses_data
 * Description     : Cover all four functions, fragmented responses and maximum RTU payloads.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::validResponses_data()
{
    QTest::addColumn<int>("function");
    QTest::addColumn<u32>("count");
    QTest::addColumn<int>("fragment");
    QTest::newRow("coils-1") << 1 << u32(1) << 256;
    QTest::newRow("coils-byte") << 1 << u32(8) << 256;
    QTest::newRow("coils-fragmented") << 1 << u32(17) << 1;
    QTest::newRow("coils-maximum") << 1 << u32(2000) << 7;
    QTest::newRow("holding-1") << 3 << u32(1) << 256;
    QTest::newRow("holding-fragmented") << 3 << u32(10) << 1;
    QTest::newRow("holding-maximum") << 3 << u32(125) << 11;
    QTest::newRow("write-coil") << 5 << u32(1) << 2;
    QTest::newRow("write-register") << 6 << u32(1) << 1;
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::validResponses
 * Description     : Execute requests against the slave and publish only correctly attributed values.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::validResponses()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusRequest input;
    SimulationModbusResult result;
    int index;
    QFETCH(int, function);
    QFETCH(u32, count);
    QFETCH(int, fragment);

    slave.fragmentSize = fragment;
    for (index = 0; index < int(count); ++index)
    {
        slave.coils[index + 10] = (index % 3) == 0;
        slave.registers[index + 10] = u16(0x9000 + index);
    }
    input = request(function, 10, count, function == 5 ? 1U : (function == 6 ? 0xbeefU : 0U));
    QVERIFY(transport.connectToStation(247, slave.callbacks()));
    QVERIFY(transport.enqueue(input));
    result = transport.processNext();
    QVERIFY2(result.ok, qPrintable(result.error));
    QCOMPARE(result.request.id, input.id);
    QCOMPARE(result.writeAcknowledged, function == 5 || function == 6);
    QVERIFY(!result.remoteStateUnconfirmed);
    QVERIFY(transport.isConnected());
    QCOMPARE(slave.writes.size(), 1);
    if (function == 1 || function == 3)
    {
        QCOMPARE(result.values.size(), int(count));
        for (index = 0; index < int(count); ++index)
        {
            QCOMPARE(result.values[index], function == 1 ? u16(index % 3 == 0) : u16(0x9000 + index));
        }
    }
    if (function == 5) { QVERIFY(slave.coils[10]); }
    if (function == 6) { QCOMPARE(slave.registers[10], u16(0xbeef)); }
    transport.disconnectFromStation();
    QCOMPARE(slave.closeCount, 1);
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::malformedResponses_data
 * Description     : Mutate one protocol property while retaining valid CRC where appropriate.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::malformedResponses_data()
{
    QTest::addColumn<int>("caseId");
    QTest::addColumn<QString>("code");
    QTest::newRow("wrong-station") << 0 << QStringLiteral("wrong_station");
    QTest::newRow("wrong-function") << 1 << QStringLiteral("wrong_function");
    QTest::newRow("wrong-byte-count") << 2 << QStringLiteral("wrong_byte_count");
    QTest::newRow("crc-damaged") << 3 << QStringLiteral("crc_mismatch");
    QTest::newRow("short-frame") << 4 << QStringLiteral("truncated_response");
    QTest::newRow("extra-byte") << 5 << QStringLiteral("extra_response_bytes");
    QTest::newRow("empty-frame") << 6 << QStringLiteral("truncated_response");
    QTest::newRow("coil-padding") << 7 << QStringLiteral("nonzero_coil_padding");
    QTest::newRow("wrong-echo-address") << 8 << QStringLiteral("write_echo_mismatch");
    QTest::newRow("wrong-echo-value") << 9 << QStringLiteral("write_echo_mismatch");
    QTest::newRow("extra-fragment-after-complete-prefix") << 10 << QStringLiteral("extra_response_bytes");
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::malformedResponses
 * Description     : Ensure damaged responses clear the remaining queue and expose no partial values.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::malformedResponses()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusResult result;
    SimulationModbusRequest input;
    QFETCH(int, caseId);
    QFETCH(QString, code);

    input = request(caseId == 7 ? 1 : ((caseId == 8 || caseId == 9) ? 6 : 3));
    slave.closeOk = false;                    // Cleanup failure must not replace the frame failure.
    slave.transform = [caseId](const QByteArray &frame) {
        QByteArray body;
        QByteArray result;

        body = frame.left(frame.size() - 2);
        if (caseId == 0) { body[0] = char(2); }
        if (caseId == 1) { body[1] = char(4); }
        if (caseId == 2) { body[2] = char(4); }
        if (caseId == 7) { body[3] = char(0x80); }
        if (caseId == 8) { body[3] = char(1); }
        if (caseId == 9) { body[5] = char(1); }
        result = referenceFrame(body);
        if (caseId == 3) { result[result.size() - 1] = char(u8(result.back()) ^ 1U); }
        if (caseId == 4) { result.chop(1); }
        if (caseId == 5 || caseId == 10) { result.append(char(0)); }
        if (caseId == 6) { result.clear(); }
        return result;
    };
    if (caseId == 10) { slave.fragmentSize = 7; }
    QVERIFY(transport.connectToStation(1, slave.callbacks()));
    QVERIFY(transport.enqueue(input));
    QVERIFY(transport.enqueue(request(3, 5, 1, 0, QStringLiteral("unsent"))));
    result = transport.processNext();
    QVERIFY(!result.ok);
    QCOMPARE(result.errorCode, code);
    QVERIFY(result.values.isEmpty());
    QVERIFY(!result.writeAcknowledged);
    QCOMPARE(result.discardedQueuedRequests, 1);
    QCOMPARE(transport.pendingCount(), 0);
    QVERIFY(!transport.isConnected());
    QCOMPARE(slave.writes.size(), 1);
    QCOMPARE(slave.closeCount, 1);
    QCOMPARE(transport.firstError(), result.error);
    QVERIFY(transport.firstError() != QStringLiteral("fake-close-error"));
    QCOMPARE(transport.processNext().errorCode, QStringLiteral("not_connected"));
    QCOMPARE(slave.writes.size(), 1);
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::exceptions_data
 * Description     : Include assigned exception codes and malformed zero, reserved and unknown codes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::exceptions_data()
{
    QTest::addColumn<int>("code");
    QTest::addColumn<bool>("assigned");
    QTest::newRow("illegal-function") << 1 << true;
    QTest::newRow("illegal-address") << 2 << true;
    QTest::newRow("illegal-value") << 3 << true;
    QTest::newRow("slave-failure") << 4 << true;
    QTest::newRow("acknowledge") << 5 << true;
    QTest::newRow("busy") << 6 << true;
    QTest::newRow("parity") << 8 << true;
    QTest::newRow("gateway-path") << 10 << true;
    QTest::newRow("gateway-target") << 11 << true;
    QTest::newRow("zero") << 0 << false;
    QTest::newRow("reserved7") << 7 << false;
    QTest::newRow("reserved9") << 9 << false;
    QTest::newRow("unknown255") << 255 << false;
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::exceptions
 * Description     : Attribute exception codes without treating busy or acknowledgement as success.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::exceptions()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusResult result;
    QFETCH(int, code);
    QFETCH(bool, assigned);

    slave.fragmentSize = 1;
    slave.transform = [code](const QByteArray &frame) {
        QByteArray body;

        body.append(frame[0]);
        body.append(char(u8(frame[1]) | 0x80U));
        body.append(char(code));
        return referenceFrame(body);
    };
    QVERIFY(transport.connectToStation(1, slave.callbacks()));
    QVERIFY(transport.enqueue(request()));
    result = transport.processNext();
    QVERIFY(!result.ok);
    QCOMPARE(int(result.exceptionCode), code);
    QCOMPARE(result.errorCode, assigned ? QStringLiteral("slave_exception") : QStringLiteral("invalid_exception_code"));
    QVERIFY(result.values.isEmpty());
    QVERIFY(!transport.isConnected());
    QCOMPARE(slave.writes.size(), 1);
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::ioFaults_data
 * Description     : Exercise write uncertainty, timeout, read errors and invalid callback output.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::ioFaults_data()
{
    QTest::addColumn<int>("caseId");
    QTest::addColumn<QString>("code");
    QTest::newRow("negative-write") << 0 << QStringLiteral("short_or_failed_write");
    QTest::newRow("zero-write") << 1 << QStringLiteral("short_or_failed_write");
    QTest::newRow("partial-write") << 2 << QStringLiteral("short_or_failed_write");
    QTest::newRow("oversized-write-return") << 3 << QStringLiteral("short_or_failed_write");
    QTest::newRow("timeout") << 4 << QStringLiteral("timeout");
    QTest::newRow("read-error") << 5 << QStringLiteral("read_error");
    QTest::newRow("read-no-progress") << 6 << QStringLiteral("read_no_progress");
    QTest::newRow("read-oversized") << 7 << QStringLiteral("read_contract_violation");
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::ioFaults
 * Description     : Verify one attempt, cleanup and explicit unknown remote status after failed writes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::ioFaults()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusResult result;
    QFETCH(int, caseId);
    QFETCH(QString, code);

    if (caseId == 0) { slave.writeReturn = -1; }
    if (caseId == 1) { slave.writeReturn = 0; }
    if (caseId == 2) { slave.writeReturn = 7; }
    if (caseId == 3) { slave.writeReturn = 9; }
    if (caseId == 4) { slave.timeout = true; }
    if (caseId == 5) { slave.readError = QStringLiteral("fake-read-error"); }
    if (caseId == 6) { slave.noProgress = true; }
    if (caseId == 7) { slave.oversize = true; }
    QVERIFY(transport.connectToStation(1, slave.callbacks()));
    QVERIFY(transport.enqueue(request(6, 0, 1, 123)));
    result = transport.processNext();
    QCOMPARE(result.errorCode, code);
    QVERIFY(!result.ok);
    QVERIFY(!result.writeAcknowledged);
    QVERIFY(result.requestMayHaveReachedSlave);
    QVERIFY(result.remoteStateUnconfirmed);
    QVERIFY(!transport.isConnected());
    QCOMPARE(slave.writes.size(), 1);
    QCOMPARE(slave.closeCount, 1);
    QCOMPARE(slave.readCount, caseId <= 3 ? 0 : 1);
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::queueAndSessionLifecycle
 * Description     : Preserve bounded unsent work, clear on exit and require explicit reconnection.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::queueAndSessionLifecycle()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusOptions options;
    SimulationModbusRequest invalid;
    QString error;

    options.maxPendingRequests = 2;
    QVERIFY(!transport.enqueue(request(), &error));
    QVERIFY(transport.connectToStation(1, slave.callbacks(), options));
    QVERIFY(!transport.connectToStation(2, slave.callbacks(), options, &error));
    QCOMPARE(slave.openCount, 1);
    QVERIFY(transport.isConnected());
    invalid = request();
    invalid.id.clear();
    QVERIFY(!transport.enqueue(invalid, &error));
    invalid.id = QString(257, QChar(u'x'));
    QVERIFY(!transport.enqueue(invalid, &error));
    QCOMPARE(transport.pendingCount(), 0);
    QVERIFY(transport.enqueue(request()));
    QVERIFY(!transport.enqueue(request(), &error));
    QCOMPARE(transport.pendingCount(), 1);
    QVERIFY(transport.enqueue(request(3, 1, 1, 0, QStringLiteral("r2"))));
    QVERIFY(!transport.enqueue(request(3, 2, 1, 0, QStringLiteral("r3")), &error));
    QCOMPARE(transport.pendingCount(), 2);
    transport.disconnectFromStation();
    QCOMPARE(slave.writes.size(), 0);
    QCOMPARE(transport.pendingCount(), 0);
    QCOMPARE(slave.closeCount, 1);
    transport.disconnectFromStation();
    QCOMPARE(slave.closeCount, 1);
    slave.closeOk = false;
    QVERIFY(transport.connectToStation(1, slave.callbacks()));
    QCOMPARE(transport.processNext().errorCode, QStringLiteral("empty_queue"));
    QVERIFY(transport.isConnected());
    transport.disconnectFromStation();
    QCOMPARE(transport.firstError(), QStringLiteral("fake-close-error"));
    slave.openOk = false;
    QVERIFY(!transport.connectToStation(1, slave.callbacks(), {}, &error));
    QCOMPARE(transport.firstError(), QStringLiteral("fake-close-error"));
    slave.openOk = true;
    slave.closeOk = true;
    QVERIFY(transport.connectToStation(1, slave.callbacks()));
    QVERIFY(transport.firstError().isEmpty());
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::disconnectDuringCallback_data
 * Description     : Exercise deferred cleanup both after write and during fragmented receive.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::disconnectDuringCallback_data()
{
    QTest::addColumn<bool>("duringRead");
    QTest::newRow("write") << false;
    QTest::newRow("read") << true;
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::disconnectDuringCallback
 * Description     : Avoid closing an endpoint inside its active callback or accepting reentrant work.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::disconnectDuringCallback()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusResult result;
    bool closedTooEarly;
    bool reentrantRejected;
    std::function<void()> cancel;
    QFETCH(bool, duringRead);

    closedTooEarly = false;
    reentrantRejected = false;
    cancel = [&]() {
        reentrantRejected = transport.processNext().errorCode == QStringLiteral("reentrant_call") &&
                            !transport.enqueue(request()) &&
                            !transport.connectToStation(1, slave.callbacks());
        transport.disconnectFromStation();
        closedTooEarly = slave.closeCount != 0;
    };
    if (duringRead) { slave.onRead = cancel; }
    else { slave.onWrite = cancel; }
    QVERIFY(transport.connectToStation(1, slave.callbacks()));
    QVERIFY(transport.enqueue(request(6, 0, 1, 123)));
    QVERIFY(transport.enqueue(request(3, 3, 1, 0, QStringLiteral("unsent"))));
    result = transport.processNext();
    QCOMPARE(result.errorCode, QStringLiteral("disconnected_during_transaction"));
    QVERIFY(reentrantRejected);
    QVERIFY(!closedTooEarly);
    QVERIFY(result.remoteStateUnconfirmed);
    QVERIFY(!result.writeAcknowledged);
    QCOMPARE(result.discardedQueuedRequests, 1);
    QCOMPARE(slave.closeCount, 1);
    QCOMPARE(slave.writes.size(), 1);
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::remoteWriteIsNotRolledBack
 * Description     : Show a slave can apply a write before an invalid acknowledgement is rejected.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::remoteWriteIsNotRolledBack()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusResult result;

    slave.transform = [](const QByteArray &frame) {
        QByteArray body;

        body = frame.left(6);
        body[3] = char(12);
        return referenceFrame(body);
    };
    QVERIFY(transport.connectToStation(1, slave.callbacks()));
    QVERIFY(transport.enqueue(request(6, 11, 1, 444)));
    result = transport.processNext();
    QCOMPARE(slave.registers[11], u16(444));
    QVERIFY(result.remoteStateUnconfirmed);
    QVERIFY(!result.writeAcknowledged);
    QCOMPARE(slave.writes.size(), 1);
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::realDeadline
 * Description     : Reject an otherwise valid reply that arrives after the single transaction deadline.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::realDeadline()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusOptions options;
    SimulationModbusResult result;

    options.timeoutMs = 10;
    slave.delayMs = 25;
    QVERIFY(transport.connectToStation(1, slave.callbacks(), options));
    QVERIFY(transport.enqueue(request()));
    result = transport.processNext();
    QCOMPARE(result.errorCode, QStringLiteral("timeout"));
    QVERIFY(result.values.isEmpty());
    QVERIFY(slave.readCount <= 1);
    if (slave.readCount == 1)
    {
        QVERIFY(slave.readTimeouts[0] >= 1 && slave.readTimeouts[0] <= 10);
    }
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::explicitFrameBoundaryRequired
 * Description     : A complete valid prefix cannot succeed without an observed frame boundary.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::explicitFrameBoundaryRequired()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusResult result;

    slave.noBoundary = true;
    QVERIFY(transport.connectToStation(1, slave.callbacks()));
    QVERIFY(transport.enqueue(request()));
    result = transport.processNext();
    QVERIFY(!result.ok);
    QCOMPARE(result.errorCode, QStringLiteral("read_no_progress"));
    QVERIFY(result.values.isEmpty());
    QCOMPARE(slave.readCount, 2);
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::writeThenReadWorkflow
 * Description     : Execute a FIFO write and read against one simulated slave process image.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::writeThenReadWorkflow()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusResult written;
    SimulationModbusResult read;

    QVERIFY(transport.connectToStation(7, slave.callbacks()));
    QVERIFY(transport.enqueue(request(6, 100, 1, 1234, QStringLiteral("write"))));
    QVERIFY(transport.enqueue(request(3, 100, 1, 0, QStringLiteral("read"))));
    written = transport.processNext();
    read = transport.processNext();
    QVERIFY(written.ok && written.writeAcknowledged);
    QVERIFY(read.ok);
    QCOMPARE(read.values, QVector<u16>{1234});
    QCOMPARE(read.request.id, QStringLiteral("read"));
    QCOMPARE(slave.writes.size(), 2);
    QCOMPARE(slave.writes[0].left(6), QByteArray::fromHex("0706006404d2"));
    QCOMPARE(slave.writes[1].left(6), QByteArray::fromHex("070300640001"));
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::physicalEvidenceRemainsFalse
 * Description     : Prevent a passing protocol replay from becoming a physical PLC or HMI claim.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::physicalEvidenceRemainsFalse()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusResult result;
    QJsonObject evidence;

    QVERIFY(transport.connectToStation(1, slave.callbacks()));
    QVERIFY(transport.enqueue(request()));
    result = transport.processNext();
    evidence = simulationModbusResultToJson(result);
    QVERIFY(evidence.value(QStringLiteral("ok")).toBool());
    QCOMPARE(evidence.value(QStringLiteral("scope")).toString(), QStringLiteral("controlled-modbus-rtu-replay"));
    QVERIFY(!evidence.value(QStringLiteral("physicalEndpointUsed")).toBool(true));
    QVERIFY(!evidence.value(QStringLiteral("hardwareTransferVerified")).toBool(true));
    QVERIFY(!evidence.value(QStringLiteral("hmiBehaviorVerified")).toBool(true));
    QCOMPARE(evidence.value(QStringLiteral("values")).toArray().size(), 1);
    QCOMPARE(evidence.value(QStringLiteral("station")).toInt(), 1);
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::endpointTransitionReentry
 * Description     : Reject reentrant work from endpoint open and close callbacks and honor open cancellation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::endpointTransitionReentry()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusIo io;
    std::function<bool(QString *)> normalOpen;
    std::function<bool(QString *)> normalClose;
    bool openRejected;
    bool closeRejected;
    bool closedInsideOpen;
    QString error;

    io = slave.callbacks();
    normalOpen = io.open;
    normalClose = io.close;
    openRejected = false;
    closeRejected = false;
    closedInsideOpen = false;
    io.open = [&](QString *callbackError) {
        openRejected = !transport.connectToStation(1, slave.callbacks()) &&
                       !transport.enqueue(request()) &&
                       transport.processNext().errorCode == QStringLiteral("reentrant_call");
        transport.disconnectFromStation();
        closedInsideOpen = slave.closeCount > 0;
        return normalOpen(callbackError);
    };
    io.close = [&](QString *callbackError) {
        closeRejected = !transport.connectToStation(1, slave.callbacks()) &&
                        !transport.enqueue(request()) &&
                        transport.processNext().errorCode == QStringLiteral("reentrant_call");
        transport.disconnectFromStation();
        return normalClose(callbackError);
    };
    QVERIFY(!transport.connectToStation(1, io, {}, &error));
    QVERIFY(openRejected);
    QVERIFY(closeRejected);
    QVERIFY(!closedInsideOpen);
    QVERIFY(!transport.isConnected());
    QCOMPARE(slave.openCount, 1);
    QCOMPARE(slave.closeCount, 1);
    QVERIFY(!error.isEmpty());
    QVERIFY(transport.connectToStation(1, slave.callbacks()));
    QVERIFY(transport.firstError().isEmpty());
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::destroyDiscardsUnsentQueue
 * Description     : Owner destruction closes once without transmitting requests that were only queued.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::destroyDiscardsUnsentQueue()
{
    FakeSlave slave;

    {
        SimulationModbusTransport transport;

        QVERIFY(transport.connectToStation(1, slave.callbacks()));
        QVERIFY(transport.enqueue(request()));
    }
    QCOMPARE(slave.closeCount, 1);
    QCOMPARE(slave.writes.size(), 0);
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::frameBoundaryMayArriveSeparately
 * Description     : Accept an empty explicit RTU boundary after the full response bytes have arrived.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::frameBoundaryMayArriveSeparately()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusIo io;
    std::function<SimulationModbusReadChunk(int, int)> normalRead;
    SimulationModbusResult result;

    io = slave.callbacks();
    normalRead = io.read;
    io.read = [&](int capacity, int timeoutMs) {
        SimulationModbusReadChunk chunk;

        if (slave.readCount > 0)
        {
            chunk.endOfFrame = true;
            return chunk;
        }
        chunk = normalRead(capacity, timeoutMs);
        chunk.endOfFrame = false;
        return chunk;
    };
    QVERIFY(transport.connectToStation(1, io));
    QVERIFY(transport.enqueue(request()));
    result = transport.processNext();
    QVERIFY(result.ok);
    QCOMPARE(result.values, QVector<u16>{0});
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::partialThenTimeoutPublishesNothing
 * Description     : Header and partial payload reception cannot publish a partial register result.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::partialThenTimeoutPublishesNothing()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusResult result;

    slave.fragmentSize = 4;
    slave.onRead = [&]() {
        if (slave.readCount == 2)
        {
            slave.timeout = true;
        }
    };
    QVERIFY(transport.connectToStation(1, slave.callbacks()));
    QVERIFY(transport.enqueue(request(3, 0, 2)));
    result = transport.processNext();
    QCOMPARE(result.errorCode, QStringLiteral("timeout"));
    QVERIFY(result.values.isEmpty());
    QCOMPARE(result.responseFrame.size(), 4);
    QVERIFY(!result.remoteStateUnconfirmed);
    QCOMPARE(slave.writes.size(), 1);
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::writeExhaustsDeadlineBeforeRead
 * Description     : Include write callback time in the transaction deadline without retrying.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::writeExhaustsDeadlineBeforeRead()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusOptions options;
    SimulationModbusResult result;

    options.timeoutMs = 1;
    slave.onWrite = []() { QTest::qWait(10); };
    QVERIFY(transport.connectToStation(1, slave.callbacks(), options));
    QVERIFY(transport.enqueue(request(6, 0, 1, 123)));
    result = transport.processNext();
    QCOMPARE(result.errorCode, QStringLiteral("timeout"));
    QVERIFY(result.remoteStateUnconfirmed);
    QCOMPARE(slave.readCount, 0);
    QCOMPARE(slave.registers[0], u16(123));
}

/******************************************************************************
 * Function Name   : SimulationModbusTransportTests::queuedRequestIsSnapshot
 * Description     : Later edits to an enqueue caller's object cannot change the transmitted request.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationModbusTransportTests::queuedRequestIsSnapshot()
{
    FakeSlave slave;
    SimulationModbusTransport transport;
    SimulationModbusRequest input;
    SimulationModbusResult result;

    input = request(6, 10, 1, 100);
    QVERIFY(transport.connectToStation(1, slave.callbacks()));
    QVERIFY(transport.enqueue(input));
    input.value = 200;
    input.address = 20;
    input.id = QStringLiteral("changed");
    result = transport.processNext();
    QVERIFY(result.ok);
    QCOMPARE(result.request.id, QStringLiteral("r1"));
    QCOMPARE(slave.registers.value(10), u16(100));
    QVERIFY(!slave.registers.contains(20));
}

QTEST_GUILESS_MAIN(SimulationModbusTransportTests)
#include "simulation_modbus_transport_tests.moc"
