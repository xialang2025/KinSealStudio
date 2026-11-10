#include "native_lamp_control.h"

#include <QRegularExpression>
#include <QtEndian>

#include <limits>

namespace
{
/******************************************************************************
 * Function Name   : lampControlError
 * Description     : Report an unsupported active field without replacing caller-owned bytes.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.09
 ******************************************************************************/
bool lampControlError(QString *error, const QString &message) //20261109-8 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : encodeLampMonitoringAddress
 * Description     : Encode one local or external monitoring address without a write flag.
 * Input Parameters: address, word, wordCount, options, encoded, error
 * Return Value    : True for a bounded address or a complete verified external record.
 * Date            : 2026.11.09
 ******************************************************************************/
bool encodeLampMonitoringAddress(const QString &address, bool word, u32 wordCount,
                                  const NativeBitButtonControlOptions &options,
                                  QByteArray *encoded, QString *error) //20261109-8 add
{
    QRegularExpression expression;
    QRegularExpressionMatch match;
    QByteArray record(36, char(0));
    QString diagnostic;
    unsigned long long number;
    bool valid;

    if (address.trimmed().startsWith(QLatin1Char('@')))
    {
        if (!options.externalAddressEncoder ||
            !options.externalAddressEncoder(address, word, false, &record, &diagnostic))
        {
            return lampControlError(error, diagnostic.isEmpty() ?
                                    QStringLiteral("指示灯外部监视地址缺少已验证的只读编码。") : diagnostic);
        }
        if (record.size() != 36)
        {
            return lampControlError(error, QStringLiteral("指示灯外部监视地址必须返回完整36字节。"));
        }
    }
    else
    {
        expression = QRegularExpression(word ? QStringLiteral("^LW([0-9]+)$") :
                                               QStringLiteral("^LB([0-9]+)$"),
                                         QRegularExpression::CaseInsensitiveOption);
        match = expression.match(address.trimmed());
        valid = false;
        number = match.hasMatch() ? match.captured(1).toULongLong(&valid, 10) : 0;
        // Match the independently validated editor range0..4096; DWORD monitoring also consumes the following word.
        if (!valid || number > 4096ULL || wordCount > 4097ULL - number)
        {
            return lampControlError(error, QStringLiteral("指示灯监视地址需要范围0至4096内的十进制%1地址，32位值不得跨越末字。")
                                    .arg(word ? QStringLiteral("LW") : QStringLiteral("LB")));
        }
        record[1] = char(word ? 0x57 : 0x42);
        record[2] = char(options.monitorAddressByte2);
        qToLittleEndian<u32>(u32(number), record.data() + 8);
    }
    record[5] = 0; // The original monitoring branch always uses read calltype8 and never toggles its source.
    encoded->replace(0x198, 36, record);
    return true;
}
}

