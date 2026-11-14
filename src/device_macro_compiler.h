#ifndef DEVICE_MACRO_COMPILER_H
#define DEVICE_MACRO_COMPILER_H

#include <QByteArray>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QJsonArray> //20261114-7 add
#include <QJsonObject>
#include <functional>

struct HmiProject;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

//20261109-0 add: Macro indices use the same QMap order as the independently compiled images.
struct DeviceMacroPackage
{
    QByteArray marcoRun;
    QByteArray marcoVar;
    QMap<QString, u8> macroIndices;
    QStringList diagnostics;
    QJsonObject compileManifest; //20261114-7 add: Published only with the complete package.
    QJsonArray structuredDiagnostics;
};

//20261114-7 add: Cancellation observes the caller's flag without pumping GUI events.
struct DeviceMacroCompileOptions
{
    int processTimeoutMs = 30000;
    int totalTimeoutMs = 120000;
    int startTimeoutMs = 5000;
    int cleanupTimeoutMs = 2000;
    int maxProcessOutputBytes = 256 * 1024;
    int maxTotalOutputBytes = 1024 * 1024;
    QString temporaryRoot; // Empty uses the process TEMP root; callers can select an owned Z directory.
    std::function<bool()> isCancellationRequested;
};

/******************************************************************************
 * Function Name   : compileDeviceMacros
 * Description     : Compile source-only project macros with an explicitly supplied ARM SDK.
 * Input Parameters: project, toolchainDirectory, package, error
 * Return Value    : True after complete compilation and transactional package construction.
 * Date            : 2026.11.09
 ******************************************************************************/
bool compileDeviceMacros(const HmiProject &project, const QString &toolchainDirectory,
                         DeviceMacroPackage *package, QString *error); //20261109-0 add

/******************************************************************************
 * Function Name   : compileDeviceMacros
 * Description     : Compile a frozen source set with bounded owned processes and an independent failure report.
 * Input Parameters: project, toolchainDirectory, package, error, options, report
 * Return Value    : True after complete package publication; failure preserves package. Report must not alias package.compileManifest.
 * Date            : 2026.11.14
 ******************************************************************************/
bool compileDeviceMacros(const HmiProject &project, const QString &toolchainDirectory,
                         DeviceMacroPackage *package, QString *error,
                         const DeviceMacroCompileOptions &options,
                         QJsonObject *report = nullptr); //20261114-7 add

QString deviceMacroSdkDirectory(); //20261109-0 add
bool validateDeviceMacroSdkDirectory(const QString &path, QString *error); //20261109-0 add

#endif
