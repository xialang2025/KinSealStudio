#include "device_transfer_plan.h"

using u32 = unsigned int;

/******************************************************************************
 * Function Name   : hasStaticMacroPayloads
 * Description     : Recognize the twelve-file layout that includes macro images without control segments.
 * Input Parameters: payloads
 * Return Value    : True for the static macro layout before structural validation.
 * Date            : 2026.11.23
 ******************************************************************************/
bool hasStaticMacroPayloads(const QVector<QByteArray> &payloads) //20261123-22 add
{
    return payloads.size() == 12 && payloads.at(0).size() == 4998 &&
        payloads.at(1).size() == 72 && payloads.at(1).at(2) == 0 &&
        (payloads.at(0).at(0x528) != 0 || payloads.at(0).at(0x82c) != 0);
}

/******************************************************************************
 * Function Name   : hasOfflineDisplayPayloads
 * Description     : Detect disk-only display layouts before constructing any USB commands.
 * Input Parameters: generated or reloaded package segments
 * Return Value    : True for the message-table layout or a control table containing a QR display.
 * Date            : 2026.11.28
 ******************************************************************************/
bool hasOfflineDisplayPayloads(const QVector<QByteArray> &payloads) //20261128-27 add
{
    if (payloads.size() == 13) { return true; }
    if (payloads.size() != 12 && payloads.size() != 14) { return false; }
    if (hasStaticMacroPayloads(payloads)) { return false; }
    const QByteArray &records = payloads.at(10);
    for (qsizetype pos = 0; pos < records.size(); pos += 640) {
        if (u8(records.at(pos)) == 35 || u8(records.at(pos)) == 52) { return true; }
    }
    return false;
}

/******************************************************************************
 * Function Name   : buildDeviceTransferPlan
 * Description     : Frame the source snapshot and insert only observed control segments.
 * Input Parameters: payloads, steps, error
 * Return Value    : True for ten static blocks or twelve dynamic blocks.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildDeviceTransferPlan(const QVector<QByteArray> &payloads,
                             QVector<HmiDownloadStep> *steps,
                             QString *error) //20261109-0 add
{
    struct StepSpec
    {
        u8 command;
        int payloadIndex;
        const char *name;
    };
    static const StepSpec specs[] = {
        {0x0a, 0, "SysPara.dat"}, {0x0b, 10, "ControlPara.dat"},
        {0x0c, 1, "SheetPara.dat"}, {0x36, 12, "MarcoVar.dat"},
        {0x1d, 13, "MarcoRun.dat"}, {0x0e, 2, "AlarmPara.dat"},
        {0x16, -1, "0x16"}, {0x15, 3, "Collect.dat"},
        {0x19, -1, "0x19"}, {0x12, 4, "AddrPara.dat"},
        {0x14, 5, "PicLenPara.dat"}, {0x0d, 6, "Curve.dat"},
        {0x1e, 7, "upLoad.knr"}, {0x10, 11, "ControlPixel.dat"},
        {0x0f, 8, "SheetPixel.dat"}, {0x11, 9, "Modbus_RTU_com1.bin"}
    };
    QVector<HmiDownloadStep> prepared;
    HmiDownloadStep step;
    u32 length;
    int index;
    int byteIndex;

    if (hasOfflineDisplayPayloads(payloads)) { //20261128-27 add
        if (error) { *error = QStringLiteral("该显示控件包已支持离线编译，直连下载仍待现场验证。"); }
        return false;
    }
    if (steps == nullptr || (payloads.size() != 10 && payloads.size() != 12 && payloads.size() != 14))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("下载数据段数量不完整，未生成传输计划。");
        }
        return false;
    }
    if (hasStaticMacroPayloads(payloads)) //20261123-22 add: Never interpret macro files as unverified control transfer segments.
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("静态调度宏包已编译，可离线导出。该段组合的直连下载尚未验证。");
        }
        return false;
    }
    //20261109-0 add: Reject a missing segment before opening a device or sending erase commands.
    for (index = 0; index < payloads.size(); ++index)
    {
        if (payloads.at(index).isEmpty() || payloads.at(index).size() > 0x7fffffff)
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("下载数据段为空或超出传输长度范围。");
            }
            return false;
        }
    }
    for (index = 0; index < int(sizeof(specs) / sizeof(specs[0])); ++index)
    {
        if (specs[index].payloadIndex >= payloads.size())
        {
            continue;
        }
        step = HmiDownloadStep();
        step.command = specs[index].command;
        step.name = QString::fromLatin1(specs[index].name);
        if (specs[index].payloadIndex >= 0)
        {
            step.bytes = payloads.at(specs[index].payloadIndex);
        }
        if (step.command == 0x1e)
        {
            length = u32(step.bytes.size());
            step.bytes.prepend(QByteArray(4, char(0)));
            for (byteIndex = 0; byteIndex < 4; ++byteIndex)
            {
                step.bytes[byteIndex] = char((length >> ((3 - byteIndex) * 8)) & 0xff);
            }
        }
        if (step.command == 0x11)
        {
            length = u32(step.bytes.size());
            for (byteIndex = 0; byteIndex < 4; ++byteIndex)
            {
                step.options.append(char((length >> (byteIndex * 8)) & 0xff));
            }
        }
        prepared.append(step);
    }
    *steps = prepared;
    return true;
}
