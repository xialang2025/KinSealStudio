#ifndef PLC_ADDRESS_DIALOG_H
#define PLC_ADDRESS_DIALOG_H

#include "plc_address.h"

class QWidget;

/******************************************************************************
 * Function Name   : editPlcConnections
 * Description     : Edit the model's COM1/COM2 PLC connections with transactional confirmation.
 * Input Parameters: parent, modelName, connections, error
 * Return Value    : True only for a validated confirmation with the modal owner still alive.
 * Date            : 2026.11.09
 ******************************************************************************/
bool editPlcConnections(QWidget *parent, const QString &modelName,
                         QVector<PlcConnection> *connections,
                         QString *error = nullptr); //20261109-0 add //20261114-8 change

/******************************************************************************
 * Function Name   : editPlcAddress
 * Description     : Edit internal or explicitly qualified external bit/word addresses.
 * Input Parameters: parent, bit, connections, qualifiedText, transferCount (explicit 1..2 span)
 * Return Value    : True only after parser validation; cancellation or owner loss preserves output.
 * Date            : 2026.11.09
 ******************************************************************************/
bool editPlcAddress(QWidget *parent, bool bit, const QVector<PlcConnection> &connections,
                    QString *qualifiedText, int transferCount = 1); //20261109-6 change //20261114-8 change

#endif
