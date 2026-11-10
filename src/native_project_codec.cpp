#include "native_project_codec.h"
#include "project_model.h"

#include <QCryptographicHash>
#include <QDataStream>
#include <QFileInfo>
#include <QFile>
#include <QFont>
#include <QMap>
#include <QRegularExpression>
#include <QSet>
#include <QVariant>
#include <cmath>
#include <memory>
#include <algorithm> //20261110-5 add: Present source field diagnostics in stream order.

namespace {

//20261109-0 add: Resource limits bound inspection before any QString allocation.
constexpr qsizetype MaximumSourceBytes = 64 * 1024 * 1024;
constexpr u32 MaximumStringBytes = 1024 * 1024;
constexpr u32 NullQString = 0xffffffffU;
constexpr qsizetype MaximumSourceFields = 262144; //20261110-5 add: Bound metadata allocation independently of per-list lengths.

/******************************************************************************
 * Function Name   : fail
 * Description     : Report a codec failure without requiring a diagnostic destination.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.09
 ******************************************************************************/
bool fail(QString *error, const QString &message) //20261109-0 add
{
    if (error != nullptr) {
        *error = message;
    }
    return false;
}

/******************************************************************************
 * Function Name   : clearError
 * Description     : Remove an obsolete diagnostic after a successful transaction.
 * Input Parameters: error
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void clearError(QString *error) //20261109-0 add
{
    if (error != nullptr) {
        error->clear();
    }
}

//20261109-0 add: Native Qt4 QStrings use a big-endian byte length and UTF-16 code units.
class SourceReader
{
public:
    /******************************************************************************
     * Function Name   : SourceReader
     * Description     : Start a bounded source record reader at a caller-supplied offset.
     * Input Parameters: bytes, offset
     * Return Value    : None.
     * Date            : 2026.11.09
     ******************************************************************************/
    SourceReader(const QByteArray &bytes, qsizetype offset) //20261109-0 add
        : m_bytes(bytes), m_offset(offset)
    {
    }

    /******************************************************************************
     * Function Name   : byte
     * Description     : Read one opaque native byte without sign conversion.
     * Input Parameters: value
     * Return Value    : True when one byte was available.
     * Date            : 2026.11.09
     ******************************************************************************/
    bool byte(u8 *value) //20261109-0 add
    {
        if (m_offset < 0 || m_offset >= m_bytes.size()) {
            return false;
        }
        *value = static_cast<u8>(m_bytes.at(m_offset));
        ++m_offset;
        return true;
    }

    /******************************************************************************
     * Function Name   : word
     * Description     : Read one native big-endian 32-bit stream value.
     * Input Parameters: value
     * Return Value    : True when all four bytes were available.
     * Date            : 2026.11.09
     ******************************************************************************/
    bool word(u32 *value) //20261109-0 add
    {
        u32 result;
        int i;

        if (m_offset < 0 || m_offset > m_bytes.size() || m_bytes.size() - m_offset < 4) {
            return false;
        }
        result = 0;
        for (i = 0; i < 4; ++i) {
            result = (result << 8) | static_cast<u8>(m_bytes.at(m_offset + i));
        }
        m_offset += 4;
        *value = result;
        return true;
    }

    /******************************************************************************
     * Function Name   : string
     * Description     : Decode a bounded Qt QString while preserving null and UTF-16 units.
     * Input Parameters: value
     * Return Value    : True for a complete native QString representation.
     * Date            : 2026.11.09
     ******************************************************************************/
    bool string(QString *value) //20261109-0 add
    {
        u32 byteCount;
        u16 codeUnit;
        qsizetype i;
        QString result;

        if (!word(&byteCount)) {
            return false;
        }
        if (byteCount == NullQString) {
            *value = QString();
            return true;
        }
        if ((byteCount & 1U) != 0 || byteCount > MaximumStringBytes ||
            static_cast<qsizetype>(byteCount) > m_bytes.size() - m_offset) {
            return false;
        }
        result = QStringLiteral("");
        result.resize(static_cast<qsizetype>(byteCount / 2));
        for (i = 0; i < result.size(); ++i) {
            codeUnit = static_cast<u16>((static_cast<u8>(m_bytes.at(m_offset + 2 * i)) << 8) |
                                        static_cast<u8>(m_bytes.at(m_offset + 2 * i + 1)));
            result[i] = QChar(codeUnit);
        }
        m_offset += byteCount;
        *value = result;
        return true;
    }

    /******************************************************************************
     * Function Name   : offset
     * Description     : Return the byte boundary reached by successful typed reads.
     * Input Parameters: begin, end
     * Return Value    : Current byte offset.
     * Date            : 2026.11.09
     ******************************************************************************/
    qsizetype offset() const //20261109-0 add
    {
        return m_offset;
    }

private:
    const QByteArray &m_bytes;
    qsizetype m_offset;
};

/******************************************************************************
 * Function Name   : appendWord
 * Description     : Append a 32-bit integer using the native big-endian stream order.
 * Input Parameters: data, value
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void appendWord(QByteArray *data, u32 value) //20261109-0 add
{
    int shift;

    for (shift = 24; shift >= 0; shift -= 8) {
        data->append(static_cast<char>((value >> shift) & 0xffU));
    }
}

/******************************************************************************
 * Function Name   : appendString
 * Description     : Append the exact Qt4 QString representation without normalizing its content.
 * Input Parameters: data, value
 * Return Value    : True for a string within the codec resource limit.
 * Date            : 2026.11.09
 ******************************************************************************/
bool appendString(QByteArray *data, const QString &value) //20261109-0 add
{
    qsizetype i;
    u16 codeUnit;

    if (value.isNull()) {
        appendWord(data, NullQString);
        return true;
    }
    if (value.size() > static_cast<qsizetype>(MaximumStringBytes / 2)) {
        return false;
    }
    appendWord(data, static_cast<u32>(value.size() * 2));
    for (i = 0; i < value.size(); ++i) {
        codeUnit = value.at(i).unicode();
        data->append(static_cast<char>(codeUnit >> 8));
        data->append(static_cast<char>(codeUnit & 0xffU));
    }
    return true;
}

//20261109-0 add: Every editable span is obtained by typed traversal, never by a fixed file offset.
struct SourceField
{
    qsizetype start = 0;
    qsizetype end = 0;
    char kind = 0;
    QVariant value;
};

class SourceDocument
{
public:
    QMap<QString, SourceField> values;
    QString error;
    QStringList objectPrefixes;
    QByteArray bytes;

    /******************************************************************************
     * Function Name   : SourceDocument
     * Description     : Retain an immutable native stream for typed boundary discovery.
     * Input Parameters: data
     * Return Value    : None.
     * Date            : 2026.11.09
     ******************************************************************************/
    explicit SourceDocument(const QByteArray &data) : bytes(data) //20261109-0 add
    {
    }

    /******************************************************************************
     * Function Name   : value
     * Description     : Read an already verified field by its structural path.
     * Input Parameters: name
     * Return Value    : Decoded primitive value.
     * Date            : 2026.11.09
     ******************************************************************************/
    QVariant value(const QString &name) const //20261109-0 add
    {
        return values.value(name).value;
    }

    /******************************************************************************
     * Function Name   : read
     * Description     : Decode one bounded Qt primitive and retain its precise source span.
     * Input Parameters: kind, name
     * Return Value    : Decoded value; errors latch and stop subsequent reads.
     * Date            : 2026.11.09
     ******************************************************************************/
    QVariant read(char kind, const QString &name) //20261109-0 add
    {
        SourceField field;
        SourceReader reader(bytes, m_position);
        u32 word;
        u8 byte;
        u8 low;
        QString string;
        double number;
        QColor color;
        qsizetype size;
        std::unique_ptr<QDataStream> graphicStream;

        if (!error.isEmpty()) {
            return QVariant();
        }
        if (values.size() >= MaximumSourceFields || values.contains(name)) { //20261110-5 add
            reject(name, QStringLiteral("typed 字段总数超过 Lite 读取保护或结构路径重复"));
            return QVariant();
        }
        field.start = m_position;
        field.kind = kind;
        size = kind == 'd' ? 8 : kind == 'c' ? 11 : 0;
        if (size != 0 && (m_position > bytes.size() || bytes.size() - m_position < size)) {
            reject(name, QStringLiteral("记录被截断"));
            return QVariant();
        }
        if (kind == 'q') {
            if (!reader.string(&string)) {
                reject(name, QStringLiteral("QString 长度无效或被截断"));
            }
            field.value = string;
        } else if (kind == 'i') {
            if (!reader.word(&word)) {
                reject(name, QStringLiteral("整数被截断"));
            } else {
                field.value = static_cast<int>(word);
            }
        } else if (kind == 'a' || kind == 'b' || kind == 's') {
            if (!reader.byte(&byte) || (kind == 's' && !reader.byte(&low))) {
                reject(name, QStringLiteral("字节被截断"));
            } else {
                field.value = kind == 's' ? static_cast<int>(static_cast<short>((byte << 8) | low)) : byte;
                // Native stale bool bytes are preserved; an edit writes a canonical 0 or 1.
            }
        } else if (kind == 'd' || kind == 'c') {
            graphicStream = std::make_unique<QDataStream>(bytes.mid(m_position, size));
            graphicStream->setVersion(QDataStream::Qt_4_8);
            if (kind == 'd') {
                *graphicStream >> number;
                if (!std::isfinite(number)) {
                    reject(name, QStringLiteral("非有限图形坐标"));
                }
                field.value = number;
            } else {
                *graphicStream >> color;
                field.value = color;
            }
            if (graphicStream->status() != QDataStream::Ok) {
                reject(name, QStringLiteral("Qt 图形数据无效"));
            }
        } else if (kind == 'x') {
            if (!reader.word(&word) || (word != NullQString &&
                (word > static_cast<u32>(MaximumSourceBytes) ||
                 static_cast<qsizetype>(word) > bytes.size() - reader.offset()))) {
                reject(name, QStringLiteral("QByteArray 长度无效"));
            } else if (word != NullQString) {
                field.value = bytes.mid(reader.offset(), word);
                size = 4 + word;
            } else {
                field.value = QByteArray();
                size = 4;
            }
        } else {
            reject(name, QStringLiteral("未知 Qt primitive"));
        }
        if (!error.isEmpty()) {
            return QVariant();
        }
        m_position = size == 0 ? reader.offset() : m_position + size;
        field.end = m_position;
        values.insert(name, field);
        return field.value;
    }

    /******************************************************************************
     * Function Name   : fields
     * Description     : Traverse a recovered fixed primitive sequence under one structural path.
     * Input Parameters: kinds, name
     * Return Value    : None.
     * Date            : 2026.11.09
     ******************************************************************************/
    void fields(const QByteArray &kinds, const QString &name) //20261109-0 add
    {
        qsizetype i;

        for (i = 0; i < kinds.size() && error.isEmpty(); ++i) {
            read(kinds.at(i), name + QStringLiteral(".") + QString::number(i));
        }
    }

    /******************************************************************************
     * Function Name   : count
     * Description     : Bound all native collection lengths before traversing their elements.
     * Input Parameters: name, maximum
     * Return Value    : Validated element count or zero after a latched failure.
     * Date            : 2026.11.09
     ******************************************************************************/
    int count(const QString &name, int maximum = 4096) //20261109-0 add
    {
        int result;

        result = read('i', name + QStringLiteral(".count")).toInt();
        if (result < 0 || result > maximum || result > bytes.size() - m_position) { //20261110-5 change: Every observed list element consumes at least one source byte.
            reject(name, QStringLiteral("集合长度超出 Lite 读取保护或剩余记录范围"));
            return 0;
        }
        return result;
    }

    /******************************************************************************
     * Function Name   : indexed
     * Description     : Form a unique structural path for an indexed native record.
     * Input Parameters: name, index
     * Return Value    : Structural field prefix.
     * Date            : 2026.11.09
     ******************************************************************************/
    static QString indexed(const QString &name, int index) //20261109-0 add
    {
        return name + QStringLiteral("[") + QString::number(index) + QStringLiteral("]");
    }

    /******************************************************************************
     * Function Name   : list
     * Description     : Traverse a bounded native QString list.
     * Input Parameters: name
     * Return Value    : None.
     * Date            : 2026.11.09
     ******************************************************************************/
    void list(const QString &name) //20261109-0 add
    {
        int n;
        int i;

        n = count(name);
        for (i = 0; i < n && error.isEmpty(); ++i) {
            read('q', indexed(name, i));
        }
    }

    /******************************************************************************
     * Function Name   : address
     * Description     : Traverse the complete recovered source address record.
     * Input Parameters: name
     * Return Value    : None.
     * Date            : 2026.11.09
     ******************************************************************************/
    void address(const QString &name) //20261109-0 add
    {
        fields(QByteArray("qaaaqqaiq"), name);
    }

    /******************************************************************************
     * Function Name   : macros
     * Description     : Retain the six typed fields of each native configuration macro record.
     * Input Parameters: name
     * Return Value    : None.
     * Date            : 2026.11.09
     ******************************************************************************/
    void macros(const QString &name) //20261109-0 add
    {
        int n;
        int i;

        n = count(name, 100);
        for (i = 0; i < n && error.isEmpty(); ++i) {
            fields(QByteArray("qqqiix"), indexed(name, i));
        }
    }

    /******************************************************************************
     * Function Name   : reject
     * Description     : Keep the first unsupported or malformed structural location.
     * Input Parameters: name, reason
     * Return Value    : None.
     * Date            : 2026.11.09
     ******************************************************************************/
    void reject(const QString &name, const QString &reason) //20261109-0 add
    {
        if (error.isEmpty()) {
            error = QStringLiteral("原生工程 %1（字节 %2）：%3。").arg(name).arg(m_position).arg(reason);
        }
    }

    bool parse(); //20261109-0 add
    void system();
    void configuration();
    void page(const QString &pagePrefix); //20261110-5 change: Distinct read-only pages must not overwrite one another's structural spans.
    void control(const QString &prefix, int kind);
    void history();

private:
    qsizetype m_position = 0;
};

/******************************************************************************
 * Function Name   : SourceDocument::system
 * Description     : Traverse model settings, serial links and nested PLC configuration tables.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void SourceDocument::system() //20261109-0 add
{
    int n;
    int m;
    int i;
    int j;
    QString name;
    QString child;

    fields(QByteArray("aqqiqqqqqqqqqiiss"), QStringLiteral("system.identity"));
    list(QStringLiteral("system.164"));
    fields(QByteArray(17, 'b') + QByteArray(8, 'a') + QByteArray("ii") + QByteArray(5, 'a') +
           QByteArray("i") + QByteArray(4, 'a') + QByteArray(6, 'q'), QStringLiteral("system.flags"));
    list(QStringLiteral("system.58"));
    fields(QByteArray("qqqcc"), QStringLiteral("system.colors"));
    list(QStringLiteral("system.11c"));
    for (i = 0; i < 8; ++i) {
        fields(QByteArray("bqq") + QByteArray(8, 'b'), indexed(QStringLiteral("system.roles"), i));
    }
    n = count(QStringLiteral("system.170"), 5);
    for (i = 0; i < n && error.isEmpty(); ++i) {
        name = indexed(QStringLiteral("system.170"), i);
        fields(QByteArray("aaias") + QByteArray(6, 'a') + QByteArray("saaasss") + QByteArray(7, 'q'), name);
        list(name + QStringLiteral(".8"));
        list(name + QStringLiteral(".c"));
    }
    n = count(QStringLiteral("system.174"), 64);
    for (i = 0; i < n && error.isEmpty(); ++i) {
        fields(QByteArray("qq"), indexed(QStringLiteral("system.174"), i));
    }
    n = count(QStringLiteral("system.178"), 128);
    for (i = 0; i < n && error.isEmpty(); ++i) {
        name = indexed(QStringLiteral("system.178"), i);
        fields(QByteArray("qqibi"), name + QStringLiteral(".head"));
        address(name + QStringLiteral(".address"));
        fields(QByteArray("ii"), name + QStringLiteral(".tail"));
    }
    n = count(QStringLiteral("system.16c"), 5);
    for (i = 0; i < n && error.isEmpty(); ++i) {
        name = indexed(QStringLiteral("system.16c"), i);
        fields(QByteArray("b") + QByteArray(10, 'i') + QByteArray("qqiii"), name + QStringLiteral(".header"));
        m = count(name + QStringLiteral(".50"), 128);
        for (j = 0; j < m && error.isEmpty(); ++j) {
            child = indexed(name + QStringLiteral(".50"), j);
            fields(QByteArray("iiiii"), child + QStringLiteral(".head"));
            list(child + QStringLiteral(".40"));
            fields(QByteArray("qiiq") + QByteArray(5, 'i') + QByteArray("qqqiqibsaa"), child + QStringLiteral(".tail"));
        }
    }
    n = count(QStringLiteral("system.180"));
    for (i = 0; i < n && error.isEmpty(); ++i) {
        name = indexed(QStringLiteral("system.180"), i);
        read('q', name + QStringLiteral(".name"));
        address(name + QStringLiteral(".address"));
        read('i', name + QStringLiteral(".value"));
    }
    fields(QByteArray("aqsi"), QStringLiteral("system.tail"));
    macros(QStringLiteral("system.194"));
}

/******************************************************************************
 * Function Name   : SourceDocument::configuration
 * Description     : Traverse project configuration without interpreting or discarding native metadata.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void SourceDocument::configuration() //20261109-0 add
{
    int n;
    int m;
    int i;
    int j;
    QString name;
    QString child;

    fields(QByteArray("qqqbqiibbbbqqiibbbbqq"), QStringLiteral("config.head"));
    list(QStringLiteral("config.40"));
    fields(QByteArray("qbaaa"), QStringLiteral("config.flags"));
    fields(QByteArray("qqiqq") + QByteArray(11, 'i'), QStringLiteral("config.48"));
    fields(QByteArray("qqiqq") + QByteArray(11, 'i'), QStringLiteral("config.88"));
    n = count(QStringLiteral("config.c8"));
    for (i = 0; i < n && error.isEmpty(); ++i) {
        name = indexed(QStringLiteral("config.c8"), i);
        fields(QByteArray("bbbbqqiii"), name + QStringLiteral(".head"));
        address(name + QStringLiteral(".28"));
        address(name + QStringLiteral(".40"));
        fields(QByteArray("qqq"), name + QStringLiteral(".texts"));
        m = count(name + QStringLiteral(".74"));
        for (j = 0; j < m && error.isEmpty(); ++j) {
            list(indexed(name + QStringLiteral(".74"), j));
        }
        list(name + QStringLiteral(".64"));
        list(name + QStringLiteral(".68"));
    }
    n = count(QStringLiteral("config.cc"));
    for (i = 0; i < n && error.isEmpty(); ++i) {
        name = indexed(QStringLiteral("config.cc"), i);
        fields(QByteArray("bbbbqiiiiii"), name + QStringLiteral(".head"));
        address(name + QStringLiteral(".30"));
        address(name + QStringLiteral(".48"));
        address(name + QStringLiteral(".60"));
        m = count(name + QStringLiteral(".78"));
        for (j = 0; j < m && error.isEmpty(); ++j) {
            fields(QByteArray("iiiibqqq"), indexed(name + QStringLiteral(".78"), j));
        }
    }
    read('i', QStringLiteral("config.d0"));
    address(QStringLiteral("config.d4"));
    fields(QByteArray("ii"), QStringLiteral("config.ec"));
    n = count(QStringLiteral("config.108"));
    for (i = 0; i < n && error.isEmpty(); ++i) {
        fields(QByteArray("bq"), indexed(QStringLiteral("config.108"), i));
    }
    address(QStringLiteral("config.10c"));
    fields(QByteArray("ii"), QStringLiteral("config.124"));
    n = count(QStringLiteral("config.138"));
    for (i = 0; i < n && error.isEmpty(); ++i) {
        for (j = 0; j < 4; ++j) {
            fields(QByteArray("ibqq"), indexed(indexed(QStringLiteral("config.138"), i), j));
        }
    }
    fields(QByteArray("bbb"), QStringLiteral("config.13c"));
    address(QStringLiteral("config.140"));
    address(QStringLiteral("config.158"));
    address(QStringLiteral("config.170"));
    fields(QByteArray("bbbb"), QStringLiteral("config.188"));
    address(QStringLiteral("config.18c"));
    address(QStringLiteral("config.1a4"));
    address(QStringLiteral("config.1bc"));
    address(QStringLiteral("config.1d4"));
    fields(QByteArray("bbb"), QStringLiteral("config.1ec"));
    fields(QByteArray("iiii"), QStringLiteral("config.1f0"));
    address(QStringLiteral("config.200"));
    address(QStringLiteral("config.218"));
    fields(QByteArray("iiii"), QStringLiteral("config.230"));
    address(QStringLiteral("config.240"));
    address(QStringLiteral("config.258"));
    fields(QByteArray("bqb"), QStringLiteral("config.270"));
    n = count(QStringLiteral("config.27c"));
    for (i = 0; i < n && error.isEmpty(); ++i) {
        fields(QByteArray("qqii"), indexed(QStringLiteral("config.27c"), i));
    }
    n = count(QStringLiteral("config.280"));
    for (i = 0; i < n && error.isEmpty(); ++i) {
        name = indexed(QStringLiteral("config.280"), i);
        fields(QByteArray("iiii"), name + QStringLiteral(".head"));
        for (j = 0; j < 3; ++j) {
            address(indexed(name + QStringLiteral(".address"), j));
        }
    }
    fields(QByteArray("ii"), QStringLiteral("config.288"));
    list(QStringLiteral("config.298"));
    fields(QByteArray("ibiiii"), QStringLiteral("config.290"));
    list(QStringLiteral("config.104"));
    list(QStringLiteral("config.100"));
    fields(QByteArray("bb"), QStringLiteral("config.134"));
    n = count(QStringLiteral("config.cc-again"));
    for (i = 0; i < n && error.isEmpty(); ++i) {
        read('b', indexed(QStringLiteral("config.cc-again"), i));
    }
    read('i', QStringLiteral("config.29c"));
    macros(QStringLiteral("config.284"));
}

/******************************************************************************
 * Function Name   : SourceDocument::control
 * Description     : Traverse verified graphical children, common properties and the typed control tail.
 * Input Parameters: prefix, kind
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void SourceDocument::control(const QString &prefix, int kind) //20261109-0 add
{
    int type;
    int n;
    int m;
    int i;
    int j;
    QString name;

    if (count(prefix + QStringLiteral("children"), 3) != (kind == 10 ? 3 : 2)) {
        reject(prefix, QStringLiteral("控件子图形数量与已核实类型不符"));
        return;
    }
    type = read('i', prefix + QStringLiteral("rectangle.type")).toInt();
    if (type != 0x1000f) {
        reject(prefix, QStringLiteral("未知位按钮矩形子类型"));
        return;
    }
    fields(QByteArray("ddddcicciddibqii"), prefix + QStringLiteral("rectangle"));
    list(prefix + QStringLiteral("rectangle.labels"));
    read('i', prefix + QStringLiteral("rectangle.1c"));
    if (kind == 10) {
        type = read('i', prefix + QStringLiteral("pixmap.type")).toInt();
        if (type == 13) {
            fields(QByteArray("qdddddddbi"), prefix + QStringLiteral("pixmap"));
            list(prefix + QStringLiteral("pixmap.labels"));
            read('i', prefix + QStringLiteral("pixmap.1c"));
        } else if (type == 0x1000f) { //20261109-8 add: The original no-image option replaces the cached pixmap child with a filled rectangle.
            fields(QByteArray("ddddcicciddibqii"), prefix + QStringLiteral("picture-rectangle"));
            list(prefix + QStringLiteral("picture-rectangle.labels"));
            read('i', prefix + QStringLiteral("picture-rectangle.1c"));
        } else {
            reject(prefix, QStringLiteral("未知位按钮图片子类型"));
            return;
        }
    }
    type = read('i', prefix + QStringLiteral("text.type")).toInt();
    if (type != 0x10016) {
        reject(prefix, QStringLiteral("未知位按钮文字子类型"));
        return;
    }
    fields(QByteArray("ddddq"), prefix + QStringLiteral("text.head"));
    read('q', prefix + QStringLiteral("font.family"));
    fields(QByteArray("di") + QByteArray(5, 'a') + QByteArray("saii"), prefix + QStringLiteral("font.tail"));
    fields(QByteArray("ccciibciqiddbi"), prefix + QStringLiteral("text.tail"));
    list(prefix + QStringLiteral("text.labels"));
    fields(QByteArray("bd"), prefix + QStringLiteral("text.end"));
    type = read('i', prefix + QStringLiteral("group.metadata-type")).toInt();
    read('q', prefix + QStringLiteral("group.metadata-name"));
    if (type == 100 || type == 101) {
        fields(QByteArray("iiii"), prefix + QStringLiteral("group.bounds"));
    }
    read('i', prefix + QStringLiteral("group.1c"));
    fields(QByteArray("bi"), prefix + QStringLiteral("common.touch"));
    address(prefix + QStringLiteral("common.40"));
    fields(QByteArray("iibi"), prefix + QStringLiteral("common.58"));
    address(prefix + QStringLiteral("common.e4"));
    fields(QByteArray("bi"), prefix + QStringLiteral("common.visibility"));
    address(prefix + QStringLiteral("common.90"));
    fields(QByteArray("iii"), prefix + QStringLiteral("common.a8"));
    address(prefix + QStringLiteral("common.60"));
    fields(QByteArray("aqqi"), prefix + QStringLiteral("common.78"));
    address(prefix + QStringLiteral("common.b0"));
    fields(QByteArray("aqqii"), prefix + QStringLiteral("common.c8"));
    address(prefix + QStringLiteral("common.fc"));
    fields(QByteArray("qbi"), prefix + QStringLiteral("common.118"));
    if (kind == 28) {
        fields(QByteArray("bbssiqssqqsqqsbbbissssssi"), prefix + QStringLiteral("static.head"));
        n = count(prefix + QStringLiteral("static.194"), 32);
        for (i = 0; i < n && error.isEmpty(); ++i) {
            fields(QByteArray("bbiqiibqibqqqqiii"), indexed(prefix + QStringLiteral("static.194"), i));
        }
        fields(QByteArray("ii"), prefix + QStringLiteral("static.tail"));
        return;
    }
    type = read('i', prefix + QStringLiteral("switch.mode")).toInt();
    if (type != 0 && type != 1 && type != 3 && type != 4) { //20261110-0 change: Function buttons contain their own address-free system-operation union.
        reject(prefix, QStringLiteral("未知控件子类型；仅支持位按钮 subtype 0、字按钮 subtype 1、功能按钮 subtype 3 和指示灯 subtype 4"));
        return;
    }
    if (type == 0) {
        address(prefix + QStringLiteral("switch.12c"));
        fields(QByteArray("ibbi"), prefix + QStringLiteral("switch.144"));
    } else if (type == 1) { //20261109-8 change: Subtype four goes directly to macro and monitor fields.
        address(prefix + QStringLiteral("word.150"));
        fields(QByteArray("iidbddb"), prefix + QStringLiteral("word.16c"));
        if (value(prefix + QStringLiteral("word.16c.6")).toInt() != 0) {
            address(prefix + QStringLiteral("word.198"));
        }
        fields(QByteArray("ii"), prefix + QStringLiteral("word.18c"));
    } else if (type == 3) { //20261110-0 add: The original writer stores function/type, an opaque QString and two bytes, then an i32 key and QString text.
        fields(QByteArray("iiqbbiq"), prefix + QStringLiteral("function.1c8"));
    }
    fields(QByteArray("bbqqbq"), prefix + QStringLiteral("switch.22c"));
    address(prefix + QStringLiteral("switch.254"));
    fields(QByteArray("bibiii"), prefix + QStringLiteral("switch.248"));
    n = count(prefix + QStringLiteral("switch.278"));
    for (i = 0; i < n && error.isEmpty(); ++i) {
        read('d', indexed(prefix + QStringLiteral("switch.278"), i));
    }
    fields(QByteArray("iii"), prefix + QStringLiteral("switch.274"));
    n = count(prefix + QStringLiteral("switch.290"), type == 4 ? 65535 : 2); //20261109-8 change: This is an allocation guard, not a claimed native UI maximum.
    for (i = 0; i < n && error.isEmpty(); ++i) {
        name = indexed(prefix + QStringLiteral("switch.290"), i);
        m = count(name, 32);
        for (j = 0; j < m && error.isEmpty(); ++j) {
            fields(QByteArray("cqiiq"), indexed(name, j));
        }
    }
    n = count(prefix + QStringLiteral("switch.294"), type == 4 ? 65535 : 2); //20261109-8 change
    for (i = 0; i < n && error.isEmpty(); ++i) {
        fields(QByteArray("iqiic"), indexed(prefix + QStringLiteral("switch.294"), i));
    }
    fields(QByteArray("biibbaabb"), prefix + QStringLiteral("switch.tail"));
}

/******************************************************************************
 * Function Name   : SourceDocument::page
 * Description     : Traverse one native page and reject every unimplemented top-level control.
 * Input Parameters: pagePrefix
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void SourceDocument::page(const QString &pagePrefix) //20261110-5 change
{
    int n;
    int i;
    int type;
    QString name;

    fields(QByteArray("ddddcciiqiiaqbbiiaqqb"), pagePrefix + QStringLiteral(".head"));
    n = count(pagePrefix + QStringLiteral(".94"));
    for (i = 0; i < n && error.isEmpty(); ++i) {
        name = indexed(pagePrefix + QStringLiteral(".94"), i);
        address(name + QStringLiteral(".address"));
        fields(QByteArray("biiiiqqs"), name + QStringLiteral(".tail"));
    }
    fields(QByteArray("bbiiibbbbqqqqi"), pagePrefix + QStringLiteral(".tail"));
    n = count(pagePrefix + QStringLiteral(".objects"), 1024);
    for (i = 0; i < n && error.isEmpty(); ++i) {
        name = indexed(pagePrefix == QStringLiteral("page") ? QStringLiteral("object") : pagePrefix + QStringLiteral(".object"), i) + QStringLiteral(".");
        read('b', name + QStringLiteral("movable"));
        type = read('i', name + QStringLiteral("type")).toInt();
        if (type != 10 && type != 28) {
            reject(name, QStringLiteral("尚未恢复的原生控件类型 %1").arg(type));
            return;
        }
        objectPrefixes.append(name);
        control(name, type);
        read('d', name + QStringLiteral("z"));
    }
}

/******************************************************************************
 * Function Name   : SourceDocument::history
 * Description     : Traverse saved macro source, compiled caches and address history records.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void SourceDocument::history() //20261109-0 add
{
    int n;
    int m;
    int i;
    int j;
    QString name;
    QString child;

    n = count(QStringLiteral("history.1e4"), 200);
    for (i = 0; i < n && error.isEmpty(); ++i) {
        name = indexed(QStringLiteral("history.1e4"), i);
        read('q', name + QStringLiteral(".source"));
        list(name + QStringLiteral(".8"));
        fields(QByteArray("bqb"), name + QStringLiteral(".head"));
        read('x', name + QStringLiteral(".code"));
        m = count(name + QStringLiteral(".c"));
        for (j = 0; j < m && error.isEmpty(); ++j) {
            child = indexed(name + QStringLiteral(".c"), j);
            address(child + QStringLiteral(".address"));
            fields(QByteArray("iiqi"), child + QStringLiteral(".tail"));
        }
    }
    n = count(QStringLiteral("history.1981f8"));
    for (i = 0; i < n && error.isEmpty(); ++i) {
        name = indexed(QStringLiteral("history.1981f8"), i);
        address(name + QStringLiteral(".address"));
        fields(QByteArray("iiqi"), name + QStringLiteral(".tail"));
    }
}

/******************************************************************************
 * Function Name   : SourceDocument::parse
 * Description     : Validate recovered version-122 page boundaries without enabling multi-page semantic edits.
 * Input Parameters: None
 * Return Value    : True only after every byte and supported control has been traversed.
 * Date            : 2026.11.09
 ******************************************************************************/
