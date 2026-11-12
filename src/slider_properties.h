#ifndef SLIDER_PROPERTIES_H
#define SLIDER_PROPERTIES_H
#include "screen_button_properties.h"
#include <QRect>

//20261112-0 add: Independent editor values; these are not recovered native slider or firmware records.
#define SLIDER_BOOL_FIELDS(X) \
    X(backgroundTransparent, false) X(borderTransparent, false) X(railBackgroundTransparent, false) X(railBorderTransparent, false) \
    X(dynamicRange, false) X(macroEnabled, false) X(notice, false) X(noticeBit, false) X(noticeWord, false) X(noticeBitValue, true)
#define SLIDER_INT_FIELDS(X) \
    X(dataType, 0) X(pointing, 0) X(direction, 1) X(railHeight, 10) X(style, 1) X(thumbWidth, 10) X(thumbHeight, 50) X(noticeWordType, 0)
#define SLIDER_STRING_FIELDS(X) \
    X(address, "LW0") X(background, "#f0f0f0") X(borderColor, "#000000") X(railBackground, "#00ff00") \
    X(fillColor, "#00f0f0") X(railBorderColor, "#00f0f0") X(thumbBackground, "#c8d2dc") X(thumbBorderColor, "#000000") \
    X(minimum, "0") X(maximum, "100") X(minimumAddress, "LW2") X(maximumAddress, "LW3") X(macroName, "") \
    X(noticeBitAddress, "LB0") X(noticeWordAddress, "LW0") X(noticeWordValue, "0")

struct SliderProperties
{
#define DECLARE_SLIDER_BOOL(name, initial) bool name = initial;
    SLIDER_BOOL_FIELDS(DECLARE_SLIDER_BOOL)
#undef DECLARE_SLIDER_BOOL
#define DECLARE_SLIDER_INT(name, initial) int name = initial;
    SLIDER_INT_FIELDS(DECLARE_SLIDER_INT)
#undef DECLARE_SLIDER_INT
#define DECLARE_SLIDER_STRING(name, initial) QString name = QStringLiteral(initial);
    SLIDER_STRING_FIELDS(DECLARE_SLIDER_STRING)
#undef DECLARE_SLIDER_STRING
    StaticTextProperties visibility;
    StaticTextProperties touch;
    SliderProperties();
    static QStringList dataTypes();
    static QStringList styles();
    QStringList directions() const;
    bool vertical() const;
    void setPointing(int value);
    void setDynamicRange(bool enabled);
    int transferWordCount() const;
    QRect constrainedGeometry(const QRect &requested) const;
    BitButtonProperties accessDraft() const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, SliderProperties *output, QString *error);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};
#endif
