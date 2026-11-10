#ifndef TEXT_PAYLOAD_COMPILER_H
#define TEXT_PAYLOAD_COMPILER_H

#include "project_model.h"

#include <QByteArray>
#include <QString>

/******************************************************************************
 * Function Name   : renderSingleTextFrame
 * Description     : Rasterize one text object into the target screen's RGB565 frame.
 * Input Parameters: project, fontFamily, fontSize, pixelSize, frame, error
 * Return Value    : True when the full 800 by 480 frame was generated.
 * Date            : 2026.11.08
 ******************************************************************************/
bool renderSingleTextFrame(const HmiProject &project, const QString &fontFamily,
                           int fontSize, bool pixelSize,
                           QByteArray *frame, QString *error); //20261108-11 add

/******************************************************************************
 * Function Name   : compressRgb565Frame
 * Description     : Encode one complete screen frame as an LZO1X stream.
 * Input Parameters: frame, compressed, error
 * Return Value    : True when compression and independent round-trip verification succeed.
 * Date            : 2026.11.08
 ******************************************************************************/
bool compressRgb565Frame(const QByteArray &frame, QByteArray *compressed,
                         QString *error); //20261108-11 add

/******************************************************************************
 * Function Name   : compressRgb565Pixels
 * Description     : Compress one packed RGB565 screen or control image and verify its round trip.
 * Input Parameters: pixels, compressed, error
 * Return Value    : True when the bounded image decodes byte-for-byte.
 * Date            : 2026.11.09
 ******************************************************************************/
bool compressRgb565Pixels(const QByteArray &pixels, QByteArray *compressed,
                          QString *error); //20261109-0 add

#endif
