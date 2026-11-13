#include "hmi_canvas.h"
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QLineF>
#include <QMouseEvent>
#include <QUuid>
#include <cmath>

/******************************************************************************
 * Function Name   : assignDirectedLine
 * Description     : Preserve start and tail roles through all quadrants and axis-aligned geometry.
 * Input Parameters: object, first, last
 * Return Value    : True for a nonzero line.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool assignDirectedLine(HmiObject *object, const QPointF &first, const QPointF &last) //20261113-2 add
{
    int left = int(std::floor(qMin(first.x(), last.x())));
    int top = int(std::floor(qMin(first.y(), last.y())));
    int width = qMax(1, int(std::ceil(qMax(first.x(), last.x()))) - left);
    int height = qMax(1, int(std::ceil(qMax(first.y(), last.y()))) - top);

    if (first == last) { return false; }
    object->geometry = QRect(left, top, width, height);
    object->graphic.points = {QPointF((first.x() - left) / width, (first.y() - top) / height), QPointF((last.x() - left) / width, (last.y() - top) / height)};
    return object->graphic.validate(object->type);
}

/******************************************************************************
 * Function Name   : assignGraphicVertices
 * Description     : Normalize every ordered vertex into an outward rounded box without discarding intermediate bends.
 * Input Parameters: object, vertices
 * Return Value    : True for valid nonzero portable geometry.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool assignGraphicVertices(HmiObject *object, const QVector<QPointF> &vertices) //20261113-2 add
{
    QPointF point;
    double minimumX;
    double minimumY;
    double maximumX;
    double maximumY;
    int left;
    int top;
    int width;
    int height;

    if (vertices.size() < 2) { return false; }
    if (vertices.size() == 2) { return assignDirectedLine(object,vertices.first(),vertices.last()); }
    minimumX = maximumX = vertices.first().x(); minimumY = maximumY = vertices.first().y();
    Q_FOREACH (point,vertices) { minimumX = qMin(minimumX,point.x()); maximumX = qMax(maximumX,point.x()); minimumY = qMin(minimumY,point.y()); maximumY = qMax(maximumY,point.y()); }
    if (minimumX == maximumX && minimumY == maximumY) { return false; }
    left = int(std::floor(minimumX)); top = int(std::floor(minimumY)); width = qMax(1,int(std::ceil(maximumX))-left); height = qMax(1,int(std::ceil(maximumY))-top);
    object->geometry = QRect(left,top,width,height); object->graphic.points.clear();
    Q_FOREACH (point,vertices) { object->graphic.points.append(QPointF((point.x()-left)/width,(point.y()-top)/height)); }
    return object->graphic.validate(object->type);
}

/******************************************************************************
 * Function Name   : HmiControlItem::boundingRect
 * Description     : Include the actual line marker extents in scene culling and repaint.
 * Input Parameters: None
 * Return Value    : Local item bounds including the marker.
 * Date            : 2026.11.13
 ******************************************************************************/
QRectF HmiControlItem::boundingRect() const //20261113-2 add
{
    if (isSupportedGraphic(m_type)) { return graphicHitPath(m_type, rect(), m_record.graphic).boundingRect().adjusted(-1, -1, 1, 1); }
    return QGraphicsRectItem::boundingRect();
}

/******************************************************************************
 * Function Name   : HmiControlItem::shape
 * Description     : Hit-test the ordered stroke rather than an empty rectangular interior.
 * Input Parameters: None
 * Return Value    : Local selectable shape.
 * Date            : 2026.11.13
 ******************************************************************************/
QPainterPath HmiControlItem::shape() const //20261113-2 add
{
    if (isSupportedGraphic(m_type)) { return graphicHitPath(m_type, rect(), m_record.graphic); }
    return QGraphicsRectItem::shape();
}

/******************************************************************************
 * Function Name   : ScreenCanvas::graphicPress
 * Description     : Begin immediate line drawing without opening an unobserved pre-placement dialog.
 * Input Parameters: event, point
 * Return Value    : True when the graphic consumes the event.
 * Date            : 2026.11.13
 ******************************************************************************/
