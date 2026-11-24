#ifndef SIMULATION_RUNTIME_H
#define SIMULATION_RUNTIME_H

#include "project_model.h"

#include <QJsonArray>
#include <QVector>
#include <functional>

//20261124-13 add: External values come only from acknowledged serial reads.
struct SimulationExternalAccess
{
    std::function<bool(const QString &, bool, int, bool, QString *)> check;
    std::function<bool(const QString &, bool, int, QVector<u16> *, QString *)> read;
    std::function<bool(const QString &, bool, const QVector<u16> &, bool, QString *)> write;
};

//20261114-6 add: This value owner has no timer, driver, process, device or physical I/O dependency.
class SimulationRuntime
{
public:
    bool start(const HmiProject &project, QString *error = nullptr, const SimulationExternalAccess &external = {}); //20261124-13 change
    bool running() const;
    bool online() const { return bool(m_external.check); } //20261124-13 add
    void refreshExternalValues();
    QString numberDisplayValue(const HmiObject &object) const;
    void stop(const QString &reason = QString());
    const HmiProject &project() const;
    QString language() const;
    int userLevel() const;
    bool setUserLevel(int level, QString *error = nullptr);
    bool writeBit(const QString &address, bool value, QString *error = nullptr);
    bool readBit(const QString &address, bool *value, QString *error = nullptr) const;
    bool writeWord(const QString &address, int type, const QString &value, QString *error = nullptr);
    bool readWord(const QString &address, int type, QString *value, QString *error = nullptr) const;
    bool press(const QString &id, QString *error = nullptr);
    bool release(bool inside, QString *error = nullptr);
    void cancel(const QString &reason = QString());
    bool submitInput(const QString &value, QString *error = nullptr);
    QString pendingInputObjectId() const;
    bool isVisible(const HmiObject &object) const;
    bool touchAllowed(const HmiObject &object) const;
    int visualState(const HmiObject &object) const;
    bool isPressed(const QString &id) const;
    QJsonObject snapshot() const;
    QJsonArray events() const;
    static QJsonObject capabilities(const HmiProject &project, const SimulationExternalAccess &external = {}); //20261124-13 change

private:
    const HmiObject *findObject(const QString &id) const;
    bool condition(const StaticTextProperties &properties) const;
    bool applyAction(const HmiObject &object, const QString *input, QString *error);
    void appendEvent(const QString &kind, const QString &id = QString(), const QString &detail = QString());
    void enforceAccess();

    bool readNumeric(const QString &address, int type, double *value, QString *error) const; //20261124-13 add
    SimulationExternalAccess m_external;
    HmiProject m_project;
    QVector<u8> m_bits = QVector<u8>(4097, 0);
    QVector<u16> m_words = QVector<u16>(4097, 0);
    QJsonArray m_events;
    QJsonArray m_warnings;
    QString m_language;
    QString m_pressed;
    QString m_pendingInput;
    int m_momentaryIndex = -1;
    int m_userLevel = 0;
    u32 m_sequence = 0;
    bool m_running = false;
};

#endif
