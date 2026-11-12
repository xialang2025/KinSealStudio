#include "recipe_display_properties.h"
#include "project_model.h"
#include <QColor>
#include <QSet>
#include <cmath>

/******************************************************************************
 * Function Name   : recipeDisplayError
 * Description     : Report an actionable display or dependency validation error.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool recipeDisplayError(QString *error, const QString &message) //20261113-2 add
{
    if (error != nullptr) { *error = message; } return false;
}

/******************************************************************************
 * Function Name   : RecipeDisplayProperties::RecipeDisplayProperties
 * Description     : Preserve the measured visibility level one and touch level two defaults.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
RecipeDisplayProperties::RecipeDisplayProperties() //20261113-2 add
{
    touch.minimumLevel = 2;
}

/******************************************************************************
 * Function Name   : RecipeDisplayProperties::validate
 * Description     : Validate measured appearance, keyboard-coordinate ranges, explicit permission states and exclusive notifications.
 * Input Parameters: error
 * Return Value    : True for a bounded recipe-display draft.
 * Date            : 2026.11.13
 ******************************************************************************/
bool RecipeDisplayProperties::validate(QString *error) const //20261113-2 add
{
    QString color;
    StaticTextProperties notification;
    if (recipeId.trimmed().isEmpty() || recipeId.size() > 128 || !QList<int>({12, 16, 24, 48}).contains(fontSize) || alignment < 0 || alignment > 2 || keyboardPageId.size() > 128 || keyboardX < 0 || keyboardX > 800 || keyboardY < 0 || keyboardY > 800 || noticeWordType < 0 || noticeWordType > 4 || (noticeBit && noticeWord) || noticeBitAddress.size() > 1024 || noticeWordAddress.size() > 1024 || noticeWordValue.size() > 1024 || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen) || (touch.visibilityEnabled && touch.bitEnabled && !touchBitChosen)) { return recipeDisplayError(error, QStringLiteral("配方显示器样式、键盘、通知或权限状态无效。")); }
    Q_FOREACH (color, (QStringList{titleColor, textColor, titleBackground, backgroundColor, gridColor})) { if (color.size() > 32 || !QColor(color).isValid()) { return false; } }
    if (!visibility.validate(error) || !touch.validate(error)) { return false; }
    notification.visibilityEnabled = notice && noticeWord; notification.wordEnabled = notification.visibilityEnabled; notification.dataType = noticeWordType; notification.compareValue = noticeWordValue; return notification.validate(error);
}

/******************************************************************************
 * Function Name   : RecipeDisplayProperties::validateAddresses
 * Description     : Validate every active visibility, touch and notification carrier without a fabricated operation address.
 * Input Parameters: connections, error
 * Return Value    : True for complete active address spans.
 * Date            : 2026.11.13
 ******************************************************************************/
bool RecipeDisplayProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261113-2 add
{
    ScreenButtonProperties shared;
    shared.common.indicator = false; shared.common.visibility = visibility; shared.common.touch = touch; shared.common.notice = notice; shared.common.noticeBit = noticeBit; shared.common.noticeBitAddress = noticeBitAddress; shared.common.noticeWord = noticeWord; shared.common.noticeWordAddress = noticeWordAddress; shared.common.noticeWordType = noticeWordType; return shared.validateAddresses(connections, error);
}

/******************************************************************************
 * Function Name   : RecipeDisplayProperties::validateContext
 * Description     : Bind one real recipe definition and require the correct stable keyboard page only for active custom editing.
 * Input Parameters: project, error
 * Return Value    : True for valid required recipe and active keyboard dependencies.
 * Date            : 2026.11.13
 ******************************************************************************/
bool RecipeDisplayProperties::validateContext(const HmiProject &project, QString *error) const //20261113-2 add
{
    HmiConfigurationNode node;
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
    return true;
}

/******************************************************************************
 * Function Name   : RecipeDisplayProperties::toJson
 * Description     : Store complete independent display drafts, including disabled keyboard coordinates and notification values.
 * Input Parameters: None
 * Return Value    : Exact typed recipe-display JSON.
 * Date            : 2026.11.13
 ******************************************************************************/
QJsonObject RecipeDisplayProperties::toJson() const //20261113-2 add
{
    return {{QStringLiteral("recipeId"), recipeId}, {QStringLiteral("titleColor"), titleColor}, {QStringLiteral("textColor"), textColor}, {QStringLiteral("titleBackground"), titleBackground}, {QStringLiteral("backgroundColor"), backgroundColor}, {QStringLiteral("gridColor"), gridColor}, {QStringLiteral("keyboardPageId"), keyboardPageId}, {QStringLiteral("noticeBitAddress"), noticeBitAddress}, {QStringLiteral("noticeWordAddress"), noticeWordAddress}, {QStringLiteral("noticeWordValue"), noticeWordValue}, {QStringLiteral("fontSize"), fontSize}, {QStringLiteral("alignment"), alignment}, {QStringLiteral("keyboardX"), keyboardX}, {QStringLiteral("keyboardY"), keyboardY}, {QStringLiteral("noticeWordType"), noticeWordType}, {QStringLiteral("showGrid"), showGrid}, {QStringLiteral("editEnabled"), editEnabled}, {QStringLiteral("customKeyboard"), customKeyboard}, {QStringLiteral("keyboardPosition"), keyboardPosition}, {QStringLiteral("visibilityBitChosen"), visibilityBitChosen}, {QStringLiteral("touchBitChosen"), touchBitChosen}, {QStringLiteral("notice"), notice}, {QStringLiteral("noticeBit"), noticeBit}, {QStringLiteral("noticeBitValue"), noticeBitValue}, {QStringLiteral("noticeWord"), noticeWord}, {QStringLiteral("visibility"), visibility.toJson()}, {QStringLiteral("touch"), touch.toJson()}};
}

