#ifndef SCREEN_BUTTON_PROPERTIES_H
#define SCREEN_BUTTON_PROPERTIES_H
#include "bit_button_properties.h"
#include "plc_address.h"

//20261112-0 add: Shared button artwork is an editor component; navigation identity is independent of native Switch layouts.
struct ScreenButtonProperties
{
    BitButtonProperties common;
    int action = 0;
    QString screenPageId;
    QString windowPageId;
    bool changeLevel = true;
    int newLevel = 1;
    bool requireLogin = true;
    ScreenButtonProperties();
    static QStringList actions();
    QString activeTarget() const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, ScreenButtonProperties *output, QString *error);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};
#endif
