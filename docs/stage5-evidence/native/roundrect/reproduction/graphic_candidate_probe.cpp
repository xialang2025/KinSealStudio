#include "graphic_properties.h"
#include <QCoreApplication>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPainterPathStroker>

int main(int argc, char **argv)
{
    QCoreApplication app(argc,argv);
    QImage image(800,480,QImage::Format_RGB16);
    QPainter painter;
    QFile output;
    GraphicProperties properties;
    QRectF geometry;
    QString type;
    QString mode;

    if (argc!=8) { return 2; }
    type=QString::fromLocal8Bit(argv[2]);mode=QString::fromLocal8Bit(argv[7]);geometry=QRectF(QString::fromLocal8Bit(argv[3]).toInt(),QString::fromLocal8Bit(argv[4]).toInt(),QString::fromLocal8Bit(argv[5]).toInt(),QString::fromLocal8Bit(argv[6]).toInt());
    if(type==QStringLiteral("polygon")){properties.background=Qt::black;properties.foreground=Qt::white;properties.points={QPointF(0,0),QPointF(1,0),QPointF(.75,1)};}
    if(type==QStringLiteral("sector")){properties.lineStyle=0;}
    if(mode.contains(QStringLiteral("rgb32"))){image=QImage(800,480,QImage::Format_RGB32);}
    image.fill(Qt::black);painter.begin(&image);painter.fillRect(QRect(1,1,799,479),QColor(197,223,255));
    if(mode.contains(QStringLiteral("stroker"))){ //20261113-16 add: Isolate the pre-Qt4.8 curved-pen outline path.
        QPainterPathStroker stroker;stroker.setWidth(1);stroker.setCapStyle(Qt::SquareCap);stroker.setJoinStyle(Qt::BevelJoin);
        painter.setRenderHint(QPainter::Antialiasing,mode.contains(QStringLiteral("-aa")) || type==QStringLiteral("ellipse") || type==QStringLiteral("sector"));
        const QPainterPath path=graphicPath(type,geometry,properties);painter.fillPath(path,properties.background);painter.fillPath(stroker.createStroke(path),Qt::black);
    }else if(mode.contains(QStringLiteral("cosmetic"))){ //20261113-16 add: Actual saved native paths use a zero-width cosmetic pen.
        painter.setRenderHint(QPainter::Antialiasing,mode.contains(QStringLiteral("-aa")) || type==QStringLiteral("ellipse") || type==QStringLiteral("sector"));
        painter.setPen(QPen(Qt::black,0,properties.lineStyle==0?Qt::NoPen:Qt::SolidLine));painter.setBrush(properties.background);
        if(type==QStringLiteral("ellipse")){painter.drawEllipse(geometry);}else{painter.drawPath(graphicPath(type,geometry,properties));}
    }else if(mode.contains(QStringLiteral("direct"))){
        painter.setRenderHint(QPainter::Antialiasing,mode.contains(QStringLiteral("-aa")) || type==QStringLiteral("ellipse") || type==QStringLiteral("sector"));
        painter.setPen(QPen(Qt::black,1,properties.lineStyle==0?Qt::NoPen:Qt::SolidLine,mode.contains(QStringLiteral("flat"))?Qt::FlatCap:Qt::SquareCap));painter.setBrush(graphicStrokeOnly(type)?QBrush(Qt::NoBrush):QBrush(properties.background));
        if(type==QStringLiteral("line") || type==QStringLiteral("freeline")){painter.drawLine(geometry.topLeft(),geometry.bottomRight());}
        else if(type==QStringLiteral("ellipse")){painter.drawEllipse(geometry);}
        else if(type==QStringLiteral("polygon")){painter.drawPolygon(QPolygonF(graphicHandles(type,geometry,properties)));}
        else if(type==QStringLiteral("sector")){painter.drawPie(geometry,45*16,90*16);}
        else if(type==QStringLiteral("roundrect")){painter.drawRoundedRect(geometry,10,10,Qt::RelativeSize);}
    }else if(mode==QStringLiteral("flat") && graphicStrokeOnly(type)){
        painter.setRenderHint(QPainter::Antialiasing,false);painter.setBrush(Qt::NoBrush);painter.setPen(QPen(Qt::black,1,Qt::SolidLine,Qt::FlatCap));painter.drawPath(graphicPath(type,geometry,properties));
    }else if(mode.contains(QStringLiteral("-aa"))){
        painter.setRenderHint(QPainter::Antialiasing,true);painter.setPen(QPen(Qt::black,1,properties.lineStyle==0?Qt::NoPen:Qt::SolidLine));painter.setBrush(properties.background);painter.drawPath(graphicPath(type,geometry,properties));
    }else{paintGraphic(&painter,type,geometry,properties);}
    painter.end();if(image.format()!=QImage::Format_RGB16){image=image.convertToFormat(QImage::Format_RGB16);}output.setFileName(QString::fromLocal8Bit(argv[1]));
    if(!output.open(QIODevice::WriteOnly)||output.write(reinterpret_cast<const char*>(image.constBits()),image.sizeInBytes())!=image.sizeInBytes()){return 3;}
    return 0;
}
