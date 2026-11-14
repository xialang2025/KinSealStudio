#include "plc_protocol_cli.h"
#include "plc_protocol_capabilities.h"
#include <QJsonDocument>
#include <cstdio>

/******************************************************************************
 * Function Name   : handlePlcProtocolCli
 * Description     : Print an embedded capability audit without discovering or opening any communication endpoint.
 * Input Parameters: arguments, exitCode
 * Return Value    : True for a handled capability command, false for another command.
 * Date            : 2026.11.14
 ******************************************************************************/
bool handlePlcProtocolCli(const QStringList &arguments, int *exitCode) //20261114-8 add
{
    QByteArray bytes;
    QJsonObject report;
    bool valid;

    if (arguments.size() < 2 || arguments.at(1) != QStringLiteral("--plc-capabilities")) { return false; }
    valid = arguments.size() == 2 || (arguments.size() == 3 && !arguments.at(2).trimmed().isEmpty());
    report = valid ? plcProtocolCapabilities(arguments.size() == 3 ? arguments.at(2) : QStringLiteral("AMT070W124-L800X480R")) :
        QJsonObject{{QStringLiteral("error"), QStringLiteral("Usage: --plc-capabilities [model]")}};
    bytes = QJsonDocument(report).toJson(QJsonDocument::Compact) + '\n';
    valid = valid && report.value(QStringLiteral("ok")).toBool(true);
    valid = std::fwrite(bytes.constData(), 1, size_t(bytes.size()), stdout) == size_t(bytes.size()) && valid;
    valid = std::fflush(stdout) == 0 && valid; //20261114-8 change: Buffered output failure must not publish a successful CLI exit.
    if (exitCode != nullptr) { *exitCode = valid ? 0 : 2; }
    return true;
}
