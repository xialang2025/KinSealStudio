#include "gif_display_properties.h"
#include <QBuffer>
#include <QImageReader>

/******************************************************************************
 * Function Name   : validateGifData
 * Description     : Decode every bounded GIF frame without rewriting delays, disposal, palette or loop bytes.
 * Input Parameters: data, error
 * Return Value    : True for a complete GIF within the explicit Lite resource limits.
 * Date            : 2026.11.12
 ******************************************************************************/
bool validateGifData(const QByteArray &data, QString *error) //20261112-0 add
{
    QBuffer buffer;
    QImageReader reader;
    QImage frame;
    QSize size;
    qint64 pixels = 0;
    int count;
    int index;
    if (data.isEmpty() || data.size() > 1024 * 1024 || (!data.startsWith("GIF87a") && !data.startsWith("GIF89a")))
    { if (error != nullptr) { *error = QStringLiteral("GIF必须为原始GIF文件且不超过1 MiB（Lite保护上限）。"); } return false; }
    buffer.setData(data); buffer.open(QIODevice::ReadOnly); reader.setDevice(&buffer); reader.setFormat(QByteArrayLiteral("gif"));
    size = reader.size(); count = reader.imageCount();
    if (size.isEmpty() || size.width() > 4096 || size.height() > 4096 || count < 1 || count > 512 || !data.endsWith(char(0x3b)))
    { if (error != nullptr) { *error = QStringLiteral("GIF尺寸、帧数超出Lite保护范围或文件不完整。"); } return false; }
    for (index = 0; index < count; ++index)
    {
        pixels += qint64(size.width()) * size.height();
        if (pixels > 16 * 1024 * 1024) { if (error != nullptr) { *error = QStringLiteral("GIF累计解码超过64 MiB保护上限。"); } return false; }
        frame = reader.read();
        if (frame.isNull() || frame.size() != size)
        { if (error != nullptr) { *error = QStringLiteral("GIF包含无法完整解码的帧。"); } return false; }
    }
    return true;
}

/******************************************************************************
 * Function Name   : gifFirstFrame
 * Description     : Read the editor canvas poster without flattening the saved animation resource.
 * Input Parameters: data
 * Return Value    : The actual first frame or an empty image.
 * Date            : 2026.11.12
 ******************************************************************************/
QImage gifFirstFrame(const QByteArray &data) //20261112-0 add
{
    QBuffer buffer;
    QImageReader reader;
    buffer.setData(data); buffer.open(QIODevice::ReadOnly); reader.setDevice(&buffer); reader.setFormat(QByteArrayLiteral("gif"));
    return reader.read();
}

/******************************************************************************
 * Function Name   : GifDisplayProperties::toJson
 * Description     : Persist raw GIF bytes and independent playback and visibility drafts.
 * Input Parameters: None
 * Return Value    : Complete editor record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject GifDisplayProperties::toJson() const //20261112-0 add
{
    return {{QStringLiteral("animation"), animation}, {QStringLiteral("gifData"), QString::fromLatin1(gifData.toBase64())},
            {QStringLiteral("bitControl"), bitControl}, {QStringLiteral("controlAddress"), controlAddress},
            {QStringLiteral("activeState"), activeState}, {QStringLiteral("visibility"), visibility.toJson()}};
}

/******************************************************************************
 * Function Name   : GifDisplayProperties::fromJson
 * Description     : Reject unknown, missing or mistyped fields before replacing an animation record.
 * Input Parameters: json, output, error
 * Return Value    : True for a complete validated record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool GifDisplayProperties::fromJson(const QJsonObject &json, GifDisplayProperties *output, QString *error) //20261112-0 add
{
    GifDisplayProperties candidate;
    QJsonObject expected = candidate.toJson();
    QJsonObject::const_iterator field;
    QString encoded;
    if (output == nullptr || json.keys() != expected.keys())
    { if (error != nullptr) { *error = QStringLiteral("GIF字段缺失、未知或接收位置为空。"); } return false; }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != expected.value(field.key()).type())
        { if (error != nullptr) { *error = QStringLiteral("GIF字段类型错误。"); } return false; }
    }
    encoded = json.value(QStringLiteral("gifData")).toString();
    if (encoded.size() > ((1024 * 1024 + 2) / 3) * 4)
    { if (error != nullptr) { *error = QStringLiteral("GIF编码超过1 MiB保护上限。"); } return false; }
    candidate.gifData = QByteArray::fromBase64(encoded.toLatin1());
    if (QString::fromLatin1(candidate.gifData.toBase64()) != encoded ||
        (json.value(QStringLiteral("activeState")).toDouble() != 0 && json.value(QStringLiteral("activeState")).toDouble() != 1))
    { if (error != nullptr) { *error = QStringLiteral("GIF编码或有效状态无效。"); } return false; }
    candidate.animation = json.value(QStringLiteral("animation")).toString(); candidate.bitControl = json.value(QStringLiteral("bitControl")).toBool();
    candidate.controlAddress = json.value(QStringLiteral("controlAddress")).toString(); candidate.activeState = json.value(QStringLiteral("activeState")).toInt();
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) ||
        candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : GifDisplayProperties::validate
 * Description     : Require a named intact animation and preserve disabled playback-address drafts.
 * Input Parameters: error
 * Return Value    : True for a savable GIF control.
 * Date            : 2026.11.12
 ******************************************************************************/
bool GifDisplayProperties::validate(QString *error) const //20261112-0 add
{
    if (animation.trimmed().isEmpty() || animation.size() > 256 || controlAddress.size() > 256 || activeState < 0 || activeState > 1 || !validateGifData(gifData))
    { if (error != nullptr) { *error = QStringLiteral("没有选择图片或图片已损坏！"); } return false; }
    return visibility.validate(error);
}

/******************************************************************************
 * Function Name   : GifDisplayProperties::validateAddresses
 * Description     : Validate active playback-bit and visibility reads with their actual word spans.
 * Input Parameters: connections, error
 * Return Value    : True for supported configured active addresses.
 * Date            : 2026.11.12
 ******************************************************************************/
bool GifDisplayProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress address;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = connections; context.role = PlcAddressRole::Bit; context.transferCount = 1U;
    if (bitControl && !parseQualifiedPlcAddressText(controlAddress, context, catalog, &address, error)) { return false; }
    if (visibility.visibilityEnabled && visibility.bitEnabled && !parseQualifiedPlcAddressText(visibility.bitAddress, context, catalog, &address, error)) { return false; }
    context.role = PlcAddressRole::Word; context.transferCount = visibility.dataType == 0 || visibility.dataType == 2 ? 1U : 2U;
    if (visibility.visibilityEnabled && visibility.wordEnabled && !parseQualifiedPlcAddressText(visibility.wordAddress, context, catalog, &address, error)) { return false; }
    return true;
}
