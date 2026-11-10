#ifndef DEVICE_SCAN_TABLE_H
#define DEVICE_SCAN_TABLE_H

#include <QByteArray>
#include <QString>
#include <QVector>

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

//20261109-0 add: Counts and starts use the native address encoder's units before scan alignment.
struct DeviceScanRead
{
    u8 type = 0;
    u8 route = 0;
    u8 node = 0;
    u32 start = 0;
    u32 count = 0;
};

/******************************************************************************
 * Function Name   : buildDeviceScanTable
 * Description     : Build one page of verified Modbus RTU scan segments with clean native padding.
 * Input Parameters: reads, scanTable, error, maximumReadWords
 * Return Value    : True with a complete 200-byte page header and twelve-byte read segments.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildDeviceScanTable(const QVector<DeviceScanRead> &reads, QByteArray *scanTable,
                          QString *error, u32 maximumReadWords = 32); //20261109-0 add

#endif