bool ScreenCanvas::graphicPress(QMouseEvent *event, const QPointF &point) //20261113-2 add
{
    QPainterPath path;
    QPointF vertex;
    QList<HmiObject> before;
    HmiObject object;
    HmiControlItem *item;
    int index;

    if (!m_hasPending || !graphicUsesVertices(m_pendingObject.type)) { return false; }
    if (m_pendingObject.type == QStringLiteral("polyline")) //20261113-2 add: Left clicks add vertices; only the observed right-click gesture commits the path.
    {
        if (event->button() == Qt::RightButton)
        {
            object = m_pendingObject;
            if (objects().size() >= 100 || !assignGraphicVertices(&object,m_graphicVertices)) { cancelInteraction(); event->accept(); return true; }
            before = objects(); delete m_graphicDraft; m_graphicDraft = nullptr; m_graphicVertices.clear(); m_drawing = false; m_hasPending = false; m_pendingObject = HmiObject();
            object.id = QUuid::createUuid().toString(QUuid::WithoutBraces); object.name = graphicName(object.type)+QString::number(before.size()+1); object.layer = before.size()+1;
            item = makeControlItem(object); m_scene->clearSelection(); m_scene->addItem(item); item->setSelected(true);
            setActiveTool(QStringLiteral("select")); publishSelection(); publishDocumentChange(before); event->accept(); return true;
        }
        if (event->button() != Qt::LeftButton) { return false; }
        vertex = QPointF(qBound(0,qRound(point.x()),m_pageWidth-1),qBound(0,qRound(point.y()),m_pageHeight-1));
        if (m_graphicVertices.size() >= 4096) { cancelInteraction(); event->accept(); return true; }
        if (m_graphicVertices.isEmpty() || m_graphicVertices.last() != vertex) { m_graphicVertices.append(vertex); }
        m_drawing = true; m_graphicEnd = vertex; path.moveTo(m_graphicVertices.first());
        for (index = 1; index < m_graphicVertices.size(); ++index) { path.lineTo(m_graphicVertices[index]); }
        if (m_graphicDraft == nullptr) { m_graphicDraft = m_scene->addPath(path,QPen(Qt::blue,0)); m_graphicDraft->setZValue(10000); }
        else { m_graphicDraft->setPath(path); }
        event->accept(); return true;
    }
    if (event->button() != Qt::LeftButton) { return false; }
    m_dragOrigin = QPointF(qBound(0, qRound(point.x()), m_pageWidth - 1), qBound(0, qRound(point.y()), m_pageHeight - 1));
    m_graphicEnd = m_dragOrigin; m_drawing = true;
    path.moveTo(m_dragOrigin); path.lineTo(m_graphicEnd);
    m_graphicDraft = m_scene->addPath(path, QPen(Qt::blue, 0)); m_graphicDraft->setZValue(10000);
    event->accept();
    return true;
}

/******************************************************************************
 * Function Name   : ScreenCanvas::graphicMove
 * Description     : Show the directed line itself while the drawing gesture remains uncommitted.
 * Input Parameters: event, point
 * Return Value    : True when the graphic consumes the event.
 * Date            : 2026.11.13
 ******************************************************************************/
bool ScreenCanvas::graphicMove(QMouseEvent *event, const QPointF &point) //20261113-2 add
{
    QPainterPath path;
    int index;

    if (!m_drawing || m_graphicDraft == nullptr) { return false; }
    m_graphicEnd = QPointF(qBound(0, qRound(point.x()), m_pageWidth - 1), qBound(0, qRound(point.y()), m_pageHeight - 1));
    if (m_pendingObject.type == QStringLiteral("polyline")) //20261113-2 add: A rubber-band tail is transient and never silently serialized.
    {
        path.moveTo(m_graphicVertices.first()); for (index = 1; index < m_graphicVertices.size(); ++index) { path.lineTo(m_graphicVertices[index]); }
        path.lineTo(m_graphicEnd); m_graphicDraft->setPath(path); event->accept(); return true;
    }
    path.moveTo(m_dragOrigin); path.lineTo(m_graphicEnd); m_graphicDraft->setPath(path); event->accept();
    return true;
}

/******************************************************************************
 * Function Name   : ScreenCanvas::graphicRelease
 * Description     : Create exactly one directed object and one document transaction on release.
 * Input Parameters: event
 * Return Value    : True when the graphic consumes the event.
 * Date            : 2026.11.13
 ******************************************************************************/
