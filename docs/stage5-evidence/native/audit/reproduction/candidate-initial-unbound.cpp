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
    QString type;

    if (argc!=8) { return 2; }
    type=QString::fromLocal8Bit(argv[2]);geometry=QRectF(QString::fromLocal8Bit(argv[3]).toInt(),QString::fromLocal8Bit(argv[4]).toInt(),QString::fromLocal8Bit(argv[5]).toInt(),QString::fromLocal8Bit(argv[6]).toInt());
    if(type==QStringLiteral("polygon")){properties.background=Qt::black;properties.points={QPointF(0,0),QPointF(1,0),QPointF(1,1)};}
    image.fill(Qt::black);painter.begin(&image);painter.fillRect(QRect(1,1,799,479),QColor(197,223,255));
    if(QString::fromLocal8Bit(argv[7])==QStringLiteral("flat") && graphicStrokeOnly(type)){
        painter.setRenderHint(QPainter::Antialiasing,false);painter.setBrush(Qt::NoBrush);painter.setPen(QPen(Qt::black,1,Qt::SolidLine,Qt::FlatCap));painter.drawPath(graphicPath(type,geometry,properties));
    }else{paintGraphic(&painter,type,geometry,properties);}
    painter.end();output.setFileName(QString::fromLocal8Bit(argv[1]));
    if(!output.open(QIODevice::WriteOnly)||output.write(reinterpret_cast<const char*>(image.constBits()),image.sizeInBytes())!=image.sizeInBytes()){return 3;}
    return 0;
}
