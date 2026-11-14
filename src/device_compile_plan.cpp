#include "device_compile_plan.h"
#include "native_project_codec.h"
#include <QJsonArray>
#include <algorithm>
#include <cmath>

/******************************************************************************
 * Function Name   : compilationIssue
 * Description     : Identify the stable page/object and exact rejected field without changing a prepared plan.
 * Input Parameters: project, object, path, value, reason, error, issue
 * Return Value    : False.
 * Date            : 2026.11.13
 ******************************************************************************/
static bool compilationIssue(const HmiProject &project, const HmiObject &object, const QString &path,
                             const QJsonValue &value, const QString &reason, QString *error, QJsonObject *issue) //20261113-11 add
{
    if (error != nullptr) { *error = QStringLiteral("页面%1，控件%2，%3：%4").arg(project.activePageId, object.id, path, reason); }
    if (issue != nullptr) { *issue = {{QStringLiteral("pageId"),project.activePageId},{QStringLiteral("objectId"),object.id},
        {QStringLiteral("property"),path},{QStringLiteral("value"),value},{QStringLiteral("reason"),reason}}; }
    return false;
}

/******************************************************************************
 * Function Name   : polygonOrientation
 * Description     : Compute the exact directed area for already bounded integer screen coordinates.
 * Input Parameters: first, second, third
 * Return Value    : Signed twice-area of the three-point triangle.
 * Date            : 2026.11.14
 ******************************************************************************/
static qint64 polygonOrientation(const QPoint &first, const QPoint &second, const QPoint &third) //20261114-1 add
{
    return qint64(second.x()-first.x())*qint64(third.y()-first.y())-
           qint64(second.y()-first.y())*qint64(third.x()-first.x());
}

