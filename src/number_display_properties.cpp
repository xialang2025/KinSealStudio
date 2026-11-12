#include "number_display_properties.h"
#include "bit_button_properties.h"
#include <QColor>
#include <QFile>
#include <QLocale>
#include <cmath>

namespace {
/******************************************************************************
 * Function Name   : numberError
 * Description     : Report a numeric editor validation failure without publishing partial state.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.12
 ******************************************************************************/
bool numberError(QString *error, const QString &message) //20261112-0 add
{
    if (error != nullptr) { *error = message; } return false;
}

/******************************************************************************
 * Function Name   : numericConstant
 * Description     : Parse a UI constant in its selected radix; VP byte variants retain unevaluated native drafts.
 * Input Parameters: text, type, value
 * Return Value    : True for a finite representable editor constant.
 * Date            : 2026.11.12
 ******************************************************************************/
bool numericConstant(const QString &text, int type, double *value) //20261112-0 add
{
    bool valid = false;
    int base = type == 3 || type == 9 ? 8 : type == 4 || type == 10 ? 16 : type == 11 ? 2 : 10;
    if (text.trimmed().isEmpty()) { return false; }
    if (type == 8) { *value = QLocale::c().toDouble(text, &valid); }
    else if (type == 0 || type == 5) { *value = double(text.toLongLong(&valid, 10)); }
    else { *value = double(text.toULongLong(&valid, base)); }
    if (!valid || !std::isfinite(*value)) { return false; }
    if (type == 0) { return *value >= -32768 && *value <= 32767; }
    if (type == 5) { return *value >= -2147483648.0 && *value <= 2147483647.0; }
    if (type == 2) { return *value <= 9999; }
    if (type == 7) { return *value <= 99999999; }
    if (type == 8) { return std::isfinite(float(*value)); }
    return *value <= (NumberDisplayProperties::wordSpan(type) == 1 ? 65535.0 : 4294967295.0);
}
}

/******************************************************************************
 * Function Name   : NumberDisplayProperties::NumberDisplayProperties
 * Description     : Initialize the distinct native visibility and touch minimum levels.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
NumberDisplayProperties::NumberDisplayProperties() //20261112-0 add
{
    touch.minimumLevel = 2;
}

/******************************************************************************
 * Function Name   : NumberDisplayProperties::dataTypes
 * Description     : Return the measured fourteen UI labels without treating their indices as device enums.
 * Input Parameters: None
 * Return Value    : Native combo order.
 * Date            : 2026.11.12
 ******************************************************************************/
QStringList NumberDisplayProperties::dataTypes() //20261112-0 add
{
    return {QStringLiteral("16位整数"), QStringLiteral("16位正整数"), QStringLiteral("16位BCD码"), QStringLiteral("16位8进制数"), QStringLiteral("16位16进制数"),
            QStringLiteral("32位整数"), QStringLiteral("32位正整数"), QStringLiteral("32位BCD码"), QStringLiteral("32位浮点数"), QStringLiteral("32位8进制数"),
            QStringLiteral("32位16进制数"), QStringLiteral("16位二进制数"), QStringLiteral("*VP高字节"), QStringLiteral("*VP低字节")};
}

/******************************************************************************
 * Function Name   : NumberDisplayProperties::maximumDigits
 * Description     : Return the measured maximum total digits; VP maxima retain the observed binary draft ceiling.
 * Input Parameters: type
 * Return Value    : Bounded editor digit maximum.
 * Date            : 2026.11.12
 ******************************************************************************/
int NumberDisplayProperties::maximumDigits(int type) //20261112-0 add
{
    static const int limits[] = {5, 5, 4, 6, 4, 10, 10, 8, 10, 11, 8, 16, 16, 16};
    return limits[qBound(0, type, 13)];
}

/******************************************************************************
 * Function Name   : NumberDisplayProperties::wordSpan
 * Description     : Use actual sixteen/thirty-two-bit address spans; VP carriers remain editor-only base words.
 * Input Parameters: type
 * Return Value    : Base carrier word count.
 * Date            : 2026.11.12
 ******************************************************************************/
