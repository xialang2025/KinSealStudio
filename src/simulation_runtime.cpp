#include "simulation_runtime.h"

#include <QJsonDocument>
#include <QRegularExpression>
#include <cmath>
#include <cstring>
#include <limits>

namespace
{
constexpr int kLocalCount = 4097; //20261114-6 add: This memory bank is isolated from every physical connection.
constexpr int kEventLimit = 512;

/******************************************************************************
 * Function Name   : fail
 * Description     : Return a precise runtime refusal without modifying caller outputs.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.14
 ******************************************************************************/
bool fail(QString *error, const QString &message) //20261114-6 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : wordSpan
 * Description     : Map the seven word interpretations to their one or two local words.
 * Input Parameters: type
 * Return Value    : Word count, or zero for an unknown type.
 * Date            : 2026.11.14
 ******************************************************************************/
int wordSpan(int type) //20261114-6 add
{
    if (type < 0 || type > 6) { return 0; }
    return type == 1 || type == 3 || type == 5 || type == 6 ? 2 : 1;
}

/******************************************************************************
 * Function Name   : conditionType
 * Description     : Translate the five permission types to the seven word-operation types.
 * Input Parameters: type
 * Return Value    : Word type, or minus one for an invalid index.
 * Date            : 2026.11.14
 ******************************************************************************/
int conditionType(int type) //20261114-6 add
{
    return type >= 0 && type <= 4 ? (type == 4 ? 6 : type) : -1;
}

/******************************************************************************
 * Function Name   : localIndex
 * Description     : Reuse canonical PLC role and span validation while refusing qualified connections.
 * Input Parameters: text, bit, span, index, error
 * Return Value    : True for one isolated LB or LW range.
 * Date            : 2026.11.14
 ******************************************************************************/
bool localIndex(const QString &text, bool bit, int span, int *index, QString *error) //20261114-6 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress address;
    bool numeric;
    u32 value;

