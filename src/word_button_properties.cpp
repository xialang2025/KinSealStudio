#include "word_button_properties.h"

#include <QRegularExpression>
#include <cmath>

namespace
{
/******************************************************************************
 * Function Name   : wordError
 * Description     : Report a word-operation validation failure without changing the draft.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.09
 ******************************************************************************/
bool wordError(QString *error, const QString &message) //20261109-6 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : wordNumber
 * Description     : Check complete decimal input against the selected integer, BCD or float domain.
 * Input Parameters: text, type, value
 * Return Value    : True for one finite representable editor value.
 * Date            : 2026.11.09
 ******************************************************************************/
bool wordNumber(const QString &text, int type, double *value) //20261109-6 add
{
    static const double lower[7] = {0, 0, -32768, -2147483648.0, 0, 0, -9.9e16};
    static const double upper[7] = {65535, 4294967295.0, 32767, 2147483647.0, 9999, 99999999, 9.9e16};
    static const QRegularExpression integer(QStringLiteral("^[+-]?[0-9]+$"));
    static const QRegularExpression real(QStringLiteral("^[+-]?(?:[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)(?:[eE][+-]?[0-9]+)?$"));
    QString input;
    double number;
    bool valid;

    input = text.trimmed();
    if (type < 0 || type > 6 || !(type == 6 ? real : integer).match(input).hasMatch()) { return false; }
    number = input.toDouble(&valid);
    if (!valid || !std::isfinite(number) || number < lower[type] || number > upper[type]) { return false; }
    *value = number;
    return true;
}
}

/******************************************************************************
 * Function Name   : WordButtonProperties::dataTypes
 * Description     : Preserve the seven V5.1 word-button data-type indices.
 * Input Parameters: None
 * Return Value    : Ordered data-type labels.
 * Date            : 2026.11.09
 ******************************************************************************/
QStringList WordButtonProperties::dataTypes() //20261109-6 add
{
    return {QStringLiteral("16位正整数"), QStringLiteral("32位正整数"), QStringLiteral("16位整数"),
            QStringLiteral("32位整数"), QStringLiteral("16位BCD码"), QStringLiteral("32位BCD码"), QStringLiteral("32位浮点数")};
}

/******************************************************************************
 * Function Name   : WordButtonProperties::operations
 * Description     : Preserve the seven observed operation indices independently of bit operations.
 * Input Parameters: None
 * Return Value    : Ordered operation labels.
 * Date            : 2026.11.09
 ******************************************************************************/
QStringList WordButtonProperties::operations() //20261109-6 add
{
    return {QStringLiteral("设置常数"), QStringLiteral("数值输入"), QStringLiteral("密码输入"),
            QStringLiteral("数值加"), QStringLiteral("数值减"), QStringLiteral("递增绕回"), QStringLiteral("递减绕回")};
}

/******************************************************************************
 * Function Name   : WordButtonProperties::resetDefaultsForDataType
 * Description     : Restore the observed UI defaults rather than substituting full numeric type limits.
 * Input Parameters: type
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void WordButtonProperties::resetDefaultsForDataType(int type) //20261109-6 add
{
    static const char *minimum[7] = {"0", "0", "-32768", "-99999", "0", "0", "-9999"};
    static const char *maximum[7] = {"65535", "99999", "32767", "99999", "9999", "9999", "9999"};

    if (type < 0 || type > 6) { return; }
    dataType = type;
    constantValue = QStringLiteral("1");
    minimumValue = QString::fromLatin1(minimum[type]);
    maximumValue = QString::fromLatin1(maximum[type]);
    totalDigits = type >= 4 ? 4 : 5;
    decimalDigits = 0;
}

/******************************************************************************
 * Function Name   : WordButtonProperties::resetDefaultsForOperation
 * Description     : Reset operation fields using the current data type when the operation changes.
 * Input Parameters: operationIndex
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void WordButtonProperties::resetDefaultsForOperation(int operationIndex) //20261109-6 add
{
    if (operationIndex < 0 || operationIndex > 6) { return; }
    operation = operationIndex;
    resetDefaultsForDataType(dataType);
}

/******************************************************************************
 * Function Name   : WordButtonProperties::usesConstant
 * Description     : Identify operations that expose the constant field.
 * Input Parameters: None
 * Return Value    : True for set, add, subtract and wrap operations.
 * Date            : 2026.11.09
 ******************************************************************************/
bool WordButtonProperties::usesConstant() const //20261109-6 add
{
    return operation == 0 || operation >= 3;
}

/******************************************************************************
 * Function Name   : WordButtonProperties::usesMinimum
 * Description     : Identify operations that expose a lower bound.
 * Input Parameters: None
 * Return Value    : True when the minimum field is active.
 * Date            : 2026.11.09
 ******************************************************************************/
bool WordButtonProperties::usesMinimum() const //20261109-6 add
{
    return operation == 1 || operation == 2 || operation == 4 || operation >= 5;
}

/******************************************************************************
 * Function Name   : WordButtonProperties::usesMaximum
 * Description     : Identify operations that expose an upper bound.
 * Input Parameters: None
 * Return Value    : True when the maximum field is active.
 * Date            : 2026.11.09
 ******************************************************************************/
bool WordButtonProperties::usesMaximum() const //20261109-6 add
{
    return operation == 1 || operation == 2 || operation == 3 || operation >= 5;
}

