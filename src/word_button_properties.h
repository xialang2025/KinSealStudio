#ifndef WORD_BUTTON_PROPERTIES_H
#define WORD_BUTTON_PROPERTIES_H

#include <QJsonObject>
#include <QStringList>

//20261109-6 add: Keep word operations separate from shared button artwork and permissions.
struct WordButtonProperties
{
    int dataType = 2;
    int operation = 0;
    QString constantValue = QStringLiteral("1");
    QString minimumValue = QStringLiteral("-32768");
    QString maximumValue = QStringLiteral("32767");
    int totalDigits = 5;
    int decimalDigits = 0;
    bool pressedEffect = false;

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, WordButtonProperties *properties, QString *error);
    bool validate(QString *error) const;
    void resetDefaultsForDataType(int type);
    void resetDefaultsForOperation(int operationIndex);
    bool usesConstant() const;
    bool usesMinimum() const;
    bool usesMaximum() const;
    bool usesTotalDigits() const;
    bool usesDecimalDigits() const;
    int transferWordCount() const;
    static QStringList dataTypes();
    static QStringList operations();
};

#endif
