#include <QJsonArray>
#include <QTemporaryFile>
#include <cmath>
#include <cstring>

/******************************************************************************
 * Function Name   : exportNativeValueRecordFields
 * Description     : Encode verified V122 value-display fields at their device offsets without filling unknown bytes.
 * Input Parameters: single local display project, new JSON path, optional diagnostic
 * Return Value    : True after the complete field map was published without replacing an existing file.
 * Date            : 2026.11.28
 ******************************************************************************/
bool exportNativeValueRecordFields(const HmiProject &project, const QString &destination, QString *error) //20261128-31 add
{
    const QFileInfo target(destination);
    QTemporaryFile output(target.absolutePath() + QStringLiteral("/.kinseal-record-XXXXXX"));
    const QList<int> numberSizes{16, 24, 32, 40, 48, 64, 96, 192};
    const QList<int> clockSizes{12, 16, 24, 48, 64, 96, 192};
    const QList<int> clockAlignments{1, 2, 0}; // The AMZ draws middle, left and right using flags 0x0e, 0x0c and 0x0d.
    QByteArray sheet(72, char(0)), record(640, char(0)), known(640, char(0)), data;
    QJsonArray fields;
    QJsonObject edited, defaults;
    QString diagnostic;
    QStringList allowed;
    const HmiObject *object;
    const NumberDisplayProperties *value;
    const TimeDisplayProperties *clock;
    bool time, numeric, ok;
    int box, text, font, fontIndex, frames, addressBase, inputBase, offset, count;
    u32 reg;
    double real;
    quint64 bits;

    // The coverage map is separate from the record. No filler value is exported for unknown offsets.
    const auto field = [&](const QString &property, int pos, int length) {
        fields.append(QJsonObject{{QStringLiteral("property"), property}, {QStringLiteral("offset"), pos},
                                  {QStringLiteral("bytes"), QString::fromLatin1(record.mid(pos, length).toHex())}});
        std::fill_n(known.data() + pos, length, char(1));
    };
    const auto word = [&](const QString &property, int pos, int val) {
        qToLittleEndian<u16>(u16(val), record.data() + pos);
        field(property, pos, 2);
    };
    const auto color = [&](const QString &property, int pos, const QString &html) {
        const QColor rgb(html);
        record[pos] = char(rgb.red()); record[pos + 1] = char(rgb.green()); record[pos + 2] = char(rgb.blue());
        // AMZ loads the RTC pen as a 32-bit word. V5.1 writes zero above the three editable RGB bytes.
        field(property, pos, (time && pos == 0x120) ? 4 : 3);
    };
    if (!error) { error = &diagnostic; }
    if (target.suffix().compare(QStringLiteral("json"), Qt::CaseInsensitive) || target.exists() || target.isSymLink() ||
        project.objects.size() != 1 || !project.connections.isEmpty() || !project.macros.isEmpty() ||
        !project.configurationNodes.isEmpty() || !project.pictures.isEmpty() || !project.animations.isEmpty()) {
        *error = QStringLiteral("控件字段导出需要单个本地显示器和新的 JSON 文件名。"); return false;
    }
    if (!project.validateSinglePageBackend(error)) { return false; }
    object = &project.objects.first();
    time = object->type == QStringLiteral("time");
    numeric = object->type == QStringLiteral("number");
    if ((!time && !numeric && object->type != QStringLiteral("ascii")) ||
        object->geometry.isEmpty() || !QRect(0, 0, 800, 480).contains(object->geometry) ||
        !object->subpixelOffset.isNull() || object->hasAppearanceFlip()) {
        *error = QStringLiteral("控件字段只支持 800×480 页面内整像素、未翻转的数值、ASCII 或时间显示器。"); return false;
    }
    if (!nativeValueDisplaySourceProfile(project.nativeSourceSnapshot, object->type, sheet, error)) { return false; }
    value = numeric ? &object->numberDisplay : &object->asciiDisplay.common;
    clock = &object->timeDisplay;
    if (!(time ? clock->validate(error) : numeric ? value->validate(error) : object->asciiDisplay.validate(error))) {
        return false;
    }
    edited = time ? clock->toJson() : value->toJson();
    defaults = time ? TimeDisplayProperties().toJson() : NumberDisplayProperties().toJson();
    allowed = {QStringLiteral("fontSize"), QStringLiteral("textColor"), QStringLiteral("backgroundColor")};
    if (time) {
        allowed += {QStringLiteral("showDate"), QStringLiteral("showTime"), QStringLiteral("showWeek"),
                    QStringLiteral("dateFormat"), QStringLiteral("timeFormat"), QStringLiteral("alignment")};
    } else {
        allowed += {QStringLiteral("monitorAddress"), QStringLiteral("inputAddress")};
        if (!value->sameMonitor || value->monitorAddress != value->inputAddress ||
            (numeric && value->dataType != 0)) {
            *error = QStringLiteral("控件字段目前只核实了同一 LW 读写地址和默认有符号十六位数值格式。"); return false;
        }
        if (numeric) {
            allowed += {QStringLiteral("totalDigits"), QStringLiteral("decimalDigits"), QStringLiteral("dataType"),
                        QStringLiteral("gain"), QStringLiteral("offset"), QStringLiteral("inputMaximum"),
                        QStringLiteral("inputMinimum"), QStringLiteral("highColor"), QStringLiteral("lowColor")};
        } else if (object->asciiDisplay.encoding != 0 || object->asciiDisplay.autoWrap ||
                   !object->asciiDisplay.swapBytes || object->asciiDisplay.modifyInput) {
            *error = QStringLiteral("ASCII 的编码、换行、字节交换或输入设置超出已核实的字段范围。"); return false;
        }
        if (!(numeric ? value->validateAddresses(project.connections, error) :
                        object->asciiDisplay.validateAddresses(project.connections, error))) { return false; }
    }
    for (const QString &key : allowed) { edited.remove(key); defaults.remove(key); }
    if (edited != defaults) {
        for (auto entry = edited.constBegin(); entry != edited.constEnd(); ++entry) {
            if (entry.value() != defaults.value(entry.key())) {
                *error = QStringLiteral("此显示器属性尚未纳入控件字段写端：%1").arg(entry.key()); return false;
            }
        }
        *error = QStringLiteral("显示器包含未核实的运行选项。"); return false;
    }
    box = time ? 0x10e : numeric ? 0x11e : 0x11c;
    text = time ? 0x120 : numeric ? 0x138 : 0x130;
    font = time ? 0x118 : 0x128;
    fontIndex = time ? clockSizes.indexOf(clock->fontSize) : numberSizes.indexOf(value->fontSize);
    if (fontIndex < 0) { *error = QStringLiteral("字号没有对应的设备字库索引。"); return false; }
    record[0] = char(time ? 17 : numeric ? 12 : 16);
    field(QStringLiteral("type"), 0, 1);
    for (int base : {4, box}) {
        qToLittleEndian<u16>(u16(object->geometry.x()), record.data() + base);
        qToLittleEndian<u16>(u16(object->geometry.y()), record.data() + base + 2);
        qToLittleEndian<u16>(u16(object->geometry.width()), record.data() + base + 4);
        qToLittleEndian<u16>(u16(object->geometry.height()), record.data() + base + 6);
        field(base == 4 ? QStringLiteral("geometry") : QStringLiteral("display.geometry"), base, 8);
    }
    frames = numeric ? 3 : 1;
    qToLittleEndian<u32>(u32(object->geometry.width() * object->geometry.height() * 2 * frames), record.data() + 16);
    field(QStringLiteral("singleControlPage.rawPixelOffset"), 12, 4);
    field(QStringLiteral("singleControlPage.rawPixelBytes"), 16, 4);
    word(QStringLiteral("fontSize"), font, fontIndex);
    color(QStringLiteral("textColor"), text, time ? clock->textColor : value->textColor);
    color(QStringLiteral("backgroundColor"), text + 4, time ? clock->backgroundColor : value->backgroundColor);
    if (time) {
        // Both gates are disabled in the admitted source/profile. AMZ tests the visibility byte after the bitmap draw.
        field(QStringLiteral("touchEnabled"), 0x14, 1);
        field(QStringLiteral("visibilityEnabled"), 0x15, 1);
        record[0x108] = char(clock->showDate); record[0x109] = char(clock->showTime); record[0x10a] = char(clock->showWeek);
        field(QStringLiteral("showDate"), 0x108, 1); field(QStringLiteral("showTime"), 0x109, 1);
        field(QStringLiteral("showWeek"), 0x10a, 1);
        word(QStringLiteral("alignment"), 0x10c, clockAlignments.at(clock->alignment));
        word(QStringLiteral("dateFormat"), 0x11a, clock->dateFormat);
        word(QStringLiteral("timeFormat"), 0x11c, clock->timeFormat);
    } else {
        addressBase = numeric ? 0x178 : 0x138;
        inputBase = numeric ? 0x250 : 0x1a4;
        for (int base : {addressBase, inputBase}) {
            const QString key = base == addressBase ? QStringLiteral("monitorAddress") : QStringLiteral("inputAddress");
            const QString address = base == addressBase ? value->monitorAddress : value->inputAddress;
            reg = address.mid(2).toUInt(&ok);
            if (!ok || !address.startsWith(QStringLiteral("LW"))) {
                *error = QStringLiteral("只核实了本地 LW 字地址。"); return false;
            }
            record[base + 1] = 'W';
            field(key + QStringLiteral(".space"), base, 2);
            qToLittleEndian<u32>(reg, record.data() + base + 8);
            field(key + QStringLiteral(".index"), base + 8, 4);
        }
        if (numeric) {
            word(QStringLiteral("totalDigits"), 0x12a, value->totalDigits);
            word(QStringLiteral("decimalDigits"), 0x12c, value->decimalDigits);
            word(QStringLiteral("dataType"), 0x12e, value->dataType);
            color(QStringLiteral("highColor"), 0x140, value->highColor);
            color(QStringLiteral("lowColor"), 0x144, value->lowColor);
            for (const QString &key : {QStringLiteral("gain"), QStringLiteral("offset"),
                                       QStringLiteral("inputMaximum"), QStringLiteral("inputMinimum")}) {
                offset = key == QStringLiteral("gain") ? 0x148 : key == QStringLiteral("offset") ? 0x150 :
                         key == QStringLiteral("inputMaximum") ? 0x168 : 0x170;
                real = value->toJson().value(key).toString().toDouble(&ok);
                if (!ok || !std::isfinite(real)) { *error = QStringLiteral("数值控件参数必须为有限数。"); return false; }
                static_assert(sizeof(real) == sizeof(bits), "The device record uses 64-bit IEEE floating-point.");
                std::memcpy(&bits, &real, sizeof(bits));
                qToLittleEndian<quint64>(bits, record.data() + offset);
                field(key, offset, 8);
            }
        } else {
            word(QStringLiteral("characterCount"), 0x12a, object->asciiDisplay.characterCount);
        }
    }
    count = known.count(char(1));
    data = QJsonDocument(QJsonObject{{QStringLiteral("format"), QStringLiteral("V122.ControlPara.fields")},
                                     {QStringLiteral("controlType"), object->type}, {QStringLiteral("recordBytes"), 640},
                                     {QStringLiteral("knownBytes"), count}, {QStringLiteral("completeRecord"), false},
                                     {QStringLiteral("fields"), fields}}).toJson(QJsonDocument::Indented);
    if (!output.open() || output.write(data) != data.size() || !output.flush() || !output.rename(target.absoluteFilePath())) {
        *error = output.errorString(); return false;
    }
    output.setAutoRemove(false);
    return true;
}
