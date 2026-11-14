#include "device_transfer_engine.h"

#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QtTest>
#include <cstring>

//20261114-5 add: The fake records actual engine calls and returns scripted faults without opening a device.
struct FakeTransferIo
{
    int openCount = 0;
    int claimCount = 0;
    int writeCount = 0;
    int readCount = 0;
    int releaseCount = 0;
    int closeCount = 0;
    int payloadCount = 0;
    int lastCommand = -1;
    int expectedTimeout = 10000;
    bool openSucceeds = true;
    int claimResult = 0;
    int releaseResult = 0;
    int closeResult = 0;
    int failedWriteCommand = -1;
    int failedWriteResult = -99;
    int failedPayload = -1;
    int failedPayloadResult = -99;
    int failedReadCommand = -1;
    int failedReadOccurrence = 1;
    int failedReadResult = 1;
    int failedReadByte = 'N';
    bool cancel = false;
    bool cancelOnOpen = false;
    bool cancelOnClaim = false;
    int cancelOnCommand = -1;
    int cancelAfterPayload = -1;
    int cancelOnReadCommand = -1;
    int cancelOnReadOccurrence = 1;
    QByteArray commands;
    QVector<QByteArray> commandFrames;
    QVector<QByteArray> payloadFrames;
    QMap<int, int> commandReads;
    QString problem;

    /******************************************************************************
     * Function Name   : FakeTransferIo::callbacks
     * Description     : Bind callbacks to this deterministic fake rather than a USB driver.
     * Input Parameters: None
     * Return Value    : Injectable I/O functions.
     * Date            : 2026.11.14
     ******************************************************************************/
    DeviceTransferIo callbacks() //20261114-5 add
    {
        DeviceTransferIo io;

        io.open = [this](QString *error) {
            ++openCount;
            cancel = cancel || cancelOnOpen;
            if (!openSucceeds && error != nullptr) { *error = QStringLiteral("fake driver missing"); }
            return openSucceeds;
        };
        io.claim = [this](int interfaceNumber) {
            ++claimCount;
            if (interfaceNumber != 0) { problem = QStringLiteral("wrong claim interface"); }
            cancel = cancel || cancelOnClaim;
            return claimResult;
        };
        io.write = [this](int endpoint, const QByteArray &bytes, int timeout) {
            return write(endpoint, bytes, timeout);
        };
        io.read = [this](int endpoint, char *bytes, int maximum, int timeout) {
            return read(endpoint, bytes, maximum, timeout);
        };
        io.release = [this](int interfaceNumber) {
            ++releaseCount;
            if (interfaceNumber != 0) { problem = QStringLiteral("wrong release interface"); }
            return releaseResult;
        };
        io.close = [this]() { ++closeCount; return closeResult; };
        return io;
    }

    /******************************************************************************
     * Function Name   : FakeTransferIo::write
     * Description     : Capture commands or payloads and inject one exact write-count failure.
     * Input Parameters: endpoint, bytes, timeout
     * Return Value    : Complete or explicitly scripted driver count.
     * Date            : 2026.11.14
     ******************************************************************************/
    int write(int endpoint, const QByteArray &bytes, int timeout) //20261114-5 add
    {
        bool command;

        ++writeCount;
        if (endpoint != 0x01 || timeout != expectedTimeout) { problem = QStringLiteral("wrong write endpoint/timeout"); }
        command = bytes.size() == 32 && bytes.startsWith(QByteArray("KinSealStudioKiss", 17));
        if (command)
        {
            lastCommand = u8(bytes.at(17));
            commands.append(char(lastCommand));
            commandFrames.append(bytes);
            cancel = cancel || lastCommand == cancelOnCommand;
            if (lastCommand == failedWriteCommand) { return failedWriteResult; }
        }
        else
        {
            payloadFrames.append(bytes);
            ++payloadCount;
            cancel = cancel || payloadCount == cancelAfterPayload;
            if (payloadCount == failedPayload) { return failedPayloadResult; }
        }
        return int(bytes.size());
    }

