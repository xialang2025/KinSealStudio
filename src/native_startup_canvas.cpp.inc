//20261128-5 add: Original V122 boot artwork and offline Mitsubishi word-address evidence.

/******************************************************************************
 * Function Name   : hasCanvasStartupArtwork
 * Description     : Verify the factory's independent boot page and its exact embedded image binding.
 * Input Parameters: fully parsed native document
 * Return Value    : True for a single ordinary page followed by a static 800x480 boot image.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool hasCanvasStartupArtwork(const SourceDocument &document)
{
    const QString reference = document.value(QStringLiteral("system.tail.1")).toString();
    const QString record = QStringLiteral("system.194[0].");
    const QString format = document.value(record + QLatin1Char('2')).toString().toUpper();
    QByteArray image = document.value(record + QLatin1Char('5')).toByteArray();
    QBuffer buffer(&image);
    QImageReader reader(&buffer, format.toLatin1());

    if (document.value(QStringLiteral("header.1")).toInt() != 122 ||
        document.value(QStringLiteral("project.pages.count")).toInt() != 1 ||
        !document.values.contains(QStringLiteral("startup.head.0")) ||
        document.value(QStringLiteral("startup.objects.count")).toInt() != 0 ||
        document.value(QStringLiteral("startup.94.count")).toInt() != 0 ||
        document.value(QStringLiteral("startup.head.6")).toInt() != 1 ||
        document.value(QStringLiteral("startup.head.2")).toDouble() != 800.0 ||
        document.value(QStringLiteral("startup.head.3")).toDouble() != 480.0 ||
        document.value(QStringLiteral("system.194.count")).toInt() != 1 ||
        reference.isEmpty() || document.value(record + QLatin1Char('0')).toString() != reference ||
        document.value(QStringLiteral("startup.head.8")).toString() != reference ||
        document.value(record + QLatin1Char('3')).toInt() != 800 ||
        document.value(record + QLatin1Char('4')).toInt() != 480 ||
        (format != QStringLiteral("PNG") && format != QStringLiteral("JPG")) ||
        image.isEmpty() || !buffer.open(QIODevice::ReadOnly)) {
        return false;
    }
    return reader.canRead() && reader.size() == QSize(800, 480) && !reader.read().isNull();
}

/******************************************************************************
 * Function Name   : hasFactoryFxClockConnection
 * Description     : Verify the COM1 Mitsubishi protocol/station and both copies of its protocol timing.
 * Input Parameters: parsed factory source
 * Return Value    : True when the source can represent the verified clock sample's D registers.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool hasFactoryFxClockConnection(const SourceDocument &document)
{
    const QString link = QStringLiteral("system.16c[0].50[0].");
    const QString head = link + QStringLiteral("head.");
    const QString tail = link + QStringLiteral("tail.");
    const QString driver = QStringLiteral("Mitsubishi_Fx3U");
    const int timing[] = {20, 32, 0, 1000, 3};
    int index;

    if (!isAmz070Target(document.value(QStringLiteral("system.identity.1")).toString()) ||
        document.value(QStringLiteral("system.16c[0].header.11")).toString() != QStringLiteral("COM1") ||
        document.value(QStringLiteral("system.16c[0].50.count")).toInt() != 1 ||
        document.value(link + QStringLiteral("40[7]")).toString() != QStringLiteral("D") ||
        document.value(tail + QStringLiteral("9")).toString() != QStringLiteral("MITSUBISHI") ||
        document.value(tail + QStringLiteral("0")).toString() != driver ||
        document.value(tail + QStringLiteral("10")).toString() != driver ||
        document.value(tail + QStringLiteral("11")).toString() != driver ||
        document.value(tail + QStringLiteral("1")).toInt() != 1) {
        return false;
    }
    for (index = 0; index < 5; ++index) {
        if (document.value(head + QString::number(index)).toInt() != timing[index] ||
            document.value(tail + QString::number(index + 4)).toInt() != timing[index]) {
            return false;
        }
    }
    return true;
}
