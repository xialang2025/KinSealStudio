// The V122 writer fixes the LZO match finder as well as the firmware token format.
#include <QtEndian>
#include <array>

namespace {
using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

/******************************************************************************
 * Function Name   : appendLzoLength
 * Description     : Emit a nonzero LZO continuation length in 255-byte groups.
 * Input Parameters: output, length
 * Return Value    : None
 * Date            : 2026.11.27
 ******************************************************************************/
void appendLzoLength(QByteArray &output, int length) //20261127-12 add
{
    while (length > 255) {
        output.append(char(0));
        length -= 255;
    }
    output.append(char(length));
}

/******************************************************************************
 * Function Name   : appendLzoLiterals
 * Description     : Emit literal bytes, using the preceding distance token for a short run.
 * Input Parameters: output, source, length, finalRun
 * Return Value    : None
 * Date            : 2026.11.27
 ******************************************************************************/
void appendLzoLiterals(QByteArray &output, const char *source, int length, bool finalRun) //20261127-12 add
{
    if (length == 0) {
        return;
    }
    if (finalRun && output.isEmpty() && length <= 238) {
        output.append(char(17 + length));
    } else if (length <= 3) {
        output[output.size() - 2] = char(u8(output.at(output.size() - 2)) | length);
    } else if (length <= 18) {
        output.append(char(length - 3));
    } else {
        output.append(char(0));
        appendLzoLength(output, length - 18);
    }
    output.append(source, length);
}

/******************************************************************************
 * Function Name   : appendLzoMatch
 * Description     : Encode a backreference with the V122 short, medium or long distance form.
 * Input Parameters: output, distance, length
 * Return Value    : None
 * Date            : 2026.11.27
 ******************************************************************************/
void appendLzoMatch(QByteArray &output, int distance, int length) //20261127-12 add
{
    int token;
    int maximum;

    if (distance <= 2048 && length <= 8) {
        --distance;
        output.append(char(((length - 1) << 5) | ((distance & 7) << 2)));
        output.append(char(distance >> 3));
        return;
    }
    if (distance <= 16384) {
        --distance;
        token = 32;
        maximum = 33;
    } else {
        distance -= 16384;
        token = 16 | ((distance >> 11) & 8);
        maximum = 9;
    }
    if (length <= maximum) {
        output.append(char(token | (length - 2)));
    } else {
        output.append(char(token));
        appendLzoLength(output, length - maximum);
    }
    output.append(char(distance << 2));
    output.append(char(distance >> 6));
}

/******************************************************************************
 * Function Name   : nativeLzoMatchLength
 * Description     : Extend the initial four-byte match with the original writer's eight-byte probe cadence.
 * Input Parameters: source, position, previous, limit (20-byte lookahead boundary)
 * Return Value    : Matched byte count
 * Date            : 2026.11.27
 ******************************************************************************/
int nativeLzoMatchLength(const char *source, int position, int previous, int limit) //20261127-12 add
{
    int length = 4;
    u32 difference;

    for (;;) {
        // After the initial two words, V122 checks the lookahead on the first word of each pair.
        if (length >= 12 && (length & 7) == 4 && position + length >= limit) {
            break;
        }
        difference = qFromLittleEndian<u32>(source + position + length) ^
                     qFromLittleEndian<u32>(source + previous + length);
        if (difference != 0) {
            while ((difference & 255U) == 0) {
                ++length;
                difference >>= 8;
            }
            break;
        }
        length += 4;
    }
    return length;
}

/******************************************************************************
 * Function Name   : nativeLzoPixels
 * Description     : Reproduce the V122 LZO1X record bytes for the given packed pixel data.
 * Input Parameters: frame (bounded by the public pixel compiler)
 * Return Value    : Complete compressed stream with its end marker
 * Date            : 2026.11.27
 ******************************************************************************/
QByteArray nativeLzoPixels(const QByteArray &frame) //20261127-12 add
{
    constexpr int blockSize = 49152;
    std::array<u16, 16384> dictionary;
    const char *source = frame.constData();
    const int size = int(frame.size());
    QByteArray output;
    int block = 0;
    int literalStart = 0;
    int blockEnd, limit, anchor, position, previous, length;
    u32 word, slot;

    output.reserve(size + size / 16 + 67);
    while (size - block > 20) {
        blockEnd = block + qMin(blockSize, size - block);
        if (blockEnd - literalStart < 32) {
            break;
        }
        dictionary.fill(0);
        limit = blockEnd - 20;
        anchor = block;
        position = qMax(block, literalStart + 4) + 1;
        while (position < limit) {
            word = qFromLittleEndian<u32>(source + position);
            slot = (word * 0x1824429dU) >> 18;
            previous = block + dictionary[slot];
            dictionary[slot] = u16(position - block);
            if (qFromLittleEndian<u32>(source + previous) == word) {
                appendLzoLiterals(output, source + literalStart, position - literalStart, false);
                length = nativeLzoMatchLength(source, position, previous, limit);
                appendLzoMatch(output, position - previous, length);
                position += length;
                literalStart = position;
                anchor = position;
            } else {
                position += 1 + ((position - anchor) >> 5);
            }
        }
        block = blockEnd;
    }
    appendLzoLiterals(output, source + literalStart, size - literalStart, true);
    output.append("\x11\0\0", 3);
    return output;
}
} // namespace