    /******************************************************************************
     * Function Name   : FakeTransferIo::read
     * Description     : Return the observed first-byte replies or one explicitly scripted length/ACK failure.
     * Input Parameters: endpoint, bytes, maximum, timeout
     * Return Value    : Driver count, including the qualified post-reset -5.
     * Date            : 2026.11.14
     ******************************************************************************/
    int read(int endpoint, char *bytes, int maximum, int timeout) //20261114-5 add
    {
        QByteArray response;
        int returned;
        int copied;

        ++readCount;
        ++commandReads[lastCommand];
        if (endpoint != 0x81 || timeout != expectedTimeout) { problem = QStringLiteral("wrong read endpoint/timeout"); }
        if (lastCommand == 0x31 || lastCommand == 0x53)
        {
            returned = 24;
            response = QByteArray(24, char(0));
            response[0] = lastCommand == 0x31 ? char(0x4c) : char(0);
            if (maximum != 32) { problem = QStringLiteral("wrong identification capacity"); }
        }
        else if (lastCommand == 0x1a)
        {
            returned = -5;
            if (maximum != 4) { problem = QStringLiteral("wrong reset capacity"); }
        }
        else
        {
            returned = 1;
            response = QByteArray(1, 'O');
            if (maximum != 4) { problem = QStringLiteral("wrong ACK capacity"); }
        }
        if (lastCommand == failedReadCommand && commandReads.value(lastCommand) == failedReadOccurrence)
        {
            returned = failedReadResult;
            response = QByteArray(qMax(0, qMin(returned, maximum)), char(failedReadByte));
        }
        copied = qMin(maximum, int(response.size()));
        if (copied > 0) { std::memcpy(bytes, response.constData(), size_t(copied)); }
        if (lastCommand == cancelOnReadCommand && commandReads.value(lastCommand) == cancelOnReadOccurrence)
        {
            cancel = true;
        }
        return returned;
    }
};

/******************************************************************************
 * Function Name   : markerPayloads
 * Description     : Build distinct bounded blocks so reordering and upload framing remain observable.
 * Input Parameters: count
 * Return Value    : Synthetic compiler blocks.
 * Date            : 2026.11.14
 ******************************************************************************/
static QVector<QByteArray> markerPayloads(int count) //20261114-5 add
{
    QVector<QByteArray> payloads;
    int index;

    for (index = 0; index < count; ++index)
    {
        payloads.append(QByteArray(257 + index, char(index + 1)));
    }
    return payloads;
}

