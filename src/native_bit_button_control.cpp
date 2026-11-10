#include "native_bit_button_control.h"

#include <QRegularExpression>

#include <limits>
#include <cstring>
#include <cmath>

/******************************************************************************
 * Function Name   : controlError
 * Description     : Report a rejected native field without replacing the caller's record.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.09
 ******************************************************************************/
static bool controlError(QString *error, const QString &message) //20261109-0 add
{
    if (error != nullptr)
    {
        *error = message;
    }
    return false;
}

/******************************************************************************
 * Function Name   : writeControlWord
 * Description     : Write a little-endian 16-bit firmware field without host alignment assumptions.
 * Input Parameters: bytes, offset, value
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
static void writeControlWord(QByteArray *bytes, int offset, u16 value) //20261109-0 add
{
    (*bytes)[offset] = char(value & 0xff);
    (*bytes)[offset + 1] = char((value >> 8) & 0xff);
}

/******************************************************************************
 * Function Name   : writeControlDword
 * Description     : Write a little-endian 32-bit firmware field independently of the host ABI.
 * Input Parameters: bytes, offset, value
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
static void writeControlDword(QByteArray *bytes, int offset, u32 value) //20261109-0 add
{
    int index;

    for (index = 0; index < 4; ++index)
    {
        (*bytes)[offset + index] = char((value >> (index * 8)) & 0xff);
    }
}

/******************************************************************************
 * Function Name   : encodeControlAddress
 * Description     : Encode local addresses or request a complete verified external record from the caller.
 * Input Parameters: address, wordAddress, byte2, toggle, options, offset, control, error
 * Return Value    : True for a bounded local address or a complete external record.
 * Date            : 2026.11.09
 ******************************************************************************/
static bool encodeControlAddress(const QString &address, bool wordAddress, u8 byte2,
                                bool toggle, const NativeBitButtonControlOptions &options,
                                int offset, QByteArray *control,
                                QString *error) //20261109-0 add
{
    QRegularExpression expression;
    QRegularExpressionMatch match;
    unsigned long long value;
    bool valid;
    QByteArray external;
    QString externalError;

    if (address.trimmed().startsWith(QLatin1Char('@')))
    {
        if (!options.externalAddressEncoder)
        {
            return controlError(error, QStringLiteral("外部 PLC 地址缺少已验证的设备编码器：%1").arg(address));
        }
        if (!options.externalAddressEncoder(address, wordAddress, toggle, &external, &externalError))
        {
            return controlError(error, externalError.isEmpty() ?
                                QStringLiteral("外部 PLC 地址未通过设备编码验证：%1").arg(address) : externalError);
        }
        if (external.size() != 36)
        {
            return controlError(error, QStringLiteral("外部 PLC 地址编码器必须返回完整的 36 字节记录：%1").arg(address));
        }
        control->replace(offset, 36, external);
        return true;
    }

    expression = QRegularExpression(wordAddress ? QStringLiteral("^LW([0-9]+)$") :
                                                  QStringLiteral("^LB([0-9]+)$"),
                                    QRegularExpression::CaseInsensitiveOption);
    match = expression.match(address.trimmed());
    valid = false;
    value = match.hasMatch() ? match.captured(1).toULongLong(&valid, 10) : 0;
    if (!valid || value > static_cast<unsigned long long>(std::numeric_limits<int>::max()))
    {
        return controlError(error, QStringLiteral("位按钮设备编译仅支持十进制内部 %1 地址：%2")
                            .arg(wordAddress ? QStringLiteral("LW") : QStringLiteral("LB"), address));
    }

    //20261109-0 add: Native local records assign only bytes 0, 1, 2, 5 and 8..11.
    (*control)[offset] = 0;
    (*control)[offset + 1] = char(wordAddress ? 0x57 : 0x42);
    (*control)[offset + 2] = char(byte2);
    (*control)[offset + 5] = char(toggle ? 1 : 0);
    writeControlDword(control, offset + 8, u32(value));
    return true;
}

