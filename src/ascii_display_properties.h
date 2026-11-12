#ifndef ASCII_DISPLAY_PROPERTIES_H
#define ASCII_DISPLAY_PROPERTIES_H

#include "number_display_properties.h"

//20261112-0 add: Shared visual/access drafts do not imply a numeric native record or a recovered string byte layout.
struct AsciiDisplayProperties
{
    NumberDisplayProperties common;
    int characterCount = 5;
    int encoding = 0;
    bool autoWrap = false;
    bool swapBytes = true;
    bool modifyInput = false;

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, AsciiDisplayProperties *output, QString *error);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};

#endif
