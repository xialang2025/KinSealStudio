#ifndef ALARM_BAR_PROPERTIES_H
#define ALARM_BAR_PROPERTIES_H
#include "screen_button_properties.h"
//20261113-2 add: Alarm text comes from global registrations; this display owns no operation address.
struct AlarmBarProperties
{
    QString frameColor = QStringLiteral("#e0e0e0");
    QString foregroundColor = QStringLiteral("#000000");
    QString backgroundColor = QStringLiteral("#c0c0c0");
    QString textColor = QStringLiteral("#000000");
    int pattern = 1;
    int speed = 3;
    int direction = 0;
    int sortOrder = 0;
    int fontSize = 16;
    bool visibilityBitChosen = false;
    StaticTextProperties visibility;
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, AlarmBarProperties *output, QString *error = nullptr);
};
#endif
