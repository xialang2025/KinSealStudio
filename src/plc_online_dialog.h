#ifndef PLC_ONLINE_DIALOG_H
#define PLC_ONLINE_DIALOG_H

#include "plc_serial_session.h"
#include <QDialog>

class QComboBox;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QThread;
struct PlcOnlineJob;

class PlcOnlineDialog : public QDialog //20261114-8 add
{
    Q_OBJECT
public:
    explicit PlcOnlineDialog(const PlcSerialIoFactory &factory = {}, QWidget *parent = nullptr);
    ~PlcOnlineDialog() override;
    bool running() const;
    QJsonObject lastReport() const;
public slots:
    void executeRequest();
    void cancelRequest();
    void reject() override;
private:
    void setRunning(bool running);
    void updateOperation();
    QComboBox *m_port;
    QComboBox *m_parity;
    QComboBox *m_stop;
    QComboBox *m_operation;
    QSpinBox *m_baud;
    QSpinBox *m_station;
    QSpinBox *m_address;
    QSpinBox *m_count;
    QSpinBox *m_timeout;
    QLineEdit *m_values;
    QPlainTextEdit *m_output;
    QPushButton *m_execute;
    QPushButton *m_cancel;
    QList<QWidget *> m_inputs;
    PlcSerialIoFactory m_factory;
    std::shared_ptr<PlcOnlineJob> m_job;
    QThread *m_thread = nullptr;
    QJsonObject m_report;
    bool m_closeWhenFinished = false;
};

#endif
