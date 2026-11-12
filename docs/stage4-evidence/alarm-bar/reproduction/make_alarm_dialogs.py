"""Build detached alarm definition and display-only two-page property editors."""
from pathlib import Path
import runpy
# Reuse the source generator's documented-function helper without rewriting model output later.
model=runpy.run_path(str(Path(__file__).with_name('make_alarm_models.py')))
f=model['f'];put=model['put'];root=model['root']
put('src/alarm_bar_dialog.h','''#ifndef ALARM_BAR_DIALOG_H
#define ALARM_BAR_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QPainter;
class QLabel;
class HmiPermissionEditor;
void paintAlarmBar(QPainter *painter, const QRectF &rect, const AlarmBarProperties &properties);
//20261113-2 add: The preview is an editor sample, independent of alarm/PLC runtime.
class AlarmBarDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AlarmBarDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    AlarmBarProperties m_draft;
    bool m_accepted = false;
    QLabel *m_preview = nullptr;
    HmiPermissionEditor *m_visibility;
    QWidget *basicPage();
    void refresh();
};
#endif
''')
s='''#include "alarm_bar_dialog.h"
#include "flow_block_properties.h"
#include "hmi_property_widgets.h"
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QJsonArray>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

'''
flow=(root/'src/flow_block_dialog.cpp').read_text(encoding='utf-8')
a=flow.index('QBrush flowBrush(');b=flow.index('\n}\n}',a)+2
body=flow[a:b];body=body[body.index('{')+2:body.rfind('}')]
s+=f('alarmBarBrush','Provide the same explicit editor gradient approximation used by other display controls.','static QBrush alarmBarBrush(const QRectF &rect, int pattern, const QColor &front, const QColor &back)',body,'rect, pattern, front, back','Clipped editor fill brush.')
s+=f('paintAlarmBar','Draw only the native editor sample text with fill, text color, font size and a clipped border.','void paintAlarmBar(QPainter *painter, const QRectF &rect, const AlarmBarProperties &properties)','''    QFont font(QStringLiteral("SimSun"));
    font.setPixelSize(properties.fontSize); painter->save(); painter->setClipRect(rect); painter->setPen(Qt::NoPen); painter->setBrush(alarmBarBrush(rect, properties.pattern, QColor(properties.foregroundColor), QColor(properties.backgroundColor))); painter->drawRect(rect); painter->setFont(font); painter->setPen(QColor(properties.textColor)); painter->drawText(rect.adjusted(4, 0, -4, 0), Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, QStringLiteral("Sample Message")); painter->setPen(QPen(QColor(properties.frameColor), 1)); painter->setBrush(Qt::NoBrush); painter->drawRect(rect.adjusted(0.5, 0.5, -0.5, -0.5)); painter->restore();''','painter, rect, properties')
