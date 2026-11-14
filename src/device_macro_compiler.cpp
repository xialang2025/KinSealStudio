#include "device_macro_compiler.h"
#include "project_model.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTemporaryDir>
#include <QSettings> //20261109-0 add
#include <QCoreApplication>
#include <QCryptographicHash> //20261114-7 add
#include <QDateTime>
#include <QJsonDocument>
#include <memory>
#include <cstring>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

//20261109-0 add: These addresses and limits belong to the verified 800x480 AMZ/AMT profile.
static const u32 macroRoBase = 0x80520400U;
static const u32 macroRwBase = 0x80440000U;
static const int macroProcessTimeout = 30000;
static const int macroTotalTimeout = 120000;
static const int macroLogLimit = 256 * 1024;

/******************************************************************************
 * Function Name   : macroFailure
 * Description     : Return a concrete diagnostic without replacing an existing package.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.09
 ******************************************************************************/
static bool macroFailure(QString *error, const QString &message) //20261109-0 add
{
    if (error != nullptr)
    {
        *error = message;
    }
    return false;
}

/******************************************************************************
 * Function Name   : macroReadU16
 * Description     : Decode a previously bounds-checked little-endian ELF field.
 * Input Parameters: bytes, offset
 * Return Value    : The unsigned 16-bit value.
 * Date            : 2026.11.09
 ******************************************************************************/
static u16 macroReadU16(const QByteArray &bytes, qsizetype offset) //20261109-0 add
{
    return u16(u8(bytes.at(offset))) | u16(u16(u8(bytes.at(offset + 1))) << 8);
}

/******************************************************************************
 * Function Name   : macroReadU32
 * Description     : Decode a previously bounds-checked little-endian ELF field.
 * Input Parameters: bytes, offset
 * Return Value    : The unsigned 32-bit value.
 * Date            : 2026.11.09
 ******************************************************************************/
static u32 macroReadU32(const QByteArray &bytes, qsizetype offset) //20261109-0 add
{
    return u32(u8(bytes.at(offset))) | (u32(u8(bytes.at(offset + 1))) << 8) |
           (u32(u8(bytes.at(offset + 2))) << 16) | (u32(u8(bytes.at(offset + 3))) << 24);
}

/******************************************************************************
 * Function Name   : macroWriteU16
 * Description     : Store an actual image length in the native macro run header.
 * Input Parameters: bytes, offset, value
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
static void macroWriteU16(QByteArray *bytes, qsizetype offset, u16 value) //20261109-0 add
{
    (*bytes)[offset] = char(value & 0xffU);
    (*bytes)[offset + 1] = char((value >> 8) & 0xffU);
}

/******************************************************************************
 * Function Name   : macroReadFile
 * Description     : Read one bounded compiler artifact from the private build directory.
 * Input Parameters: path, maximum, bytes, error
 * Return Value    : True when all bytes were read.
 * Date            : 2026.11.09
 ******************************************************************************/
static bool macroReadFile(const QString &path, qsizetype maximum, QByteArray *bytes,
                          QString *error) //20261109-0 add
{
    QFile file;

    file.setFileName(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return macroFailure(error, QStringLiteral("无法读取宏编译产物 %1：%2")
                                       .arg(QFileInfo(path).fileName(), file.errorString()));
    }
    if (file.size() < 1 || file.size() > maximum)
    {
        return macroFailure(error, QStringLiteral("宏编译产物 %1 长度无效：%2 字节。")
                                       .arg(QFileInfo(path).fileName()).arg(file.size()));
    }
    *bytes = file.read(maximum + 1); //20261114-7 change: Bound the actual read even if an artifact grows after its size check.
    if (bytes->size() > maximum || bytes->size() != file.size() || file.error() != QFileDevice::NoError)
    {
        return macroFailure(error, QStringLiteral("宏编译产物读取不完整：%1。")
                                       .arg(QFileInfo(path).fileName()));
    }
    return true;
}

/******************************************************************************
 * Function Name   : macroHasEntry
 * Description     : Verify that the linked ARM image really starts with Macro_Entry.
 * Input Parameters: image, expectedAddress
 * Return Value    : True for a bounded ELF32 ARM image with the required entry symbol.
 * Date            : 2026.11.09
 ******************************************************************************/
static bool macroHasEntry(const QByteArray &image, u32 expectedAddress) //20261109-0 add
{
    qsizetype sectionTable;
    qsizetype sectionSize;
    qsizetype sectionCount;
    qsizetype section;
    qsizetype symbols;
    qsizetype symbolBytes;
    qsizetype symbolSize;
    qsizetype stringSection;
    qsizetype strings;
    qsizetype stringBytes;
    qsizetype symbol;
    qsizetype name;
    qsizetype end;
    int index;

    if (image.size() < 52 || image.left(4) != QByteArray::fromHex("7f454c46") ||
        u8(image.at(4)) != 1 || u8(image.at(5)) != 1 || macroReadU16(image, 16) != 2 ||
        macroReadU16(image, 18) != 40 || macroReadU32(image, 24) != expectedAddress)
    {
        return false;
    }
    sectionTable = macroReadU32(image, 32);
    sectionSize = macroReadU16(image, 46);
    sectionCount = macroReadU16(image, 48);
    if (sectionSize < 40 || sectionCount < 1 || sectionTable > image.size() ||
        sectionCount * sectionSize > image.size() - sectionTable)
    {
        return false;
    }
    for (index = 0; index < sectionCount; ++index)
    {
        section = sectionTable + index * sectionSize;
        if (macroReadU32(image, section + 4) != 2)
        {
            continue;
        }
        symbols = macroReadU32(image, section + 16);
        symbolBytes = macroReadU32(image, section + 20);
        stringSection = macroReadU32(image, section + 24);
        symbolSize = macroReadU32(image, section + 36);
        if (symbolSize < 16 || stringSection >= sectionCount || symbols > image.size() ||
            symbolBytes > image.size() - symbols)
        {
            return false;
        }
        stringSection = sectionTable + stringSection * sectionSize;
        strings = macroReadU32(image, stringSection + 16);
        stringBytes = macroReadU32(image, stringSection + 20);
        if (strings > image.size() || stringBytes > image.size() - strings)
        {
            return false;
        }
        for (symbol = symbols; symbol + 16 <= symbols + symbolBytes; symbol += symbolSize)
        {
            name = macroReadU32(image, symbol);
            if (name >= stringBytes)
            {
                continue;
            }
            end = image.indexOf('\0', strings + name);
            if (end < strings + name || end >= strings + stringBytes)
            {
                continue;
            }
            if (image.mid(strings + name, end - strings - name) == QByteArray("Macro_Entry") &&
                macroReadU32(image, symbol + 4) == expectedAddress &&
                macroReadU16(image, symbol + 14) != 0)
            {
                return true;
            }
        }
    }
    return false;
}

