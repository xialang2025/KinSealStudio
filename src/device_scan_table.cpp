#include "device_scan_table.h"

#include <QtEndian>
#include <limits>

namespace
{
using u64 = unsigned long long;

//20261109-0 add: Common.dll 6414796c and 64149e70 use this fixed ALL_ADDR_REG order.
const u8 nativeTypes[12] = {0x4d, 0x58, 0x48, 0x4b, 0x56, 0x4c,
                           0x52, 0x44, 0x74, 0x63, 0x65, 0x66};

/******************************************************************************
 * Function Name   : scanError
 * Description     : Report a rejected subscription without changing the caller's compiled table.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.09
 ******************************************************************************/
bool scanError(QString *error, const QString &message) //20261109-0 add
{
    if (error != nullptr)
    {
        *error = message;
    }
    return false;
}

/******************************************************************************
 * Function Name   : familyIndex
 * Description     : Resolve a verified native Modbus family byte to the original scan order.
 * Input Parameters: type
 * Return Value    : Family index, or minus one for an unsupported family.
 * Date            : 2026.11.09
 ******************************************************************************/
int familyIndex(u8 type) //20261109-0 add
{
    int index;

    for (index = 0; index < 12; ++index)
    {
        if (nativeTypes[index] == type)
        {
            return index;
        }
    }
    return -1;
}

/******************************************************************************
 * Function Name   : appendAlignedRead
 * Description     : Align packed bit reads to whole words and split them at the native read limit.
 * Input Parameters: read, unit, maximum, segments, baseline, error
 * Return Value    : True after every resulting segment is bounded and appended.
 * Date            : 2026.11.09
 ******************************************************************************/
bool appendAlignedRead(const DeviceScanRead &read, u32 unit, u32 maximum,
                       QVector<DeviceScanRead> *segments, u32 *baseline,
                       QString *error) //20261109-0 add
{
    DeviceScanRead segment;
    u64 remaining;
    u64 start;
    u64 size;

    start = u64(read.start / unit) * unit;
    remaining = u64(read.count) + read.start % unit;
    while (remaining != 0)
    {
        if (segments->size() >= 255)
        {
            return scanError(error, QStringLiteral("单页PLC扫描段数超过8位页计数范围。"));
        }
        size = qMin(remaining, u64(maximum));
        segment = read;
        segment.start = u32(start);
        segment.count = u32((size + unit - 1) / unit);
        segments->append(segment);
        *baseline = segment.start;
        start += size;
        remaining -= size;
    }
    return true;
}

/******************************************************************************
 * Function Name   : mergeFamily
 * Description     : Reproduce Common.dll scan-only sorting, permitted gaps and family alignment.
 * Input Parameters: family, index, maximumReadWords, segments, error
 * Return Value    : True after the family's native segments have been appended.
 * Date            : 2026.11.09
 ******************************************************************************/
bool mergeFamily(QVector<DeviceScanRead> family, int index, u32 maximumReadWords,
                  QVector<DeviceScanRead> *segments, QString *error) //20261109-0 add
{
    DeviceScanRead read;
    DeviceScanRead *last;
    u32 unit;
    u32 maximum;
    u32 baseline;
    long long span;
    u64 gap;
    int first;
    int position;
    int candidate;

    if (family.isEmpty())
    {
        return true;
    }
    unit = index >= 2 && index <= 5 ? 16U : 1U;
    maximum = index < 2 ? 64U : qMin(maximumReadWords, 256U) * unit;
    first = segments->size();
    baseline = 0;
    // Common.dll 64135ac0 swaps each smaller later start into the current position.
    for (position = 0; position + 1 < family.size(); ++position)
    {
        for (candidate = position + 1; candidate < family.size(); ++candidate)
        {
            if (family[candidate].start < family[position].start)
            {
                qSwap(family[candidate], family[position]);
            }
        }
    }
    if (!appendAlignedRead(family.first(), unit, maximum, segments, &baseline, error))
    {
        return false;
    }
    for (position = 1; position < family.size(); ++position)
    {
        read = family[position];
        span = static_cast<long long>(read.start) + read.count - baseline;
        if (read.start < baseline)
        {
            read.start = baseline;
            read.count = u32(span);
        }
        last = &segments->last();
        gap = read.start > u64(last->start) + last->count ?
              read.start - u64(last->start) - last->count : 0;
        // All subscriptions have native call type 8, so the original merge gate (> 6) is open.
        if (span <= maximum && gap <= 32U * unit)
        {
            last->count = u32((qMax(static_cast<long long>(last->count), span) + unit - 1) / unit);
        }
        else if (!appendAlignedRead(read, unit, maximum, segments, &baseline, error))
        {
            return false;
        }
    }
    if (index == 8 || index == 9)
    {
        // The native _dw post-pass rounds starts down and odd transfer counts up to an even pair.
        for (position = first; position < segments->size(); ++position)
        {
            (*segments)[position].start &= ~1U;
            (*segments)[position].count += (*segments)[position].count & 1U;
        }
    }
    return true;
}
}

/******************************************************************************
 * Function Name   : buildDeviceScanTable
 * Description     : Encode clean Modbus RTU scan records after validating every input transactionally.
 * Input Parameters: reads, scanTable, error, maximumReadWords
 * Return Value    : True when all reads fit a single native page's scan count.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildDeviceScanTable(const QVector<DeviceScanRead> &reads, QByteArray *scanTable,
                          QString *error, u32 maximumReadWords) //20261109-0 add
{
    QVector<DeviceScanRead> groups;
    QVector<DeviceScanRead> family;
    QVector<DeviceScanRead> segments;
    DeviceScanRead read;
    DeviceScanRead key;
    QByteArray result;
    QByteArray record;
    bool known;
    int index;
    int groupIndex;

    if (scanTable == nullptr || maximumReadWords == 0)
    {
        return scanError(error, QStringLiteral("PLC扫描表输出为空或读取上限无效。"));
    }
    Q_FOREACH (read, reads)
    {
        if (familyIndex(read.type) < 0 || read.route == 0 || read.count == 0 ||
            u64(read.start) + read.count > u64(std::numeric_limits<u32>::max()) + 1)
        {
            return scanError(error, QStringLiteral("PLC扫描地址族、路由、长度或地址范围无效。"));
        }
        known = false;
        Q_FOREACH (key, groups)
        {
            if (key.route == read.route && key.node == read.node)
            {
                known = true;
                break;
            }
        }
        if (!known)
        {
            groups.append(read);
        }
    }
    for (groupIndex = 0; groupIndex < groups.size(); ++groupIndex)
    {
        key = groups[groupIndex];
        for (index = 0; index < 12; ++index)
        {
            family.clear();
            Q_FOREACH (read, reads)
            {
                if (read.route == key.route && read.node == key.node && read.type == nativeTypes[index])
                {
                    family.append(read);
                }
            }
            if (!mergeFamily(family, index, maximumReadWords, &segments, error))
            {
                return false;
            }
        }
    }
    result = QByteArray(200, char(0));
    result[0] = char(segments.size());
    Q_FOREACH (read, segments)
    {
        record = QByteArray(12, char(0));
        record[0] = char(read.type);
        record[1] = char(read.route);
        record[3] = char(read.node);
        qToLittleEndian<u16>(u16(read.count), record.data() + 4);
        qToLittleEndian<u32>(read.start, record.data() + 8);
        result.append(record);
    }
    *scanTable = result;
    if (error != nullptr)
    {
        error->clear();
    }
    return true;
}
