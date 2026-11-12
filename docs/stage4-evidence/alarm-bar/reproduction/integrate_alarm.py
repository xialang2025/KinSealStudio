"""Connect C27 through typed persistence, all-page dependencies and native shell commands."""
from pathlib import Path
import runpy
root=Path(__file__).resolve().parent/'worktree'
sources={}
def edit(path,before,after):
    s=sources.get(path,(root/path).read_text(encoding='utf-8'));assert s.count(before)==1,(path,before,s.count(before));sources[path]=s.replace(before,after)
edit('CMakeLists.txt','    src/data_group_dialog.cpp','    src/data_group_dialog.cpp\n    src/alarm_bar_properties.h #20261113-2 add\n    src/alarm_bar_properties.cpp\n    src/alarm_bar_dialog.h\n    src/alarm_bar_dialog.cpp\n    src/alarm_registration_properties.h\n    src/alarm_registration_properties.cpp\n    src/alarm_registration_dialog.h\n    src/alarm_registration_dialog.cpp')
edit('CMakeLists.txt','src/data_group_properties.cpp)','src/data_group_properties.cpp src/alarm_bar_properties.cpp src/alarm_registration_properties.cpp)')
edit('src/project_model.h','#include "data_group_properties.h"','#include "alarm_bar_properties.h" //20261113-2 add\n#include "alarm_registration_properties.h"\n#include "data_group_properties.h"')
edit('src/project_model.h','    DataGroupProperties dataGroup;','    AlarmBarProperties alarmBar; //20261113-2 add: Alarm rows belong to global registration nodes.\n    DataGroupProperties dataGroup;')
edit('src/project_model.cpp','    if (type == QStringLiteral("datagroup")) { json.insert','    if (type == QStringLiteral("alarmbar")) { json.insert(QStringLiteral("alarmBar"), alarmBar.toJson()); } //20261113-2 add\n    if (type == QStringLiteral("datagroup")) { json.insert')
edit('src/project_model.cpp','QStringLiteral("datagroup"), QStringLiteral("extensions")','QStringLiteral("datagroup"), QStringLiteral("alarmBar"), QStringLiteral("extensions")')
edit('src/project_model.cpp','        else if (key == QStringLiteral("datagroup"))','        else if (key == QStringLiteral("alarmBar")) //20261113-2 add\n        {\n            if (!item.value().isObject() || !AlarmBarProperties::fromJson(item.value().toObject(), &object->alarmBar, error)) { return false; }\n        }\n        else if (key == QStringLiteral("datagroup"))')
edit('src/project_model.cpp','    if ((object->type == QStringLiteral("datagroup"))','    if ((object->type == QStringLiteral("alarmbar")) != json.contains(QStringLiteral("alarmBar"))) { return modelError(error, QStringLiteral("动态报警条类型与专有属性不匹配。")); } //20261113-2 add\n    if ((object->type == QStringLiteral("datagroup"))')
edit('src/project_model.cpp','         object->type != QStringLiteral("datagroup") &&','         object->type != QStringLiteral("alarmbar") && //20261113-2 add\n         object->type != QStringLiteral("datagroup") &&')
edit('src/project_model.cpp','object.type == QStringLiteral("chart") || object.type == QStringLiteral("trend")','object.type == QStringLiteral("alarmbar") || object.type == QStringLiteral("chart") || object.type == QStringLiteral("trend")')
edit('src/project_model.cpp','value.toObject().contains(QStringLiteral("datagroup")))','value.toObject().contains(QStringLiteral("datagroup")) || value.toObject().contains(QStringLiteral("alarmBar")))')
edit('src/project_model.cpp','        if (object.type == QStringLiteral("datagroup") && (!object.address.isEmpty()', '        if (object.type == QStringLiteral("alarmbar") && (!object.address.isEmpty() || !object.text.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || object.hasImageDisplayProperties || !object.alarmBar.validate(error) || !object.alarmBar.validateAddresses(connections, error))) { return false; } //20261113-2 add\n        if (object.type == QStringLiteral("datagroup") && (!object.address.isEmpty()')
edit('src/project_model.cpp','    if (candidate.kind == QStringLiteral("dataCollection") &&','''    if (candidate.kind == QStringLiteral("alarm") && candidate.implemented && candidate.active && candidate.ownerPageId.isEmpty()) //20261113-2 add
    {
        AlarmRegistrationProperties properties;
        if (candidate.id.trimmed().isEmpty() || candidate.name.trimmed().isEmpty() || candidate.name.size() > 48 || !AlarmRegistrationProperties::fromJson(candidate.properties, &properties, error) || !inactiveExtensions(candidate.extensions, QStringLiteral("configurationNode.extensions"), error)) { return false; } *node = candidate; return true;
    }
    if (candidate.kind == QStringLiteral("dataCollection") &&''')
