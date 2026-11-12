"""Build the measured three-page C30 editor and bounded shared table renderer."""
from make_history_data_model import root, put, f
put('src/history_data_dialog.h', '''#ifndef HISTORY_DATA_DIALOG_H
#define HISTORY_DATA_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QPainter;
class QComboBox;
class QGridLayout;
class QLineEdit;
class QTableWidget;
class QLabel;
class HmiPermissionEditor;
void paintHistoryData(QPainter *painter, const QRectF &rect, const HistoryDataProperties &properties, const QString &language);
//20261113-2 add: Detached table edits do not create or query device history records.
class HistoryDataDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HistoryDataDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    HistoryDataProperties m_draft;
    DataCollectionProperties m_collection;
    QString m_language;
    bool m_collectionFound = false;
    bool m_accepted = false;
    QLabel *m_preview = nullptr;
    QComboBox *m_languages = nullptr;
    QComboBox *m_dataLanguages = nullptr;
    QLineEdit *m_date = nullptr;
    QLineEdit *m_time = nullptr;
    QLineEdit *m_name = nullptr;
    QTableWidget *m_table = nullptr;
    HmiPermissionEditor *m_visibility;
    QWidget *basicPage();
    QWidget *dataPage();
    void addColor(QGridLayout *grid, int row, const QString &label, int index);
    QString *color(int index);
    void changeLanguage(const QString &language);
    void refreshNames();
    void selectColumn(int row);
    void refresh();
};
#endif
''')
s = '''#include "history_data_dialog.h"
#include "hmi_property_widgets.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QVBoxLayout>
'''
s += f('paintHistoryData', 'Draw time, date and visible collection captions with bounded blank rows and independent grid switches.', 'void paintHistoryData(QPainter *painter, const QRectF &rect, const HistoryDataProperties &properties, const QString &language)', '''    QStringList titles;
    HistoryDataCaptions captions = properties.captionsFor(language);
    HistoryDataColumn column;
    QFont font(QStringLiteral("SimSun"));
    QRectF cell;
    qreal rowHeight;
    qreal columnWidth;
    int index;
    if (properties.showTime) { titles.append(captions.time); } if (properties.showDate) { titles.append(captions.date); }
    Q_FOREACH (column, properties.columns) { if (column.shown) { titles.append(column.names.value(language)); } }
    if (titles.isEmpty() || rect.isEmpty() || properties.rows < 0 || properties.rows > 20) { return; }
    rowHeight = rect.height() / (properties.rows + 1); columnWidth = rect.width() / titles.size(); font.setPixelSize(properties.fontSize);
    painter->save(); painter->setClipRect(rect, Qt::IntersectClip); painter->fillRect(rect, QColor(properties.backgroundColor)); painter->fillRect(QRectF(rect.left(), rect.top(), rect.width(), rowHeight), QColor(properties.titleBackground)); painter->setFont(font); painter->setPen(QColor(properties.titleColor));
    for (index = 0; index < titles.size(); ++index)
    {
        cell = QRectF(rect.left() + index * columnWidth, rect.top(), columnWidth, rowHeight); painter->save(); painter->setClipRect(cell.adjusted(1, 1, -1, -1), Qt::IntersectClip); painter->drawText(cell.adjusted(2, 0, -2, 0), Qt::AlignCenter | Qt::TextSingleLine, titles[index]); painter->restore();
    }
    painter->setPen(QPen(QColor(properties.lineColor), 1));
    if (properties.verticalLines) { for (index = 1; index < titles.size(); ++index) { painter->drawLine(QPointF(rect.left() + index * columnWidth, rect.top()), QPointF(rect.left() + index * columnWidth, rect.bottom())); } }
    if (properties.horizontalLines) { for (index = 1; index <= properties.rows; ++index) { painter->drawLine(QPointF(rect.left(), rect.top() + index * rowHeight), QPointF(rect.right(), rect.top() + index * rowHeight)); } }
    painter->setPen(QPen(QColor(properties.frameColor), 1)); painter->setBrush(Qt::NoBrush); painter->drawRect(rect.adjusted(0.5, 0.5, -0.5, -0.5)); painter->restore();''', 'painter, rect, properties, language')
