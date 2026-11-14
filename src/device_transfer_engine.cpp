#include "device_transfer_engine.h"

#include <QCryptographicHash>
#include <QJsonArray>
#include <limits>

namespace
{
/******************************************************************************
 * Function Name   : appendEvent
 * Description     : Append a bounded operation record before notifying a synchronous observer.
 * Input Parameters: result, options, event
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void appendEvent(DeviceTransferResult *result, const DeviceTransferOptions &options,
                 DeviceTransferEvent event) //20261114-5 add
{
    event.sequence = int(result->events.size()) + 1;
    result->events.append(event);
    if (options.onEvent)
    {
        options.onEvent(event);
    }
}

/******************************************************************************
 * Function Name   : rememberFailure
 * Description     : Preserve the first protocol or cleanup error without replacing earlier device-state evidence.
 * Input Parameters: result, kind, phase, command, segment, requested, returned, message, expectedReplyLength
 * Return Value    : False
 * Date            : 2026.11.14
 ******************************************************************************/
bool rememberFailure(DeviceTransferResult *result, const QString &kind, const QString &phase,
                     int command, const QString &segment, int requested, int returned,
                     const QString &message, int expectedReplyLength = 0) //20261114-5 add
{
    if (!result->firstError.present)
    {
        result->firstError.present = true;
        result->firstError.kind = kind;
        result->firstError.phase = phase;
        result->firstError.command = command;
        result->firstError.segment = segment;
        result->firstError.requested = requested;
        result->firstError.returned = returned;
        result->firstError.expectedReplyLength = expectedReplyLength;
        result->firstError.message = message;
        result->message = message;
    }
    return false;
}

/******************************************************************************
 * Function Name   : cancellationBoundary
 * Description     : Stop before the next side effect while preventing cancellation from truncating an already written commit.
 * Input Parameters: result, options, phase, command, segment
 * Return Value    : True when cancellation was accepted before commit.
 * Date            : 2026.11.14
 ******************************************************************************/
bool cancellationBoundary(DeviceTransferResult *result, const DeviceTransferOptions &options,
                          const QString &phase, int command, const QString &segment) //20261114-5 add
{
    DeviceTransferEvent event;

    if (result->commitCommandWritten || !options.cancelRequested || !options.cancelRequested())
    {
        return false;
    }
    result->cancelled = true;
    event.phase = phase;
    event.operation = QStringLiteral("cancel");
    event.command = command;
    event.segment = segment;
    event.message = QStringLiteral("下载已在提交前停止；已发送内容不能视为回滚，屏内状态未确认。");
    appendEvent(result, options, event);
    rememberFailure(result, QStringLiteral("cancelled"), phase, command, segment, 0, 0, event.message);
    return true;
}

/******************************************************************************
 * Function Name   : writeBytes
 * Description     : Require one complete bulk OUT operation without retrying partial or failed writes.
 * Input Parameters: result, io, options, phase, command, segment, bytes, payload
 * Return Value    : True only for an exact write count.
 * Date            : 2026.11.14
 ******************************************************************************/
bool writeBytes(DeviceTransferResult *result, const DeviceTransferIo &io,
                const DeviceTransferOptions &options, const QString &phase, int command,
                const QString &segment, const QByteArray &bytes, bool payload) //20261114-5 add
{
    DeviceTransferEvent event;
    int returned;

    result->anyWriteAttempted = true;
    returned = io.write(0x01, bytes, options.timeoutMs);
    event.phase = phase;
    event.operation = payload ? QStringLiteral("payload-write") : QStringLiteral("command-write");
    event.command = command;
    event.segment = segment;
    event.requested = int(bytes.size());
    event.returned = returned;
    event.accepted = returned == bytes.size();
    if (payload)
    {
        event.sha256 = QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex();
    }
    else
    {
        event.bytes = bytes;
    }
    appendEvent(result, options, event);
    if (!event.accepted)
    {
        return rememberFailure(result, QStringLiteral("write_count"), phase, command, segment,
                               int(bytes.size()), returned,
                               QStringLiteral("%1：USB写入%2/%3字节，已停止，屏内状态未确认。")
                                   .arg(phase).arg(returned).arg(bytes.size()));
    }
    return true;
}

/******************************************************************************
 * Function Name   : readExpectedReply
 * Description     : Accept only an exact reply length and its observed first byte without retries or implicit concatenation.
 * Input Parameters: result, io, options, phase, command, segment, expectedLength, expectedByte, captured
 * Return Value    : True for the exact observed reply shape.
 * Date            : 2026.11.14
 ******************************************************************************/
bool readExpectedReply(DeviceTransferResult *result, const DeviceTransferIo &io,
                       const DeviceTransferOptions &options, const QString &phase, int command,
                       const QString &segment, int expectedLength, int expectedByte,
                       QByteArray *captured) //20261114-5 add
{
    DeviceTransferEvent event;
    QByteArray buffer(expectedLength == 24 ? 32 : 4, char(0));
    int returned;
    int bounded;
    int capacity;
    bool lengthMatches;
    bool byteMatches;

    capacity = int(buffer.size());
    returned = io.read(0x81, buffer.data(), capacity, options.timeoutMs);
    bounded = qMax(0, qMin(returned, int(buffer.size())));
    buffer.truncate(bounded);
    lengthMatches = returned == expectedLength;
    byteMatches = lengthMatches && !buffer.isEmpty() && u8(buffer.at(0)) == u8(expectedByte);
    event.phase = phase;
    event.operation = QStringLiteral("reply-read");
    event.command = command;
    event.segment = segment;
    event.requested = capacity;
    event.expectedReplyLength = expectedLength;
    event.returned = returned;
    event.accepted = lengthMatches && byteMatches;
    event.bytes = buffer;
    appendEvent(result, options, event);
    if (captured != nullptr)
    {
        *captured = buffer;
    }
    if (!lengthMatches)
    {
        return rememberFailure(result, QStringLiteral("read_length"), phase, command, segment,
                               capacity, returned,
                               QStringLiteral("%1：设备应答长度%2，预期%3；已停止，屏内状态未确认。")
                                   .arg(phase).arg(returned).arg(expectedLength), expectedLength);
    }
    if (!byteMatches)
    {
        return rememberFailure(result, QStringLiteral("reply_byte"), phase, command, segment,
                               capacity, returned,
                               QStringLiteral("%1：设备应答字节不符合预期，已停止，屏内状态未确认。")
                                   .arg(phase), expectedLength);
    }
    return true;
}

/******************************************************************************
 * Function Name   : sendCommand
 * Description     : Write a zero-filled command and pair its reply before accepting another cancellation boundary.
 * Input Parameters: result, io, options, phase, command, commandOptions, segment, replyLength, replyByte, captured
 * Return Value    : True when the command and any required reply complete.
 * Date            : 2026.11.14
 ******************************************************************************/
bool sendCommand(DeviceTransferResult *result, const DeviceTransferIo &io,
                 const DeviceTransferOptions &options, const QString &phase, int command,
                 const QByteArray &commandOptions, const QString &segment, int replyLength,
                 int replyByte, QByteArray *captured = nullptr) //20261114-5 add
{
    QByteArray frame(32, char(0));
    const QByteArray prefix("KinSealStudioKiss", 17);
    int index;

    if (cancellationBoundary(result, options, phase, command, segment))
    {
        return false;
    }
    frame.replace(0, prefix.size(), prefix);
    frame[17] = char(command);
    for (index = 0; index < commandOptions.size(); ++index)
    {
        frame[18 + index] = commandOptions.at(index);
    }
    if (command == 0x1b)
    {
        result->commitWriteAttempted = true;
    }
    if (!writeBytes(result, io, options, phase, command, segment, frame, false))
    {
        return false;
    }
    if (command == 0x1b)
    {
        result->commitCommandWritten = true;
    }
    if (replyLength == 0)
    {
        return true;
    }
    return readExpectedReply(result, io, options, phase, command, segment,
                             replyLength, replyByte, captured);
}

/******************************************************************************
 * Function Name   : executePlan
 * Description     : Execute observed AMT command order and finish its post-reset read after an acknowledged commit.
 * Input Parameters: result, io, options, steps
 * Return Value    : True after the qualified post-reset read returns.
 * Date            : 2026.11.14
 ******************************************************************************/
bool executePlan(DeviceTransferResult *result, const DeviceTransferIo &io,
                 const DeviceTransferOptions &options, const QVector<HmiDownloadStep> &steps) //20261114-5 add
{
    DeviceTransferEvent event;
    HmiDownloadStep step;
    QByteArray resetReply(4, char(0));
    int index;
    int returned;

    if (!sendCommand(result, io, options, QStringLiteral("identify"), 0x31,
                      QByteArray(1, char(2)), QString(), 24, 0x4c, &result->modelReply) ||
        !sendCommand(result, io, options, QStringLiteral("parameters"), 0x53,
                      QByteArray(1, char(0)), QString(), 24, 0x00, &result->parametersReply) ||
        !sendCommand(result, io, options, QStringLiteral("start"), 0x34,
                      options.startOptions, QString(), 1, 'O'))
    {
        return false;
    }
    result->startAccepted = true;
    if (!sendCommand(result, io, options, QStringLiteral("start-status"), 0x35,
                      QByteArray(), QString(), 1, 'O'))
    {
        return false;
    }
    for (index = 0; index < steps.size(); ++index)
    {
        step = steps.at(index);
        if (!sendCommand(result, io, options, QStringLiteral("segment-command"), step.command,
                          step.options, step.name, 1, 'O'))
        {
            return false;
        }
        if (!step.bytes.isEmpty())
        {
            if (cancellationBoundary(result, options, QStringLiteral("segment-data"), step.command, step.name) ||
                !writeBytes(result, io, options, QStringLiteral("segment-data"), step.command,
                            step.name, step.bytes, true))
            {
                return false;
            }
            result->writtenSegments.append(step.name);
        }
        if (!sendCommand(result, io, options, QStringLiteral("segment-status"), 0x35,
                          QByteArray(), step.name, 1, 'O'))
        {
            return false;
        }
        result->acknowledgedSegments.append(step.name);
        ++result->completedSteps;
    }
    result->transferAccepted = true;
    if (!sendCommand(result, io, options, QStringLiteral("commit"), 0x1b,
                      QByteArray(), QString(), 1, 'O'))
    {
        return false;
    }
    result->commitAccepted = true;
    if (!sendCommand(result, io, options, QStringLiteral("reset"), 0x1a,
                      QByteArray(), QString(), 0, -1))
    {
        return false;
    }
    result->resetWritten = true;
    returned = io.read(0x81, resetReply.data(), int(resetReply.size()), options.timeoutMs);
    result->postResetReadReturned = true;
    result->postResetReadResult = returned;
    // The only accepted disconnect is the observed -5 after confirmed start, all segments, commit ACK, and reset write.
    result->postResetReadCompleted = returned == -5;
    event.phase = QStringLiteral("post-reset-read");
    event.operation = QStringLiteral("reset-read");
    event.command = 0x1a;
    event.requested = int(resetReply.size());
    event.returned = returned;
    event.accepted = result->postResetReadCompleted;
    if (returned > 0)
    {
        event.bytes = resetReply.left(qMin(returned, int(resetReply.size())));
    }
    appendEvent(result, options, event);
    if (!result->postResetReadCompleted)
    {
        return rememberFailure(result, QStringLiteral("post_reset_read"), event.phase, 0x1a,
                               QString(), event.requested, returned,
                               QStringLiteral("提交已获确认且重启已写出，但重启后读取结果%1未符合已验证的-5结束；屏端状态未确认。")
                                   .arg(returned));
    }
    return true;
}

/******************************************************************************
 * Function Name   : closeOwnedIo
 * Description     : Release and close every successfully opened session while preserving its original failure.
 * Input Parameters: result, io, options
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void closeOwnedIo(DeviceTransferResult *result, const DeviceTransferIo &io,
                  const DeviceTransferOptions &options) //20261114-5 add
{
    DeviceTransferEvent event;

    if (result->interfaceClaimed)
    {
        result->releaseAttempted = true;
        result->releaseResult = io.release(0);
        result->interfaceReleased = result->releaseResult == 0;
        event.phase = QStringLiteral("release");
        event.operation = QStringLiteral("release");
        event.returned = result->releaseResult;
        event.accepted = result->interfaceReleased;
        appendEvent(result, options, event);
        if (!result->interfaceReleased)
        {
            rememberFailure(result, QStringLiteral("release"), event.phase, -1, QString(), 0,
                            result->releaseResult, QStringLiteral("USB接口释放失败，错误%1。")
                                .arg(result->releaseResult));
        }
    }
    if (result->opened)
    {
        result->closeAttempted = true;
        result->closeResult = io.close();
        result->closed = result->closeResult == 0;
        event = DeviceTransferEvent();
        event.phase = QStringLiteral("close");
        event.operation = QStringLiteral("close");
        event.returned = result->closeResult;
        event.accepted = result->closed;
        appendEvent(result, options, event);
        if (!result->closed)
        {
            rememberFailure(result, QStringLiteral("close"), event.phase, -1, QString(), 0,
                            result->closeResult, QStringLiteral("USB设备关闭失败，错误%1。")
                                .arg(result->closeResult));
        }
    }
}

/******************************************************************************
 * Function Name   : classifyOutcome
 * Description     : Distinguish unopened, partial, uncertain commit, committed reset, and cleanup outcomes.
 * Input Parameters: result
 * Return Value    : Stable outcome name for recovery decisions.
 * Date            : 2026.11.14
 ******************************************************************************/
QString classifyOutcome(const DeviceTransferResult &result) //20261114-5 add
{
    if (result.success) { return QStringLiteral("transport_complete"); }
    if (result.cancelled && !result.anyWriteAttempted) { return QStringLiteral("cancelled_before_first_command"); }
    if (result.cancelled && !result.startAccepted) { return QStringLiteral("cancelled_before_download_start"); }
    if (result.cancelled) { return QStringLiteral("cancelled_before_commit"); }
    if (result.commitAccepted && !result.resetWritten) { return QStringLiteral("committed_reset_not_written"); }
    if (result.commitAccepted && !result.postResetReadCompleted) { return QStringLiteral("committed_reset_unconfirmed"); }
    if (result.commitAccepted && result.postResetReadCompleted) { return QStringLiteral("completed_cleanup_failed"); }
    if (result.commitWriteAttempted) { return QStringLiteral("commit_status_unknown"); }
    if (result.startAccepted) { return QStringLiteral("partial_transfer_commit_unconfirmed"); }
    if (result.opened) { return QStringLiteral("stopped_before_download_start"); }
    if (result.planValidated) { return QStringLiteral("device_open_failed"); }
    return QStringLiteral("preflight_rejected");
}
}

