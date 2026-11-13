#include "hmi_canvas.h"
#include <QContextMenuEvent>
#include <QGraphicsScene>
#include <QGraphicsPathItem> //20261113-2 add
#include <QInputDialog>
#include <QJsonArray>
#include <QLineF>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QUuid>
#include <algorithm>

/******************************************************************************
 * Function Name   : ScreenCanvas::beginStaticTextPlacement
 * Description     : Enter crosshair drag mode only after the appearance dialog was accepted.
 * Input Parameters: object
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void ScreenCanvas::beginStaticTextPlacement(const HmiObject &object) //20261108-14 add
{
    beginControlPlacement(object); //20261108-15 change: Share the same transaction for text and buttons.
}

/******************************************************************************
 * Function Name   : ScreenCanvas::beginControlPlacement
 * Description     : Retain a full control draft after selecting its tool and cancelling previous placement.
 * Input Parameters: object
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void ScreenCanvas::beginControlPlacement(const HmiObject &object) //20261108-15 add
{
    cancelInteraction(); //20261110-8 add: Cancel the preceding interaction before accepting a new property draft.
    if (objects().size() >= 100) { emit controlLimitReached(); return; }
    setActiveTool(QStringLiteral("select"));
    setActiveTool(object.type);
    m_pendingObject = object;
    m_hasPending = true;
}

/******************************************************************************
 * Function Name   : ScreenCanvas::setLanguage
 * Description     : Refresh every item from its own persisted translation map.
 * Input Parameters: language
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void ScreenCanvas::setLanguage(const QString &language) //20261108-14 add
{
    QList<QGraphicsItem *> items;
    QGraphicsItem *item;

    m_language = language;
    items = m_scene->items();
    Q_FOREACH (item, items)
    {
        item->setData(10, language);
        item->update();
    }
    publishSelection(); //20261108-14 change: Refresh property editors before they can apply text to the new language.
}

/******************************************************************************
 * Function Name   : ScreenCanvas::beginResize
 * Description     : Identify any of the eight selected-object handles in page coordinates.
 * Input Parameters: point
 * Return Value    : True when a resize gesture starts.
 * Date            : 2026.11.08
 ******************************************************************************/
bool ScreenCanvas::beginResize(const QPointF &point) //20261108-14 add
{
    HmiObject object;
    bool selected;
    QPointF handle;
    int index;
    QPointF viewportHandle;
    QPointF viewportPoint;
    double nearest;
    double distance;
    int chosen;

    object = selectedObject(&selected);
    if (!selected || m_scene->selectedItems().size() != 1) { return false; }
    if (graphicUsesVertices(object.type)) { return beginGraphicResize(object, point); } //20261113-2 add
    viewportPoint = viewportTransform().map(point);
    nearest = 6.0; chosen = -1;
    for (index = 0; index < 9; ++index)
    {
        if (index == 4) { continue; }
        handle = QPointF(object.geometry.x() + (index % 3) * object.geometry.width() / 2.0,
                          object.geometry.y() + (index / 3) * object.geometry.height() / 2.0);
        viewportHandle = viewportTransform().map(handle);
        distance = QLineF(viewportPoint, viewportHandle).length();
        if (distance <= nearest) //20261110-8 change: Prefer the nearest handle when small controls have overlapping hit zones.
        {
            nearest = distance; chosen = index;
        }
    }
    if (chosen >= 0)
    {
        m_interactionOriginal = objects(); m_interactionSelection = selectedObjectIds();
        m_resizeId = object.id; m_resizeOriginal = object.geometry;
        m_resizeHandle = (chosen % 3 == 0 ? 1 : chosen % 3 == 2 ? 2 : 0) |
                         (chosen / 3 == 0 ? 4 : chosen / 3 == 2 ? 8 : 0);
        return true;
    }
    return false;
}

