#include "static_text_payload_compiler.h"
#include "static_text_renderer.h"
#include "text_payload_compiler.h"

#include <QImage>
#include <QPainter>
#include <QtEndian>

/******************************************************************************
 * Function Name   : stackedTextChannel
 * Description     : Composite the solid face twice in native page-color precision before RGB565 truncation.
 * Input Parameters: source channel, page-background channel and fill alpha
 * Return Value    : Eight-bit channel for the isolated upper language sprite.
 * Date            : 2026.11.28
 ******************************************************************************/
static int stackedTextChannel(int source, int background, int alpha) //20261128-14 add
{
    auto multiply = [](int channel, int weight) {
        int product = channel * weight;
        return (product + (product >> 8) + 128) >> 8;
    };
    int foreground = multiply(source, alpha);
    int first = foreground + multiply(background, 255 - alpha);
    return foreground + multiply(first, 255 - alpha);
}

/******************************************************************************
 * Function Name   : prepareIndependentStaticTextStack
 * Description     : Recognize the tested self-contained, bilingual type-28 pair and prepare its upper sprite.
 * Input Parameters: two objects in compiler draw order, saved languages and destination sprite object
 * Return Value    : True after preparing an independent sprite; false leaves the output unchanged.
 * Date            : 2026.11.28
 ******************************************************************************/
bool prepareIndependentStaticTextStack(const QList<HmiObject> &objects, const QStringList &languages,
                                       HmiObject *upperSprite) //20261128-14 add
{
    HmiObject upper;
    QColor fill;
    int alpha;

    if (!upperSprite || objects.size() != 2 || languages.size() != 2) { return false; }
    const HmiObject &lower = objects.at(0);
    const HmiObject &top = objects.at(1);
    if (lower.layer >= top.layer || !lower.geometry.contains(top.geometry) ||
        lower.geometry.x() < 1 || lower.geometry.y() < 1 ||
        lower.type != QStringLiteral("text") || top.type != QStringLiteral("text") ||
        !lower.hasStaticTextProperties || !top.hasStaticTextProperties ||
        lower.hasAppearanceFlip() || top.hasAppearanceFlip() ||
        lower.staticText.visibilityEnabled || top.staticText.visibilityEnabled ||
        lower.staticText.style != 2 || lower.staticText.opacity != 255 ||
        lower.staticText.foregroundColor.alpha() != 255 || lower.staticText.backgroundColor.alpha() != 255 ||
        !staticTextContent(lower, languages.first()).isEmpty() || !staticTextContent(lower, languages.last()).isEmpty() ||
        staticTextContent(top, languages.first()) != staticTextContent(top, languages.last()) ||
        top.staticText.textColor.alpha() != 255 || top.staticText.backgroundColor.alpha() != 255) {
        return false;
    }
    if (!((top.staticText.style == 0 && top.staticText.opacity == 255) ||
          (top.staticText.style == 1 && top.staticText.opacity == 128))) {
        return false;
    }
    upper = top;
    if (upper.staticText.style == 1) {
        // Only the upper resource's color changes. Its control record and editable source retain the original opacity.
        alpha = upper.staticText.opacity;
        fill = upper.staticText.backgroundColor;
        upper.staticText.backgroundColor = QColor(stackedTextChannel(fill.red(), 197, alpha),
                                                  stackedTextChannel(fill.green(), 222, alpha),
                                                  stackedTextChannel(fill.blue(), 255, alpha));
        upper.staticText.opacity = 255;
    }
    *upperSprite = upper;
    return true;
}

/******************************************************************************
 * Function Name   : paintNativeHorizontalTextFill
 * Description     : Sample V122's opaque horizontal gradient into a single device scanline.
 * Input Parameters: active painter, integral control rectangle, static text colors
 * Return Value    : None
 * Date            : 2026.11.28
 ******************************************************************************/
