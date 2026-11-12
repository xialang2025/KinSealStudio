"""Integrate G01 without modifying native encoders or the user's separate stage3 changes."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'

def edit(name, replacements):
    p=root/name;s=p.read_text(encoding='utf-8')
    for old,new in replacements:
        assert s.count(old)==1,(name,old[:100],s.count(old))
        s=s.replace(old,new,1)
    p.write_text(s,encoding='utf-8',newline='\n')

edit('CMakeLists.txt', [('    src/recipe_display_dialog.cpp','    src/recipe_display_dialog.cpp\n    src/graphic_properties.h #20261113-2 add\n    src/graphic_properties.cpp\n    src/graphic_dialog.h\n    src/graphic_dialog.cpp\n    src/graphic_canvas.cpp'), ('src/recipe_csv.cpp)', 'src/recipe_csv.cpp src/graphic_properties.cpp)')])
edit('src/project_model.h', [('#include "recipe_display_properties.h"', '#include "graphic_properties.h" //20261113-2 add\n#include "recipe_display_properties.h"'), ('    RecipeDisplayProperties recipeDisplay;', '    GraphicProperties graphic; //20261113-2 add: Ordered normalized geometry is a Lite contract.\n    RecipeDisplayProperties recipeDisplay;')])
edit('src/project_model.cpp', [
('    if (type == QStringLiteral("recipe")) { json.insert', '    if (isSupportedGraphic(type)) { json.insert(QStringLiteral("graphic"), graphic.toJson()); } //20261113-2 add\n    if (type == QStringLiteral("recipe")) { json.insert'),
('QStringLiteral("recipeDisplay"), QStringLiteral("extensions")', 'QStringLiteral("recipeDisplay"), QStringLiteral("graphic"), QStringLiteral("extensions")'),
('        else if (key == QStringLiteral("recipeDisplay"))', '        else if (key == QStringLiteral("graphic")) //20261113-2 add\n        {\n            if (!item.value().isObject() || !GraphicProperties::fromJson(item.value().toObject(), &object->graphic, error)) { return false; }\n        }\n        else if (key == QStringLiteral("recipeDisplay"))'),
('    if ((object->type == QStringLiteral("recipe")) != json.contains', '    if (isSupportedGraphic(object->type) != json.contains(QStringLiteral("graphic")) || (isSupportedGraphic(object->type) && !object->graphic.validate(object->type, error))) { return modelError(error, QStringLiteral("图形类型与专有数据不匹配。")); } //20261113-2 add\n    if ((object->type == QStringLiteral("recipe")) != json.contains'),
('        (object->type != QStringLiteral("switch") &&', '        (!isSupportedGraphic(object->type) && object->type != QStringLiteral("switch") &&'),
('object.type == QStringLiteral("recipe") || object.type == QStringLiteral("track")', 'object.type == QStringLiteral("recipe") || isSupportedGraphic(object.type) || object.type == QStringLiteral("track")'),
('value.toObject().contains(QStringLiteral("recipeDisplay")))', 'value.toObject().contains(QStringLiteral("recipeDisplay")) || value.toObject().contains(QStringLiteral("graphic")))'),
('    Q_FOREACH (object, allObjects)\n    {', '    Q_FOREACH (object, allObjects)\n    {\n        if (isSupportedGraphic(object.type) && (!object.text.isEmpty() || !object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || object.hasImageDisplayProperties || !object.graphic.validate(object.type, error))) { return modelError(error, QStringLiteral("静态图形必须使用有效独立属性且没有PLC地址或文本。")); } //20261113-2 add')])
edit('src/hmi_canvas.h', [
('class QGraphicsScene;', 'class QGraphicsScene;\nclass QGraphicsPathItem;'),
('    void setObject(const HmiObject &object);', '    void setObject(const HmiObject &object);\n    QRectF boundingRect() const override; //20261113-2 add\n    QPainterPath shape() const override;'),
('    bool m_grid = false;', '    QGraphicsPathItem *m_graphicDraft = nullptr; //20261113-2 add\n    QPointF m_graphicEnd;\n    bool graphicPress(QMouseEvent *event, const QPointF &point);\n    bool graphicMove(QMouseEvent *event, const QPointF &point);\n    bool graphicRelease(QMouseEvent *event);\n    bool beginGraphicResize(const HmiObject &object, const QPointF &point);\n    bool updateGraphicResize(const QPointF &point);\n    bool m_grid = false;')])
edit('src/hmi_canvas.cpp', [
('    m_record = object; //20261108-14 add', '    prepareGeometryChange(); //20261113-2 add: Marker and hit extents can change independently of the box.\n    m_record = object; //20261108-14 add'),
('    if (m_type == QStringLiteral("gif"))', '    if (isSupportedGraphic(m_type)) { paintGraphic(painter, m_type, item_rect, m_record.graphic); } //20261113-2 add\n    else if (m_type == QStringLiteral("gif"))'),
('    if (event->button() == Qt::LeftButton && m_hasPending)', '    if (graphicPress(event, point)) { return; } //20261113-2 add\n    if (event->button() == Qt::LeftButton && m_hasPending)'),
('    if (m_drawing && m_draftRect != nullptr)\n    {\n        point.setX', '    if (graphicMove(event, point)) { return; } //20261113-2 add\n    if (m_drawing && m_draftRect != nullptr)\n    {\n        point.setX'),
('    if (event->button() != Qt::LeftButton) { QGraphicsView::mouseReleaseEvent(event); return; }', '    if (graphicRelease(event)) { return; } //20261113-2 add\n    if (event->button() != Qt::LeftButton) { QGraphicsView::mouseReleaseEvent(event); return; }')])
edit('src/canvas_editing.cpp', [
('#include <QGraphicsScene>', '#include <QGraphicsScene>\n#include <QGraphicsPathItem> //20261113-2 add'),
('    viewportPoint = viewportTransform().map(point);', '    if (isSupportedGraphic(object.type)) { return beginGraphicResize(object, point); } //20261113-2 add\n    viewportPoint = viewportTransform().map(point);'),
('    left = m_resizeOriginal.x(); top = m_resizeOriginal.y();', '    if (updateGraphicResize(point)) { return; } //20261113-2 add\n    left = m_resizeOriginal.x(); top = m_resizeOriginal.y();'),
('    bounds = item->toHmiObject().geometry;', '    bounds = item->toHmiObject().geometry;\n    if (isSupportedGraphic(item->toHmiObject().type)) //20261113-2 add: Lines expose their two roles, never eight empty box handles.\n    {\n        painter->save(); painter->setPen(QPen(QColor(36, 111, 206), 0)); painter->setBrush(Qt::white);\n        Q_FOREACH (handle, graphicHandles(item->toHmiObject().type, bounds, item->toHmiObject().graphic)) { painter->drawRect(QRectF(handle.x() - 2.0 * 100 / m_zoomPercent, handle.y() - 2.0 * 100 / m_zoomPercent, 4.0 * 100 / m_zoomPercent, 4.0 * 100 / m_zoomPercent)); }\n        painter->restore(); return;\n    }'),
('    delete m_draftRect; m_draftRect = nullptr;', '    delete m_draftRect; m_draftRect = nullptr;\n    delete m_graphicDraft; m_graphicDraft = nullptr; //20261113-2 add')])
edit('src/native_shell_ui.cpp', [
('            if (id == QStringLiteral("action_qrcode"))', '            if (id == QStringLiteral("action_line")) //20261113-2 add: Native line creation enters drawing immediately.\n            {\n                connect(action, &QAction::triggered, this, [this]() { HmiObject object; object.type = QStringLiteral("line"); object.text.clear(); object.address.clear(); m_canvas->beginControlPlacement(object); });\n            }\n            else if (id == QStringLiteral("action_qrcode"))'),
('object.type == QStringLiteral("recipe") ? QStringLiteral("配方显示器")', 'isSupportedGraphic(object.type) ? graphicName(object.type) : object.type == QStringLiteral("recipe") ? QStringLiteral("配方显示器")')])
edit('src/main_window.cpp', [('    m_textEdit->setEnabled(selected);', '    m_textEdit->setEnabled(selected);\n    if (selected && isSupportedGraphic(object.type)) { m_textEdit->setEnabled(false); m_addressEdit->setEnabled(false); } //20261113-2 add'), ('    if (selected && object.hasNumberDisplayProperties)', '    if (selected && isSupportedGraphic(object.type)) { m_addressEdit->setEnabled(false); } //20261113-2 add\n    if (selected && object.hasNumberDisplayProperties)')])
edit('src/static_text_actions.cpp', [('    if (selected && (object.type == QStringLiteral("message")', '    if (selected && isSupportedGraphic(object.type)) { editStage4Control(); return; } //20261113-2 add\n    if (selected && (object.type == QStringLiteral("message")')])
edit('src/stage4_control_actions.cpp', [
('#include "main_window.h"', '#include "main_window.h"\n#include "graphic_dialog.h" //20261113-2 add'),
('    if (object.type == QStringLiteral("message")) { dialog = new MessageDisplayDialog', '    if (isSupportedGraphic(object.type)) { dialog = new GraphicDialog(object, this); } //20261113-2 add\n    else if (object.type == QStringLiteral("message")) { dialog = new MessageDisplayDialog'),
('        if (object.type == QStringLiteral("message")) { object = qobject_cast<MessageDisplayDialog', '        if (isSupportedGraphic(object.type)) { object = qobject_cast<GraphicDialog *>(dialog)->object(); } //20261113-2 add\n        else if (object.type == QStringLiteral("message")) { object = qobject_cast<MessageDisplayDialog')])
print('Integrated directed line editor; SM/device allowlists unchanged.')