s+=f('AlarmBarDialog::basicPage','Bind four colors, thirteen patterns, measured speed and independent direction/order radio groups.','QWidget *AlarmBarDialog::basicPage()','''    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *fill = new QGroupBox(QStringLiteral("填充"), page);
    QGridLayout *grid = new QGridLayout(fill);
    QStringList labels = {QStringLiteral("边框色:"), QStringLiteral("前景色:"), QStringLiteral("背景色:"), QStringLiteral("文本色:")};
    QStringList names = {QStringLiteral("FrameColor"), QStringLiteral("ForegroundColor"), QStringLiteral("BackgroundColor"), QStringLiteral("TextColor")};
    QList<QString *> colors = {&m_draft.frameColor, &m_draft.foregroundColor, &m_draft.backgroundColor, &m_draft.textColor};
    QPushButton *button;
    QComboBox *pattern = new QComboBox(fill);
    QComboBox *font = new QComboBox(page);
    QSpinBox *speed = new QSpinBox(page);
    QGroupBox *group;
    QVBoxLayout *column;
    QRadioButton *first;
    QRadioButton *second;
    int *value;
    int index;
    for (index = 0; index < 4; ++index)
    {
        button = new QPushButton(fill); button->setObjectName(QStringLiteral("Alarm") + names.at(index)); button->setMinimumSize(90, 24); button->setStyleSheet(QStringLiteral("background:%1;").arg(*colors.at(index))); grid->addWidget(new QLabel(labels.at(index), fill), index, 0); grid->addWidget(button, index, 1);
        connect(button, &QPushButton::clicked, this, [this, button, colors, index]() { QColor color = QColorDialog::getColor(QColor(*colors.at(index)), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog); if (color.isValid()) { *colors.at(index) = color.name(); button->setStyleSheet(QStringLiteral("background:%1;").arg(color.name())); refresh(); } });
    }
    pattern->setObjectName(QStringLiteral("AlarmPattern")); pattern->addItems(FlowBlockProperties::patterns()); pattern->setCurrentIndex(m_draft.pattern); grid->addWidget(new QLabel(QStringLiteral("样式:"), fill), 4, 0); grid->addWidget(pattern, 4, 1); connect(pattern, &QComboBox::currentIndexChanged, this, [this](int v) { m_draft.pattern = v; refresh(); }); layout->addWidget(fill, 0, 0, 3, 1);
    speed->setObjectName(QStringLiteral("AlarmSpeed")); speed->setRange(0, 30); speed->setValue(m_draft.speed); layout->addWidget(new QLabel(QStringLiteral("移动速度 (x100ms):"), page), 0, 1); layout->addWidget(speed, 0, 2); connect(speed, &QSpinBox::valueChanged, this, [this](int v) { m_draft.speed = v; });
    for (index = 0; index < 2; ++index)
    {
        value = index == 0 ? &m_draft.direction : &m_draft.sortOrder; group = new QGroupBox(index == 0 ? QStringLiteral("方向") : QStringLiteral("报警排序"), page); column = new QVBoxLayout(group); first = new QRadioButton(index == 0 ? QStringLiteral("向左") : QStringLiteral("按时间顺序"), group); second = new QRadioButton(index == 0 ? QStringLiteral("向右") : QStringLiteral("按时间逆序"), group); first->setObjectName(index == 0 ? QStringLiteral("AlarmLeft") : QStringLiteral("AlarmTime")); second->setObjectName(index == 0 ? QStringLiteral("AlarmRight") : QStringLiteral("AlarmReverseTime")); first->setChecked(*value == 0); second->setChecked(*value == 1); column->addWidget(first); column->addWidget(second); layout->addWidget(group, index + 1, 1, 1, 2); connect(first, &QRadioButton::toggled, this, [value](bool checked) { if (checked) { *value = 0; } }); connect(second, &QRadioButton::toggled, this, [value](bool checked) { if (checked) { *value = 1; } });
    }
    font->setObjectName(QStringLiteral("AlarmFontSize")); font->addItems({QStringLiteral("12"), QStringLiteral("16"), QStringLiteral("24"), QStringLiteral("48"), QStringLiteral("64")}); font->setCurrentText(QString::number(m_draft.fontSize)); layout->addWidget(new QLabel(QStringLiteral("字体大小:"), page), 3, 0); layout->addWidget(font, 3, 1); connect(font, &QComboBox::currentTextChanged, this, [this](const QString &v) { m_draft.fontSize = v.toInt(); refresh(); }); layout->setRowStretch(4, 1); return page;''',returns='Detached basic-settings page.')
