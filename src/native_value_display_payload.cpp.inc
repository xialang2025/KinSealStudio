#include "text_payload_compiler.h"
#include "native_project_codec.h"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtEndian>

/******************************************************************************
 * Function Name   : nativeValueDisplaySourceProfile
 * Description     : Check the original page style, hidden address slots and private display options before rendering.
 * Input Parameters: original V122 snapshot, normalized display family, error
 * Return Value    : True for a local, solid display on a verified single-page source profile.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool nativeValueDisplaySourceProfile(const QByteArray &source, const QString &type, QByteArray &sheet, QString *error) //20261128-25 add
{
    QList<NativeSourceFieldSpan> fields;
    QCryptographicHash hash(QCryptographicHash::Sha256);
    QByteArray signature;
    QList<int> editable;
    QStringList path;
    int index;
    if (source.isEmpty()) {
        return true; // Source-free KSP settings are fully available in the model.
    }
    if (!inspectNativeProjectSourceFields(source, &fields, error) || !nativeProjectUsesSupportedDeviceDefaults(source, error)) {
        return false;
    }
    // Source colors, font sizes and the displayed value can change without selecting another frame layout.
    editable = type == QStringLiteral("number") ? QList<int>{2,8,9,10,12,13,14,15} :
               type == QStringLiteral("ascii") ? QList<int>{2,7,8,11,12} : QList<int>{4,8,9,10,11,12,14};
    for (const NativeSourceFieldSpan &field : fields) {
        if (field.path == QStringLiteral("page.tail.1") && source.at(field.start) != 0) {
            *error = QStringLiteral("页面含活动动作，显示器页表的动作索引尚待核定。");
            return false;
        } else if (field.path == QStringLiteral("page.tail.2")) {
            qToLittleEndian<u16>(u16(qFromBigEndian<u32>(source.constData() + field.start)), sheet.data() + 56);
        } else if (field.path == QStringLiteral("page.tail.13")) {
            sheet[1] = char(qFromBigEndian<u32>(source.constData() + field.start) & 0xffU);
        } else if (field.path.startsWith(QStringLiteral("object[0].common."))) {
            if ((field.path.endsWith(QStringLiteral(".visibility.0")) || field.path.endsWith(QStringLiteral(".touch.0")) ||
                 field.path.endsWith(QStringLiteral(".58.2")) || (field.kind == 'a' && field.path.endsWith(QStringLiteral(".1")))) &&
                source.at(field.start) != 0) {
                *error = QStringLiteral("源工程的触控、显现、通知或外部连接超出显示器散件验证范围。");
                return false;
            }
        } else if (field.path.startsWith(QStringLiteral("page.head."))) {
            if (field.path == QStringLiteral("page.head.18") || field.path == QStringLiteral("page.head.19")) { continue; }
            hash.addData(QByteArrayView(source).sliced(field.start, field.end - field.start));
        } else if (field.path.startsWith(QStringLiteral("object[0].display."))) {
            path = field.path.split(QLatin1Char('.'));
            index = path.at(2).toInt();
            if (editable.contains(index) || (path.size() == 4 && (path.last() == QStringLiteral("0") || path.last() == QStringLiteral("4")))) {
                continue; // Typed address slot device, family and register flags remain in the digest.
            }
            hash.addData(QByteArrayView(source).sliced(field.start, field.end - field.start));
        }
    }
    signature = hash.result().toHex();
    // These identify only the remaining SM input fields. Device outputs are generated from the current model below.
    if ((type == QStringLiteral("number") && (signature == "3e5450aed3cd831c8e8877d67b9c9adda883c7b94432c8948d58e07821d86a0b" ||
                                             signature == "09c40783bb6b695a5b53bb15e39aa30c14cdb1f9426b96344fb40c59a6d823c7")) ||
        (type == QStringLiteral("ascii") && (signature == "61f42f106190f0d1e34515ed0d61dde9ddce70e7dc8d534b663d43b4fb22cd4b" ||
                                            signature == "6b62c2d2af675156c72a155f40771e950b76968b08a249291c4039d7dcd5804a")) ||
        (type == QStringLiteral("time") && (signature == "907613014c94393d4f6083cd46b455a8880fe9418fac80ba5fadc81f31c714ac" ||
                                           signature == "abbf860a228ca65568f5892c954842c840c578809533ee2b723033d9e0a35e64"))) {
        return true;
    }
    *error = QStringLiteral("原厂显示器的隐藏属性或页面样式超出已核实的纯色帧范围。");
    return false;
}

/******************************************************************************
 * Function Name   : exportNativeValueDisplayParts
 * Description     : Compile the page image, local scan index, control sprites and frame table for a solid value display.
 * Input Parameters: loaded SM or KSP, new destination directory, error
 * Return Value    : True after all five independent vendor-format files have been published together.
 * Date            : 2026.11.28
 ******************************************************************************/