/******************************************************************************
 * Function Name   : runDeviceTransfer
 * Description     : Validate every wire block and callback before opening an injected transport session.
 * Input Parameters: payloads, io, options
 * Return Value    : Complete protocol and cleanup evidence without a screen acceptance claim.
 * Date            : 2026.11.14
 ******************************************************************************/
DeviceTransferResult runDeviceTransfer(const QVector<QByteArray> &payloads,
                                      const DeviceTransferIo &io,
                                      const DeviceTransferOptions &options) //20261114-5 add
{
    DeviceTransferResult result;
    DeviceTransferEvent event;
    QVector<HmiDownloadStep> steps;
    QString error;
    bool executed;
    int claimed;
    int index;

    result.payloadBlocks = int(payloads.size());
    result.startOptions = options.startOptions;
    result.timeoutMs = options.timeoutMs;
    if (!io.open || !io.claim || !io.write || !io.read || !io.release || !io.close)
    {
        rememberFailure(&result, QStringLiteral("callbacks"), QStringLiteral("preflight"), -1, QString(), 0, 0,
                        QStringLiteral("传输回调不完整，尚未打开设备。"));
    }
    else if (options.timeoutMs <= 0 ||
             (options.startOptions != QByteArray::fromHex("010101") &&
              options.startOptions != QByteArray::fromHex("000000")))
    {
        rememberFailure(&result, QStringLiteral("options"), QStringLiteral("preflight"), -1, QString(), 0, 0,
                        QStringLiteral("超时或开始下载选项未在已验证范围内，尚未打开设备。"));
    }
    else if (payloads.size() > 7 && payloads.at(7).size() > std::numeric_limits<int>::max() - 4)
    {
        rememberFailure(&result, QStringLiteral("wire_length"), QStringLiteral("preflight"), 0x1e,
                        QStringLiteral("upLoad.knr"), 0, 0,
                        QStringLiteral("源快照加线缆长度头后超出USB长度范围，尚未打开设备。"));
    }
    else if (!buildDeviceTransferPlan(payloads, &steps, &error))
    {
        rememberFailure(&result, QStringLiteral("plan"), QStringLiteral("preflight"), -1, QString(), 0, 0, error);
    }
    else
    {
        result.planValidated = true;
        result.plannedSteps = int(steps.size());
        for (index = 0; index < steps.size(); ++index)
        {
            if (steps.at(index).bytes.size() > std::numeric_limits<int>::max() || steps.at(index).options.size() > 14)
            {
                result.planValidated = false;
                rememberFailure(&result, QStringLiteral("wire_length"), QStringLiteral("preflight"),
                                steps.at(index).command, steps.at(index).name, 0, 0,
                                QStringLiteral("传输帧参数或长度不完整，尚未打开设备。"));
                break;
            }
        }
    }
    if (!result.firstError.present && !cancellationBoundary(&result, options, QStringLiteral("open"), -1, QString()))
    {
        result.opened = io.open(&error);
        event.phase = QStringLiteral("open");
        event.operation = QStringLiteral("open");
        event.returned = result.opened ? 0 : -1;
        event.accepted = result.opened;
        event.message = error;
        appendEvent(&result, options, event);
        if (!result.opened)
        {
            rememberFailure(&result, QStringLiteral("open"), event.phase, -1, QString(), 0, -1,
                            error.isEmpty() ? QStringLiteral("无法打开USB设备。") : error);
        }
        else if (!cancellationBoundary(&result, options, QStringLiteral("claim"), -1, QString()))
        {
            claimed = io.claim(0);
            result.interfaceClaimed = claimed == 0;
            event = DeviceTransferEvent();
            event.phase = QStringLiteral("claim");
            event.operation = QStringLiteral("claim");
            event.returned = claimed;
            event.accepted = result.interfaceClaimed;
            appendEvent(&result, options, event);
            if (!result.interfaceClaimed)
            {
                rememberFailure(&result, QStringLiteral("claim"), event.phase, -1, QString(), 0, claimed,
                                QStringLiteral("无法占用USB接口0，错误%1。").arg(claimed));
            }
            else
            {
                executed = executePlan(&result, io, options, steps);
                Q_UNUSED(executed);
            }
        }
    }
    closeOwnedIo(&result, io, options);
    result.success = result.planValidated && result.startAccepted && result.transferAccepted &&
                     result.commitAccepted && result.resetWritten && result.postResetReadCompleted &&
                     result.interfaceReleased && result.closed && !result.firstError.present;
    result.outcome = classifyOutcome(result);
    if (result.success)
    {
        result.message = QStringLiteral("数据段及提交已获确认，重启写入和其后读取已结束；实屏显示/触控仍需独立观察。");
    }
    return result;
}

