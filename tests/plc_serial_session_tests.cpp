#include "plc_serial_session.h"

#include <QJsonArray>
#include <QThread>
#include <QtTest>

//20261114-8 add: These fixtures exchange pinned RTU frames without opening a serial device.
namespace
{
struct GoldenExchange
{
    SimulationModbusRequest request;
    QByteArray outgoing;
    QByteArray incoming;
    QVector<u16> expectedValues;
};

/******************************************************************************
 * Function Name   : independentlySeal
 * Description     : Seal a mutated fixture using an independent division-based CRC loop.
 * Input Parameters: body
 * Return Value    : PDU plus low-byte-first RTU CRC.
 * Date            : 2026.11.14
 ******************************************************************************/
QByteArray independentlySeal(const QByteArray &body) //20261114-8 add
{
    QByteArray result;
    u32 crc;
    int index;
    int bit;

    result = body;
    crc = 65535;
    for (index = 0; index < body.size(); ++index)
    {
        crc ^= u8(body[index]);
        for (bit = 0; bit < 8; ++bit)
        {
            crc = crc % 2U == 0 ? crc / 2U : (crc / 2U) ^ 40961U;
        }
    }
    result.append(char(crc % 256U));
    result.append(char(crc / 256U));
    return result;
}

/******************************************************************************
 * Function Name   : goldenExchange
 * Description     : Supply fixed request and response vectors without any production encoder call.
 * Input Parameters: function, id
 * Return Value    : An attributed request and independent full-frame oracle for station 17.
 * Date            : 2026.11.14
 ******************************************************************************/
GoldenExchange goldenExchange(int function, const QString &id = QStringLiteral("golden")) //20261114-8 add
{
    GoldenExchange result;

    result.request.id = id;
    result.request.operation = SimulationModbusOperation(function);
    if (function == 1)
    {
        result.request.address = 19;
        result.request.count = 10;
        result.outgoing = QByteArray::fromHex("11010013000a4f58");
        result.incoming = QByteArray::fromHex("110102cd01ed6f");
        result.expectedValues = {1, 0, 1, 1, 0, 0, 1, 1, 1, 0};
    }
    else if (function == 2)
    {
        result.request.address = 196;
        result.request.count = 9;
        result.outgoing = QByteArray::fromHex("110200c40009fb61");
        result.incoming = QByteArray::fromHex("110202550186eb");
        result.expectedValues = {1, 0, 1, 0, 1, 0, 1, 0, 1};
    }
    else if (function == 3)
    {
        result.request.address = 107;
        result.request.count = 3;
        result.outgoing = QByteArray::fromHex("1103006b00037687");
        result.incoming = QByteArray::fromHex("11030601028000ffffbcd4");
        result.expectedValues = {0x0102, 0x8000, 0xffff};
    }
    else if (function == 4)
    {
        result.request.address = 8;
        result.outgoing = QByteArray::fromHex("110400080001b298");
        result.incoming = QByteArray::fromHex("110402a55a8398");
        result.expectedValues = {0xa55a};
    }
    else if (function == 5)
    {
        result.request.address = 172;
        result.request.value = 1;
        result.outgoing = QByteArray::fromHex("110500acff004e8b");
        result.incoming = result.outgoing;
    }
    else if (function == 6)
    {
        result.request.address = 1;
        result.request.value = 3;
        result.outgoing = QByteArray::fromHex("1106000100039a9b");
        result.incoming = result.outgoing;
    }
    else if (function == 15)
    {
        result.request.address = 19;
        result.request.count = 10;
        result.request.values = {1, 0, 1, 1, 0, 0, 1, 1, 1, 0};
        result.outgoing = QByteArray::fromHex("110f0013000a02cd01bf0b");
        result.incoming = QByteArray::fromHex("110f0013000a2699");
    }
    else if (function == 16)
    {
        result.request.address = 1;
        result.request.count = 2;
        result.request.values = {10, 258};
        result.outgoing = QByteArray::fromHex("11100001000204000a0102c6f0");
        result.incoming = QByteArray::fromHex("1110000100021298");
    }
    return result;
}

/******************************************************************************
 * Function Name   : serialSettings
 * Description     : Provide an explicit fictitious PC port identity for injected-only tests.
 * Input Parameters: None
 * Return Value    : Valid Lite serial settings; no device is created by this function.
 * Date            : 2026.11.14
 ******************************************************************************/
PlcSerialSettings serialSettings() //20261114-8 add
{
    PlcSerialSettings result;

    result.portName = QStringLiteral("CONTROLLED-NO-PHYSICAL-PORT");
    return result;
}

struct ScriptedEndpoint
{
    QVector<GoldenExchange> exchanges;
    QVector<QByteArray> writes;
    QVector<QThread *> callbackThreads;
    QVector<int> writeBudgets;
    QVector<int> readBudgets;
    PlcSerialSettings observedSettings;
    std::function<bool()> observedCancellation;
    std::function<void()> onFactory;
    std::function<void()> onOpen;
    std::function<void()> onWrite;
    std::function<void()> onRead;
    QByteArray remainingReply;
    QThread *factoryThread = nullptr;
    int factoryCount = 0;
    int openCount = 0;
    int closeCount = 0;
    int readCount = 0;
    int fragmentSize = 256;
    int forcedWriteReturn = -999;
    int missingCallback = -1;
    bool openOk = true;
    bool closeOk = true;
    bool stillOpen = false;
    bool matched = true;
    bool noBoundary = false;
    bool timeout = false;
    bool noProgress = false;
    QString readError;

