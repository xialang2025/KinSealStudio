#include "native_static_text_control.h"

#include <QRegularExpression>
#include <QtEndian>

#include <cstring>
#include <limits>

namespace
{
/******************************************************************************
 * Function Name   : textControlError
 * Description     : Return a concrete encoding error without replacing the caller's record.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.09
 ******************************************************************************/
bool textControlError(QString *error, const QString &message) //20261109-0 add
{
    if (error != nullptr)
    {
        *error = message;
    }
    return false;
}

/******************************************************************************
 * Function Name   : writeTextControlDouble
 * Description     : Store one visibility comparison value as a little-endian IEEE-754 double.
 * Input Parameters: bytes, offset, value
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void writeTextControlDouble(QByteArray *bytes, int offset, double value) //20261109-0 add
{
    u8 raw[sizeof(double)];
    int index;

    static_assert(sizeof(double) == 8, "Native visibility values require eight-byte doubles.");
    std::memcpy(raw, &value, sizeof(value));
    for (index = 0; index < 8; ++index)
    {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        (*bytes)[offset + index] = char(raw[7 - index]);
#else
        (*bytes)[offset + index] = char(raw[index]);
#endif
    }
}

/******************************************************************************
 * Function Name   : encodeTextConditionAddress
 * Description     : Encode one local visibility address or obtain a complete verified external slot.
 * Input Parameters: address, word, byte2, options, offset, bytes, error
 * Return Value    : True when the active address has a complete native representation.
 * Date            : 2026.11.09
 ******************************************************************************/
bool encodeTextConditionAddress(const QString &address, bool word, u8 byte2,
                                const NativeStaticTextControlOptions &options,
                                int offset, QByteArray *bytes, QString *error) //20261109-0 add
{
    QRegularExpression expression;
    QRegularExpressionMatch match;
    QByteArray encoded;
    QString diagnostic;
    unsigned long long number;
    bool valid;

    if (address.trimmed().startsWith(QLatin1Char('@')))
    {
        if (!options.externalAddressEncoder)
        {
            return textControlError(error, QStringLiteral("静态文本外部显现地址缺少已验证的编码器：%1").arg(address));
        }
        if (!options.externalAddressEncoder(address, word, false, &encoded, &diagnostic))
        {
            return textControlError(error, diagnostic.isEmpty() ?
                                    QStringLiteral("静态文本显现地址编码失败：%1").arg(address) : diagnostic);
        }
        if (encoded.size() != 36)
        {
            return textControlError(error, QStringLiteral("静态文本显现地址必须返回完整的36字节记录。"));
        }
        bytes->replace(offset, 36, encoded);
        return true;
    }
    expression = QRegularExpression(word ? QStringLiteral("^LW([0-9]+)$") :
                                           QStringLiteral("^LB([0-9]+)$"),
                                    QRegularExpression::CaseInsensitiveOption);
    match = expression.match(address.trimmed());
    valid = false;
    number = match.hasMatch() ? match.captured(1).toULongLong(&valid) : 0;
    if (!valid || number > static_cast<unsigned long long>(std::numeric_limits<int>::max()))
    {
        return textControlError(error, QStringLiteral("静态文本显现需要有效的十进制%1地址：%2")
                                .arg(word ? QStringLiteral("LW") : QStringLiteral("LB"), address));
    }
    (*bytes)[offset + 1] = char(word ? 0x57 : 0x42);
    (*bytes)[offset + 2] = char(byte2);
    qToLittleEndian<u32>(u32(number), bytes->data() + offset + 8);
    return true;
}

/******************************************************************************
 * Function Name   : encodeTextVisibility
 * Description     : Write the shared visibility block recovered from the original type28 writer.
 * Input Parameters: condition, options, bytes, error
 * Return Value    : True when all enabled bit, level and word gates are encoded.
 * Date            : 2026.11.09
 ******************************************************************************/
bool encodeTextVisibility(const StaticTextProperties &condition,
                          const NativeStaticTextControlOptions &options,
                          QByteArray *bytes, QString *error) //20261109-0 add
{
    const u8 nativeTypes[5] = {4, 5, 2, 3, 8};

    if (!condition.visibilityEnabled)
    {
        return true;
    }
    (*bytes)[0x15] = 1;
    (*bytes)[0x22] = char((condition.bitEnabled ? 4 : 0) | (condition.wordEnabled ? 2 : 0) |
                         (condition.levelEnabled ? 1 : 0));
    if (condition.bitEnabled)
    {
        (*bytes)[0x1e] = char(condition.bitState ? 1 : 0);
        if (!encodeTextConditionAddress(condition.bitAddress, false, options.bitAddressByte2,
                                        options, 0x94, bytes, error))
        {
            return false;
        }
    }
    if (condition.levelEnabled)
    {
        (*bytes)[0x1f] = char(condition.minimumLevel - 1);
    }
    if (condition.wordEnabled)
    {
        (*bytes)[0x18] = char(nativeTypes[condition.dataType]);
        (*bytes)[0x1b] = char(condition.comparison);
        writeTextControlDouble(bytes, 0x34, condition.compareValue.toDouble());
        writeTextControlDouble(bytes, 0x3c, condition.tolerance.toDouble());
        if (!encodeTextConditionAddress(condition.wordAddress, true, options.wordAddressByte2,
                                        options, 0xdc, bytes, error))
        {
            return false;
        }
    }
    return true;
}
}

