#include "graphic_properties.h"
#include <QJsonArray>
#include <QPainter>
#include <QPainterPathStroker>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QTransform>
#include <cmath>

/******************************************************************************
 * Function Name   : graphicError
 * Description     : Report a geometry validation failure without replacing caller data.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool graphicError(QString *error, const QString &message) //20261113-2 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : isSupportedGraphic
 * Description     : Limit editing to individually delivered graphics families.
 * Input Parameters: type
 * Return Value    : True for a supported portable graphic.
 * Date            : 2026.11.13
 ******************************************************************************/
bool isSupportedGraphic(const QString &type) //20261113-2 add
{
    return type == QStringLiteral("line") || type == QStringLiteral("rect") || type == QStringLiteral("ellipse") || type == QStringLiteral("roundrect") || type == QStringLiteral("polyline") || type == QStringLiteral("polygon"); //20261113-2 change
}

/******************************************************************************
 * Function Name   : graphicUsesVertices
 * Description     : Distinguish ordered endpoint editing from ordinary closed-shape box resizing.
 * Input Parameters: type
 * Return Value    : True for a delivered vertex-based family.
 * Date            : 2026.11.13
 ******************************************************************************/
bool graphicUsesVertices(const QString &type) //20261113-2 add
{
    return type == QStringLiteral("line") || type == QStringLiteral("polyline") || type == QStringLiteral("polygon"); //20261113-2 change: Each polyline vertex keeps its ordered role.
}

/******************************************************************************
 * Function Name   : graphicStrokeOnly
 * Description     : Separate open stroke styling from closed vertex topology.
 * Input Parameters: type
 * Return Value    : True when the family has arrows and no fill.
 * Date            : 2026.11.13
 ******************************************************************************/
bool graphicStrokeOnly(const QString &type) //20261113-4 add
{
    return type == QStringLiteral("line") || type == QStringLiteral("polyline");
}

/******************************************************************************
 * Function Name   : graphicName
 * Description     : Return the verified native label of a supported drawing tool.
 * Input Parameters: type
 * Return Value    : Localized graphic name.
 * Date            : 2026.11.13
 ******************************************************************************/
QString graphicName(const QString &type) //20261113-2 add
{
    if (type == QStringLiteral("line")) { return QStringLiteral("直线"); }
    if (type == QStringLiteral("rect")) { return QStringLiteral("矩形"); }
    if (type == QStringLiteral("ellipse")) { return QStringLiteral("圆/椭圆"); }
    if (type == QStringLiteral("roundrect")) { return QStringLiteral("圆角矩形"); } //20261113-2 add
    if (type == QStringLiteral("polygon")) { return QStringLiteral("多边形"); } //20261113-4 add
    if (type == QStringLiteral("polyline")) { return QStringLiteral("折线"); } //20261113-2 add
    return QString();
}

/******************************************************************************
 * Function Name   : GraphicProperties::toJson
 * Description     : Preserve stroke drafts and ordered normalized points independently of the enclosing box.
 * Input Parameters: None
 * Return Value    : Complete portable graphic properties.
 * Date            : 2026.11.13
 ******************************************************************************/
QJsonObject GraphicProperties::toJson(const QString &type) const //20261113-2 change
{
    QJsonArray vertices;
    QPointF point;
    QJsonObject json;

    json = {{QStringLiteral("lineStyle"), lineStyle}, {QStringLiteral("lineWidth"), lineWidth},
            {QStringLiteral("lineColor"), lineColor.name(QColor::HexArgb)}, {QStringLiteral("alpha"), alpha}};
    if (graphicUsesVertices(type))
    {
        Q_FOREACH (point, points) { vertices.append(QJsonArray{point.x(), point.y()}); }
        json.insert(QStringLiteral("points"), vertices);
    }
    if (graphicStrokeOnly(type)) //20261113-4 change: Topology and fill are independent for polygons.
    {
        json.insert(QStringLiteral("endArrow"), endArrow); json.insert(QStringLiteral("endCap"), endCap);
    }
    else
    {
        json.insert(QStringLiteral("fillStyle"), fillStyle); json.insert(QStringLiteral("foreground"), foreground.name(QColor::HexArgb)); json.insert(QStringLiteral("background"), background.name(QColor::HexArgb));
        if (type == QStringLiteral("roundrect")) //20261113-2 add: Radius drafts belong only to rounded rectangles.
        {
            json.insert(QStringLiteral("radiusX"), radiusX); json.insert(QStringLiteral("radiusY"), radiusY);
        }
    }
    return json;
}