bool SourceDocument::parse() //20261109-0 add
{
    NativeProjectEnvelope envelope;
    int n;
    int i;

    if (!inspectNativeProjectEnvelope(bytes, &envelope, &error)) {
        return false;
    }
    fields(QByteArray("qiq"), QStringLiteral("header"));
    system();
    configuration();
    n = count(QStringLiteral("project.pages"), 200);
    if (n < 1) { //20261110-5 change: Multi-page typed inspection stays separate from semantic import capability.
        reject(QStringLiteral("project.pages"), QStringLiteral("页面集合不能为空"));
    }
    for (i = 0; i < n && error.isEmpty(); ++i) {
        page(i == 0 ? QStringLiteral("page") : indexed(QStringLiteral("page"), i));
    }
    history();
    read('i', QStringLiteral("trailer.param3"));
    fields(QByteArray(12, 'q') + QByteArray("cciqbbi"), QStringLiteral("trailer.settings"));
    n = count(QStringLiteral("trailer.page-map"), 200);
    for (i = 0; i < n && error.isEmpty(); ++i) {
        fields(QByteArray("ii"), indexed(QStringLiteral("trailer.page-map"), i));
    }
    if (error.isEmpty() && m_position != bytes.size()) {
        reject(QStringLiteral("file-end"), QStringLiteral("存在未解析的原生记录或尾部数据"));
    }
    return error.isEmpty();
}

/******************************************************************************
 * Function Name   : sourceNumberType
 * Description     : Translate the five recovered native numeric type codes to editor indices.
 * Input Parameters: nativeType
 * Return Value    : Editor index, or minus one for an unknown native type.
 * Date            : 2026.11.09
 ******************************************************************************/
int sourceNumberType(int nativeType) //20261109-0 add
{
    const int types[5] = {4, 5, 2, 3, 8};
    int i;

    for (i = 0; i < 5; ++i) {
        if (nativeType == types[i]) {
            return i;
        }
    }
    return -1;
}

/******************************************************************************
 * Function Name   : sourceWordNumberType
 * Description     : Translate the word-button numeric enum independently of permission conditions.
 * Input Parameters: nativeType
 * Return Value    : Editor index, or minus one for an unsupported native value.
 * Date            : 2026.11.09
 ******************************************************************************/
int sourceWordNumberType(int nativeType) //20261109-6 add
{
    // The subtype-one writer copies the UI enum directly; its width branch uses 1/3/5/6 for two words.
    return nativeType >= 0 && nativeType < 7 ? nativeType : -1;
}

/******************************************************************************
 * Function Name   : canonicalSourceAddress
 * Description     : Normalize decimal local address text while keeping original bytes in the snapshot.
 * Input Parameters: text
 * Return Value    : Canonical local text, or the original text for unsupported external syntax.
 * Date            : 2026.11.09
 ******************************************************************************/
QString canonicalSourceAddress(const QString &text) //20261109-0 add
{
    QRegularExpression expression(QStringLiteral("^(LB|LW)([0-9]+)$"));
    QRegularExpressionMatch match;
    bool ok;
    u32 number;

    match = expression.match(text);
    if (!match.hasMatch()) {
        return text;
    }
    number = match.captured(2).toUInt(&ok);
    if (!ok) {
        return text;
    }
    return match.captured(1) + QString::number(number);
}

/******************************************************************************
 * Function Name   : sourceCondition
 * Description     : Restore touch or visibility conditions from verified common-object offsets.
 * Input Parameters: document, prefix, touch, warnings
 * Return Value    : Editable access condition with native level indices translated to display levels.
 * Date            : 2026.11.09
 ******************************************************************************/
StaticTextProperties sourceCondition(const SourceDocument &document, const QString &prefix, bool touch,
                                      QStringList *warnings) //20261109-0 add
{
    StaticTextProperties result;
    QString gate;
    QString bitAddress;
    QString bitState;
    QString level;
    QString wordAddress;
    QString numeric;
    QString numberType;
    int flags;
    int raw;

    gate = prefix + (touch ? QStringLiteral("common.touch") : QStringLiteral("common.visibility"));
    bitAddress = prefix + (touch ? QStringLiteral("common.40.0") : QStringLiteral("common.90.0"));
    bitState = prefix + (touch ? QStringLiteral("common.58.0") : QStringLiteral("common.a8.0"));
    level = prefix + (touch ? QStringLiteral("common.58.1") : QStringLiteral("common.a8.1"));
    numberType = prefix + (touch ? QStringLiteral("common.a8.2") : QStringLiteral("common.78.3"));
    wordAddress = prefix + (touch ? QStringLiteral("common.60.0") : QStringLiteral("common.b0.0"));
    numeric = prefix + (touch ? QStringLiteral("common.78") : QStringLiteral("common.c8"));
    flags = document.value(gate + QStringLiteral(".1")).toInt();
    result.visibilityEnabled = document.value(gate + QStringLiteral(".0")).toInt() != 0;
    result.bitEnabled = (flags & 4) != 0;
    result.levelEnabled = (flags & 1) != 0;
    result.wordEnabled = (flags & 2) != 0;
    result.bitAddress = canonicalSourceAddress(document.value(bitAddress).toString());
    result.bitState = document.value(bitState).toInt() != 0;
    result.minimumLevel = document.value(level).toInt() + 1;
    result.wordAddress = canonicalSourceAddress(document.value(wordAddress).toString());
    result.dataType = sourceNumberType(document.value(numberType).toInt());
    if ((!result.visibilityEnabled || !result.wordEnabled) && result.dataType < 0) {
        raw = document.value(numberType).toInt();
        result.dataType = 0;
        if (raw != 0) {
            warnings->append(QStringLiteral("%1 的未启用数值类型含原厂脏值 %2；草稿显示默认类型，原始字节保留。")
                                 .arg(numeric).arg(raw));
        }
    }
    result.compareValue = document.value(numeric + QStringLiteral(".1")).toString();
    result.tolerance = document.value(numeric + QStringLiteral(".2")).toString();
    result.comparison = document.value(numeric + QStringLiteral(".0")).toInt();
    if ((!result.visibilityEnabled || !result.wordEnabled) && (result.comparison < 0 || result.comparison > 5)) {
        warnings->append(QStringLiteral("%1 的未启用比较符含原厂脏值 %2；草稿显示默认比较符，原始字节保留。")
                             .arg(gate).arg(result.comparison));
        result.comparison = 0;
    }
    if ((!result.visibilityEnabled || !result.levelEnabled) && (result.minimumLevel < 1 || result.minimumLevel > 8)) {
        warnings->append(QStringLiteral("%1 的未启用权限等级含原厂脏值 %2；草稿显示等级 1，原始字节保留。")
                             .arg(gate).arg(result.minimumLevel - 1));
        result.minimumLevel = 1;
    }
    raw = document.value(bitState).toInt();
    if (raw != 0 && raw != 1) {
        if (result.visibilityEnabled && result.bitEnabled) {
            result.comparison = -1;
        } else {
            warnings->append(QStringLiteral("%1 的未启用位状态含原厂脏值 %2；草稿显示默认状态，原始字节保留。")
                                 .arg(gate).arg(raw));
            result.bitState = true;
        }
    }
    if ((flags & ~7) != 0) {
        result.comparison = -1; // Force validation to reject unsupported condition flags instead of discarding them.
    }
    if (result.compareValue.isEmpty()) {
        result.compareValue = QStringLiteral("0");
    }
    if (result.tolerance.isEmpty()) {
        result.tolerance = QStringLiteral("0");
    }
    return result;
}

/******************************************************************************
 * Function Name   : modelSourceAddress
 * Description     : Restore local addresses or the independently observed default COM1 Modbus binding.
 * Input Parameters: document, name, text, external, error
 * Return Value    : True for a verified address-to-connection mapping.
 * Date            : 2026.11.09
 ******************************************************************************/
bool modelSourceAddress(const SourceDocument &document, const QString &name, QString *text,
                          bool *external, QString *error) //20261109-0 add
{
    QString family;
    QString numberText;
    QStringList families;
    u32 number;
    bool ok;

    if (document.value(name + QStringLiteral(".1")).toInt() == 0) {
        *text = canonicalSourceAddress(document.value(name + QStringLiteral(".0")).toString());
        return true;
    }
    family = document.value(name + QStringLiteral(".5")).toString();
    numberText = document.value(name + QStringLiteral(".4")).toString();
    families = QStringList{QStringLiteral("0x"), QStringLiteral("1x"), QStringLiteral("3x_bit"), QStringLiteral("4x_bit"),
                           QStringLiteral("6x_bit"), QStringLiteral("10x_bit"), QStringLiteral("3x"), QStringLiteral("4x"),
                           QStringLiteral("3x_dw"), QStringLiteral("4x_dw"), QStringLiteral("6x"), QStringLiteral("10x")};
    number = numberText.toUInt(&ok, 10);
    if (document.value(name + QStringLiteral(".1")).toInt() != 1 ||
        document.value(name + QStringLiteral(".8")).toString() != QStringLiteral("Modbus_RTU") ||
        !families.contains(family) || !ok || !nativeProjectUsesSupportedDeviceDefaults(document.bytes, error)) {
        return fail(error, QStringLiteral("尚未核实原生源外部地址的协议连接绑定：%1。").arg(name));
    }
    *text = QStringLiteral("@com1:") + family + QString::number(number);
    *external = true;
    return true;
}

/******************************************************************************
 * Function Name   : sourceStaticText
 * Description     : Restore static-text languages only when their shared appearance is fully representable.
 * Input Parameters: document, prefix, index, languages, object, warnings, external, error
 * Return Value    : True for a representable native static-text control.
 * Date            : 2026.11.09
 ******************************************************************************/
bool sourceStaticText(const SourceDocument &document, const QString &prefix, int index, const QStringList &languages,
                        HmiObject *object, QStringList *warnings, bool *external, QString *error) //20261109-0 add
{
    HmiObject result;
    StaticTextProperties properties;
    QString row;
    double x;
    double y;
    double width;
    double height;
    int flags;
    int language;
    int field;
    QString other;
    const int styleFields[13] = {0, 1, 3, 5, 6, 7, 9, 12, 13, 14, 15, 16, 8};

    if (document.value(prefix + QStringLiteral("static.194.count")).toInt() != languages.size()) {
        return fail(error, QStringLiteral("原生静态文本语言记录数量与工程语言注册表不对应。"));
    }
    row = SourceDocument::indexed(prefix + QStringLiteral("static.194"), 0);
    result.id = QStringLiteral("native-text-%1").arg(index);
    result.type = QStringLiteral("text");
    result.name = document.value(prefix + QStringLiteral("group.metadata-name")).toString();
    if (result.name.isEmpty()) {
        result.name = QStringLiteral("静态文本%1").arg(index + 1);
    }
    x = document.value(prefix + QStringLiteral("rectangle.0")).toDouble() + document.value(prefix + QStringLiteral("rectangle.9")).toDouble();
    y = document.value(prefix + QStringLiteral("rectangle.1")).toDouble() + document.value(prefix + QStringLiteral("rectangle.10")).toDouble();
    width = document.value(prefix + QStringLiteral("text.head.2")).toDouble();
    height = document.value(prefix + QStringLiteral("text.head.3")).toDouble();
    if (std::abs(x) > 32768 || std::abs(y) > 32768 || width < 1 || width > 4096 || height < 1 || height > 4096) {
        return fail(error, QStringLiteral("原生静态文本几何数据无法表示。"));
    }
    result.geometry = QRect(qRound(x), qRound(y), qRound(width), qRound(height));
    result.layer = document.value(prefix + QStringLiteral("z")).toDouble();
    result.hasStaticTextProperties = true;
    result.text = document.value(row + QStringLiteral(".10")).toString();
    properties = sourceCondition(document, prefix, false, warnings);
    if (!modelSourceAddress(document, prefix + QStringLiteral("common.90"), &properties.bitAddress, external, error) ||
        !modelSourceAddress(document, prefix + QStringLiteral("common.b0"), &properties.wordAddress, external, error)) {
        return false;
    }
    properties.fontFamily = document.value(row + QStringLiteral(".3")).toString();
    properties.fontSize = document.value(row + QStringLiteral(".5")).toInt();
    properties.spacing = document.value(row + QStringLiteral(".16")).toInt();
    properties.alignment = document.value(prefix + QStringLiteral("static.head.2")).toInt();
    properties.adaptive = document.value(row + QStringLiteral(".0")).toInt() != 0;
    properties.bold = document.value(row + QStringLiteral(".1")).toInt() != 0;
    properties.italic = document.value(row + QStringLiteral(".6")).toInt() != 0;
    properties.underline = document.value(row + QStringLiteral(".9")).toInt() != 0;
    properties.textColor = QColor(document.value(row + QStringLiteral(".7")).toString());
    properties.backgroundColor = QColor(document.value(row + QStringLiteral(".12")).toString());
    properties.foregroundColor = QColor(document.value(row + QStringLiteral(".13")).toString());
    properties.opacity = document.value(row + QStringLiteral(".14")).toInt();
    properties.style = document.value(row + QStringLiteral(".15")).toInt();
    for (language = 0; language < languages.size(); ++language) {
        other = SourceDocument::indexed(prefix + QStringLiteral("static.194"), language);
        for (field = 0; field < 13; ++field) {
            if (document.value(other + QLatin1Char('.') + QString::number(styleFields[field])) !=
                document.value(row + QLatin1Char('.') + QString::number(styleFields[field]))) {
                return fail(error, QStringLiteral("原生静态文本各语言的字体或样式不同，当前共用样式模型不能无损表示。"));
            }
        }
        properties.languageTexts.insert(languages.at(language), document.value(other + QStringLiteral(".10")).toString());
    }
    flags = document.value(prefix + QStringLiteral("static.head.3")).toInt();
    if ((flags & ~11) != 0) {
        return fail(error, QStringLiteral("原生静态文本字体样式含未支持的标志位。"));
    }
    if (!properties.validate(error)) {
        return false;
    }
    result.staticText = properties;
    *object = result;
    return true;
}

/******************************************************************************
 * Function Name   : sourceLampMonitoring
 * Description     : Restore verified lamp monitoring while preserving dormant and implicit native state metadata.
 * Input Parameters: document, prefix, object, shared, stateCount, warnings, external, error
 * Return Value    : True for supported dual or sequential multi-state lamp records.
 * Date            : 2026.11.09
 ******************************************************************************/
bool sourceLampMonitoring(const SourceDocument &document, const QString &prefix, HmiObject *object,
                             BitButtonProperties *shared, int stateCount, QStringList *warnings,
                             bool *external, QString *error) //20261109-8 add
{
    int mode;
    int type;
    int count;
    int i;
    int words;

    mode = document.value(prefix + QStringLiteral("switch.248.5")).toInt();
    if (mode < 1 || mode > 2 || stateCount < 1 || stateCount > 32 ||
        document.value(prefix + QStringLiteral("switch.248.3")).toInt() != stateCount) {
        return fail(error, QStringLiteral("原生指示灯监视模式或状态数量超出已核实范围。"));
    }
    if (shared->touch.visibilityEnabled || shared->pressTimeEnabled || shared->macroEnabled || shared->notice ||
        document.value(prefix + QStringLiteral("switch.tail.8")).toInt() != 0) {
        return fail(error, QStringLiteral("原生指示灯含未开放的活动宏、通知、触控、按压时间或按压效果；仅可保留原始归档，不能作为已支持指示灯导入。"));
    }
    object->hasLampProperties = true;
    object->lamp.monitorMode = mode;
    shared->indicator = mode != 0;
    shared->dualMonitor = mode == 1;
    shared->monitorSame = false;
    shared->monitorType = 0;
    shared->bitIndex = 0;
    if (mode == 1) {
        type = document.value(prefix + QStringLiteral("switch.248.0")).toInt();
        if (stateCount != 2 || (type != 0 && type != 1)) {
            return fail(error, QStringLiteral("原生双态指示灯地址类型或外观状态数无效。"));
        }
        shared->monitorType = type;
        shared->monitorSame = document.value(prefix + QStringLiteral("switch.248.2")).toInt() != 0;
        if (type == 1) {
            shared->bitIndex = document.value(prefix + QStringLiteral("switch.248.1")).toInt();
        }
        if (!modelSourceAddress(document, prefix + QStringLiteral("switch.254"),
                                  type == 0 ? &shared->monitorBitAddress : &shared->monitorWordAddress, external, error)) {
            return false;
        }
        if (document.value(prefix + QStringLiteral("switch.248.4")).toInt() < 0 ||
            document.value(prefix + QStringLiteral("switch.248.4")).toInt() > 1) {
            warnings->append(QStringLiteral("控件 %1 的未启用多态数值类型含原厂脏值；草稿显示默认类型，原始字节保留。")
                                 .arg(object->name));
        }
    } else if (mode == 2) {
        type = document.value(prefix + QStringLiteral("switch.248.4")).toInt();
        count = document.value(prefix + QStringLiteral("switch.278.count")).toInt();
        if (stateCount < 2 || type < 0 || type > 1 || (count != 0 && count != stateCount)) {
            return fail(error, QStringLiteral("原生多态指示灯数据类型、状态数量或状态值表无法无损映射。"));
        }
        for (i = 0; i < count; ++i) {
            if (document.value(SourceDocument::indexed(prefix + QStringLiteral("switch.278"), i)).toDouble() != i) {
                return fail(error, QStringLiteral("原生多态指示灯启用了非顺序状态值，当前按状态序号编辑模型无法无损表示。"));
            }
        }
        object->lamp.multiDataType = type;
        object->lamp.multiStateCount = stateCount;
        object->lamp.multiMonitorSame = document.value(prefix + QStringLiteral("switch.248.2")).toInt() != 0;
        if (!modelSourceAddress(document, prefix + QStringLiteral("switch.254"), &object->lamp.multiMonitorAddress, external, error)) {
            return false;
        }
        if (count == 0) {
            warnings->append(QStringLiteral("控件 %1 的原厂多态状态值表为空；草稿按状态序号显示，源保存保留原始空表，不开放状态值编辑。")
                                 .arg(object->name));
        }
    }
    words = mode == 2 ? object->lamp.transferWordCount() : 1;
    if (document.value(prefix + QStringLiteral("switch.254.7")).toInt() != words) { //20261109-8 add: The original U32 lamp source explicitly stores a two-word address width at source+268.
        return fail(error, QStringLiteral("原生指示灯监视地址字长与活动数据类型不对应，不能无损导入。"));
    }
    object->lamp.ensureStateCount(stateCount, *shared);
    return true;
}

