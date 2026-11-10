#include "bit_button_renderer.h"
#include "static_text_renderer.h"
#include "kvg_library.h" //20261109-0 add
#include <QImage>
#include <QFont> //20261109-0 add
#include <QLinearGradient>
#include <QPainter>
#include <QPolygonF>

/******************************************************************************
 * Function Name   : paintBitButtonCaption
 * Description     : Draw native button captions at the original rectangle baseline.
 * Input Parameters: painter, rect, content, properties
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
static void paintBitButtonCaption(QPainter *painter, const QRectF &rect, const QString &content,
                                   const StaticTextProperties &properties) //20261109-0 add
{
    QFont font(properties.fontFamily);
    int alignment;

    painter->save();
    painter->setClipRect(rect, Qt::IntersectClip);
    painter->setRenderHint(QPainter::TextAntialiasing, false);
    font.setPointSizeF(properties.fontSize * 0.75);
    font.setLetterSpacing(QFont::AbsoluteSpacing, properties.spacing);
    font.setItalic(properties.italic);
    font.setBold(properties.bold);
    font.setUnderline(properties.underline);
    font.setStyleStrategy(QFont::NoAntialias);
    painter->setFont(font);
    painter->setPen(properties.textColor);
    alignment = Qt::AlignVCenter | (properties.alignment == 1 ? Qt::AlignLeft :
                                    properties.alignment == 2 ? Qt::AlignRight : Qt::AlignHCenter);
    // Original ON/OFF captions match this rectangle exactly; static text has a separate calibration.
    painter->drawText(rect, alignment, content);
    painter->restore();
}

/******************************************************************************
 * Function Name   : bitButtonBuiltinPictures
 * Description     : Keep the four legacy identifiers mapped to recovered native SVG resources.
 * Input Parameters: None
 * Return Value    : Supported built-in shape identifiers.
 * Date            : 2026.11.08
 ******************************************************************************/
QStringList bitButtonBuiltinPictures() //20261109-0 change
{
    return {QStringLiteral("switch_OFF.kvg"), QStringLiteral("switch_ON.kvg"),
            QStringLiteral("default.kvg"), QStringLiteral("gray.kvg")};
}

/******************************************************************************
 * Function Name   : bitButtonContent
 * Description     : Resolve shared-language text without replacing any saved per-language drafts.
 * Input Parameters: state, language
 * Return Value    : Current editable display text.
 * Date            : 2026.11.08
 ******************************************************************************/
QString bitButtonContent(const BitButtonState &state, const QString &language) //20261108-15 add
{
    return state.sameLanguage ? state.sharedText : state.text.languageTexts.value(language); //20261109-0 change: Newly added native language rows start empty.
}

/******************************************************************************
 * Function Name   : bitButtonNeedsUnderlay
 * Description     : Detect exposed artwork alpha at the device size without rejecting fully opaque pictures.
 * Input Parameters: state, size: bounded device control dimensions.
 * Return Value    : True when image alpha, opacity or the terminal transparency flag needs lower pixels.
 * Date            : 2026.11.09
 ******************************************************************************/
bool bitButtonNeedsUnderlay(const BitButtonState &state, const QSize &size) //20261109-4 add
{
    BitButtonState artworkState;
    QImage coverage;
    QPainter painter;
    const QRgb *row;
    int x;
    int y;

    if (state.transparent || state.opacity < 255) { return true; }
    if (!state.useImage) { return false; }
    if (size.isEmpty() || size.width() > 800 || size.height() > 480) { return true; }
    artworkState = state; artworkState.sameLanguage = true; artworkState.sharedText.clear();
    artworkState.effect3d = false; // Device effects and captions cannot establish that the artwork itself covers lower controls.
    coverage = QImage(size, QImage::Format_ARGB32_Premultiplied); coverage.fill(Qt::transparent);
    if (coverage.isNull()) { return true; } // Failed coverage allocation cannot establish that an image is opaque.
    painter.begin(&coverage); paintBitButton(&painter, coverage.rect(), artworkState, QString()); painter.end();
    for (y = 0; y < coverage.height(); ++y)
    {
        row = reinterpret_cast<const QRgb *>(coverage.constScanLine(y));
        for (x = 0; x < coverage.width(); ++x)
        {
            if (qAlpha(row[x]) < 255) { return true; }
        }
    }
    return false;
}

/******************************************************************************
 * Function Name   : paintBitButton
 * Description     : Render a selected OFF/ON state consistently in canvas, library icons and preview.
 * Input Parameters: painter, rect, state, language, pressed
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void paintBitButton(QPainter *painter, const QRectF &rect, const BitButtonState &state,
                    const QString &language, bool pressed) //20261109-4 change
{
    StaticTextProperties text;
    QImage image;
    QRectF imageRect;
    QSizeF imageSize;
    QString content;

    painter->save(); painter->setClipRect(rect, Qt::IntersectClip);
    painter->setOpacity(state.opacity / 255.0);
    //20261109-4 change: Artwork owns its alpha; a control-wide fill would turn every transparent corner into a matte.
    // Preserve the configured solid fill only for buttons whose artwork is explicitly disabled.
    if (!state.useImage && !state.transparent) { painter->fillRect(rect, state.background); }
    if (state.useImage)
    {
        if (!state.picturePng.isEmpty())
        {
            image = QImage::fromData(state.picturePng, "PNG"); imageRect = rect;
            if (state.imageMode != 0)
            {
                imageSize = image.size();
                if (state.imageMode == 1) { imageSize.scale(rect.size(), Qt::KeepAspectRatio); }
                imageRect = QRectF(rect.center() - QPointF(imageSize.width() / 2, imageSize.height() / 2), imageSize);
            }
            painter->drawImage(imageRect, image);
        }
        else if (!paintKvgPicture(painter, rect, state.picture, state.imageMode)) //20261109-0 change
        {
            painter->fillRect(rect, QBrush(QColor(220, 220, 220), Qt::BDiagPattern));
            painter->setPen(Qt::darkRed);
            painter->drawText(rect, Qt::AlignTop | Qt::TextWordWrap, QStringLiteral("图库资源不可用"));
        }
    }
    painter->setOpacity(1);
    if (state.effect3d)
    {
        painter->setPen(QPen(pressed ? Qt::darkGray : Qt::white, 2));
        painter->drawLine(rect.topLeft(), rect.topRight()); painter->drawLine(rect.topLeft(), rect.bottomLeft());
        painter->setPen(QPen(pressed ? Qt::white : Qt::darkGray, 2));
        painter->drawLine(rect.bottomLeft(), rect.bottomRight()); painter->drawLine(rect.topRight(), rect.bottomRight());
    }
    text = state.text; text.style = 0;
    content = bitButtonContent(state, language);
    if (text.adaptive || content.contains(QChar(u'\n')))
    {
        paintStaticText(painter, rect, content, text); //20261109-0 change: Preserve unverified adaptive and multiline behavior.
    }
    else
    {
        paintBitButtonCaption(painter, rect, content, text); //20261109-0 add: Button captions do not inherit the static-text minus-one offset.
    }
    painter->restore();
}
