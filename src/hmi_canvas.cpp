#include "hmi_canvas.h"
#include "static_text_renderer.h" //20261108-14 add
#include "bit_button_renderer.h" //20261108-15 add

#include <QBrush>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>
#include <QUuid>

/******************************************************************************
 * Function Name   : HmiControlItem::HmiControlItem
 * Description     : Create a selectable canvas item from one portable HMI object record.
 * Input Parameters: object
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
HmiControlItem::HmiControlItem(const HmiObject &object) //20261107-6 add
{
    setObject(object);
}

/******************************************************************************
 * Function Name   : HmiControlItem::toHmiObject
 * Description     : Capture current visual geometry and portable properties for saving.
 * Input Parameters: None
 * Return Value    : HMI object record representing this graphics item.
 * Date            : 2026.11.07
 ******************************************************************************/
HmiObject HmiControlItem::toHmiObject() const //20261107-6 add
{
    HmiObject object;
    QRect geometry;

    object = m_record; //20261108-14 change
    object.layer = zValue();
    object.id = m_id;
    object.type = m_type;
    object.name = m_name;
    object.text = m_text;
    object.address = m_address;
    geometry = rect().toRect();
    geometry.moveTopLeft(pos().toPoint());
    object.geometry = geometry;
    return object;
}

/******************************************************************************
 * Function Name   : HmiControlItem::setObject
 * Description     : Apply object properties while keeping the item movable and selectable.
 * Input Parameters: object
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void HmiControlItem::setObject(const HmiObject &object) //20261107-6 add
{
    m_record = object; //20261108-14 add
    m_id = object.id;
    m_type = object.type;
    m_name = object.name;
    m_text = object.text;
    m_address = object.address;
    setRect(QRectF(0.0, 0.0, object.geometry.width(), object.geometry.height()));
    setPos(object.geometry.topLeft());
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setZValue(object.layer); //20261108-14 change
}

/******************************************************************************
 * Function Name   : HmiControlItem::paint
 * Description     : Draw a readable prototype control using the vendor editor's light canvas style.
 * Input Parameters: painter, option, widget
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void HmiControlItem::paint(QPainter *painter,
                           const QStyleOptionGraphicsItem *option,
                           QWidget *widget) //20261107-6 add
{
    QRectF item_rect;
    QRectF inset_rect;
    QLinearGradient gradient;
    QPen outline_pen;
    QPen selection_pen;
    QColor fill_color;
    QString display_text;
    QPointF handle;
    int handle_index;
    int button_state; //20261109-6 add
    const BitButtonState *lamp_state; //20261109-8 add

    Q_UNUSED(widget);
    item_rect = rect();
    painter->setRenderHint(QPainter::Antialiasing, true);
    outline_pen = QPen(QColor(70, 90, 120), 1.0);
    painter->setPen(outline_pen);

    if (m_type == QStringLiteral("switch"))
    {
        if (m_record.hasBitButtonProperties) //20261108-15 add
        {
            button_state = data(11).toInt() == 1 ? 1 : 0; //20261109-6 change: A word button exposes the pressed state only while its effect is enabled.
            if (m_record.hasWordButtonProperties && !m_record.wordButton.pressedEffect) { button_state = 0; }
            if (m_record.hasFunctionButtonProperties) { button_state = 0; } //20261110-0 add: System functions retain one editable appearance.
            if (m_record.hasLampProperties) //20261109-8 add: Preview every active lamp state without modifying the saved project.
            {
                button_state = qBound(0, data(11).toInt(), m_record.lamp.stateCount() - 1);
                lamp_state = m_record.lamp.state(m_record.bitButton, button_state);
                if (lamp_state != nullptr) { paintBitButton(painter, item_rect, *lamp_state, data(10).toString()); }
            }
            else { paintBitButton(painter, item_rect, m_record.bitButton.states[button_state], data(10).toString()); }
        }
        else
        {
        gradient = QLinearGradient(item_rect.topLeft(), item_rect.bottomLeft());
        gradient.setColorAt(0.0, QColor(53, 160, 226));
        gradient.setColorAt(1.0, QColor(16, 113, 190));
        painter->setBrush(gradient);
        painter->drawRoundedRect(item_rect, 5.0, 5.0);
        painter->setPen(Qt::white);
        painter->drawText(item_rect, Qt::AlignCenter, m_text);
        }
    }
    else if (m_type == QStringLiteral("number"))
    {
        painter->setBrush(QColor(255, 255, 255));
        painter->drawRect(item_rect);
        inset_rect = item_rect.adjusted(4.0, 4.0, -4.0, -4.0);
        painter->setPen(QColor(45, 60, 75));
        painter->drawText(inset_rect, Qt::AlignRight | Qt::AlignVCenter,
                          QStringLiteral("000.00"));
    }
    else if (m_type == QStringLiteral("image"))
    {
        painter->setBrush(QColor(242, 246, 252));
        painter->setPen(QPen(QColor(92, 118, 151), 1.0, Qt::DashLine));
        painter->drawRect(item_rect);
        painter->setPen(QColor(63, 103, 150));
        painter->drawText(item_rect, Qt::AlignCenter, QStringLiteral("圖片"));
    }
    else
    {
#if 0 //20261108-14 del: Shared static-text painting now owns fonts, fills and clipping.
        fill_color = QColor(255, 255, 255, 100);
        painter->setBrush(fill_color);
        painter->setPen(Qt::NoPen);
        painter->drawRect(item_rect);
        painter->setPen(QColor(35, 52, 75));
        display_text = m_text.isEmpty() ? m_name : m_text;
        painter->drawText(item_rect, Qt::AlignCenter, display_text);
#endif
        paintStaticText(painter, item_rect, staticTextContent(m_record, data(10).toString()), m_record.staticText);
    }

    if (option->state & QStyle::State_Selected)
    {
        selection_pen = QPen(QColor(36, 111, 206), 1.0, Qt::DashLine);
        painter->setPen(selection_pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(item_rect.adjusted(1.0, 1.0, -1.0, -1.0));
        painter->setBrush(Qt::white); //20261108-14 add: Eight handles correspond to the resize hit zones.
        for (handle_index = 0; handle_index < 9; ++handle_index)
        {
            if (handle_index == 4) { continue; }
            handle = QPointF(item_rect.left() + (handle_index % 3) * item_rect.width() / 2,
                              item_rect.top() + (handle_index / 3) * item_rect.height() / 2);
            painter->drawRect(QRectF(handle.x() - 2, handle.y() - 2, 4, 4));
        }
    }
}

/******************************************************************************
 * Function Name   : HmiControlItem::itemChange
 * Description     : Keep interactive control movement within the selected HMI screen bounds.
 * Input Parameters: change, value
 * Return Value    : The clamped position or the base graphics-item change result.
 * Date            : 2026.11.07
 ******************************************************************************/