/******************************************************************************
 * Function Name   : macroHasBoundedRw
 * Description     : Keep writable ELF sections inside the macro's isolated 1 KiB slot.
 * Input Parameters: image, expectedAddress
 * Return Value    : True when allocated RW and zero-initialized data cannot overlap another macro.
 * Date            : 2026.11.09
 ******************************************************************************/
static bool macroHasBoundedRw(const QByteArray &image, u32 expectedAddress) //20261109-0 add
{
    qsizetype table;
    qsizetype stride;
    qsizetype count;
    qsizetype section;
    u32 address;
    u32 size;
    int index;

    table = macroReadU32(image, 32);
    stride = macroReadU16(image, 46);
    count = macroReadU16(image, 48);
    for (index = 0; index < count; ++index)
    {
        section = table + index * stride;
        if ((macroReadU32(image, section + 8) & 3U) != 3U)
        {
            continue;
        }
        address = macroReadU32(image, section + 12);
        size = macroReadU32(image, section + 20);
        if (address < expectedAddress || address - expectedAddress > 1024U ||
            size > 1024U - (address - expectedAddress))
        {
            return false;
        }
    }
    return true;
}

//20261114-7 add: All state belongs to one invocation; no global cancellation or process handle is used.
struct MacroCompileContext
{
    DeviceMacroCompileOptions options;
    QElapsedTimer timer;
    QJsonObject report;
    QJsonArray stages;
    QJsonArray sources;
    QJsonArray mappings;
    QJsonArray diagnostics;
    QString firstError;
    QString firstCode;
    QString macroName;
    QString phase;
    QString workDirectory;
    int macroIndex = -1;
    qint64 retainedOutput = 0;
};

/******************************************************************************
 * Function Name   : macroSha
 * Description     : Bind exact bytes without normalizing source, licenses or compiler output.
 * Input Parameters: bytes
 * Return Value    : Lowercase SHA256 text.
 * Date            : 2026.11.14
 ******************************************************************************/
static QString macroSha(const QByteArray &bytes) //20261114-7 add
{
    return QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
}

/******************************************************************************
 * Function Name   : macroRecordFailure
 * Description     : Preserve the first structured failure while retaining later cleanup diagnostics.
 * Input Parameters: context, code, message
 * Return Value    : False.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool macroRecordFailure(MacroCompileContext *context, const QString &code, const QString &message) //20261114-7 add
{
    QJsonObject diagnostic;

    diagnostic = QJsonObject{{QStringLiteral("severity"), QStringLiteral("error")},
        {QStringLiteral("code"), code}, {QStringLiteral("message"), message.left(4096)},
        {QStringLiteral("stage"), context->phase}, {QStringLiteral("macroName"), context->macroName.left(1024)},
        {QStringLiteral("compiledIndex"), context->macroIndex}};
    context->diagnostics.append(diagnostic);
    if (context->firstCode.isEmpty())
    {
        context->firstCode = code;
        context->firstError = message.left(4096);
        context->report.insert(QStringLiteral("firstError"), diagnostic);
    }
    return false;
}

/******************************************************************************
 * Function Name   : macroCheckpoint
 * Description     : Observe cancellation and the whole invocation deadline without dispatching GUI events.
 * Input Parameters: context
 * Return Value    : True while work remains authorized.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool macroCheckpoint(MacroCompileContext *context) //20261114-7 add
{
    bool cancelled = false;

    try
    {
        if (context->options.isCancellationRequested) { cancelled = context->options.isCancellationRequested(); }
    }
    catch (...)
    {
        return macroRecordFailure(context, QStringLiteral("cancellation-callback-failed"),
                                  QStringLiteral("宏取消检查回调抛出异常，编译已停止。"));
    }
    if (cancelled)
    {
        context->report.insert(QStringLiteral("cancelled"), true);
        return macroRecordFailure(context, QStringLiteral("cancelled"), QStringLiteral("宏编译已取消。"));
    }
    if (context->timer.elapsed() >= context->options.totalTimeoutMs)
    {
        return macroRecordFailure(context, QStringLiteral("total-timeout"), QStringLiteral("宏编译超过本次总耗时保护。"));
    }
    return true;
}

/******************************************************************************
 * Function Name   : macroRequiredFiles
 * Description     : Enumerate the complete dependency set required by the existing zero-variable SDK contract.
 * Input Parameters: None
 * Return Value    : Stable relative file names, not an authenticity allowlist.
 * Date            : 2026.11.14
 ******************************************************************************/
static QStringList macroRequiredFiles() //20261114-7 add
{
    return {QStringLiteral("armcc.exe"), QStringLiteral("armcc.dll"), QStringLiteral("armlink.exe"),
        QStringLiteral("armlink.dll"), QStringLiteral("fromelf.exe"), QStringLiteral("fromelf.dll"),
        QStringLiteral("Include/macrolib.h"), QStringLiteral("Include/glib.h"), QStringLiteral("Include/math.h"),
        QStringLiteral("Include/stdio.h"), QStringLiteral("Include/stdlib.h"), QStringLiteral("script_amzlib.a"),
        QStringLiteral("guilib70_amz.a"), QStringLiteral("licenses/LICENSE.DAT"), QStringLiteral("Lib/armlib/c_a__un.l")};
}

/******************************************************************************
 * Function Name   : macroPeMetadata
 * Description     : Read PE architecture and fixed version resources without executing the inspected module.
 * Input Parameters: path, prefix
 * Return Value    : Observed metadata; absent version resources remain explicitly absent.
 * Date            : 2026.11.14
 ******************************************************************************/