s+=f('AlarmBarDialog::AlarmBarDialog','Build basic and display-only permissions pages with independent transaction lifetime.','AlarmBarDialog::AlarmBarDialog(const HmiObject &object, const HmiProject &project, QWidget *parent)\n    : QDialog(parent), m_original(object), m_project(project), m_draft(object.alarmBar)','''    QGridLayout *layout = new QGridLayout(this);
    QTabWidget *tabs = new QTabWidget(this);
    QWidget *permission = new QWidget(this);
    QVBoxLayout *permissions = new QVBoxLayout(permission);
    QGroupBox *preview = new QGroupBox(QStringLiteral("预览"), this);
    QVBoxLayout *previewLayout = new QVBoxLayout(preview);
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    setObjectName(QStringLiteral("AlarmBarDialog")); setWindowTitle(QStringLiteral("动态报警条")); resize(790, 550); setStyleSheet(QStringLiteral("QLineEdit, QComboBox, QSpinBox {background:#caeace;} QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled {background:#b1ceed;}")); Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    tabs->setObjectName(QStringLiteral("AlarmTabs")); tabs->addTab(basicPage(), QStringLiteral("基本设置")); m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("AlarmVisible"), m_draft.visibility, false, permission); m_visibility->configureUnchosenBitState(m_draft.visibilityBitChosen); permissions->addWidget(m_visibility); permissions->addStretch(); tabs->addTab(permission, QStringLiteral("权限设置")); layout->addWidget(tabs, 0, 0); layout->setColumnStretch(0, 1); m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("AlarmPreview")); m_preview->setFixedSize(140, 120); previewLayout->addWidget(m_preview); layout->addWidget(preview, 0, 1, Qt::AlignTop);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("AlarmButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons, 1, 0, 1, 2); connect(buttons, &QDialogButtonBox::accepted, this, &AlarmBarDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("动态报警条帮助"), QStringLiteral("先在报警登录中配置数位地址报警或条件报警，再设置显示外观、滚动方向、排序与显现条件。")); }); refresh();''','object, project, parent')
s+=f('AlarmBarDialog::refresh','Render a fixed editor sample without simulating alarm generation or scan timing.','void AlarmBarDialog::refresh()','''    QImage image(280, 240, QImage::Format_ARGB32);
    QPainter painter;
    if (m_preview == nullptr) { return; } image.fill(QColor(QStringLiteral("#f0f0f0"))); painter.begin(&image); paintAlarmBar(&painter, QRectF(12, 75, 256, 80), m_draft); painter.end(); m_preview->setPixmap(QPixmap::fromImage(image).scaled(140, 120, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));''')
s+=f('AlarmBarDialog::object','Return a detached display object with no inherited operation or monitor carrier.','HmiObject AlarmBarDialog::object() const','''    HmiObject object = m_original;
    object.type = QStringLiteral("alarmbar"); object.alarmBar = m_draft; object.address.clear(); object.text.clear(); object.hasBitButtonProperties = false; object.hasStaticTextProperties = false; object.hasNumberDisplayProperties = false; object.hasImageDisplayProperties = false; object.alarmBar.visibility = m_visibility->properties(); object.alarmBar.visibilityBitChosen = m_visibility->bitStateChosen() || m_accepted; return object;''',returns='Complete detached display object.')
s+=f('AlarmBarDialog::accept','Check registration prerequisites and all active visibility carriers before acceptance.','void AlarmBarDialog::accept()','''    HmiObject candidate;
    QString error;
    m_accepted = true; candidate = object();
    if (!hasAlarmRegistrations(m_project)) { m_accepted = false; QMessageBox::warning(this, QStringLiteral("动态报警条"), QStringLiteral("位报警和条件报警都不存在")); return; }
    if (!candidate.alarmBar.validate(&error) || !candidate.alarmBar.validateAddresses(m_project.connections, &error)) { m_accepted = false; QMessageBox::warning(this, QStringLiteral("动态报警条属性未提交"), error); return; } QDialog::accept();''')
