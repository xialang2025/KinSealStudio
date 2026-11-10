#ifndef STATIC_TEXT_PROPERTIES_H
#define STATIC_TEXT_PROPERTIES_H

#include <QColor>
#include <QJsonObject>
#include <QMap>
#include <QStringList>

//20261108-14 add: Keep editable appearance and access conditions independent of raster output.
struct StaticTextProperties
{
    QString fontFamily = QStringLiteral("微软雅黑");
    double fontSize = 16;
    double spacing = 0;
    int alignment = 0;
    bool italic = false;
    bool bold = false;
    bool adaptive = false;
    bool underline = false;
    QColor textColor = Qt::black;
    QColor backgroundColor = Qt::white;
    QColor foregroundColor = Qt::red;
    int style = 0;
    int opacity = 255;
    QMap<QString, QString> languageTexts;
    bool visibilityEnabled = false;
    bool bitEnabled = false;
    QString bitAddress = QStringLiteral("LB0");
    bool bitState = true;
    bool levelEnabled = false;
    int minimumLevel = 1;
    bool wordEnabled = false;
    int dataType = 0;
    QString wordAddress = QStringLiteral("LW0");
    int comparison = 0;
    QString compareValue = QStringLiteral("0");
    QString tolerance = QStringLiteral("0");

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, StaticTextProperties *properties,
                         QString *error);
    static QStringList styles();
    static QStringList dataTypes();
    static QStringList comparisons();
    bool validate(QString *error) const;
};

#endif
