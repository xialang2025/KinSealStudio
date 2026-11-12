"""Wire C31 definitions, display, all-page dependencies and transient canvas context."""
from pathlib import Path
import ast
root=Path(__file__).resolve().parent/'worktree';sources={}
tree=ast.parse(Path(__file__).with_name('make_recipe_models.py').read_text(encoding='utf-8'));ns={};exec(compile(ast.Module(body=[n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='f'],type_ignores=[]),'<format>','exec'),ns);f=ns['f']
def edit(path,before,after):
    s=sources.get(path,(root/path).read_text(encoding='utf-8'));assert s.count(before)==1,(path,before,s.count(before));sources[path]=s.replace(before,after)
tree=ast.parse(Path(__file__).with_name('integrate_chart.py').read_text(encoding='utf-8'))
for n in tree.body:
    if not isinstance(n,ast.Expr) or not isinstance(n.value,ast.Call) or not isinstance(n.value.func,ast.Name) or n.value.func.id!='edit':continue
    path,before,after=map(ast.literal_eval,n.value.args)
    after=after.replace('chart_properties','recipe_display_properties').replace('chart_dialog','recipe_display_dialog').replace('ChartProperties','RecipeDisplayProperties').replace('ChartDialog','RecipeDisplayDialog').replace('paintChart','paintRecipeDisplay').replace('object->chart','object->recipeDisplay').replace('object.chart','object.recipeDisplay').replace('m_record.chart','m_record.recipeDisplay').replace('chart.toJson()','recipeDisplay.toJson()').replace('RecipeDisplayProperties chart','RecipeDisplayProperties recipeDisplay').replace('QStringLiteral("chart")','QStringLiteral("recipe")').replace('QStringLiteral("图表','QStringLiteral("配方显示器').replace('//20261112-1','//20261113-2')
    if path=='CMakeLists.txt':
        if before.endswith('properties.cpp)'):before='src/history_data_properties.cpp)';after='src/history_data_properties.cpp src/recipe_properties.cpp src/recipe_display_properties.cpp src/recipe_csv.cpp)'
        else:after+='\n    src/recipe_properties.h\n    src/recipe_properties.cpp\n    src/recipe_csv.h\n    src/recipe_csv.cpp\n    src/recipe_dialog.h\n    src/recipe_dialog.cpp';after=after.replace('src/recipe_display_properties.h #20261112-1','src/recipe_display_properties.h #20261113-2')
    if path=='src/project_model.cpp':
        if before=='QStringLiteral("moveTrack"), QStringLiteral("extensions")':before='QStringLiteral("historyData"), QStringLiteral("extensions")';after='QStringLiteral("historyData"), QStringLiteral("recipeDisplay"), QStringLiteral("extensions")'
        elif before=='value.toObject().contains(QStringLiteral("moveTrack")))':before='value.toObject().contains(QStringLiteral("historyData")))';after='value.toObject().contains(QStringLiteral("historyData")) || value.toObject().contains(QStringLiteral("recipeDisplay")))'
        after=after.replace('json.insert(QStringLiteral("recipe")','json.insert(QStringLiteral("recipeDisplay")').replace('key == QStringLiteral("recipe")','key == QStringLiteral("recipeDisplay")').replace('json.contains(QStringLiteral("recipe"))','json.contains(QStringLiteral("recipeDisplay"))')
    if path=='src/stage3_project_edits.cpp':after='''    if (object->type == QStringLiteral("recipe")) //20261113-2 add: The recipe definition stays shared; duplicate only per-display permission and notification addresses.
    {
        addConditionSlots(&object->recipeDisplay.visibility, false, &addressSlots); addConditionSlots(&object->recipeDisplay.touch, true, &addressSlots);
        if (object->recipeDisplay.notice && object->recipeDisplay.noticeBit) { addressSlots.append({Stage3AddressField::NoticeBit, &object->recipeDisplay.noticeBitAddress, PlcAddressRole::Bit, 1U}); }
        if (object->recipeDisplay.notice && object->recipeDisplay.noticeWord) { addressSlots.append({Stage3AddressField::NoticeWord, &object->recipeDisplay.noticeWordAddress, PlcAddressRole::Word, object->recipeDisplay.noticeWordType == 0 || object->recipeDisplay.noticeWordType == 2 ? 1U : 2U}); }
    }
    if (object->type == QStringLiteral("track")) //'''
    if path=='src/native_shell_ui.cpp':
        after=after.replace('graphicsitem','action_recipetable').replace('btn_metre','btn__recipetable')
        if before.startswith('!selected ||'):before='!selected || object.type == QStringLiteral("historydata")';after='!selected || object.type == QStringLiteral("recipe") || object.type == QStringLiteral("historydata")'
    if path=='src/hmi_canvas.cpp':
        if before.startswith('object.name = '):before='object.name = (object.type == QStringLiteral("historydata")';after='object.name = (object.type == QStringLiteral("recipe") ? QStringLiteral("配方显示器%1") : object.type == QStringLiteral("historydata")'
        else:after=after.replace('paintRecipeDisplay(painter, item_rect, m_record.recipeDisplay)','paintRecipeDisplay(painter, item_rect, m_record.recipeDisplay, data(12).toStringList(), data(13).toInt())')
    if path in ('src/main_window.cpp','src/static_text_actions.cpp') and before=='|| object.type == QStringLiteral("track")))':before='|| (object.type == QStringLiteral("alarmdisplay") || object.type == QStringLiteral("historyalarm"))))';after='|| object.type == QStringLiteral("recipe") || (object.type == QStringLiteral("alarmdisplay") || object.type == QStringLiteral("historyalarm"))))'
    if path=='src/main_window.cpp' and before=='    if (object.type == QStringLiteral("track")) //':after='    if (object.type == QStringLiteral("recipe")) { object.address.clear(); object.text.clear(); } //20261113-2 add\n    if (object.type == QStringLiteral("track")) //'
    if path=='src/main_window.cpp' and before.startswith('        if (object.type == QStringLiteral("track")) { m_addressEdit'):continue
    if path=='src/stage4_control_actions.cpp' and before=='    else if (type == QStringLiteral("track")) //':after='''    else if (type == QStringLiteral("recipe")) //20261113-2 add: The actual native prerequisite is a real global recipe, not a placeholder tree node.
    {
        HmiConfigurationNode definition;
        bool found = false;
        Q_FOREACH (definition, currentEditorProject().configurationNodes) { if (definition.kind == QStringLiteral("recipe") && definition.active && definition.implemented) { found = true; break; } }
        if (!found) { QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("配方不存在，请先创建配方")); return; }
        object.name = QStringLiteral("配方显示器"); object.geometry = QRect(0, 0, 320, 180); dialog = new RecipeDisplayDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("track")) //'''
    edit(path,before,after)