/******************************************************************************
 * Function Name   : WordButtonProperties::usesTotalDigits
 * Description     : Identify the two keypad operations that expose a digit count.
 * Input Parameters: None
 * Return Value    : True for numeric or password input.
 * Date            : 2026.11.09
 ******************************************************************************/
bool WordButtonProperties::usesTotalDigits() const //20261109-6 add
{
    return operation == 1 || operation == 2;
}

/******************************************************************************
 * Function Name   : WordButtonProperties::usesDecimalDigits
 * Description     : Keep decimal input hidden for the password operation.
 * Input Parameters: None
 * Return Value    : True only for numeric input.
 * Date            : 2026.11.09
 ******************************************************************************/
bool WordButtonProperties::usesDecimalDigits() const //20261109-6 add
{
    return operation == 1;
}

/******************************************************************************
 * Function Name   : WordButtonProperties::transferWordCount
 * Description     : Return the source-word width without changing the address family index.
 * Input Parameters: None
 * Return Value    : One or two words.
 * Date            : 2026.11.09
 ******************************************************************************/
int WordButtonProperties::transferWordCount() const //20261109-6 add
{
    return dataType == 1 || dataType == 3 || dataType == 5 || dataType == 6 ? 2 : 1;
}

/******************************************************************************
 * Function Name   : WordButtonProperties::validate
 * Description     : Validate active numeric fields while retaining dormant operation drafts.
 * Input Parameters: error
 * Return Value    : True for supported editor input, without asserting device execution semantics.
 * Date            : 2026.11.09
 ******************************************************************************/
bool WordButtonProperties::validate(QString *error) const //20261109-6 add
{
    double constant;
    double minimum;
    double maximum;

    if (dataType < 0 || dataType > 6 || operation < 0 || operation > 6)
    {
        return wordError(error, QStringLiteral("字按钮数据类型或操作类型无效。"));
    }
    if ((usesConstant() && !wordNumber(constantValue, dataType, &constant)) ||
        (usesMinimum() && !wordNumber(minimumValue, dataType, &minimum)) ||
        (usesMaximum() && !wordNumber(maximumValue, dataType, &maximum)))
    {
        return wordError(error, QStringLiteral("字按钮的活动数值必须是所选数据类型范围内的完整有限数值。"));
    }
    if (usesMinimum() && usesMaximum() && minimum > maximum)
    {
        return wordError(error, QStringLiteral("字按钮最小值不能大于最大值。"));
    }
    // The editor bounds the decimal layout; these guards are not claimed as native firmware limits.
    if (totalDigits < 1 || totalDigits > 10 || decimalDigits < 0 || decimalDigits > 9 ||
        (usesDecimalDigits() && decimalDigits >= totalDigits))
    {
        return wordError(error, QStringLiteral("本版字按钮总位数范围为1..10，小数位为0..9且须小于总位数。"));
    }
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : WordButtonProperties::toJson
 * Description     : Persist the independent word-operation record without shared artwork fields.
 * Input Parameters: None
 * Return Value    : Versioned word-button JSON.
 * Date            : 2026.11.09
 ******************************************************************************/
QJsonObject WordButtonProperties::toJson() const //20261109-6 add
{
    QJsonObject json;

    json.insert(QStringLiteral("version"), 1);
    json.insert(QStringLiteral("dataType"), dataType);
    json.insert(QStringLiteral("operation"), operation);
    json.insert(QStringLiteral("constantValue"), constantValue);
    json.insert(QStringLiteral("minimumValue"), minimumValue);
    json.insert(QStringLiteral("maximumValue"), maximumValue);
    json.insert(QStringLiteral("totalDigits"), totalDigits);
    json.insert(QStringLiteral("decimalDigits"), decimalDigits);
    json.insert(QStringLiteral("pressedEffect"), pressedEffect);
    return json;
}

/******************************************************************************
 * Function Name   : WordButtonProperties::fromJson
 * Description     : Restore a complete word record transactionally and validate its active fields.
 * Input Parameters: json, properties, error
 * Return Value    : True when the output is replaced by a valid record.
 * Date            : 2026.11.09
 ******************************************************************************/
bool WordButtonProperties::fromJson(const QJsonObject &json, WordButtonProperties *properties, QString *error) //20261109-6 add
{
    WordButtonProperties result;

    if (properties == nullptr || json.value(QStringLiteral("version")).toInt() != 1)
    {
        return wordError(error, QStringLiteral("不支持的字按钮属性版本或输出为空。"));
    }
    result.dataType = json.value(QStringLiteral("dataType")).toInt(2);
    result.operation = json.value(QStringLiteral("operation")).toInt();
    result.constantValue = json.value(QStringLiteral("constantValue")).toString(QStringLiteral("1"));
    result.minimumValue = json.value(QStringLiteral("minimumValue")).toString(QStringLiteral("-32768"));
    result.maximumValue = json.value(QStringLiteral("maximumValue")).toString(QStringLiteral("32767"));
    result.totalDigits = json.value(QStringLiteral("totalDigits")).toInt(5);
    result.decimalDigits = json.value(QStringLiteral("decimalDigits")).toInt();
    result.pressedEffect = json.value(QStringLiteral("pressedEffect")).toBool();
    if (!result.validate(error)) { return false; }
    *properties = result;
    return true;
}