static void paintNativeHorizontalTextFill(QPainter &painter, const QRect &rect,
                                          const StaticTextProperties &style) //20261128-15 add
{
    QImage row(rect.width(), 1, QImage::Format_RGB32);
    const QColor start = style.foregroundColor;
    const QColor end = style.backgroundColor;
    const int increment = (1023 * 256) / rect.width();
    int position = (1023 * 128) / rect.width();
    int index;
    int weight;
    int x;

    // The old raster engine advances its palette coordinate in 24.8 fixed point.
    // Each of the 1024 samples then interpolates the two colors using 8-bit weights.
    for (x = 0; x < rect.width(); ++x) {
        index = qBound(0, (position + 128) >> 8, 1023);
        weight = index == 0 ? 0 : index == 1023 ? 256 : (2 * index + 5) / 8;
        row.setPixel(x, 0, qRgb((start.red() * (256 - weight) + end.red() * weight) >> 8,
                                (start.green() * (256 - weight) + end.green() * weight) >> 8,
                                (start.blue() * (256 - weight) + end.blue() * weight) >> 8));
        position += increment;
    }
    painter.drawImage(rect, row);
}

/******************************************************************************
 * Function Name   : buildStaticTextSprites
 * Description     : Render bounded text images in saved language order and construct native LZO indices.
 * Input Parameters: object, languages, sprites, error
 * Return Value    : True with a complete image set; outputs survive any failure.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildStaticTextSprites(const HmiObject &object, const QStringList &languages,
                            StaticTextSpriteSet *sprites, QString *error) //20261109-0 add
{
    StaticTextSpriteSet result;
    StaticTextProperties textStyle; //20261128-15 add
    QImage scene;
    QImage image;
    QPainter painter;
    QString language;
    QByteArray pixels;
    QByteArray compressed;
    QByteArray index;
    u32 padded;
    int row;

    if (sprites == nullptr || object.type != QStringLiteral("text") || object.geometry.isEmpty() ||
        object.geometry.x() < 0 || object.geometry.y() < 0 || object.geometry.right() >= 800 ||
        object.geometry.bottom() >= 480 || languages.isEmpty() || languages.size() > 8 ||
        !object.staticText.validate(error))
    {
        if (error != nullptr && error->isEmpty()) { *error = QStringLiteral("静态文本状态图参数无效。"); }
        return false;
    }
    Q_FOREACH (language, languages)
    {
        scene = QImage(800, 480, QImage::Format_RGB16); scene.fill(Qt::black);
        painter.begin(&scene);
        painter.fillRect(QRect(1, 1, 799, 479), QColor(197, 223, 255));
        painter.setWorldTransform(object.appearanceTransform(object.geometry), true); //20261124-15 add
        textStyle = object.staticText;
        if (textStyle.style == 2 && textStyle.opacity == 255 && textStyle.foregroundColor.alpha() == 255 &&
            textStyle.backgroundColor.alpha() == 255) { //20261128-15 add
            paintNativeHorizontalTextFill(painter, object.geometry, textStyle);
            textStyle.style = 0;  // The remaining pass draws the foreground caption on the finished fill.
        }
        paintStaticText(&painter, object.geometry, staticTextContent(object, language), textStyle);
        painter.end();
        image = scene.copy(object.geometry);
        pixels.clear(); pixels.reserve(image.width() * image.height() * 2);
        for (row = 0; row < image.height(); ++row)
        {
            pixels.append(reinterpret_cast<const char *>(image.constScanLine(row)), image.width() * 2);
        }
        if (!compressRgb565Pixels(pixels, &compressed, error)) { return false; }
        padded = (u32(compressed.size()) + 31) & ~u32(31);
        index = QByteArray(12, char(0));
        qToLittleEndian<u32>(padded, index.data() + 4);
        qToLittleEndian<u32>(u32(compressed.size()), index.data() + 8);
        result.lengthRecords.append(index);
        result.pixelData.append(compressed);
        result.pixelData.append(QByteArray(int(padded) - compressed.size(), char(0)));
        result.rawLength += u32(pixels.size());
    }
    *sprites = result;
    if (error != nullptr) { error->clear(); }
    return true;
}
