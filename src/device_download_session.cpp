#include "device_download_session.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QProcess>
#include <QPointer>
#include <QSaveFile>
#include <QTemporaryDir>
#include <functional>
#include <memory>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace
{
constexpr qint64 kMaximumStdoutBytes = 1024 * 1024; //20261114-5 add: Bound both complete lines and an unterminated JSON record.

/******************************************************************************
 * Function Name   : retainDetachedWorker
 * Description     : Preserve an active worker and its input after owner destruction without turning destruction into cancellation.
 * Input Parameters: process, temporary
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void retainDetachedWorker(QProcess *process, QTemporaryDir *temporary) //20261114-5 add
{
    std::shared_ptr<bool> cleaned = std::make_shared<bool>(false);
    std::function<void()> cleanup;

    process->setParent(nullptr);
    cleanup = [process, temporary, cleaned]() {
        if (*cleaned) { return; }
        *cleaned = true;
        delete temporary;
        process->deleteLater();
    };
    QObject::connect(process, &QProcess::readyReadStandardOutput, process, [process]() { process->readAllStandardOutput(); });
    QObject::connect(process, &QProcess::readyReadStandardError, process, [process]() { process->readAllStandardError(); });
    QObject::connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), process,
                     [cleanup](int exitCode, QProcess::ExitStatus status) { Q_UNUSED(exitCode); Q_UNUSED(status); cleanup(); });
    QObject::connect(process, &QProcess::errorOccurred, process, [cleanup](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) { cleanup(); }
    });
    if (process->state() == QProcess::NotRunning) { cleanup(); }
}

/******************************************************************************
 * Function Name   : appendUniqueSegment
 * Description     : Retain segment milestones without duplicating a progress event or overwriting earlier evidence.
 * Input Parameters: result, key, segment
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void appendUniqueSegment(QJsonObject *result, const QString &key, const QString &segment) //20261114-5 add
{
    QJsonArray values;
    QJsonValue value;

    if (segment.isEmpty()) { return; }
    values = result->value(key).toArray();
    Q_FOREACH (value, values)
    {
        if (value.toString() == segment) { return; }
    }
    values.append(segment);
    result->insert(key, values);
}
}

/******************************************************************************
 * Function Name   : DeviceDownloadSession::DeviceDownloadSession
 * Description     : Initialize an idle worker owner without creating files or starting a process.
 * Input Parameters: parent
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
DeviceDownloadSession::DeviceDownloadSession(QObject *parent) : QObject(parent) //20261114-5 add
{
}

/******************************************************************************
 * Function Name   : DeviceDownloadSession::~DeviceDownloadSession
 * Description     : Keep a still-running worker's immutable files until exit instead of killing it or deleting its input.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
DeviceDownloadSession::~DeviceDownloadSession() //20261114-5 add
{
    if (m_process != nullptr && m_running)
    {
        QObject::disconnect(m_process, nullptr, this, nullptr);
        retainDetachedWorker(m_process, m_temp);
        m_process = nullptr;
        m_temp = nullptr;
    }
    delete m_temp;
}

/******************************************************************************
 * Function Name   : DeviceDownloadSession::start
 * Description     : Atomically save a private snapshot before launching one hidden worker with its unique cancellation path.
 * Input Parameters: snapshot, workerProgram
 * Return Value    : True when an owned asynchronous start was requested; false for synchronous validation or file errors.
 * Date            : 2026.11.14
 ******************************************************************************/
bool DeviceDownloadSession::start(const QByteArray &snapshot, const QString &workerProgram) //20261114-5 add
{
    QSaveFile file;
    QProcess *process;
    QString program;
    QString snapshotPath;

    if (m_running)
    {
        m_lastError = QStringLiteral("当前下载任务仍在进行。");
        return false;
    }
    if (snapshot.isEmpty())
    {
        m_lastError = QStringLiteral("下载工程快照为空，未启动进程。");
        return false;
    }
    m_temp = new QTemporaryDir(QDir::tempPath() + QStringLiteral("/KinSealLiteDownload-XXXXXX"));
    if (!m_temp->isValid())
    {
        delete m_temp;
        m_temp = nullptr;
        m_lastError = QStringLiteral("无法建立私有下载临时目录。");
        return false;
    }
    snapshotPath = m_temp->filePath(QStringLiteral("LiteDownload.ksp"));
    file.setFileName(snapshotPath);
    if (!file.open(QIODevice::WriteOnly) || file.write(snapshot) != snapshot.size() || !file.commit())
    {
        m_lastError = QStringLiteral("无法原子保存私有下载快照：%1").arg(file.errorString());
        // Close an uncommitted temporary file before removing its directory, including disk-full short writes on Windows.
        if (file.isOpen()) { file.cancelWriting(); file.commit(); }
        delete m_temp;
        m_temp = nullptr;
        return false;
    }
    m_cancelPath = m_temp->filePath(QStringLiteral("cancel.request"));
    m_lastError.clear();
    m_lineBuffer.clear();
    m_stderrTail.clear();
    m_stdoutBytes = 0;
    m_cancelRequested = false;
    m_commitStarted = false;
    m_finalSeen = false;
    m_streamInvalid = false;
    m_outputLimitExceeded = false;
    m_result = QJsonObject();
    m_finalResult = QJsonObject();
    m_sessionError = QJsonObject();
    m_result.insert(QStringLiteral("snapshotSha256"), QString::fromLatin1(QCryptographicHash::hash(snapshot, QCryptographicHash::Sha256).toHex()));
    m_result.insert(QStringLiteral("snapshotBytes"), qint64(snapshot.size()));
    m_result.insert(QStringLiteral("temporaryDirectory"), m_temp->path());
    m_result.insert(QStringLiteral("screenDisplayAndTouchConfirmed"), false);
    process = new QProcess(this);
    program = workerProgram.isEmpty() ? QCoreApplication::applicationFilePath() : workerProgram;
    process->setProgram(program);
    process->setArguments({QStringLiteral("--download-project"), snapshotPath,
                           QStringLiteral("--cancel-file"), m_cancelPath});
    process->setProcessChannelMode(QProcess::SeparateChannels);
#ifdef Q_OS_WIN
    process->setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *arguments) {
        arguments->flags |= CREATE_NO_WINDOW;
    });
