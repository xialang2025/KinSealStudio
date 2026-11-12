#include "move_track_properties.h"
#include "plc_native_catalog.h"
#include <QRegularExpression>
#include <cmath>

/******************************************************************************
 * Function Name   : MoveTrackProperties::MoveTrackProperties
 * Description     : Initialize the observed silver graphic and separate touch permission level.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
MoveTrackProperties::MoveTrackProperties() //20261112-0 add
{
    appearance.picture = QStringLiteral(":/system-gallery/default/gray.kvg");
    touch.minimumLevel = 2;
}

/******************************************************************************
 * Function Name   : MoveTrackProperties::directions
 * Description     : Use the freshly enumerated V5.1 order instead of the older manual ordering.
 * Input Parameters: None
 * Return Value    : Four direction labels.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList MoveTrackProperties::directions() //20261112-0 add
{
    return {QStringLiteral("向上"), QStringLiteral("向下"), QStringLiteral("向左"), QStringLiteral("向右")};
}

/******************************************************************************
 * Function Name   : MoveTrackProperties::accessDraft
 * Description     : Adapt only permissions and notifications without importing button operations or macros.
 * Input Parameters: None
 * Return Value    : Detached shared-access carrier.
 * Date            : 2026.11.12
 ******************************************************************************/
BitButtonProperties MoveTrackProperties::accessDraft() const //20261112-0 add
{
    BitButtonProperties access;
    access.indicator = false; access.dualMonitor = false; access.monitorSame = false; access.bitIndex = 0; access.pressTime = 0;
    access.visibility = visibility; access.touch = touch; access.notice = notice; access.noticeBit = noticeBit; access.noticeWord = noticeWord;
    access.noticeBitValue = noticeBitValue; access.noticeBitAddress = noticeBitAddress; access.noticeWordAddress = noticeWordAddress;
    access.noticeWordType = noticeWordType; access.noticeWordValue = noticeWordValue;
    return access;
}

/******************************************************************************
 * Function Name   : MoveTrackProperties::toJson
 * Description     : Preserve constant and address drafts independently, including inactive resource and access values.
 * Input Parameters: None
 * Return Value    : Complete typed KSP record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject MoveTrackProperties::toJson() const //20261112-0 add
{
    QJsonObject json;
#define SAVE_TRACK(name, initial) json.insert(QStringLiteral(#name), name);
    TRACK_BOOL_FIELDS(SAVE_TRACK) TRACK_INT_FIELDS(SAVE_TRACK) TRACK_STRING_FIELDS(SAVE_TRACK)
#undef SAVE_TRACK
    json.insert(QStringLiteral("appearance"), appearance.toJson()); json.insert(QStringLiteral("visibility"), visibility.toJson()); json.insert(QStringLiteral("touch"), touch.toJson());
    return json;
}

/******************************************************************************
 * Function Name   : MoveTrackProperties::fromJson
 * Description     : Reject unknown shapes, lossy numbers and malformed image or permission carriers atomically.
 * Input Parameters: json, output, error
 * Return Value    : True for a complete supported editor record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool MoveTrackProperties::fromJson(const QJsonObject &json, MoveTrackProperties *output, QString *error) //20261112-0 add
{
    MoveTrackProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject image;
    QJsonObject::const_iterator field;
    QByteArray encoded;
    QByteArray::FromBase64Result decoded;
    double number;
    if (output == nullptr || json.keys() != shape.keys()) { if (error != nullptr) { *error = QStringLiteral("移动轨迹字段缺失或未知。"); } return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return false; }
        if (field.value().isDouble()) { number = field.value().toDouble(); if (!std::isfinite(number) || std::floor(number) != number || number < 0 || number > 900) { return false; } }
    }
#define READ_TRACK_BOOL(name, initial) candidate.name = json.value(QStringLiteral(#name)).toBool();
    TRACK_BOOL_FIELDS(READ_TRACK_BOOL)
#undef READ_TRACK_BOOL
#define READ_TRACK_INT(name, initial) candidate.name = json.value(QStringLiteral(#name)).toInt();
    TRACK_INT_FIELDS(READ_TRACK_INT)
#undef READ_TRACK_INT
#define READ_TRACK_STRING(name, initial) candidate.name = json.value(QStringLiteral(#name)).toString();
    TRACK_STRING_FIELDS(READ_TRACK_STRING)
#undef READ_TRACK_STRING
    image = json.value(QStringLiteral("appearance")).toObject();
    if (image.keys() != candidate.appearance.toJson().keys() || !image.value(QStringLiteral("picture")).isString() || !image.value(QStringLiteral("picturePng")).isString() || image.value(QStringLiteral("picturePng")).toString().size() > 24 * 1024 * 1024) { return false; }
    encoded = image.value(QStringLiteral("picturePng")).toString().toLatin1(); decoded = QByteArray::fromBase64Encoding(encoded, QByteArray::AbortOnBase64DecodingErrors);
    if (!decoded || decoded.decoded.toBase64() != encoded) { return false; }
    candidate.appearance.picture = image.value(QStringLiteral("picture")).toString(); candidate.appearance.picturePng = decoded.decoded;
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !StaticTextProperties::fromJson(json.value(QStringLiteral("touch")).toObject(), &candidate.touch, error) || candidate.touch.toJson() != json.value(QStringLiteral("touch")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : MoveTrackProperties::validate
 * Description     : Enforce observed menu, distance and scalar bounds while keeping a stated Lite ordered-range guard.
 * Input Parameters: error
 * Return Value    : True for the declared editor slice.
 * Date            : 2026.11.12
 ******************************************************************************/