/******************************************************************************
 * Function Name   : GraphicProperties::fromJson
 * Description     : Parse bounded finite vertices and exact field types atomically.
 * Input Parameters: json, output, error
 * Return Value    : True after replacing a valid complete draft.
 * Date            : 2026.11.13
 ******************************************************************************/
bool GraphicProperties::fromJson(const QJsonObject &json, GraphicProperties *output, QString *error, const QString &type) //20261113-2 change
{
    GraphicProperties candidate;
    QJsonObject expected = candidate.toJson(type);
    QJsonObject::const_iterator item;
    QJsonArray vertices;
    QJsonArray coordinates;
    QJsonValue vertex;
    double number;

    if (output == nullptr || json.keys() != expected.keys()) { return graphicError(error, QStringLiteral("图形字段缺失或未知。")); }
    for (item = json.constBegin(); item != json.constEnd(); ++item)
    {
        if (item.value().type() != expected.value(item.key()).type()) { return graphicError(error, QStringLiteral("图形字段类型错误。")); }
        if (item.value().isDouble())
        {
            number = item.value().toDouble();
            if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 255) { return graphicError(error, QStringLiteral("图形选项必须为范围内整数。")); }
        }
    }
    candidate.lineStyle = json.value(QStringLiteral("lineStyle")).toInt(); candidate.lineWidth = json.value(QStringLiteral("lineWidth")).toInt();
    candidate.alpha = json.value(QStringLiteral("alpha")).toInt(); candidate.lineColor = QColor(json.value(QStringLiteral("lineColor")).toString());
    if (graphicUsesVertices(type))
    {
        vertices = json.value(QStringLiteral("points")).toArray();
        if (vertices.size() < 2 || vertices.size() > 4096) { return graphicError(error, QStringLiteral("图形点数超出本地资源限制。")); }
        candidate.points.clear();
        Q_FOREACH (vertex, vertices)
        {
            coordinates = vertex.toArray();
            if (!vertex.isArray() || coordinates.size() != 2 || !coordinates[0].isDouble() || !coordinates[1].isDouble()) { return graphicError(error, QStringLiteral("图形坐标必须为二元数值。")); }
            candidate.points.append(QPointF(coordinates[0].toDouble(), coordinates[1].toDouble()));
        }
    }
    if (graphicStrokeOnly(type)) //20261113-4 change: Parse fill on closed vertex objects instead of arrows.
    { candidate.endArrow = json.value(QStringLiteral("endArrow")).toInt(); candidate.endCap = json.value(QStringLiteral("endCap")).toInt(); }
    else { candidate.fillStyle = json.value(QStringLiteral("fillStyle")).toInt(); candidate.foreground = QColor(json.value(QStringLiteral("foreground")).toString()); candidate.background = QColor(json.value(QStringLiteral("background")).toString()); }
    if (type == QStringLiteral("roundrect")) //20261113-2 add: Preserve both independent radius values across resize.
    {
        candidate.radiusX = json.value(QStringLiteral("radiusX")).toInt(); candidate.radiusY = json.value(QStringLiteral("radiusY")).toInt();
    }
    if (!candidate.validate(type, error)) { return false; }
    *output = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : GraphicProperties::validate
 * Description     : Apply observed stroke ranges and portable directed-line invariants.
 * Input Parameters: type, error
 * Return Value    : True for an editable bounded graphic.
 * Date            : 2026.11.13
 ******************************************************************************/
