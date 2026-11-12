"""Reuse the verified table editor while retaining a distinct historical-alarm object identity."""
from pathlib import Path
import re
root=Path(__file__).resolve().parent/'worktree'
def put(path,s):(root/path).write_text(s,encoding='utf-8',newline='\n')
for path in ['src/project_model.cpp','src/stage3_project_edits.cpp','src/main_window.cpp','src/static_text_actions.cpp','src/alarm_registration_properties.cpp']:
    s=(root/path).read_text(encoding='utf-8')
    s=re.sub(r'(object(?:->|\.)type) == QStringLiteral\("alarmdisplay"\)',lambda m:'('+m.group(0)+' || '+m.group(1)+' == QStringLiteral("historyalarm"))',s)
    if path=='src/project_model.cpp':
        before='    if (type == QStringLiteral("alarmdisplay")) { json.insert';assert s.count(before)==1;s=s.replace(before,'    if (type == QStringLiteral("alarmdisplay") || type == QStringLiteral("historyalarm")) { json.insert')
        before='         object->type != QStringLiteral("alarmdisplay") &&';assert s.count(before)==1;s=s.replace(before,'         object->type != QStringLiteral("historyalarm") && //20261113-2 add: Historical records keep a distinct type while sharing table attributes.\n'+before)
    put(path,s)
p=root/'src/stage4_control_actions.cpp';s=p.read_text(encoding='utf-8');before='''    else if (type == QStringLiteral("alarmdisplay")) //20261113-2 add
    {
        if (!hasAlarmRegistrations(currentEditorProject())) { QMessageBox::warning(this, QStringLiteral("报警显示器"), QStringLiteral("位报警和条件报警都不存在")); return; }
        object.name = QStringLiteral("报警显示器"); object.geometry = QRect(0, 0, 320, 160); dialog = new AlarmDisplayDialog(object, currentEditorProject(), this);
    }''';after='''    else if (type == QStringLiteral("alarmdisplay") || type == QStringLiteral("historyalarm")) //20261113-2 change: The native table forms match, but their runtime data sources and object kinds remain distinct.
    {
        object.name = type == QStringLiteral("historyalarm") ? QStringLiteral("历史报警显示器") : QStringLiteral("报警显示器");
        if (!hasAlarmRegistrations(currentEditorProject())) { QMessageBox::warning(this, object.name, QStringLiteral("位报警和条件报警都不存在")); return; }
        object.geometry = QRect(0, 0, 320, 160); dialog = new AlarmDisplayDialog(object, currentEditorProject(), this);
    }''';assert s.count(before)==1;s=s.replace(before,after)
s=s.replace('else if (type == QStringLiteral("alarmdisplay")) { object =','else if (type == QStringLiteral("alarmdisplay") || type == QStringLiteral("historyalarm")) { object =').replace('else if (object.type == QStringLiteral("alarmdisplay")) { dialog =','else if (object.type == QStringLiteral("alarmdisplay") || object.type == QStringLiteral("historyalarm")) { dialog =').replace('else if (object.type == QStringLiteral("alarmdisplay")) { object =','else if (object.type == QStringLiteral("alarmdisplay") || object.type == QStringLiteral("historyalarm")) { object =');put('src/stage4_control_actions.cpp',s)
p=root/'src/alarm_display_dialog.cpp';s=p.read_text(encoding='utf-8');s=s.replace('setWindowTitle(QStringLiteral("报警显示器"));','setWindowTitle(object.type == QStringLiteral("historyalarm") ? QStringLiteral("历史报警显示器") : QStringLiteral("报警显示器"));').replace('object.type = QStringLiteral("alarmdisplay"); object.alarmDisplay =','object.type = m_original.type == QStringLiteral("historyalarm") ? QStringLiteral("historyalarm") : QStringLiteral("alarmdisplay"); object.alarmDisplay =').replace('QMessageBox::warning(this, QStringLiteral("报警显示器"),','QMessageBox::warning(this, windowTitle(),').replace('QMessageBox::information(this, QStringLiteral("报警显示器帮助"),','QMessageBox::information(this, windowTitle() + QStringLiteral("帮助"),');put('src/alarm_display_dialog.cpp',s)
p=root/'src/hmi_canvas.cpp';s=p.read_text(encoding='utf-8');s=s.replace('else if (m_type == QStringLiteral("alarmdisplay")) { paintAlarmDisplay','else if (m_type == QStringLiteral("alarmdisplay") || m_type == QStringLiteral("historyalarm")) { paintAlarmDisplay');before='object.name = (object.type == QStringLiteral("alarmdisplay")';assert s.count(before)==1;s=s.replace(before,'object.name = (object.type == QStringLiteral("historyalarm") ? QStringLiteral("历史报警显示器%1") : object.type == QStringLiteral("alarmdisplay")');put('src/hmi_canvas.cpp',s)
p=root/'src/native_shell_ui.cpp';s=p.read_text(encoding='utf-8');before='            else if (id == QStringLiteral("action_arlamcontrol"))';assert s.count(before)==1;s=s.replace(before,'            else if (id == QStringLiteral("action_histotyarlarm")) //20261113-2 add: Keep the native historical-alarm entry independent of current alarms.\n            { connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("historyalarm")); }); }\n'+before)
before='!selected || object.type == QStringLiteral("alarmdisplay")';assert s.count(before)==1;s=s.replace(before,'!selected || object.type == QStringLiteral("historyalarm") || object.type == QStringLiteral("alarmdisplay")');before='object.type == QStringLiteral("alarmdisplay") ? QStringLiteral("报警显示器")';assert s.count(before)==1;s=s.replace(before,'object.type == QStringLiteral("historyalarm") ? QStringLiteral("历史报警显示器") : '+before);put('src/native_shell_ui.cpp',s)
print('Connected distinct historical-alarm identity with the measured shared table editor and all dependency/visibility/language paths.')
