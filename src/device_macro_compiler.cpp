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
    *bytes = file.readAll();
    if (bytes->size() != file.size() || file.error() != QFileDevice::NoError)
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

/******************************************************************************
 * Function Name   : macroRunProcess
 * Description     : Run one hidden SDK stage with bounded elapsed time and diagnostics.
 * Input Parameters: sdk, workDirectory, executable, arguments, label, timer, diagnostics, error
 * Return Value    : True only after a normal zero exit status.
 * Date            : 2026.11.09
 ******************************************************************************/
static bool macroRunProcess(const QDir &sdk, const QString &workDirectory,
                            const QString &executable, const QStringList &arguments,
                            const QString &label, QElapsedTimer *timer,
                            QStringList *diagnostics, QString *error) //20261109-0 add
{
    QProcess process;
    QProcessEnvironment environment;
    QElapsedTimer stageTimer;
    QByteArray output;
    QString detail;
    int remaining;

    remaining = macroTotalTimeout - int(timer->elapsed());
    if (remaining <= 0)
    {
        return macroFailure(error, label + QStringLiteral("：宏编译总耗时超过 120 秒。"));
    }
    environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LM_LICENSE_FILE"), sdk.filePath(QStringLiteral("licenses/LICENSE.DAT")));
    environment.insert(QStringLiteral("ARMLIB"), sdk.filePath(QStringLiteral("Lib/armlib")));
    process.setProcessEnvironment(environment);
    process.setWorkingDirectory(workDirectory);
    process.setProgram(sdk.filePath(executable));
    process.setArguments(arguments);
    process.setProcessChannelMode(QProcess::MergedChannels);
#ifdef Q_OS_WIN
    process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *arguments) {
        arguments->flags |= CREATE_NO_WINDOW; //20261109-0 add: SDK tools must not open or activate a console.
    });
#endif
    stageTimer.start();
    process.start();
    if (!process.waitForStarted(qMin(remaining, 5000)))
    {
        process.kill();
        process.waitForFinished(3000);
        return macroFailure(error, label + QStringLiteral("：无法启动 %1：%2")
                                               .arg(executable, process.errorString()));
    }
    while (process.state() != QProcess::NotRunning)
    {
        process.waitForFinished(100);
        output.append(process.readAll());
        if (output.size() > macroLogLimit || stageTimer.elapsed() >= macroProcessTimeout ||
            timer->elapsed() >= macroTotalTimeout)
        {
            process.kill();
            process.waitForFinished(3000);
            detail = output.size() > macroLogLimit ? QStringLiteral("诊断输出超过 256 KiB。") :
                                                    QStringLiteral("编译进程超时（单步 30 秒，总计 120 秒）。");
            return macroFailure(error, label + QStringLiteral("：") + detail + QLatin1Char('\n') +
                                           QString::fromLocal8Bit(output.left(macroLogLimit)));
        }
    }
    output.append(process.readAll());
    detail = QString::fromLocal8Bit(output.left(macroLogLimit)).trimmed();
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
    {
        return macroFailure(error, label + QStringLiteral("：%1 失败（退出码 %2）。\n%3")
                                               .arg(executable).arg(process.exitCode()).arg(detail));
    }
    if (!detail.isEmpty())
    {
        diagnostics->append(label + QStringLiteral("：\n") + detail);
    }
    return true;
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
 * Description     : Build verified raw ARM images and native zero-variable macro containers.
 * Input Parameters: project, toolchainDirectory, package, error
 * Return Value    : True after every macro and package bound was validated.
 * Date            : 2026.11.09
 ******************************************************************************/