s += f('HistoryDataDialog::color', 'Resolve each of the six measured common color drafts.', 'QString *HistoryDataDialog::color(int index)', '''    if (index == 0) { return &m_draft.textColor; } if (index == 1) { return &m_draft.lineColor; } if (index == 2) { return &m_draft.titleBackground; } if (index == 3) { return &m_draft.titleColor; } if (index == 4) { return &m_draft.backgroundColor; } return &m_draft.frameColor;''', 'index', 'Selected common color field.')
s += f('HistoryDataDialog::addColor', 'Open the shared color picker without publishing a canceled color or parent draft.', 'void HistoryDataDialog::addColor(QGridLayout *grid, int row, const QString &label, int index)', '''    QPushButton *button = new QPushButton(this);
    button->setObjectName(QStringLiteral("HistoryDataColor%1").arg(index)); button->setFixedSize(100, 24); button->setStyleSheet(QStringLiteral("background:%1;").arg(*color(index))); grid->addWidget(new QLabel(label, this), row, 0); grid->addWidget(button, row, 1, Qt::AlignLeft);
    connect(button, &QPushButton::clicked, this, [this, button, index]() { QColor selected = QColorDialog::getColor(QColor(*color(index)), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog); if (selected.isValid()) { *color(index) = selected.name(); button->setStyleSheet(QStringLiteral("background:%1;").arg(selected.name())); refresh(); } });''', 'grid, row, label, index')
s += f('HistoryDataDialog::basicPage', 'Reproduce general groups, four font sizes, six colors and editable hidden date/time captions.', 'QWidget *HistoryDataDialog::basicPage()', '''    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *text = new QGroupBox(page);
    QGroupBox *lines = new QGroupBox(QStringLiteral("格线"), page);
    QGroupBox *title = new QGroupBox(QStringLiteral("标题列"), page);
    QGroupBox *fill = new QGroupBox(QStringLiteral("填充"), page);
    QGroupBox *date = new QGroupBox(page);
    QGroupBox *time = new QGroupBox(page);
    QGridLayout *grid = new QGridLayout(text);
    QSpinBox *rows = new QSpinBox(text);
    QCheckBox *vertical = new QCheckBox(QStringLiteral("垂直"), lines);
    QCheckBox *horizontal = new QCheckBox(QStringLiteral("水平"), lines);
    QComboBox *font = new QComboBox(title);
    QComboBox *size = new QComboBox(title);
    QCheckBox *dateCheck = new QCheckBox(QStringLiteral("日期显示"), date);
    QCheckBox *timeCheck = new QCheckBox(QStringLiteral("时间显示"), time);
    HistoryDataCaptions captions = m_draft.captionsFor(m_language);
    int value;
    layout->setContentsMargins(12, 16, 110, 100); layout->setHorizontalSpacing(38); layout->setVerticalSpacing(10); layout->setColumnMinimumWidth(0, 210); layout->setColumnMinimumWidth(1, 210);
    addColor(grid, 0, QStringLiteral("文本色:"), 0); rows->setObjectName(QStringLiteral("HistoryDataRows")); rows->setRange(0, 20); rows->setValue(m_draft.rows); grid->addWidget(new QLabel(QStringLiteral("行数:"), text), 1, 0); grid->addWidget(rows, 1, 1); layout->addWidget(text, 0, 0);
    grid = new QGridLayout(lines); vertical->setObjectName(QStringLiteral("HistoryDataVertical")); horizontal->setObjectName(QStringLiteral("HistoryDataHorizontal")); vertical->setChecked(m_draft.verticalLines); horizontal->setChecked(m_draft.horizontalLines); grid->addWidget(vertical, 0, 0); grid->addWidget(horizontal, 0, 1); addColor(grid, 1, QStringLiteral("颜色:"), 1); layout->addWidget(lines, 1, 0);
    m_languages = new QComboBox(page); m_languages->setObjectName(QStringLiteral("HistoryDataLanguage")); m_languages->addItems(m_project.languages); m_languages->setCurrentText(m_language); grid = new QGridLayout(); grid->addWidget(new QLabel(QStringLiteral("语言:"), page), 0, 0); grid->addWidget(m_languages, 0, 1); layout->addLayout(grid, 2, 0);
    grid = new QGridLayout(title); font->setObjectName(QStringLiteral("HistoryDataFont")); font->addItem(QStringLiteral("宋体")); size->setObjectName(QStringLiteral("HistoryDataFontSize")); Q_FOREACH (value, HistoryDataProperties::fontSizes()) { size->addItem(QString::number(value)); } size->setCurrentText(QString::number(m_draft.fontSize)); grid->addWidget(new QLabel(QStringLiteral("字体:"), title), 0, 0); grid->addWidget(font, 0, 1); grid->addWidget(new QLabel(QStringLiteral("字体大小:"), title), 1, 0); grid->addWidget(size, 1, 1); addColor(grid, 2, QStringLiteral("背景色:"), 2); addColor(grid, 3, QStringLiteral("文字颜色:"), 3); layout->addWidget(title, 0, 1, 2, 1);
    grid = new QGridLayout(fill); addColor(grid, 0, QStringLiteral("前景色:"), 4); addColor(grid, 1, QStringLiteral("边框色:"), 5); layout->addWidget(fill, 2, 1);
    dateCheck->setObjectName(QStringLiteral("HistoryDataShowDate")); dateCheck->setChecked(m_draft.showDate); m_date = new QLineEdit(captions.date, date); m_date->setObjectName(QStringLiteral("HistoryDataDateCaption")); m_date->setMaxLength(32767); grid = new QGridLayout(date); grid->addWidget(dateCheck, 0, 0, 1, 2); grid->addWidget(new QLabel(QStringLiteral("日期:"), date), 1, 0); grid->addWidget(m_date, 1, 1); layout->addWidget(date, 3, 0);
    timeCheck->setObjectName(QStringLiteral("HistoryDataShowTime")); timeCheck->setChecked(m_draft.showTime); m_time = new QLineEdit(captions.time, time); m_time->setObjectName(QStringLiteral("HistoryDataTimeCaption")); m_time->setMaxLength(32767); grid = new QGridLayout(time); grid->addWidget(timeCheck, 0, 0, 1, 2); grid->addWidget(new QLabel(QStringLiteral("时间:"), time), 1, 0); grid->addWidget(m_time, 1, 1); layout->addWidget(time, 3, 1); layout->setRowStretch(4, 1);
    connect(m_languages, &QComboBox::currentTextChanged, this, &HistoryDataDialog::changeLanguage); connect(m_date, &QLineEdit::textChanged, this, [this](const QString &value) { m_draft.captions[m_language].date = value; refresh(); }); connect(m_time, &QLineEdit::textChanged, this, [this](const QString &value) { m_draft.captions[m_language].time = value; refresh(); });
    connect(rows, &QSpinBox::valueChanged, this, [this](int value) { m_draft.rows = value; refresh(); }); connect(size, &QComboBox::currentTextChanged, this, [this](const QString &value) { m_draft.fontSize = value.toInt(); refresh(); }); connect(vertical, &QCheckBox::toggled, this, [this](bool value) { m_draft.verticalLines = value; refresh(); }); connect(horizontal, &QCheckBox::toggled, this, [this](bool value) { m_draft.horizontalLines = value; refresh(); }); connect(dateCheck, &QCheckBox::toggled, this, [this](bool value) { m_draft.showDate = value; refresh(); }); connect(timeCheck, &QCheckBox::toggled, this, [this](bool value) { m_draft.showTime = value; refresh(); }); return page;''', returns='General page.')
