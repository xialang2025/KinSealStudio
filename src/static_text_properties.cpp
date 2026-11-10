#include "static_text_properties.h"

#include <cmath>
#include <limits>

/******************************************************************************
 * Function Name   : StaticTextProperties::styles
 * Description     : Return the thirteen background labels in the observed native order.
 * Input Parameters: None
 * Return Value    : Ordered style names.
 * Date            : 2026.11.08
 ******************************************************************************/
QStringList StaticTextProperties::styles() //20261108-14 add
{
    return {QStringLiteral("透明"), QStringLiteral("纯色"), QStringLiteral("横向过渡"),
            QStringLiteral("横向对称过渡"), QStringLiteral("纵向过渡"), QStringLiteral("纵向对称过渡"),
            QStringLiteral("斜上过渡"), QStringLiteral("斜上对称过渡"), QStringLiteral("斜下过渡"),
            QStringLiteral("斜下对称过渡"), QStringLiteral("右上角辐射"), QStringLiteral("左上角辐射"),
            QStringLiteral("中心辐射")};
}

/******************************************************************************
 * Function Name   : StaticTextProperties::dataTypes
 * Description     : Return the five native visibility word interpretations.
 * Input Parameters: None
 * Return Value    : Ordered data type names.
 * Date            : 2026.11.08
 ******************************************************************************/
QStringList StaticTextProperties::dataTypes() //20261108-14 add
{
    return {QStringLiteral("16位正整数"), QStringLiteral("32位正整数"), QStringLiteral("16位整数"),
            QStringLiteral("32位整数"), QStringLiteral("32位浮点数")};
}

/******************************************************************************
 * Function Name   : StaticTextProperties::comparisons
 * Description     : Preserve the comparison order shared by the dialog and serialization.
 * Input Parameters: None
 * Return Value    : Ordered comparison operators.
 * Date            : 2026.11.08
 ******************************************************************************/
QStringList StaticTextProperties::comparisons() //20261108-14 add
{
    return {QStringLiteral("<"), QStringLiteral(">"), QStringLiteral("=="),
            QStringLiteral("<>"), QStringLiteral("<="), QStringLiteral(">=")};
}

/******************************************************************************
 * Function Name   : StaticTextProperties::toJson
 * Description     : Persist every appearance and dormant permission input without losing values.
 * Input Parameters: None
 * Return Value    : Versioned static-text property object.
 * Date            : 2026.11.08
 ******************************************************************************/
QJsonObject StaticTextProperties::toJson() const //20261108-14 add
{
    QJsonObject json;
    QJsonObject texts;
    QMap<QString, QString>::const_iterator iterator;

    json.insert(QStringLiteral("version"), 1);
    json.insert(QStringLiteral("fontFamily"), fontFamily);
    json.insert(QStringLiteral("fontSize"), fontSize);
    json.insert(QStringLiteral("spacing"), spacing);
    json.insert(QStringLiteral("alignment"), alignment);
    json.insert(QStringLiteral("italic"), italic);
    json.insert(QStringLiteral("bold"), bold);
    json.insert(QStringLiteral("adaptive"), adaptive);
    json.insert(QStringLiteral("underline"), underline);
    json.insert(QStringLiteral("textColor"), textColor.name(QColor::HexArgb));
    json.insert(QStringLiteral("backgroundColor"), backgroundColor.name(QColor::HexArgb));
    json.insert(QStringLiteral("foregroundColor"), foregroundColor.name(QColor::HexArgb));
    json.insert(QStringLiteral("style"), style);
    json.insert(QStringLiteral("opacity"), opacity);
    for (iterator = languageTexts.constBegin(); iterator != languageTexts.constEnd(); ++iterator)
    {
        texts.insert(iterator.key(), iterator.value());
    }
    json.insert(QStringLiteral("texts"), texts);
    json.insert(QStringLiteral("visibilityEnabled"), visibilityEnabled);
    json.insert(QStringLiteral("bitEnabled"), bitEnabled);
    json.insert(QStringLiteral("bitAddress"), bitAddress);
    json.insert(QStringLiteral("bitState"), bitState);
    json.insert(QStringLiteral("levelEnabled"), levelEnabled);
    json.insert(QStringLiteral("minimumLevel"), minimumLevel);
    json.insert(QStringLiteral("wordEnabled"), wordEnabled);
    json.insert(QStringLiteral("dataType"), dataType);
    json.insert(QStringLiteral("wordAddress"), wordAddress);
    json.insert(QStringLiteral("comparison"), comparison);
    json.insert(QStringLiteral("compareValue"), compareValue);
    json.insert(QStringLiteral("tolerance"), tolerance);
    return json;
}

/******************************************************************************
 * Function Name   : StaticTextProperties::validate
 * Description     : Reject corrupt enums, colors and numeric storage before painting or saving.
 * Input Parameters: error
 * Return Value    : True for a representable property record; native numeric limits remain unverified.
 * Date            : 2026.11.08
 ******************************************************************************/
