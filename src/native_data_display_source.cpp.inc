//20261127-13 add: V122 DataDisplayItem stream used by the three native display families.

/******************************************************************************
 * Function Name   : readNativeDataDisplay
 * Description     : Traverse the counted preview group and the exact mode-dependent V122 private record.
 * Input Parameters: document, object prefix, root type
 * Return Value    : None. Structural or mode errors latch on the document.
 * Date            : 2026.11.27
 ******************************************************************************/
static void readNativeDataDisplay(SourceDocument &d, const QString &p, int type)
{
    const int expected = type == 12 ? 0 : type == 16 ? 1 : 2;
    static const QByteArray patterns[] = {
        QByteArrayLiteral("iqqbbssqsss" "i" "qqqqqAbqqssAAqqbAsbbsqqAA" "b" "qqibbqb" "b" "bbq" "baiib" "bqA"),
        QByteArrayLiteral("iqqsbsqss" "is" "qqAqAbqqsbAb" "b" "ibb" "b" "b" "bbq" "b" "baiib" "bqA"),
        QByteArrayLiteral("iqsqssssqqbbbiqsss")
    };
    int mode;
    d.container(p);
    mode = d.read('i', p + QStringLiteral("display.mode")).toInt();
    d.read('i', p + QStringLiteral("display.variant"));
    if (mode != expected) { d.reject(p, QStringLiteral("数据显示根类型与记录模式矛盾")); return; }
    if (d.value(QStringLiteral("header.1")).toInt() == 113) { //20261128-1 add: Number-mode writer exits at 0xb0a8b3 before the newer input flags.
        if (mode != 0) {
            d.reject(p, QStringLiteral("尚未核实此 V113 数据显示模式"));
            return;
        }
        nativeRecordPattern(d, p + QStringLiteral("display"), "iqqbbssqsssiqqqqqAbqqssAAqqbAsbbsqqAAbqqibbqb");
    } else {
        nativeRecordPattern(d, p + QStringLiteral("display"), patterns[mode]);
    }
    if (d.value(p + QStringLiteral("display.0")).toInt() != mode) {
        d.reject(p, QStringLiteral("数据显示模式副本不一致"));
    }
}

/******************************************************************************
 * Function Name   : dataDisplayColor
 * Description     : Decode the three native decimal RGB components without losing the source spelling.
 * Input Parameters: original Qt string
 * Return Value    : Valid color if all three components are in range.
 * Date            : 2026.11.27
 ******************************************************************************/
static QColor dataDisplayColor(const QString &native)
{
    const QStringList parts = native.split(QLatin1Char('+'));
    int rgb[3];
    bool ok;
    int i;
    if (parts.size() != 3) { return {}; }
    for (i = 0; i < 3; ++i) {
        rgb[i] = parts.at(i).toInt(&ok);
        if (!ok || rgb[i] < 0 || rgb[i] > 255) { return {}; }
    }
    return QColor(rgb[0], rgb[1], rgb[2]);
}

/******************************************************************************
 * Function Name   : dataDisplaySizes
 * Description     : Supply the native index order for the two independently measured font-size lists.
 * Input Parameters: mode
 * Return Value    : Ordered sizes in pixels.
 * Date            : 2026.11.27
 ******************************************************************************/
static QStringList dataDisplaySizes(int mode)
{
    if (mode == 2) { return TimeDisplayProperties::fontSizes(); }
    return { QStringLiteral("16"), QStringLiteral("24"), QStringLiteral("32"), QStringLiteral("40"), QStringLiteral("48"),
             QStringLiteral("64"), QStringLiteral("96"), QStringLiteral("192") };
}

/******************************************************************************
 * Function Name   : sourceDataDisplay
 * Description     : Recover editable geometry, text formatting, local addresses and input metadata; preserve remaining records in the snapshot.
 * Input Parameters: source, field prefix, object ordinal, output, external reference flag, diagnostic
 * Return Value    : True after the represented fields agree with the native display.
 * Date            : 2026.11.27
 ******************************************************************************/
