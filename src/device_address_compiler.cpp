#include "device_address_compiler.h"
#include "device_scan_table.h" //20261109-0 add

#include <QtEndian>

namespace
{
using ScanRange = DeviceScanRead; //20261109-0 change

/******************************************************************************
 * Function Name   : addressCompileError
 * Description     : Reject a device configuration before any payload or USB side effect.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.09
 ******************************************************************************/
bool addressCompileError(QString *error, const QString &message) //20261109-0 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : addScanAddress
 * Description     : Collect only external reads, preserving the verified route/type/node tuple.
 * Input Parameters: compiler, text, word, count, ranges, error
 * Return Value    : True when no external read is needed or a verified read was collected.
 * Date            : 2026.11.09
 ******************************************************************************/
bool addScanAddress(const DeviceAddressCompiler &compiler, const QString &text, bool word,
                    u32 count, QVector<ScanRange> *ranges, QString *error) //20261109-0 add
{
    QByteArray record;
    ScanRange range;

    if (!compiler.encode(text, word, false, &record, error, count)) { return false; }
    if (u8(record[0]) == 0) { return true; } //20261109-0 change: Validate local read bounds without creating a PLC subscription.
    range.type = u8(record[1]);
    range.route = u8(record[0]);
    range.node = u8(record[2]);
    range.start = qFromLittleEndian<u32>(record.constData() + 8);
    range.count = count;
    ranges->append(range);
    return true;
}

/******************************************************************************
 * Function Name   : addConditionReads
 * Description     : Add enabled bit and numeric access conditions without scanning dormant fields.
 * Input Parameters: compiler, condition, ranges, error
 * Return Value    : True after all active reads were validated.
 * Date            : 2026.11.09
 ******************************************************************************/
bool addConditionReads(const DeviceAddressCompiler &compiler, const StaticTextProperties &condition,
                       QVector<ScanRange> *ranges, QString *error) //20261109-0 add
{
    if (!condition.visibilityEnabled) { return true; }
    if (condition.bitEnabled &&
        !addScanAddress(compiler, condition.bitAddress, false, 1, ranges, error)) { return false; }
    if (condition.wordEnabled &&
        !addScanAddress(compiler, condition.wordAddress, true,
                        (condition.dataType == 0 || condition.dataType == 2) ? 1U : 2U,
                        ranges, error)) { return false; }
    return true;
}

/******************************************************************************
 * Function Name   : encodeScanTable
 * Description     : Write the native 200 page-count bytes and bounded twelve-byte read segments.
 * Input Parameters: reads, scanTable, error
 * Return Value    : True with a transactional one-page scan table.
 * Date            : 2026.11.09
 ******************************************************************************/
bool encodeScanTable(const QVector<ScanRange> &reads, QByteArray *scanTable, QString *error) //20261109-0 change
{
    return buildDeviceScanTable(reads, scanTable, error, 32);
}
}

/******************************************************************************
 * Function Name   : DeviceAddressCompiler::encode
 * Description     : Validate a local or verified default Modbus address and emit a clean complete control slot.
 * Input Parameters: text, word, toggle, record, error, transferCount
 * Return Value    : True for a validated external address; output is unchanged on failure.
 * Date            : 2026.11.09
 ******************************************************************************/
bool DeviceAddressCompiler::encode(const QString &text, bool word, bool toggle,
                                   QByteArray *record, QString *error, u32 transferCount) const //20261109-0 add
{
    PlcAddressContext context;
    PlcAddress address;
    PlcNativeBinding binding;
    PlcNativeAddressRecord encoded;
    QByteArray result;

    if (!m_ready || record == nullptr)
    {
        return addressCompileError(error, QStringLiteral("PLC设备地址编译器未初始化或输出为空。"));
    }
    context = m_context; context.role = word ? PlcAddressRole::Word : PlcAddressRole::Bit;
    context.transferCount = transferCount; //20261109-0 add: Native DWORD access consumes two source words.
    if (!parseQualifiedPlcAddressText(text, context, m_catalog, &address, error)) { return false; }
    binding.connectionId = address.connectionId; binding.channel = PlcChannel::Com1;
    binding.channelByte = 1; binding.commonV51CodecVerified = true; binding.routeVerified = true;
    binding.evidence = QStringLiteral("Common 0x6414796c/0x641484dc; native COM1 0x/4x UI compilation and scan oracles");
    if (!encodePlcNativeAddress(address, context, m_catalog, binding, &encoded, error) ||
        !encoded.overlay(QByteArray(36, char(0)), &result, error)) { return false; }
    result[5] = char(toggle ? 1 : 0);
    *record = result;
    return true;
}

