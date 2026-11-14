#ifndef DEVICE_ELLIPSE_RASTER_H
#define DEVICE_ELLIPSE_RASTER_H

#include "graphic_properties.h"
class QPainter;

/******************************************************************************
 * Function Name   : paintDeviceWhiteCurvePath
 * Description     : Render a bounded independently constructed white curve with the verified scene-path stroke.
 * Input Parameters: painter, geometry, properties, path, error
 * Return Value    : True after both masks succeed and the isolated patch is painted; false preserves the target.
 * Date            : 2026.11.14
 ******************************************************************************/
bool paintDeviceWhiteCurvePath(QPainter *painter, const QRect &geometry,
                              const GraphicProperties &properties, const QPainterPath &path,
                              QString *error); //20261114-3 add

/******************************************************************************
 * Function Name   : paintDeviceEllipseGraphic
 * Description     : Render guarded ellipse fill and stroke after both independent coverage masks succeed.
 * Input Parameters: painter, geometry, properties, error
 * Return Value    : True after painting; false without touching the active painter.
 * Date            : 2026.11.14
 ******************************************************************************/
bool paintDeviceEllipseGraphic(QPainter *painter, const QRect &geometry,
                              const GraphicProperties &properties, QString *error); //20261114-2 add

#endif
