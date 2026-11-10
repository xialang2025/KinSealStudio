#ifndef NATIVE_WORD_BUTTON_CONTROL_H
#define NATIVE_WORD_BUTTON_CONTROL_H

#include "native_bit_button_control.h"

/******************************************************************************
 * Function Name   : buildNativeWordButtonControl
 * Description     : Encode the verified type10 subtype1 word-operation record.
 * Input Parameters: object, options, control, error
 * Return Value    : True with one complete 640-byte record; failure preserves output.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildNativeWordButtonControl(const HmiObject &object,
                                  const NativeBitButtonControlOptions &options,
                                  QByteArray *control, QString *error); //20261109-6 add

#endif