s += f('HistoryDataDialog::dataPage', 'Edit collection-derived names and shared shown flags with both full-table synchronization commands.', 'QWidget *HistoryDataDialog::dataPage()', '''    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QPushButton *current = new QPushButton(QStringLiteral("将第一个语言名称\\n同步到当前语言"), page);
    QPushButton *all = new QPushButton(QStringLiteral("将第一个语言名称\\n同步到所有语言"), page);
    QTableWidgetItem *name;
    QTableWidgetItem *shown;
    int row;
    layout->setContentsMargins(12, 12, 110, 10); m_table = new QTableWidget(m_draft.columns.size(), 2, page); m_table->setObjectName(QStringLiteral("HistoryDataColumns")); m_table->setHorizontalHeaderLabels({QStringLiteral("名称"), QStringLiteral("显示")}); m_table->verticalHeader()->hide(); m_table->setSelectionBehavior(QAbstractItemView::SelectRows); m_table->setSelectionMode(QAbstractItemView::SingleSelection); m_table->setEditTriggers(QAbstractItemView::NoEditTriggers); m_table->setStyleSheet(QStringLiteral("QTableWidget {background:#caeace;}")); m_table->setColumnWidth(0, 100); m_table->setColumnWidth(1, 100); m_table->setMinimumHeight(300);
    for (row = 0; row < m_draft.columns.size(); ++row) { name = new QTableWidgetItem(m_draft.columns[row].names.value(m_language)); shown = new QTableWidgetItem(); shown->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable); shown->setCheckState(m_draft.columns[row].shown ? Qt::Checked : Qt::Unchecked); m_table->setItem(row, 0, name); m_table->setItem(row, 1, shown); }
    layout->addWidget(m_table, 0, 0, 1, 4); layout->setRowStretch(1, 1); m_dataLanguages = new QComboBox(page); m_dataLanguages->setObjectName(QStringLiteral("HistoryDataDataLanguage")); m_dataLanguages->addItems(m_project.languages); m_dataLanguages->setCurrentText(m_language); m_name = new QLineEdit(page); m_name->setObjectName(QStringLiteral("HistoryDataColumnName")); m_name->setMaxLength(32767); layout->addWidget(new QLabel(QStringLiteral("语言:"), page), 2, 0); layout->addWidget(m_dataLanguages, 2, 1); layout->addWidget(new QLabel(QStringLiteral("名称:"), page), 2, 2); layout->addWidget(m_name, 2, 3); current->setObjectName(QStringLiteral("HistoryDataSyncCurrent")); all->setObjectName(QStringLiteral("HistoryDataSyncAll")); current->setMinimumHeight(48); all->setMinimumHeight(48); layout->addWidget(current, 3, 0, 1, 2); layout->addWidget(all, 3, 2, 1, 2);
    connect(m_dataLanguages, &QComboBox::currentTextChanged, this, &HistoryDataDialog::changeLanguage); connect(m_table, &QTableWidget::currentCellChanged, this, [this](int row, int, int, int) { selectColumn(row); }); connect(m_table, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *item) { if (item->column() == 1) { m_draft.columns[item->row()].shown = item->checkState() == Qt::Checked; refresh(); } }); connect(m_name, &QLineEdit::textChanged, this, [this](const QString &value) { int row = m_table->currentRow(); if (row >= 0 && row < m_draft.columns.size()) { QSignalBlocker blocker(m_table); m_draft.columns[row].names[m_language] = value; m_table->item(row, 0)->setText(value); refresh(); } });
    connect(current, &QPushButton::clicked, this, [this]() { m_draft.synchronizeNames(m_project.languages.first(), {m_language}); refreshNames(); }); connect(all, &QPushButton::clicked, this, [this]() { m_draft.synchronizeNames(m_project.languages.first(), m_project.languages); refreshNames(); }); if (m_table->rowCount() > 0) { m_table->setCurrentCell(0, 0); } else { selectColumn(-1); } return page;''', returns='Collection column page.')