/******************************************************************************
 * Function Name   : deviceTransferEventToJson
 * Description     : Export bounded event bytes and payload digests with their exact driver counts.
 * Input Parameters: event
 * Return Value    : Structured event object.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject deviceTransferEventToJson(const DeviceTransferEvent &event) //20261114-5 add
{
    QJsonObject object;

    object.insert(QStringLiteral("sequence"), event.sequence);
    object.insert(QStringLiteral("phase"), event.phase);
    object.insert(QStringLiteral("operation"), event.operation);
    object.insert(QStringLiteral("command"), event.command);
    object.insert(QStringLiteral("segment"), event.segment);
    object.insert(QStringLiteral("requested"), event.requested);
    object.insert(QStringLiteral("returned"), event.returned);
    object.insert(QStringLiteral("expectedReplyLength"), event.expectedReplyLength);
    object.insert(QStringLiteral("accepted"), event.accepted);
    object.insert(QStringLiteral("bytesHex"), QString::fromLatin1(event.bytes.toHex()));
    object.insert(QStringLiteral("sha256"), QString::fromLatin1(event.sha256));
    object.insert(QStringLiteral("message"), event.message);
    return object;
}

/******************************************************************************
 * Function Name   : deviceTransferResultToJson
 * Description     : Keep transport milestones independent from unobserved firmware display and touch behavior.
 * Input Parameters: result
 * Return Value    : JSON result suitable for a CLI final record or UI detail view.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject deviceTransferResultToJson(const DeviceTransferResult &result) //20261114-5 add
{
    QJsonObject object;
    QJsonObject failure;
    QJsonArray events;
    DeviceTransferEvent event;

    object.insert(QStringLiteral("ok"), result.success);
    object.insert(QStringLiteral("message"), result.message);
    object.insert(QStringLiteral("outcome"), result.outcome);
    object.insert(QStringLiteral("planValidated"), result.planValidated);
    object.insert(QStringLiteral("opened"), result.opened);
    object.insert(QStringLiteral("interfaceClaimed"), result.interfaceClaimed);
    object.insert(QStringLiteral("releaseAttempted"), result.releaseAttempted);
    object.insert(QStringLiteral("interfaceReleased"), result.interfaceReleased);
    object.insert(QStringLiteral("closeAttempted"), result.closeAttempted);
    object.insert(QStringLiteral("closed"), result.closed);
    object.insert(QStringLiteral("cancelled"), result.cancelled);
    object.insert(QStringLiteral("anyWriteAttempted"), result.anyWriteAttempted);
    object.insert(QStringLiteral("startAccepted"), result.startAccepted);
    object.insert(QStringLiteral("transferAccepted"), result.transferAccepted);
    object.insert(QStringLiteral("commitWriteAttempted"), result.commitWriteAttempted);
    object.insert(QStringLiteral("commitCommandWritten"), result.commitCommandWritten);
    object.insert(QStringLiteral("commitAccepted"), result.commitAccepted);
    object.insert(QStringLiteral("resetWritten"), result.resetWritten);
    object.insert(QStringLiteral("postResetReadReturned"), result.postResetReadReturned);
    object.insert(QStringLiteral("postResetReadCompleted"), result.postResetReadCompleted);
    object.insert(QStringLiteral("postResetReadResult"), result.postResetReadResult);
    object.insert(QStringLiteral("screenDisplayAndTouchConfirmed"), false);
    object.insert(QStringLiteral("completedSteps"), result.completedSteps);
    object.insert(QStringLiteral("plannedSteps"), result.plannedSteps);
    object.insert(QStringLiteral("payloadBlocks"), result.payloadBlocks);
    object.insert(QStringLiteral("releaseResult"), result.releaseResult);
    object.insert(QStringLiteral("closeResult"), result.closeResult);
    object.insert(QStringLiteral("writtenSegments"), QJsonArray::fromStringList(result.writtenSegments));
    object.insert(QStringLiteral("acknowledgedSegments"), QJsonArray::fromStringList(result.acknowledgedSegments));
    object.insert(QStringLiteral("modelReplyHex"), QString::fromLatin1(result.modelReply.toHex()));
    object.insert(QStringLiteral("parametersReplyHex"), QString::fromLatin1(result.parametersReply.toHex()));
    object.insert(QStringLiteral("startOptionsHex"), QString::fromLatin1(result.startOptions.toHex()));
    object.insert(QStringLiteral("timeoutMs"), result.timeoutMs);
    failure.insert(QStringLiteral("present"), result.firstError.present);
    failure.insert(QStringLiteral("kind"), result.firstError.kind);
    failure.insert(QStringLiteral("phase"), result.firstError.phase);
    failure.insert(QStringLiteral("command"), result.firstError.command);
    failure.insert(QStringLiteral("segment"), result.firstError.segment);
    failure.insert(QStringLiteral("requested"), result.firstError.requested);
    failure.insert(QStringLiteral("returned"), result.firstError.returned);
    failure.insert(QStringLiteral("expectedReplyLength"), result.firstError.expectedReplyLength);
    failure.insert(QStringLiteral("message"), result.firstError.message);
    object.insert(QStringLiteral("firstError"), failure);
    Q_FOREACH (event, result.events)
    {
        events.append(deviceTransferEventToJson(event));
    }
    object.insert(QStringLiteral("events"), events);
    return object;
}
