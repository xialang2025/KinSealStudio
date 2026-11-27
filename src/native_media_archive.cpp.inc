#include <QDataStream>
#include <QDateTime>
#include <QRegularExpression>
#include <QSet>
#include <QTemporaryFile>
#include <array>

namespace {
/******************************************************************************
 * Function Name   : nativeZipCrc32
 * Description     : Calculate the standard reflected ZIP CRC-32 over one loose file.
 * Input Parameters: data
 * Return Value    : Unsigned 32-bit member checksum.
 * Date            : 2026.11.27
 ******************************************************************************/
u32 nativeZipCrc32(const QByteArray &data) //20261127-12 add
{
    static const std::array<u32, 256> table = [] {
        std::array<u32, 256> result{};
        u32 value;
        for (u32 i = 0; i < result.size(); ++i) {
            value = i;
            for (int bit = 0; bit < 8; ++bit) {
                value = (value >> 1) ^ ((value & 1U) != 0 ? 0xedb88320U : 0);
            }
            result[i] = value;
        }
        return result;
    }();
    u32 crc = 0xffffffffU;

    for (char byte : data) {
        crc = (crc >> 8) ^ table[(crc ^ u8(byte)) & 0xffU];
    }
    return ~crc;
}
}

/******************************************************************************
 * Function Name   : packNativeMediaArchive
 * Description     : Recover the native KNS ZIP32 container from a complete compiler output directory.
 * Input Parameters: sourceDirectory, destination, error
 * Return Value    : True after a complete new file has been published.
 * Date            : 2026.11.27
 ******************************************************************************/
