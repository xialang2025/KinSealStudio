#ifndef TREND_PROPERTIES_H
#define TREND_PROPERTIES_H
#include "screen_button_properties.h"
#include <array>

//20261112-1 add: Trend sampling metadata and independent channels are editor data, not a live PLC buffer.
struct TrendChannel
{
    QString minimum = QStringLiteral("0");
    QString maximum = QStringLiteral("1000");
    int lineStyle = 0;
    QString color = QStringLiteral("#ff0000");
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, TrendChannel *output, QString *error = nullptr);
};

struct TrendAxis
{
    bool ticks = true;
    bool grid = true;
    bool marks = true;
    QString axisColor = QStringLiteral("#000000");
    QString gridColor = QStringLiteral("#00ff00");
    int major = 4;
    int minor = 2;
    int fontSize = 16;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, TrendAxis *output, QString *error = nullptr);
};

struct TrendProperties
{
    QString triggerAddress = QStringLiteral("LB0");
    QString readAddress = QStringLiteral("LW0");
    QString clearAddress = QStringLiteral("LB0");
    int dataType = 0;
    int samplePoints = 60;
    int sampleTime = 1;
    int channelCount = 1;
    int currentChannel = 0;
    bool rightToLeft = false;
    QString bottomColor = QStringLiteral("#ffff00");
    QString frameColor = QStringLiteral("#c0c0c0");
    QString backgroundColor = QStringLiteral("#ffffff");
    QString yMinimum = QStringLiteral("0");
    QString yMaximum = QStringLiteral("1000");
    int totalDigits = 4;
    int fractionalDigits = 0;
    bool visibilityBitChosen = false;
    std::array<TrendChannel, 8> channels;
    TrendAxis xAxis;
    TrendAxis yAxis;
    StaticTextProperties visibility;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, TrendProperties *output, QString *error = nullptr);
    static QStringList dataTypes();
    static unsigned int wordCount(int type);
    static QPair<QString, QString> typeRange(int type);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};

#endif
