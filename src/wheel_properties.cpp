#include "wheel_properties.h"
#include "bit_button_properties.h"
#include "plc_native_catalog.h"
#include <QFile>
#include <QRegularExpression>
#include <cmath>

/******************************************************************************
 * Function Name   : WheelProperties::WheelProperties
 * Description     : Initialize the native touch permission level independently of visibility.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
WheelProperties::WheelProperties() //20261112-0 add
{
    touch.minimumLevel = 2;
}

/******************************************************************************
 * Function Name   : WheelProperties::fontSizes
 * Description     : Return the complete observed fixed font-size menu shared by high and low rows.
 * Input Parameters: None
 * Return Value    : Seven pixel sizes.
 * Date            : 2026.11.12
 ******************************************************************************/
QList<int> WheelProperties::fontSizes() //20261112-0 add
{
    return {8, 12, 16, 24, 32, 40, 48};
}

/******************************************************************************
 * Function Name   : WheelProperties::editorSampleValue
 * Description     : Produce the measured integer midpoint preview without reading or writing any PLC register.
 * Input Parameters: None
 * Return Value    : Bounded editor sample value.
 * Date            : 2026.11.12
 ******************************************************************************/
int WheelProperties::editorSampleValue() const //20261112-0 add
{
    int lower = minimum.toInt();
    int upper = maximum.toInt();
    return lower + (upper - lower) / 2;
}

/******************************************************************************
 * Function Name   : WheelProperties::toJson
 * Description     : Preserve independent numerical, image-resource and permission drafts in typed KSP JSON.
 * Input Parameters: None
 * Return Value    : Complete editor record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject WheelProperties::toJson() const //20261112-0 add
{
    QJsonObject json;
#define SAVE_WHEEL(name, initial) json.insert(QStringLiteral(#name), name);
    WHEEL_BOOL_FIELDS(SAVE_WHEEL) WHEEL_INT_FIELDS(SAVE_WHEEL) WHEEL_STRING_FIELDS(SAVE_WHEEL)
#undef SAVE_WHEEL
    json.insert(QStringLiteral("highlight"), highlight.toJson()); json.insert(QStringLiteral("visibility"), visibility.toJson()); json.insert(QStringLiteral("touch"), touch.toJson()); return json;
}

/******************************************************************************
 * Function Name   : WheelProperties::fromJson
 * Description     : Validate exact typed fields, canonical image bytes and complete permission objects atomically.
 * Input Parameters: json, output, error
 * Return Value    : True for a complete valid editor record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool WheelProperties::fromJson(const QJsonObject &json, WheelProperties *output, QString *error) //20261112-0 add
{
    WheelProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject image;
    QJsonObject::const_iterator field;
    QByteArray encoded;
    QByteArray::FromBase64Result decoded;
    double number;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("滚轮字段缺失或未知。"); } return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 255) { return false; } }
    }
#define READ_WHEEL_BOOL(name, initial) candidate.name = json.value(QStringLiteral(#name)).toBool();
    WHEEL_BOOL_FIELDS(READ_WHEEL_BOOL)
#undef READ_WHEEL_BOOL
#define READ_WHEEL_INT(name, initial) candidate.name = json.value(QStringLiteral(#name)).toInt();
    WHEEL_INT_FIELDS(READ_WHEEL_INT)
#undef READ_WHEEL_INT
#define READ_WHEEL_STRING(name, initial) candidate.name = json.value(QStringLiteral(#name)).toString();
    WHEEL_STRING_FIELDS(READ_WHEEL_STRING)
#undef READ_WHEEL_STRING
    image = json.value(QStringLiteral("highlight")).toObject();
    if (image.keys() != candidate.highlight.toJson().keys() || !image.value(QStringLiteral("picture")).isString() || !image.value(QStringLiteral("picturePng")).isString() || image.value(QStringLiteral("picturePng")).toString().size() > 24 * 1024 * 1024) { return false; }
    encoded = image.value(QStringLiteral("picturePng")).toString().toLatin1(); decoded = QByteArray::fromBase64Encoding(encoded, QByteArray::AbortOnBase64DecodingErrors);
    if (!decoded || decoded.decoded.toBase64() != encoded) { return false; }
    candidate.highlight.picture = image.value(QStringLiteral("picture")).toString(); candidate.highlight.picturePng = decoded.decoded;
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !StaticTextProperties::fromJson(json.value(QStringLiteral("touch")).toObject(), &candidate.touch, error) || candidate.touch.toJson() != json.value(QStringLiteral("touch")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : WheelProperties::validate
 * Description     : Enforce measured U16 fields and menu bounds plus explicit Lite nonzero-step and ordered-range guards.
 * Input Parameters: error
 * Return Value    : True for the declared editor slice.
 * Date            : 2026.11.12
 ******************************************************************************/
bool WheelProperties::validate(QString *error) const //20261112-0 add
{
    QRegularExpression integer(QStringLiteral("^[0-9]{1,15}$"));
    QString text;
    QString color;
    qulonglong value;
    bool ok;
    if (sensitivity < 1 || sensitivity > 99 || opacity < 0 || opacity > 255 || rowCount < 2 || rowCount > 5 || !fontSizes().contains(lowFontSize) || !fontSizes().contains(highFontSize) || address.size() > 1024 || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen) || (touch.visibilityEnabled && touch.bitEnabled && !touchBitChosen))
    { if (error != nullptr) { *error = QStringLiteral("滚轮灵敏度、透明度、行数、字号或活动权限选择无效。"); } return false; }
    Q_FOREACH (text, QStringList({minimum, maximum, step})) { value = text.toULongLong(&ok); if (!integer.match(text).hasMatch() || !ok || value > 65535) { if (error != nullptr) { *error = QStringLiteral("滚轮最小值、最大值和步距必须是0..65535整数。"); } return false; } }
    if (minimum.toUInt() >= maximum.toUInt() || step.toUInt() == 0) { if (error != nullptr) { *error = QStringLiteral("本版要求最小值小于最大值且步距大于0。"); } return false; }
    Q_FOREACH (color, QStringList({background, lowColor, highColor})) { if (color.size() > 32 || !QColor(color).isValid()) { return false; } }
    if (highlight.picture.size() > 1024 || (highlight.picture.isEmpty() && !highlight.picturePng.isEmpty()) || (!highlight.picturePng.isEmpty() && !validateButtonImage(highlight.picturePng)) || (!highlight.picture.isEmpty() && highlight.picturePng.isEmpty() && (!highlight.picture.startsWith(QStringLiteral(":/system-gallery/")) || !QFile::exists(highlight.picture)))) { if (error != nullptr) { *error = QStringLiteral("滚轮高亮底图无效。"); } return false; }
    return visibility.validate(error) && touch.validate(error);
}

/******************************************************************************
 * Function Name   : WheelProperties::validateAddresses
 * Description     : Check the single U16 operation carrier and independent full-width visibility/touch conditions without device writes.
 * Input Parameters: connections, error
 * Return Value    : True for all active carrier addresses.
 * Date            : 2026.11.12
 ******************************************************************************/
bool WheelProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress parsed;
    ScreenButtonProperties shared;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections; context.role = PlcAddressRole::Word;
    if (!parseQualifiedPlcAddressText(address, context, catalog, &parsed, error)) { return false; }
    shared.common.indicator = false; shared.common.visibility = visibility; shared.common.touch = touch; return shared.validateAddresses(connections, error);
}
