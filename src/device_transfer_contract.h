#ifndef DEVICE_TRANSFER_CONTRACT_H
#define DEVICE_TRANSFER_CONTRACT_H

#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

/******************************************************************************
 * Function Name   : buildDeviceTransferContract
 * Description     : Describe the observed wire sequence without acquiring USB resources.
 * Input Parameters: payloads, identity, contract, error, startOptions
 * Return Value    : True with a complete immutable command and payload manifest.
 * Date            : 2026.11.14
 ******************************************************************************/
bool buildDeviceTransferContract(const QVector<QByteArray> &payloads,
                                 const QJsonObject &identity, QJsonObject *contract,
                                 QString *error,
                                 const QByteArray &startOptions = QByteArray::fromHex("000000")); //20261114-5 add

/******************************************************************************
 * Function Name   : transferContractWouldReplaceInput
 * Description     : Reject case aliases, resolved links, and existing Windows file identities.
 * Input Parameters: input, output
 * Return Value    : True when output could replace the input file.
 * Date            : 2026.11.14
 ******************************************************************************/
bool transferContractWouldReplaceInput(const QString &input, const QString &output); //20261114-5 add

#endif
