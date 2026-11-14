#include "device_download_session.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest>
#include <cstdio>
#include <cstdlib>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

/******************************************************************************
 * Function Name   : writeBytes
 * Description     : Flush deterministic fake-worker stdout through the QProcess pipe.
 * Input Parameters: bytes
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
static void writeBytes(const QByteArray &bytes) //20261114-5 add
{
    std::fwrite(bytes.constData(), 1, size_t(bytes.size()), stdout);
    std::fflush(stdout);
}

/******************************************************************************
 * Function Name   : writeRecord
 * Description     : Emit one compact fake-worker JSON record without any USB operation.
 * Input Parameters: object, newline
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
static void writeRecord(const QJsonObject &object, bool newline = true) //20261114-5 add
{
    QByteArray bytes;

    bytes = QJsonDocument(object).toJson(QJsonDocument::Compact);
    if (newline) { bytes.append('\n'); }
    writeBytes(bytes);
}

/******************************************************************************
 * Function Name   : progressRecord
 * Description     : Construct the same bounded event envelope as the product worker protocol.
 * Input Parameters: phase, operation, command, accepted, segment
 * Return Value    : Structured fake event.
 * Date            : 2026.11.14
 ******************************************************************************/
static QJsonObject progressRecord(const QString &phase, const QString &operation, int command,
                                  bool accepted, const QString &segment = QString()) //20261114-5 add
{
    QJsonObject record;
    QJsonObject event;

    event.insert(QStringLiteral("phase"), phase);
    event.insert(QStringLiteral("operation"), operation);
    event.insert(QStringLiteral("command"), command);
    event.insert(QStringLiteral("accepted"), accepted);
    event.insert(QStringLiteral("segment"), segment);
    event.insert(QStringLiteral("requested"), 32);
    event.insert(QStringLiteral("returned"), accepted ? 32 : -7);
    record.insert(QStringLiteral("kind"), QStringLiteral("transfer-event"));
    record.insert(QStringLiteral("event"), event);
    return record;
}

/******************************************************************************
 * Function Name   : finalRecord
 * Description     : Construct an explicit fake success with every required milestone and an intentionally untrusted screen flag.
 * Input Parameters: snapshotPath, cancelPath
 * Return Value    : Structured final record.
 * Date            : 2026.11.14
 ******************************************************************************/
static QJsonObject finalRecord(const QString &snapshotPath, const QString &cancelPath) //20261114-5 add
{
    QJsonObject result;

    result.insert(QStringLiteral("kind"), QStringLiteral("transfer-result"));
    result.insert(QStringLiteral("ok"), true);
    result.insert(QStringLiteral("message"), QStringLiteral("fake transport complete"));
    result.insert(QStringLiteral("compiled"), true);
    result.insert(QStringLiteral("transferAccepted"), true);
    result.insert(QStringLiteral("commitWriteAttempted"), true);
    result.insert(QStringLiteral("commitCommandWritten"), true);
    result.insert(QStringLiteral("commitAccepted"), true);
    result.insert(QStringLiteral("resetWritten"), true);
    result.insert(QStringLiteral("postResetReadCompleted"), true);
    result.insert(QStringLiteral("screenDisplayAndTouchConfirmed"), true);
    result.insert(QStringLiteral("snapshotExistsBeforeExit"), QFileInfo::exists(snapshotPath));
    result.insert(QStringLiteral("cancelExistsBeforeExit"), QFileInfo::exists(cancelPath));
    return result;
}

/******************************************************************************
 * Function Name   : snapshotFor
 * Description     : Encode a fake-worker scenario in the private snapshot without changing the worker command-line contract.
 * Input Parameters: mode, delay
 * Return Value    : Synthetic immutable snapshot bytes.
 * Date            : 2026.11.14
 ******************************************************************************/
