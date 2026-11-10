#ifndef NATIVE_BIT_BUTTON_CONTROL_H
#define NATIVE_BIT_BUTTON_CONTROL_H

#include "project_model.h"

#include <QByteArray>
#include <QString>

#include <functional>

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

//20261109-0 add: These offsets describe decoded RGB565 data within one screen.
struct NativeBitButtonControlOptions
{
    u32 uncompressedPixelOffset = 0;
    u32 uncompressedPixelBytes = 0;
    u32 languageCount = 1; //20261109-0 add: Native per-control raw bytes include every language's state images.
    u8 addressByte2 = 0;
    u8 monitorAddressByte2 = 0;
    int macroIndex = -1;
    //20261109-0 add: The caller supplies only address records whose external route and node were verified.
    std::function<bool(const QString &, bool, bool, QByteArray *, QString *)> externalAddressEncoder;
};

/******************************************************************************
 * Function Name   : buildNativeBitButtonControl
 * Description     : Encode verified V5.1 bit-button fields into a clean 640-byte record.
 * Input Parameters: object, options, control, error
 * Return Value    : True when every active control condition is representable.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildNativeBitButtonControl(const HmiObject &object,
                                 const NativeBitButtonControlOptions &options,
                                 QByteArray *control, QString *error); //20261109-0 add

#endif