s += f('HistoryDataDialog::HistoryDataDialog', 'Bind the required real collector and construct a detached three-page historical-data editor.', 'HistoryDataDialog::HistoryDataDialog(const HmiObject &object, const HmiProject &project, QWidget *parent)\n    : QDialog(parent), m_original(object), m_project(project), m_draft(object.historyData), m_language(project.activeLanguage)', '''    QGridLayout *layout = new QGridLayout(this);
    QTabWidget *tabs = new QTabWidget(this);
    QWidget *permission = new QWidget(this);
    QVBoxLayout *permissions = new QVBoxLayout(permission);
    QGroupBox *preview = new QGroupBox(QStringLiteral("预览"), this);
    QVBoxLayout *previewLayout = new QVBoxLayout(preview);
    QDialogButtonBox *buttons;
    HmiConfigurationNode node;
    QJsonArray connections;
    PlcConnection connection;
    QString error;
    Q_FOREACH (node, project.configurationNodes)
    {
        if (node.kind != QStringLiteral("dataCollection") || !node.active || !node.implemented || (!m_draft.collectionId.isEmpty() && node.id != m_draft.collectionId)) { continue; }
        m_collectionFound = DataCollectionProperties::fromJson(node.properties, &m_collection, &error); if (m_collectionFound && m_draft.collectionId.isEmpty()) { m_draft.collectionId = node.id; m_draft.reconcileCollection(m_collection, project.languages); } break;
    }
    m_draft.setLanguages(project.languages); setObjectName(QStringLiteral("HistoryDataDialog")); setWindowTitle(QStringLiteral("历史数据显示器")); resize(812, 565); setFont(QFont(QStringLiteral("SimSun"), 9)); setStyleSheet(QStringLiteral("QLineEdit, QComboBox, QSpinBox {background:#caeace;} QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled {background:#b1ceed;}")); Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    tabs->setObjectName(QStringLiteral("HistoryDataTabs")); tabs->addTab(basicPage(), QStringLiteral("一般")); tabs->addTab(dataPage(), QStringLiteral("数据")); m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("HistoryDataVisible"), m_draft.visibility, false, permission); m_visibility->configureUnchosenBitState(m_draft.visibilityBitChosen); permissions->addWidget(m_visibility); permissions->addStretch(); tabs->addTab(permission, QStringLiteral("权限设置")); layout->addWidget(tabs, 0, 0); layout->setColumnStretch(0, 1); m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("HistoryDataPreview")); m_preview->setFixedSize(130, 130); previewLayout->addWidget(m_preview); layout->addWidget(preview, 0, 1, Qt::AlignTop);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("HistoryDataButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons, 1, 0, 1, 2); connect(buttons, &QDialogButtonBox::accepted, this, &HistoryDataDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, windowTitle() + QStringLiteral("帮助"), QStringLiteral("先建立数据采集，再设置表格样式、数据列名称和显现权限。两个同步按钮复制第一语言的全部数据列名。")); }); refresh();''', 'object, project, parent')