/******************************************************************************
 * Function Name   : sourceProject
 * Description     : Convert verified native controls and retain the complete original source snapshot.
 * Input Parameters: document, project, error
 * Return Value    : True for representable source projects.
 * Date            : 2026.11.09
 ******************************************************************************/
bool sourceProject(const SourceDocument &document, HmiProject *project, QString *error) //20261109-8 change
{
    HmiProject result;
    HmiObject object;
    BitButtonProperties properties;
    QString prefix;
    QString row;
    QString picture;
    QString macro;
    QString source;
    QString languageName;
    QString languageRow;
    QString firstLanguageRow;
    int i;
    int state;
    int stateCount;
    int flags;
    int monitorMode;
    int macroCount;
    int pictureCount;
    int language;
    int languageCount;
    int field;
    qsizetype summarizedWarnings; //20261110-5 add: Generated diagnostics must fit the bounded KSP warning carrier.
    bool external;
    bool wordButton; //20261109-6 add
    bool lamp; //20261109-8 add
    bool functionButton; //20261110-0 add
    BitButtonState *appearance;
    PlcConnection connection;

    if (document.value(QStringLiteral("project.pages.count")).toInt() != 1) { //20261110-5 add: Read-only multi-page boundary recognition is not a semantic editing contract.
        return fail(error, QStringLiteral("原生源含多个页面；当前语义互通仅支持一个普通页面，请保留整源归档，不能截取第一页导入。"));
    }
    result.name = QFileInfo(document.value(QStringLiteral("trailer.settings.1")).toString()).completeBaseName();
    if (result.name.isEmpty()) {
        result.name = QStringLiteral("NativeProject");
    }
    result.targetModel = document.value(QStringLiteral("header.2")).toString();
    result.pageName = document.value(QStringLiteral("page.head.18")).toString();
    if (document.value(QStringLiteral("config.head.4")).toString() != result.pageName ||
        document.value(QStringLiteral("config.head.11")).toString() != result.pageName ||
        document.value(QStringLiteral("trailer.settings.11")).toString() != result.pageName) { //20261110-5 add: Distinct startup/window references cannot be flattened into a one-page model.
        return fail(error, QStringLiteral("原生页面名称与工程启动/画面镜像不一致；当前单页源模型不能无损表达该配置。"));
    }
    if (document.value(QStringLiteral("page.head.2")).toDouble() != 800 ||
        document.value(QStringLiteral("page.head.3")).toDouble() != 480) {
        return fail(error, QStringLiteral("原生页面尺寸与已核实型号的 800×480 分辨率不符。"));
    }
    result.pageWidth = qRound(document.value(QStringLiteral("page.head.2")).toDouble());
    result.pageHeight = qRound(document.value(QStringLiteral("page.head.3")).toDouble());
    result.languages.clear();
    languageCount = document.value(QStringLiteral("system.164.count")).toInt();
    if (languageCount < 1 || languageCount > 8 || document.value(QStringLiteral("system.identity.15")).toInt() != languageCount ||
        document.value(QStringLiteral("system.identity.16")).toInt() < 0 ||
        document.value(QStringLiteral("system.identity.16")).toInt() >= languageCount ||
        document.value(QStringLiteral("config.290.0")).toInt() < 0 ||
        document.value(QStringLiteral("config.290.0")).toInt() >= languageCount) { //20261110-6 change: Native UI saves the dialog selection independently of the editor/compiler selection.
        return fail(error, QStringLiteral("原生工程语言数量、编辑语言、设置窗口语言或注册表无效。"));
    }
    for (language = 0; language < languageCount; ++language) {
        languageName = document.value(SourceDocument::indexed(QStringLiteral("system.164"), language)).toString();
        if (languageName.isEmpty() || languageName.size() > 64 || result.languages.contains(languageName)) {
            return fail(error, QStringLiteral("原生工程语言名称为空、重复或超出已验证长度。"));
        }
        result.languages.append(languageName);
    }
    result.activeLanguage = result.languages.at(document.value(QStringLiteral("system.identity.16")).toInt());
    result.objects.clear();
    result.nativeSourceSnapshot = document.bytes;
    external = false;
    for (i = 0; i < document.objectPrefixes.size(); ++i) {
        prefix = document.objectPrefixes.at(i);
        if (document.value(prefix + QStringLiteral("type")).toInt() == 28) {
            if (!sourceStaticText(document, prefix, i, result.languages, &object, &result.nativeSourceWarnings, &external, error)) {
                return false;
            }
            result.objects.append(object);
            continue;
        }
        object = HmiObject();
        properties = BitButtonProperties();
        wordButton = document.value(prefix + QStringLiteral("switch.mode")).toInt() == 1; //20261109-6 add
        lamp = document.value(prefix + QStringLiteral("switch.mode")).toInt() == 4; //20261109-8 add
        functionButton = document.value(prefix + QStringLiteral("switch.mode")).toInt() == 3; //20261110-0 add
        object.id = QStringLiteral("native-switch-%1").arg(i);
        object.type = QStringLiteral("switch");
        object.name = document.value(prefix + QStringLiteral("group.metadata-name")).toString();
        if (object.name.isEmpty()) {
            object.name = (lamp ? QStringLiteral("指示灯%1") : functionButton ? QStringLiteral("功能按钮%1") : wordButton ? QStringLiteral("字按钮%1") : QStringLiteral("位按钮%1")).arg(i + 1); //20261110-0 change
        }
        if (!lamp && !functionButton && !modelSourceAddress(document, prefix + (wordButton ? QStringLiteral("word.150") : QStringLiteral("switch.12c")),
                                  &object.address, &external, error)) { //20261110-0 change: Lamps and system functions have no operation address.
            return false;
        }
        if (std::abs(document.value(prefix + QStringLiteral("rectangle.0")).toDouble()) > 32768 ||
            std::abs(document.value(prefix + QStringLiteral("rectangle.1")).toDouble()) > 32768 ||
            std::abs(document.value(prefix + QStringLiteral("rectangle.9")).toDouble()) > 32768 ||
            std::abs(document.value(prefix + QStringLiteral("rectangle.10")).toDouble()) > 32768 ||
            document.value(prefix + QStringLiteral("text.head.2")).toDouble() < 1 ||
            document.value(prefix + QStringLiteral("text.head.2")).toDouble() > 4096 ||
            document.value(prefix + QStringLiteral("text.head.3")).toDouble() < 1 ||
            document.value(prefix + QStringLiteral("text.head.3")).toDouble() > 4096) {
            return fail(error, QStringLiteral("原生位按钮几何数据无法表示为当前编辑模型。"));
        }
        object.geometry = QRect(qRound(document.value(prefix + QStringLiteral("rectangle.0")).toDouble() +
                                       document.value(prefix + QStringLiteral("rectangle.9")).toDouble()),
                                qRound(document.value(prefix + QStringLiteral("rectangle.1")).toDouble() +
                                       document.value(prefix + QStringLiteral("rectangle.10")).toDouble()),
                                qRound(document.value(prefix + QStringLiteral("text.head.2")).toDouble()),
                                qRound(document.value(prefix + QStringLiteral("text.head.3")).toDouble()));
        object.layer = document.value(prefix + QStringLiteral("z")).toDouble();
        object.hasBitButtonProperties = true;
        if (!wordButton && !lamp && !functionButton) { //20261110-0 change
            properties.operation = document.value(prefix + QStringLiteral("switch.144.0")).toInt();
        }
        properties.macroEnabled = document.value(prefix + QStringLiteral("switch.22c.1")).toInt() != 0;
        properties.macroName = document.value(prefix + QStringLiteral("switch.22c.2")).toString();
        if (properties.macroEnabled &&
            (properties.macroName.isEmpty() || document.value(prefix + QStringLiteral("switch.22c.0")).toInt() != 1 ||
             document.value(prefix + QStringLiteral("switch.22c.4")).toInt() != 1 ||
             (!document.value(prefix + QStringLiteral("switch.22c.3")).toString().isEmpty() &&
              document.value(prefix + QStringLiteral("switch.22c.3")).toString() != QStringLiteral("MACRO_MAIN")) ||
             !document.value(prefix + QStringLiteral("switch.22c.5")).toString().isEmpty())) {
            return fail(error, QStringLiteral("原生按钮宏绑定含未核实的入口、扩展标志或云端宏信息。"));
        }
        if (!wordButton && !lamp && !functionButton) { //20261110-0 change
            monitorMode = document.value(prefix + QStringLiteral("switch.248.5")).toInt();
            if (monitorMode < 0 || monitorMode > 2) {
                return fail(error, QStringLiteral("原生位按钮监视模式未恢复。"));
            }
            properties.indicator = monitorMode != 0;
            properties.dualMonitor = monitorMode == 1;
            properties.monitorSame = document.value(prefix + QStringLiteral("switch.248.2")).toInt() != 0;
            properties.monitorType = document.value(prefix + QStringLiteral("switch.248.0")).toInt() != 0 ? 1 : 0;
            properties.bitIndex = document.value(prefix + QStringLiteral("switch.248.1")).toInt();
            if (properties.monitorType == 0) {
                properties.monitorBitAddress = canonicalSourceAddress(document.value(prefix + QStringLiteral("switch.254.0")).toString());
            } else {
                properties.monitorWordAddress = canonicalSourceAddress(document.value(prefix + QStringLiteral("switch.254.0")).toString());
            }
        }
        properties.touch = sourceCondition(document, prefix, true, &result.nativeSourceWarnings);
        properties.visibility = sourceCondition(document, prefix, false, &result.nativeSourceWarnings);
        if (!modelSourceAddress(document, prefix + QStringLiteral("common.40"), &properties.touch.bitAddress, &external, error) ||
            !modelSourceAddress(document, prefix + QStringLiteral("common.60"), &properties.touch.wordAddress, &external, error) ||
            !modelSourceAddress(document, prefix + QStringLiteral("common.90"), &properties.visibility.bitAddress, &external, error) ||
            !modelSourceAddress(document, prefix + QStringLiteral("common.b0"), &properties.visibility.wordAddress, &external, error)) {
            return false;
        }
        properties.notice = document.value(prefix + QStringLiteral("common.58.2")).toInt() != 0;
        flags = document.value(prefix + QStringLiteral("common.c8.3")).toInt();
        properties.noticeBit = (flags & 1) != 0;
        properties.noticeWord = (flags & 2) != 0;
        properties.noticeBitValue = document.value(prefix + QStringLiteral("common.58.3")).toInt() != 0;
        properties.noticeBitAddress = canonicalSourceAddress(document.value(prefix + QStringLiteral("common.e4.0")).toString());
        properties.noticeWordAddress = canonicalSourceAddress(document.value(prefix + QStringLiteral("common.fc.0")).toString());
        if (!modelSourceAddress(document, prefix + QStringLiteral("common.e4"), &properties.noticeBitAddress, &external, error) ||
            !modelSourceAddress(document, prefix + QStringLiteral("common.fc"), &properties.noticeWordAddress, &external, error) ||
            (!wordButton && !lamp && !functionButton && !modelSourceAddress(document, prefix + QStringLiteral("switch.254"), //20261110-0 change
                                 properties.monitorType == 0 ? &properties.monitorBitAddress : &properties.monitorWordAddress, &external, error))) { //20261109-6 change
            return false;
        }
        properties.noticeWordType = sourceNumberType(document.value(prefix + QStringLiteral("common.c8.4")).toInt());
        if ((!properties.notice || !properties.noticeWord) && properties.noticeWordType < 0) {
            if (document.value(prefix + QStringLiteral("common.c8.4")).toInt() != 0) {
                result.nativeSourceWarnings.append(QStringLiteral("控件 %1 的未启用通知数值类型含原厂脏值 %2；草稿显示默认类型，原始字节保留。")
                                                       .arg(object.name).arg(document.value(prefix + QStringLiteral("common.c8.4")).toInt()));
            }
            properties.noticeWordType = 0;
        }
        if ((flags & ~3) != 0) {
            return fail(error, QStringLiteral("原生位按钮通知条件含未知标志位。"));
        }
        properties.noticeWordValue = document.value(prefix + QStringLiteral("common.118.0")).toString();
        if (document.value(prefix + QStringLiteral("common.58.3")).toInt() != 0 &&
            document.value(prefix + QStringLiteral("common.58.3")).toInt() != 1) {
            if (properties.notice && properties.noticeBit) {
                return fail(error, QStringLiteral("原生位按钮已启用通知的位状态不是已核实枚举。"));
            }
            properties.noticeBitValue = true;
            result.nativeSourceWarnings.append(QStringLiteral("控件 %1 的未启用通知位状态含原厂脏值；草稿显示默认状态，原始字节保留。")
                                                   .arg(object.name));
        }
        if (properties.noticeWordValue.isEmpty()) {
            properties.noticeWordValue = QStringLiteral("0");
        }
        properties.pressTimeEnabled = document.value(prefix + QStringLiteral("common.118.1")).toInt() != 0;
        properties.pressTime = document.value(prefix + QStringLiteral("common.118.2")).toInt();
        stateCount = document.value(prefix + QStringLiteral("switch.290.count")).toInt();
        pictureCount = document.value(prefix + QStringLiteral("switch.294.count")).toInt();
        if (stateCount < 1 || stateCount > (lamp ? 32 : 2) || pictureCount != stateCount) { //20261109-8 change
            return fail(error, QStringLiteral("原生位按钮状态文字/图片数量不对应，不能无损映射。"));
        }
        if (lamp && !sourceLampMonitoring(document, prefix, &object, &properties, stateCount,
                                           &result.nativeSourceWarnings, &external, error)) { //20261109-8 add
            return false;
        }
        if (functionButton) { //20261110-0 add: Unused monitor and opaque function-union integers remain in the retained source snapshot.
            object.hasFunctionButtonProperties = true;
            object.functionButton.function = document.value(prefix + QStringLiteral("function.1c8.0")).toInt();
            object.functionButton.type = FunctionButtonProperties::types(object.functionButton.function, result.languages).isEmpty() ?
                                             0 : document.value(prefix + QStringLiteral("function.1c8.1")).toInt();
            if (object.functionButton.function == 14) {
                object.functionButton.keyboardKey = document.value(prefix + QStringLiteral("function.1c8.5")).toInt();
                object.functionButton.keyboardText = document.value(prefix + QStringLiteral("function.1c8.6")).toString();
            }
            object.functionButton.pressedEffect = document.value(prefix + QStringLiteral("switch.tail.8")).toInt() != 0;
            properties.indicator = false;
            properties.dualMonitor = false;
            if (stateCount != (object.functionButton.pressedEffect ? 2 : 1) || properties.macroEnabled || (properties.noticeBit && properties.noticeWord) ||
                properties.pressTime < 0 || properties.pressTime > 255 ||
                !object.functionButton.validate(error) || (object.functionButton.function == 13 && object.functionButton.type >= languageCount)) {
                return fail(error, QStringLiteral("原生功能按钮的按下外观、禁用宏、互斥通知或系统功能字段不能无损映射。"));
            }
        }
        if (wordButton) { //20261109-6 add: Preserve dormant fields and opaque secondary-register behavior in the source snapshot.
            object.hasWordButtonProperties = true;
            properties.indicator = false;
            properties.dualMonitor = false;
            object.wordButton.resetDefaultsForDataType(sourceWordNumberType(document.value(prefix + QStringLiteral("word.16c.0")).toInt()));
            object.wordButton.dataType = sourceWordNumberType(document.value(prefix + QStringLiteral("word.16c.0")).toInt());
            object.wordButton.operation = document.value(prefix + QStringLiteral("word.16c.1")).toInt();
            if (object.wordButton.usesConstant()) {
                object.wordButton.constantValue = QString::number(document.value(prefix + QStringLiteral("word.16c.2")).toDouble(), 'g', 17);
            }
            if (object.wordButton.usesMinimum()) {
                object.wordButton.minimumValue = QString::number(document.value(prefix + QStringLiteral("word.16c.4")).toDouble(), 'g', 17);
            }
            if (object.wordButton.usesMaximum()) {
                object.wordButton.maximumValue = QString::number(document.value(prefix + QStringLiteral("word.16c.5")).toDouble(), 'g', 17);
            }
            if (object.wordButton.usesTotalDigits()) {
                object.wordButton.totalDigits = document.value(prefix + QStringLiteral("word.18c.0")).toInt();
            }
            if (object.wordButton.usesDecimalDigits()) {
                object.wordButton.decimalDigits = document.value(prefix + QStringLiteral("word.18c.1")).toInt();
            }
            object.wordButton.pressedEffect = document.value(prefix + QStringLiteral("switch.tail.8")).toInt() != 0;
            if (document.value(prefix + QStringLiteral("word.16c.3")).toInt() != 0 ||
                document.value(prefix + QStringLiteral("word.16c.6")).toInt() != 0) {
                result.nativeSourceWarnings.append(QStringLiteral("控件 %1 的原厂字操作标志为 %2/%3，第二地址记录尚未映射到编辑属性；源保存会逐字节保留，无法在此编辑其行为。")
                                                       .arg(object.name).arg(document.value(prefix + QStringLiteral("word.16c.3")).toInt())
                                                       .arg(document.value(prefix + QStringLiteral("word.16c.6")).toInt()));
            }
            if (stateCount != (object.wordButton.pressedEffect ? 2 : 1) || !object.wordButton.validate(error)) {
                return fail(error, QStringLiteral("原生字按钮数值属性或按压状态列表不能无损映射：%1。").arg(error != nullptr ? *error : QString()));
            }
        }
        for (state = 0; state < stateCount; ++state) {
            appearance = lamp ? object.lamp.state(properties, state) : &properties.states[state]; //20261109-8 add
            row = SourceDocument::indexed(prefix + QStringLiteral("switch.290"), state);
            if (document.value(row + QStringLiteral(".count")).toInt() != languageCount) {
                return fail(error, QStringLiteral("原生按钮状态语言数量与工程语言注册表不对应。"));
            }
            row = SourceDocument::indexed(row, 0);
            firstLanguageRow = row;
            picture = SourceDocument::indexed(prefix + QStringLiteral("switch.294"), state);
            appearance->sharedText = document.value(row + QStringLiteral(".4")).toString();
            appearance->text.languageTexts.clear();
            // Native null state-family fields select Song Ti; four original raster masks verify this fallback.
            appearance->text.fontFamily = QStringLiteral("宋体");
            for (language = 0; language < languageCount; ++language) {
                languageRow = SourceDocument::indexed(SourceDocument::indexed(prefix + QStringLiteral("switch.290"), state), language);
                for (field = 0; field < 4; ++field) {
                    if (document.value(languageRow + QLatin1Char('.') + QString::number(field)) !=
                        document.value(firstLanguageRow + QLatin1Char('.') + QString::number(field))) {
                        return fail(error, QStringLiteral("原生位按钮各语言的字体或样式不同，当前共用样式模型不能无损表示。"));
                    }
                }
                appearance->text.languageTexts.insert(result.languages.at(language), document.value(languageRow + QStringLiteral(".4")).toString());
            }
            if (!document.value(row + QStringLiteral(".1")).toString().isEmpty()) {
                appearance->text.fontFamily = document.value(row + QStringLiteral(".1")).toString();
            }
            appearance->text.fontSize = document.value(row + QStringLiteral(".3")).toInt();
            appearance->text.alignment = document.value(row + QStringLiteral(".2")).toInt();
            appearance->text.textColor = document.value(row + QStringLiteral(".0")).value<QColor>();
            appearance->picture = document.value(picture + QStringLiteral(".1")).toString();
            appearance->picture.remove(QStringLiteral(":/btn/images/"));
            if ((lamp || functionButton) && document.value(picture + QStringLiteral(".0")).toInt() != 1 &&
                document.value(picture + QStringLiteral(".0")).toInt() != 3) { //20261109-8 add
                return fail(error, QStringLiteral("原生按钮状态含尚未核实的图片类型；不能无损导入。")); //20261110-0 change
            }
            if (!lamp && !functionButton && document.value(picture + QStringLiteral(".0")).toInt() == 3) { //20261110-0 change
                return fail(error, QStringLiteral("原生位/字按钮含尚未核实的无图片状态；仅可保留原始归档，不能按图片状态导入。")); //20261110-0 change
            }
            appearance->useImage = (lamp || functionButton) ? document.value(picture + QStringLiteral(".0")).toInt() == 1 :
                                         document.value(picture + QStringLiteral(".0")).toInt() != 0;
            appearance->sameLanguage = (lamp || functionButton) && document.value(prefix + QStringLiteral("switch.tail.4")).toInt() != 0; //20261110-0 change
            appearance->flash = document.value(picture + QStringLiteral(".2")).toInt() != 0;
            appearance->opacity = document.value(picture + QStringLiteral(".3")).toInt();
            appearance->background = document.value(picture + QStringLiteral(".4")).value<QColor>();
            appearance->effect3d = document.value(prefix + QStringLiteral("switch.tail.0")).toInt() != 0;
            appearance->transparent = document.value(prefix + QStringLiteral("switch.tail.7")).toInt() != 0;
        }
        if (stateCount == 1) {
            properties.states[1] = properties.states[0];
            result.nativeSourceWarnings.append(((wordButton || functionButton) ?
                                                QStringLiteral("控件 %1 的原厂无按压效果源仅保存一个显示状态；草稿中的第二状态由第一状态复制，隐藏按压状态不属于该源记录。") :
                                                QStringLiteral("控件 %1 的原厂无监视源仅保存一个显示状态；草稿中的第二状态由第一状态复制，隐藏 ON 状态不属于该源记录。")) //20261109-6 change
                                                   .arg(object.name));
        }
        object.text = properties.states[0].sharedText;
        object.bitButton = properties;
        if (!properties.validate(error, functionButton) || (lamp && !object.lamp.validate(error))) { //20261110-0 change
            return false;
        }
        result.objects.append(object);
    }
    macroCount = document.value(QStringLiteral("history.1e4.count")).toInt();
    if (document.value(QStringLiteral("history.1981f8.count")).toInt() != 0 ||
        document.value(QStringLiteral("system.194.count")).toInt() != 0 || document.value(QStringLiteral("config.284.count")).toInt() != 0) {
        return fail(error, QStringLiteral("原生工程包含尚未恢复到编辑模型的宏变量、启动宏或系统宏；仅可保留原始归档，不能作为普通宏工程导入。"));
    }
    for (i = 0; i < macroCount; ++i) {
        prefix = SourceDocument::indexed(QStringLiteral("history.1e4"), i);
        macro = document.value(prefix + QStringLiteral(".head.1")).toString();
        source = document.value(prefix + QStringLiteral(".source")).toString();
        if (macro.isEmpty() || result.macros.contains(macro)) {
            return fail(error, QStringLiteral("原生宏名称为空或重复，不能无损映射。"));
        }
        if (document.value(prefix + QStringLiteral(".8.count")).toInt() != 0 ||
            document.value(prefix + QStringLiteral(".c.count")).toInt() != 0 ||
            document.value(prefix + QStringLiteral(".head.2")).toInt() != 1 ||
            (document.value(prefix + QStringLiteral(".head.0")).toInt() == 0 ?
                !document.value(prefix + QStringLiteral(".code")).toByteArray().isEmpty() :
                document.value(prefix + QStringLiteral(".head.0")).toInt() != 1 ||
                document.value(prefix + QStringLiteral(".code")).toByteArray().size() != 16)) {
            return fail(error, QStringLiteral("宏 %1 含未恢复的依赖、变量、MD5或运行标志，不能导入为普通源码宏。").arg(macro));
        }
        result.macros.insert(macro, source);
        if (document.value(prefix + QStringLiteral(".head.0")).toInt() == 0) {
            result.nativeSourceWarnings.append(QStringLiteral("宏 %1 是原厂可编辑的未编译源码；原厂离线编译前须在宏编辑器编译，独立下载将重新编译源码。")
                                                   .arg(macro));
        }
    }
    for (i = 0; i < result.objects.size(); ++i) {
        if (result.objects.at(i).hasBitButtonProperties && result.objects.at(i).bitButton.macroEnabled &&
            !result.macros.contains(result.objects.at(i).bitButton.macroName)) {
            return fail(error, QStringLiteral("原生按钮绑定的宏在工程宏注册表中不存在：%1。").arg(result.objects.at(i).bitButton.macroName));
        }
    }
    if (external) {
        connection.id = QStringLiteral("com1");
        connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
        connection.channel = PlcChannel::Com1;
        connection.station = QStringLiteral("1");
        connection.nodeKnown = true;
        connection.node = 1;
        result.connections.append(connection);
    }
    if (result.nativeSourceWarnings.size() > 128) { //20261110-5 add: Bound diagnostics explicitly while preserving every original source byte.
        summarizedWarnings = result.nativeSourceWarnings.size() - 127;
        result.nativeSourceWarnings = result.nativeSourceWarnings.mid(0, 127);
        result.nativeSourceWarnings.append(QStringLiteral("另有 %1 条休眠字段诊断未逐条展示；全部原始值仍完整保留在 nativeSourceSnapshot，可通过 typed 字段检查定位。此汇总不表示未知活动语义已支持。")
                                              .arg(summarizedWarnings));
    }
    if (!result.validateSinglePageBackend(error)) { //20261110-5 add: Syntax acceptance cannot bypass the active-model capability gate.
        return false;
    }
    *project = result;
    clearError(error);
    return true;
}

