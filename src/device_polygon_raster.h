#ifndef DEVICE_POLYGON_RASTER_H
#define DEVICE_POLYGON_RASTER_H
#include "graphic_properties.h"
class QPainter;

//20261114-1 add: The compile plan admits only bounded simple integer polygons and their sampled black styles.
bool paintDevicePolygonGraphic(QPainter *painter, const QRect &geometry, const GraphicProperties &properties, QString *error);
#endif
