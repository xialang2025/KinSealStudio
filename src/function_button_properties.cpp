#include "function_button_properties.h"

namespace
{
/******************************************************************************
 * Function Name   : functionError
 * Description     : Report invalid system-function input without publishing a partial draft.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.10
 ******************************************************************************/
bool functionError(QString *error, const QString &message) //20261110-0 add
{
    if (error != nullptr) { *error = message; }
    return false;
}
}

/******************************************************************************
 * Function Name   : FunctionButtonProperties::functions
 * Description     : Preserve the eighteen observed V5.1 system-function indices.
 * Input Parameters: None
 * Return Value    : Ordered function labels.
 * Date            : 2026.11.10
 ******************************************************************************/
QStringList FunctionButtonProperties::functions() //20261110-0 add
{
    return {QStringLiteral("背景灯开关"), QStringLiteral("系统设置"), QStringLiteral("系统重启"),
            QStringLiteral("设置屏保时间"), QStringLiteral("触摸校正"), QStringLiteral("设置系统时间"),
            QStringLiteral("面板保护解锁"), QStringLiteral("更改用户等级"), QStringLiteral("注销用户等级"),
            QStringLiteral("触摸声音开关"), QStringLiteral("历史数据操作"), QStringLiteral("配方操作"),
            QStringLiteral("报警操作"), QStringLiteral("语言切换"), QStringLiteral("键盘组件"),
            QStringLiteral("无线设置"), QStringLiteral("背光调节"), QStringLiteral("下载方式")};
}

/******************************************************************************
 * Function Name   : FunctionButtonProperties::types
 * Description     : Return the native type order and retain the project's language order.
 * Input Parameters: function, languages
 * Return Value    : Active type labels, or an empty list for functions without a type.
 * Date            : 2026.11.10
 ******************************************************************************/
QStringList FunctionButtonProperties::types(int function, const QStringList &languages) //20261110-0 add
{
    switch (function)
    {
    case 10:
        return {QStringLiteral("清除全部历史数据"), QStringLiteral("历史数据上移"), QStringLiteral("历史数据下移"),
                QStringLiteral("历史数据上翻页"), QStringLiteral("历史数据下翻页"),
                QStringLiteral("U盘历史数据导出"), QStringLiteral("SD卡历史数据导出")};
    case 11:
        return {QStringLiteral("当前配方写入PLC"), QStringLiteral("从PLC读当前配方"), QStringLiteral("前一个配方"),
                QStringLiteral("后一个配方"), QStringLiteral("当前配方保存"), QStringLiteral("配方读取及保存"),
                QStringLiteral("重载配方"), QStringLiteral("当前配方删除"), QStringLiteral("U盘配方导出"),
                QStringLiteral("U盘配方导入"), QStringLiteral("配方添加"), QStringLiteral("SD卡配方导出"),
                QStringLiteral("SD卡配方导入")};
    case 12:
        return {QStringLiteral("确认报警"), QStringLiteral("清除报警"), QStringLiteral("报警声音开关"),
                QStringLiteral("清除历史报警数据"), QStringLiteral("报警控件报警上移"), QStringLiteral("报警控件报警下移"),
                QStringLiteral("报警控件报警上翻页"), QStringLiteral("报警控件报警下翻页"),
                QStringLiteral("历史报警控件报警上移"), QStringLiteral("历史报警控件报警下移"),
                QStringLiteral("历史报警控件报警上翻页"), QStringLiteral("历史报警控件报警下翻页"),
                QStringLiteral("U盘历史报警导出"), QStringLiteral("SD卡历史报警导出")};
    case 13:
        return languages.isEmpty() ? QStringList{QStringLiteral("中文")} : languages;
    case 17:
        return {QStringLiteral("USB下载"), QStringLiteral("U盘下载")};
    default:
        return {};
    }
}

/******************************************************************************
 * Function Name   : FunctionButtonProperties::keyboardKeys
 * Description     : Preserve the ten radio-button positions independently of their device key codes.
 * Input Parameters: None
 * Return Value    : Ordered keyboard labels.
 * Date            : 2026.11.10
 ******************************************************************************/
QStringList FunctionButtonProperties::keyboardKeys() //20261110-0 add
{
    return {QStringLiteral("[Enter]"), QStringLiteral("[BackSpace]"), QStringLiteral("[Clear]"),
            QStringLiteral("[ESC]"), QStringLiteral("[Delete]"), QStringLiteral("[Caps]"),
            QStringLiteral("[Left]"), QStringLiteral("[Right]"), QStringLiteral("[Space]"),
            QStringLiteral("[ASCII]/Unicode")};
}

