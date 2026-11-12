"""Connect C30's strict independent table and atomic collector reconciliation."""
from pathlib import Path
import ast
from make_history_data_model import f
root=Path(__file__).resolve().parent/'worktree'; sources={}
def edit(path,before,after):
    s=sources.get(path,(root/path).read_text(encoding='utf-8')); assert s.count(before)==1,(path,before,s.count(before)); sources[path]=s.replace(before,after)
tree=ast.parse(Path(__file__).with_name('integrate_chart.py').read_text(encoding='utf-8'))
for n in tree.body:
    if not isinstance(n,ast.Expr) or not isinstance(n.value,ast.Call) or not isinstance(n.value.func,ast.Name) or n.value.func.id!='edit': continue
    path,before,after=map(ast.literal_eval,n.value.args)
    after=after.replace('chart_properties','history_data_properties').replace('chart_dialog','history_data_dialog').replace('ChartProperties','HistoryDataProperties').replace('ChartDialog','HistoryDataDialog').replace('paintChart','paintHistoryData').replace('object->chart','object->historyData').replace('object.chart','object.historyData').replace('m_record.chart','m_record.historyData').replace('chart.toJson()','historyData.toJson()').replace('HistoryDataProperties chart','HistoryDataProperties historyData').replace('QStringLiteral("chart")','QStringLiteral("historydata")').replace('QStringLiteral("图表','QStringLiteral("历史数据显示器').replace('//20261112-1','//20261113-2')
    if path=='CMakeLists.txt' and before.endswith('properties.cpp)'):
        before='src/alarm_display_properties.cpp)'; after='src/alarm_display_properties.cpp src/history_data_properties.cpp)'
    if path=='src/project_model.cpp':
        if before=='QStringLiteral("moveTrack"), QStringLiteral("extensions")': before='QStringLiteral("alarmDisplay"), QStringLiteral("extensions")';after='QStringLiteral("alarmDisplay"), QStringLiteral("historyData"), QStringLiteral("extensions")'
        elif before=='value.toObject().contains(QStringLiteral("moveTrack")))': before='value.toObject().contains(QStringLiteral("alarmDisplay")))'; after='value.toObject().contains(QStringLiteral("alarmDisplay")) || value.toObject().contains(QStringLiteral("historyData")))'
        after=after.replace('json.insert(QStringLiteral("historydata")','json.insert(QStringLiteral("historyData")').replace('key == QStringLiteral("historydata")','key == QStringLiteral("historyData")').replace('json.contains(QStringLiteral("historydata"))','json.contains(QStringLiteral("historyData"))')
    if path=='src/stage3_project_edits.cpp': after='    if (object->type == QStringLiteral("historydata")) { addConditionSlots(&object->historyData.visibility, false, &addressSlots); } //20261113-2 add\n    if (object->type == QStringLiteral("track")) //'
    if path=='src/native_shell_ui.cpp':
        after=after.replace('graphicsitem','action_historyrecord').replace('btn_metre','btn_hisdataDisplay')
        if before.startswith('!selected ||'): before='!selected || object.type == QStringLiteral("historyalarm")'; after='!selected || object.type == QStringLiteral("historydata") || object.type == QStringLiteral("historyalarm")'
    if path=='src/hmi_canvas.cpp':
        if before.startswith('object.name = '):before='object.name = (object.type == QStringLiteral("historyalarm")';after='object.name = (object.type == QStringLiteral("historydata") ? QStringLiteral("历史数据显示器%1") : object.type == QStringLiteral("historyalarm")'
        else:after=after.replace('paintHistoryData(painter, item_rect, m_record.historyData)','paintHistoryData(painter, item_rect, m_record.historyData, data(10).toString())')
    if path in ('src/main_window.cpp','src/static_text_actions.cpp') and before=='|| object.type == QStringLiteral("track")))': before='|| (object.type == QStringLiteral("alarmdisplay") || object.type == QStringLiteral("historyalarm"))))';after='|| object.type == QStringLiteral("historydata") || (object.type == QStringLiteral("alarmdisplay") || object.type == QStringLiteral("historyalarm"))))'
    if path=='src/main_window.cpp' and before=='    if (object.type == QStringLiteral("track")) //':after='    if (object.type == QStringLiteral("historydata")) { object.address.clear(); object.text.clear(); } //20261113-2 add\n    if (object.type == QStringLiteral("track")) //'
    if path=='src/main_window.cpp' and before.startswith('        if (object.type == QStringLiteral("track")) { m_addressEdit'):continue
    if path=='src/stage4_control_actions.cpp' and before=='    else if (type == QStringLiteral("track")) //':
        after='''    else if (type == QStringLiteral("historydata")) //20261113-2 add: Match the measured collection prerequisite before constructing a detached editor.
    {
        HmiConfigurationNode collection;
        bool found = false;
        Q_FOREACH (collection, currentEditorProject().configurationNodes) { if (collection.kind == QStringLiteral("dataCollection") && collection.active && collection.implemented) { found = true; break; } }
        if (!found) { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), QStringLiteral("没有建立数据采集")); return; }
        object.name = QStringLiteral("历史数据显示器"); object.geometry = QRect(0, 0, 320, 180); dialog = new HistoryDataDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("track")) //'''
    edit(path,before,after)