int NumberDisplayProperties::wordSpan(int type) //20261112-0 add
{
    return type >= 5 && type <= 10 ? 2 : 1;
}

/******************************************************************************
 * Function Name   : NumberDisplayProperties::decimalEnabled
 * Description     : Preserve but disable decimal drafts for nondecimal and VP UI types.
 * Input Parameters: type
 * Return Value    : Whether the measured decimal editor is enabled.
 * Date            : 2026.11.12
 ******************************************************************************/
bool NumberDisplayProperties::decimalEnabled(int type) //20261112-0 add
{
    return type >= 0 && type <= 8 && type != 3 && type != 4;
}

/******************************************************************************
 * Function Name   : NumberDisplayProperties::changeDataType
 * Description     : Refresh input limits and clamp total digits without resetting a smaller user digit draft.
 * Input Parameters: type
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void NumberDisplayProperties::changeDataType(int type) //20261112-0 add
{
    static const char *minimums[] = {"-32768", "0", "0", "0", "0", "-2147483648", "0", "0", "-2147483648", "0", "0", "0"};
    static const char *maximums[] = {"32767", "65535", "9999", "177777", "FFFF", "2147483647", "4294967295", "99999999", "2147483647", "17777777777", "FFFFFFFF", "1111111111111111"};
    dataType = qBound(0, type, 13); totalDigits = qBound(1, totalDigits, maximumDigits(dataType));
    if (dataType < 12)
    {
        inputMinimum = QString::fromLatin1(minimums[dataType]); inputMaximum = QString::fromLatin1(maximums[dataType]);
        displayMinimum = inputMinimum; displayMaximum = dataType == 9 ? QStringLiteral("37777777777") : inputMaximum.toLower(); //20261112-0 add: Native 32-bit octal input and display ceilings differ.
    }
}

/******************************************************************************
 * Function Name   : NumberDisplayProperties::toJson
 * Description     : Serialize all typed numeric fields, hidden drafts, artwork and independent permission columns.
 * Input Parameters: None
 * Return Value    : Complete editor-only numeric record.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonObject NumberDisplayProperties::toJson() const //20261112-0 add
{
    QJsonObject json;
#define WRITE_NUMBER(name, initial) json.insert(QStringLiteral(#name), name);
    NUMBER_BOOLEAN_FIELDS(WRITE_NUMBER)
    NUMBER_INTEGER_FIELDS(WRITE_NUMBER)
    NUMBER_STRING_FIELDS(WRITE_NUMBER)
#undef WRITE_NUMBER
    json.insert(QStringLiteral("appearance"), appearance.toJson()); json.insert(QStringLiteral("visibility"), visibility.toJson()); json.insert(QStringLiteral("touch"), touch.toJson()); return json;
}

/******************************************************************************
 * Function Name   : NumberDisplayProperties::fromJson
 * Description     : Reject missing/unknown fields, wrong types, fractional enums and corrupt embedded artwork atomically.
 * Input Parameters: json, output, error
 * Return Value    : True for a complete valid numeric record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool NumberDisplayProperties::fromJson(const QJsonObject &json, NumberDisplayProperties *output, QString *error) //20261112-0 add
{
    NumberDisplayProperties candidate;
    QJsonObject shape = candidate.toJson();
    QJsonObject::const_iterator field;
    QJsonObject appearance;
    QString encoded;
    double value;
    if (output == nullptr || shape.keys() != json.keys()) { return numberError(error, QStringLiteral("数值显示字段缺失、未知或接收位置为空。")); }
    for (field = json.constBegin(); field != json.constEnd(); ++field)
    {
        if (field.value().type() != shape.value(field.key()).type()) { return numberError(error, QStringLiteral("数值显示字段类型错误。")); }
        if (field.value().isDouble())
        {
            value = field.value().toDouble();
            if (!std::isfinite(value) || value != std::floor(value) || value < 0 || value > 65535) { return numberError(error, QStringLiteral("数值显示整数参数无效。")); }
        }
        if (field.value().isString() && field.value().toString().size() > 32767) { return numberError(error, QStringLiteral("数值显示文本字段超限。")); }
    }
#define READ_NUMBER_BOOL(name, initial) candidate.name = json.value(QStringLiteral(#name)).toBool();
    NUMBER_BOOLEAN_FIELDS(READ_NUMBER_BOOL)
#undef READ_NUMBER_BOOL
#define READ_NUMBER_INT(name, initial) candidate.name = json.value(QStringLiteral(#name)).toInt();
    NUMBER_INTEGER_FIELDS(READ_NUMBER_INT)
#undef READ_NUMBER_INT
#define READ_NUMBER_STRING(name, initial) candidate.name = json.value(QStringLiteral(#name)).toString();
    NUMBER_STRING_FIELDS(READ_NUMBER_STRING)
#undef READ_NUMBER_STRING
    appearance = json.value(QStringLiteral("appearance")).toObject();
    if (appearance.keys() != candidate.appearance.toJson().keys() || !appearance.value(QStringLiteral("picture")).isString() || !appearance.value(QStringLiteral("picturePng")).isString()) { return numberError(error, QStringLiteral("数值外形字段无效。")); }
    candidate.appearance.picture = appearance.value(QStringLiteral("picture")).toString(); encoded = appearance.value(QStringLiteral("picturePng")).toString();
    if (encoded.size() > 24 * 1024 * 1024) { return numberError(error, QStringLiteral("数值外形编码超限。")); }
    candidate.appearance.picturePng = QByteArray::fromBase64(encoded.toLatin1());
    if (QString::fromLatin1(candidate.appearance.picturePng.toBase64()) != encoded) { return numberError(error, QStringLiteral("数值外形Base64无效。")); }
    if (!StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &candidate.visibility, error) || candidate.visibility.toJson() != json.value(QStringLiteral("visibility")).toObject() ||
        !StaticTextProperties::fromJson(json.value(QStringLiteral("touch")).toObject(), &candidate.touch, error) || candidate.touch.toJson() != json.value(QStringLiteral("touch")).toObject() || !candidate.validate(error)) { return false; }
    *output = candidate; return true;
}

/******************************************************************************
 * Function Name   : NumberDisplayProperties::validate
 * Description     : Validate active numeric constants and known UI bounds while retaining disabled ranges and address drafts.
 * Input Parameters: error
 * Return Value    : True for a bounded savable numeric editor record.
 * Date            : 2026.11.12
 ******************************************************************************/