static QJsonObject macroPeMetadata(const QString &path, const QByteArray &prefix) //20261114-7 add
{
    QJsonObject result{{QStringLiteral("isPe"), false}, {QStringLiteral("versionResourcePresent"), false}};
    u32 offset;
    u16 magic;
#ifdef Q_OS_WIN
    HMODULE versionModule;
    using VersionSize = DWORD (WINAPI *)(LPCWSTR, LPDWORD);
    using VersionData = BOOL (WINAPI *)(LPCWSTR, DWORD, DWORD, LPVOID);
    using VersionQuery = BOOL (WINAPI *)(LPCVOID, LPCWSTR, LPVOID *, PUINT);
    VersionSize sizeFunction;
    VersionData dataFunction;
    VersionQuery queryFunction;
    FARPROC procedure;
    DWORD ignored = 0;
    DWORD size;
    QByteArray resource;
    VS_FIXEDFILEINFO *fixed = nullptr;
    UINT length = 0;
#endif

    if (prefix.size() < 64 || prefix.left(2) != QByteArray("MZ")) { return result; }
    offset = macroReadU32(prefix, 60);
    if (offset > u32(prefix.size()) || u32(prefix.size()) - offset < 26U ||
        prefix.mid(offset, 4) != QByteArray("PE\0\0", 4)) { return result; }
    magic = macroReadU16(prefix, qsizetype(offset) + 24);
    result.insert(QStringLiteral("isPe"), true);
    result.insert(QStringLiteral("machine"), QStringLiteral("0x%1").arg(macroReadU16(prefix, qsizetype(offset) + 4), 4, 16, QLatin1Char('0')));
    result.insert(QStringLiteral("bits"), magic == 0x10b ? 32 : magic == 0x20b ? 64 : 0);
#ifdef Q_OS_WIN
    versionModule = LoadLibraryExW(L"version.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (versionModule == nullptr) { return result; }
    static_assert(sizeof(sizeFunction) == sizeof(procedure) && sizeof(dataFunction) == sizeof(procedure) &&
                  sizeof(queryFunction) == sizeof(procedure), "Windows procedure pointer size must match.");
    procedure = GetProcAddress(versionModule, "GetFileVersionInfoSizeW");
    std::memcpy(&sizeFunction, &procedure, sizeof(sizeFunction));
    procedure = GetProcAddress(versionModule, "GetFileVersionInfoW");
    std::memcpy(&dataFunction, &procedure, sizeof(dataFunction));
    procedure = GetProcAddress(versionModule, "VerQueryValueW");
    std::memcpy(&queryFunction, &procedure, sizeof(queryFunction));
    if (sizeFunction != nullptr && dataFunction != nullptr && queryFunction != nullptr)
    {
        size = sizeFunction(reinterpret_cast<LPCWSTR>(path.utf16()), &ignored);
        if (size > 0 && size <= 1024U * 1024U)
        {
            resource.resize(int(size));
            if (dataFunction(reinterpret_cast<LPCWSTR>(path.utf16()), 0, size, resource.data()) &&
                queryFunction(resource.constData(), L"\\", reinterpret_cast<LPVOID *>(&fixed), &length) &&
                length >= sizeof(VS_FIXEDFILEINFO) && fixed != nullptr && fixed->dwSignature == 0xfeef04bdU)
            {
                result.insert(QStringLiteral("versionResourcePresent"), true);
                result.insert(QStringLiteral("fileVersion"), QStringLiteral("%1.%2.%3.%4")
                    .arg(HIWORD(fixed->dwFileVersionMS)).arg(LOWORD(fixed->dwFileVersionMS))
                    .arg(HIWORD(fixed->dwFileVersionLS)).arg(LOWORD(fixed->dwFileVersionLS)));
                result.insert(QStringLiteral("productVersion"), QStringLiteral("%1.%2.%3.%4")
                    .arg(HIWORD(fixed->dwProductVersionMS)).arg(LOWORD(fixed->dwProductVersionMS))
                    .arg(HIWORD(fixed->dwProductVersionLS)).arg(LOWORD(fixed->dwProductVersionLS)));
            }
        }
    }
    FreeLibrary(versionModule);
#else
    Q_UNUSED(path);
#endif
    return result;
}

/******************************************************************************
 * Function Name   : macroSdkManifest
 * Description     : Hash every required file through bounded reads and record observed PE metadata.
 * Input Parameters: sdk, context, files
 * Return Value    : True for a complete stable read, without authenticating the SDK vendor.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool macroSdkManifest(const QDir &sdk, MacroCompileContext *context, QJsonArray *files) //20261114-7 add
{
    QString name;
    QFileInfo before;
    QFileInfo after;
    QFile file;
    QByteArray chunk;
    QByteArray prefix;
    QCryptographicHash hash(QCryptographicHash::Sha256);
    QJsonObject row;
    qint64 total = 0;
    qint64 read;
    QStringList required = macroRequiredFiles();

    *files = QJsonArray();
    Q_FOREACH (name, required)
    {
        if (!macroCheckpoint(context)) { return false; }
        before.setFile(sdk.filePath(name));
        file.setFileName(before.absoluteFilePath());
        if (!before.isFile() || before.isSymLink() || before.size() > 128 * 1024 * 1024 || !file.open(QIODevice::ReadOnly))
        {
            return macroRecordFailure(context, QStringLiteral("sdk-file-unreadable"),
                                      QStringLiteral("ARM宏工具链缺少或无法有界读取文件：%1。").arg(name));
        }
        hash.reset();
        prefix.clear();
        read = 0;
        while (!file.atEnd())
        {
            if (!macroCheckpoint(context)) { return false; }
            chunk = file.read(64 * 1024);
            if (chunk.isEmpty() && !file.atEnd()) { break; }
            read += chunk.size();
            total += chunk.size();
            if (read > 128 * 1024 * 1024 || total > 512 * 1024 * 1024)
            { return macroRecordFailure(context, QStringLiteral("sdk-size-limit"), QStringLiteral("SDK文件或总读取超过保护上限。")); }
            hash.addData(chunk);
            if (prefix.size() < 1024 * 1024) { prefix.append(chunk.left(1024 * 1024 - prefix.size())); }
        }
        after.setFile(before.absoluteFilePath());
        if (file.error() != QFileDevice::NoError || read != before.size() || file.size() != before.size() ||
            after.size() != before.size() || after.lastModified() != before.lastModified())
        { return macroRecordFailure(context, QStringLiteral("sdk-changed-during-read"), QStringLiteral("SDK读取期间发生变化：%1。").arg(name)); }
        file.close();
        row = QJsonObject{{QStringLiteral("relativePath"), name}, {QStringLiteral("bytes"), double(read)},
            {QStringLiteral("sha256"), QString::fromLatin1(hash.result().toHex())}};
        if (name.endsWith(QStringLiteral(".exe")) || name.endsWith(QStringLiteral(".dll")))
        { row.insert(QStringLiteral("pe"), macroPeMetadata(before.absoluteFilePath(), prefix)); }
        files->append(row);
    }
    return true;
}

/******************************************************************************
 * Function Name   : macroCaptureOutput
 * Description     : Retain only the allowed prefix while consuming a bounded available process chunk.
 * Input Parameters: process, context, output, observed, truncated
 * Return Value    : True within both per-process and total diagnostic budgets.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool macroCaptureOutput(QProcess *process, MacroCompileContext *context, QByteArray *output,
                               qint64 *observed, bool *truncated) //20261114-7 add
{
    QByteArray chunk;
    qint64 room;
    qint64 kept;

    while (process->bytesAvailable() > 0)
    {
        chunk = process->read(64 * 1024);
        *observed += chunk.size();
        room = qMin(qint64(context->options.maxProcessOutputBytes - output->size()),
                    qint64(context->options.maxTotalOutputBytes) - context->retainedOutput);
        kept = qMin(qMax(qint64(0), room), qint64(chunk.size()));
        output->append(chunk.constData(), qsizetype(kept));
        context->retainedOutput += kept;
        if (kept != chunk.size())
        {
            *truncated = true;
            return macroRecordFailure(context, QStringLiteral("diagnostic-output-limit"),
                                      QStringLiteral("宏编译诊断输出超过单阶段或累计字节保护。"));
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : macroRunProcess
 * Description     : Run one owned hidden SDK child with polling cancellation, deadlines and bounded output.
 * Input Parameters: sdk, workDirectory, executable, arguments, label, context, diagnostics
 * Return Value    : True only for normal zero exit within the invocation's resource limits.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool macroRunProcess(const QDir &sdk, const QString &workDirectory,
                            const QString &executable, const QStringList &arguments,
                            const QString &label, MacroCompileContext *context,
                            QStringList *diagnostics) //20261114-7 change
{
    QProcess process;
    QProcessEnvironment environment;
    QElapsedTimer stageTimer;
    QByteArray output;
    QJsonObject stage;
    QJsonArray argv;
    QString argument;
    bool ok = true;
    bool truncated = false;
    bool killed = false;
    bool cleanupFinished = true;
    qint64 observed = 0;
    qint64 processId = 0;
    int wait;

    context->phase = label;
    if (!macroCheckpoint(context)) { return false; }
    Q_FOREACH (argument, arguments) { argv.append(argument); }
    stage = QJsonObject{{QStringLiteral("stage"), label}, {QStringLiteral("macroName"), context->macroName},
        {QStringLiteral("compiledIndex"), context->macroIndex}, {QStringLiteral("program"), sdk.filePath(executable)},
        {QStringLiteral("argv"), argv}, {QStringLiteral("workingDirectory"), workDirectory}};
    environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LM_LICENSE_FILE"), sdk.filePath(QStringLiteral("licenses/LICENSE.DAT")));
    environment.insert(QStringLiteral("ARMLIB"), sdk.filePath(QStringLiteral("Lib/armlib")));
    environment.insert(QStringLiteral("TEMP"), workDirectory);
    environment.insert(QStringLiteral("TMP"), workDirectory);
    environment.insert(QStringLiteral("TMPDIR"), workDirectory);
    process.setProcessEnvironment(environment); // Preserve PATH; no installed SDK or host-license file is modified.
    process.setWorkingDirectory(workDirectory);
    process.setProgram(sdk.filePath(executable));
    process.setArguments(arguments);
    process.setProcessChannelMode(QProcess::MergedChannels);
#ifdef Q_OS_WIN
    process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *created) {
        created->flags |= CREATE_NO_WINDOW;
    });
#endif
    stageTimer.start();
    process.start();
    while (process.state() == QProcess::Starting && ok)
    {
        process.waitForStarted(20);
        ok = macroCheckpoint(context);
        if (ok && stageTimer.elapsed() >= context->options.startTimeoutMs)
        { ok = macroRecordFailure(context, QStringLiteral("start-timeout"), QStringLiteral("%1：启动超时。").arg(label)); }
    }
    processId = process.processId();
    if (ok && process.state() == QProcess::NotRunning && process.error() == QProcess::FailedToStart)
    { ok = macroRecordFailure(context, QStringLiteral("start-failed"), QStringLiteral("%1：无法启动%2：%3").arg(label, executable, process.errorString())); }
    while (ok && process.state() != QProcess::NotRunning)
    {
        wait = qMax(1, qMin(20, context->options.totalTimeoutMs - int(context->timer.elapsed())));
        process.waitForFinished(wait);
        ok = macroCaptureOutput(&process, context, &output, &observed, &truncated);
        if (ok) { ok = macroCheckpoint(context); }
        if (ok && stageTimer.elapsed() >= context->options.processTimeoutMs)
        { ok = macroRecordFailure(context, QStringLiteral("stage-timeout"), QStringLiteral("%1：编译进程单阶段超时。").arg(label)); }
    }
    if (!ok && process.state() != QProcess::NotRunning)
    {
        killed = true;
        process.kill(); // Only this QProcess instance's owned child is terminated.
        cleanupFinished = process.waitForFinished(context->options.cleanupTimeoutMs);
    }
    if (!macroCaptureOutput(&process, context, &output, &observed, &truncated)) { ok = false; }
    if (ok) { ok = macroCheckpoint(context); }
    if (ok && stageTimer.elapsed() >= context->options.processTimeoutMs)
    { ok = macroRecordFailure(context, QStringLiteral("stage-timeout"), QStringLiteral("%1：编译进程单阶段超时。").arg(label)); }
    if (ok && (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0))
    { ok = macroRecordFailure(context, QStringLiteral("sdk-stage-failed"), QStringLiteral("%1：%2失败（退出码%3）。\n%4")
        .arg(label, executable).arg(process.exitCode()).arg(QString::fromLocal8Bit(output).left(3000))); }
    stage.insert(QStringLiteral("ok"), ok);
    stage.insert(QStringLiteral("processId"), double(processId));
    stage.insert(QStringLiteral("elapsedMs"), double(stageTimer.elapsed()));
    stage.insert(QStringLiteral("exitCode"), process.exitCode());
    stage.insert(QStringLiteral("normalExit"), process.exitStatus() == QProcess::NormalExit);
    stage.insert(QStringLiteral("ownedProcessKilled"), killed);
    stage.insert(QStringLiteral("ownedProcessCleanupFinished"), cleanupFinished);
    stage.insert(QStringLiteral("capturedBytes"), output.size());
    stage.insert(QStringLiteral("observedOutputBytesAtLeast"), double(observed));
    stage.insert(QStringLiteral("outputTruncated"), truncated);
    stage.insert(QStringLiteral("capturedOutputSha256"), macroSha(output));
    stage.insert(QStringLiteral("output"), QString::fromLocal8Bit(output));
    if (!ok) { stage.insert(QStringLiteral("firstErrorCode"), context->firstCode); }
    context->stages.append(stage);
    if (!output.trimmed().isEmpty()) { diagnostics->append(label + QStringLiteral("：\n") + QString::fromLocal8Bit(output)); }
    if (!cleanupFinished)
    { macroRecordFailure(context, QStringLiteral("owned-process-cleanup-failed"), QStringLiteral("已请求终止自有SDK子进程，但限时等待尚未确认退出。")); }
    return ok && cleanupFinished;
}

/******************************************************************************
 * Function Name   : deviceMacroSdkDirectory
 * Description     : Resolve the explicit environment override, saved SDK selection, then portable SDK folder.
 * Input Parameters: None
 * Return Value    : The selected ARM SDK directory.
 * Date            : 2026.11.09
 ******************************************************************************/
QString deviceMacroSdkDirectory() //20261109-0 add
{
    QString path;
    QSettings settings(QStringLiteral("KinSealStudioLite"), QStringLiteral("DeviceCompiler"));

    path = qEnvironmentVariable("KINSEAL_ARM_SDK");
    if (path.isEmpty()) { path = settings.value(QStringLiteral("armSdkDirectory")).toString(); }
    if (path.isEmpty()) { path = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("arm-sdk")); }
    return path;
}

