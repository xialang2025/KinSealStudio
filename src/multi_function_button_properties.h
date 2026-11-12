#ifndef MULTI_FUNCTION_BUTTON_PROPERTIES_H
#define MULTI_FUNCTION_BUTTON_PROPERTIES_H
#include "screen_button_properties.h"

//20261112-0 add: Command indices belong to the MF editor, never to the native Switch union.
struct MultiFunctionCommand
{
    int kind = 0; // 0: bit, 1: navigation, 2: word.
    int operation = 0;
    QString address = QStringLiteral("LB0");
    int dataType = 2;
    QString value = QStringLiteral("1");
    QString screenPageId;
    QString windowPageId;
    QString activeTarget() const;
    int transferWordCount() const;
    QString label() const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, MultiFunctionCommand *output, QString *error);
    bool validate(QString *error) const;
    static QStringList bitOperations();
    static QStringList wordOperations();
    static QStringList dataTypes();
};

struct MultiFunctionButtonProperties
{
    BitButtonProperties common;
    QVector<MultiFunctionCommand> commands;
    bool pressedEffect = false;
    bool visibilityBitChosen = false;
    bool touchBitChosen = false;
    MultiFunctionButtonProperties();
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, MultiFunctionButtonProperties *output, QString *error);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};
#endif
