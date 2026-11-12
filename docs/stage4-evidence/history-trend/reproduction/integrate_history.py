"""Wire C24 to strict project records, actual commands, tree transactions and safe clipboard dependencies."""
from pathlib import Path
import ast
root=Path(__file__).resolve().parent/'worktree'
sources={}
def edit(path,before,after):
 s=sources.get(path,(root/path).read_text(encoding='utf-8'))
 assert s.count(before)==1,(path,before,s.count(before))
 sources[path]=s.replace(before,after)
tree=ast.parse((root.parent/'integrate_chart.py').read_text(encoding='utf-8'))
for node in tree.body:
 if not isinstance(node,ast.Expr) or not isinstance(node.value,ast.Call) or not isinstance(node.value.func,ast.Name) or node.value.func.id!='edit': continue
 path,before,after=map(ast.literal_eval,node.value.args)
 if path in ('src/stage3_project_edits.cpp','src/main_window.cpp'): continue
 after=after.replace('chart_properties','history_trend_properties').replace('chart_dialog','history_trend_dialog').replace('ChartProperties','HistoryTrendProperties').replace('ChartDialog','HistoryTrendDialog').replace('paintChart','paintHistoryTrend').replace('object->chart','object->historyTrend').replace('chart.toJson()','historyTrend.toJson()').replace('object.chart','object.historyTrend').replace('ChartProperties chart','HistoryTrendProperties historyTrend').replace('QStringLiteral("chart")','QStringLiteral("historytrend")').replace('QStringLiteral("图表','QStringLiteral("历史趋势图')
 if path=='CMakeLists.txt' and before.endswith('properties.cpp)'):
  before='src/trend_properties.cpp)';after='src/trend_properties.cpp src/history_trend_properties.cpp src/data_collection_properties.cpp)'
 if path=='CMakeLists.txt' and before=='    src/move_track_dialog.cpp': after+='\n    src/data_collection_properties.h\n    src/data_collection_properties.cpp\n    src/data_collection_dialog.h\n    src/data_collection_dialog.cpp'
 if path=='src/project_model.cpp' and before=='QStringLiteral("moveTrack"), QStringLiteral("extensions")':
  before='QStringLiteral("trend"), QStringLiteral("extensions")';after='QStringLiteral("trend"), QStringLiteral("historytrend"), QStringLiteral("extensions")'
 if path=='src/project_model.cpp' and before=='value.toObject().contains(QStringLiteral("moveTrack")))':
  before='value.toObject().contains(QStringLiteral("trend")))';after='value.toObject().contains(QStringLiteral("trend")) || value.toObject().contains(QStringLiteral("historytrend")))'
 if path=='src/project_model.h' and before=='    MoveTrackProperties moveTrack;': after=after.replace('HistoryTrendProperties chart;','HistoryTrendProperties historyTrend;').replace('Separate bar, meter and donut drafts.','Stable collection and item references for historical display.')
 if path=='src/native_shell_ui.cpp':after=after.replace('graphicsitem','action_DatagroupDisplay').replace('btn_metre','btn_DatagroupDisplay')
 if path=='src/hmi_canvas.cpp' and before.startswith('object.name = '):
  before='object.name = (object.type == QStringLiteral("trend")';after='object.name = (object.type == QStringLiteral("historytrend") ? QStringLiteral("历史趋势图%1") : object.type == QStringLiteral("trend")'
 if path=='src/native_shell_ui.cpp' and before.startswith('!selected ||'):
  before='!selected || object.type == QStringLiteral("trend")';after='!selected || object.type == QStringLiteral("historytrend") || object.type == QStringLiteral("trend")'
 if path=='src/static_text_actions.cpp' and before=='|| object.type == QStringLiteral("track")))':
  before='|| object.type == QStringLiteral("trend")))';after='|| object.type == QStringLiteral("trend") || object.type == QStringLiteral("historytrend")))'
 after=after.replace('QRect(0, 0, 170, 130)','QRect(0, 0, 240, 160)').replace('m_record.chart','m_record.historyTrend')
 if path=='src/stage4_control_actions.cpp' and before=='    else if (type == QStringLiteral("track")) //':
  after=after.replace('{ object.name =', '''{
        HmiConfigurationNode collection;
        bool found = false;
        Q_FOREACH (collection, currentEditorProject().configurationNodes) { if (collection.kind == QStringLiteral("dataCollection") && collection.active && collection.implemented) { found = true; break; } }
        if (!found) { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), QStringLiteral("尚未建立数据采集！")); return; }
        object.name =''')
 edit(path,before,after)
