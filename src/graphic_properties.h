#ifndef GRAPHIC_PROPERTIES_H
#define GRAPHIC_PROPERTIES_H

#include <QColor>
#include <QJsonObject>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <QVector>

class QPainter;

//20261113-2 add: Lite geometry is explicit and independent of opaque native SM paths.
struct GraphicProperties
{
    int lineStyle = 1;
    int lineWidth = 1;
    int endArrow = 0;
    int endCap = 0;
    QColor lineColor = Qt::black;
    int alpha = 255;
    int fillStyle = 1;
    QColor foreground = Qt::black;
    QColor background = Qt::white;
    int radiusX = 10; // Common closed-shape storage; only the rounded-rectangle family will serialize radii.
    int radiusY = 10;
    QVector<QPointF> points = {QPointF(0, 0), QPointF(1, 1)};
    QJsonObject toJson(const QString &type = QStringLiteral("line")) const;
    static bool fromJson(const QJsonObject &json, GraphicProperties *output, QString *error, const QString &type = QStringLiteral("line"));
    bool validate(const QString &type, QString *error = nullptr) const;
};

bool isSupportedGraphic(const QString &type);
bool graphicUsesVertices(const QString &type);
bool graphicStrokeOnly(const QString &type); //20261113-4 add
QString graphicName(const QString &type);
QPainterPath graphicPath(const QString &type, const QRectF &rect, const GraphicProperties &properties);
QPainterPath graphicHitPath(const QString &type, const QRectF &rect, const GraphicProperties &properties);
void paintGraphic(QPainter *painter, const QString &type, const QRectF &rect, const GraphicProperties &properties);
QVector<QPointF> graphicHandles(const QString &type, const QRectF &rect, const GraphicProperties &properties);

#endif