#endif
    m_process = process;
    m_running = true;
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() { consumeStdout(process, false); });
    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        if (m_process == process) { m_stderrTail = (m_stderrTail + process->readAllStandardError()).right(4096); }
    });
    connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError error) {
        if (m_process != process || !m_running) { return; }
        if (error == QProcess::FailedToStart)
        {
            rememberSessionError(QStringLiteral("failed_to_start"), QStringLiteral("无法启动下载进程：%1").arg(process->errorString()));
            finishProcess(process, false, -1);
        }
        else if (error == QProcess::Crashed)
        {
            rememberSessionError(QStringLiteral("worker_crashed"), QStringLiteral("下载进程异常终止；屏内状态未确认。"));
        }
        else
        {
            rememberSessionError(QStringLiteral("process_error"), process->errorString());
        }
    });
    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
             [this, process](int exitCode, QProcess::ExitStatus status) {
                 finishProcess(process, status == QProcess::NormalExit, exitCode);
             });
    process->start();
    return true;
}

/******************************************************************************
 * Function Name   : DeviceDownloadSession::isRunning
 * Description     : Cover starting, running, and finalization ownership with one state.
 * Input Parameters: None
 * Return Value    : True until the worker exits and private files are cleaned.
 * Date            : 2026.11.14
 ******************************************************************************/
bool DeviceDownloadSession::isRunning() const //20261114-5 add
{
    return m_running;
}

/******************************************************************************
 * Function Name   : DeviceDownloadSession::requestCancel
 * Description     : Atomically request cancellation before commit while preserving the live process and its private input.
 * Input Parameters: None
 * Return Value    : True only when this request wrote the unique cancellation marker.
 * Date            : 2026.11.14
 ******************************************************************************/
