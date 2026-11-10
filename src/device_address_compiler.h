#ifndef DEVICE_ADDRESS_COMPILER_H
#define DEVICE_ADDRESS_COMPILER_H

#include "project_model.h"

//20261109-0 add: The fixed device image contains the independently verified default COM1 driver.
class DeviceAddressCompiler
{
public:
    bool prepare(const HmiProject &project, QByteArray *scanTable, QString *error);
    bool encode(const QString &text, bool word, bool toggle, QByteArray *record,
                QString *error, u32 transferCount = 1) const;

private:
    PlcCatalog m_catalog;
    PlcAddressContext m_context;
    bool m_ready = false;
};

#endif
