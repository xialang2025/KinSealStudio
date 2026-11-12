#ifndef NUMBER_DISPLAY_PROPERTIES_H
#define NUMBER_DISPLAY_PROPERTIES_H

#include "image_display_properties.h"

//20261112-0 add: These typed editor fields are independent of the native type12 parameter layout.
#define NUMBER_BOOLEAN_FIELDS(X) \
    X(allowInput, true) X(sameMonitor, true) X(password, false) X(inputMarker, false) \
    X(offsetEnabled, false) X(displayAddress, true) X(scaling, false) X(macroEnabled, false) \
    X(customDisplay, false) X(displayRange, false) X(effect3d, false) X(transparent, false) \
    X(notice, false) X(noticeBit, false) X(noticeBitValue, true) X(noticeWord, false) \
    X(pressTimeEnabled, false) X(customKeyboard, false) X(keyboardPosition, false)
#define NUMBER_INTEGER_FIELDS(X) \
    X(dataType, 0) X(inputRangeSource, 0) X(font, 0) X(fontSize, 16) X(totalDigits, 5) X(decimalDigits, 0) \
    X(alignment, 0) X(adjustment, 0) X(opacity, 255) X(displayRangeSource, 0) X(noticeWordType, 0) \
    X(pressTime, 255) X(keyboardX, 0) X(keyboardY, 0)
#define NUMBER_STRING_FIELDS(X) \
    X(monitorAddress, "LW0") X(inputAddress, "LW0") X(offsetAddress, "LW0") \
    X(inputMinimum, "-32768") X(inputMaximum, "32767") X(inputMinimumAddress, "LW0") X(inputMaximumAddress, "LW0") \
    X(gain, "1") X(offset, "0") X(macroName, "") X(customText, "") \
    X(textColor, "#000000") X(backgroundColor, "#a0a0ff") X(lowColor, "#ff0000") X(highColor, "#00ffff") \
    X(displayMinimum, "-32768") X(displayMaximum, "32767") X(displayMinimumAddress, "LW0") X(displayMaximumAddress, "LW0") \
    X(noticeBitAddress, "LB0") X(noticeWordAddress, "LW0") X(noticeWordValue, "0") X(keyboardPageId, "")

struct NumberDisplayProperties
{
#define DECLARE_NUMBER_BOOL(name, initial) bool name = initial;
    NUMBER_BOOLEAN_FIELDS(DECLARE_NUMBER_BOOL)
#undef DECLARE_NUMBER_BOOL
#define DECLARE_NUMBER_INT(name, initial) int name = initial;
    NUMBER_INTEGER_FIELDS(DECLARE_NUMBER_INT)
#undef DECLARE_NUMBER_INT
#define DECLARE_NUMBER_STRING(name, initial) QString name = QStringLiteral(initial);
    NUMBER_STRING_FIELDS(DECLARE_NUMBER_STRING)
#undef DECLARE_NUMBER_STRING
    ImageDisplayState appearance;
    StaticTextProperties visibility;
    StaticTextProperties touch;

    NumberDisplayProperties();
    static QStringList dataTypes();
    static int maximumDigits(int type);
    static int wordSpan(int type);
    static bool decimalEnabled(int type);
    void changeDataType(int type);
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, NumberDisplayProperties *output, QString *error, bool numericFieldsActive = true); //20261112-0 change
    bool validate(QString *error = nullptr, bool numericFieldsActive = true) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};

#endif
