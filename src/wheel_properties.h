#ifndef WHEEL_PROPERTIES_H
#define WHEEL_PROPERTIES_H
#include "screen_button_properties.h"
#include "image_display_properties.h"

//20261112-0 add: Wheel values are independent from slider direction, ranges and notification drafts.
#define WHEEL_BOOL_FIELDS(X) X(rollover, false) X(visibilityBitChosen, false) X(touchBitChosen, false)
#define WHEEL_INT_FIELDS(X) X(sensitivity, 20) X(opacity, 255) X(rowCount, 3) X(lowFontSize, 16) X(highFontSize, 24)
#define WHEEL_STRING_FIELDS(X) X(address, "LW0") X(background, "#55aaff") X(lowColor, "#000000") X(highColor, "#ffffff") X(minimum, "0") X(maximum, "100") X(step, "1")

struct WheelProperties
{
#define DECLARE_WHEEL_BOOL(name, initial) bool name = initial;
    WHEEL_BOOL_FIELDS(DECLARE_WHEEL_BOOL)
#undef DECLARE_WHEEL_BOOL
#define DECLARE_WHEEL_INT(name, initial) int name = initial;
    WHEEL_INT_FIELDS(DECLARE_WHEEL_INT)
#undef DECLARE_WHEEL_INT
#define DECLARE_WHEEL_STRING(name, initial) QString name = QStringLiteral(initial);
    WHEEL_STRING_FIELDS(DECLARE_WHEEL_STRING)
#undef DECLARE_WHEEL_STRING
    ImageDisplayState highlight;
    StaticTextProperties visibility;
    StaticTextProperties touch;
    WheelProperties();
    static QList<int> fontSizes();
    int editorSampleValue() const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, WheelProperties *output, QString *error);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};
#endif
