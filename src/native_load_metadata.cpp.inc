//20261128-0 add: Generate the vendor's V122 offline media descriptor from source and compiler metadata.
#include "device_package.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QSizeF>
#include <QtEndian>

namespace {
/******************************************************************************
 * Function Name   : loadMetadataJson
 * Description     : Read one bounded compiler-context or complete metadata JSON object.
 * Input Parameters: path, output, error
 * Return Value    : True for a readable JSON object.
 * Date            : 2026.11.28
 ******************************************************************************/
bool loadMetadataJson(const QString &path, QJsonObject *output, QString *error)
{
    QFile file(path);
    QJsonParseError parseError;
    QJsonDocument json;
    if (!file.open(QIODevice::ReadOnly) || file.size() > 1024 * 1024) {
        return fail(error, QStringLiteral("介质元数据 JSON 无法读取或超过 1 MiB。"));
    }
    json = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (!json.isObject() || parseError.error != QJsonParseError::NoError) {
        return fail(error, QStringLiteral("介质元数据需要有效 JSON 对象。"));
    }
    *output = json.object();
    return true;
}

/******************************************************************************
 * Function Name   : putLoadMetadata
 * Description     : Encode all V122 load.bin fields in native order and preserve null versus empty strings.
 * Input Parameters: m, bytes, error
 * Return Value    : True after checking all scalar and collection fields.
 * Date            : 2026.11.28
 ******************************************************************************/
bool putLoadMetadata(const QJsonObject &m, QByteArray *bytes, QString *error)
{
    QByteArray result;
    QDataStream out(&result, QIODevice::WriteOnly);
    const QStringList intKeys = {QStringLiteral("pageCount"), QStringLiteral("startupType"),
        QStringLiteral("macroCount"), QStringLiteral("panelType"), QStringLiteral("compilerCounter838"),
        QStringLiteral("connectionCount"), QStringLiteral("compilerCounter688"), QStringLiteral("historyCount")};
    const QStringList stringKeys = {QStringLiteral("model"), QStringLiteral("startupImage"), QStringLiteral("optionText")};
    const QStringList listKeys = {QStringLiteral("macroFiles"), QStringLiteral("driverPaths"), QStringLiteral("assetPairs"),
                                 QStringLiteral("flags"), QStringLiteral("pageSizes")};
    const QStringList keys = intKeys + stringKeys + listKeys;
    QJsonArray values;
    QJsonArray pair;
    int i;
    auto validString = [](QJsonValue v) { return v.isNull() || (v.isString() && v.toString().size() <= 4096); };
    auto stringValue = [](QJsonValue v) { return v.isNull() ? QString() : (v.toString().isEmpty() ? QStringLiteral("") : v.toString()); };

    for (const QString &key : m.keys()) {
        if (!keys.contains(key)) { return fail(error, QStringLiteral("未知介质元数据字段 %1。").arg(key)); }
    }
    for (const QString &key : intKeys) {
        if (!m.value(key).isDouble() || m.value(key).toInteger(-1) < 0 || m.value(key).toInteger(-1) > 1000000) {
            return fail(error, QStringLiteral("字段 %1 缺失或不是有效的非负整数。").arg(key));
        }
    }
    for (const QString &key : stringKeys) {
        if (!validString(m.value(key))) { return fail(error, QStringLiteral("字段 %1 缺失或不是有效字符串。").arg(key)); }
    }
    for (const QString &key : listKeys) {
        if (!m.value(key).isArray() || m.value(key).toArray().size() > 4096) {
            return fail(error, QStringLiteral("字段 %1 缺失或超过集合上限。").arg(key));
        }
    }
    if (m.value(QStringLiteral("model")).toString().isEmpty() || m.value(QStringLiteral("startupType")).toInt() > 127 ||
        m.value(QStringLiteral("pageCount")).toInt() < 1 || m.value(QStringLiteral("pageCount")).toInt() > 200 ||
        m.value(QStringLiteral("macroCount")).toInt() != m.value(QStringLiteral("macroFiles")).toArray().size() ||
        m.value(QStringLiteral("flags")).toArray().size() != 4 || m.value(QStringLiteral("pageSizes")).toArray().size() != m.value(QStringLiteral("pageCount")).toInt()) {
        return fail(error, QStringLiteral("面板名称、类型、画面数量、标志或尺寸表无效。"));
    }
    out.setVersion(QDataStream::Qt_4_8);
    out << QStringLiteral("KinSealStudio-V1.0.2") << qint32(122) << m.value(QStringLiteral("model")).toString()
        << qint32(m.value(QStringLiteral("pageCount")).toInt()) << qint8(m.value(QStringLiteral("startupType")).toInt())
        << stringValue(m.value(QStringLiteral("startupImage")));
    for (const QJsonValue flag : m.value(QStringLiteral("flags")).toArray()) {
        if (!flag.isBool()) { return fail(error, QStringLiteral("介质元数据需要四个布尔标志。")); }
        out << flag.toBool();
    }
    out << qint32(m.value(QStringLiteral("macroCount")).toInt());
    for (i = 0; i < 3; ++i) {
        values = m.value(listKeys.at(i)).toArray();
        out << qint32(values.size());
        for (const QJsonValue value : values) {
            pair = i == 2 ? value.toArray() : QJsonArray{value};
            if ((i == 2 && !value.isArray()) || pair.size() != (i == 2 ? 2 : 1)) {
                return fail(error, QStringLiteral("介质资源对表必须逐行给出两个 QString。"));
            }
            for (const QJsonValue item : pair) {
                if (!validString(item)) { return fail(error, QStringLiteral("介质文件标识必须是 QString。")); }
                out << stringValue(item);
            }
        }
    }
    out << qint32(m.value(QStringLiteral("panelType")).toInt()) << stringValue(m.value(QStringLiteral("optionText")))
        << qint32(m.value(QStringLiteral("compilerCounter838")).toInt()) << qint32(m.value(QStringLiteral("connectionCount")).toInt())
        << qint32(m.value(QStringLiteral("compilerCounter688")).toInt());
    values = m.value(QStringLiteral("pageSizes")).toArray();
    out << qint32(values.size());
    for (const QJsonValue value : values) {
        pair = value.toArray();
        if (pair.size() != 2) { return fail(error, QStringLiteral("画面尺寸需要两个数值。")); }
        const double width = pair.at(0).toDouble(-1);
        const double height = pair.at(1).toDouble(-1);
        if (!(width > 0 && width <= 65536 && height > 0 && height <= 65536)) {
            return fail(error, QStringLiteral("画面尺寸必须是两个有效的正数。"));
        }
        out << QSizeF(width, height);
    }
    out << qint32(m.value(QStringLiteral("historyCount")).toInt());
    if (out.status() != QDataStream::Ok) { return fail(error, QStringLiteral("V122 介质元数据写入失败。")); }
    *bytes = result;
    return true;
}

/******************************************************************************
 * Function Name   : loadMetadataFromParts
 * Description     : Resolve the verified COM1 driver and message count from independent compiler files.
 * Input Parameters: d, directory, context, error
 * Return Value    : True for the measured V122 two-port Modbus profile.
 * Date            : 2026.11.28
 ******************************************************************************/
bool loadMetadataFromParts(const SourceDocument &d, const QString &directory, QJsonObject *context, QString *error)
{
    const QDir parts(directory);
    const QString prefix = QStringLiteral("system.16c[0].50[0].tail.");
    const QString driver = QStringLiteral("Modbus_RTU_com1.bin");
    QFile control(parts.filePath(QStringLiteral("ControlPara.dat")));
    QFile sheets(parts.filePath(QStringLiteral("SheetPara.dat")));
    QByteArray record;
    QByteArray sheet;
    const int pageCount = d.value(QStringLiteral("project.pages.count")).toInt();
    int expectedControls = 0;
    int messages = 0;
    int index;

    if (!parts.exists() || pageCount < 1 || pageCount > 200 || !sheets.open(QIODevice::ReadOnly) || sheets.size() != qint64(pageCount) * 72) {
        return fail(error, QStringLiteral("同源 SM 与每页 72 字节的 SheetPara.dat 记录数不符。"));
    }
    sheet = sheets.readAll();
    if (sheet.size() != qint64(pageCount) * 72) { return fail(error, QStringLiteral("读取画面描述失败。")); }
    for (index = 0; index < pageCount; ++index) {
        const char *pageRecord = sheet.constData() + index * 72;
        const QString pageKey = index == 0 ? QStringLiteral("page") : SourceDocument::indexed(QStringLiteral("page"), index);
        if (qFromLittleEndian<u16>(pageRecord + 48) != d.value(pageKey + QStringLiteral(".head.2")).toDouble() ||
            qFromLittleEndian<u16>(pageRecord + 50) != d.value(pageKey + QStringLiteral(".head.3")).toDouble()) {
            return fail(error, QStringLiteral("同源 SM 与 SheetPara.dat 第 %1 页尺寸不相符。").arg(index + 1));
        }
        // The little-endian count at +2 is local to this sheet, while ControlPara is the concatenated table.
        expectedControls += qFromLittleEndian<u16>(pageRecord + 2);
    }
    if (!context->contains(QStringLiteral("driverPaths"))) {
        if (d.value(QStringLiteral("system.16c.count")).toInt() != 2 ||
            d.value(QStringLiteral("system.16c[0].50.count")).toInt() != 1 ||
            d.value(QStringLiteral("system.16c[1].50.count")).toInt() != 0 ||
            d.value(QStringLiteral("system.16c[0].header.11")).toString() != QStringLiteral("COM1") ||
            d.value(prefix + QStringLiteral("9")).toString() != QStringLiteral("MODBUS") ||
            d.value(prefix + QStringLiteral("10")).toString() != QStringLiteral("Modbus_RTU")) {
            return fail(error, QStringLiteral("此连接组合需显式提供原厂编译器解析后的 driverPaths。"));
        }
        context->insert(QStringLiteral("driverPaths"), QJsonArray{QStringLiteral("Link/serial_amz/MODBUS/") +
            (QFileInfo(parts.filePath(driver)).isFile() ? driver : QStringLiteral("_com1.bin"))});
    }
    if (!context->contains(QStringLiteral("compilerCounter688"))) {
        if (expectedControls != 0 && !control.exists()) {
            return fail(error, QStringLiteral("画面描述引用动态控件但缺少 ControlPara.dat。"));
        }
        if (control.exists()) {
            if (!control.open(QIODevice::ReadOnly) || control.size() > MaximumSourceBytes || control.size() != qint64(expectedControls) * 640) {
                return fail(error, QStringLiteral("ControlPara.dat 无法按原厂 640 字节控件记录读取。"));
            }
            while (!control.atEnd()) {
                record = control.read(640);
                if (record.size() != 640) { return fail(error, QStringLiteral("读取控件记录失败。")); }
                if (static_cast<u8>(record.at(0)) == 35) { ++messages; }
            }
        }
        context->insert(QStringLiteral("compilerCounter688"), messages);
    }
    return true;
}

/******************************************************************************
 * Function Name   : extractLoadMetadata
 * Description     : Copy project-owned values from verified Qt stream fields and preserve exact source strings.
 * Input Parameters: project bytes, compiler context JSON, output metadata, error
 * Return Value    : True when runtime-owned fields are supplied separately and the source layout is known.
 * Date            : 2026.11.28
 ******************************************************************************/
bool extractLoadMetadata(const QByteArray &data, const QJsonObject &context, QJsonObject *metadata, QString *error)
{
    SourceDocument d(data);
    QJsonArray flags;
    QJsonArray pages;
    QJsonArray macros;
    QJsonObject fields = context;
    const QStringList expected = {QStringLiteral("driverPaths"), QStringLiteral("assetPairs"), QStringLiteral("compilerCounter838"), QStringLiteral("compilerCounter688"), QStringLiteral("macroFiles")};
    const QStringList flagsInStream = {QStringLiteral("config.flags.4"), QStringLiteral("config.head.18"), QStringLiteral("config.head.16"), QStringLiteral("config.head.15")};
    QString prefix;
    int i;
    auto retainedQString = [&d](const QString &key) -> QJsonValue {
        const SourceField field = d.values.value(key);
        if (d.bytes.mid(field.start, 4) == QByteArray(4, char(0xff))) { return QJsonValue(QJsonValue::Null); }
        return d.value(key).toString();
    };

    if (!d.parse()) { return fail(error, d.error); }
    if (d.value(QStringLiteral("header.1")).toInt() != 122) { return fail(error, QStringLiteral("此介质元数据写端仅核实原厂 V122 源工程。")); }
    // Native download-dialog constructors clear +0x838. The isDownLoadWord click slot writes its boolean state.
    if (fields.contains(QStringLiteral("downloadCharacterLibrary"))) {
        const QJsonValue download = fields.take(QStringLiteral("downloadCharacterLibrary"));
        if (!download.isBool() || fields.contains(QStringLiteral("compilerCounter838"))) {
            return fail(error, QStringLiteral("downloadCharacterLibrary 需要布尔值，且不能同时指定 compilerCounter838。"));
        }
        fields.insert(QStringLiteral("compilerCounter838"), download.toBool() ? 1 : 0);
    } else if (!fields.contains(QStringLiteral("compilerCounter838"))) {
        fields.insert(QStringLiteral("compilerCounter838"), 0);
    }
    if (context.contains(QStringLiteral("macroFiles"))) { return fail(error, QStringLiteral("工程中的宏文件列表自动从宏注册表读取。")); }
    if (fields.contains(QStringLiteral("compiledDirectory"))) {
        const QJsonValue directory = fields.take(QStringLiteral("compiledDirectory"));
        if (!directory.isString() || directory.toString().isEmpty() || !loadMetadataFromParts(d, directory.toString(), &fields, error)) {
            return false;
        }
    }
    for (i = 0; i < d.value(QStringLiteral("history.1e4.count")).toInt(); ++i) {
        prefix = SourceDocument::indexed(QStringLiteral("history.1e4"), i);
        macros.append(d.value(prefix + QStringLiteral(".head.1")).toString() + QStringLiteral(".bin"));
    }
    fields.insert(QStringLiteral("macroFiles"), macros);
    for (const QString &key : fields.keys()) {
        if (!expected.contains(key)) { return fail(error, QStringLiteral("编译上下文不应覆盖源工程字段 %1。").arg(key)); }
    }
    for (const QString &key : expected) {
        if (!fields.contains(key)) { return fail(error, QStringLiteral("编译上下文缺少 %1。").arg(key)); }
    }
    fields.insert(QStringLiteral("model"), retainedQString(QStringLiteral("system.identity.1")));
    fields.insert(QStringLiteral("pageCount"), d.value(QStringLiteral("project.pages.count")).toInt());
    fields.insert(QStringLiteral("startupType"), d.value(QStringLiteral("system.tail.0")).toInt());
    fields.insert(QStringLiteral("startupImage"), retainedQString(QStringLiteral("system.tail.1")));
    for (const QString &key : flagsInStream) {
        flags.append(d.value(key).toInt() != 0);
    }
    fields.insert(QStringLiteral("flags"), flags);
    fields.insert(QStringLiteral("macroCount"), d.value(QStringLiteral("history.1e4.count")).toInt());
    fields.insert(QStringLiteral("panelType"), d.value(QStringLiteral("system.identity.3")).toInt());
    fields.insert(QStringLiteral("optionText"), retainedQString(QStringLiteral("config.head.19")));
    fields.insert(QStringLiteral("connectionCount"), d.value(QStringLiteral("system.16c.count")).toInt());
    fields.insert(QStringLiteral("historyCount"), d.value(QStringLiteral("config.cc.count")).toInt());
    for (i = 0; i < fields.value(QStringLiteral("pageCount")).toInt(); ++i) {
        prefix = i == 0 ? QStringLiteral("page") : SourceDocument::indexed(QStringLiteral("page"), i);
        pages.append(QJsonArray{d.value(prefix + QStringLiteral(".head.2")).toDouble(),
                                d.value(prefix + QStringLiteral(".head.3")).toDouble()});
    }
    fields.insert(QStringLiteral("pageSizes"), pages);
    *metadata = fields;
    return true;
}
}

