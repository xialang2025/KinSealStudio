#include "static_text_renderer.h"
#include <QFont>
#include <QLinearGradient>
#include <QPainter>
#include <QRadialGradient>
#include <cmath>

/******************************************************************************
 * Function Name   : staticTextContent
 * Description     : Resolve one language without replacing the legacy primary text.
 * Input Parameters: object, language
 * Return Value    : Translation when present, otherwise primary text.
 * Date            : 2026.11.08
 ******************************************************************************/
QString staticTextContent(const HmiObject &object, const QString &language) //20261108-14 add
{
    return object.hasStaticTextProperties && object.staticText.languageTexts.contains(language)
               ? object.staticText.languageTexts.value(language) : object.text;
}

/******************************************************************************
 * Function Name   : paintStaticText
 * Description     : Share clipping, font effects and thirteen fills between editor, preview and compiler.
 * Input Parameters: painter, rect, text, properties
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void paintStaticText(QPainter *painter, const QRectF &rect, const QString &text,
                      const StaticTextProperties &properties) //20261108-14 add
{
    QFont font(properties.fontFamily);
    QLinearGradient linear;
    QRadialGradient radial;
    QBrush brush;
    QPointF start;
    QPointF end;
    QRectF bounds;
    QRectF textRect;
    int alignment;
    bool symmetric;

    painter->save();
    painter->setClipRect(rect, Qt::IntersectClip);
    painter->setRenderHint(QPainter::TextAntialiasing, false);
    if (properties.style != 0)
    {
        start = rect.topLeft();
        end = rect.topRight();
        symmetric = properties.style == 3 || properties.style == 5 ||
                    properties.style == 7 || properties.style == 9;
        if (properties.style == 4 || properties.style == 5)
        {
            end = rect.bottomLeft();
        }
        else if (properties.style == 6 || properties.style == 7)
        {
            end = rect.bottomRight();
        }
        else if (properties.style == 8 || properties.style == 9)
        {
            start = rect.topRight();
            end = rect.bottomLeft();
        }
        linear = QLinearGradient(start, end);
        linear.setColorAt(0, properties.foregroundColor); //20261108-14 change: Native white/red preview shows foreground at the gradient start.
        linear.setColorAt(symmetric ? 0.5 : 1.0, properties.backgroundColor);
        if (symmetric)
        {
            linear.setColorAt(1, properties.foregroundColor);
        }
        brush = QBrush(linear);
        if (properties.style == 1)
        {
            brush = QBrush(properties.backgroundColor);
        }
        else if (properties.style == 10 || properties.style == 11)
        {
            // Native corner-radiation previews interpolate linearly along the opposing diagonal.
            linear = QLinearGradient(properties.style == 10 ? rect.topRight() : rect.topLeft(),
                                      properties.style == 10 ? rect.bottomLeft() : rect.bottomRight());
            linear.setColorAt(0, properties.backgroundColor); linear.setColorAt(1, properties.foregroundColor);
            brush = QBrush(linear);
        }
        else if (properties.style == 12)
        {
            radial = QRadialGradient(rect.center(), qMin(rect.width(), rect.height()) * 0.5);
            radial.setColorAt(0, properties.backgroundColor);
            radial.setColorAt(1, properties.foregroundColor);
            brush = QBrush(radial);
        }
        painter->setOpacity(properties.opacity / 255.0);
        painter->fillRect(rect, brush);
        painter->setOpacity(1);
    }
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
    if (properties.adaptive && !text.isEmpty())
    {
        bounds = painter->boundingRect(QRectF(0, 0, 1000000, 1000000), Qt::AlignLeft | Qt::AlignTop, text);
        if (bounds.width() > 0 && bounds.height() > 0)
        {
            painter->translate(rect.topLeft());
            painter->scale(rect.width() / bounds.width(), rect.height() / bounds.height());
            painter->translate(-bounds.topLeft());
            painter->drawText(bounds, Qt::AlignLeft | Qt::AlignTop, text);
        }
    }
    else
    {
        // Native bold TEST and earlier one/three-line samples share this Qt 4 rounding offset.
        textRect = rect.translated(0, text.count(QLatin1Char('\n')) % 2 == 0 ? -1 : 0);
        painter->drawText(textRect, alignment, text);
    }
    painter->restore();
}
