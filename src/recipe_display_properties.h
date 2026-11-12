#ifndef RECIPE_DISPLAY_PROPERTIES_H
#define RECIPE_DISPLAY_PROPERTIES_H
#include "alarm_bar_properties.h"
#include "recipe_properties.h"
struct HmiProject;
//20261113-2 add: Only observed editing/keyboard controls are exposed; hidden row/column strings are not invented fields.
struct RecipeDisplayProperties
{
    QString recipeId;
    int fontSize = 16;
    int alignment = 1;
    QString titleColor = QStringLiteral("#0000ff");
    QString textColor = QStringLiteral("#000000");
    QString titleBackground = QStringLiteral("#ff6666");
    QString backgroundColor = QStringLiteral("#ffffff");
    QString gridColor = QStringLiteral("#000000");
    bool showGrid = true;
    bool editEnabled = false;
    bool customKeyboard = false;
    QString keyboardPageId;
    bool keyboardPosition = false;
    int keyboardX = 0;
    int keyboardY = 0;
    StaticTextProperties visibility;
    StaticTextProperties touch;
    bool visibilityBitChosen = false;
    bool touchBitChosen = false;
    bool notice = false;
    bool noticeBit = false;
    QString noticeBitAddress = QStringLiteral("LB0");
    bool noticeBitValue = true;
    bool noticeWord = false;
    QString noticeWordAddress = QStringLiteral("LW0");
    int noticeWordType = 0;
    QString noticeWordValue = QStringLiteral("0");
    RecipeDisplayProperties();
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
    bool validateContext(const HmiProject &project, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, RecipeDisplayProperties *output, QString *error = nullptr);
};
bool validateRecipeDefinitions(const HmiProject &project, QString *error = nullptr);
#endif
