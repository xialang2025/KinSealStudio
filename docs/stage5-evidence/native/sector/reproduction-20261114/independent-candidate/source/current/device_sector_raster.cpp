#include "device_sector_raster.h"
#include "device_ellipse_raster.h"
#include <QTransform>

/******************************************************************************
 * Function Name   : paintDeviceSectorGraphic
 * Description     : Construct eight local open arc elements and map only the proved integer translation.
 * Input Parameters: painter, geometry, properties, error
 * Return Value    : True after the shared raster paints both masks; false leaves the target untouched.
 * Date            : 2026.11.14
 ******************************************************************************/
bool paintDeviceSectorGraphic(QPainter *painter, const QRect &geometry,
                             const GraphicProperties &properties, QString *error) //20261114-3 add
{
    QRectF localRect;
    QPainterPath localPath;
    QPainterPath scenePath;
    QTransform translation;
    QString curveError;

    if (geometry.width()!=geometry.height() || geometry.width()<100 || geometry.width()>240
            || geometry.x()<2 || geometry.y()<2
            || qint64(geometry.x())+geometry.width()>698 || qint64(geometry.y())+geometry.height()>378)
    {
        if (error!=nullptr) { *error=QStringLiteral("扇形设备栅格仅允许已核固定角度正方形范围，未绘制输出。"); }
        return false;
    }
    localRect=QRectF(0,0,geometry.width(),geometry.height());
    localPath.moveTo(localRect.center());localPath.arcTo(localRect,45,90);
    translation.translate(geometry.x(),geometry.y());scenePath=translation.map(localPath);
    // Fill closes implicitly; adding closeSubpath would invent a ninth edge in the stroke layer.
    if (localPath.elementCount()!=8 || scenePath.elementCount()!=8
            || QPointF(scenePath.elementAt(0))==QPointF(scenePath.elementAt(7)))
    {
        if (error!=nullptr) { *error=QStringLiteral("扇形设备路径未保持8元素开放语义，未绘制输出。"); }
        return false;
    }
    if (!paintDeviceWhiteCurvePath(painter,geometry,properties,scenePath,&curveError))
    {
        if (error!=nullptr)
        {
            curveError.replace(QStringLiteral("曲线设备栅格"),QStringLiteral("扇形设备栅格"));
            *error=curveError;
        }
        return false;
    }
    if (error!=nullptr) { error->clear(); }
    return true;
}
