#include "control_rendering.h"
#include <QBuffer>
#include <QCache>
#include <QCryptographicHash>
#include <QFontInfo>
#include <QImageReader>
#include <QMutex>
#include <QMutexLocker>
#include <QtMath>

/******************************************************************************
 * Function Name   : decodedControlPng
 * Description     : Decode bounded PNG pixels by content identity without caching invalid inputs or rendered control state.
 * Input Parameters: png, error
 * Return Value    : Premultiplied source pixels, or a null image with a diagnostic.
 * Date            : 2026.11.10
 ******************************************************************************/
QImage decodedControlPng(const QByteArray &png, QString *error) //20261110-8 add
{
    static QCache<QByteArray, QImage> cache(32 * 1024);
    static QMutex mutex;
    QByteArray identity;
    QImage image;
    QImage *cached;
    QBuffer buffer;
    QImageReader reader;
    QSize size;
    int cost;

    if (error != nullptr) { error->clear(); }
    if (png.isEmpty() || png.size() > 16 * 1024 * 1024 ||
        !png.startsWith(QByteArray::fromHex(QByteArrayLiteral("89504e470d0a1a0a")))) {
        if (error != nullptr) { *error = QStringLiteral("工程图片必须为不超过16 MiB的有效PNG。"); }
        return image;
    }
    identity = QCryptographicHash::hash(png, QCryptographicHash::Sha256);
    {
        QMutexLocker locker(&mutex);
        cached = cache.object(identity);
        if (cached != nullptr) { return *cached; }
    }
    buffer.setData(png);
    buffer.open(QIODevice::ReadOnly);
    reader.setDevice(&buffer);
    reader.setFormat(QByteArrayLiteral("PNG"));
    size = reader.size();
    if (!size.isValid() || size.width() > 4096 || size.height() > 4096) {
        if (error != nullptr) { *error = QStringLiteral("工程PNG尺寸必须为1..4096像素。"); }
        return image;
    }
    image = reader.read();
    if (image.isNull() || image.size() != size) {
        if (error != nullptr) { *error = QStringLiteral("工程PNG内容不完整或无法解码。"); }
        return QImage();
    }
    //20261110-8 change: Preserve the reader's original channel precision; eager premultiplication changes RGB565 alpha rounding.
    cost = qMax(1, int((image.sizeInBytes() + 1023) / 1024));
    if (cost <= cache.maxCost()) {
        QMutexLocker locker(&mutex);
        cache.insert(identity, new QImage(image), cost);
    }
    return image;
}

/******************************************************************************
 * Function Name   : controlPixelFont
 * Description     : Express HMI font sizes in page pixels independently of the editor device's logical DPI.
 * Input Parameters: properties
 * Return Value    : Font carrying the shared style and pixel-size contract.
 * Date            : 2026.11.10
 ******************************************************************************/
QFont controlPixelFont(const StaticTextProperties &properties, int logicalDpiY) //20261110-8 add
{
    QFont font(properties.fontFamily);
    if (properties.fontSize == qRound(properties.fontSize)) { font.setPixelSize(qRound(properties.fontSize)); }
    else { // Fractional pixel sizes require floating point point units resolved against this paint device's logical DPI.
        font.setPointSizeF(properties.fontSize * 72.0 / qMax(1, logicalDpiY));
    }
    font.setLetterSpacing(QFont::AbsoluteSpacing, properties.spacing);
    font.setItalic(properties.italic);
    font.setBold(properties.bold);
    font.setUnderline(properties.underline);
    font.setStyleStrategy(QFont::NoAntialias);
    return font;
}

/******************************************************************************
 * Function Name   : controlFontDiagnostics
 * Description     : Report actual host font resolution without claiming that Qt4 and Qt6 glyph rasterizers are identical.
 * Input Parameters: properties
 * Return Value    : Requested and resolved font identity, pixel contract and Qt version.
 * Date            : 2026.11.10
 ******************************************************************************/
QJsonObject controlFontDiagnostics(const StaticTextProperties &properties) //20261110-8 add
{
    QFont font = controlPixelFont(properties);
    QFontInfo info(font);
    QJsonObject result;
    result.insert(QStringLiteral("requestedFamily"), properties.fontFamily);
    result.insert(QStringLiteral("resolvedFamily"), info.family());
    result.insert(QStringLiteral("exactMatch"), info.exactMatch());
    result.insert(QStringLiteral("requestedPixelSize"), properties.fontSize);
    result.insert(QStringLiteral("fractionalPointSizeF"), font.pointSizeF());
    result.insert(QStringLiteral("fractionalReferenceDpiY"), 96);
    result.insert(QStringLiteral("resolvedPixelSize"), info.pixelSize());
    result.insert(QStringLiteral("letterSpacingPixels"), properties.spacing);
    result.insert(QStringLiteral("qtVersion"), QString::fromLatin1(qVersion()));
    result.insert(QStringLiteral("nativePixelEquivalenceConfirmed"), false);
    return result;
}
