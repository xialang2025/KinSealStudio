#ifndef CONTROL_RENDERING_H
#define CONTROL_RENDERING_H

#include "static_text_properties.h"
#include <QFont>
#include <QImage>
#include <QJsonObject>

//20261110-8 add: Cache immutable source pixels at their original channel precision; composition remains a live input.
QImage decodedControlPng(const QByteArray &png, QString *error = nullptr);
QFont controlPixelFont(const StaticTextProperties &properties, int logicalDpiY = 96);
QJsonObject controlFontDiagnostics(const StaticTextProperties &properties);

#endif
