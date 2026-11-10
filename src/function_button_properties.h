#ifndef FUNCTION_BUTTON_PROPERTIES_H
#define FUNCTION_BUTTON_PROPERTIES_H

#include <QJsonObject>
#include <QStringList>

//20261110-0 add: Keep system-function selection independent of shared artwork and permission drafts.
struct FunctionButtonProperties
{
    int function = 0;
    int type = 0;
    int keyboardKey = 0;
    QString keyboardText;
    bool pressedEffect = false;

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, FunctionButtonProperties *properties, QString *error);
    bool validate(QString *error) const;
    static QStringList functions();
    static QStringList types(int function, const QStringList &languages);
    static QStringList keyboardKeys();
};

#endif
