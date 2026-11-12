#ifndef IMAGE_DISPLAY_PROPERTIES_H
#define IMAGE_DISPLAY_PROPERTIES_H

#include "static_text_properties.h"
#include "plc_address.h"
#include <QVector>

//20261112-0 add: A picture row stores an exact resource binding, independent of button text or touch behavior.
struct ImageDisplayState
{
    QString picture;
    QByteArray picturePng;
    QJsonObject toJson() const;
};

struct ImageDisplayProperties
{
    int displayType = 0;
    bool conditionEnabled = false;
    int condition = 0;
    int stateCount = 1;
    QString monitorAddress;
    int interval = 1;
    QVector<ImageDisplayState> states = {ImageDisplayState()};
    StaticTextProperties visibility;

    void setStateCount(int count);
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, ImageDisplayProperties *output, QString *error);
    bool validate(QString *error) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error) const;
};

#endif
