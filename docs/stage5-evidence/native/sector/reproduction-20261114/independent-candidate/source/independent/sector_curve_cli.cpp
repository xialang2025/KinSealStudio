#include "../ellipse_algorithm/ellipse_curve_candidate.h"
#include "../../kinseal-stage4-20261112/worktree/src/device_polygon_raster.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainterPathStroker>
#include <QTransform>
#include <cstdio>

/******************************************************************************
 * Function Name   : saveBytes
 * Description     : Preserve each diagnostic artifact without replacing an earlier sample.
 * Input Parameters: path, bytes
 * Return Value    : True when a new file contains the entire artifact.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool saveBytes(const QString &path, const QByteArray &bytes) //20261114-3 add
{
    QFile file(path);

    return file.open(QIODevice::WriteOnly|QIODevice::NewOnly) && file.write(bytes)==bytes.size();
}

/******************************************************************************
 * Function Name   : streamPath
 * Description     : Serialize independently constructed geometry in the established Qt path format.
 * Input Parameters: path
 * Return Value    : Qt_4_8 path stream bytes.
 * Date            : 2026.11.14
 ******************************************************************************/
static QByteArray streamPath(const QPainterPath &path) //20261114-3 add
{
    QByteArray bytes;
    QDataStream stream(&bytes,QIODevice::WriteOnly);

    stream.setVersion(QDataStream::Qt_4_8);stream<<path;
    return stream.status()==QDataStream::Ok?bytes:QByteArray();
}

/******************************************************************************
 * Function Name   : divide255
 * Description     : Apply the observed integer quotient before adding source and destination terms.
 * Input Parameters: numerator in 0..65025
 * Return Value    : Observed Qt4 byte product quotient.
 * Date            : 2026.11.14
 ******************************************************************************/
static qint64 divide255(qint64 numerator) //20261114-3 add
{
    return (numerator+(numerator>>8)+128)>>8;
}

/******************************************************************************
 * Function Name   : composeFrame
 * Description     : Composite independent white fill and black stroke masks before RGB565 quantization.
 * Input Parameters: fill, stroke
 * Return Value    : Complete little-endian 800x480 RGB565 page.
 * Date            : 2026.11.14
 ******************************************************************************/
static QByteArray composeFrame(const QByteArray &fill,const QByteArray &stroke) //20261114-3 add
{
    QByteArray frame(768000,0);
    qint64 channels[3];
    qint64 alpha;
    qint64 inverse;
    qint64 pixel;
    qint64 offset;
    qint64 x;
    qint64 y;
    qint64 channel;

    for (y=0;y<480;y++)
    {
        for (x=0;x<800;x++)
        {
            offset=y*800+x;channels[0]=197;channels[1]=222;channels[2]=255;
            if (x==0 || y==0) { channels[0]=0;channels[1]=0;channels[2]=0; }
            alpha=static_cast<unsigned char>(fill[int(offset)]);inverse=255-alpha;
            for (channel=0;channel<3;channel++) { channels[channel]=alpha+divide255(channels[channel]*inverse); }
            alpha=static_cast<unsigned char>(stroke[int(offset)]);inverse=255-alpha;
            for (channel=0;channel<3;channel++) { channels[channel]=divide255(channels[channel]*inverse); }
            pixel=((channels[0]>>3)<<11)|((channels[1]>>2)<<5)|(channels[2]>>3);
            frame[int(offset*2)]=char(pixel&255);frame[int(offset*2+1)]=char((pixel>>8)&255);
        }
    }
    return frame;
}

/******************************************************************************
 * Function Name   : main
 * Description     : Generate exactly eight open sector elements and independent complete coverage/frame artifacts.
 * Input Parameters: output prefix, x, y, width, height, optional lineStyle and fillStyle
 * Return Value    : Zero after writing all new artifacts; nonzero on refusal or I/O failure.
 * Date            : 2026.11.14
 ******************************************************************************/
