#ifndef ONLINE_SIMULATION_LINK_H
#define ONLINE_SIMULATION_LINK_H

#include "plc_serial_session.h"
#include "simulation_runtime.h"
#include <QObject>

class QThread;
class OnlineSimulationWorker;

//20261124-13 add: Bridge project addresses to a single PC serial endpoint and a confirmed read cache.
class OnlineSimulationLink : public QObject
{
    Q_OBJECT
public:
    OnlineSimulationLink(const HmiProject &project, const PlcSerialIoFactory &factory, QObject *parent);
    ~OnlineSimulationLink() override;

    SimulationExternalAccess access();
    bool available() const { return !m_scans.isEmpty(); }
    bool active() const { return m_active; }
    PlcSerialSettings settings() const { return m_settings; }
    int station() const { return m_station; }
    QString status() const { return m_status; }
    bool connectPort(const QString &port, QString *error);
    void disconnectPort();

signals:
    void updated();

private:
    friend class OnlineSimulationWorker;
    struct Binding
    {
        SimulationModbusRequest scan;
        int registerBit = -1;
        bool writable = false;
    };

    bool bind(const QString &text, bool bit, int span, bool write, QString *error);
    bool read(const QString &text, bool bit, int span, QVector<u16> *values, QString *error) const;
    bool queueWrite(const QString &text, bool bit, const QVector<u16> &values, bool toggle, QString *error);
    void receive(const SimulationModbusResult &result, bool write);
    void failed(const QString &error);

    PlcAddressContext m_context;
    PlcCatalog m_catalog;
    QString m_catalogError;
    QList<HmiConfigurationNode> m_configuration;
    QString m_connectionId;
    QMap<QString, Binding> m_bindings;
    QMap<QString, SimulationModbusRequest> m_scans;
    QMap<QString, QVector<u16>> m_values;
    PlcSerialSettings m_settings;
    PlcSerialIoFactory m_factory;
    QThread *m_thread = nullptr;
    OnlineSimulationWorker *m_worker = nullptr;
    std::shared_ptr<std::atomic_bool> m_cancel;
    QString m_status;
    int m_station = 0;
    int m_frameInterval = 20;
    bool m_active = false;
    bool m_writePending = false;
};

#endif