/******************************************************************************
 * Function Name   : validateDeviceMacroSdkDirectory
 * Description     : Check all required compiler and library files without running any executable.
 * Input Parameters: path, error
 * Return Value    : True for a complete supplied SDK directory.
 * Date            : 2026.11.09
 ******************************************************************************/
bool validateDeviceMacroSdkDirectory(const QString &path, QString *error) //20261109-0 add
{
    QDir sdk;
    QStringList required;
    QString item;

    if (path.trimmed().isEmpty() || !QFileInfo(path).isDir())
    {
        return macroFailure(error, QStringLiteral("ARM宏工具链目录无效，请通过工具菜单选择完整SDK目录，或设置KINSEAL_ARM_SDK。"));
    }
    sdk.setPath(path);
    required = {QStringLiteral("armcc.exe"), QStringLiteral("armcc.dll"),
                QStringLiteral("armlink.exe"), QStringLiteral("armlink.dll"),
                QStringLiteral("fromelf.exe"), QStringLiteral("fromelf.dll"),
                QStringLiteral("Include/macrolib.h"), QStringLiteral("Include/glib.h"),
                QStringLiteral("Include/math.h"), QStringLiteral("Include/stdio.h"),
                QStringLiteral("Include/stdlib.h"), QStringLiteral("script_amzlib.a"),
                QStringLiteral("guilib70_amz.a"), QStringLiteral("licenses/LICENSE.DAT"),
                QStringLiteral("Lib/armlib/c_a__un.l")};
    Q_FOREACH (item, required)
    {
        if (!QFileInfo(sdk.filePath(item)).isFile())
        {
            return macroFailure(error, QStringLiteral("ARM宏工具链缺少文件：%1。请提供完整SDK目录。").arg(item));
        }
    }
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : compileDeviceMacros
 * Description     : Preserve the original API while using the bounded evidence-producing implementation.
 * Input Parameters: project, toolchainDirectory, package, error
 * Return Value    : True after transactional package publication.
 * Date            : 2026.11.14
 ******************************************************************************/
bool compileDeviceMacros(const HmiProject &project, const QString &toolchainDirectory,
                         DeviceMacroPackage *package, QString *error) //20261114-7 change
{
    return compileDeviceMacros(project, toolchainDirectory, package, error, DeviceMacroCompileOptions(), nullptr);
}

/******************************************************************************
 * Function Name   : compileDeviceMacros
 * Description     : Freeze sources, identify the supplied SDK, compile owned stages and publish only a complete package.
 * Input Parameters: project, toolchainDirectory, package, error, options, report
 * Return Value    : True on complete publication; independent report describes failures without replacing package.
 * Date            : 2026.11.14
 ******************************************************************************/
bool compileDeviceMacros(const HmiProject &project, const QString &toolchainDirectory,
                         DeviceMacroPackage *package, QString *error,
                         const DeviceMacroCompileOptions &options, QJsonObject *report) //20261114-7 add
{
    const QMap<QString, QString> frozenMacros = project.macros;
    const QString targetModel = project.targetModel;
    MacroCompileContext context;
    DeviceMacroPackage prepared;
    QMap<QString, QString>::const_iterator macro;
    QDir sdk;
    QStringList arguments;
    QString fileName;
    QString detail;
    QString rootPath;
    QString label;
    QByteArray header;
    QByteArray source;
    QByteArray utf16;
    QByteArray image;
    QByteArray binary;
    QByteArray bodies;
    QJsonArray sdkFiles;
    QJsonArray sdkAfter;
    QJsonObject sdkIdentity;
    QJsonObject row;
    QJsonObject stable;
    QFile file;
    std::unique_ptr<QTemporaryDir> directory;
    u32 roAddress = macroRoBase;
    u32 rwAddress;
    u16 unit;
    QChar character;
    int index = 0;
    bool ok = false;
    bool cleaned = true;

    if (error != nullptr) { error->clear(); }
    if (package != nullptr && report == &package->compileManifest)
    { return macroFailure(error, QStringLiteral("失败报告必须独立于既有运行包，不能别名compileManifest。")); }
    context.options = options;
    context.timer.start();
    context.phase = QStringLiteral("preflight");
    context.report = QJsonObject{{QStringLiteral("format"), QStringLiteral("KinSealLite-device-macro-compilation-v1")},
        {QStringLiteral("ok"), false}, {QStringLiteral("cancelled"), false},
        {QStringLiteral("targetModel"), targetModel}, {QStringLiteral("addressVariableCount"), 0},
        {QStringLiteral("hardwareExecutionVerified"), false}, {QStringLiteral("nativeRuntimeCompatibilityVerified"), false},
        {QStringLiteral("sdkAuthenticated"), false}, {QStringLiteral("sourceSetFrozen"), true},
        {QStringLiteral("macroCount"), frozenMacros.size()}, {QStringLiteral("qtVersion"), QString::fromLatin1(qVersion())},
        {QStringLiteral("processTimeoutMs"), options.processTimeoutMs}, {QStringLiteral("totalTimeoutMs"), options.totalTimeoutMs},
        {QStringLiteral("cleanupTimeoutMs"), options.cleanupTimeoutMs},
        {QStringLiteral("deadlineScope"), QStringLiteral("whole invocation; bounded owned-child cleanup follows cancellation/deadline")},
        {QStringLiteral("maxProcessOutputBytes"), options.maxProcessOutputBytes},
        {QStringLiteral("maxTotalOutputBytes"), options.maxTotalOutputBytes}};
#ifdef Q_OS_WIN
    context.report.insert(QStringLiteral("ansiCodePage"), int(GetACP()));
    context.report.insert(QStringLiteral("ansiEncoding"), QStringLiteral("Windows-ACP-Qt-toLocal8Bit"));
#else
    context.report.insert(QStringLiteral("ansiCodePage"), QJsonValue());
    context.report.insert(QStringLiteral("ansiEncoding"), QStringLiteral("Qt-local8Bit-platform-rule"));
#endif
    do
    {
        if (package == nullptr)
        { macroRecordFailure(&context, QStringLiteral("null-package"), QStringLiteral("宏编译输出参数为空。")); break; }
        if (options.processTimeoutMs < 1 || options.processTimeoutMs > macroProcessTimeout ||
            options.totalTimeoutMs < 1 || options.totalTimeoutMs > macroTotalTimeout ||
            options.startTimeoutMs < 1 || options.startTimeoutMs > 5000 ||
            options.cleanupTimeoutMs < 1 || options.cleanupTimeoutMs > 5000 ||
            options.maxProcessOutputBytes < 1 || options.maxProcessOutputBytes > macroLogLimit ||
            options.maxTotalOutputBytes < 1 || options.maxTotalOutputBytes > 1024 * 1024)
        { macroRecordFailure(&context, QStringLiteral("invalid-options"), QStringLiteral("宏编译限时或日志上限无效。")); break; }
        if (!macroCheckpoint(&context)) { break; }
        if (frozenMacros.isEmpty()) { ok = true; break; }
        if (frozenMacros.size() > 100)
        { macroRecordFailure(&context, QStringLiteral("macro-count-limit"), QStringLiteral("设备最多支持100条宏。")); break; }
        if (targetModel != QStringLiteral("AMT070W124-L800X480R") && targetModel != QStringLiteral("AMZ070W01"))
        { macroRecordFailure(&context, QStringLiteral("unsupported-model"), QStringLiteral("型号%1的宏内存配置尚未核定。").arg(targetModel.left(256))); break; }
        for (macro = frozenMacros.constBegin(); macro != frozenMacros.constEnd(); ++macro)
        {
            context.macroName = macro.key();
            context.macroIndex = index;
            if (macro.key().trimmed().isEmpty() || macro.key().size() > 1024 ||
                macro.value().size() > 1024 * 1024 || macro.value().toUtf8().size() > 1024 * 1024)
            { macroRecordFailure(&context, QStringLiteral("source-limit"), QStringLiteral("宏名称为空/超过1024字符，或源代码超过1 MiB：%1。").arg(macro.key().left(256))); break; }
            source = macro.value().toLocal8Bit();
            utf16.clear();
            utf16.reserve(macro.value().size() * 2);
            Q_FOREACH (character, macro.value())
            {
                unit = character.unicode();
                utf16.append(char(unit & 0xffU));
                utf16.append(char(unit >> 8));
            }
            row = QJsonObject{{QStringLiteral("macroName"), macro.key().left(1024)},
                {QStringLiteral("compiledIndex"), index}, {QStringLiteral("qstringUtf16CodeUnits"), macro.value().size()},
                {QStringLiteral("qstringUtf16LeSha256"), macroSha(utf16)},
                {QStringLiteral("sourceUtf8Bytes"), macro.value().toUtf8().size()},
                {QStringLiteral("sourceUtf8Sha256"), macroSha(macro.value().toUtf8())},
                {QStringLiteral("actualAnsiBytes"), source.size()}, {QStringLiteral("actualAnsiSha256"), macroSha(source)},
                {QStringLiteral("ansiRoundTripExact"), QString::fromLocal8Bit(source) == macro.value()}};
            context.sources.append(row);
            if (QString::fromLocal8Bit(source) != macro.value())
            { macroRecordFailure(&context, QStringLiteral("ansi-conversion-loss"), QStringLiteral("宏源代码无法无损转换为当前系统ANSI编码。")); break; }
            ++index;
        }
        if (!context.firstCode.isEmpty()) { break; }
        context.macroName.clear();
        context.macroIndex = -1;
        if (!validateDeviceMacroSdkDirectory(toolchainDirectory, &detail))
        { macroRecordFailure(&context, QStringLiteral("sdk-validation-failed"), detail); break; }
        sdk.setPath(QFileInfo(toolchainDirectory).canonicalFilePath());
        context.phase = QStringLiteral("sdk-identity-before");
        if (!macroSdkManifest(sdk, &context, &sdkFiles)) { break; }
        sdkIdentity = QJsonObject{{QStringLiteral("directory"), sdk.absolutePath()},
            {QStringLiteral("requiredFiles"), sdkFiles}, {QStringLiteral("requiredFileCount"), macroRequiredFiles().size()},
            {QStringLiteral("fullyHashed"), true}, {QStringLiteral("vendorAuthenticated"), false},
            {QStringLiteral("identitySha256"), macroSha(QJsonDocument(sdkFiles).toJson(QJsonDocument::Compact))}};
        context.report.insert(QStringLiteral("sdk"), sdkIdentity);
        rootPath = options.temporaryRoot.isEmpty() ? QDir::tempPath() : options.temporaryRoot;
        rootPath = QFileInfo(rootPath).canonicalFilePath();
        if (rootPath.isEmpty() || !QFileInfo(rootPath).isDir() || rootPath == sdk.absolutePath() ||
            rootPath.startsWith(sdk.absolutePath() + QDir::separator(), Qt::CaseInsensitive))
        { macroRecordFailure(&context, QStringLiteral("temporary-root-invalid"), QStringLiteral("宏临时根目录无效或位于只读SDK目录内。")); break; }
        directory.reset(new QTemporaryDir(QDir(rootPath).filePath(QStringLiteral("kinseal-macro-XXXXXX"))));
        if (!directory->isValid())
        { macroRecordFailure(&context, QStringLiteral("temporary-directory-failed"), QStringLiteral("无法创建自有宏临时目录。")); break; }
        directory->setAutoRemove(false);
        context.workDirectory = directory->path();
        context.report.insert(QStringLiteral("temporaryRoot"), rootPath);
        context.report.insert(QStringLiteral("workingDirectory"), context.workDirectory);
        header = QByteArray("#ifndef _FUNCTION_\n#define _FUNCTION_\n#include <math.h>\n"
            "#include <stdio.h>\n#include <stdlib.h>\n"
            "#define LocalBit ((volatile char*)*(unsigned int*)0x804c5010)\n"
            "#define LocalWord ((volatile short*)*(unsigned int*)0x804c5020)\n"
            "#define RWIWord ((volatile short*)*(unsigned int*)0x804c5030)\n#endif\n");
        context.report.insert(QStringLiteral("generatedHeaderSha256"), macroSha(header));
        file.setFileName(QDir(directory->path()).filePath(QStringLiteral("macro.h")));
        if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly) || file.write(header) != header.size())
        { macroRecordFailure(&context, QStringLiteral("header-write-failed"), QStringLiteral("无法写入自有宏头文件。")); break; }
        file.close();
        prepared.marcoRun = QByteArray(1024, char(0));
        prepared.marcoRun[0] = char(frozenMacros.size());
        prepared.marcoRun[1] = char(0xff);
        prepared.marcoVar = QByteArray(frozenMacros.size() * 904, char(0));
        index = 0;
        for (macro = frozenMacros.constBegin(); macro != frozenMacros.constEnd(); ++macro)
        {
            context.macroIndex = index;
            context.macroName = macro.key();
            context.phase = QStringLiteral("source-write");
            if (!macroCheckpoint(&context)) { break; }
            fileName = QStringLiteral("macro_%1").arg(index, 3, 10, QLatin1Char('0'));
            label = QStringLiteral("宏“%1”").arg(macro.key());
            source = macro.value().toLocal8Bit();
            file.setFileName(QDir(directory->path()).filePath(fileName + QStringLiteral(".c")));
            if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly) || file.write(source) != source.size())
            { macroRecordFailure(&context, QStringLiteral("source-write-failed"), label + QStringLiteral("：无法写入临时源文件。")); break; }
            file.close();
            arguments = {QStringLiteral("-O2"), QStringLiteral("-g+"), QStringLiteral("-cpu"), QStringLiteral("ARM920T"),
                QStringLiteral("-I"), sdk.absolutePath(), QStringLiteral("-I"), sdk.filePath(QStringLiteral("Include")),
                QStringLiteral("-c"), QStringLiteral("-o"), fileName + QStringLiteral(".o"), fileName + QStringLiteral(".c")};
            if (!macroRunProcess(sdk, directory->path(), QStringLiteral("armcc.exe"), arguments, label + QStringLiteral(" 编译"), &context, &prepared.diagnostics)) { break; }
            rwAddress = macroRwBase + u32(index) * 1024U;
            arguments = {QStringLiteral("-ro-base"), QStringLiteral("0x%1").arg(roAddress, 0, 16),
                QStringLiteral("-rw-base"), QStringLiteral("0x%1").arg(rwAddress, 0, 16),
                QStringLiteral("-entry"), QStringLiteral("0x%1").arg(roAddress, 0, 16),
                QStringLiteral("-map"), QStringLiteral("-symbols"), QStringLiteral("-list"), fileName + QStringLiteral(".map"),
                QStringLiteral("-libpath"), sdk.filePath(QStringLiteral("Lib")), QStringLiteral("-o"),
                fileName + QStringLiteral(".axf"), fileName + QStringLiteral(".o"),
                sdk.filePath(QStringLiteral("guilib70_amz.a")), sdk.filePath(QStringLiteral("script_amzlib.a"))};
            if (!macroRunProcess(sdk, directory->path(), QStringLiteral("armlink.exe"), arguments, label + QStringLiteral(" 链接"), &context, &prepared.diagnostics)) { break; }
            context.phase = QStringLiteral("linked-image-validation");
            if (!macroReadFile(QDir(directory->path()).filePath(fileName + QStringLiteral(".axf")), 16 * 1024 * 1024, &image, &detail))
            { macroRecordFailure(&context, QStringLiteral("linked-image-read-failed"), detail); break; }
            if (!macroHasEntry(image, roAddress))
            { macroRecordFailure(&context, QStringLiteral("missing-entry"), label + QStringLiteral("：ARM镜像缺少首地址Macro_Entry；辅助函数定义应放在入口之后。")); break; }
            if (!macroHasBoundedRw(image, rwAddress))
            { macroRecordFailure(&context, QStringLiteral("rw-slot-overflow"), label + QStringLiteral("：RW/ZI数据超过每宏1024字节内存槽。")); break; }
            arguments = {QStringLiteral("-c"), QStringLiteral("-bin"), QStringLiteral("-output"),
                fileName + QStringLiteral(".bin"), fileName + QStringLiteral(".axf")};
            if (!macroRunProcess(sdk, directory->path(), QStringLiteral("fromelf.exe"), arguments, label + QStringLiteral(" 提取"), &context, &prepared.diagnostics)) { break; }
            context.phase = QStringLiteral("raw-image-validation");
            if (!macroReadFile(QDir(directory->path()).filePath(fileName + QStringLiteral(".bin")), 65535, &binary, &detail))
            { macroRecordFailure(&context, QStringLiteral("raw-image-read-failed"), detail); break; }
            if (binary.left(4) == QByteArray::fromHex("7f454c46") || 1024 + bodies.size() + binary.size() > 0x20000)
            { macroRecordFailure(&context, QStringLiteral("raw-package-limit"), label + QStringLiteral("：宏机器码格式无效或运行包超过128 KiB。")); break; }
            macroWriteU16(&prepared.marcoRun, 102 + index * 2, u16(binary.size()));
            bodies.append(binary);
            prepared.macroIndices.insert(macro.key(), u8(index));
            row = QJsonObject{{QStringLiteral("macroName"), macro.key()}, {QStringLiteral("compiledIndex"), index},
                {QStringLiteral("roAddress"), double(roAddress)}, {QStringLiteral("rwAddress"), double(rwAddress)},
                {QStringLiteral("rawBytes"), binary.size()}, {QStringLiteral("rawSha256"), macroSha(binary)},
                {QStringLiteral("linkedElfSha256"), macroSha(image)}, {QStringLiteral("addressVariableCount"), 0}};
            context.mappings.append(row);
            prepared.diagnostics.append(label + QStringLiteral("：%1字节，RO=0x%2，RW=0x%3，地址变量=0。")
                .arg(binary.size()).arg(roAddress, 0, 16).arg(rwAddress, 0, 16));
            roAddress += (u32(binary.size()) + 1023U) & ~1023U;
            ++index;
        }
        if (!context.firstCode.isEmpty()) { break; }
        context.phase = QStringLiteral("sdk-identity-after");
        context.macroName.clear();
        context.macroIndex = -1;
        if (!macroSdkManifest(sdk, &context, &sdkAfter)) { break; }
        if (sdkAfter != sdkFiles)
        { macroRecordFailure(&context, QStringLiteral("sdk-changed"), QStringLiteral("SDK必需文件在编译期间发生变化，未发布运行包。")); break; }
        sdkIdentity.insert(QStringLiteral("recheckedUnchangedAfterCompile"), true);
        context.report.insert(QStringLiteral("sdk"), sdkIdentity);
        if (!macroCheckpoint(&context)) { break; }
        prepared.marcoRun.append(bodies);
        context.report.insert(QStringLiteral("marcoRunBytes"), prepared.marcoRun.size());
        context.report.insert(QStringLiteral("marcoRunSha256"), macroSha(prepared.marcoRun));
        context.report.insert(QStringLiteral("marcoVarBytes"), prepared.marcoVar.size());
        context.report.insert(QStringLiteral("marcoVarSha256"), macroSha(prepared.marcoVar));
        stable = QJsonObject{{QStringLiteral("sdkIdentitySha256"), sdkIdentity.value(QStringLiteral("identitySha256"))},
            {QStringLiteral("targetModel"), targetModel}, {QStringLiteral("sources"), context.sources},
            {QStringLiteral("macros"), context.mappings}, {QStringLiteral("headerSha256"), macroSha(header)},
            {QStringLiteral("marcoRunSha256"), macroSha(prepared.marcoRun)}, {QStringLiteral("marcoVarSha256"), macroSha(prepared.marcoVar)},
            {QStringLiteral("ansiCodePage"), context.report.value(QStringLiteral("ansiCodePage"))}};
        context.report.insert(QStringLiteral("reproducibilityKeySha256"), macroSha(QJsonDocument(stable).toJson(QJsonDocument::Compact)));
        ok = true;
    } while (false);
    file.close();
    if (directory && directory->isValid())
    {
        cleaned = directory->remove();
        if (!cleaned)
        {
            context.phase = QStringLiteral("cleanup");
            macroRecordFailure(&context, QStringLiteral("temporary-cleanup-failed"), QStringLiteral("自有宏临时目录未清理完，保留路径：%1").arg(directory->path()));
            ok = false;
        }
    }
    context.report.insert(QStringLiteral("temporaryCleanupSucceeded"), cleaned);
    context.report.insert(QStringLiteral("elapsedMs"), double(context.timer.elapsed()));
    context.report.insert(QStringLiteral("sources"), context.sources);
    context.report.insert(QStringLiteral("macros"), context.mappings);
    context.report.insert(QStringLiteral("stages"), context.stages);
    context.report.insert(QStringLiteral("diagnostics"), context.diagnostics);
    context.report.insert(QStringLiteral("capturedOutputBytes"), double(context.retainedOutput));
    context.report.insert(QStringLiteral("ok"), ok);
    if (report != nullptr) { *report = context.report; }
    if (!ok)
    {
        if (error != nullptr) { *error = context.firstError; }
        return false;
    }
    prepared.compileManifest = context.report;
    prepared.structuredDiagnostics = context.diagnostics;
    *package = prepared;
    return true;
}
