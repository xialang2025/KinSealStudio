#include "device_package.h"
#include "device_target.h" //20261122-3 add
#include "device_transfer_plan.h"
#include "native_project_codec.h"
#include "lzokay.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDataStream> //20261127-12 add
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstring> //20261128-12 add
#include <memory>

using u16 = unsigned short;
using u32 = unsigned int;
using u8 = unsigned char;

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
static u32 packageU32(const QByteArray &bytes, qsizetype offset) //20261128-12 change
{
    u32 value = 0;
    int index;

    if (offset < 0 || offset > bytes.size()-4) { return 0; }
    for (index=0;index<4;++index) { value |= u32(static_cast<unsigned char>(bytes.at(offset+index))) << (index*8); }
    return value;
}

/******************************************************************************
 * Function Name   : validateMacroSchedulePayloads
 * Description     : Validate that schedule rows address the actual macro image and have safe local trigger offsets.
 * Input Parameters: payloads, staticMacros, error
 * Return Value    : True if there is no schedule or all serialized schedule rows can be resolved.
 * Date            : 2026.11.23
 ******************************************************************************/
static bool validateMacroSchedulePayloads(const QVector<QByteArray> &payloads, bool staticMacros,
                                          QString *error) //20261123-22 add
{
    const QByteArray &system = payloads.at(0);
    const int global = u8(system.at(0x528));
    const int initial = u8(system.at(0x82c));
    const int macroVarIndex = staticMacros ? 10 : 12;
    int macroCount;
    int row;
    int trigger;
    u16 mode;
    u16 bodySize;
    qsizetype imageSize;

    if (global == 0 && initial == 0)
    {
        return true;
    }
    if (global > 64 || initial > 16 || payloads.size() != macroVarIndex + 2 ||
        payloads.last().size() <= 1024 || payloads.last().size() > 0x20000)
    {
        return packageFailure(error, QStringLiteral("调度宏行数无效或设备包缺少宏编译数据。"));
    }
    macroCount = u8(payloads.last().at(0));
    if (macroCount < 1 || macroCount > 100 || payloads.at(macroVarIndex).size() != macroCount * 904)
    {
        return packageFailure(error, QStringLiteral("调度宏数量和宏变量长度不匹配。"));
    }
    imageSize = 1024;
    for (row = 0; row < macroCount; ++row)
    {
        bodySize = qFromLittleEndian<u16>(payloads.last().constData() + 102 + row * 2);
        if (bodySize == 0)
        {
            return packageFailure(error, QStringLiteral("调度宏包包含没有机器码的宏。"));
        }
        imageSize += bodySize;
    }
    if (imageSize != payloads.last().size())
    {
        return packageFailure(error, QStringLiteral("调度宏包的机器码长度与目录不匹配。"));
    }
    for (row = 0; row < initial; ++row)
    {
        if (u8(system.at(0x844 + row)) >= macroCount)
        {
            return packageFailure(error, QStringLiteral("第%1条初始化宏的索引超出已编译宏列表。").arg(row + 1));
        }
    }
    for (row = 0; row < global; ++row)
    {
        trigger = 0x882 + row * 40;
        mode = qFromLittleEndian<u16>(system.constData() + trigger);
        if (u8(system.at(0x52c + row * 12)) >= macroCount || packageU32(system, 0x534 + row * 12) > 65535 ||
            mode > 1 || (mode == 1 && (u8(system.at(trigger + 6)) != 100 || packageU32(system, trigger + 12) > 4096)))
        {
            return packageFailure(error, QStringLiteral("第%1条全局宏的索引、次数或本地触发地址无效。").arg(row + 1));
        }
    }
    return true;
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
 * Function Name   : nativeControlFrameTail
 * Description     : Read the unmodified compressed-output tail before overlaying the next LZO frame at offset zero.
 * Input Parameters: workspace seeded with the compressed background, actual compressed bytes, aligned length
 * Return Value    : Exactly the native padding bytes for this frame.
 * Date            : 2026.11.28
 ******************************************************************************/
static QByteArray nativeControlFrameTail(QByteArray *workspace, QByteArrayView frame, qsizetype aligned) //20261128-12 add
{
    const qsizetype actual = frame.size();
    const qsizetype retained = qMin(workspace->size(), aligned) - actual;
    QByteArray tail(aligned - actual, char(0));

    if (retained > 0) {
        std::memcpy(tail.data(), workspace->constData() + actual, size_t(retained));
    }
    // Shorter frames leave all older bytes above their end available to later frames.
    if (workspace->size() < actual) {
        workspace->resize(actual);
    }
    std::memcpy(workspace->data(), frame.data(), size_t(actual));
    return tail;
}

/******************************************************************************
 * Function Name   : applyNativeControlPixelPadding
 * Description     : Finalize sprite frame alignment after the page background and all actual compressed lengths are known.
 * Input Parameters: compressed background, 12-byte control image records without the page header, pixels, error
 * Return Value    : True after replacing only alignment bytes; invalid indexes leave the caller's pixels intact.
 * Date            : 2026.11.28
 ******************************************************************************/
bool applyNativeControlPixelPadding(const QByteArray &background, const QByteArray &records,
                                    QByteArray *pixels, QString *error) //20261128-12 add
{
    QByteArray workspace = background;
    QByteArray result;
    QByteArray tail;
    qsizetype offset = 0;
    qsizetype index;
    u32 actual;
    u32 aligned;

    if (!pixels || records.size() % 12 != 0) {
        return packageFailure(error, QStringLiteral("控件图帧的补齐索引无效。"));
    }
    result = *pixels;
    for (index = 0; index < records.size(); index += 12) {
        aligned = packageU32(records, index + 4);
        actual = packageU32(records, index + 8);
        if (packageU32(records, index) != 0 || !actual ||
            ((quint64(actual) + 31) & ~quint64(31)) != aligned || quint64(result.size() - offset) < aligned) {
            return packageFailure(error, QStringLiteral("控件图帧长度与补齐索引不符。"));
        }
        tail = nativeControlFrameTail(&workspace, QByteArrayView(*pixels).sliced(offset, actual), aligned);
        if (!tail.isEmpty()) {
            std::memcpy(result.data() + offset + actual, tail.constData(), size_t(tail.size()));
        }
        offset += aligned;
    }
    if (offset != result.size()) {
        return packageFailure(error, QStringLiteral("控件像素段末尾存在未索引的字节。"));
    }
    *pixels = result;
    return true;
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
    const bool staticMacros = hasStaticMacroPayloads(payloads); //20261123-22 add
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
    QByteArray spriteWorkspace; //20261128-12 add
    QByteArray observedTail;
    QByteArray nativeTail;
    bool allNativeTails = true;
    bool allZeroTails = true;

    if (!staticMacros && !buildDeviceTransferPlan(payloads,&steps,error)) { return false; } //20261123-22 change
    if (staticMacros)
    {
        // The native files establish an offline shape. The live planner still requires its known segment combinations.
        for (const QByteArray &bytes : payloads)
        {
            if (bytes.isEmpty() || bytes.size() > 0x7fffffff)
            {
                return packageFailure(error, QStringLiteral("静态调度宏包包含空段或超长段。"));
            }
        }
    }
    if (!packageSnapshotKind(payloads[7],&snapshotKind,error)) { return false; }
    if (payloads[0].size() != 4998 || payloads[1].size() != 72 || payloads[5].size() < 2400 || (payloads[5].size()-2400)%12 != 0)
    { return packageFailure(error,QStringLiteral("设备段头长度或图像索引长度错误。")); }
    if (!validateMacroSchedulePayloads(payloads, staticMacros, error)) { return false; } //20261123-22 add
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
    if (payloads.size() == 10 || staticMacros) //20261123-22 change
    {
        if (count != 0 || pictureCount != 0 || packageU32(*lengths,0x320) != 0 || qFromLittleEndian<u32>(payloads[1].constData()+0x44) != 0)
        { return packageFailure(error,QStringLiteral("静态包残留动态控件或图片索引。")); }
        return true;
    }
    if (count == 0 || payloads[10].size() != count*640 || packageU32(*lengths,0x320) != u32(payloads[11].size()))
    { return packageFailure(error,QStringLiteral("控制记录数量或控制像素长度不一致。")); }
    spriteWorkspace = payloads[8]; //20261128-12 add: The original compressor writes the page immediately before the first control frame.
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
            if (!decodePackageFrame(chunk,frameBytes)) { //20261128-12 change
                return packageFailure(error,QStringLiteral("控件图块解压失败。"));
            }
            observedTail = payloads[11].mid(compressedOffset + actual, padded - actual); //20261128-12 change
            nativeTail = nativeControlFrameTail(&spriteWorkspace, chunk, padded);
            allNativeTails = allNativeTails && observedTail == nativeTail;
            allZeroTails = allZeroTails && observedTail.count(char(0)) == observedTail.size();
            // One encoding must explain the entire payload, including every frame of every control.
            if (!allNativeTails && !allZeroTails) {
                return packageFailure(error, QStringLiteral("控件图块的补齐字节不符合整包编码。"));
            }
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
    QJsonDocument snapshot; //20261122-3 add
    QList<NativeSourceFieldSpan> nativeFields; //20261127-12 add
    QByteArray nativeSource;
    QString nativeTarget;
    QByteArray inputDigest;
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
    bool staticMacros; //20261123-22 add

    if (!file.open(QIODevice::ReadOnly)) { return packageFailure(error,QStringLiteral("缺少设备包完成清单。")); }
    candidate = QJsonDocument::fromJson(file.readAll()).object(); entries = candidate.value(QStringLiteral("files")).toObject(); count = entries.size();
    if (candidate.value(QStringLiteral("packageFormat")).toString() != QStringLiteral("kinseal-device-package-v1") ||
        !candidate.value(QStringLiteral("complete")).isBool() || !candidate.value(QStringLiteral("complete")).toBool() ||
        deviceTargetIdentityByte(candidate.value(QStringLiteral("targetModel")).toString()) < 0 || //20261122-3 change
        !candidate.value(QStringLiteral("deviceAcknowledged")).isBool() || candidate.value(QStringLiteral("deviceAcknowledged")).toBool() ||
        !candidate.value(QStringLiteral("usbAccessed")).isBool() || candidate.value(QStringLiteral("usbAccessed")).toBool() ||
        (count != 10 && count != 12 && count != 14))
    { return packageFailure(error,QStringLiteral("设备包标识、目标、离线状态或段数量错误。")); }
    staticMacros = count == 12 && entries.contains(QStringLiteral("MarcoVar.dat")) && entries.contains(QStringLiteral("MarcoRun.dat"));
    expected.append(QStringLiteral("compiler-manifest.json"));
    for (index=0;index<count;++index)
    {
        name = QString::fromLatin1(packageNames[index + (staticMacros && index >= 10 ? 2 : 0)]); //20261123-22 change
        expected.append(name); entry = entries.value(name).toObject();
        file.close(); file.setFileName(dir.filePath(name));
        if (QFileInfo(file).isSymLink() || !file.open(QIODevice::ReadOnly)) { return packageFailure(error,QStringLiteral("设备包文件缺失或为重定向链接：%1").arg(name)); }
        bytes = file.readAll();
        if (!entry.value(QStringLiteral("length")).isDouble() || entry.value(QStringLiteral("length")).toDouble() != double(bytes.size()) || entry.value(QStringLiteral("sha256")).toString() != packageSha(bytes))
        { return packageFailure(error,QStringLiteral("设备包文件长度或SHA256错误：%1").arg(name)); }
        payloads.append(bytes);
    }
    actual = dir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::Hidden|QDir::System); expected.sort(); actual.sort();
    if (actual != expected) { return packageFailure(error,QStringLiteral("设备包包含额外旧文件或未知目录。")); }
    if (staticMacros != hasStaticMacroPayloads(payloads) ||
        (staticMacros && candidate.value(QStringLiteral("offlineOnly")) != QJsonValue(true))) //20261123-22 add
    { return packageFailure(error, QStringLiteral("设备包静态调度布局、文件名或离线属性不一致。")); }
    if (!validateDevicePayloadStructure(payloads,error)) { return false; }
    if (!packageSnapshotKind(payloads[7],&snapshotKind,error) || candidate.value(QStringLiteral("sourceSnapshot")).toString() != snapshotKind)
    { return packageFailure(error,QStringLiteral("清单源快照身份与实际内容不一致。")); }
    inputDigest = candidate.value(QStringLiteral("inputProjectSha256")).toString().toLatin1();
    if (inputDigest.size() != 64 || QByteArray::fromHex(inputDigest).toHex() != inputDigest) //20261127-12 add
    {
        return packageFailure(error, QStringLiteral("输入工程 SHA256 必须为完整的小写十六进制摘要。"));
    }
    if (snapshotKind == QStringLiteral("native-sm"))
    {
        nativeSource = qUncompress(payloads[7]);
        if (!inspectNativeProjectSourceFields(nativeSource, &nativeFields, error)) {
            return false;
        }
        // V122 keeps the original template model in its envelope after a target change.
        // The system identity string selects the active hardware for USB and compilation.
        for (const NativeSourceFieldSpan &field : nativeFields) {
            if (field.path == QStringLiteral("system.identity.1")) {
                QDataStream identity(nativeSource.mid(field.start, field.end - field.start));
                identity >> nativeTarget;
                if (identity.status() != QDataStream::Ok || !identity.atEnd()) {
                    return packageFailure(error, QStringLiteral("工程实际目标型号字段不可读。"));
                }
                break;
            }
        }
        if (deviceTargetIdentityByte(nativeTarget) !=
            deviceTargetIdentityByte(candidate.value(QStringLiteral("targetModel")).toString())) {
            return packageFailure(error, QStringLiteral("原生源快照与清单目标型号不一致。"));
        }
    }
    if (snapshotKind == QStringLiteral("lite-project-json")) //20261122-3 change
    {
        snapshot = QJsonDocument::fromJson(qUncompress(payloads[7]));
        if (snapshot.object().value(QStringLiteral("targetModel")) != candidate.value(QStringLiteral("targetModel")) ||
            candidate.value(QStringLiteral("inputProjectSha256")).toString() != packageSha(snapshot.toJson(QJsonDocument::Compact)))
        {
            return packageFailure(error,QStringLiteral("源快照与清单目标型号或输入工程SHA256不一致。"));
        }
    }
    if (!staticMacros && !buildDeviceTransferPlan(payloads,&steps,error)) { return false; } //20261123-22 change
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
    const bool staticMacros = hasStaticMacroPayloads(payloads); //20261123-22 add
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
    if (!validateDevicePayloadStructure(payloads,error) || (!staticMacros && !buildDeviceTransferPlan(payloads,&steps,error))) { return false; } //20261123-22 change
    if (!parent.exists() && !QDir().mkpath(parent.absolutePath())) { return packageFailure(error,QStringLiteral("无法建立输出父目录。")); }
    staging = std::make_unique<QTemporaryDir>(parent.filePath(QStringLiteral(".kinseal-compile-XXXXXX")));
    if (!staging->isValid()) { return packageFailure(error,QStringLiteral("无法在输出所在卷建立暂存目录。")); }
    for (index=0;index<payloads.size();++index)
    {
        name = QString::fromLatin1(packageNames[index + (staticMacros && index >= 10 ? 2 : 0)]); //20261123-22 change
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
    if (staticMacros) { metadata.insert(QStringLiteral("offlineOnly"), true); } //20261123-22 add
    if (program.open(QIODevice::ReadOnly)) { metadata.insert(QStringLiteral("compilerExecutableSha256"),packageSha(program.readAll())); }
    if (!write(QDir(staging->path()).filePath(QStringLiteral("compiler-manifest.json")),QJsonDocument(metadata).toJson(QJsonDocument::Indented),error) || !verifyDevicePackage(staging->path(),nullptr,error)) { return false; }
    if (QFileInfo::exists(target.absoluteFilePath()) || !parent.rename(QFileInfo(staging->path()).fileName(),target.fileName()))
    { return packageFailure(error,QStringLiteral("包发布失败或目标被占用；已有目录保持不变，暂存输出将回收。")); }
    staging->setAutoRemove(false);
    return true;
}

#include "native_media_archive.cpp.inc" //20261127-12 add
