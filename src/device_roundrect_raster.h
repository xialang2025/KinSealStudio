#ifndef DEVICE_ROUNDRECT_RASTER_H
#define DEVICE_ROUNDRECT_RASTER_H

#include "graphic_properties.h"
class QPainter;

//20261113-16 add: Only the bounded single-page compile plan admits this raster.
void paintDeviceRoundrectGraphic(QPainter *painter, const QRect &geometry, const GraphicProperties &properties);

#endif
