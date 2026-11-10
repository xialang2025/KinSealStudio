#include "native_function_button_control.h"

#include <QtEndian>
#include <limits>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace
{
/******************************************************************************
 * Function Name   : functionControlError
 * Description     : Report a rejected active system-function field without changing output.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.10
 ******************************************************************************/
bool functionControlError(QString *error, const QString &message) //20261110-0 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : encodeNativeKeyboardCharacter
 * Description     : Match the original Qt4 Windows local-codepage first-byte keyboard field.
 * Input Parameters: text, value, error
 * Return Value    : True for an empty or representable single UTF16 character.
 * Date            : 2026.11.10
 ******************************************************************************/
bool encodeNativeKeyboardCharacter(const QString &text, u8 *value, QString *error) //20261110-0 add
{
#ifdef Q_OS_WIN
    char bytes[8] = {};
    wchar_t character;
    int count;
    BOOL substituted = FALSE;
    UINT codepage;
#else
    QByteArray bytes;
#endif

    *value = 0;
    if (text.isEmpty()) { return true; }
    if (text.size() != 1 || text.at(0).isSurrogate())
    {
        return functionControlError(error, QStringLiteral("功能按钮键盘字符必须是单个完整UTF16字符。"));
    }
#ifdef Q_OS_WIN
    character = wchar_t(text.at(0).unicode());
    codepage = GetACP();
    count = WideCharToMultiByte(codepage, 0, &character, 1, bytes, sizeof(bytes), nullptr,
                                codepage == CP_UTF8 ? nullptr : &substituted);
    if (count <= 0 || substituted)
    {
        return functionControlError(error, QStringLiteral("键盘字符不能由原厂Windows本地代码页表示。"));
    }
    *value = u8(bytes[0]);
#else
    // ASCII is portable; the vendor Windows multibyte field must not silently become UTF8 on another host.
    if (text.at(0).unicode() > 127)
    {
        return functionControlError(error, QStringLiteral("非ASCII键盘字符需要原厂Windows本地代码页编码。"));
    }
    *value = u8(text.at(0).unicode());
#endif
    return true;
}
}

/******************************************************************************
 * Function Name   : buildNativeFunctionButtonControl
 * Description     : Reuse shared permissions while replacing every inactive bit/word operation union.
 * Input Parameters: object, options, control, error
 * Return Value    : True after all active function, language, keyboard and image fields are verified.
 * Date            : 2026.11.10
 ******************************************************************************/
bool buildNativeFunctionButtonControl(const HmiObject &object,
                                     const NativeBitButtonControlOptions &options,
                                     QByteArray *control, QString *error) //20261110-0 add
{
    HmiObject common;
    NativeBitButtonControlOptions commonOptions;
    QByteArray encoded;
    unsigned long long frameBytes;
    unsigned long long expectedBytes;
    unsigned long long pixelEnd;
    u32 languages;
    u32 frames;
    u8 character;
    int nativeType;

    if (control == nullptr || object.type != QStringLiteral("switch") ||
        !object.hasBitButtonProperties || !object.hasFunctionButtonProperties ||
        object.hasWordButtonProperties || object.hasLampProperties)
    {
        return functionControlError(error, QStringLiteral("设备功能按钮需要独立功能属性和完整公共外观。"));
    }
    if (!object.functionButton.validate(error)) { return false; }
    if (object.bitButton.macroEnabled)
    {
        return functionControlError(error, QStringLiteral("原厂功能按钮的宏指令组禁用，不能下载启用宏的功能按钮。"));
    }
    if (object.bitButton.pressTime < 0 || object.bitButton.pressTime > 255)
    {
        return functionControlError(error, QStringLiteral("原厂功能按钮按压时间范围为0至255个100毫秒。"));
    }
    if (object.bitButton.states[0].text.fontSize < 1 || object.bitButton.states[0].text.fontSize > 128)
    {
        return functionControlError(error, QStringLiteral("原厂功能按钮字体大小范围为1至128。"));
    }
    if (options.languageCount < 1 || options.languageCount > 8 ||
        (object.functionButton.function == 13 && object.functionButton.type >= int(options.languageCount)))
    {
        return functionControlError(error, QStringLiteral("功能按钮的语言数量或语言切换目标无效。"));
    }
    languages = object.bitButton.states[0].sameLanguage ? 1U : options.languageCount;
    frames = 1U + u32(object.functionButton.pressedEffect ? 1 : 0) +
             u32(object.bitButton.states[0].flash ? 1 : 0);
    frameBytes = static_cast<unsigned long long>(object.geometry.width()) *
                 static_cast<unsigned long long>(object.geometry.height()) * 2;
    expectedBytes = frameBytes * frames * languages;
    pixelEnd = static_cast<unsigned long long>(options.uncompressedPixelOffset) + options.uncompressedPixelBytes;
    if (expectedBytes > std::numeric_limits<u32>::max() || expectedBytes != options.uncompressedPixelBytes ||
        pixelEnd > std::numeric_limits<u32>::max())
    {
        return functionControlError(error, QStringLiteral("功能按钮图像长度或解压偏移与按下、闪烁、语言数量不一致。"));
    }
    character = 0;
    if (object.functionButton.function == 14 && object.functionButton.keyboardKey == 9 &&
        !encodeNativeKeyboardCharacter(object.functionButton.keyboardText, &character, error)) { return false; }

    common = object;
    common.hasFunctionButtonProperties = false;
    common.address = QStringLiteral("LB0");
    common.bitButton.operation = 0;
    common.bitButton.indicator = false;
    common.bitButton.dualMonitor = false;
    common.bitButton.pressTime = qMax(1, object.bitButton.pressTime);
    commonOptions = options;
    commonOptions.languageCount = languages;
    commonOptions.uncompressedPixelOffset = 0;
    commonOptions.uncompressedPixelBytes = u32(frameBytes * (1U + u32(object.bitButton.states[0].flash ? 1 : 0)) * languages);
    if (!buildNativeBitButtonControl(common, commonOptions, &encoded, error)) { return false; }

    qToLittleEndian<u32>(options.uncompressedPixelOffset, encoded.data() + 0x0c);
    qToLittleEndian<u32>(options.uncompressedPixelBytes, encoded.data() + 0x10);
    encoded.replace(0x100, 8, QByteArray(8, char(0)));
    encoded[0x103] = char(object.functionButton.pressedEffect ? 1 : 0);
    qToLittleEndian<u16>(u16(3), encoded.data() + 0x106);
    encoded[0x10b] = 0; // The function editor has one editable flash state; disabled ON residue is never firmware data.
    encoded.replace(0x10c, 0x1bd - 0x10c, QByteArray(0x1bd - 0x10c, char(0)));
    nativeType = FunctionButtonProperties::types(object.functionButton.function, QStringList()).isEmpty() ? -1 : object.functionButton.type;
    if (object.functionButton.function == 13) { nativeType = object.functionButton.type; }
    qToLittleEndian<u32>(u32(object.functionButton.function), encoded.data() + 0x118);
    qToLittleEndian<u32>(u32(nativeType), encoded.data() + 0x11c);
    if (object.functionButton.function == 14)
    {
        encoded[0x120] = char(object.functionButton.keyboardKey);
        encoded[0x121] = char(character);
    }
    encoded[0x1bc] = char(object.bitButton.states[0].sameLanguage ? 1 : 0);
    qToLittleEndian<u16>(u16(object.bitButton.pressTime * 100), encoded.data() + 0x1be);
    *control = encoded;
    if (error != nullptr) { error->clear(); }
    return true;
}