s += f('HistoryDataDialog::selectColumn', 'Load one localized name under a blocker without changing any other column.', 'void HistoryDataDialog::selectColumn(int row)', '''    QSignalBlocker blocker(m_name);
    m_name->setEnabled(row >= 0 && row < m_draft.columns.size()); m_name->setText(m_name->isEnabled() ? m_draft.columns[row].names.value(m_language) : QString());''', 'row')
s += f('HistoryDataDialog::refreshNames', 'Refresh all names after language or synchronization changes while preserving selection and flags.', 'void HistoryDataDialog::refreshNames()', '''    QSignalBlocker blocker(m_table);
    int row;
    for (row = 0; row < m_draft.columns.size(); ++row) { m_table->item(row, 0)->setText(m_draft.columns[row].names.value(m_language)); } selectColumn(m_table->currentRow()); refresh();''')
s += f('HistoryDataDialog::changeLanguage', 'Keep both language selectors in sync without overwriting localized captions or project language.', 'void HistoryDataDialog::changeLanguage(const QString &language)', '''    QSignalBlocker general(m_languages);
    QSignalBlocker data(m_dataLanguages);
    QSignalBlocker date(m_date);
    QSignalBlocker time(m_time);
    HistoryDataCaptions captions = m_draft.captionsFor(language);
    m_language = language; m_languages->setCurrentText(language); m_dataLanguages->setCurrentText(language); m_date->setText(captions.date); m_time->setText(captions.time); refreshNames();''', 'language')
s += f('HistoryDataDialog::refresh', 'Render only illustrative blank rows, including safe zero-row tables.', 'void HistoryDataDialog::refresh()', '''    QImage image(650, 650, QImage::Format_ARGB32);
    QPainter painter;
    if (m_preview == nullptr) { return; } image.fill(QColor(QStringLiteral("#f0f0f0"))); painter.begin(&image); paintHistoryData(&painter, QRectF(50, 190, 550, 280), m_draft, m_language); painter.end(); m_preview->setPixmap(QPixmap::fromImage(image).scaled(130, 130, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));''')
s += f('HistoryDataDialog::object', 'Return the complete detached type and visibility draft with no generic operation carrier.', 'HmiObject HistoryDataDialog::object() const', '''    HmiObject object = m_original;
    object.type = QStringLiteral("historydata"); object.historyData = m_draft; object.address.clear(); object.text.clear(); object.hasBitButtonProperties = false; object.hasStaticTextProperties = false; object.hasNumberDisplayProperties = false; object.hasImageDisplayProperties = false; object.historyData.visibility = m_visibility->properties(); object.historyData.visibilityBitChosen = m_visibility->bitStateChosen() || m_accepted; return object;''', returns='Complete detached historical-data display.')
s += f('HistoryDataDialog::accept', 'Reject missing collections, empty display columns and invalid active visibility before one parent transaction.', 'void HistoryDataDialog::accept()', '''    HmiObject candidate;
    QString error;
    m_accepted = true; candidate = object();
    if (!m_collectionFound) { m_accepted = false; QMessageBox::warning(this, QStringLiteral("KinSealStudio"), QStringLiteral("没有建立数据采集")); return; }
    if (!candidate.historyData.validate(&error) || !candidate.historyData.validateCollection(m_collection, &error) || !candidate.historyData.validateAddresses(m_project.connections, &error)) { m_accepted = false; QMessageBox::warning(this, QStringLiteral("设置错误"), error); return; } QDialog::accept();''')
put('src/history_data_dialog.cpp', s)
print('Created measured three-page history-data editor and blank-row shared renderer.')