bool StaticTextProperties::validate(QString *error) const //20261108-14 add
{
    bool numberOk;
    bool toleranceOk;
    double number;
    double epsilon;
    double minimum;
    double maximum;

    number = compareValue.toDouble(&numberOk);
    epsilon = tolerance.toDouble(&toleranceOk);
    minimum = dataType == 0 || dataType == 1 ? 0 : dataType == 2 ? -32768.0 : dataType == 3 ? -2147483648.0 : -std::numeric_limits<float>::max();
    maximum = dataType == 0 ? 65535.0 : dataType == 1 ? 4294967295.0 : dataType == 2 ? 32767.0 : dataType == 3 ? 2147483647.0 : std::numeric_limits<float>::max();
    if (fontFamily.isEmpty() || !std::isfinite(fontSize) || fontSize <= 0 || fontSize > 4096 ||
        !std::isfinite(spacing) || std::abs(spacing) > 4096 ||
        alignment < 0 || alignment > 2 || style < 0 || style >= styles().size() ||
        opacity < 0 || opacity > 255 || !textColor.isValid() || !backgroundColor.isValid() ||
        !foregroundColor.isValid() || minimumLevel < 1 || minimumLevel > 8 ||
        dataType < 0 || dataType > 4 || comparison < 0 || comparison > 5 ||
        (visibilityEnabled && bitEnabled && bitAddress.trimmed().isEmpty()) ||
        (visibilityEnabled && wordEnabled && (wordAddress.trimmed().isEmpty() ||
         !numberOk || !toleranceOk || !std::isfinite(number) || !std::isfinite(epsilon) ||
         number < minimum || number > maximum || (dataType < 4 && std::floor(number) != number))))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("静态文本属性无效：请检查字体、字号、颜色、权限地址和比较数值。");
        }
        return false;
    }
    return true;
}

/******************************************************************************
 * Function Name   : StaticTextProperties::fromJson
 * Description     : Read the complete property record while preserving unchecked condition inputs.
 * Input Parameters: json, properties, error
 * Return Value    : True when all serialized fields form a supported record.
 * Date            : 2026.11.08
 ******************************************************************************/
bool StaticTextProperties::fromJson(const QJsonObject &json, StaticTextProperties *properties,
                                    QString *error) //20261108-14 add
{
    StaticTextProperties result;
    QJsonObject texts;
    QJsonObject::const_iterator iterator;

    if (properties == nullptr || json.value(QStringLiteral("version")).toInt() != 1)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("不支持的静态文本属性版本。");
        }
        return false;
    }
    result.fontFamily = json.value(QStringLiteral("fontFamily")).toString(result.fontFamily);
    result.fontSize = json.value(QStringLiteral("fontSize")).toDouble(16);
    result.spacing = json.value(QStringLiteral("spacing")).toDouble();
    result.alignment = json.value(QStringLiteral("alignment")).toInt();
    result.italic = json.value(QStringLiteral("italic")).toBool();
    result.bold = json.value(QStringLiteral("bold")).toBool();
    result.adaptive = json.value(QStringLiteral("adaptive")).toBool();
    result.underline = json.value(QStringLiteral("underline")).toBool();
    result.textColor = QColor(json.value(QStringLiteral("textColor")).toString(QStringLiteral("#ff000000")));
    result.backgroundColor = QColor(json.value(QStringLiteral("backgroundColor")).toString(QStringLiteral("#ffffffff")));
    result.foregroundColor = QColor(json.value(QStringLiteral("foregroundColor")).toString(QStringLiteral("#ffff0000")));
    result.style = json.value(QStringLiteral("style")).toInt();
    result.opacity = json.value(QStringLiteral("opacity")).toInt(255);
    texts = json.value(QStringLiteral("texts")).toObject();
    for (iterator = texts.constBegin(); iterator != texts.constEnd(); ++iterator)
    {
        if (!iterator.value().isString())
        {
            return false;
        }
        result.languageTexts.insert(iterator.key(), iterator.value().toString());
    }
    result.visibilityEnabled = json.value(QStringLiteral("visibilityEnabled")).toBool();
    result.bitEnabled = json.value(QStringLiteral("bitEnabled")).toBool();
    result.bitAddress = json.value(QStringLiteral("bitAddress")).toString(result.bitAddress);
    result.bitState = json.value(QStringLiteral("bitState")).toBool(true);
    result.levelEnabled = json.value(QStringLiteral("levelEnabled")).toBool();
    result.minimumLevel = json.value(QStringLiteral("minimumLevel")).toInt(1);
    result.wordEnabled = json.value(QStringLiteral("wordEnabled")).toBool();
    result.dataType = json.value(QStringLiteral("dataType")).toInt();
    result.wordAddress = json.value(QStringLiteral("wordAddress")).toString(result.wordAddress);
    result.comparison = json.value(QStringLiteral("comparison")).toInt();
    result.compareValue = json.value(QStringLiteral("compareValue")).toString(QStringLiteral("0"));
    result.tolerance = json.value(QStringLiteral("tolerance")).toString(QStringLiteral("0"));
    if (!result.validate(error))
    {
        return false;
    }
    *properties = result;
    return true;
}