bool DeviceDownloadSession::requestCancel() //20261114-5 add
{
    QSaveFile file;
    const QByteArray marker("cancel\n");

    if (!m_running || m_commitStarted || m_cancelRequested || m_temp == nullptr)
    {
        return false;
    }
    file.setFileName(m_cancelPath);
    if (!file.open(QIODevice::WriteOnly) || file.write(marker) != marker.size() || !file.commit())
    {
        m_lastError = QStringLiteral("无法保存取消请求：%1").arg(file.errorString());
        return false;
    }
    m_cancelRequested = true;
    m_result.insert(QStringLiteral("cancelRequested"), true);
    return true;
}

/******************************************************************************
 * Function Name   : DeviceDownloadSession::tempDirectory
 * Description     : Expose the current private path for read-only ownership and lifetime verification.
 * Input Parameters: None
 * Return Value    : Active private directory, or an empty string after cleanup.
 * Date            : 2026.11.14
 ******************************************************************************/
QString DeviceDownloadSession::tempDirectory() const //20261114-5 add
{
    return m_temp == nullptr ? QString() : m_temp->path();
}

/******************************************************************************
 * Function Name   : DeviceDownloadSession::result
 * Description     : Return a detached value copy of retained final or partial milestones.
 * Input Parameters: None
 * Return Value    : Current structured result.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject DeviceDownloadSession::result() const //20261114-5 add
{
    return m_result;
}

/******************************************************************************
 * Function Name   : DeviceDownloadSession::lastError
 * Description     : Expose synchronous start/cancel failures without replacing an active worker's protocol evidence.
 * Input Parameters: None
 * Return Value    : Last owner error message.
 * Date            : 2026.11.14
 ******************************************************************************/
QString DeviceDownloadSession::lastError() const //20261114-5 add
{
    return m_lastError;
}

