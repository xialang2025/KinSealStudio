#include "device_polygon_raster.h"
#include <QImage>
#include <QPainter>
#include <QPainterPathStroker>
#include <algorithm>
#include <array>
#include <cmath>
#include <map>

//20261114-1 add: Integrate straight edges independently; no original application or public-library raster code is linked.
struct PolygonFixedPoint { qint64 x; qint64 y; };
struct PolygonCoverageCell { qint64 cover=0; qint64 area=0; };
using PolygonCoverageRows=std::array<std::map<int,PolygonCoverageCell>,480>;

/******************************************************************************
 * Function Name   : polygonFloorDivide
 * Description     : Keep signed rational intersections on the lower integer lattice without C++ truncation bias.
 * Input Parameters: numerator, positiveDenominator
 * Return Value    : Mathematical floor of the quotient.
 * Date            : 2026.11.14
 ******************************************************************************/
static qint64 polygonFloorDivide(qint64 numerator, qint64 positiveDenominator) //20261114-1 add
{
    qint64 quotient=numerator/positiveDenominator;
    if (numerator<0 && numerator%positiveDenominator!=0) { --quotient; }
    return quotient;
}

/******************************************************************************
 * Function Name   : polygonGridCuts
 * Description     : Intersect an oriented straight segment with every crossed row or column grid boundary.
 * Input Parameters: a, b, splitY
 * Return Value    : Ordered endpoints and rational intersections on the 256-unit lattice.
 * Date            : 2026.11.14
 ******************************************************************************/
static QVector<PolygonFixedPoint> polygonGridCuts(const PolygonFixedPoint &a, const PolygonFixedPoint &b, bool splitY) //20261114-1 add
{
    QVector<PolygonFixedPoint> cuts;
    QVector<PolygonFixedPoint> interior;
    PolygonFixedPoint point;
    qint64 start=splitY?a.y:a.x;
    qint64 end=splitY?b.y:b.x;
    qint64 otherStart=splitY?a.x:a.y;
    qint64 otherEnd=splitY?b.x:b.y;
    qint64 lower=qMin(start,end);
    qint64 upper=qMax(start,end);
    qint64 boundary;
    qint64 other;
    int i;

    cuts.append(a);
    if (start!=end)
    {
        for (boundary=(polygonFloorDivide(lower,256)+1)*256;boundary<upper;boundary+=256)
        {
            other=otherStart+polygonFloorDivide((otherEnd-otherStart)*qAbs(boundary-start),qAbs(end-start));
            point=splitY?PolygonFixedPoint{other,boundary}:PolygonFixedPoint{boundary,other};interior.append(point);
        }
        if (end<start) { std::reverse(interior.begin(),interior.end()); }
        for (i=0;i<interior.size();++i) { cuts.append(interior[i]); }
    }
    cuts.append(b);return cuts;
}

/******************************************************************************
 * Function Name   : polygonIntegrateEdge
 * Description     : Sum signed cover and twice-area per cell after separate row and column intersections.
 * Input Parameters: a, b, cells
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
static void polygonIntegrateEdge(const PolygonFixedPoint &a, const PolygonFixedPoint &b, PolygonCoverageRows *cells) //20261114-1 add
{
    QVector<PolygonFixedPoint> rows;
    QVector<PolygonFixedPoint> columns;
    PolygonFixedPoint p;
    PolygonFixedPoint q;
    PolygonCoverageCell *cell;
    int rowIndex;
    int columnIndex;
    int x;
    int y;
    qint64 deltaY;

    if (a.y==b.y) { return; }
    rows=polygonGridCuts(a,b,true);
    for (rowIndex=0;rowIndex+1<rows.size();++rowIndex)
    {
        y=int(polygonFloorDivide(rows[rowIndex].y+rows[rowIndex+1].y,512));
        if (y<0 || y>=480) { continue; }
        columns=polygonGridCuts(rows[rowIndex],rows[rowIndex+1],false);
        for (columnIndex=0;columnIndex+1<columns.size();++columnIndex)
        {
            p=columns[columnIndex];q=columns[columnIndex+1];x=int(polygonFloorDivide(p.x+q.x,512));deltaY=q.y-p.y;
            cell=&(*cells)[y][x];cell->cover+=deltaY;cell->area+=(p.x-x*256+q.x-x*256)*deltaY;
        }
    }
}

/******************************************************************************
 * Function Name   : polygonCoverageAlpha
 * Description     : Fold signed winding coverage into the selected fill rule and saturate a full pixel to 255.
 * Input Parameters: signedCoverage, rule
 * Return Value    : Byte-range alpha.
 * Date            : 2026.11.14
 ******************************************************************************/