/******************************************************************************
 * Function Name   : ScreenCanvas::updateResize
 * Description     : Keep the opposite edges fixed and clamp handle movement to the screen.
 * Input Parameters: point
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void ScreenCanvas::updateResize(const QPointF &point) //20261108-14 add
{
    int left;
    int top;
    int right;
    int bottom;
    HmiControlItem *item;
    HmiObject object;
    QGraphicsItem *graphicsItem;

    if (updateGraphicResize(point)) { return; } //20261113-2 add
    left = m_resizeOriginal.x(); top = m_resizeOriginal.y();
    right = left + m_resizeOriginal.width(); bottom = top + m_resizeOriginal.height();
    if (m_resizeHandle & 1) { left = qBound(0, qRound(point.x()), right - 1); }
    if (m_resizeHandle & 2) { right = qBound(left + 1, qRound(point.x()), m_pageWidth); }
    if (m_resizeHandle & 4) { top = qBound(0, qRound(point.y()), bottom - 1); }
    if (m_resizeHandle & 8) { bottom = qBound(top + 1, qRound(point.y()), m_pageHeight); }
    Q_FOREACH (graphicsItem, m_scene->selectedItems())
    {
        item = dynamic_cast<HmiControlItem *>(graphicsItem);
        if (item != nullptr && item->toHmiObject().id == m_resizeId)
        {
            object = item->toHmiObject();
            object.geometry = QRect(left, top, right - left, bottom - top);
            item->setObject(object);
            item->update();
        }
    }
}

/******************************************************************************
 * Function Name   : ScreenCanvas::mouseDoubleClickEvent
 * Description     : Reopen the selected text dialog rather than editing uncommitted inline values.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void ScreenCanvas::mouseDoubleClickEvent(QMouseEvent *event) //20261108-14 add
{
    HmiControlItem *item;

    if (m_hasPending && m_pendingObject.type == QStringLiteral("polyline") && graphicPress(event,mapToScene(event->position().toPoint()))) { return; } //20261113-2 add: Do not open an existing object's properties while a multi-click draft owns the gesture.
    item = dynamic_cast<HmiControlItem *>(itemAt(event->position().toPoint()));
    if (event->button() == Qt::LeftButton && item != nullptr)
    {
        m_scene->clearSelection(); item->setSelected(true); publishSelection();
        emit editObjectRequested();
        event->accept(); return;
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

/******************************************************************************
 * Function Name   : ScreenCanvas::recordHistory
 * Description     : Retain complete property snapshots and drop obsolete redo branches.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void ScreenCanvas::recordHistory() //20261108-14 add
{
    QList<HmiObject> snapshot;
    QJsonArray now;
    QJsonArray previous;
    HmiObject object;

    if (m_restoring) { return; }
    snapshot = objects();
    Q_FOREACH (object, snapshot) { now.append(object.toJson()); }
    if (m_historyIndex >= 0)
    {
        Q_FOREACH (object, m_history.at(m_historyIndex)) { previous.append(object.toJson()); }
        if (now == previous) { return; }
    }
    while (m_history.size() > m_historyIndex + 1) { m_history.removeLast(); }
    m_history.append(snapshot);
    if (m_history.size() > 100) { m_history.removeFirst(); }
    m_historyIndex = m_history.size() - 1;
}

/******************************************************************************
 * Function Name   : ScreenCanvas::restoreObjects
 * Description     : Rebuild canvas items without resetting the current language or undo history.
 * Input Parameters: records
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void ScreenCanvas::restoreObjects(const QList<HmiObject> &records) //20261108-14 add
{
    HmiObject object;
    m_previousSelectionValid = false;
    rebuildPage(m_pageWidth, m_pageHeight, false);
    Q_FOREACH (object, records) { m_scene->addItem(new HmiControlItem(object)); }
    setLanguage(m_language);
    publishSelection();
}

/******************************************************************************
 * Function Name   : ScreenCanvas::executeEdit
 * Description     : Share clipboard, arrangement, grouping and history operations across menus and keys.
 * Input Parameters: command
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void ScreenCanvas::executeEdit(const QString &command) //20261108-14 add
{
    QList<QGraphicsItem *> selected;
    QList<QGraphicsItem *> graphicsItems;
    QList<HmiControlItem *> all;
    QList<HmiControlItem *> ordered;
    QList<HmiObject> source;
    QGraphicsItem *graphicsItem;
    HmiControlItem *item;
    HmiObject object;
    QRect bounds;
    QString group;
    bool ok;
    int index;
    int copy;
    int copies;
    int offset;
    int position;
    QList<HmiObject> before; //20261110-8 add: Mutating commands share the same no-op transaction guard.
    QString referenceId;
    QRect referenceGeometry;
    bool horizontal;
    qreal totalSize;
    qreal gap;
    qreal coordinate;

    cancelInteraction(); before = objects();
    selected = m_scene->selectedItems();
    if (command == QStringLiteral("properties")) { emit editObjectRequested(); return; }
    if (command == QStringLiteral("undo") || command == QStringLiteral("redo"))
    {
        position = m_historyIndex + (command == QStringLiteral("undo") ? -1 : 1);
        if (position < 0 || position >= m_history.size()) { return; }
        m_historyIndex = position;
        m_restoring = true; restoreObjects(m_history.at(position)); publishDocumentChange(before); m_restoring = false;
        return;
    }
    if (command == QStringLiteral("grid")) { m_grid = !m_grid; viewport()->update(); return; }
    if (command.startsWith(QStringLiteral("simulate"))) //20261109-8 change: Lamp previews include all native multi-state indices.
    {
        Q_FOREACH (graphicsItem, selected)
        {
            item = dynamic_cast<HmiControlItem *>(graphicsItem);
            if (item == nullptr) { continue; }
            position = command.mid(8).toInt();
            if (position < 0 || position >= (item->toHmiObject().type == QStringLiteral("message") ? item->toHmiObject().messageDisplay.stateCount : item->toHmiObject().hasFunctionButtonProperties ? 1 : //20261112-0 change
                item->toHmiObject().hasLampProperties ? item->toHmiObject().lamp.stateCount() : 2)) { continue; }
            graphicsItem->setData(11, position); graphicsItem->update();
        }
        return;
    }
    graphicsItems = m_scene->items(Qt::AscendingOrder);
    Q_FOREACH (graphicsItem, graphicsItems)
    {
        item = dynamic_cast<HmiControlItem *>(graphicsItem);
        if (item != nullptr) { all.append(item); }
    }
    if (command == QStringLiteral("selectAll"))
    {
        Q_FOREACH (item, all) { item->setSelected(true); }
        publishSelection(); return;
    }
    if (command == QStringLiteral("copy") || command == QStringLiteral("cut"))
    {
        m_clipboard.clear();
        Q_FOREACH (item, all) { if (item->isSelected()) { m_clipboard.append(item->toHmiObject()); } }
        if (command == QStringLiteral("copy")) { return; }
    }
    if (command == QStringLiteral("delete") || command == QStringLiteral("cut"))
    {
        if (selected.isEmpty()) { return; }
        Q_FOREACH (graphicsItem, selected) { delete graphicsItem; }
        m_previousSelectionValid = false; publishSelection(); publishDocumentChange(before); return;
    }
    if (command == QStringLiteral("paste") || command == QStringLiteral("multiCopy"))
    {
        source = m_clipboard;
        copies = 1;
        if (command == QStringLiteral("multiCopy"))
        {
            source.clear();
            Q_FOREACH (item, all) { if (item->isSelected()) { source.append(item->toHmiObject()); } }
            if (source.isEmpty()) { return; }
            copies = QInputDialog::getInt(this, QStringLiteral("多重复制"), QStringLiteral("复制数量"), 1, 1, 99, 1, &ok);
            if (!ok) { return; }
        }
        if (source.isEmpty()) { return; }
        if (all.size() + source.size() * copies > 100) { emit controlLimitReached(); return; }
        m_scene->clearSelection();
        group = QUuid::createUuid().toString(QUuid::WithoutBraces);
        for (copy = 0; copy < copies; ++copy)
        {
            Q_FOREACH (object, source)
            {
                offset = 16 * (copy + 1);
                object.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                object.name += QStringLiteral(" 副本%1").arg(copy + 1);
                if (!object.group.isEmpty()) { object.group = group + object.group + QString::number(copy); }
                object.geometry.moveTo(qBound(0, object.geometry.x() + offset, m_pageWidth - object.geometry.width()),
                                        qBound(0, object.geometry.y() + offset, m_pageHeight - object.geometry.height()));
                object.layer = all.size() + 1;
                item = new HmiControlItem(object); item->setData(10, m_language);
                m_scene->addItem(item); all.append(item); item->setSelected(true);
            }
        }
        publishSelection(); publishDocumentChange(before); return;
    }
    if (selected.isEmpty()) { return; }
    if (command == QStringLiteral("alignHorizontalSpacing") || command == QStringLiteral("alignVerticalSpacing")) //20261110-8 add
    {
        Q_FOREACH (item, all) { if (item->isSelected()) { ordered.append(item); } }
        if (ordered.size() < 3) { return; }
        horizontal = command == QStringLiteral("alignHorizontalSpacing");
        std::stable_sort(ordered.begin(), ordered.end(), [horizontal](HmiControlItem *first, HmiControlItem *second) {
            return horizontal ? first->toHmiObject().geometry.x() < second->toHmiObject().geometry.x() :
                                first->toHmiObject().geometry.y() < second->toHmiObject().geometry.y();
        });
        totalSize = 0;
        Q_FOREACH (item, ordered)
        {
            object = item->toHmiObject(); totalSize += horizontal ? object.geometry.width() : object.geometry.height();
        }
        bounds = ordered.first()->toHmiObject().geometry; object = ordered.last()->toHmiObject();
        coordinate = horizontal ? bounds.x() : bounds.y();
        gap = ((horizontal ? object.geometry.x() + object.geometry.width() : object.geometry.y() + object.geometry.height()) -
               coordinate - totalSize) / (ordered.size() - 1);
        // Native r3 preserves both endpoint objects and distributes the free edge-to-edge space.
        for (index = 0; index < ordered.size(); ++index)
        {
            item = ordered.at(index); object = item->toHmiObject();
            if (index > 0 && index + 1 < ordered.size())
            {
                if (horizontal) { object.geometry.moveLeft(qRound(coordinate)); }
                else { object.geometry.moveTop(qRound(coordinate)); }
                item->setObject(object); item->update();
            }
            coordinate += (horizontal ? object.geometry.width() : object.geometry.height()) + gap;
        }
        publishSelection(); publishDocumentChange(before); return;
    }
    if (command.startsWith(QStringLiteral("imageMode"))) //20261108-15 add: Image sizing applies to both states, without altering text.
    {
        Q_FOREACH (item, all)
        {
            if (!item->isSelected()) { continue; }
            object = item->toHmiObject();
            if (object.hasBitButtonProperties)
            {
                object.bitButton.states[0].imageMode = command.right(1).toInt();
                object.bitButton.states[1].imageMode = command.right(1).toInt();
                if (object.hasLampProperties) //20261109-8 add: Apply shared canvas image sizing to every retained lamp appearance.
                {
                    for (index = 0; index < object.lamp.extraStates.size(); ++index)
                    {
                        object.lamp.extraStates[index].imageMode = command.right(1).toInt();
                    }
                }
                item->setObject(object); item->update();
            }
        }
        publishDocumentChange(before); return;
    }
    if (command.startsWith(QStringLiteral("layer")))
    {
        ordered = all;
        if (command == QStringLiteral("layerTop") || command == QStringLiteral("layerBottom"))
        {
            ordered.clear();
            Q_FOREACH (item, all) { if (item->isSelected() == (command == QStringLiteral("layerBottom"))) { ordered.append(item); } }
            Q_FOREACH (item, all) { if (item->isSelected() != (command == QStringLiteral("layerBottom"))) { ordered.append(item); } }
        }
        else if (command == QStringLiteral("layerUp"))
        {
            for (index = ordered.size() - 2; index >= 0; --index)
            {
                if (ordered.at(index)->isSelected() && !ordered.at(index + 1)->isSelected()) { ordered.swapItemsAt(index, index + 1); }
            }
        }
        else
        {
            for (index = 1; index < ordered.size(); ++index)
            {
                if (ordered.at(index)->isSelected() && !ordered.at(index - 1)->isSelected()) { ordered.swapItemsAt(index, index - 1); }
            }
        }
        if (ordered != all) //20261110-8 change: An ineffective layer command must retain existing fractional layer values.
        {
            for (index = 0; index < ordered.size(); ++index) { ordered.at(index)->setZValue(index + 1); }
        }
    }
    else
    {
        referenceId = arrangementReferenceObjectId(command); //20261110-8 change: Native r2/r3 position and size operations use different list operands.
        Q_FOREACH (item, all)
        {
            if (item->toHmiObject().id == referenceId) { referenceGeometry = item->toHmiObject().geometry; break; }
        }
        group = command == QStringLiteral("group") ? QUuid::createUuid().toString(QUuid::WithoutBraces) : QString();
        Q_FOREACH (item, all)
        {
            if (!item->isSelected()) { continue; }
            object = item->toHmiObject();
            if (command == QStringLiteral("group") || command == QStringLiteral("ungroup")) { object.group = group; }
            else if (command == QStringLiteral("alignLeft")) { object.geometry.moveLeft(referenceGeometry.x()); }
            else if (command == QStringLiteral("alignRight")) { object.geometry.moveLeft(referenceGeometry.x() + referenceGeometry.width() - object.geometry.width()); }
            else if (command == QStringLiteral("alignTop")) { object.geometry.moveTop(referenceGeometry.y()); }
            else if (command == QStringLiteral("alignBottom")) { object.geometry.moveTop(referenceGeometry.y() + referenceGeometry.height() - object.geometry.height()); }
            else if (command == QStringLiteral("alignCenter")) { object.geometry.moveLeft(qRound(referenceGeometry.x() + (referenceGeometry.width() - object.geometry.width()) / 2.0)); }
            else if (command == QStringLiteral("alignMiddle")) { object.geometry.moveTop(qRound(referenceGeometry.y() + (referenceGeometry.height() - object.geometry.height()) / 2.0)); }
            else if (command == QStringLiteral("alignWidth")) { object.geometry.setWidth(referenceGeometry.width()); }
            else if (command == QStringLiteral("alignHeight")) { object.geometry.setHeight(referenceGeometry.height()); }
            else if (command == QStringLiteral("alignSize")) { object.geometry.setSize(referenceGeometry.size()); }
            item->setObject(object); item->update();
        }
    }
    publishSelection(); publishDocumentChange(before);
}

/******************************************************************************
 * Function Name   : ScreenCanvas::contextMenuEvent
 * Description     : Expose the verified common canvas commands without stealing another window's input.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void ScreenCanvas::contextMenuEvent(QContextMenuEvent *event) //20261108-14 add
{
    QMenu menu(this);
    QMenu *align;
    QMenu *layers;
    QMenu *groups;
    QMenu *images; //20261108-15 add
    QMenu *simulation;
    QAction *action;
    QStringList labels;
    QStringList commands;
    int index;
    int stateCount; //20261109-8 add
    HmiControlItem *item;

    item = dynamic_cast<HmiControlItem *>(itemAt(event->pos()));
    if (item != nullptr && !item->isSelected()) { m_scene->clearSelection(); item->setSelected(true); publishSelection(); }
    labels = {QStringLiteral("属性"),QStringLiteral("剪切\tCtrl+X"),QStringLiteral("复制\tCtrl+C"),QStringLiteral("多重复制"),
              QStringLiteral("粘贴\tCtrl+V"),QStringLiteral("删除\tDel"),QStringLiteral("全选\tCtrl+A")};
    commands = {QStringLiteral("properties"),QStringLiteral("cut"),QStringLiteral("copy"),QStringLiteral("multiCopy"),
                QStringLiteral("paste"),QStringLiteral("delete"),QStringLiteral("selectAll")};
    for (index = 0; index < labels.size(); ++index) { action = menu.addAction(labels.at(index)); action->setData(commands.at(index)); }
    align = menu.addMenu(QStringLiteral("对齐"));
    labels = {QStringLiteral("左对齐"),QStringLiteral("右对齐"),QStringLiteral("上对齐"),QStringLiteral("下对齐"),QStringLiteral("水平居中"),QStringLiteral("垂直居中"),
              QStringLiteral("宽度相同"),QStringLiteral("高度相同"),QStringLiteral("宽度高度都相同"),QStringLiteral("水平等间距"),QStringLiteral("垂直等间距")}; //20261110-8 change
    commands = {QStringLiteral("alignLeft"),QStringLiteral("alignRight"),QStringLiteral("alignTop"),QStringLiteral("alignBottom"),QStringLiteral("alignCenter"),QStringLiteral("alignMiddle"),
                QStringLiteral("alignWidth"),QStringLiteral("alignHeight"),QStringLiteral("alignSize"),QStringLiteral("alignHorizontalSpacing"),QStringLiteral("alignVerticalSpacing")};
    for (index = 0; index < labels.size(); ++index) { align->addAction(labels.at(index))->setData(commands.at(index)); }
    layers = menu.addMenu(QStringLiteral("层次"));
    labels = {QStringLiteral("移到最上层"),QStringLiteral("移到上一层"),QStringLiteral("移到最下层"),QStringLiteral("移到下一层")};
    commands = {QStringLiteral("layerTop"),QStringLiteral("layerUp"),QStringLiteral("layerBottom"),QStringLiteral("layerDown")};
    for (index = 0; index < labels.size(); ++index) { layers->addAction(labels.at(index))->setData(commands.at(index)); }
    groups = menu.addMenu(QStringLiteral("组合"));
    groups->addAction(QStringLiteral("组合"))->setData(QStringLiteral("group"));
    groups->addAction(QStringLiteral("取消组合"))->setData(QStringLiteral("ungroup"));
    menu.addAction(QStringLiteral("网格"))->setData(QStringLiteral("grid"));
    images = menu.addMenu(QStringLiteral("图像比例")); //20261108-15 add
    images->addAction(QStringLiteral("拉伸"))->setData(QStringLiteral("imageMode0"));
    images->addAction(QStringLiteral("保持比例"))->setData(QStringLiteral("imageMode1"));
    images->addAction(QStringLiteral("原始大小"))->setData(QStringLiteral("imageMode2"));
    simulation = menu.addMenu(QStringLiteral("模拟状态"));
    stateCount = item != nullptr && item->toHmiObject().type == QStringLiteral("message") ? item->toHmiObject().messageDisplay.stateCount : item != nullptr && item->toHmiObject().hasFunctionButtonProperties ? 1 : //20261112-0 change
                 item != nullptr && item->toHmiObject().hasLampProperties ? item->toHmiObject().lamp.stateCount() : 2;
    for (index = 0; index < stateCount; ++index)
    {
        simulation->addAction(stateCount == 2 ? (index == 0 ? QStringLiteral("0 / OFF") : QStringLiteral("1 / ON")) :
                              QStringLiteral("状态 %1").arg(index))->setData(QStringLiteral("simulate%1").arg(index));
    }
    action = menu.exec(event->globalPos());
    if (action != nullptr) { requestEditCommand(action->data().toString()); } //20261110-7 change
}

/******************************************************************************
 * Function Name   : ScreenCanvas::keyPressEvent
 * Description     : Dispatch edit shortcuts and cancel pending drawing with Escape.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void ScreenCanvas::keyPressEvent(QKeyEvent *event) //20261108-14 add
{
    QString command;

    if (event->matches(QKeySequence::Copy)) { command = QStringLiteral("copy"); }
    else if (event->matches(QKeySequence::Cut)) { command = QStringLiteral("cut"); }
    else if (event->matches(QKeySequence::Paste)) { command = QStringLiteral("paste"); }
    else if (event->matches(QKeySequence::SelectAll)) { command = QStringLiteral("selectAll"); }
    else if (event->matches(QKeySequence::Undo)) { command = QStringLiteral("undo"); }
    else if (event->matches(QKeySequence::Redo)) { command = QStringLiteral("redo"); }
    else if (event->key() == Qt::Key_Delete) { command = QStringLiteral("delete"); }
    else if (event->key() == Qt::Key_Escape)
    {
        cancelInteraction(); //20261110-8 change: Restore original geometry as well as discarding unplaced resources.
        setActiveTool(QStringLiteral("select")); event->accept(); return;
    }
    if (!command.isEmpty()) { requestEditCommand(command); event->accept(); return; } //20261110-7 change
    QGraphicsView::keyPressEvent(event);
}

/******************************************************************************
 * Function Name   : ScreenCanvas::drawForeground
 * Description     : Paint an optional non-exported editor grid in page pixel coordinates.
 * Input Parameters: painter, rect
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void ScreenCanvas::drawForeground(QPainter *painter, const QRectF &rect) //20261110-8 change
{
    QGraphicsItem *graphics;
    HmiControlItem *item;
    QRectF bounds;
    QPointF handle;
    QTransform mapping;
    int x;
    int y;
    int index;
    Q_UNUSED(rect);

    if (m_grid)
    {
        painter->setPen(QPen(QColor(100, 120, 140, 110), 0));
        for (y = 10; y < m_pageHeight; y += 10)
        {
            for (x = 10; x < m_pageWidth; x += 10) { painter->drawPoint(x, y); }
        }
    }
    if (m_scene->selectedItems().size() != 1) { return; }
    graphics = m_scene->selectedItems().first(); item = dynamic_cast<HmiControlItem *>(graphics);
    if (item == nullptr) { return; }
    bounds = item->toHmiObject().geometry;
    if (graphicUsesVertices(item->toHmiObject().type)) //20261113-2 add: Lines expose their two roles, never eight empty box handles.
    {
        painter->save(); painter->setPen(QPen(QColor(36, 111, 206), 0)); painter->setBrush(Qt::white);
        Q_FOREACH (handle, graphicHandles(item->toHmiObject().type, bounds, item->toHmiObject().graphic)) { painter->drawRect(QRectF(handle.x() - 2.0 * 100 / m_zoomPercent, handle.y() - 2.0 * 100 / m_zoomPercent, 4.0 * 100 / m_zoomPercent, 4.0 * 100 / m_zoomPercent)); }
        painter->restore(); return;
    }
    // Remove the world transform only for editor adornments; device rasterizers never see these pixels.
    mapping = painter->worldTransform(); painter->save(); painter->resetTransform();
    painter->setPen(QPen(QColor(36, 111, 206), 1)); painter->setBrush(Qt::white);
    for (index = 0; index < 9; ++index)
    {
        if (index == 4) { continue; }
        handle = mapping.map(QPointF(bounds.x() + (index % 3) * bounds.width() / 2,
                                     bounds.y() + (index / 3) * bounds.height() / 2));
        painter->drawRect(QRectF(handle.x() - 2, handle.y() - 2, 4, 4));
    }
    painter->restore();
}

/******************************************************************************
 * Function Name   : ScreenCanvas::hasPendingPlacement
 * Description     : Let the host distinguish accepted property resources from a committed control.
 * Input Parameters: None
 * Return Value    : True until the placement is canceled or committed
 * Date            : 2026.11.10
 ******************************************************************************/
