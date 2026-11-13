#include <QCoreApplication>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPainterPath>

int main(int argc, char **argv)
{
    QCoreApplication app(argc,argv);
    QImage image(800,480,QImage::Format_RGB16);
    QPainter painter;
    QPainterPath path;
    QFile output;
    QPen pen;
    QString mode;
    int sx,sy,ex,ey,dx,dy,error,step,x,y,temporary;

    if(argc!=6 && argc!=7){return 2;}
    mode=argc==7?QString::fromLocal8Bit(argv[6]):QStringLiteral("path-width1");
    image.fill(Qt::black);painter.begin(&image);painter.fillRect(QRect(1,1,799,479),QColor(197,223,255));
    painter.setRenderHint(QPainter::Antialiasing,false);painter.setBrush(Qt::NoBrush);pen=QPen(Qt::black,mode==QStringLiteral("width0")?0:1,Qt::SolidLine,Qt::FlatCap);if(mode==QStringLiteral("cosmetic")){pen.setCosmetic(true);}painter.setPen(pen);
    if(mode==QStringLiteral("half")){painter.translate(.5,.5);}
    path.moveTo(QString::fromLocal8Bit(argv[2]).toDouble(),QString::fromLocal8Bit(argv[3]).toDouble());path.lineTo(QString::fromLocal8Bit(argv[4]).toDouble(),QString::fromLocal8Bit(argv[5]).toDouble());
    if(mode.startsWith(QStringLiteral("integer"))){painter.drawLine(QPoint(qRound(QString::fromLocal8Bit(argv[2]).toDouble()),qRound(QString::fromLocal8Bit(argv[3]).toDouble())),QPoint(qRound(QString::fromLocal8Bit(argv[4]).toDouble()),qRound(QString::fromLocal8Bit(argv[5]).toDouble())));}else if(mode!=QStringLiteral("bresenham")){painter.drawPath(path);}painter.end();
    if(mode==QStringLiteral("bresenham")){
        sx=qRound(QString::fromLocal8Bit(argv[2]).toDouble());sy=qRound(QString::fromLocal8Bit(argv[3]).toDouble());ex=qRound(QString::fromLocal8Bit(argv[4]).toDouble());ey=qRound(QString::fromLocal8Bit(argv[5]).toDouble());dx=qAbs(ex-sx);dy=qAbs(ey-sy);
        if(dx>=dy){if(sx>ex){temporary=sx;sx=ex;ex=temporary;temporary=sy;sy=ey;ey=temporary;}step=ey>=sy?1:-1;y=sy;error=dx;for(x=sx;x<ex && !(dx==1 && dy==1);++x){image.setPixel(x,y,qRgb(0,0,0));error-=2*dy;if(error<0){y+=step;error+=2*dx;}}}
        else{if(sy>ey){temporary=sx;sx=ex;ex=temporary;temporary=sy;sy=ey;ey=temporary;}step=ex>=sx?1:-1;x=sx;error=dy;for(y=sy;y<ey;++y){image.setPixel(x,y,qRgb(0,0,0));error-=2*dx;if(error<0){x+=step;error+=2*dy;}}}
    }
    output.setFileName(QString::fromLocal8Bit(argv[1]));if(!output.open(QIODevice::WriteOnly)||output.write(reinterpret_cast<const char*>(image.constBits()),image.sizeInBytes())!=image.sizeInBytes()){return 3;}
    return 0;
}
