#ifndef QR_CODE_PROPERTIES_H
#define QR_CODE_PROPERTIES_H

#include "static_text_properties.h"
#include "plc_address.h"

//20261112-0 add: Store the observed QR fields independently from unverified PLC string and firmware layouts.
struct QrCodeProperties
{
    QString monitorAddress = QStringLiteral("LW0");
    int encoding = 0;
    bool swapBytes = false;
    int characterLength = 25;
    QString description;
    StaticTextProperties visibility;

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, QrCodeProperties *output, QString *error);
    bool validate(QString *error) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error) const;
};

#endif
