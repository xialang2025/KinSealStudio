"""Build C28 native group structure and a detached multilingual table preview."""
from pathlib import Path
import ast
root=Path(__file__).resolve().parent/'worktree'
tree=ast.parse(Path(__file__).with_name('make_alarm_models.py').read_text(encoding='utf-8'));ns={};exec(compile(ast.Module(body=[n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='f'],type_ignores=[]),'<function-helper>','exec'),ns);f=ns['f']
def put(path,text):(root/path).write_text(text,encoding='utf-8',newline='\n')
put('src/alarm_display_dialog.h','''#ifndef ALARM_DISPLAY_DIALOG_H
#define ALARM_DISPLAY_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QPainter;
class QGridLayout;
class QComboBox;
class QLineEdit;
class QLabel;
class QPushButton;
class HmiPermissionEditor;
void paintAlarmDisplay(QPainter *painter, const QRectF &rect, const AlarmDisplayProperties &properties, const QString &language);
//20261113-2 add: Table samples are editor illustrations; registration and firmware runtime remain separate.
class AlarmDisplayDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AlarmDisplayDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    AlarmDisplayProperties m_draft;
    QString m_language;
    bool m_accepted = false;
    QLabel *m_preview = nullptr;
    QComboBox *m_languages;
    QLineEdit *m_message;
    QLineEdit *m_date;
    QLineEdit *m_time;
    HmiPermissionEditor *m_visibility;
    QWidget *basicPage();
    QString *color(int index);
    void addColor(QGridLayout *layout, int row, const QString &label, int index);
    void changeLanguage(const QString &language);
    void saveCaptions();
    void refresh();
};
#endif
''')
s='''#include "alarm_display_dialog.h"
#include "hmi_property_widgets.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

'''
s+=f('paintAlarmDisplay','Draw a bounded blank-row editor table with fixed column order and localized captions, without inventing alarm history.','void paintAlarmDisplay(QPainter *painter, const QRectF &rect, const AlarmDisplayProperties &properties, const QString &language)','''    AlarmDisplayCaptions captions = properties.captionsFor(language);
    QStringList titles = {QStringLiteral("序号")};
    QFont font(QStringLiteral("SimSun"));
    QRectF cell;
    qreal rowHeight = rect.height() / (properties.rows + 1);
    qreal width;
    qreal x;
    int column;
    int row;
    if (properties.showDate) { titles.append(captions.date); } if (properties.showTime) { titles.append(captions.time); } titles.append(captions.message); font.setPixelSize(properties.fontSize); painter->save(); painter->setClipRect(rect); painter->fillRect(rect, QColor(properties.backgroundColor)); painter->setFont(font);
    for (row = 0; row <= properties.rows; ++row)
    {
        x = rect.left();
        for (column = 0; column < titles.size(); ++column)
        {
            width = column == 0 ? rect.width() * 0.15 : rect.width() * 0.85 / (titles.size() - 1); cell = QRectF(x, rect.top() + row * rowHeight, width, rowHeight); if (row == 0 || column == 0) { painter->fillRect(cell, QColor(properties.titleBackground)); } painter->setPen(QPen(QColor(properties.frameColor), 1)); painter->setBrush(Qt::NoBrush); painter->drawRect(cell); painter->save(); painter->setClipRect(cell.adjusted(1, 1, -1, -1), Qt::IntersectClip); painter->setPen(QColor(row == 0 ? properties.titleColor : properties.textColor)); if (row == 0) { painter->drawText(cell.adjusted(2, 0, -2, 0), Qt::AlignCenter | Qt::TextSingleLine, titles.at(column)); } else if (column == 0) { painter->drawText(cell, Qt::AlignCenter, QString::number(row)); } painter->restore(); x += width;
        }
    }
    painter->restore();''','painter, rect, properties, language')
