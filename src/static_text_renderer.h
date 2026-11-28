#ifndef STATIC_TEXT_RENDERER_H
#define STATIC_TEXT_RENDERER_H
#include "project_model.h"
#include <QRectF>
class QPainter;
#ifdef Q_OS_WIN
class QFont;
bool paintNativeKernedText(QPainter *painter, const QRectF &rect, const QString &text,
                           const StaticTextProperties &properties, const QFont &font); //20261128-20 add
#endif
QString staticTextContent(const HmiObject &object, const QString &language);
void paintStaticText(QPainter *painter, const QRectF &rect, const QString &text,
                      const StaticTextProperties &properties);
#endif
