#include "slider_properties.h"
#include "word_button_properties.h"
#include "plc_native_catalog.h"
#include <cmath>

/******************************************************************************
 * Function Name   : SliderProperties::SliderProperties
 * Description     : Initialize the measured independent touch access level.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
SliderProperties::SliderProperties() //20261112-0 add
{
    touch.minimumLevel = 2;
}

/******************************************************************************
 * Function Name   : SliderProperties::dataTypes
 * Description     : Return the measured five-type order, without the button family's BCD entries.
 * Input Parameters: None
 * Return Value    : Observed type labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList SliderProperties::dataTypes() //20261112-0 add
{
    return {QStringLiteral("16位正整数"), QStringLiteral("32位正整数"), QStringLiteral("16位整数"), QStringLiteral("32位整数"), QStringLiteral("32位浮点数")};
}

/******************************************************************************
 * Function Name   : SliderProperties::styles
 * Description     : Preserve the four native thumb style labels and order.
 * Input Parameters: None
 * Return Value    : Style labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList SliderProperties::styles() //20261112-0 add
{
    return {QStringLiteral("样式1(圆)"), QStringLiteral("样式2(矩形)"), QStringLiteral("样式3(单向)"), QStringLiteral("样式4(双向)")};
}

/******************************************************************************
 * Function Name   : SliderProperties::vertical
 * Description     : Select the rail axis from the thumb's left/right pointing group.
 * Input Parameters: None
 * Return Value    : True for a vertical rail.
 * Date            : 2026.11.12
 ******************************************************************************/
bool SliderProperties::vertical() const //20261112-0 add
{
    return pointing >= 2;
}

/******************************************************************************
 * Function Name   : SliderProperties::directions
 * Description     : Return the direction pair belonging to the active rail axis.
 * Input Parameters: None
 * Return Value    : Two native direction labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList SliderProperties::directions() const //20261112-0 add
{
    return vertical() ? QStringList{QStringLiteral("向上"), QStringLiteral("向下")} : QStringList{QStringLiteral("向左"), QStringLiteral("向右")};
}

/******************************************************************************
 * Function Name   : SliderProperties::setPointing
 * Description     : Reset the direction when crossing axes, matching the measured up-to-left transition.
 * Input Parameters: value
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void SliderProperties::setPointing(int value) //20261112-0 add
{
    bool before = vertical();
    pointing = qBound(0, value, 3);
    if (before != vertical()) { direction = 0; }
}

/******************************************************************************
 * Function Name   : SliderProperties::setDynamicRange
 * Description     : Preserve constant drafts while resetting dynamic addresses on each fresh enable as observed.
 * Input Parameters: enabled
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void SliderProperties::setDynamicRange(bool enabled) //20261112-0 add
{
    if (enabled && !dynamicRange) { minimumAddress = QStringLiteral("LW2"); maximumAddress = QStringLiteral("LW3"); }
    dynamicRange = enabled;
}

/******************************************************************************
 * Function Name   : SliderProperties::transferWordCount
 * Description     : Preserve the full carrier width for all five data types.
 * Input Parameters: None
 * Return Value    : One or two words.
 * Date            : 2026.11.12
 ******************************************************************************/
int SliderProperties::transferWordCount() const //20261112-0 add
{
    return dataType == 0 || dataType == 2 ? 1 : 2;
}

/******************************************************************************
 * Function Name   : SliderProperties::constrainedGeometry
 * Description     : Apply the observed perpendicular thumb-height plus four-pixel envelope; retain the dragged axial length.
 * Input Parameters: requested
 * Return Value    : Editor rectangle, not a decoded firmware geometry record.
 * Date            : 2026.11.12
 ******************************************************************************/
QRect SliderProperties::constrainedGeometry(const QRect &requested) const //20261112-0 add
{
    QRect result = requested;
    if (vertical()) { result.setWidth(thumbHeight + 4); }
    else { result.setHeight(thumbHeight + 4); }
    return result;
}

/******************************************************************************
 * Function Name   : SliderProperties::accessDraft
 * Description     : Adapt shared permissions and notification validation without adopting button operations.
 * Input Parameters: None
 * Return Value    : Temporary common-access carrier.
 * Date            : 2026.11.12
 ******************************************************************************/
BitButtonProperties SliderProperties::accessDraft() const //20261112-0 add
{
    BitButtonProperties access;
    access.indicator = false; access.dualMonitor = false; access.monitorSame = false; access.bitIndex = 0; access.pressTime = 0;
    access.visibility = visibility; access.touch = touch; access.notice = notice; access.noticeBit = noticeBit; access.noticeWord = noticeWord; access.noticeBitValue = noticeBitValue; access.noticeBitAddress = noticeBitAddress; access.noticeWordAddress = noticeWordAddress; access.noticeWordType = noticeWordType; access.noticeWordValue = noticeWordValue;
    return access;
}