s+=f('AlarmDisplayDialog::color','Resolve the measured five common color fields independently of caption language selection.','QString *AlarmDisplayDialog::color(int index)','''    if (index == 0) { return &m_draft.titleBackground; } if (index == 1) { return &m_draft.titleColor; } if (index == 2) { return &m_draft.frameColor; } if (index == 3) { return &m_draft.backgroundColor; } return &m_draft.textColor;''','index','Selected common color draft.')
s+=f('AlarmDisplayDialog::addColor','Bind one color swatch to the shared color dialog and local draft only.','void AlarmDisplayDialog::addColor(QGridLayout *layout, int row, const QString &label, int index)','''    QPushButton *button = new QPushButton(this);
    button->setObjectName(QStringLiteral("AlarmDisplayColor%1").arg(index)); button->setFixedSize(140, 24); button->setStyleSheet(QStringLiteral("background:%1;").arg(*color(index))); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(button, row, 1, Qt::AlignLeft); connect(button, &QPushButton::clicked, this, [this, button, index]() { QColor selected = QColorDialog::getColor(QColor(*color(index)), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog); if (selected.isValid()) { *color(index) = selected.name(); button->setStyleSheet(QStringLiteral("background:%1;").arg(selected.name())); refresh(); } });''','layout, row, label, index')
s+=f('AlarmDisplayDialog::basicPage','Match native title, frame, text, date and time groups with common styles and independent language captions.','QWidget *AlarmDisplayDialog::basicPage()','''    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *title = new QGroupBox(QStringLiteral("标题填充"), page);
    QGroupBox *frame = new QGroupBox(QStringLiteral("边框填充"), page);
    QGroupBox *text = new QGroupBox(QStringLiteral("文本属性"), page);
    QGroupBox *date = new QGroupBox(page);
    QGroupBox *time = new QGroupBox(page);
    QGridLayout *grid = new QGridLayout(title);
    QComboBox *font = new QComboBox(title);
    QComboBox *size = new QComboBox(title);
    QSpinBox *rows = new QSpinBox(text);
    QCheckBox *dateCheck = new QCheckBox(QStringLiteral("日期显示"), date);
    QCheckBox *timeCheck = new QCheckBox(QStringLiteral("时间显示"), time);
    QLabel *dateLabel = new QLabel(QStringLiteral("日期:"), date);
    QLabel *timeLabel = new QLabel(QStringLiteral("时间:"), time);
    AlarmDisplayCaptions captions = m_draft.captionsFor(m_language);
    int value;
    layout->setContentsMargins(12, 14, 30, 30); layout->setHorizontalSpacing(10); layout->setVerticalSpacing(10); layout->setColumnStretch(0, 1); layout->setColumnStretch(1, 1); m_languages = new QComboBox(title); m_languages->setObjectName(QStringLiteral("AlarmDisplayLanguage")); m_languages->addItems(m_project.languages); m_languages->setCurrentText(m_language); grid->addWidget(new QLabel(QStringLiteral("语言:"), title), 0, 0); grid->addWidget(m_languages, 0, 1); addColor(grid, 1, QStringLiteral("背景色:"), 0); addColor(grid, 2, QStringLiteral("文字颜色:"), 1); font->setObjectName(QStringLiteral("AlarmDisplayFont")); font->addItem(QStringLiteral("宋体")); grid->addWidget(new QLabel(QStringLiteral("字体:"), title), 3, 0); grid->addWidget(font, 3, 1); size->setObjectName(QStringLiteral("AlarmDisplayFontSize")); Q_FOREACH (value, AlarmDisplayProperties::fontSizes()) { size->addItem(QString::number(value)); } size->setCurrentText(QString::number(m_draft.fontSize)); grid->addWidget(new QLabel(QStringLiteral("大小:"), title), 4, 0); grid->addWidget(size, 4, 1); m_message = new QLineEdit(captions.message, title); m_message->setObjectName(QStringLiteral("AlarmDisplayMessageCaption")); m_message->setMaxLength(32767); grid->addWidget(new QLabel(QStringLiteral("消息:"), title), 5, 0); grid->addWidget(m_message, 5, 1); layout->addWidget(title, 0, 0, 2, 1);
    grid = new QGridLayout(frame); addColor(grid, 0, QStringLiteral("边框色:"), 2); addColor(grid, 1, QStringLiteral("背景色:"), 3); layout->addWidget(frame, 0, 1); grid = new QGridLayout(text); addColor(grid, 0, QStringLiteral("文本色:"), 4); rows->setObjectName(QStringLiteral("AlarmDisplayRows")); rows->setRange(1, 20); rows->setValue(m_draft.rows); rows->setFixedWidth(120); grid->addWidget(new QLabel(QStringLiteral("行数:"), text), 1, 0); grid->addWidget(rows, 1, 1); layout->addWidget(text, 1, 1);
    m_date = new QLineEdit(captions.date, date); m_date->setObjectName(QStringLiteral("AlarmDisplayDateCaption")); m_date->setMaxLength(32767); dateCheck->setObjectName(QStringLiteral("AlarmDisplayShowDate")); dateCheck->setChecked(m_draft.showDate); dateLabel->setEnabled(m_draft.showDate); m_date->setEnabled(m_draft.showDate); grid = new QGridLayout(date); grid->addWidget(dateCheck, 0, 0, 1, 2); grid->addWidget(dateLabel, 1, 0); grid->addWidget(m_date, 1, 1); layout->addWidget(date, 2, 0);
    m_time = new QLineEdit(captions.time, time); m_time->setObjectName(QStringLiteral("AlarmDisplayTimeCaption")); m_time->setMaxLength(32767); timeCheck->setObjectName(QStringLiteral("AlarmDisplayShowTime")); timeCheck->setChecked(m_draft.showTime); timeLabel->setEnabled(m_draft.showTime); m_time->setEnabled(m_draft.showTime); grid = new QGridLayout(time); grid->addWidget(timeCheck, 0, 0, 1, 2); grid->addWidget(timeLabel, 1, 0); grid->addWidget(m_time, 1, 1); layout->addWidget(time, 2, 1); layout->setRowStretch(3, 1);
    connect(m_languages, &QComboBox::currentTextChanged, this, &AlarmDisplayDialog::changeLanguage); connect(m_message, &QLineEdit::textChanged, this, [this]() { saveCaptions(); }); connect(m_date, &QLineEdit::textChanged, this, [this]() { saveCaptions(); }); connect(m_time, &QLineEdit::textChanged, this, [this]() { saveCaptions(); }); connect(size, &QComboBox::currentTextChanged, this, [this](const QString &value) { m_draft.fontSize = value.toInt(); refresh(); }); connect(rows, &QSpinBox::valueChanged, this, [this](int value) { m_draft.rows = value; refresh(); }); connect(dateCheck, &QCheckBox::toggled, this, [this, dateLabel](bool checked) { m_draft.showDate = checked; m_date->setEnabled(checked); dateLabel->setEnabled(checked); refresh(); }); connect(timeCheck, &QCheckBox::toggled, this, [this, timeLabel](bool checked) { m_draft.showTime = checked; m_time->setEnabled(checked); timeLabel->setEnabled(checked); refresh(); }); return page;''',returns='Detached native basic page.')