    /******************************************************************************
     * Function Name   : ScriptedEndpoint::factory
     * Description     : Construct an observable byte-scripted endpoint in the calling worker only.
     * Input Parameters: None
     * Return Value    : Injected factory; no serial port, socket, PLC, USB or subprocess is used.
     * Date            : 2026.11.14
     ******************************************************************************/
    PlcSerialIoFactory factory() //20261114-8 add
    {
        return [this](const PlcSerialSettings &settings, const std::function<bool()> &cancelled) {
            SimulationModbusIo io;

            ++factoryCount;
            factoryThread = QThread::currentThread();
            observedSettings = settings;
            observedCancellation = cancelled;
            if (onFactory) { onFactory(); }
            io.open = [this](QString *error) {
                callbackThreads.append(QThread::currentThread());
                ++openCount;
                if (onOpen) { onOpen(); }
                stillOpen = openOk;
                if (!openOk && error != nullptr) { *error = QStringLiteral("controlled-open-error"); }
                return openOk;
            };
            io.write = [this](const QByteArray &frame, int timeoutMs) {
                int index;

                callbackThreads.append(QThread::currentThread());
                index = int(writes.size());
                writes.append(frame);
                writeBudgets.append(timeoutMs);
                if (onWrite) { onWrite(); }
                if (index >= exchanges.size() || frame != exchanges[index].outgoing)
                {
                    matched = false;
                    return -901;
                }
                remainingReply = exchanges[index].incoming;
                return forcedWriteReturn == -999 ? int(frame.size()) : forcedWriteReturn;
            };
            io.read = [this](int maximum, int timeoutMs) {
                SimulationModbusReadChunk chunk;

                callbackThreads.append(QThread::currentThread());
                ++readCount;
                readBudgets.append(timeoutMs);
                if (onRead) { onRead(); }
                if (timeout) { chunk.timedOut = true; }
                else if (!noProgress)
                {
                    chunk.bytes = remainingReply.left(qMin(maximum, fragmentSize));
                    remainingReply.remove(0, chunk.bytes.size());
                    chunk.endOfFrame = remainingReply.isEmpty() && !noBoundary;
                }
                chunk.error = readError;
                return chunk;
            };
            io.close = [this](QString *error) {
                callbackThreads.append(QThread::currentThread());
                ++closeCount;
                if (closeOk) { stillOpen = false; }
                else if (error != nullptr) { *error = QStringLiteral("controlled-close-error"); }
                return closeOk;
            };
            if (missingCallback == 0) { io.open = {}; }
            if (missingCallback == 1) { io.write = {}; }
            if (missingCallback == 2) { io.read = {}; }
            if (missingCallback == 3) { io.close = {}; }
            return io;
        };
    }
};
}

