#include <QCoreApplication>
#include <QDataStream>
#include <QFile>
#include <QPainterPath>
#include <QPainterPathStroker>

/******************************************************************************
 * Function Name   : main
 * Description     : Generate independent rounded-box fill and stroke geometry.
 * Input Parameters: fill path, stroke path, width, height, radiusX, radiusY
 * Return Value    : Process status.
 * Date            : 2026.11.13
 ******************************************************************************/
int main(int argc, char **argv) //20261113-16 add
{
    QCoreApplication app(argc,argv);
    QPainterPath path;
    QPainterPath stroke;
    QPainterPathStroker stroker;
    QFile fillFile;
    QFile strokeFile;
    if(argc!=7){return 2;}
    path.addRoundedRect(QRectF(0,0,QString::fromLocal8Bit(argv[3]).toInt(),QString::fromLocal8Bit(argv[4]).toInt()),QString::fromLocal8Bit(argv[5]).toInt(),QString::fromLocal8Bit(argv[6]).toInt());
    stroker.setWidth(1);stroker.setCapStyle(Qt::SquareCap);stroker.setJoinStyle(Qt::BevelJoin);stroke=stroker.createStroke(path);
    fillFile.setFileName(QString::fromLocal8Bit(argv[1]));strokeFile.setFileName(QString::fromLocal8Bit(argv[2]));
    if(!fillFile.open(QIODevice::WriteOnly)||!strokeFile.open(QIODevice::WriteOnly)){return 3;}
    QDataStream fillStream(&fillFile),strokeStream(&strokeFile);fillStream.setVersion(QDataStream::Qt_4_8);strokeStream.setVersion(QDataStream::Qt_4_8);
    fillStream<<path;strokeStream<<stroke;
    return fillStream.status()==QDataStream::Ok&&strokeStream.status()==QDataStream::Ok?0:4;
}