static int polygonCoverageAlpha(qint64 signedCoverage, Qt::FillRule rule) //20261114-1 add
{
    qint64 alpha=qAbs(signedCoverage);
    if (rule==Qt::OddEvenFill) { alpha%=512;if (alpha>256) { alpha=512-alpha; } }
    return int(qMin(qint64(255),alpha));
}

/******************************************************************************
 * Function Name   : polygonPathCoverageWithPrecision
 * Description     : Convert a straight closed path to coverage, rejecting curves instead of flattening unproved shapes.
 * Input Parameters: path, precision
 * Return Value    : Full-page alpha mask, or an empty array for an invalid path.
 * Date            : 2026.11.14
 ******************************************************************************/
static QByteArray polygonPathCoverageWithPrecision(const QPainterPath &path, DeviceStraightCoveragePrecision precision) //20261114-2 change
{
    PolygonCoverageRows cells;
    PolygonFixedPoint first={0,0};
    PolygonFixedPoint previous={0,0};
    PolygonFixedPoint target;
    QPainterPath::Element element;
    QByteArray mask(800*480,0);
    std::map<int,PolygonCoverageCell>::const_iterator cell;
    qint64 running;
    int index;
    int y;
    int x;
    int last;
    int alpha;
    bool started=false;

    for (index=0;index<path.elementCount();++index)
    {
        element=path.elementAt(index);
        // Preserve polygon rounding while allowing already-flattened ellipse points to keep all eight fraction bits.
        target=precision==DeviceStraightCoveragePrecision::Quantized26_6
                ?PolygonFixedPoint{qint64(element.x*64)*4,qint64(element.y*64)*4}
                :PolygonFixedPoint{qint64(element.x*256),qint64(element.y*256)};
        if (element.type==QPainterPath::MoveToElement)
        {
            if (started) { polygonIntegrateEdge(previous,first,&cells); }
            first=previous=target;started=true;
        }
        else if (element.type==QPainterPath::LineToElement && started) { polygonIntegrateEdge(previous,target,&cells);previous=target; }
        else { return QByteArray(); }
    }
    if (!started) { return QByteArray(); }
    polygonIntegrateEdge(previous,first,&cells);
    for (y=0;y<480;++y)
    {
        running=0;last=0;
        for (cell=cells[y].begin();cell!=cells[y].end();++cell)
        {
            alpha=polygonCoverageAlpha(running,path.fillRule());
            for (x=qMax(0,last);x<qMin(800,cell->first);++x) { mask[y*800+x]=char(alpha); }
            running+=cell->second.cover;
            alpha=polygonCoverageAlpha(polygonFloorDivide(running*512-cell->second.area,512),path.fillRule());
            if (cell->first>=0 && cell->first<800) { mask[y*800+cell->first]=char(alpha); }
            last=cell->first+1;
        }
    }
    return mask;
}

/******************************************************************************
 * Function Name   : deviceStraightPathCoverage
 * Description     : Validate controlled input before applying the existing signed straight-edge integral.
 * Input Parameters: path, precision, output, error
 * Return Value    : True on a complete page mask; false without modifying output.
 * Date            : 2026.11.14
 ******************************************************************************/
