#ifndef STATIC_TEXT_RENDERER_H
#define STATIC_TEXT_RENDERER_H
#include "project_model.h"
#include <QRectF>
class QPainter;
QString staticTextContent(const HmiObject &object, const QString &language);
void paintStaticText(QPainter *painter, const QRectF &rect, const QString &text,
                      const StaticTextProperties &properties);
#endif
