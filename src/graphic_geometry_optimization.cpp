#include "graphic_geometry_optimization.h"
#include "project_model.h"

#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QPen>
#include <cmath>

namespace {
using u16 = unsigned short;
/******************************************************************************
 * Function Name   : nativeVertexBounds
 * Description     : Measure the same scene item geometry as the original top-level SM graphic.
 * Input Parameters: object, vertices
 * Return Value    : Scene bounds including the native pen where applicable
 * Date            : 2026.11.25
 ******************************************************************************/
QRectF nativeVertexBounds(const HmiObject &object, const QVector<QPointF> &vertices) //20261125-83 add
{
    const bool curve = object.type == QStringLiteral("linecurve");
    const bool polygon = object.type == QStringLiteral("polygon");
    const GraphicProperties &graphics = object.graphic;
    const bool cosmetic = graphics.lineWidth == 1 && (curve || object.type == QStringLiteral("line"));
    QPen pen(Qt::black);
    QPainterPath path;
    QGraphicsPathItem pathItem;
    QGraphicsPolygonItem polygonItem;

    // The default SM for line/curve stores width zero at the UI's first width.
    pen.setWidth(cosmetic ? 0 : graphics.lineWidth);
    pen.setStyle(static_cast<Qt::PenStyle>(graphics.lineStyle));
    if (!polygon) {
        pen.setCapStyle(graphics.endCap == 1 ? Qt::RoundCap : Qt::FlatCap);
        if (object.type == QStringLiteral("freeline")) {
            pen.setJoinStyle(graphics.endCap == 1 ? Qt::RoundJoin : Qt::MiterJoin);
        }
    }
    if (polygon) {
        polygonItem.setPen(pen);
        polygonItem.setPolygon(QPolygonF(vertices));
        return polygonItem.boundingRect();
    }
    if (curve) { path = curveDrawingPath(vertices, graphics.curveControls); }
    else { path.addPolygon(QPolygonF(vertices)); } // addPolygon leaves the outline open.
    if (cosmetic || graphics.lineStyle == 0) { return path.controlPointRect(); }
    pathItem.setPen(pen);
    pathItem.setPath(path);
    return pathItem.boundingRect();
}

/******************************************************************************
 * Function Name   : nativePixel
 * Description     : Replicate the x87 truncation followed by the native unsigned 16-bit conversion.
 * Input Parameters: coordinate
 * Return Value    : Device-grid coordinate or extent
 * Date            : 2026.11.25
 ******************************************************************************/
int nativePixel(double coordinate) //20261125-83 add
{
    return static_cast<u16>(static_cast<int>(coordinate + 0.5));
}

/******************************************************************************
 * Function Name   : scaleAxis
 * Description     : Scale scene vertices about the left or top of the pen-inclusive native bounds.
 * Input Parameters: points, origin, current, target, horizontal, shrinkOnly
 * Return Value    : True when this axis changed
 * Date            : 2026.11.25
 ******************************************************************************/
bool scaleAxis(QVector<QPointF> &points, double origin, double current, int target,
               bool horizontal, bool shrinkOnly) //20261125-83 add
{
    double ratio;
    if (current <= 0 || qAbs(target - current) < 1e-9 || (shrinkOnly && target >= current)) { return false; }
    ratio = target / current;
    for (QPointF &point : points) {
        double &coordinate = horizontal ? point.rx() : point.ry();
        coordinate = origin + (coordinate - origin) * ratio;
    }
    return true;
}
}

/******************************************************************************
 * Function Name   : optimizeGraphicVertices
 * Description     : Snap the visual origin, then run the native width and height operations in sequence.
 * Input Parameters: object
 * Return Value    : True for handled top-level vertex types, including unchanged shapes
 * Date            : 2026.11.25
 ******************************************************************************/
bool optimizeGraphicVertices(HmiObject *object) //20261125-83 add
{
    QVector<QPointF> vertices, normalized;
    QRectF initial, measured, frame;
    QPointF translation, origin, offset;
    QRect geometry;
    bool shrinkOnly, scaled;
    GraphicProperties candidate;

    if (!object || !graphicUsesVertices(object->type) || object->hasAppearanceFlip() ||
        object->graphic.endArrow != 0 || !object->graphic.validate(object->type)) { return false; }
    // Retain incoming curve handles until the legacy command's polygonization can be round-tripped.
    if (!object->graphic.curveControls.isEmpty()) { return false; }
    frame = QRectF(object->geometry).translated(object->subpixelOffset);
    vertices = graphicHandles(object->type, frame, object->graphic);
    initial = nativeVertexBounds(*object, vertices);
    translation = QPointF(nativePixel(initial.left()) - initial.left(), nativePixel(initial.top()) - initial.top());
    for (QPointF &point : vertices) { point += translation; }
    measured = initial.translated(translation);
    shrinkOnly = object->type == QStringLiteral("polyline") || object->type == QStringLiteral("linecurve");
    scaled = scaleAxis(vertices, measured.left(), measured.width(), nativePixel(initial.width()), true, shrinkOnly);
    if (scaled) { measured = nativeVertexBounds(*object, vertices); } // Width changes the diagonal stroke projection onto Y.
    scaled = scaleAxis(vertices, measured.top(), measured.height(), nativePixel(initial.height()), false, shrinkOnly) || scaled;
    if (!scaled) {
        if (translation.isNull()) { return true; }
        origin = frame.topLeft() + translation;
        object->geometry.moveTopLeft(QPoint(qFloor(origin.x()), qFloor(origin.y())));
        object->subpixelOffset = origin - object->geometry.topLeft();
        return true;
    }
    frame = QPolygonF(vertices).boundingRect();
    geometry = QRect(qFloor(frame.left()), qFloor(frame.top()), qMax(1, qCeil(frame.width())), qMax(1, qCeil(frame.height())));
    offset = frame.topLeft() - geometry.topLeft();
    normalized.reserve(vertices.size());
    for (const QPointF &point : std::as_const(vertices)) {
        normalized.append(QPointF((point.x() - frame.left()) / geometry.width(),
                                  (point.y() - frame.top()) / geometry.height()));
    }
    candidate = object->graphic;
    candidate.points = normalized;
    if (!candidate.validate(object->type)) { return false; }
    object->geometry = geometry;
    object->subpixelOffset = offset;
    object->graphic.points = std::move(normalized);
    return true;
}