edit('src/project_model.cpp','node.kind == QStringLiteral("dataCollection") && node.implemented && node.active) { schema = 12;','(node.kind == QStringLiteral("dataCollection") || node.kind == QStringLiteral("alarm")) && node.implemented && node.active) { schema = 12;')
edit('src/project_model.cpp','    if (hasSystemSettings && !readStage2SystemSettings','    if (!validateAlarmRegistrations(*this, error)) { return false; } //20261113-2 add: Resolve global alarms and all-page consumers before publishing any edit.\n    if (hasSystemSettings && !readStage2SystemSettings')
# Let language changes retain known labels without accumulating stale hidden language records.
edit('src/project_model.cpp','    candidate.languages = newLanguages;','''    //20261113-2 add: Keep each alarm row aligned with the accepted project language transaction.
    {
        AlarmRegistrationProperties alarms;
        QMap<QString, QString> messages;
        QString language;
        int nodeIndex;
        int rowIndex;
        for (nodeIndex = 0; nodeIndex < candidate.configurationNodes.size(); ++nodeIndex)
        {
            if (candidate.configurationNodes[nodeIndex].kind != QStringLiteral("alarm") || !candidate.configurationNodes[nodeIndex].active || !candidate.configurationNodes[nodeIndex].implemented) { continue; }
            if (!AlarmRegistrationProperties::fromJson(candidate.configurationNodes[nodeIndex].properties, &alarms, error)) { return false; }
            for (rowIndex = 0; rowIndex < alarms.rows.size(); ++rowIndex) { messages.clear(); Q_FOREACH (language, newLanguages) { messages.insert(language, alarms.rows[rowIndex].messages.value(language)); } alarms.rows[rowIndex].messages = messages; }
            candidate.configurationNodes[nodeIndex].properties = alarms.toJson();
        }
    }
    candidate.languages = newLanguages;''')
edit('src/stage3_project_edits.cpp','    if (object->type == QStringLiteral("datagroup")) //','    if (object->type == QStringLiteral("alarmbar")) { addConditionSlots(&object->alarmBar.visibility, false, &addressSlots); } //20261113-2 add\n    if (object->type == QStringLiteral("datagroup")) //')
edit('src/stage3_project_edits.cpp','        if (object.type == QStringLiteral("historytrend")) //','''        if (object.type == QStringLiteral("alarmbar")) //20261113-2 add: Copy global alarm definitions only into a compatible project and validate navigation identities.
        {
            HmiConfigurationNode required;
            HmiConfigurationNode peer;
            AlarmRegistrationProperties alarms;
            AlarmRegistrationProperties peerAlarms;
            AlarmRegistrationRow row;
            QString pageId;
            bool reused;
            Q_FOREACH (required, clipboard.source.configurationNodes)
            {
                if (required.kind != QStringLiteral("alarm") || !required.active || !required.implemented) { continue; }
                if (!AlarmRegistrationProperties::fromJson(required.properties, &alarms, error)) { return false; }
                reused = false;
                Q_FOREACH (peer, candidate->configurationNodes)
                {
                    if (peer.id == required.id && peer.toJson() == required.toJson()) { reused = true; }
                    else if (peer.id == required.id || (peer.kind == QStringLiteral("alarm") && peer.active && peer.implemented && AlarmRegistrationProperties::fromJson(peer.properties, &peerAlarms) && peerAlarms.family == alarms.family)) { return editError(error, QStringLiteral("目标工程已有不同的报警登录，粘贴已取消。")); }
                }
                if (alarms.family == 0) { Q_FOREACH (row, alarms.rows) { if (!row.used) { continue; } Q_FOREACH (pageId, (QStringList{row.jumpPageId, row.backPageId})) { if (pageId.isEmpty()) { continue; } reference = HmiReference(); reference.id = QStringLiteral("alarm-navigation"); reference.targetKind = QStringLiteral("page"); reference.targetId = pageId; if (!mergeReferenceTarget(clipboard.source, target, candidate, &reference, &mappedIds, error)) { return false; } } } }
                if (!reused) { candidate->configurationNodes.append(required); }
            }
        }
        if (object.type == QStringLiteral("historytrend")) //''')
