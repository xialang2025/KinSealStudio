#ifndef NATIVE_LAMP_CONTROL_H
#define NATIVE_LAMP_CONTROL_H

#include "native_bit_button_control.h"

/******************************************************************************
 * Function Name   : buildNativeLampControl
 * Description     : Encode the verified read-only subtype4 monitoring record.
 * Input Parameters: object, options, control, error
 * Return Value    : True when monitoring, shared appearance and pixel bounds are representable.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildNativeLampControl(const HmiObject &object,
                            const NativeBitButtonControlOptions &options,
                            QByteArray *control, QString *error); //20261109-8 add

#endif