//20261114-5 add: All tests are app-less and use callbacks; no native discovery or USB symbol is linked.
class DeviceTransferEngineTests : public QObject
{
    Q_OBJECT
private slots:
    void normalPlans_data();
    void normalPlans();
    void preflightRejections_data();
    void preflightRejections();
    void openAndClaimFailures_data();
    void openAndClaimFailures();
    void writeFailures_data();
    void writeFailures();
    void replyFailures_data();
    void replyFailures();
    void postResetFailures_data();
    void postResetFailures();
    void cancellation_data();
    void cancellation();
    void commitIgnoresCancellation();
    void cleanupPreservesFirstFailure();
    void cleanupFailureAfterCommit();
    void jsonKeepsEvidenceBounded();
};

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::normalPlans_data
 * Description     : Cover three segment sets and both observed start-option byte sequences.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::normalPlans_data() //20261114-5 add
{
    QTest::addColumn<int>("blocks");
    QTest::addColumn<QByteArray>("startOptions");
    QTest::newRow("static-no-clear") << 10 << QByteArray::fromHex("000000");
    QTest::newRow("dynamic-no-clear") << 12 << QByteArray::fromHex("000000");
    QTest::newRow("macro-no-clear") << 14 << QByteArray::fromHex("000000");
    QTest::newRow("static-legacy") << 10 << QByteArray::fromHex("010101");
    QTest::newRow("dynamic-legacy") << 12 << QByteArray::fromHex("010101");
    QTest::newRow("macro-legacy") << 14 << QByteArray::fromHex("010101");
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::normalPlans
 * Description     : Verify wire order, length framing, complete lifecycle, and the absence of a screen acceptance claim.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::normalPlans() //20261114-5 add
{
    QFETCH(int, blocks);
    QFETCH(QByteArray, startOptions);
    FakeTransferIo fake;
    DeviceTransferOptions options;
    DeviceTransferResult result;
    QVector<QByteArray> payloads;
    QByteArray expected;
    DeviceTransferEvent event;
    int observed;
    int uploadIndex;

    payloads = markerPayloads(blocks);
    options.startOptions = startOptions;
    options.timeoutMs = 1234;
    fake.expectedTimeout = 1234;
    observed = 0;
    options.onEvent = [&observed](const DeviceTransferEvent &current) { Q_UNUSED(current); ++observed; };
    result = runDeviceTransfer(payloads, fake.callbacks(), options);
    QVERIFY2(result.success, qPrintable(result.message));
    QCOMPARE(fake.problem, QString());
    expected = blocks == 10 ? QByteArray::fromHex("315334350a350c350e35163515351935123514350d351e350f3511351b1a") :
               blocks == 12 ? QByteArray::fromHex("315334350a350b350c350e35163515351935123514350d351e3510350f3511351b1a") :
                              QByteArray::fromHex("315334350a350b350c3536351d350e35163515351935123514350d351e3510350f3511351b1a");
    QCOMPARE(fake.commands, expected);
    QCOMPARE(fake.commandFrames.at(2).mid(18, 3), startOptions);
    QCOMPARE(fake.commandFrames.at(2).mid(21), QByteArray(11, char(0)));
    QCOMPARE(fake.writeCount, (blocks + 2) * 2 + 6 + blocks);
    QCOMPARE(fake.readCount, (blocks + 2) * 2 + 6);
    QCOMPARE(fake.payloadCount, blocks);
    uploadIndex = blocks == 10 ? 7 : blocks == 12 ? 8 : 10;
    QCOMPARE(fake.payloadFrames.at(uploadIndex).left(4).toHex(), QByteArray("00000108"));
    QCOMPARE(fake.payloadFrames.at(uploadIndex).mid(4), payloads.at(7));
    QCOMPARE(fake.commandFrames.at(fake.commandFrames.size() - 4).mid(18, 4).toHex(), QByteArray("0a010000"));
    QCOMPARE(fake.openCount, 1);
    QCOMPARE(fake.claimCount, 1);
    QCOMPARE(fake.releaseCount, 1);
    QCOMPARE(fake.closeCount, 1);
    QCOMPARE(result.writtenSegments.size(), blocks);
    QCOMPARE(result.acknowledgedSegments.size(), blocks + 2);
    QVERIFY(result.opened && result.interfaceClaimed && result.interfaceReleased && result.closed);
    QVERIFY(result.startAccepted && result.transferAccepted && result.commitCommandWritten && result.commitAccepted);
    QVERIFY(result.resetWritten && result.postResetReadReturned && result.postResetReadCompleted);
    QCOMPARE(result.postResetReadResult, -5);
    QCOMPARE(result.outcome, QStringLiteral("transport_complete"));
    QCOMPARE(observed, result.events.size());
    QVERIFY(!result.firstError.present && !result.cancelled && !result.screenDisplayAndTouchConfirmed);
    Q_FOREACH (event, result.events)
    {
        if (event.operation == QStringLiteral("command-write")) { QCOMPARE(event.bytes.size(), 32); }
        if (event.operation == QStringLiteral("payload-write")) { QVERIFY(event.bytes.isEmpty()); QCOMPARE(event.sha256.size(), 64); }
        if (event.operation == QStringLiteral("reply-read")) { QCOMPARE(event.requested, event.expectedReplyLength == 24 ? 32 : 4); }
    }
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::preflightRejections_data
 * Description     : Prepare invalid counts, missing data/callbacks, and unsupported options without device access.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::preflightRejections_data() //20261114-5 add
{
    QTest::addColumn<int>("scenario");
    QTest::addColumn<int>("argument");
    QTest::newRow("zero-blocks") << 0 << 0;
    QTest::newRow("nine-blocks") << 0 << 9;
    QTest::newRow("eleven-blocks") << 0 << 11;
    QTest::newRow("thirteen-blocks") << 0 << 13;
    QTest::newRow("fifteen-blocks") << 0 << 15;
    QTest::newRow("empty-static") << 1 << 10;
    QTest::newRow("empty-dynamic") << 1 << 12;
    QTest::newRow("empty-macro") << 1 << 14;
    QTest::newRow("missing-open") << 2 << 0;
    QTest::newRow("missing-claim") << 2 << 1;
    QTest::newRow("missing-write") << 2 << 2;
    QTest::newRow("missing-read") << 2 << 3;
    QTest::newRow("missing-release") << 2 << 4;
    QTest::newRow("missing-close") << 2 << 5;
    QTest::newRow("zero-timeout") << 3 << 0;
    QTest::newRow("negative-timeout") << 3 << -1;
    QTest::newRow("unknown-options") << 4 << 0;
    QTest::newRow("short-options") << 4 << 1;
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::preflightRejections
 * Description     : Prove every invalid transfer is rejected before open, claim, or any write callback.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::preflightRejections() //20261114-5 add
{
    QFETCH(int, scenario);
    QFETCH(int, argument);
    FakeTransferIo fake;
    DeviceTransferOptions options;
    DeviceTransferIo io;
    DeviceTransferResult result;
    QVector<QByteArray> payloads;

    payloads = markerPayloads(scenario == 0 || scenario == 1 ? argument : 10);
    io = fake.callbacks();
    if (scenario == 1) { payloads.last().clear(); }
    if (scenario == 2)
    {
        if (argument == 0) { io.open = {}; }
        if (argument == 1) { io.claim = {}; }
        if (argument == 2) { io.write = {}; }
        if (argument == 3) { io.read = {}; }
        if (argument == 4) { io.release = {}; }
        if (argument == 5) { io.close = {}; }
    }
    if (scenario == 3) { options.timeoutMs = argument; }
    if (scenario == 4) { options.startOptions = argument == 0 ? QByteArray::fromHex("000100") : QByteArray::fromHex("0000"); }
    result = runDeviceTransfer(payloads, io, options);
    QVERIFY(!result.success && !result.planValidated && !result.opened && !result.interfaceClaimed);
    QVERIFY(result.firstError.present);
    QCOMPARE(result.firstError.phase, QStringLiteral("preflight"));
    QCOMPARE(result.outcome, QStringLiteral("preflight_rejected"));
    QCOMPARE(fake.openCount + fake.claimCount + fake.writeCount + fake.readCount + fake.releaseCount + fake.closeCount, 0);
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::openAndClaimFailures_data
 * Description     : Distinguish missing-driver/open failure from an occupied interface.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::openAndClaimFailures_data() //20261114-5 add
{
    QTest::addColumn<bool>("failedOpen");
    QTest::newRow("driver-missing") << true;
    QTest::newRow("interface-busy") << false;
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::openAndClaimFailures
 * Description     : Close an opened but unclaimed session and issue no protocol write after acquisition failure.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::openAndClaimFailures() //20261114-5 add
{
    QFETCH(bool, failedOpen);
    FakeTransferIo fake;
    DeviceTransferResult result;

    fake.openSucceeds = !failedOpen;
    fake.claimResult = -16;
    result = runDeviceTransfer(markerPayloads(10), fake.callbacks());
    QVERIFY(!result.success && result.planValidated && !result.interfaceClaimed);
    QCOMPARE(result.opened, !failedOpen);
    QCOMPARE(result.firstError.kind, failedOpen ? QStringLiteral("open") : QStringLiteral("claim"));
    QCOMPARE(fake.openCount, 1);
    QCOMPARE(fake.claimCount, failedOpen ? 0 : 1);
    QCOMPARE(fake.writeCount + fake.readCount + fake.releaseCount, 0);
    QCOMPARE(fake.closeCount, failedOpen ? 0 : 1);
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::writeFailures_data
 * Description     : Cover command short/error counts, a partial data block, and reset failure after commit ACK.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::writeFailures_data() //20261114-5 add
{
    QTest::addColumn<int>("scenario");
    QTest::newRow("identify-short") << 0;
    QTest::newRow("identify-failed") << 1;
    QTest::newRow("first-payload-short") << 2;
    QTest::newRow("reset-short-after-commit") << 3;
    QTest::newRow("commit-short-is-unknown") << 4;
    QTest::newRow("commit-write-error-is-unknown") << 5;
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::writeFailures
 * Description     : Retain exact first write counts and stop without retrying, committing, or concealing an accepted commit.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::writeFailures() //20261114-5 add
{
    QFETCH(int, scenario);
    FakeTransferIo fake;
    DeviceTransferResult result;

    if (scenario < 2) { fake.failedWriteCommand = 0x31; fake.failedWriteResult = scenario == 0 ? 31 : -7; }
    if (scenario == 2) { fake.failedPayload = 1; fake.failedPayloadResult = 256; }
    if (scenario == 3) { fake.failedWriteCommand = 0x1a; fake.failedWriteResult = 31; }
    if (scenario >= 4) { fake.failedWriteCommand = 0x1b; fake.failedWriteResult = scenario == 4 ? 31 : -7; }
    result = runDeviceTransfer(markerPayloads(10), fake.callbacks());
    QVERIFY(!result.success && result.firstError.present);
    QCOMPARE(result.firstError.kind, QStringLiteral("write_count"));
    QCOMPARE(fake.releaseCount, 1);
    QCOMPARE(fake.closeCount, 1);
    if (scenario < 2)
    {
        QCOMPARE(fake.commands.toHex(), QByteArray("31"));
        QCOMPARE(fake.readCount, 0);
        QCOMPARE(result.firstError.requested, 32);
        QCOMPARE(result.firstError.returned, scenario == 0 ? 31 : -7);
    }
    if (scenario == 2)
    {
        QCOMPARE(fake.commands.toHex(), QByteArray("315334350a"));
        QCOMPARE(result.firstError.segment, QStringLiteral("SysPara.dat"));
        QCOMPARE(result.firstError.requested, 257);
        QCOMPARE(result.firstError.returned, 256);
        QVERIFY(result.writtenSegments.isEmpty());
        QVERIFY(!result.commitCommandWritten && !result.transferAccepted);
        QCOMPARE(result.outcome, QStringLiteral("partial_transfer_commit_unconfirmed"));
    }
    if (scenario == 3)
    {
        QVERIFY(result.transferAccepted && result.commitCommandWritten && result.commitAccepted);
        QVERIFY(!result.resetWritten && !result.postResetReadReturned);
        QCOMPARE(result.outcome, QStringLiteral("committed_reset_not_written"));
        QCOMPARE(result.writtenSegments.size(), 10);
        QCOMPARE(fake.commandReads.value(0x1b), 1);
        QCOMPARE(fake.commandReads.value(0x1a), 0);
    }
    if (scenario >= 4)
    {
        QVERIFY(result.transferAccepted && result.commitWriteAttempted && !result.commitCommandWritten && !result.commitAccepted);
        QCOMPARE(result.outcome, QStringLiteral("commit_status_unknown"));
        QCOMPARE(result.firstError.command, 0x1b);
        QCOMPARE(fake.commandReads.value(0x1b), 0);
        QVERIFY(!fake.commands.contains(char(0x1a)));
    }
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::replyFailures_data
 * Description     : Cover malformed identification, wrong ACK, zero/long replies, timeout, and uncertain commit status.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::replyFailures_data() //20261114-5 add
{
    QTest::addColumn<int>("command");
    QTest::addColumn<int>("returned");
    QTest::addColumn<int>("byte");
    QTest::newRow("identify-short-not-reassembled") << 0x31 << 12 << 0x4c;
    QTest::newRow("identify-wrong-model") << 0x31 << 24 << 0x4d;
    QTest::newRow("parameters-short") << 0x53 << 23 << 0;
    QTest::newRow("segment-wrong-ack") << 0x0a << 1 << int('N');
    QTest::newRow("segment-empty-ack") << 0x0a << 0 << 0;
    QTest::newRow("segment-long-ack") << 0x0a << 2 << int('O');
    QTest::newRow("status-timeout") << 0x35 << -116 << 0;
    QTest::newRow("disconnect-before-commit-is-error") << 0x35 << -5 << 0;
    QTest::newRow("commit-wrong-ack") << 0x1b << 1 << int('N');
    QTest::newRow("commit-timeout") << 0x1b << -116 << 0;
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::replyFailures
 * Description     : Retain malformed replies and prevent all subsequent data or reset after the first reply failure.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::replyFailures() //20261114-5 add
{
    QFETCH(int, command);
    QFETCH(int, returned);
    QFETCH(int, byte);
    FakeTransferIo fake;
    DeviceTransferResult result;

    fake.failedReadCommand = command;
    fake.failedReadResult = returned;
    fake.failedReadByte = byte;
    result = runDeviceTransfer(markerPayloads(10), fake.callbacks());
    QVERIFY(!result.success && result.firstError.present && !result.resetWritten);
    QCOMPARE(result.firstError.command, command);
    QCOMPARE(result.firstError.returned, returned);
    QCOMPARE(fake.commandReads.value(command), 1);
    QVERIFY(!fake.commands.contains(char(0x1a)));
    QCOMPARE(fake.releaseCount, 1);
    QCOMPARE(fake.closeCount, 1);
    if (command == 0x31 || command == 0x53 || command == 0x35 || command == 0x0a)
    {
        QCOMPARE(fake.payloadCount, 0);
        QVERIFY(!result.commitCommandWritten && !result.commitAccepted);
    }
    if (command == 0x1b)
    {
        QVERIFY(result.transferAccepted && result.commitCommandWritten && !result.commitAccepted);
        QCOMPARE(result.outcome, QStringLiteral("commit_status_unknown"));
        QCOMPARE(result.writtenSegments.size(), 10);
    }
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::postResetFailures_data
 * Description     : Reject unrelated errors and positive/empty post-reset reads rather than inventing a reset ACK.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::postResetFailures_data() //20261114-5 add
{
    QTest::addColumn<int>("returned");
    QTest::newRow("empty") << 0;
    QTest::newRow("unproved-positive-reply") << 1;
    QTest::newRow("other-error") << -7;
    QTest::newRow("other-timeout") << -116;
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::postResetFailures
 * Description     : Preserve accepted commit and reset write while keeping an unqualified reset-read outcome incomplete.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::postResetFailures() //20261114-5 add
{
    QFETCH(int, returned);
    FakeTransferIo fake;
    DeviceTransferResult result;

    fake.failedReadCommand = 0x1a;
    fake.failedReadResult = returned;
    result = runDeviceTransfer(markerPayloads(10), fake.callbacks());
    QVERIFY(!result.success && result.transferAccepted && result.commitAccepted && result.resetWritten);
    QVERIFY(result.postResetReadReturned && !result.postResetReadCompleted);
    QCOMPARE(result.postResetReadResult, returned);
    QCOMPARE(result.firstError.kind, QStringLiteral("post_reset_read"));
    QCOMPARE(result.outcome, QStringLiteral("committed_reset_unconfirmed"));
    QCOMPARE(fake.commandReads.value(0x1a), 1);
    QCOMPARE(fake.releaseCount, 1);
    QCOMPARE(fake.closeCount, 1);
    QVERIFY(!result.screenDisplayAndTouchConfirmed);
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::cancellation_data
 * Description     : Exercise cancellation before open, after acquisition, before data/status, and immediately before commit.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::cancellation_data() //20261114-5 add
{
    QTest::addColumn<int>("scenario");
    QTest::newRow("before-open") << 0;
    QTest::newRow("after-open-before-claim") << 1;
    QTest::newRow("after-claim-before-first-command") << 2;
    QTest::newRow("after-segment-command-before-data") << 3;
    QTest::newRow("after-data-before-status") << 4;
    QTest::newRow("all-segments-before-commit") << 5;
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::cancellation
 * Description     : Stop at the requested pre-commit boundary, retain partial-write history, and never claim rollback.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::cancellation() //20261114-5 add
{
    QFETCH(int, scenario);
    FakeTransferIo fake;
    DeviceTransferOptions options;
    DeviceTransferResult result;

    fake.cancel = scenario == 0;
    fake.cancelOnOpen = scenario == 1;
    fake.cancelOnClaim = scenario == 2;
    if (scenario == 3) { fake.cancelOnReadCommand = 0x0a; }
    if (scenario == 4) { fake.cancelAfterPayload = 1; }
    if (scenario == 5) { fake.cancelOnReadCommand = 0x35; fake.cancelOnReadOccurrence = 13; }
    options.cancelRequested = [&fake]() { return fake.cancel; };
    result = runDeviceTransfer(markerPayloads(10), fake.callbacks(), options);
    QVERIFY(!result.success && result.cancelled && !result.commitCommandWritten && !result.commitAccepted);
    QCOMPARE(result.firstError.kind, QStringLiteral("cancelled"));
    QVERIFY(!fake.commands.contains(char(0x1b)) && !fake.commands.contains(char(0x1a)));
    if (scenario == 0)
    {
        QVERIFY(!result.opened && !result.interfaceClaimed);
        QCOMPARE(fake.openCount + fake.claimCount + fake.writeCount + fake.readCount + fake.releaseCount + fake.closeCount, 0);
    }
    else
    {
        QCOMPARE(fake.openCount, 1);
        QCOMPARE(fake.closeCount, 1);
        QCOMPARE(fake.releaseCount, scenario == 1 ? 0 : 1);
    }
    if (scenario <= 2) { QCOMPARE(fake.writeCount, 0); QCOMPARE(result.outcome, QStringLiteral("cancelled_before_first_command")); }
    if (scenario == 3) { QCOMPARE(result.writtenSegments.size(), 0); QCOMPARE(result.completedSteps, 0); }
    if (scenario == 4) { QCOMPARE(result.writtenSegments.size(), 1); QCOMPARE(result.completedSteps, 0); }
    if (scenario == 5) { QVERIFY(result.transferAccepted); QCOMPARE(result.completedSteps, 12); QCOMPARE(result.writtenSegments.size(), 10); }
    QVERIFY(!result.screenDisplayAndTouchConfirmed);
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::commitIgnoresCancellation
 * Description     : Finish commit ACK, reset, and post-reset read when cancellation arrives during the full commit write.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::commitIgnoresCancellation() //20261114-5 add
{
    FakeTransferIo fake;
    DeviceTransferOptions options;
    DeviceTransferResult result;

    fake.cancelOnCommand = 0x1b;
    options.cancelRequested = [&fake]() { return fake.cancel; };
    result = runDeviceTransfer(markerPayloads(10), fake.callbacks(), options);
    QVERIFY(result.success && !result.cancelled && fake.cancel);
    QVERIFY(result.commitAccepted && result.resetWritten && result.postResetReadCompleted);
    QCOMPARE(fake.commands.right(2).toHex(), QByteArray("1b1a"));
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::cleanupPreservesFirstFailure
 * Description     : Record cleanup failures without replacing an earlier payload-write failure or continuing the protocol.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::cleanupPreservesFirstFailure() //20261114-5 add
{
    FakeTransferIo fake;
    DeviceTransferResult result;

    fake.failedPayload = 1;
    fake.failedPayloadResult = 12;
    fake.releaseResult = -9;
    fake.closeResult = -10;
    result = runDeviceTransfer(markerPayloads(10), fake.callbacks());
    QCOMPARE(result.firstError.phase, QStringLiteral("segment-data"));
    QCOMPARE(result.firstError.kind, QStringLiteral("write_count"));
    QCOMPARE(result.firstError.returned, 12);
    QVERIFY(result.releaseAttempted && !result.interfaceReleased && result.closeAttempted && !result.closed);
    QCOMPARE(result.releaseResult, -9);
    QCOMPARE(result.closeResult, -10);
    QCOMPARE(result.events.at(result.events.size() - 2).phase, QStringLiteral("release"));
    QCOMPARE(result.events.last().phase, QStringLiteral("close"));
    QVERIFY(!result.success && !result.commitCommandWritten);
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::cleanupFailureAfterCommit
 * Description     : Preserve a completed device lifecycle separately from an unsuccessful interface cleanup.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::cleanupFailureAfterCommit() //20261114-5 add
{
    FakeTransferIo fake;
    DeviceTransferResult result;

    fake.releaseResult = -9;
    result = runDeviceTransfer(markerPayloads(10), fake.callbacks());
    QVERIFY(!result.success && result.commitAccepted && result.resetWritten && result.postResetReadCompleted);
    QVERIFY(result.closed && !result.interfaceReleased);
    QCOMPARE(result.firstError.kind, QStringLiteral("release"));
    QCOMPARE(result.outcome, QStringLiteral("completed_cleanup_failed"));
}

/******************************************************************************
 * Function Name   : DeviceTransferEngineTests::jsonKeepsEvidenceBounded
 * Description     : Export command bytes, payload digests, and exact milestones without embedding a large payload or screen claim.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceTransferEngineTests::jsonKeepsEvidenceBounded() //20261114-5 add
{
    FakeTransferIo fake;
    DeviceTransferResult result;
    QVector<QByteArray> payloads;
    QJsonObject object;
    QJsonObject eventObject;
    QJsonArray events;
    QJsonValue eventValue;
    int payloadEvents;
    int commandEvents;

    payloads = markerPayloads(10);
    payloads[0] = QByteArray(200000, 'P');
    result = runDeviceTransfer(payloads, fake.callbacks());
    QVERIFY(result.success);
    result.screenDisplayAndTouchConfirmed = true;
    object = deviceTransferResultToJson(result);
    QVERIFY(object.value(QStringLiteral("ok")).toBool());
    QVERIFY(!object.value(QStringLiteral("screenDisplayAndTouchConfirmed")).toBool());
    QCOMPARE(object.value(QStringLiteral("startOptionsHex")).toString(), QStringLiteral("000000"));
    QCOMPARE(object.value(QStringLiteral("timeoutMs")).toInt(), 10000);
    events = object.value(QStringLiteral("events")).toArray();
    payloadEvents = 0;
    commandEvents = 0;
    Q_FOREACH (eventValue, events)
    {
        eventObject = eventValue.toObject();
        if (eventObject.value(QStringLiteral("operation")).toString() == QStringLiteral("payload-write"))
        {
            ++payloadEvents;
            QVERIFY(eventObject.value(QStringLiteral("bytesHex")).toString().isEmpty());
            QCOMPARE(eventObject.value(QStringLiteral("sha256")).toString().size(), 64);
        }
        if (eventObject.value(QStringLiteral("operation")).toString() == QStringLiteral("command-write"))
        {
            ++commandEvents;
            QCOMPARE(eventObject.value(QStringLiteral("bytesHex")).toString().size(), 64);
        }
    }
    QCOMPARE(payloadEvents, 10);
    QCOMPARE(commandEvents, 30);
    QVERIFY(QJsonDocument(object).toJson(QJsonDocument::Compact).size() < 50000);
}

QTEST_APPLESS_MAIN(DeviceTransferEngineTests)
#include "device_transfer_engine_tests.moc"
