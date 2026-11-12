#ifndef HISTORY_TREND_PROPERTIES_H
#define HISTORY_TREND_PROPERTIES_H
#include "trend_properties.h"
#include "data_collection_properties.h"
#include <array>

//20261112-1 add: Bind editor identities to a collection definition without pretending to read native history files.
struct HistoryTrendCurve
{
    QString sourceItemId = QStringLiteral("");
    QString minimum = QStringLiteral("0");
    QString maximum = QStringLiteral("1000");
    int lineStyle = 0;
    QString color = QStringLiteral("#ff0000");
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, HistoryTrendCurve *output, QString *error = nullptr);
};

struct HistoryTrendProperties
{
    QString collectionId = QStringLiteral("");
    int curveCount = 1;
    int currentCurve = 0;
    QString bottomColor = QStringLiteral("#ffff00");
    QString backgroundColor = QStringLiteral("#f0f0f0");
    int hours = 1;
    int minutes = 0;
    int seconds = 0;
    int timeFormat = 0;
    QString yMinimum = QStringLiteral("0");
    QString yMaximum = QStringLiteral("1000");
    int totalDigits = 4;
    int fractionalDigits = 0;
    bool visibilityBitChosen = false;
    std::array<HistoryTrendCurve, 8> curves;
    TrendAxis xAxis;
    TrendAxis yAxis;
    StaticTextProperties visibility;
    bool validate(QString *error = nullptr) const;
    bool validateCollection(const DataCollectionProperties &collection, QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, HistoryTrendProperties *output, QString *error = nullptr);
};

#endif
