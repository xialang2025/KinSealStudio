#include "simulation_cli.h"
#include "simulation_window.h"
#include "device_transfer_contract.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>
#include <cmath>
#include <cstdio>

/******************************************************************************
 * Function Name   : simulationDigest
 * Description     : Bind source, event input, and PC image bytes to reproducible session evidence.
 * Input Parameters: bytes
 * Return Value    : Lowercase SHA256.
 * Date            : 2026.11.14
 ******************************************************************************/
static QString simulationDigest(const QByteArray &bytes) //20261114-6 add
{
    return QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
}

/******************************************************************************
 * Function Name   : simulationReportState
 * Description     : Omit all raw address values in sessions containing password-input controls.
 * Input Parameters: runtime, maskMemory
 * Return Value    : A diagnostic snapshot, never a protected credential store.
 * Date            : 2026.11.14
 ******************************************************************************/
static QJsonObject simulationReportState(const SimulationRuntime &runtime, bool maskMemory) //20261114-6 add
{
    QJsonObject state = runtime.snapshot();

    if (maskMemory)
    {
        state.remove(QStringLiteral("LB"));
        state.remove(QStringLiteral("LW"));
        state.insert(QStringLiteral("memoryValuesOmitted"), true);
    }
    return state;
}

/******************************************************************************
 * Function Name   : simulationInput
 * Description     : Read one bounded immutable JSON input before any runtime action.
 * Input Parameters: path, limit, bytes, error
 * Return Value    : True without changing caller bytes after failure.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool simulationInput(const QString &path, qint64 limit, QByteArray *bytes, QString *error) //20261114-6 add
{
    QFile file(path);
    QByteArray prepared;

    if (!file.open(QIODevice::ReadOnly) || file.size() > limit)
    {
        *error = QStringLiteral("无法读取模拟输入，或超过读取大小保护：%1").arg(path);
        return false;
    }
    prepared = file.read(limit + 1);
    if (prepared.size() > limit || file.error() != QFileDevice::NoError)
    {
        *error = QStringLiteral("模拟输入读取未完整结束：%1").arg(path);
        return false;
    }
    *bytes = prepared;
    return true;
}

/******************************************************************************
 * Function Name   : simulationEvent
 * Description     : Execute only typed bounded memory events, rejecting ambiguous field types.
 * Input Parameters: runtime, event, error
 * Return Value    : True for one accepted memory action.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool simulationEvent(SimulationRuntime *runtime, const QJsonObject &event, QString *error) //20261114-6 add
{
    QString operation = event.value(QStringLiteral("op")).toString();
    QJsonValue value = event.value(QStringLiteral("value"));
    QJsonValue type = event.value(QStringLiteral("type"));

    if (operation == QStringLiteral("press") && event.value(QStringLiteral("id")).isString())
    { return runtime->press(event.value(QStringLiteral("id")).toString(), error); }
    if (operation == QStringLiteral("release") && event.value(QStringLiteral("inside")).isBool())
    { return runtime->release(event.value(QStringLiteral("inside")).toBool(), error); }
    if (operation == QStringLiteral("cancel"))
    { runtime->cancel(event.value(QStringLiteral("reason")).toString()); return true; }
    if (operation == QStringLiteral("stop"))
    { runtime->stop(event.value(QStringLiteral("reason")).toString()); return true; }
    if (operation == QStringLiteral("level") && value.isDouble() && value.toDouble() == std::floor(value.toDouble()))
    { return runtime->setUserLevel(value.toInt(-1), error); }
    if (operation == QStringLiteral("write-bit") && value.isBool())
    { return runtime->writeBit(event.value(QStringLiteral("address")).toString(), value.toBool(), error); }
    if (operation == QStringLiteral("write-word") && value.isString() && type.isDouble() && type.toDouble() == std::floor(type.toDouble()))
    { return runtime->writeWord(event.value(QStringLiteral("address")).toString(), type.toInt(-1), value.toString(), error); }
    if (operation == QStringLiteral("input") && value.isString())
    { return runtime->submitInput(value.toString(), error); }
    *error = QStringLiteral("模拟事件的操作或字段类型不支持：%1").arg(operation);
    return false;
}

/******************************************************************************
 * Function Name   : handleSimulationCli
 * Description     : Report capabilities or replay a frozen memory session with atomic JSON/embedded-frame output.
 * Input Parameters: arguments, exitCode
 * Return Value    : True if a simulation CLI verb was handled; never opens physical I/O.
 * Date            : 2026.11.14
 ******************************************************************************/
