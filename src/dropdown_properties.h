#ifndef DROPDOWN_PROPERTIES_H
#define DROPDOWN_PROPERTIES_H
#include "screen_button_properties.h"

//20261112-0 add: Dropdown values and labels are a typed editor table, not DropListText firmware records.
#define DROPDOWN_BOOL_FIELDS(X) \
    X(macroEnabled, false) X(visibilityBitChosen, false) X(touchBitChosen, false) \
    X(notice, false) X(noticeBit, false) X(noticeWord, false) X(noticeBitValue, true)
#define DROPDOWN_INT_FIELDS(X) \
    X(fontSize, 16) X(alignment, 2) X(direction, 0) X(dataType, 0) X(noticeWordType, 0)
#define DROPDOWN_STRING_FIELDS(X) \
    X(address, "LW0") X(background, "#646464") X(textColor, "#ffffff") X(macroName, "") \
    X(noticeBitAddress, "LB0") X(noticeWordAddress, "LW0") X(noticeWordValue, "0")

struct DropdownEntry
{
    QString value;
    QString label;
};

struct DropdownProperties
{
#define DECLARE_DROPDOWN_BOOL(name, initial) bool name = initial;
    DROPDOWN_BOOL_FIELDS(DECLARE_DROPDOWN_BOOL)
#undef DECLARE_DROPDOWN_BOOL
#define DECLARE_DROPDOWN_INT(name, initial) int name = initial;
    DROPDOWN_INT_FIELDS(DECLARE_DROPDOWN_INT)
#undef DECLARE_DROPDOWN_INT
#define DECLARE_DROPDOWN_STRING(name, initial) QString name = QStringLiteral(initial);
    DROPDOWN_STRING_FIELDS(DECLARE_DROPDOWN_STRING)
#undef DECLARE_DROPDOWN_STRING
    QVector<DropdownEntry> entries;
    StaticTextProperties visibility;
    StaticTextProperties touch;
    DropdownProperties();
    void setCount(int count);
    int transferWordCount() const;
    BitButtonProperties accessDraft() const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, DropdownProperties *output, QString *error);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};
#endif