bool GraphicProperties::validate(const QString &type, QString *error) const //20261113-2 add
{
    QPointF point;
    QPointF previous;
    bool hasPrevious = false;

    if (!isSupportedGraphic(type) || lineStyle < 0 || lineStyle > 5 || lineWidth < 1 || lineWidth > 9 || endArrow < 0 || endArrow > 5 || endCap < 0 || endCap > 1 || alpha < 0 || alpha > 255 || !lineColor.isValid() || lineColor.alpha() != 255)
    { return graphicError(error, QStringLiteral("图形线型、线宽、颜色、箭头、端点或透明度无效。")); }
    if (!graphicStrokeOnly(type)) //20261113-4 change: Polygon vertices still carry independent fill drafts.
    {
        if (fillStyle < 0 || fillStyle > 12 || !foreground.isValid() || !background.isValid() || foreground.alpha() != 255 || background.alpha() != 255 || endArrow != 0 || endCap != 0) { return graphicError(error, QStringLiteral("封闭图形填充样式或颜色无效。")); }
        if (type == QStringLiteral("roundrect") && (radiusX < 0 || radiusX > 99 || radiusY < 0 || radiusY > 99)) { return graphicError(error, QStringLiteral("圆角X/Y半径必须为0到99。")); } //20261113-2 add
        if (!graphicUsesVertices(type)) { return true; }
    }
    if (type == QStringLiteral("polygon") && (points.size() < 3 || points.size() > 4096 || points.first() == points.last())) { return graphicError(error, QStringLiteral("多边形需要3到4096个顶点，末点不重复首点。")); } //20261113-4 add: This is an explicit local resource/closure policy.
    if (type == QStringLiteral("line") && (points.size() != 2 || points.first() == points.last())) { return graphicError(error, QStringLiteral("直线需要两个不同端点。")); }
    if (type == QStringLiteral("polyline") && (points.size() < 2 || points.size() > 4096)) { return graphicError(error, QStringLiteral("折线需要2到4096个有序顶点。")); } //20261113-2 add: The resource ceiling is a local editor constraint.
    Q_FOREACH (point, points)
    {
        if (!std::isfinite(point.x()) || !std::isfinite(point.y()) || point.x() < 0 || point.x() > 1 || point.y() < 0 || point.y() > 1) { return graphicError(error, QStringLiteral("图形归一坐标超出有限范围。")); }
        if ((type == QStringLiteral("polyline") || type == QStringLiteral("polygon")) && hasPrevious && point == previous) { return graphicError(error, QStringLiteral("折线相邻顶点不能重合。")); } //20261113-2 add
        previous = point; hasPrevious = true;
    }
    return true;
}

/******************************************************************************
 * Function Name   : graphicHandles
 * Description     : Map ordered endpoint roles into the current geometry after move or resize.
 * Input Parameters: type, rect, properties
 * Return Value    : Ordered editor handles in the supplied coordinate system.
 * Date            : 2026.11.13
 ******************************************************************************/
QVector<QPointF> graphicHandles(const QString &type, const QRectF &rect, const GraphicProperties &properties) //20261113-2 add
{
    QVector<QPointF> result;
    QPointF point;

    if (!isSupportedGraphic(type)) { return result; }
    Q_FOREACH (point, properties.points) { result.append(QPointF(rect.x() + point.x() * rect.width(), rect.y() + point.y() * rect.height())); }
    return result;
}

/******************************************************************************
 * Function Name   : graphicPath
 * Description     : Construct the directed path without substituting its bounding rectangle.
 * Input Parameters: type, rect, properties
 * Return Value    : Ordered line path.
 * Date            : 2026.11.13
 ******************************************************************************/
