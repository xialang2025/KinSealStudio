#include "device_package.h"
#include "device_transfer_plan.h"
#include "native_project_codec.h"
#include "lzokay.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QtEndian>
#include <memory>

using u16 = unsigned short;
using u32 = unsigned int;

//20261113-11 add: Names are fixed by segment identity, never supplied by a manifest path.
static const char *packageNames[] = {"SysPara.dat","SheetPara.dat","AlarmPara.dat","Collect.dat","AddrPara.dat",
    "PicLenPara.dat","Curve.dat","upLoad.knr","SheetPixel.dat","Modbus_RTU_com1.bin",
    "ControlPara.dat","ControlPixel.dat","MarcoVar.dat","MarcoRun.dat"};

/******************************************************************************
 * Function Name   : packageFailure
 * Description     : Return an offline diagnostic without touching any device or published directory.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool packageFailure(QString *error, const QString &message) //20261113-11 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : packageSha
 * Description     : Hash the exact file bytes used for publication and re-read checks.
 * Input Parameters: bytes
 * Return Value    : Lowercase hexadecimal SHA256.
 * Date            : 2026.11.13
 ******************************************************************************/
static QString packageSha(const QByteArray &bytes) //20261113-11 add
{
    return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());
}

/******************************************************************************
 * Function Name   : packageU32
 * Description     : Read a bounded little-endian index without relying on QByteArray's empty-data specialization.
 * Input Parameters: bytes, offset
 * Return Value    : The unsigned field, or zero outside its validated containing header.
 * Date            : 2026.11.13
 ******************************************************************************/
static u32 packageU32(const QByteArray &bytes, int offset) //20261113-11 add
{
    u32 value = 0;
    int index;

    if (offset < 0 || offset > bytes.size()-4) { return 0; }
    for (index=0;index<4;++index) { value |= u32(static_cast<unsigned char>(bytes.at(offset+index))) << (index*8); }
    return value;
}

/******************************************************************************
 * Function Name   : packageSnapshotKind
 * Description     : Bound decompression and distinguish a valid Lite model from a fully parsed supported native source.
 * Input Parameters: compressed, kind, error
 * Return Value    : True with an honest snapshot format; no USB or native compiler is called.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool packageSnapshotKind(const QByteArray &compressed, QString *kind, QString *error) //20261113-11 add
{
    QByteArray source;
    QJsonDocument json;
    HmiProject project;

    if (compressed.size() < 4 || qFromBigEndian<u32>(compressed.constData()) > 256U*1024U*1024U)
    { return packageFailure(error,QStringLiteral("源快照缺少长度或超过Lite的256MiB解压保护上限。")); }
    source = qUncompress(compressed);
    if (source.isEmpty()) { return packageFailure(error,QStringLiteral("源快照解压失败。")); }
    json = QJsonDocument::fromJson(source);
    if (json.isObject() && json.object().value(QStringLiteral("format")).toString() == QStringLiteral("KinSealStudio-Prototype"))
    {
        if (!HmiProject::fromJson(json.object(),&project,error)) { return false; }
        *kind = QStringLiteral("lite-project-json"); return true;
    }
    if (!validateNativeProjectSourceStructure(source,error)) { return false; }
    *kind = QStringLiteral("native-sm"); return true;
}

/******************************************************************************
 * Function Name   : decodePackageFrame
 * Description     : Verify one independently compressed RGB565 block against its bounded raw length.
 * Input Parameters: compressed, rawLength
 * Return Value    : True for a complete LZO frame of the declared size.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool decodePackageFrame(const QByteArray &compressed, u32 rawLength) //20261113-11 add
{
    QByteArray decoded;
    std::size_t actual = 0;

    if (compressed.isEmpty() || rawLength == 0 || rawLength > 768000 || rawLength % 2 != 0) { return false; }
    decoded.resize(rawLength);
    return lzokay::decompress(reinterpret_cast<const unsigned char *>(compressed.constData()),std::size_t(compressed.size()),
        reinterpret_cast<unsigned char *>(decoded.data()),std::size_t(decoded.size()),actual) == lzokay::EResult::Success && actual == rawLength;
}

/******************************************************************************
 * Function Name   : validateDevicePayloadStructure
 * Description     : Cross-check segment counts, control raw offsets, every frame, padding, and all single-page image indexes.
 * Input Parameters: payloads, error
 * Return Value    : True when the established ten/twelve/fourteen-segment structure closes.
 * Date            : 2026.11.13
 ******************************************************************************/
