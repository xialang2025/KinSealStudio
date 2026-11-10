#include "native_word_button_control.h"

#include <QRegularExpression>
#include <QtEndian>

#include <cstring>
#include <limits>

namespace
{
/******************************************************************************
 * Function Name   : wordControlError
 * Description     : Report an encoding failure without replacing caller-owned output.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.09
 ******************************************************************************/
bool wordControlError(QString *error, const QString &message) //20261109-6 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : writeWordControlDouble
 * Description     : Store an original word-operation constant as little-endian f64.
 * Input Parameters: bytes, offset, value
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void writeWordControlDouble(QByteArray *bytes, int offset, double value) //20261109-6 add
{
    u8 raw[sizeof(double)];
    int index;

    static_assert(sizeof(double) == 8, "Native word operations require eight-byte doubles.");
    std::memcpy(raw, &value, sizeof(value));
    for (index = 0; index < 8; ++index) {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        (*bytes)[offset + index] = char(raw[7 - index]);
#else
        (*bytes)[offset + index] = char(raw[index]);
#endif
    }
}

/******************************************************************************
 * Function Name   : encodeWordOperatingAddress
 * Description     : Encode the word slot while keeping its rel5 flag separate from scan collection.
 * Input Parameters: object, options, bytes, error
 * Return Value    : True for one local LW address or a complete external address record.
 * Date            : 2026.11.09
 ******************************************************************************/
bool encodeWordOperatingAddress(const HmiObject &object,
                                const NativeBitButtonControlOptions &options,
                                QByteArray *bytes, QString *error) //20261109-6 add
{
    QRegularExpression expression(QStringLiteral("^LW([0-9]+)$"), QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match;
    QByteArray address;
    QString diagnostic;
    unsigned long long number;
    bool valid;

    address = QByteArray(36, char(0));
    if (object.address.trimmed().startsWith(QLatin1Char('@'))) {
        if (!options.externalAddressEncoder ||
            !options.externalAddressEncoder(object.address, true, false, &address, &diagnostic)) {
            return wordControlError(error, diagnostic.isEmpty() ?
                                    QStringLiteral("字按钮外部字地址缺少已验证的设备编码。") : diagnostic);
        }
        if (address.size() != 36) {
            return wordControlError(error, QStringLiteral("字按钮外部地址编码必须返回完整36字节。"));
        }
    } else {
        match = expression.match(object.address.trimmed());
        valid = false;
        number = match.hasMatch() ? match.captured(1).toULongLong(&valid, 10) : 0;
        if (!valid || number > static_cast<unsigned long long>(std::numeric_limits<int>::max())) {
            return wordControlError(error, QStringLiteral("字按钮需要十进制LW地址或已验证的外部字地址。"));
        }
        address[1] = char(0x57);
        address[2] = char(options.addressByte2);
        qToLittleEndian<u32>(u32(number), address.data() + 8);
    }
    // The original subtype1 writer sets rel5 for set/keypad/password, not for read-modify-write operations.
    address[5] = char(object.wordButton.operation < 3 ? 1 : 0);
    bytes->replace(0x138, 36, address);
    return true;
}
}

/******************************************************************************
 * Function Name   : buildNativeWordButtonControl
 * Description     : Reuse the verified common button header and replace the subtype-specific union.
 * Input Parameters: object, options, control, error
 * Return Value    : True after all active numeric, address and pixel fields are representable.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildNativeWordButtonControl(const HmiObject &object,
                                  const NativeBitButtonControlOptions &options,
                                  QByteArray *control, QString *error) //20261109-6 add
{
    HmiObject common;
    QByteArray encoded;
    const WordButtonProperties *word;

    if (control == nullptr || object.type != QStringLiteral("switch") ||
        !object.hasBitButtonProperties || !object.hasWordButtonProperties) {
        return wordControlError(error, QStringLiteral("设备字按钮记录需要完整公共属性和字操作属性。"));
    }
    word = &object.wordButton;
    if (!word->validate(error)) { return false; }

    common = object;
    common.hasWordButtonProperties = false;
    common.address = QStringLiteral("LB0");
    common.bitButton.operation = 0;
    common.bitButton.indicator = word->pressedEffect;
    common.bitButton.dualMonitor = true;
    common.bitButton.monitorSame = true;
    common.bitButton.monitorType = 0;
    common.bitButton.monitorBitAddress = QStringLiteral("LB0");
    common.bitButton.monitorWordAddress = QStringLiteral("LW0");
    // This temporary bit representation encodes only the common permission/macro/notification header and frame-byte bounds.
    if (!buildNativeBitButtonControl(common, options, &encoded, error)) { return false; }

    encoded.replace(0x100, 8, QByteArray(8, char(0)));
    encoded[0x103] = char(word->pressedEffect ? 1 : 0);
    qToLittleEndian<u16>(u16(1), encoded.data() + 0x106);
    encoded.replace(0x10c, 0x1bd - 0x10c, QByteArray(0x1bd - 0x10c, char(0)));
    if (!word->pressedEffect) { encoded[0x10b] = 0; }
    // Unexposed flags at10c/10d and the secondary address at15c contain native constructor residue and remain zero here.
    if (word->usesTotalDigits()) {
        qToLittleEndian<u32>(u32(word->totalDigits), encoded.data() + 0x110);
    }
    if (word->usesDecimalDigits()) {
        qToLittleEndian<u32>(u32(word->decimalDigits), encoded.data() + 0x114);
    }
    qToLittleEndian<u32>(u32(word->operation), encoded.data() + 0x118);
    qToLittleEndian<u32>(u32(word->dataType), encoded.data() + 0x11c);
    if (word->usesConstant()) { writeWordControlDouble(&encoded, 0x120, word->constantValue.toDouble()); }
    if (word->usesMinimum()) { writeWordControlDouble(&encoded, 0x128, word->minimumValue.toDouble()); }
    if (word->usesMaximum()) { writeWordControlDouble(&encoded, 0x130, word->maximumValue.toDouble()); }
    if (!encodeWordOperatingAddress(object, options, &encoded, error)) { return false; }

    *control = encoded;
    if (error != nullptr) { error->clear(); }
    return true;
}
