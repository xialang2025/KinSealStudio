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
        if (object.type != QStringLiteral("polyline"))
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
            if (std::abs(absolute.x()-std::round(absolute.x())) > 1e-8 || std::abs(absolute.y()-std::round(absolute.y())) > 1e-8 || absolute.x() > 799 || absolute.y() > 479 || (!first && absolute.x() != previous.x() && absolute.y() != previous.y()))
            { return compilationIssue(project,object,QStringLiteral("graphic.points"),QJsonArray{absolute.x(),absolute.y()},QStringLiteral("仅开放整数像素、屏内、水平或垂直线段；斜线/小数坐标尚未核定。"),error,issue); }
            previous = absolute; first = false;
        }
        Q_FOREACH (peer,prepared.objects)
        {
            if (peer.id != object.id && peer.geometry.intersects(object.geometry))
            { return compilationIssue(project,object,QStringLiteral("geometry.overlap"),peer.id,QStringLiteral("折线与其它对象重叠的设备合成尚未核定。"),error,issue); }
        }
        ++prepared.staticPolylineCount;
    }
    prepared.sourceSnapshot = project.toJson();
    *plan = prepared;
    if (issue != nullptr) { *issue = QJsonObject(); }
    if (error != nullptr) { error->clear(); }
    return true;
}
