#include "graphic_properties.h"
#include <QCoreApplication>
#include <QImage>
#include <QPainter>
#include <QFile>
int main(int argc,char **argv)
{
    QCoreApplication app(argc,argv);QImage image;QPainter painter;QFile file;GraphicProperties properties;GraphicProperties fill;QPainterPath grid;int mask;int index;
    if(argc!=2 && argc!=3){return 2;}
    properties.background=QColor(QStringLiteral("#e0e0e0"));
    for(mask=0;mask<8;++mask){
        properties.showHorizontal=mask&1;properties.showVertical=mask&2;properties.showOutline=mask&4;
        image=QImage(800,480,QImage::Format_RGB16);image.fill(Qt::black);painter.begin(&image);painter.fillRect(QRect(1,1,799,479),QColor(197,223,255));
        if(argc==2){paintGraphic(&painter,QStringLiteral("table"),QRectF(100,100,200,100),properties);}
        else{
            fill=properties;fill.showHorizontal=false;fill.showVertical=false;fill.showOutline=false;paintGraphic(&painter,QStringLiteral("table"),QRectF(100,100,200,100),fill);grid=QPainterPath();
            for(index=1;index<4;++index){if(mask&1){grid.moveTo(100,100+25*index);grid.lineTo(300,100+25*index);}if(mask&2){grid.moveTo(100+50*index,100);grid.lineTo(100+50*index,200);}}
            painter.setBrush(Qt::NoBrush);painter.setPen(QPen(Qt::black,1,Qt::SolidLine,Qt::FlatCap));painter.drawPath(grid);
            if(mask&4){painter.setPen(QPen(Qt::black,1,Qt::SolidLine));painter.drawRect(QRectF(100,100,200,100));}
        }
        painter.end();
        file.setFileName(QString::fromLocal8Bit(argv[1])+QString::number(mask)+QStringLiteral(".rgb565"));if(!file.open(QIODevice::WriteOnly)){return 3;}file.write(reinterpret_cast<const char*>(image.constBits()),image.sizeInBytes());file.close();
    }
    return 0;
}