bool validateDevicePayloadStructure(const QVector<QByteArray> &payloads, QString *error) //20261113-11 add
{
    QVector<HmiDownloadStep> steps;
    QByteArray record;
    QByteArray chunk;
    const QByteArray *lengths;
    u32 pictureCount;
    u32 pictureIndex = 0;
    u32 compressedOffset = 0;
    u32 rawOffset = 0;
    u32 frameBytes;
    u32 objectBytes;
    u32 padded;
    u32 actual;
    u32 frame;
    u32 type;
    u16 subtype;
    int index;
    int count;
    int slot;
    QString snapshotKind;

    if (!buildDeviceTransferPlan(payloads,&steps,error)) { return false; }
    if (!packageSnapshotKind(payloads[7],&snapshotKind,error)) { return false; }
    if (payloads[0].size() != 4998 || payloads[1].size() != 72 || payloads[5].size() < 2400 || (payloads[5].size()-2400)%12 != 0)
    { return packageFailure(error,QStringLiteral("设备段头长度或图像索引长度错误。")); }
    if (!decodePackageFrame(payloads[8],768000)) { return packageFailure(error,QStringLiteral("页面背景LZO解压长度错误。")); }
    lengths = &payloads[5];
    pictureCount = packageU32(*lengths,0x640);
    if (pictureCount != u32((lengths->size()-2400)/12) || packageU32(*lengths,0) != u32(payloads[8].size()))
    { return packageFailure(error,QStringLiteral("页面图像计数或背景长度不一致。")); }
    for (index=1;index<200;++index)
    {
        for (slot=0;slot<3;++slot)
        {
            if (packageU32(*lengths,slot*800+index*4) != 0)
            { return packageFailure(error,QStringLiteral("单页设备包包含未支持的其它页面索引。")); }
        }
    }
    count = static_cast<unsigned char>(payloads[1][2]);
    if (payloads.size() == 10)
    {
        if (count != 0 || pictureCount != 0 || packageU32(*lengths,0x320) != 0 || qFromLittleEndian<u32>(payloads[1].constData()+0x44) != 0)
        { return packageFailure(error,QStringLiteral("静态十段包残留动态控件或图片索引。")); }
        return true;
    }
    if (count == 0 || payloads[10].size() != count*640 || packageU32(*lengths,0x320) != u32(payloads[11].size()))
    { return packageFailure(error,QStringLiteral("控制记录数量或控制像素长度不一致。")); }
    for (index=0;index<count;++index)
    {
        record = payloads[10].mid(index*640,640);
        type = static_cast<unsigned char>(record.at(0)); subtype = qFromLittleEndian<u16>(record.constData()+0x106); // The following byte is a macro flag, not the high byte of a u32 type.
        if ((type != 10 && type != 28) || (type == 10 && subtype != 0 && subtype != 1 && subtype != 3 && subtype != 4) ||
            u32(qFromLittleEndian<u16>(record.constData()+4))+qFromLittleEndian<u16>(record.constData()+8) > 800 ||
            u32(qFromLittleEndian<u16>(record.constData()+6))+qFromLittleEndian<u16>(record.constData()+10) > 480)
        { return packageFailure(error,QStringLiteral("控制记录type/subtype或屏内几何尚未支持。")); }
        frameBytes = u32(qFromLittleEndian<u16>(record.constData()+8))*qFromLittleEndian<u16>(record.constData()+10)*2U;
        objectBytes = qFromLittleEndian<u32>(record.constData()+0x10);
        if (frameBytes == 0 || frameBytes > 768000 || objectBytes == 0 || objectBytes%frameBytes != 0 || qFromLittleEndian<u32>(record.constData()+0x0c) != rawOffset || quint64(rawOffset)+objectBytes > 0xffffffffULL)
        { return packageFailure(error,QStringLiteral("控件%1的rawOffset/rawBytes或帧尺寸错误。").arg(index)); }
        for (frame=0;frame<objectBytes/frameBytes;++frame)
        {
            if (pictureIndex >= pictureCount) { return packageFailure(error,QStringLiteral("控件帧数量超过图片长度表。")); }
            slot = 2400+int(pictureIndex)*12;
            padded = packageU32(*lengths,slot+4);
            actual = packageU32(*lengths,slot+8);
            if (packageU32(*lengths,slot) != 0 || actual == 0 || padded%32 != 0 || padded != ((quint64(actual)+31)&~quint64(31)) || quint64(compressedOffset)+padded > quint64(payloads[11].size()))
            { return packageFailure(error,QStringLiteral("控件图块长度、编码或32字节补齐错误。")); }
            chunk = payloads[11].mid(compressedOffset,actual);
            if (!decodePackageFrame(chunk,frameBytes) || payloads[11].mid(compressedOffset+actual,padded-actual) != QByteArray(padded-actual,char(0)))
            { return packageFailure(error,QStringLiteral("控件图块解压或填充字节错误。")); }
            compressedOffset += padded; ++pictureIndex;
        }
        rawOffset += objectBytes;
    }
    if (pictureIndex != pictureCount || compressedOffset != u32(payloads[11].size()) || rawOffset != qFromLittleEndian<u32>(payloads[1].constData()+0x44))
    { return packageFailure(error,QStringLiteral("图帧累计数量、压缩偏移或页面raw总量不一致。")); }
    return true;
}