/******************************************************************************
 * Function Name   : buildNativeStaticTextControl
 * Description     : Encode verified type28 fields while clearing uninitialized native header residue.
 * Input Parameters: object, options, control, error
 * Return Value    : True with a complete record; the previous record survives every failure.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildNativeStaticTextControl(const HmiObject &object,
                                  const NativeStaticTextControlOptions &options,
                                  QByteArray *control, QString *error) //20261109-0 add
{
    StaticTextProperties condition;
    QByteArray encoded;
    unsigned long long expectedBytes;
    unsigned long long pixelEnd;

    if (control == nullptr || object.type != QStringLiteral("text") || object.hasBitButtonProperties)
    {
        return textControlError(error, QStringLiteral("动态静态文本记录需要文本对象和有效输出。"));
    }
    if (options.languageCount < 1 || options.languageCount > 8 ||
        object.geometry.x() < 0 || object.geometry.y() < 0 ||
        object.geometry.x() > 65535 || object.geometry.y() > 65535 ||
        object.geometry.width() < 1 || object.geometry.height() < 1 ||
        object.geometry.width() > 65535 || object.geometry.height() > 65535)
    {
        return textControlError(error, QStringLiteral("静态文本语言数量必须为1至8，位置和尺寸必须在16位字段范围内。"));
    }
    condition = object.hasStaticTextProperties ? object.staticText : StaticTextProperties();
    if (!condition.validate(error))
    {
        return false;
    }
    expectedBytes = static_cast<unsigned long long>(object.geometry.width()) *
                    static_cast<unsigned long long>(object.geometry.height()) * 2 * options.languageCount;
    pixelEnd = static_cast<unsigned long long>(options.uncompressedPixelOffset) + options.uncompressedPixelBytes;
    if (expectedBytes > std::numeric_limits<u32>::max() || options.uncompressedPixelBytes != expectedBytes ||
        pixelEnd > std::numeric_limits<u32>::max())
    {
        return textControlError(error, QStringLiteral("静态文本解压像素长度或偏移与每语言单图数量不一致。"));
    }
    encoded = QByteArray(640, char(0));
    encoded[0] = 28;
    qToLittleEndian<u16>(u16(object.geometry.x()), encoded.data() + 4);
    qToLittleEndian<u16>(u16(object.geometry.y()), encoded.data() + 6);
    qToLittleEndian<u16>(u16(object.geometry.width()), encoded.data() + 8);
    qToLittleEndian<u16>(u16(object.geometry.height()), encoded.data() + 10);
    qToLittleEndian<u32>(options.uncompressedPixelOffset, encoded.data() + 0x0c);
    qToLittleEndian<u32>(options.uncompressedPixelBytes, encoded.data() + 0x10);
    if (!encodeTextVisibility(condition, options, &encoded, error))
    {
        return false;
    }
    // Original 76ee90 writes u16 zero at100 and the system language count at102;104..27f stay zero.
    qToLittleEndian<u16>(u16(options.languageCount), encoded.data() + 0x102);
    *control = encoded;
    if (error != nullptr)
    {
        error->clear();
    }
    return true;
}