/******************************************************************************
 * Function Name   : RecipeDisplayProperties::fromJson
 * Description     : Require exact field shapes and permission records before atomically replacing a recipe display.
 * Input Parameters: json, output, error
 * Return Value    : True after exact complete display validation.
 * Date            : 2026.11.13
 ******************************************************************************/
bool RecipeDisplayProperties::fromJson(const QJsonObject &json, RecipeDisplayProperties *output, QString *error) //20261113-2 add
{
    RecipeDisplayProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double value;
    if (output == nullptr || json.keys() != shape.keys()) { return recipeDisplayError(error, QStringLiteral("配方显示字段缺失或未知。")); }
    for (field = json.constBegin(); field != json.constEnd(); ++field) { if (field.value().type() != shape.value(field.key()).type()) { return false; } if (field.value().isDouble()) { value = field.value().toDouble(); if (!std::isfinite(value) || value < 0 || value > 800 || value != std::floor(value)) { return false; } } }
    candidate.recipeId = json.value(QStringLiteral("recipeId")).toString();
    candidate.titleColor = json.value(QStringLiteral("titleColor")).toString();
    candidate.textColor = json.value(QStringLiteral("textColor")).toString();
    candidate.titleBackground = json.value(QStringLiteral("titleBackground")).toString();
    candidate.backgroundColor = json.value(QStringLiteral("backgroundColor")).toString();
    candidate.gridColor = json.value(QStringLiteral("gridColor")).toString();
    candidate.keyboardPageId = json.value(QStringLiteral("keyboardPageId")).toString();
    candidate.noticeBitAddress = json.value(QStringLiteral("noticeBitAddress")).toString();
    candidate.noticeWordAddress = json.value(QStringLiteral("noticeWordAddress")).toString();
    candidate.noticeWordValue = json.value(QStringLiteral("noticeWordValue")).toString();
    candidate.fontSize = json.value(QStringLiteral("fontSize")).toInt();
    candidate.alignment = json.value(QStringLiteral("alignment")).toInt();
    candidate.keyboardX = json.value(QStringLiteral("keyboardX")).toInt();
    candidate.keyboardY = json.value(QStringLiteral("keyboardY")).toInt();
    candidate.noticeWordType = json.value(QStringLiteral("noticeWordType")).toInt();
    candidate.showGrid = json.value(QStringLiteral("showGrid")).toBool();
    candidate.editEnabled = json.value(QStringLiteral("editEnabled")).toBool();
    candidate.customKeyboard = json.value(QStringLiteral("customKeyboard")).toBool();
    candidate.keyboardPosition = json.value(QStringLiteral("keyboardPosition")).toBool();
    candidate.visibilityBitChosen = json.value(QStringLiteral("visibilityBitChosen")).toBool();
    candidate.touchBitChosen = json.value(QStringLiteral("touchBitChosen")).toBool();
    candidate.notice = json.value(QStringLiteral("notice")).toBool();
    candidate.noticeBit = json.value(QStringLiteral("noticeBit")).toBool();
    candidate.noticeBitValue = json.value(QStringLiteral("noticeBitValue")).toBool();
    candidate.noticeWord = json.value(QStringLiteral("noticeWord")).toBool();
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !StaticTextProperties::fromJson(json.value(QStringLiteral("touch")).toObject(), &candidate.touch, error) || candidate.touch.toJson() != json.value(QStringLiteral("touch")).toObject() || !candidate.validate(error)) { return false; } *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : validateRecipeDefinitions
 * Description     : Validate real global recipes and protect consumers on all pages against missing definitions and keyboards.
 * Input Parameters: project, error
 * Return Value    : True for valid all-page recipe dependencies.
 * Date            : 2026.11.13
 ******************************************************************************/
bool validateRecipeDefinitions(const HmiProject &project, QString *error) //20261113-2 add
{
    HmiConfigurationNode node;
    HmiPage page;
    HmiObject object;
    RecipeProperties properties;
    QSet<QString> names;
    Q_FOREACH (node, project.configurationNodes)
    {
        if (node.kind != QStringLiteral("recipe") || !node.active || !node.implemented) { continue; }
        if (!names.isEmpty()) { return recipeDisplayError(error, QStringLiteral("已经达到最大配方数量，请删除后再新建。")); } //20261113-2 add: The target supports one definition with multiple recipe columns.
        if (!RecipeProperties::fromJson(node.properties, &properties, error) || !properties.validateAddresses(project.connections, error)) { return recipeDisplayError(error, QStringLiteral("配方名称重复，或配方属性/完整读写跨度无效。")); } names.insert(node.name);
    }
    Q_FOREACH (page, project.allPages()) { Q_FOREACH (object, page.objects) { if (object.type == QStringLiteral("recipe") && !object.recipeDisplay.validateContext(project, error)) { return false; } } } return true;
}