bool ScreenCanvas::hasPendingPlacement() const //20261110-8 add
{
    return m_hasPending;
}

/******************************************************************************
 * Function Name   : ScreenCanvas::hasActiveInteraction
 * Description     : Detect drawing, movement or resize state without exposing scene items.
 * Input Parameters: None
 * Return Value    : True for an uncommitted interaction
 * Date            : 2026.11.10
 ******************************************************************************/
bool ScreenCanvas::hasActiveInteraction() const //20261110-8 add
{
    return m_hasPending || m_drawing || m_moving || !m_resizeId.isEmpty();
}

/******************************************************************************
 * Function Name   : ScreenCanvas::cancelInteraction
 * Description     : Roll back gesture geometry and discard pending placement without dirty/history events.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void ScreenCanvas::cancelInteraction() //20261110-8 add
{
    bool canceled;
    bool restore;
    QList<HmiObject> original;
    QStringList selection;

    canceled = hasActiveInteraction(); restore = m_moving || !m_resizeId.isEmpty();
    original = m_interactionOriginal; selection = m_interactionSelection;
    m_hasPending = false; m_drawing = false; m_moving = false;
    m_resizeId.clear(); m_resizeHandle = 0; m_pendingObject = HmiObject();
    m_interactionOriginal.clear(); m_interactionSelection.clear();
    delete m_draftRect; m_draftRect = nullptr;
    delete m_graphicDraft; m_graphicDraft = nullptr; m_graphicVertices.clear(); //20261113-2 change: A canceled multi-click draft cannot leak into the next tool.
    if (canceled)
    {
        m_activeTool = QStringLiteral("select");
        setDragMode(QGraphicsView::RubberBandDrag); setCursor(Qt::ArrowCursor);
    }
    if (restore) { restoreObjects(original); selectObjects(selection); }
    viewport()->update();
    if (canceled) { emit interactionCancelled(); }
}

/******************************************************************************
 * Function Name   : ScreenCanvas::updateMove
 * Description     : Apply one clamped logical delta to the whole selection, preserving relative positions.
 * Input Parameters: point
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void ScreenCanvas::updateMove(const QPointF &point) //20261110-8 add
{
    HmiObject original;
    HmiObject updated;
    QGraphicsItem *graphics;
    HmiControlItem *item;
    int dx;
    int dy;

    dx = qBound(-m_moveBounds.x(), qRound(point.x() - m_moveOrigin.x()), m_pageWidth - m_moveBounds.x() - m_moveBounds.width());
    dy = qBound(-m_moveBounds.y(), qRound(point.y() - m_moveOrigin.y()), m_pageHeight - m_moveBounds.y() - m_moveBounds.height());
    Q_FOREACH (original, m_interactionOriginal)
    {
        if (!m_interactionSelection.contains(original.id)) { continue; }
        Q_FOREACH (graphics, m_scene->items())
        {
            item = dynamic_cast<HmiControlItem *>(graphics);
            if (item != nullptr && item->toHmiObject().id == original.id)
            {
                updated = original; updated.geometry.translate(dx, dy); item->setObject(updated); break;
            }
        }
    }
}

/******************************************************************************
 * Function Name   : ScreenCanvas::selectedObjectIds
 * Description     : Return stable control identities without exposing scene pointers to project tools.
 * Input Parameters: None
 * Return Value    : Selected object IDs.
 * Date            : 2026.11.10
 ******************************************************************************/
