#ifndef DEVICE_MACRO_COMPILER_H
#define DEVICE_MACRO_COMPILER_H

#include <QByteArray>
#include <QMap>
#include <QString>
#include <QStringList>

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

QString deviceMacroSdkDirectory(); //20261109-0 add
bool validateDeviceMacroSdkDirectory(const QString &path, QString *error); //20261109-0 add

#endif
