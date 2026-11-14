#ifndef PLC_PROTOCOL_CAPABILITIES_H
#define PLC_PROTOCOL_CAPABILITIES_H

#include <QJsonObject>
#include <QString>

/******************************************************************************
 * Function Name   : plcProtocolCapabilities
 * Description     : Separate observed profile files, address grammar and execution support without accessing an endpoint.
 * Input Parameters: model
 * Return Value    : Per-profile capability report bound to embedded file observations.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject plcProtocolCapabilities(const QString &model = QStringLiteral("AMT070W124-L800X480R")); //20261114-8 add

#endif
