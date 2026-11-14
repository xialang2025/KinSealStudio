#ifndef DEVICE_POLYGON_RASTER_H
#define DEVICE_POLYGON_RASTER_H
#include "graphic_properties.h"
#include <QByteArray>
class QPainter;

//20261114-2 add: Curved candidates already produce 8-bit lattice points; polygons retain their 26.6 quantization.
enum class DeviceStraightCoveragePrecision
{
    Quantized26_6,
    Exact8Bit
};

/******************************************************************************
 * Function Name   : deviceStraightPathCoverage
 * Description     : Integrate a bounded straight path with explicitly selected input precision.
 * Input Parameters: path, precision, output, error
 * Return Value    : True after replacing output with the full-page alpha mask; false preserves output.
 * Date            : 2026.11.14
 ******************************************************************************/
bool deviceStraightPathCoverage(const QPainterPath &path, DeviceStraightCoveragePrecision precision,
                               QByteArray *output, QString *error); //20261114-2 add

//20261114-1 add: The compile plan admits only bounded simple integer polygons and their sampled black styles.
bool paintDevicePolygonGraphic(QPainter *painter, const QRect &geometry, const GraphicProperties &properties, QString *error);
#endif
