#include "macro.h"

/******************************************************************************
 * Function Name   : Macro_Entry
 * Description     : Set the internal LB1 value when invoked by the target macro runtime.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void Macro_Entry(void) /* //20261114-7 add */
{
    LocalBit[1] = 1; /* Access the internal bit bank without an address-variable table. */
}
