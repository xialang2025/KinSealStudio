#include "hmi_canvas.h"
#include <QContextMenuEvent>
#include <QGraphicsScene>
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
    double tolerance;

    object = selectedObject(&selected);
    if (!selected || m_scene->selectedItems().size() != 1) { return false; }
    tolerance = 6.0 * 100 / m_zoomPercent;
    for (index = 0; index < 9; ++index)
    {
        if (index == 4) { continue; }
        handle = QPointF(object.geometry.x() + (index % 3) * object.geometry.width() / 2.0,
                          object.geometry.y() + (index / 3) * object.geometry.height() / 2.0);
        if (QLineF(point, handle).length() <= tolerance)
        {
            m_resizeId = object.id;
            m_resizeOriginal = object.geometry;
            m_resizeHandle = (index % 3 == 0 ? 1 : index % 3 == 2 ? 2 : 0) |
                             (index / 3 == 0 ? 4 : index / 3 == 2 ? 8 : 0);
            return true;
        }
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

    selected = m_scene->selectedItems();
    if (command == QStringLiteral("properties")) { emit editObjectRequested(); return; }
    if (command == QStringLiteral("undo") || command == QStringLiteral("redo"))
    {
        position = m_historyIndex + (command == QStringLiteral("undo") ? -1 : 1);
        if (position < 0 || position >= m_history.size()) { return; }
        m_historyIndex = position;
        m_restoring = true; restoreObjects(m_history.at(position)); emit documentChanged(); m_restoring = false;
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
            if (position < 0 || position >= (item->toHmiObject().hasFunctionButtonProperties ? 1 : //20261110-0 change
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
        m_previousSelectionValid = false; publishSelection(); emit documentChanged(); return;
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
        publishSelection(); emit documentChanged(); return;
    }
    if (selected.isEmpty()) { return; }
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
        emit documentChanged(); return;
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
        for (index = 0; index < ordered.size(); ++index) { ordered.at(index)->setZValue(index + 1); }
    }
    else
    {
        Q_FOREACH (item, all) { if (item->isSelected()) { bounds = bounds.united(item->toHmiObject().geometry); } }
        group = command == QStringLiteral("group") ? QUuid::createUuid().toString(QUuid::WithoutBraces) : QString();
        Q_FOREACH (item, all)
        {
            if (!item->isSelected()) { continue; }
            object = item->toHmiObject();
            if (command == QStringLiteral("group") || command == QStringLiteral("ungroup")) { object.group = group; }
            else if (command == QStringLiteral("alignLeft")) { object.geometry.moveLeft(bounds.left()); }
            else if (command == QStringLiteral("alignRight")) { object.geometry.moveRight(bounds.right()); }
            else if (command == QStringLiteral("alignTop")) { object.geometry.moveTop(bounds.top()); }
            else if (command == QStringLiteral("alignBottom")) { object.geometry.moveBottom(bounds.bottom()); }
            else if (command == QStringLiteral("alignCenter")) { object.geometry.moveCenter(QPoint(bounds.center().x(), object.geometry.center().y())); }
            else if (command == QStringLiteral("alignMiddle")) { object.geometry.moveCenter(QPoint(object.geometry.center().x(), bounds.center().y())); }
            item->setObject(object); item->update();
        }
    }
    publishSelection(); emit documentChanged();
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
    labels = {QStringLiteral("左对齐"),QStringLiteral("右对齐"),QStringLiteral("顶端对齐"),QStringLiteral("底端对齐"),QStringLiteral("水平居中"),QStringLiteral("垂直居中")};
    commands = {QStringLiteral("alignLeft"),QStringLiteral("alignRight"),QStringLiteral("alignTop"),QStringLiteral("alignBottom"),QStringLiteral("alignCenter"),QStringLiteral("alignMiddle")};
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
    stateCount = item != nullptr && item->toHmiObject().hasFunctionButtonProperties ? 1 : //20261110-0 change
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
        delete m_draftRect; m_draftRect = nullptr; m_drawing = false; m_hasPending = false; m_resizeId.clear();
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
void ScreenCanvas::drawForeground(QPainter *painter, const QRectF &rect) //20261108-14 add
{
    int x;
    int y;
    Q_UNUSED(rect);
    if (!m_grid) { return; }
    painter->setPen(QPen(QColor(100, 120, 140, 110), 0));
    for (y = 10; y < m_pageHeight; y += 10)
    {
        for (x = 10; x < m_pageWidth; x += 10) { painter->drawPoint(x, y); }
    }
}