s+=f('AlarmDisplayDialog::AlarmDisplayDialog','Build the measured two-page table editor with one common style and per-language column captions.','AlarmDisplayDialog::AlarmDisplayDialog(const HmiObject &object, const HmiProject &project, QWidget *parent)\n    : QDialog(parent), m_original(object), m_project(project), m_draft(object.alarmDisplay), m_language(project.activeLanguage)','''    QGridLayout *layout = new QGridLayout(this);
    QTabWidget *tabs = new QTabWidget(this);
    QWidget *permission = new QWidget(this);
    QVBoxLayout *permissions = new QVBoxLayout(permission);
    QGroupBox *preview = new QGroupBox(QStringLiteral("预览"), this);
    QVBoxLayout *previewLayout = new QVBoxLayout(preview);
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    setObjectName(QStringLiteral("AlarmDisplayDialog")); setWindowTitle(QStringLiteral("报警显示器")); resize(834, 565); m_draft.setLanguages(project.languages); setFont(QFont(QStringLiteral("SimSun"), 9)); setStyleSheet(QStringLiteral("QLineEdit, QComboBox, QSpinBox {background:#caeace;} QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled {background:#b1ceed;}")); Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    tabs->setObjectName(QStringLiteral("AlarmDisplayTabs")); tabs->addTab(basicPage(), QStringLiteral("基本设置")); m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("AlarmDisplayVisible"), m_draft.visibility, false, permission); m_visibility->configureUnchosenBitState(m_draft.visibilityBitChosen); permissions->addWidget(m_visibility); permissions->addStretch(); tabs->addTab(permission, QStringLiteral("权限设置")); layout->addWidget(tabs, 0, 0); layout->setColumnStretch(0, 1); m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("AlarmDisplayPreview")); m_preview->setFixedSize(130, 130); previewLayout->addWidget(m_preview); layout->addWidget(preview, 0, 1, Qt::AlignTop);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("AlarmDisplayButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons, 1, 0, 1, 2); connect(buttons, &QDialogButtonBox::accepted, this, &AlarmDisplayDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("报警显示器帮助"), QStringLiteral("先配置报警登录，再设置表格外观、各语言列名、显示行数和日期/时间列。预览显示空白示意行。")); }); refresh();''','object, project, parent')
s+=f('AlarmDisplayDialog::saveCaptions','Write the current language only; no style or hidden-column flags are localized.','void AlarmDisplayDialog::saveCaptions()','''    AlarmDisplayCaptions captions;
    captions.message = m_message->text(); captions.date = m_date->text(); captions.time = m_time->text(); m_draft.captions.insert(m_language, captions); refresh();''')