s=s.replace('#include "flow_block_properties.h"','#include "flow_block_properties.h"\n#include "alarm_registration_properties.h"')
put('src/alarm_bar_dialog.cpp',s)
put('src/alarm_registration_dialog.h','''#ifndef ALARM_REGISTRATION_DIALOG_H
#define ALARM_REGISTRATION_DIALOG_H
#include "project_model.h"
#include "alarm_registration_properties.h"
#include <QDialog>
class QComboBox;
class QLineEdit;
class QTableWidget;
class QSpinBox;
//20261113-2 add: Registration edits publish once and preserve row/language selection as editor state only.
class AlarmRegistrationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AlarmRegistrationDialog(const HmiConfigurationNode &node, const HmiProject &project, int family, QWidget *parent = nullptr);
    HmiConfigurationNode node() const;
protected:
    void accept() override;
private:
    HmiConfigurationNode m_original;
    HmiProject m_project;
    AlarmRegistrationProperties m_draft;
    bool m_initialValid = true;
    bool m_loading = false;
    QLineEdit *m_read;
    QComboBox *m_mode;
    QComboBox *m_count;
    QSpinBox *m_scan;
    QTableWidget *m_table;
    QComboBox *m_language;
    QLineEdit *m_message;
    QComboBox *m_dataType;
    QLineEdit *m_limit;
    QLineEdit *m_rowAddress;
    QLineEdit *m_rowType;
    QComboBox *m_jump;
    QComboBox *m_back;
    QString rowLabel(int row) const;
    void rebuild();
    void selectRow(int row);
    void saveRow();
    void changeMode(int mode);
};
#endif
''')
s='''#include "alarm_registration_dialog.h"
#include "hmi_property_widgets.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>

'''
s+=f('AlarmRegistrationDialog::AlarmRegistrationDialog','Build the observed registration table, common scan/type fields and selected multilingual row editor.','AlarmRegistrationDialog::AlarmRegistrationDialog(const HmiConfigurationNode &node, const HmiProject &project, int family, QWidget *parent)\n    : QDialog(parent), m_original(node), m_project(project)','''    QGridLayout *layout = new QGridLayout(this);
    QPushButton *picker = new QPushButton(QStringLiteral("…"), this);
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    HmiPage page;
    int index;
    m_draft.family = family; m_draft.readAddress = family == 0 ? QStringLiteral("LB0") : QStringLiteral("LW0"); if (!node.properties.isEmpty()) { m_initialValid = AlarmRegistrationProperties::fromJson(node.properties, &m_draft) && m_draft.family == family; }
    setObjectName(QStringLiteral("AlarmRegistrationDialog")); setWindowTitle(family == 0 ? QStringLiteral("数位地址报警") : QStringLiteral("条件报警")); resize(760, 610); setStyleSheet(QStringLiteral("QLineEdit, QComboBox, QSpinBox {background:#caeace;} QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled {background:#b1ceed;}")); Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    m_read = new QLineEdit(m_draft.readAddress, this); m_read->setObjectName(QStringLiteral("AlarmReadAddress")); m_read->setReadOnly(true); picker->setObjectName(QStringLiteral("AlarmReadPicker")); picker->setFixedWidth(28); layout->addWidget(new QLabel(QStringLiteral("读取地址:"), this), 0, 0); layout->addWidget(m_read, 0, 1); layout->addWidget(picker, 0, 2); connect(picker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_read, m_draft.family == 0 && m_draft.bitType == 0, int(qMax(1U, m_draft.readSpan()))); }); connect(m_read, &QLineEdit::textChanged, this, [this](const QString &text) { m_draft.readAddress = text; rebuild(); });
    m_mode = new QComboBox(this); m_mode->setObjectName(QStringLiteral("AlarmRegistrationMode")); m_mode->addItems({QStringLiteral("连续位"), QStringLiteral("连续字的位"), QStringLiteral("字的值")}); m_mode->setCurrentIndex(m_draft.bitType); m_mode->setVisible(family == 0); layout->addWidget(m_mode, 0, 3, 1, 2); connect(m_mode, &QComboBox::currentIndexChanged, this, &AlarmRegistrationDialog::changeMode);
    m_count = new QComboBox(this); m_count->setObjectName(QStringLiteral("AlarmRegistrationCount")); for (index = 0; index <= (family == 0 ? 511 : 31); ++index) { m_count->addItem(QString::number(index)); } m_count->setCurrentIndex(m_draft.count); layout->addWidget(new QLabel(family == 0 ? QStringLiteral("报警条数:") : QStringLiteral("大小:"), this), 1, 0); layout->addWidget(m_count, 1, 1); connect(m_count, &QComboBox::currentIndexChanged, this, [this](int value) { m_draft.resizeRows(value, m_project.languages); rebuild(); });
    m_scan = new QSpinBox(this); m_scan->setObjectName(QStringLiteral("AlarmScanSeconds")); m_scan->setRange(1, 3600); m_scan->setValue(m_draft.scanSeconds); layout->addWidget(new QLabel(QStringLiteral("扫描时间 (秒):"), this), 1, 3); layout->addWidget(m_scan, 1, 4); connect(m_scan, &QSpinBox::valueChanged, this, [this](int v) { m_draft.scanSeconds = v; });
    m_table = new QTableWidget(this); m_table->setObjectName(QStringLiteral("AlarmRegistrationTable")); m_table->setColumnCount(family == 0 ? 5 : 2); m_table->setHorizontalHeaderLabels(family == 0 ? QStringList{QStringLiteral("编号"), QStringLiteral("使用"), QStringLiteral("消息"), QStringLiteral("跳转画面"), QStringLiteral("跳回画面")} : QStringList{QStringLiteral("条件"), QStringLiteral("消息")}); m_table->setSelectionBehavior(QAbstractItemView::SelectRows); m_table->setSelectionMode(QAbstractItemView::SingleSelection); m_table->setEditTriggers(QAbstractItemView::NoEditTriggers); m_table->horizontalHeader()->setStretchLastSection(true); layout->addWidget(m_table, 2, 0, 1, 5); connect(m_table, &QTableWidget::currentCellChanged, this, [this](int row) { selectRow(row); }); connect(m_table, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *item) { int row = item->row(); if (!m_loading && row >= 0 && row < m_draft.rows.size() && item->column() == (m_draft.family == 0 ? 1 : 0)) { m_draft.rows[row].used = item->checkState() == Qt::Checked; } });
    m_language = new QComboBox(this); m_language->setObjectName(QStringLiteral("AlarmLanguage")); m_language->addItems(project.languages); m_language->setCurrentText(project.activeLanguage); layout->addWidget(new QLabel(QStringLiteral("语言:"), this), 3, 0); layout->addWidget(m_language, 3, 1); connect(m_language, &QComboBox::currentIndexChanged, this, [this]() { rebuild(); });
    m_rowAddress = new QLineEdit(this); m_rowAddress->setObjectName(QStringLiteral("AlarmRowAddress")); m_rowAddress->setReadOnly(true); layout->addWidget(new QLabel(QStringLiteral("地址/编号:"), this), 4, 0); layout->addWidget(m_rowAddress, 4, 1); m_rowType = new QLineEdit(this); m_rowType->setObjectName(QStringLiteral("AlarmRowType")); m_rowType->setReadOnly(true); layout->addWidget(m_rowType, 4, 3, 1, 2);
    m_message = new QLineEdit(this); m_message->setObjectName(QStringLiteral("AlarmMessage")); m_message->setMaxLength(32767); layout->addWidget(new QLabel(QStringLiteral("消息:"), this), 5, 0); layout->addWidget(m_message, 5, 1, 1, 4); connect(m_message, &QLineEdit::textEdited, this, [this]() { saveRow(); });
    m_dataType = new QComboBox(this); m_dataType->setObjectName(QStringLiteral("AlarmConditionDataType")); m_dataType->addItems(AlarmRegistrationProperties::dataTypes()); m_dataType->setCurrentIndex(m_draft.dataType); m_dataType->setVisible(family == 1); layout->addWidget(m_dataType, 6, 0, 1, 2); connect(m_dataType, &QComboBox::currentIndexChanged, this, [this](int v) { m_draft.dataType = v; selectRow(m_table->currentRow()); }); m_limit = new QLineEdit(this); m_limit->setObjectName(QStringLiteral("AlarmConditionLimit")); m_limit->setMaxLength(80); m_limit->setVisible(family == 1); layout->addWidget(m_limit, 6, 3, 1, 2); connect(m_limit, &QLineEdit::textEdited, this, [this]() { saveRow(); });
    m_jump = new QComboBox(this); m_back = new QComboBox(this); m_jump->setObjectName(QStringLiteral("AlarmJumpPage")); m_back->setObjectName(QStringLiteral("AlarmBackPage")); m_jump->addItem(QStringLiteral("未使用"), QString()); m_back->addItem(QStringLiteral("未使用"), QString()); Q_FOREACH (page, project.allPages()) { if (page.kind == QStringLiteral("normal")) { m_jump->addItem(page.name, page.id); m_back->addItem(page.name, page.id); } } m_jump->setVisible(family == 0); m_back->setVisible(family == 0); layout->addWidget(m_jump, 7, 0, 1, 2); layout->addWidget(m_back, 7, 3, 1, 2); connect(m_jump, &QComboBox::currentIndexChanged, this, [this]() { saveRow(); }); connect(m_back, &QComboBox::currentIndexChanged, this, [this]() { saveRow(); });
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("AlarmRegistrationButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons, 8, 0, 1, 5); connect(buttons, &QDialogButtonBox::accepted, this, &AlarmRegistrationDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, windowTitle(), QStringLiteral("配置读取范围、扫描时间、使用条目和各语言消息；数位报警可指定跳转与跳回画面，条件报警按低低/低/高/高高配置阈值。")); }); rebuild();''','node, project, family, parent')
s+=f('AlarmRegistrationDialog::rowLabel','Display native local offsets exactly and keep external offsets explicit rather than guessing family radix.','QString AlarmRegistrationDialog::rowLabel(int row) const','''    QRegularExpression expression(QStringLiteral("^(L[BW])([0-9]+)$"));
    QRegularExpressionMatch match = expression.match(m_draft.readAddress);
    int offset = m_draft.family == 1 ? (row / 4) * (m_draft.dataType == 0 || m_draft.dataType == 2 || m_draft.dataType == 4 ? 1 : 2) : m_draft.bitType == 1 ? row / 16 : m_draft.bitType == 2 ? 0 : row;
    QString address = match.hasMatch() ? match.captured(1) + QString::number(match.captured(2).toInt() + offset) : QStringLiteral("%1 + %2").arg(m_draft.readAddress).arg(offset);
    if (m_draft.family == 0 && m_draft.bitType == 1) { address += QStringLiteral(".%1").arg(row % 16); } if (m_draft.family == 0 && m_draft.bitType == 2) { address += QStringLiteral(" = %1").arg(row); } return address;''','row','Editor address/value label.')