//20261109-0 add: Changed fields are encoded independently and spliced only at parsed boundaries.
class SourceUpdates
{
public:
    const SourceDocument &document;
    QMap<qsizetype, QPair<qsizetype, QByteArray>> replacements;
    QString error;

    /******************************************************************************
     * Function Name   : SourceUpdates
     * Description     : Start an update transaction against one fully parsed source snapshot.
     * Input Parameters: source
     * Return Value    : None.
     * Date            : 2026.11.09
     ******************************************************************************/
    explicit SourceUpdates(const SourceDocument &source) : document(source) //20261109-0 add
    {
    }

    /******************************************************************************
     * Function Name   : set
     * Description     : Replace a known primitive span using the native Qt4 representation.
     * Input Parameters: name, value
     * Return Value    : None; the first encoding error is retained.
     * Date            : 2026.11.09
     ******************************************************************************/
    void set(const QString &name, const QVariant &value) //20261109-0 add
    {
        SourceField field;
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        if (!error.isEmpty()) {
            return;
        }
        if (!document.values.contains(name)) {
            error = QStringLiteral("原生源字段不存在，不能写入：%1。").arg(name);
            return;
        }
        field = document.values.value(name);
        stream.setVersion(QDataStream::Qt_4_8);
        if (field.kind == 'q') {
            if (!appendString(&data, value.toString())) {
                error = QStringLiteral("原生源 QString 长度超出范围。");
                return;
            }
        } else if (field.kind == 'i') {
            appendWord(&data, static_cast<u32>(value.toInt()));
        } else if (field.kind == 's') {
            stream << static_cast<short>(value.toInt());
        } else if (field.kind == 'a' || field.kind == 'b') {
            data.append(static_cast<char>(value.toInt()));
        } else if (field.kind == 'd') {
            stream << value.toDouble();
        } else if (field.kind == 'c') {
            stream << value.value<QColor>();
        } else if (field.kind == 'x') {
            stream << value.toByteArray();
        } else {
            error = QStringLiteral("该原生源字段尚未验证可写：%1。").arg(name);
            return;
        }
        replacements.insert(field.start, qMakePair(field.end, data));
    }

    /******************************************************************************
     * Function Name   : address
     * Description     : Update local address strings and the two verified bit/word discriminator bytes.
     * Input Parameters: name, text
     * Return Value    : None; unsupported external addresses latch a diagnostic.
     * Date            : 2026.11.09
     ******************************************************************************/
    void address(const QString &name, const QString &text) //20261109-0 add
    {
        QRegularExpression expression(QStringLiteral("^(LB|LW)([0-9]+)$"));
        QRegularExpressionMatch match;
        u32 number;
        bool ok;
        int word;

        match = expression.match(text);
        number = match.captured(2).toUInt(&ok);
        if (!match.hasMatch() || !ok || number > 4096) {
            error = QStringLiteral("原生源地址编辑目前仅验证 LB/LW 0..4096：%1。").arg(text);
            return;
        }
        word = match.captured(1) == QStringLiteral("LW") ? 1 : 0;
        set(name + QStringLiteral(".0"), match.captured(1) + QString::number(number));
        set(name + QStringLiteral(".1"), 0);
        set(name + QStringLiteral(".2"), word);
        set(name + QStringLiteral(".4"), QString::number(number));
        set(name + QStringLiteral(".5"), match.captured(1));
        set(name + QStringLiteral(".6"), word);
    }

    /******************************************************************************
     * Function Name   : render
     * Description     : Build a new stream while copying every untouched byte exactly once.
     * Input Parameters: begin, end
     * Return Value    : Updated source stream, empty after a transaction failure.
     * Date            : 2026.11.09
     ******************************************************************************/
    QByteArray render(qsizetype begin = 0, qsizetype end = -1) //20261109-0 add
    {
        QByteArray result;
        qsizetype position;
        QMap<qsizetype, QPair<qsizetype, QByteArray>>::const_iterator iterator;

        if (!error.isEmpty()) {
            return QByteArray();
        }
        if (end == -1) {
            end = document.bytes.size();
        }
        if (begin < 0 || end < begin || end > document.bytes.size()) {
            error = QStringLiteral("原生源更新边界无效。");
            return QByteArray();
        }
        position = begin;
        for (iterator = replacements.constBegin(); iterator != replacements.constEnd(); ++iterator) {
            if (iterator.value().first <= begin || iterator.key() >= end) {
                continue;
            }
            if (iterator.key() < position || iterator.value().first > end) {
                error = QStringLiteral("原生源更新区域重叠，已取消保存。");
                return QByteArray();
            }
            result.append(document.bytes.mid(position, iterator.key() - position));
            result.append(iterator.value().second);
            position = iterator.value().first;
        }
        result.append(document.bytes.mid(position, end - position));
        return result;
    }
};

/******************************************************************************
 * Function Name   : patchScalar
 * Description     : Consume one supported model property and update its native field only after an edit.
 * Input Parameters: before, after, key, field, updates
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void patchScalar(QJsonObject *before, QJsonObject *after, const QString &key,
                  const QString &field, SourceUpdates *updates) //20261109-0 add
{
    if (before->value(key) != after->value(key)) {
        updates->set(field, after->value(key).toVariant());
    }
    before->remove(key);
    after->remove(key);
}

/******************************************************************************
 * Function Name   : patchAddress
 * Description     : Consume one editable address while preserving unmodified native flags and spelling.
 * Input Parameters: before, after, key, field, updates
 * Return Value    : None.
 * Date            : 2026.11.09
 ******************************************************************************/
void patchAddress(QJsonObject *before, QJsonObject *after, const QString &key,
                   const QString &field, SourceUpdates *updates) //20261109-0 add
{
    if (before->value(key) != after->value(key)) {
        updates->address(field, after->value(key).toString());
    }
    before->remove(key);
    after->remove(key);
}

/******************************************************************************
 * Function Name   : patchCondition
 * Description     : Apply the recovered condition bitmask, address, level and numeric source mapping.
 * Input Parameters: before, after, prefix, touch, updates
 * Return Value    : True when all edited condition properties have a native representation.
 * Date            : 2026.11.09
 ******************************************************************************/
bool patchCondition(QJsonObject before, QJsonObject after, const QString &prefix,
                     bool touch, SourceUpdates *updates) //20261109-0 add
{
    const int nativeTypes[5] = {4, 5, 2, 3, 8};
    QString gate;
    QString numeric;
    QString numberType;
    QString key;
    QStringList flags;
    QJsonObject blankTexts;
    int bits;
    int i;
    bool activateBit;
    bool activateWord;
    bool activateLevel;

    gate = prefix + (touch ? QStringLiteral("common.touch") : QStringLiteral("common.visibility"));
    numeric = prefix + (touch ? QStringLiteral("common.78") : QStringLiteral("common.c8"));
    numberType = prefix + (touch ? QStringLiteral("common.a8.2") : QStringLiteral("common.78.3"));
    blankTexts.insert(QStringLiteral("中文"), QString());
    // Empty condition captions have no source field; both editor representations are equivalent.
    if ((before.value(QStringLiteral("texts")).toObject().isEmpty() || before.value(QStringLiteral("texts")).toObject() == blankTexts) &&
        (after.value(QStringLiteral("texts")).toObject().isEmpty() || after.value(QStringLiteral("texts")).toObject() == blankTexts)) {
        before.remove(QStringLiteral("texts"));
        after.remove(QStringLiteral("texts"));
    }
    activateBit = after.value(QStringLiteral("visibilityEnabled")).toBool() && after.value(QStringLiteral("bitEnabled")).toBool() &&
                  (!before.value(QStringLiteral("visibilityEnabled")).toBool() || !before.value(QStringLiteral("bitEnabled")).toBool());
    activateWord = after.value(QStringLiteral("visibilityEnabled")).toBool() && after.value(QStringLiteral("wordEnabled")).toBool() &&
                   (!before.value(QStringLiteral("visibilityEnabled")).toBool() || !before.value(QStringLiteral("wordEnabled")).toBool());
    activateLevel = after.value(QStringLiteral("visibilityEnabled")).toBool() && after.value(QStringLiteral("levelEnabled")).toBool() &&
                    (!before.value(QStringLiteral("visibilityEnabled")).toBool() || !before.value(QStringLiteral("levelEnabled")).toBool());
    if (activateBit) {
        updates->address(prefix + (touch ? QStringLiteral("common.40") : QStringLiteral("common.90")), after.value(QStringLiteral("bitAddress")).toString());
        updates->set(prefix + (touch ? QStringLiteral("common.58.0") : QStringLiteral("common.a8.0")), after.value(QStringLiteral("bitState")).toBool());
    }
    if (activateWord) {
        updates->address(prefix + (touch ? QStringLiteral("common.60") : QStringLiteral("common.b0")), after.value(QStringLiteral("wordAddress")).toString());
        updates->set(numberType, nativeTypes[after.value(QStringLiteral("dataType")).toInt()]);
        updates->set(numeric + QStringLiteral(".1"), after.value(QStringLiteral("compareValue")).toString());
        updates->set(numeric + QStringLiteral(".2"), after.value(QStringLiteral("tolerance")).toString());
        updates->set(numeric + QStringLiteral(".0"), after.value(QStringLiteral("comparison")).toInt());
    }
    if (activateLevel) {
        updates->set(prefix + (touch ? QStringLiteral("common.58.1") : QStringLiteral("common.a8.1")), after.value(QStringLiteral("minimumLevel")).toInt() - 1);
    }
    patchScalar(&before, &after, QStringLiteral("visibilityEnabled"), gate + QStringLiteral(".0"), updates);
    flags = QStringList{QStringLiteral("bitEnabled"), QStringLiteral("wordEnabled"), QStringLiteral("levelEnabled")};
    if (before.value(flags.at(0)) != after.value(flags.at(0)) ||
        before.value(flags.at(1)) != after.value(flags.at(1)) ||
        before.value(flags.at(2)) != after.value(flags.at(2))) {
        bits = (after.value(flags.at(0)).toBool() ? 4 : 0) |
               (after.value(flags.at(1)).toBool() ? 2 : 0) |
               (after.value(flags.at(2)).toBool() ? 1 : 0);
        updates->set(gate + QStringLiteral(".1"), bits);
    }
    for (i = 0; i < flags.size(); ++i) {
        before.remove(flags.at(i));
        after.remove(flags.at(i));
    }
    patchAddress(&before, &after, QStringLiteral("bitAddress"),
                 prefix + (touch ? QStringLiteral("common.40") : QStringLiteral("common.90")), updates);
    patchScalar(&before, &after, QStringLiteral("bitState"),
                prefix + (touch ? QStringLiteral("common.58.0") : QStringLiteral("common.a8.0")), updates);
    if (before.value(QStringLiteral("minimumLevel")) != after.value(QStringLiteral("minimumLevel"))) {
        updates->set(prefix + (touch ? QStringLiteral("common.58.1") : QStringLiteral("common.a8.1")),
                      after.value(QStringLiteral("minimumLevel")).toInt() - 1);
    }
    before.remove(QStringLiteral("minimumLevel"));
    after.remove(QStringLiteral("minimumLevel"));
    patchAddress(&before, &after, QStringLiteral("wordAddress"),
                 prefix + (touch ? QStringLiteral("common.60") : QStringLiteral("common.b0")), updates);
    if (before.value(QStringLiteral("dataType")) != after.value(QStringLiteral("dataType"))) {
        updates->set(numberType, nativeTypes[after.value(QStringLiteral("dataType")).toInt()]);
    }
    before.remove(QStringLiteral("dataType"));
    after.remove(QStringLiteral("dataType"));
    patchScalar(&before, &after, QStringLiteral("compareValue"), numeric + QStringLiteral(".1"), updates);
    patchScalar(&before, &after, QStringLiteral("tolerance"), numeric + QStringLiteral(".2"), updates);
    patchScalar(&before, &after, QStringLiteral("comparison"), numeric + QStringLiteral(".0"), updates);
    return before == after;
}

/******************************************************************************
 * Function Name   : patchState
 * Description     : Update verified native state text, font size, colors, image path and flash fields.
 * Input Parameters: before, after, prefix, state, languages, updates
 * Return Value    : True when no unimplemented visual property was changed.
 * Date            : 2026.11.09
 ******************************************************************************/
bool patchState(QJsonObject before, QJsonObject after, const QString &prefix,
                 int state, const QStringList &languages, SourceUpdates *updates) //20261109-0 add
{
    QJsonObject beforeText;
    QJsonObject afterText;
    QJsonObject beforeLanguages;
    QJsonObject afterLanguages;
    QString row;
    QString picture;
    QString text;
    QString path;
    QString list;
    QString languageName;
    QByteArray languageBytes;
    SourceUpdates rowUpdates(updates->document);
    QStringList sourceLanguages;
    QStringList keys;
    qsizetype listStart;
    qsizetype listEnd;
    int language;
    int sourceIndex;
    int i;
    bool textChanged;
    bool lamp; //20261109-8 add
    bool singleStateFunction; //20261110-0 add
    bool sharedLanguage;
    bool previewState;

    lamp = updates->document.value(prefix + QStringLiteral("switch.mode")).toInt() == 4;
    singleStateFunction = updates->document.value(prefix + QStringLiteral("switch.mode")).toInt() == 3;
    sharedLanguage = (lamp || singleStateFunction) && after.value(QStringLiteral("sameLanguage")).toBool(); //20261110-0 change
    previewState = lamp ? updates->document.value(prefix + QStringLiteral("switch.274.1")).toInt() == state : state == 0;
    row = SourceDocument::indexed(SourceDocument::indexed(prefix + QStringLiteral("switch.290"), state), 0);
    picture = SourceDocument::indexed(prefix + QStringLiteral("switch.294"), state);
    beforeText = before.value(QStringLiteral("text")).toObject();
    afterText = after.value(QStringLiteral("text")).toObject();
    beforeLanguages = beforeText.value(QStringLiteral("texts")).toObject();
    afterLanguages = afterText.value(QStringLiteral("texts")).toObject();
    keys = afterLanguages.keys();
    for (i = 0; i < keys.size(); ++i) {
        if (!languages.contains(keys.at(i))) {
            return false;
        }
    }
    for (i = 0; i < updates->document.value(QStringLiteral("system.164.count")).toInt(); ++i) {
        sourceLanguages.append(updates->document.value(SourceDocument::indexed(QStringLiteral("system.164"), i)).toString());
    }
    textChanged = beforeLanguages != afterLanguages || before.value(QStringLiteral("sharedText")) != after.value(QStringLiteral("sharedText")) ||
                  sourceLanguages != languages || ((lamp || singleStateFunction) && before.value(QStringLiteral("sameLanguage")) != after.value(QStringLiteral("sameLanguage"))); //20261110-0 change
    if (textChanged || beforeText.value(QStringLiteral("fontFamily")) != afterText.value(QStringLiteral("fontFamily")) ||
        beforeText.value(QStringLiteral("fontSize")) != afterText.value(QStringLiteral("fontSize")) ||
        beforeText.value(QStringLiteral("textColor")) != afterText.value(QStringLiteral("textColor"))) {
        list = SourceDocument::indexed(prefix + QStringLiteral("switch.290"), state);
        listStart = updates->document.values.value(list + QStringLiteral(".count")).start;
        listEnd = updates->document.values.value(SourceDocument::indexed(list, sourceLanguages.size() - 1) + QStringLiteral(".4")).end;
        appendWord(&languageBytes, static_cast<u32>(languages.size()));
        for (language = 0; language < languages.size(); ++language) {
            languageName = languages.at(language);
            sourceIndex = sourceLanguages.indexOf(languageName);
            row = SourceDocument::indexed(list, sourceIndex >= 0 ? sourceIndex : 0);
            text = sharedLanguage ? after.value(QStringLiteral("sharedText")).toString() : afterLanguages.value(languageName).toString(); //20261109-8 change
            if (language == 0 && beforeLanguages == afterLanguages && before.value(QStringLiteral("sharedText")) != after.value(QStringLiteral("sharedText"))) {
                text = after.value(QStringLiteral("sharedText")).toString();
            }
            rowUpdates.replacements.clear();
            if (text != updates->document.value(row + QStringLiteral(".4")).toString()) {
                rowUpdates.set(row + QStringLiteral(".4"), text);
            }
            if (beforeText.value(QStringLiteral("fontFamily")) != afterText.value(QStringLiteral("fontFamily"))) {
                rowUpdates.set(row + QStringLiteral(".1"), afterText.value(QStringLiteral("fontFamily")).toString());
            }
            if (beforeText.value(QStringLiteral("fontSize")) != afterText.value(QStringLiteral("fontSize"))) {
                rowUpdates.set(row + QStringLiteral(".3"), afterText.value(QStringLiteral("fontSize")).toInt());
            }
            if (beforeText.value(QStringLiteral("textColor")) != afterText.value(QStringLiteral("textColor"))) {
                rowUpdates.set(row + QStringLiteral(".0"), QColor(afterText.value(QStringLiteral("textColor")).toString()));
            }
            languageBytes.append(rowUpdates.render(updates->document.values.value(row + QStringLiteral(".0")).start,
                                                    updates->document.values.value(row + QStringLiteral(".4")).end));
            if (!rowUpdates.error.isEmpty()) {
                updates->error = rowUpdates.error;
                return false;
            }
        }
        updates->replacements.insert(listStart, qMakePair(listEnd, languageBytes));
        if (previewState) {
            text = sharedLanguage ? after.value(QStringLiteral("sharedText")).toString() : afterLanguages.value(languages.first()).toString(); //20261109-8 change
            if (beforeLanguages == afterLanguages && before.value(QStringLiteral("sharedText")) != after.value(QStringLiteral("sharedText"))) {
                text = after.value(QStringLiteral("sharedText")).toString();
            }
            updates->set(prefix + QStringLiteral("text.head.4"), text);
        }
    }
    beforeText.remove(QStringLiteral("texts"));
    afterText.remove(QStringLiteral("texts"));
    before.remove(QStringLiteral("sharedText"));
    after.remove(QStringLiteral("sharedText"));
    if (lamp || singleStateFunction) { //20261110-0 change: These controls use the verified global shared-language flag and the image/no-image selector.
        before.remove(QStringLiteral("sameLanguage"));
        after.remove(QStringLiteral("sameLanguage"));
        if (before.value(QStringLiteral("useImage")) != after.value(QStringLiteral("useImage"))) {
            updates->set(picture + QStringLiteral(".0"), after.value(QStringLiteral("useImage")).toBool() ? 1 : 3);
        }
        before.remove(QStringLiteral("useImage"));
        after.remove(QStringLiteral("useImage"));
    }
    if (beforeText.value(QStringLiteral("fontSize")) != afterText.value(QStringLiteral("fontSize")) &&
        std::floor(afterText.value(QStringLiteral("fontSize")).toDouble()) != afterText.value(QStringLiteral("fontSize")).toDouble()) {
        return false;
    }
    if (previewState && beforeText.value(QStringLiteral("fontSize")) != afterText.value(QStringLiteral("fontSize"))) {
        updates->set(prefix + QStringLiteral("font.tail.1"), afterText.value(QStringLiteral("fontSize")).toInt());
    }
    if (previewState && beforeText.value(QStringLiteral("fontFamily")) != afterText.value(QStringLiteral("fontFamily"))) {
        updates->set(prefix + QStringLiteral("font.family"), afterText.value(QStringLiteral("fontFamily")).toString());
    }
    beforeText.remove(QStringLiteral("fontFamily")); afterText.remove(QStringLiteral("fontFamily"));
    beforeText.remove(QStringLiteral("fontSize")); afterText.remove(QStringLiteral("fontSize"));
    if (beforeText.value(QStringLiteral("textColor")) != afterText.value(QStringLiteral("textColor"))) {
        if (previewState) {
            updates->set(prefix + QStringLiteral("text.tail.0"), QColor(afterText.value(QStringLiteral("textColor")).toString()));
        }
    }
    beforeText.remove(QStringLiteral("textColor"));
    afterText.remove(QStringLiteral("textColor"));
    if (beforeText != afterText) {
        return false;
    }
    before.remove(QStringLiteral("text"));
    after.remove(QStringLiteral("text"));
    patchScalar(&before, &after, QStringLiteral("flash"), picture + QStringLiteral(".2"), updates);
    patchScalar(&before, &after, QStringLiteral("opacity"), picture + QStringLiteral(".3"), updates);
    if (before.value(QStringLiteral("background")) != after.value(QStringLiteral("background"))) {
        updates->set(picture + QStringLiteral(".4"), QColor(after.value(QStringLiteral("background")).toString()));
    }
    before.remove(QStringLiteral("background"));
    after.remove(QStringLiteral("background"));
    if (before.value(QStringLiteral("picture")) != after.value(QStringLiteral("picture"))) {
        path = after.value(QStringLiteral("picture")).toString();
        if (!path.startsWith(QStringLiteral(":/btn/images/"))) {
            if (path.contains(QLatin1Char('/')) || path.contains(QLatin1Char('\\')) || !path.endsWith(QStringLiteral(".kvg"))) {
                return false;
            }
            path.prepend(QStringLiteral(":/btn/images/"));
        }
        updates->set(picture + QStringLiteral(".1"), path);
        if (previewState && updates->document.values.contains(prefix + QStringLiteral("pixmap.0"))) { //20261109-8 change
            updates->set(prefix + QStringLiteral("pixmap.0"), path);
        }
    }
    before.remove(QStringLiteral("picture"));
    after.remove(QStringLiteral("picture"));
    before.remove(QStringLiteral("effect3d"));
    after.remove(QStringLiteral("effect3d"));
    before.remove(QStringLiteral("transparent"));
    after.remove(QStringLiteral("transparent"));
    return before == after;
}