/******************************************************************************
 * Function Name   : DeviceDownloadSession::rememberSessionError
 * Description     : Preserve the first owner/stream failure while retaining an earlier protocol failure and known milestones.
 * Input Parameters: kind, message
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSession::rememberSessionError(const QString &kind, const QString &message) //20261114-5 add
{
    QJsonObject failure;

    if (!m_sessionError.isEmpty()) { return; }
    m_lastError = message;
    m_sessionError.insert(QStringLiteral("kind"), kind);
    m_sessionError.insert(QStringLiteral("message"), message);
    m_result.insert(QStringLiteral("sessionError"), m_sessionError);
    failure = m_result.value(QStringLiteral("firstError")).toObject();
    if (!failure.value(QStringLiteral("present")).toBool())
    {
        failure.insert(QStringLiteral("present"), true);
        failure.insert(QStringLiteral("kind"), kind);
        failure.insert(QStringLiteral("phase"), QStringLiteral("worker-session"));
        failure.insert(QStringLiteral("message"), message);
        m_result.insert(QStringLiteral("firstError"), failure);
    }
}

/******************************************************************************
 * Function Name   : DeviceDownloadSession::consumeStdout
 * Description     : Drain bounded stdout and parse records only after a complete line or a final process-exit drain.
 * Input Parameters: process, finalDrain
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSession::consumeStdout(QProcess *process, bool finalDrain) //20261114-5 add
{
    QByteArray bytes;
    QByteArray line;
    int newline;
    QPointer<DeviceDownloadSession> lifetime(this);

    if (m_process != process || !m_running) { return; }
    bytes = process->readAllStandardOutput();
    m_stdoutBytes += bytes.size();
    if (m_stdoutBytes > kMaximumStdoutBytes)
    {
        m_outputLimitExceeded = true;
        m_streamInvalid = true;
        m_lineBuffer.clear();
        rememberSessionError(QStringLiteral("stdout_limit"), QStringLiteral("下载进程输出超过1MiB；等待进程退出，未自动中断或重试。"));
    }
    if (m_outputLimitExceeded) { return; }
    m_lineBuffer.append(bytes);
    while ((newline = int(m_lineBuffer.indexOf('\n'))) >= 0)
    {
        line = m_lineBuffer.left(newline);
        m_lineBuffer.remove(0, newline + 1);
        consumeLine(line);
        if (lifetime.isNull()) { return; }
    }
    if (finalDrain && !m_lineBuffer.trimmed().isEmpty())
    {
        line = m_lineBuffer;
        m_lineBuffer.clear();
        consumeLine(line);
    }
}

/******************************************************************************
 * Function Name   : DeviceDownloadSession::consumeLine
 * Description     : Accept typed progress/final JSON records and reject ambiguous or malformed worker output.
 * Input Parameters: line
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSession::consumeLine(const QByteArray &line) //20261114-5 add
{
    QJsonParseError error;
    QJsonDocument document;
    QJsonObject object;
    QJsonObject payload;
    QString kind;

    if (line.trimmed().isEmpty()) { return; }
    document = QJsonDocument::fromJson(line, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
    {
        m_streamInvalid = true;
        rememberSessionError(QStringLiteral("invalid_json"), QStringLiteral("下载进程输出不是有效JSON记录；屏内状态未确认。"));
        return;
    }
    object = document.object();
    kind = object.value(QStringLiteral("kind")).toString();
    if (kind == QStringLiteral("transfer-event"))
    {
        if (m_finalSeen || (object.contains(QStringLiteral("event")) && !object.value(QStringLiteral("event")).isObject()))
        {
            m_streamInvalid = true;
            rememberSessionError(QStringLiteral("invalid_event"), QStringLiteral("下载进程事件结构或顺序不完整。"));
            return;
        }
        payload = object.contains(QStringLiteral("event")) ? object.value(QStringLiteral("event")).toObject() : object;
        payload.remove(QStringLiteral("kind"));
        if (!payload.value(QStringLiteral("accepted")).isBool() ||
            payload.value(QStringLiteral("phase")).toString().isEmpty() ||
            payload.value(QStringLiteral("operation")).toString().isEmpty())
        {
            m_streamInvalid = true;
            rememberSessionError(QStringLiteral("invalid_event"), QStringLiteral("下载进程事件缺少阶段、操作或确认状态。"));
            return;
        }
        mergeProgress(payload);
        emit progress(payload);
    }
    else if (kind == QStringLiteral("transfer-result"))
    {
        if (m_finalSeen || (object.contains(QStringLiteral("result")) && !object.value(QStringLiteral("result")).isObject()))
        {
            m_streamInvalid = true;
            rememberSessionError(QStringLiteral("duplicate_or_invalid_final"), QStringLiteral("下载进程最终结果重复或结构不完整。"));
            return;
        }
        payload = object.contains(QStringLiteral("result")) ? object.value(QStringLiteral("result")).toObject() : object;
        payload.remove(QStringLiteral("kind"));
        m_finalSeen = true;
        m_finalResult = payload;
        mergeFinal(payload);
    }
    else
    {
        m_streamInvalid = true;
        rememberSessionError(QStringLiteral("unknown_record"), QStringLiteral("下载进程输出了未识别类型的记录。"));
    }
}

/******************************************************************************
 * Function Name   : DeviceDownloadSession::mergeProgress
 * Description     : Retain explicit event milestones, segment history, and the first failed operation before final output exists.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSession::mergeProgress(const QJsonObject &event) //20261114-5 add
{
    QJsonObject failure;
    QString phase;
    QString operation;
    QString segment;
    int command;
    bool accepted;

    phase = event.value(QStringLiteral("phase")).toString();
    operation = event.value(QStringLiteral("operation")).toString();
    segment = event.value(QStringLiteral("segment")).toString();
    command = event.value(QStringLiteral("command")).toInt(-1);
    accepted = event.value(QStringLiteral("accepted")).toBool();
    if (command == 0x1b && phase == QStringLiteral("commit"))
    {
        m_commitStarted = true;
        m_result.insert(QStringLiteral("commitWriteAttempted"), true);
    }
    if (operation == QStringLiteral("command-write") && command == 0x1b && accepted)
    {
        m_result.insert(QStringLiteral("commitCommandWritten"), true);
    }
    if (operation == QStringLiteral("reply-read") && accepted)
    {
        if (command == 0x34) { m_result.insert(QStringLiteral("startAccepted"), true); }
        if (command == 0x1b) { m_result.insert(QStringLiteral("commitAccepted"), true); }
        if (phase == QStringLiteral("segment-status")) { appendUniqueSegment(&m_result, QStringLiteral("acknowledgedSegments"), segment); }
    }
    if (operation == QStringLiteral("command-write") && command == 0x1a && accepted)
    {
        m_result.insert(QStringLiteral("resetWritten"), true);
    }
    if (operation == QStringLiteral("reset-read"))
    {
        m_result.insert(QStringLiteral("postResetReadReturned"), true);
        m_result.insert(QStringLiteral("postResetReadResult"), event.value(QStringLiteral("returned")));
        if (accepted) { m_result.insert(QStringLiteral("postResetReadCompleted"), true); }
    }
    if (operation == QStringLiteral("payload-write") && accepted)
    {
        appendUniqueSegment(&m_result, QStringLiteral("writtenSegments"), segment);
    }
    if (!accepted && !m_result.value(QStringLiteral("firstError")).toObject().value(QStringLiteral("present")).toBool())
    {
        failure = event;
        failure.insert(QStringLiteral("present"), true);
        failure.insert(QStringLiteral("kind"), operation == QStringLiteral("cancel") ? QStringLiteral("cancelled") : QStringLiteral("operation_failed"));
        m_result.insert(QStringLiteral("firstError"), failure);
    }
    m_result.insert(QStringLiteral("screenDisplayAndTouchConfirmed"), false);
}

/******************************************************************************
 * Function Name   : DeviceDownloadSession::mergeFinal
 * Description     : Preserve known true milestones and segment history while adding the worker's structured final detail.
 * Input Parameters: result
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSession::mergeFinal(const QJsonObject &result) //20261114-5 add
{
    QJsonObject::const_iterator iterator;
    QJsonObject previousError;
    QJsonObject finalError;
    QJsonArray values;
    QJsonValue value;
    QString key;

    previousError = m_result.value(QStringLiteral("firstError")).toObject();
    for (iterator = result.constBegin(); iterator != result.constEnd(); ++iterator)
    {
        key = iterator.key();
        if (key == QStringLiteral("screenDisplayAndTouchConfirmed") || key == QStringLiteral("firstError") ||
            key == QStringLiteral("sessionError") || key == QStringLiteral("snapshotSha256") ||
            key == QStringLiteral("snapshotBytes") || key == QStringLiteral("temporaryDirectory")) { continue; }
        if (key == QStringLiteral("writtenSegments") || key == QStringLiteral("acknowledgedSegments"))
        {
            values = iterator.value().toArray();
            Q_FOREACH (value, values) { appendUniqueSegment(&m_result, key, value.toString()); }
            continue;
        }
        if (iterator.value().isBool() && m_result.value(key).isBool() && m_result.value(key).toBool()) { continue; }
        m_result.insert(key, iterator.value());
    }
    finalError = result.value(QStringLiteral("firstError")).toObject();
    if (finalError.value(QStringLiteral("present")).toBool() &&
        (!previousError.value(QStringLiteral("present")).toBool() ||
         (previousError.value(QStringLiteral("phase")) == finalError.value(QStringLiteral("phase")) &&
          previousError.value(QStringLiteral("command")) == finalError.value(QStringLiteral("command")))))
    {
        m_result.insert(QStringLiteral("firstError"), finalError);
    }
    m_commitStarted = m_commitStarted || result.value(QStringLiteral("commitWriteAttempted")).toBool() ||
                      result.value(QStringLiteral("commitCommandWritten")).toBool() || result.value(QStringLiteral("commitAccepted")).toBool();
    m_result.insert(QStringLiteral("screenDisplayAndTouchConfirmed"), false);
}

/******************************************************************************
 * Function Name   : DeviceDownloadSession::finishProcess
 * Description     : Finalize once after exit, enforce all transport milestones, and only then remove the private input directory.
 * Input Parameters: process, normalExit, exitCode
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void DeviceDownloadSession::finishProcess(QProcess *process, bool normalExit, int exitCode) //20261114-5 add
{
    static const char *required[] = {"compiled", "transferAccepted", "commitAccepted", "resetWritten", "postResetReadCompleted"};
    QJsonObject finalResult;
    QString key;
    bool milestones;
    bool success;
    bool temporaryRemoved;
    int index;
    QPointer<DeviceDownloadSession> lifetime(this);

    if (m_process != process || !m_running) { return; }
    consumeStdout(process, true);
    if (lifetime.isNull()) { return; }
    m_stderrTail = (m_stderrTail + process->readAllStandardError()).right(4096);
    if (!m_finalSeen) { rememberSessionError(QStringLiteral("missing_final"), QStringLiteral("下载进程未返回最终结构化结果；屏内状态未确认。")); }
    if (!normalExit && m_sessionError.isEmpty()) { rememberSessionError(QStringLiteral("worker_crashed"), QStringLiteral("下载进程异常退出；屏内状态未确认。")); }
    if (normalExit && exitCode != 0 && m_finalResult.value(QStringLiteral("ok")).toBool())
    {
        rememberSessionError(QStringLiteral("nonzero_exit"), QStringLiteral("下载进程退出码%1，最终成功标志未被接受。").arg(exitCode));
    }
    milestones = true;
    for (index = 0; index < int(sizeof(required) / sizeof(required[0])); ++index)
    {
        key = QString::fromLatin1(required[index]);
        milestones = milestones && m_finalResult.value(key).isBool() && m_finalResult.value(key).toBool();
    }
    if (m_finalSeen && m_finalResult.value(QStringLiteral("ok")).toBool() && !milestones)
    {
        rememberSessionError(QStringLiteral("incomplete_final"), QStringLiteral("下载进程未完整确认编译、传输、提交及重启读取，不能作为成功。"));
    }
    if (m_finalSeen && m_finalResult.value(QStringLiteral("ok")).toBool() &&
        m_result.value(QStringLiteral("firstError")).toObject().value(QStringLiteral("present")).toBool())
    {
        rememberSessionError(QStringLiteral("contradictory_result"), QStringLiteral("下载进程已有失败记录，最终成功标志与已知状态矛盾。"));
    }
    success = normalExit && exitCode == 0 && m_finalSeen && !m_streamInvalid && m_sessionError.isEmpty() &&
              m_finalResult.value(QStringLiteral("ok")).isBool() && m_finalResult.value(QStringLiteral("ok")).toBool() && milestones &&
              !m_result.value(QStringLiteral("firstError")).toObject().value(QStringLiteral("present")).toBool();
    temporaryRemoved = m_temp == nullptr || m_temp->remove();
    if (!temporaryRemoved)
    {
        rememberSessionError(QStringLiteral("temporary_cleanup"), QStringLiteral("下载进程已退出，但私有临时目录清理未完成。"));
        success = false;
    }
    delete m_temp;
    m_temp = nullptr;
    m_result.insert(QStringLiteral("ok"), success);
    m_result.insert(QStringLiteral("normalExit"), normalExit);
    m_result.insert(QStringLiteral("exitCode"), exitCode);
    m_result.insert(QStringLiteral("finalResultReceived"), m_finalSeen);
    m_result.insert(QStringLiteral("stdoutBytes"), m_stdoutBytes);
    m_result.insert(QStringLiteral("temporaryCleanupComplete"), temporaryRemoved);
    m_result.insert(QStringLiteral("screenDisplayAndTouchConfirmed"), false);
    if (!m_sessionError.isEmpty())
    {
        m_result.insert(QStringLiteral("sessionError"), m_sessionError);
        if (!m_result.value(QStringLiteral("message")).toString().isEmpty())
        {
            m_result.insert(QStringLiteral("workerMessage"), m_result.value(QStringLiteral("message")));
        }
        m_result.insert(QStringLiteral("message"), m_lastError);
    }
    if (!m_stderrTail.isEmpty()) { m_result.insert(QStringLiteral("stderrTail"), QString::fromUtf8(m_stderrTail)); }
    finalResult = m_result;
    m_running = false;
    m_process = nullptr;
    m_cancelPath.clear();
    process->deleteLater();
    emit finished(success, finalResult);
}
