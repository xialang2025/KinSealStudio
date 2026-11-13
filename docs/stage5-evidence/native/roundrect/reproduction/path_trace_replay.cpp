#include <QCoreApplication>
#include <QDataStream>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPainterPath>

/******************************************************************************
 * Function Name   : main
 * Description     : Replay a serialized observed path with controlled raster state.
 * Input Parameters: output, path stream, mode
 * Return Value    : Process status.
 * Date            : 2026.11.13
 ******************************************************************************/
int main(int argc, char **argv) //20261113-16 add
{
    QCoreApplication app(argc, argv);
    QImage image(800,480,QImage::Format_RGB32);
    QPainterPath path;
    QPainter painter;
    QFile input;
    QFile output;
    QDataStream stream;
    QString mode;
    bool sector;

    if(argc!=4 && argc!=5){return 2;}
    mode=QString::fromLocal8Bit(argv[3]);sector=mode.contains(QStringLiteral("sector"));
    input.setFileName(QString::fromLocal8Bit(argv[2]));if(!input.open(QIODevice::ReadOnly)){return 3;}
    stream.setDevice(&input);stream.setVersion(QDataStream::Qt_4_8);stream>>path;
    if(stream.status()!=QDataStream::Ok){return 4;}
    if(mode.contains(QStringLiteral("rgb16"))){image=QImage(800,480,QImage::Format_RGB16);}
    image.fill(Qt::black);painter.begin(&image);painter.fillRect(QRect(1,1,799,479),QColor(197,223,255));
    painter.setRenderHint(QPainter::Antialiasing,sector || mode.contains(QStringLiteral("aa")));
    painter.setPen(QPen(Qt::black,0,sector?Qt::NoPen:Qt::SolidLine));painter.setBrush(Qt::white);
    if(mode.contains(QStringLiteral("absolute"))){path.translate(100,100);}else{painter.translate(100,100);}
    if(mode.contains(QStringLiteral("generated"))){path=QPainterPath();path.addRoundedRect(QRectF(0,0,200,100),10,10);}
    if(argc==5){
        QPainterPath stroke;QFile strokeInput(QString::fromLocal8Bit(argv[4]));if(!strokeInput.open(QIODevice::ReadOnly)){return 6;}
        QDataStream strokeStream(&strokeInput);strokeStream.setVersion(QDataStream::Qt_4_8);strokeStream>>stroke;
        painter.fillPath(path,Qt::white);painter.fillPath(stroke,Qt::black);
    }else{painter.drawPath(path);}
    painter.end();
    image=image.convertToFormat(QImage::Format_RGB16);output.setFileName(QString::fromLocal8Bit(argv[1]));
    if(!output.open(QIODevice::WriteOnly)||output.write(reinterpret_cast<const char*>(image.constBits()),image.sizeInBytes())!=image.sizeInBytes()){return 5;}
    return 0;
}