edit('src/main_window.cpp','object.type == QStringLiteral("historydata") || object.type == QStringLiteral("alarmbar")','object.type == QStringLiteral("recipe") || object.type == QStringLiteral("historydata") || object.type == QStringLiteral("alarmbar")')
edit('src/project_model.cpp','    if (candidate.kind == QStringLiteral("dataCollection") && candidate.implemented','''    if (candidate.kind == QStringLiteral("recipe") && candidate.implemented && candidate.active && candidate.ownerPageId.isEmpty()) //20261113-2 add
    {
        RecipeProperties properties;
        if (candidate.id.trimmed().isEmpty() || candidate.name.trimmed().isEmpty() || candidate.name.size() > 48 || !RecipeProperties::fromJson(candidate.properties, &properties, error) || !inactiveExtensions(candidate.extensions, QStringLiteral("configurationNode.extensions"), error)) { return false; } *node = candidate; return true;
    }
    if (candidate.kind == QStringLiteral("dataCollection") && candidate.implemented''')
edit('src/project_model.cpp','node.kind == QStringLiteral("dataCollection") || node.kind == QStringLiteral("alarm")','node.kind == QStringLiteral("recipe") || node.kind == QStringLiteral("dataCollection") || node.kind == QStringLiteral("alarm")')
edit('src/project_model.cpp','    if (!validateAlarmRegistrations(*this, error)) { return false; }','    if (!validateRecipeDefinitions(*this, error)) { return false; } //20261113-2 add: Resolve recipe and keyboard consumers on every page.\n    if (!validateAlarmRegistrations(*this, error)) { return false; }')
edit('src/project_model.cpp','        if (object.type == QStringLiteral("recipe") && (!object.address.isEmpty()', '        if (object.type == QStringLiteral("recipe") && (!object.text.isEmpty() || !object.address.isEmpty()')
edit('src/stage3_project_edits.cpp','        if (object.type == QStringLiteral("historytrend") || object.type == QStringLiteral("historydata")) //','''        if (object.type == QStringLiteral("recipe")) //20261113-2 add: Copy the exact required single recipe definition and active keyboard dependency atomically.
        {
            HmiConfigurationNode required;
            HmiConfigurationNode peer;
            bool found = false;
            bool reused = false;
            Q_FOREACH (peer, clipboard.source.configurationNodes) { if (peer.id == object.recipeDisplay.recipeId && peer.kind == QStringLiteral("recipe") && peer.active && peer.implemented) { required = peer; found = true; break; } }
            if (!found) { return editError(error, QStringLiteral("剪贴板缺少配方定义。")); }
            Q_FOREACH (peer, candidate->configurationNodes) { if (peer.id == required.id && peer.toJson() == required.toJson()) { reused = true; } else if (peer.id == required.id || (peer.kind == QStringLiteral("recipe") && peer.active && peer.implemented)) { return editError(error, QStringLiteral("目标工程已有不同的配方，粘贴已取消。")); } }
            if (!reused) { candidate->configurationNodes.append(required); }
            if (object.recipeDisplay.editEnabled && object.recipeDisplay.customKeyboard) { reference = HmiReference(); reference.id = QStringLiteral("recipe-keyboard"); reference.targetKind = QStringLiteral("page"); reference.targetId = object.recipeDisplay.keyboardPageId; if (!mergeReferenceTarget(clipboard.source, target, candidate, &reference, &mappedIds, error)) { return false; } object.recipeDisplay.keyboardPageId = reference.targetId; }
        }
        if (object.type == QStringLiteral("historytrend") || object.type == QStringLiteral("historydata")) //''')
edit('src/main_window.h','    void configureDataCollection(const QString &id = QString());','    void configureRecipe(const QString &id = QString()); //20261113-2 add\n    void deleteRecipe(const QString &id);\n    void configureDataCollection(const QString &id = QString());')
edit('src/stage4_control_actions.cpp','#include "data_collection_dialog.h"','#include "recipe_dialog.h" //20261113-2 add\n#include "data_collection_dialog.h"')
edit('src/native_shell_ui.cpp','            else if (id == QStringLiteral("action_datasamp"))','            else if (id == QStringLiteral("action_rep")) { connect(action, &QAction::triggered, this, [this]() { configureRecipe(); }); } //20261113-2 add\n            else if (id == QStringLiteral("action_datasamp"))') if 'action_datasamp' in sources.get('src/native_shell_ui.cpp','') else edit('src/native_shell_ui.cpp','            else if (id == QStringLiteral("action_datasamp"))','')
# The existing shell uses the normalized acquisition action spelling from its retained inventory.
for path,s in sources.items():(root/path).write_text(s,encoding='utf-8',newline='\n')
print('Integrated core recipe display/model/dependency routes; tree and transient canvas context follow separately.')