bool ScreenCanvas::graphicRelease(QMouseEvent *event) //20261113-2 add
{
    QList<HmiObject> before;
    HmiObject object;
    HmiControlItem *item;
    QPointF point;

    if (event->button() != Qt::LeftButton || !m_drawing || m_graphicDraft == nullptr) { return false; }
    if (m_pendingObject.type == QStringLiteral("polyline")) { event->accept(); return true; } //20261113-2 add: Releasing one click does not create a separate object or undo entry.
    point = mapToScene(event->position().toPoint());
    m_graphicEnd = QPointF(qBound(0, qRound(point.x()), m_pageWidth - 1), qBound(0, qRound(point.y()), m_pageHeight - 1));
    object = m_pendingObject;
    if (objects().size() >= 100 || !assignDirectedLine(&object, m_dragOrigin, m_graphicEnd)) { cancelInteraction(); event->accept(); return true; }
    before = objects(); delete m_graphicDraft; m_graphicDraft = nullptr;
    m_drawing = false; m_hasPending = false; m_pendingObject = HmiObject();
    object.id = QUuid::createUuid().toString(QUuid::WithoutBraces); object.name = graphicName(object.type) + QString::number(before.size() + 1); object.layer = before.size() + 1;
    item = makeControlItem(object); m_scene->clearSelection(); m_scene->addItem(item); item->setSelected(true);
    setActiveTool(QStringLiteral("select")); publishSelection(); publishDocumentChange(before); event->accept();
    return true;
}

/******************************************************************************
 * Function Name   : ScreenCanvas::beginGraphicResize
 * Description     : Capture an ordered endpoint and the complete pre-gesture snapshot.
 * Input Parameters: object, point
 * Return Value    : True when an endpoint is within the editor hit tolerance.
 * Date            : 2026.11.13
 ******************************************************************************/
bool ScreenCanvas::beginGraphicResize(const HmiObject &object, const QPointF &point) //20261113-2 add
{
    QVector<QPointF> handles = graphicHandles(object.type, object.geometry, object.graphic);
    int index;
    int chosen = -1;
    double nearest = 6;
    double distance;

    for (index = 0; index < handles.size(); ++index)
    {
        distance = QLineF(viewportTransform().map(point), viewportTransform().map(handles[index])).length();
        if (distance <= nearest) { chosen = index; nearest = distance; }
    }
    if (chosen < 0) { return false; }
    m_interactionOriginal = objects(); m_interactionSelection = selectedObjectIds(); m_resizeId = object.id;
    m_resizeOriginal = object.geometry; m_resizeHandle = -chosen - 1;
    return true;
}

/******************************************************************************
 * Function Name   : ScreenCanvas::updateGraphicResize
 * Description     : Move one endpoint without reversing its role, with cancellation handled by the shared snapshot.
 * Input Parameters: point
 * Return Value    : True when processing a graphic endpoint gesture.
 * Date            : 2026.11.13
 ******************************************************************************/
bool ScreenCanvas::updateGraphicResize(const QPointF &point) //20261113-2 add
{
    HmiObject original;
    HmiObject object;
    QGraphicsItem *graphics;
    HmiControlItem *item;
    QVector<QPointF> handles;

    if (m_resizeHandle >= 0 || m_resizeId.isEmpty()) { return false; }
    Q_FOREACH (original, m_interactionOriginal)
    {
        if (original.id != m_resizeId || !isSupportedGraphic(original.type)) { continue; }
        handles = graphicHandles(original.type, original.geometry, original.graphic);
        handles[-m_resizeHandle - 1] = QPointF(qBound(0, qRound(point.x()), m_pageWidth - 1), qBound(0, qRound(point.y()), m_pageHeight - 1));
        object = original;
        if (!assignGraphicVertices(&object,handles)) { return true; } //20261113-2 change: Moving an intermediate handle must retain all other vertices.
        Q_FOREACH (graphics, m_scene->selectedItems())
        {
            item = dynamic_cast<HmiControlItem *>(graphics);
            if (item != nullptr && item->toHmiObject().id == object.id) { item->setObject(object); item->update(); }
        }
        return true;
    }
    return false;
}
