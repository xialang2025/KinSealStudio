#ifndef PLC_SERIAL_SESSION_H
#define PLC_SERIAL_SESSION_H

#include "simulation_modbus_transport.h"
#include <atomic>
#include <memory>

//20261114-8 add: PC serial parameters never overwrite an HMI connection or infer a native profile mapping.
struct PlcSerialSettings
{
    QString portName;
    int baudRate = 9600;
    int dataBits = 8;
    int parity = 0;                 // QSerialPort values: 0 none, 2 even, 3 odd.
    int stopBits = 1;
    int timeoutMs = 1000;           // Explicit Lite defaults, not inferred from an INI enum.
    int totalTimeoutMs = 120000;
};

using PlcSerialIoFactory = std::function<SimulationModbusIo(const PlcSerialSettings &,
                                                            const std::function<bool()> &)>;

/******************************************************************************
 * Function Name   : validatePlcSerialSettings
 * Description     : Reject invalid explicit PC port parameters before creating a transport.
 * Input Parameters: settings, error
 * Return Value    : True for the bounded eight-data-bit RTU parameter subset.
 * Date            : 2026.11.14
 ******************************************************************************/
bool validatePlcSerialSettings(const PlcSerialSettings &settings, QString *error = nullptr); //20261114-8 add

/******************************************************************************
 * Function Name   : runPlcSerialBatch
 * Description     : Execute a frozen manual RTU batch on one owned endpoint, then close it without retries.
 * Input Parameters: settings, station, requests, cancellation, factory
 * Return Value    : Per-request receipts and separate physical-PC/HMI evidence flags.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject runPlcSerialBatch(const PlcSerialSettings &settings, int station,
                              const QVector<SimulationModbusRequest> &requests,
                              const std::shared_ptr<std::atomic_bool> &cancellation,
                              const PlcSerialIoFactory &factory = {}); //20261114-8 add

#endif
