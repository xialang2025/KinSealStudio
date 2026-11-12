"""Connect C28 to the existing typed editor and verified global alarm dependencies."""
from pathlib import Path
import ast
root=Path(__file__).resolve().parent/'worktree';sources={}
def edit(path,before,after):
    s=sources.get(path,(root/path).read_text(encoding='utf-8'));assert s.count(before)==1,(path,before,s.count(before));sources[path]=s.replace(before,after)
tree=ast.parse(Path(__file__).with_name('integrate_chart.py').read_text(encoding='utf-8'))
for n in tree.body:
    if not isinstance(n,ast.Expr) or not isinstance(n.value,ast.Call) or not isinstance(n.value.func,ast.Name) or n.value.func.id!='edit':continue
    path,before,after=map(ast.literal_eval,n.value.args)
    after=after.replace('chart_properties','alarm_display_properties').replace('chart_dialog','alarm_display_dialog').replace('ChartProperties','AlarmDisplayProperties').replace('ChartDialog','AlarmDisplayDialog').replace('paintChart','paintAlarmDisplay').replace('object->chart','object->alarmDisplay').replace('object.chart','object.alarmDisplay').replace('m_record.chart','m_record.alarmDisplay').replace('chart.toJson()','alarmDisplay.toJson()').replace('AlarmDisplayProperties chart','AlarmDisplayProperties alarmDisplay').replace('QStringLiteral("chart")','QStringLiteral("alarmdisplay")').replace('QStringLiteral("图表','QStringLiteral("报警显示器').replace('//20261112-1','//20261113-2')
    if path=='CMakeLists.txt' and before.endswith('properties.cpp)'):
        before='src/alarm_registration_properties.cpp)';after='src/alarm_registration_properties.cpp src/alarm_display_properties.cpp)'
    if path=='src/project_model.cpp':
        if before=='QStringLiteral("moveTrack"), QStringLiteral("extensions")':before='QStringLiteral("alarmBar"), QStringLiteral("extensions")';after='QStringLiteral("alarmBar"), QStringLiteral("alarmDisplay"), QStringLiteral("extensions")'
        elif before=='value.toObject().contains(QStringLiteral("moveTrack")))':before='value.toObject().contains(QStringLiteral("alarmBar")))';after='value.toObject().contains(QStringLiteral("alarmBar")) || value.toObject().contains(QStringLiteral("alarmDisplay")))'
        elif 'json.insert(QStringLiteral("alarmdisplay")' in after:after=after.replace('json.insert(QStringLiteral("alarmdisplay")','json.insert(QStringLiteral("alarmDisplay")')
        elif 'key == QStringLiteral("alarmdisplay")' in after:after=after.replace('key == QStringLiteral("alarmdisplay")','key == QStringLiteral("alarmDisplay")')
        elif 'json.contains(QStringLiteral("alarmdisplay"))' in after:after=after.replace('json.contains(QStringLiteral("alarmdisplay"))','json.contains(QStringLiteral("alarmDisplay"))')
    if path=='src/stage3_project_edits.cpp':after='    if (object->type == QStringLiteral("alarmdisplay")) { addConditionSlots(&object->alarmDisplay.visibility, false, &addressSlots); } //20261113-2 add\n    if (object->type == QStringLiteral("track")) //'
    if path=='src/native_shell_ui.cpp':
        after=after.replace('graphicsitem','action_arlamcontrol').replace('btn_metre','btn_alarmDisplay')
        if before.startswith('!selected ||'):before='!selected || object.type == QStringLiteral("alarmbar")';after='!selected || object.type == QStringLiteral("alarmdisplay") || object.type == QStringLiteral("alarmbar")'
    if path=='src/hmi_canvas.cpp':
        if before.startswith('object.name = '):before='object.name = (object.type == QStringLiteral("alarmbar")';after='object.name = (object.type == QStringLiteral("alarmdisplay") ? QStringLiteral("报警显示器%1") : object.type == QStringLiteral("alarmbar")'
        else:after=after.replace('paintAlarmDisplay(painter, item_rect, m_record.alarmDisplay)','paintAlarmDisplay(painter, item_rect, m_record.alarmDisplay, data(10).toString())')
    if path in ['src/main_window.cpp','src/static_text_actions.cpp'] and before=='|| object.type == QStringLiteral("track")))':before='|| object.type == QStringLiteral("alarmbar")))';after='|| object.type == QStringLiteral("alarmbar") || object.type == QStringLiteral("alarmdisplay")))'
    if path=='src/main_window.cpp' and before=='    if (object.type == QStringLiteral("track")) //':after='    if (object.type == QStringLiteral("alarmdisplay")) { object.address.clear(); object.text.clear(); } //20261113-2 add\n    if (object.type == QStringLiteral("track")) //'
    if path=='src/main_window.cpp' and before.startswith('        if (object.type == QStringLiteral("track")) { m_addressEdit'):continue
    if path=='src/stage4_control_actions.cpp' and before=='    else if (type == QStringLiteral("track")) //':
        after='''    else if (type == QStringLiteral("alarmdisplay")) //20261113-2 add
    {
        if (!hasAlarmRegistrations(currentEditorProject())) { QMessageBox::warning(this, QStringLiteral("报警显示器"), QStringLiteral("位报警和条件报警都不存在")); return; }
        object.name = QStringLiteral("报警显示器"); object.geometry = QRect(0, 0, 320, 160); dialog = new AlarmDisplayDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("track")) //'''
    edit(path,before,after)
edit('src/project_model.cpp','            if (!object->hasFunctionButtonProperties || object->functionButton.function != 13) { continue; }','            if (object->type == QStringLiteral("alarmdisplay")) { object->alarmDisplay.setLanguages(newLanguages); } //20261113-2 add\n            if (!object->hasFunctionButtonProperties || object->functionButton.function != 13) { continue; }')
edit('src/main_window.cpp','object.type == QStringLiteral("alarmbar") || object.type == QStringLiteral("historytrend") || object.type == QStringLiteral("time")','object.type == QStringLiteral("alarmdisplay") || object.type == QStringLiteral("alarmbar") || object.type == QStringLiteral("historytrend") || object.type == QStringLiteral("time")')
edit('src/stage3_project_edits.cpp','        if (object.type == QStringLiteral("alarmbar")) //20261113-2 add:','        if (object.type == QStringLiteral("alarmbar") || object.type == QStringLiteral("alarmdisplay")) //20261113-2 add:')
edit('src/alarm_registration_properties.cpp','if (object.type == QStringLiteral("alarmbar") && !hasAlarmRegistrations(project))','if ((object.type == QStringLiteral("alarmbar") || object.type == QStringLiteral("alarmdisplay")) && !hasAlarmRegistrations(project))')
for path,s in sources.items():(root/path).write_text(s,encoding='utf-8',newline='\n')
print('Connected C28 type, schema12, localized captions, shared alarm prerequisites/copy and display-only canvas/editor routes.')
