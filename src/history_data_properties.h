#ifndef HISTORY_DATA_PROPERTIES_H
#define HISTORY_DATA_PROPERTIES_H
#include "alarm_bar_properties.h"
#include "data_collection_properties.h"
#include <QMap>
//20261113-2 add: Column identities follow the collector; captions are localized and visibility is shared.
struct HistoryDataColumn
{
    QString sourceItemId;
    bool shown = true;
    QMap<QString, QString> names;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, HistoryDataColumn *output, QString *error = nullptr);
};
struct HistoryDataCaptions
{
    QString date = QStringLiteral("日期");
    QString time = QStringLiteral("时间");
    QJsonObject toJson() const;
};
struct HistoryDataProperties
{
    QString collectionId;
    QString textColor = QStringLiteral("#000000");
    QString lineColor = QStringLiteral("#00c000");
    QString titleBackground = QStringLiteral("#ffff00");
    QString titleColor = QStringLiteral("#000000");
    QString backgroundColor = QStringLiteral("#e0e0e0");
    QString frameColor = QStringLiteral("#000000");
    int fontSize = 16;
    int rows = 5;
    bool verticalLines = true;
    bool horizontalLines = true;
    bool showDate = true;
    bool showTime = true;
    bool visibilityBitChosen = false;
    QMap<QString, HistoryDataCaptions> captions;
    QVector<HistoryDataColumn> columns;
    StaticTextProperties visibility;
    HistoryDataProperties();
    static QList<int> fontSizes();
    HistoryDataCaptions captionsFor(const QString &language) const;
    void setLanguages(const QStringList &languages);
    void reconcileCollection(const DataCollectionProperties &collection, const QStringList &languages);
    void synchronizeNames(const QString &first, const QStringList &targets);
    bool validate(QString *error = nullptr) const;
    bool validateCollection(const DataCollectionProperties &collection, QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, HistoryDataProperties *output, QString *error = nullptr);
};
#endif