/******************************************************************************
 * Function Name   : buildNativeLoadMetadata
 * Description     : Generate a new V122 load.bin transactionally from source plus context or a complete JSON manifest.
 * Input Parameters: inputPath, outputPath, contextPath, error
 * Return Value    : True after committing the complete binary.
 * Date            : 2026.11.28
 ******************************************************************************/
bool buildNativeLoadMetadata(const QString &inputPath, const QString &outputPath, const QString &contextPath, QString *error)
{
    QFile input(inputPath);
    QSaveFile output(outputPath);
    QJsonObject metadata;
    QJsonObject context;
    QByteArray encoded;
    if (QFileInfo(inputPath).suffix().compare(QStringLiteral("sm"), Qt::CaseInsensitive) == 0) {
        if (contextPath.isEmpty()) { return fail(error, QStringLiteral("原厂驱动分辨结果及编译时计数由上下文 JSON 提供，请指定编译上下文。")); }
        if (!input.open(QIODevice::ReadOnly) || input.size() > MaximumSourceBytes) { return fail(error, QStringLiteral("原厂工程无法读取或超出大小限制。")); }
        if (!loadMetadataJson(contextPath, &context, error) || !extractLoadMetadata(input.readAll(), context, &metadata, error)) { return false; }
    } else if (QFileInfo(inputPath).suffix().compare(QStringLiteral("json"), Qt::CaseInsensitive) == 0 && contextPath.isEmpty()) {
        if (!loadMetadataJson(inputPath, &metadata, error)) { return false; }
    } else { return fail(error, QStringLiteral("输入必须是原厂 SM 配合编译上下文 JSON，或完整介质元数据 JSON。")); }
    if (!putLoadMetadata(metadata, &encoded, error)) { return false; }
    if (QFileInfo(outputPath).fileName().compare(QStringLiteral("load.bin"), Qt::CaseInsensitive) != 0) {
        return fail(error, QStringLiteral("输出文件名必须为 load.bin。"));
    }
    if (!output.open(QIODevice::WriteOnly) || output.write(encoded) != encoded.size() || !output.commit()) {
        return fail(error, output.errorString());
    }
    return true;
}