bool handleSimulationCli(const QStringList &arguments, int *exitCode) //20261114-6 add
{
    HmiProject project;
    SimulationRuntime runtime;
    QByteArray source;
    QByteArray eventBytes;
    QByteArray encoded;
    QByteArray framePng;
    QJsonParseError parseError;
    QJsonDocument document;
    QJsonObject result;
    QJsonObject row;
    QJsonObject state;
    QJsonObject loggedEvent;
    QJsonArray trace;
    QJsonArray events;
    QImage frame;
    QBuffer buffer(&framePng);
    QSaveFile output;
    QString error;
    bool ok = false;
    bool capabilities;
    int index;
    qint64 traceBytes = 0;
    bool passwordInput;
    bool maskMemory = false;

    if (arguments.size() < 2 || (arguments.at(1) != QStringLiteral("--simulation-capabilities") &&
                                 arguments.at(1) != QStringLiteral("--simulate-offline"))) { return false; }
    capabilities = arguments.at(1) == QStringLiteral("--simulation-capabilities");
    *exitCode = 25;
    do
    {
        if (arguments.size() != (capabilities ? 3 : 5))
        {
            error = QStringLiteral("用法：--simulation-capabilities 工程.ksp，或 --simulate-offline 工程.ksp 事件.json 输出.json。");
            break;
        }
        if (!simulationInput(arguments.at(2), 16 * 1024 * 1024, &source, &error)) { break; }
        document = QJsonDocument::fromJson(source, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject() ||
            !HmiProject::fromJson(document.object(), &project, &error))
        {
            if (error.isEmpty()) { error = QStringLiteral("模拟输入需要完整Lite JSON工程；原厂模拟器文件契约另行恢复。"); }
            break;
        }
        result = SimulationRuntime::capabilities(project);
        if (QJsonDocument(result).toJson(QJsonDocument::Compact).size() > 1024 * 1024)
        { error = QStringLiteral("模拟能力报告超过1MiB保护。"); break; }
        for (const HmiObject &object : project.objects)
        {
            if (object.hasWordButtonProperties && object.wordButton.operation == 2) { maskMemory = true; break; }
        }
        result.insert(QStringLiteral("sourceSha256"), simulationDigest(source));
        if (capabilities) { ok = true; break; }
        if (QFileInfo(arguments.at(4)).suffix().compare(QStringLiteral("json"), Qt::CaseInsensitive) != 0 ||
            transferContractWouldReplaceInput(arguments.at(2), arguments.at(4)) ||
            transferContractWouldReplaceInput(arguments.at(3), arguments.at(4)))
        {
            error = QStringLiteral("模拟结果必须写入独立.json文件，不能覆盖工程或事件输入。");
            break;
        }
        if (!simulationInput(arguments.at(3), 1024 * 1024, &eventBytes, &error)) { break; }
        document = QJsonDocument::fromJson(eventBytes, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isArray() || document.array().size() > 1024)
        { error = QStringLiteral("模拟事件必须为最多1024项的JSON数组。"); break; }
        if (!runtime.start(project, &error)) { break; }
        events = document.array();
        for (index = 0; index < events.size(); ++index)
        {
            loggedEvent = events.at(index).toObject();
            passwordInput = false;
            if (loggedEvent.value(QStringLiteral("op")).toString() == QStringLiteral("input"))
            {
                for (const HmiObject &object : runtime.project().objects)
                {
                    if (object.id == runtime.pendingInputObjectId() && object.hasWordButtonProperties && object.wordButton.operation == 2)
                    { passwordInput = true; break; }
                }
            }
            if (!events.at(index).isObject() || !simulationEvent(&runtime, events.at(index).toObject(), &error)) { break; }
            frame = simulationFrame(runtime);
            row = QJsonObject();
            row.insert(QStringLiteral("index"), index);
            if (passwordInput || (maskMemory && loggedEvent.contains(QStringLiteral("value"))))
            { loggedEvent.insert(QStringLiteral("value"), QStringLiteral("<masked-input>")); }
            row.insert(QStringLiteral("event"), loggedEvent);
            state = simulationReportState(runtime, maskMemory);
            state.remove(QStringLiteral("events")); //20261114-6 add: Avoid quadratic copies of the complete event history in every trace row.
            state.remove(QStringLiteral("warnings")); //20261114-6 change: Session warnings are reported once, never multiplied by the event count.
            row.insert(QStringLiteral("state"), state);
            row.insert(QStringLiteral("frameArgb32Sha256"), simulationDigest(QByteArray(reinterpret_cast<const char *>(frame.constBits()), int(frame.sizeInBytes()))));
            traceBytes += QJsonDocument(row).toJson(QJsonDocument::Compact).size();
            if (traceBytes > 8 * 1024 * 1024) { error = QStringLiteral("模拟逐步记录超过8MiB保护，未覆盖输出。"); break; }
            trace.append(row);
        }
        if (index != events.size())
        {
            if (error.isEmpty()) { error = QStringLiteral("模拟事件结构不完整。"); }
            runtime.stop(QStringLiteral("trace-rejected"));
            break;
        }
        frame = simulationFrame(runtime);
        buffer.open(QIODevice::WriteOnly);
        if (!frame.save(&buffer, "PNG")) { error = QStringLiteral("无法保存模拟最终画面。"); break; }
        if (!maskMemory) { result.insert(QStringLiteral("eventsSha256"), simulationDigest(eventBytes)); }
        else { result.insert(QStringLiteral("eventsIdentity"), QStringLiteral("password-session-raw-event-digest-omitted")); }
        result.insert(QStringLiteral("trace"), trace);
        result.insert(QStringLiteral("stateBeforeStop"), simulationReportState(runtime, maskMemory));
        runtime.stop(QStringLiteral("trace-finished")); //20261114-6 add: Process completion is a real cancellation boundary for held momentary controls.
        result.insert(QStringLiteral("finalState"), simulationReportState(runtime, maskMemory));
        result.insert(QStringLiteral("frameCapturedBeforeClose"), true);
        result.insert(QStringLiteral("finalFramePngBase64"), QString::fromLatin1(framePng.toBase64()));
        result.insert(QStringLiteral("ok"), true);
        result.insert(QStringLiteral("hardwareIo"), false);
        result.insert(QStringLiteral("hmiBehaviorConfirmed"), false);
        output.setFileName(arguments.at(4));
        encoded = QJsonDocument(result).toJson(QJsonDocument::Indented);
        if (encoded.size() > 16 * 1024 * 1024) { error = QStringLiteral("模拟结果超过16MiB保护，未覆盖输出。"); break; }
        if (transferContractWouldReplaceInput(arguments.at(2), arguments.at(4)) ||
            transferContractWouldReplaceInput(arguments.at(3), arguments.at(4)) || !output.open(QIODevice::WriteOnly) ||
            output.write(encoded) != encoded.size() || !output.commit())
        { error = QStringLiteral("模拟结果未原子发布：%1").arg(output.errorString()); break; }
        result.remove(QStringLiteral("trace"));
        result.remove(QStringLiteral("finalFramePngBase64"));
        result.insert(QStringLiteral("output"), QFileInfo(arguments.at(4)).absoluteFilePath());
        ok = true;
    } while (false);
    if (!ok) { result = QJsonObject(); } //20261114-6 change: Never emit a rejected oversized trace again through stdout.
    result.insert(QStringLiteral("ok"), ok);
    result.insert(QStringLiteral("hardwareIo"), false);
    result.insert(QStringLiteral("hmiBehaviorConfirmed"), false);
    if (!ok) { result.insert(QStringLiteral("message"), error.left(2048)); }
    encoded = QJsonDocument(result).toJson(QJsonDocument::Compact) + '\n';
    std::fwrite(encoded.constData(), 1, size_t(encoded.size()), stdout);
    std::fflush(stdout);
    if (ok) { *exitCode = 0; }
    return true;
}
