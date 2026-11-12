#include "main_window.h"
#include "hmi_canvas.h"
#include "recipe_dialog.h" //20261113-2 add
#include "data_collection_dialog.h" //20261112-1 add
#include "message_display_dialog.h"
#include "scale_dialog.h"
#include "qr_code_dialog.h" //20261112-0 add
#include "image_display_dialog.h" //20261112-0 add
#include "gif_display_dialog.h" //20261112-0 add
#include "chart_dialog.h" //20261112-1 add
#include "trend_dialog.h" //20261112-1 add
#include "history_trend_dialog.h" //20261112-1 add
#include "xy_curve_dialog.h" //20261113-1 add
#include "alarm_bar_dialog.h" //20261113-2 add
#include "alarm_registration_dialog.h"
#include "data_group_dialog.h" //20261113-1 add
#include "alarm_display_dialog.h" //20261113-2 add
#include "history_data_dialog.h" //20261113-2 add
#include "recipe_display_dialog.h" //20261113-2 add
#include "move_track_dialog.h" //20261112-0 add
#include "wheel_dialog.h" //20261112-0 add
#include "slider_dialog.h" //20261112-0 add
#include "flow_block_dialog.h" //20261112-0 add
#include "dropdown_dialog.h" //20261112-0 add
#include "bit_button_dialog.h" //20261112-0 add
#include "number_display_dialog.h" //20261112-0 add
#include <QMessageBox>
#include <QStatusBar>

