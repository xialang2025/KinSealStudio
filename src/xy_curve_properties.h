#ifndef XY_CURVE_PROPERTIES_H
#define XY_CURVE_PROPERTIES_H
#include "trend_properties.h"

//20261113-1 add: X/Y coordinate drafts are distinct from timed trend channels.
struct XyCurveProperties
{
    QString triggerAddress = QStringLiteral("LB0");
    QString readAddress = QStringLiteral("LW0");
    QString clearAddress = QStringLiteral("LB0");
    int dataType = 0;
    int samplePoints = 60;
    int sampleTime = 1;
    int channelCount = 1;
    int currentChannel = 0;
    QString bottomColor = QStringLiteral("#ffff00");
    QString frameColor = QStringLiteral("#c0c0c0");
    QString backgroundColor = QStringLiteral("#ffffff");
    QString xMinimum = QStringLiteral("0");
    QString xMaximum = QStringLiteral("1000");
    int xTotalDigits = 4;
    int xFractionalDigits = 0;
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
    static bool fromJson(const QJsonObject &json, XyCurveProperties *output, QString *error = nullptr);
    static QStringList dataTypes();
    static u32 wordCount(int type);
    static QPair<QString, QString> typeRange(int type);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};

#endif
