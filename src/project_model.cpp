#include "project_model.h"
#include "native_test_project.h" //20261108-9 add
#include "native_project_codec.h" //20261109-0 add
#include "stage2_system_settings.h" //20261110-7 add: Validate only the explicitly recovered system-property schema.

#include <QFile>
#include <QFileInfo> //20261108-9 add
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>
#include <QSet>
#include <QUuid> //20261110-5 add
#include <limits>
#include <cmath> //20261108-14 add

namespace { //20261110-5 add: Keep JSON type checks ahead of every lossy Qt conversion.

/******************************************************************************
 * Function Name   : modelError
 * Description     : Return a localized validation failure without publishing model state.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.10
 ******************************************************************************/
bool modelError(QString *error, const QString &message)
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : exactInteger
 * Description     : Reject null, fractional and overflowing values before integer conversion.
 * Input Parameters: value
 * Return Value    : True for an exactly represented signed int.
 * Date            : 2026.11.10
 ******************************************************************************/
bool exactInteger(const QJsonValue &value)
{
    double number;

    number = value.toDouble(std::numeric_limits<double>::quiet_NaN());
    return value.isDouble() && std::isfinite(number) && std::floor(number) == number &&
           number >= std::numeric_limits<int>::min() && number <= std::numeric_limits<int>::max();
}

/******************************************************************************
 * Function Name   : knownKeys
 * Description     : Refuse unknown active fields instead of discarding them during serialization.
 * Input Parameters: json, keys, path, error
 * Return Value    : True if every supplied field has a declared owner.
 * Date            : 2026.11.10
 ******************************************************************************/
bool knownKeys(const QJsonObject &json, const QStringList &keys, const QString &path, QString *error)
{
    QJsonObject::const_iterator item;

    for (item = json.constBegin(); item != json.constEnd(); ++item)
    {
        if (!keys.contains(item.key()))
        {
            return modelError(error, QStringLiteral("%1.%2：未知活动字段，无法安全保全。").arg(path, item.key()));
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : inactiveExtensions
 * Description     : Bound explicitly inactive metadata by namespace, schema and encoded size.
 * Input Parameters: json, path, error
 * Return Value    : True for versioned inert metadata; it never enables native functionality.
 * Date            : 2026.11.10
 ******************************************************************************/
bool inactiveExtensions(const QJsonObject &json, const QString &path, QString *error)
{
    QJsonObject::const_iterator item;
    QJsonObject envelope;

    if (json.size() > 64 || QJsonDocument(json).toJson(QJsonDocument::Compact).size() > 64 * 1024)
    {
        return modelError(error, path + QStringLiteral("：扩展元数据超过Lite保护上限。"));
    }
    for (item = json.constBegin(); item != json.constEnd(); ++item)
    {
        envelope = item.value().toObject();
        if (!item.key().contains(QLatin1Char('.')) || item.key().size() > 128 || !item.value().isObject() ||
            !knownKeys(envelope, {QStringLiteral("version"), QStringLiteral("inactive"), QStringLiteral("data")}, path, error) ||
            !exactInteger(envelope.value(QStringLiteral("version"))) || envelope.value(QStringLiteral("version")).toInt() < 1 ||
            !envelope.value(QStringLiteral("inactive")).isBool() || !envelope.value(QStringLiteral("inactive")).toBool() ||
            !envelope.value(QStringLiteral("data")).isObject())
        {
            return modelError(error, path + QStringLiteral("：扩展仅允许带版本和inactive=true的命名空间元数据。"));
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : recordShape
 * Description     : Validate supplied property keys and types against their declared serializer shape.
 * Input Parameters: value, prototype, path, error
 * Return Value    : True without normalizing malformed booleans, numbers, maps or arrays.
 * Date            : 2026.11.10
 ******************************************************************************/
bool recordShape(const QJsonValue &value, const QJsonValue &prototype, const QString &path, QString *error)
{
    QJsonObject object;
    QJsonObject expected;
    QJsonObject::const_iterator item;
    QJsonArray array;
    QJsonArray expectedArray;
    QJsonValue exemplar;
    int index;

    if (value.type() != prototype.type()) { return modelError(error, path + QStringLiteral("：字段类型无效。")); }
    if (value.isDouble() && (!std::isfinite(value.toDouble()) ||
        (!path.endsWith(QStringLiteral(".fontSize")) && !path.endsWith(QStringLiteral(".spacing")) && !exactInteger(value))))
    {
        return modelError(error, path + QStringLiteral("：数值类型或范围无效。"));
    }
    if (value.isObject())
    {
        object = value.toObject(); expected = prototype.toObject();
        if (path.endsWith(QStringLiteral(".languageTexts")) || path.endsWith(QStringLiteral(".texts")))
        {
            for (item = object.constBegin(); item != object.constEnd(); ++item)
            {
                if (item.key().isEmpty() || !item.value().isString() || item.value().toString().size() > 1024 * 1024)
                {
                    return modelError(error, path + QStringLiteral("：翻译映射无效。"));
                }
            }
            return true;
        }
        if (!knownKeys(object, expected.keys(), path, error)) { return false; }
        for (item = object.constBegin(); item != object.constEnd(); ++item)
        {
            if (!recordShape(item.value(), expected.value(item.key()), path + QLatin1Char('.') + item.key(), error)) { return false; }
        }
    }
    if (value.isArray())
    {
        array = value.toArray(); expectedArray = prototype.toArray();
        exemplar = expectedArray.isEmpty() ? QJsonValue(BitButtonState().toJson()) : expectedArray.first();
        if (array.size() > 32) { return modelError(error, path + QStringLiteral("：状态数组超过Lite保护上限。")); }
        for (index = 0; index < array.size(); ++index)
        {
            if (!recordShape(array.at(index), exemplar, path + QStringLiteral("[%1]").arg(index), error)) { return false; }
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : activePage
 * Description     : Construct a value view from the sole active-page storage.
 * Input Parameters: project
 * Return Value    : Independent page value.
 * Date            : 2026.11.10
 ******************************************************************************/
HmiPage activePage(const HmiProject &project)
{
    HmiPage page;

    page.id = project.activePageId; page.kind = project.activePageKind;
    page.name = project.pageName; page.width = project.pageWidth; page.height = project.pageHeight;
    page.objects = project.objects; page.extensions = project.activePageExtensions;
    return page;
}

/******************************************************************************
 * Function Name   : assignActivePage
 * Description     : Move a selected value into the legacy active fields without retaining a mirror.
 * Input Parameters: project, page
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void assignActivePage(HmiProject *project, const HmiPage &page)
{
    project->activePageId = page.id; project->activePageKind = page.kind;
    project->pageName = page.name; project->pageWidth = page.width; project->pageHeight = page.height;
    project->objects = page.objects; project->activePageExtensions = page.extensions;
}

/******************************************************************************
 * Function Name   : hasStage1Data
 * Description     : Require schema9 only when the old single-page schemas cannot express content.
 * Input Parameters: project
 * Return Value    : True for additional topology, startup selection or explicit metadata.
 * Date            : 2026.11.10
 ******************************************************************************/
bool hasStage1Data(const HmiProject &project)
{
    HmiObject object;

    if (project.projectId != QStringLiteral("project-1") || !project.inactivePages.isEmpty() || project.activePageId != QStringLiteral("page-1") ||
        project.activePageKind != QStringLiteral("normal") || !project.activePageExtensions.isEmpty() ||
        project.pageOrder != QStringList{QStringLiteral("page-1")} || project.startupPageId != QStringLiteral("page-1") ||
        !project.startupLanguage.isEmpty() || !project.configurationNodes.isEmpty() || !project.references.isEmpty() ||
        !project.macroIds.isEmpty() || !project.extensions.isEmpty()) { return true; }
    Q_FOREACH (object, project.objects) { if (!object.extensions.isEmpty()) { return true; } }
    return false;
}

} // namespace

/******************************************************************************
 * Function Name   : HmiObject::toJson
 * Description     : Serialize one editable HMI object into the prototype schema.
 * Input Parameters: None
 * Return Value    : JSON object containing identity, appearance, address, and geometry.
 * Date            : 2026.11.07
 ******************************************************************************/
QJsonObject HmiObject::toJson() const //20261107-6 add
{
    QJsonObject json;

    json.insert(QStringLiteral("id"), id);
    json.insert(QStringLiteral("type"), type);
    json.insert(QStringLiteral("name"), name);
    json.insert(QStringLiteral("text"), text);
    json.insert(QStringLiteral("address"), address);
    json.insert(QStringLiteral("x"), geometry.x());
    json.insert(QStringLiteral("y"), geometry.y());
    json.insert(QStringLiteral("width"), geometry.width());
    json.insert(QStringLiteral("height"), geometry.height());
    json.insert(QStringLiteral("layer"), layer); //20261108-14 add
    json.insert(QStringLiteral("group"), group);
    if (!extensions.isEmpty()) { json.insert(QStringLiteral("extensions"), extensions); } //20261110-5 add
    if (type == QStringLiteral("text") && hasStaticTextProperties)
    {
        json.insert(QStringLiteral("staticText"), staticText.toJson());
    }
    if (type == QStringLiteral("switch") && hasBitButtonProperties) //20261108-15 add
    {
        json.insert(QStringLiteral("bitButton"), bitButton.toJson());
    }
    if (type == QStringLiteral("switch") && hasWordButtonProperties) //20261109-6 add
    {
        json.insert(QStringLiteral("wordButton"), wordButton.toJson());
    }
    if (type == QStringLiteral("switch") && hasLampProperties) //20261109-8 add
    {
        json.insert(QStringLiteral("lamp"), lamp.toJson());
    }
    if (type == QStringLiteral("switch") && hasFunctionButtonProperties) //20261110-0 add
    {
        json.insert(QStringLiteral("functionButton"), functionButton.toJson());
    }
    if (type == QStringLiteral("message")) { json.insert(QStringLiteral("messageDisplay"), messageDisplay.toJson()); } //20261112-0 add
    if (type == QStringLiteral("scale")) { json.insert(QStringLiteral("scale"), scale.toJson()); } //20261112-0 add
    if (type == QStringLiteral("qrcode")) { json.insert(QStringLiteral("qrCode"), qrCode.toJson()); } //20261112-0 add
    if (type == QStringLiteral("gif")) { json.insert(QStringLiteral("gifDisplay"), gifDisplay.toJson()); } //20261112-0 add
    if (type == QStringLiteral("number") && hasNumberDisplayProperties) { json.insert(QStringLiteral("numberDisplay"), numberDisplay.toJson()); } //20261112-0 add
    if (type == QStringLiteral("chart")) { json.insert(QStringLiteral("chart"), chart.toJson()); } //20261112-1 add
    if (type == QStringLiteral("trend")) { json.insert(QStringLiteral("trend"), trend.toJson()); } //20261112-1 add
    if (type == QStringLiteral("historytrend")) { json.insert(QStringLiteral("historytrend"), historyTrend.toJson()); } //20261112-1 add
    if (type == QStringLiteral("track")) { json.insert(QStringLiteral("moveTrack"), moveTrack.toJson()); } //20261112-0 add
    if (type == QStringLiteral("wheel")) { json.insert(QStringLiteral("wheel"), wheel.toJson()); } //20261112-0 add
    if (type == QStringLiteral("slider")) { json.insert(QStringLiteral("slider"), slider.toJson()); } //20261112-0 add
    if (type == QStringLiteral("flow")) { json.insert(QStringLiteral("flowBlock"), flowBlock.toJson()); } //20261112-0 add
    if (type == QStringLiteral("dropdown")) { json.insert(QStringLiteral("dropdown"), dropdown.toJson()); } //20261112-0 add
    if (type == QStringLiteral("multifunction")) { json.insert(QStringLiteral("multiFunctionButton"), multiFunctionButton.toJson()); } //20261112-0 add
    if (type == QStringLiteral("multistate")) { json.insert(QStringLiteral("multiStateButton"), multiStateButton.toJson()); } //20261112-0 add
    if (type == QStringLiteral("screen")) { json.insert(QStringLiteral("screenButton"), screenButton.toJson()); } //20261112-0 add
    if (type == QStringLiteral("time")) { json.insert(QStringLiteral("timeDisplay"), timeDisplay.toJson()); } //20261112-0 add
    if (type == QStringLiteral("ascii")) { json.insert(QStringLiteral("asciiDisplay"), asciiDisplay.toJson()); } //20261112-0 add
    if (type == QStringLiteral("image") && hasImageDisplayProperties) { json.insert(QStringLiteral("imageDisplay"), imageDisplay.toJson()); } //20261112-0 add
    return json;
}

/******************************************************************************
 * Function Name   : HmiObject::fromJson
 * Description     : Validate and restore one editable object from the prototype schema.
 * Input Parameters: json, output, error
 * Return Value    : True when the record is valid and was restored.
 * Date            : 2026.11.07
 ******************************************************************************/
bool HmiObject::fromJson(const QJsonObject &json, HmiObject *output, QString *error) //20261110-5 change
{
    HmiObject candidate;
    HmiObject *object;
    QJsonObject::const_iterator item;
    QString key;
    QJsonValue prototype;
    int x;
    int y;
    int width;
    int height;

    object = &candidate;
    if (output == nullptr)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("对象接收位置为空。");
        }
        return false;
    }

    if (!knownKeys(json, {QStringLiteral("id"), QStringLiteral("type"), QStringLiteral("name"),
        QStringLiteral("text"), QStringLiteral("address"), QStringLiteral("x"), QStringLiteral("y"),
        QStringLiteral("width"), QStringLiteral("height"), QStringLiteral("layer"), QStringLiteral("group"),
        QStringLiteral("staticText"), QStringLiteral("bitButton"), QStringLiteral("wordButton"),
        QStringLiteral("lamp"), QStringLiteral("functionButton"), QStringLiteral("messageDisplay"), QStringLiteral("scale"), QStringLiteral("qrCode"), QStringLiteral("imageDisplay"), QStringLiteral("gifDisplay"), QStringLiteral("numberDisplay"), QStringLiteral("asciiDisplay"), QStringLiteral("timeDisplay"), QStringLiteral("screenButton"), QStringLiteral("multiStateButton"), QStringLiteral("multiFunctionButton"), QStringLiteral("dropdown"), QStringLiteral("flowBlock"), QStringLiteral("slider"), QStringLiteral("wheel"), QStringLiteral("moveTrack"), QStringLiteral("chart"), QStringLiteral("trend"), QStringLiteral("historytrend"), QStringLiteral("extensions")}, QStringLiteral("object"), error)) { return false; } //20261112-0 change
    for (item = json.constBegin(); item != json.constEnd(); ++item)
    {
        key = item.key();
        if (key == QStringLiteral("x") || key == QStringLiteral("y") || key == QStringLiteral("width") || key == QStringLiteral("height"))
        {
            if (!exactInteger(item.value())) { return modelError(error, QStringLiteral("object.%1：几何必须为整数。").arg(key)); }
        }
        else if (key == QStringLiteral("layer"))
        {
            if (!item.value().isDouble() || !std::isfinite(item.value().toDouble())) { return modelError(error, QStringLiteral("控件层次无效。")); }
        }
        else if (key == QStringLiteral("extensions"))
        {
            if (!item.value().isObject() || !inactiveExtensions(item.value().toObject(), QStringLiteral("object.extensions"), error)) { return false; }
        }
        else if (key == QStringLiteral("chart")) //20261112-1 add
        {
            if (!item.value().isObject() || !ChartProperties::fromJson(item.value().toObject(), &object->chart, error)) { return false; }
        }
        else if (key == QStringLiteral("trend")) //20261112-1 add
        {
            if (!item.value().isObject() || !TrendProperties::fromJson(item.value().toObject(), &object->trend, error)) { return false; }
        }
        else if (key == QStringLiteral("historytrend")) //20261112-1 add
        {
            if (!item.value().isObject() || !HistoryTrendProperties::fromJson(item.value().toObject(), &object->historyTrend, error)) { return false; }
        }
        else if (key == QStringLiteral("moveTrack")) //20261112-0 add
        {
            if (!item.value().isObject() || !MoveTrackProperties::fromJson(item.value().toObject(), &object->moveTrack, error)) { return false; }
        }
        else if (key == QStringLiteral("wheel")) //20261112-0 add
        {
            if (!item.value().isObject() || !WheelProperties::fromJson(item.value().toObject(), &object->wheel, error)) { return false; }
        }
        else if (key == QStringLiteral("slider")) //20261112-0 add
        {
            if (!item.value().isObject() || !SliderProperties::fromJson(item.value().toObject(), &object->slider, error)) { return false; }
        }
        else if (key == QStringLiteral("flowBlock")) //20261112-0 add
        {
            if (!item.value().isObject() || !FlowBlockProperties::fromJson(item.value().toObject(), &object->flowBlock, error)) { return false; }
        }
        else if (key == QStringLiteral("dropdown")) //20261112-0 add
        {
            if (!item.value().isObject() || !DropdownProperties::fromJson(item.value().toObject(), &object->dropdown, error)) { return false; }
        }
        else if (key == QStringLiteral("multiFunctionButton")) //20261112-0 add
        {
            if (!item.value().isObject() || !MultiFunctionButtonProperties::fromJson(item.value().toObject(), &object->multiFunctionButton, error)) { return false; }
        }
        else if (key == QStringLiteral("multiStateButton")) //20261112-0 add
        {
            if (!item.value().isObject() || !MultiStateButtonProperties::fromJson(item.value().toObject(), &object->multiStateButton, error)) { return false; }
        }
        else if (key == QStringLiteral("screenButton")) //20261112-0 add
        {
            if (!item.value().isObject() || !ScreenButtonProperties::fromJson(item.value().toObject(), &object->screenButton, error)) { return false; }
        }
        else if (key == QStringLiteral("timeDisplay")) //20261112-0 add
        {
            if (!item.value().isObject() || !TimeDisplayProperties::fromJson(item.value().toObject(), &object->timeDisplay, error)) { return false; }
        }
        else if (key == QStringLiteral("asciiDisplay")) //20261112-0 add
        {
            if (!item.value().isObject() || !AsciiDisplayProperties::fromJson(item.value().toObject(), &object->asciiDisplay, error)) { return false; }
        }
        else if (key == QStringLiteral("numberDisplay")) //20261112-0 add
        {
            if (!item.value().isObject() || !NumberDisplayProperties::fromJson(item.value().toObject(), &object->numberDisplay, error)) { return false; }
        }
        else if (key == QStringLiteral("gifDisplay")) //20261112-0 add
        {
            if (!item.value().isObject() || !GifDisplayProperties::fromJson(item.value().toObject(), &object->gifDisplay, error)) { return false; }
        }
        else if (key == QStringLiteral("imageDisplay")) //20261112-0 add
        {
            if (!item.value().isObject() || !ImageDisplayProperties::fromJson(item.value().toObject(), &object->imageDisplay, error)) { return false; }
        }
        else if (key == QStringLiteral("qrCode")) //20261112-0 add
        {
            if (!item.value().isObject() || !QrCodeProperties::fromJson(item.value().toObject(), &object->qrCode, error)) { return false; }
        }
        else if (key == QStringLiteral("scale")) //20261112-0 add
        {
            if (!item.value().isObject() || !ScaleProperties::fromJson(item.value().toObject(), &object->scale, error)) { return false; }
        }
        else if (key == QStringLiteral("messageDisplay")) //20261112-0 add: Its 128-row language/state schema has its own strict parser.
        {
            if (!item.value().isObject() || !MessageDisplayProperties::fromJson(item.value().toObject(), &object->messageDisplay, error)) { return false; }
        }
        else if (key == QStringLiteral("staticText") || key == QStringLiteral("bitButton") || key == QStringLiteral("wordButton") ||
                 key == QStringLiteral("lamp") || key == QStringLiteral("functionButton"))
        {
            if (key == QStringLiteral("staticText")) { prototype = StaticTextProperties().toJson(); }
            if (key == QStringLiteral("bitButton")) { prototype = BitButtonProperties().toJson(); }
            if (key == QStringLiteral("wordButton")) { prototype = WordButtonProperties().toJson(); }
            if (key == QStringLiteral("lamp")) { prototype = LampProperties().toJson(); }
            if (key == QStringLiteral("functionButton")) { prototype = FunctionButtonProperties().toJson(); }
            if (!recordShape(item.value(), prototype, QStringLiteral("object.") + key, error)) { return false; }
        }
        else if (!item.value().isString()) { return modelError(error, QStringLiteral("object.%1：必须为字符串。").arg(key)); }
    }
    object->extensions = json.value(QStringLiteral("extensions")).toObject();
    object->id = json.value(QStringLiteral("id")).toString();
    object->type = json.value(QStringLiteral("type")).toString();
    if ((object->type == QStringLiteral("message")) != json.contains(QStringLiteral("messageDisplay"))) //20261112-0 add
    { return modelError(error, QStringLiteral("消息显示类型与专有属性不匹配。")); }
    if ((object->type == QStringLiteral("scale")) != json.contains(QStringLiteral("scale")))
    { return modelError(error, QStringLiteral("刻度类型与专有属性不匹配。")); }
    if ((object->type == QStringLiteral("qrcode")) != json.contains(QStringLiteral("qrCode"))) //20261112-0 add
    { return modelError(error, QStringLiteral("二维码类型与专有属性不匹配。")); }
    if ((object->type == QStringLiteral("gif")) != json.contains(QStringLiteral("gifDisplay"))) //20261112-0 add
    { return modelError(error, QStringLiteral("GIF类型与专有属性不匹配。")); }
    object->hasImageDisplayProperties = json.contains(QStringLiteral("imageDisplay")); //20261112-0 add
    object->hasNumberDisplayProperties = json.contains(QStringLiteral("numberDisplay")); //20261112-0 add
    if (object->hasNumberDisplayProperties && object->type != QStringLiteral("number")) { return modelError(error, QStringLiteral("数值类型与专有属性不匹配。")); }
    if ((object->type == QStringLiteral("chart")) != json.contains(QStringLiteral("chart"))) { return modelError(error, QStringLiteral("图表类型与专有属性不匹配。")); } //20261112-1 add
    if ((object->type == QStringLiteral("trend")) != json.contains(QStringLiteral("trend"))) { return modelError(error, QStringLiteral("趋势图类型与专有属性不匹配。")); } //20261112-1 add
    if ((object->type == QStringLiteral("historytrend")) != json.contains(QStringLiteral("historytrend"))) { return modelError(error, QStringLiteral("历史趋势图类型与专有属性不匹配。")); } //20261112-1 add
    if ((object->type == QStringLiteral("track")) != json.contains(QStringLiteral("moveTrack"))) { return modelError(error, QStringLiteral("移动轨迹类型与专有属性不匹配。")); } //20261112-0 add
    if ((object->type == QStringLiteral("wheel")) != json.contains(QStringLiteral("wheel"))) { return modelError(error, QStringLiteral("滚轮类型与专有属性不匹配。")); } //20261112-0 add
    if ((object->type == QStringLiteral("slider")) != json.contains(QStringLiteral("slider"))) { return modelError(error, QStringLiteral("滑动开关类型与专有属性不匹配。")); } //20261112-0 add
    if ((object->type == QStringLiteral("flow")) != json.contains(QStringLiteral("flowBlock"))) { return modelError(error, QStringLiteral("流动块类型与专有属性不匹配。")); } //20261112-0 add
    if ((object->type == QStringLiteral("dropdown")) != json.contains(QStringLiteral("dropdown"))) { return modelError(error, QStringLiteral("下拉框类型与专有属性不匹配。")); } //20261112-0 add
    if ((object->type == QStringLiteral("multifunction")) != json.contains(QStringLiteral("multiFunctionButton"))) { return modelError(error, QStringLiteral("多功能按钮类型与专有属性不匹配。")); } //20261112-0 add
    if ((object->type == QStringLiteral("multistate")) != json.contains(QStringLiteral("multiStateButton"))) { return modelError(error, QStringLiteral("多态按钮类型与专有属性不匹配。")); } //20261112-0 add
    if ((object->type == QStringLiteral("screen")) != json.contains(QStringLiteral("screenButton"))) { return modelError(error, QStringLiteral("画面按钮类型与专有属性不匹配。")); } //20261112-0 add
    if ((object->type == QStringLiteral("time")) != json.contains(QStringLiteral("timeDisplay")) || (object->type == QStringLiteral("time") && !json.value(QStringLiteral("address")).toString().isEmpty())) { return modelError(error, QStringLiteral("时间类型、专有属性或空RTC载体不匹配。")); } //20261112-0 add
    if ((object->type == QStringLiteral("ascii")) != json.contains(QStringLiteral("asciiDisplay"))) { return modelError(error, QStringLiteral("ASCII类型与专有属性不匹配。")); } //20261112-0 add
    if (object->hasImageDisplayProperties && object->type != QStringLiteral("image")) { return modelError(error, QStringLiteral("图片显示类型与专有属性不匹配。")); }
    object->name = json.value(QStringLiteral("name")).toString();
    object->text = json.value(QStringLiteral("text")).toString();
    object->address = json.value(QStringLiteral("address")).toString();
    object->layer = json.value(QStringLiteral("layer")).toDouble(1); //20261108-14 add
    object->group = json.value(QStringLiteral("group")).toString();
    object->hasStaticTextProperties = json.contains(QStringLiteral("staticText"));
    object->hasBitButtonProperties = json.contains(QStringLiteral("bitButton")); //20261108-15 add
    if (!object->hasBitButtonProperties) { object->bitButton = BitButtonProperties(); }
    if (object->hasBitButtonProperties &&
        !BitButtonProperties::fromJson(json.value(QStringLiteral("bitButton")).toObject(), &object->bitButton, error,
                                      json.contains(QStringLiteral("functionButton")))) //20261110-0 change
    {
        return false;
    }
    object->hasWordButtonProperties = json.contains(QStringLiteral("wordButton")); //20261109-6 add
    object->wordButton = WordButtonProperties();
    if (object->hasWordButtonProperties &&
        (!json.value(QStringLiteral("wordButton")).isObject() ||
         !WordButtonProperties::fromJson(json.value(QStringLiteral("wordButton")).toObject(), &object->wordButton, error)))
    {
        return false;
    }
    object->hasLampProperties = json.contains(QStringLiteral("lamp")); //20261109-8 add
    object->lamp = LampProperties();
    if (object->hasLampProperties &&
        (!json.value(QStringLiteral("lamp")).isObject() ||
         !LampProperties::fromJson(json.value(QStringLiteral("lamp")).toObject(), &object->lamp, error)))
    {
        return false;
    }
    object->hasFunctionButtonProperties = json.contains(QStringLiteral("functionButton")); //20261110-0 add
    object->functionButton = FunctionButtonProperties();
    if (object->hasFunctionButtonProperties &&
        (!json.value(QStringLiteral("functionButton")).isObject() ||
         !FunctionButtonProperties::fromJson(json.value(QStringLiteral("functionButton")).toObject(), &object->functionButton, error)))
    {
        return false;
    }
    if (!object->hasStaticTextProperties) { object->staticText = StaticTextProperties(); } //20261108-14 add
    if (object->hasStaticTextProperties &&
        !StaticTextProperties::fromJson(json.value(QStringLiteral("staticText")).toObject(),
                                         &object->staticText, error))
    {
        return false;
    }
    x = json.value(QStringLiteral("x")).toInt();
    y = json.value(QStringLiteral("y")).toInt();
    width = json.value(QStringLiteral("width")).toInt();
    height = json.value(QStringLiteral("height")).toInt();

    if (object->id.isEmpty() || object->name.isEmpty() || width <= 0 || height <= 0 || !std::isfinite(object->layer) ||
        (object->hasStaticTextProperties && object->type != QStringLiteral("text")) ||
        (object->hasBitButtonProperties && object->type != QStringLiteral("switch")) || //20261108-15 add
        (object->hasWordButtonProperties && (object->type != QStringLiteral("switch") ||
         !object->hasBitButtonProperties || object->bitButton.indicator || object->bitButton.dualMonitor)) || //20261109-6 add
        (object->hasLampProperties && (object->type != QStringLiteral("switch") || !object->hasBitButtonProperties ||
         object->hasWordButtonProperties || object->bitButton.indicator != (object->lamp.monitorMode != 0) ||
         object->bitButton.dualMonitor != (object->lamp.monitorMode == 1) ||
         object->bitButton.touch.visibilityEnabled || object->bitButton.pressTimeEnabled ||
         object->bitButton.macroEnabled || object->bitButton.notice || object->lamp.monitorMode == 0 ||
         object->bitButton.monitorSame || object->lamp.multiMonitorSame)) || //20261109-8 add
        (object->hasFunctionButtonProperties && (object->type != QStringLiteral("switch") || //20261110-0 add
         !object->hasBitButtonProperties || object->hasWordButtonProperties || object->hasLampProperties ||
         object->bitButton.indicator || object->bitButton.dualMonitor || object->bitButton.macroEnabled ||
         !object->address.isEmpty() || (object->bitButton.noticeBit && object->bitButton.noticeWord) ||
         (object->functionButton.pressedEffect && object->bitButton.states[0].sameLanguage != object->bitButton.states[1].sameLanguage) ||
         object->bitButton.pressTime > 255 || object->bitButton.states[0].text.fontSize > 128)) ||
        (object->type != QStringLiteral("switch") &&
         object->type != QStringLiteral("number") &&
         object->type != QStringLiteral("chart") && //20261112-1 add
         object->type != QStringLiteral("trend") && //20261112-1 add
         object->type != QStringLiteral("historytrend") && //20261112-1 add
         object->type != QStringLiteral("track") && //20261112-0 add
         object->type != QStringLiteral("wheel") && //20261112-0 add
         object->type != QStringLiteral("slider") && //20261112-0 add
         object->type != QStringLiteral("flow") && //20261112-0 add
         object->type != QStringLiteral("dropdown") && //20261112-0 add
         object->type != QStringLiteral("multifunction") && //20261112-0 add
         object->type != QStringLiteral("multistate") && //20261112-0 add
         object->type != QStringLiteral("screen") && //20261112-0 add
         object->type != QStringLiteral("time") && //20261112-0 add
         object->type != QStringLiteral("ascii") && //20261112-0 add
         object->type != QStringLiteral("image") &&
         object->type != QStringLiteral("message") && //20261112-0 add
         object->type != QStringLiteral("scale") && //20261112-0 add
         object->type != QStringLiteral("gif") && //20261112-0 add
         object->type != QStringLiteral("qrcode") && //20261112-0 add
         object->type != QStringLiteral("text")))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("工程包含无效的控件记录。");
        }
        return false;
    }

    if (qint64(x) + width > std::numeric_limits<int>::max() || qint64(y) + height > std::numeric_limits<int>::max())
    {
        return modelError(error, QStringLiteral("控件几何整数溢出。"));
    }
    object->geometry = QRect(x, y, width, height);
    *output = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : HmiProject::HmiProject
 * Description     : Initialize an 800 by 480 prototype page for the verified target model.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
HmiProject::HmiProject() //20261107-6 add
    : name(QStringLiteral("新建工程")),
      targetModel(QStringLiteral("AMT070W124-L800X480R")),
      pageName(QStringLiteral("画面1")),
      pageWidth(800),
      pageHeight(480)
{
}

/******************************************************************************
 * Function Name   : HmiPage::toJson
 * Description     : Serialize a complete independent page value in schema9.
 * Input Parameters: None
 * Return Value    : Page identity, kind, geometry and ordered objects.
 * Date            : 2026.11.10
 ******************************************************************************/
QJsonObject HmiPage::toJson() const //20261110-5 add
{
    QJsonObject json;
    QJsonArray items;
    HmiObject object;

    Q_FOREACH (object, objects) { items.append(object.toJson()); }
    json.insert(QStringLiteral("id"), id); json.insert(QStringLiteral("kind"), kind);
    json.insert(QStringLiteral("name"), name); json.insert(QStringLiteral("width"), width);
    json.insert(QStringLiteral("height"), height); json.insert(QStringLiteral("objects"), items);
    if (!extensions.isEmpty()) { json.insert(QStringLiteral("extensions"), extensions); }
    return json;
}

/******************************************************************************
 * Function Name   : HmiPage::fromJson
 * Description     : Parse one bounded K-layer page without publishing partially parsed objects.
 * Input Parameters: json, page, error
 * Return Value    : True after all fields and child records pass validation.
 * Date            : 2026.11.10
 ******************************************************************************/
bool HmiPage::fromJson(const QJsonObject &json, HmiPage *page, QString *error) //20261110-5 add
{
    HmiPage candidate;
    HmiObject object;
    QJsonArray items;
    QJsonValue item;
    QString key;
    QSet<QString> ids;

    if (page == nullptr || !knownKeys(json, {QStringLiteral("id"), QStringLiteral("kind"), QStringLiteral("name"),
        QStringLiteral("width"), QStringLiteral("height"), QStringLiteral("objects"), QStringLiteral("extensions")}, QStringLiteral("page"), error)) { return false; }
    Q_FOREACH (key, (QStringList{QStringLiteral("id"), QStringLiteral("kind"), QStringLiteral("name")}))
    {
        if (!json.value(key).isString() || json.value(key).toString().trimmed().isEmpty())
        {
            return modelError(error, QStringLiteral("page.%1：必须为非空字符串。").arg(key));
        }
    }
    if (!exactInteger(json.value(QStringLiteral("width"))) || !exactInteger(json.value(QStringLiteral("height"))) ||
        !json.value(QStringLiteral("objects")).isArray() ||
        (json.contains(QStringLiteral("extensions")) && !json.value(QStringLiteral("extensions")).isObject()))
    {
        return modelError(error, QStringLiteral("页面尺寸、对象列表或扩展类型无效。"));
    }
    candidate.id = json.value(QStringLiteral("id")).toString(); candidate.kind = json.value(QStringLiteral("kind")).toString();
    candidate.name = json.value(QStringLiteral("name")).toString(); candidate.width = json.value(QStringLiteral("width")).toInt();
    candidate.height = json.value(QStringLiteral("height")).toInt(); candidate.extensions = json.value(QStringLiteral("extensions")).toObject();
    if (!QStringList{QStringLiteral("normal"), QStringLiteral("window"), QStringLiteral("keyboard"), QStringLiteral("logo")}.contains(candidate.kind) ||
        candidate.width < 1 || candidate.height < 1 || candidate.width > 800 || candidate.height > 480 ||
        !inactiveExtensions(candidate.extensions, QStringLiteral("page.extensions"), error))
    {
        return modelError(error, QStringLiteral("页面类型或尺寸超出当前AMT编辑范围。"));
    }
    items = json.value(QStringLiteral("objects")).toArray();
    if (items.size() > 100) { return modelError(error, QStringLiteral("页面超过Lite每页100控件保护上限。")); }
    Q_FOREACH (item, items)
    {
        if (!item.isObject() || !HmiObject::fromJson(item.toObject(), &object, error)) { return false; }
        if (ids.contains(object.id) || object.geometry.x() < 0 || object.geometry.y() < 0 ||
            qint64(object.geometry.x()) + object.geometry.width() > candidate.width ||
            qint64(object.geometry.y()) + object.geometry.height() > candidate.height)
        {
            return modelError(error, QStringLiteral("页面%1：控件%2标识重复或位置越界。").arg(candidate.id, object.id));
        }
        ids.insert(object.id); candidate.objects.append(object);
    }
    *page = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : HmiConfigurationNode::toJson
 * Description     : Preserve an explicitly unimplemented inactive configuration entity.
 * Input Parameters: None
 * Return Value    : Versioned editor configuration; no inferred native default values.
 * Date            : 2026.11.10
 ******************************************************************************/
QJsonObject HmiConfigurationNode::toJson() const //20261110-5 add
{
    QJsonObject json;

    json.insert(QStringLiteral("id"), id); json.insert(QStringLiteral("kind"), kind); json.insert(QStringLiteral("name"), name);
    json.insert(QStringLiteral("ownerPageId"), ownerPageId); json.insert(QStringLiteral("implemented"), implemented);
    json.insert(QStringLiteral("active"), active); json.insert(QStringLiteral("properties"), properties);
    if (!extensions.isEmpty()) { json.insert(QStringLiteral("extensions"), extensions); }
    return json;
}

/******************************************************************************
 * Function Name   : HmiConfigurationNode::fromJson
 * Description     : Reject unknown kinds and activation of configurations whose semantics are unrecovered.
 * Input Parameters: json, node, error
 * Return Value    : True for an inert K entity only.
 * Date            : 2026.11.10
 ******************************************************************************/
bool HmiConfigurationNode::fromJson(const QJsonObject &json, HmiConfigurationNode *node, QString *error) //20261110-5 add
{
    HmiConfigurationNode candidate;
    QString key;

    if (node == nullptr || !knownKeys(json, {QStringLiteral("id"), QStringLiteral("kind"), QStringLiteral("name"),
        QStringLiteral("ownerPageId"), QStringLiteral("implemented"), QStringLiteral("active"),
        QStringLiteral("properties"), QStringLiteral("extensions")}, QStringLiteral("configurationNode"), error)) { return false; }
    Q_FOREACH (key, (QStringList{QStringLiteral("id"), QStringLiteral("kind"), QStringLiteral("name"), QStringLiteral("ownerPageId")}))
    {
        if (!json.value(key).isString()) { return modelError(error, QStringLiteral("配置节点%1类型无效。").arg(key)); }
    }
    if (!json.value(QStringLiteral("implemented")).isBool() || !json.value(QStringLiteral("active")).isBool() ||
        !json.value(QStringLiteral("properties")).isObject() ||
        (json.contains(QStringLiteral("extensions")) && !json.value(QStringLiteral("extensions")).isObject()))
    {
        return modelError(error, QStringLiteral("配置节点状态或元数据类型无效。"));
    }
    candidate.id = json.value(QStringLiteral("id")).toString(); candidate.name = json.value(QStringLiteral("name")).toString();
    candidate.kind = json.value(QStringLiteral("kind")).toString(); candidate.ownerPageId = json.value(QStringLiteral("ownerPageId")).toString();
    candidate.implemented = json.value(QStringLiteral("implemented")).toBool(); candidate.active = json.value(QStringLiteral("active")).toBool();
    candidate.properties = json.value(QStringLiteral("properties")).toObject(); candidate.extensions = json.value(QStringLiteral("extensions")).toObject();
    if (candidate.kind == QStringLiteral("dataCollection") && candidate.implemented && candidate.active && candidate.ownerPageId.isEmpty()) //20261112-1 add
    {
        DataCollectionProperties properties;
        if (candidate.id.trimmed().isEmpty() || candidate.name.trimmed().isEmpty() || candidate.name.size() > 48 || !DataCollectionProperties::fromJson(candidate.properties, &properties, error) || !inactiveExtensions(candidate.extensions, QStringLiteral("configurationNode.extensions"), error)) { return false; }
        *node = candidate; return true;
    }
    if (candidate.kind == QStringLiteral("system") && candidate.implemented && candidate.active && candidate.ownerPageId.isEmpty()) { //20261110-7 add
        if (candidate.id.trimmed().isEmpty() || candidate.name.trimmed().isEmpty() ||
            !validateStage2SystemSettingsProperties(candidate.properties, error) ||
            !inactiveExtensions(candidate.extensions, QStringLiteral("configurationNode.extensions"), error)) { return false; }
        *node = candidate;
        return true;
    }
    if (candidate.id.trimmed().isEmpty() || candidate.name.trimmed().isEmpty() ||
        !QStringList{QStringLiteral("system"), QStringLiteral("project"), QStringLiteral("dataCollection"),
        QStringLiteral("alarm"), QStringLiteral("recipe"), QStringLiteral("macro"), QStringLiteral("dataTransfer")}.contains(candidate.kind) ||
        candidate.implemented || candidate.active || !candidate.properties.isEmpty() ||
        !inactiveExtensions(candidate.extensions, QStringLiteral("configurationNode.extensions"), error))
    {
        return modelError(error, QStringLiteral("配置节点%1：仅支持已声明类别的未实施空节点；未知活动配置拒绝保存或下载。").arg(candidate.id));
    }
    *node = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : HmiReference::toJson
 * Description     : Store stable entity references independently of display order and names.
 * Input Parameters: None
 * Return Value    : Explicit owner, field and target identities.
 * Date            : 2026.11.10
 ******************************************************************************/
QJsonObject HmiReference::toJson() const //20261110-5 add
{
    return {{QStringLiteral("id"), id}, {QStringLiteral("ownerKind"), ownerKind}, {QStringLiteral("ownerId"), ownerId},
            {QStringLiteral("field"), field}, {QStringLiteral("targetKind"), targetKind}, {QStringLiteral("targetId"), targetId}};
}

/******************************************************************************
 * Function Name   : HmiReference::fromJson
 * Description     : Validate reference syntax before project-level resolution.
 * Input Parameters: json, reference, error
 * Return Value    : True without changing the output on failure.
 * Date            : 2026.11.10
 ******************************************************************************/
bool HmiReference::fromJson(const QJsonObject &json, HmiReference *reference, QString *error) //20261110-5 add
{
    HmiReference candidate;
    QString key;
    QStringList keys;

    keys = {QStringLiteral("id"), QStringLiteral("ownerKind"), QStringLiteral("ownerId"), QStringLiteral("field"),
            QStringLiteral("targetKind"), QStringLiteral("targetId")};
    if (reference == nullptr || !knownKeys(json, keys, QStringLiteral("reference"), error)) { return false; }
    Q_FOREACH (key, keys)
    {
        if (!json.value(key).isString() || json.value(key).toString().trimmed().isEmpty())
        {
            return modelError(error, QStringLiteral("引用%1必须为非空字符串。").arg(key));
        }
    }
    candidate.id = json.value(QStringLiteral("id")).toString(); candidate.ownerKind = json.value(QStringLiteral("ownerKind")).toString();
    candidate.ownerId = json.value(QStringLiteral("ownerId")).toString(); candidate.field = json.value(QStringLiteral("field")).toString();
    candidate.targetKind = json.value(QStringLiteral("targetKind")).toString(); candidate.targetId = json.value(QStringLiteral("targetId")).toString();
    if (!QStringList{QStringLiteral("project"), QStringLiteral("page"), QStringLiteral("object"), QStringLiteral("configuration")}.contains(candidate.ownerKind) ||
        !QStringList{QStringLiteral("page"), QStringLiteral("object"), QStringLiteral("configuration"), QStringLiteral("picture"),
        QStringLiteral("macro"), QStringLiteral("animation"), QStringLiteral("connection")}.contains(candidate.targetKind))
    {
        return modelError(error, QStringLiteral("引用%1的所有者或目标类别未实现。").arg(candidate.id));
    }
    *reference = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : HmiProject::toJson
 * Description     : Serialize all editor pages using the lowest schema that preserves their semantics.
 * Input Parameters: None
 * Return Value    : Independent .ksp JSON; schema9 explicitly stores topology and K-only entities.
 * Date            : 2026.11.10
 ******************************************************************************/
QJsonObject HmiProject::toJson() const //20261110-5 change
{
    QJsonObject json;
    QJsonObject resolution;
    QJsonObject pageJson;
    QJsonObject macrosJson;
    QJsonObject picturesJson;
    QJsonObject animationJson; //20261112-0 add
    QJsonObject macroIdsJson;
    QJsonArray pagesJson;
    QJsonArray objectsJson;
    QJsonArray connectionsJson;
    QJsonArray nodesJson;
    QJsonArray referencesJson;
    HmiPage page;
    HmiObject object;
    PlcConnection connection;
    HmiConfigurationNode node;
    HmiReference reference;
    QMap<QString, QString>::const_iterator macro;
    QMap<QString, QByteArray>::const_iterator picture;
    int schema;
    bool stage1;

    schema = 1;
    stage1 = hasStage1Data(*this);
    if (languages != QStringList{QStringLiteral("中文")} || activeLanguage != QStringLiteral("中文")) { schema = 2; }
    Q_FOREACH (page, allPages())
    {
        Q_FOREACH (object, page.objects)
        {
            if (object.hasStaticTextProperties || object.layer != 1 || !object.group.isEmpty()) { schema = qMax(schema, 2); }
            if (object.hasBitButtonProperties) { schema = qMax(schema, 3); }
            if (object.hasWordButtonProperties) { schema = qMax(schema, 6); }
            if (object.hasLampProperties) { schema = qMax(schema, 7); }
            if (object.hasFunctionButtonProperties) { schema = qMax(schema, 8); }
        }
    }
    if (!macros.isEmpty() || !pictures.isEmpty()) { schema = qMax(schema, 3); }
    if (!connections.isEmpty()) { schema = qMax(schema, 4); }
    if (!nativeSourceSnapshot.isEmpty() || !nativeSourceWarnings.isEmpty()) { schema = qMax(schema, 5); }
    if (stage1) { schema = 9; }
    //20261112-0 add: Schema10 is reserved for stage-3 editor flags; message data starts at schema11.
    Q_FOREACH (page, allPages())
    {
        Q_FOREACH (object, page.objects) { if (object.type == QStringLiteral("message") || object.type == QStringLiteral("scale") || object.type == QStringLiteral("qrcode") || object.hasImageDisplayProperties) { schema = 11; stage1 = true; } }
    }
    Q_FOREACH (node, configurationNodes) { if (node.kind == QStringLiteral("dataCollection") && node.implemented && node.active) { schema = 12; stage1 = true; } } //20261112-1 add: Collector-only files must emit every required schema12 container.
    if (!animations.isEmpty()) { schema = 12; stage1 = true; } //20261112-0 add
    Q_FOREACH (page, allPages())
    { Q_FOREACH (object, page.objects) { if (object.type == QStringLiteral("chart") || object.type == QStringLiteral("trend") || object.type == QStringLiteral("historytrend") || object.type == QStringLiteral("track") || object.type == QStringLiteral("wheel") || object.type == QStringLiteral("slider") || object.type == QStringLiteral("flow") || object.type == QStringLiteral("dropdown") || object.type == QStringLiteral("multifunction") || object.type == QStringLiteral("multistate") || object.type == QStringLiteral("screen") || object.type == QStringLiteral("time") || object.type == QStringLiteral("gif") || object.type == QStringLiteral("ascii") || object.hasNumberDisplayProperties) { schema = 12; stage1 = true; } } } //20261112-0 change: Clock-only documents must declare their typed schema before saving or restoring history.
    if (schema >= 12)
    {
        for (picture = animations.constBegin(); picture != animations.constEnd(); ++picture) { animationJson.insert(picture.key(), QString::fromLatin1(picture.value().toBase64())); }
        json.insert(QStringLiteral("animations"), animationJson);
    }
    resolution = {{QStringLiteral("width"), pageWidth}, {QStringLiteral("height"), pageHeight}};
    if (stage1)
    {
        Q_FOREACH (page, allPages()) { pagesJson.append(page.toJson()); }
        json.insert(QStringLiteral("projectId"), projectId);
        json.insert(QStringLiteral("activePageId"), activePageId);
        json.insert(QStringLiteral("startupPageId"), startupPageId);
        json.insert(QStringLiteral("startupLanguage"), startupLanguage);
        json.insert(QStringLiteral("pageOrder"), QJsonArray::fromStringList(pageOrder));
        Q_FOREACH (node, configurationNodes) { nodesJson.append(node.toJson()); }
        Q_FOREACH (reference, references) { referencesJson.append(reference.toJson()); }
        json.insert(QStringLiteral("configurationNodes"), nodesJson);
        json.insert(QStringLiteral("references"), referencesJson);
        for (macro = macroIds.constBegin(); macro != macroIds.constEnd(); ++macro) { macroIdsJson.insert(macro.key(), macro.value()); }
        json.insert(QStringLiteral("macroIds"), macroIdsJson);
        if (!extensions.isEmpty()) { json.insert(QStringLiteral("extensions"), extensions); }
    }
    else
    {
        Q_FOREACH (object, objects) { objectsJson.append(object.toJson()); }
        pageJson = {{QStringLiteral("name"), pageName}, {QStringLiteral("objects"), objectsJson}};
        pagesJson.append(pageJson);
    }
    if (schema >= 3)
    {
        for (macro = macros.constBegin(); macro != macros.constEnd(); ++macro) { macrosJson.insert(macro.key(), macro.value()); }
        for (picture = pictures.constBegin(); picture != pictures.constEnd(); ++picture)
        {
            picturesJson.insert(picture.key(), QString::fromLatin1(picture.value().toBase64()));
        }
        json.insert(QStringLiteral("macros"), macrosJson); json.insert(QStringLiteral("pictures"), picturesJson);
    }
    if (!connections.isEmpty())
    {
        Q_FOREACH (connection, connections) { connectionsJson.append(connection.toJson()); }
        json.insert(QStringLiteral("connections"), connectionsJson);
    }
    if (!nativeSourceSnapshot.isEmpty() || !nativeSourceWarnings.isEmpty())
    {
        json.insert(QStringLiteral("nativeSourceSnapshot"), QString::fromLatin1(nativeSourceSnapshot.toBase64()));
        json.insert(QStringLiteral("nativeSourceWarnings"), QJsonArray::fromStringList(nativeSourceWarnings));
    }
    json.insert(QStringLiteral("format"), QStringLiteral("KinSealStudio-Prototype"));
    json.insert(QStringLiteral("schemaVersion"), schema);
    json.insert(QStringLiteral("projectName"), name); json.insert(QStringLiteral("targetModel"), targetModel);
    json.insert(QStringLiteral("resolution"), resolution); json.insert(QStringLiteral("pages"), pagesJson);
    json.insert(QStringLiteral("languages"), QJsonArray::fromStringList(languages)); json.insert(QStringLiteral("activeLanguage"), activeLanguage);
    return json;
}

/******************************************************************************
 * Function Name   : HmiProject::fromJson
 * Description     : Migrate legacy single-page files and validate schema9 before replacing project data.
 * Input Parameters: json, project, error
 * Return Value    : True only after every page, resource and reference has been restored atomically.
 * Date            : 2026.11.10
 ******************************************************************************/
bool HmiProject::fromJson(const QJsonObject &json, HmiProject *project, QString *error) //20261110-5 change
{
    HmiProject loaded;
    HmiPage page;
    HmiConfigurationNode node;
    HmiReference reference;
    PlcConnection connection;
    QJsonObject resolution;
    QJsonObject resources;
    QJsonObject pageJson;
    QJsonObject::const_iterator item;
    QJsonArray pages;
    QJsonValue value;
    QSet<QString> parsedPageIds;
    QByteArray encoded;
    QByteArray decoded;
    QString key;
    QString text;
    QStringList allowed;
    int schema;
    int index;
    qint64 resourceBytes;
    bool activeFound;

    if (project == nullptr || json.value(QStringLiteral("format")).toString() != QStringLiteral("KinSealStudio-Prototype"))
    {
        return modelError(error, QStringLiteral("文件不是 KinSealStudio 原型工程格式（.ksp）。"));
    }
    if (!exactInteger(json.value(QStringLiteral("schemaVersion")))) { return modelError(error, QStringLiteral("工程版本必须为整数。")); }
    schema = json.value(QStringLiteral("schemaVersion")).toInt();
    if (schema < 1 || schema > 12) { return modelError(error, QStringLiteral("工程版本不受支持。")); } //20261112-0 change
    allowed = {QStringLiteral("format"), QStringLiteral("schemaVersion"), QStringLiteral("projectName"), QStringLiteral("targetModel"),
        QStringLiteral("resolution"), QStringLiteral("pages"), QStringLiteral("languages"), QStringLiteral("activeLanguage"),
        QStringLiteral("macros"), QStringLiteral("pictures"), QStringLiteral("connections"), QStringLiteral("nativeSourceSnapshot"),
        QStringLiteral("nativeSourceWarnings")};
    if (schema >= 9) //20261112-0 change
    {
        allowed.append({QStringLiteral("projectId"), QStringLiteral("activePageId"), QStringLiteral("startupPageId"), QStringLiteral("startupLanguage"),
            QStringLiteral("pageOrder"), QStringLiteral("configurationNodes"), QStringLiteral("references"), QStringLiteral("macroIds"), QStringLiteral("extensions")});
    }
    if (schema >= 12) { allowed.append(QStringLiteral("animations")); } //20261112-0 add
    if (!knownKeys(json, allowed, QStringLiteral("project"), error)) { return false; }
    if ((schema < 3 && (json.contains(QStringLiteral("macros")) || json.contains(QStringLiteral("pictures")))) ||
        (schema < 4 && json.contains(QStringLiteral("connections"))) ||
        (schema < 5 && (json.contains(QStringLiteral("nativeSourceSnapshot")) || json.contains(QStringLiteral("nativeSourceWarnings")))))
    {
        return modelError(error, QStringLiteral("工程能力与schema版本不匹配，不允许降级标记高版本字段。"));
    }
    Q_FOREACH (key, (QStringList{QStringLiteral("projectName"), QStringLiteral("targetModel")}))
    {
        if (!json.value(key).isString() || json.value(key).toString().trimmed().isEmpty())
        {
            return modelError(error, QStringLiteral("project.%1：必须为非空字符串。").arg(key));
        }
    }
    loaded.name = json.value(QStringLiteral("projectName")).toString(); loaded.targetModel = json.value(QStringLiteral("targetModel")).toString();
    resolution = json.value(QStringLiteral("resolution")).toObject();
    if (!json.value(QStringLiteral("resolution")).isObject() ||
        !knownKeys(resolution, {QStringLiteral("width"), QStringLiteral("height")}, QStringLiteral("resolution"), error) ||
        !exactInteger(resolution.value(QStringLiteral("width"))) || !exactInteger(resolution.value(QStringLiteral("height"))) ||
        !json.value(QStringLiteral("pages")).isArray())
    {
        return modelError(error, QStringLiteral("工程分辨率或页面列表类型无效。"));
    }
    if (json.contains(QStringLiteral("languages")))
    {
        if (!json.value(QStringLiteral("languages")).isArray()) { return modelError(error, QStringLiteral("工程语言列表类型无效。")); }
        loaded.languages.clear();
        Q_FOREACH (value, json.value(QStringLiteral("languages")).toArray())
        {
            if (!value.isString()) { return modelError(error, QStringLiteral("工程语言必须为字符串。")); }
            loaded.languages.append(value.toString());
        }
    }
    if (json.contains(QStringLiteral("activeLanguage")))
    {
        if (!json.value(QStringLiteral("activeLanguage")).isString()) { return modelError(error, QStringLiteral("编辑语言类型无效。")); }
        loaded.activeLanguage = json.value(QStringLiteral("activeLanguage")).toString();
    }
    if (json.contains(QStringLiteral("nativeSourceSnapshot")))
    {
        text = json.value(QStringLiteral("nativeSourceSnapshot")).toString(); encoded = text.toLatin1();
        if (!json.value(QStringLiteral("nativeSourceSnapshot")).isString() || encoded.size() > ((16 * 1024 * 1024 + 2) / 3) * 4 ||
            QString::fromLatin1(encoded) != text) { return modelError(error, QStringLiteral("原厂快照类型无效或超过16 MiB保护上限。")); }
        decoded = QByteArray::fromBase64(encoded);
        if (decoded.size() > 16 * 1024 * 1024 || decoded.toBase64() != encoded)
        {
            return modelError(error, QStringLiteral("原厂工程元数据快照Base64无效或超过16 MiB。"));
        }
        loaded.nativeSourceSnapshot = decoded;
    }
    if (json.contains(QStringLiteral("nativeSourceWarnings")))
    {
        if (!json.value(QStringLiteral("nativeSourceWarnings")).isArray() ||
            json.value(QStringLiteral("nativeSourceWarnings")).toArray().size() > 128)
        {
            return modelError(error, QStringLiteral("原厂导入警告列表无效。"));
        }
        Q_FOREACH (value, json.value(QStringLiteral("nativeSourceWarnings")).toArray())
        {
            if (!value.isString() || value.toString().size() > 4096) { return modelError(error, QStringLiteral("原厂导入警告记录无效。")); }
            loaded.nativeSourceWarnings.append(value.toString());
        }
    }
    Q_FOREACH (key, (QStringList{QStringLiteral("macros"), QStringLiteral("pictures"), QStringLiteral("macroIds"), QStringLiteral("extensions")}))
    {
        if (schema <= 8 && key == QStringLiteral("macros") && json.value(key).isArray() &&
            json.value(key).toArray().isEmpty()) //20261110-5 add: Only the released stage0 helper's lossless empty-map alias migrates from [] to {}.
        {
            continue;
        }
        if (json.contains(key) && !json.value(key).isObject()) { return modelError(error, QStringLiteral("project.%1：必须为对象。").arg(key)); }
    }
    resources = json.value(QStringLiteral("macros")).toObject();
    for (item = resources.constBegin(); item != resources.constEnd(); ++item)
    {
        if (!item.value().isString()) { return modelError(error, QStringLiteral("工程宏源码类型无效。")); }
        loaded.macros.insert(item.key(), item.value().toString());
    }
    resources = json.value(QStringLiteral("pictures")).toObject(); resourceBytes = 0;
    if (resources.size() > 256) { return modelError(error, QStringLiteral("工程图库超过256项Lite保护上限。")); }
    for (item = resources.constBegin(); item != resources.constEnd(); ++item)
    {
        text = item.value().toString(); encoded = text.toLatin1();
        if (!item.value().isString() || item.key().trimmed().isEmpty() || encoded.size() > ((16 * 1024 * 1024 + 2) / 3) * 4 ||
            QString::fromLatin1(encoded) != text) { return modelError(error, QStringLiteral("工程图库记录类型或大小无效。")); }
        decoded = QByteArray::fromBase64(encoded); resourceBytes += decoded.size();
        if (decoded.toBase64() != encoded || !validateButtonImage(decoded) || resourceBytes > 64 * 1024 * 1024)
        {
            return modelError(error, QStringLiteral("工程图库Base64、图片内容或总大小无效。"));
        }
        loaded.pictures.insert(item.key(), decoded);
    }
    if (schema >= 12) //20261112-0 add: Validate raw animation encoding before publishing the loaded project.
    {
        if (!json.value(QStringLiteral("animations")).isObject()) { return modelError(error, QStringLiteral("动画资源池缺失或类型错误。")); }
        resources = json.value(QStringLiteral("animations")).toObject(); resourceBytes = 0;
        if (resources.size() > 256) { return modelError(error, QStringLiteral("动画资源超过256项Lite保护上限。")); }
        for (item = resources.constBegin(); item != resources.constEnd(); ++item)
        {
            text = item.value().toString(); encoded = text.toLatin1();
            if (!item.value().isString() || encoded.size() > ((1024 * 1024 + 2) / 3) * 4 || QString::fromLatin1(encoded) != text)
            { return modelError(error, QStringLiteral("动画编码字段无效或超限。")); }
            decoded = QByteArray::fromBase64(encoded); resourceBytes += decoded.size();
            if (decoded.toBase64() != encoded || resourceBytes > 32 * 1024 * 1024 || !validateGifData(decoded, error)) { return false; }
            loaded.animations.insert(item.key(), decoded);
        }
    }
    if (json.contains(QStringLiteral("connections")))
    {
        if (!json.value(QStringLiteral("connections")).isArray() || json.value(QStringLiteral("connections")).toArray().size() > 2)
        {
            return modelError(error, QStringLiteral("连接配置类型或数量无效。"));
        }
        Q_FOREACH (value, json.value(QStringLiteral("connections")).toArray())
        {
            if (!value.isObject() || !knownKeys(value.toObject(), PlcConnection().toJson().keys(), QStringLiteral("connection"), error) ||
                !PlcConnection::fromJson(value.toObject(), &connection, error)) { return false; }
            loaded.connections.append(connection);
        }
    }
    pages = json.value(QStringLiteral("pages")).toArray();
    if (pages.isEmpty() || pages.size() > 200 || (schema < 9 && pages.size() != 1))
    {
        return modelError(error, QStringLiteral("页面数量无效；旧schema只支持一个画面，多页需版本9。"));
    }
    if (schema >= 9) //20261112-0 change
    {
        Q_FOREACH (key, (QStringList{QStringLiteral("projectId"), QStringLiteral("activePageId"), QStringLiteral("startupPageId"), QStringLiteral("startupLanguage")}))
        {
            if (!json.value(key).isString()) { return modelError(error, QStringLiteral("project.%1：必须为字符串。").arg(key)); }
        }
        loaded.projectId = json.value(QStringLiteral("projectId")).toString(); loaded.activePageId = json.value(QStringLiteral("activePageId")).toString();
        loaded.startupPageId = json.value(QStringLiteral("startupPageId")).toString(); loaded.startupLanguage = json.value(QStringLiteral("startupLanguage")).toString();
        if (!json.value(QStringLiteral("pageOrder")).isArray() || !json.value(QStringLiteral("configurationNodes")).isArray() ||
            !json.value(QStringLiteral("references")).isArray()) { return modelError(error, QStringLiteral("页面顺序、配置节点或引用列表类型无效。")); }
        loaded.pageOrder.clear();
        Q_FOREACH (value, json.value(QStringLiteral("pageOrder")).toArray())
        {
            if (!value.isString()) { return modelError(error, QStringLiteral("页面顺序必须保存稳定ID。")); }
            loaded.pageOrder.append(value.toString());
        }
        Q_FOREACH (value, json.value(QStringLiteral("configurationNodes")).toArray())
        {
            if (!value.isObject() || !HmiConfigurationNode::fromJson(value.toObject(), &node, error)) { return false; }
            loaded.configurationNodes.append(node);
        }
        Q_FOREACH (value, json.value(QStringLiteral("references")).toArray())
        {
            if (!value.isObject() || !HmiReference::fromJson(value.toObject(), &reference, error)) { return false; }
            loaded.references.append(reference);
        }
        resources = json.value(QStringLiteral("macroIds")).toObject();
        for (item = resources.constBegin(); item != resources.constEnd(); ++item)
        {
            if (!item.value().isString()) { return modelError(error, QStringLiteral("宏稳定ID必须为字符串。")); }
            loaded.macroIds.insert(item.key(), item.value().toString());
        }
        loaded.extensions = json.value(QStringLiteral("extensions")).toObject();
    }
    loaded.inactivePages.clear(); activeFound = false;
    for (index = 0; index < pages.size(); ++index)
    {
        if (!pages.at(index).isObject()) { return modelError(error, QStringLiteral("页面记录必须为对象。")); }
        pageJson = pages.at(index).toObject();
        if (schema < 9)
        {
            if (!knownKeys(pageJson, {QStringLiteral("name"), QStringLiteral("objects")}, QStringLiteral("page"), error)) { return false; }
            Q_FOREACH (value, pageJson.value(QStringLiteral("objects")).toArray())
            {
                if (value.toObject().contains(QStringLiteral("extensions"))) { return modelError(error, QStringLiteral("控件扩展必须使用schema9。")); }
            }
            pageJson.insert(QStringLiteral("id"), QStringLiteral("page-1")); pageJson.insert(QStringLiteral("kind"), QStringLiteral("normal"));
            pageJson.insert(QStringLiteral("width"), resolution.value(QStringLiteral("width")));
            pageJson.insert(QStringLiteral("height"), resolution.value(QStringLiteral("height")));
        }
        if (!HmiPage::fromJson(pageJson, &page, error)) { return false; }
        if (schema < 12) //20261112-0 change: A lower schema cannot conceal newly typed numeric or animation fields.
        { Q_FOREACH (value, pageJson.value(QStringLiteral("objects")).toArray()) { if (value.toObject().contains(QStringLiteral("gifDisplay")) || value.toObject().contains(QStringLiteral("numberDisplay")) || value.toObject().contains(QStringLiteral("asciiDisplay")) || value.toObject().contains(QStringLiteral("timeDisplay")) || value.toObject().contains(QStringLiteral("screenButton")) || value.toObject().contains(QStringLiteral("multiStateButton")) || value.toObject().contains(QStringLiteral("multiFunctionButton")) || value.toObject().contains(QStringLiteral("dropdown")) || value.toObject().contains(QStringLiteral("flowBlock")) || value.toObject().contains(QStringLiteral("slider")) || value.toObject().contains(QStringLiteral("wheel")) || value.toObject().contains(QStringLiteral("moveTrack")) || value.toObject().contains(QStringLiteral("chart")) || value.toObject().contains(QStringLiteral("trend")) || value.toObject().contains(QStringLiteral("historytrend"))) { return modelError(error, QStringLiteral("GIF、数值、ASCII、时间或画面按钮专有数据必须使用schema12。")); } } }
        if (parsedPageIds.contains(page.id)) { return modelError(error, QStringLiteral("页面稳定ID重复：%1。").arg(page.id)); }
        parsedPageIds.insert(page.id);
        if (page.id == loaded.activePageId) { assignActivePage(&loaded, page); activeFound = true; }
        else { loaded.inactivePages.append(page); }
    }
    if (!activeFound) { return modelError(error, QStringLiteral("当前编辑页ID未出现在页面列表中。")); }
    if (loaded.pageWidth != resolution.value(QStringLiteral("width")).toInt() || loaded.pageHeight != resolution.value(QStringLiteral("height")).toInt())
    {
        return modelError(error, QStringLiteral("编辑页尺寸与顶层分辨率镜像不一致。"));
    }
    if (!loaded.validateModel(error)) { return false; }
    if (loaded.toJson().value(QStringLiteral("schemaVersion")).toInt() > schema)
    {
        return modelError(error, QStringLiteral("工程活动字段需要更高schema版本，不允许静默接受降级文件。"));
    }
    *project = loaded;
    return true;
}

/******************************************************************************
 * Function Name   : HmiProject::save
 * Description     : Write an independent prototype project without claiming vendor compatibility.
 * Input Parameters: path, error
 * Return Value    : True when the JSON project was completely written to disk.
 * Date            : 2026.11.07
 ******************************************************************************/
bool HmiProject::save(const QString &path, QString *error) const //20261107-6 add
{
    QSaveFile file(path);
    QByteArray bytes;
    QJsonDocument document;

    if (!validateModel(error)) //20261110-5 change: Validate every page and resource before opening the atomic replacement.
    {
        return false;
    }
    if (nativeSourceSnapshot.size() > 16 * 1024 * 1024) //20261109-0 add
    {
        if (error != nullptr) { *error = QStringLiteral("原厂工程元数据快照超过16 MiB。"); }
        return false;
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        if (error != nullptr)
        {
            *error = file.errorString();
        }
        return false;
    }

    document = QJsonDocument(toJson());
    bytes = document.toJson(QJsonDocument::Indented);
    if (file.write(bytes) != bytes.size())
    {
        if (error != nullptr)
        {
            *error = file.errorString();
        }
        return false;
    }
    if (!file.commit())
    {
        if (error != nullptr)
        {
            *error = file.errorString();
        }
        return false;
    }
    return true;
}

/******************************************************************************
 * Function Name   : HmiProject::allPages
 * Description     : Return pages in explicit order while retaining malformed extras for validation.
 * Input Parameters: None
 * Return Value    : Value copies; active objects are never cached in inactivePages.
 * Date            : 2026.11.10
 ******************************************************************************/
QList<HmiPage> HmiProject::allPages() const //20261110-5 add
{
    QList<HmiPage> source;
    QList<HmiPage> ordered;
    QSet<int> used;
    QString id;
    int index;

    source.append(activePage(*this)); source.append(inactivePages);
    Q_FOREACH (id, pageOrder)
    {
        for (index = 0; index < source.size(); ++index)
        {
            if (!used.contains(index) && source.at(index).id == id)
            {
                ordered.append(source.at(index)); used.insert(index); break;
            }
        }
    }
    for (index = 0; index < source.size(); ++index) { if (!used.contains(index)) { ordered.append(source.at(index)); } }
    return ordered;
}

/******************************************************************************
 * Function Name   : HmiProject::pageCount
 * Description     : Count physical page values, including the sole active page.
 * Input Parameters: None
 * Return Value    : Page count.
 * Date            : 2026.11.10
 ******************************************************************************/
int HmiProject::pageCount() const //20261110-5 add
{
    return 1 + inactivePages.size();
}

/******************************************************************************
 * Function Name   : HmiProject::validateModel
 * Description     : Check all K entities and stable references before any transaction commits.
 * Input Parameters: error
 * Return Value    : True for a serializable bounded model; this does not imply native compilation.
 * Date            : 2026.11.10
 ******************************************************************************/
bool HmiProject::validateModel(QString *error) const //20261110-5 add
{
    QList<HmiPage> pages;
    HmiPage page;
    HmiPage parsedPage;
    HmiObject object;
    HmiConfigurationNode node;
    HmiConfigurationNode parsedNode;
    Stage2SystemSettings systemSettings; //20261110-7 add: Active system settings have a unique project owner.
    bool hasSystemSettings = false;
    HmiReference reference;
    HmiReference parsedReference;
    QSet<QString> pageIds;
    QSet<QString> objectIds;
    QSet<QString> nodeIds;
    QSet<QString> referenceIds;
    QSet<QString> macroStableIds;
    QSet<QString> languageNames;
    QSet<QString> referenceFields;
    QMap<QString, QString> navigationPageKinds; //20261112-0 add
    QSet<QString> keyboardPageIds; //20261112-0 add
    QMap<QString, QSet<QString>> entities;
    QMap<QString, QString>::const_iterator macro;
    QMap<QString, QByteArray>::const_iterator picture;
    PlcConnection connection;
    PlcConnection parsedConnection;
    QString language;
    QString warning;
    QString id;
    QString binding;
    qint64 macroUnits;
    qint64 pictureBytes;
    int subtypeCount;

    if (projectId.trimmed().isEmpty() || projectId.size() > 256 || name.trimmed().isEmpty() ||
        targetModel != QStringLiteral("AMT070W124-L800X480R") || pageCount() > 200 ||
        configurationNodes.size() > 256 || references.size() > 4096 || nativeSourceSnapshot.size() > 16 * 1024 * 1024 ||
        nativeSourceWarnings.size() > 128 || (!nativeSourceWarnings.isEmpty() && nativeSourceSnapshot.isEmpty()))
    {
        return modelError(error, QStringLiteral("工程身份、目标或实体数量超出Lite保护范围。"));
    }
    if (languages.isEmpty() || languages.size() > 8 || !languages.contains(activeLanguage) ||
        (!startupLanguage.isEmpty() && !languages.contains(startupLanguage)))
    {
        return modelError(error, QStringLiteral("编辑语言或启动语言未登记；Lite目前支持1..8语言。"));
    }
    Q_FOREACH (language, languages)
    {
        if (language.trimmed().isEmpty() || language.size() > 256 || languageNames.contains(language))
        {
            return modelError(error, QStringLiteral("工程语言为空、过长或重复。"));
        }
        languageNames.insert(language);
    }
    Q_FOREACH (warning, nativeSourceWarnings)
    {
        if (warning.size() > 4096) { return modelError(error, QStringLiteral("原厂导入警告超过Lite长度保护。")); }
    }
    if (!nativeSourceSnapshot.isEmpty() && !validateNativeProjectSourceStructure(nativeSourceSnapshot, error)) //20261110-5 add
    {
        return modelError(error, QStringLiteral("原厂元数据快照结构损坏：%1").arg(error == nullptr ? QString() : *error));
    }
    pages = allPages();
    Q_FOREACH (page, pages) { navigationPageKinds.insert(page.id, page.kind); } //20261112-0 add
    Q_FOREACH (page, pages) { if (page.kind == QStringLiteral("keyboard")) { keyboardPageIds.insert(page.id); } } //20261112-0 add
    Q_FOREACH (page, pages)
    {
        if (page.id.size() > 256 || pageIds.contains(page.id) || !HmiPage::fromJson(page.toJson(), &parsedPage, error))
        {
            return modelError(error, QStringLiteral("页面%1无效或稳定ID重复。%2").arg(page.id, error == nullptr ? QString() : *error));
        }
        pageIds.insert(page.id);
        Q_FOREACH (object, page.objects)
        {
            subtypeCount = int(object.hasWordButtonProperties) + int(object.hasLampProperties) + int(object.hasFunctionButtonProperties);
            if (object.id.size() > 256 || objectIds.contains(object.id) || subtypeCount > 1 ||
                (object.hasStaticTextProperties && (object.type != QStringLiteral("text") || object.hasBitButtonProperties || subtypeCount != 0)) ||
                (object.hasBitButtonProperties && object.type != QStringLiteral("switch")) ||
                (subtypeCount != 0 && !object.hasBitButtonProperties))
            {
                return modelError(error, QStringLiteral("控件%1：跨页ID重复或专有属性互相冲突。").arg(object.id));
            }
            if (object.type == QStringLiteral("slider") && object.slider.macroEnabled && !macros.contains(object.slider.macroName)) { return modelError(error, QStringLiteral("滑动开关引用的宏不存在。")); } //20261112-0 add
            if (object.type == QStringLiteral("dropdown") && object.dropdown.macroEnabled && !macros.contains(object.dropdown.macroName)) { return modelError(error, QStringLiteral("下拉框引用的宏不存在。")); } //20261112-0 add
            if (object.hasBitButtonProperties && object.bitButton.macroEnabled && !macros.contains(object.bitButton.macroName))
            {
                return modelError(error, QStringLiteral("控件%1引用了不存在的宏%2。").arg(object.id, object.bitButton.macroName));
            }
            if (object.type == QStringLiteral("multifunction")) //20261112-0 add: Every command carries its own stable navigation dependency.
            {
                MultiFunctionCommand command;
                Q_FOREACH (command, object.multiFunctionButton.commands)
                {
                    if (command.kind == 1 && (command.operation == 0 || command.operation == 2) && (!navigationPageKinds.contains(command.activeTarget()) || navigationPageKinds.value(command.activeTarget()) != (command.operation == 0 ? QStringLiteral("normal") : QStringLiteral("window")))) { return modelError(error, QStringLiteral("多功能按钮命令目标缺失或页面类型不匹配。")); }
                }
            }
            if (object.type == QStringLiteral("screen") && (!object.screenButton.activeTarget().isEmpty() || object.screenButton.action == 0 || object.screenButton.action == 2)) //20261112-0 add
            {
                if (!navigationPageKinds.contains(object.screenButton.activeTarget()) || navigationPageKinds.value(object.screenButton.activeTarget()) != (object.screenButton.action == 0 ? QStringLiteral("normal") : QStringLiteral("window"))) { return modelError(error, QStringLiteral("画面按钮目标缺失或页面类型不匹配。")); }
            }
            objectIds.insert(object.id);
            if (object.hasNumberDisplayProperties && object.numberDisplay.allowInput && object.numberDisplay.customKeyboard && !keyboardPageIds.contains(object.numberDisplay.keyboardPageId))
            { return modelError(error, QStringLiteral("数值控件的自定义键盘画面缺失或类型错误。")); } //20261112-0 add
            if (object.type == QStringLiteral("ascii") && object.asciiDisplay.common.allowInput && object.asciiDisplay.common.customKeyboard && !keyboardPageIds.contains(object.asciiDisplay.common.keyboardPageId))
            { return modelError(error, QStringLiteral("ASCII控件的自定义键盘画面缺失或类型错误。")); } //20261112-0 add
        }
    }
    if (pageOrder.size() != pages.size() || !pageIds.contains(activePageId) || !pageIds.contains(startupPageId))
    {
        return modelError(error, QStringLiteral("页面顺序不完整或编辑/启动页引用悬空。"));
    }
    pageIds.clear();
    Q_FOREACH (id, pageOrder)
    {
        if (id.isEmpty() || pageIds.contains(id)) { return modelError(error, QStringLiteral("页面顺序含重复ID。")); }
        pageIds.insert(id);
    }
    Q_FOREACH (page, pages)
    {
        if (!pageIds.contains(page.id)) { return modelError(error, QStringLiteral("页面顺序包含不存在的ID或遗漏页面。")); }
        if (page.id == startupPageId && page.kind != QStringLiteral("normal"))
        {
            return modelError(error, QStringLiteral("设备启动页必须引用普通画面。"));
        }
    }
    Q_FOREACH (node, configurationNodes)
    {
        if (nodeIds.contains(node.id) || node.id.size() > 256 ||
            (!node.ownerPageId.isEmpty() && !pageIds.contains(node.ownerPageId)) ||
            !HmiConfigurationNode::fromJson(node.toJson(), &parsedNode, error))
        {
            return modelError(error, QStringLiteral("配置节点%1重复、悬空或包含未恢复活动字段。").arg(node.id));
        }
        nodeIds.insert(node.id);
        if (node.kind == QStringLiteral("system") && node.implemented && node.active) { hasSystemSettings = true; }
    }
    //20261112-1 add: Resolve collection identities over all pages before allowing a rename, removal or paste to publish.
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
    if (hasSystemSettings && !readStage2SystemSettings(*this, &systemSettings, error)) { return false; } //20261110-7 add
    if (macros.size() > 128 || pictures.size() > 256) { return modelError(error, QStringLiteral("宏或图库数量超过Lite保护上限。")); }
    macroUnits = 0;
    for (macro = macros.constBegin(); macro != macros.constEnd(); ++macro)
    {
        macroUnits += macro.value().size();
        if (macro.key().trimmed().isEmpty() || macro.value().size() > 1024 * 1024 || macroUnits > 8 * 1024 * 1024)
        {
            return modelError(error, QStringLiteral("工程宏名称或源码超过Lite保护上限。"));
        }
    }
    for (macro = macroIds.constBegin(); macro != macroIds.constEnd(); ++macro)
    {
        if (!macros.contains(macro.key()) || macro.value().trimmed().isEmpty() || macro.value().size() > 256 || macroStableIds.contains(macro.value()))
        {
            return modelError(error, QStringLiteral("宏稳定ID重复或名称引用悬空。"));
        }
        macroStableIds.insert(macro.value());
    }
    pictureBytes = 0;
    for (picture = pictures.constBegin(); picture != pictures.constEnd(); ++picture)
    {
        pictureBytes += picture.value().size();
        if (picture.key().trimmed().isEmpty() || !validateButtonImage(picture.value()) || pictureBytes > 64 * 1024 * 1024)
        {
            return modelError(error, QStringLiteral("工程图片内容或总大小无效。"));
        }
    }
    pictureBytes = 0; //20261112-0 add: Animation bytes have a separate bounded resource budget.
    if (animations.size() > 256) { return modelError(error, QStringLiteral("动画资源超过256项Lite保护上限。")); }
    for (picture = animations.constBegin(); picture != animations.constEnd(); ++picture)
    {
        pictureBytes += picture.value().size();
        if (picture.key().trimmed().isEmpty() || picture.key().size() > 256 || pictureBytes > 32 * 1024 * 1024 || !validateGifData(picture.value(), error))
        { return modelError(error, QStringLiteral("动画名称、内容或资源总量无效。")); }
        entities[QStringLiteral("animation")].insert(picture.key());
    }
    entities.insert(QStringLiteral("project"), QSet<QString>{projectId}); entities.insert(QStringLiteral("page"), pageIds);
    entities.insert(QStringLiteral("object"), objectIds); entities.insert(QStringLiteral("configuration"), nodeIds);
    entities.insert(QStringLiteral("macro"), macroStableIds);
    for (picture = pictures.constBegin(); picture != pictures.constEnd(); ++picture) { entities[QStringLiteral("picture")].insert(picture.key()); }
    Q_FOREACH (connection, connections)
    {
        if (!PlcConnection::fromJson(connection.toJson(), &parsedConnection, error)) { return false; }
        entities[QStringLiteral("connection")].insert(connection.id);
    }
    Q_FOREACH (reference, references)
    {
        binding = reference.ownerKind + QLatin1Char('/') + reference.ownerId + QLatin1Char('/') + reference.field;
        if (referenceIds.contains(reference.id) || referenceFields.contains(binding) || reference.id.size() > 256 ||
            !HmiReference::fromJson(reference.toJson(), &parsedReference, error) ||
            !entities.value(reference.ownerKind).contains(reference.ownerId) || !entities.value(reference.targetKind).contains(reference.targetId))
        {
            return modelError(error, QStringLiteral("引用%1重复、所有者或目标悬空；旧宏名称不属于稳定ID。").arg(reference.id));
        }
        referenceIds.insert(reference.id); referenceFields.insert(binding);
    }
    if (!inactiveExtensions(extensions, QStringLiteral("project.extensions"), error)) { return false; }
    return validatePlcConfiguration(error);
}

/******************************************************************************
 * Function Name   : HmiProject::validateSinglePageBackend
 * Description     : Refuse editor structures that single-page source and device backends cannot encode.
 * Input Parameters: error
 * Return Value    : True only for the established topology; subtype checks remain backend-owned.
 * Date            : 2026.11.10
 ******************************************************************************/
bool HmiProject::validateSinglePageBackend(QString *error) const //20261110-5 add
{
    if (!validateModel(error)) { return false; }
    if (!animations.isEmpty()) //20261112-0 add: Unreferenced GIF resources also need a recovered native resource format.
    { return modelError(error, QStringLiteral("工程包含尚未恢复原厂源/设备格式的GIF动画资源。")); }
    if (pageCount() != 1 || activePageKind != QStringLiteral("normal") || startupPageId != activePageId ||
        pageWidth != 800 || pageHeight != 480 || !configurationNodes.isEmpty() || !references.isEmpty())
    {
        return modelError(error, QStringLiteral("当前源/设备后端仅支持单个800×480普通启动画面；多页、配置节点及显式引用尚未恢复，拒绝截取当前页下载。"));
    }
    return true;
}

/******************************************************************************
 * Function Name   : HmiProject::selectPage
 * Description     : Swap active storage atomically while keeping startup selection independent.
 * Input Parameters: id, error
 * Return Value    : True after a valid page becomes editable.
 * Date            : 2026.11.10
 ******************************************************************************/
bool HmiProject::selectPage(const QString &id, QString *error) //20261110-5 add
{
    HmiProject candidate;
    HmiPage selected;
    int index;

    if (!validateModel(error)) { return false; }
    if (id == activePageId) { return true; }
    candidate = *this;
    for (index = 0; index < candidate.inactivePages.size(); ++index)
    {
        if (candidate.inactivePages.at(index).id != id) { continue; }
        selected = candidate.inactivePages.takeAt(index); candidate.inactivePages.append(activePage(*this));
        assignActivePage(&candidate, selected);
        if (!candidate.validateModel(error)) { return false; }
        *this = candidate; return true;
    }
    return modelError(error, QStringLiteral("编辑页%1不存在。").arg(id));
}

/******************************************************************************
 * Function Name   : HmiProject::addPage
 * Description     : Append a caller-identified page only after complete reference and identity validation.
 * Input Parameters: page, error
 * Return Value    : True on atomic append.
 * Date            : 2026.11.10
 ******************************************************************************/
bool HmiProject::addPage(const HmiPage &page, QString *error) //20261110-5 add
{
    HmiProject candidate;

    candidate = *this; candidate.inactivePages.append(page); candidate.pageOrder.append(page.id);
    if (!candidate.validateModel(error)) { return false; }
    *this = candidate; return true;
}

/******************************************************************************
 * Function Name   : HmiProject::renamePage
 * Description     : Change display text without changing stable identities or references.
 * Input Parameters: id, newName, error
 * Return Value    : True after validation; failures leave the model untouched.
 * Date            : 2026.11.10
 ******************************************************************************/
bool HmiProject::renamePage(const QString &id, const QString &newName, QString *error) //20261110-5 add
{
    HmiProject candidate;
    int index;
    bool found;

    candidate = *this; found = false;
    if (candidate.activePageId == id) { candidate.pageName = newName; found = true; }
    for (index = 0; index < candidate.inactivePages.size(); ++index)
    {
        if (candidate.inactivePages.at(index).id == id) { candidate.inactivePages[index].name = newName; found = true; }
    }
    if (!found) { return modelError(error, QStringLiteral("待改名页面不存在。")); }
    if (!candidate.validateModel(error)) { return false; }
    *this = candidate; return true;
}

/******************************************************************************
 * Function Name   : HmiProject::copyPage
 * Description     : Clone a page with fresh page, object, group, configuration and owned-reference identities.
 * Input Parameters: id, newName, newId, error
 * Return Value    : True after all internal references are remapped and the transaction validates.
 * Date            : 2026.11.10
 ******************************************************************************/
bool HmiProject::copyPage(const QString &id, const QString &newName, QString *newId, QString *error) //20261110-5 add
{
    HmiProject candidate;
    HmiPage page;
    HmiPage copy;
    HmiConfigurationNode node;
    HmiReference reference;
    QMap<QString, QString> ids;
    QMap<QString, QString> groups;
    QString key;
    int index;
    bool found;

    if (!validateModel(error)) { return false; }
    found = false; candidate = *this;
    Q_FOREACH (page, allPages()) { if (page.id == id) { copy = page; found = true; break; } }
    if (!found) { return modelError(error, QStringLiteral("待复制页面不存在。")); }
    copy.id = QUuid::createUuid().toString(QUuid::WithoutBraces); copy.name = newName;
    ids.insert(QStringLiteral("page/") + id, copy.id);
    for (index = 0; index < copy.objects.size(); ++index)
    {
        key = copy.objects.at(index).id; copy.objects[index].id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        ids.insert(QStringLiteral("object/") + key, copy.objects.at(index).id);
        if (copy.objects[index].type == QStringLiteral("multifunction")) //20261112-0 add: Remap all self-navigation command drafts on page copy.
        {
            for (MultiFunctionCommand &command : copy.objects[index].multiFunctionButton.commands)
            { if (command.screenPageId == id) { command.screenPageId = copy.id; } if (command.windowPageId == id) { command.windowPageId = copy.id; } }
        }
        if (copy.objects[index].type == QStringLiteral("screen")) //20261112-0 add: Self-navigation follows the duplicated page identity.
        { if (copy.objects[index].screenButton.screenPageId == id) { copy.objects[index].screenButton.screenPageId = copy.id; } if (copy.objects[index].screenButton.windowPageId == id) { copy.objects[index].screenButton.windowPageId = copy.id; } }
        key = copy.objects.at(index).group;
        if (!key.isEmpty())
        {
            if (!groups.contains(key)) { groups.insert(key, QUuid::createUuid().toString(QUuid::WithoutBraces)); }
            copy.objects[index].group = groups.value(key);
        }
    }
    Q_FOREACH (node, configurationNodes)
    {
        if (node.ownerPageId != id) { continue; }
        key = node.id; node.id = QUuid::createUuid().toString(QUuid::WithoutBraces); node.ownerPageId = copy.id;
        ids.insert(QStringLiteral("configuration/") + key, node.id); candidate.configurationNodes.append(node);
    }
    Q_FOREACH (reference, references)
    {
        key = reference.ownerKind + QLatin1Char('/') + reference.ownerId;
        if (!ids.contains(key)) { continue; }
        reference.id = QUuid::createUuid().toString(QUuid::WithoutBraces); reference.ownerId = ids.value(key);
        key = reference.targetKind + QLatin1Char('/') + reference.targetId;
        if (ids.contains(key)) { reference.targetId = ids.value(key); }
        candidate.references.append(reference);
    }
    candidate.inactivePages.append(copy); candidate.pageOrder.append(copy.id);
    if (!candidate.validateModel(error)) { return false; }
    *this = candidate;
    if (newId != nullptr) { *newId = copy.id; }
    return true;
}

/******************************************************************************
 * Function Name   : HmiProject::removePage
 * Description     : Reject inbound references before removing a non-startup page and its owned entities.
 * Input Parameters: id, error
 * Return Value    : True after atomic removal; failed removal never mutates resources or selection.
 * Date            : 2026.11.10
 ******************************************************************************/
bool HmiProject::removePage(const QString &id, QString *error) //20261110-5 add
{
    HmiProject candidate;
    HmiPage page;
    HmiObject object;
    HmiConfigurationNode node;
    HmiReference reference;
    QSet<QString> removed;
    int index;
    bool found;

    if (!validateModel(error)) { return false; }
    if (id == startupPageId || pageCount() == 1) { return modelError(error, QStringLiteral("不能删除启动页或唯一页面，请先显式调整启动引用。")); }
    found = false; removed.insert(QStringLiteral("page/") + id);
    Q_FOREACH (page, allPages())
    {
        if (page.id != id) { continue; }
        found = true; Q_FOREACH (object, page.objects) { removed.insert(QStringLiteral("object/") + object.id); }
    }
    if (!found) { return modelError(error, QStringLiteral("待删除页面不存在。")); }
    Q_FOREACH (node, configurationNodes) { if (node.ownerPageId == id) { removed.insert(QStringLiteral("configuration/") + node.id); } }
    Q_FOREACH (reference, references)
    {
        if (removed.contains(reference.targetKind + QLatin1Char('/') + reference.targetId) &&
            !removed.contains(reference.ownerKind + QLatin1Char('/') + reference.ownerId))
        {
            return modelError(error, QStringLiteral("页面仍被引用%1使用，删除事务已取消。").arg(reference.id));
        }
    }
    candidate = *this;
    if (candidate.activePageId == id && !candidate.selectPage(startupPageId, error)) { return false; }
    for (index = candidate.inactivePages.size() - 1; index >= 0; --index) { if (candidate.inactivePages.at(index).id == id) { candidate.inactivePages.removeAt(index); } }
    for (index = candidate.configurationNodes.size() - 1; index >= 0; --index) { if (candidate.configurationNodes.at(index).ownerPageId == id) { candidate.configurationNodes.removeAt(index); } }
    for (index = candidate.references.size() - 1; index >= 0; --index)
    {
        reference = candidate.references.at(index);
        if (removed.contains(reference.ownerKind + QLatin1Char('/') + reference.ownerId)) { candidate.references.removeAt(index); }
    }
    candidate.pageOrder.removeAll(id);
    if (!candidate.validateModel(error)) { return false; }
    *this = candidate; return true;
}

/******************************************************************************
 * Function Name   : HmiProject::renameMacro
 * Description     : Rename a macro, preserving optional stable IDs and every page's live or dormant binding.
 * Input Parameters: oldName, newName, error
 * Return Value    : True on an atomic rename; existing sources are never overwritten.
 * Date            : 2026.11.10
 ******************************************************************************/
bool HmiProject::renameMacro(const QString &oldName, const QString &newName, QString *error) //20261110-5 add
{
    HmiProject candidate;
    QMap<QString, QString>::const_iterator macro;
    int pageIndex;
    int objectIndex;

    if (!validateModel(error)) { return false; }
    if (!macros.contains(oldName) || newName.trimmed().isEmpty() || (newName != oldName && macros.contains(newName)))
    {
        return modelError(error, QStringLiteral("宏不存在、目标名称为空或与已有宏冲突。"));
    }
    if (oldName == newName) { return true; }
    candidate = *this;
    for (macro = macros.constBegin(); macro != macros.constEnd(); ++macro)
    {
        if (!candidate.macroIds.contains(macro.key())) { candidate.macroIds.insert(macro.key(), QUuid::createUuid().toString(QUuid::WithoutBraces)); }
    }
    candidate.macros.insert(newName, candidate.macros.take(oldName)); candidate.macroIds.insert(newName, candidate.macroIds.take(oldName));
    for (objectIndex = 0; objectIndex < candidate.objects.size(); ++objectIndex)
    {
        if (candidate.objects.at(objectIndex).type == QStringLiteral("slider") && candidate.objects.at(objectIndex).slider.macroName == oldName) { candidate.objects[objectIndex].slider.macroName = newName; } //20261112-0 add
        if (candidate.objects.at(objectIndex).type == QStringLiteral("dropdown") && candidate.objects.at(objectIndex).dropdown.macroName == oldName) { candidate.objects[objectIndex].dropdown.macroName = newName; } //20261112-0 add
        if (candidate.objects.at(objectIndex).hasBitButtonProperties && candidate.objects.at(objectIndex).bitButton.macroName == oldName)
        {
            candidate.objects[objectIndex].bitButton.macroName = newName;
        }
    }
    for (pageIndex = 0; pageIndex < candidate.inactivePages.size(); ++pageIndex)
    {
        for (objectIndex = 0; objectIndex < candidate.inactivePages.at(pageIndex).objects.size(); ++objectIndex)
        {
            if (candidate.inactivePages.at(pageIndex).objects.at(objectIndex).type == QStringLiteral("slider") && candidate.inactivePages.at(pageIndex).objects.at(objectIndex).slider.macroName == oldName) { candidate.inactivePages[pageIndex].objects[objectIndex].slider.macroName = newName; } //20261112-0 add
            if (candidate.inactivePages.at(pageIndex).objects.at(objectIndex).type == QStringLiteral("dropdown") && candidate.inactivePages.at(pageIndex).objects.at(objectIndex).dropdown.macroName == oldName) { candidate.inactivePages[pageIndex].objects[objectIndex].dropdown.macroName = newName; } //20261112-0 add
            if (candidate.inactivePages.at(pageIndex).objects.at(objectIndex).hasBitButtonProperties &&
                candidate.inactivePages.at(pageIndex).objects.at(objectIndex).bitButton.macroName == oldName)
            {
                candidate.inactivePages[pageIndex].objects[objectIndex].bitButton.macroName = newName;
            }
        }
    }
    if (!candidate.validateModel(error)) { return false; }
    *this = candidate; return true;
}

/******************************************************************************
 * Function Name   : HmiProject::setLanguages
 * Description     : Remap function-language targets by name without deleting dormant translation drafts.
 * Input Parameters: newLanguages, error
 * Return Value    : True after an atomic list change; live startup or function targets cannot be deleted.
 * Date            : 2026.11.10
 ******************************************************************************/
bool HmiProject::setLanguages(const QStringList &newLanguages, QString *error) //20261110-5 add
{
    HmiProject candidate;
    QList<HmiPage> pages;
    HmiObject *object;
    QString targetLanguage;
    int pageIndex;
    int objectIndex;

    if (!validateModel(error)) { return false; }
    if (!startupLanguage.isEmpty() && !newLanguages.contains(startupLanguage))
    {
        return modelError(error, QStringLiteral("语言%1仍被设备启动语言引用，修改已取消。").arg(startupLanguage));
    }
    candidate = *this; pages = candidate.allPages();
    for (pageIndex = 0; pageIndex < pages.size(); ++pageIndex)
    {
        for (objectIndex = 0; objectIndex < pages.at(pageIndex).objects.size(); ++objectIndex)
        {
            object = &pages[pageIndex].objects[objectIndex];
            if (!object->hasFunctionButtonProperties || object->functionButton.function != 13) { continue; }
            if (object->functionButton.type < 0 || object->functionButton.type >= languages.size())
            {
                return modelError(error, QStringLiteral("功能按钮%1的旧语言目标无效。").arg(object->id));
            }
            targetLanguage = languages.at(object->functionButton.type);
            if (!newLanguages.contains(targetLanguage))
            {
                return modelError(error, QStringLiteral("语言%1仍被功能按钮%2引用，修改已取消。").arg(targetLanguage, object->id));
            }
            object->functionButton.type = newLanguages.indexOf(targetLanguage);
        }
    }
    candidate.inactivePages.clear();
    for (pageIndex = 0; pageIndex < pages.size(); ++pageIndex)
    {
        if (pages.at(pageIndex).id == candidate.activePageId) { assignActivePage(&candidate, pages.at(pageIndex)); }
        else { candidate.inactivePages.append(pages.at(pageIndex)); }
    }
    candidate.languages = newLanguages;
    if (!newLanguages.contains(candidate.activeLanguage)) { candidate.activeLanguage = newLanguages.value(0); }
    if (!candidate.validateModel(error)) { return false; }
    *this = candidate; return true;
}

/******************************************************************************
 * Function Name   : HmiProject::validatePlcConfiguration
 * Description     : Keep qualified addresses bound to existing, distinct project connections.
 * Input Parameters: error
 * Return Value    : True when every external reference can be restored without an implicit default route.
 * Date            : 2026.11.09
 ******************************************************************************/
bool HmiProject::validatePlcConfiguration(QString *error) const //20261109-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress decoded;
    PlcConnection connection;
    HmiObject object;
    HmiPage page;
    QList<HmiObject> allObjects;
    QSet<QString> ids;
    QSet<int> channels;
    QList<QPair<QString, PlcAddressRole>> addresses;
    QPair<QString, PlcAddressRole> address;
    const StaticTextProperties *condition;
    int index;
    bool hasQualified;

    hasQualified = false;
    Q_FOREACH (page, allPages()) { allObjects.append(page.objects); } //20261110-5 change
    Q_FOREACH (object, allObjects)
    {
        if (object.type == QStringLiteral("gif") && (object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasImageDisplayProperties || !object.address.isEmpty() ||
            !object.gifDisplay.validate(error) || !object.gifDisplay.validateAddresses(connections, error))) { return false; } //20261112-0 add
        if (object.hasNumberDisplayProperties && (object.type != QStringLiteral("number") || object.hasBitButtonProperties || object.hasStaticTextProperties || !object.numberDisplay.validate(error) || !object.numberDisplay.validateAddresses(connections, error))) { return false; } //20261112-0 add
        if (object.type == QStringLiteral("chart") && (!object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || object.hasImageDisplayProperties || !object.chart.validate(error) || !object.chart.validateAddresses(connections, error))) { return false; } //20261112-1 add
        if (object.type == QStringLiteral("trend") && (!object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || object.hasImageDisplayProperties || !object.trend.validate(error) || !object.trend.validateAddresses(connections, error))) { return false; } //20261112-1 add
        if (object.type == QStringLiteral("historytrend") && (!object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || object.hasImageDisplayProperties || !object.historyTrend.validate(error) || !object.historyTrend.validateAddresses(connections, error))) { return false; } //20261112-1 add
        if (object.type == QStringLiteral("track") && (!object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || object.hasImageDisplayProperties || !object.moveTrack.validate(error) || !object.moveTrack.validateAddresses(connections, error))) { return false; } //20261112-0 add
        if (object.type == QStringLiteral("wheel") && (!object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || !object.wheel.validate(error) || !object.wheel.validateAddresses(connections, error))) { return false; } //20261112-0 add
        if (object.type == QStringLiteral("slider") && (!object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || !object.slider.validate(error) || !object.slider.validateAddresses(connections, error))) { return false; } //20261112-0 add
        if (object.type == QStringLiteral("flow") && (!object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || !object.flowBlock.validate(error) || !object.flowBlock.validateAddresses(connections, error))) { return false; } //20261112-0 add
        if (object.type == QStringLiteral("dropdown") && (!object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || !object.dropdown.validate(error) || !object.dropdown.validateAddresses(connections, error))) { return false; } //20261112-0 add
        if (object.type == QStringLiteral("multifunction") && (!object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || !object.multiFunctionButton.validate(error) || !object.multiFunctionButton.validateAddresses(connections, error))) { return false; } //20261112-0 add
        if (object.type == QStringLiteral("multistate") && (!object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || !object.multiStateButton.validate(error) || !object.multiStateButton.validateAddresses(connections, error))) { return false; } //20261112-0 add
        if (object.type == QStringLiteral("screen") && (!object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || !object.screenButton.validate(error) || !object.screenButton.validateAddresses(connections, error))) { return false; } //20261112-0 add
        if (object.type == QStringLiteral("time") && (!object.address.isEmpty() || object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || !object.timeDisplay.validate(error) || !object.timeDisplay.validateAddresses(connections, error))) { return false; } //20261112-0 add
        if (object.type == QStringLiteral("ascii") && (object.hasBitButtonProperties || object.hasStaticTextProperties || object.hasNumberDisplayProperties || !object.asciiDisplay.validate(error) || !object.asciiDisplay.validateAddresses(connections, error))) { return false; } //20261112-0 add
        if (object.hasImageDisplayProperties && (object.type != QStringLiteral("image") || object.hasBitButtonProperties || !object.imageDisplay.validate(error) || !object.imageDisplay.validateAddresses(connections, error))) { return false; } //20261112-0 add
        if (object.type == QStringLiteral("message") && !object.messageDisplay.validateAddresses(connections, error)) { return false; } //20261112-0 add
        if (object.type == QStringLiteral("qrcode") && (!object.qrCode.validate(error) || !object.qrCode.validateAddresses(connections, error))) { return false; } //20261112-0 add
        if (object.hasWordButtonProperties && (object.type != QStringLiteral("switch") ||
            !object.hasBitButtonProperties || object.bitButton.indicator || object.bitButton.dualMonitor ||
            !object.wordButton.validate(error))) //20261109-6 add: Enforce the same subtype contract for in-memory compile and save calls.
        {
            if (error != nullptr && error->isEmpty()) { *error = QStringLiteral("字按钮必须使用开关外观且不能启用指示灯监视。"); }
            return false;
        }
        if (object.hasLampProperties && (object.type != QStringLiteral("switch") || !object.hasBitButtonProperties ||
            object.hasWordButtonProperties || object.bitButton.indicator != (object.lamp.monitorMode != 0) ||
            object.bitButton.dualMonitor != (object.lamp.monitorMode == 1) || object.bitButton.touch.visibilityEnabled ||
            object.bitButton.pressTimeEnabled || object.bitButton.macroEnabled || object.bitButton.notice ||
            object.lamp.monitorMode == 0 || object.bitButton.monitorSame || object.lamp.multiMonitorSame ||
            !object.lamp.validate(error))) //20261109-8 add
        {
            if (error != nullptr && error->isEmpty()) { *error = QStringLiteral("指示灯必须使用独立双态或多态监视，不能启用触控、按压时间、宏和通知。"); }
            return false;
        }
        if (object.hasFunctionButtonProperties && (object.type != QStringLiteral("switch") || //20261110-0 add
            !object.hasBitButtonProperties || object.hasWordButtonProperties || object.hasLampProperties ||
            object.bitButton.indicator || object.bitButton.dualMonitor || object.bitButton.macroEnabled ||
            !object.address.isEmpty() || (object.bitButton.noticeBit && object.bitButton.noticeWord) ||
            (object.functionButton.pressedEffect && object.bitButton.states[0].sameLanguage != object.bitButton.states[1].sameLanguage) ||
            object.bitButton.pressTime < 0 || object.bitButton.pressTime > 255 ||
            object.bitButton.states[0].text.fontSize > 128 ||
            !object.functionButton.validate(error) ||
            (object.functionButton.function == 13 && object.functionButton.type >= languages.size())))
        {
            if (error != nullptr && error->isEmpty()) { *error = QStringLiteral("功能按钮必须使用单态开关外观、有效功能类型和工程语言，宏指令与原生版本一样禁用。"); }
            return false;
        }
        if (!object.hasLampProperties && !object.hasFunctionButtonProperties && !object.hasImageDisplayProperties && !object.hasNumberDisplayProperties && object.type != QStringLiteral("gif") && object.type != QStringLiteral("ascii")) //20261112-0 change: Typed monitors validate their actual bit/word role separately.
        {
            addresses.append(qMakePair(object.address, object.type == QStringLiteral("switch") && !object.hasWordButtonProperties ?
                                        PlcAddressRole::Bit : PlcAddressRole::Word));
        }
        else if (object.hasLampProperties) { addresses.append(qMakePair(object.lamp.multiMonitorAddress, PlcAddressRole::Word)); }
        if (object.hasBitButtonProperties)
        {
            addresses.append(qMakePair(object.bitButton.monitorBitAddress, PlcAddressRole::Bit));
            addresses.append(qMakePair(object.bitButton.monitorWordAddress, PlcAddressRole::Word));
            addresses.append(qMakePair(object.bitButton.noticeBitAddress, PlcAddressRole::Bit));
            addresses.append(qMakePair(object.bitButton.noticeWordAddress, PlcAddressRole::Word));
            for (index = 0; index < 2; ++index)
            {
                condition = index == 0 ? &object.bitButton.visibility : &object.bitButton.touch;
                addresses.append(qMakePair(condition->bitAddress, PlcAddressRole::Bit));
                addresses.append(qMakePair(condition->wordAddress, PlcAddressRole::Word));
            }
        }
        if (object.hasStaticTextProperties)
        {
            addresses.append(qMakePair(object.staticText.bitAddress, PlcAddressRole::Bit));
            addresses.append(qMakePair(object.staticText.wordAddress, PlcAddressRole::Word));
        }
    }
    Q_FOREACH (address, addresses)
    {
        hasQualified = hasQualified || address.first.startsWith(QLatin1Char('@'));
    }
    if (connections.isEmpty() && !hasQualified) { return true; }
    if (connections.size() > 2 || !catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error))
    {
        if (error != nullptr && error->isEmpty()) { *error = QStringLiteral("PLC连接配置无效。"); }
        return false;
    }
    Q_FOREACH (connection, connections)
    {
        if (ids.contains(connection.id) || channels.contains(int(connection.channel)) ||
            catalog.profile(connection.profileId) == nullptr || connection.channel == PlcChannel::Unspecified)
        {
            if (error != nullptr) { *error = QStringLiteral("PLC连接重复、未指定通道或协议不存在。"); }
            return false;
        }
        ids.insert(connection.id);
        channels.insert(int(connection.channel));
    }
    context.connections = connections;
    Q_FOREACH (address, addresses)
    {
        if (!address.first.startsWith(QLatin1Char('@'))) { continue; }
        context.role = address.second;
        if (!parseQualifiedPlcAddressText(address.first, context, catalog, &decoded, error)) { return false; }
    }
    return true;
}

/******************************************************************************
 * Function Name   : HmiProject::load
 * Description     : Read a prototype project or the verified native one-text project.
 * Input Parameters: path, project, error
 * Return Value    : True when the file contains a supported project schema.
 * Date            : 2026.11.07
 ******************************************************************************/
bool HmiProject::load(const QString &path, HmiProject *project, QString *error) //20261108-9 change
{
    QFile file(path);
    HmiProject imported; //20261110-5 add: Legacy and typed importers never receive the caller's live model.
    QByteArray bytes;
    QJsonParseError parse_error;
    QJsonDocument document;

    if (project == nullptr) { return modelError(error, QStringLiteral("工程接收位置为空。")); }
    if (QFileInfo(path).suffix().compare(QStringLiteral("sm"), Qt::CaseInsensitive) == 0) //20261108-9 add
    {
        if (importNativeTestProject(path, &imported, error)) //20261110-5 change
        {
            if (!imported.validateModel(error)) { return false; }
            *project = imported; return true;
        }
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        if (error != nullptr)
        {
            *error = file.errorString();
        }
        return false;
    }

    if (file.size() > 128 * 1024 * 1024) //20261110-5 add: Carrier allocation guard, not a native limit.
    {
        return modelError(error, QStringLiteral("工程文件超过128 MiB的Lite读取保护上限。"));
    }
    bytes = file.readAll();
    if (file.error() != QFileDevice::NoError) { return modelError(error, file.errorString()); }
    if (QFileInfo(path).suffix().compare(QStringLiteral("sm"), Qt::CaseInsensitive) == 0) //20261109-0 add
    {
        if (!importNativeProjectSource(bytes, &imported, error) || !imported.validateModel(error)) { return false; } //20261110-5 change
        *project = imported; return true;
    }
    document = QJsonDocument::fromJson(bytes, &parse_error);
    if (parse_error.error != QJsonParseError::NoError || !document.isObject())
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("无法解析工程文件：%1").arg(parse_error.errorString());
        }
        return false;
    }
    return fromJson(document.object(), project, error);
}