    if (span < 1 || span > 2 || text.contains(QChar(u'@')) || text.contains(QChar(u':')))
    { return fail(error, QStringLiteral("离线模拟只接受未限定连接的内部LB/LW地址。")); }
    context.localAddressCount = kLocalCount;
    context.transferCount = u32(span);
    context.role = bit ? PlcAddressRole::Bit : span == 2 ? PlcAddressRole::Dword : PlcAddressRole::Word;
    if (!parseQualifiedPlcAddressText(text, context, catalog, &address, error)) { return false; }
    if (address.space != PlcAddressSpace::Internal || !address.connectionId.isEmpty() ||
        address.family != (bit ? QStringLiteral("LB") : QStringLiteral("LW")))
    { return fail(error, QStringLiteral("模拟地址族与位/字用途不匹配。")); }
    value = address.value.toUInt(&numeric);
    if (!numeric || value >= u32(kLocalCount) || value + u32(span) > u32(kLocalCount))
    { return fail(error, QStringLiteral("模拟地址或跨字范围超出0..4096。")); }
    if (index != nullptr) { *index = int(value); }
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : encodeNumber
 * Description     : Encode a finite bounded value using explicit two's complement, packed BCD or IEEE float bits.
 * Input Parameters: value, type, raw, error
 * Return Value    : True without integer overflow or undefined signed conversions.
 * Date            : 2026.11.14
 ******************************************************************************/
bool encodeNumber(double value, int type, u32 *raw, QString *error) //20261114-6 add
{
    static const double lower[] = {0, 0, -32768, -2147483648.0, 0, 0, -double(std::numeric_limits<float>::max())};
    static const double upper[] = {65535, 4294967295.0, 32767, 2147483647.0, 9999, 99999999, double(std::numeric_limits<float>::max())};
    u32 result = 0;
    u32 decimal;
    float floating;
    int digit;

    static_assert(sizeof(float) == sizeof(u32) && std::numeric_limits<float>::is_iec559,
                  "The offline memory contract requires IEEE binary32.");
    if (type < 0 || type > 6 || !std::isfinite(value) || value < lower[type] || value > upper[type] ||
        (type != 6 && value != std::floor(value)))
    { return fail(error, QStringLiteral("数值超出所选模拟数据类型，未写入。")); }
    if (type == 6)
    {
        floating = float(value);
        if (!std::isfinite(floating) || (value != 0 && floating == 0))
        { return fail(error, QStringLiteral("浮点数溢出或下溢，未写入。")); }
        std::memcpy(&result, &floating, sizeof(result));
    }
    else if (type == 4 || type == 5)
    {
        decimal = u32(value);
        for (digit = 0; digit < (type == 4 ? 4 : 8); ++digit)
        {
            result |= (decimal % 10u) << (digit * 4);
            decimal /= 10u;
        }
    }
    else
    {
        result = u32(qint64(value));
        if (wordSpan(type) == 1) { result &= 0xffffu; }
    }
    if (raw != nullptr) { *raw = result; }
    return true;
}

/******************************************************************************
 * Function Name   : parseNumber
 * Description     : Require complete decimal syntax and quantize finite floats to binary32.
 * Input Parameters: text, type, value, error
 * Return Value    : True for a representable value, leaving output unchanged on failure.
 * Date            : 2026.11.14
 ******************************************************************************/
bool parseNumber(const QString &text, int type, double *value, QString *error) //20261114-6 add
{
    static const QRegularExpression integer(QStringLiteral("^[+-]?[0-9]+$"));
    static const QRegularExpression real(QStringLiteral("^[+-]?(?:[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)(?:[eE][+-]?[0-9]+)?$"));
    QString input;
    double number;
    bool converted;

    input = text.trimmed();
    if (type < 0 || type > 6 || input.size() > 128 || !(type == 6 ? real : integer).match(input).hasMatch())
    { return fail(error, QStringLiteral("模拟数值必须是完整的十进制数。")); }
    number = input.toDouble(&converted);
    if (!converted) { return fail(error, QStringLiteral("模拟数值无法转换为有限数值。")); }
    if (!encodeNumber(number, type, nullptr, error)) { return false; }
    if (type == 6) { number = double(float(number)); }
    if (value != nullptr) { *value = number; }
    return true;
}

/******************************************************************************
 * Function Name   : numberText
 * Description     : Return stable decimal text that round-trips a local typed value.
 * Input Parameters: value, type
 * Return Value    : Decimal text.
 * Date            : 2026.11.14
 ******************************************************************************/
QString numberText(double value, int type) //20261114-6 add
{
    return type == 6 ? QString::number(value, 'g', std::numeric_limits<float>::max_digits10)
                     : QString::number(qint64(value));
}

/******************************************************************************
 * Function Name   : readNumber
 * Description     : Decode low-word-first local storage and reject invalid BCD digits or nonfinite floats.
 * Input Parameters: words, index, type, value, error
 * Return Value    : True for one valid typed interpretation.
 * Date            : 2026.11.14
 ******************************************************************************/
bool readNumber(const QVector<u16> &words, int index, int type, double *value, QString *error) //20261114-6 add
{
    u32 raw;
    u32 bcd;
    u32 place = 1;
    u32 decimal = 0;
    float floating;
    double result;
    int digit;

    raw = words.at(index);
    if (wordSpan(type) == 2) { raw |= u32(words.at(index + 1)) << 16; }
    if (type == 6)
    {
        std::memcpy(&floating, &raw, sizeof(floating));
        if (!std::isfinite(floating)) { return fail(error, QStringLiteral("内存内容不是有限浮点数。")); }
        result = floating;
    }
    else if (type == 4 || type == 5)
    {
        bcd = raw;
        for (digit = 0; digit < (type == 4 ? 4 : 8); ++digit)
        {
            if ((bcd & 15u) > 9u) { return fail(error, QStringLiteral("内存包含非法BCD数位。")); }
            decimal += (bcd & 15u) * place;
            place *= 10u;
            bcd >>= 4;
        }
        result = decimal;
    }
    else if (type == 2) { result = raw >= 0x8000u ? double(raw) - 65536.0 : double(raw); }
    else if (type == 3) { result = raw >= 0x80000000u ? double(raw) - 4294967296.0 : double(raw); }
    else { result = raw; }
    if (value != nullptr) { *value = result; }
    return true;
}

/******************************************************************************
 * Function Name   : storeNumber
 * Description     : Stage both words only after the complete value has been encoded.
 * Input Parameters: words, address, type, value, error
 * Return Value    : True for a complete candidate-memory update.
 * Date            : 2026.11.14
 ******************************************************************************/
bool storeNumber(QVector<u16> *words, const QString &address, int type, double value, QString *error) //20261114-6 add
{
    int index;
    u32 raw;

    if (!localIndex(address, false, wordSpan(type), &index, error) || !encodeNumber(value, type, &raw, error)) { return false; }
    (*words)[index] = u16(raw & 0xffffu);
    if (wordSpan(type) == 2) { (*words)[index + 1] = u16(raw >> 16); }
    return true;
}

/******************************************************************************
 * Function Name   : addError
 * Description     : Keep capability refusals associated with a stable object and property.
 * Input Parameters: errors, id, property, message
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void addError(QJsonArray *errors, const QString &id, const QString &property, const QString &message) //20261114-6 add
{
    QJsonObject item;

    if (errors->size() >= 128) { return; }
    item.insert(QStringLiteral("objectId"), id.left(256));
    item.insert(QStringLiteral("property"), property.left(128));
    item.insert(QStringLiteral("message"), message.left(1024));
    errors->append(item);
}

/******************************************************************************
 * Function Name   : checkCondition
 * Description     : Admit one known condition while refusing unverified condition composition or tolerance semantics.
 * Input Parameters: properties, error
 * Return Value    : True for a bounded local-only gate.
 * Date            : 2026.11.14
 ******************************************************************************/
bool checkCondition(const StaticTextProperties &properties, QString *error) //20261114-6 add
{
    int active;
    double number;
    double tolerance;
    bool numeric;

    if (!properties.validate(error)) { return false; }
    if (!properties.visibilityEnabled) { return true; }
    active = int(properties.bitEnabled) + int(properties.levelEnabled) + int(properties.wordEnabled);
    if (active == 0) { return fail(error, QStringLiteral("权限已启用但没有活动条件，其原厂语义未核定。")); }
    if (active > 1) { return fail(error, QStringLiteral("多条件AND/OR尚未核定，离线模拟拒绝该权限组合。")); }
    if (properties.bitEnabled && !localIndex(properties.bitAddress, true, 1, nullptr, error)) { return false; }
    if (properties.wordEnabled)
    {
        tolerance = properties.tolerance.toDouble(&numeric);
        if (!numeric || tolerance != 0) { return fail(error, QStringLiteral("非零容差语义未核定，离线模拟拒绝该条件。")); }
        if (!localIndex(properties.wordAddress, false, wordSpan(conditionType(properties.dataType)), nullptr, error) ||
            !parseNumber(properties.compareValue, conditionType(properties.dataType), &number, error)) { return false; }
    }
    return true;
}

/******************************************************************************
 * Function Name   : checkNotice
 * Description     : Validate every active notification and refuse overlap with the operation's own range.
 * Input Parameters: object, error
 * Return Value    : True when notifications can join one unambiguous memory transaction.
 * Date            : 2026.11.14
 ******************************************************************************/
bool checkNotice(const HmiObject &object, QString *error) //20261114-6 add
{
    const BitButtonProperties &shared = object.bitButton;
    int target;
    int notice;
    int span;
    double value;

    if (!shared.notice) { return true; }
    if (shared.noticeBit)
    {
        if (!localIndex(shared.noticeBitAddress, true, 1, &notice, error)) { return false; }
        if (!object.hasWordButtonProperties && !object.hasFunctionButtonProperties && !object.hasLampProperties &&
            localIndex(object.address, true, 1, &target, nullptr) && target == notice)
        { return fail(error, QStringLiteral("通知位与操作位重叠，事件顺序未核定。")); }
    }
    if (shared.noticeWord)
    {
        span = wordSpan(conditionType(shared.noticeWordType));
        if (!localIndex(shared.noticeWordAddress, false, span, &notice, error) ||
            !parseNumber(shared.noticeWordValue, conditionType(shared.noticeWordType), &value, error)) { return false; }
        if (object.hasWordButtonProperties && localIndex(object.address, false, object.wordButton.transferWordCount(), &target, nullptr) &&
            target < notice + span && notice < target + object.wordButton.transferWordCount())
        { return fail(error, QStringLiteral("通知字与操作字跨字重叠，事件顺序未核定。")); }
    }
    return true;
}

/******************************************************************************
 * Function Name   : checkObject
 * Description     : Reject unsupported control behavior before any runtime session can replace its predecessor.
 * Input Parameters: object, project, error
 * Return Value    : True for one of the five bounded control families.
 * Date            : 2026.11.14
 ******************************************************************************/
bool checkObject(const HmiObject &object, const HmiProject &project, QString *error) //20261114-6 add
{
    const BitButtonProperties &shared = object.bitButton;
    BitButtonState appearance;

    if (object.type == QStringLiteral("text"))
    { return !object.hasStaticTextProperties || checkCondition(object.staticText, error); }
    if (object.type != QStringLiteral("switch") || !object.hasBitButtonProperties)
    { return fail(error, QStringLiteral("离线模拟尚不支持此对象类型：%1。").arg(object.type)); }
    if (!shared.validate(error, object.hasFunctionButtonProperties) || !checkCondition(shared.visibility, error) ||
        !checkCondition(shared.touch, error) || !checkNotice(object, error)) { return false; }
    if (shared.pressTimeEnabled || shared.states[0].flash || shared.states[1].flash)
    { return fail(error, QStringLiteral("按压计时/闪烁时序尚未核定，离线模拟不推测单位或帧率。")); }
    if (shared.states[0].text.visibilityEnabled || shared.states[1].text.visibilityEnabled)
    { return fail(error, QStringLiteral("按钮状态内嵌文字的独立权限尚未接入模拟，不能静默忽略。")); }
    if (object.hasWordButtonProperties)
    {
        if (!object.wordButton.validate(error) || !localIndex(object.address, false, object.wordButton.transferWordCount(), nullptr, error)) { return false; }
        if (object.wordButton.operation == 1 && object.wordButton.dataType != 6 && object.wordButton.decimalDigits != 0)
        { return fail(error, QStringLiteral("整数字的定点小数缩放未核定，离线模拟不缩放输入。")); }
        return true;
    }
    if (object.hasFunctionButtonProperties)
    {
        if (!object.functionButton.validate(error)) { return false; }
        if (object.functionButton.function != 8 && object.functionButton.function != 13)
        { return fail(error, QStringLiteral("离线模拟未实现系统功能：%1。").arg(FunctionButtonProperties::functions().value(object.functionButton.function))); }
        if (object.functionButton.function == 13 && object.functionButton.type >= project.languages.size())
        { return fail(error, QStringLiteral("语言按钮目标不在工程语言列表。")); }
        return true;
    }
    if (object.hasLampProperties)
    {
        if (!object.lamp.validate(error)) { return false; }
        Q_FOREACH (appearance, object.lamp.extraStates)
        {
            if (appearance.flash || appearance.text.visibilityEnabled)
            { return fail(error, QStringLiteral("多态灯的闪烁/状态内嵌文字权限未核定。")); }
        }
        if (object.lamp.monitorMode == 2)
        { return localIndex(object.lamp.multiMonitorAddress, false, object.lamp.transferWordCount(), nullptr, error); }
    }
    else if (!localIndex(object.address, true, 1, nullptr, error)) { return false; }
    if (shared.indicator && shared.dualMonitor && !shared.monitorSame)
    {
        return localIndex(shared.monitorType == 0 ? shared.monitorBitAddress : shared.monitorWordAddress,
                          shared.monitorType == 0, 1, nullptr, error);
    }
    return true;
}
}