class PlcSerialSessionTests : public QObject
{
    Q_OBJECT

private slots:
    void settingsBounds_data();
    void settingsBounds();
    void invalidBatchNeverCreatesEndpoint_data();
    void invalidBatchNeverCreatesEndpoint();
    void goldenTransactions_data();
    void goldenTransactions();
    void factoryAndCallbacksStayInWorker();
    void oneOpenForFrozenBatch();
    void maximumBatch();
    void openingFailures_data();
    void openingFailures();
    void middleFailurePreservesConfirmedReceipts_data();
    void middleFailurePreservesConfirmedReceipts();
    void closeFailureIsNotClosed_data();
    void closeFailureIsNotClosed();
    void cancellationBoundaries_data();
    void cancellationBoundaries();
    void totalDeadline_data();
    void totalDeadline();
};

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::settingsBounds_data
 * Description     : Define public settings boundaries independently from implementation branches.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::settingsBounds_data() //20261114-8 add
{
    int index;
    const char *names[] = {"explicit-defaults", "baud50", "baud4000000", "baud49", "baud4000001",
        "data7", "data9", "even", "odd", "unsupported-parity1", "unsupported-parity4", "two-stops",
        "zero-stops", "three-stops", "timeout1", "timeout60000", "timeout0", "timeout60001",
        "total-before-transaction", "total120001", "empty-port", "leading-space", "trailing-space",
        "port-newline", "port-del", "port256", "port257", "port-nul", "total0", "negative-parity"};
    const bool accepted[] = {true, true, true, false, false, false, false, true, true, false,
        false, true, false, false, true, true, false, false, false, false, false, false, false,
        false, false, true, false, false, false, false};

    QTest::addColumn<int>("caseId");
    QTest::addColumn<bool>("accepted");
    for (index = 0; index < 30; ++index)
    {
        QTest::newRow(names[index]) << index << accepted[index];
    }
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::settingsBounds
 * Description     : Validate each adopted PC setting without any endpoint creation or INI inference.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::settingsBounds() //20261114-8 add
{
    PlcSerialSettings settings;
    QString error;
    QFETCH(int, caseId);
    QFETCH(bool, accepted);

    settings = serialSettings();
    if (caseId == 1) { settings.baudRate = 50; }
    if (caseId == 2) { settings.baudRate = 4000000; }
    if (caseId == 3) { settings.baudRate = 49; }
    if (caseId == 4) { settings.baudRate = 4000001; }
    if (caseId == 5) { settings.dataBits = 7; }
    if (caseId == 6) { settings.dataBits = 9; }
    if (caseId == 7) { settings.parity = 2; }
    if (caseId == 8) { settings.parity = 3; }
    if (caseId == 9) { settings.parity = 1; }
    if (caseId == 10) { settings.parity = 4; }
    if (caseId == 11) { settings.stopBits = 2; }
    if (caseId == 12) { settings.stopBits = 0; }
    if (caseId == 13) { settings.stopBits = 3; }
    if (caseId == 14) { settings.timeoutMs = 1; settings.totalTimeoutMs = 1; }
    if (caseId == 15) { settings.timeoutMs = 60000; settings.totalTimeoutMs = 120000; }
    if (caseId == 16) { settings.timeoutMs = 0; }
    if (caseId == 17) { settings.timeoutMs = 60001; }
    if (caseId == 18) { settings.totalTimeoutMs = 999; }
    if (caseId == 19) { settings.totalTimeoutMs = 120001; }
    if (caseId == 20) { settings.portName.clear(); }
    if (caseId == 21) { settings.portName.prepend(QLatin1Char(' ')); }
    if (caseId == 22) { settings.portName.append(QLatin1Char(' ')); }
    if (caseId == 23) { settings.portName.append(QLatin1Char('\n')); }
    if (caseId == 24) { settings.portName.append(QChar(u16(127))); }
    if (caseId == 25) { settings.portName = QString(256, QLatin1Char('P')); }
    if (caseId == 26) { settings.portName = QString(257, QLatin1Char('P')); }
    if (caseId == 27) { settings.portName.append(QChar(u16(0))); }
    if (caseId == 28) { settings.totalTimeoutMs = 0; }
    if (caseId == 29) { settings.parity = -1; }
    QCOMPARE(validatePlcSerialSettings(settings, &error), accepted);
    if (!accepted) { QVERIFY(!error.isEmpty()); }
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::invalidBatchNeverCreatesEndpoint_data
 * Description     : Include later invalid requests so validation cannot stop after only the first item.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::invalidBatchNeverCreatesEndpoint_data() //20261114-8 add
{
    int index;
    const char *names[] = {"station0", "station248", "later-count0", "later-address65536",
        "later-wrap", "later-register126", "later-coil2001", "later-multi-count-mismatch",
        "later-multi-register-overflow", "later-nonlogical-multi-coil", "later-duplicate-id",
        "later-empty-id", "later-unsupported-function", "empty-batch", "batch257", "invalid-settings"};

    QTest::addColumn<int>("caseId");
    for (index = 0; index < 16; ++index) { QTest::newRow(names[index]) << index; }
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::invalidBatchNeverCreatesEndpoint
 * Description     : Require whole-batch validation before factory construction, open or any write.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::invalidBatchNeverCreatesEndpoint() //20261114-8 add
{
    ScriptedEndpoint endpoint;
    PlcSerialSettings settings;
    QVector<SimulationModbusRequest> requests;
    QJsonObject report;
    int station;
    int index;
    QFETCH(int, caseId);

    settings = serialSettings();
    station = 17;
    requests = {goldenExchange(3, QStringLiteral("first")).request,
                goldenExchange(16, QStringLiteral("later")).request};
    if (caseId == 0) { station = 0; }
    if (caseId == 1) { station = 248; }
    if (caseId == 2) { requests[1].count = 0; }
    if (caseId == 3) { requests[1].address = 65536; }
    if (caseId == 4) { requests[1].address = 65535; }
    if (caseId == 5) { requests[1] = goldenExchange(3).request; requests[1].count = 126; }
    if (caseId == 6) { requests[1] = goldenExchange(1).request; requests[1].count = 2001; }
    if (caseId == 7) { requests[1].values.removeLast(); }
    if (caseId == 8) { requests[1].values[0] = 65536; }
    if (caseId == 9) { requests[1] = goldenExchange(15).request; requests[1].values[0] = 2; }
    if (caseId == 10) { requests[1].id = requests[0].id; }
    if (caseId == 11) { requests[1].id = QStringLiteral(" \t "); }
    if (caseId == 12) { requests[1].operation = SimulationModbusOperation(17); }
    if (caseId == 13) { requests.clear(); }
    if (caseId == 14)
    {
        requests.clear();
        for (index = 0; index < 257; ++index)
        {
            requests.append(goldenExchange(4, QString::number(index)).request);
        }
    }
    if (caseId == 15) { settings.dataBits = 7; }
    report = runPlcSerialBatch(settings, station, requests, {}, endpoint.factory());
    QVERIFY(!report.value(QStringLiteral("ok")).toBool());
    QVERIFY(!report.value(QStringLiteral("error")).toString().isEmpty());
    QCOMPARE(endpoint.factoryCount, 0);
    QCOMPARE(endpoint.openCount, 0);
    QCOMPARE(endpoint.closeCount, 0);
    QVERIFY(endpoint.writes.isEmpty());
    QVERIFY(!report.value(QStringLiteral("opened")).toBool());
    QVERIFY(report.value(QStringLiteral("sessionInactive")).toBool());
    QVERIFY(report.value(QStringLiteral("closed")).toBool());
    QVERIFY(!report.value(QStringLiteral("closeAttempted")).toBool());
    QVERIFY(!report.value(QStringLiteral("closeConfirmed")).toBool());
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::goldenTransactions_data
 * Description     : Run all eight standard functions both whole-frame and byte-fragmented.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::goldenTransactions_data() //20261114-8 add
{
    int function;
    int fragment;
    QByteArray name;

    QTest::addColumn<int>("function");
    QTest::addColumn<int>("fragment");
    for (function = 1; function <= 16; ++function)
    {
        if (function > 6 && function < 15) { continue; }
        for (fragment = 1; fragment <= 256; fragment += 255)
        {
            name = QByteArray::number(function) + "-fragment-" + QByteArray::number(fragment);
            QTest::newRow(name.constData()) << function << fragment;
        }
    }
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::goldenTransactions
 * Description     : Match full fixed wire vectors and explicit values before claiming confirmation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::goldenTransactions() //20261114-8 add
{
    ScriptedEndpoint endpoint;
    GoldenExchange exchange;
    QJsonObject report;
    QJsonObject result;
    QJsonArray values;
    int index;
    QFETCH(int, function);
    QFETCH(int, fragment);

    exchange = goldenExchange(function);
    endpoint.exchanges = {exchange};
    endpoint.fragmentSize = fragment;
    QCOMPARE(independentlySeal(exchange.outgoing.left(exchange.outgoing.size() - 2)), exchange.outgoing);
    QCOMPARE(independentlySeal(exchange.incoming.left(exchange.incoming.size() - 2)), exchange.incoming);
    report = runPlcSerialBatch(serialSettings(), 17, {exchange.request}, {}, endpoint.factory());
    QVERIFY2(report.value(QStringLiteral("ok")).toBool(), qPrintable(report.value(QStringLiteral("error")).toString()));
    QVERIFY(endpoint.matched);
    QCOMPARE(endpoint.writes, QVector<QByteArray>{exchange.outgoing});
    QCOMPARE(endpoint.factoryCount, 1);
    QCOMPARE(endpoint.openCount, 1);
    QCOMPARE(endpoint.closeCount, 1);
    QVERIFY(!endpoint.stillOpen);
    QCOMPARE(report.value(QStringLiteral("completedCount")).toInt(), 1);
    result = report.value(QStringLiteral("results")).toArray()[0].toObject();
    QCOMPARE(result.value(QStringLiteral("requestHex")).toString(), QString::fromLatin1(exchange.outgoing.toHex()));
    QCOMPARE(result.value(QStringLiteral("responseHex")).toString(), QString::fromLatin1(exchange.incoming.toHex()));
    QCOMPARE(result.value(QStringLiteral("writeAcknowledged")).toBool(), function == 5 || function == 6 || function >= 15);
    QVERIFY(!result.value(QStringLiteral("remoteStateUnconfirmed")).toBool());
    values = result.value(QStringLiteral("values")).toArray();
    QCOMPARE(values.size(), exchange.expectedValues.size());
    for (index = 0; index < values.size(); ++index) { QCOMPARE(values[index].toInt(), int(exchange.expectedValues[index])); }
    QVERIFY(report.value(QStringLiteral("sessionInactive")).toBool());
    QVERIFY(report.value(QStringLiteral("closeAttempted")).toBool());
    QVERIFY(report.value(QStringLiteral("closeConfirmed")).toBool());
    QVERIFY(report.value(QStringLiteral("closed")).toBool());
    QCOMPARE(report.value(QStringLiteral("endpointKind")).toString(), QStringLiteral("injected-controlled-endpoint"));
    QCOMPARE(result.value(QStringLiteral("scope")).toString(), QStringLiteral("controlled-rtu-session"));
    QVERIFY(!report.value(QStringLiteral("physicalEndpointUsed")).toBool(true));
    QVERIFY(!report.value(QStringLiteral("hmiExecutionConfirmed")).toBool(true));
    QVERIFY(!report.value(QStringLiteral("projectAddressMapping")).toBool(true));
    QVERIFY(!result.value(QStringLiteral("physicalEndpointUsed")).toBool(true));
    QCOMPARE(report.value(QStringLiteral("automaticRetries")).toInt(-1), 0);
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::factoryAndCallbacksStayInWorker
 * Description     : Construct the endpoint only after entering the designated worker thread.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::factoryAndCallbacksStayInWorker() //20261114-8 add
{
    ScriptedEndpoint endpoint;
    PlcSerialSettings settings;
    QJsonObject report;
    QThread *mainThread;
    QThread *worker;
    int index;

    settings = serialSettings();
    settings.baudRate = 115200;
    settings.parity = 2;
    settings.stopBits = 2;
    endpoint.exchanges = {goldenExchange(4)};
    mainThread = QThread::currentThread();
    worker = QThread::create([&]() {
        report = runPlcSerialBatch(settings, 17, {endpoint.exchanges[0].request}, {}, endpoint.factory());
    });
    QCOMPARE(endpoint.factoryCount, 0);
    worker->start();
    QVERIFY(worker->wait(5000));
    QVERIFY(report.value(QStringLiteral("ok")).toBool());
    QVERIFY(endpoint.factoryThread != mainThread);
    QCOMPARE(endpoint.factoryThread, worker);
    for (index = 0; index < endpoint.callbackThreads.size(); ++index)
    {
        QCOMPARE(endpoint.callbackThreads[index], worker);
    }
    delete worker;
    QCOMPARE(endpoint.observedSettings.portName, settings.portName);
    QCOMPARE(endpoint.observedSettings.baudRate, 115200);
    QCOMPARE(endpoint.observedSettings.parity, 2);
    QCOMPARE(endpoint.observedSettings.stopBits, 2);
    QVERIFY(bool(endpoint.observedCancellation));
    QVERIFY(!endpoint.stillOpen);
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::oneOpenForFrozenBatch
 * Description     : Serialize attributed requests through one endpoint without merging or reordering.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::oneOpenForFrozenBatch() //20261114-8 add
{
    ScriptedEndpoint endpoint;
    QVector<SimulationModbusRequest> requests;
    QJsonObject report;
    QJsonArray results;
    int function;
    int index;

    for (function = 1; function <= 16; ++function)
    {
        if (function > 6 && function < 15) { continue; }
        endpoint.exchanges.append(goldenExchange(function, QString::number(function)));
        requests.append(endpoint.exchanges.last().request);
    }
    report = runPlcSerialBatch(serialSettings(), 17, requests, {}, endpoint.factory());
    QVERIFY(report.value(QStringLiteral("ok")).toBool());
    QCOMPARE(endpoint.openCount, 1);
    QCOMPARE(endpoint.closeCount, 1);
    QCOMPARE(endpoint.writes.size(), 8);
    results = report.value(QStringLiteral("results")).toArray();
    QCOMPARE(results.size(), 8);
    for (index = 0; index < results.size(); ++index)
    {
        QCOMPARE(results[index].toObject().value(QStringLiteral("requestId")).toString(), requests[index].id);
        QVERIFY(results[index].toObject().value(QStringLiteral("ok")).toBool());
        QCOMPARE(endpoint.writes[index], endpoint.exchanges[index].outgoing);
    }
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::maximumBatch
 * Description     : Accept the documented 256-request boundary with exactly one open and close.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::maximumBatch() //20261114-8 add
{
    ScriptedEndpoint endpoint;
    QVector<SimulationModbusRequest> requests;
    QJsonObject report;
    int index;

    for (index = 0; index < 256; ++index)
    {
        endpoint.exchanges.append(goldenExchange(4, QString::number(index)));
        requests.append(endpoint.exchanges.last().request);
    }
    report = runPlcSerialBatch(serialSettings(), 17, requests, {}, endpoint.factory());
    QVERIFY(report.value(QStringLiteral("ok")).toBool());
    QCOMPARE(report.value(QStringLiteral("completedCount")).toInt(), 256);
    QCOMPARE(report.value(QStringLiteral("results")).toArray().size(), 256);
    QCOMPARE(endpoint.writes.size(), 256);
    QCOMPARE(endpoint.factoryCount, 1);
    QCOMPARE(endpoint.openCount, 1);
    QCOMPARE(endpoint.closeCount, 1);
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::openingFailures_data
 * Description     : Separate failed-open cleanup obligations from malformed injected callback sets.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::openingFailures_data() //20261114-8 add
{
    QTest::addColumn<int>("missing");
    QTest::newRow("open-failure") << -1;
    QTest::newRow("missing-open") << 0;
    QTest::newRow("missing-write") << 1;
    QTest::newRow("missing-read") << 2;
    QTest::newRow("missing-close") << 3;
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::openingFailures
 * Description     : Reject before writing and avoid claiming a close callback was attempted.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::openingFailures() //20261114-8 add
{
    ScriptedEndpoint endpoint;
    QJsonObject report;
    QFETCH(int, missing);

    endpoint.openOk = false;
    endpoint.missingCallback = missing;
    report = runPlcSerialBatch(serialSettings(), 17, {goldenExchange(3).request}, {}, endpoint.factory());
    QVERIFY(!report.value(QStringLiteral("ok")).toBool());
    QVERIFY(!report.value(QStringLiteral("opened")).toBool());
    QCOMPARE(endpoint.factoryCount, 1);
    QCOMPARE(endpoint.openCount, missing == -1 ? 1 : 0);
    QCOMPARE(endpoint.closeCount, 0);
    QVERIFY(endpoint.writes.isEmpty());
    QVERIFY(report.value(QStringLiteral("sessionInactive")).toBool());
    QVERIFY(report.value(QStringLiteral("closed")).toBool());
    QVERIFY(!report.value(QStringLiteral("closeAttempted")).toBool());
    QVERIFY(!report.value(QStringLiteral("closeConfirmed")).toBool());
    QVERIFY(!report.value(QStringLiteral("error")).toString().isEmpty());
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::middleFailurePreservesConfirmedReceipts_data
 * Description     : Distinguish read failures from ambiguous writes after an earlier confirmed result.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::middleFailurePreservesConfirmedReceipts_data() //20261114-8 add
{
    int function;
    int fault;
    QByteArray name;

    QTest::addColumn<int>("function");
    QTest::addColumn<int>("fault");
    for (function = 3; function <= 16; ++function)
    {
        if (function != 3 && function != 6 && function != 15 && function != 16) { continue; }
        for (fault = 0; fault < 4; ++fault)
        {
            name = QByteArray::number(function) + "-fault-" + QByteArray::number(fault);
            QTest::newRow(name.constData()) << function << fault;
        }
    }
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::middleFailurePreservesConfirmedReceipts
 * Description     : Preserve earlier confirmation, publish the failed attempt and discard later work.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::middleFailurePreservesConfirmedReceipts() //20261114-8 add
{
    ScriptedEndpoint endpoint;
    QVector<SimulationModbusRequest> requests;
    QJsonObject report;
    QJsonArray results;
    QJsonObject failed;
    QByteArray body;
    QString expectedCode;
    int index;
    QFETCH(int, function);
    QFETCH(int, fault);

    endpoint.exchanges = {goldenExchange(4, QStringLiteral("confirmed")),
        goldenExchange(function, QStringLiteral("fails")), goldenExchange(16, QStringLiteral("discard"))};
    body = endpoint.exchanges[1].incoming.left(endpoint.exchanges[1].incoming.size() - 2);
    if (fault == 0)
    {
        endpoint.exchanges[1].incoming[endpoint.exchanges[1].incoming.size() - 1] = char(0);
        expectedCode = QStringLiteral("crc_mismatch");
    }
    if (fault == 1)
    {
        body[0] = char(18);
        endpoint.exchanges[1].incoming = independentlySeal(body);
        expectedCode = QStringLiteral("wrong_station");
    }
    if (fault == 2)
    {
        endpoint.exchanges[1].incoming.append(endpoint.exchanges[1].incoming);
        expectedCode = QStringLiteral("extra_response_bytes");
    }
    if (fault == 3)
    {
        body[function == 3 ? 2 : 5] = char(1);
        endpoint.exchanges[1].incoming = independentlySeal(body);
        expectedCode = function == 3 ? QStringLiteral("wrong_byte_count") : QStringLiteral("write_echo_mismatch");
    }
    for (index = 0; index < endpoint.exchanges.size(); ++index) { requests.append(endpoint.exchanges[index].request); }
    report = runPlcSerialBatch(serialSettings(), 17, requests, {}, endpoint.factory());
    QVERIFY(!report.value(QStringLiteral("ok")).toBool());
    results = report.value(QStringLiteral("results")).toArray();
    QCOMPARE(results.size(), 2);
    QVERIFY(results[0].toObject().value(QStringLiteral("ok")).toBool());
    QCOMPARE(results[0].toObject().value(QStringLiteral("values")).toArray()[0].toInt(), 0xa55a);
    failed = results[1].toObject();
    QCOMPARE(failed.value(QStringLiteral("errorCode")).toString(), expectedCode);
    QVERIFY(!failed.value(QStringLiteral("ok")).toBool());
    QVERIFY(failed.value(QStringLiteral("values")).toArray().isEmpty());
    QVERIFY(!failed.value(QStringLiteral("writeAcknowledged")).toBool());
    QCOMPARE(failed.value(QStringLiteral("remoteStateUnconfirmed")).toBool(), function != 3);
    QCOMPARE(failed.value(QStringLiteral("discardedQueuedRequests")).toInt(), 1);
    QCOMPARE(endpoint.writes.size(), 2);
    QCOMPARE(endpoint.openCount, 1);
    QCOMPARE(endpoint.closeCount, 1);
    QVERIFY(report.value(QStringLiteral("closeConfirmed")).toBool());
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::closeFailureIsNotClosed_data
 * Description     : Cover cleanup failure alone and after a transaction failure that must stay first.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::closeFailureIsNotClosed_data() //20261114-8 add
{
    QTest::addColumn<bool>("transactionFails");
    QTest::newRow("close-only") << false;
    QTest::newRow("transaction-error-before-close") << true;
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::closeFailureIsNotClosed
 * Description     : Never equate an inactive transport with confirmed endpoint release.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::closeFailureIsNotClosed() //20261114-8 add
{
    ScriptedEndpoint endpoint;
    QJsonObject report;
    QJsonObject result;
    QString transactionError;
    QFETCH(bool, transactionFails);

    endpoint.exchanges = {goldenExchange(6)};
    endpoint.closeOk = false;
    if (transactionFails)
    {
        endpoint.exchanges[0].incoming[7] = char(0);
    }
    report = runPlcSerialBatch(serialSettings(), 17, {endpoint.exchanges[0].request}, {}, endpoint.factory());
    QVERIFY(!report.value(QStringLiteral("ok")).toBool());
    QCOMPARE(endpoint.closeCount, 1);
    QVERIFY(endpoint.stillOpen);
    QVERIFY(report.value(QStringLiteral("opened")).toBool());
    QVERIFY(report.value(QStringLiteral("sessionInactive")).toBool());
    QVERIFY(report.value(QStringLiteral("closeAttempted")).toBool());
    QVERIFY(!report.value(QStringLiteral("closeConfirmed")).toBool());
    QVERIFY(!report.value(QStringLiteral("closed")).toBool(true));
    result = report.value(QStringLiteral("results")).toArray()[0].toObject();
    if (transactionFails)
    {
        transactionError = result.value(QStringLiteral("error")).toString();
        QCOMPARE(result.value(QStringLiteral("errorCode")).toString(), QStringLiteral("crc_mismatch"));
        QVERIFY(!transactionError.isEmpty());
        QCOMPARE(report.value(QStringLiteral("error")).toString(), transactionError);
        QVERIFY(report.value(QStringLiteral("error")).toString() != QStringLiteral("controlled-close-error"));
        QVERIFY(result.value(QStringLiteral("remoteStateUnconfirmed")).toBool());
    }
    else
    {
        QVERIFY(result.value(QStringLiteral("ok")).toBool());
        QVERIFY(result.value(QStringLiteral("writeAcknowledged")).toBool());
        QVERIFY(!result.value(QStringLiteral("remoteStateUnconfirmed")).toBool());
        QCOMPARE(report.value(QStringLiteral("error")).toString(), QStringLiteral("controlled-close-error"));
    }
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::cancellationBoundaries_data
 * Description     : Stop before creation, after factory, in open/write/read or between fragments.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::cancellationBoundaries_data() //20261114-8 add
{
    QTest::addColumn<int>("phase");
    QTest::newRow("before-factory") << 0;
    QTest::newRow("factory-cancels-before-open") << 1;
    QTest::newRow("open-cancels-before-write") << 2;
    QTest::newRow("write-cancels-before-read") << 3;
    QTest::newRow("first-read-cancels") << 4;
    QTest::newRow("second-read-cancels") << 5;
    QTest::newRow("complete-prefix-without-boundary") << 6;
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::cancellationBoundaries
 * Description     : Discard pending work without accepting partial data or retrying ambiguous writes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::cancellationBoundaries() //20261114-8 add
{
    ScriptedEndpoint endpoint;
    std::shared_ptr<std::atomic_bool> cancellation;
    QJsonObject report;
    QJsonArray results;
    QJsonObject result;
    QFETCH(int, phase);

    endpoint.exchanges = {goldenExchange(16, QStringLiteral("attempt")), goldenExchange(6, QStringLiteral("unsent"))};
    cancellation = std::make_shared<std::atomic_bool>(phase == 0);
    if (phase == 1) { endpoint.onFactory = [&]() { cancellation->store(true); }; }
    if (phase == 2) { endpoint.onOpen = [&]() { cancellation->store(true); }; }
    if (phase == 3) { endpoint.onWrite = [&]() { cancellation->store(true); }; }
    if (phase == 4) { endpoint.onRead = [&]() { cancellation->store(true); }; }
    if (phase == 5)
    {
        endpoint.fragmentSize = 1;
        endpoint.onRead = [&]() { if (endpoint.readCount == 2) { cancellation->store(true); } };
    }
    if (phase == 6)
    {
        endpoint.noBoundary = true;
        endpoint.onRead = [&]() { if (endpoint.readCount == 2) { cancellation->store(true); } };
    }
    report = runPlcSerialBatch(serialSettings(), 17,
        {endpoint.exchanges[0].request, endpoint.exchanges[1].request}, cancellation, endpoint.factory());
    QVERIFY(!report.value(QStringLiteral("ok")).toBool());
    QCOMPARE(endpoint.factoryCount, phase == 0 ? 0 : 1);
    QCOMPARE(endpoint.openCount, phase <= 1 ? 0 : 1);
    QCOMPARE(endpoint.closeCount, phase <= 1 ? 0 : 1);
    QCOMPARE(endpoint.writes.size(), phase <= 2 ? 0 : 1);
    QCOMPARE(endpoint.readCount, phase <= 3 ? 0 : (phase == 4 ? 1 : 2));
    QVERIFY(report.value(QStringLiteral("cancelled")).toBool());
    QVERIFY(report.value(QStringLiteral("sessionInactive")).toBool());
    QVERIFY(report.value(QStringLiteral("closed")).toBool());
    results = report.value(QStringLiteral("results")).toArray();
    if (phase >= 2)
    {
        QCOMPARE(results.size(), 1);
        result = results[0].toObject();
        QCOMPARE(result.value(QStringLiteral("errorCode")).toString(), QStringLiteral("cancelled"));
        QVERIFY(result.value(QStringLiteral("values")).toArray().isEmpty());
        QVERIFY(!result.value(QStringLiteral("writeAcknowledged")).toBool());
        QCOMPARE(result.value(QStringLiteral("requestMayHaveReachedSlave")).toBool(), phase >= 3);
        QCOMPARE(result.value(QStringLiteral("remoteStateUnconfirmed")).toBool(), phase >= 3);
        QCOMPARE(result.value(QStringLiteral("discardedQueuedRequests")).toInt(), 1);
    }
    else { QVERIFY(results.isEmpty()); }
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::totalDeadline_data
 * Description     : Cover total-budget expiry before open/write and after earlier confirmation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::totalDeadline_data() //20261114-8 add
{
    QTest::addColumn<int>("phase");
    QTest::newRow("factory-exhausts-total") << 0;
    QTest::newRow("open-exhausts-total") << 1;
    QTest::newRow("write-exhausts-total") << 2;
    QTest::newRow("second-response-exhausts-total") << 3;
}

/******************************************************************************
 * Function Name   : PlcSerialSessionTests::totalDeadline
 * Description     : Stop terminally at the explicit batch budget and preserve confirmed receipts.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void PlcSerialSessionTests::totalDeadline() //20261114-8 add
{
    ScriptedEndpoint endpoint;
    PlcSerialSettings settings;
    QJsonObject report;
    QJsonArray results;
    QJsonObject last;
    QFETCH(int, phase);

    settings = serialSettings();
    settings.timeoutMs = 30;
    settings.totalTimeoutMs = 30;
    endpoint.exchanges = {goldenExchange(4, QStringLiteral("first")),
        goldenExchange(16, QStringLiteral("second")), goldenExchange(6, QStringLiteral("never"))};
    if (phase == 0) { endpoint.onFactory = []() { QTest::qWait(50); }; }
    if (phase == 1) { endpoint.onOpen = []() { QTest::qWait(50); }; }
    if (phase == 2) { endpoint.onWrite = []() { QTest::qWait(50); }; }
    if (phase == 3)
    {
        endpoint.onRead = [&]() { if (endpoint.writes.size() == 2) { QTest::qWait(50); } };
    }
    report = runPlcSerialBatch(settings, 17,
        {endpoint.exchanges[0].request, endpoint.exchanges[1].request, endpoint.exchanges[2].request}, {}, endpoint.factory());
    QVERIFY(!report.value(QStringLiteral("ok")).toBool());
    QVERIFY(report.value(QStringLiteral("totalDeadlineReached")).toBool());
    QVERIFY(!report.value(QStringLiteral("cancelled")).toBool());
    QCOMPARE(endpoint.factoryCount, 1);
    QCOMPARE(endpoint.openCount, phase == 0 ? 0 : 1);
    QCOMPARE(endpoint.writes.size(), phase <= 1 ? 0 : (phase == 2 ? 1 : 2));
    QCOMPARE(endpoint.closeCount, phase == 0 ? 0 : 1);
    QVERIFY(report.value(QStringLiteral("sessionInactive")).toBool());
    QVERIFY(report.value(QStringLiteral("closed")).toBool());
    results = report.value(QStringLiteral("results")).toArray();
    if (phase >= 1)
    {
        QCOMPARE(results.size(), phase == 3 ? 2 : 1);
        last = results.last().toObject();
        QVERIFY(!last.value(QStringLiteral("ok")).toBool());
        QVERIFY(last.value(QStringLiteral("values")).toArray().isEmpty());
        QVERIFY(!last.value(QStringLiteral("writeAcknowledged")).toBool());
        QCOMPARE(last.value(QStringLiteral("requestMayHaveReachedSlave")).toBool(), phase >= 2);
        QCOMPARE(last.value(QStringLiteral("remoteStateUnconfirmed")).toBool(), phase == 3);
        if (phase == 3)
        {
            QVERIFY(results[0].toObject().value(QStringLiteral("ok")).toBool());
            QCOMPARE(last.value(QStringLiteral("discardedQueuedRequests")).toInt(), 1);
        }
    }
    else { QVERIFY(results.isEmpty()); }
}

QTEST_GUILESS_MAIN(PlcSerialSessionTests)
#include "plc_serial_session_tests.moc"
