#include "device_roundrect_raster.h"
#include <QPainter>
#include <QPainterPathStroker>
#include <algorithm>
#include <array>
#include <cmath>

//20261113-16 add: Coordinates and intersections are bounded by the admitted 800x480 page.
struct RoundedFixedPoint
{
    qint64 x;
    qint64 y;
};

struct RoundedIntersection
{
    qint64 x;
    int winding;
};

using RoundedScanRows = std::array<QVector<RoundedIntersection>,480>;

/******************************************************************************
 * Function Name   : roundedMidpoint
 * Description     : Keep each subdivision on the observed 26.6 integer lattice.
 * Input Parameters: a, b
 * Return Value    : Truncated integer midpoint.
 * Date            : 2026.11.13
 ******************************************************************************/
static RoundedFixedPoint roundedMidpoint(const RoundedFixedPoint &a, const RoundedFixedPoint &b) //20261113-16 add
{
    return {(a.x+b.x)/2,(a.y+b.y)/2};
}

/******************************************************************************
 * Function Name   : roundedScanEdge
 * Description     : Emit fixed-point intersections at each covered page scan row.
 * Input Parameters: a, b, rows
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
static void roundedScanEdge(RoundedFixedPoint a, RoundedFixedPoint b, RoundedScanRows *rows) //20261113-16 add
{
    int winding = 1;
    int top;
    int bottom;
    int y;
    qint64 slope;
    qint64 initial;

    if (a.y>b.y) { std::swap(a,b); winding=-1; }
    a.x+=32; a.y+=32; b.x+=32; b.y+=32;
    top=qMax(0,int((a.y+31)>>6)); bottom=qMin(479,int((b.y-33)>>6));
    if (top>bottom) { return; }
    slope=b.y==a.y?0:qint64(double(b.x-a.x)/double(b.y-a.y)*65536.0);
    // Intersections use 16.16 arithmetic; the final subtraction preserves the native downward tie.
    initial=32768+a.x*1024-1+((slope*(qint64(top)*65536+32768-a.y*1024))>>16);
    for (y=top;y<=bottom;++y) { (*rows)[y].append({(initial+(y-top)*slope)>>16,winding}); }
}

/******************************************************************************
 * Function Name   : roundedScanCubic
 * Description     : Subdivide a cubic using the observed quarter-pixel integer flatness rule.
 * Input Parameters: a, b, c, d, depth, rows
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
static void roundedScanCubic(const RoundedFixedPoint &a, const RoundedFixedPoint &b,
                             const RoundedFixedPoint &c, const RoundedFixedPoint &d,
                             int depth, RoundedScanRows *rows) //20261113-16 add
{
    qint64 dx=d.x-a.x;
    qint64 dy=d.y-a.y;
    qint64 length=qAbs(dx)+qAbs(dy);
    qint64 distance;
    bool flat;
    RoundedFixedPoint ab;
    RoundedFixedPoint bc;
    RoundedFixedPoint cd;
    RoundedFixedPoint left;
    RoundedFixedPoint right;
    RoundedFixedPoint center;

    if (length>64)
    {
        distance=qAbs((b.x-a.x)*dy-(b.y-a.y)*dx)+qAbs((c.x-a.x)*dy-(c.y-a.y)*dx);
        flat=distance<=16*length;
    }
    else { flat=qAbs(b.x-a.x)+qAbs(b.y-a.y)+qAbs(c.x-a.x)+qAbs(c.y-a.y)<=16; }
    if (flat || depth==32) { roundedScanEdge(a,d,rows); return; }
    ab=roundedMidpoint(a,b); bc=roundedMidpoint(b,c); cd=roundedMidpoint(c,d);
    left=roundedMidpoint(ab,bc); right=roundedMidpoint(bc,cd); center=roundedMidpoint(left,right);
    roundedScanCubic(a,ab,left,center,depth+1,rows);
    roundedScanCubic(center,right,cd,d,depth+1,rows);
}

/******************************************************************************
 * Function Name   : roundedQuantizedPoint
 * Description     : Quantize scene coordinates only after applying the integer object translation.
 * Input Parameters: element, offset
 * Return Value    : Fixed 26.6 scene point.
 * Date            : 2026.11.13
 ******************************************************************************/
