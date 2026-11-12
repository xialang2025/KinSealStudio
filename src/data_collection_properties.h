#ifndef DATA_COLLECTION_PROPERTIES_H
#define DATA_COLLECTION_PROPERTIES_H
#include "plc_address.h"

//20261112-1 add: A collection has one common type and stable per-item identities; native encoding remains separate.
struct DataCollectionItem
{
    QString id = QStringLiteral("");
    QString name = QStringLiteral("");
    int cacheIndex = 0;
    int displayType = 0;
    int digits = 4;
    int fractionalDigits = 0;
    bool scaled = false;
    QString gain = QStringLiteral("");
    QString offset = QStringLiteral("");
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, DataCollectionItem *output, QString *error = nullptr);
};

struct DataCollectionProperties
{
    bool upload = false;
    int sampleLength = 1;
    int sampleCount = 1;
    int dataType = 0;
    QString readAddress = QStringLiteral("LW0");
    int samplingMode = 0;
    int interval = 1;
    QString triggerAddress = QStringLiteral("LB0");
    int triggerMinute = 1;
    bool fullStop = false;
    bool notice = false;
    QString noticeAddress = QStringLiteral("LB0");
    int fullPercent = 90;
    QVector<DataCollectionItem> items;
    DataCollectionProperties();
    static DataCollectionItem createItem(int index);
    static QStringList dataTypes();
    static QStringList displayTypes();
    u32 wordCount() const;
    u32 memoryBytes() const;
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, DataCollectionProperties *output, QString *error = nullptr);
};

#endif