/******************************************************************************
 * Function Name   : SliderProperties::toJson
 * Description     : Persist independent constants, dynamic carriers, seven colors and all permission drafts.
 * Input Parameters: None
 * Return Value    : Complete typed editor JSON.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject SliderProperties::toJson() const //20261112-0 add
{
    QJsonObject json;
#define SAVE_SLIDER(name, initial) json.insert(QStringLiteral(#name), name);
    SLIDER_BOOL_FIELDS(SAVE_SLIDER) SLIDER_INT_FIELDS(SAVE_SLIDER) SLIDER_STRING_FIELDS(SAVE_SLIDER)
#undef SAVE_SLIDER
    json.insert(QStringLiteral("visibility"), visibility.toJson()); json.insert(QStringLiteral("touch"), touch.toJson()); return json;
}

/******************************************************************************
 * Function Name   : SliderProperties::fromJson
 * Description     : Reject missing, unknown, fractional or malformed typed records before publishing any output.
 * Input Parameters: json, output, error
 * Return Value    : True for a complete validated model.
 * Date            : 2026.11.12
 ******************************************************************************/
bool SliderProperties::fromJson(const QJsonObject &json, SliderProperties *output, QString *error) //20261112-0 add
{
    SliderProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    double number;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("滑动开关字段缺失或未知。"); } return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 800) { return false; } }
    }
#define READ_SLIDER_BOOL(name, initial) candidate.name = json.value(QStringLiteral(#name)).toBool();
    SLIDER_BOOL_FIELDS(READ_SLIDER_BOOL)
#undef READ_SLIDER_BOOL
#define READ_SLIDER_INT(name, initial) candidate.name = json.value(QStringLiteral(#name)).toInt();
    SLIDER_INT_FIELDS(READ_SLIDER_INT)
#undef READ_SLIDER_INT
#define READ_SLIDER_STRING(name, initial) candidate.name = json.value(QStringLiteral(#name)).toString();
    SLIDER_STRING_FIELDS(READ_SLIDER_STRING)
#undef READ_SLIDER_STRING
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !StaticTextProperties::fromJson(json.value(QStringLiteral("touch")).toObject(), &candidate.touch, error) || candidate.touch.toJson() != json.value(QStringLiteral("touch")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : SliderProperties::validate
 * Description     : Validate PC bounds and finite ordered constants using the explicit five-to-seven type mapping.
 * Input Parameters: error
 * Return Value    : True for the declared editor slice.
 * Date            : 2026.11.12
 ******************************************************************************/
bool SliderProperties::validate(QString *error) const //20261112-0 add
{
    const int types[5] = {0, 1, 2, 3, 6};
    WordButtonProperties scalar;
    ScreenButtonProperties shared;
    QString color;
    if (dataType < 0 || dataType > 4 || pointing < 0 || pointing > 3 || direction < 0 || direction > 1 || style < 0 || style > 3 || railHeight < 1 || railHeight > 800 || thumbWidth < 1 || thumbWidth > 800 || thumbHeight < 1 || thumbHeight > 800 || minimum.size() > 256 || maximum.size() > 256 || address.size() > 1024 || minimumAddress.size() > 1024 || maximumAddress.size() > 1024 || macroName.size() > 256 || (macroEnabled && macroName.trimmed().isEmpty()))
    { if (error != nullptr) { *error = QStringLiteral("滑动开关类型、尺寸、范围或宏选择无效。"); } return false; }
    Q_FOREACH (color, QStringList({background, borderColor, railBackground, fillColor, railBorderColor, thumbBackground, thumbBorderColor})) { if (color.size() > 32 || !QColor(color).isValid()) { return false; } }
    shared.common = accessDraft(); if (!shared.validate(error)) { return false; }
    if (!dynamicRange)
    {
        scalar.resetDefaultsForDataType(types[dataType]); scalar.operation = 0; scalar.constantValue = minimum; if (!scalar.validate(error)) { return false; }
        scalar.constantValue = maximum; if (!scalar.validate(error)) { return false; }
        if (minimum.toDouble() >= maximum.toDouble()) { if (error != nullptr) { *error = QStringLiteral("本版编辑范围要求最小值小于最大值。"); } return false; }
    }
    return true;
}

/******************************************************************************
 * Function Name   : SliderProperties::validateAddresses
 * Description     : Validate full-width operation and active dynamic carriers independently, allowing measured overlaps.
 * Input Parameters: connections, error
 * Return Value    : True for active addresses; no device access occurs.
 * Date            : 2026.11.12
 ******************************************************************************/
bool SliderProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress parsed;
    ScreenButtonProperties shared;
    QString text;
    QStringList carriers = {address};
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections; context.role = PlcAddressRole::Word; context.transferCount = u32(transferWordCount());
    if (dynamicRange) { carriers.append(minimumAddress); carriers.append(maximumAddress); }
    Q_FOREACH (text, carriers) { if (!parseQualifiedPlcAddressText(text, context, catalog, &parsed, error)) { return false; } }
    shared.common = accessDraft(); return shared.validateAddresses(connections, error);
}
