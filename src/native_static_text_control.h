#ifndef NATIVE_STATIC_TEXT_CONTROL_H
#define NATIVE_STATIC_TEXT_CONTROL_H

#include "project_model.h"

#include <QByteArray>
#include <QString>
#include <functional>

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

//20261109-0 add: Type28 owns one RGB565 image per language and no macro or operation state.
struct NativeStaticTextControlOptions
{
    u32 uncompressedPixelOffset = 0;
    u32 uncompressedPixelBytes = 0;
    u32 languageCount = 1;
    u8 bitAddressByte2 = 0;
    u8 wordAddressByte2 = 0;
    std::function<bool(const QString &, bool, bool, QByteArray *, QString *)> externalAddressEncoder;
};

/******************************************************************************
 * Function Name   : buildNativeStaticTextControl
 * Description     : Encode a clean type28 record with verified visibility and per-language image lengths.
 * Input Parameters: object, options, control, error
 * Return Value    : True after every active field fits the native 640-byte record.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildNativeStaticTextControl(const HmiObject &object,
                                  const NativeStaticTextControlOptions &options,
                                  QByteArray *control, QString *error); //20261109-0 add

#endif
