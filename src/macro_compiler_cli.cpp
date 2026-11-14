#include "macro_compiler_cli.h"
#include "device_macro_compiler.h"
#include "device_transfer_contract.h"
#include "project_model.h"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>
#include <cstdio>

/******************************************************************************
 * Function Name   : handleMacroCompilerCli
 * Description     : Freeze the Lite source before invoking an explicit SDK and publish only complete successful macro packages.
 * Input Parameters: arguments, exitCode
 * Return Value    : True for --compile-macros, including handled failures.
 * Date            : 2026.11.14
 ******************************************************************************/
bool handleMacroCompilerCli(const QStringList &arguments, int *exitCode) //20261114-7 add
{
    QFile source;
    QByteArray bytes;
    QByteArray encoded;
    QJsonParseError parseError;
    QJsonDocument document;
    QJsonObject result;
    QJsonObject report;
    QJsonObject indices;
    QSaveFile output;
    HmiProject project;
    DeviceMacroPackage package;
    DeviceMacroCompileOptions options;
    QString error;
    bool ok = false;

    if (arguments.size() < 2 || arguments.at(1) != QStringLiteral("--compile-macros")) { return false; }
    *exitCode = 26;
    do
    {
        if (arguments.size() != 4)
        { error = QStringLiteral("用法：--compile-macros 工程.ksp 报告.json；SDK由KINSEAL_ARM_SDK或工具链设置选择。"); break; }
        if (QFileInfo(arguments.at(3)).suffix().compare(QStringLiteral("json"), Qt::CaseInsensitive) != 0 ||
            transferContractWouldReplaceInput(arguments.at(2), arguments.at(3)))
        { error = QStringLiteral("宏报告必须为独立.json文件，不能覆盖输入工程。"); break; }
        source.setFileName(arguments.at(2));
        if (!source.open(QIODevice::ReadOnly) || source.size() > 16 * 1024 * 1024)
        { error = QStringLiteral("无法读取工程，或超过16MiB输入保护。"); break; }
        bytes = source.read(16 * 1024 * 1024 + 1);
        if (source.error() != QFileDevice::NoError || !source.atEnd() || bytes.size() > 16 * 1024 * 1024)
        { error = QStringLiteral("工程读取不完整或超限。"); break; }
        source.close();
        document = QJsonDocument::fromJson(bytes, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject() || !HmiProject::fromJson(document.object(), &project, &error))
        { if (error.isEmpty()) { error = QStringLiteral("宏编译输入需要完整Lite JSON工程。"); } break; }
        options.temporaryRoot = qEnvironmentVariable("KINSEAL_MACRO_TEMP_ROOT");
        if (options.temporaryRoot.isEmpty()) { options.temporaryRoot = QDir::tempPath(); }
        if (!compileDeviceMacros(project, deviceMacroSdkDirectory(), &package, &error, options, &report)) { break; }
        result.insert(QStringLiteral("compiler"), report);
        result.insert(QStringLiteral("sourceSha256"), QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex()));
        result.insert(QStringLiteral("marcoRunBase64"), QString::fromLatin1(package.marcoRun.toBase64()));
        result.insert(QStringLiteral("marcoVarBase64"), QString::fromLatin1(package.marcoVar.toBase64()));
        for (auto entry = package.macroIndices.constBegin(); entry != package.macroIndices.constEnd(); ++entry)
        { indices.insert(entry.key(), entry.value()); }
        result.insert(QStringLiteral("macroIndices"), indices);
        result.insert(QStringLiteral("format"), QStringLiteral("KinSealLite-ordinary-macro-report-v1"));
        result.insert(QStringLiteral("ok"), true);
        result.insert(QStringLiteral("hardwareIo"), false);
        result.insert(QStringLiteral("hmiExecutionConfirmed"), false);
        encoded = QJsonDocument(result).toJson(QJsonDocument::Indented);
        if (encoded.size() > 8 * 1024 * 1024) { error = QStringLiteral("宏报告超过8MiB保护，未覆盖输出。"); break; }
        output.setFileName(arguments.at(3));
        if (transferContractWouldReplaceInput(arguments.at(2), arguments.at(3)) || !output.open(QIODevice::WriteOnly) ||
            output.write(encoded) != encoded.size() || !output.commit())
        { error = QStringLiteral("宏报告未完整发布：%1").arg(output.errorString()); break; }
        result.remove(QStringLiteral("marcoRunBase64")); result.remove(QStringLiteral("marcoVarBase64"));
        result.insert(QStringLiteral("output"), QFileInfo(arguments.at(3)).absoluteFilePath());
        ok = true;
    } while (false);
    if (!ok)
    {
        result = QJsonObject();
        result.insert(QStringLiteral("message"), error.left(8192));
        if (QJsonDocument(report).toJson(QJsonDocument::Compact).size() <= 2 * 1024 * 1024)
        { result.insert(QStringLiteral("compiler"), report); }
    }
    result.insert(QStringLiteral("ok"), ok);
    result.insert(QStringLiteral("hardwareIo"), false);
    result.insert(QStringLiteral("hmiExecutionConfirmed"), false);
    encoded = QJsonDocument(result).toJson(QJsonDocument::Compact) + '\n';
    std::fwrite(encoded.constData(), 1, size_t(encoded.size()), stdout); std::fflush(stdout);
    if (ok) { *exitCode = 0; }
    return true;
}