bool NumberDisplayProperties::validate(QString *error) const //20261112-0 add
{
    double minimum;
    double maximum;
    double gainValue;
    bool valid;
    StaticTextProperties notification;
    QStringList sizes = {QStringLiteral("16"), QStringLiteral("24"), QStringLiteral("32"), QStringLiteral("40"), QStringLiteral("48"), QStringLiteral("64"), QStringLiteral("96"), QStringLiteral("192")};
    if (dataType < 0 || dataType > 13 || font < 0 || font > 2 || !sizes.contains(QString::number(fontSize)) || totalDigits < 1 || totalDigits > maximumDigits(dataType) || decimalDigits < 0 || decimalDigits > 99 ||
        alignment < 0 || alignment > 2 || adjustment < 0 || adjustment > 2 || opacity < 0 || opacity > 255 || inputRangeSource < 0 || inputRangeSource > 1 || displayRangeSource < 0 || displayRangeSource > 1 ||
        noticeWordType < 0 || noticeWordType > 4 || pressTime < 0 || pressTime > 255 || keyboardX < 0 || keyboardX > 800 || keyboardY < 0 || keyboardY > 480 || macroEnabled || (noticeBit && noticeWord) ||
        !QColor(textColor).isValid() || !QColor(backgroundColor).isValid() || !QColor(lowColor).isValid() || !QColor(highColor).isValid()) { return numberError(error, QStringLiteral("数值显示参数超出已核定编辑范围。")); }
    if (allowInput && inputRangeSource == 0 && dataType < 12 && (!numericConstant(inputMinimum, dataType, &minimum) || !numericConstant(inputMaximum, dataType, &maximum) || minimum > maximum))
    { return numberError(error, QStringLiteral("数值输入范围为空、格式或量程无效，或最小值大于最大值。")); }
    if (displayRange && displayRangeSource == 0 && dataType < 12 && (!numericConstant(displayMinimum, dataType, &minimum) || !numericConstant(displayMaximum, dataType, &maximum) || minimum > maximum))
    { return numberError(error, QStringLiteral("范围显示的低限值或高限值无效。")); }
    if (scaling)
    {
        gainValue = QLocale::c().toDouble(gain, &valid);
        if (!valid || !std::isfinite(gainValue) || gainValue == 0) { return numberError(error, QStringLiteral("缩放增益必须为有限非零数（Lite输入保护）。")); }
        gainValue = QLocale::c().toDouble(offset, &valid); if (!valid || !std::isfinite(gainValue)) { return numberError(error, QStringLiteral("缩放偏移量必须为有限数。")); }
    }
    if (appearance.picture.size() > 1024 || (!appearance.picturePng.isEmpty() && !validateButtonImage(appearance.picturePng)) ||
        (!appearance.picture.isEmpty() && appearance.picturePng.isEmpty() && (!appearance.picture.startsWith(QStringLiteral(":/system-gallery/")) || !QFile::exists(appearance.picture)))) { return numberError(error, QStringLiteral("数值外形图片缺失或已损坏。")); }
    if (!visibility.validate(error) || !touch.validate(error)) { return false; }
    notification.visibilityEnabled = notice && noticeWord && allowInput; notification.wordEnabled = notification.visibilityEnabled; notification.dataType = noticeWordType; notification.compareValue = noticeWordValue;
    return notification.validate(error);
}