edit('src/hmi_canvas.cpp','#include "data_group_dialog.h"','#include "alarm_bar_dialog.h" //20261113-2 add\n#include "data_group_dialog.h"')
edit('src/hmi_canvas.cpp','    else if (m_type == QStringLiteral("datagroup"))','    else if (m_type == QStringLiteral("alarmbar")) { paintAlarmBar(painter, item_rect, m_record.alarmBar); } //20261113-2 add\n    else if (m_type == QStringLiteral("datagroup"))')
edit('src/hmi_canvas.cpp','object.name = (object.type == QStringLiteral("datagroup")','object.name = (object.type == QStringLiteral("alarmbar") ? QStringLiteral("动态报警条%1") : object.type == QStringLiteral("datagroup")')
edit('src/native_shell_ui.cpp','            else if (id == QStringLiteral("action_dataGroup"))','            else if (id == QStringLiteral("action_dynamicarlarm")) //20261113-2 add\n            { connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("alarmbar")); }); }\n            else if (id == QStringLiteral("action_dataGroup"))')
edit('src/native_shell_ui.cpp','!selected || object.type == QStringLiteral("datagroup")','!selected || object.type == QStringLiteral("alarmbar") || object.type == QStringLiteral("datagroup")')
edit('src/native_shell_ui.cpp','object.type == QStringLiteral("datagroup") ? QStringLiteral("数据群组")','object.type == QStringLiteral("alarmbar") ? QStringLiteral("动态报警条") : object.type == QStringLiteral("datagroup") ? QStringLiteral("数据群组")')
edit('src/main_window.cpp','    if (object.type == QStringLiteral("historytrend")) { object.address.clear();','    if (object.type == QStringLiteral("alarmbar")) { object.address.clear(); object.text.clear(); } //20261113-2 add\n    if (object.type == QStringLiteral("historytrend")) { object.address.clear();')
edit('src/main_window.cpp','object.type == QStringLiteral("historytrend") || object.type == QStringLiteral("time")','object.type == QStringLiteral("alarmbar") || object.type == QStringLiteral("historytrend") || object.type == QStringLiteral("time")')
for path in ['src/main_window.cpp','src/static_text_actions.cpp']:
    edit(path,'|| object.type == QStringLiteral("datagroup")))','|| object.type == QStringLiteral("datagroup") || object.type == QStringLiteral("alarmbar")))')
edit('src/stage4_control_actions.cpp','#include "data_group_dialog.h"','#include "alarm_bar_dialog.h" //20261113-2 add\n#include "alarm_registration_dialog.h"\n#include "data_group_dialog.h"')
edit('src/stage4_control_actions.cpp','    else if (type == QStringLiteral("datagroup")) //','''    else if (type == QStringLiteral("alarmbar")) //20261113-2 add
    {
        if (!hasAlarmRegistrations(currentEditorProject())) { QMessageBox::warning(this, QStringLiteral("动态报警条"), QStringLiteral("位报警和条件报警都不存在")); return; }
        object.name = QStringLiteral("动态报警条"); object.geometry = QRect(0, 0, 320, 60); dialog = new AlarmBarDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("datagroup")) //''')
