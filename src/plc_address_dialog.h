#ifndef PLC_ADDRESS_DIALOG_H
#define PLC_ADDRESS_DIALOG_H

#include "plc_address.h"

class QWidget;

/******************************************************************************
 * Function Name   : editPlcConnections
 * Description     : Edit the model's COM1/COM2 PLC connections with transactional confirmation.
 * Input Parameters: parent, modelName, connections, error
 * Return Value    : True only when the user confirms a valid connection set
 * Date            : 2026.11.09
 ******************************************************************************/
bool editPlcConnections(QWidget *parent, const QString &modelName,
                         QVector<PlcConnection> *connections,
                         QString *error = nullptr); //20261109-0 add

/******************************************************************************
 * Function Name   : editPlcAddress
 * Description     : Edit internal or explicitly qualified external bit/word addresses.
 * Input Parameters: parent, bit, connections, qualifiedText
 * Return Value    : True only after a supported address is confirmed
 * Date            : 2026.11.09
 ******************************************************************************/
bool editPlcAddress(QWidget *parent, bool bit, const QVector<PlcConnection> &connections,
                    QString *qualifiedText, int transferCount = 1); //20261109-6 change

#endif
