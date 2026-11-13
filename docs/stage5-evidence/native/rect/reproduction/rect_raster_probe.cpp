#include "graphic_properties.h"
#include <QCoreApplication>
#include <QFile>
#include <QImage>
#include <QPainter>

int main(int argc, char **argv)
{
    QCoreApplication app(argc,argv);
    QImage image(800,480,QImage::Format_RGB16);
    QPainter painter;
    QFile output;
    GraphicProperties properties;
    QRectF geometry;

    if (argc!=8 && argc!=12) { return 2; }
    geometry=QRectF(QString::fromLocal8Bit(argv[2]).toInt(),QString::fromLocal8Bit(argv[3]).toInt(),QString::fromLocal8Bit(argv[4]).toInt(),QString::fromLocal8Bit(argv[5]).toInt());
    properties.lineStyle=QString::fromLocal8Bit(argv[6]).toInt();properties.fillStyle=QString::fromLocal8Bit(argv[7]).toInt();
    image.fill(Qt::black); painter.begin(&image); painter.fillRect(QRect(1,1,799,479),QColor(197,223,255)); paintGraphic(&painter,QStringLiteral("rect"),geometry,properties);
    if(argc==12){geometry=QRectF(QString::fromLocal8Bit(argv[8]).toInt(),QString::fromLocal8Bit(argv[9]).toInt(),QString::fromLocal8Bit(argv[10]).toInt(),QString::fromLocal8Bit(argv[11]).toInt());paintGraphic(&painter,QStringLiteral("rect"),geometry,properties);}
    painter.end();
    output.setFileName(QString::fromLocal8Bit(argv[1]));
    if (!output.open(QIODevice::WriteOnly) || output.write(reinterpret_cast<const char *>(image.constBits()),image.sizeInBytes())!=image.sizeInBytes()) { return 3; }
    return 0;
}
