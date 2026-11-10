#ifndef DEVICE_TRANSFER_PLAN_H
#define DEVICE_TRANSFER_PLAN_H

#include <QByteArray>
#include <QString>
#include <QVector>

using u8 = unsigned char;

//20261109-0 add: A prepared data step contains no USB handle or device side effect.
struct HmiDownloadStep
{
    u8 command;
    QString name;
    QByteArray options;
    QByteArray bytes;
};

/******************************************************************************
 * Function Name   : buildDeviceTransferPlan
 * Description     : Prepare the observed static or bit-button transfer order offline.
 * Input Parameters: payloads, steps, error
 * Return Value    : True when all commands and wire payloads were prepared.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildDeviceTransferPlan(const QVector<QByteArray> &payloads,
                             QVector<HmiDownloadStep> *steps,
                             QString *error); //20261109-0 add

#endif
