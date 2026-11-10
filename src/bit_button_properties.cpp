#include "bit_button_properties.h"
#include "control_rendering.h" //20261110-8 add
#include <QJsonArray>
#include <QImage>
#include <QBuffer>
#include <QImageReader>

/******************************************************************************
 * Function Name   : validateButtonImage
 * Description     : Bound decoded dimensions before allocating an embedded PNG image.
 * Input Parameters: png
 * Return Value    : True for a supported PNG within editor resource limits.
 * Date            : 2026.11.08
 ******************************************************************************/
bool validateButtonImage(const QByteArray &png) //20261110-8 change: Reuse bounded content-addressed source decoding.
{
    return !decodedControlPng(png).isNull();
}

/******************************************************************************
 * Function Name   : BitButtonProperties::BitButtonProperties
 * Description     : Initialize the observed V5.1 defaults without merging the two states.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
BitButtonProperties::BitButtonProperties() //20261108-15 add
{
    states[0].picture = QStringLiteral("switch_OFF.kvg");
    states[1].picture = QStringLiteral("switch_ON.kvg");
    states[0].text.languageTexts.insert(QStringLiteral("中文"), QStringLiteral("OFF"));
    states[1].text.languageTexts.insert(QStringLiteral("中文"), QStringLiteral("ON"));
    states[0].sharedText = QStringLiteral("OFF"); states[1].sharedText = QStringLiteral("ON");
    touch.minimumLevel = 2;
}

/******************************************************************************
 * Function Name   : BitButtonProperties::operations
 * Description     : Keep the native operation ordering stable in files and controls.
 * Input Parameters: None
 * Return Value    : Ordered operation labels.
 * Date            : 2026.11.08
 ******************************************************************************/
QStringList BitButtonProperties::operations() //20261108-15 add
{
    return {QStringLiteral("置位"), QStringLiteral("复位"), QStringLiteral("点动"), QStringLiteral("交替")};
}

/******************************************************************************
 * Function Name   : BitButtonState::toJson
 * Description     : Store editable text separately from shape and embed imported image bytes.
 * Input Parameters: None
 * Return Value    : State JSON.
 * Date            : 2026.11.08
 ******************************************************************************/
QJsonObject BitButtonState::toJson() const //20261108-15 add
{
    QJsonObject json;
    json.insert(QStringLiteral("text"), text.toJson());
    json.insert(QStringLiteral("flash"), flash);
    json.insert(QStringLiteral("sameLanguage"), sameLanguage);
    json.insert(QStringLiteral("sharedText"), sharedText);
    json.insert(QStringLiteral("useImage"), useImage);
    json.insert(QStringLiteral("picture"), picture);
    json.insert(QStringLiteral("picturePng"), QString::fromLatin1(picturePng.toBase64()));
    json.insert(QStringLiteral("effect3d"), effect3d);
    json.insert(QStringLiteral("transparent"), transparent);
    json.insert(QStringLiteral("background"), background.name(QColor::HexArgb));
    json.insert(QStringLiteral("opacity"), opacity);
    json.insert(QStringLiteral("imageMode"), imageMode);
    return json;
}

/******************************************************************************
 * Function Name   : BitButtonState::fromJson
 * Description     : Restore one visual state and reject malformed or oversized image data.
 * Input Parameters: json, state, error
 * Return Value    : True when the state can be edited and rendered.
 * Date            : 2026.11.08
 ******************************************************************************/
