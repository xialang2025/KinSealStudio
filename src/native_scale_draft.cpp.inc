/******************************************************************************
 * Function Name   : writeNewNativeScale
 * Description     : Build a V122 scale from an isolated, typed vendor object and apply the current canvas properties.
 * Input Parameters: new scale object, resulting type-31 object body, optional diagnostic
 * Return Value    : True when its scene graph can be regenerated without using any donor project settings.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool writeNewNativeScale(const HmiObject &obj, QByteArray *output, QString *error) //20261128-35 add
{
    const QString prefix = QStringLiteral("object[0].");
    const ScaleProperties &scale = obj.scale;
    QString form;
    QFile file;
    SourceDocument donor(QByteArray{});
    HmiObject original;
    int type;

    if (!output || obj.type != QStringLiteral("scale") || !scale.validate(error)) { return false; }
    if (scale.position == 0) {
        form = QStringLiteral("position-top");
    } else if (scale.majorCount == 6) {
        form = QStringLiteral("main6");
    } else if (scale.minorCount == 4) {
        form = QStringLiteral("sub4");
    } else if (scale.minimum >= 0 && scale.maximum < 10) {
        // Single-digit labels use a shorter cached text box and a different horizontal line span.
        form = QStringLiteral("equal-zero");
    } else {
        form = QStringLiteral("default");
    }
    file.setFileName(QStringLiteral(":/native/scale-%1.body").arg(form));
    if (!file.open(QIODevice::ReadOnly)) { return fail(error, QStringLiteral("缺少原厂刻度对象体：%1").arg(form)); }
    donor.bytes = file.readAll();
    // Only type through private.17 are stored in the resource. Supply the schema version to the child parser.
    donor.values[QStringLiteral("header.1")].value = 122;
    type = donor.read('i', prefix + QStringLiteral("type")).toInt();
    if (type != 31) { return fail(error, QStringLiteral("刻度对象体的类型字段无效。")); }
    readRetainedRecord(donor, prefix, type);
    if (!donor.error.isEmpty() || donor.values.value(prefix + QStringLiteral("private.17")).end != donor.bytes.size()) {
        return fail(error, donor.error.isEmpty() ? QStringLiteral("刻度对象体边界与 V122 结构不符。") : donor.error);
    }
    if (!sourceNativeScale(donor, prefix, 0, &original, error)) { return false; }
    // The common writer rejects incompatible size, style, topology and unattested label metrics before rendering.
    return writeNativeScale(obj, original, donor, prefix, output, error);
}