QStringList ScreenCanvas::selectedObjectIds() const //20261110-7 add
{
    QStringList ids;
    QGraphicsItem *graphics;
    HmiControlItem *item;

    Q_FOREACH (graphics, m_scene->selectedItems()) {
        item = dynamic_cast<HmiControlItem *>(graphics);
        if (item != nullptr) { ids.append(item->toHmiObject().id); }
    }
    return ids;
}

/******************************************************************************
 * Function Name   : ScreenCanvas::arrangementReferenceObjectId
 * Description     : Report the current position/size operand, keeping Qt collection-order uncertainty visible to the host.
 * Input Parameters: command
 * Return Value    : Selected reference ID, or an empty string for operations without a reference
 * Date            : 2026.11.10
 ******************************************************************************/
QString ScreenCanvas::arrangementReferenceObjectId(const QString &command) const //20261110-8 add
{
    QStringList ids;
    bool sizeCommand;
    bool positionCommand;

    sizeCommand = command == QStringLiteral("alignWidth") || command == QStringLiteral("alignHeight") || command == QStringLiteral("alignSize");
    positionCommand = command == QStringLiteral("alignLeft") || command == QStringLiteral("alignRight") || command == QStringLiteral("alignTop") ||
                      command == QStringLiteral("alignBottom") || command == QStringLiteral("alignCenter") || command == QStringLiteral("alignMiddle");
    if (!sizeCommand && !positionCommand) { return QString(); }
    ids = selectedObjectIds();
    if (ids.isEmpty()) { return QString(); }
    if (ids.contains(m_arrangementReferenceId)) { return m_arrangementReferenceId; }
    // Two native probes support list-first for position and list-last for size; neither establishes mouse selection chronology.
    return sizeCommand ? ids.last() : ids.first();
}