bool BitButtonState::fromJson(const QJsonObject &json, BitButtonState *state, QString *error) //20261108-15 add
{
    BitButtonState result;
    QByteArray encoded;
    if (state == nullptr || !StaticTextProperties::fromJson(json.value(QStringLiteral("text")).toObject(), &result.text, error))
    {
        return false;
    }
    result.flash = json.value(QStringLiteral("flash")).toBool();
    result.sameLanguage = json.value(QStringLiteral("sameLanguage")).toBool();
    result.sharedText = json.value(QStringLiteral("sharedText")).toString();
    result.useImage = json.value(QStringLiteral("useImage")).toBool(true);
    result.picture = json.value(QStringLiteral("picture")).toString();
    encoded = json.value(QStringLiteral("picturePng")).toString().toLatin1();
    if (encoded.size() > 24 * 1024 * 1024)
    {
        if (error != nullptr) { *error = QStringLiteral("位按钮图片超过本版 16 MiB 输入保护范围。"); }
        return false;
    }
    result.picturePng = QByteArray::fromBase64(encoded);
    if ((!encoded.isEmpty() && result.picturePng.toBase64() != encoded) ||
        (!result.picturePng.isEmpty() && !validateButtonImage(result.picturePng)))
    {
        if (error != nullptr) { *error = QStringLiteral("位按钮的内嵌图片无效。"); }
        return false;
    }
    result.effect3d = json.value(QStringLiteral("effect3d")).toBool();
    result.transparent = json.value(QStringLiteral("transparent")).toBool();
    result.background = QColor(json.value(QStringLiteral("background")).toString(QStringLiteral("#ffffffff")));
    result.opacity = json.value(QStringLiteral("opacity")).toInt(255);
    result.imageMode = json.value(QStringLiteral("imageMode")).toInt();
    *state = result;
    return true;
}

/******************************************************************************
 * Function Name   : BitButtonProperties::toJson
 * Description     : Preserve inactive monitor, permission and notification values for future editing.
 * Input Parameters: None
 * Return Value    : Versioned bit-button record.
 * Date            : 2026.11.08
 ******************************************************************************/
QJsonObject BitButtonProperties::toJson() const //20261108-15 add
{
    QJsonObject json;
    json.insert(QStringLiteral("version"), 1);
    json.insert(QStringLiteral("operation"), operation);
    json.insert(QStringLiteral("macroEnabled"), macroEnabled);
    json.insert(QStringLiteral("macroName"), macroName);
    json.insert(QStringLiteral("indicator"), indicator);
    json.insert(QStringLiteral("dualMonitor"), dualMonitor);
    json.insert(QStringLiteral("monitorSame"), monitorSame);
    json.insert(QStringLiteral("monitorType"), monitorType);
    json.insert(QStringLiteral("monitorBitAddress"), monitorBitAddress);
    json.insert(QStringLiteral("monitorWordAddress"), monitorWordAddress);
    json.insert(QStringLiteral("bitIndex"), bitIndex);
    json.insert(QStringLiteral("states"), QJsonArray{states[0].toJson(), states[1].toJson()});
    json.insert(QStringLiteral("visibility"), visibility.toJson());
    json.insert(QStringLiteral("touch"), touch.toJson());
    json.insert(QStringLiteral("notice"), notice);
    json.insert(QStringLiteral("noticeBit"), noticeBit);
    json.insert(QStringLiteral("noticeBitAddress"), noticeBitAddress);
    json.insert(QStringLiteral("noticeBitValue"), noticeBitValue);
    json.insert(QStringLiteral("noticeWord"), noticeWord);
    json.insert(QStringLiteral("noticeWordAddress"), noticeWordAddress);
    json.insert(QStringLiteral("noticeWordType"), noticeWordType);
    json.insert(QStringLiteral("noticeWordValue"), noticeWordValue);
    json.insert(QStringLiteral("pressTimeEnabled"), pressTimeEnabled);
    json.insert(QStringLiteral("pressTime"), pressTime);
    return json;
}

/******************************************************************************
 * Function Name   : BitButtonProperties::validate
 * Description     : Validate representable UI records without guessing firmware permission semantics.
 * Input Parameters: error, allowZeroPressTime (native function-button context)
 * Return Value    : True for supported editor input.
 * Date            : 2026.11.08
 ******************************************************************************/