edit('src/main_window.cpp','object.type == QStringLiteral("alarmbar") || object.type == QStringLiteral("historytrend") || object.type == QStringLiteral("time")','object.type == QStringLiteral("historydata") || object.type == QStringLiteral("alarmbar") || object.type == QStringLiteral("historytrend") || object.type == QStringLiteral("time")')
edit('src/project_model.cpp','            if (!object->hasFunctionButtonProperties || object->functionButton.function != 13) { continue; }','            if (object->type == QStringLiteral("historydata")) { object->historyData.setLanguages(newLanguages); } //20261113-2 add\n            if (!object->hasFunctionButtonProperties || object->functionButton.function != 13) { continue; }')
edit('src/project_model.cpp','                if (object.type == QStringLiteral("historytrend") && (!collections.contains', '                if (object.type == QStringLiteral("historydata") && (!collections.contains(object.historyData.collectionId) || !object.historyData.validateCollection(collections.value(object.historyData.collectionId), error))) { return modelError(error, QStringLiteral("历史数据显示器的数据采集或列引用无效。")); } //20261113-2 add\n                if (object.type == QStringLiteral("historytrend") && (!collections.contains')
edit('src/project_model.cpp','        if (object.type == QStringLiteral("historydata") && (!object.address.isEmpty()', '''        if (object.type == QStringLiteral("historydata")) //20261113-2 add: Do not let cross-project paste silently discard source caption languages.
        {
            QString language;
            Q_FOREACH (language, object.historyData.captions.keys()) { if (!languages.contains(language)) { return modelError(error, QStringLiteral("历史数据列名语言%1未在目标工程启用，事务已取消。").arg(language)); } }
        }
        if (object.type == QStringLiteral("historydata") && (!object.text.isEmpty() || !object.address.isEmpty()''')
edit('src/stage3_project_edits.cpp','        if (object.type == QStringLiteral("historytrend")) //20261112-1 add:', '        if (object.type == QStringLiteral("historytrend") || object.type == QStringLiteral("historydata")) //20261113-2 change:')
edit('src/stage3_project_edits.cpp','peer.id == object.historyTrend.collectionId && peer.kind','peer.id == (object.type == QStringLiteral("historydata") ? object.historyData.collectionId : object.historyTrend.collectionId) && peer.kind')
edit('src/project_model.h','    bool setLanguages(const QStringList &newLanguages, QString *error = nullptr);','    bool replaceDataCollection(const HmiConfigurationNode &node, QString *error = nullptr); //20261113-2 add\n    bool setLanguages(const QStringList &newLanguages, QString *error = nullptr);')
edit('src/stage4_control_actions.cpp','    if (existing < 0) { candidate.configurationNodes.append(dialog->node()); } else { candidate.configurationNodes[existing] = dialog->node(); }\n    delete dialog;','    node = dialog->node(); delete dialog;\n    if (!candidate.replaceDataCollection(node, &error)) { QMessageBox::warning(this, QStringLiteral("数据采集未提交"), error); return; } //20261113-2 change: Reconcile all historical-data columns before one undoable publish.')
sources['src/project_model.cpp'] += f('HmiProject::replaceDataCollection', 'Replace a collector and reconcile every table in one validated candidate; preserve unrelated history-trend removal guards.', 'bool HmiProject::replaceDataCollection(const HmiConfigurationNode &node, QString *error)', '''    HmiProject candidate = *this;
    DataCollectionProperties collection;
    QList<HmiPage> pages;
    HmiObject *object;
    int index;
    int pageIndex;
    int objectIndex;
    bool replaced = false;
    if (!validateModel(error) || node.kind != QStringLiteral("dataCollection") || !node.active || !node.implemented || !DataCollectionProperties::fromJson(node.properties, &collection, error)) { return false; }
    for (index = 0; index < candidate.configurationNodes.size(); ++index) { if (candidate.configurationNodes[index].id == node.id) { if (candidate.configurationNodes[index].kind != QStringLiteral("dataCollection")) { return false; } candidate.configurationNodes[index] = node; replaced = true; break; } }
    if (!replaced) { candidate.configurationNodes.append(node); }
    pages = candidate.allPages();
    for (pageIndex = 0; pageIndex < pages.size(); ++pageIndex)
    {
        for (objectIndex = 0; objectIndex < pages[pageIndex].objects.size(); ++objectIndex)
        {
            object = &pages[pageIndex].objects[objectIndex]; if (object->type == QStringLiteral("historydata") && object->historyData.collectionId == node.id) { object->historyData.reconcileCollection(collection, languages); }
        }
    }
    candidate.inactivePages.clear();
    for (pageIndex = 0; pageIndex < pages.size(); ++pageIndex) { if (pages[pageIndex].id == candidate.activePageId) { assignActivePage(&candidate, pages[pageIndex]); } else { candidate.inactivePages.append(pages[pageIndex]); } }
    if (!candidate.validateModel(error)) { return false; } *this = candidate; return true;''', 'node, error', 'True after an atomic collection-and-table update.')
for path,s in sources.items():(root/path).write_text(s,encoding='utf-8',newline='\n')
print('Integrated C30 strict schema, language/clipboard/visibility paths, native command and all-page collection transaction.')
