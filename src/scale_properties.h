#ifndef SCALE_PROPERTIES_H
#define SCALE_PROPERTIES_H

#include <QColor>
#include <QJsonObject>

//20261112-0 add: A static scale has no monitor, permission, touch or notification fields.
struct ScaleProperties
{
    int fontSize = 16;
    int direction = 0;
    int position = 3;
    int minimum = 0;
    int maximum = 100;
    int decimals = 0;
    QColor textColor = Qt::black;
    QColor lineColor = Qt::black;
    bool showValues = true;
    int majorCount = 4;
    int minorCount = 1;

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, ScaleProperties *output, QString *error);
    bool validate(QString *error) const;
};

#endif