QVariant HmiControlItem::itemChange(GraphicsItemChange change,
                                    const QVariant &value) //20261107-6 add
{
    QPointF position;
    QRectF page_rect;
    QRectF control_rect;
    qreal maximum_x;
    qreal maximum_y;

    if (change == QGraphicsItem::ItemPositionChange && scene() != nullptr)
    {
        position = value.toPointF();
        page_rect = scene()->sceneRect();
        control_rect = rect();
        maximum_x = qMax(page_rect.left(), page_rect.right() - control_rect.width());
        maximum_y = qMax(page_rect.top(), page_rect.bottom() - control_rect.height());
        position.setX(qBound(page_rect.left(), position.x(), maximum_x));
        position.setY(qBound(page_rect.top(), position.y(), maximum_y));
        return position;
    }
    return QGraphicsRectItem::itemChange(change, value);
}

/******************************************************************************
 * Function Name   : ScreenCanvas::ScreenCanvas
 * Description     : Create a scrollable 800 by 480 editor page at the reference screenshot zoom.
 * Input Parameters: parent
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
ScreenCanvas::ScreenCanvas(QWidget *parent) //20261107-6 add
    : QGraphicsView(parent),
      m_scene(new QGraphicsScene(this)),
      m_pageItem(nullptr),
      m_activeTool(QStringLiteral("select")),
      m_pageWidth(800),
      m_pageHeight(480),
      m_zoomPercent(100), //20261108-7 change: Show target pixels at the vendor editor's default scale.
      m_previousSelectionValid(false)
{
    setScene(m_scene);
    setBackgroundBrush(QBrush(Qt::white));
    setFrameShape(QFrame::NoFrame);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setDragMode(QGraphicsView::RubberBandDrag);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    setMouseTracking(true);
    rebuildPage(m_pageWidth, m_pageHeight, false);
    connect(this, &ScreenCanvas::documentChanged, this, &ScreenCanvas::recordHistory); //20261108-14 add
    recordHistory();
}

/******************************************************************************
 * Function Name   : ScreenCanvas::setProject
 * Description     : Rebuild the canvas from one validated project while retaining page scale.
 * Input Parameters: project
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void ScreenCanvas::setProject(const HmiProject &project) //20261107-6 add
{
    QList<HmiObject>::const_iterator object_iterator;

    m_pageWidth = project.pageWidth;
    m_pageHeight = project.pageHeight;
    m_previousSelectionValid = false;
    m_language = project.activeLanguage; //20261108-14 add
    m_draftRect = nullptr;
    m_hasPending = false;
    m_drawing = false;
    m_resizeId.clear();
    setActiveTool(QStringLiteral("select"));
    rebuildPage(m_pageWidth, m_pageHeight, false);
    for (object_iterator = project.objects.constBegin();
         object_iterator != project.objects.constEnd(); ++object_iterator)
    {
        m_scene->addItem(new HmiControlItem(*object_iterator));
    }
    publishSelection();
    setLanguage(project.activeLanguage);
    m_history.clear();
    m_historyIndex = -1;
    recordHistory();
}

/******************************************************************************
 * Function Name   : ScreenCanvas::objects
 * Description     : Return all movable controls in page coordinates for serialization.
 * Input Parameters: None
 * Return Value    : List of current object properties and geometry.
 * Date            : 2026.11.07
 ******************************************************************************/