QPainterPath graphicPath(const QString &type, const QRectF &rect, const GraphicProperties &properties) //20261113-2 add
{
    QVector<QPointF> vertices = graphicHandles(type, rect, properties);
    QPainterPath path;
    int index;

    if (type == QStringLiteral("rect")) { path.addRect(rect); return path; }
    if (type == QStringLiteral("ellipse")) { path.addEllipse(rect); return path; }
    if (type == QStringLiteral("roundrect")) { path.addRoundedRect(rect, properties.radiusX, properties.radiusY, Qt::AbsoluteSize); return path; } //20261113-2 add: Default native raster indicates equal absolute radii on a non-square box; extreme clipping remains a local rule.
    if (type == QStringLiteral("polyline") || type == QStringLiteral("polygon")) //20261113-4 change: Every stored point contributes a genuine segment, without automatic closure.
    {
        if (vertices.size() < 2) { return path; }
        path.moveTo(vertices.first()); for (index = 1; index < vertices.size(); ++index) { path.lineTo(vertices[index]); }
        if (type == QStringLiteral("polygon")) { path.closeSubpath(); } //20261113-4 add: Closure is explicit and never substitutes the enclosing box.
        return path;
    }
    if (vertices.size() != 2) { return path; }
    path.moveTo(vertices[0]); path.lineTo(vertices[1]);
    return path;
}

/******************************************************************************
 * Function Name   : graphicArrow
 * Description     : Build a local editor arrow at the ordered tail; exact native dimensions remain unverified.
 * Input Parameters: type, rect, properties
 * Return Value    : Filled local marker path.
 * Date            : 2026.11.13
 ******************************************************************************/
static QPainterPath graphicArrow(const QString &type, const QRectF &rect, const GraphicProperties &properties) //20261113-2 add
{
    QVector<QPointF> vertices = graphicHandles(type, rect, properties);
    QPainterPath path;
    QTransform transform;
    double size = 6.0 + properties.lineWidth;
    QPointF delta;

    if (!graphicStrokeOnly(type) || vertices.size() < 2 || properties.endArrow == 0) { return path; }
    delta = vertices.last() - vertices[vertices.size() - 2]; transform.translate(vertices.last().x(), vertices.last().y()); //20261113-2 change: Polyline arrows follow the final segment, not the overall box diagonal.
    transform.rotate(std::atan2(delta.y(), delta.x()) * 180.0 / 3.14159265358979323846);
    if (properties.endArrow == 5) { path.addEllipse(QRectF(-size, -size / 2, size, size)); }
    else if (properties.endArrow == 2)
    {
        // A shaft distinguishes the local right-arrow preview from the folded triangle.
        path.moveTo(0, 0); path.lineTo(-size, -size / 2); path.lineTo(-size, -size / 5);
        path.lineTo(-size * 1.6, -size / 5); path.lineTo(-size * 1.6, size / 5);
        path.lineTo(-size, size / 5); path.lineTo(-size, size / 2); path.closeSubpath();
    }
    else
    {
        path.moveTo(0, 0); path.lineTo(-size, -size / 2);
        if (properties.endArrow == 3) { path.lineTo(-size * .65, 0); }
        if (properties.endArrow == 4) { path.lineTo(-size * 2, 0); }
        path.lineTo(-size, size / 2); path.closeSubpath();
    }
    return transform.map(path);
}

/******************************************************************************
 * Function Name   : graphicHitPath
 * Description     : Include the visible stroke and marker with a bounded editor hit tolerance.
 * Input Parameters: type, rect, properties
 * Return Value    : Selectable path excluding empty box corners.
 * Date            : 2026.11.13
 ******************************************************************************/
QPainterPath graphicHitPath(const QString &type, const QRectF &rect, const GraphicProperties &properties) //20261113-2 add
{
    QPainterPathStroker stroker;

    stroker.setWidth(qMax(6, properties.lineWidth));
    stroker.setCapStyle(properties.endCap == 1 ? Qt::RoundCap : Qt::SquareCap);
    if (!graphicStrokeOnly(type)) { return stroker.createStroke(graphicPath(type, rect, properties)).united(graphicPath(type, rect, properties)); }
    return stroker.createStroke(graphicPath(type, rect, properties)).united(graphicArrow(type, rect, properties));
}

