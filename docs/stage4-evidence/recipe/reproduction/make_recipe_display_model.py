"""Define the separate recipe display, keyboard and notification contract."""
from pathlib import Path
import ast
root=Path(__file__).resolve().parent/'worktree';tree=ast.parse(Path(__file__).with_name('make_recipe_models.py').read_text(encoding='utf-8'));ns={};exec(compile(ast.Module(body=[n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='f'],type_ignores=[]),'<function-format>','exec'),ns);f=ns['f']
def put(path,text):(root/path).write_text(text,encoding='utf-8',newline='\n')
put('src/recipe_display_properties.h','''#ifndef RECIPE_DISPLAY_PROPERTIES_H
#define RECIPE_DISPLAY_PROPERTIES_H
#include "alarm_bar_properties.h"
#include "recipe_properties.h"
struct HmiProject;
//20261113-2 add: Only observed editing/keyboard controls are exposed; hidden row/column strings are not invented fields.
struct RecipeDisplayProperties
{
    QString recipeId;
    int fontSize = 16;
    int alignment = 1;
    QString titleColor = QStringLiteral("#0000ff");
    QString textColor = QStringLiteral("#000000");
    QString titleBackground = QStringLiteral("#ff6666");
    QString backgroundColor = QStringLiteral("#ffffff");
    QString gridColor = QStringLiteral("#000000");
    bool showGrid = true;
    bool editEnabled = false;
    bool customKeyboard = false;
    QString keyboardPageId;
    bool keyboardPosition = false;
    int keyboardX = 0;
    int keyboardY = 0;
    StaticTextProperties visibility;
    StaticTextProperties touch;
    bool visibilityBitChosen = false;
    bool touchBitChosen = false;
    bool notice = false;
    bool noticeBit = false;
    QString noticeBitAddress = QStringLiteral("LB0");
    bool noticeBitValue = true;
    bool noticeWord = false;
    QString noticeWordAddress = QStringLiteral("LW0");
    int noticeWordType = 0;
    QString noticeWordValue = QStringLiteral("0");
    RecipeDisplayProperties();
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
    bool validateContext(const HmiProject &project, QString *error = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, RecipeDisplayProperties *output, QString *error = nullptr);
};
bool validateRecipeDefinitions(const HmiProject &project, QString *error = nullptr);
#endif
''')
s='''#include "recipe_display_properties.h"
#include "project_model.h"
#include <QColor>
#include <QSet>
#include <cmath>
'''
s+=f('recipeDisplayError','Report an actionable display or dependency validation error.','static bool recipeDisplayError(QString *error, const QString &message)','    if (error != nullptr) { *error = message; } return false;','error, message','False.')
s+=f('RecipeDisplayProperties::RecipeDisplayProperties','Preserve the measured visibility level one and touch level two defaults.','RecipeDisplayProperties::RecipeDisplayProperties()', '    touch.minimumLevel = 2;')
s+=f('RecipeDisplayProperties::validate','Validate measured appearance, keyboard-coordinate ranges, explicit permission states and exclusive notifications.','bool RecipeDisplayProperties::validate(QString *error) const', '''    QString color;
    StaticTextProperties notification;
    if (recipeId.trimmed().isEmpty() || recipeId.size() > 128 || !QList<int>({12, 16, 24, 48}).contains(fontSize) || alignment < 0 || alignment > 2 || keyboardPageId.size() > 128 || keyboardX < 0 || keyboardX > 800 || keyboardY < 0 || keyboardY > 800 || noticeWordType < 0 || noticeWordType > 4 || (noticeBit && noticeWord) || noticeBitAddress.size() > 1024 || noticeWordAddress.size() > 1024 || noticeWordValue.size() > 1024 || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen) || (touch.visibilityEnabled && touch.bitEnabled && !touchBitChosen)) { return recipeDisplayError(error, QStringLiteral("配方显示器样式、键盘、通知或权限状态无效。")); }
    Q_FOREACH (color, (QStringList{titleColor, textColor, titleBackground, backgroundColor, gridColor})) { if (color.size() > 32 || !QColor(color).isValid()) { return false; } }
    if (!visibility.validate(error) || !touch.validate(error)) { return false; }
    notification.visibilityEnabled = notice && noticeWord; notification.wordEnabled = notification.visibilityEnabled; notification.dataType = noticeWordType; notification.compareValue = noticeWordValue; return notification.validate(error);''','error','True for a bounded recipe-display draft.')
s+=f('RecipeDisplayProperties::validateAddresses','Validate every active visibility, touch and notification carrier without a fabricated operation address.','bool RecipeDisplayProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const', '''    ScreenButtonProperties shared;
    shared.common.indicator = false; shared.common.visibility = visibility; shared.common.touch = touch; shared.common.notice = notice; shared.common.noticeBit = noticeBit; shared.common.noticeBitAddress = noticeBitAddress; shared.common.noticeWord = noticeWord; shared.common.noticeWordAddress = noticeWordAddress; shared.common.noticeWordType = noticeWordType; return shared.validateAddresses(connections, error);''','connections, error','True for complete active address spans.')