static bool sourceDataDisplay(const SourceDocument &d, const QString &p, int index, HmiObject *out, bool *external, QString *error)
{
    const int mode = d.value(p + QStringLiteral("display.mode")).toInt();
    const QString text = p + QStringLiteral("child[0].");
    const auto v = [&](int k) { return d.value(p + QStringLiteral("display.") + QString::number(k)); };
    const auto geom = [&](const char *k) { return d.value(text + QString::fromLatin1(k)).toDouble(); };
    const double x = geom("text.head.0") + geom("text.tail.10");
    const double y = geom("text.head.1") + geom("text.tail.11");
    const double w = geom("text.head.2");
    const double h = geom("text.head.3");
    const int sizeIndex = v(mode == 0 ? 8 : mode == 1 ? 7 : 4).toInt();
    const QStringList sizes = dataDisplaySizes(mode);
    const QColor fg = dataDisplayColor(v(mode == 0 ? 12 : mode == 1 ? 11 : 8).toString());
    const QColor bg = dataDisplayColor(v(mode == 0 ? 13 : mode == 1 ? 12 : 9).toString());
    HmiObject object;
    NumberDisplayProperties &n = mode == 0 ? object.numberDisplay : object.asciiDisplay.common;
    TimeDisplayProperties &time = object.timeDisplay;
    QString displayFormat;
    int size;
    int date;
    if (d.value(p + QStringLiteral("children.count")).toInt() != 1 || d.value(text + QStringLiteral("type")).toInt() != 0x10016 ||
        std::abs(x) > 32768 || std::abs(y) > 32768 || w < 1 || h < 1 || w > 4096 || h > 4096 ||
        !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(w) || !std::isfinite(h) || sizeIndex < 0 || sizeIndex >= sizes.size() ||
        !fg.isValid() || !bg.isValid()) {
        return fail(error, QStringLiteral("数据显示器的预览图元、尺寸、字体或颜色尚未核实。"));
    }
    size = sizes.at(sizeIndex).toInt();
    object.type = mode == 0 ? QStringLiteral("number") : mode == 1 ? QStringLiteral("ascii") : QStringLiteral("time");
    object.id = QStringLiteral("native-data-%1").arg(index);
    object.name = d.value(p + QStringLiteral("group.metadata-name")).toString();
    if (object.name.isEmpty()) {
        object.name = (mode == 0 ? QStringLiteral("数值显示器") : mode == 1 ? QStringLiteral("ASCII显示器") : QStringLiteral("时间显示器")) + QString::number(index + 1);
    }
    object.geometry = QRect(int(std::floor(x)), int(std::floor(y)), qRound(w), qRound(h));
    object.subpixelOffset = QPointF(x - std::floor(x), y - std::floor(y));
    if (mode < 2) {
        if (!modelSourceAddress(d, p + (mode == 0 ? QStringLiteral("display.17") : QStringLiteral("display.13")), &n.monitorAddress, external, error) ||
            !modelSourceAddress(d, p + (mode == 0 ? QStringLiteral("display.28") : QStringLiteral("display.21")), &n.inputAddress, external, error)) { return false; }
        object.address = n.monitorAddress;
        n.allowInput = v(mode == 0 ? 3 : 4).toBool();
        n.sameMonitor = v(mode == 0 ? 27 : 20).toBool();
        n.fontSize = size;
        n.opacity = v(mode == 0 ? 40 : 24).toInt();
        n.textColor = fg.name(); n.backgroundColor = bg.name();
        n.macroEnabled = v(mode == 0 ? 18 : 16).toBool();
        n.macroName = v(mode == 0 ? 19 : 17).toString();
        if (mode == 0) {
            object.hasNumberDisplayProperties = true;
            n.totalDigits = v(9).toInt(); n.decimalDigits = v(10).toInt();
            n.inputMaximum = v(25).toString(); n.inputMinimum = v(26).toString();
            n.gain = v(38).toString(); n.offset = v(39).toString();
            n.highColor = dataDisplayColor(v(14).toString()).name();
            n.lowColor = dataDisplayColor(v(15).toString()).name();
        } else {
            object.asciiDisplay.characterCount = v(8).toInt();
        }
    } else {
        time.fontSize = size; time.textColor = fg.name(); time.backgroundColor = bg.name(); time.opacity = v(17).toInt();
        time.showDate = v(10).toBool(); time.showTime = v(11).toBool(); time.showWeek = v(12).toBool(); //20261128-5 change
        displayFormat = v(14).toString(); date = TimeDisplayProperties::dateFormats().indexOf(displayFormat);
        if (time.showDate && !time.showWeek && !time.showTime && date >= 0) {
            time.dateFormat = date;
        } else if (time.showDate && time.showTime && !time.showWeek) {
            const QStringList tokens = displayFormat.split(QLatin1Char(' '));
            time.dateFormat = tokens.size() == 2 ? TimeDisplayProperties::dateFormats().indexOf(tokens.first()) : -1;
            time.timeFormat = tokens.size() == 2 ? TimeDisplayProperties::timeFormats().indexOf(tokens.last()) : -1;
        }
        if (time.sample() != displayFormat || d.value(text + QStringLiteral("text.head.4")).toString() != displayFormat) {
            return fail(error, QStringLiteral("时间显示格式与原厂预览不一致或含未映射的日期时间组合。"));
        }
    }
    *out = object;
    return true;
}