s+=f('AlarmRegistrationDialog::rebuild','Refresh used flags and language-specific cells without writing selection changes into another row.','void AlarmRegistrationDialog::rebuild()','''    QSignalBlocker blocker(m_table);
    QTableWidgetItem *item;
    AlarmRegistrationRow row;
    QStringList levels = {QStringLiteral("低低"), QStringLiteral("低"), QStringLiteral("高"), QStringLiteral("高高")};
    int selected = qBound(0, m_table->currentRow(), qMax(0, m_draft.rows.size() - 1));
    int index;
    int column;
    m_loading = true; m_table->setRowCount(m_draft.rows.size());
    for (index = 0; index < m_draft.rows.size(); ++index)
    {
        row = m_draft.rows.at(index); for (column = 0; column < m_table->columnCount(); ++column) { m_table->setItem(index, column, new QTableWidgetItem); }
        if (m_draft.family == 0) { m_table->item(index, 0)->setText(QString::number(index)); m_table->item(index, 2)->setText(row.messages.value(m_language->currentText())); m_table->item(index, 3)->setText(m_jump->itemText(m_jump->findData(row.jumpPageId))); m_table->item(index, 4)->setText(m_back->itemText(m_back->findData(row.backPageId))); }
        else { m_table->item(index, 0)->setText(levels.at(index % 4)); m_table->item(index, 1)->setText(row.messages.value(m_language->currentText())); }
        item = m_table->item(index, m_draft.family == 0 ? 1 : 0); item->setFlags(item->flags() | Qt::ItemIsUserCheckable); item->setCheckState(row.used ? Qt::Checked : Qt::Unchecked);
    }
    m_table->setCurrentCell(m_draft.rows.isEmpty() ? -1 : selected, 0); m_loading = false; selectRow(m_table->currentRow());''')
