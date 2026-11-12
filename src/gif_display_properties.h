#ifndef GIF_DISPLAY_PROPERTIES_H
#define GIF_DISPLAY_PROPERTIES_H

#include "static_text_properties.h"
#include "plc_address.h"
#include <QImage>

//20261112-0 add: Original animation bytes stay separate from the PNG-only picture pool.
bool validateGifData(const QByteArray &data, QString *error = nullptr);
QImage gifFirstFrame(const QByteArray &data);

struct GifDisplayProperties
{
    QString animation;
    QByteArray gifData;
    bool bitControl = false;
    QString controlAddress = QStringLiteral("LB0");
    int activeState = 1;
    StaticTextProperties visibility;

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, GifDisplayProperties *output, QString *error);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};

#endif
