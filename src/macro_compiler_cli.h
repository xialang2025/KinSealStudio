#ifndef MACRO_COMPILER_CLI_H
#define MACRO_COMPILER_CLI_H
#include <QStringList>

/******************************************************************************
 * Function Name   : handleMacroCompilerCli
 * Description     : Compile ordinary source macros into a source-bound atomic PC report, without USB or execution.
 * Input Parameters: arguments, exitCode
 * Return Value    : True when the dedicated CLI verb was handled.
 * Date            : 2026.11.14
 ******************************************************************************/
bool handleMacroCompilerCli(const QStringList &arguments, int *exitCode); //20261114-7 add
#endif