bool MoveTrackProperties::validate(QString *error) const //20261112-0 add
{
    QRegularExpression integer(QStringLiteral("^[0-9]{1,15}$"));
    ImageDisplayProperties picture;
    ScreenButtonProperties shared;
    QString value;
    bool ok;
    qulonglong number;
    if (direction < 0 || direction > 3 || distance < 0 || distance > 900 || minimum.size() > 256 || maximum.size() > 256 || address.size() > 1024 || minimumAddress.size() > 1024 || maximumAddress.size() > 1024 || (visibility.visibilityEnabled && visibility.bitEnabled && !visibilityBitChosen) || (touch.visibilityEnabled && touch.bitEnabled && !touchBitChosen))
    { if (error != nullptr) { *error = QStringLiteral("移动轨迹方向、距离、范围或活动权限选择无效。"); } return false; }
    if (!dynamicRange)
    {
        Q_FOREACH (value, QStringList({minimum, maximum})) { number = value.toULongLong(&ok); if (!integer.match(value).hasMatch() || !ok || number > 65535) { if (error != nullptr) { *error = QStringLiteral("移动轨迹常量须为0..65535整数。"); } return false; } }
        if (minimum.toUInt() >= maximum.toUInt()) { if (error != nullptr) { *error = QStringLiteral("本版编辑范围要求最小值小于最大值。"); } return false; }
    }
    picture.states[0] = appearance; shared.common = accessDraft();
    return picture.validate(error) && shared.validate(error);
}

/******************************************************************************
 * Function Name   : MoveTrackProperties::validateAddresses
 * Description     : Validate independent word carriers and shared full-width notification or permission addresses.
 * Input Parameters: connections, error
 * Return Value    : True for active carriers; no runtime movement or device writes are simulated.
 * Date            : 2026.11.12
 ******************************************************************************/
bool MoveTrackProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress parsed;
    ScreenButtonProperties shared;
    QString value;
    QStringList carriers = {address};
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections; context.role = PlcAddressRole::Word; context.transferCount = 1U;
    if (dynamicRange) { carriers.append(minimumAddress); carriers.append(maximumAddress); }
    Q_FOREACH (value, carriers) { if (!parseQualifiedPlcAddressText(value, context, catalog, &parsed, error)) { return false; } }
    shared.common = accessDraft(); return shared.validateAddresses(connections, error);
}
