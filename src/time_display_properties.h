#ifndef TIME_DISPLAY_PROPERTIES_H
#define TIME_DISPLAY_PROPERTIES_H

#include "number_display_properties.h"

//20261112-0 add: RTC display has no numeric carrier, input keyboard or touch/notification record.
struct TimeDisplayProperties
{
    bool showDate = true;
    bool showWeek = false;
    bool showTime = false;
    int dateFormat = 0;
    int timeFormat = 0;
    int fontSize = 16;
    int alignment = 0;
    int opacity = 255;
    bool effect3d = false;
    bool transparent = false;
    QString textColor = QStringLiteral("#000000");
    QString backgroundColor = QStringLiteral("#a0a0ff");
    ImageDisplayState appearance;
    StaticTextProperties visibility;

    static QStringList dateFormats();
    static QStringList timeFormats();
    static QStringList fontSizes();
    QString sample() const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, TimeDisplayProperties *output, QString *error);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};

#endif
