#include "bit_button_payload_compiler.h"
#include "bit_button_renderer.h"
#include "text_payload_compiler.h"
#include "static_text_renderer.h" //20261109-0 add
#include "kvg_library.h" //20261109-0 add

#include <QImage>
#include <QPainter>

/******************************************************************************
 * Function Name   : appendLittleEndian32
 * Description     : Append one unsigned length in the native picture index byte order.
 * Input Parameters: bytes, value
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
static void appendLittleEndian32(QByteArray *bytes, u32 value) //20261109-0 add
{
    int index;

    for (index = 0; index < 4; ++index)
    {
        bytes->append(char((value >> (index * 8)) & 0xff));
    }
}

/******************************************************************************
 * Function Name   : buildBitButtonSprites
 * Description     : Preserve odd-width rows and pad each verified LZO stream to 32 bytes.
 * Input Parameters: object, language, stateIndices, sprites, error
 * Return Value    : True for the requested ordered OFF/ON images within the target screen.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildBitButtonSprites(const HmiObject &object, const QString &language,
                            const QList<int> &stateIndices, BitButtonSpriteSet *sprites,
                            QString *error, const QImage &underlay,
                            const QList<HmiObject> &overlays) //20261109-0 add
{
    BitButtonSpriteSet generated;
    QImage image;
    QPainter painter;
    QByteArray pixels;
    QByteArray compressed;
    QString imageWarning; //20261109-0 add
    BitButtonState renderedState;
    const BitButtonState *sourceState; //20261109-8 add
    HmiObject overlay;
    int width;
    int height;
    int stateIndex;
    int row;
    int variant;
    int variantCount;
    int activeStateCount; //20261110-0 add
    u32 paddedLength;

    width = object.geometry.width();
    height = object.geometry.height();
    activeStateCount = object.hasFunctionButtonProperties ? (object.functionButton.pressedEffect ? 2 : 1) :
                       object.hasLampProperties ? object.lamp.stateCount() : 2; //20261110-0 add
    if (sprites == nullptr || object.type != QStringLiteral("switch") ||
        !object.hasBitButtonProperties || width <= 0 || width > 800 || height <= 0 || height > 480 ||
        stateIndices.isEmpty() || stateIndices.size() > activeStateCount ||
        !object.bitButton.validate(error, object.hasFunctionButtonProperties) || //20261110-0 change: Native function buttons allow a zero press-time setting.
        (object.hasLampProperties && !object.lamp.validate(error)) ||
        (!underlay.isNull() && underlay.size() != object.geometry.size()))
    {
        if (error != nullptr && error->isEmpty())
        {
            *error = QStringLiteral("位按钮状态图参数无效。");
        }
        return false;
    }
    Q_FOREACH (stateIndex, stateIndices)
    {
        sourceState = object.hasLampProperties ? object.lamp.state(object.bitButton, stateIndex) :
                      (stateIndex >= 0 && stateIndex < 2 ? &object.bitButton.states[stateIndex] : nullptr); //20261109-8 change
        if (stateIndex < 0 || stateIndex >= activeStateCount || sourceState == nullptr)
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("状态图索引超出当前控件的有效状态范围。");
            }
            return false;
        }
        if (sourceState->useImage) //20261109-8 change: Preview placeholders are never valid device artwork.
        {
            renderedState = *sourceState;
            imageWarning = renderedState.picturePng.isEmpty() ? kvgPictureWarning(renderedState.picture) : QString();
            image = renderedState.picturePng.isEmpty() ? kvgPictureImage(renderedState.picture, QSize(1, 1)) :
                                                        QImage::fromData(renderedState.picturePng, "PNG");
            if (!imageWarning.isEmpty() || image.isNull())
            {
                if (error != nullptr)
                {
                    *error = QStringLiteral("位按钮状态%1的图库资源不可用：%2。%3")
                        .arg(stateIndex).arg(renderedState.picture, imageWarning);
                }
                return false;
            }
        }
        variantCount = sourceState->flash && !(object.hasLampProperties && object.lamp.monitorMode == 2) &&
                       !(object.hasFunctionButtonProperties && stateIndex == 1) ? 2 : 1; //20261110-0 change: Functions emit normal/flash OFF followed by the hidden pressed ON frame.
        for (variant = 0; variant < variantCount; ++variant) //20261109-0 change: A flashing state is immediately followed by its text-free image.
        {
            renderedState = *sourceState;
            renderedState.effect3d = false; //20261109-0 change: Native 3D samples preserve identical pixels; firmware applies the global 0x108 effect.
            if (variant == 1)
            {
                renderedState.sharedText.clear();
                renderedState.text.languageTexts.clear();
            }
            if (underlay.isNull())
            {
                image = QImage(width, height, QImage::Format_RGB16);
                image.fill(QColor(197, 223, 255));
            }
            else
            {
                image = underlay.convertToFormat(QImage::Format_RGB16).copy(); //20261109-0 add: Preserve static pixels beneath translucent button content.
            }
            painter.begin(&image);
            paintBitButton(&painter, image.rect(), renderedState, language);
            painter.translate(-object.geometry.topLeft()); //20261109-0 add: Static objects above this control must remain above every normal and flash image.
            Q_FOREACH (overlay, overlays)
            {
                paintStaticText(&painter, overlay.geometry, staticTextContent(overlay, language), overlay.staticText);
            }
            painter.end();
            pixels.clear();
            pixels.reserve(width * height * 2);
            //20261109-0 add: QImage pads odd-width rows; the device image stream does not.
            for (row = 0; row < height; ++row)
            {
                pixels.append(reinterpret_cast<const char *>(image.constScanLine(row)), width * 2);
            }
            if (!compressRgb565Pixels(pixels, &compressed, error))
            {
                return false;
            }
            paddedLength = (u32(compressed.size()) + 31) & ~u32(31);
            appendLittleEndian32(&generated.lengthRecords, 0); // Plain LZO image encoding, no alternate native codec.
            appendLittleEndian32(&generated.lengthRecords, paddedLength);
            appendLittleEndian32(&generated.lengthRecords, u32(compressed.size()));
            generated.pixelData.append(compressed);
            generated.pixelData.append(QByteArray(int(paddedLength) - compressed.size(), char(0)));
            generated.rawLength += u32(pixels.size());
        }
    }
    *sprites = generated;
    return true;
}