/******************************************************************************
 * Function Name   : patchStaticText
 * Description     : Update native static titles, verified typography, colors, geometry and visibility conditions.
 * Input Parameters: original, current, prefix, languages, updates
 * Return Value    : True when every model difference has a supported native representation.
 * Date            : 2026.11.09
 ******************************************************************************/
bool patchStaticText(const HmiObject &original, const HmiObject &current, const QString &prefix, const QStringList &languages,
                       SourceUpdates *updates) //20261109-0 add
{
    StaticTextProperties properties;
    QJsonObject before;
    QJsonObject after;
    QJsonObject condition;
    QJsonObject beforeObject;
    QJsonObject afterObject;
    QStringList conditionKeys;
    QStringList colorKeys;
    QStringList fontKeys;
    QString key;
    QString row;
    QString text;
    QByteArray fontBytes;
    QByteArray languageBytes;
    QDataStream fontStream(&fontBytes, QIODevice::WriteOnly);
    QFont font;
    bool fontChanged;
    int i;
    int flags;
    int language;
    int sourceIndex;
    int field;
    QStringList sourceLanguages;
    QStringList keys;
    QString languageRow;
    QString firstRow;
    QString fieldName;
    QString firstFieldName;
    QString primaryText;
    SourceUpdates rowUpdates(updates->document);
    SourceField sourceField;
    qsizetype listStart;
    qsizetype listEnd;
    QMap<qsizetype, QPair<qsizetype, QByteArray>>::iterator replacement;
    const int languageFields[13] = {0, 1, 5, 6, 7, 9, 10, 12, 13, 14, 15, 16, 3};
    qsizetype fontStart;
    qsizetype fontEnd;

    properties = current.staticText;
    if (!properties.validate(&updates->error) || !current.hasStaticTextProperties ||
        std::floor(properties.fontSize) != properties.fontSize ||
        std::floor(properties.spacing) != properties.spacing ||
        properties.fontFamily != original.staticText.fontFamily) {
        return false;
    }
    keys = properties.languageTexts.keys();
    for (i = 0; i < keys.size(); ++i) {
        if (!languages.contains(keys.at(i))) {
            return false;
        }
    }
    before = original.staticText.toJson();
    after = properties.toJson();
    condition = before;
    conditionKeys = QStringList{QStringLiteral("visibilityEnabled"), QStringLiteral("bitEnabled"), QStringLiteral("bitAddress"),
                                QStringLiteral("bitState"), QStringLiteral("levelEnabled"), QStringLiteral("minimumLevel"),
                                QStringLiteral("wordEnabled"), QStringLiteral("wordAddress"), QStringLiteral("dataType"),
                                QStringLiteral("comparison"), QStringLiteral("compareValue"), QStringLiteral("tolerance")};
    for (i = 0; i < conditionKeys.size(); ++i) {
        condition.insert(conditionKeys.at(i), after.value(conditionKeys.at(i)));
    }
    if (!patchCondition(before, condition, prefix, false, updates)) {
        return false;
    }
    for (i = 0; i < conditionKeys.size(); ++i) {
        before.remove(conditionKeys.at(i));
        after.remove(conditionKeys.at(i));
    }
    row = SourceDocument::indexed(prefix + QStringLiteral("static.194"), 0);
    text = properties.languageTexts.value(languages.first());
    if (text == original.staticText.languageTexts.value(languages.first()) && current.text != original.text) {
        text = current.text;
    } else if (current.text != original.text && current.text != text) {
        return false;
    }
    primaryText = text;
    if (text != original.text) {
        updates->set(row + QStringLiteral(".10"), text);
        updates->set(prefix + QStringLiteral("static.head.5"), text);
        updates->set(prefix + QStringLiteral("text.head.4"), text);
    }
    before.remove(QStringLiteral("texts"));
    after.remove(QStringLiteral("texts"));
    fontKeys = QStringList{QStringLiteral("fontSize"), QStringLiteral("spacing"), QStringLiteral("bold"),
                           QStringLiteral("italic"), QStringLiteral("underline")};
    fontChanged = false;
    for (i = 0; i < fontKeys.size(); ++i) {
        fontChanged = fontChanged || before.value(fontKeys.at(i)) != after.value(fontKeys.at(i));
    }
    if (fontChanged) {
        font.setFamily(properties.fontFamily);
        font.setPixelSize(qRound(properties.fontSize));
        font.setBold(properties.bold);
        font.setItalic(properties.italic);
        font.setUnderline(properties.underline);
        font.setLetterSpacing(QFont::AbsoluteSpacing, properties.spacing);
        fontStream.setVersion(QDataStream::Qt_4_8);
        fontStream << font;
        fontStart = updates->document.values.value(prefix + QStringLiteral("font.family")).start;
        fontEnd = updates->document.values.value(prefix + QStringLiteral("font.tail.10")).end;
        updates->replacements.insert(fontStart, qMakePair(fontEnd, fontBytes));
        updates->set(row + QStringLiteral(".5"), qRound(properties.fontSize));
        updates->set(prefix + QStringLiteral("static.head.7"), qRound(properties.fontSize));
        updates->set(row + QStringLiteral(".16"), qRound(properties.spacing));
        updates->set(prefix + QStringLiteral("static.head.24"), qRound(properties.spacing));
        updates->set(row + QStringLiteral(".1"), properties.bold);
        updates->set(row + QStringLiteral(".6"), properties.italic);
        updates->set(row + QStringLiteral(".9"), properties.underline);
        updates->set(prefix + QStringLiteral("static.head.14"), properties.bold);
        updates->set(prefix + QStringLiteral("static.head.15"), properties.italic);
        updates->set(prefix + QStringLiteral("static.head.16"), properties.underline);
        flags = (properties.bold ? 1 : 0) | (properties.italic ? 2 : 0) | (properties.underline ? 8 : 0);
        updates->set(prefix + QStringLiteral("static.head.3"), flags);
    }
    for (i = 0; i < fontKeys.size(); ++i) {
        before.remove(fontKeys.at(i));
        after.remove(fontKeys.at(i));
    }
    patchScalar(&before, &after, QStringLiteral("adaptive"), row + QStringLiteral(".0"), updates);
    if (properties.adaptive != original.staticText.adaptive) {
        updates->set(prefix + QStringLiteral("static.head.0"), properties.adaptive);
    }
    patchScalar(&before, &after, QStringLiteral("alignment"), prefix + QStringLiteral("static.head.2"), updates);
    if (properties.alignment != original.staticText.alignment) {
        updates->set(prefix + QStringLiteral("text.tail.4"), properties.alignment);
    }
    colorKeys = QStringList{QStringLiteral("textColor"), QStringLiteral("backgroundColor"), QStringLiteral("foregroundColor")};
    for (i = 0; i < colorKeys.size(); ++i) {
        key = colorKeys.at(i);
        if (before.value(key) != after.value(key)) {
            if (QColor(after.value(key).toString()).alpha() != 255) {
                return false;
            }
            updates->set(row + (i == 0 ? QStringLiteral(".7") : i == 1 ? QStringLiteral(".12") : QStringLiteral(".13")),
                           QColor(after.value(key).toString()).name(QColor::HexRgb));
            updates->set(prefix + QStringLiteral("text.tail.") + QString::number(i), QColor(after.value(key).toString()));
            if (i == 0) {
                updates->set(prefix + QStringLiteral("static.head.8"), QColor(after.value(key).toString()).name(QColor::HexRgb));
            } else {
                updates->set(prefix + QStringLiteral("rectangle.") + QString::number(i == 1 ? 6 : 7), QColor(after.value(key).toString()));
                updates->set(prefix + (i == 1 ? QStringLiteral("static.head.12") : QStringLiteral("static.head.11")),
                               QStringLiteral("%1+%2+%3").arg(QColor(after.value(key).toString()).red())
                                   .arg(QColor(after.value(key).toString()).green()).arg(QColor(after.value(key).toString()).blue()));
            }
        }
        before.remove(key);
        after.remove(key);
    }
    patchScalar(&before, &after, QStringLiteral("style"), row + QStringLiteral(".15"), updates);
    if (properties.style != original.staticText.style) {
        updates->set(prefix + QStringLiteral("static.head.10"), properties.style);
        updates->set(prefix + QStringLiteral("text.tail.3"), properties.style);
        updates->set(prefix + QStringLiteral("rectangle.8"), properties.style);
    }
    patchScalar(&before, &after, QStringLiteral("opacity"), row + QStringLiteral(".14"), updates);
    if (properties.opacity != original.staticText.opacity) {
        updates->set(prefix + QStringLiteral("static.head.4"), properties.opacity);
        updates->set(prefix + QStringLiteral("text.tail.7"), properties.opacity);
    }
    if (before != after) {
        return false;
    }
    for (i = 0; i < updates->document.value(QStringLiteral("system.164.count")).toInt(); ++i) {
        sourceLanguages.append(updates->document.value(SourceDocument::indexed(QStringLiteral("system.164"), i)).toString());
    }
    firstRow = SourceDocument::indexed(prefix + QStringLiteral("static.194"), 0);
    listStart = updates->document.values.value(prefix + QStringLiteral("static.194.count")).start;
    listEnd = updates->document.values.value(SourceDocument::indexed(prefix + QStringLiteral("static.194"), sourceLanguages.size() - 1) + QStringLiteral(".16")).end;
    appendWord(&languageBytes, static_cast<u32>(languages.size()));
    for (language = 0; language < languages.size(); ++language) {
        sourceIndex = sourceLanguages.indexOf(languages.at(language));
        languageRow = SourceDocument::indexed(prefix + QStringLiteral("static.194"), sourceIndex >= 0 ? sourceIndex : 0);
        rowUpdates.replacements = updates->replacements;
        for (field = 0; field < 13; ++field) {
            firstFieldName = firstRow + QLatin1Char('.') + QString::number(languageFields[field]);
            fieldName = languageRow + QLatin1Char('.') + QString::number(languageFields[field]);
            sourceField = updates->document.values.value(firstFieldName);
            if (languageFields[field] != 10 && updates->replacements.contains(sourceField.start)) {
                sourceField = updates->document.values.value(fieldName);
                rowUpdates.replacements.insert(sourceField.start, qMakePair(sourceField.end,
                    updates->replacements.value(updates->document.values.value(firstFieldName).start).second));
            }
        }
        text = language == 0 ? primaryText : properties.languageTexts.value(languages.at(language));
        if (text != updates->document.value(languageRow + QStringLiteral(".10")).toString()) {
            rowUpdates.set(languageRow + QStringLiteral(".10"), text);
        }
        languageBytes.append(rowUpdates.render(updates->document.values.value(languageRow + QStringLiteral(".0")).start,
                                                updates->document.values.value(languageRow + QStringLiteral(".16")).end));
        if (!rowUpdates.error.isEmpty()) {
            updates->error = rowUpdates.error;
            return false;
        }
    }
    replacement = updates->replacements.lowerBound(listStart);
    while (replacement != updates->replacements.end() && replacement.key() < listEnd) {
        replacement = updates->replacements.erase(replacement);
    }
    updates->replacements.insert(listStart, qMakePair(listEnd, languageBytes));
    beforeObject = original.toJson();
    afterObject = current.toJson();
    beforeObject.remove(QStringLiteral("staticText"));
    afterObject.remove(QStringLiteral("staticText"));
    beforeObject.remove(QStringLiteral("text"));
    afterObject.remove(QStringLiteral("text"));
    patchScalar(&beforeObject, &afterObject, QStringLiteral("name"), prefix + QStringLiteral("group.metadata-name"), updates);
    patchScalar(&beforeObject, &afterObject, QStringLiteral("layer"), prefix + QStringLiteral("z"), updates);
    if (current.geometry != original.geometry) {
        if (current.geometry.width() < 1 || current.geometry.height() < 1 ||
            current.geometry.width() > 4096 || current.geometry.height() > 4096) {
            return false;
        }
        updates->set(prefix + QStringLiteral("rectangle.0"), 0.0);
        updates->set(prefix + QStringLiteral("rectangle.1"), 0.0);
        updates->set(prefix + QStringLiteral("rectangle.2"), current.geometry.width());
        updates->set(prefix + QStringLiteral("rectangle.3"), current.geometry.height());
        updates->set(prefix + QStringLiteral("rectangle.9"), current.geometry.x());
        updates->set(prefix + QStringLiteral("rectangle.10"), current.geometry.y());
        updates->set(prefix + QStringLiteral("text.head.0"), 0.0);
        updates->set(prefix + QStringLiteral("text.head.1"), 0.0);
        updates->set(prefix + QStringLiteral("text.head.2"), current.geometry.width());
        updates->set(prefix + QStringLiteral("text.head.3"), current.geometry.height());
        updates->set(prefix + QStringLiteral("text.tail.10"), current.geometry.x());
        updates->set(prefix + QStringLiteral("text.tail.11"), current.geometry.y());
    }
    beforeObject.remove(QStringLiteral("x")); afterObject.remove(QStringLiteral("x"));
    beforeObject.remove(QStringLiteral("y")); afterObject.remove(QStringLiteral("y"));
    beforeObject.remove(QStringLiteral("width")); afterObject.remove(QStringLiteral("width"));
    beforeObject.remove(QStringLiteral("height")); afterObject.remove(QStringLiteral("height"));
    return beforeObject == afterObject;
}

/******************************************************************************
 * Function Name   : patchButton
 * Description     : Apply supported bit-button edits and reject every unconsumed model difference.
 * Input Parameters: original, current, prefix, languages, updates
 * Return Value    : True for a fully represented native source update.
 * Date            : 2026.11.09
 ******************************************************************************/
bool patchButton(const HmiObject &original, const HmiObject &current, const QString &prefix, const QStringList &languages,
                   SourceUpdates *updates) //20261109-0 add
{
    const int nativeTypes[5] = {4, 5, 2, 3, 8};
    QJsonObject before;
    QJsonObject after;
    QJsonObject beforeObject;
    QJsonObject afterObject;
    BitButtonProperties oldProperties;
    BitButtonProperties properties;
    QString currentMonitor;
    QString oldMonitor;
    int flags;
    int stateCount;
    int state;
    bool effect3d;
    bool transparent;

    oldProperties = original.bitButton;
    properties = current.bitButton;
    before = oldProperties.toJson();
    after = properties.toJson();
    if (properties.macroEnabled != oldProperties.macroEnabled) {
        updates->set(prefix + QStringLiteral("switch.22c.1"), properties.macroEnabled);
    }
    if (properties.macroName != oldProperties.macroName || (properties.macroEnabled && !oldProperties.macroEnabled)) {
        updates->set(prefix + QStringLiteral("switch.22c.0"), 1);
        updates->set(prefix + QStringLiteral("switch.22c.2"), properties.macroName);
        updates->set(prefix + QStringLiteral("switch.22c.3"), properties.macroName.isEmpty() ? QString() : QStringLiteral("MACRO_MAIN"));
        updates->set(prefix + QStringLiteral("switch.22c.4"), 1);
    }
    before.remove(QStringLiteral("macroEnabled")); after.remove(QStringLiteral("macroEnabled"));
    before.remove(QStringLiteral("macroName")); after.remove(QStringLiteral("macroName"));
    if (properties.operation != oldProperties.operation) {
        updates->set(prefix + QStringLiteral("switch.144.0"), properties.operation);
        updates->set(prefix + QStringLiteral("switch.144.1"), properties.operation == 0 ? 255 : 0);
        updates->set(prefix + QStringLiteral("switch.144.2"), properties.operation == 0 ? 255 : 0);
        updates->set(prefix + QStringLiteral("switch.144.3"), properties.operation == 0 ? -1 : 0);
    }
    before.remove(QStringLiteral("operation"));
    after.remove(QStringLiteral("operation"));
    if (properties.indicator != oldProperties.indicator || properties.dualMonitor != oldProperties.dualMonitor) {
        stateCount = updates->document.value(prefix + QStringLiteral("switch.290.count")).toInt();
        if ((properties.indicator ? 2 : 1) != stateCount) {
            return false;
        }
        updates->set(prefix + QStringLiteral("switch.248.5"), properties.indicator ? (properties.dualMonitor ? 1 : 2) : 0);
    }
    before.remove(QStringLiteral("indicator"));
    after.remove(QStringLiteral("indicator"));
    before.remove(QStringLiteral("dualMonitor"));
    after.remove(QStringLiteral("dualMonitor"));
    patchScalar(&before, &after, QStringLiteral("monitorSame"), prefix + QStringLiteral("switch.248.2"), updates);
    patchScalar(&before, &after, QStringLiteral("monitorType"), prefix + QStringLiteral("switch.248.0"), updates);
    patchScalar(&before, &after, QStringLiteral("bitIndex"), prefix + QStringLiteral("switch.248.1"), updates);
    currentMonitor = properties.monitorType == 0 ? properties.monitorBitAddress : properties.monitorWordAddress;
    oldMonitor = oldProperties.monitorType == 0 ? oldProperties.monitorBitAddress : oldProperties.monitorWordAddress;
    if (currentMonitor != oldMonitor || properties.monitorType != oldProperties.monitorType) {
        updates->address(prefix + QStringLiteral("switch.254"), currentMonitor);
    }
    if (properties.monitorType == 0 && properties.monitorWordAddress != oldProperties.monitorWordAddress) {
        return false;
    }
    if (properties.monitorType == 1 && properties.monitorBitAddress != oldProperties.monitorBitAddress) {
        return false;
    }
    before.remove(QStringLiteral("monitorBitAddress"));
    after.remove(QStringLiteral("monitorBitAddress"));
    before.remove(QStringLiteral("monitorWordAddress"));
    after.remove(QStringLiteral("monitorWordAddress"));
    patchScalar(&before, &after, QStringLiteral("notice"), prefix + QStringLiteral("common.58.2"), updates);
    if (properties.notice && properties.noticeBit && (!oldProperties.notice || !oldProperties.noticeBit)) {
        updates->address(prefix + QStringLiteral("common.e4"), properties.noticeBitAddress);
        updates->set(prefix + QStringLiteral("common.58.3"), properties.noticeBitValue);
    }
    if (properties.notice && properties.noticeWord && (!oldProperties.notice || !oldProperties.noticeWord)) {
        updates->address(prefix + QStringLiteral("common.fc"), properties.noticeWordAddress);
        updates->set(prefix + QStringLiteral("common.c8.4"), nativeTypes[properties.noticeWordType]);
        updates->set(prefix + QStringLiteral("common.118.0"), properties.noticeWordValue);
    }
    if (properties.noticeBit != oldProperties.noticeBit || properties.noticeWord != oldProperties.noticeWord) {
        flags = (properties.noticeBit ? 1 : 0) | (properties.noticeWord ? 2 : 0);
        updates->set(prefix + QStringLiteral("common.c8.3"), flags);
    }
    before.remove(QStringLiteral("noticeBit"));
    after.remove(QStringLiteral("noticeBit"));
    before.remove(QStringLiteral("noticeWord"));
    after.remove(QStringLiteral("noticeWord"));
    patchAddress(&before, &after, QStringLiteral("noticeBitAddress"), prefix + QStringLiteral("common.e4"), updates);
    patchAddress(&before, &after, QStringLiteral("noticeWordAddress"), prefix + QStringLiteral("common.fc"), updates);
    patchScalar(&before, &after, QStringLiteral("noticeBitValue"), prefix + QStringLiteral("common.58.3"), updates);
    if (properties.noticeWordType != oldProperties.noticeWordType) {
        updates->set(prefix + QStringLiteral("common.c8.4"), nativeTypes[properties.noticeWordType]);
    }
    before.remove(QStringLiteral("noticeWordType"));
    after.remove(QStringLiteral("noticeWordType"));
    patchScalar(&before, &after, QStringLiteral("noticeWordValue"), prefix + QStringLiteral("common.118.0"), updates);
    patchScalar(&before, &after, QStringLiteral("pressTimeEnabled"), prefix + QStringLiteral("common.118.1"), updates);
    patchScalar(&before, &after, QStringLiteral("pressTime"), prefix + QStringLiteral("common.118.2"), updates);
    if (!patchCondition(before.value(QStringLiteral("touch")).toObject(), after.value(QStringLiteral("touch")).toObject(), prefix, true, updates) ||
        !patchCondition(before.value(QStringLiteral("visibility")).toObject(), after.value(QStringLiteral("visibility")).toObject(), prefix, false, updates)) {
        return false;
    }
    before.remove(QStringLiteral("touch"));
    after.remove(QStringLiteral("touch"));
    before.remove(QStringLiteral("visibility"));
    after.remove(QStringLiteral("visibility"));
    effect3d = properties.states[0].effect3d;
    transparent = properties.states[0].transparent;
    if (properties.states[1].effect3d != effect3d || properties.states[1].transparent != transparent) {
        return false;
    }
    if (effect3d != oldProperties.states[0].effect3d) {
        updates->set(prefix + QStringLiteral("switch.tail.0"), effect3d);
    }
    if (transparent != oldProperties.states[0].transparent) {
        updates->set(prefix + QStringLiteral("switch.tail.7"), transparent);
    }
    stateCount = updates->document.value(prefix + QStringLiteral("switch.290.count")).toInt();
    if (updates->document.value(prefix + QStringLiteral("switch.mode")).toInt() == 4) { //20261109-8 add: Additional lamp states are patched by their independent property record.
        stateCount = qMin(stateCount, 2);
    }
    for (state = 0; state < stateCount; ++state) {
        if (!patchState(oldProperties.states[state].toJson(), properties.states[state].toJson(), prefix, state, languages, updates)) {
            return false;
        }
    }
    before.remove(QStringLiteral("states"));
    after.remove(QStringLiteral("states"));
    if (before != after) {
        return false;
    }
    beforeObject = original.toJson();
    afterObject = current.toJson();
    beforeObject.remove(QStringLiteral("bitButton"));
    afterObject.remove(QStringLiteral("bitButton"));
    patchAddress(&beforeObject, &afterObject, QStringLiteral("address"), prefix + QStringLiteral("switch.12c"), updates);
    if (properties.monitorSame && current.address != original.address) {
        updates->address(prefix + QStringLiteral("switch.254"), current.address);
    }
    patchScalar(&beforeObject, &afterObject, QStringLiteral("name"), prefix + QStringLiteral("group.metadata-name"), updates);
    patchScalar(&beforeObject, &afterObject, QStringLiteral("layer"), prefix + QStringLiteral("z"), updates);
    if (current.geometry != original.geometry) {
        if (current.geometry.width() < 1 || current.geometry.height() < 1 ||
            current.geometry.width() > 4096 || current.geometry.height() > 4096) {
            return false;
        }
        updates->set(prefix + QStringLiteral("rectangle.9"), current.geometry.x());
        updates->set(prefix + QStringLiteral("rectangle.10"), current.geometry.y());
        updates->set(prefix + QStringLiteral("rectangle.0"), 0.0);
        updates->set(prefix + QStringLiteral("rectangle.1"), 0.0);
        updates->set(prefix + QStringLiteral("rectangle.2"), current.geometry.width());
        updates->set(prefix + QStringLiteral("rectangle.3"), current.geometry.height());
        if (updates->document.value(prefix + QStringLiteral("pixmap.type")).toInt() == 13) { //20261109-8 change
            updates->set(prefix + QStringLiteral("pixmap.2"), current.geometry.x());
            updates->set(prefix + QStringLiteral("pixmap.3"), current.geometry.y());
            updates->set(prefix + QStringLiteral("pixmap.4"), current.geometry.width());
            updates->set(prefix + QStringLiteral("pixmap.5"), current.geometry.height());
            updates->set(prefix + QStringLiteral("pixmap.6"), current.geometry.x());
            updates->set(prefix + QStringLiteral("pixmap.7"), current.geometry.y());
        } else {
            updates->set(prefix + QStringLiteral("picture-rectangle.2"), current.geometry.width());
            updates->set(prefix + QStringLiteral("picture-rectangle.3"), current.geometry.height());
            updates->set(prefix + QStringLiteral("picture-rectangle.9"), current.geometry.x());
            updates->set(prefix + QStringLiteral("picture-rectangle.10"), current.geometry.y());
        }
        updates->set(prefix + QStringLiteral("text.head.2"), current.geometry.width());
        updates->set(prefix + QStringLiteral("text.head.3"), current.geometry.height());
        updates->set(prefix + QStringLiteral("text.head.0"), 0.0);
        updates->set(prefix + QStringLiteral("text.head.1"), 0.0);
        updates->set(prefix + QStringLiteral("text.tail.10"), current.geometry.x());
        updates->set(prefix + QStringLiteral("text.tail.11"), current.geometry.y());
        if (updates->document.values.contains(prefix + QStringLiteral("group.bounds.0"))) {
            updates->set(prefix + QStringLiteral("group.bounds.0"), current.geometry.x());
            updates->set(prefix + QStringLiteral("group.bounds.1"), current.geometry.y());
            updates->set(prefix + QStringLiteral("group.bounds.2"), current.geometry.right());
            updates->set(prefix + QStringLiteral("group.bounds.3"), current.geometry.bottom());
        }
    }
    beforeObject.remove(QStringLiteral("x")); afterObject.remove(QStringLiteral("x"));
    beforeObject.remove(QStringLiteral("y")); afterObject.remove(QStringLiteral("y"));
    beforeObject.remove(QStringLiteral("width")); afterObject.remove(QStringLiteral("width"));
    beforeObject.remove(QStringLiteral("height")); afterObject.remove(QStringLiteral("height"));
    if (current.text != original.text && current.text != properties.states[0].text.languageTexts.value(languages.first())) {
        return false;
    }
    beforeObject.remove(QStringLiteral("text"));
    afterObject.remove(QStringLiteral("text"));
    return beforeObject == afterObject;
}