bool deviceStraightPathCoverage(const QPainterPath &path, DeviceStraightCoveragePrecision precision,
                               QByteArray *output, QString *error) //20261114-2 add
{
    QPainterPath::Element element;
    QByteArray candidate;
    qreal scaledX;
    qreal scaledY;
    int index;

    if (output==nullptr || path.elementCount()==0 || path.elementCount()>4096
            || (precision!=DeviceStraightCoveragePrecision::Quantized26_6
                && precision!=DeviceStraightCoveragePrecision::Exact8Bit))
    {
        if (error!=nullptr) { *error=QStringLiteral("设备直边覆盖输入、精度或元素预算无效，未写输出。"); }
        return false;
    }
    for (index=0;index<path.elementCount();++index)
    {
        element=path.elementAt(index);
        if ((element.type!=QPainterPath::MoveToElement && element.type!=QPainterPath::LineToElement)
                || !std::isfinite(element.x) || !std::isfinite(element.y)
                || element.x<0 || element.x>800 || element.y<0 || element.y>480)
        {
            if (error!=nullptr) { *error=QStringLiteral("设备直边覆盖出现曲线、非有限或越界坐标，未写输出。"); }
            return false;
        }
        if (precision==DeviceStraightCoveragePrecision::Exact8Bit)
        {
            scaledX=element.x*256;scaledY=element.y*256;
            if (scaledX!=qreal(qint64(scaledX)) || scaledY!=qreal(qint64(scaledY)))
            {
                if (error!=nullptr) { *error=QStringLiteral("设备直边覆盖需要精确的8位子像素格点，未写输出。"); }
                return false;
            }
        }
    }
    candidate=polygonPathCoverageWithPrecision(path,precision);
    if (candidate.size()!=800*480)
    {
        if (error!=nullptr) { *error=QStringLiteral("设备直边覆盖未产生完整页掩码，未写输出。"); }
        return false;
    }
    *output=candidate;
    if (error!=nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : polygonBlackOver
 * Description     : Blend black coverage in the observed 8-bit background space before RGB565 quantization.
 * Input Parameters: pixel, alpha
 * Return Value    : Opaque RGB32 pixel.
 * Date            : 2026.11.14
 ******************************************************************************/
static QRgb polygonBlackOver(QRgb pixel, int alpha) //20261114-1 add
{
    int red=qRed(pixel)*(255-alpha);
    int green=qGreen(pixel)*(255-alpha);
    int blue=qBlue(pixel)*(255-alpha);
    return qRgb((red+(red>>8)+128)>>8,(green+(green>>8)+128)>>8,(blue+(blue>>8)+128)>>8);
}

/******************************************************************************
 * Function Name   : paintDevicePolygonGraphic
 * Description     : Bake sampled black polygon fill/stroke coverage into an isolated background patch.
 * Input Parameters: painter, geometry, properties, error
 * Return Value    : True after painting; false before any drawing if path coverage is unsupported.
 * Date            : 2026.11.14
 ******************************************************************************/
bool paintDevicePolygonGraphic(QPainter *painter, const QRect &geometry, const GraphicProperties &properties, QString *error) //20261114-1 add
{
    QPainterPath path;
    QPainterPathStroker stroker;
    QByteArray fill(800*480,0);
    QByteArray stroke(800*480,0);
    QRect patchRect=geometry.adjusted(-1,-1,2,2).intersected(QRect(0,0,800,480));
    QImage patch(patchRect.size(),QImage::Format_RGB32);
    QRgb *row;
    QRgb pixel;
    int y;
    int x;
    int absoluteX;
    int absoluteY;
    int index;
    QPoint point;

    // Reconstruct the already-validated integer semantics, avoiding normalized-double values just below an integer.
    for (index=0;index<properties.points.size();++index)
    {
        point=QPoint(qRound(geometry.x()+properties.points[index].x()*geometry.width()),qRound(geometry.y()+properties.points[index].y()*geometry.height()));
        if (index==0) { path.moveTo(point); } else { path.lineTo(point); }
    }
    path.closeSubpath();
    if (properties.fillStyle==1) { fill=polygonPathCoverageWithPrecision(path,DeviceStraightCoveragePrecision::Quantized26_6); } //20261114-2 change
    if (properties.lineStyle==1)
    {
        stroker.setWidth(1);stroker.setCapStyle(Qt::SquareCap);stroker.setJoinStyle(Qt::BevelJoin);
        stroke=polygonPathCoverageWithPrecision(stroker.createStroke(path),DeviceStraightCoveragePrecision::Quantized26_6); //20261114-2 change
    }
    if (painter==nullptr || patch.isNull() || fill.size()!=800*480 || stroke.size()!=800*480)
    {
        if (error!=nullptr) { *error=QStringLiteral("多边形设备栅格出现未核路径，未绘制输出。"); }
        return false;
    }
    for (y=0;y<patch.height();++y)
    {
        row=reinterpret_cast<QRgb *>(patch.scanLine(y));absoluteY=patchRect.y()+y;
        for (x=0;x<patch.width();++x)
        {
            absoluteX=patchRect.x()+x;
            // The plan excludes touching peers, so only the verified page background can lie below this patch.
            pixel=absoluteX==0 || absoluteY==0?qRgb(0,0,0):qRgb(197,222,255);
            pixel=polygonBlackOver(pixel,static_cast<unsigned char>(fill[absoluteY*800+absoluteX]));
            row[x]=polygonBlackOver(pixel,static_cast<unsigned char>(stroke[absoluteY*800+absoluteX]));
        }
    }
    painter->save();painter->setRenderHint(QPainter::SmoothPixmapTransform,false);
    painter->drawImage(patchRect.topLeft(),patch);painter->restore();return true;
}