s+=f('AlarmRegistrationDialog::selectRow','Restore one row without emitting message, threshold or navigation writes.','void AlarmRegistrationDialog::selectRow(int row)','''    bool valid = row >= 0 && row < m_draft.rows.size();
    QSignalBlocker jumpBlock(m_jump);
    QSignalBlocker backBlock(m_back);
    QStringList levels = {QStringLiteral("低低"), QStringLiteral("低"), QStringLiteral("高"), QStringLiteral("高高")};
    m_loading = true; m_message->setEnabled(valid); m_limit->setEnabled(valid); m_jump->setEnabled(valid); m_back->setEnabled(valid); m_rowAddress->setText(valid ? rowLabel(row) : QString()); m_rowType->setText(valid && m_draft.family == 1 ? levels.at(row % 4) : QString()); m_message->setText(valid ? m_draft.rows.at(row).messages.value(m_language->currentText()) : QString()); m_limit->setText(valid ? m_draft.rows.at(row).limit : QString()); m_jump->setCurrentIndex(valid ? m_jump->findData(m_draft.rows.at(row).jumpPageId) : 0); m_back->setCurrentIndex(valid ? m_back->findData(m_draft.rows.at(row).backPageId) : 0); m_loading = false;''','row')
s+=f('AlarmRegistrationDialog::saveRow','Write exactly the selected language and row while keeping disabled navigation drafts bounded.','void AlarmRegistrationDialog::saveRow()','''    int index = m_table->currentRow();
    AlarmRegistrationRow *row;
    QSignalBlocker blocker(m_table);
    if (m_loading || index < 0 || index >= m_draft.rows.size()) { return; } row = &m_draft.rows[index]; row->messages.insert(m_language->currentText(), m_message->text()); row->limit = m_limit->text(); if (m_draft.family == 0) { row->jumpPageId = m_jump->currentData().toString(); row->backPageId = m_back->currentData().toString(); m_table->item(index, 3)->setText(m_jump->currentText()); m_table->item(index, 4)->setText(m_back->currentText()); } m_table->item(index, m_draft.family == 0 ? 2 : 1)->setText(m_message->text());''')