edit('src/project_model.cpp','    if (candidate.kind == QStringLiteral("system")', '''    if (candidate.kind == QStringLiteral("dataCollection") && candidate.implemented && candidate.active && candidate.ownerPageId.isEmpty()) //20261112-1 add
    {
        DataCollectionProperties properties;
        if (candidate.id.trimmed().isEmpty() || candidate.name.trimmed().isEmpty() || candidate.name.size() > 48 || !DataCollectionProperties::fromJson(candidate.properties, &properties, error) || !inactiveExtensions(candidate.extensions, QStringLiteral("configurationNode.extensions"), error)) { return false; }
        *node = candidate; return true;
    }
    if (candidate.kind == QStringLiteral("system")''')
edit('src/project_model.cpp','    if (!animations.isEmpty()) { schema = 12; stage1 = true; }','    Q_FOREACH (node, configurationNodes) { if (node.kind == QStringLiteral("dataCollection") && node.implemented && node.active) { schema = 12; stage1 = true; } } //20261112-1 add: Collector-only files include all schema12 containers.\n    if (!animations.isEmpty()) { schema = 12; stage1 = true; }')
edit('src/project_model.cpp','    if (hasSystemSettings && !readStage2SystemSettings', '''    //20261112-1 add: Resolve collection identities over all pages before allowing a rename, removal or paste to publish.
    {
        QMap<QString, DataCollectionProperties> collections;
        DataCollectionProperties properties;
        Q_FOREACH (node, configurationNodes)
        {
            if (node.kind != QStringLiteral("dataCollection") || !node.active || !node.implemented) { continue; }
            if (!DataCollectionProperties::fromJson(node.properties, &properties, error) || !properties.validateAddresses(connections, error)) { return false; }
            collections.insert(node.id, properties);
        }
        if (collections.size() > 1) { return modelError(error, QStringLiteral("已经达到最大数据采集数量，请删除后再新建。")); }
        Q_FOREACH (page, pages)
        {
            Q_FOREACH (object, page.objects)
            {
                if (object.type == QStringLiteral("historytrend") && (!collections.contains(object.historyTrend.collectionId) || !object.historyTrend.validateCollection(collections.value(object.historyTrend.collectionId), error))) { return modelError(error, QStringLiteral("历史趋势图的数据采集或数据项引用无效。")); }
            }
        }
    }
    if (hasSystemSettings && !readStage2SystemSettings''')
edit('src/main_window.cpp','    if (object.type == QStringLiteral("trend")) //','    if (object.type == QStringLiteral("historytrend")) { object.address.clear(); object.text.clear(); } //20261112-1 add: A historical curve has no direct operation carrier.\n    if (object.type == QStringLiteral("trend")) //')
edit('src/main_window.cpp','object.type == QStringLiteral("time") || object.type == QStringLiteral("scale") || object.type == QStringLiteral("gif") || object.hasImageDisplayProperties))','object.type == QStringLiteral("historytrend") || object.type == QStringLiteral("time") || object.type == QStringLiteral("scale") || object.type == QStringLiteral("gif") || object.hasImageDisplayProperties))')
edit('src/stage3_project_edits.cpp','    if (object->type == QStringLiteral("trend")) //','    if (object->type == QStringLiteral("historytrend")) { addConditionSlots(&object->historyTrend.visibility, false, &addressSlots); } //20261112-1 add\n    if (object->type == QStringLiteral("trend")) //')
edit('src/stage3_project_edits.cpp','        if (object.type == QStringLiteral("slider") && !object.slider.macroName', '''        if (object.type == QStringLiteral("historytrend")) //20261112-1 add: Reuse the exact collection or copy it only into an empty target; never overwrite a conflicting collector.
        {
            HmiConfigurationNode required;
            HmiConfigurationNode peer;
            bool found = false;
            bool reused = false;
            Q_FOREACH (peer, clipboard.source.configurationNodes) { if (peer.id == object.historyTrend.collectionId && peer.kind == QStringLiteral("dataCollection") && peer.active && peer.implemented) { required = peer; found = true; break; } }
            if (!found) { return editError(error, QStringLiteral("剪贴板缺少历史趋势图的数据采集定义。")); }
            Q_FOREACH (peer, candidate->configurationNodes)
            {
                if (peer.id == required.id && peer.toJson() == required.toJson()) { reused = true; }
                else if (peer.id == required.id || (peer.kind == QStringLiteral("dataCollection") && peer.active && peer.implemented)) { return editError(error, QStringLiteral("目标工程已有不同的数据采集，粘贴已取消。")); }
            }
            if (!reused) { candidate->configurationNodes.append(required); }
        }
        if (object.type == QStringLiteral("slider") && !object.slider.macroName''')