/******************************************************************************
 * Function Name   : buildNativeLampControl
 * Description     : Reuse the verified visibility header while clearing every inactive touch-operation union.
 * Input Parameters: object, options, control, error
 * Return Value    : True after the original subtype4 fields and complete image bounds are encoded.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildNativeLampControl(const HmiObject &object,
                            const NativeBitButtonControlOptions &options,
                            QByteArray *control, QString *error) //20261109-8 add
{
    HmiObject common;
    NativeBitButtonControlOptions commonOptions;
    QByteArray encoded;
    QString monitoringAddress;
    const BitButtonState *appearance;
    unsigned long long frameBytes;
    unsigned long long expectedBytes;
    unsigned long long end;
    u32 effectiveLanguageCount;
    int state;
    int frames;
    bool word;
    bool sameLanguage;

    if (control == nullptr || object.type != QStringLiteral("switch") ||
        !object.hasBitButtonProperties || !object.hasLampProperties || object.hasWordButtonProperties)
    {
        return lampControlError(error, QStringLiteral("设备指示灯记录需要完整公共外观和独立监视属性。"));
    }
    if (!object.lamp.validate(error)) { return false; }
    if (object.lamp.monitorMode < 1)
    {
        return lampControlError(error, QStringLiteral("指示灯必须先确定双态或多态监视，不能下载未确认的临时模式。"));
    }
    if (object.bitButton.macroEnabled || object.bitButton.notice ||
        object.bitButton.touch.visibilityEnabled || object.bitButton.pressTimeEnabled)
    {
        return lampControlError(error, QStringLiteral("原厂指示灯禁用宏和通知，且没有触控条件或按压时间。"));
    }
    if ((object.lamp.monitorMode == 1 && object.bitButton.monitorSame) ||
        (object.lamp.monitorMode == 2 && object.lamp.multiMonitorSame))
    {
        return lampControlError(error, QStringLiteral("指示灯没有操作地址，原厂两处同操作地址选项均不可用。"));
    }
    sameLanguage = object.bitButton.states[0].sameLanguage;
    for (state = 0; state < object.lamp.stateCount(); ++state)
    {
        appearance = object.lamp.state(object.bitButton, state);
        if (appearance == nullptr ||
            appearance->effect3d != object.bitButton.states[0].effect3d ||
            appearance->transparent != object.bitButton.states[0].transparent)
        {
            return lampControlError(error, QStringLiteral("设备指示灯的立体和透明效果是所有活动状态共用的字段。"));
        }
        if (appearance->sameLanguage != sameLanguage)
        {
            return lampControlError(error, QStringLiteral("原厂指示灯的使用同一段文字是全局字段，所有活动状态必须一致。"));
        }
    }

    frames = object.lamp.stateCount();
    if (object.lamp.monitorMode == 1)
    {
        frames += object.bitButton.states[0].flash ? 1 : 0;
        frames += object.bitButton.states[1].flash ? 1 : 0;
    }
    frameBytes = static_cast<unsigned long long>(object.geometry.width()) *
                 static_cast<unsigned long long>(object.geometry.height()) * 2;
    effectiveLanguageCount = sameLanguage ? 1U : options.languageCount;
    expectedBytes = frameBytes * static_cast<unsigned long long>(frames) * effectiveLanguageCount;
    end = static_cast<unsigned long long>(options.uncompressedPixelOffset) + options.uncompressedPixelBytes;
    if (options.languageCount < 1 || options.languageCount > 8 ||
        expectedBytes > std::numeric_limits<u32>::max() ||
        expectedBytes != options.uncompressedPixelBytes || end > std::numeric_limits<u32>::max())
    {
        return lampControlError(error, QStringLiteral("指示灯图像长度、语言数量或解压偏移与活动监视状态不一致。"));
    }

    common = object;
    common.hasLampProperties = false;
    common.address = QStringLiteral("LB0");
    common.bitButton.operation = 0;
    common.bitButton.indicator = true;
    common.bitButton.dualMonitor = true;
    common.bitButton.monitorSame = false;
    common.bitButton.monitorType = 0;
    common.bitButton.monitorBitAddress = QStringLiteral("LB0");
    common.bitButton.states[0].flash = false;
    common.bitButton.states[1].flash = false;
    commonOptions = options;
    commonOptions.languageCount = effectiveLanguageCount;
    commonOptions.uncompressedPixelOffset = 0;
    commonOptions.uncompressedPixelBytes = u32(frameBytes * 2 * effectiveLanguageCount);
    if (!buildNativeBitButtonControl(common, commonOptions, &encoded, error)) { return false; }

    qToLittleEndian<u32>(options.uncompressedPixelOffset, encoded.data() + 0x0c);
    qToLittleEndian<u32>(options.uncompressedPixelBytes, encoded.data() + 0x10);
    encoded.replace(0x100, 8, QByteArray(8, char(0)));
    qToLittleEndian<u16>(u16(4), encoded.data() + 0x106);
    encoded[0x10a] = char(object.bitButton.states[0].flash ? 1 : 0);
    encoded[0x10b] = char(object.bitButton.states[1].flash ? 1 : 0);
    // Subtype4 never assigns the bit/word touch union; native stack residues are not firmware constants.
    encoded.replace(0x10c, 0xb0, QByteArray(0xb0, char(0)));
    qToLittleEndian<u32>(u32(object.lamp.monitorMode), encoded.data() + 0x184);
    qToLittleEndian<u32>(u32(object.lamp.stateCount()), encoded.data() + 0x190);
    if (object.lamp.monitorMode == 2)
    {
        word = true;
        monitoringAddress = object.lamp.multiMonitorAddress;
        qToLittleEndian<u32>(u32(object.lamp.multiDataType), encoded.data() + 0x18c);
    }
    else
    {
        word = object.bitButton.monitorType == 1;
        monitoringAddress = word ? object.bitButton.monitorWordAddress : object.bitButton.monitorBitAddress;
        qToLittleEndian<u32>(u32(object.bitButton.bitIndex), encoded.data() + 0x188);
    }
    encoded[0x180] = char(word ? 1 : 0);
    encoded[0x1bc] = char(sameLanguage ? 1 : 0); // Native source22f/output1bc selects one shared image set for every project language.
    if (!encodeLampMonitoringAddress(monitoringAddress, word,
                                    object.lamp.monitorMode == 2 ? u32(object.lamp.transferWordCount()) : 1U,
                                    options, &encoded, error)) { return false; }
    *control = encoded;
    if (error != nullptr) { error->clear(); }
    return true;
}