/******************************************************************************
 * Function Name   : SimulationRuntime::capabilities
 * Description     : Describe all refusals and design boundaries without starting a session or touching hardware.
 * Input Parameters: project
 * Return Value    : Structured support and warning report.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject SimulationRuntime::capabilities(const HmiProject &project) //20261114-6 add
{
    QJsonObject result;
    QJsonArray errors;
    QJsonArray warnings;
    QString error;
    HmiObject object;
    HmiConfigurationNode node;

    if (!project.validateModel(&error)) { addError(&errors, QString(), QStringLiteral("model"), error); }
    if (project.pageCount() != 1 || project.activePageKind != QStringLiteral("normal") ||
        project.startupPageId != project.activePageId || project.pageWidth != 800 || project.pageHeight != 480)
    { addError(&errors, QString(), QStringLiteral("pages"), QStringLiteral("离线运行器只支持单个800×480普通启动画面。")); }
    if (!project.nativeSourceSnapshot.isEmpty())
    { addError(&errors, QString(), QStringLiteral("nativeSourceSnapshot"), QStringLiteral("不透明原厂快照的初值/保持及全局行为未核定；请使用明确的Lite模型。")); }
    Q_FOREACH (node, project.configurationNodes)
    {
        if (node.active) { addError(&errors, node.id, QStringLiteral("configuration"), QStringLiteral("活动全局配置尚未接入离线调度：%1。").arg(node.kind)); }
    }
    if (!project.animations.isEmpty()) { addError(&errors, QString(), QStringLiteral("animations"), QStringLiteral("动画时序尚未实现。")); }
    Q_FOREACH (object, project.objects)
    {
        error.clear();
        if (!checkObject(object, project, &error)) { addError(&errors, object.id, QStringLiteral("runtime"), error); }
        if (object.hasBitButtonProperties && object.bitButton.macroEnabled && warnings.size() < 128)
        { warnings.append(QStringLiteral("对象%1绑定宏%2；离线模式跳过执行。").arg(object.id.left(256), object.bitButton.macroName.left(128))); }
    }
    if (!project.macros.isEmpty()) { warnings.append(QStringLiteral("离线模式保存宏来源但不执行宏，不模拟初始化/周期/事件调度。")); }
    warnings.append(QStringLiteral("LB/LW共各4097项、零初值、32位低字优先和等级0是Lite运行规则，不是原厂保持/字序的验证结论。"));
    result.insert(QStringLiteral("format"), QStringLiteral("KinSealLite-offline-capabilities-v1"));
    result.insert(QStringLiteral("supported"), errors.isEmpty());
    result.insert(QStringLiteral("ok"), errors.isEmpty());
    result.insert(QStringLiteral("hardwareIo"), false);
    result.insert(QStringLiteral("macroExecution"), false);
    result.insert(QStringLiteral("errors"), errors);
    result.insert(QStringLiteral("warnings"), warnings);
    result.insert(QStringLiteral("localAddressCount"), kLocalCount);
    result.insert(QStringLiteral("wordOrder"), QStringLiteral("low-word-first-lite-rule"));
    result.insert(QStringLiteral("maximumEvents"), kEventLimit);
    return result;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::appendEvent
 * Description     : Retain a bounded lifecycle history without logging entered values or macro bodies.
 * Input Parameters: kind, id, detail
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntime::appendEvent(const QString &kind, const QString &id, const QString &detail) //20261114-6 add
{
    QJsonObject event;

    event.insert(QStringLiteral("sequence"), double(++m_sequence));
    event.insert(QStringLiteral("kind"), kind);
    event.insert(QStringLiteral("objectId"), id.left(256));
    event.insert(QStringLiteral("detail"), detail.left(1024));
    m_events.append(event);
    while (m_events.size() > kEventLimit) { m_events.removeFirst(); }
}

/******************************************************************************
 * Function Name   : SimulationRuntime::start
 * Description     : Validate a detached project before replacing memory, language, access and interaction state.
 * Input Parameters: project, error
 * Return Value    : True after a new zero-initialized session is published.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::start(const HmiProject &project, QString *error) //20261114-6 add
{
    QJsonObject report;

    report = capabilities(project);
    if (!report.value(QStringLiteral("supported")).toBool())
    { return fail(error, QString::fromUtf8(QJsonDocument(report.value(QStringLiteral("errors")).toArray()).toJson(QJsonDocument::Compact))); }
    if (m_running) { stop(QStringLiteral("session-replaced")); }
    m_project = project;
    m_bits.fill(0, kLocalCount);
    m_words.fill(0, kLocalCount);
    m_warnings = report.value(QStringLiteral("warnings")).toArray();
    m_language = project.startupLanguage.isEmpty() ? project.languages.first() : project.startupLanguage;
    m_userLevel = 0;
    m_pressed.clear();
    m_pendingInput.clear();
    m_momentaryIndex = -1;
    m_running = true;
    appendEvent(QStringLiteral("start"), QString(), QStringLiteral("zero-initialized; low-word-first; no hardware I/O"));
    if (!project.macros.isEmpty()) { appendEvent(QStringLiteral("macro-skipped"), QString(), QStringLiteral("offline-start: no macro scheduler")); }
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::running
 * Description     : Read the in-memory session lifecycle state.
 * Input Parameters: None
 * Return Value    : True for an active offline session.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::running() const //20261114-6 add
{
    return m_running;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::stop
 * Description     : Clear pending input and force a held momentary bit false before stopping.
 * Input Parameters: reason
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntime::stop(const QString &reason) //20261114-6 add
{
    cancel(reason.isEmpty() ? QStringLiteral("stop") : reason);
    m_running = false;
    appendEvent(QStringLiteral("stop"), QString(), reason);
}

/******************************************************************************
 * Function Name   : SimulationRuntime::project
 * Description     : Expose only the runtime-owned immutable input value.
 * Input Parameters: None
 * Return Value    : Frozen project reference.
 * Date            : 2026.11.14
 ******************************************************************************/