/******************************************************************************
 * Function Name   : writeControlDouble
 * Description     : Store the native IEEE-754 comparison value in little-endian byte order.
 * Input Parameters: control, offset, value
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
static void writeControlDouble(QByteArray *control, int offset, double value) //20261109-0 add
{
    u8 bytes[sizeof(double)];
    int index;

    static_assert(sizeof(double) == 8, "The native condition format requires an eight-byte double.");
    std::memcpy(bytes, &value, sizeof(value));
    for (index = 0; index < 8; ++index)
    {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        (*control)[offset + index] = char(bytes[7 - index]);
#else
        (*control)[offset + index] = char(bytes[index]);
#endif
    }
}

/******************************************************************************
 * Function Name   : encodeAccessCondition
 * Description     : Encode native shared touch or visibility fields verified against both original controls.
 * Input Parameters: condition, visibility, options, control, error
 * Return Value    : True when every enabled condition has a verified address and valid comparison.
 * Date            : 2026.11.09
 ******************************************************************************/
static bool encodeAccessCondition(const StaticTextProperties &condition, bool visibility,
                                   const NativeBitButtonControlOptions &options,
                                   QByteArray *control, QString *error) //20261109-0 add
{
    const u8 dataTypes[5] = {4, 5, 2, 3, 8};
    int enabledOffset;
    int flagsOffset;
    int stateOffset;
    int levelOffset;
    int comparisonOffset;
    int typeOffset;
    int valueOffset;
    int bitAddressOffset;
    int wordAddressOffset;
    u8 flags;
    double value;
    double tolerance;

    if (!condition.visibilityEnabled)
    {
        return true;
    }
    enabledOffset = visibility ? 0x15 : 0x14;
    flagsOffset = visibility ? 0x22 : 0x21;
    stateOffset = visibility ? 0x1e : 0x1c;
    levelOffset = visibility ? 0x1f : 0x1d;
    comparisonOffset = visibility ? 0x1b : 0x1a;
    typeOffset = visibility ? 0x18 : 0x17;
    valueOffset = visibility ? 0x34 : 0x24;
    bitAddressOffset = visibility ? 0x94 : 0x4c;
    wordAddressOffset = visibility ? 0xdc : 0xb8;
    flags = u8((condition.bitEnabled ? 4 : 0) | (condition.wordEnabled ? 2 : 0) |
               (condition.levelEnabled ? 1 : 0));
    (*control)[enabledOffset] = 1;
    (*control)[flagsOffset] = char(flags);
    if (condition.bitEnabled)
    {
        (*control)[stateOffset] = char(condition.bitState ? 1 : 0);
        if (!encodeControlAddress(condition.bitAddress, false, 0, false, options,
                                bitAddressOffset, control, error))
        {
            return false;
        }
    }
    if (condition.levelEnabled)
    {
        (*control)[levelOffset] = char(condition.minimumLevel - 1);
    }
    if (condition.wordEnabled)
    {
        value = condition.compareValue.toDouble();
        tolerance = condition.tolerance.toDouble();
        (*control)[typeOffset] = char(dataTypes[condition.dataType]);
        (*control)[comparisonOffset] = char(condition.comparison);
        writeControlDouble(control, valueOffset, value);
        writeControlDouble(control, valueOffset + 8, tolerance);
        if (!encodeControlAddress(condition.wordAddress, true, 0, false, options,
                                wordAddressOffset, control, error))
        {
            return false;
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : encodeNotification
 * Description     : Encode the original shared notification block without discarding a second active address.
 * Input Parameters: properties, options, control, error
 * Return Value    : True when the enabled notification has a verified native representation.
 * Date            : 2026.11.09
 ******************************************************************************/
static bool encodeNotification(const BitButtonProperties &properties,
                                const NativeBitButtonControlOptions &options, QByteArray *control,
                                QString *error) //20261109-0 add
{
    const u8 dataTypes[5] = {4, 5, 2, 3, 8};
    double value;
    double minimum;
    double maximum;
    bool valid;

    if (!properties.notice)
    {
        return true;
    }
    if (properties.noticeBit && properties.noticeWord)
    {
        return controlError(error, QStringLiteral("设备通知记录只有一个地址字段，不能同时无损编译位通知和字通知。"));
    }
    (*control)[0x16] = 1;
    (*control)[0x23] = char(properties.noticeBit ? 1 : (properties.noticeWord ? 2 : 0));
    if (properties.noticeBit)
    {
        (*control)[0x20] = char(properties.noticeBitValue ? 1 : 0);
        if (!encodeControlAddress(properties.noticeBitAddress, false, 0, false, options, 0x70, control, error))
        {
            return false;
        }
        //20261109-0 add: Native notification bit records set rel6 before copying; rel5 remains zero.
        (*control)[0x76] = 1;
    }
    if (properties.noticeWord)
    {
        valid = false;
        value = properties.noticeWordValue.toDouble(&valid);
        minimum = properties.noticeWordType < 2 ? 0 : properties.noticeWordType == 2 ? -32768.0 :
                  properties.noticeWordType == 3 ? -2147483648.0 : -std::numeric_limits<float>::max();
        maximum = properties.noticeWordType == 0 ? 65535.0 : properties.noticeWordType == 1 ? 4294967295.0 :
                  properties.noticeWordType == 2 ? 32767.0 : properties.noticeWordType == 3 ? 2147483647.0 :
                  std::numeric_limits<float>::max();
        if (!valid || !std::isfinite(value) || value < minimum || value > maximum ||
            (properties.noticeWordType < 4 && std::floor(value) != value))
        {
            return controlError(error, QStringLiteral("位按钮字通知值不是所选数据类型可表示的有限数值。"));
        }
        (*control)[0x19] = char(dataTypes[properties.noticeWordType]);
        writeControlDouble(control, 0x44, value);
        if (!encodeControlAddress(properties.noticeWordAddress, true, 0, false, options, 0x70, control, error))
        {
            return false;
        }
        //20261109-0 add: The shared notification address carries its native numeric type in rel6.
        (*control)[0x76] = char(dataTypes[properties.noticeWordType]);
    }
    return true;
}

/******************************************************************************
 * Function Name   : buildNativeBitButtonControl
 * Description     : Construct the observed 640-byte record while clearing native stack padding.
 * Input Parameters: object, options, control, error
 * Return Value    : True when all enabled properties have a verified native encoding.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildNativeBitButtonControl(const HmiObject &object,
                                 const NativeBitButtonControlOptions &options,
                                 QByteArray *control, QString *error) //20261109-0 add
{
    const BitButtonProperties *properties;
    QByteArray encoded;
    QString monitorAddress;
    unsigned long long expectedPixelBytes;
    unsigned long long pixelEnd;
    u32 monitorMode;
    int stateCount;
    bool wordMonitor;
    bool sameMonitor;

    if (control == nullptr || object.type != QStringLiteral("switch") ||
        !object.hasBitButtonProperties)
    {
        return controlError(error, QStringLiteral("设备控制记录需要完整的位按钮对象。"));
    }
    properties = &object.bitButton;
    if (!properties->validate(error))
    {
        return false;
    }
    if (object.geometry.x() < 0 || object.geometry.y() < 0 ||
        object.geometry.x() > 65535 || object.geometry.y() > 65535 ||
        object.geometry.width() < 1 || object.geometry.height() < 1 ||
        object.geometry.width() > 65535 || object.geometry.height() > 65535)
    {
        return controlError(error, QStringLiteral("位按钮位置和尺寸超出设备的 16 位坐标范围。"));
    }
    if (properties->states[0].effect3d != properties->states[1].effect3d ||
        properties->states[0].transparent != properties->states[1].transparent)
    {
        return controlError(error, QStringLiteral("设备位按钮的立体与透明属性为全局字段，两个状态必须设置一致。"));
    }
    if (properties->macroEnabled && (options.macroIndex < 0 || options.macroIndex > 99 ||
                                     properties->macroName.isEmpty()))
    {
        return controlError(error, QStringLiteral("位按钮宏指令需要已经编译的有效宏索引。"));
    }
    if (properties->pressTimeEnabled && properties->pressTime > 655)
    {
        return controlError(error, QStringLiteral("启用的位按钮按压时间超过 16 位毫秒字段可表示范围（最大 655）。"));
    }
    if (options.languageCount < 1 || options.languageCount > 8)
    {
        return controlError(error, QStringLiteral("位按钮设备编译的语言数量必须为1至8。"));
    }

    stateCount = properties->indicator && properties->dualMonitor ? 2 : 1;
    //20261109-0 change: Dual mode emits ON/OFF; disabled and single monitoring emit only source row0 OFF.
    stateCount += properties->states[0].flash ? 1 : 0;
    if (properties->indicator && properties->dualMonitor && properties->states[1].flash)
    {
        ++stateCount;
    }
    expectedPixelBytes = static_cast<unsigned long long>(object.geometry.width()) *
                         static_cast<unsigned long long>(object.geometry.height()) *
                         2 * static_cast<unsigned long long>(stateCount) *
                         static_cast<unsigned long long>(options.languageCount); //20261109-0 change: Each language owns a complete ordered state/flash image set.
    pixelEnd = static_cast<unsigned long long>(options.uncompressedPixelOffset) +
               options.uncompressedPixelBytes;
    if (expectedPixelBytes > std::numeric_limits<u32>::max() ||
        options.uncompressedPixelBytes != expectedPixelBytes ||
        pixelEnd > std::numeric_limits<u32>::max())
    {
        return controlError(error, QStringLiteral("位按钮解压像素偏移或长度与状态数量不一致。"));
    }

    //20261109-0 add: Native 0x69e420 copies a partly initialized 448-byte object into this slot.
    encoded = QByteArray(640, char(0));
    encoded[0] = char(0x0a);
    encoded[1] = char(properties->macroEnabled ? 1 : 0);
    encoded[2] = char(properties->macroEnabled ? options.macroIndex : 0);
    writeControlWord(&encoded, 4, u16(object.geometry.x()));
    writeControlWord(&encoded, 6, u16(object.geometry.y()));
    writeControlWord(&encoded, 8, u16(object.geometry.width()));
    writeControlWord(&encoded, 10, u16(object.geometry.height()));
    writeControlDword(&encoded, 0x0c, options.uncompressedPixelOffset);
    writeControlDword(&encoded, 0x10, options.uncompressedPixelBytes);
    if (!encodeAccessCondition(properties->touch, false, options, &encoded, error) ||
        !encodeAccessCondition(properties->visibility, true, options, &encoded, error) ||
        !encodeNotification(*properties, options, &encoded, error))
    {
        return false;
    }
    //20261109-0 add: Original UI samples distinguish global appearance from the two per-state flash flags.
    encoded[0x108] = char(properties->states[0].effect3d ? 1 : 0);
    encoded[0x109] = char(properties->states[0].transparent ? 1 : 0);
    encoded[0x10a] = char(properties->states[0].flash ? 1 : 0);
    encoded[0x10b] = char(properties->states[1].flash ? 1 : 0);
    writeControlDword(&encoded, 0x114, u32(properties->operation));
    if (properties->operation == 0)
    {
        //20261109-0 add: The original set operation writes both native bit-value flags as well as its all-one value.
        encoded[0x10c] = 1;
        encoded[0x10d] = 1;
        writeControlDword(&encoded, 0x110, 0xffffffffU);
    }
    if (!encodeControlAddress(object.address, false, options.addressByte2,
                              properties->operation == 3, options, 0x118, &encoded, error))
    {
        return false;
    }

    monitorMode = properties->indicator ? (properties->dualMonitor ? 1U : 2U) : 0U;
    writeControlDword(&encoded, 0x184, monitorMode);
    if (monitorMode != 0)
    {
        //20261109-0 add: Native single-state monitoring always uses a word source.
        sameMonitor = properties->dualMonitor && properties->monitorSame;
        wordMonitor = !properties->dualMonitor || (!sameMonitor && properties->monitorType == 1);
        monitorAddress = sameMonitor ? object.address : (wordMonitor ? properties->monitorWordAddress :
                                                                       properties->monitorBitAddress);
        encoded[0x180] = char(wordMonitor ? 1 : 0);
        encoded[0x181] = char(sameMonitor ? 1 : 0);
        writeControlDword(&encoded, 0x188, u32(properties->bitIndex));
        //20261109-0 add: Both original monitor modes retain the two logical states at source270/output190.
        writeControlDword(&encoded, 0x190, 2U);
        if (!encodeControlAddress(monitorAddress, wordMonitor,
                                sameMonitor ? options.addressByte2 : options.monitorAddressByte2,
                                false, options, 0x198, &encoded, error))
        {
            return false;
        }
    }
    encoded[0x1bd] = char(properties->pressTimeEnabled ? 1 : 0);
    writeControlWord(&encoded, 0x1be,
                     u16(u32(properties->pressTime <= 655 ? properties->pressTime : 255) * 100U));
    *control = encoded;
    if (error != nullptr)
    {
        error->clear();
    }
    return true;
}
