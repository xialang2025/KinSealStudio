#include <QCoreApplication>
#include <QFile>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QDataStream>
#include <QVector>
#include <cstring>
extern "C" {
#define QT_FT_BEGIN_HEADER
#define QT_FT_END_HEADER
#include "public-qt-sources/qgrayraster_p.h"
}

/******************************************************************************
 * Function Name   : retainCoverage
 * Description     : Retain spans in a bounded diagnostic alpha mask without applying a color compositor.
 * Input Parameters: count, spans, user
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
static void retainCoverage(int count, const QT_FT_Span *spans, void *user) //20261114-1 add
{
    QByteArray *mask = static_cast<QByteArray *>(user);
    int i;
    int left;
    int right;

    for (i=0;i<count;++i)
    {
        if (spans[i].y<0 || spans[i].y>=480) { continue; }
        left=qMax(0,int(spans[i].x));right=qMin(800,int(spans[i].x)+int(spans[i].len));
        if (right>left) { std::memset(mask->data()+spans[i].y*800+left,spans[i].coverage,right-left); }
    }
}

/******************************************************************************
 * Function Name   : main
 * Description     : Separate historical scan coverage from the independently generated path and final RGB565 mixing.
 * Input Parameters: output, family, fill-or-stroke, optional serialized path
 * Return Value    : Actual diagnostic status.
 * Date            : 2026.11.14
 ******************************************************************************/
int main(int argc, char **argv) //20261114-1 add
{
    QCoreApplication app(argc,argv);
    QPainterPath path;
    QPainterPathStroker stroker;
    QString family;
    QString layer;
    QFile output;
    QFile input;
    QDataStream stream;
    QVector<QT_FT_Vector> points;
    QVector<char> tags;
    QVector<int> contours;
    QT_FT_Outline outline={};
    QT_FT_Raster raster=nullptr;
    QT_FT_Raster_Params parameters={};
    QByteArray pool(1024*1024,0);
    QByteArray mask(800*480,0);
    QPainterPath::Element element;
    int i;
    int result;
    char tag;

    if (argc!=4 && argc!=5) { return 2; }
    family=QString::fromLocal8Bit(argv[2]);layer=QString::fromLocal8Bit(argv[3]);
    if (argc==5)
    {
        input.setFileName(QString::fromLocal8Bit(argv[4]));if (!input.open(QIODevice::ReadOnly)) { return 3; }
        stream.setDevice(&input);stream.setVersion(QDataStream::Qt_4_8);stream>>path;
        if (stream.status()!=QDataStream::Ok) { return 4; }
    }
    else if (family==QStringLiteral("polygon")) { path.moveTo(100,100);path.lineTo(300,100);path.lineTo(250,220);path.closeSubpath(); }
    else if (family==QStringLiteral("sector")) { path.moveTo(200,200);path.arcTo(QRectF(100,100,200,200),45,90); }
    else if (family==QStringLiteral("ellipse")) { path.addEllipse(QRectF(100,100,200,100)); }
    else { return 5; }
    if (layer==QStringLiteral("stroke"))
    {
        stroker.setWidth(1);stroker.setCapStyle(Qt::SquareCap);stroker.setJoinStyle(Qt::BevelJoin);path=stroker.createStroke(path);
    }
    for (i=0;i<path.elementCount();++i)
    {
        element=path.elementAt(i);
        if (element.type==QPainterPath::MoveToElement && i>0) { contours.append(points.size()-1); }
        points.append({int(element.x*64),int(element.y*64)});
        tag=element.type==QPainterPath::CurveToElement || (element.type==QPainterPath::CurveToDataElement && i>0 && path.elementAt(i-1).type==QPainterPath::CurveToElement)?QT_FT_CURVE_TAG_CUBIC:QT_FT_CURVE_TAG_ON;
        tags.append(tag);
    }
    contours.append(points.size()-1);outline.n_contours=contours.size();outline.n_points=points.size();
    outline.points=points.data();outline.tags=tags.data();outline.contours=contours.data();
    outline.flags=path.fillRule()==Qt::OddEvenFill?QT_FT_OUTLINE_EVEN_ODD_FILL:0;
    parameters.source=&outline;parameters.flags=QT_FT_RASTER_FLAG_AA|QT_FT_RASTER_FLAG_DIRECT|QT_FT_RASTER_FLAG_CLIP;
    parameters.gray_spans=retainCoverage;parameters.user=&mask;parameters.clip_box={0,0,800,480};
    result=qt_ft_grays_raster.raster_new(nullptr,&raster);if (result!=0) { return 6; }
    qt_ft_grays_raster.raster_reset(raster,reinterpret_cast<unsigned char *>(pool.data()),pool.size());result=qt_ft_grays_raster.raster_render(raster,&parameters);
    qt_ft_grays_raster.raster_done(raster);if (result!=0) { return 7; }
    output.setFileName(QString::fromLocal8Bit(argv[1]));if (!output.open(QIODevice::WriteOnly) || output.write(mask)!=mask.size()) { return 8; }
    output.close();output.setFileName(QString::fromLocal8Bit(argv[1])+QStringLiteral(".pathstream"));
    if (!output.open(QIODevice::WriteOnly)) { return 9; }
    stream.setDevice(&output);stream.setVersion(QDataStream::Qt_4_8);stream<<path;
    return 0;
}
