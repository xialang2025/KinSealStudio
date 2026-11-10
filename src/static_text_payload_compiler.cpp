#include "static_text_payload_compiler.h"
#include "static_text_renderer.h"
#include "text_payload_compiler.h"

#include <QImage>
#include <QPainter>
#include <QtEndian>

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
        paintStaticText(&painter, object.geometry, staticTextContent(object, language), object.staticText);
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
