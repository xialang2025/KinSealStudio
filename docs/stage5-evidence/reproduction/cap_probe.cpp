#include <QCoreApplication>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QFile>
int main(int argc,char **argv)
{
    QCoreApplication app(argc,argv);
    QImage image;
    QPainter p;
    QPainterPath path;
    QFile f;
    int cap;
    const Qt::PenCapStyle caps[]={Qt::FlatCap,Qt::SquareCap,Qt::RoundCap};
    if(argc!=2){return 2;}
    for(cap=0;cap<3;++cap){
        image=QImage(800,480,QImage::Format_RGB16);image.fill(Qt::black);p.begin(&image);p.fillRect(QRect(1,1,799,479),QColor(197,223,255));
        p.setRenderHint(QPainter::Antialiasing,false);p.setPen(QPen(Qt::black,1,Qt::SolidLine,caps[cap]));path=QPainterPath();path.moveTo(100,100);path.lineTo(250,100);path.lineTo(250,200);p.drawPath(path);p.end();
        f.setFileName(QString::fromLocal8Bit(argv[1])+QString::number(cap)+QStringLiteral(".rgb565"));if(!f.open(QIODevice::WriteOnly)){return 3;}f.write(reinterpret_cast<const char*>(image.constBits()),image.sizeInBytes());f.close();
    }
    return 0;
}