static QByteArray snapshotFor(const QString &mode, int delay = 100) //20261114-5 add
{
    QJsonObject object;

    object.insert(QStringLiteral("mode"), mode);
    object.insert(QStringLiteral("delay"), delay);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

/******************************************************************************
 * Function Name   : runFakeWorker
 * Description     : Implement a private test-only subprocess mode with timers, pipes, and files rather than USB or Python.
 * Input Parameters: application, arguments
 * Return Value    : Fake worker exit code.
 * Date            : 2026.11.14
 ******************************************************************************/
static int runFakeWorker(QCoreApplication &application, const QStringList &arguments) //20261114-5 add
{
    QFile input;
    QJsonObject config;
    QString snapshotPath;
    QString cancelPath;
    QString mode;
    int cancelIndex;
    int delay;

    if (arguments.size() < 5) { return 20; }
    snapshotPath = arguments.at(2);
    cancelIndex = int(arguments.indexOf(QStringLiteral("--cancel-file")));
    if (cancelIndex < 0 || cancelIndex + 1 >= arguments.size()) { return 21; }
    cancelPath = arguments.at(cancelIndex + 1);
    input.setFileName(snapshotPath);
    if (!input.open(QIODevice::ReadOnly)) { return 22; }
    config = QJsonDocument::fromJson(input.readAll()).object();
    input.close();
    mode = config.value(QStringLiteral("mode")).toString();
    delay = config.value(QStringLiteral("delay")).toInt(100);
    if (mode == QStringLiteral("commit-cancel"))
    {
        writeRecord(progressRecord(QStringLiteral("commit"), QStringLiteral("command-write"), 0x1b, true));
    }
    else if (mode == QStringLiteral("compile-reject") || mode == QStringLiteral("cancel-before-start"))
    {
        writeRecord(progressRecord(QStringLiteral("preflight"), QStringLiteral("ready"), -1, true));
    }
    else
    {
        writeRecord(progressRecord(QStringLiteral("segment-data"), QStringLiteral("payload-write"),
                                    0x0a, mode != QStringLiteral("fault-then-ok"), QStringLiteral("SysPara.dat")));
    }
    if (mode == QStringLiteral("crash"))
    {
        writeRecord(progressRecord(QStringLiteral("commit"), QStringLiteral("reply-read"), 0x1b, true));
        QTimer::singleShot(delay, &application, []() {
#ifdef Q_OS_WIN
            // Deliberately terminate only this owned fake worker with an exception code; no crash dialog is opened.
            TerminateProcess(GetCurrentProcess(), 0xc0000005);
#else
            std::abort();
#endif
        });
    }
    else
    {
        QTimer::singleShot(delay, &application, [&application, config, snapshotPath, cancelPath, mode]() {
            QJsonObject result;
            QJsonObject failure;
            QJsonObject wrapper;
            QByteArray bytes;
            QFile witness;
            QString omitted;

            result = finalRecord(snapshotPath, cancelPath);
            if (mode == QStringLiteral("missing-final")) { application.exit(0); return; }
            if (mode == QStringLiteral("compile-reject"))
            {
                result.insert(QStringLiteral("ok"), false);
                result.insert(QStringLiteral("compiled"), false);
                result.insert(QStringLiteral("transferAccepted"), false);
                result.insert(QStringLiteral("commitWriteAttempted"), false);
                result.insert(QStringLiteral("commitCommandWritten"), false);
                result.insert(QStringLiteral("commitAccepted"), false);
                result.insert(QStringLiteral("resetWritten"), false);
                result.insert(QStringLiteral("postResetReadCompleted"), false);
                failure.insert(QStringLiteral("present"), true);
                failure.insert(QStringLiteral("kind"), QStringLiteral("compile_rejected"));
                failure.insert(QStringLiteral("phase"), QStringLiteral("compile"));
                failure.insert(QStringLiteral("message"), QStringLiteral("fake compile rejected"));
                result.insert(QStringLiteral("firstError"), failure);
                writeRecord(result);
                application.exit(14);
                return;
            }
            if (mode == QStringLiteral("cancel-before-start"))
            {
                result.insert(QStringLiteral("ok"), false);
                result.insert(QStringLiteral("opened"), false);
                result.insert(QStringLiteral("cancelled"), QFileInfo::exists(cancelPath));
                result.insert(QStringLiteral("transferAccepted"), false);
                result.insert(QStringLiteral("commitWriteAttempted"), false);
                result.insert(QStringLiteral("commitCommandWritten"), false);
                result.insert(QStringLiteral("commitAccepted"), false);
                result.insert(QStringLiteral("resetWritten"), false);
                result.insert(QStringLiteral("postResetReadCompleted"), false);
                failure.insert(QStringLiteral("present"), true);
                failure.insert(QStringLiteral("kind"), QStringLiteral("cancelled"));
                failure.insert(QStringLiteral("phase"), QStringLiteral("open"));
                result.insert(QStringLiteral("firstError"), failure);
                writeRecord(result);
                application.exit(14);
                return;
            }
            if (mode == QStringLiteral("ok-only"))
            {
                result = QJsonObject();
                result.insert(QStringLiteral("kind"), QStringLiteral("transfer-result"));
                result.insert(QStringLiteral("ok"), true);
            }
            omitted = config.value(QStringLiteral("omit")).toString();
            if (!omitted.isEmpty()) { result.remove(omitted); }
            if (mode == QStringLiteral("malformed")) { writeBytes(QByteArray("{broken}\n")); }
            if (mode == QStringLiteral("stdout-limit")) { writeBytes(QByteArray(1024 * 1024 + 1, 'X')); }
            if (!config.value(QStringLiteral("witness")).toString().isEmpty())
            {
                witness.setFileName(config.value(QStringLiteral("witness")).toString());
                if (witness.open(QIODevice::WriteOnly)) { witness.write(QFileInfo::exists(snapshotPath) ? "input alive" : "input missing"); witness.close(); }
            }
            if (mode == QStringLiteral("nested-final"))
            {
                result.remove(QStringLiteral("kind"));
                wrapper.insert(QStringLiteral("kind"), QStringLiteral("transfer-result"));
                wrapper.insert(QStringLiteral("result"), result);
                result = wrapper;
            }
            if (mode == QStringLiteral("split"))
            {
                bytes = QJsonDocument(result).toJson(QJsonDocument::Compact) + '\n';
                writeBytes(bytes.left(7));
                QTimer::singleShot(30, &application, [&application, bytes]() { writeBytes(bytes.mid(7)); application.exit(0); });
                return;
            }
            writeRecord(result, mode != QStringLiteral("no-newline"));
            if (mode == QStringLiteral("duplicate-final")) { writeRecord(result); }
            application.exit(mode == QStringLiteral("nonzero-success") ? 9 : 0);
        });
    }
    return application.exec();
}

//20261114-5 add: Tests use only QCoreApplication and this executable's fake child mode.
class DeviceDownloadSessionTests : public QObject
{
    Q_OBJECT
private slots:
    void successfulWorkerOwnsSnapshot();
    void processCrashRetainsMilestones();
    void failedStartCleansExactlyOnce();
    void duplicateStartIsRejected();
    void compileRejectionIsNotTransportSuccess();
    void cancellationKeepsWorkerAndInput();
    void framedJson_data();
    void framedJson();
    void incompleteSuccess_data();
    void incompleteSuccess();
    void invalidOutput_data();
    void invalidOutput();
    void cancellationAfterCommitIsRejected();
    void ownerDestructionKeepsActiveInput();
    void destructionDuringProgressKeepsWorker();
    void emptySnapshotDoesNotStart();
};

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::successfulWorkerOwnsSnapshot
 * Description     : Prove the snapshot exists throughout a live child and is deleted only after a fully accepted final result.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::successfulWorkerOwnsSnapshot() //20261114-5 add
{
    DeviceDownloadSession session;
    QSignalSpy finished(&session, &DeviceDownloadSession::finished);
    QSignalSpy progress(&session, &DeviceDownloadSession::progress);
    QString directory;
    QJsonObject result;
    QFile snapshot;
    QByteArray expected;

    expected = snapshotFor(QStringLiteral("success"), 200);
    QVERIFY(session.start(expected));
    directory = session.tempDirectory();
    QVERIFY(session.isRunning() && QFileInfo::exists(directory));
    snapshot.setFileName(QDir(directory).filePath(QStringLiteral("LiteDownload.ksp")));
    QVERIFY(snapshot.open(QIODevice::ReadOnly));
    QCOMPARE(snapshot.readAll(), expected);
    snapshot.close();
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 5000);
    result = qvariant_cast<QJsonObject>(finished.first().at(1));
    QVERIFY(finished.first().at(0).toBool());
    QVERIFY(result.value(QStringLiteral("snapshotExistsBeforeExit")).toBool());
    QVERIFY(result.value(QStringLiteral("temporaryCleanupComplete")).toBool());
    QVERIFY(!result.value(QStringLiteral("screenDisplayAndTouchConfirmed")).toBool());
    QVERIFY(!session.isRunning() && session.tempDirectory().isEmpty());
    QVERIFY(!QFileInfo::exists(directory));
    QVERIFY(progress.size() >= 1);
    QVERIFY(!session.requestCancel());
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::processCrashRetainsMilestones
 * Description     : Retain a known commit ACK and written segment after an actual fake-worker crash without inventing reset success.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::processCrashRetainsMilestones() //20261114-5 add
{
    DeviceDownloadSession session;
    QSignalSpy finished(&session, &DeviceDownloadSession::finished);
    QString directory;
    QJsonObject result;

    QVERIFY(session.start(snapshotFor(QStringLiteral("crash"))));
    directory = session.tempDirectory();
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 5000);
    result = session.result();
    QVERIFY(!finished.first().at(0).toBool());
    QVERIFY(!result.value(QStringLiteral("normalExit")).toBool());
    QVERIFY(result.value(QStringLiteral("commitAccepted")).toBool());
    QVERIFY(!result.value(QStringLiteral("resetWritten")).toBool());
    QVERIFY(!result.value(QStringLiteral("finalResultReceived")).toBool());
    QCOMPARE(result.value(QStringLiteral("writtenSegments")).toArray().size(), 1);
    QVERIFY(result.value(QStringLiteral("firstError")).toObject().value(QStringLiteral("present")).toBool());
    QVERIFY(!QFileInfo::exists(directory));
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::failedStartCleansExactlyOnce
 * Description     : Finalize FailedToStart once and remove its already saved private input.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::failedStartCleansExactlyOnce() //20261114-5 add
{
    DeviceDownloadSession session;
    QSignalSpy finished(&session, &DeviceDownloadSession::finished);
    QTemporaryDir missingRoot;
    QString directory;

    QVERIFY(session.start(snapshotFor(QStringLiteral("success")), missingRoot.filePath(QStringLiteral("missing-worker.exe"))));
    directory = session.tempDirectory();
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 5000);
    QVERIFY(!finished.first().at(0).toBool());
    QCOMPARE(session.result().value(QStringLiteral("sessionError")).toObject().value(QStringLiteral("kind")).toString(), QStringLiteral("failed_to_start"));
    QVERIFY(!session.isRunning());
    QVERIFY(directory.isEmpty() || !QFileInfo::exists(directory));
    QTest::qWait(30);
    QCOMPARE(finished.size(), 1);
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::duplicateStartIsRejected
 * Description     : Keep the active worker and its input when another start request arrives before exit.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::duplicateStartIsRejected() //20261114-5 add
{
    DeviceDownloadSession session;
    QSignalSpy finished(&session, &DeviceDownloadSession::finished);
    QString directory;

    QVERIFY(session.start(snapshotFor(QStringLiteral("success"), 200)));
    directory = session.tempDirectory();
    QVERIFY(!session.start(snapshotFor(QStringLiteral("compile-reject"))));
    QCOMPARE(session.tempDirectory(), directory);
    QVERIFY(session.isRunning() && !session.lastError().isEmpty());
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 5000);
    QVERIFY(finished.first().at(0).toBool());
    QVERIFY(!QFileInfo::exists(directory));
    QVERIFY(session.start(snapshotFor(QStringLiteral("success"))));
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 2, 5000);
    QVERIFY(finished.last().at(0).toBool());
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::compileRejectionIsNotTransportSuccess
 * Description     : Preserve the child's compile failure instead of treating normal process exit as a completed transfer.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::compileRejectionIsNotTransportSuccess() //20261114-5 add
{
    DeviceDownloadSession session;
    QSignalSpy finished(&session, &DeviceDownloadSession::finished);

    QVERIFY(session.start(snapshotFor(QStringLiteral("compile-reject"))));
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 5000);
    QVERIFY(!finished.first().at(0).toBool());
    QVERIFY(!session.result().value(QStringLiteral("compiled")).toBool());
    QCOMPARE(session.result().value(QStringLiteral("firstError")).toObject().value(QStringLiteral("kind")).toString(), QStringLiteral("compile_rejected"));
    QVERIFY(!session.result().value(QStringLiteral("transferAccepted")).toBool());
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::cancellationKeepsWorkerAndInput
 * Description     : Write one unique cancellation marker before start without killing the child or deleting its input early.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::cancellationKeepsWorkerAndInput() //20261114-5 add
{
    DeviceDownloadSession session;
    QSignalSpy finished(&session, &DeviceDownloadSession::finished);
    QString directory;

    QVERIFY(session.start(snapshotFor(QStringLiteral("cancel-before-start"), 200)));
    directory = session.tempDirectory();
    QVERIFY(session.requestCancel());
    QVERIFY(!session.requestCancel());
    QVERIFY(session.isRunning() && QFileInfo::exists(directory));
    QVERIFY(QFileInfo::exists(QDir(directory).filePath(QStringLiteral("cancel.request"))));
    QVERIFY(QFileInfo::exists(QDir(directory).filePath(QStringLiteral("LiteDownload.ksp"))));
    QCOMPARE(finished.size(), 0);
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 5000);
    QVERIFY(!finished.first().at(0).toBool());
    QVERIFY(session.result().value(QStringLiteral("cancelled")).toBool());
    QVERIFY(session.result().value(QStringLiteral("cancelExistsBeforeExit")).toBool());
    QVERIFY(session.result().value(QStringLiteral("snapshotExistsBeforeExit")).toBool());
    QVERIFY(!QFileInfo::exists(directory));
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::framedJson_data
 * Description     : Cover split pipe records, an unterminated final line, and the documented nested final envelope.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::framedJson_data() //20261114-5 add
{
    QTest::addColumn<QString>("mode");
    QTest::newRow("split") << QStringLiteral("split");
    QTest::newRow("no-newline") << QStringLiteral("no-newline");
    QTest::newRow("nested-final") << QStringLiteral("nested-final");
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::framedJson
 * Description     : Parse complete JSON records across arbitrary readyRead boundaries and the final exit drain.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::framedJson() //20261114-5 add
{
    QFETCH(QString, mode);
    DeviceDownloadSession session;
    QSignalSpy finished(&session, &DeviceDownloadSession::finished);

    QVERIFY(session.start(snapshotFor(mode)));
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 5000);
    QVERIFY2(finished.first().at(0).toBool(), qPrintable(session.result().value(QStringLiteral("message")).toString()));
    QVERIFY(session.result().value(QStringLiteral("finalResultReceived")).toBool());
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::incompleteSuccess_data
 * Description     : Omit each independently required success milestone or supply only an ok flag.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::incompleteSuccess_data() //20261114-5 add
{
    QTest::addColumn<QString>("omitted");
    QTest::newRow("ok-only") << QStringLiteral("all");
    QTest::newRow("compiled") << QStringLiteral("compiled");
    QTest::newRow("transferAccepted") << QStringLiteral("transferAccepted");
    QTest::newRow("commitAccepted") << QStringLiteral("commitAccepted");
    QTest::newRow("resetWritten") << QStringLiteral("resetWritten");
    QTest::newRow("postResetReadCompleted") << QStringLiteral("postResetReadCompleted");
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::incompleteSuccess
 * Description     : Reject a normal exit0/ok final when any required compilation or lifecycle confirmation is absent.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::incompleteSuccess() //20261114-5 add
{
    QFETCH(QString, omitted);
    DeviceDownloadSession session;
    QSignalSpy finished(&session, &DeviceDownloadSession::finished);
    QJsonObject config;

    config.insert(QStringLiteral("mode"), omitted == QStringLiteral("all") ? QStringLiteral("ok-only") : QStringLiteral("success"));
    if (omitted != QStringLiteral("all")) { config.insert(QStringLiteral("omit"), omitted); }
    QVERIFY(session.start(QJsonDocument(config).toJson(QJsonDocument::Compact)));
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 5000);
    QVERIFY(!finished.first().at(0).toBool());
    QCOMPARE(session.result().value(QStringLiteral("sessionError")).toObject().value(QStringLiteral("kind")).toString(), QStringLiteral("incomplete_final"));
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::invalidOutput_data
 * Description     : Cover absent/duplicate/malformed/oversized finals and a nonzero exit contradicting success.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::invalidOutput_data() //20261114-5 add
{
    QTest::addColumn<QString>("mode");
    QTest::addColumn<QString>("errorKind");
    QTest::newRow("missing-final") << QStringLiteral("missing-final") << QStringLiteral("missing_final");
    QTest::newRow("duplicate-final") << QStringLiteral("duplicate-final") << QStringLiteral("duplicate_or_invalid_final");
    QTest::newRow("malformed") << QStringLiteral("malformed") << QStringLiteral("invalid_json");
    QTest::newRow("stdout-limit") << QStringLiteral("stdout-limit") << QStringLiteral("stdout_limit");
    QTest::newRow("nonzero-success") << QStringLiteral("nonzero-success") << QStringLiteral("nonzero_exit");
    QTest::newRow("fault-then-ok") << QStringLiteral("fault-then-ok") << QStringLiteral("contradictory_result");
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::invalidOutput
 * Description     : Keep the process alive until exit but reject its incomplete or contradictory output exactly once.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::invalidOutput() //20261114-5 add
{
    QFETCH(QString, mode);
    QFETCH(QString, errorKind);
    DeviceDownloadSession session;
    QSignalSpy finished(&session, &DeviceDownloadSession::finished);
    QString directory;

    QVERIFY(session.start(snapshotFor(mode)));
    directory = session.tempDirectory();
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 5000);
    QVERIFY(!finished.first().at(0).toBool());
    QCOMPARE(session.result().value(QStringLiteral("sessionError")).toObject().value(QStringLiteral("kind")).toString(), errorKind);
    QVERIFY(!session.isRunning() && !QFileInfo::exists(directory));
    QVERIFY(!session.result().value(QStringLiteral("screenDisplayAndTouchConfirmed")).toBool());
    QTest::qWait(20);
    QCOMPARE(finished.size(), 1);
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::cancellationAfterCommitIsRejected
 * Description     : Disable marker creation once the worker reports a commit command and keep ownership through exit.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::cancellationAfterCommitIsRejected() //20261114-5 add
{
    DeviceDownloadSession session;
    QSignalSpy finished(&session, &DeviceDownloadSession::finished);
    QSignalSpy progress(&session, &DeviceDownloadSession::progress);
    QString directory;

    QVERIFY(session.start(snapshotFor(QStringLiteral("commit-cancel"), 300)));
    directory = session.tempDirectory();
    QTRY_VERIFY_WITH_TIMEOUT(progress.size() >= 1, 5000);
    QVERIFY(session.isRunning());
    QVERIFY(!session.requestCancel());
    QVERIFY(!QFileInfo::exists(QDir(directory).filePath(QStringLiteral("cancel.request"))));
    QVERIFY(QFileInfo::exists(QDir(directory).filePath(QStringLiteral("LiteDownload.ksp"))));
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 5000);
    QVERIFY(finished.first().at(0).toBool());
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::ownerDestructionKeepsActiveInput
 * Description     : Preserve the child and input until actual exit even if the QObject owner is destroyed early.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::ownerDestructionKeepsActiveInput() //20261114-5 add
{
    DeviceDownloadSession *session;
    QTemporaryDir witnessRoot;
    QJsonObject config;
    QString directory;
    QString witnessPath;
    QFile witness;

    session = new DeviceDownloadSession;
    witnessPath = witnessRoot.filePath(QStringLiteral("worker-exited.txt"));
    config.insert(QStringLiteral("mode"), QStringLiteral("success"));
    config.insert(QStringLiteral("delay"), 200);
    config.insert(QStringLiteral("witness"), witnessPath);
    QVERIFY(session->start(QJsonDocument(config).toJson(QJsonDocument::Compact)));
    directory = session->tempDirectory();
    delete session;
    QVERIFY(QFileInfo::exists(QDir(directory).filePath(QStringLiteral("LiteDownload.ksp"))));
    QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(witnessPath), 5000);
    witness.setFileName(witnessPath);
    QVERIFY(witness.open(QIODevice::ReadOnly));
    QCOMPARE(witness.readAll(), QByteArray("input alive"));
    witness.close();
    QTRY_VERIFY_WITH_TIMEOUT(!QFileInfo::exists(directory), 5000);
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::emptySnapshotDoesNotStart
 * Description     : Reject an empty input without any process, temporary directory, or completion signal.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::emptySnapshotDoesNotStart() //20261114-5 add
{
    DeviceDownloadSession session;
    QSignalSpy finished(&session, &DeviceDownloadSession::finished);

    QVERIFY(!session.start(QByteArray()));
    QVERIFY(!session.isRunning() && session.tempDirectory().isEmpty());
    QVERIFY(!session.lastError().isEmpty());
    QCOMPARE(finished.size(), 0);
}

/******************************************************************************
 * Function Name   : DeviceDownloadSessionTests::destructionDuringProgressKeepsWorker
 * Description     : Guard parser lifetime when a synchronous progress observer destroys its owner during stdout dispatch.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSessionTests::destructionDuringProgressKeepsWorker() //20261114-5 add
{
    DeviceDownloadSession *session;
    QTemporaryDir witnessRoot;
    QJsonObject config;
    QString directory;
    QString witnessPath;
    QFile witness;

    session = new DeviceDownloadSession;
    witnessPath = witnessRoot.filePath(QStringLiteral("worker-progress-exit.txt"));
    config.insert(QStringLiteral("mode"), QStringLiteral("success"));
    config.insert(QStringLiteral("delay"), 200);
    config.insert(QStringLiteral("witness"), witnessPath);
    QObject::connect(session, &DeviceDownloadSession::progress, QCoreApplication::instance(),
                      [&session](const QJsonObject &event) { Q_UNUSED(event); delete session; session = nullptr; });
    QVERIFY(session->start(QJsonDocument(config).toJson(QJsonDocument::Compact)));
    directory = session->tempDirectory();
    QTRY_VERIFY_WITH_TIMEOUT(session == nullptr, 5000);
    QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(witnessPath), 5000);
    witness.setFileName(witnessPath);
    QVERIFY(witness.open(QIODevice::ReadOnly));
    QCOMPARE(witness.readAll(), QByteArray("input alive"));
    witness.close();
    QTRY_VERIFY_WITH_TIMEOUT(!QFileInfo::exists(directory), 5000);
}

/******************************************************************************
 * Function Name   : main
 * Description     : Dispatch this test executable's private fake-worker mode before running app-less owner tests.
 * Input Parameters: argc, argv
 * Return Value    : Fake worker or QtTest exit status.
 * Date            : 2026.11.14
 ******************************************************************************/
int main(int argc, char **argv) //20261114-5 add
{
    QCoreApplication application(argc, argv);
    DeviceDownloadSessionTests tests;

    if (application.arguments().contains(QStringLiteral("--download-project")))
    {
        return runFakeWorker(application, application.arguments());
    }
    return QTest::qExec(&tests, argc, argv);
}

#include "device_download_session_tests.moc"
