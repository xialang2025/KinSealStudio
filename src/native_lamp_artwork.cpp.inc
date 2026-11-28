//20261128-20 add: Rasterize the two native stretched switch drawings from their installed vector definitions.
#include <QCache>
#include <QFile>
#include <QRegularExpression>
#include <QSvgRenderer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <cmath>

namespace {
/******************************************************************************
 * Function Name   : writeLampGradient
 * Description     : Quantize the opaque SVG stops to the native 1,024-entry eight-bit interpolation table.
 * Input Parameters: xml at the start of a gradient, output positioned inside the same gradient.
 * Return Value    : True when the original multi-stop table was fully consumed.
 * Date            : 2026.11.28
 ******************************************************************************/
bool writeLampGradient(QXmlStreamReader &xml, QXmlStreamWriter &output)
{
    QGradientStops stops;
    QXmlStreamAttributes attributes;
    QColor color;
    QColor left;
    QColor right;
    double position;
    int high = 0;
    int weight;
    int index;

    while (xml.readNextStartElement()) {
        if (xml.name() != u"stop") { return false; }
        attributes = xml.attributes();
        color = QColor::fromString(attributes.value(u"style").toString().section(u'#', 1).prepend(u'#'));
        position = attributes.value(u"offset").toDouble();
        if (!color.isValid() || (!stops.isEmpty() && stops.last().first >= position)) { return false; }
        stops.append({position, color});
        xml.skipCurrentElement();
    }
    if (xml.hasError() || stops.size() < 3) { return false; }
    for (index = 0; index < 1024; ++index) {
        position = (index + 0.5) / 1024.0;
        while (high < stops.size() && stops.at(high).first < position) { ++high; }
        if (index == 0 || high == 0) {
            color = stops.first().second;
        } else if (index == 1023 || high == stops.size()) {
            color = stops.last().second;
        } else {
            left = stops.at(high - 1).second;
            right = stops.at(high).second;
            weight = qRound(256 * (position - stops.at(high - 1).first) /
                            (stops.at(high).first - stops.at(high - 1).first));
            color.setRgb((left.red() * (256 - weight) + right.red() * weight) >> 8,
                         (left.green() * (256 - weight) + right.green() * weight) >> 8,
                         (left.blue() * (256 - weight) + right.blue() * weight) >> 8);
        }
        output.writeEmptyElement(QStringLiteral("stop"));
        output.writeAttribute(QStringLiteral("offset"), QString::number(position, 'g', 15));
        output.writeAttribute(QStringLiteral("stop-color"), color.name());
    }
    return true;
}

/******************************************************************************
 * Function Name   : lampPathCoordinate
 * Description     : Truncate a stretched vector vertex on the original 26.6 device grid.
 * Input Parameters: coordinate in the view box, axis origin, view extent and output pixel count.
 * Return Value    : Coordinate in the source SVG system after device-space quantization.
 * Date            : 2026.11.28
 ******************************************************************************/
double lampPathCoordinate(double value, double origin, double extent, int pixels)
{
    const double device = (value - origin) * pixels / extent;
    // Native device-space edge vertices land just before the exact grid line on all four sides.
    const double ticks = qAbs(device) < 1e-7 ? -1.0 :
                         qAbs(device - pixels) < 1e-7 ? pixels * 64.0 - 1 : std::floor(device * 64 + 1e-9);
    return origin + ticks * extent / (64 * pixels);
}

/******************************************************************************
 * Function Name   : nativeLampArtwork
 * Description     : Render the bounded rectangle/polygon subset used by the native default lamp artwork.
 * Input Parameters: current state and device sprite dimensions.
 * Return Value    : Premultiplied layer, or a null image for artwork handled by the ordinary painter.
 * Date            : 2026.11.28
 ******************************************************************************/
QImage nativeLampArtwork(const BitButtonState &state, const QSize &size)
{
    static thread_local QCache<QString, QImage> cache(8192);
    static const QRegularExpression separators(QStringLiteral("[,\\s]+"));
    QString resource;
    QString cacheKey;
    QFile source;
    QByteArray prepared;
    QXmlStreamReader xml;
    QXmlStreamWriter output(&prepared);
    QXmlStreamAttributes attributes;
    QString tag;
    QString key;
    QString value;
    QStringList numbers;
    QRectF view;
    QImage layer;
    QPainter painter;
    QSvgRenderer renderer;
    QImage *cached;
    QPointF start;
    QPointF finish;
    double coordinate;
    int index;

    if (!state.useImage || !state.picturePng.isEmpty() || state.imageMode != 0) { return {}; }
    resource = kvgPictureResource(state.picture);
    if (resource != QStringLiteral(":/system-gallery/default/switch_ON.kvg") &&
        resource != QStringLiteral(":/system-gallery/default/switch_OFF.kvg")) { return {}; }
    cacheKey = resource + QStringLiteral("/%1x%2").arg(size.width()).arg(size.height());
    cached = cache.object(cacheKey);
    if (cached != nullptr) { return *cached; }
    source.setFileName(resource);
    if (!source.open(QIODevice::ReadOnly)) { return {}; }
    xml.setDevice(&source);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement()) { output.writeEndElement(); }
        if (!xml.isStartElement()) { continue; }
        attributes = xml.attributes();
        tag = xml.name().toString();
        output.writeStartElement(tag);
        if (tag == u"svg") {
            output.writeDefaultNamespace(QStringLiteral("http://www.w3.org/2000/svg"));
            numbers = attributes.value(u"viewBox").toString().split(separators, Qt::SkipEmptyParts);
            if (numbers.size() != 4) { return {}; }
            view = QRectF(numbers.at(0).toDouble(), numbers.at(1).toDouble(),
                          numbers.at(2).toDouble(), numbers.at(3).toDouble());
            if (view.isEmpty()) { return {}; }
        } else if (tag == u"rect") {
            start = QPointF(attributes.value(u"x").toDouble(), attributes.value(u"y").toDouble());
            finish = start + QPointF(attributes.value(u"width").toDouble(), attributes.value(u"height").toDouble());
            start = QPointF(lampPathCoordinate(start.x(), view.x(), view.width(), size.width()),
                            lampPathCoordinate(start.y(), view.y(), view.height(), size.height()));
            finish = QPointF(lampPathCoordinate(finish.x(), view.x(), view.width(), size.width()),
                             lampPathCoordinate(finish.y(), view.y(), view.height(), size.height()));
        }
        for (const QXmlStreamAttribute &attribute : attributes) {
            key = attribute.name().toString();
            value = attribute.value().toString();
            if (tag == u"rect") {
                if (key == u"x") { value = QString::number(start.x(), 'g', 16); }
                if (key == u"y") { value = QString::number(start.y(), 'g', 16); }
                if (key == u"width") { value = QString::number(finish.x() - start.x(), 'g', 16); }
                if (key == u"height") { value = QString::number(finish.y() - start.y(), 'g', 16); }
            } else if (tag == u"polygon" && key == u"points") {
                numbers = value.split(separators, Qt::SkipEmptyParts);
                if (numbers.size() % 2 != 0) { return {}; }
                for (index = 0; index < numbers.size(); ++index) {
                    coordinate = numbers.at(index).toDouble();
                    coordinate = (index & 1) ? lampPathCoordinate(coordinate, view.y(), view.height(), size.height()) :
                                               lampPathCoordinate(coordinate, view.x(), view.width(), size.width());
                    numbers[index] = QString::number(coordinate, 'g', 16);
                }
                value = numbers.join(u' ');
            }
            output.writeAttribute(key, value);
        }
        if (tag == u"linearGradient" || tag == u"radialGradient") {
            if (!writeLampGradient(xml, output)) { return {}; }
            output.writeEndElement();
        }
    }
    if (xml.hasError()) { return {}; }
    if (!renderer.load(prepared)) { return {}; }
    layer = QImage(size, QImage::Format_ARGB32_Premultiplied);
    if (layer.isNull()) { return {}; }
    layer.fill(Qt::transparent);
    if (!painter.begin(&layer)) { return {}; }
    renderer.render(&painter, QRectF(QPointF(), size));
    painter.end();
    cache.insert(cacheKey, new QImage(layer), int((layer.sizeInBytes() + 1023) / 1024));
    return layer;
}
}
