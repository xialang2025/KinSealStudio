"""Integrate the independently typed track editor, resource bindings and conservative export gates."""
from pathlib import Path
root = Path(__file__).resolve().parent / 'worktree'
sources = {}


def edit(relative, before, after):
    source = sources.get(relative, (root / relative).read_text(encoding='utf-8'))
    assert source.count(before) == 1, (relative, before, source.count(before))
    sources[relative] = source.replace(before, after)


edit('CMakeLists.txt', '    src/wheel_dialog.cpp', '    src/wheel_dialog.cpp\n    src/move_track_properties.h #20261112-0 add\n    src/move_track_properties.cpp\n    src/move_track_dialog.h\n    src/move_track_dialog.cpp')
edit('CMakeLists.txt', 'src/slider_properties.cpp src/wheel_properties.cpp)', 'src/slider_properties.cpp src/wheel_properties.cpp src/move_track_properties.cpp)')
edit('src/project_model.h', '#include "wheel_properties.h"', '#include "move_track_properties.h" //20261112-0 add\n#include "wheel_properties.h"')
edit('src/project_model.h', '    WheelProperties wheel;', '    MoveTrackProperties moveTrack; //20261112-0 add: Independent distance, graphic and constant/address drafts.\n    WheelProperties wheel;')
edit('src/project_model.cpp', '    if (type == QStringLiteral("wheel")) { json.insert', '    if (type == QStringLiteral("track")) { json.insert(QStringLiteral("moveTrack"), moveTrack.toJson()); } //20261112-0 add\n    if (type == QStringLiteral("wheel")) { json.insert')
edit('src/project_model.cpp', 'QStringLiteral("wheel"), QStringLiteral("extensions")', 'QStringLiteral("wheel"), QStringLiteral("moveTrack"), QStringLiteral("extensions")')
edit('src/project_model.cpp', '        else if (key == QStringLiteral("wheel"))', '        else if (key == QStringLiteral("moveTrack")) //20261112-0 add\n        {\n            if (!item.value().isObject() || !MoveTrackProperties::fromJson(item.value().toObject(), &object->moveTrack, error)) { return false; }\n        }\n        else if (key == QStringLiteral("wheel"))')
edit('src/project_model.cpp', '    if ((object->type == QStringLiteral("wheel"))', '    if ((object->type == QStringLiteral("track")) != json.contains(QStringLiteral("moveTrack"))) { return modelError(error, QStringLiteral("移动轨迹类型与专有属性不匹配。")); } //20261112-0 add\n    if ((object->type == QStringLiteral("wheel"))')
edit('src/project_model.cpp', '         object->type != QStringLiteral("wheel") &&', '         object->type != QStringLiteral("track") && //20261112-0 add\n         object->type != QStringLiteral("wheel") &&')
edit('src/project_model.cpp', 'object.type == QStringLiteral("wheel") || object.type == QStringLiteral("slider")', 'object.type == QStringLiteral("track") || object.type == QStringLiteral("wheel") || object.type == QStringLiteral("slider")')
edit('src/project_model.cpp', 'value.toObject().contains(QStringLiteral("wheel")))', 'value.toObject().contains(QStringLiteral("wheel")) || value.toObject().contains(QStringLiteral("moveTrack")))')
edit('src/project_model.cpp', '        if (object.type == QStringLiteral("wheel") && (!object.address.isEmpty()', '        if (object.type == QStringLiteral("track") && (!object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || object.hasImageDisplayProperties || !object.moveTrack.validate(error) || !object.moveTrack.validateAddresses(connections, error))) { return false; } //20261112-0 add\n        if (object.type == QStringLiteral("wheel") && (!object.address.isEmpty()')
edit('src/stage3_project_edits.cpp', '    if (object->type == QStringLiteral("wheel")) //', '''    if (object->type == QStringLiteral("track")) //20261112-0 add: Track constants and dynamic ranges have independent word-carrier roles.
    {
        addressSlots.append({Stage3AddressField::Operation, &object->moveTrack.address, PlcAddressRole::Word, 1U});
        if (object->moveTrack.dynamicRange) { addressSlots.append({Stage3AddressField::DisplayMinimumWord, &object->moveTrack.minimumAddress, PlcAddressRole::Word, 1U}); addressSlots.append({Stage3AddressField::DisplayMaximumWord, &object->moveTrack.maximumAddress, PlcAddressRole::Word, 1U}); }
        addConditionSlots(&object->moveTrack.visibility, false, &addressSlots); addConditionSlots(&object->moveTrack.touch, true, &addressSlots);
        if (object->moveTrack.notice && object->moveTrack.noticeBit) { addressSlots.append({Stage3AddressField::NoticeBit, &object->moveTrack.noticeBitAddress, PlcAddressRole::Bit, 1U}); }
        if (object->moveTrack.notice && object->moveTrack.noticeWord) { addressSlots.append({Stage3AddressField::NoticeWord, &object->moveTrack.noticeWordAddress, PlcAddressRole::Word, object->moveTrack.noticeWordType == 0 || object->moveTrack.noticeWordType == 2 ? 1U : 2U}); }
    }
    if (object->type == QStringLiteral("wheel")) //''')
