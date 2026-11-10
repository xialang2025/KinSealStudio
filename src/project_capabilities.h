#ifndef PROJECT_CAPABILITIES_H
#define PROJECT_CAPABILITIES_H

#include <QJsonObject>

struct HmiProject;

/******************************************************************************
 * Function Name   : describeProjectCapabilities
 * Description     : Separate editable model support from native and device backend eligibility.
 * Input Parameters: project
 * Return Value    : Explicit capability gates; candidate status still requires actual export or compilation.
 * Date            : 2026.11.10
 ******************************************************************************/
QJsonObject describeProjectCapabilities(const HmiProject &project); //20261110-5 add

#endif