/******************************************************************************
 * Function Name   : NumberDisplayProperties::validateAddresses
 * Description     : Validate active read/write/range/offset and permission spans without rewriting inactive drafts.
 * Input Parameters: connections, error
 * Return Value    : True for configured base carriers; runtime offset values and VP interpretation remain unverified.
 * Date            : 2026.11.12
 ******************************************************************************/
bool NumberDisplayProperties::validateAddresses(const QVector<PlcConnection> &connections, QString *error) const //20261112-0 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress address;
    QList<QPair<QString, int>> words;
    QStringList bits;
    QPair<QString, int> word;
    QString bit;
    StaticTextProperties permission;
    int index;
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    words.append({monitorAddress, wordSpan(dataType)}); if (allowInput && !sameMonitor) { words.append({inputAddress, wordSpan(dataType)}); }
    if (offsetEnabled) { words.append({offsetAddress, 1}); }
    if (allowInput && inputRangeSource == 1) { words.append({inputMinimumAddress, wordSpan(dataType)}); words.append({inputMaximumAddress, wordSpan(dataType)}); }
    if (displayRange && displayRangeSource == 1) { words.append({displayMinimumAddress, wordSpan(dataType)}); words.append({displayMaximumAddress, wordSpan(dataType)}); }
    for (index = 0; index < 2; ++index)
    {
        permission = index == 0 ? visibility : touch; if (!permission.visibilityEnabled || (index == 1 && !allowInput)) { continue; }
        if (permission.bitEnabled) { bits.append(permission.bitAddress); }
        if (permission.wordEnabled) { words.append({permission.wordAddress, permission.dataType == 0 || permission.dataType == 2 ? 1 : 2}); }
    }
    if (allowInput && notice && noticeBit) { bits.append(noticeBitAddress); }
    if (allowInput && notice && noticeWord) { words.append({noticeWordAddress, noticeWordType == 0 || noticeWordType == 2 ? 1 : 2}); }
    context.connections = connections; context.role = PlcAddressRole::Word;
    Q_FOREACH (word, words) { context.transferCount = word.second; if (!parseQualifiedPlcAddressText(word.first, context, catalog, &address, error)) { return false; } }
    context.role = PlcAddressRole::Bit; context.transferCount = 1U;
    Q_FOREACH (bit, bits) { if (!parseQualifiedPlcAddressText(bit, context, catalog, &address, error)) { return false; } } return true;
}