/******************************************************************************
 * Function Name   : DeviceAddressCompiler::prepare
 * Description     : Bind the embedded default COM1 driver and collect only runtime read addresses.
 * Input Parameters: project, scanTable, error
 * Return Value    : True with a complete scan table; the caller output survives any failure.
 * Date            : 2026.11.09
 ******************************************************************************/
bool DeviceAddressCompiler::prepare(const HmiProject &project, QByteArray *scanTable,
                                    QString *error) //20261109-0 add
{
    DeviceAddressCompiler prepared;
    QVector<ScanRange> reads;
    PlcConnection connection;
    HmiObject object;
    QString monitor;
    bool word;
    QByteArray result;
    QByteArray writeRecord;

    if (scanTable == nullptr || !project.validateSinglePageBackend(error) ||
        !project.validatePlcConfiguration(error)) { return false; } //20261110-5 change: Do not silently omit scans from inactive pages.
    if (!prepared.m_catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    Q_FOREACH (connection, project.connections)
    {
        if (project.connections.size() != 1 || connection.profileId != QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini") ||
            connection.channel != PlcChannel::Com1 || connection.station != QStringLiteral("1") ||
            !connection.nodeKnown || connection.node != 1)
        {
            return addressCompileError(error, QStringLiteral("设备编译当前支持默认Modbus RTU COM1、站号1和已确认节点1；所选PLC驱动参数尚未恢复，未打开USB。"));
        }
    }
    prepared.m_context.connections = project.connections;
    prepared.m_ready = true;
    Q_FOREACH (object, project.objects)
    {
        if (object.type == QStringLiteral("text") && object.hasStaticTextProperties) //20261109-0 add
        {
            if (!addConditionReads(prepared, object.staticText, &reads, error)) { return false; }
        }
        if (object.type != QStringLiteral("switch")) { continue; }
        if (!object.hasLampProperties && !object.hasFunctionButtonProperties && //20261110-0 change: A system function has no operation write slot.
            !prepared.encode(object.address, object.hasWordButtonProperties,
            object.hasWordButtonProperties ? object.wordButton.operation >= 3 : object.bitButton.operation == 3,
            &writeRecord, error, object.hasWordButtonProperties ? u32(object.wordButton.transferWordCount()) : 1U)) //20261109-6 change
        {
            return false; //20261109-0 add: Direct project files must obey the same local address bound as the picker.
        }
        if (object.bitButton.notice && object.bitButton.noticeBit &&
            !prepared.encode(object.bitButton.noticeBitAddress, false, false, &writeRecord, error)) { return false; }
        if (object.bitButton.notice && object.bitButton.noticeWord &&
            !prepared.encode(object.bitButton.noticeWordAddress, true, false, &writeRecord, error,
                             (object.bitButton.noticeWordType == 0 || object.bitButton.noticeWordType == 2) ? 1U : 2U))
        {
            return false; //20261109-0 add: Validate wide writes too, while keeping them out of the scan table.
        }
        if (!object.hasLampProperties && !object.hasFunctionButtonProperties && //20261110-0 change
            !object.hasWordButtonProperties && object.bitButton.operation == 3 &&
            !addScanAddress(prepared, object.address, false, 1, &reads, error)) { return false; }
        if (object.hasWordButtonProperties && object.wordButton.operation >= 3 &&
            !addScanAddress(prepared, object.address, true, u32(object.wordButton.transferWordCount()), &reads, error)) //20261109-6 add
        {
            return false; // Word arithmetic reads the current value; keypad and constant operations only write.
        }
        if (object.bitButton.indicator)
        {
            if (object.hasLampProperties && object.lamp.monitorMode == 2) //20261109-8 add: DWORD lamps read two contiguous words.
            {
                if (!addScanAddress(prepared, object.lamp.multiMonitorAddress, true,
                                    u32(object.lamp.transferWordCount()), &reads, error)) { return false; }
            }
            else
            {
                word = !object.bitButton.dualMonitor ||
                       (!object.bitButton.monitorSame && object.bitButton.monitorType == 1);
                monitor = object.bitButton.dualMonitor && object.bitButton.monitorSame ? object.address :
                          (word ? object.bitButton.monitorWordAddress : object.bitButton.monitorBitAddress);
                if (!addScanAddress(prepared, monitor, word, 1, &reads, error)) { return false; }
            }
        }
        if (!addConditionReads(prepared, object.bitButton.touch, &reads, error) ||
            !addConditionReads(prepared, object.bitButton.visibility, &reads, error)) { return false; }
        // Notifications and set/reset/jog operation addresses are writes, not scan subscriptions.
    }
    if (!encodeScanTable(reads, &result, error)) { return false; }
    *this = prepared; *scanTable = result;
    if (error != nullptr) { error->clear(); }
    return true;
}