/******************************************************************************
 * Function Name   : verifyDevicePackage
 * Description     : Re-read the exact allowed segment set, hashes, target, structure and offline completion marker.
 * Input Parameters: directory, manifest, error
 * Return Value    : True with verified manifest; caller manifest survives failure.
 * Date            : 2026.11.13
 ******************************************************************************/
bool verifyDevicePackage(const QString &directory, QJsonObject *manifest, QString *error) //20261113-11 add
{
    QDir dir(directory);
    QFile file(dir.filePath(QStringLiteral("compiler-manifest.json")));
    QJsonObject candidate;
    QJsonObject entries;
    QJsonObject entry;
    QVector<HmiDownloadStep> steps;
    HmiDownloadStep step;
    QJsonArray transfer;
    QVector<QByteArray> payloads;
    QByteArray bytes;
    QStringList expected;
    QStringList actual;
    QString name;
    QString snapshotKind;
    int count;
    int index;

    if (!file.open(QIODevice::ReadOnly)) { return packageFailure(error,QStringLiteral("缺少设备包完成清单。")); }
    candidate = QJsonDocument::fromJson(file.readAll()).object(); entries = candidate.value(QStringLiteral("files")).toObject(); count = entries.size();
    if (candidate.value(QStringLiteral("packageFormat")).toString() != QStringLiteral("kinseal-device-package-v1") || !candidate.value(QStringLiteral("complete")).isBool() || !candidate.value(QStringLiteral("complete")).toBool() || candidate.value(QStringLiteral("targetModel")).toString() != QStringLiteral("AMT070W124-L800X480R") || !candidate.value(QStringLiteral("deviceAcknowledged")).isBool() || candidate.value(QStringLiteral("deviceAcknowledged")).toBool() || !candidate.value(QStringLiteral("usbAccessed")).isBool() || candidate.value(QStringLiteral("usbAccessed")).toBool() || (count != 10 && count != 12 && count != 14))
    { return packageFailure(error,QStringLiteral("设备包标识、目标、离线状态或段数量错误。")); }
    expected.append(QStringLiteral("compiler-manifest.json"));
    for (index=0;index<count;++index)
    {
        name = QString::fromLatin1(packageNames[index]); expected.append(name); entry = entries.value(name).toObject();
        file.close(); file.setFileName(dir.filePath(name));
        if (QFileInfo(file).isSymLink() || !file.open(QIODevice::ReadOnly)) { return packageFailure(error,QStringLiteral("设备包文件缺失或为重定向链接：%1").arg(name)); }
        bytes = file.readAll();
        if (!entry.value(QStringLiteral("length")).isDouble() || entry.value(QStringLiteral("length")).toDouble() != double(bytes.size()) || entry.value(QStringLiteral("sha256")).toString() != packageSha(bytes))
        { return packageFailure(error,QStringLiteral("设备包文件长度或SHA256错误：%1").arg(name)); }
        payloads.append(bytes);
    }
    actual = dir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::Hidden|QDir::System); expected.sort(); actual.sort();
    if (actual != expected) { return packageFailure(error,QStringLiteral("设备包包含额外旧文件或未知目录。")); }
    if (!validateDevicePayloadStructure(payloads,error)) { return false; }
    if (!packageSnapshotKind(payloads[7],&snapshotKind,error) || candidate.value(QStringLiteral("sourceSnapshot")).toString() != snapshotKind)
    { return packageFailure(error,QStringLiteral("清单源快照身份与实际内容不一致。")); }
    if (snapshotKind == QStringLiteral("lite-project-json") && candidate.value(QStringLiteral("inputProjectSha256")).toString() != packageSha(QJsonDocument::fromJson(qUncompress(payloads[7])).toJson(QJsonDocument::Compact)))
    { return packageFailure(error,QStringLiteral("源快照与输入工程SHA256不一致。")); }
    if (!buildDeviceTransferPlan(payloads,&steps,error)) { return false; }
    Q_FOREACH (step,steps) { transfer.append(QJsonObject{{QStringLiteral("command"),int(step.command)},{QStringLiteral("name"),step.name},
        {QStringLiteral("optionsHex"),QString::fromLatin1(step.options.toHex())},{QStringLiteral("wireBytes"),int(step.bytes.size())},{QStringLiteral("wireSha256"),packageSha(step.bytes)}}); }
    if (candidate.value(QStringLiteral("transferPlan")).toArray() != transfer)
    { return packageFailure(error,QStringLiteral("清单传输计划与实际设备段不一致。")); }
    if (manifest != nullptr) { *manifest = candidate; }
    return true;
}

