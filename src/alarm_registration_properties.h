#ifndef ALARM_REGISTRATION_PROPERTIES_H
#define ALARM_REGISTRATION_PROPERTIES_H
#include "plc_address.h"
#include <QMap>
struct HmiProject;
//20261113-2 add: Stable editor rows have no claim to recovered firmware alarm record identities.
struct AlarmRegistrationRow
{
    bool used = true;
    QMap<QString, QString> messages;
    QString limit = QStringLiteral("0");
    QString jumpPageId;
    QString backPageId;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, AlarmRegistrationRow *output, QString *error = nullptr);
};
struct AlarmRegistrationProperties
{
    int family = 0;
    int bitType = 0;
    int count = 0;
    int scanSeconds = 1;
    int dataType = 0;
    QString readAddress = QStringLiteral("LB0");
    QVector<AlarmRegistrationRow> rows;
    static QStringList dataTypes();
    int rowCount() const;
    u32 readSpan() const;
    void resizeRows(int value, const QStringList &languages);
    bool validate(QString *error = nullptr) const;
    bool validateContext(const HmiProject &project, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, AlarmRegistrationProperties *output, QString *error = nullptr);
};
bool hasAlarmRegistrations(const HmiProject &project);
bool validateAlarmRegistrations(const HmiProject &project, QString *error = nullptr);
#endif