/******************************************************************************
 * Function Name   : ScreenCanvas::setArrangementReferenceObject
 * Description     : Allow an explicit selected reference without relying on cross-version Qt set iteration order.
 * Input Parameters: id; empty or unselected IDs restore the observed native operand policy
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void ScreenCanvas::setArrangementReferenceObject(const QString &id) //20261110-8 add
{
    m_arrangementReferenceId = selectedObjectIds().contains(id) ? id : QString();
}

/******************************************************************************
 * Function Name   : ScreenCanvas::canExecuteEdit
 * Description     : Derive edit availability from actual selection, clipboard and local history state.
 * Input Parameters: command
 * Return Value    : True when the canvas has the required command operands.
 * Date            : 2026.11.10
 ******************************************************************************/
bool ScreenCanvas::canExecuteEdit(const QString &command) const //20261110-7 add
{
    int selected;

    selected = selectedObjectIds().size();
    if (command == QStringLiteral("undo")) { return m_historyIndex > 0; }
    if (command == QStringLiteral("redo")) { return m_historyIndex + 1 < m_history.size(); }
    if (command == QStringLiteral("paste")) { return !m_clipboard.isEmpty() && objects().size() < 100; }
    if (command == QStringLiteral("selectAll")) { return !objects().isEmpty(); }
    if (command == QStringLiteral("grid")) { return true; }
    if (command == QStringLiteral("alignHorizontalSpacing") || command == QStringLiteral("alignVerticalSpacing")) { return selected > 2; } //20261110-8 add
    if (command.startsWith(QStringLiteral("align")) || command == QStringLiteral("group")) { return selected > 1; }
    if (command == QStringLiteral("properties")) { return selected == 1; }
    return selected > 0;
}

/******************************************************************************
 * Function Name   : ScreenCanvas::requestEditCommand
 * Description     : Preserve standalone canvas behavior while sharing hosted key/menu transactions.
 * Input Parameters: command
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void ScreenCanvas::requestEditCommand(const QString &command) //20261110-7 add
{
    if (m_externalEditRouting) { emit editCommandRequested(command); }
    else { executeEdit(command); }
}