s+=f('RecipeDisplayProperties::validateContext','Bind one real recipe definition and require the correct stable keyboard page only for active custom editing.','bool RecipeDisplayProperties::validateContext(const HmiProject &project, QString *error) const', '''    HmiConfigurationNode node;
    HmiPage page;
    RecipeProperties definition;
    bool found = false;
    bool keyboard = false;
    Q_FOREACH (node, project.configurationNodes) { if (node.id == recipeId && node.kind == QStringLiteral("recipe") && node.active && node.implemented) { if (!RecipeProperties::fromJson(node.properties, &definition, error)) { return false; } found = true; break; } }
    if (!found) { return recipeDisplayError(error, QStringLiteral("配方不存在，请先创建配方")); }
    if (editEnabled && customKeyboard)
    {
        if (keyboardPageId.isEmpty()) { return recipeDisplayError(error, QStringLiteral("自定义键盘不能为空")); }
        Q_FOREACH (page, project.allPages()) { if (page.id == keyboardPageId && page.kind == QStringLiteral("keyboard")) { keyboard = true; break; } }
        if (!keyboard) { return recipeDisplayError(error, QStringLiteral("配方显示器的自定义键盘引用无效。")); }
    }
    return true;''','project, error','True for valid required recipe and active keyboard dependencies.')
strings=['recipeId','titleColor','textColor','titleBackground','backgroundColor','gridColor','keyboardPageId','noticeBitAddress','noticeWordAddress','noticeWordValue'];ints=['fontSize','alignment','keyboardX','keyboardY','noticeWordType'];bools=['showGrid','editEnabled','customKeyboard','keyboardPosition','visibilityBitChosen','touchBitChosen','notice','noticeBit','noticeBitValue','noticeWord']
s+=f('RecipeDisplayProperties::toJson','Store complete independent display drafts, including disabled keyboard coordinates and notification values.','QJsonObject RecipeDisplayProperties::toJson() const','    return {'+', '.join('{QStringLiteral("'+k+'"), '+k+'}' for k in strings+ints+bools)+', {QStringLiteral("visibility"), visibility.toJson()}, {QStringLiteral("touch"), touch.toJson()}};',returns='Exact typed recipe-display JSON.')
s+=f('RecipeDisplayProperties::fromJson','Require exact field shapes and permission records before atomically replacing a recipe display.','bool RecipeDisplayProperties::fromJson(const QJsonObject &json, RecipeDisplayProperties *output, QString *error)', '''    RecipeDisplayProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double value;
    if (output == nullptr || json.keys() != shape.keys()) { return recipeDisplayError(error, QStringLiteral("配方显示字段缺失或未知。")); }
    for (field = json.constBegin(); field != json.constEnd(); ++field) { if (field.value().type() != shape.value(field.key()).type()) { return false; } if (field.value().isDouble()) { value = field.value().toDouble(); if (!std::isfinite(value) || value < 0 || value > 800 || value != std::floor(value)) { return false; } } }
'''+''.join(f'    candidate.{k} = json.value(QStringLiteral("{k}")).to{kind}();\n' for keys,kind in [(strings,'String'),(ints,'Int'),(bools,'Bool')] for k in keys)+'''    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !StaticTextProperties::fromJson(json.value(QStringLiteral("touch")).toObject(), &candidate.touch, error) || candidate.touch.toJson() != json.value(QStringLiteral("touch")).toObject() || !candidate.validate(error)) { return false; } *output = candidate; return true;''','json, output, error','True after exact complete display validation.')
s+=f('validateRecipeDefinitions','Validate real global recipes and protect consumers on all pages against missing definitions and keyboards.','bool validateRecipeDefinitions(const HmiProject &project, QString *error)', '''    HmiConfigurationNode node;
    HmiPage page;
    HmiObject object;
    RecipeProperties properties;
    QSet<QString> names;
    Q_FOREACH (node, project.configurationNodes)
    {
        if (node.kind != QStringLiteral("recipe") || !node.active || !node.implemented) { continue; }
        if (names.contains(node.name) || !RecipeProperties::fromJson(node.properties, &properties, error) || !properties.validateAddresses(project.connections, error)) { return recipeDisplayError(error, QStringLiteral("配方名称重复，或配方属性/完整读写跨度无效。")); } names.insert(node.name);
    }
    Q_FOREACH (page, project.allPages()) { Q_FOREACH (object, page.objects) { if (object.type == QStringLiteral("recipe") && !object.recipeDisplay.validateContext(project, error)) { return false; } } } return true;''','project, error','True for valid all-page recipe dependencies.')
put('src/recipe_display_properties.cpp',s)
print('Created recipe display model with stable recipe/keyboard references and independent touch/notice validation.')
