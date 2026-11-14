#ifndef DEVICE_SECTOR_RASTER_H
#define DEVICE_SECTOR_RASTER_H

#include "graphic_properties.h"
class QPainter;

/******************************************************************************
 * Function Name   : paintDeviceSectorGraphic
 * Description     : Render the fixed-angle square-sector candidate after independent masks succeed.
 * Input Parameters: painter, geometry, properties, error
 * Return Value    : True after painting; false without changing the target.
 * Date            : 2026.11.14
 ******************************************************************************/
bool paintDeviceSectorGraphic(QPainter *painter, const QRect &geometry,
                             const GraphicProperties &properties, QString *error); //20261114-3 add

#endif
