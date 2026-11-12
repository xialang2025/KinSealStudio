"""Connect C22 typed chart forms to editor commands, persistence, canvas and active address roles."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
sources={}
def edit(path,before,after):
 s=sources.get(path,(root/path).read_text(encoding='utf-8'))
 assert s.count(before)==1,(path,before,s.count(before))
 sources[path]=s.replace(before,after)
edit('CMakeLists.txt','    src/move_track_dialog.cpp','    src/move_track_dialog.cpp\n    src/chart_properties.h #20261112-1 add\n    src/chart_properties.cpp\n    src/chart_dialog.h\n    src/chart_dialog.cpp')
edit('CMakeLists.txt','src/wheel_properties.cpp src/move_track_properties.cpp)','src/wheel_properties.cpp src/move_track_properties.cpp src/chart_properties.cpp)')
edit('src/project_model.h','#include "move_track_properties.h"','#include "chart_properties.h" //20261112-1 add\n#include "move_track_properties.h"')
edit('src/project_model.h','    MoveTrackProperties moveTrack;','    ChartProperties chart; //20261112-1 add: Separate bar, meter and donut drafts.\n    MoveTrackProperties moveTrack;')
edit('src/project_model.cpp','    if (type == QStringLiteral("track")) { json.insert','    if (type == QStringLiteral("chart")) { json.insert(QStringLiteral("chart"), chart.toJson()); } //20261112-1 add\n    if (type == QStringLiteral("track")) { json.insert')
edit('src/project_model.cpp','QStringLiteral("moveTrack"), QStringLiteral("extensions")','QStringLiteral("moveTrack"), QStringLiteral("chart"), QStringLiteral("extensions")')
edit('src/project_model.cpp','        else if (key == QStringLiteral("moveTrack"))','        else if (key == QStringLiteral("chart")) //20261112-1 add\n        {\n            if (!item.value().isObject() || !ChartProperties::fromJson(item.value().toObject(), &object->chart, error)) { return false; }\n        }\n        else if (key == QStringLiteral("moveTrack"))')
edit('src/project_model.cpp','    if ((object->type == QStringLiteral("track"))','    if ((object->type == QStringLiteral("chart")) != json.contains(QStringLiteral("chart"))) { return modelError(error, QStringLiteral("图表类型与专有属性不匹配。")); } //20261112-1 add\n    if ((object->type == QStringLiteral("track"))')
edit('src/project_model.cpp','         object->type != QStringLiteral("track") &&','         object->type != QStringLiteral("chart") && //20261112-1 add\n         object->type != QStringLiteral("track") &&')
edit('src/project_model.cpp','object.type == QStringLiteral("track") || object.type == QStringLiteral("wheel")','object.type == QStringLiteral("chart") || object.type == QStringLiteral("track") || object.type == QStringLiteral("wheel")')
edit('src/project_model.cpp','value.toObject().contains(QStringLiteral("moveTrack")))','value.toObject().contains(QStringLiteral("moveTrack")) || value.toObject().contains(QStringLiteral("chart")))')
edit('src/project_model.cpp','        if (object.type == QStringLiteral("track") && (!object.address.isEmpty()', '        if (object.type == QStringLiteral("chart") && (!object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || object.hasImageDisplayProperties || !object.chart.validate(error) || !object.chart.validateAddresses(connections, error))) { return false; } //20261112-1 add\n        if (object.type == QStringLiteral("track") && (!object.address.isEmpty()')
edit('src/stage3_project_edits.cpp','    if (object->type == QStringLiteral("track")) //','''    if (object->type == QStringLiteral("chart")) //20261112-1 add: Increment only the active monitor and dynamic source carriers.
    {
        if (object->chart.shape == 0)
        {
            addressSlots.append({Stage3AddressField::Operation, &object->chart.bar.address, PlcAddressRole::Word, ChartProperties::wordCount(object->chart.bar.dataType)});
            if (object->chart.bar.dynamicRange) { addressSlots.append({Stage3AddressField::DisplayMinimumWord, &object->chart.bar.minimumAddress, PlcAddressRole::Word, ChartProperties::wordCount(object->chart.bar.dataType)}); addressSlots.append({Stage3AddressField::DisplayMaximumWord, &object->chart.bar.maximumAddress, PlcAddressRole::Word, ChartProperties::wordCount(object->chart.bar.dataType)}); }
        }
        else if (object->chart.shape == 1) { addressSlots.append({Stage3AddressField::Operation, &object->chart.meter.address, PlcAddressRole::Word, ChartProperties::wordCount(object->chart.meter.dataType)}); }
        else { addressSlots.append({Stage3AddressField::Operation, &object->chart.donut.address, PlcAddressRole::Word, ChartProperties::wordCount(object->chart.donut.dataType)}); }
        addConditionSlots(&object->chart.visibility, false, &addressSlots);
    }
    if (object->type == QStringLiteral("track")) //''')
edit('src/hmi_canvas.cpp','#include "move_track_dialog.h"','#include "chart_dialog.h" //20261112-1 add\n#include "move_track_dialog.h"')
edit('src/hmi_canvas.cpp','    else if (m_type == QStringLiteral("track"))','    else if (m_type == QStringLiteral("chart")) { paintChart(painter, item_rect, m_record.chart); } //20261112-1 add\n    else if (m_type == QStringLiteral("track"))')
edit('src/hmi_canvas.cpp','object.name = (object.type == QStringLiteral("track")','object.name = (object.type == QStringLiteral("chart") ? QStringLiteral("图表%1") : object.type == QStringLiteral("track")')
edit('src/native_shell_ui.cpp','            else if (id == QStringLiteral("action_moveTrack"))','            else if (id == QStringLiteral("graphicsitem")) //20261112-1 add: Verified native menu and btn_metre share this action.\n            { connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("chart")); }); }\n            else if (id == QStringLiteral("action_moveTrack"))')
edit('src/native_shell_ui.cpp','!selected || object.type == QStringLiteral("track")','!selected || object.type == QStringLiteral("chart") || object.type == QStringLiteral("track")')
edit('src/native_shell_ui.cpp','object.type == QStringLiteral("track") ? QStringLiteral("移动轨迹")','object.type == QStringLiteral("chart") ? QStringLiteral("图表") : object.type == QStringLiteral("track") ? QStringLiteral("移动轨迹")')
edit('src/main_window.cpp','    if (object.type == QStringLiteral("track")) //','    if (object.type == QStringLiteral("chart")) //20261112-1 add: Compact edits affect only the selected chart form monitor.\n    { object.chart.setAddress(object.address.trimmed()); object.address.clear(); if (!object.chart.validateAddresses(m_project.connections, &controlError)) { QMessageBox::warning(this, QStringLiteral("图表属性未提交"), controlError); return; } }\n    if (object.type == QStringLiteral("track")) //')
edit('src/main_window.cpp','|| object.type == QStringLiteral("track")))','|| object.type == QStringLiteral("track") || object.type == QStringLiteral("chart")))')
edit('src/main_window.cpp','        if (object.type == QStringLiteral("track")) { m_addressEdit','        if (object.type == QStringLiteral("chart")) { m_addressEdit->setText(object.chart.address()); } //20261112-1 add\n        if (object.type == QStringLiteral("track")) { m_addressEdit')
edit('src/static_text_actions.cpp','|| object.type == QStringLiteral("track")))','|| object.type == QStringLiteral("track") || object.type == QStringLiteral("chart")))')
edit('src/stage4_control_actions.cpp','#include "move_track_dialog.h"','#include "chart_dialog.h" //20261112-1 add\n#include "move_track_dialog.h"')
edit('src/stage4_control_actions.cpp','    else if (type == QStringLiteral("track")) //','    else if (type == QStringLiteral("chart")) //20261112-1 add\n    { object.name = QStringLiteral("图表"); object.geometry = QRect(0, 0, 170, 130); dialog = new ChartDialog(object, currentEditorProject(), this); }\n    else if (type == QStringLiteral("track")) //')
edit('src/stage4_control_actions.cpp','        else if (type == QStringLiteral("track"))','        else if (type == QStringLiteral("chart")) { object = qobject_cast<ChartDialog *>(dialog)->object(); } //20261112-1 add\n        else if (type == QStringLiteral("track"))')
edit('src/stage4_control_actions.cpp','    else if (object.type == QStringLiteral("track")) { dialog','    else if (object.type == QStringLiteral("chart")) { dialog = new ChartDialog(object, currentEditorProject(), this); } //20261112-1 add\n    else if (object.type == QStringLiteral("track")) { dialog')
edit('src/stage4_control_actions.cpp','        else if (object.type == QStringLiteral("track"))','        else if (object.type == QStringLiteral("chart")) { object = qobject_cast<ChartDialog *>(dialog)->object(); } //20261112-1 add\n        else if (object.type == QStringLiteral("track"))')
for path,s in sources.items(): (root/path).write_text(s,encoding='utf-8',newline='\n')
print('Integrated chart editor, strict schema12, commands, static canvas and active address routing.')