/******************************************************************************
 * Function Name   : patchWordButton
 * Description     : Save typed word operations and reuse the verified shared button appearance and access updates.
 * Input Parameters: original, current, prefix, languages, updates
 * Return Value    : True when every changed word and shared property is represented.
 * Date            : 2026.11.09
 ******************************************************************************/
bool patchWordButton(const HmiObject &original, const HmiObject &current, const QString &prefix,
                       const QStringList &languages, SourceUpdates *updates) //20261109-6 add
{
    const QStringList hiddenBitFields = {QStringLiteral("operation"), QStringLiteral("indicator"), QStringLiteral("dualMonitor"),
                                        QStringLiteral("monitorSame"), QStringLiteral("monitorType"), QStringLiteral("monitorBitAddress"),
                                        QStringLiteral("monitorWordAddress"), QStringLiteral("bitIndex")};
    const QStringList numericFields = {QStringLiteral("constantValue"), QStringLiteral("minimumValue"), QStringLiteral("maximumValue")};
    const QStringList nativeFields = {QStringLiteral("word.16c.2"), QStringLiteral("word.16c.4"), QStringLiteral("word.16c.5")};
    HmiObject beforeShared;
    HmiObject afterShared;
    QJsonObject before;
    QJsonObject after;
    QJsonObject beforeBit;
    QJsonObject afterBit;
    QString field;
    double value;
    bool valid;
    bool newOperation;
    bool active;
    int i;

    if (!original.hasWordButtonProperties || !current.hasWordButtonProperties || !current.wordButton.validate(nullptr)) {
        return false;
    }
    beforeBit = original.bitButton.toJson();
    afterBit = current.bitButton.toJson();
    for (i = 0; i < hiddenBitFields.size(); ++i) {
        field = hiddenBitFields.at(i);
        if (beforeBit.value(field) != afterBit.value(field)) {
            return false;
        }
    }
    before = original.wordButton.toJson();
    after = current.wordButton.toJson();
    newOperation = original.wordButton.operation != current.wordButton.operation || original.wordButton.dataType != current.wordButton.dataType;
    if (original.wordButton.dataType != current.wordButton.dataType) {
        updates->set(prefix + QStringLiteral("word.16c.0"), current.wordButton.dataType);
    }
    before.remove(QStringLiteral("dataType"));
    after.remove(QStringLiteral("dataType"));
    patchScalar(&before, &after, QStringLiteral("operation"), prefix + QStringLiteral("word.16c.1"), updates);
    for (i = 0; i < numericFields.size(); ++i) {
        field = numericFields.at(i);
        active = i == 0 ? current.wordButton.usesConstant() : i == 1 ? current.wordButton.usesMinimum() : current.wordButton.usesMaximum();
        if (before.value(field) != after.value(field) || (newOperation && active)) { // Restore newly activated fields even when their display defaults match the old draft.
            value = after.value(field).toString().toDouble(&valid);
            if (!valid || !std::isfinite(value)) {
                return false;
            }
            updates->set(prefix + nativeFields.at(i), value);
        }
        before.remove(field);
        after.remove(field);
    }
    patchScalar(&before, &after, QStringLiteral("totalDigits"), prefix + QStringLiteral("word.18c.0"), updates);
    patchScalar(&before, &after, QStringLiteral("decimalDigits"), prefix + QStringLiteral("word.18c.1"), updates);
    if (newOperation && current.wordButton.usesTotalDigits()) {
        updates->set(prefix + QStringLiteral("word.18c.0"), current.wordButton.totalDigits);
    }
    if (newOperation && current.wordButton.usesDecimalDigits()) {
        updates->set(prefix + QStringLiteral("word.18c.1"), current.wordButton.decimalDigits);
    }
    patchScalar(&before, &after, QStringLiteral("pressedEffect"), prefix + QStringLiteral("switch.tail.8"), updates);
    if (before != after) {
        return false;
    }
    if (original.address != current.address) {
        updates->address(prefix + QStringLiteral("word.150"), current.address);
    }
    beforeShared = original;
    afterShared = current;
    beforeShared.hasWordButtonProperties = false;
    afterShared.hasWordButtonProperties = false;
    afterShared.address = beforeShared.address; // Keep the bit-only address updater outside the word operation record.
    return patchButton(beforeShared, afterShared, prefix, languages, updates);
}

/******************************************************************************
 * Function Name   : patchControlPreview
 * Description     : Keep a lamp or function button's normal cached graphical child consistent with its image selection.
 * Input Parameters: original, current, prefix, updates
 * Return Value    : True after preserving or rebuilding the bounded preview child.
 * Date            : 2026.11.10
 ******************************************************************************/
bool patchControlPreview(const HmiObject &original, const HmiObject &current, const QString &prefix,
                            SourceUpdates *updates) //20261110-0 change
{
    const BitButtonState *appearance;
    const BitButtonState *before;
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    QMap<qsizetype, QPair<qsizetype, QByteArray>>::iterator iterator;
    QString path;
    qsizetype begin;
    qsizetype end;
    double x;
    double y;
    double width;
    double height;
    int selected;
    int kind;

    selected = current.hasLampProperties ? updates->document.value(prefix + QStringLiteral("switch.274.1")).toInt() : 0;
    appearance = current.hasLampProperties ? current.lamp.state(current.bitButton, selected) : &current.bitButton.states[0];
    before = original.hasLampProperties ? original.lamp.state(original.bitButton, selected) : &original.bitButton.states[0];
    if (selected < 0 || (current.hasLampProperties && selected >= current.lamp.stateCount()) || appearance == nullptr || before == nullptr) {
        return false;
    }
    kind = updates->document.value(prefix + QStringLiteral("pixmap.type")).toInt();
    if ((appearance->useImage && kind == 13) || (!appearance->useImage && kind == 0x1000f)) {
        if (!appearance->useImage && appearance->background != before->background) {
            updates->set(prefix + QStringLiteral("picture-rectangle.4"), appearance->background);
            updates->set(prefix + QStringLiteral("picture-rectangle.6"), appearance->background);
        }
        return true;
    }
    x = current.geometry.x();
    y = current.geometry.y();
    width = current.geometry.width();
    height = current.geometry.height();
    if (current.geometry == original.geometry) {
        x = updates->document.value(prefix + QStringLiteral("rectangle.0")).toDouble() + updates->document.value(prefix + QStringLiteral("rectangle.9")).toDouble();
        y = updates->document.value(prefix + QStringLiteral("rectangle.1")).toDouble() + updates->document.value(prefix + QStringLiteral("rectangle.10")).toDouble();
        width = updates->document.value(prefix + QStringLiteral("text.head.2")).toDouble();
        height = updates->document.value(prefix + QStringLiteral("text.head.3")).toDouble();
    }
    stream.setVersion(QDataStream::Qt_4_8);
    if (appearance->useImage) {
        path = appearance->picture;
        if (!path.startsWith(QStringLiteral(":/btn/images/"))) {
            if (path.contains(QLatin1Char('/')) || path.contains(QLatin1Char('\\')) || !path.endsWith(QStringLiteral(".kvg"))) {
                return false;
            }
            path.prepend(QStringLiteral(":/btn/images/"));
        }
        stream << 13 << path << 0.0 << x << y << width << height << x << y << true << 0 << QStringList{} << 0;
    } else {
        // The original state-two no-image oracle supplies this exact filled-rectangle primitive sequence.
        stream << 0x1000f << 0.0 << 0.0 << width << height << appearance->background << 0 << appearance->background
               << QColor(Qt::black) << 1 << x << y << 1 << true << QStringLiteral("1000") << 0 << 0 << QStringList{} << 0;
    }
    if (stream.status() != QDataStream::Ok) {
        return false;
    }
    begin = updates->document.values.value(prefix + QStringLiteral("pixmap.type")).start;
    end = updates->document.values.value(prefix + QStringLiteral("text.type")).start;
    iterator = updates->replacements.begin();
    while (iterator != updates->replacements.end()) {
        if (iterator.key() >= begin && iterator.value().first <= end) {
            iterator = updates->replacements.erase(iterator);
        } else {
            ++iterator;
        }
    }
    updates->replacements.insert(begin, qMakePair(end, bytes));
    return true;
}

/******************************************************************************
 * Function Name   : patchFunctionButton
 * Description     : Save verified system selections and pressed appearances while retaining opaque function-union and inactive monitor bytes.
 * Input Parameters: original, current, prefix, languages, updates
 * Return Value    : True when the active function properties have a native source representation.
 * Date            : 2026.11.10
 ******************************************************************************/
bool patchFunctionButton(const HmiObject &original, const HmiObject &current, const QString &prefix,
                            const QStringList &languages, SourceUpdates *updates) //20261110-0 add
{
    HmiObject beforeShared;
    HmiObject afterShared;
    bool functionChanged;
    int nativeType;

    if (!original.hasFunctionButtonProperties || !current.hasFunctionButtonProperties || current.hasWordButtonProperties ||
        current.hasLampProperties || !current.functionButton.validate(nullptr) || !current.address.isEmpty() ||
        current.bitButton.indicator || current.bitButton.dualMonitor || current.bitButton.macroEnabled ||
        current.bitButton.pressTime < 0 || current.bitButton.pressTime > 255 ||
        (current.bitButton.noticeBit && current.bitButton.noticeWord) ||
        (current.functionButton.pressedEffect && current.bitButton.states[1].sameLanguage != current.bitButton.states[0].sameLanguage) ||
        (current.functionButton.function == 13 && current.functionButton.type >= languages.size()) ||
        updates->document.value(prefix + QStringLiteral("switch.290.count")).toInt() != (current.functionButton.pressedEffect ? 2 : 1)) {
        return false;
    }
    functionChanged = current.functionButton.function != original.functionButton.function;
    if (functionChanged) {
        updates->set(prefix + QStringLiteral("function.1c8.0"), current.functionButton.function);
    }
    nativeType = FunctionButtonProperties::types(current.functionButton.function, languages).isEmpty() ? -1 : current.functionButton.type;
    if (functionChanged || current.functionButton.type != original.functionButton.type) {
        updates->set(prefix + QStringLiteral("function.1c8.1"), nativeType);
    }
    if (current.functionButton.function == 14) {
        if (functionChanged || current.functionButton.keyboardKey != original.functionButton.keyboardKey) {
            updates->set(prefix + QStringLiteral("function.1c8.5"), current.functionButton.keyboardKey);
        }
        if (functionChanged || current.functionButton.keyboardText != original.functionButton.keyboardText) {
            updates->set(prefix + QStringLiteral("function.1c8.6"), current.functionButton.keyboardText);
        }
    }
    if (current.functionButton.pressedEffect != original.functionButton.pressedEffect) {
        updates->set(prefix + QStringLiteral("switch.tail.8"), current.functionButton.pressedEffect);
    }
    if (current.bitButton.states[0].sameLanguage != original.bitButton.states[0].sameLanguage) {
        updates->set(prefix + QStringLiteral("switch.tail.4"), current.bitButton.states[0].sameLanguage);
    }
    beforeShared = original;
    afterShared = current;
    beforeShared.hasFunctionButtonProperties = false;
    afterShared.hasFunctionButtonProperties = false;
    // The function dialog has no monitor page; these editor drafts cannot replace source metadata.
    afterShared.bitButton.operation = beforeShared.bitButton.operation;
    afterShared.bitButton.monitorSame = beforeShared.bitButton.monitorSame;
    afterShared.bitButton.monitorType = beforeShared.bitButton.monitorType;
    afterShared.bitButton.monitorBitAddress = beforeShared.bitButton.monitorBitAddress;
    afterShared.bitButton.monitorWordAddress = beforeShared.bitButton.monitorWordAddress;
    afterShared.bitButton.bitIndex = beforeShared.bitButton.bitIndex;
    if (!current.functionButton.pressedEffect) {
        beforeShared.bitButton.states[1] = beforeShared.bitButton.states[0];
        afterShared.bitButton.states[1] = afterShared.bitButton.states[0];
    }
    afterShared.bitButton.macroName = beforeShared.bitButton.macroName;
    if (!afterShared.bitButton.touch.visibilityEnabled) {
        afterShared.bitButton.touch = beforeShared.bitButton.touch;
        afterShared.bitButton.touch.visibilityEnabled = false;
    }
    if (!afterShared.bitButton.visibility.visibilityEnabled) {
        afterShared.bitButton.visibility = beforeShared.bitButton.visibility;
        afterShared.bitButton.visibility.visibilityEnabled = false;
    }
    if (!afterShared.bitButton.notice || !afterShared.bitButton.noticeBit) {
        afterShared.bitButton.noticeBitAddress = beforeShared.bitButton.noticeBitAddress;
        afterShared.bitButton.noticeBitValue = beforeShared.bitButton.noticeBitValue;
    }
    if (!afterShared.bitButton.notice || !afterShared.bitButton.noticeWord) {
        afterShared.bitButton.noticeWordAddress = beforeShared.bitButton.noticeWordAddress;
        afterShared.bitButton.noticeWordType = beforeShared.bitButton.noticeWordType;
        afterShared.bitButton.noticeWordValue = beforeShared.bitButton.noticeWordValue;
    }
    if (!afterShared.bitButton.pressTimeEnabled) {
        afterShared.bitButton.pressTime = beforeShared.bitButton.pressTime;
    }
    afterShared.text = beforeShared.text; // The shared state's real caption updates its native preview; this field is only the canvas cache.
    if (!patchButton(beforeShared, afterShared, prefix, languages, updates)) {
        return false;
    }
    return patchControlPreview(original, current, prefix, updates);
}

/******************************************************************************
 * Function Name   : patchLamp
 * Description     : Save verified read-only monitoring and every active appearance without creating a touch-operation payload.
 * Input Parameters: original, current, prefix, languages, updates
 * Return Value    : True after every supported lamp difference is represented.
 * Date            : 2026.11.09
 ******************************************************************************/
bool patchLamp(const HmiObject &original, const HmiObject &current, const QString &prefix,
                  const QStringList &languages, SourceUpdates *updates) //20261109-8 add
{
    HmiObject beforeShared;
    HmiObject afterShared;
    QJsonObject before;
    QJsonObject after;
    QByteArray mapping;
    QDataStream stream(&mapping, QIODevice::Append);
    const BitButtonState *beforeState;
    const BitButtonState *afterState;
    QString monitor;
    QString oldMonitor;
    qsizetype start;
    qsizetype end;
    int count;
    int state;
    bool changedMode;
    bool sameLanguage;

    if (!original.hasLampProperties || !current.hasLampProperties || current.hasWordButtonProperties ||
        !current.lamp.validate(nullptr) || current.lamp.monitorMode == 0 || current.address != original.address ||
        current.bitButton.operation != original.bitButton.operation ||
        current.bitButton.touch.visibilityEnabled || //20261109-8 change: Lamp has no touch page; inactive editor drafts must not replace opaque source defaults.
        current.bitButton.pressTimeEnabled || current.bitButton.macroEnabled || current.bitButton.notice ||
        current.bitButton.pressTime != original.bitButton.pressTime) {
        return false;
    }
    count = current.lamp.stateCount();
    sameLanguage = current.bitButton.states[0].sameLanguage;
    for (state = 0; state < count; ++state) { //20261109-8 add: The original same-language checkbox is global across all active states.
        afterState = current.lamp.state(current.bitButton, state);
        if (afterState == nullptr || afterState->sameLanguage != sameLanguage) {
            return false;
        }
    }
    if (sameLanguage != original.bitButton.states[0].sameLanguage) {
        updates->set(prefix + QStringLiteral("switch.tail.4"), sameLanguage);
    }
    changedMode = current.lamp.monitorMode != original.lamp.monitorMode;
    if (updates->document.value(prefix + QStringLiteral("switch.290.count")).toInt() != count ||
        current.bitButton.indicator != true || current.bitButton.dualMonitor != (current.lamp.monitorMode == 1)) {
        return false;
    }
    before = original.lamp.toJson();
    after = current.lamp.toJson();
    updates->set(prefix + QStringLiteral("switch.248.5"), current.lamp.monitorMode);
    if (current.lamp.monitorMode == 1) {
        monitor = current.bitButton.monitorType == 0 ? current.bitButton.monitorBitAddress : current.bitButton.monitorWordAddress;
        oldMonitor = original.bitButton.monitorType == 0 ? original.bitButton.monitorBitAddress : original.bitButton.monitorWordAddress;
        if (changedMode || current.bitButton.monitorType != original.bitButton.monitorType) {
            updates->set(prefix + QStringLiteral("switch.248.0"), current.bitButton.monitorType);
        }
        if (changedMode || current.bitButton.bitIndex != original.bitButton.bitIndex) {
            updates->set(prefix + QStringLiteral("switch.248.1"), current.bitButton.bitIndex);
        }
        if (current.bitButton.monitorSame != original.bitButton.monitorSame) {
            return false; // The native Lamp dialog disables this operation-address coupling control.
        }
        if (changedMode) {
            updates->set(prefix + QStringLiteral("switch.248.2"), current.bitButton.monitorSame);
        }
        if (changedMode || monitor != oldMonitor || current.bitButton.monitorType != original.bitButton.monitorType) {
            updates->address(prefix + QStringLiteral("switch.254"), monitor);
            updates->set(prefix + QStringLiteral("switch.254.7"), 1);
        }
    } else {
        if (current.lamp.multiMonitorSame != original.lamp.multiMonitorSame) {
            return false;
        }
        if (changedMode || current.lamp.multiDataType != original.lamp.multiDataType) {
            updates->set(prefix + QStringLiteral("switch.248.4"), current.lamp.multiDataType);
        }
        if (changedMode) {
            updates->set(prefix + QStringLiteral("switch.248.2"), current.lamp.multiMonitorSame);
        }
        if (changedMode || current.lamp.multiMonitorAddress != original.lamp.multiMonitorAddress) {
            updates->address(prefix + QStringLiteral("switch.254"), current.lamp.multiMonitorAddress);
        }
        if (changedMode || current.lamp.multiDataType != original.lamp.multiDataType ||
            current.lamp.multiMonitorAddress != original.lamp.multiMonitorAddress) {
            updates->set(prefix + QStringLiteral("switch.254.7"), current.lamp.transferWordCount());
        }
        if (changedMode || current.lamp.multiStateCount != original.lamp.multiStateCount) {
            stream.setVersion(QDataStream::Qt_4_8);
            stream << count;
            for (state = 0; state < count; ++state) {
                stream << static_cast<double>(state);
            }
            start = updates->document.values.value(prefix + QStringLiteral("switch.278.count")).start;
            end = updates->document.values.value(prefix + QStringLiteral("switch.274.0")).start;
            updates->replacements.insert(start, qMakePair(end, mapping));
        }
    }
    for (state = 2; state < count; ++state) {
        beforeState = original.lamp.state(original.bitButton, state);
        afterState = current.lamp.state(current.bitButton, state);
        if (beforeState == nullptr || afterState == nullptr ||
            afterState->effect3d != current.bitButton.states[0].effect3d ||
            afterState->transparent != current.bitButton.states[0].transparent ||
            !patchState(beforeState->toJson(), afterState->toJson(), prefix, state, languages, updates)) {
            return false;
        }
    }
    before.remove(QStringLiteral("monitorMode")); after.remove(QStringLiteral("monitorMode"));
    before.remove(QStringLiteral("multiDataType")); after.remove(QStringLiteral("multiDataType"));
    before.remove(QStringLiteral("multiStateCount")); after.remove(QStringLiteral("multiStateCount"));
    before.remove(QStringLiteral("multiMonitorSame")); after.remove(QStringLiteral("multiMonitorSame"));
    before.remove(QStringLiteral("multiMonitorAddress")); after.remove(QStringLiteral("multiMonitorAddress"));
    before.remove(QStringLiteral("extraStates")); after.remove(QStringLiteral("extraStates"));
    if (before != after) {
        return false;
    }
    beforeShared = original;
    afterShared = current;
    beforeShared.hasLampProperties = false;
    afterShared.hasLampProperties = false;
    afterShared.bitButton.operation = beforeShared.bitButton.operation;
    afterShared.bitButton.indicator = beforeShared.bitButton.indicator;
    afterShared.bitButton.dualMonitor = beforeShared.bitButton.dualMonitor;
    afterShared.bitButton.monitorSame = beforeShared.bitButton.monitorSame;
    afterShared.bitButton.monitorType = beforeShared.bitButton.monitorType;
    afterShared.bitButton.monitorBitAddress = beforeShared.bitButton.monitorBitAddress;
    afterShared.bitButton.monitorWordAddress = beforeShared.bitButton.monitorWordAddress;
    afterShared.bitButton.bitIndex = beforeShared.bitButton.bitIndex;
    afterShared.bitButton.touch = beforeShared.bitButton.touch; //20261109-8 add: Preserve the original disabled touch block while retaining KSP-only drafts.
    if (!patchButton(beforeShared, afterShared, prefix, languages, updates)) { //20261109-8 change
        return false;
    }
    return patchControlPreview(original, current, prefix, updates);
}

/******************************************************************************
 * Function Name   : resizeButtonSourceStates
 * Description     : Rebuild the two typed visual lists when native monitor mode changes the number of saved states.
 * Input Parameters: document, prefix, count, data, error
 * Return Value    : True after a complete source snapshot contains the supported requested appearance count.
 * Date            : 2026.11.09
 ******************************************************************************/