bool exportNativeValueDisplayParts(const HmiProject &project, const QString &destination, QString *error) //20261128-25 add
{
    const QFileInfo target(destination);
    QDir parent(target.absolutePath());
    const QRect screen(0, 0, 800, 480);
    QByteArray page(800 * 480 * 2, char(0)), compressedPage, pixels, frame, compressed, lengths(2400, char(0));
    QByteArray sheet(72, char(0));
    QMap<QString, QByteArray> parts;
    QList<QColor> fills;
    u32 alignment;
    u16 color;
    QString diagnostic;
    int x, y, position;
    if (!error) { error = &diagnostic; }
    if (!project.validateSinglePageBackend(error)) { return false; }
    if (project.objects.size() != 1 || !project.connections.isEmpty() || !project.macros.isEmpty() ||
        !project.configurationNodes.isEmpty() || !project.pictures.isEmpty() || !project.animations.isEmpty()) {
        *error = QStringLiteral("显示器散件需要单个本地显示器和默认页面。");
        return false;
    }
    const HmiObject &object = project.objects.first();
    const bool time = object.type == QStringLiteral("time");
    const bool numeric = object.type == QStringLiteral("number");
    const NumberDisplayProperties &value = numeric ? object.numberDisplay : object.asciiDisplay.common;
    const TimeDisplayProperties &clock = object.timeDisplay;
    const ImageDisplayState &style = time ? clock.appearance : value.appearance;
    if ((!time && !numeric && object.type != QStringLiteral("ascii")) || object.geometry.isEmpty() ||
        !screen.contains(object.geometry) || !object.subpixelOffset.isNull() || object.hasAppearanceFlip() ||
        !style.picture.isEmpty() || !style.picturePng.isEmpty() ||
        (time ? (clock.opacity != 255 || clock.transparent || clock.effect3d || clock.visibility.visibilityEnabled) :
                (value.opacity != 255 || value.transparent || value.effect3d || value.visibility.visibilityEnabled ||
                 value.touch.visibilityEnabled || value.notice || value.macroEnabled || value.offsetEnabled)) ||
        (!time && (!value.monitorAddress.startsWith(QStringLiteral("LW")) || !value.inputAddress.startsWith(QStringLiteral("LW"))))) {
        *error = QStringLiteral("该显示器的外形、坐标、透明度、地址或活动条件超出本地纯色帧范围。");
        return false;
    }
    if (!time && !(numeric ? value.validateAddresses(project.connections, error) :
                   object.asciiDisplay.validateAddresses(project.connections, error))) { return false; }
    sheet[1] = 1;
    qToLittleEndian<u16>(50, sheet.data() + 56);
    if (!nativeValueDisplaySourceProfile(project.nativeSourceSnapshot, object.type, sheet, error)) { return false; }
    // Firmware decoding of all six original page buffers gives a black top/left edge and RGB565 C6FF elsewhere.
    for (y = 1; y < screen.height(); ++y) {
        for (x = 1; x < screen.width(); ++x) {
            qToLittleEndian<u16>(0xc6ff, page.data() + 2 * (y * screen.width() + x));
        }
    }
    if (!compressRgb565Frame(page, &compressedPage, error)) { return false; }
    fills.append(QColor(time ? clock.backgroundColor : value.backgroundColor));
    if (numeric) {
        fills.append(QColor(value.highColor));
        fills.append(QColor(value.lowColor));
    }
    for (const QColor &fill : fills) {
        if (!fill.isValid() || fill.alpha() != 255) {
            *error = QStringLiteral("纯色显示器帧需要有效的不透明背景色。");
            return false;
        }
        color = u16(((fill.red() >> 3) << 11) | ((fill.green() >> 2) << 5) | (fill.blue() >> 3));
        frame.resize(object.geometry.width() * object.geometry.height() * 2);
        for (position = 0; position < frame.size(); position += 2) {
            qToLittleEndian<u16>(color, frame.data() + position);
        }
        if (!compressRgb565Pixels(frame, &compressed, error)) { return false; }
        alignment = (u32(compressed.size()) + 31U) & ~31U;
        position = lengths.size();
        lengths.append(12, char(0));
        qToLittleEndian<u32>(alignment, lengths.data() + position + 4);
        qToLittleEndian<u32>(u32(compressed.size()), lengths.data() + position + 8);
        pixels.append(compressed);
        pixels.append(alignment - u32(compressed.size()), char(0));
    }
    if (!applyNativeControlPixelPadding(compressedPage, lengths.mid(2400), &pixels, error)) { return false; }
    qToLittleEndian<u32>(u32(compressedPage.size()), lengths.data());
    qToLittleEndian<u32>(u32(pixels.size()), lengths.data() + 800);
    qToLittleEndian<u32>(u32(fills.size()), lengths.data() + 1600);
    // A single ordinary page owns one dynamic record and the sum of its uncompressed frame sizes.
    sheet[2] = 1;
    writeNativeSheetTitle(sheet, project.pageName);
    qToLittleEndian<u16>(u16(screen.width()), sheet.data() + 48);
    qToLittleEndian<u16>(u16(screen.height()), sheet.data() + 50);
    std::fill_n(sheet.data() + 60, 8, char(0xff));
    qToLittleEndian<u32>(u32(frame.size()) * u32(fills.size()), sheet.data() + 68);
    parts.insert(QStringLiteral("SheetPara.dat"), sheet);
    parts.insert(QStringLiteral("SheetPixel.dat"), compressedPage);
    parts.insert(QStringLiteral("ControlPixel.dat"), pixels);
    parts.insert(QStringLiteral("PicLenPara.dat"), lengths);
    parts.insert(QStringLiteral("AddrPara.dat"), QByteArray(200, char(0))); // No external PLC scan rows in a local display.
    if (destination.trimmed().isEmpty() || target.fileName().isEmpty() || target.exists() || target.isSymLink() ||
        !QDir().mkpath(parent.absolutePath())) {
        *error = QStringLiteral("显示器散件须生成到新目录。");
        return false;
    }
    QTemporaryDir staging(parent.filePath(QStringLiteral(".kinseal-display-XXXXXX")));
    if (!staging.isValid()) { *error = staging.errorString(); return false; }
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
        QFile output(staging.filePath(it.key()));
        if (!output.open(QIODevice::WriteOnly | QIODevice::NewOnly) || output.write(it.value()) != it.value().size() || !output.flush()) {
            *error = output.errorString(); return false;
        }
    }
    if (QFileInfo::exists(target.absoluteFilePath()) || !parent.rename(QFileInfo(staging.path()).fileName(), target.fileName())) {
        *error = QStringLiteral("无法发布完整的显示器散件目录。");
        return false;
    }
    staging.setAutoRemove(false);
    return true;
}