/******************************************************************************
 * Function Name   : graphicBrush
 * Description     : Render the thirteen observed fill choices; native pixel calibration remains separate.
 * Input Parameters: rect, properties
 * Return Value    : Local fill brush with independent foreground and background colors.
 * Date            : 2026.11.13
 ******************************************************************************/
static QBrush graphicBrush(const QRectF &rect, const GraphicProperties &properties) //20261113-2 add
{
    QColor front = properties.foreground;
    QColor back = properties.background;
    QPointF start = rect.topLeft();
    QPointF end = rect.topRight();
    QLinearGradient linear;
    QRadialGradient radial;
    bool symmetric = properties.fillStyle == 3 || properties.fillStyle == 5 || properties.fillStyle == 7 || properties.fillStyle == 9;

    front.setAlpha(properties.alpha); back.setAlpha(properties.alpha);
    if (properties.fillStyle == 0) { return QBrush(Qt::NoBrush); }
    if (properties.fillStyle == 1) { return QBrush(back); }
    if (properties.fillStyle == 4 || properties.fillStyle == 5) { end = rect.bottomLeft(); }
    else if (properties.fillStyle == 6 || properties.fillStyle == 7) { end = rect.bottomRight(); }
    else if (properties.fillStyle == 8 || properties.fillStyle == 9) { start = rect.topRight(); end = rect.bottomLeft(); }
    if (properties.fillStyle == 12) { radial = QRadialGradient(rect.center(), qMin(rect.width(), rect.height()) / 2); radial.setColorAt(0, back); radial.setColorAt(1, front); return QBrush(radial); }
    if (properties.fillStyle == 10 || properties.fillStyle == 11) { start = properties.fillStyle == 10 ? rect.topRight() : rect.topLeft(); end = properties.fillStyle == 10 ? rect.bottomLeft() : rect.bottomRight(); linear = QLinearGradient(start, end); linear.setColorAt(0, back); linear.setColorAt(1, front); return QBrush(linear); }
    linear = QLinearGradient(start, end); linear.setColorAt(0, front); linear.setColorAt(symmetric ? .5 : 1.0, back);
    if (symmetric) { linear.setColorAt(1, front); }
    return QBrush(linear);
}

/******************************************************************************
 * Function Name   : paintGraphic
 * Description     : Render the persisted line and its tail without changing surrounding painter state.
 * Input Parameters: painter, type, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void paintGraphic(QPainter *painter, const QString &type, const QRectF &rect, const GraphicProperties &properties) //20261113-2 add
{
    QColor color = properties.lineColor;
    QPen pen;
    const Qt::PenStyle styles[] = {Qt::NoPen, Qt::SolidLine, Qt::DashLine, Qt::DotLine, Qt::DashDotLine, Qt::DashDotDotLine};

    if (painter == nullptr || !properties.validate(type)) { return; }
    color.setAlpha(properties.alpha); pen = QPen(color, properties.lineWidth, styles[properties.lineStyle], properties.endCap == 1 ? Qt::RoundCap : Qt::SquareCap);
    painter->save(); painter->setRenderHint(QPainter::Antialiasing, type == QStringLiteral("ellipse")); painter->setPen(pen); painter->setBrush(graphicStrokeOnly(type) ? QBrush(Qt::NoBrush) : graphicBrush(rect, properties)); //20261113-2 change: Native ellipse edges contain intermediate antialias colors.
    painter->drawPath(graphicPath(type, rect, properties));
    if (properties.lineStyle != 0) { painter->setPen(Qt::NoPen); painter->setBrush(color); painter->drawPath(graphicArrow(type, rect, properties)); }
    painter->restore();
}