s+=f('AlarmDisplayDialog::changeLanguage','Switch three captions under signal blockers so values cannot leak between project languages.','void AlarmDisplayDialog::changeLanguage(const QString &language)','''    QSignalBlocker messageBlock(m_message);
    QSignalBlocker dateBlock(m_date);
    QSignalBlocker timeBlock(m_time);
    AlarmDisplayCaptions captions = m_draft.captionsFor(language);
    m_language = language; m_message->setText(captions.message); m_date->setText(captions.date); m_time->setText(captions.time); refresh();''','language')
s+=f('AlarmDisplayDialog::refresh','Render one bounded illustrative table without reading alarm data or fabricating timestamps.','void AlarmDisplayDialog::refresh()','''    QImage image(650, 650, QImage::Format_ARGB32);
    QPainter painter;
    if (m_preview == nullptr) { return; } image.fill(QColor(QStringLiteral("#f0f0f0"))); painter.begin(&image); paintAlarmDisplay(&painter, QRectF(50, 190, 550, 280), m_draft, m_language); painter.end(); m_preview->setPixmap(QPixmap::fromImage(image).scaled(130, 130, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));''')
s+=f('AlarmDisplayDialog::object','Return a complete detached table display without a generic operation or text carrier.','HmiObject AlarmDisplayDialog::object() const','''    HmiObject object = m_original;
    object.type = QStringLiteral("alarmdisplay"); object.alarmDisplay = m_draft; object.address.clear(); object.text.clear(); object.hasBitButtonProperties = false; object.hasStaticTextProperties = false; object.hasNumberDisplayProperties = false; object.hasImageDisplayProperties = false; object.alarmDisplay.visibility = m_visibility->properties(); object.alarmDisplay.visibilityBitChosen = m_visibility->bitStateChosen() || m_accepted; return object;''',returns='Complete detached alarm table.')
s+=f('AlarmDisplayDialog::accept','Require nonempty registrations and valid display/visibility fields before one transactional publish.','void AlarmDisplayDialog::accept()','''    HmiObject candidate;
    QString error;
    m_accepted = true; candidate = object(); if (!hasAlarmRegistrations(m_project)) { m_accepted = false; QMessageBox::warning(this, QStringLiteral("报警显示器"), QStringLiteral("位报警和条件报警都不存在")); return; } if (!candidate.alarmDisplay.validate(&error) || !candidate.alarmDisplay.validateAddresses(m_project.connections, &error)) { m_accepted = false; QMessageBox::warning(this, QStringLiteral("报警显示器属性未提交"), error); return; } QDialog::accept();''')
put('src/alarm_display_dialog.cpp',s)
print('Created C28 native group layout, global style and independent multilingual captions.')