QList<HmiObject> ScreenCanvas::objects() const //20261107-6 add
{
    QList<HmiObject> result;
    QList<QGraphicsItem *> graphics_items;
    QGraphicsItem *graphics_item;
    HmiControlItem *control_item;
    int item_index;

    graphics_items = m_scene->items(Qt::AscendingOrder);
    for (item_index = 0; item_index < graphics_items.size(); ++item_index)
    {
        graphics_item = graphics_items.at(item_index);
        control_item = dynamic_cast<HmiControlItem *>(graphics_item);
        if (control_item != nullptr)
        {
            result.append(control_item->toHmiObject());
        }
    }
    return result;
}

/******************************************************************************
 * Function Name   : ScreenCanvas::selectedObject
 * Description     : Read the selected control without exposing the graphics scene internals.
 * Input Parameters: selected
 * Return Value    : Selected object; selected reports whether an item exists.
 * Date            : 2026.11.07
 ******************************************************************************/
HmiObject ScreenCanvas::selectedObject(bool *selected) const //20261107-6 add
{
    HmiObject object;
    QList<QGraphicsItem *> selected_items;
    HmiControlItem *control_item;

    selected_items = m_scene->selectedItems();
    control_item = nullptr;
    if (!selected_items.isEmpty())
    {
        control_item = dynamic_cast<HmiControlItem *>(selected_items.first());
    }
    if (selected != nullptr)
    {
        *selected = control_item != nullptr;
    }
    if (control_item != nullptr)
    {
        object = control_item->toHmiObject();
    }
    return object;
}