bool compileDeviceMacros(const HmiProject &project, const QString &toolchainDirectory,
                         DeviceMacroPackage *package, QString *error) //20261109-0 add
{
    DeviceMacroPackage prepared;
    QMap<QString, QString>::const_iterator macro;
    QDir sdk;
    QStringList arguments;
    QString fileName;
    QString label;
    QByteArray header;
    QByteArray source;
    QByteArray image;
    QByteArray binary;
    QByteArray bodies;
    QFile file;
    QElapsedTimer timer;
    QTemporaryDir directory;
    u32 roAddress;
    u32 rwAddress;
    int index;

    if (error != nullptr)
    {
        error->clear();
    }
    if (package == nullptr)
    {
        return macroFailure(error, QStringLiteral("宏编译输出参数为空。"));
    }
    if (project.macros.isEmpty())
    {
        *package = prepared;
        return true;
    }
    if (project.macros.size() > 100)
    {
        return macroFailure(error, QStringLiteral("设备最多支持 100 条宏。"));
    }
    if (project.targetModel != QStringLiteral("AMT070W124-L800X480R") &&
        project.targetModel != QStringLiteral("AMZ070W01"))
    {
        return macroFailure(error, QStringLiteral("型号 %1 的宏内存配置尚未核定。")
                                       .arg(project.targetModel));
    }
    for (macro = project.macros.constBegin(); macro != project.macros.constEnd(); ++macro)
    {
        if (macro.key().trimmed().isEmpty() || macro.value().toUtf8().size() > 1024 * 1024)
        {
            return macroFailure(error, QStringLiteral("宏名称为空，或源代码超过 1 MiB：%1。")
                                           .arg(macro.key()));
        }
    }
    if (!validateDeviceMacroSdkDirectory(toolchainDirectory, error)) { return false; } //20261109-0 change
    sdk.setPath(QFileInfo(toolchainDirectory).canonicalFilePath());
    if (!directory.isValid())
    {
        return macroFailure(error, QStringLiteral("无法创建宏编译临时目录：%1。")
                                       .arg(directory.errorString()));
    }
    header = QByteArray("#ifndef _FUNCTION_\n#define _FUNCTION_\n#include <math.h>\n"
                        "#include <stdio.h>\n#include <stdlib.h>\n"
                        "#define LocalBit ((volatile char*)*(unsigned int*)0x804c5010)\n"
                        "#define LocalWord ((volatile short*)*(unsigned int*)0x804c5020)\n"
                        "#define RWIWord ((volatile short*)*(unsigned int*)0x804c5030)\n#endif\n");
    file.setFileName(QDir(directory.path()).filePath(QStringLiteral("macro.h")));
    if (!file.open(QIODevice::WriteOnly) || file.write(header) != header.size())
    {
        return macroFailure(error, QStringLiteral("无法写入宏头文件：%1。")
                                       .arg(file.errorString()));
    }
    file.close();
    prepared.marcoRun = QByteArray(1024, char(0));
    prepared.marcoRun[0] = char(project.macros.size());
    prepared.marcoRun[1] = char(0xff);
    prepared.marcoVar = QByteArray(project.macros.size() * 904, char(0));
    roAddress = macroRoBase;
    index = 0;
    timer.start();
    for (macro = project.macros.constBegin(); macro != project.macros.constEnd(); ++macro)
    {
        fileName = QStringLiteral("macro_%1").arg(index, 3, 10, QLatin1Char('0'));
        label = QStringLiteral("宏“%1”").arg(macro.key());
        source = macro.value().toLocal8Bit();
        if (QString::fromLocal8Bit(source) != macro.value())
        {
            return macroFailure(error, label + QStringLiteral("：源代码无法无损转换为当前系统 ANSI 编码。"));
        }
        file.setFileName(QDir(directory.path()).filePath(fileName + QStringLiteral(".c")));
        if (!file.open(QIODevice::WriteOnly) || file.write(source) != source.size())
        {
            return macroFailure(error, label + QStringLiteral("：无法写入临时源文件：%1。").arg(file.errorString()));
        }
        file.close();
        arguments = {QStringLiteral("-O2"), QStringLiteral("-g+"), QStringLiteral("-cpu"),
                     QStringLiteral("ARM920T"), QStringLiteral("-I"), sdk.absolutePath(),
                     QStringLiteral("-I"), sdk.filePath(QStringLiteral("Include")),
                     QStringLiteral("-c"), QStringLiteral("-o"), fileName + QStringLiteral(".o"),
                     fileName + QStringLiteral(".c")};
        if (!macroRunProcess(sdk, directory.path(), QStringLiteral("armcc.exe"), arguments,
                             label + QStringLiteral(" 编译"), &timer, &prepared.diagnostics, error))
        {
            return false;
        }
        rwAddress = macroRwBase + u32(index) * 1024U;
        arguments = {QStringLiteral("-ro-base"), QStringLiteral("0x%1").arg(roAddress, 0, 16),
                     QStringLiteral("-rw-base"), QStringLiteral("0x%1").arg(rwAddress, 0, 16),
                     QStringLiteral("-entry"), QStringLiteral("0x%1").arg(roAddress, 0, 16),
                     QStringLiteral("-map"), QStringLiteral("-symbols"), QStringLiteral("-list"),
                     fileName + QStringLiteral(".map"), QStringLiteral("-libpath"),
                     sdk.filePath(QStringLiteral("Lib")), QStringLiteral("-o"),
                     fileName + QStringLiteral(".axf"), fileName + QStringLiteral(".o"),
                     sdk.filePath(QStringLiteral("guilib70_amz.a")),
                     sdk.filePath(QStringLiteral("script_amzlib.a"))};
        if (!macroRunProcess(sdk, directory.path(), QStringLiteral("armlink.exe"), arguments,
                             label + QStringLiteral(" 链接"), &timer, &prepared.diagnostics, error) ||
            !macroReadFile(QDir(directory.path()).filePath(fileName + QStringLiteral(".axf")),
                           16 * 1024 * 1024, &image, error))
        {
            return false;
        }
        if (!macroHasEntry(image, roAddress))
        {
            return macroFailure(error, label + QStringLiteral("：ARM 镜像缺少位于首地址的 Macro_Entry；请保留入口，并将辅助函数定义放在入口之后。"));
        }
        if (!macroHasBoundedRw(image, rwAddress))
        {
            return macroFailure(error, label + QStringLiteral("：RW/ZI 数据超过每宏 1024 字节的内存槽；请减少全局或 static 变量。"));
        }
        arguments = {QStringLiteral("-c"), QStringLiteral("-bin"), QStringLiteral("-output"),
                     fileName + QStringLiteral(".bin"), fileName + QStringLiteral(".axf")};
        if (!macroRunProcess(sdk, directory.path(), QStringLiteral("fromelf.exe"), arguments,
                             label + QStringLiteral(" 提取"), &timer, &prepared.diagnostics, error) ||
            !macroReadFile(QDir(directory.path()).filePath(fileName + QStringLiteral(".bin")),
                           65535, &binary, error))
        {
            return false;
        }
        if (binary.left(4) == QByteArray::fromHex("7f454c46") ||
            1024 + bodies.size() + binary.size() > 0x20000)
        {
            return macroFailure(error, label + QStringLiteral("：宏机器码格式无效或总运行包超过型号的 128 KiB 限制。"));
        }
        macroWriteU16(&prepared.marcoRun, 102 + index * 2, u16(binary.size()));
        bodies.append(binary);
        prepared.macroIndices.insert(macro.key(), u8(index));
        prepared.diagnostics.append(label + QStringLiteral("：%1 字节，RO=0x%2，RW=0x%3，地址变量=0。")
                                                .arg(binary.size()).arg(roAddress, 0, 16).arg(rwAddress, 0, 16));
        roAddress += (u32(binary.size()) + 1023U) & ~1023U;
        ++index;
    }
    prepared.marcoRun.append(bodies); //20261109-0 add: Container bytes are contiguous; firmware RO allocation uses 1 KiB alignment.
    *package = prepared;
    return true;
}