/******************************************************************************
 * Function Name   : polygonPointOnSegment
 * Description     : Include both endpoints when detecting unproved polygon edge contacts.
 * Input Parameters: point, first, second
 * Return Value    : True when the point lies on the closed integer segment.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool polygonPointOnSegment(const QPoint &point, const QPoint &first, const QPoint &second) //20261114-1 add
{
    return polygonOrientation(first,second,point)==0 &&
           point.x()>=qMin(first.x(),second.x()) && point.x()<=qMax(first.x(),second.x()) &&
           point.y()>=qMin(first.y(),second.y()) && point.y()<=qMax(first.y(),second.y());
}

/******************************************************************************
 * Function Name   : polygonSegmentsTouch
 * Description     : Reject nonadjacent crossings, endpoint contacts and collinear overlaps without floating-point ties.
 * Input Parameters: first, second, third, fourth
 * Return Value    : True when the two closed integer segments intersect or touch.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool polygonSegmentsTouch(const QPoint &first, const QPoint &second, const QPoint &third, const QPoint &fourth) //20261114-1 add
{
    qint64 firstSide=polygonOrientation(first,second,third);
    qint64 secondSide=polygonOrientation(first,second,fourth);
    qint64 thirdSide=polygonOrientation(third,fourth,first);
    qint64 fourthSide=polygonOrientation(third,fourth,second);

    if (polygonPointOnSegment(third,first,second) || polygonPointOnSegment(fourth,first,second) ||
        polygonPointOnSegment(first,third,fourth) || polygonPointOnSegment(second,third,fourth))
    {
        return true;
    }
    return ((firstSide<0 && secondSide>0) || (firstSide>0 && secondSide<0)) &&
           ((thirdSide<0 && fourthSide>0) || (thirdSide>0 && fourthSide<0));
}

/******************************************************************************
 * Function Name   : prepareDevicePolygon
 * Description     : Admit only sampled simple polygons whose isolated background patches cannot erase another object.
 * Input Parameters: project, object, objects, error, issue
 * Return Value    : True for the bounded polygon slice; false with an exact rejected field.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool prepareDevicePolygon(const HmiProject &project, const HmiObject &object, const QList<HmiObject> &objects,
                                 QString *error, QJsonObject *issue) //20261114-1 add
{
    QVector<QPoint> vertices;
    QPointF normalized;
    QPointF absolute;
    HmiObject peer;
    QRect patch=object.geometry.adjusted(-1,-1,2,2).intersected(QRect(0,0,800,480));
    QRect peerPatch;
    qint64 twiceArea=0;
    int index;
    int other;
    int count=object.graphic.points.size();

    if (object.graphic.lineStyle!=0 && object.graphic.lineStyle!=1)
    { return compilationIssue(project,object,QStringLiteral("graphic.lineStyle"),object.graphic.lineStyle,QStringLiteral("多边形只开放已核无线/实线。"),error,issue); }
    if (object.graphic.fillStyle!=0 && object.graphic.fillStyle!=1)
    { return compilationIssue(project,object,QStringLiteral("graphic.fillStyle"),object.graphic.fillStyle,QStringLiteral("多边形只开放已核无填充/纯色填充。"),error,issue); }
    if (object.graphic.lineWidth!=1)
    { return compilationIssue(project,object,QStringLiteral("graphic.lineWidth"),object.graphic.lineWidth,QStringLiteral("多边形本切片只开放宽度1。"),error,issue); }
    if (object.graphic.alpha!=255)
    { return compilationIssue(project,object,QStringLiteral("graphic.alpha"),object.graphic.alpha,QStringLiteral("多边形本切片只开放不透明颜色。"),error,issue); }
    if (object.graphic.lineColor!=QColor(Qt::black) || object.graphic.foreground!=QColor(Qt::white) || object.graphic.background!=QColor(Qt::black))
    { return compilationIssue(project,object,QStringLiteral("graphic.colors"),QJsonArray{object.graphic.lineColor.name(QColor::HexArgb),object.graphic.foreground.name(QColor::HexArgb),object.graphic.background.name(QColor::HexArgb)},QStringLiteral("多边形本切片只开放已核黑边/白前景/黑背景草稿，包括休眠颜色。"),error,issue); }
    if (object.graphic.endArrow!=0 || object.graphic.endCap!=0)
    { return compilationIssue(project,object,QStringLiteral("graphic.endpoints"),QJsonArray{object.graphic.endArrow,object.graphic.endCap},QStringLiteral("多边形本切片只开放无箭头及端点0。"),error,issue); }
    if (object.geometry.width()<100 || object.geometry.height()<100)
    { return compilationIssue(project,object,QStringLiteral("geometry.sampledSize"),QJsonArray{object.geometry.width(),object.geometry.height()},QStringLiteral("多边形本切片只开放宽高至少100的已采样范围。"),error,issue); }
    if (count<3 || count>8)
    { return compilationIssue(project,object,QStringLiteral("graphic.points"),count,QStringLiteral("多边形本切片只开放3到8个有序顶点。"),error,issue); }
    Q_FOREACH (normalized,object.graphic.points)
    {
        absolute=QPointF(object.geometry.x()+normalized.x()*object.geometry.width(),object.geometry.y()+normalized.y()*object.geometry.height());
        // Check finiteness and screen bounds before rounding so malformed input never reaches integer conversion.
        if (!std::isfinite(normalized.x()) || !std::isfinite(normalized.y()) || normalized.x()<0 || normalized.x()>1 || normalized.y()<0 || normalized.y()>1 ||
            !std::isfinite(absolute.x()) || !std::isfinite(absolute.y()) || absolute.x()<2 || absolute.x()>698 || absolute.y()<2 || absolute.y()>398 ||
            std::abs(absolute.x()-std::round(absolute.x()))>1e-8 || std::abs(absolute.y()-std::round(absolute.y()))>1e-8)
        { return compilationIssue(project,object,QStringLiteral("graphic.points"),QJsonArray{absolute.x(),absolute.y()},QStringLiteral("多边形仅开放已采样整数顶点，X范围2到698、Y范围2到398。"),error,issue); }
        vertices.append(QPoint(qRound(absolute.x()),qRound(absolute.y())));
    }
    for (index=0;index<count;++index)
    {
        for (other=index+1;other<count;++other)
        {
            if (vertices[index]==vertices[other])
            { return compilationIssue(project,object,QStringLiteral("graphic.points.duplicate"),QJsonArray{index,other},QStringLiteral("多边形取整后不得含任何重复顶点。"),error,issue); }
        }
        if (polygonOrientation(vertices[index],vertices[(index+1)%count],vertices[(index+2)%count])==0)
        { return compilationIssue(project,object,QStringLiteral("graphic.points.collinear"),index,QStringLiteral("多边形连续三个顶点共线的路径尚未核定。"),error,issue); }
        twiceArea+=qint64(vertices[index].x())*vertices[(index+1)%count].y()-qint64(vertices[index].y())*vertices[(index+1)%count].x();
    }
    if (twiceArea==0)
    { return compilationIssue(project,object,QStringLiteral("graphic.points.area"),0,QStringLiteral("多边形面积不能为零。"),error,issue); }
    for (index=0;index<count;++index)
    {
        for (other=index+1;other<count;++other)
        {
            // Consecutive edges share exactly one endpoint; all other edge contacts are outside this simple-path slice.
            if (other==index+1 || (index==0 && other==count-1)) { continue; }
            if (polygonSegmentsTouch(vertices[index],vertices[(index+1)%count],vertices[other],vertices[(other+1)%count]))
            { return compilationIssue(project,object,QStringLiteral("graphic.points.intersection"),QJsonArray{index,other},QStringLiteral("多边形非邻边不能交叉或接触。"),error,issue); }
        }
    }
    Q_FOREACH (peer,objects)
    {
        if (peer.id==object.id) { continue; }
        peerPatch=peer.geometry.adjusted(-1,-1,2,2).intersected(QRect(0,0,800,480));
        // Treat the physical pixel patch boundaries as closed, preserving even a one-pixel gap before composition is proved.
        if (patch.x()<=peerPatch.x()+peerPatch.width() && peerPatch.x()<=patch.x()+patch.width() &&
            patch.y()<=peerPatch.y()+peerPatch.height() && peerPatch.y()<=patch.y()+patch.height())
        { return compilationIssue(project,object,QStringLiteral("geometry.overlap"),peer.id,QStringLiteral("多边形背景补片与其它对象扩展补片相交或接触的合成尚未核定。"),error,issue); }
    }
    return true;
}

/******************************************************************************
 * Function Name   : prepareDeviceEllipse
 * Description     : Admit the source-bound opaque ellipse slice only where its reconstructed page patch is isolated.
 * Input Parameters: project, object, objects, error, issue
 * Return Value    : True for supported geometry and appearance; false identifies the rejected property.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool prepareDeviceEllipse(const HmiProject &project, const HmiObject &object, const QList<HmiObject> &objects,
                                 QString *error, QJsonObject *issue) //20261114-2 add
{
    HmiObject peer;
    QRect patch=object.geometry.adjusted(-1,-1,2,2).intersected(QRect(0,0,800,480));
    QRect peerPatch;

    if (object.geometry.x()<2 || object.geometry.y()<2 || object.geometry.width()<100 || object.geometry.height()<100 ||
        qint64(object.geometry.x())+object.geometry.width()>698 || qint64(object.geometry.y())+object.geometry.height()>378)
    { return compilationIssue(project,object,QStringLiteral("geometry.ellipseBounds"),QJsonArray{object.geometry.x(),object.geometry.y(),object.geometry.width(),object.geometry.height()},QStringLiteral("椭圆本切片要求X/Y至少2、宽高至少100，右端不超过698、下端不超过378。"),error,issue); }
    if ((object.graphic.lineStyle!=0 && object.graphic.lineStyle!=1) || (object.graphic.fillStyle!=0 && object.graphic.fillStyle!=1))
    { return compilationIssue(project,object,QStringLiteral("graphic.styles"),QJsonArray{object.graphic.lineStyle,object.graphic.fillStyle},QStringLiteral("椭圆仅开放已核无线/实线及无填充/纯色组合。"),error,issue); }
    if (object.graphic.lineWidth!=1 || object.graphic.alpha!=255)
    { return compilationIssue(project,object,QStringLiteral("graphic.stroke"),QJsonArray{object.graphic.lineWidth,object.graphic.alpha},QStringLiteral("椭圆仅开放线宽1、Alpha255，休眠草稿同样保持此限制。"),error,issue); }
    if (object.graphic.lineColor!=QColor(Qt::black) || object.graphic.foreground!=QColor(Qt::black) || object.graphic.background!=QColor(Qt::white))
    { return compilationIssue(project,object,QStringLiteral("graphic.colors"),QJsonArray{object.graphic.lineColor.name(QColor::HexArgb),object.graphic.foreground.name(QColor::HexArgb),object.graphic.background.name(QColor::HexArgb)},QStringLiteral("椭圆仅开放黑边/黑前景/白背景草稿，包括关闭图层。"),error,issue); }
    if (object.graphic.endArrow!=0 || object.graphic.endCap!=0)
    { return compilationIssue(project,object,QStringLiteral("graphic.endpoints"),QJsonArray{object.graphic.endArrow,object.graphic.endCap},QStringLiteral("椭圆没有箭头或端帽字段，结构体值须为0。"),error,issue); }
    Q_FOREACH (peer,objects)
    {
        if (peer.id==object.id) { continue; }
        peerPatch=peer.geometry.adjusted(-1,-1,2,2).intersected(QRect(0,0,800,480));
        // The renderer rebuilds the verified page background, so even physical patch boundary contact is excluded.
        if (patch.x()<=peerPatch.x()+peerPatch.width() && peerPatch.x()<=patch.x()+patch.width() &&
            patch.y()<=peerPatch.y()+peerPatch.height() && peerPatch.y()<=patch.y()+patch.height())
        { return compilationIssue(project,object,QStringLiteral("geometry.overlap"),peer.id,QStringLiteral("椭圆背景补片与其它对象扩展补片相交或接触的合成尚未核定。"),error,issue); }
    }
    return true;
}

/******************************************************************************
 * Function Name   : prepareDeviceSector
 * Description     : Admit the source-bound opaque sector slice only where its reconstructed page patch is isolated.
 * Input Parameters: project, object, objects, error, issue
 * Return Value    : True for supported geometry and appearance; false identifies the rejected property.
 * Date            : 2026.11.14
 ******************************************************************************/
