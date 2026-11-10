#include "device_transfer_plan.h"

using u32 = unsigned int;

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

    if (steps == nullptr || (payloads.size() != 10 && payloads.size() != 12 && payloads.size() != 14))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("下载数据段数量不完整，未生成传输计划。");
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
