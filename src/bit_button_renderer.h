#ifndef BIT_BUTTON_RENDERER_H
#define BIT_BUTTON_RENDERER_H
#include "bit_button_properties.h"
#include <QRectF>
#include <QSize> //20261109-4 add
class QPainter;
void paintBitButton(QPainter *painter, const QRectF &rect, const BitButtonState &state,
                    const QString &language, bool pressed = false, bool nativeLampCaption = false); //20261128-20 change
QStringList bitButtonBuiltinPictures();
QString bitButtonContent(const BitButtonState &state, const QString &language);
bool bitButtonNeedsUnderlay(const BitButtonState &state, const QSize &size); //20261109-4 add
#endif
