#ifndef ALARM_DISPLAY_PROPERTIES_H
#define ALARM_DISPLAY_PROPERTIES_H
#include "alarm_bar_properties.h"
#include <QMap>
//20261113-2 add: Only the three captions are localized; all observed styling and column flags are common.
struct AlarmDisplayCaptions
{
    QString message = QStringLiteral("消息");
    QString date = QStringLiteral("日期");
    QString time = QStringLiteral("时间");
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, AlarmDisplayCaptions *output, QString *error = nullptr);
};
struct AlarmDisplayProperties
{
    QString titleBackground = QStringLiteral("#ffff00");
    QString titleColor = QStringLiteral("#000000");
    QString frameColor = QStringLiteral("#000000");
    QString backgroundColor = QStringLiteral("#e0e0e0");
    QString textColor = QStringLiteral("#000000");
    int fontSize = 16;
    int rows = 5;
    bool showDate = true;
    bool showTime = true;
    bool visibilityBitChosen = false;
    QMap<QString, AlarmDisplayCaptions> captions;
    StaticTextProperties visibility;
    AlarmDisplayProperties();
    static QList<int> fontSizes();
    AlarmDisplayCaptions captionsFor(const QString &language) const;
    void setLanguages(const QStringList &languages);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, AlarmDisplayProperties *output, QString *error = nullptr);
};
#endif