edit('src/main_window.h','    void editStage4Control();','    void editStage4Control();\n    void configureDataCollection(const QString &id = QString()); //20261112-1 add\n    void deleteDataCollection(const QString &id);')
edit('src/native_shell_ui.cpp','            else if (id == QStringLiteral("action_trendchart"))','            else if (id == QStringLiteral("action_datasamp")) { connect(action, &QAction::triggered, this, [this]() { configureDataCollection(); }); } //20261112-1 add\n            else if (id == QStringLiteral("action_trendchart"))')
edit('src/main_window.cpp','        else if (role == QStringLiteral("macros"))','        else if (role == QStringLiteral("dataCollection")) { configureDataCollection(pageId); } //20261112-1 add\n        else if (role == QStringLiteral("macros"))')
edit('src/main_window.cpp','    page = new QWidget(this); layout = new QVBoxLayout(page); //20261111-1 change','''    m_systemSettingsTree->setContextMenuPolicy(Qt::CustomContextMenu); //20261112-1 add
    connect(m_systemSettingsTree, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint &position) {
        QTreeWidgetItem *item = m_systemSettingsTree->itemAt(position);
        QString id;
        QMenu menu(this);
        if (item == nullptr || item->data(0, Qt::UserRole).toString() != QStringLiteral("dataCollection")) { return; }
        id = item->data(0, Qt::UserRole + 1).toString();
        if (id.isEmpty()) { menu.addAction(QStringLiteral("新增"), this, [this]() { configureDataCollection(); }); }
        else { menu.addAction(QStringLiteral("编辑"), this, [this, id]() { configureDataCollection(id); }); menu.addAction(QStringLiteral("删除"), this, [this, id]() { deleteDataCollection(id); }); }
        menu.exec(m_systemSettingsTree->viewport()->mapToGlobal(position));
    });
    page = new QWidget(this); layout = new QVBoxLayout(page); //20261111-1 change''')
edit('src/main_window_workflow.cpp','    HmiPage page;\n    PlcConnection connection;\n    QString label;','    HmiPage page;\n    HmiConfigurationNode collection; //20261112-1 add\n    QTreeWidgetItem *collectionItem;\n    PlcConnection connection;\n    QString label;')
edit('src/main_window_workflow.cpp','    item = new QTreeWidgetItem(settings, {QStringLiteral("数据采集")});\n    item->setData(0, Qt::UserRole, QStringLiteral("unsupported"));','''    item = new QTreeWidgetItem(settings, {QStringLiteral("数据采集")});
    item->setData(0, Qt::UserRole, QStringLiteral("dataCollection")); //20261112-1 change
    Q_FOREACH (collection, m_project.configurationNodes)
    {
        if (collection.kind != QStringLiteral("dataCollection")) { continue; }
        collectionItem = new QTreeWidgetItem(item, {collection.name}); collectionItem->setData(0, Qt::UserRole, QStringLiteral("dataCollection")); collectionItem->setData(0, Qt::UserRole + 1, collection.id);
    }''')