edit('src/stage3_project_edits.cpp', '    if (object->type == QStringLiteral("wheel") && !object->wheel.highlight.picturePng.isEmpty())', '''    if (object->type == QStringLiteral("track") && !object->moveTrack.appearance.picturePng.isEmpty()) //20261112-0 add: Preserve image identity during resource-conflict copies.
    {
        if (!mergePicture(candidate, object->moveTrack.appearance.picture, object->moveTrack.appearance.picturePng, &resolved, error)) { return false; }
        object->moveTrack.appearance.picture = resolved;
    }
    if (object->type == QStringLiteral("wheel") && !object->wheel.highlight.picturePng.isEmpty())''')
edit('src/stage3_resources.cpp', '    if (object.type == QStringLiteral("wheel")) { visitor', '    if (object.type == QStringLiteral("track")) { visitor(object.moveTrack.appearance); } //20261112-0 add: Include inactive pages in track image reference protection.\n    if (object.type == QStringLiteral("wheel")) { visitor')
edit('src/hmi_canvas.cpp', '#include "wheel_dialog.h"', '#include "move_track_dialog.h" //20261112-0 add\n#include "wheel_dialog.h"')
edit('src/hmi_canvas.cpp', '    else if (m_type == QStringLiteral("wheel"))', '    else if (m_type == QStringLiteral("track")) { paintMoveTrack(painter, item_rect, m_record.moveTrack); } //20261112-0 add\n    else if (m_type == QStringLiteral("wheel"))')
edit('src/hmi_canvas.cpp', 'object.name = (object.type == QStringLiteral("wheel")', 'object.name = (object.type == QStringLiteral("track") ? QStringLiteral("移动轨迹%1") : object.type == QStringLiteral("wheel")')
edit('src/native_shell_ui.cpp', '            else if (id == QStringLiteral("action_wheelView"))', '            else if (id == QStringLiteral("action_moveTrack")) //20261112-0 add\n            {\n                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("track")); });\n            }\n            else if (id == QStringLiteral("action_wheelView"))')
edit('src/native_shell_ui.cpp', '!selected || object.type == QStringLiteral("wheel")', '!selected || object.type == QStringLiteral("track") || object.type == QStringLiteral("wheel")')
edit('src/native_shell_ui.cpp', 'object.type == QStringLiteral("wheel") ? QStringLiteral("滚轮控件")', 'object.type == QStringLiteral("track") ? QStringLiteral("移动轨迹") : object.type == QStringLiteral("wheel") ? QStringLiteral("滚轮控件")')
edit('src/main_window.cpp', '    if (object.type == QStringLiteral("wheel")) //', '    if (object.type == QStringLiteral("track")) //20261112-0 add: Compact edits update only the typed operation carrier.\n    { object.moveTrack.address = object.address.trimmed(); object.address.clear(); if (!object.moveTrack.validateAddresses(m_project.connections, &controlError)) { QMessageBox::warning(this, QStringLiteral("移动轨迹属性未提交"), controlError); return; } }\n    if (object.type == QStringLiteral("wheel")) //')
edit('src/main_window.cpp', '|| object.type == QStringLiteral("wheel")))', '|| object.type == QStringLiteral("wheel") || object.type == QStringLiteral("track")))')
edit('src/main_window.cpp', '        if (object.type == QStringLiteral("wheel")) { m_addressEdit', '        if (object.type == QStringLiteral("track")) { m_addressEdit->setText(object.moveTrack.address); } //20261112-0 add\n        if (object.type == QStringLiteral("wheel")) { m_addressEdit')
edit('src/static_text_actions.cpp', '|| object.type == QStringLiteral("wheel")))', '|| object.type == QStringLiteral("wheel") || object.type == QStringLiteral("track")))')
edit('src/stage4_control_actions.cpp', '#include "wheel_dialog.h"', '#include "move_track_dialog.h" //20261112-0 add\n#include "wheel_dialog.h"')
edit('src/stage4_control_actions.cpp', '    else if (type == QStringLiteral("wheel")) //', '    else if (type == QStringLiteral("track")) //20261112-0 add\n    {\n        object.name = QStringLiteral("移动轨迹"); object.geometry = QRect(0, 0, 200, 100); dialog = new MoveTrackDialog(object, currentEditorProject(), this);\n    }\n    else if (type == QStringLiteral("wheel")) //')
edit('src/stage4_control_actions.cpp', '        else if (type == QStringLiteral("wheel"))', '        else if (type == QStringLiteral("track")) { pictures = qobject_cast<MoveTrackDialog *>(dialog)->pictures(); object = qobject_cast<MoveTrackDialog *>(dialog)->object(); } //20261112-0 add\n        else if (type == QStringLiteral("wheel"))')
edit('src/stage4_control_actions.cpp', '    else if (object.type == QStringLiteral("wheel")) { dialog', '    else if (object.type == QStringLiteral("track")) { dialog = new MoveTrackDialog(object, currentEditorProject(), this); } //20261112-0 add\n    else if (object.type == QStringLiteral("wheel")) { dialog')
edit('src/stage4_control_actions.cpp', '        else if (object.type == QStringLiteral("wheel"))', '        else if (object.type == QStringLiteral("track")) { pictures = qobject_cast<MoveTrackDialog *>(dialog)->pictures(); object = qobject_cast<MoveTrackDialog *>(dialog)->object(); } //20261112-0 add\n        else if (object.type == QStringLiteral("wheel"))')

