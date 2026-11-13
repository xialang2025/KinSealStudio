"""Add a closed ordered-vertex family while keeping stroke fields separate from topology."""
from pathlib import Path
root = Path(__file__).resolve().parent / 'worktree'

def update(relative, pairs):
    path = root / relative
    text = path.read_text(encoding='utf-8')
    for old, new in pairs:
        assert old in text, (relative, old[:100])
        text = text.replace(old, new)
    path.write_text(text, encoding='utf-8', newline='\n')

update('src/graphic_properties.h', [('bool graphicUsesVertices(const QString &type);', 'bool graphicUsesVertices(const QString &type);\nbool graphicStrokeOnly(const QString &type); //20261113-4 add')])
update('src/graphic_properties.cpp', [
('|| type == QStringLiteral("polyline");', '|| type == QStringLiteral("polyline") || type == QStringLiteral("polygon");'),
('QString graphicName(const QString &type)', 'QString graphicName(const QString &type)'),
('    if (type == QStringLiteral("polyline")) { return QStringLiteral("折线"); }', '    if (type == QStringLiteral("polygon")) { return QStringLiteral("多边形"); } //20261113-4 add\n    if (type == QStringLiteral("polyline")) { return QStringLiteral("折线"); }'),
('json.insert(QStringLiteral("points"), vertices); json.insert(QStringLiteral("endArrow"), endArrow); json.insert(QStringLiteral("endCap"), endCap);', 'json.insert(QStringLiteral("points"), vertices);\n    }\n    if (graphicStrokeOnly(type)) //20261113-4 change: Topology and fill are independent for polygons.\n    {\n        json.insert(QStringLiteral("endArrow"), endArrow); json.insert(QStringLiteral("endCap"), endCap);'),
('        candidate.endArrow = json.value(QStringLiteral("endArrow")).toInt(); candidate.endCap = json.value(QStringLiteral("endCap")).toInt();\n', ''),
('    else { candidate.fillStyle =', '    if (graphicStrokeOnly(type)) //20261113-4 change: Parse fill on closed vertex objects instead of arrows.\n    { candidate.endArrow = json.value(QStringLiteral("endArrow")).toInt(); candidate.endCap = json.value(QStringLiteral("endCap")).toInt(); }\n    else { candidate.fillStyle ='),
('    if (!graphicUsesVertices(type))\n', '    if (!graphicStrokeOnly(type)) //20261113-4 change: Polygon vertices still carry independent fill drafts.\n'),
('        return true;\n    }\n    if (type == QStringLiteral("line")', '        if (!graphicUsesVertices(type)) { return true; }\n    }\n    if (type == QStringLiteral("polygon") && (points.size() < 3 || points.size() > 4096 || points.first() == points.last())) { return graphicError(error, QStringLiteral("多边形需要3到4096个顶点，末点不重复首点。")); } //20261113-4 add: This is an explicit local resource/closure policy.\n    if (type == QStringLiteral("line")'),
('if (type == QStringLiteral("polyline") && hasPrevious', 'if ((type == QStringLiteral("polyline") || type == QStringLiteral("polygon")) && hasPrevious'),
('if (type == QStringLiteral("polyline")) //20261113-2 add: Every', 'if (type == QStringLiteral("polyline") || type == QStringLiteral("polygon")) //20261113-4 change: Every'),
('        return path;\n    }\n    if (vertices.size() != 2)', '        if (type == QStringLiteral("polygon")) { path.closeSubpath(); } //20261113-4 add: Closure is explicit and never substitutes the enclosing box.\n        return path;\n    }\n    if (vertices.size() != 2)'),
('if (!graphicUsesVertices(type) || vertices.size()', 'if (!graphicStrokeOnly(type) || vertices.size()'),
('if (!graphicUsesVertices(type)) { return stroker', 'if (!graphicStrokeOnly(type)) { return stroker'),
('painter->setBrush(graphicUsesVertices(type) ?', 'painter->setBrush(graphicStrokeOnly(type) ?'),
])
# Insert the semantic helper before the name function's documentation.
p = root / 'src/graphic_properties.cpp'
s = p.read_text(encoding='utf-8'); marker = '/******************************************************************************\n * Function Name   : graphicName'
helper = '''/******************************************************************************
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

'''
assert marker in s; p.write_text(s.replace(marker, helper + marker, 1), encoding='utf-8', newline='\n')
for relative in ['src/graphic_canvas.cpp', 'src/canvas_editing.cpp']:
    update(relative, [('m_pendingObject.type == QStringLiteral("polyline")', '(m_pendingObject.type == QStringLiteral("polyline") || m_pendingObject.type == QStringLiteral("polygon"))')])
update('src/graphic_canvas.cpp', [
('        path.lineTo(m_graphicEnd); m_graphicDraft->setPath(path);', '        path.lineTo(m_graphicEnd); if (m_pendingObject.type == QStringLiteral("polygon")) { path.closeSubpath(); } //20261113-4 add: The closing preview edge is transient.\n        m_graphicDraft->setPath(path);')])
update('src/graphic_dialog.cpp', [
('graphicUsesVertices(object.type)', 'graphicStrokeOnly(object.type)'),
('graphicUsesVertices(m_original.type)', 'graphicStrokeOnly(m_original.type)'),
('    preview = m_draft; preview.points = {QPointF(0, 0), QPointF(1, 1)};', '    preview = m_draft; preview.points = {QPointF(0, 0), QPointF(1, 1)};\n    if (m_original.type == QStringLiteral("polygon")) { preview.points = {QPointF(0,0), QPointF(1,0), QPointF(.75,1)}; } //20261113-4 add: Canonical triangle preview does not overwrite the object vertices.')])
update('src/native_shell_ui.cpp', [
('|| id == QStringLiteral("action_foldline"))', '|| id == QStringLiteral("action_foldline") || id == QStringLiteral("action_polygon"))'),
(': QStringLiteral("polyline"); object.text.clear();', ': id == QStringLiteral("action_foldline") ? QStringLiteral("polyline") : QStringLiteral("polygon"); if (object.type == QStringLiteral("polygon")) { object.graphic.foreground = Qt::white; object.graphic.background = Qt::black; } //20261113-4 change: Native polygons default to white foreground and black solid background.\n                    object.text.clear();')])
print('Implemented polygon topology, fill isolation and observed menu gesture.')