bool resizeButtonSourceStates(const SourceDocument &document, const QString &prefix, int count,
                                QByteArray *data, QString *error, int lampMode = -1) //20261109-8 change
{
    SourceUpdates updates(document);
    QByteArray texts;
    QByteArray pictures;
    QByteArray result;
    QByteArray mapping; //20261109-8 add
    QDataStream stream(&mapping, QIODevice::WriteOnly);
    QString row;
    int originalCount;
    int languages;
    int state;
    int sourceState;
    int selected; //20261109-8 add
    qsizetype start;
    qsizetype end;
    bool lamp;

    lamp = document.value(prefix + QStringLiteral("switch.mode")).toInt() == 4; //20261109-8 add
    originalCount = document.value(prefix + QStringLiteral("switch.290.count")).toInt();
    if (data == nullptr || count < 1 || count > (lamp ? 32 : 2) || originalCount < 1 || originalCount > (lamp ? 32 : 2)) { //20261109-8 change
        return fail(error, QStringLiteral("原生按钮显示状态数量无效。"));
    }
    appendWord(&texts, static_cast<u32>(count));
    appendWord(&pictures, static_cast<u32>(count));
    for (state = 0; state < count; ++state) {
        sourceState = qMin(state, originalCount - 1);
        row = SourceDocument::indexed(prefix + QStringLiteral("switch.290"), sourceState);
        languages = document.value(row + QStringLiteral(".count")).toInt();
        if (languages < 1 || languages > 8) {
            return fail(error, QStringLiteral("原生按钮状态语言记录数量无效。"));
        }
        start = document.values.value(row + QStringLiteral(".count")).start;
        end = document.values.value(SourceDocument::indexed(row, languages - 1) + QStringLiteral(".4")).end;
        texts.append(document.bytes.mid(start, end - start));
        row = SourceDocument::indexed(prefix + QStringLiteral("switch.294"), sourceState);
        start = document.values.value(row + QStringLiteral(".0")).start;
        end = document.values.value(row + QStringLiteral(".4")).end;
        pictures.append(document.bytes.mid(start, end - start));
    }
    updates.set(prefix + QStringLiteral("switch.248.3"), count);
    if (lamp) { //20261109-8 add: Keep the active monitor mode consistent during the intermediate typed reparse.
        if (lampMode < 1 || lampMode > 2 || (lampMode == 1 && count != 2)) {
            return fail(error, QStringLiteral("原生指示灯状态重建需要已验证的双态或多态模式。"));
        }
        updates.set(prefix + QStringLiteral("switch.248.5"), lampMode);
        updates.set(prefix + QStringLiteral("switch.254.7"), 1); // Match the intermediate U16/dual record before reparsing; patchLamp restores the final width.
        selected = qBound(0, document.value(prefix + QStringLiteral("switch.274.1")).toInt(), count - 1);
        if (selected != document.value(prefix + QStringLiteral("switch.274.1")).toInt()) { // Clamp the saved preview when its old state was deleted.
            updates.set(prefix + QStringLiteral("switch.274.1"), selected);
            row = SourceDocument::indexed(SourceDocument::indexed(prefix + QStringLiteral("switch.290"), qMin(selected, originalCount - 1)), 0);
            updates.set(prefix + QStringLiteral("text.head.4"), document.value(row + QStringLiteral(".4")));
            updates.set(prefix + QStringLiteral("font.family"), document.value(row + QStringLiteral(".1")));
            updates.set(prefix + QStringLiteral("font.tail.1"), document.value(row + QStringLiteral(".3")));
            updates.set(prefix + QStringLiteral("text.tail.0"), document.value(row + QStringLiteral(".0")));
        }
        if (lampMode == 2) {
            updates.set(prefix + QStringLiteral("switch.248.4"), 0);
            stream.setVersion(QDataStream::Qt_4_8);
            stream << count;
            for (state = 0; state < count; ++state) {
                stream << static_cast<double>(state);
            }
            start = document.values.value(prefix + QStringLiteral("switch.278.count")).start;
            end = document.values.value(prefix + QStringLiteral("switch.274.0")).start;
            updates.replacements.insert(start, qMakePair(end, mapping));
        }
    }
    if (document.value(prefix + QStringLiteral("switch.mode")).toInt() == 1 ||
        document.value(prefix + QStringLiteral("switch.mode")).toInt() == 3) { //20261110-0 change
        updates.set(prefix + QStringLiteral("switch.tail.8"), count == 2);
    }
    start = document.values.value(prefix + QStringLiteral("switch.290.count")).start;
    end = document.values.value(prefix + QStringLiteral("switch.294.count")).start;
    updates.replacements.insert(start, qMakePair(end, texts));
    start = end;
    end = document.values.value(prefix + QStringLiteral("switch.tail.0")).start;
    updates.replacements.insert(start, qMakePair(end, pictures));
    result = updates.render();
    if (!updates.error.isEmpty()) {
        return fail(error, updates.error);
    }
    *data = result;
    clearError(error);
    return true;
}

/******************************************************************************
 * Function Name   : patchSourceMacros
 * Description     : Save changed macro sources in the verified native uncompiled state while retaining untouched original records.
 * Input Parameters: document, original, current, updates, error
 * Return Value    : True for source-only macro edits without unsupported startup, dependency or variable metadata.
 * Date            : 2026.11.09
 ******************************************************************************/
bool patchSourceMacros(const SourceDocument &document, const HmiProject &original, const HmiProject &current,
                         SourceUpdates *updates, QString *error) //20261109-0 add
{
    SourceUpdates macroUpdates(document);
    QByteArray block;
    QByteArray record;
    QString prefix;
    QString name;
    QRegularExpression identifier(QStringLiteral("^[A-Za-z_][A-Za-z0-9_]{0,63}$"));
    QMap<QString, QString>::const_iterator macro;
    qsizetype start;
    qsizetype end;
    int count;
    int i;

    if (current.macros == original.macros) {
        return true;
    }
    if (current.macros.size() > 100 || document.value(QStringLiteral("history.1981f8.count")).toInt() != 0 ||
        document.value(QStringLiteral("system.194.count")).toInt() != 0 || document.value(QStringLiteral("config.284.count")).toInt() != 0) {
        return fail(error, QStringLiteral("原生宏源编辑仅支持最多100项普通宏；启动宏、全局变量或其它宏类型元数据尚未核实。"));
    }
    for (macro = current.macros.constBegin(); macro != current.macros.constEnd(); ++macro) {
        if (!identifier.match(macro.key()).hasMatch() || macro.value().size() > static_cast<int>(MaximumStringBytes / 2)) {
            return fail(error, QStringLiteral("原生宏名称须为 1..64 位 ASCII 标识符，源码须在原生 QString 长度范围内。"));
        }
    }
    appendWord(&block, static_cast<u32>(current.macros.size()));
    count = document.value(QStringLiteral("history.1e4.count")).toInt();
    for (i = 0; i < count; ++i) {
        prefix = SourceDocument::indexed(QStringLiteral("history.1e4"), i);
        name = document.value(prefix + QStringLiteral(".head.1")).toString();
        if ((!current.macros.contains(name) || current.macros.value(name) != original.macros.value(name)) &&
            (document.value(prefix + QStringLiteral(".8.count")).toInt() != 0 ||
             document.value(prefix + QStringLiteral(".c.count")).toInt() != 0 ||
             document.value(prefix + QStringLiteral(".head.2")).toInt() != 1 ||
             (document.value(prefix + QStringLiteral(".code")).toByteArray().size() != 0 &&
              document.value(prefix + QStringLiteral(".code")).toByteArray().size() != 16))) {
            return fail(error, QStringLiteral("宏 %1 含未恢复的依赖、变量、校验或运行标志，不能改写或删除其源码元数据。").arg(name));
        }
        if (!current.macros.contains(name)) {
            continue;
        }
        start = document.values.value(prefix + QStringLiteral(".source")).start;
        end = i + 1 < count ? document.values.value(SourceDocument::indexed(QStringLiteral("history.1e4"), i + 1) + QStringLiteral(".source")).start :
                             document.values.value(QStringLiteral("history.1981f8.count")).start;
        if (current.macros.value(name) == original.macros.value(name)) {
            block.append(document.bytes.mid(start, end - start));
            continue;
        }
        macroUpdates.replacements.clear();
        macroUpdates.set(prefix + QStringLiteral(".source"), current.macros.value(name));
        macroUpdates.set(prefix + QStringLiteral(".head.0"), false);
        macroUpdates.set(prefix + QStringLiteral(".code"), QByteArray());
        record = macroUpdates.render(start, end);
        if (!macroUpdates.error.isEmpty()) {
            return fail(error, macroUpdates.error);
        }
        block.append(record);
    }
    for (macro = current.macros.constBegin(); macro != current.macros.constEnd(); ++macro) {
        if (original.macros.contains(macro.key())) {
            continue;
        }
        appendString(&block, macro.value());
        appendWord(&block, 0); // No unverified macro dependency list is manufactured.
        block.append(char(0)); // Saving source invalidates the previous binary MD5 and compile-success flag.
        appendString(&block, macro.key());
        block.append(char(1)); // Original editor's ordinary macro record flag.
        appendWord(&block, NullQString);
        appendWord(&block, 0); // Variables remain empty until the original macro editor compiles the source.
    }
    start = document.values.value(QStringLiteral("history.1e4.count")).start;
    end = document.values.value(QStringLiteral("history.1981f8.count")).start;
    updates->replacements.insert(start, qMakePair(end, block));
    return true;
}

}

/******************************************************************************
 * Function Name   : inspectNativeProjectEnvelope
 * Description     : Recognize only the observed version and model, retaining all opaque bytes.
 * Input Parameters: data, envelope, error
 * Return Value    : True for a verified header, with explicit body validation limitations.
 * Date            : 2026.11.09
 ******************************************************************************/
bool inspectNativeProjectEnvelope(const QByteArray &data, NativeProjectEnvelope *envelope,
                                  QString *error) //20261109-0 add
{
    NativeProjectEnvelope result;
    SourceReader reader(data, 0);

    if (envelope == nullptr) {
        return fail(error, QStringLiteral("原生工程结构输出为空。"));
    }
    if (data.size() > MaximumSourceBytes) {
        return fail(error, QStringLiteral("原生工程超过 64 MiB 结构检查上限。"));
    }
    if (!reader.string(&result.signature)) {
        return fail(error, QStringLiteral("原生工程签名被截断或 QString 长度无效。"));
    }
    if (result.signature != QStringLiteral("KinSealStudio-V1.0.2")) {
        return fail(error, QStringLiteral("不支持的原生工程签名；文件必须是 KinSeal Qt 原生流。"));
    }
    if (!reader.word(&result.version)) {
        return fail(error, QStringLiteral("原生工程缺少版本号。"));
    }
    if (result.version != 122) {
        return fail(error, QStringLiteral("不支持的原生工程版本 %1；目前仅核实版本 122 的文件头。")
                               .arg(result.version));
    }
    if (!reader.string(&result.targetModel)) {
        return fail(error, QStringLiteral("原生工程型号 QString 被截断或长度无效。"));
    }
    if (result.targetModel != QStringLiteral("AMT070W124-L800X480R")) {
        return fail(error, QStringLiteral("尚未核实该型号的原生工程结构：%1。")
                               .arg(result.targetModel));
    }
    if (reader.offset() == data.size()) {
        return fail(error, QStringLiteral("原生工程只有文件头，缺少工程主体。"));
    }
    result.payloadOffset = reader.offset();
    result.originalBytes = data;
    result.originalSha256 = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    result.limitations.append(QStringLiteral("仅验证文件头；系统配置、页面、控件和尾部记录尚未解析，主体完整性未知。"));
    result.limitations.append(QStringLiteral("快照可逐字节恢复；不支持语义导入、编辑后原生保存或生成新 .sm 工程。"));
    *envelope = result;
    clearError(error);
    return true;
}

/******************************************************************************
 * Function Name   : restoreNativeProjectArchive
 * Description     : Reject changed snapshots before restoring the original source stream.
 * Input Parameters: envelope, data, error
 * Return Value    : True after lossless snapshot restoration.
 * Date            : 2026.11.09
 ******************************************************************************/
bool restoreNativeProjectArchive(const NativeProjectEnvelope &envelope, QByteArray *data,
                                  QString *error) //20261109-0 add
{
    NativeProjectEnvelope checked;

    if (data == nullptr) {
        return fail(error, QStringLiteral("原生工程快照输出为空。"));
    }
    if (envelope.originalSha256.size() != 32 ||
        QCryptographicHash::hash(envelope.originalBytes, QCryptographicHash::Sha256) !=
            envelope.originalSha256) {
        return fail(error, QStringLiteral("原生工程快照已改变，不能按无损归档恢复。"));
    }
    if (!inspectNativeProjectEnvelope(envelope.originalBytes, &checked, error)) {
        return false;
    }
    if (envelope.signature != checked.signature || envelope.version != checked.version ||
        envelope.targetModel != checked.targetModel || envelope.payloadOffset != checked.payloadOffset) {
        return fail(error, QStringLiteral("原生工程快照的文件头元数据与原始字节不一致。"));
    }
    *data = envelope.originalBytes;
    clearError(error);
    return true;
}

/******************************************************************************
 * Function Name   : validateNativeProjectSourceStructure
 * Description     : Validate the entire recovered source grammar without changing editable state.
 * Input Parameters: data, error
 * Return Value    : True for a complete supported source stream.
 * Date            : 2026.11.09
 ******************************************************************************/
bool validateNativeProjectSourceStructure(const QByteArray &data, QString *error) //20261109-0 add
{
    SourceDocument document(data);

    if (!document.parse()) {
        return fail(error, document.error);
    }
    clearError(error);
    return true;
}

/******************************************************************************
 * Function Name   : inspectNativeProjectSourceFields
 * Description     : Publish typed paths and source spans after full parsing and EOF validation.
 * Input Parameters: data, fields, error
 * Return Value    : True after atomic replacement of the caller span list.
 * Date            : 2026.11.10
 ******************************************************************************/
bool inspectNativeProjectSourceFields(const QByteArray &data, QList<NativeSourceFieldSpan> *fields,
                                      QString *error) //20261110-5 add
{
    SourceDocument document(data);
    QList<NativeSourceFieldSpan> result;
    NativeSourceFieldSpan span;
    QMap<QString, SourceField>::const_iterator iterator;

    if (fields == nullptr) {
        return fail(error, QStringLiteral("原生 typed 字段输出为空。"));
    }
    if (!document.parse()) {
        return fail(error, document.error);
    }
    result.reserve(document.values.size());
    for (iterator = document.values.cbegin(); iterator != document.values.cend(); ++iterator) {
        span.path = iterator.key();
        span.kind = iterator.value().kind;
        span.start = iterator.value().start;
        span.end = iterator.value().end;
        result.append(span);
    }
    std::sort(result.begin(), result.end(), [](const NativeSourceFieldSpan &left, const NativeSourceFieldSpan &right) {
        return left.start < right.start;
    });
    *fields = result;
    clearError(error);
    return true;
}

/******************************************************************************
 * Function Name   : nativeProjectUsesSupportedDeviceDefaults
 * Description     : Restrict device compilation to independently verified native configuration bytes.
 * Input Parameters: data, error
 * Return Value    : True for the observed system and project configuration fingerprints.
 * Date            : 2026.11.09
 ******************************************************************************/
bool nativeProjectUsesSupportedDeviceDefaults(const QByteArray &data, QString *error) //20261109-0 add
{
    SourceDocument document(data);
    qsizetype systemStart;
    qsizetype configStart;
    qsizetype configEnd;
    QByteArray systemHash;
    QByteArray configHash;
    QByteArray languages;
    QByteArray normalizedSystem;
    QByteArray normalizedConfig;
    SourceUpdates updates(document);
    QStringList languageNames;
    QStringList presets;
    QStringList nativePresets; //20261110-6 add: Keep both observed encodings of inactive native labels explicit.
    QString name;
    qsizetype registryStart;
    qsizetype registryEnd;
    int count;
    int current;
    int i;

    if (!document.parse()) {
        return fail(error, document.error);
    }
    if (document.value(QStringLiteral("project.pages.count")).toInt() != 1) { //20261110-5 add: Read-only page traversal does not widen the one-page device configuration contract.
        return fail(error, QStringLiteral("多页原生源仅支持结构检查或整源归档，不能按单页设备默认配置编译。"));
    }
    systemStart = document.values.value(QStringLiteral("system.identity.0")).start;
    configStart = document.values.value(QStringLiteral("config.head.0")).start;
    configEnd = document.values.value(QStringLiteral("project.pages.count")).start;
    count = document.value(QStringLiteral("system.164.count")).toInt();
    current = document.value(QStringLiteral("system.identity.16")).toInt();
    if (count < 1 || count > 8 || document.value(QStringLiteral("system.identity.15")).toInt() != count ||
        current < 0 || current >= count) {
        return fail(error, QStringLiteral("原生工程语言数量、初始语言序号或注册表不对应。"));
    }
    for (i = 0; i < count; ++i) {
        name = document.value(SourceDocument::indexed(QStringLiteral("system.164"), i)).toString();
        if (name.isEmpty() || name.size() > 64 || languageNames.contains(name)) {
            return fail(error, QStringLiteral("原生工程语言名称为空、重复或超过已验证长度。"));
        }
        languageNames.append(name);
    }
    // Normalize only the independently observed language settings before checking every other original byte.
    updates.set(QStringLiteral("system.identity.15"), 1);
    updates.set(QStringLiteral("system.identity.16"), 0);
    if (document.value(QStringLiteral("config.290.0")).toInt() < 0 ||
        document.value(QStringLiteral("config.290.0")).toInt() >= count) { //20261110-6 change: Check both independent indices without requiring equality.
        return fail(error, QStringLiteral("原生语言设置窗口保存的语言序号越界。"));
    }
    updates.set(QStringLiteral("config.290.0"), 0);
    appendWord(&languages, 1);
    appendString(&languages, QStringLiteral("中文"));
    registryStart = document.values.value(QStringLiteral("system.164.count")).start;
    registryEnd = document.values.value(QStringLiteral("system.flags.0")).start;
    updates.replacements.insert(registryStart, qMakePair(registryEnd, languages));
    presets = QStringList{QStringLiteral("中文"), QStringLiteral("English"), QStringLiteral("德语"),
                          QStringLiteral("韩语"), QStringLiteral("日语"), QStringLiteral("法语")};
    nativePresets = QStringList{QStringLiteral("中文"), QStringLiteral("English"), QStringLiteral("µÂÓï"),
                                QStringLiteral("º«Óï"), QStringLiteral("ÈÕÓï"), QStringLiteral("·¨Óï")}; //20261110-6 add
    if (document.value(QStringLiteral("config.298.count")).toInt() != 0) {
        if (document.value(QStringLiteral("config.298.count")).toInt() != presets.size() || count > presets.size()) { //20261110-6 change
            return fail(error, QStringLiteral("原生工程含尚未核实的语言预设注册表。"));
        }
        for (i = 0; i < presets.size(); ++i) {
            name = document.value(SourceDocument::indexed(QStringLiteral("config.298"), i)).toString();
            if (i < count ? name != languageNames.at(i) : (name != presets.at(i) && name != nativePresets.at(i))) { //20261110-6 change: Active labels mirror the registry; unused slots retain an observed default.
                return fail(error, QStringLiteral("原生工程语言预设与原厂已核实列表不同。"));
            }
        }
        if (document.value(QStringLiteral("config.288.1")).toInt() != count - 1 || //20261110-6 change: The native count combo stores its zero-based selection.
            document.value(QStringLiteral("config.290.1")).toInt() != 1) {
            return fail(error, QStringLiteral("原生工程语言设置保存标志组合尚未核实。"));
        }
        updates.set(QStringLiteral("config.288.1"), 0);
        updates.set(QStringLiteral("config.290.1"), 0);
        languages.clear();
        appendWord(&languages, 0);
        registryStart = document.values.value(QStringLiteral("config.298.count")).start;
        registryEnd = document.values.value(QStringLiteral("config.290.0")).start;
        updates.replacements.insert(registryStart, qMakePair(registryEnd, languages));
    }
    normalizedSystem = updates.render(systemStart, configStart);
    normalizedConfig = updates.render(configStart, configEnd);
    if (!updates.error.isEmpty()) {
        return fail(error, updates.error);
    }
    systemHash = QCryptographicHash::hash(normalizedSystem, QCryptographicHash::Sha256).toHex();
    configHash = QCryptographicHash::hash(normalizedConfig, QCryptographicHash::Sha256).toHex();
    if (systemHash != QByteArray("61a579ec33b23d2872dc3d31f9900a72b24f6e56b1826bcd40b73dd7fbbc14a6") ||
        configHash != QByteArray("ca3da07ec299bf5bedab28b73682a1b8ea0ee4867413dfc1a1ca4e91fda3c6ba")) {
        return fail(error, QStringLiteral("该原生工程的系统、通信、语言或工程配置与已验证设备默认配置不同；可保留原始快照，但不能用默认系统段覆盖后下载。"));
    }
    clearError(error);
    return true;
}

/******************************************************************************
 * Function Name   : nativeProjectInitialLanguageIndex
 * Description     : Resolve the original startup language by name without confusing transient editor selection with firmware settings.
 * Input Parameters: project, index, error
 * Return Value    : True after replacing index; malformed snapshots leave the output unchanged.
 * Date            : 2026.11.09
 ******************************************************************************/
bool nativeProjectInitialLanguageIndex(const HmiProject &project, u16 *index,
                                       QString *error) //20261109-0 add
{
    SourceDocument document(project.nativeSourceSnapshot);
    QStringList names;
    QString name;
    int count;
    int initial;
    int mapped;
    int i;

    if (index == nullptr) {
        return fail(error, QStringLiteral("原生初始语言序号输出为空。"));
    }
    if (!project.validateSinglePageBackend(error)) { //20261110-5 add: Do not ignore new page/configuration metadata in source-backed backend calls.
        return false;
    }
    if (project.nativeSourceSnapshot.isEmpty()) {
        *index = static_cast<u16>(project.startupLanguage.isEmpty() ? 0 : project.languages.indexOf(project.startupLanguage)); //20261110-5 change
        clearError(error);
        return true;
    }
    if (!document.parse()) {
        return fail(error, document.error);
    }
    if (document.value(QStringLiteral("project.pages.count")).toInt() != 1) { //20261110-5 add: A multi-page snapshot cannot supply startup settings for a flattened single-page model.
        return fail(error, QStringLiteral("原生快照含多个页面，不能为单页活动模型派生启动语言。"));
    }
    count = document.value(QStringLiteral("system.164.count")).toInt();
    initial = document.value(QStringLiteral("system.identity.16")).toInt();
    if (count < 1 || count > 8 || document.value(QStringLiteral("system.identity.15")).toInt() != count ||
        initial < 0 || initial >= count || project.languages.isEmpty() || project.languages.size() > 8 ||
        document.value(QStringLiteral("config.290.0")).toInt() < 0 ||
        document.value(QStringLiteral("config.290.0")).toInt() >= count) { //20261110-6 change: The persisted dialog index does not supply the compiled system index.
        return fail(error, QStringLiteral("原生语言数量、初始语言序号或当前注册表无效。"));
    }
    for (i = 0; i < count; ++i) {
        name = document.value(SourceDocument::indexed(QStringLiteral("system.164"), i)).toString();
        if (name.isEmpty() || name.size() > 64 || names.contains(name)) {
            return fail(error, QStringLiteral("原生语言注册表存在空名称、重复名称或超长名称。"));
        }
        names.append(name);
    }
    for (i = 0; i < project.languages.size(); ++i) {
        if (project.languages.at(i).isEmpty() || project.languages.at(i).size() > 64 || project.languages.indexOf(project.languages.at(i)) != i) {
            return fail(error, QStringLiteral("当前语言注册表存在空名称、重复名称或超长名称。"));
        }
    }
    mapped = project.languages.indexOf(project.startupLanguage.isEmpty() ? names.at(initial) : project.startupLanguage); //20261110-5 change
    *index = static_cast<u16>(mapped >= 0 ? mapped : 0);
    clearError(error);
    return true;
}

/******************************************************************************
 * Function Name   : readNativeSourceAddress
 * Description     : Decode FUN_00574860 field order without converting it to a device address.
 * Input Parameters: data, offset, address, nextOffset, error
 * Return Value    : True for a complete source address record.
 * Date            : 2026.11.09
 ******************************************************************************/
bool readNativeSourceAddress(const QByteArray &data, qsizetype offset,
                              NativeSourceAddress *address, qsizetype *nextOffset,
                              QString *error) //20261109-0 add
{
    NativeSourceAddress result;
    SourceReader reader(data, offset);

    if (address == nullptr || nextOffset == nullptr) {
        return fail(error, QStringLiteral("原生源地址输出为空。"));
    }
    if (data.size() > MaximumSourceBytes || offset < 0 || offset >= data.size()) {
        return fail(error, QStringLiteral("原生源地址读取范围无效。"));
    }
    if (!reader.string(&result.text) || !reader.byte(&result.field0) ||
        !reader.byte(&result.field1) || !reader.byte(&result.field2) ||
        !reader.string(&result.numericText) || !reader.string(&result.registerText) ||
        !reader.byte(&result.field3) || !reader.word(&result.rawLength) ||
        !reader.string(&result.auxiliary)) {
        return fail(error, QStringLiteral("原生源地址记录被截断或 QString 长度无效。"));
    }
    *address = result;
    *nextOffset = reader.offset();
    clearError(error);
    return true;
}