# Reuse the measured common access widgets, retaining the track's independent fresh-radio state.
slider = (root / 'src/slider_dialog.cpp').read_text(encoding='utf-8')
start = slider.index('/******************************************************************************\n * Function Name   : SliderDialog::createPermissions')
end = slider.index('/******************************************************************************', start + 90)
permissions = slider[start:end].replace('SliderDialog', 'MoveTrackDialog').replace('Slider', 'Track')
permissions = permissions.replace('native selected-one bit conditions', 'native fresh empty bit conditions')
permissions = permissions.replace('layout->addWidget(m_visibility, 0, 0);', 'm_visibility->configureUnchosenBitState(m_draft.visibilityBitChosen); m_touch->configureUnchosenBitState(m_draft.touchBitChosen); layout->addWidget(m_visibility, 0, 0);')
assert 'MoveTrackDialog::createPermissions' in permissions and 'm_draft.noticeWordValue' in permissions
dialog_path = root / 'src/move_track_dialog.cpp'
assert 'MoveTrackDialog::createPermissions' not in dialog_path.read_text(encoding='utf-8')
sources['src/move_track_dialog.cpp'] = dialog_path.read_text(encoding='utf-8') + '\n\n' + permissions
for relative, source in sources.items():
    (root / relative).write_text(source, encoding='utf-8', newline='\n')
print('Integrated track model, atomic editor/resources, address roles, renderer and guarded type.')