edit('src/stage4_control_actions.cpp','        else if (type == QStringLiteral("datagroup"))','        else if (type == QStringLiteral("alarmbar")) { object = qobject_cast<AlarmBarDialog *>(dialog)->object(); } //20261113-2 add\n        else if (type == QStringLiteral("datagroup"))')
edit('src/stage4_control_actions.cpp','    else if (object.type == QStringLiteral("datagroup")) { dialog','    else if (object.type == QStringLiteral("alarmbar")) { dialog = new AlarmBarDialog(object, currentEditorProject(), this); } //20261113-2 add\n    else if (object.type == QStringLiteral("datagroup")) { dialog')
edit('src/stage4_control_actions.cpp','        else if (object.type == QStringLiteral("datagroup"))','        else if (object.type == QStringLiteral("alarmbar")) { object = qobject_cast<AlarmBarDialog *>(dialog)->object(); } //20261113-2 add\n        else if (object.type == QStringLiteral("datagroup"))')
edit('src/main_window.h','    void configureDataCollection(', '    void configureAlarmRegistration(int family); //20261113-2 add\n    void deleteAlarmRegistration(int family);\n    void configureDataCollection(')
edit('src/main_window_workflow.cpp','item = new QTreeWidgetItem(parent, {label}); item->setData(0, Qt::UserRole, QStringLiteral("unsupported"));','item = new QTreeWidgetItem(parent, {label}); item->setData(0, Qt::UserRole, label == QStringLiteral("数位地址报警") ? QStringLiteral("alarm-bit") : QStringLiteral("alarm-condition")); //20261113-2 change')
edit('src/main_window.cpp','        else if (role == QStringLiteral("dataCollection"))','        else if (role == QStringLiteral("alarm-bit") || role == QStringLiteral("alarm-condition")) { configureAlarmRegistration(role == QStringLiteral("alarm-bit") ? 0 : 1); } //20261113-2 add\n        else if (role == QStringLiteral("dataCollection"))')
edit('src/main_window.cpp','        if (item == nullptr || item->data(0, Qt::UserRole).toString() != QStringLiteral("dataCollection")) { return; }','''        if (item != nullptr && (item->data(0, Qt::UserRole).toString() == QStringLiteral("alarm-bit") || item->data(0, Qt::UserRole).toString() == QStringLiteral("alarm-condition"))) //20261113-2 add
        {
            int family = item->data(0, Qt::UserRole).toString() == QStringLiteral("alarm-bit") ? 0 : 1;
            menu.addAction(QStringLiteral("配置"), this, [this, family]() { configureAlarmRegistration(family); }); menu.addAction(QStringLiteral("删除"), this, [this, family]() { deleteAlarmRegistration(family); }); menu.exec(m_systemSettingsTree->viewport()->mapToGlobal(position)); return;
        }
        if (item == nullptr || item->data(0, Qt::UserRole).toString() != QStringLiteral("dataCollection")) { return; }''')
for path,s in sources.items():(root/path).write_text(s,encoding='utf-8',newline='\n')
# Import the helper as data to avoid rerunning model generation after any later repairs.
namespace={};script=Path(__file__).with_name('make_alarm_models.py').read_text(encoding='utf-8');exec(script[script.index('def f('):script.index("put('src/alarm_registration_properties.h'")],namespace);f=namespace['f']
p=root/'src/stage4_control_actions.cpp';s=p.read_text(encoding='utf-8')
s+='\n'+f('MainWindow::configureAlarmRegistration','Publish a complete global registration draft through one validated undoable project transaction.','void MainWindow::configureAlarmRegistration(int family)','''    HmiProject candidate = currentEditorProject();
    HmiConfigurationNode node;
    AlarmRegistrationProperties properties;
    AlarmRegistrationDialog *dialog;
    QString error;
    int existing = -1;
    int index;
    if (!m_hasProject || family < 0 || family > 1) { return; }
    for (index = 0; index < candidate.configurationNodes.size(); ++index) { if (candidate.configurationNodes[index].kind == QStringLiteral("alarm") && candidate.configurationNodes[index].active && candidate.configurationNodes[index].implemented && AlarmRegistrationProperties::fromJson(candidate.configurationNodes[index].properties, &properties) && properties.family == family) { node = candidate.configurationNodes[index]; existing = index; break; } }
    dialog = new AlarmRegistrationDialog(node, candidate, family, this); if (dialog->exec() != QDialog::Accepted) { delete dialog; return; } if (existing < 0) { candidate.configurationNodes.append(dialog->node()); } else { candidate.configurationNodes[existing] = dialog->node(); } delete dialog; if (!commitProjectDraft(candidate, &error)) { QMessageBox::warning(this, QStringLiteral("报警登录未提交"), error); }
''','family')
s+=f('MainWindow::deleteAlarmRegistration','Refuse clearing the last required registration or a referenced page by validating every page before publish.','void MainWindow::deleteAlarmRegistration(int family)','''    HmiProject candidate = currentEditorProject();
    AlarmRegistrationProperties properties;
    QString error;
    int index;
    for (index = 0; index < candidate.configurationNodes.size(); ++index) { if (candidate.configurationNodes[index].kind == QStringLiteral("alarm") && candidate.configurationNodes[index].active && candidate.configurationNodes[index].implemented && AlarmRegistrationProperties::fromJson(candidate.configurationNodes[index].properties, &properties) && properties.family == family) { candidate.configurationNodes.removeAt(index); if (!commitProjectDraft(candidate, &error)) { QMessageBox::warning(this, QStringLiteral("报警登录正在使用"), error); } return; } }
''','family')
p.write_text(s,encoding='utf-8',newline='\n')
print('Integrated C27 model, global dependency guards, copy, language, tree, shell and renderer.')