static bool prepareDeviceSector(const HmiProject &project, const HmiObject &object, const QList<HmiObject> &objects,
                                 QString *error, QJsonObject *issue) //20261114-3 add
{
    HmiObject peer;
    QRect patch=object.geometry.adjusted(-1,-1,2,2).intersected(QRect(0,0,800,480));
    QRect peerPatch;

    if (object.geometry.x()<2 || object.geometry.y()<2 || object.geometry.width()<100 || object.geometry.height()<100 || object.geometry.width()!=object.geometry.height() || object.geometry.width()>240 ||
        qint64(object.geometry.x())+object.geometry.width()>698 || qint64(object.geometry.y())+object.geometry.height()>378)
    { return compilationIssue(project,object,QStringLiteral("geometry.sectorBounds"),QJsonArray{object.geometry.x(),object.geometry.y(),object.geometry.width(),object.geometry.height()},QStringLiteral("扇形本切片要求X/Y至少2、边长100到240的正方形，右端不超过698、下端不超过378。"),error,issue); }
    if ((object.graphic.lineStyle!=0 && object.graphic.lineStyle!=1) || (object.graphic.fillStyle!=0 && object.graphic.fillStyle!=1))
    { return compilationIssue(project,object,QStringLiteral("graphic.styles"),QJsonArray{object.graphic.lineStyle,object.graphic.fillStyle},QStringLiteral("扇形仅开放已核无线/实线及无填充/纯色组合。"),error,issue); }
    if (object.graphic.lineWidth!=1 || object.graphic.alpha!=255)
    { return compilationIssue(project,object,QStringLiteral("graphic.stroke"),QJsonArray{object.graphic.lineWidth,object.graphic.alpha},QStringLiteral("扇形仅开放线宽1、Alpha255，休眠草稿同样保持此限制。"),error,issue); }
    if (object.graphic.lineColor!=QColor(Qt::black) || object.graphic.foreground!=QColor(Qt::black) || object.graphic.background!=QColor(Qt::white))
    { return compilationIssue(project,object,QStringLiteral("graphic.colors"),QJsonArray{object.graphic.lineColor.name(QColor::HexArgb),object.graphic.foreground.name(QColor::HexArgb),object.graphic.background.name(QColor::HexArgb)},QStringLiteral("扇形仅开放黑边/黑前景/白背景草稿，包括关闭图层。"),error,issue); }
    if (object.graphic.endArrow!=0 || object.graphic.endCap!=0)
    { return compilationIssue(project,object,QStringLiteral("graphic.endpoints"),QJsonArray{object.graphic.endArrow,object.graphic.endCap},QStringLiteral("扇形没有箭头或端帽字段，结构体值须为0。"),error,issue); }
    Q_FOREACH (peer,objects)
    {
        if (peer.id==object.id) { continue; }
        peerPatch=peer.geometry.adjusted(-1,-1,2,2).intersected(QRect(0,0,800,480));
        // The renderer rebuilds the verified page background, so even physical patch boundary contact is excluded.
        if (patch.x()<=peerPatch.x()+peerPatch.width() && peerPatch.x()<=patch.x()+patch.width() &&
            patch.y()<=peerPatch.y()+peerPatch.height() && peerPatch.y()<=patch.y()+patch.height())
        { return compilationIssue(project,object,QStringLiteral("geometry.overlap"),peer.id,QStringLiteral("扇形背景补片与其它对象扩展补片相交或接触的合成尚未核定。"),error,issue); }
    }
    return true;
}