/******************************************************************************
 * Function Name   : ScreenCanvas::addControl
 * Description     : Place one standard prototype control at a point on the HMI page.
 * Input Parameters: type, position
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void ScreenCanvas::addControl(const QString &type, const QPointF &position) //20261107-6 add
{
    HmiObject object;
    QString type_name;
    QRect geometry;
    HmiControlItem *item;
    QList<QGraphicsItem *> existing_items;
    QGraphicsItem *existing_item;
    HmiControlItem *existing_control;
    QPoint placement;
    int control_count;
    int item_index;

    type_name = type;
    if (type == QStringLiteral("switch"))
    {
        object.name = QStringLiteral("开关1");
        object.text = QStringLiteral("启动");
        geometry = QRect(0, 0, 100, 48);
    }
    else if (type == QStringLiteral("number"))
    {
        object.name = QStringLiteral("数值显示1");
        object.text = QStringLiteral("0.00");
        object.address = QStringLiteral("LW0");
        geometry = QRect(0, 0, 120, 40);
    }
    else if (type == QStringLiteral("image"))
    {
        object.name = QStringLiteral("图片显示1");
        object.text = QStringLiteral("图片");
        geometry = QRect(0, 0, 72, 64);
    }
    else
    {
        object.name = QStringLiteral("静态文本1");
        object.text = QStringLiteral("文本");
        geometry = QRect(0, 0, 120, 32);
    }

    object.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    object.type = type_name;
    existing_items = m_scene->items();
    control_count = 0;
    for (item_index = 0; item_index < existing_items.size(); ++item_index)
    {
        existing_item = existing_items.at(item_index);
        existing_control = dynamic_cast<HmiControlItem *>(existing_item);
        if (existing_control != nullptr)
        {
            ++control_count;
            if (existing_control->toHmiObject().type == type)
            {
                object.name = QStringLiteral("%1%2").arg(object.name.chopped(1))
                                                         .arg(control_count);
            }
        }
    }
    if (control_count >= 100)
    {
        setActiveTool(QStringLiteral("select"));
        emit controlLimitReached();
        return;
    }
    placement = position.toPoint();
    placement.setX(qBound(0, placement.x(), qMax(0, m_pageWidth - geometry.width())));
    placement.setY(qBound(0, placement.y(), qMax(0, m_pageHeight - geometry.height())));
    geometry.moveTopLeft(placement);
    object.geometry = geometry;
    item = new HmiControlItem(object);
    item->setData(10, m_language); //20261108-14 add
    m_scene->clearSelection();
    m_scene->addItem(item);
    item->setSelected(true);
    setActiveTool(QStringLiteral("select"));
    publishSelection();
    emit documentChanged();
}

/******************************************************************************
 * Function Name   : ScreenCanvas::updateSelectedObject
 * Description     : Update the selected control after the user edits its property dialog.
 * Input Parameters: object
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void ScreenCanvas::updateSelectedObject(const HmiObject &object) //20261107-6 add
{
    bool selected;
    QList<QGraphicsItem *> selected_items;
    HmiControlItem *control_item;

    selected_items = m_scene->selectedItems();
    selected = !selected_items.isEmpty();
    control_item = nullptr;
    if (selected)
    {
        control_item = dynamic_cast<HmiControlItem *>(selected_items.first());
    }
    if (control_item != nullptr)
    {
        control_item->setObject(object);
        control_item->update();
        publishSelection();
        emit documentChanged();
    }
}

/******************************************************************************
 * Function Name   : ScreenCanvas::setZoomPercent
 * Description     : Apply the selected percentage zoom used by the canvas status control.
 * Input Parameters: percent
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void ScreenCanvas::setZoomPercent(int percent) //20261107-6 add
{
    qreal factor;

    m_zoomPercent = qBound(25, percent, 200);
    factor = static_cast<qreal>(m_zoomPercent) / 100.0;
    resetTransform();
    scale(factor, factor);
    centerOn(0.0, 0.0);
}

/******************************************************************************
 * Function Name   : ScreenCanvas::setActiveTool
 * Description     : Select placement mode or normal object selection mode.
 * Input Parameters: type
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void ScreenCanvas::setActiveTool(const QString &type) //20261107-6 add
{
    if (type != QStringLiteral("text")) //20261108-14 add: Leaving drawing mode must not retain a stale draft rectangle.
    {
        delete m_draftRect;
        m_draftRect = nullptr;
        m_drawing = false;
        m_hasPending = false;
    }
    m_activeTool = type;
    if (m_activeTool == QStringLiteral("select"))
    {
        setDragMode(QGraphicsView::RubberBandDrag);
        setCursor(Qt::ArrowCursor);
    }
    else
    {
        setDragMode(QGraphicsView::NoDrag);
        setCursor(Qt::CrossCursor);
    }
}

/******************************************************************************
 * Function Name   : ScreenCanvas::selectObject
 * Description     : Select one canvas item by its persistent project object identifier.
 * Input Parameters: id
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void ScreenCanvas::selectObject(const QString &id) //20261107-6 add
{
    QList<QGraphicsItem *> graphics_items;
    QGraphicsItem *graphics_item;
    HmiControlItem *control_item;
    int item_index;

    m_scene->clearSelection();
    graphics_items = m_scene->items();
    for (item_index = 0; item_index < graphics_items.size(); ++item_index)
    {
        graphics_item = graphics_items.at(item_index);
        control_item = dynamic_cast<HmiControlItem *>(graphics_item);
        if (control_item != nullptr && control_item->toHmiObject().id == id)
        {
            control_item->setSelected(true);
            publishSelection();
            return;
        }
    }
    publishSelection();
}

/******************************************************************************
 * Function Name   : ScreenCanvas::mousePressEvent
 * Description     : Place one object on click or pass selection input to the graphics view.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void ScreenCanvas::mousePressEvent(QMouseEvent *event) //20261107-6 add
{
    QPointF scene_point;
    HmiObject selected_object; //20261108-14 add
    HmiControlItem *member;
    QGraphicsItem *graphics_item;
    bool selected;

    scene_point = mapToScene(event->position().toPoint());
    if (event->button() == Qt::LeftButton && m_hasPending) //20261108-14 add
    {
        m_dragOrigin = QPointF(qBound(0.0, scene_point.x(), double(m_pageWidth)),
                               qBound(0.0, scene_point.y(), double(m_pageHeight)));
        m_drawing = true;
        m_draftRect = m_scene->addRect(QRectF(m_dragOrigin, QSizeF()), QPen(Qt::blue, 1, Qt::DashLine));
        m_draftRect->setZValue(10000);
        event->accept();
        return;
    }
    if (event->button() == Qt::LeftButton && m_activeTool == QStringLiteral("select") && beginResize(scene_point))
    {
        event->accept();
        return;
    }
    if (m_activeTool != QStringLiteral("select") && event->button() == Qt::LeftButton)
    {
        addControl(m_activeTool, scene_point);
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
    selected_object = selectedObject(&selected); //20261108-14 add: Selecting one grouped item selects all members for common movement.
    if (selected && !selected_object.group.isEmpty())
    {
        Q_FOREACH (graphics_item, m_scene->items())
        {
            member = dynamic_cast<HmiControlItem *>(graphics_item);
            if (member != nullptr && member->toHmiObject().group == selected_object.group) { member->setSelected(true); }
        }
    }
    publishSelection();
}

/******************************************************************************
 * Function Name   : ScreenCanvas::mouseMoveEvent
 * Description     : Report current HMI page coordinates for the editor status bar.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void ScreenCanvas::mouseMoveEvent(QMouseEvent *event) //20261107-6 add
{
    QPoint point;

    point = mapToScene(event->position().toPoint()).toPoint();
    emit pageCoordinatesChanged(point);
    if (m_drawing && m_draftRect != nullptr) //20261108-14 add
    {
        point.setX(qBound(0, point.x(), m_pageWidth));
        point.setY(qBound(0, point.y(), m_pageHeight));
        m_draftRect->setRect(QRectF(m_dragOrigin, QPointF(point)).normalized());
        event->accept();
        return;
    }
    if (!m_resizeId.isEmpty())
    {
        updateResize(point);
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

/******************************************************************************
 * Function Name   : ScreenCanvas::mouseReleaseEvent
 * Description     : Publish final dragged geometry and document changes after a pointer release.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void ScreenCanvas::mouseReleaseEvent(QMouseEvent *event) //20261107-6 add
{
    HmiObject object;
    HmiControlItem *item;
    QRect geometry;

    if (m_drawing && m_draftRect != nullptr) //20261108-14 add
    {
        geometry = m_draftRect->rect().toRect();
        delete m_draftRect;
        m_draftRect = nullptr;
        m_drawing = false;
        if (geometry.width() > 0 && geometry.height() > 0 && objects().size() < 100)
        {
            object = m_pendingObject;
            object.geometry = geometry;
            object.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            object.name = (object.hasLampProperties ? QStringLiteral("指示灯%1") : //20261109-8 change
                           object.hasFunctionButtonProperties ? QStringLiteral("功能按钮%1") : //20261110-0 add
                           object.hasWordButtonProperties ? QStringLiteral("字按钮%1") :
                           object.type == QStringLiteral("switch") ? QStringLiteral("位按钮%1") :
                           QStringLiteral("静态文本%1")).arg(objects().size() + 1); //20261109-6 change
            object.layer = objects().size() + 1;
            item = new HmiControlItem(object);
            item->setData(10, m_language);
            m_scene->clearSelection();
            m_scene->addItem(item);
            item->setSelected(true);
            emit documentChanged();
        }
        m_hasPending = false;
        setActiveTool(QStringLiteral("select"));
        publishSelection();
        event->accept();
        return;
    }
    if (!m_resizeId.isEmpty())
    {
        m_resizeId.clear();
        publishSelection();
        emit documentChanged();
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
    publishSelection();
}

/******************************************************************************
 * Function Name   : ScreenCanvas::rebuildPage
 * Description     : Replace the background page and preserve the editor's selectable zoom.
 * Input Parameters: width, height
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void ScreenCanvas::rebuildPage(int width, int height, bool preserveItems) //20261107-6 change
{
    QList<QGraphicsItem *> existing_items;
    QGraphicsItem *existing_item;
    HmiControlItem *existing_control;
    QList<HmiObject> previous_objects;
    QGraphicsRectItem *page_item;
    int item_index;

    existing_items = m_scene->items();
    for (item_index = 0; preserveItems && item_index < existing_items.size(); ++item_index)
    {
        existing_item = existing_items.at(item_index);
        existing_control = dynamic_cast<HmiControlItem *>(existing_item);
        if (existing_control != nullptr)
        {
            previous_objects.append(existing_control->toHmiObject());
        }
    }
    m_scene->clear();
    m_draftRect = nullptr; //20261108-14 add
    m_pageWidth = width;
    m_pageHeight = height;
    page_item = m_scene->addRect(QRectF(0.0, 0.0, width, height),
                                 QPen(QColor(90, 101, 116), 1.0),
                                 QBrush(QColor(194, 219, 249)));
    page_item->setZValue(-10.0);
    page_item->setFlag(QGraphicsItem::ItemIsSelectable, false);
    page_item->setFlag(QGraphicsItem::ItemIsMovable, false);
    m_pageItem = page_item;
    m_scene->setSceneRect(QRectF(0.0, 0.0, width, height));
    setZoomPercent(m_zoomPercent);

    for (item_index = 0; item_index < previous_objects.size(); ++item_index)
    {
        m_scene->addItem(new HmiControlItem(previous_objects.at(item_index)));
    }
}

/******************************************************************************
 * Function Name   : ScreenCanvas::publishSelection
 * Description     : Send the selected object record to the property editor or clear its selection.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void ScreenCanvas::publishSelection() //20261107-6 add
{
    bool selected;
    HmiObject object;

    object = selectedObject(&selected);
    emit objectSelectionChanged(object, selected);
    if (selected && m_previousSelectionValid &&
        m_previousSelection.id == object.id &&
        m_previousSelection.geometry != object.geometry)
    {
        emit documentChanged();
    }
    m_previousSelection = object;
    m_previousSelectionValid = selected;
}
