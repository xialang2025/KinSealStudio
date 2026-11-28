#ifndef DEVICE_THIN_CURVE_RASTER_H
#define DEVICE_THIN_CURVE_RASTER_H

#include <QRect>

class QImage;
class QPainter;
struct GraphicProperties;

//20261128-11 add: Return false before drawing if the path needs the general renderer.
bool paintDeviceThinCurve(QPainter *painter, const QImage &page, const QRect &geometry,
                          const GraphicProperties &properties, bool connected = false); //20261128-39 change
#endif