/******************************************************************************
 * Function Name   : MainWindow::createStage4Control
 * Description     : Open a typed control dialog immediately and begin placement only after a valid accepted draft.
 * Input Parameters: type
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MainWindow::createStage4Control(const QString &type) //20261112-0 add
{
    HmiObject object;
    QDialog *dialog;
    QMap<QString, QString> controlMacros = m_project.macros; //20261112-0 add: Publish dropdown macro edits only with accepted object placement/edit.
    QMap<QString, QByteArray> pictures = m_project.pictures; //20261112-0 add: Child resource edits publish together with the object.
    QMap<QString, QByteArray> animations = m_project.animations; //20261112-0 add
    QString error;
    object.type = type;
    if (type == QStringLiteral("message"))
    {
        object.name = QStringLiteral("消息显示器"); object.text = QStringLiteral("0"); object.address = QStringLiteral("LW0"); object.geometry = QRect(0, 0, 200, 110);
        dialog = new MessageDisplayDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("qrcode")) //20261112-0 add
    {
        object.name = QStringLiteral("二维码"); object.address = QStringLiteral("LW0"); object.geometry = QRect(0, 0, 160, 110);
        dialog = new QrCodeDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("number")) //20261112-0 add
    {
        object.name = QStringLiteral("数值显示器"); object.geometry = QRect(0, 0, 120, 60); object.address = QStringLiteral("LW0"); object.hasNumberDisplayProperties = true;
        dialog = new NumberDisplayDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("chart")) //20261112-1 add
    { object.name = QStringLiteral("图表"); object.geometry = QRect(0, 0, 170, 130); dialog = new ChartDialog(object, currentEditorProject(), this); }
    else if (type == QStringLiteral("trend")) //20261112-1 add
    { object.name = QStringLiteral("趋势图"); object.geometry = QRect(0, 0, 200, 120); dialog = new TrendDialog(object, currentEditorProject(), this); }
    else if (type == QStringLiteral("historytrend")) //20261112-1 add
    {
        HmiConfigurationNode collection;
        bool found = false;
        Q_FOREACH (collection, currentEditorProject().configurationNodes) { if (collection.kind == QStringLiteral("dataCollection") && collection.active && collection.implemented) { found = true; break; } }
        if (!found) { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), QStringLiteral("尚未建立数据采集！")); return; }
        object.name = QStringLiteral("历史趋势图"); object.geometry = QRect(0, 0, 240, 160); dialog = new HistoryTrendDialog(object, currentEditorProject(), this); }
    else if (type == QStringLiteral("xycurve")) //20261113-1 add
    { object.name = QStringLiteral("X-Y曲线"); object.geometry = QRect(0, 0, 240, 160); dialog = new XyCurveDialog(object, currentEditorProject(), this); }
    else if (type == QStringLiteral("alarmbar")) //20261113-2 add
    {
        if (!hasAlarmRegistrations(currentEditorProject())) { QMessageBox::warning(this, QStringLiteral("动态报警条"), QStringLiteral("位报警和条件报警都不存在")); return; }
        object.name = QStringLiteral("动态报警条"); object.geometry = QRect(0, 0, 320, 60); dialog = new AlarmBarDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("datagroup")) //20261113-1 add
    { object.name = QStringLiteral("数据群组"); object.geometry = QRect(0, 0, 240, 160); dialog = new DataGroupDialog(object, currentEditorProject(), this); }
    else if (type == QStringLiteral("alarmdisplay") || type == QStringLiteral("historyalarm")) //20261113-2 change: The native table forms match, but their runtime data sources and object kinds remain distinct.
    {
        object.name = type == QStringLiteral("historyalarm") ? QStringLiteral("历史报警显示器") : QStringLiteral("报警显示器");
        if (!hasAlarmRegistrations(currentEditorProject())) { QMessageBox::warning(this, object.name, QStringLiteral("位报警和条件报警都不存在")); return; }
        object.geometry = QRect(0, 0, 320, 160); dialog = new AlarmDisplayDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("historydata")) //20261113-2 add: Match the measured collection prerequisite before constructing a detached editor.
    {
        HmiConfigurationNode collection;
        bool found = false;
        Q_FOREACH (collection, currentEditorProject().configurationNodes) { if (collection.kind == QStringLiteral("dataCollection") && collection.active && collection.implemented) { found = true; break; } }
        if (!found) { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), QStringLiteral("没有建立数据采集")); return; }
        object.name = QStringLiteral("历史数据显示器"); object.geometry = QRect(0, 0, 320, 180); dialog = new HistoryDataDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("recipe")) //20261113-2 add: The actual native prerequisite is a real global recipe, not a placeholder tree node.
    {
        HmiConfigurationNode definition;
        bool found = false;
        Q_FOREACH (definition, currentEditorProject().configurationNodes) { if (definition.kind == QStringLiteral("recipe") && definition.active && definition.implemented) { found = true; break; } }
        if (!found) { QMessageBox::warning(this, QStringLiteral("提示"), QStringLiteral("配方不存在，请先创建配方")); return; }
        object.name = QStringLiteral("配方显示器"); object.geometry = QRect(0, 0, 320, 180); dialog = new RecipeDisplayDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("track")) //20261112-0 add
    {
        object.name = QStringLiteral("移动轨迹"); object.geometry = QRect(0, 0, 200, 100); dialog = new MoveTrackDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("wheel")) //20261112-0 add
    {
        object.name = QStringLiteral("滚轮控件"); object.geometry = QRect(0, 0, 200, 100); dialog = new WheelDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("slider")) //20261112-0 add
    {
        object.name = QStringLiteral("滑动开关"); object.geometry = QRect(0, 0, 200, 54); dialog = new SliderDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("flow")) //20261112-0 add
    {
        object.name = QStringLiteral("流动块"); object.geometry = QRect(0, 0, 160, 60); dialog = new FlowBlockDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("dropdown")) //20261112-0 add
    {
        object.name = QStringLiteral("下拉框"); object.geometry = QRect(0, 0, 160, 80); dialog = new DropdownDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("multifunction")) //20261112-0 add
    {
        object.name = QStringLiteral("多功能按钮"); object.geometry = QRect(0, 0, 160, 80); dialog = new BitButtonDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("multistate")) //20261112-0 add
    {
        object.name = QStringLiteral("多态按钮"); object.geometry = QRect(0, 0, 120, 60); dialog = new BitButtonDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("screen")) //20261112-0 add
    {
        object.name = QStringLiteral("画面按钮"); object.geometry = QRect(0, 0, 120, 60); object.screenButton.screenPageId = m_project.startupPageId;
        dialog = new BitButtonDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("time")) //20261112-0 add
    {
        object.name = QStringLiteral("时间显示器"); object.geometry = QRect(0, 0, 120, 60);
        dialog = new NumberDisplayDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("ascii")) //20261112-0 add
    {
        object.name = QStringLiteral("ASCII显示器"); object.geometry = QRect(0, 0, 120, 60); object.address = QStringLiteral("LW0");
        dialog = new NumberDisplayDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("gif")) //20261112-0 add
    {
        object.name = QStringLiteral("GIF显示器"); object.geometry = QRect(0, 0, 100, 100);
        dialog = new GifDisplayDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("image")) //20261112-0 add
    {
        object.name = QStringLiteral("图片显示器"); object.hasImageDisplayProperties = true; object.geometry = QRect(0, 0, 200, 100);
        dialog = new ImageDisplayDialog(object, currentEditorProject(), this);
    }
    else if (type == QStringLiteral("scale"))
    {
        object.name = QStringLiteral("刻度"); object.geometry = QRect(0, 0, 160, 110);
        dialog = new ScaleDialog(object, this);
    }
    else { return; }
    if (dialog->exec() == QDialog::Accepted)
    {
        if (type == QStringLiteral("message")) { object = qobject_cast<MessageDisplayDialog *>(dialog)->object(); }
        else if (type == QStringLiteral("chart")) { object = qobject_cast<ChartDialog *>(dialog)->object(); } //20261112-1 add
        else if (type == QStringLiteral("trend")) { object = qobject_cast<TrendDialog *>(dialog)->object(); } //20261112-1 add
        else if (type == QStringLiteral("historytrend")) { object = qobject_cast<HistoryTrendDialog *>(dialog)->object(); } //20261112-1 add
        else if (type == QStringLiteral("xycurve")) { object = qobject_cast<XyCurveDialog *>(dialog)->object(); } //20261113-1 add
        else if (type == QStringLiteral("alarmbar")) { object = qobject_cast<AlarmBarDialog *>(dialog)->object(); } //20261113-2 add
        else if (type == QStringLiteral("datagroup")) { object = qobject_cast<DataGroupDialog *>(dialog)->object(); } //20261113-1 add
        else if (type == QStringLiteral("alarmdisplay") || type == QStringLiteral("historyalarm")) { object = qobject_cast<AlarmDisplayDialog *>(dialog)->object(); } //20261113-2 add
        else if (type == QStringLiteral("historydata")) { object = qobject_cast<HistoryDataDialog *>(dialog)->object(); } //20261113-2 add
        else if (type == QStringLiteral("recipe")) { object = qobject_cast<RecipeDisplayDialog *>(dialog)->object(); } //20261113-2 add
        else if (type == QStringLiteral("track")) { pictures = qobject_cast<MoveTrackDialog *>(dialog)->pictures(); object = qobject_cast<MoveTrackDialog *>(dialog)->object(); } //20261112-0 add
        else if (type == QStringLiteral("wheel")) { pictures = qobject_cast<WheelDialog *>(dialog)->pictures(); object = qobject_cast<WheelDialog *>(dialog)->object(); } //20261112-0 add
        else if (type == QStringLiteral("slider")) { controlMacros = qobject_cast<SliderDialog *>(dialog)->macros(); object = qobject_cast<SliderDialog *>(dialog)->object(); } //20261112-0 add
        else if (type == QStringLiteral("flow")) { object = qobject_cast<FlowBlockDialog *>(dialog)->object(); } //20261112-0 add
        else if (type == QStringLiteral("dropdown")) { controlMacros = qobject_cast<DropdownDialog *>(dialog)->macros(); object = qobject_cast<DropdownDialog *>(dialog)->object(); } //20261112-0 add
        else if (type == QStringLiteral("multifunction") || type == QStringLiteral("multistate") || type == QStringLiteral("screen")) { pictures = qobject_cast<BitButtonDialog *>(dialog)->pictures(); object = qobject_cast<BitButtonDialog *>(dialog)->object(); }
        else if (type == QStringLiteral("qrcode")) { object = qobject_cast<QrCodeDialog *>(dialog)->object(); }
        else if (type == QStringLiteral("gif")) { animations = qobject_cast<GifDisplayDialog *>(dialog)->animations(); object = qobject_cast<GifDisplayDialog *>(dialog)->object(); }
        else if (type == QStringLiteral("number")) { pictures = qobject_cast<NumberDisplayDialog *>(dialog)->pictures(); object = qobject_cast<NumberDisplayDialog *>(dialog)->object(); }
        else if (type == QStringLiteral("time") || type == QStringLiteral("ascii")) { pictures = qobject_cast<NumberDisplayDialog *>(dialog)->pictures(); object = qobject_cast<NumberDisplayDialog *>(dialog)->object(); }
        else if (type == QStringLiteral("image")) { pictures = qobject_cast<ImageDisplayDialog *>(dialog)->pictures(); object = qobject_cast<ImageDisplayDialog *>(dialog)->object(); }
        else { object = qobject_cast<ScaleDialog *>(dialog)->object(); } //20261112-0 change
        if (!applyControlDraft(object, controlMacros, pictures, false, &error, &animations)) //20261112-0 change
        { QMessageBox::warning(this, QStringLiteral("控件未创建"), error); delete dialog; return; }
        beginControlDraftPlacement(object, controlMacros, pictures, &animations); //20261112-0 change
        statusBar()->showMessage(QStringLiteral("在画布拖出%1；按 Esc 取消。").arg(object.name), 10000);
    }
    else { m_canvas->setActiveTool(QStringLiteral("select")); }
    delete dialog;
}

/******************************************************************************
 * Function Name   : MainWindow::editStage4Control
 * Description     : Apply a complete detached control draft as one project transaction after property acceptance.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MainWindow::editStage4Control() //20261112-0 add
{
    HmiObject object;
    QDialog *dialog;
    QMap<QString, QString> controlMacros = m_project.macros; //20261112-0 add: Publish dropdown macro edits only with accepted object placement/edit.
    QMap<QString, QByteArray> pictures = m_project.pictures; //20261112-0 add
    QMap<QString, QByteArray> animations = m_project.animations; //20261112-0 add
    QString error;
    bool selected;
    object = m_canvas->selectedObject(&selected);
    if (!selected) { return; }
    if (object.type == QStringLiteral("message")) { dialog = new MessageDisplayDialog(object, currentEditorProject(), this); }
    else if (object.type == QStringLiteral("chart")) { dialog = new ChartDialog(object, currentEditorProject(), this); } //20261112-1 add
    else if (object.type == QStringLiteral("trend")) { dialog = new TrendDialog(object, currentEditorProject(), this); } //20261112-1 add
    else if (object.type == QStringLiteral("historytrend")) { dialog = new HistoryTrendDialog(object, currentEditorProject(), this); } //20261112-1 add
    else if (object.type == QStringLiteral("xycurve")) { dialog = new XyCurveDialog(object, currentEditorProject(), this); } //20261113-1 add
    else if (object.type == QStringLiteral("alarmbar")) { dialog = new AlarmBarDialog(object, currentEditorProject(), this); } //20261113-2 add
    else if (object.type == QStringLiteral("datagroup")) { dialog = new DataGroupDialog(object, currentEditorProject(), this); } //20261113-1 add
    else if (object.type == QStringLiteral("alarmdisplay") || object.type == QStringLiteral("historyalarm")) { dialog = new AlarmDisplayDialog(object, currentEditorProject(), this); } //20261113-2 add
    else if (object.type == QStringLiteral("historydata")) { dialog = new HistoryDataDialog(object, currentEditorProject(), this); } //20261113-2 add
    else if (object.type == QStringLiteral("recipe")) { dialog = new RecipeDisplayDialog(object, currentEditorProject(), this); } //20261113-2 add
    else if (object.type == QStringLiteral("track")) { dialog = new MoveTrackDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("wheel")) { dialog = new WheelDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("slider")) { dialog = new SliderDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("flow")) { dialog = new FlowBlockDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("dropdown")) { dialog = new DropdownDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("multifunction") || object.type == QStringLiteral("multistate") || object.type == QStringLiteral("screen")) { dialog = new BitButtonDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("qrcode")) { dialog = new QrCodeDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("gif")) { dialog = new GifDisplayDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("number")) { dialog = new NumberDisplayDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("time") || object.type == QStringLiteral("ascii")) { dialog = new NumberDisplayDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("image")) { dialog = new ImageDisplayDialog(object, currentEditorProject(), this); } //20261112-0 add
    else if (object.type == QStringLiteral("scale")) { dialog = new ScaleDialog(object, this); }
    else { return; }
    if (dialog->exec() == QDialog::Accepted)
    {
        if (object.type == QStringLiteral("message")) { object = qobject_cast<MessageDisplayDialog *>(dialog)->object(); }
        else if (object.type == QStringLiteral("chart")) { object = qobject_cast<ChartDialog *>(dialog)->object(); } //20261112-1 add
        else if (object.type == QStringLiteral("trend")) { object = qobject_cast<TrendDialog *>(dialog)->object(); } //20261112-1 add
        else if (object.type == QStringLiteral("historytrend")) { object = qobject_cast<HistoryTrendDialog *>(dialog)->object(); } //20261112-1 add
        else if (object.type == QStringLiteral("xycurve")) { object = qobject_cast<XyCurveDialog *>(dialog)->object(); } //20261113-1 add
        else if (object.type == QStringLiteral("alarmbar")) { object = qobject_cast<AlarmBarDialog *>(dialog)->object(); } //20261113-2 add
        else if (object.type == QStringLiteral("datagroup")) { object = qobject_cast<DataGroupDialog *>(dialog)->object(); } //20261113-1 add
        else if (object.type == QStringLiteral("alarmdisplay") || object.type == QStringLiteral("historyalarm")) { object = qobject_cast<AlarmDisplayDialog *>(dialog)->object(); } //20261113-2 add
        else if (object.type == QStringLiteral("historydata")) { object = qobject_cast<HistoryDataDialog *>(dialog)->object(); } //20261113-2 add
        else if (object.type == QStringLiteral("recipe")) { object = qobject_cast<RecipeDisplayDialog *>(dialog)->object(); } //20261113-2 add
        else if (object.type == QStringLiteral("track")) { pictures = qobject_cast<MoveTrackDialog *>(dialog)->pictures(); object = qobject_cast<MoveTrackDialog *>(dialog)->object(); } //20261112-0 add
        else if (object.type == QStringLiteral("wheel")) { pictures = qobject_cast<WheelDialog *>(dialog)->pictures(); object = qobject_cast<WheelDialog *>(dialog)->object(); } //20261112-0 add
        else if (object.type == QStringLiteral("slider")) { controlMacros = qobject_cast<SliderDialog *>(dialog)->macros(); object = qobject_cast<SliderDialog *>(dialog)->object(); } //20261112-0 add
        else if (object.type == QStringLiteral("flow")) { object = qobject_cast<FlowBlockDialog *>(dialog)->object(); } //20261112-0 add
        else if (object.type == QStringLiteral("dropdown")) { controlMacros = qobject_cast<DropdownDialog *>(dialog)->macros(); object = qobject_cast<DropdownDialog *>(dialog)->object(); } //20261112-0 add
        else if (object.type == QStringLiteral("multifunction") || object.type == QStringLiteral("multistate") || object.type == QStringLiteral("screen")) { pictures = qobject_cast<BitButtonDialog *>(dialog)->pictures(); object = qobject_cast<BitButtonDialog *>(dialog)->object(); }
        else if (object.type == QStringLiteral("qrcode")) { object = qobject_cast<QrCodeDialog *>(dialog)->object(); }
        else if (object.type == QStringLiteral("gif")) { animations = qobject_cast<GifDisplayDialog *>(dialog)->animations(); object = qobject_cast<GifDisplayDialog *>(dialog)->object(); }
        else if (object.type == QStringLiteral("number")) { pictures = qobject_cast<NumberDisplayDialog *>(dialog)->pictures(); object = qobject_cast<NumberDisplayDialog *>(dialog)->object(); }
        else if (object.type == QStringLiteral("time") || object.type == QStringLiteral("ascii")) { pictures = qobject_cast<NumberDisplayDialog *>(dialog)->pictures(); object = qobject_cast<NumberDisplayDialog *>(dialog)->object(); }
        else if (object.type == QStringLiteral("image")) { pictures = qobject_cast<ImageDisplayDialog *>(dialog)->pictures(); object = qobject_cast<ImageDisplayDialog *>(dialog)->object(); }
        else { object = qobject_cast<ScaleDialog *>(dialog)->object(); } //20261112-0 change
        if (!applyControlDraft(object, controlMacros, pictures, true, &error, &animations)) { QMessageBox::warning(this, QStringLiteral("控件修改未提交"), error); } //20261112-0 change
    }
    delete dialog;
}

/******************************************************************************
 * Function Name   : MainWindow::configureDataCollection
 * Description     : Commit an accepted collection definition as one undoable project transaction.
 * Input Parameters: id
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MainWindow::configureDataCollection(const QString &id) //20261112-1 add
{
    HmiProject candidate = currentEditorProject();
    HmiConfigurationNode node;
    HmiConfigurationNode peer;
    QString error;
    int index;
    int existing = -1;
    DataCollectionDialog *dialog;
    if (!m_hasProject) { return; }
    for (index = 0; index < candidate.configurationNodes.size(); ++index)
    {
        peer = candidate.configurationNodes[index];
        if (!id.isEmpty() && peer.id == id && peer.kind == QStringLiteral("dataCollection")) { node = peer; existing = index; }
        if (id.isEmpty() && peer.kind == QStringLiteral("dataCollection") && peer.active && peer.implemented) { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), QStringLiteral("已经达到最大数据采集数量，请删除后再新建。")); return; }
    }
    if (!id.isEmpty() && existing < 0) { return; }
    if (id.isEmpty()) { node.kind = QStringLiteral("dataCollection"); node.name = QStringLiteral("数据采集1"); }
    dialog = new DataCollectionDialog(node, candidate, this);
    if (dialog->exec() != QDialog::Accepted) { delete dialog; return; }
    node = dialog->node(); delete dialog;
    if (!candidate.replaceDataCollection(node, &error)) { QMessageBox::warning(this, QStringLiteral("数据采集未提交"), error); return; } //20261113-2 change: Reconcile all historical-data columns before one undoable publish.
    if (!commitProjectDraft(candidate, &error)) { QMessageBox::warning(this, QStringLiteral("数据采集未提交"), error); }

}

/******************************************************************************
 * Function Name   : MainWindow::deleteDataCollection
 * Description     : Refuse referenced collector removal through the whole-project validator before publishing.
 * Input Parameters: id
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MainWindow::deleteDataCollection(const QString &id) //20261112-1 add
{
    HmiProject candidate = currentEditorProject();
    QString error;
    int index;
    for (index = 0; index < candidate.configurationNodes.size(); ++index)
    {
        if (candidate.configurationNodes[index].id == id && candidate.configurationNodes[index].kind == QStringLiteral("dataCollection"))
        {
            candidate.configurationNodes.removeAt(index);
            if (!commitProjectDraft(candidate, &error)) { QMessageBox::warning(this, QStringLiteral("数据采集正在使用"), error); }
            return;
        }
    }

}

/******************************************************************************
 * Function Name   : MainWindow::configureAlarmRegistration
 * Description     : Publish a complete global registration draft through one validated undoable project transaction.
 * Input Parameters: family
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void MainWindow::configureAlarmRegistration(int family) //20261113-2 add
{
    HmiProject candidate = currentEditorProject();
    HmiConfigurationNode node;
    AlarmRegistrationProperties properties;
    AlarmRegistrationDialog *dialog;
    QString error;
    int existing = -1;
    int index;
    if (!m_hasProject || family < 0 || family > 1) { return; }
    for (index = 0; index < candidate.configurationNodes.size(); ++index) { if (candidate.configurationNodes[index].kind == QStringLiteral("alarm") && candidate.configurationNodes[index].active && candidate.configurationNodes[index].implemented && AlarmRegistrationProperties::fromJson(candidate.configurationNodes[index].properties, &properties) && properties.family == family) { node = candidate.configurationNodes[index]; existing = index; break; } }
    dialog = new AlarmRegistrationDialog(node, candidate, family, this); if (dialog->exec() != QDialog::Accepted) { delete dialog; return; } if (existing < 0) { candidate.configurationNodes.append(dialog->node()); } else { candidate.configurationNodes[existing] = dialog->node(); } delete dialog; if (!commitProjectDraft(candidate, &error)) { QMessageBox::warning(this, QStringLiteral("报警登录未提交"), error); }

}

/******************************************************************************
 * Function Name   : MainWindow::deleteAlarmRegistration
 * Description     : Refuse clearing the last required registration or a referenced page by validating every page before publish.
 * Input Parameters: family
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void MainWindow::deleteAlarmRegistration(int family) //20261113-2 add
{
    HmiProject candidate = currentEditorProject();
    AlarmRegistrationProperties properties;
    QString error;
    int index;
    for (index = 0; index < candidate.configurationNodes.size(); ++index) { if (candidate.configurationNodes[index].kind == QStringLiteral("alarm") && candidate.configurationNodes[index].active && candidate.configurationNodes[index].implemented && AlarmRegistrationProperties::fromJson(candidate.configurationNodes[index].properties, &properties) && properties.family == family) { candidate.configurationNodes.removeAt(index); if (!commitProjectDraft(candidate, &error)) { QMessageBox::warning(this, QStringLiteral("报警登录正在使用"), error); } return; } }

}

/******************************************************************************
 * Function Name   : MainWindow::configureRecipe
 * Description     : Commit one accepted recipe definition as a complete undoable project transaction.
 * Input Parameters: id
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void MainWindow::configureRecipe(const QString &id) //20261113-2 add
{
    HmiProject candidate = currentEditorProject();
    HmiConfigurationNode node;
    HmiConfigurationNode peer;
    RecipeDialog *dialog;
    QString error;
    int index;
    int existing = -1;
    if (!m_hasProject) { return; }
    for (index = 0; index < candidate.configurationNodes.size(); ++index) { peer = candidate.configurationNodes[index]; if (!id.isEmpty() && peer.id == id && peer.kind == QStringLiteral("recipe")) { node = peer; existing = index; } if (id.isEmpty() && peer.kind == QStringLiteral("recipe") && peer.active && peer.implemented) { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), QStringLiteral("已经达到最大配方数量，请删除后再新建。")); return; } }
    if (!id.isEmpty() && existing < 0) { return; } if (id.isEmpty()) { node.kind = QStringLiteral("recipe"); }
    dialog = new RecipeDialog(node, candidate, this); if (dialog->exec() != QDialog::Accepted) { delete dialog; return; }
    if (existing < 0) { candidate.configurationNodes.append(dialog->node()); } else { candidate.configurationNodes[existing] = dialog->node(); } delete dialog;
    if (!commitProjectDraft(candidate, &error)) { QMessageBox::warning(this, QStringLiteral("配方未提交"), error); }
}

/******************************************************************************
 * Function Name   : MainWindow::deleteRecipe
 * Description     : Refuse removal while any active or inactive page consumes the stable recipe definition.
 * Input Parameters: id
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void MainWindow::deleteRecipe(const QString &id) //20261113-2 add
{
    HmiProject candidate = currentEditorProject();
    QString error;
    int index;
    for (index = 0; index < candidate.configurationNodes.size(); ++index) { if (candidate.configurationNodes[index].id == id && candidate.configurationNodes[index].kind == QStringLiteral("recipe")) { candidate.configurationNodes.removeAt(index); if (!commitProjectDraft(candidate, &error)) { QMessageBox::warning(this, QStringLiteral("配方正在使用"), error); } return; } }
}
