#ifndef NATIVE_FUNCTION_BUTTON_CONTROL_H
#define NATIVE_FUNCTION_BUTTON_CONTROL_H

#include "native_bit_button_control.h"

/******************************************************************************
 * Function Name   : buildNativeFunctionButtonControl
 * Description     : Encode the verified V5.1 type10 subtype3 system-function record.
 * Input Parameters: object, options, control, error
 * Return Value    : True with one 640-byte record; failure preserves caller output.
 * Date            : 2026.11.10
 ******************************************************************************/
bool buildNativeFunctionButtonControl(const HmiObject &object,
                                     const NativeBitButtonControlOptions &options,
                                     QByteArray *control, QString *error); //20261110-0 add

#endif