s+=f('AlarmRegistrationDialog::changeMode','Require the observed confirmation and rebuild type-specific rows only after acceptance.','void AlarmRegistrationDialog::changeMode(int mode)','''    QSignalBlocker blocker(m_mode);
    int count = m_draft.count;
    if (mode == m_draft.bitType) { return; }
    if (QMessageBox::question(this, QStringLiteral("提示"), QStringLiteral("是否更改当前报警类型"), QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok) { m_mode->setCurrentIndex(m_draft.bitType); return; }
    m_draft.bitType = mode; m_draft.readAddress = mode == 0 ? QStringLiteral("LB0") : QStringLiteral("LW0"); m_draft.rows.clear(); m_draft.resizeRows(count, m_project.languages); m_read->setText(m_draft.readAddress); rebuild();''','mode')
s+=f('AlarmRegistrationDialog::node','Publish a typed active global node while preserving its existing stable identity.','HmiConfigurationNode AlarmRegistrationDialog::node() const','''    HmiConfigurationNode node = m_original;
    if (node.id.isEmpty()) { node.id = m_draft.family == 0 ? QStringLiteral("alarm-bit") : QStringLiteral("alarm-condition"); } node.kind = QStringLiteral("alarm"); node.name = m_draft.family == 0 ? QStringLiteral("数位地址报警") : QStringLiteral("条件报警"); node.ownerPageId.clear(); node.active = true; node.implemented = true; node.properties = m_draft.toJson(); return node;''',returns='Complete configuration node.')
s+=f('AlarmRegistrationDialog::accept','Validate every definition dependency before accepting a whole-project transaction.','void AlarmRegistrationDialog::accept()','''    QString error;
    if (!m_initialValid || !m_draft.validateContext(m_project, &error)) { QMessageBox::warning(this, QStringLiteral("报警登录未提交"), error.isEmpty() ? QStringLiteral("原报警登录字段无效。") : error); return; } QDialog::accept();''')
put('src/alarm_registration_dialog.cpp',s)
print('Created two transactional alarm dialogs and static editor renderer.')
