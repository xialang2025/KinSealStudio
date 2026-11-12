#ifndef DATA_GROUP_PROPERTIES_H
#define DATA_GROUP_PROPERTIES_H
#include "trend_properties.h"

//20261113-1 add: A data group is one buffered curve; it has no history-collection or paired-X/Y identity.
struct DataGroupProperties
{
    QString triggerAddress = QStringLiteral("LB0");
    QString readAddress = QStringLiteral("LW0");
    int dataType = 0;
    int sampleMode = 0;
    int samplePoints = 100;
    QString sampleAddress = QStringLiteral("LW0");
    int lineStyle = 0;
    QString channelColor = QStringLiteral("#ff0000");
    QString bottomColor = QStringLiteral("#0000ff");
    QString frameColor = QStringLiteral("#0000ff");
    QString backgroundColor = QStringLiteral("#ffffff");
    int rangeSource = 0;
    QString yMinimum = QStringLiteral("0");
    QString yMaximum = QStringLiteral("1000");
    QString maximumAddress = QStringLiteral("LW0");
    int totalDigits = 4;
    int fractionalDigits = 0;
    bool visibilityBitChosen = false;
    TrendAxis xAxis;
    TrendAxis yAxis;
    StaticTextProperties visibility;
    static QStringList dataTypes();
    static u32 wordCount(int type);
    u32 readWordSpan() const;
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, DataGroupProperties *output, QString *error = nullptr);
};
#endif