bool BitButtonProperties::validate(QString *error, bool allowZeroPressTime) const //20261110-0 change
{
    int index;
    StaticTextProperties noticeCondition; //20261109-0 add
    if (operation < 0 || operation > 3 || monitorType < 0 || monitorType > 1 ||
        bitIndex < 0 || bitIndex > 15 || pressTime < (allowZeroPressTime ? 0 : 1) || pressTime > 65535 ||
        noticeWordType < 0 || noticeWordType > 4 || !visibility.validate(error) || !touch.validate(error) ||
        (notice && noticeBit && noticeBitAddress.trimmed().isEmpty())) //20261109-0 change
    {
        if (error != nullptr && error->isEmpty()) { *error = QStringLiteral("位按钮操作、监视或权限属性无效。"); }
        return false;
    }
    //20261109-0 add: Notification values use the same five numeric types as access conditions.
    noticeCondition.visibilityEnabled = notice && noticeWord;
    noticeCondition.wordEnabled = notice && noticeWord;
    noticeCondition.dataType = noticeWordType;
    noticeCondition.wordAddress = noticeWordAddress;
    noticeCondition.compareValue = noticeWordValue;
    if (!noticeCondition.validate(error)) { return false; }
    for (index = 0; index < 2; ++index)
    {
        if (!states[index].text.validate(error) || !states[index].background.isValid() ||
            states[index].opacity < 0 || states[index].opacity > 255 ||
            states[index].imageMode < 0 || states[index].imageMode > 2 || states[index].picturePng.size() > 16 * 1024 * 1024)
        {
            if (error != nullptr) { *error = QStringLiteral("位按钮状态文字、颜色、透明度或图片属性无效。"); }
            return false;
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : BitButtonProperties::fromJson
 * Description     : Read every field transactionally and reject incompatible state counts and versions.
 * Input Parameters: json, properties, error, allowZeroPressTime (native function-button context)
 * Return Value    : True when the complete record is restored.
 * Date            : 2026.11.08
 ******************************************************************************/
bool BitButtonProperties::fromJson(const QJsonObject &json, BitButtonProperties *properties, QString *error,
                                   bool allowZeroPressTime) //20261110-0 change
{
    BitButtonProperties result;
    QJsonArray statesJson;
    statesJson = json.value(QStringLiteral("states")).toArray();
    if (properties == nullptr || json.value(QStringLiteral("version")).toInt() != 1 || statesJson.size() != 2)
    {
        if (error != nullptr) { *error = QStringLiteral("不支持的位按钮属性版本或状态数量。"); }
        return false;
    }
    result.operation = json.value(QStringLiteral("operation")).toInt(3);
    result.macroEnabled = json.value(QStringLiteral("macroEnabled")).toBool();
    result.macroName = json.value(QStringLiteral("macroName")).toString();
    result.indicator = json.value(QStringLiteral("indicator")).toBool(true);
    result.dualMonitor = json.value(QStringLiteral("dualMonitor")).toBool(true);
    result.monitorSame = json.value(QStringLiteral("monitorSame")).toBool(true);
    result.monitorType = json.value(QStringLiteral("monitorType")).toInt();
    result.monitorBitAddress = json.value(QStringLiteral("monitorBitAddress")).toString(QStringLiteral("LB0"));
    result.monitorWordAddress = json.value(QStringLiteral("monitorWordAddress")).toString(QStringLiteral("LW0"));
    result.bitIndex = json.value(QStringLiteral("bitIndex")).toInt(15);
    if (!BitButtonState::fromJson(statesJson.at(0).toObject(), &result.states[0], error) ||
        !BitButtonState::fromJson(statesJson.at(1).toObject(), &result.states[1], error) ||
        !StaticTextProperties::fromJson(json.value(QStringLiteral("visibility")).toObject(), &result.visibility, error) ||
        !StaticTextProperties::fromJson(json.value(QStringLiteral("touch")).toObject(), &result.touch, error))
    {
        return false;
    }
    result.notice = json.value(QStringLiteral("notice")).toBool();
    result.noticeBit = json.value(QStringLiteral("noticeBit")).toBool();
    result.noticeBitAddress = json.value(QStringLiteral("noticeBitAddress")).toString(QStringLiteral("LB0"));
    result.noticeBitValue = json.value(QStringLiteral("noticeBitValue")).toBool(true);
    result.noticeWord = json.value(QStringLiteral("noticeWord")).toBool();
    result.noticeWordAddress = json.value(QStringLiteral("noticeWordAddress")).toString(QStringLiteral("LW0"));
    result.noticeWordType = json.value(QStringLiteral("noticeWordType")).toInt();
    result.noticeWordValue = json.value(QStringLiteral("noticeWordValue")).toString(QStringLiteral("0"));
    result.pressTimeEnabled = json.value(QStringLiteral("pressTimeEnabled")).toBool();
    result.pressTime = json.value(QStringLiteral("pressTime")).toInt(255);
    if (!result.validate(error, allowZeroPressTime)) { return false; }
    *properties = result;
    return true;
}
