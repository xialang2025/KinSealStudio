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
        if (object.type != QStringLiteral("polyline") && object.type != QStringLiteral("line")) //20261113-14 change
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
            if (object.type == QStringLiteral("line") && !first && qRound(absolute.x()) == qRound(previous.x()) && qRound(absolute.y()) == qRound(previous.y())) //20261113-14 add: Distinct normalized doubles may collapse to one device pixel.
            { return compilationIssue(project,object,QStringLiteral("graphic.points"),QJsonArray{absolute.x(),absolute.y()},QStringLiteral("直线取整后端点不能重合。"),error,issue); }
            previous = absolute; first = false;
        }
        Q_FOREACH (peer,prepared.objects)
        {
            if (peer.id != object.id && peer.geometry.intersects(object.geometry))
            { return compilationIssue(project,object,QStringLiteral("geometry.overlap"),peer.id,QStringLiteral("线段与其它对象重叠的设备合成尚未核定。"),error,issue); } //20261113-14 change
        }
        if (object.type == QStringLiteral("line")) { ++prepared.staticLineCount; } //20261113-14 change
        else { ++prepared.staticPolylineCount; }
    }
    prepared.sourceSnapshot = project.toJson();
    *plan = prepared;
    if (issue != nullptr) { *issue = QJsonObject(); }
    if (error != nullptr) { error->clear(); }
    return true;
}
