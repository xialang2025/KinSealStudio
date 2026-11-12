#ifndef CHART_PROPERTIES_H
#define CHART_PROPERTIES_H
#include "screen_button_properties.h"

//20261112-1 add: Three separate native form drafts; no firmware layout is inferred.
struct ChartBarProperties
{
    int dataType = 2;
    QString address = QStringLiteral("LW0");
    QString minimum = QStringLiteral("0");
    QString maximum = QStringLiteral("100");
    bool twoPoles = false;
    QString midpoint = QStringLiteral("");
    bool dynamicRange = false;
    QString sourceMinimum = QStringLiteral("0");
    QString sourceMaximum = QStringLiteral("100");
    QString minimumAddress = QStringLiteral("LW0");
    QString maximumAddress = QStringLiteral("LW0");
    QString foreground = QStringLiteral("#ff0000");
    QString background = QStringLiteral("#ffff00");
    QString patternColor = QStringLiteral("#000000");
    int pattern = 1;
    int direction = 0;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, ChartBarProperties *output);
};

struct ChartMeterProperties
{
    int dataType = 2;
    QString address = QStringLiteral("LW0");
    QString minimum = QStringLiteral("0");
    QString maximum = QStringLiteral("100");
    int direction = 0;
    bool transparent = false;
    QString foreground = QStringLiteral("#ff0000");
    QString background = QStringLiteral("#00ffff");
    int pointer = 1;
    int shape = 1;
    int mainDivisions = 4;
    bool showValues = true;
    bool showAxis = true;
    int fontSize = 12;
    QString scaleColor = QStringLiteral("#ff0000");
    bool showSubScale = true;
    int subDivisions = 2;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, ChartMeterProperties *output);
};

struct ChartDonutProperties
{
    int dataType = 0;
    QString address = QStringLiteral("LW0");
    QString minimum = QStringLiteral("0");
    QString maximum = QStringLiteral("100");
    bool arc = false;
    int lineType = 1;
    int lineWidth = 1;
    QString radius = QStringLiteral("12");
    QString startAngle = QStringLiteral("0");
    QString spanAngle = QStringLiteral("360");
    int opacity = 255;
    QString lineColor = QStringLiteral("#ff0000");
    int fillWidth = 1;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, ChartDonutProperties *output);
};

struct ChartProperties
{
    int shape = 0;
    ChartBarProperties bar;
    ChartMeterProperties meter;
    ChartDonutProperties donut;
    StaticTextProperties visibility;
    bool visibilityBitChosen = false;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, ChartProperties *output, QString *error);
    static QStringList dataTypes();
    static QStringList patterns();
    static QStringList lineTypes();
    static QStringList directions();
    static QStringList meterShapes();
    static QStringList pointers();
    static QPair<QString, QString> typeRange(int type);
    static unsigned int wordCount(int type);
    QString address() const;
    void setAddress(const QString &text);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};
#endif
