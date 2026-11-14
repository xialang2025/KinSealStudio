#include "device_ellipse_raster.h"
#ifndef OLD_ELLIPSE
#include "device_sector_raster.h"
#endif
#include <QCoreApplication>
#include <QImage>
#include <QPainter>
#include <QFile>
#include <cstdio>
int main(int argc,char **argv)
{
    QCoreApplication app(argc,argv);
    GraphicProperties properties;
    QImage image(800,480,QImage::Format_RGB32);
    QImage converted;
    QPainter painter;
    QFile output;
    QString error;
    QRect geometry;
    int i;
    bool painted;
    if(argc!=9){return 2;}
    geometry=QRect(QString::fromLocal8Bit(argv[2]).toInt(),QString::fromLocal8Bit(argv[3]).toInt(),QString::fromLocal8Bit(argv[4]).toInt(),QString::fromLocal8Bit(argv[5]).toInt());
    properties.lineStyle=QString::fromLocal8Bit(argv[6]).toInt();properties.fillStyle=QString::fromLocal8Bit(argv[7]).toInt();
    image.fill(qRgb(197,222,255));
    for(i=0;i<800;i++){image.setPixel(i,0,qRgb(0,0,0));}
    for(i=0;i<480;i++){image.setPixel(0,i,qRgb(0,0,0));}
    if(!painter.begin(&image)){return 3;}
    if(QString::fromLocal8Bit(argv[1])==QStringLiteral("ellipse")){painted=paintDeviceEllipseGraphic(&painter,geometry,properties,&error);}
#ifndef OLD_ELLIPSE
    else if(QString::fromLocal8Bit(argv[1])==QStringLiteral("sector")){painted=paintDeviceSectorGraphic(&painter,geometry,properties,&error);}
#endif
    else{painted=false;}
    painter.end();
    if(!painted){std::fprintf(stderr,"%s\n",error.toUtf8().constData());return 4;}
    converted=image.convertToFormat(QImage::Format_RGB16);output.setFileName(QString::fromLocal8Bit(argv[8]));
    if(!output.open(QIODevice::WriteOnly|QIODevice::NewOnly)||output.write(reinterpret_cast<const char *>(converted.constBits()),converted.sizeInBytes())!=768000){return 5;}
    return 0;
}
