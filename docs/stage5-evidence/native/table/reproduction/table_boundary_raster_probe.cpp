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

    if (argc!=8) { return 2; }
    geometry=QRectF(QString::fromLocal8Bit(argv[2]).toInt(),QString::fromLocal8Bit(argv[3]).toInt(),QString::fromLocal8Bit(argv[4]).toInt(),QString::fromLocal8Bit(argv[5]).toInt());
    properties.rows=QString::fromLocal8Bit(argv[6]).toInt(); properties.columns=QString::fromLocal8Bit(argv[7]).toInt(); properties.background=QColor(QStringLiteral("#e0e0e0"));
    image.fill(Qt::black); painter.begin(&image); painter.fillRect(QRect(1,1,799,479),QColor(197,223,255)); paintDeviceTableGraphic(&painter,geometry,properties); painter.end();
    output.setFileName(QString::fromLocal8Bit(argv[1]));
    if (!output.open(QIODevice::WriteOnly) || output.write(reinterpret_cast<const char *>(image.constBits()),image.sizeInBytes())!=image.sizeInBytes()) { return 3; }
    return 0;
}