edit('src/stage4_control_actions.cpp','#include "hmi_canvas.h"','#include "hmi_canvas.h"\n#include "data_collection_dialog.h" //20261112-1 add')
for path,s in sources.items(): (root/path).write_text(s,encoding='utf-8',newline='\n')
# Reuse the standard documentation format for the two new tree actions.
from importlib.util import spec_from_file_location, module_from_spec
# Do not import the UI generation script: it writes files on import.
def function(name,description,signature,body):
 return f'\n/******************************************************************************\n * Function Name   : {name}\n * Description     : {description}\n * Input Parameters: id\n * Return Value    : None.\n * Date            : 2026.11.12\n ******************************************************************************/\n{signature} //20261112-1 add\n{{\n{body}\n}}\n'
p=root/'src/stage4_control_actions.cpp';s=p.read_text(encoding='utf-8')
s+=function('MainWindow::configureDataCollection','Commit an accepted collection definition as one undoable project transaction.', 'void MainWindow::configureDataCollection(const QString &id)', '''    HmiProject candidate = currentEditorProject();
    HmiConfigurationNode node;
    HmiConfigurationNode peer;
    QString error;
    int index;
    int existing = -1;
    if (!m_hasProject) { return; }
    for (index = 0; index < candidate.configurationNodes.size(); ++index)
    {
        peer = candidate.configurationNodes[index];
        if (!id.isEmpty() && peer.id == id && peer.kind == QStringLiteral("dataCollection")) { node = peer; existing = index; }
        if (id.isEmpty() && peer.kind == QStringLiteral("dataCollection") && peer.active && peer.implemented) { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), QStringLiteral("已经达到最大数据采集数量，请删除后再新建。")); return; }
    }
    if (!id.isEmpty() && existing < 0) { return; }
    if (id.isEmpty()) { node.kind = QStringLiteral("dataCollection"); node.name = QStringLiteral("数据采集1"); }
    DataCollectionDialog dialog(node, candidate, this);
    if (dialog.exec() != QDialog::Accepted) { return; }
    if (existing < 0) { candidate.configurationNodes.append(dialog.node()); } else { candidate.configurationNodes[existing] = dialog.node(); }
    if (!commitProjectDraft(candidate, &error)) { QMessageBox::warning(this, QStringLiteral("数据采集未提交"), error); }
''')
s+=function('MainWindow::deleteDataCollection','Refuse referenced collector removal through the whole-project validator before publishing.', 'void MainWindow::deleteDataCollection(const QString &id)', '''    HmiProject candidate = currentEditorProject();
    QString error;
    int index;
    for (index = 0; index < candidate.configurationNodes.size(); ++index)
    {
        if (candidate.configurationNodes[index].id == id && candidate.configurationNodes[index].kind == QStringLiteral("dataCollection"))
        {
            candidate.configurationNodes.removeAt(index);
            if (!commitProjectDraft(candidate, &error)) { QMessageBox::warning(this, QStringLiteral("数据采集正在使用"), error); }
            return;
        }
    }
''')
# Keep construction local declarations at the beginning of the collection edit function.
s=s.replace('    int existing = -1;\n    if (!m_hasProject)', '    int existing = -1;\n    DataCollectionDialog *dialog;\n    if (!m_hasProject)').replace('    DataCollectionDialog dialog(node, candidate, this);\n    if (dialog.exec() != QDialog::Accepted) { return; }\n    if (existing < 0) { candidate.configurationNodes.append(dialog.node()); } else { candidate.configurationNodes[existing] = dialog.node(); }','    dialog = new DataCollectionDialog(node, candidate, this);\n    if (dialog->exec() != QDialog::Accepted) { delete dialog; return; }\n    if (existing < 0) { candidate.configurationNodes.append(dialog->node()); } else { candidate.configurationNodes[existing] = dialog->node(); }\n    delete dialog;')
p.write_text(s,encoding='utf-8',newline='\n')
print('Integrated history models, active collection schema12, all-page references, tree lifecycle and clipboard dependency guards.')