/******************************************************************************
 * Function Name   : writeNativeSourceAddress
 * Description     : Reproduce source address strings, four opaque bytes and raw length exactly.
 * Input Parameters: address, data, error
 * Return Value    : True after a complete source record has been encoded.
 * Date            : 2026.11.09
 ******************************************************************************/
bool writeNativeSourceAddress(const NativeSourceAddress &address, QByteArray *data,
                               QString *error) //20261109-0 add
{
    QByteArray result;

    if (data == nullptr) {
        return fail(error, QStringLiteral("原生源地址编码输出为空。"));
    }
    if (!appendString(&result, address.text)) {
        return fail(error, QStringLiteral("原生源地址文本超过 QString 长度上限。"));
    }
    result.append(static_cast<char>(address.field0));
    result.append(static_cast<char>(address.field1));
    result.append(static_cast<char>(address.field2));
    if (!appendString(&result, address.numericText) || !appendString(&result, address.registerText)) {
        return fail(error, QStringLiteral("原生源地址寄存器文本超过 QString 长度上限。"));
    }
    result.append(static_cast<char>(address.field3));
    appendWord(&result, address.rawLength);
    if (!appendString(&result, address.auxiliary)) {
        return fail(error, QStringLiteral("原生源地址辅助文本超过 QString 长度上限。"));
    }
    *data = result;
    clearError(error);
    return true;
}

/******************************************************************************
 * Function Name   : importNativeProjectSource
 * Description     : Import recovered single-page switch-family and static-text source records transactionally.
 * Input Parameters: data, project, error
 * Return Value    : True after complete parsing and supported semantic conversion.
 * Date            : 2026.11.09
 ******************************************************************************/
bool importNativeProjectSource(const QByteArray &data, HmiProject *project,
                                QString *error) //20261109-0 add
{
    SourceDocument document(data);

    if (project == nullptr) {
        return fail(error, QStringLiteral("原生工程导入目标为空。"));
    }
    if (!document.parse()) {
        return fail(error, document.error);
    }
    return sourceProject(document, project, error);
}

/******************************************************************************
 * Function Name   : exportNativeProjectSource
 * Description     : Preserve native metadata while replacing only supported parsed fields.
 * Input Parameters: project, data, error
 * Return Value    : True after a complete supported stream has been rebuilt and validated.
 * Date            : 2026.11.09
 ******************************************************************************/
bool exportNativeProjectSource(const HmiProject &project, QByteArray *data,
                                QString *error) //20261109-0 add
{
    SourceDocument document(project.nativeSourceSnapshot);
    SourceDocument verifiedDocument(QByteArray{}); //20261110-5 add: Validate semantic readability before publishing replacement bytes.
    SourceDocument seed(QByteArray{});
    SourceDocument textSeed(QByteArray{});
    SourceDocument wordSeed(QByteArray{}); //20261109-6 add
    SourceDocument lampSeed(QByteArray{}); //20261109-8 add
    SourceDocument functionSeed(QByteArray{}); //20261110-0 add
    SourceUpdates updates(document);
    HmiProject original;
    HmiProject verifiedProject; //20261110-5 add
    HmiProject seedProject;
    HmiProject textSeedProject;
    HmiProject wordSeedProject; //20261109-6 add
    HmiProject lampSeedProject; //20261109-8 add
    HmiProject functionSeedProject; //20261110-0 add
    HmiProject resizedProject;
    HmiObject originalObject;
    QJsonObject before;
    QJsonObject after;
    QByteArray result;
    QByteArray objectBytes;
    QByteArray objectBlock;
    QByteArray languageBlock;
    QByteArray settingsLanguageBlock; //20261110-6 add: Preserve unused native dialog slots while updating active language labels.
    QByteArray resizedBytes;
    QFile seedFile(QStringLiteral(":/native/bit-button-default.sm"));
    QFile textSeedFile(QStringLiteral(":/samples/native_text.sm"));
    QFile wordSeedFile(QStringLiteral(":/native/word-button-default.sm")); //20261109-6 add
    QFile lampSeedFile(QStringLiteral(":/native/lamp-default.sm")); //20261109-8 add
    QFile functionSeedFile(QStringLiteral(":/native/function-button-default.sm")); //20261110-0 add
    QMap<QString, int> originalIndices;
    QSet<QString> ids;
    const SourceDocument *objectDocument;
    std::unique_ptr<SourceUpdates> objectUpdates;
    std::unique_ptr<SourceDocument> resizedDocument;
    QString prefix;
    QString languageName;
    qsizetype objectStart;
    qsizetype objectEnd;
    qsizetype listStart;
    qsizetype listEnd;
    bool newProject;
    bool isText;
    bool isWord; //20261109-6 add
    bool isLamp; //20261109-8 add
    bool isFunction; //20261110-0 add
    int originalIndex;
    int desiredStates;
    int objectIndex;
    int i;
    int settingsIndex; //20261110-6 add: Remap the independent native dialog selection by its original registered name.
    int settingsCount;
    u16 startupIndex; //20261110-5 add

    if (data == nullptr) {
        return fail(error, QStringLiteral("原生工程编码输出为空。"));
    }
    if (!project.validateSinglePageBackend(error)) { //20261110-5 add: Never flatten multi-page/configuration entities into only the legacy active-page view.
        return false;
    }
    if (!project.activePageExtensions.isEmpty()) { //20261110-5 add: Page extensions have no recovered source carrier; stripping the pages JSON must not silently discard them.
        return fail(error, QStringLiteral("当前画面扩展元数据尚无已恢复的原生源映射，已取消保存以避免丢失。"));
    }
    newProject = project.nativeSourceSnapshot.isEmpty();
    if (newProject) {
        if (!seedFile.open(QIODevice::ReadOnly)) {
            return fail(error, QStringLiteral("未嵌入已核实原生seed资源 :/native/bit-button-default.sm，不能生成新工程。"));
        }
        document.bytes = seedFile.readAll();
        seedFile.close();
    }
    if (!document.parse()) {
        return fail(error, document.error);
    }
    if (!sourceProject(document, &original, error)) {
        return false;
    }
    if (project.objects.size() > 1024) {
        return fail(error, QStringLiteral("原生单页控件数量超过已验证的 1024 输入保护范围。"));
    }
    if (project.languages.isEmpty() || project.languages.size() > 8 || !project.languages.contains(project.activeLanguage)) {
        return fail(error, QStringLiteral("原生源语言数量须为 1..8 且当前语言须存在于注册表中。"));
    }
    for (i = 0; i < project.languages.size(); ++i) {
        languageName = project.languages.at(i);
        if (languageName.isEmpty() || languageName.size() > 64 || project.languages.indexOf(languageName) != i) {
            return fail(error, QStringLiteral("原生源语言名称为空、重复或超出已验证长度。"));
        }
    }
    before = original.toJson();
    after = project.toJson();
    before.remove(QStringLiteral("projectName")); after.remove(QStringLiteral("projectName"));
    before.remove(QStringLiteral("resolution")); after.remove(QStringLiteral("resolution"));
    before.remove(QStringLiteral("pages")); after.remove(QStringLiteral("pages"));
    before.remove(QStringLiteral("nativeSourceSnapshot")); after.remove(QStringLiteral("nativeSourceSnapshot"));
    before.remove(QStringLiteral("nativeSourceWarnings")); after.remove(QStringLiteral("nativeSourceWarnings"));
    before.remove(QStringLiteral("schemaVersion")); after.remove(QStringLiteral("schemaVersion"));
    before.remove(QStringLiteral("languages")); after.remove(QStringLiteral("languages"));
    before.remove(QStringLiteral("activeLanguage")); after.remove(QStringLiteral("activeLanguage"));
    before.remove(QStringLiteral("activePageId")); after.remove(QStringLiteral("activePageId")); //20261110-5 add: These IDs belong to KSP editing, not a recovered native ID mapping.
    before.remove(QStringLiteral("startupPageId")); after.remove(QStringLiteral("startupPageId"));
    before.remove(QStringLiteral("pageOrder")); after.remove(QStringLiteral("pageOrder"));
    before.remove(QStringLiteral("startupLanguage")); after.remove(QStringLiteral("startupLanguage"));
    before.remove(QStringLiteral("projectId")); after.remove(QStringLiteral("projectId"));
    before.remove(QStringLiteral("macroIds")); after.remove(QStringLiteral("macroIds"));
    before.remove(QStringLiteral("configurationNodes")); after.remove(QStringLiteral("configurationNodes")); // The capability gate above already requires these collections to be empty.
    before.remove(QStringLiteral("references")); after.remove(QStringLiteral("references"));
    if (project.pictures != original.pictures) {
        return fail(error, QStringLiteral("原生源保存尚未验证图库条目的改写，已取消保存。"));
    }
    if (!patchSourceMacros(document, original, project, &updates, error)) {
        return false;
    }
    before.remove(QStringLiteral("macros")); after.remove(QStringLiteral("macros"));
    before.remove(QStringLiteral("pictures")); after.remove(QStringLiteral("pictures"));
    if (before != after) {
        return fail(error, QStringLiteral("原生源保存尚不支持本次系统、通信、语言、图片库或宏配置编辑；已取消保存以保留原始元数据。"));
    }
    if (project.name != original.name) {
        updates.set(QStringLiteral("trailer.settings.1"), project.name + QStringLiteral(".sm"));
    }
    if (project.pageName != original.pageName) {
        updates.set(QStringLiteral("page.head.18"), project.pageName);
        updates.set(QStringLiteral("config.head.4"), project.pageName);
        updates.set(QStringLiteral("config.head.11"), project.pageName);
        updates.set(QStringLiteral("trailer.settings.11"), project.pageName);
    }
    if (project.pageWidth != original.pageWidth || project.pageHeight != original.pageHeight) {
        return fail(error, QStringLiteral("原生型号分辨率绑定尚未验证可写，不能改变页面尺寸。"));
    }
    if (project.languages != original.languages) {
        updates.set(QStringLiteral("system.identity.15"), project.languages.size());
        // The editor's current language is transient; preserve the original startup language when it still exists.
        updates.set(QStringLiteral("system.identity.16"), qMax(0, project.languages.indexOf(original.activeLanguage)));
        settingsIndex = document.value(QStringLiteral("config.290.0")).toInt(); //20261110-6 change
        updates.set(QStringLiteral("config.290.0"), qMax(0, project.languages.indexOf(original.languages.at(settingsIndex))));
        appendWord(&languageBlock, static_cast<u32>(project.languages.size()));
        for (i = 0; i < project.languages.size(); ++i) {
            if (!appendString(&languageBlock, project.languages.at(i))) {
                return fail(error, QStringLiteral("原生源语言名称编码失败。"));
            }
        }
        listStart = document.values.value(QStringLiteral("system.164.count")).start;
        listEnd = document.values.value(QStringLiteral("system.flags.0")).start;
        updates.replacements.insert(listStart, qMakePair(listEnd, languageBlock));
        settingsCount = document.value(QStringLiteral("config.298.count")).toInt(); //20261110-6 add
        if (settingsCount > 0) {
            if (settingsCount != document.value(QStringLiteral("config.288.0")).toInt() ||
                original.languages.size() > settingsCount ||
                document.value(QStringLiteral("config.288.1")).toInt() != original.languages.size() - 1 ||
                document.value(QStringLiteral("config.290.1")).toInt() != 1) {
                return fail(error, QStringLiteral("原生语言设置窗口的保存组合尚未恢复，不能重排或改写语言表。"));
            }
            for (i = 0; i < original.languages.size(); ++i) {
                if (document.value(SourceDocument::indexed(QStringLiteral("config.298"), i)).toString() != original.languages.at(i)) {
                    return fail(error, QStringLiteral("原生语言设置槽与活动语言表不同，不能用当前注册表覆盖未知配置。"));
                }
            }
            if (project.languages.size() > settingsCount) {
                return fail(error, QStringLiteral("当前语言数量超过原生设置窗口已保存的语言槽，源文件未修改。"));
            }
            appendWord(&settingsLanguageBlock, static_cast<u32>(settingsCount));
            for (i = 0; i < settingsCount; ++i) {
                languageName = i < project.languages.size() ? project.languages.at(i) :
                    document.value(SourceDocument::indexed(QStringLiteral("config.298"), i)).toString();
                if (!appendString(&settingsLanguageBlock, languageName)) {
                    return fail(error, QStringLiteral("原生语言设置窗口名称编码失败。"));
                }
            }
            updates.set(QStringLiteral("config.288.1"), project.languages.size() - 1);
            listStart = document.values.value(QStringLiteral("config.298.count")).start;
            listEnd = document.values.value(QStringLiteral("config.290.0")).start;
            updates.replacements.insert(listStart, qMakePair(listEnd, settingsLanguageBlock));
        }
    }
    if (!project.startupLanguage.isEmpty()) { //20261110-6 change: Native compilation uses the system selection; preserve the separate dialog value.
        if (!nativeProjectInitialLanguageIndex(project, &startupIndex, error)) {
            return false;
        }
        updates.set(QStringLiteral("system.identity.16"), startupIndex);
    }
    for (i = 0; i < original.objects.size(); ++i) {
        originalIndices.insert(original.objects.at(i).id, i);
    }
    for (i = 0; i < project.objects.size(); ++i) {
        if (project.objects.at(i).hasBitButtonProperties && project.objects.at(i).bitButton.macroEnabled &&
            !project.macros.contains(project.objects.at(i).bitButton.macroName)) {
            return fail(error, QStringLiteral("控件绑定的宏不在当前工程注册表中：%1。").arg(project.objects.at(i).bitButton.macroName));
        }
        isText = project.objects.at(i).type == QStringLiteral("text") && project.objects.at(i).hasStaticTextProperties;
        isWord = project.objects.at(i).hasWordButtonProperties; //20261109-6 add
        isLamp = project.objects.at(i).hasLampProperties; //20261109-8 add
        isFunction = project.objects.at(i).hasFunctionButtonProperties; //20261110-0 add
        if ((!isText && (project.objects.at(i).type != QStringLiteral("switch") || !project.objects.at(i).hasBitButtonProperties)) ||
            project.objects.at(i).id.isEmpty() || !project.objects.at(i).group.isEmpty() || ids.contains(project.objects.at(i).id)) {
            return fail(error, QStringLiteral("原生源生成仅支持独立、唯一编号且带完整属性的已验证位按钮、字按钮、指示灯、功能按钮和静态文本控件。")); //20261110-0 change
        }
        ids.insert(project.objects.at(i).id);
        if (!(isText ? project.objects.at(i).staticText.validate(error) : project.objects.at(i).bitButton.validate(error, isFunction))) { //20261110-0 change
            return false;
        }
        if (isWord && (isText || !project.objects.at(i).wordButton.validate(error))) { //20261109-6 add
            return false;
        }
        if (isLamp && (isText || isWord || !project.objects.at(i).lamp.validate(error))) { //20261109-8 add
            return false;
        }
        if (isFunction && (isText || isWord || isLamp || !project.objects.at(i).functionButton.validate(error))) { //20261110-0 add
            return false;
        }
        if (isLamp && project.objects.at(i).lamp.monitorMode == 0) { //20261109-8 add: The original Lamp dialog saves both unchecked groups as multi-state, and its reader rewrites mode zero.
            return fail(error, QStringLiteral("原厂指示灯尚未证实可持久保存无监视模式；请使用双态或多态监视，源文件未写入。"));
        }
        originalIndex = newProject ? -1 : originalIndices.value(project.objects.at(i).id, -1);
        if (originalIndex >= 0) {
            objectDocument = &document;
            originalObject = original.objects.at(originalIndex);
            prefix = document.objectPrefixes.at(originalIndex);
            if (originalObject.type != project.objects.at(i).type || originalObject.hasWordButtonProperties != isWord ||
                originalObject.hasLampProperties != isLamp || originalObject.hasFunctionButtonProperties != isFunction) { //20261110-0 change
                return fail(error, QStringLiteral("原生源已有控件不能直接改换控件种类：%1。").arg(project.objects.at(i).name));
            }
        } else if (isText) {
            if (textSeed.bytes.isEmpty()) {
                if (!textSeedFile.open(QIODevice::ReadOnly)) {
                    return fail(error, QStringLiteral("新增原生静态文本需要已核实seed资源 :/samples/native_text.sm。"));
                }
                textSeed.bytes = textSeedFile.readAll();
                textSeedFile.close();
                if (!textSeed.parse() || !sourceProject(textSeed, &textSeedProject, error) || textSeedProject.objects.size() != 1 ||
                    textSeedProject.objects.first().type != QStringLiteral("text")) {
                    return fail(error, QStringLiteral("嵌入的原生静态文本seed结构无效：%1").arg(textSeed.error));
                }
            }
            objectDocument = &textSeed;
            originalObject = textSeedProject.objects.first();
            originalObject.id = project.objects.at(i).id;
            prefix = textSeed.objectPrefixes.first();
        } else if (isFunction) { //20261110-0 add: System functions require the true subtype-three seed, including its opaque dormant union.
            if (functionSeed.bytes.isEmpty()) {
                if (!functionSeedFile.open(QIODevice::ReadOnly)) {
                    return fail(error, QStringLiteral("新增原生功能按钮需要已核实seed资源 :/native/function-button-default.sm。"));
                }
                functionSeed.bytes = functionSeedFile.readAll();
                functionSeedFile.close();
                if (!functionSeed.parse() || !sourceProject(functionSeed, &functionSeedProject, error) || functionSeedProject.objects.size() != 1 ||
                    !functionSeedProject.objects.first().hasFunctionButtonProperties) {
                    return fail(error, QStringLiteral("嵌入的原生功能按钮seed结构无效：%1").arg(functionSeed.error));
                }
            }
            objectDocument = &functionSeed;
            originalObject = functionSeedProject.objects.first();
            originalObject.id = project.objects.at(i).id;
            prefix = functionSeed.objectPrefixes.first();
        } else if (isLamp) { //20261109-8 add: Lamps require their own original subtype-four source record.
            if (lampSeed.bytes.isEmpty()) {
                if (!lampSeedFile.open(QIODevice::ReadOnly)) {
                    return fail(error, QStringLiteral("新增原生指示灯需要已核实seed资源 :/native/lamp-default.sm。"));
                }
                lampSeed.bytes = lampSeedFile.readAll();
                lampSeedFile.close();
                if (!lampSeed.parse() || !sourceProject(lampSeed, &lampSeedProject, error) || lampSeedProject.objects.size() != 1 ||
                    !lampSeedProject.objects.first().hasLampProperties) {
                    return fail(error, QStringLiteral("嵌入的原生指示灯seed结构无效：%1").arg(lampSeed.error));
                }
            }
            objectDocument = &lampSeed;
            originalObject = lampSeedProject.objects.first();
            originalObject.id = project.objects.at(i).id;
            prefix = lampSeed.objectPrefixes.first();
        } else if (isWord) {
            if (wordSeed.bytes.isEmpty()) {
                if (!wordSeedFile.open(QIODevice::ReadOnly)) {
                    return fail(error, QStringLiteral("新增原生字按钮需要已核实seed资源 :/native/word-button-default.sm。"));
                }
                wordSeed.bytes = wordSeedFile.readAll();
                wordSeedFile.close();
                if (!wordSeed.parse() || !sourceProject(wordSeed, &wordSeedProject, error) || wordSeedProject.objects.size() != 1 ||
                    !wordSeedProject.objects.first().hasWordButtonProperties) {
                    return fail(error, QStringLiteral("嵌入的原生字按钮seed结构无效：%1").arg(wordSeed.error));
                }
            }
            objectDocument = &wordSeed;
            originalObject = wordSeedProject.objects.first();
            originalObject.id = project.objects.at(i).id;
            prefix = wordSeed.objectPrefixes.first();
        } else {
            if (seed.bytes.isEmpty()) {
                if (!seedFile.open(QIODevice::ReadOnly)) {
                    return fail(error, QStringLiteral("新增原生位按钮需要已核实seed资源 :/native/bit-button-default.sm。"));
                }
                seed.bytes = seedFile.readAll();
                seedFile.close();
                if (!seed.parse() || !sourceProject(seed, &seedProject, error) || seedProject.objects.size() != 1) {
                    return fail(error, QStringLiteral("嵌入的原生位按钮seed结构无效：%1").arg(seed.error));
                }
            }
            objectDocument = &seed;
            originalObject = seedProject.objects.first();
            originalObject.id = project.objects.at(i).id; // Source object IDs are editor metadata and are regenerated on import.
            prefix = seed.objectPrefixes.first();
        }
        if (!isText) {
            desiredStates = isFunction ? (project.objects.at(i).functionButton.pressedEffect ? 2 : 1) : isLamp ? project.objects.at(i).lamp.stateCount() : //20261110-0 change
                            (isWord ? project.objects.at(i).wordButton.pressedEffect : project.objects.at(i).bitButton.indicator) ? 2 : 1; //20261109-8 change
            if (objectDocument->value(prefix + QStringLiteral("switch.290.count")).toInt() != desiredStates) {
                if (!resizeButtonSourceStates(*objectDocument, prefix, desiredStates, &resizedBytes, error,
                                                 isLamp ? project.objects.at(i).lamp.monitorMode : -1)) { //20261109-8 change
                    return false;
                }
                objectIndex = originalIndex >= 0 ? originalIndex : 0;
                resizedDocument = std::make_unique<SourceDocument>(resizedBytes);
                if (!resizedDocument->parse() || !sourceProject(*resizedDocument, &resizedProject, error)) {
                    return fail(error, QStringLiteral("重建按钮状态后原生源无法完整解析：%1").arg(resizedDocument->error));
                }
                objectDocument = resizedDocument.get();
                originalObject = resizedProject.objects.at(objectIndex);
                originalObject.id = project.objects.at(i).id;
                prefix = objectDocument->objectPrefixes.at(objectIndex);
            }
        }
        objectUpdates = std::make_unique<SourceUpdates>(*objectDocument);
        if (!(isText ? patchStaticText(originalObject, project.objects.at(i), prefix, project.languages, objectUpdates.get()) :
              isFunction ? patchFunctionButton(originalObject, project.objects.at(i), prefix, project.languages, objectUpdates.get()) : //20261110-0 add
              isLamp ? patchLamp(originalObject, project.objects.at(i), prefix, project.languages, objectUpdates.get()) : //20261109-8 add
              isWord ? patchWordButton(originalObject, project.objects.at(i), prefix, project.languages, objectUpdates.get()) :
                       patchButton(originalObject, project.objects.at(i), prefix, project.languages, objectUpdates.get()))) { //20261109-6 change
            return fail(error, QStringLiteral("控件 %1 含尚未验证可无损保存的原生属性编辑；已取消保存。")
                                   .arg(project.objects.at(i).name));
        }
        objectStart = objectDocument->values.value(prefix + QStringLiteral("movable")).start;
        objectEnd = objectDocument->values.value(prefix + QStringLiteral("z")).end;
        objectBytes = objectUpdates->render(objectStart, objectEnd);
        if (!objectUpdates->error.isEmpty()) {
            return fail(error, objectUpdates->error);
        }
        objectBlock.append(objectBytes);
    }
    listStart = document.values.value(QStringLiteral("page.objects.count")).end;
    listEnd = document.values.value(QStringLiteral("history.1e4.count")).start;
    updates.set(QStringLiteral("page.objects.count"), project.objects.size());
    updates.replacements.insert(listStart, qMakePair(listEnd, objectBlock));
    if (!updates.error.isEmpty()) {
        return fail(error, updates.error);
    }
    result = updates.render();
    if (!updates.error.isEmpty()) {
        return fail(error, updates.error);
    }
    verifiedDocument.bytes = result; //20261110-5 change: Structured output must also remain representable by the supported semantic importer.
    if (!verifiedDocument.parse()) {
        return fail(error, verifiedDocument.error);
    }
    if (!sourceProject(verifiedDocument, &verifiedProject, error)) {
        return false;
    }
    if (!project.startupLanguage.isEmpty() && verifiedProject.activeLanguage != project.startupLanguage) {
        return fail(error, QStringLiteral("原生源启动语言写回后与当前工程请求不一致，已取消保存。"));
    }
    if (verifiedProject.objects.size() != project.objects.size()) {
        return fail(error, QStringLiteral("原生源输出控件数与活动工程不一致，已取消保存。"));
    }
    *data = result;
    clearError(error);
    return true;
}