bool packNativeMediaArchive(const QString &sourceDirectory, const QString &destination, QString *error) //20261127-12 add
{
    const QSet<QString> required = {QStringLiteral("AddrPara.dat"), QStringLiteral("AlarmMuti.dat"),
        QStringLiteral("AlarmPara.dat"), QStringLiteral("Collect.dat"), QStringLiteral("Curve.dat"),
        QStringLiteral("load.bin"), QStringLiteral("MarcoVar.dat"), QStringLiteral("PicLenPara.dat"),
        QStringLiteral("SheetPara.dat"), QStringLiteral("SheetPixel.dat"), QStringLiteral("SysPara.dat"),
        QStringLiteral("upLoad.knr"), QStringLiteral("Usbload.bin")};
    const QSet<QString> optional = {QStringLiteral("ControlPara.dat"), QStringLiteral("ControlPixel.dat"),
        QStringLiteral("MarcoRun.dat"), QStringLiteral("Message.dat"), QStringLiteral("SimMessage.dat"),
        QStringLiteral("Recipe.dat"), QStringLiteral("LogoPixel.dat"), QStringLiteral("AMZ070W01.bin"),
        QStringLiteral("AMT070W124-L800X480R.bin"), QStringLiteral("AMZ101W01.bin"),
        QStringLiteral("HZ.bin"), QStringLiteral("HZ_ASmall.bin")};
    const QRegularExpression driver(QStringLiteral("^[a-zA-Z0-9_. -]+_com[0-9]+\\.bin$"),
                                    QRegularExpression::CaseInsensitiveOption);
    const QFileInfo source(sourceDirectory), target(destination);
    const QDir directory(source.absoluteFilePath());
    QStringList members;
    QTemporaryFile output(target.absolutePath() + QStringLiteral("/.kinseal-media-XXXXXX"));
    QByteArray central, name, content, encoded;
    QDataStream headers(&central, QIODevice::WriteOnly);
    QDataStream archive(&output);
    qint64 total = 0;
    u16 dosDate, dosTime;
    u32 crc, offset, centralOffset;

    if (!source.isDir() || source.isSymLink() || target.suffix().compare(QStringLiteral("kns"), Qt::CaseInsensitive) != 0 ||
        target.exists() || target.isSymLink() || !target.dir().exists()) {
        return packageFailure(error, QStringLiteral("请选择编译散件目录和一个尚不存在的 .kns 输出文件。"));
    }
    for (const QFileInfo &entry : directory.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden)) {
        const QString ext = entry.suffix().toLower();
        if (ext != QStringLiteral("dat") && ext != QStringLiteral("knr") && ext != QStringLiteral("bin")) {
            continue;
        }
        if (!entry.isFile() || entry.isSymLink() ||
            !(required.contains(entry.fileName()) || optional.contains(entry.fileName()) || driver.match(entry.fileName()).hasMatch())) {
            return packageFailure(error, QStringLiteral("无法识别的编译散件：%1").arg(entry.fileName()));
        }
        members.append(entry.fileName());
    }
    for (const QString &member : required) {
        if (!members.contains(member)) {
            return packageFailure(error, QStringLiteral("介质包缺少编译散件：%1").arg(member));
        }
    }
    members.sort(Qt::CaseInsensitive);
    if (!output.open()) {
        return packageFailure(error, output.errorString());
    }
    archive.setByteOrder(QDataStream::LittleEndian);
    headers.setByteOrder(QDataStream::LittleEndian);
    for (const QString &member : members) {
        const QFileInfo entry(directory.filePath(member));
        const QDateTime modified = entry.lastModified().toLocalTime();
        const QDate date = modified.date();
        const QTime time = modified.time();
        QFile file(entry.absoluteFilePath());
        if (!modified.isValid() || date.year() < 1980 || date.year() > 2107 ||
            entry.size() < 0 || entry.size() > 64 * 1024 * 1024 || total + entry.size() > 128 * 1024 * 1024 ||
            !file.open(QIODevice::ReadOnly)) {
            return packageFailure(error, QStringLiteral("编译散件的日期、长度或读取状态无效：%1").arg(member));
        }
        content = file.read(entry.size() + 1);
        if (file.error() != QFile::NoError || content.size() != entry.size() ||
            QFileInfo(file).lastModified() != modified) {
            return packageFailure(error, QStringLiteral("封装时编译散件发生变化：%1").arg(member));
        }
        total += content.size();
        // Strip the Qt length prefix, the zlib header, and Adler-32 to retain only raw deflate.
        encoded = content.isEmpty() ? QByteArray::fromHex("0300") : qCompress(content, 6);
        if (!content.isEmpty()) {
            if (encoded.size() <= 10) {
                return packageFailure(error, QStringLiteral("散件压缩失败：%1").arg(member));
            }
            encoded = encoded.mid(6, encoded.size() - 10);
        }
        name = member.toLatin1();
        crc = nativeZipCrc32(content);
        dosDate = u16((date.year() - 1980) << 9 | date.month() << 5 | date.day());
        dosTime = u16(time.hour() << 11 | time.minute() << 5 | time.second() / 2);
        offset = u32(output.pos());
        archive << u32(0x04034b50) << u16(20) << u16(8) << u16(8) << dosTime << dosDate
                << crc << u32(encoded.size()) << u32(content.size()) << u16(name.size()) << u16(0);
        archive.writeRawData(name.constData(), name.size());
        archive.writeRawData(encoded.constData(), encoded.size());
        archive << u32(0x08074b50) << crc << u32(encoded.size()) << u32(content.size());
        headers << u32(0x02014b50) << u16(0x031e) << u16(20) << u16(8) << u16(8) << dosTime << dosDate
                << crc << u32(encoded.size()) << u32(content.size()) << u16(name.size()) << u16(0)
                << u16(0) << u16(0) << u16(0) << u32(0x81b60000) << offset;
        headers.writeRawData(name.constData(), name.size());
    }
    centralOffset = u32(output.pos());
    archive.writeRawData(central.constData(), central.size());
    archive << u32(0x06054b50) << u16(0) << u16(0) << u16(members.size()) << u16(members.size())
            << u32(central.size()) << centralOffset << u16(0);
    if (archive.status() != QDataStream::Ok || headers.status() != QDataStream::Ok ||
        !output.flush() || !output.rename(target.absoluteFilePath())) {
        return packageFailure(error, output.errorString());
    }
    output.setAutoRemove(false);
    return true;
}