/******************************************************************************
 * Function Name   : patchDisplayColor
 * Description     : Update both native RGB text and the QGraphicsTextItem cache, retaining its alpha.
 * Input Parameters: edits, private field, cache field, old and new colors
 * Return Value    : None.
 * Date            : 2026.11.27
 ******************************************************************************/
static void patchDisplayColor(SourceUpdates &u, const QString &field, const QString &cache, const QString &before, const QString &after)
{
    QColor color(after);
    if (before == after) { return; }
    u.set(field, QStringLiteral("%1+%2+%3").arg(color.red()).arg(color.green()).arg(color.blue()));
    color.setAlpha(u.document.value(cache).value<QColor>().alpha());
    u.set(cache, color);
}

/******************************************************************************
 * Function Name   : writeNativeDataDisplay
 * Description     : Patch verified property and scene fields in a native object, retain unmodified private and cached data.
 * Input Parameters: edited object, prior object, source document, object prefix, output, error
 * Return Value    : True when every requested edit has a verified native encoding.
 * Date            : 2026.11.27
 ******************************************************************************/
static bool writeNativeDataDisplay(const HmiObject &o, const HmiObject *original, const SourceDocument &input, QString p, QByteArray *output, QString *error)
{
    QFile file(QStringLiteral(":/native/data-%1.sm").arg(o.type));
    SourceDocument seed(QByteArray{});
    const SourceDocument *src = &input;
    HmiObject baseline;
    bool external = false;
    int mode;
    int size0;
    int size1;
    QString color0, color1, back0, back1;
    QString text;
    QString sample;
    QString newSample;
    QString oldAddress;
    QString newAddress;
    QJsonObject a, z;
    QStringList allowed;
    const NumberDisplayProperties *n0;
    const NumberDisplayProperties *n1;
    qsizetype end;
    if (!original) {
        if (!file.open(QIODevice::ReadOnly)) { return fail(error, QStringLiteral("缺少数据控件的原厂新建种子。")); }
        seed.bytes = file.readAll();
        if (!seed.parse() || seed.objectPrefixes.size() != 1) { return fail(error, seed.error); }
        src = &seed; p = seed.objectPrefixes.first();
        if (!sourceDataDisplay(seed, p, 0, &baseline, &external, error)) { return false; }
        original = &baseline;
    }
    mode = src->value(p + QStringLiteral("display.mode")).toInt();
    text = p + QStringLiteral("child[0].");
    const auto field = [&](int k) { return p + QStringLiteral("display.") + QString::number(k); };
    SourceUpdates u(*src);
    if (o.type != original->type || !o.text.isEmpty() || !o.extensions.isEmpty() || o.flipHorizontal || o.flipVertical ||
        (mode == 0 && !o.hasNumberDisplayProperties)) { return fail(error, QStringLiteral("数据控件含未映射的翻转、根文字、扩展或属性。")); }
    if (o.name != original->name || src == &seed) { u.set(p + QStringLiteral("group.metadata-name"), o.name); }
    if (o.geometry.topLeft() != original->geometry.topLeft() || o.subpixelOffset != original->subpixelOffset) {
        u.set(text + QStringLiteral("text.tail.10"), o.geometry.x() + o.subpixelOffset.x() - src->value(text + QStringLiteral("text.head.0")).toDouble());
        u.set(text + QStringLiteral("text.tail.11"), o.geometry.y() + o.subpixelOffset.y() - src->value(text + QStringLiteral("text.head.1")).toDouble());
    }
    if (o.geometry.width() != original->geometry.width()) { //20261128-5 change: Preserve an untouched fractional dimension when only its other axis is resized.
        u.set(text + QStringLiteral("text.head.2"), double(o.geometry.width()));
    }
    if (o.geometry.height() != original->geometry.height()) {
        u.set(text + QStringLiteral("text.head.3"), double(o.geometry.height()));
    }
    if (mode < 2) {
        n0 = mode == 0 ? &original->numberDisplay : &original->asciiDisplay.common;
        n1 = mode == 0 ? &o.numberDisplay : &o.asciiDisplay.common;
        a = n0->toJson(); z = n1->toJson();
        allowed = QStringList{QStringLiteral("fontSize"), QStringLiteral("textColor"), QStringLiteral("backgroundColor"),
                              QStringLiteral("monitorAddress"), QStringLiteral("inputAddress")};
        if (mode == 0) {
            allowed += QStringList{QStringLiteral("totalDigits"), QStringLiteral("decimalDigits")};
        } else {
            QJsonObject oldAscii = original->asciiDisplay.toJson();
            QJsonObject newAscii = o.asciiDisplay.toJson();
            oldAscii.remove(QStringLiteral("common")); newAscii.remove(QStringLiteral("common"));
            oldAscii.remove(QStringLiteral("characterCount")); newAscii.remove(QStringLiteral("characterCount"));
            if (oldAscii != newAscii) { return fail(error, QStringLiteral("ASCII显示器的编码或输入设置尚不能独立写回。")); }
        }
        size0 = n0->fontSize; size1 = n1->fontSize;
        color0 = n0->textColor; color1 = n1->textColor; back0 = n0->backgroundColor; back1 = n1->backgroundColor;
        oldAddress = n0->monitorAddress; newAddress = n1->monitorAddress;
        if (o.address != newAddress) { return fail(error, QStringLiteral("数据控件的根地址与监视地址不一致。")); }
        if (oldAddress != newAddress) {
            u.address(field(mode == 0 ? 17 : 13), newAddress);
            u.set(field(2), newAddress.startsWith(QStringLiteral("@com1:D")) ?
                            QStringLiteral("[Mitsubishi_Fx3U]") + newAddress.mid(6) : newAddress);
        }
        if (n1->inputAddress != n0->inputAddress) {
            //20261127-13 change: Preserve an explicitly edited input draft, including a dormant linked address.
            u.address(field(mode == 0 ? 28 : 21), n1->inputAddress);
        } else if (oldAddress != newAddress && n1->sameMonitor && n0->inputAddress == oldAddress) {
            u.address(field(mode == 0 ? 28 : 21), newAddress);
        }
        if ((mode == 0 && (n0->totalDigits != n1->totalDigits || n0->decimalDigits != n1->decimalDigits)) ||
            (mode == 1 && original->asciiDisplay.characterCount != o.asciiDisplay.characterCount) || oldAddress != newAddress) {
            const auto preview = [&](int digits, int decimals, const QString &address) {
                QString value;
                int i;
                for (i = 0; i < digits; ++i) { value.append(mode == 0 ? QLatin1Char('8') : QLatin1Char(char('A' + (i % 26)))); }
                if (mode == 0 && decimals > 0 && decimals < digits) { value.insert(digits - decimals, QLatin1Char('.')); }
                return value + QLatin1Char('\n') + (address.startsWith(QStringLiteral("@com1:D")) ? address.mid(6) : address);
            };
            if ((mode == 0 && n1->decimalDigits >= n1->totalDigits) || (mode == 1 && o.asciiDisplay.characterCount > 26)) {
                return fail(error, QStringLiteral("字符数或小数位组合的预览超出已核实范围。"));
            }
            sample = preview(mode == 0 ? n0->totalDigits : original->asciiDisplay.characterCount, n0->decimalDigits, oldAddress);
            newSample = preview(mode == 0 ? n1->totalDigits : o.asciiDisplay.characterCount, n1->decimalDigits, newAddress);
            if (src->value(text + QStringLiteral("text.head.4")).toString() != sample) {
                return fail(error, QStringLiteral("数据控件使用自定义预览，不能按标准数字或字符重建。"));
            }
            u.set(text + QStringLiteral("text.head.4"), newSample);
            if (mode == 0 && n0->totalDigits != n1->totalDigits) { u.set(field(9), n1->totalDigits); }
            if (mode == 0 && n0->decimalDigits != n1->decimalDigits) { u.set(field(10), n1->decimalDigits); }
            if (mode == 1 && original->asciiDisplay.characterCount != o.asciiDisplay.characterCount) { u.set(field(8), o.asciiDisplay.characterCount); }
        }
    } else {
        const TimeDisplayProperties &t0 = original->timeDisplay;
        const TimeDisplayProperties &t1 = o.timeDisplay;
        a = t0.toJson(); z = t1.toJson();
        allowed = QStringList{QStringLiteral("fontSize"), QStringLiteral("textColor"), QStringLiteral("backgroundColor"), QStringLiteral("dateFormat")};
        size0 = t0.fontSize; size1 = t1.fontSize; color0 = t0.textColor; color1 = t1.textColor; back0 = t0.backgroundColor; back1 = t1.backgroundColor;
        if (t0.dateFormat != t1.dateFormat) {
            sample = TimeDisplayProperties::dateFormats().value(t0.dateFormat);
            if (!t0.showDate || t0.showWeek || t0.showTime || src->value(field(14)).toString() != sample ||
                src->value(text + QStringLiteral("text.head.4")).toString() != sample) {
                return fail(error, QStringLiteral("当前源时间控件含组合格式，不能按单一日期格式替换。"));
            }
            newSample = TimeDisplayProperties::dateFormats().value(t1.dateFormat);
            u.set(field(14), newSample); u.set(text + QStringLiteral("text.head.4"), newSample);
        }
    }
    for (const QString &key : allowed) { a.remove(key); z.remove(key); }
    if (a != z) { return fail(error, QStringLiteral("数据控件含尚未建立原厂写入映射的属性更改。")); }
    if (size0 != size1) {
        const int sizeIndex = dataDisplaySizes(mode).indexOf(QString::number(size1));
        if (sizeIndex < 0) { return fail(error, QStringLiteral("显示器字号超出原厂字库范围。")); }
        u.set(field(mode == 0 ? 8 : mode == 1 ? 7 : 4), sizeIndex); u.set(text + QStringLiteral("font.tail.1"), size1);
    }
    patchDisplayColor(u, field(mode == 0 ? 12 : mode == 1 ? 11 : 8), text + QStringLiteral("text.tail.0"), color0, color1);
    patchDisplayColor(u, field(mode == 0 ? 13 : mode == 1 ? 12 : 9), text + QStringLiteral("text.tail.1"), back0, back1);
    if (!u.error.isEmpty()) { return fail(error, u.error); }
    end = src->values.value(mode == 2 ? field(17) : (field(mode == 0 ? 56 : 40) + QStringLiteral(".8"))).end;
    *output = u.render(src->values.value(p + QStringLiteral("type")).start, end);
    if (output->isEmpty()) { return fail(error, QStringLiteral("数据控件写入失败。")); }
    return true;
}