/******************************************************************************
 * Function Name   : FunctionButtonProperties::validate
 * Description     : Bound function/type/key indices while leaving project language bounds to the model.
 * Input Parameters: error
 * Return Value    : True for structurally valid function properties.
 * Date            : 2026.11.10
 ******************************************************************************/
bool FunctionButtonProperties::validate(QString *error) const //20261110-0 add
{
    QStringList choices;

    if (function < 0 || function >= functions().size() || keyboardKey < 0 || keyboardKey >= keyboardKeys().size())
    {
        return functionError(error, QStringLiteral("功能按钮的功能或键盘键索引无效。"));
    }
    choices = types(function, {});
    if (type < 0 || (function != 13 && (choices.isEmpty() ? type != 0 : type >= choices.size())))
    {
        return functionError(error, QStringLiteral("功能按钮的类型索引不属于所选功能。"));
    }
    if (keyboardText.size() > 1 || (function == 14 && keyboardKey == 9 &&
        !keyboardText.isEmpty() && keyboardText.at(0).isSurrogate()))
    {
        // Native maxLength is one UTF-16 code unit; reject a truncated surrogate before JSON or device output can replace it.
        return functionError(error, QStringLiteral("键盘组件字符最多保存一个UTF-16编码单元，不支持需代理对编码的字符。"));
    }
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : FunctionButtonProperties::toJson
 * Description     : Persist system-function selection without copying shared artwork or permissions.
 * Input Parameters: None
 * Return Value    : Versioned function properties.
 * Date            : 2026.11.10
 ******************************************************************************/
QJsonObject FunctionButtonProperties::toJson() const //20261110-0 add
{
    QJsonObject json;

    json.insert(QStringLiteral("version"), 1);
    json.insert(QStringLiteral("function"), function);
    json.insert(QStringLiteral("type"), type);
    json.insert(QStringLiteral("keyboardKey"), keyboardKey);
    json.insert(QStringLiteral("keyboardText"), keyboardText);
    json.insert(QStringLiteral("pressedEffect"), pressedEffect);
    return json;
}

/******************************************************************************
 * Function Name   : FunctionButtonProperties::fromJson
 * Description     : Restore and validate the complete record before replacing caller-owned properties.
 * Input Parameters: json, properties, error
 * Return Value    : True when a valid record was published.
 * Date            : 2026.11.10
 ******************************************************************************/
bool FunctionButtonProperties::fromJson(const QJsonObject &json, FunctionButtonProperties *properties, QString *error) //20261110-0 add
{
    FunctionButtonProperties result;
    QString name;
    QStringList integers;
    QJsonValue value;

    if (properties == nullptr || !json.value(QStringLiteral("version")).isDouble() ||
        json.value(QStringLiteral("version")).toDouble() != 1)
    {
        return functionError(error, QStringLiteral("功能按钮属性版本无效。"));
    }
    integers = {QStringLiteral("function"), QStringLiteral("type"), QStringLiteral("keyboardKey")};
    Q_FOREACH (name, integers)
    {
        value = json.value(name);
        if (!value.isUndefined() && (!value.isDouble() || value.toDouble() != value.toInt(-1)))
        {
            return functionError(error, QStringLiteral("功能按钮的功能、类型与键盘键必须为整数。"));
        }
    }
    if ((!json.value(QStringLiteral("keyboardText")).isUndefined() && !json.value(QStringLiteral("keyboardText")).isString()) ||
        (!json.value(QStringLiteral("pressedEffect")).isUndefined() && !json.value(QStringLiteral("pressedEffect")).isBool()))
    {
        return functionError(error, QStringLiteral("功能按钮键盘文字或按下效果字段无效。"));
    }
    result.function = json.value(QStringLiteral("function")).toInt();
    result.type = json.value(QStringLiteral("type")).toInt();
    result.keyboardKey = json.value(QStringLiteral("keyboardKey")).toInt();
    result.keyboardText = json.value(QStringLiteral("keyboardText")).toString();
    result.pressedEffect = json.value(QStringLiteral("pressedEffect")).toBool();
    if (!result.validate(error)) { return false; }
    *properties = result;
    return true;
}