const HmiProject &SimulationRuntime::project() const //20261114-6 add
{
    return m_project;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::language
 * Description     : Read the current session language without changing the editor project.
 * Input Parameters: None
 * Return Value    : Language name.
 * Date            : 2026.11.14
 ******************************************************************************/
QString SimulationRuntime::language() const //20261114-6 add
{
    return m_language;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::userLevel
 * Description     : Read the isolated simulation access level.
 * Input Parameters: None
 * Return Value    : Level zero through eight.
 * Date            : 2026.11.14
 ******************************************************************************/
int SimulationRuntime::userLevel() const //20261114-6 add
{
    return m_userLevel;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::setUserLevel
 * Description     : Apply a test access stimulus and release interactions whose permission is lost.
 * Input Parameters: level, error
 * Return Value    : True for an active session and a valid level.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::setUserLevel(int level, QString *error) //20261114-6 add
{
    if (!m_running || level < 0 || level > 8) { return fail(error, QStringLiteral("模拟等级必须为0..8且运行器已启动。")); }
    m_userLevel = level;
    appendEvent(QStringLiteral("user-level"));
    enforceAccess();
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::writeBit
 * Description     : Apply one explicit internal-bit stimulus with no external address fallback.
 * Input Parameters: address, value, error
 * Return Value    : True after the memory write.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::writeBit(const QString &address, bool value, QString *error) //20261114-6 add
{
    int index;

    if (!m_running) { return fail(error, QStringLiteral("离线运行器未启动。")); }
    if (!localIndex(address, true, 1, &index, error)) { return false; }
    m_bits[index] = u8(value);
    appendEvent(QStringLiteral("bit-stimulus"), QString(), QStringLiteral("LB%1").arg(index));
    enforceAccess();
    return true;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::readBit
 * Description     : Inspect local bit memory including after stop, preserving output on refusal.
 * Input Parameters: address, value, error
 * Return Value    : True for a valid internal bit and non-null output.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::readBit(const QString &address, bool *value, QString *error) const //20261114-6 add
{
    int index;

    if (value == nullptr) { return fail(error, QStringLiteral("位读取输出为空。")); }
    if (!localIndex(address, true, 1, &index, error)) { return false; }
    *value = m_bits.at(index) != 0;
    return true;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::writeWord
 * Description     : Stage a complete typed value before writing one or two low-first words.
 * Input Parameters: address, type, value, error
 * Return Value    : True after an atomic memory update.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::writeWord(const QString &address, int type, const QString &value, QString *error) //20261114-6 add
{
    QVector<u16> candidate;
    double number;

    if (!m_running) { return fail(error, QStringLiteral("离线运行器未启动。")); }
    if (!parseNumber(value, type, &number, error)) { return false; }
    candidate = m_words;
    if (!storeNumber(&candidate, address, type, number, error)) { return false; }
    m_words = candidate;
    appendEvent(QStringLiteral("word-stimulus"), QString(), address);
    enforceAccess();
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::readWord
 * Description     : Read a bounded typed interpretation without exposing raw-memory mutation.
 * Input Parameters: address, type, value, error
 * Return Value    : True for a finite valid interpretation and non-null output.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::readWord(const QString &address, int type, QString *value, QString *error) const //20261114-6 add
{
    int index;
    double number;

    if (value == nullptr) { return fail(error, QStringLiteral("字读取输出为空。")); }
    if (!localIndex(address, false, wordSpan(type), &index, error) || !readNumber(m_words, index, type, &number, error)) { return false; }
    *value = numberText(number, type);
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::findObject
 * Description     : Resolve object identity against the frozen runtime project only.
 * Input Parameters: id
 * Return Value    : Stable object pointer, or null.
 * Date            : 2026.11.14
 ******************************************************************************/
const HmiObject *SimulationRuntime::findObject(const QString &id) const //20261114-6 add
{
    int index;

    for (index = 0; index < m_project.objects.size(); ++index)
    {
        if (m_project.objects.at(index).id == id) { return &m_project.objects.at(index); }
    }
    return nullptr;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::condition
 * Description     : Evaluate one validated bit, level or typed-word predicate using the model enum order.
 * Input Parameters: properties
 * Return Value    : Predicate value, false for unreadable memory.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::condition(const StaticTextProperties &properties) const //20261114-6 add
{
    bool bit;
    int index;
    int type;
    double value;
    double expected;

    if (!properties.visibilityEnabled) { return true; }
    if (properties.bitEnabled)
    { return readBit(properties.bitAddress, &bit, nullptr) && bit == properties.bitState; }
    if (properties.levelEnabled) { return m_userLevel >= properties.minimumLevel; }
    if (!properties.wordEnabled) { return true; }
    type = conditionType(properties.dataType);
    if (!localIndex(properties.wordAddress, false, wordSpan(type), &index, nullptr) ||
        !readNumber(m_words, index, type, &value, nullptr) || !parseNumber(properties.compareValue, type, &expected, nullptr)) { return false; }
    switch (properties.comparison)
    {
    case 0: return value < expected;
    case 1: return value > expected;
    case 2: return value == expected;
    case 3: return value != expected;
    case 4: return value <= expected;
    case 5: return value >= expected;
    default: return false;
    }
}

/******************************************************************************
 * Function Name   : SimulationRuntime::isVisible
 * Description     : Evaluate visibility from the immutable object's active permission carrier.
 * Input Parameters: object
 * Return Value    : True for a running, owned and visible object.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::isVisible(const HmiObject &object) const //20261114-6 add
{
    const HmiObject *owned;

    owned = findObject(object.id);
    if (!m_running || owned == nullptr) { return false; }
    return owned->type == QStringLiteral("text") ? (!owned->hasStaticTextProperties || condition(owned->staticText))
                                                  : condition(owned->bitButton.visibility);
}

/******************************************************************************
 * Function Name   : SimulationRuntime::touchAllowed
 * Description     : Exclude display-only controls and combine visibility with one validated touch gate.
 * Input Parameters: object
 * Return Value    : True for an owned interactive button currently allowed by access state.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::touchAllowed(const HmiObject &object) const //20261114-6 add
{
    const HmiObject *owned;

    owned = findObject(object.id);
    return owned != nullptr && owned->type == QStringLiteral("switch") && !owned->hasLampProperties &&
           isVisible(*owned) && condition(owned->bitButton.touch);
}

/******************************************************************************
 * Function Name   : SimulationRuntime::enforceAccess
 * Description     : Immediately clear a held momentary operation or pending input when its gate is lost.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntime::enforceAccess() //20261114-6 add
{
    const HmiObject *object;

    object = findObject(m_pressed.isEmpty() ? m_pendingInput : m_pressed);
    if (object != nullptr && !touchAllowed(*object)) { cancel(QStringLiteral("access-changed")); }
}

/******************************************************************************
 * Function Name   : SimulationRuntime::applyAction
 * Description     : Stage operation, notifications and local function changes before publishing one event transaction.
 * Input Parameters: object, input, error
 * Return Value    : True only after the complete action is committed.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::applyAction(const HmiObject &object, const QString *input, QString *error) //20261114-6 add
{
    QVector<u8> bits = m_bits;
    QVector<u16> words = m_words;
    const BitButtonProperties &shared = object.bitButton;
    const WordButtonProperties &word = object.wordButton;
    QString nextLanguage = m_language;
    int nextLevel = m_userLevel;
    int index;
    double current = 0;
    double constant = 0;
    double minimum = 0;
    double maximum = 0;
    double value = 0;
    QString digits;
    int point;

    if (object.hasFunctionButtonProperties)
    {
        if (object.functionButton.function == 8) { nextLevel = 0; }
        else if (object.functionButton.function == 13) { nextLanguage = m_project.languages.at(object.functionButton.type); }
        else { return fail(error, QStringLiteral("离线系统功能未实现。")); }
    }
    else if (object.hasWordButtonProperties)
    {
        if (!localIndex(object.address, false, word.transferWordCount(), &index, error)) { return false; }
        if (word.operation >= 3 && !readNumber(words, index, word.dataType, &current, error)) { return false; }
        if ((word.usesConstant() && !parseNumber(word.constantValue, word.dataType, &constant, error)) ||
            (word.usesMinimum() && !parseNumber(word.minimumValue, word.dataType, &minimum, error)) ||
            (word.usesMaximum() && !parseNumber(word.maximumValue, word.dataType, &maximum, error))) { return false; }
        if (word.operation == 0) { value = constant; }
        else if (word.operation == 1 || word.operation == 2)
        {
            if (input == nullptr || !parseNumber(*input, word.dataType, &value, error)) { return false; }
            digits = input->trimmed();
            if (digits.startsWith(QChar(u'+')) || digits.startsWith(QChar(u'-'))) { digits.remove(0, 1); }
            point = int(digits.indexOf(QChar(u'.')));
            if (digits.contains(QChar(u'e'), Qt::CaseInsensitive) ||
                (point >= 0 && (word.operation == 2 || digits.size() - point - 1 > word.decimalDigits)))
            { return fail(error, QStringLiteral("键盘输入小数位超出当前布局；不接受指数输入。")); }
            digits.remove(QChar(u'.'));
            if (digits.size() > word.totalDigits || value < minimum || value > maximum)
            { return fail(error, QStringLiteral("键盘输入超出位数或最小/最大值，事务未写入。")); }
        }
        else
        {
            value = word.operation == 3 || word.operation == 5 ? current + constant : current - constant;
            if (word.operation >= 5)
            {
                if (current < minimum || current > maximum) { return fail(error, QStringLiteral("当前绕回值不在设定区间。")); }
                if (word.operation == 5 && value > maximum) { value = minimum; }
                else if (word.operation == 6 && value < minimum) { value = maximum; }
                if (value < minimum || value > maximum) { return fail(error, QStringLiteral("绕回运算越过未定义的相反边界。")); }
            }
            else if ((word.operation == 3 && value > maximum) || (word.operation == 4 && value < minimum))
            { return fail(error, QStringLiteral("加减运算超出配置边界，事务未写入。")); }
        }
        if (!storeNumber(&words, object.address, word.dataType, value, error)) { return false; }
    }
    else
    {
        if (!localIndex(object.address, true, 1, &index, error)) { return false; }
        if (shared.operation == 0 || shared.operation == 2) { bits[index] = 1; }
        else if (shared.operation == 1) { bits[index] = 0; }
        else if (shared.operation == 3) { bits[index] = u8(bits.at(index) == 0); }
        else { return fail(error, QStringLiteral("未知位操作。")); }
    }
    if (shared.notice && shared.noticeBit)
    {
        if (!localIndex(shared.noticeBitAddress, true, 1, &index, error)) { return false; }
        bits[index] = u8(shared.noticeBitValue);
    }
    if (shared.notice && shared.noticeWord)
    {
        if (!parseNumber(shared.noticeWordValue, conditionType(shared.noticeWordType), &value, error) ||
            !storeNumber(&words, shared.noticeWordAddress, conditionType(shared.noticeWordType), value, error)) { return false; }
    }
    m_bits = bits;
    m_words = words;
    m_language = nextLanguage;
    m_userLevel = nextLevel;
    appendEvent(input == nullptr ? QStringLiteral("action") : QStringLiteral("input-accepted"), object.id);
    if (shared.macroEnabled) { appendEvent(QStringLiteral("macro-skipped"), object.id, QStringLiteral("offline mode does not execute bound macros")); }
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::press
 * Description     : Begin one permitted interaction; input buttons defer all writes and notices until submission.
 * Input Parameters: id, error
 * Return Value    : True for an accepted button press.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::press(const QString &id, QString *error) //20261114-6 add
{
    const HmiObject *object;
    int index;

    if (!m_running || !m_pressed.isEmpty() || !m_pendingInput.isEmpty())
    { return fail(error, QStringLiteral("模拟未启动或已有尚未结束的按压/输入。")); }
    object = findObject(id);
    if (object == nullptr || !touchAllowed(*object)) { return fail(error, QStringLiteral("对象不可触摸、隐藏或权限不足。")); }
    if (object->hasWordButtonProperties && (object->wordButton.operation == 1 || object->wordButton.operation == 2))
    {
        m_pendingInput = id;
        m_pressed = id;
        appendEvent(QStringLiteral("input-requested"), id);
        if (error != nullptr) { error->clear(); }
        return true;
    }
    if (!applyAction(*object, nullptr, error)) { return false; }
    m_pressed = id;
    if (!object->hasWordButtonProperties && !object->hasFunctionButtonProperties && object->bitButton.operation == 2)
    {
        if (localIndex(object->address, true, 1, &index, nullptr)) { m_momentaryIndex = index; }
    }
    enforceAccess();
    return true;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::release
 * Description     : End visual pressing and safely clear momentary memory even after pointer movement or access changes.
 * Input Parameters: inside, error
 * Return Value    : True when the session can process a release, including an idempotent release.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::release(bool inside, QString *error) //20261114-6 add
{
    QString id;

    if (!m_running) { return fail(error, QStringLiteral("离线运行器未启动。")); }
    id = m_pressed;
    if (m_momentaryIndex >= 0) { m_bits[m_momentaryIndex] = 0; }
    m_momentaryIndex = -1;
    m_pressed.clear();
    if (!inside) { m_pendingInput.clear(); }
    appendEvent(inside ? QStringLiteral("release") : QStringLiteral("move-out"), id);
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::cancel
 * Description     : Discard pending input and force only the owned momentary bit to safe false.
 * Input Parameters: reason
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationRuntime::cancel(const QString &reason) //20261114-6 add
{
    QString id = m_pressed.isEmpty() ? m_pendingInput : m_pressed;

    if (m_momentaryIndex >= 0) { m_bits[m_momentaryIndex] = 0; }
    m_momentaryIndex = -1;
    m_pressed.clear();
    m_pendingInput.clear();
    appendEvent(QStringLiteral("cancel"), id, reason);
}

/******************************************************************************
 * Function Name   : SimulationRuntime::submitInput
 * Description     : Recheck permission and atomically apply the pending value with its notifications.
 * Input Parameters: value, error
 * Return Value    : True on complete submission; failed values retain the pending request for correction.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::submitInput(const QString &value, QString *error) //20261114-6 add
{
    const HmiObject *object;

    object = findObject(m_pendingInput);
    if (!m_running || object == nullptr || !touchAllowed(*object))
    { return fail(error, QStringLiteral("没有有效输入请求或输入权限已经失效。")); }
    if (!applyAction(*object, &value, error)) { return false; }
    m_pendingInput.clear();
    m_pressed.clear();
    enforceAccess();
    return true;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::pendingInputObjectId
 * Description     : Expose the one pending input owner for a separate UI keypad.
 * Input Parameters: None
 * Return Value    : Stable object ID, or empty.
 * Date            : 2026.11.14
 ******************************************************************************/
QString SimulationRuntime::pendingInputObjectId() const //20261114-6 add
{
    return m_pendingInput;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::isPressed
 * Description     : Query visual pressing independently of monitored values.
 * Input Parameters: id
 * Return Value    : True only for the active owned pointer interaction.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationRuntime::isPressed(const QString &id) const //20261114-6 add
{
    return m_running && !id.isEmpty() && m_pressed == id;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::visualState
 * Description     : Resolve button effects, bit or word-bit monitors, and bounded multi-state lamps.
 * Input Parameters: object
 * Return Value    : State index; minus one for unknown identity or an unrepresented monitored value.
 * Date            : 2026.11.14
 ******************************************************************************/
int SimulationRuntime::visualState(const HmiObject &object) const //20261114-6 add
{
    const HmiObject *owned;
    const BitButtonProperties *shared;
    int index;
    double value;
    bool bit;

    owned = findObject(object.id);
    if (!m_running || owned == nullptr) { return -1; }
    if (owned->type == QStringLiteral("text")) { return 0; }
    shared = &owned->bitButton;
    if (owned->hasWordButtonProperties) { return owned->wordButton.pressedEffect && isPressed(owned->id) ? 1 : 0; }
    if (owned->hasFunctionButtonProperties) { return owned->functionButton.pressedEffect && isPressed(owned->id) ? 1 : 0; }
    if (owned->hasLampProperties && owned->lamp.monitorMode == 2)
    {
        if (!localIndex(owned->lamp.multiMonitorAddress, false, owned->lamp.transferWordCount(), &index, nullptr) ||
            !readNumber(m_words, index, owned->lamp.multiDataType, &value, nullptr) || value >= owned->lamp.stateCount()) { return -1; }
        return int(value);
    }
    if (!shared->indicator || !shared->dualMonitor) { return 0; }
    if (shared->monitorSame)
    { return readBit(owned->address, &bit, nullptr) ? int(bit) : -1; }
    if (shared->monitorType == 0)
    { return readBit(shared->monitorBitAddress, &bit, nullptr) ? int(bit) : -1; }
    if (!localIndex(shared->monitorWordAddress, false, 1, &index, nullptr)) { return -1; }
    return int((m_words.at(index) >> shared->bitIndex) & 1u);
}

/******************************************************************************
 * Function Name   : SimulationRuntime::snapshot
 * Description     : Export bounded sparse banks and lifecycle evidence without claiming native runtime equivalence.
 * Input Parameters: None
 * Return Value    : Detached JSON snapshot.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject SimulationRuntime::snapshot() const //20261114-6 add
{
    QJsonObject result;
    QJsonObject bits;
    QJsonObject words;
    int index;

    for (index = 0; index < kLocalCount; ++index)
    {
        if (m_bits.at(index) != 0) { bits.insert(QStringLiteral("LB%1").arg(index), true); }
        if (m_words.at(index) != 0) { words.insert(QStringLiteral("LW%1").arg(index), m_words.at(index)); }
    }
    result.insert(QStringLiteral("format"), QStringLiteral("KinSealLite-offline-memory-v1"));
    result.insert(QStringLiteral("running"), m_running);
    result.insert(QStringLiteral("projectId"), m_project.projectId);
    result.insert(QStringLiteral("language"), m_language);
    result.insert(QStringLiteral("userLevel"), m_userLevel);
    result.insert(QStringLiteral("pressedObjectId"), m_pressed);
    result.insert(QStringLiteral("pendingInputObjectId"), m_pendingInput);
    result.insert(QStringLiteral("LB"), bits);
    result.insert(QStringLiteral("LW"), words);
    result.insert(QStringLiteral("events"), m_events);
    result.insert(QStringLiteral("warnings"), m_warnings);
    result.insert(QStringLiteral("hardwareIo"), false);
    result.insert(QStringLiteral("macroExecution"), false);
    result.insert(QStringLiteral("wordOrder"), QStringLiteral("low-word-first-lite-rule"));
    result.insert(QStringLiteral("zeroInitialValuesAreLiteRule"), true);
    return result;
}

/******************************************************************************
 * Function Name   : SimulationRuntime::events
 * Description     : Return a value copy of the bounded event history across lifecycle changes.
 * Input Parameters: None
 * Return Value    : Up to 512 events.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonArray SimulationRuntime::events() const //20261114-6 add
{
    return m_events;
}