int main(int argc,char **argv) //20261114-3 add
{
    QCoreApplication app(argc,argv);
    QPainterPath path;
    QPainterPath localPath;
    QPainterPath strokePath;
    QPainterPath flattenedFill;
    QPainterPath flattenedStroke;
    QPainterPathStroker stroker;
    QByteArray fill(384000,0);
    QByteArray stroke(384000,0);
    QByteArray frame;
    QByteArray pathBytes;
    QJsonObject result;
    QJsonArray coordinatesJson;
    QJsonArray elements;
    QJsonArray localElements;
    QJsonArray transform;
    QJsonArray element;
    QRectF rect;
    QTransform translation;
    QString prefix;
    QString error;
    qint64 geometry[4];
    qint64 index;
    int lineStyle=0;
    int fillStyle=1;
    bool valid;

    if (argc!=6 && argc!=8)
    {
        std::fprintf(stderr,"Usage: sector_curve_cli prefix x y width height [lineStyle fillStyle]\n");return 2;
    }
    for (index=0;index<4;index++)
    {
        geometry[index]=QString::fromLocal8Bit(argv[index+2]).toLongLong(&valid);
        if (!valid || geometry[index]<2 || geometry[index]>(index%2==0?698:378)) { return 3; }
        coordinatesJson.append(geometry[index]);
    }
    if (geometry[2]!=geometry[3] || geometry[2]<100 || geometry[2]>240
            || geometry[0]+geometry[2]>698 || geometry[1]+geometry[3]>378) { return 3; }
    if (argc==8)
    {
        lineStyle=QString::fromLocal8Bit(argv[6]).toInt(&valid);if (!valid || lineStyle<0 || lineStyle>1) { return 3; }
        fillStyle=QString::fromLocal8Bit(argv[7]).toInt(&valid);if (!valid || fillStyle<0 || fillStyle>1) { return 3; }
    }
    rect=QRectF(0,0,qreal(geometry[2]),qreal(geometry[3]));
    localPath.moveTo(rect.center());localPath.arcTo(rect,45,90);
    translation.translate(qreal(geometry[0]),qreal(geometry[1]));path=translation.map(localPath);
    if (localPath.elementCount()!=8 || path.elementCount()!=8
            || QPointF(path.elementAt(0))==QPointF(path.elementAt(7))) { return 4; }
    // Leave the path open: implicit fill closure must not create a ninth stroked radius edge.
    stroker.setWidth(1);stroker.setCapStyle(Qt::SquareCap);stroker.setJoinStyle(Qt::BevelJoin);strokePath=stroker.createStroke(path);
    if (!flattenHistoricalEllipseCandidate(path,&flattenedFill,&error,nullptr)
            || !flattenHistoricalEllipseCandidate(strokePath,&flattenedStroke,&error,nullptr))
    {
        std::fprintf(stderr,"%s\n",error.toUtf8().constData());return 5;
    }
    if ((fillStyle==1 && !deviceStraightPathCoverage(flattenedFill,DeviceStraightCoveragePrecision::Exact8Bit,&fill,&error))
            || (lineStyle==1 && !deviceStraightPathCoverage(flattenedStroke,DeviceStraightCoveragePrecision::Exact8Bit,&stroke,&error)))
    {
        std::fprintf(stderr,"%s\n",error.toUtf8().constData());return 5;
    }
    prefix=QString::fromLocal8Bit(argv[1]);
    if (!QDir().mkpath(QFileInfo(prefix).absolutePath())) { return 6; }
    pathBytes=streamPath(path);frame=composeFrame(fill,stroke);
    if (!saveBytes(prefix+QStringLiteral("-path.pathstream"),pathBytes)
            || !saveBytes(prefix+QStringLiteral("-local.pathstream"),streamPath(localPath))
            || !saveBytes(prefix+QStringLiteral("-stroke.pathstream"),streamPath(strokePath))
            || !saveBytes(prefix+QStringLiteral("-fill-flat.pathstream"),streamPath(flattenedFill))
            || !saveBytes(prefix+QStringLiteral("-stroke-flat.pathstream"),streamPath(flattenedStroke))
            || !saveBytes(prefix+QStringLiteral("-fill.mask"),fill)
            || !saveBytes(prefix+QStringLiteral("-stroke.mask"),stroke)
            || !saveBytes(prefix+QStringLiteral(".rgb565"),frame)) { return 6; }
    for (index=0;index<path.elementCount();index++)
    {
        element=QJsonArray();element.append(int(path.elementAt(int(index)).type));
        element.append(path.elementAt(int(index)).x);element.append(path.elementAt(int(index)).y);elements.append(element);
        element=QJsonArray();element.append(int(localPath.elementAt(int(index)).type));
        element.append(localPath.elementAt(int(index)).x);element.append(localPath.elementAt(int(index)).y);localElements.append(element);
    }
    transform=QJsonArray{1,0,0,1,geometry[0],geometry[1],0,0,1};
    result.insert(QStringLiteral("geometry"),coordinatesJson);result.insert(QStringLiteral("lineStyle"),lineStyle);
    result.insert(QStringLiteral("fillStyle"),fillStyle);result.insert(QStringLiteral("elements"),elements);
    result.insert(QStringLiteral("localElements"),localElements);result.insert(QStringLiteral("transform"),transform);
    result.insert(QStringLiteral("pathOrigin"),QStringLiteral("local rect(0,0,w,h), then pure integer scene translation"));
    result.insert(QStringLiteral("pathElementCount"),path.elementCount());result.insert(QStringLiteral("pathExplicitlyClosed"),false);
    result.insert(QStringLiteral("startAngle"),45);result.insert(QStringLiteral("sweepAngle"),90);
    result.insert(QStringLiteral("pathSha256"),QString::fromLatin1(QCryptographicHash::hash(pathBytes,QCryptographicHash::Sha256).toHex()));
    result.insert(QStringLiteral("frameSha256"),QString::fromLatin1(QCryptographicHash::hash(frame,QCryptographicHash::Sha256).toHex()));
    result.insert(QStringLiteral("rounding"),QStringLiteral("D(v)=(v+(v>>8)+128)>>8; separate source/destination terms"));
    result.insert(QStringLiteral("nativeProcessLaunched"),false);result.insert(QStringLiteral("usbAccessed"),false);
    result.insert(QStringLiteral("scope"),QStringLiteral("Independent fixed-angle square sector candidate only; no gate support claim."));
    frame=QJsonDocument(result).toJson(QJsonDocument::Indented);
    if (!saveBytes(prefix+QStringLiteral(".json"),frame)) { return 6; }
    std::fwrite(frame.constData(),1,size_t(frame.size()),stdout);return 0;
}
