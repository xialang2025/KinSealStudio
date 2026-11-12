#ifndef RECIPE_PROPERTIES_H
#define RECIPE_PROPERTIES_H
#include "plc_address.h"
//20261113-2 add: Visible dimensions are independent of retained shrink/regrow drafts.
struct RecipeProperties
{
    QString description;
    int length = 4;
    int count = 1;
    int dataType = 0;
    bool writeEnabled = false;
    bool readEnabled = false;
    bool sameReadWrite = true;
    QString writeAddress = QStringLiteral("LW0");
    QString readAddress = QStringLiteral("LW0");
    QStringList itemNames;
    QStringList recipeNames;
    QVector<QStringList> values;
    RecipeProperties();
    static QStringList dataTypes();
    static int maximumLength(int type);
    int wordSpan() const;
    void resizeTable(int rows, int columns);
    void changeType(int type);
    void copyValues(int source, int target);
    static bool validValue(const QString &value, int type);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, RecipeProperties *output, QString *error = nullptr);
};
#endif