static RoundedFixedPoint roundedQuantizedPoint(const QPainterPath::Element &element, const QPoint &offset) //20261113-16 add
{
    return {qint64((element.x+offset.x())*64),qint64((element.y+offset.y())*64)};
}

/******************************************************************************
 * Function Name   : paintRoundedFixedPath
 * Description     : Fill a bounded path with the verified integer scan conversion instead of Qt6 curve rasterization.
 * Input Parameters: painter, path, offset, color
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
static void paintRoundedFixedPath(QPainter *painter, const QPainterPath &path, const QPoint &offset, const QColor &color) //20261113-16 add
{
    RoundedScanRows rows;
    RoundedFixedPoint first = {0,0};
    RoundedFixedPoint previous = {0,0};
    RoundedFixedPoint target;
    RoundedFixedPoint second;
    RoundedFixedPoint third;
    RoundedIntersection intersection;
    QPainterPath::Element element;
    int index;
    int y;
    int winding;
    int last;
    int current;
    bool started=false;

    for (index=0;index<path.elementCount();++index)
    {
        element=path.elementAt(index); target=roundedQuantizedPoint(element,offset);
        if (element.type==QPainterPath::MoveToElement)
        {
            if (started) { roundedScanEdge(previous,first,&rows); }
            first=previous=target; started=true;
        }
        else if (element.type==QPainterPath::LineToElement) { roundedScanEdge(previous,target,&rows); previous=target; }
        else if (element.type==QPainterPath::CurveToElement && index+2<path.elementCount())
        {
            second=roundedQuantizedPoint(path.elementAt(index+1),offset); third=roundedQuantizedPoint(path.elementAt(index+2),offset);
            roundedScanCubic(previous,target,second,third,0,&rows); previous=third; index+=2;
        }
    }
    if (started) { roundedScanEdge(previous,first,&rows); }
    for (y=0;y<480;++y)
    {
        std::sort(rows[y].begin(),rows[y].end(),[](const RoundedIntersection &a,const RoundedIntersection &b) { return a.x<b.x; });
        winding=0; last=0;
        Q_FOREACH (intersection,rows[y])
        {
            current=qBound(0,int(intersection.x),800);
            if ((path.fillRule()==Qt::WindingFill?winding!=0:(winding&1)!=0) && current>last) { painter->fillRect(QRect(last,y,current-last,1),color); }
            winding+=intersection.winding; last=current;
        }
    }
}

/******************************************************************************
 * Function Name   : paintDeviceRoundrectGraphic
 * Description     : Independently build the admitted rounded-box fill/stroke paths and bake their exact native background spans.
 * Input Parameters: painter, geometry, properties
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void paintDeviceRoundrectGraphic(QPainter *painter, const QRect &geometry, const GraphicProperties &properties) //20261113-16 add
{
    QPainterPath path;
    QPainterPathStroker stroker;

    if (painter==nullptr) { return; }
    path.addRoundedRect(QRectF(0,0,geometry.width(),geometry.height()),10,10,Qt::AbsoluteSize);
    stroker.setWidth(1); stroker.setCapStyle(Qt::SquareCap); stroker.setJoinStyle(Qt::BevelJoin);
    painter->save(); painter->setRenderHint(QPainter::Antialiasing,false);
    if (properties.fillStyle==1) { paintRoundedFixedPath(painter,path,geometry.topLeft(),Qt::white); }
    if (properties.lineStyle==1) { paintRoundedFixedPath(painter,stroker.createStroke(path),geometry.topLeft(),Qt::black); }
    painter->restore();
}