/******************************************************************************
 * Function Name   : writePackageFile
 * Description     : Commit one staging file; a directory is published only after all re-read checks pass.
 * Input Parameters: path, bytes, error
 * Return Value    : True after complete write and file commit.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool writePackageFile(const QString &path, const QByteArray &bytes, QString *error) //20261113-11 add
{
    QSaveFile file(path);

    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
    { return packageFailure(error,QStringLiteral("暂存文件写入失败：%1；%2").arg(path,file.errorString())); }
    return true;
}

/******************************************************************************
 * Function Name   : publishDevicePackage
 * Description     : Stage on the destination volume, verify all bytes, then rename to a new unoccupied package path.
 * Input Parameters: project, payloads, directory, error, writer
 * Return Value    : True for a newly published package; an existing destination is never changed.
 * Date            : 2026.11.13
 ******************************************************************************/
bool publishDevicePackage(const HmiProject &project, const QVector<QByteArray> &payloads, const QString &directory,
                          QString *error, const DevicePackageWriter &writer) //20261113-11 add
{
    QFileInfo target(directory);
    QDir parent(target.absolutePath());
    QJsonObject metadata;
    QJsonObject entries;
    QJsonObject entry;
    QJsonDocument snapshot;
    QVector<HmiDownloadStep> steps;
    QJsonArray transfer;
    HmiDownloadStep step;
    QFile program(QCoreApplication::applicationFilePath());
    DevicePackageWriter write = writer ? writer : writePackageFile;
    std::unique_ptr<QTemporaryDir> staging;
    QString name;
    int index;

    if (directory.trimmed().isEmpty() || target.exists() || target.isSymLink() || target.fileName().isEmpty())
    { return packageFailure(error,QStringLiteral("请使用尚不存在的新包目录；现有目录和有效包保持不变。")); }
    if (!validateDevicePayloadStructure(payloads,error) || !buildDeviceTransferPlan(payloads,&steps,error)) { return false; }
    if (!parent.exists() && !QDir().mkpath(parent.absolutePath())) { return packageFailure(error,QStringLiteral("无法建立输出父目录。")); }
    staging = std::make_unique<QTemporaryDir>(parent.filePath(QStringLiteral(".kinseal-compile-XXXXXX")));
    if (!staging->isValid()) { return packageFailure(error,QStringLiteral("无法在输出所在卷建立暂存目录。")); }
    for (index=0;index<payloads.size();++index)
    {
        name = QString::fromLatin1(packageNames[index]);
        if (!write(QDir(staging->path()).filePath(name),payloads[index],error)) { return false; }
        entry = {{QStringLiteral("length"),int(payloads[index].size())},{QStringLiteral("sha256"),packageSha(payloads[index])}};
        entries.insert(name,entry);
    }
    Q_FOREACH (step,steps) { transfer.append(QJsonObject{{QStringLiteral("command"),int(step.command)},{QStringLiteral("name"),step.name},
        {QStringLiteral("optionsHex"),QString::fromLatin1(step.options.toHex())},{QStringLiteral("wireBytes"),int(step.bytes.size())},{QStringLiteral("wireSha256"),packageSha(step.bytes)}}); }
    snapshot = QJsonDocument::fromJson(qUncompress(payloads[7]));
    metadata = {{QStringLiteral("packageFormat"),QStringLiteral("kinseal-device-package-v1")},{QStringLiteral("complete"),true},
        {QStringLiteral("files"),entries},{QStringLiteral("targetModel"),project.targetModel},{QStringLiteral("deviceAcknowledged"),false},
        {QStringLiteral("usbAccessed"),false},{QStringLiteral("firmwareVersionConfirmed"),false},
        {QStringLiteral("inputProjectSha256"),packageSha(QJsonDocument(project.toJson()).toJson(QJsonDocument::Compact))},
        {QStringLiteral("sourceSnapshot"),snapshot.isObject() && snapshot.object().value(QStringLiteral("format")).toString() == QStringLiteral("KinSealStudio-Prototype") ? QStringLiteral("lite-project-json") : QStringLiteral("native-sm")},
        {QStringLiteral("transferPlan"),transfer}};
    if (program.open(QIODevice::ReadOnly)) { metadata.insert(QStringLiteral("compilerExecutableSha256"),packageSha(program.readAll())); }
    if (!write(QDir(staging->path()).filePath(QStringLiteral("compiler-manifest.json")),QJsonDocument(metadata).toJson(QJsonDocument::Indented),error) || !verifyDevicePackage(staging->path(),nullptr,error)) { return false; }
    if (QFileInfo::exists(target.absoluteFilePath()) || !parent.rename(QFileInfo(staging->path()).fileName(),target.fileName()))
    { return packageFailure(error,QStringLiteral("包发布失败或目标被占用；已有目录保持不变，暂存输出将回收。")); }
    staging->setAutoRemove(false);
    return true;
}