/******************************************************************************
 * Function Name   : prepareDeviceCompilePlan
 * Description     : Freeze supported ordered semantics and reject unproved graphics before allocating output segments.
 * Input Parameters: project, plan, error, issue
 * Return Value    : True with a complete projection; caller plan is unchanged on failure.
 * Date            : 2026.11.13
 ******************************************************************************/
bool prepareDeviceCompilePlan(const HmiProject &project, DeviceCompilePlan *plan, QString *error, QJsonObject *issue) //20261113-11 add
{
    DeviceCompilePlan prepared;
    HmiObject object;
    HmiObject peer;
    HmiObject empty;
    QPointF point;
    QPointF previous;
    QPointF absolute;
    bool first;
    QString diagnostic;

    if (plan == nullptr) { return compilationIssue(project,empty,QStringLiteral("output"),QJsonValue(),QStringLiteral("编译计划输出为空。"),error,issue); }
    if (!project.validateSinglePageBackend(&diagnostic)) { return compilationIssue(project,empty,QStringLiteral("project.topology"),project.pageCount(),diagnostic,error,issue); }
    if (project.targetModel != QStringLiteral("AMT070W124-L800X480R") || project.pageWidth != 800 || project.pageHeight != 480 || project.objects.isEmpty() || project.objects.size() > 100 || project.languages.isEmpty() || project.languages.size() > 8)
    { return compilationIssue(project,empty,QStringLiteral("project.target"),project.targetModel,QStringLiteral("只支持800×480、1..100对象、1..8语言的已核型号。"),error,issue); }
    if (!project.nativeSourceSnapshot.isEmpty() && !nativeProjectUsesSupportedDeviceDefaults(project.nativeSourceSnapshot,&diagnostic))
    { return compilationIssue(project,empty,QStringLiteral("nativeSourceSnapshot"),QStringLiteral("retained"),diagnostic,error,issue); }
    prepared.objects = project.objects;
    std::stable_sort(prepared.objects.begin(),prepared.objects.end(),[](const HmiObject &a,const HmiObject &b) { return a.layer < b.layer; });
    Q_FOREACH (object,prepared.objects)
    {
        if (object.geometry.isEmpty() || object.geometry.x() < 0 || object.geometry.y() < 0 || object.geometry.right() >= 800 || object.geometry.bottom() >= 480)
        { return compilationIssue(project,object,QStringLiteral("geometry"),QJsonArray{object.geometry.x(),object.geometry.y(),object.geometry.width(),object.geometry.height()},QStringLiteral("控件必须完整位于800×480画面内。"),error,issue); }
        if (object.type == QStringLiteral("text") || object.type == QStringLiteral("switch")) { continue; }
        if (object.type==QStringLiteral("sector")) //20261114-3 add: Only the native default-angle square path may reach independent background composition.
        {
            if (!prepareDeviceSector(project,object,prepared.objects,error,issue)) { return false; }
            ++prepared.staticSectorCount;
            continue;
        }
        if (object.type==QStringLiteral("ellipse")) //20261114-2 add: Keep curve recovery separate from the editor's Qt6 raster and complete native-source support.
        {
            if (!prepareDeviceEllipse(project,object,prepared.objects,error,issue)) { return false; }
            ++prepared.staticEllipseCount;
            continue;
        }
        if (object.type==QStringLiteral("polygon")) //20261114-1 add: Only the proven integer simple-path slice reaches the independent background raster.
        {
            if (!prepareDevicePolygon(project,object,prepared.objects,error,issue)) { return false; }
            ++prepared.staticPolygonCount;
            continue;
        }
        if (object.type == QStringLiteral("roundrect")) //20261113-16 add: Gate the sampled closed rectangle, including the independently observed line/fill switches.
        {
            if (object.graphic.radiusX != 10 || object.graphic.radiusY != 10)
            { return compilationIssue(project,object,QStringLiteral("graphic.radius"),QJsonArray{object.graphic.radiusX,object.graphic.radiusY},QStringLiteral("圆角矩形本切片只开放已核半径10/10。"),error,issue); }
            if (object.geometry.x() < 1 || object.geometry.y() < 1)
            { return compilationIssue(project,object,QStringLiteral("geometry.sceneBoundary"),QJsonArray{object.geometry.x(),object.geometry.y()},QStringLiteral("圆角矩形本切片保留至少1像素上/左边距。"),error,issue); }
            if (object.graphic.lineStyle != 0 && object.graphic.lineStyle != 1)
            { return compilationIssue(project,object,QStringLiteral("graphic.lineStyle"),object.graphic.lineStyle,QStringLiteral("圆角矩形只开放已核无线/实线。"),error,issue); }
            if (object.graphic.fillStyle != 0 && object.graphic.fillStyle != 1)
            { return compilationIssue(project,object,QStringLiteral("graphic.fillStyle"),object.graphic.fillStyle,QStringLiteral("圆角矩形只开放已核无填充/纯色填充。"),error,issue); }
            if (object.graphic.lineWidth != 1)
            { return compilationIssue(project,object,QStringLiteral("graphic.lineWidth"),object.graphic.lineWidth,QStringLiteral("圆角矩形本切片只开放宽度1。"),error,issue); }
            if (object.graphic.alpha != 255)
            { return compilationIssue(project,object,QStringLiteral("graphic.alpha"),object.graphic.alpha,QStringLiteral("圆角矩形本切片只开放不透明颜色。"),error,issue); }
            if (object.graphic.lineColor != QColor(Qt::black) || object.graphic.foreground != QColor(Qt::black) || object.graphic.background != QColor(Qt::white))
            { return compilationIssue(project,object,QStringLiteral("graphic.colors"),QJsonArray{object.graphic.lineColor.name(QColor::HexArgb),object.graphic.foreground.name(QColor::HexArgb),object.graphic.background.name(QColor::HexArgb)},QStringLiteral("圆角矩形本切片只开放已核黑边/黑前景/白背景草稿。"),error,issue); }
            if (object.geometry.width() < 100 || object.geometry.height() < 100)
            { return compilationIssue(project,object,QStringLiteral("geometry.sampledSize"),QJsonArray{object.geometry.width(),object.geometry.height()},QStringLiteral("圆角矩形本切片只开放已采样宽高至少100的范围；更小尺寸待核。"),error,issue); }
            if (object.geometry.x()+object.geometry.width() > 799 || object.geometry.y()+object.geometry.height() > 479)
            { return compilationIssue(project,object,QStringLiteral("geometry.strokeExtent"),QJsonArray{object.geometry.x()+object.geometry.width(),object.geometry.y()+object.geometry.height()},QStringLiteral("圆角矩形外框端点必须完整位于800×480画面内。"),error,issue); }
            Q_FOREACH (peer,prepared.objects)
            {
                // Include the closed outline's final right/bottom pixels, even when dormant line/fill fields are retained.
                if (peer.id != object.id && peer.geometry.adjusted(0,0,1,1).intersects(object.geometry.adjusted(0,0,1,1)))
                { return compilationIssue(project,object,QStringLiteral("geometry.overlap"),peer.id,QStringLiteral("圆角矩形与其它对象相交或边缘接触的设备合成尚未核定。"),error,issue); }
            }
            ++prepared.staticRoundrectCount;
            continue;
        }
        if (object.type == QStringLiteral("rect")) //20261113-13 add: Gate the sampled closed rectangle, including the independently observed line/fill switches.
        {
            if (object.graphic.lineStyle != 0 && object.graphic.lineStyle != 1)
            { return compilationIssue(project,object,QStringLiteral("graphic.lineStyle"),object.graphic.lineStyle,QStringLiteral("矩形只开放已核无线/实线。"),error,issue); }
            if (object.graphic.fillStyle != 0 && object.graphic.fillStyle != 1)
            { return compilationIssue(project,object,QStringLiteral("graphic.fillStyle"),object.graphic.fillStyle,QStringLiteral("矩形只开放已核无填充/纯色填充。"),error,issue); }
            if (object.graphic.lineWidth != 1)
            { return compilationIssue(project,object,QStringLiteral("graphic.lineWidth"),object.graphic.lineWidth,QStringLiteral("矩形本切片只开放宽度1。"),error,issue); }
            if (object.graphic.alpha != 255)
            { return compilationIssue(project,object,QStringLiteral("graphic.alpha"),object.graphic.alpha,QStringLiteral("矩形本切片只开放不透明颜色。"),error,issue); }
            if (object.graphic.lineColor != QColor(Qt::black) || object.graphic.foreground != QColor(Qt::black) || object.graphic.background != QColor(Qt::white))
            { return compilationIssue(project,object,QStringLiteral("graphic.colors"),QJsonArray{object.graphic.lineColor.name(QColor::HexArgb),object.graphic.foreground.name(QColor::HexArgb),object.graphic.background.name(QColor::HexArgb)},QStringLiteral("矩形本切片只开放已核黑边/黑前景/白背景草稿。"),error,issue); }
            if (object.geometry.width() < 100 || object.geometry.height() < 100)
            { return compilationIssue(project,object,QStringLiteral("geometry.sampledSize"),QJsonArray{object.geometry.width(),object.geometry.height()},QStringLiteral("矩形本切片只开放已采样宽高至少100的范围；更小尺寸待核。"),error,issue); }
            if (object.geometry.x()+object.geometry.width() > 799 || object.geometry.y()+object.geometry.height() > 479)
            { return compilationIssue(project,object,QStringLiteral("geometry.strokeExtent"),QJsonArray{object.geometry.x()+object.geometry.width(),object.geometry.y()+object.geometry.height()},QStringLiteral("矩形外框端点必须完整位于800×480画面内。"),error,issue); }
            Q_FOREACH (peer,prepared.objects)
            {
                // Include the closed outline's final right/bottom pixels, even when dormant line/fill fields are retained.
                if (peer.id != object.id && peer.geometry.adjusted(0,0,1,1).intersects(object.geometry.adjusted(0,0,1,1)))
                { return compilationIssue(project,object,QStringLiteral("geometry.overlap"),peer.id,QStringLiteral("矩形与其它对象相交或边缘接触的设备合成尚未核定。"),error,issue); }
            }
            ++prepared.staticRectCount;
            continue;
        }
        if (object.type == QStringLiteral("table")) //20261113-12 add: Admit only the canonical opaque static-table raster contract.
        {
            if (object.graphic.lineStyle != 0 && object.graphic.lineStyle != 1)
            { return compilationIssue(project,object,QStringLiteral("graphic.lineStyle"),object.graphic.lineStyle,QStringLiteral("表格只开放已核无线/实线；其它线型尚未核定。"),error,issue); }
            if (object.graphic.alpha != 255)
            { return compilationIssue(project,object,QStringLiteral("graphic.alpha"),object.graphic.alpha,QStringLiteral("表格本切片只开放不透明背景。"),error,issue); }
            if (object.graphic.lineColor != QColor(Qt::black))
            { return compilationIssue(project,object,QStringLiteral("graphic.lineColor"),object.graphic.lineColor.name(QColor::HexArgb),QStringLiteral("表格本切片只开放黑色内部线。"),error,issue); }
            if (object.graphic.frameColor != QColor(Qt::black))
            { return compilationIssue(project,object,QStringLiteral("graphic.frameColor"),object.graphic.frameColor.name(QColor::HexArgb),QStringLiteral("表格本切片只开放黑色外框。"),error,issue); }
            if (object.graphic.background != QColor(QStringLiteral("#e0e0e0")))
            { return compilationIssue(project,object,QStringLiteral("graphic.background"),object.graphic.background.name(QColor::HexArgb),QStringLiteral("表格本切片只开放已核灰色背景#e0e0e0。"),error,issue); }
            if (object.geometry.width() < 40 || object.geometry.height() < 40)
            { return compilationIssue(project,object,QStringLiteral("geometry.minimum"),QJsonArray{object.geometry.width(),object.geometry.height()},QStringLiteral("本切片采用原厂创建时已核的最小40×40表格尺寸。"),error,issue); }
            if (object.geometry.width() % object.graphic.columns != 0 || object.geometry.height() % object.graphic.rows != 0)
            { return compilationIssue(project,object,QStringLiteral("graphic.cellSize"),QJsonArray{object.graphic.rows,object.graphic.columns},QStringLiteral("表格只开放整数像素单元格；非整除尺寸尚未核定。"),error,issue); }
            if (object.geometry.x()+object.geometry.width() > 799 || object.geometry.y()+object.geometry.height() > 479)
            { return compilationIssue(project,object,QStringLiteral("geometry.strokeExtent"),QJsonArray{object.geometry.x()+object.geometry.width(),object.geometry.y()+object.geometry.height()},QStringLiteral("表格外框端点必须完整位于800×480画面内。"),error,issue); }
            Q_FOREACH (peer,prepared.objects)
            {
                // Include the right and bottom stroke pixels, which QRect's stored width otherwise excludes.
                if (peer.id != object.id && peer.geometry.adjusted(0,0,1,1).intersects(object.geometry.adjusted(0,0,1,1)))
                { return compilationIssue(project,object,QStringLiteral("geometry.overlap"),peer.id,QStringLiteral("表格与其它对象相交或边缘接触的设备合成尚未核定。"),error,issue); }
            }
            ++prepared.staticTableCount;
            continue;
        }
        if (object.type != QStringLiteral("polyline") && object.type != QStringLiteral("line") && object.type != QStringLiteral("freeline")) //20261113-15 change
        { return compilationIssue(project,object,QStringLiteral("type"),object.type,QStringLiteral("该类型尚未恢复独立设备编码。"),error,issue); }
        if (object.graphic.lineStyle != 1) { return compilationIssue(project,object,QStringLiteral("graphic.lineStyle"),object.graphic.lineStyle,QStringLiteral("本切片只开放已核实线。"),error,issue); }
        if (object.graphic.lineWidth != 1) { return compilationIssue(project,object,QStringLiteral("graphic.lineWidth"),object.graphic.lineWidth,QStringLiteral("本切片只开放宽度1。"),error,issue); }
        if (object.graphic.alpha != 255)
        { return compilationIssue(project,object,QStringLiteral("graphic.alpha"),object.graphic.alpha,QStringLiteral("本切片只开放不透明黑色。"),error,issue); }
        if (object.graphic.lineColor != QColor(Qt::black))
        { return compilationIssue(project,object,QStringLiteral("graphic.lineColor"),object.graphic.lineColor.name(QColor::HexArgb),QStringLiteral("本切片只开放不透明黑色。"),error,issue); }
        if (object.graphic.endArrow != 0)
        { return compilationIssue(project,object,QStringLiteral("graphic.endArrow"),object.graphic.endArrow,QStringLiteral("本切片只开放无箭头。"),error,issue); }
        if (object.graphic.endCap != 0)
        { return compilationIssue(project,object,QStringLiteral("graphic.endCap"),object.graphic.endCap,QStringLiteral("本切片只开放已核端点0。"),error,issue); }
        first = true;
        Q_FOREACH (point,object.graphic.points)
        {
            absolute = QPointF(object.geometry.x()+point.x()*object.geometry.width(),object.geometry.y()+point.y()*object.geometry.height());
            if (std::abs(absolute.x()-std::round(absolute.x())) > 1e-8 || std::abs(absolute.y()-std::round(absolute.y())) > 1e-8 || absolute.x() > 799 || absolute.y() > 479 || (object.type == QStringLiteral("polyline") && !first && absolute.x() != previous.x() && absolute.y() != previous.y())) //20261113-14 change: Oblique straight lines do not relax the orthogonal polyline contract.
            { return compilationIssue(project,object,QStringLiteral("graphic.points"),QJsonArray{absolute.x(),absolute.y()},QStringLiteral("只开放屏内整数像素端点；折线仍只支持水平或垂直线段。"),error,issue); }
            if (object.type != QStringLiteral("polyline") && !first && qRound(absolute.x()) == qRound(previous.x()) && qRound(absolute.y()) == qRound(previous.y())) //20261113-15 change: Both admitted two-point families require distinct device pixels.
            { return compilationIssue(project,object,QStringLiteral("graphic.points"),QJsonArray{absolute.x(),absolute.y()},QStringLiteral("直线取整后端点不能重合。"),error,issue); }
            if (object.type == QStringLiteral("freeline") && !first && qMax(qAbs(qRound(absolute.x())-qRound(previous.x())),qAbs(qRound(absolute.y())-qRound(previous.y()))) < 40) //20261113-15 add
            { return compilationIssue(project,object,QStringLiteral("graphic.sampledSpan"),QJsonArray{absolute.x(),absolute.y()},QStringLiteral("自由直线本切片保守开放主轴跨度至少40像素；小尺寸有效原厂路径未核。"),error,issue); }
            if (object.type == QStringLiteral("freeline") && (absolute.x() < 1 || absolute.y() < 1)) //20261113-15 add: The native boundary source retains a fractional item translation outside this identity-transform slice.
            { return compilationIssue(project,object,QStringLiteral("graphic.sceneBoundary"),QJsonArray{absolute.x(),absolute.y()},QStringLiteral("自由直线本切片仅开放端点X/Y至少1像素；贴上/左边界的原厂小数场景变换待恢复。"),error,issue); }
            previous = absolute; first = false;
        }
        Q_FOREACH (peer,prepared.objects)
        {
            if (peer.id != object.id && peer.geometry.intersects(object.geometry))
            { return compilationIssue(project,object,QStringLiteral("geometry.overlap"),peer.id,QStringLiteral("线段与其它对象重叠的设备合成尚未核定。"),error,issue); } //20261113-14 change
        }
        if (object.type == QStringLiteral("line")) { ++prepared.staticLineCount; } //20261113-15 change
        else if (object.type == QStringLiteral("freeline")) { ++prepared.staticFreelineCount; }
        else { ++prepared.staticPolylineCount; }
    }
    prepared.sourceSnapshot = project.toJson();
    *plan = prepared;
    if (issue != nullptr) { *issue = QJsonObject(); }
    if (error != nullptr) { error->clear(); }
    return true;
}
