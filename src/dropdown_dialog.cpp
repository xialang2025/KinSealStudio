#include "dropdown_dialog.h"
#include "hmi_property_widgets.h"
#include "word_button_properties.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QUuid>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : paintDropdown
 * Description     : Draw the observed blank editor face and direction arrow without inventing a live PLC selection.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintDropdown(QPainter *painter, const QRectF &rect, const DropdownProperties &properties) //20261112-0 add
{
    QRectF strip(rect.right() - rect.width() * 0.22, rect.top(), rect.width() * 0.22, rect.height());
    qreal width = qMin(strip.width() * 0.88, rect.height() * 0.4);
    qreal top = rect.top() + rect.height() * 0.1;
    qreal center = strip.center().x();
    QPolygonF arrow;
    painter->save(); painter->setClipRect(rect); painter->fillRect(rect, QColor(properties.background)); painter->fillRect(strip, QColor(QStringLiteral("#c0c0c0"))); painter->setPen(Qt::NoPen); painter->setBrush(Qt::black);
    if (properties.direction == 0) { arrow << QPointF(center - width / 2, top) << QPointF(center + width / 2, top) << QPointF(center, top + width); }
    else { arrow << QPointF(center - width / 2, top + width) << QPointF(center + width / 2, top + width) << QPointF(center, top); }
    painter->drawPolygon(arrow); painter->restore();
}

/******************************************************************************
 * Function Name   : DropdownDialog::DropdownDialog
 * Description     : Build the three measured pages with isolated table/macro drafts and an explicit default confirmation button.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
DropdownDialog::DropdownDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261112-0 add
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.dropdown)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QTabWidget *tabs = new QTabWidget(this);
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    setObjectName(QStringLiteral("DropdownDialog")); setWindowTitle(QStringLiteral("下拉框")); resize(676, 559);
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    tabs->setObjectName(QStringLiteral("DropdownTabs")); tabs->addTab(createGeneral(), QStringLiteral("基本属性"));
    m_table = new QTableWidget(this); m_table->setObjectName(QStringLiteral("DropdownTable")); m_table->setColumnCount(3); m_table->setHorizontalHeaderLabels({QStringLiteral("项目"), QStringLiteral("数据"), QStringLiteral("项目资料")}); m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch); m_table->verticalHeader()->setVisible(false); loadRows(); tabs->addTab(m_table, QStringLiteral("下拉列表")); tabs->addTab(createPermissions(), QStringLiteral("权限设置")); layout->addWidget(tabs);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("DropdownButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Ok)->setDefault(true); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &DropdownDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("下拉框帮助"), QStringLiteral("设置2至15个项目，分别编辑数据与项目资料。方向决定编辑外观箭头。\n选中项写值、原厂文本编译与屏端弹出行为仍需原生和设备验证。")); });
    connect(m_count, &QSpinBox::valueChanged, this, &DropdownDialog::resizeRows);
}

/******************************************************************************
 * Function Name   : DropdownDialog::createGeneral
 * Description     : Build measured bounds, color drafts, word carrier and enabled macro editor entry.
 * Input Parameters: None
 * Return Value    : Basic properties page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *DropdownDialog::createGeneral() //20261112-0 add
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    QGroupBox *properties = new QGroupBox(QStringLiteral("属性"), page);
    QGridLayout *grid = new QGridLayout(properties);
    QGroupBox *address;
    QPushButton *picker;
    QPushButton *write;
    layout->setContentsMargins(18, 16, 18, 16); layout->setSpacing(16); properties->setFixedHeight(160); //20261112-0 change: Match the native three-group vertical rhythm instead of collapsing the forms to their minimum size.
    page->setStyleSheet(QStringLiteral("QSpinBox, QComboBox, QLineEdit {background:#caeace;} QSpinBox:disabled, QComboBox:disabled, QLineEdit:disabled {background:#f0f0f0;}"));
    m_count = new QSpinBox(properties); m_count->setObjectName(QStringLiteral("DropdownCount")); m_count->setRange(2, 15); m_count->setValue(m_draft.entries.size()); m_fontSize = new QSpinBox(properties); m_fontSize->setObjectName(QStringLiteral("DropdownFontSize")); m_fontSize->setRange(8, 40); m_fontSize->setValue(m_draft.fontSize);
    m_alignment = new QComboBox(properties); m_alignment->setObjectName(QStringLiteral("DropdownAlignment")); m_alignment->addItems({QStringLiteral("左对齐"), QStringLiteral("右对齐"), QStringLiteral("居中")}); m_alignment->setCurrentIndex(m_draft.alignment); m_direction = new QComboBox(properties); m_direction->setObjectName(QStringLiteral("DropdownDirection")); m_direction->addItems({QStringLiteral("向下"), QStringLiteral("向上")}); m_direction->setCurrentIndex(m_draft.direction);
    m_background = new QPushButton(properties); m_background->setObjectName(QStringLiteral("DropdownBackground")); m_background->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.background)); m_textColor = new QPushButton(properties); m_textColor->setObjectName(QStringLiteral("DropdownTextColor")); m_textColor->setStyleSheet(QStringLiteral("background:%1").arg(m_draft.textColor));
    grid->addWidget(new QLabel(QStringLiteral("总项目数:"), properties), 0, 0); grid->addWidget(m_count, 0, 1); grid->addWidget(new QLabel(QStringLiteral("背景色:"), properties), 0, 2); grid->addWidget(m_background, 0, 3); grid->addWidget(new QLabel(QStringLiteral("字体大小:"), properties), 1, 0); grid->addWidget(m_fontSize, 1, 1); grid->addWidget(new QLabel(QStringLiteral("文本色:"), properties), 1, 2); grid->addWidget(m_textColor, 1, 3); grid->addWidget(new QLabel(QStringLiteral("对齐:"), properties), 2, 0); grid->addWidget(m_alignment, 2, 1); grid->addWidget(new QLabel(QStringLiteral("方向:"), properties), 2, 2); grid->addWidget(m_direction, 2, 3); layout->addWidget(properties);
    address = new QGroupBox(QStringLiteral("地址"), page); address->setFixedHeight(100); grid = new QGridLayout(address); m_address = new QLineEdit(m_draft.address, address); m_address->setObjectName(QStringLiteral("DropdownAddress")); m_address->setMaximumWidth(150); picker = new QPushButton(QStringLiteral("…"), address); picker->setObjectName(QStringLiteral("DropdownAddressPicker")); picker->setFixedWidth(24); m_type = new QComboBox(address); m_type->setObjectName(QStringLiteral("DropdownDataType")); m_type->addItems(WordButtonProperties::dataTypes()); m_type->setCurrentIndex(m_draft.dataType); grid->addWidget(new QLabel(QStringLiteral("字地址:"), address), 0, 0); grid->addWidget(m_address, 0, 1); grid->addWidget(picker, 0, 2); grid->addWidget(new QLabel(QStringLiteral("数据类型:"), address), 0, 3); grid->addWidget(m_type, 0, 4); layout->addWidget(address); //20261112-0 change: Native operation carrier and data type share one horizontal row.
    m_macro = new QGroupBox(QStringLiteral("宏指令"), page); m_macro->setObjectName(QStringLiteral("DropdownMacro")); m_macro->setCheckable(true); m_macro->setChecked(m_draft.macroEnabled); grid = new QGridLayout(m_macro); m_macroList = new QComboBox(m_macro); m_macroList->setObjectName(QStringLiteral("DropdownMacroList")); m_macroList->addItems(m_project.macros.keys()); if (!m_draft.macroName.isEmpty() && !m_project.macros.contains(m_draft.macroName)) { m_macroList->addItem(m_draft.macroName); } m_macroList->setCurrentIndex(m_macroList->findText(m_draft.macroName)); write = new QPushButton(QStringLiteral("写宏"), m_macro); write->setObjectName(QStringLiteral("DropdownWriteMacro")); grid->addWidget(new QLabel(QStringLiteral("使用宏:"), m_macro), 0, 0); grid->addWidget(m_macroList, 0, 1); grid->addWidget(write, 0, 2); layout->addWidget(m_macro); layout->addStretch();
    m_macro->setFixedHeight(90); //20261112-0 add: Retain the native macro group height even while its fields are disabled.
    connect(m_background, &QPushButton::clicked, this, [this]() { chooseColor(true); }); connect(m_textColor, &QPushButton::clicked, this, [this]() { chooseColor(false); }); connect(picker, &QPushButton::clicked, this, [this]() { DropdownProperties properties; properties.dataType = m_type->currentIndex(); editHmiAddress(this, m_address, false, properties.transferWordCount()); }); connect(write, &QPushButton::clicked, this, &DropdownDialog::editMacro);
    return page;
}

/******************************************************************************
 * Function Name   : DropdownDialog::createPermissions
 * Description     : Reuse unchosen bit conditions, expose bounded notification drafts and omit the native-absent hold-time row.
 * Input Parameters: None
 * Return Value    : Permission page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *DropdownDialog::createPermissions() //20261112-0 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGridLayout *grid;
    QWidget *statePanel;
    QHBoxLayout *row;
    QRadioButton *off;
    QPushButton *bitPicker;
    QPushButton *wordPicker;
    m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("DropdownVisibility"), m_draft.visibility, false, page); m_touch = new HmiPermissionEditor(QStringLiteral("触控有效"), QStringLiteral("DropdownTouch"), m_draft.touch, true, page); m_visibility->configureUnchosenBitState(m_draft.visibilityBitChosen); m_touch->configureUnchosenBitState(m_draft.touchBitChosen); layout->addWidget(m_visibility, 0, 0); layout->addWidget(m_touch, 0, 1);
    m_notice = new QGroupBox(QStringLiteral("通知"), page); m_notice->setObjectName(QStringLiteral("DropdownNotice")); m_notice->setCheckable(true); m_notice->setChecked(m_draft.notice); grid = new QGridLayout(m_notice);
    m_noticeBit = new QCheckBox(QStringLiteral("通知位"), m_notice); m_noticeBit->setObjectName(QStringLiteral("DropdownNoticeBit")); m_noticeBit->setChecked(m_draft.noticeBit); m_noticeWord = new QCheckBox(QStringLiteral("通知字"), m_notice); m_noticeWord->setObjectName(QStringLiteral("DropdownNoticeWord")); m_noticeWord->setChecked(m_draft.noticeWord);
    m_noticeBitAddress = new QLineEdit(m_draft.noticeBitAddress, m_notice); m_noticeBitAddress->setObjectName(QStringLiteral("DropdownNoticeBitAddress")); m_noticeWordAddress = new QLineEdit(m_draft.noticeWordAddress, m_notice); m_noticeWordAddress->setObjectName(QStringLiteral("DropdownNoticeWordAddress")); bitPicker = new QPushButton(QStringLiteral("…"), m_notice); wordPicker = new QPushButton(QStringLiteral("…"), m_notice);
    statePanel = new QWidget(m_notice); row = new QHBoxLayout(statePanel); row->setContentsMargins(0, 0, 0, 0); m_noticeBitOn = new QRadioButton(QStringLiteral("1"), statePanel); m_noticeBitOn->setObjectName(QStringLiteral("DropdownNoticeBitOn")); off = new QRadioButton(QStringLiteral("0"), statePanel); off->setObjectName(QStringLiteral("DropdownNoticeBitOff")); m_noticeBitOn->setChecked(m_draft.noticeBitValue); off->setChecked(!m_draft.noticeBitValue); row->addWidget(m_noticeBitOn); row->addWidget(off);
    m_noticeWordType = new QComboBox(m_notice); m_noticeWordType->setObjectName(QStringLiteral("DropdownNoticeWordType")); m_noticeWordType->addItems(StaticTextProperties::dataTypes()); m_noticeWordType->setCurrentIndex(m_draft.noticeWordType); m_noticeWordValue = new QLineEdit(m_draft.noticeWordValue, m_notice); m_noticeWordValue->setObjectName(QStringLiteral("DropdownNoticeWordValue"));
    grid->addWidget(m_noticeBit, 0, 0); grid->addWidget(m_noticeBitAddress, 0, 1); grid->addWidget(bitPicker, 0, 2); grid->addWidget(m_noticeWord, 0, 3); grid->addWidget(m_noticeWordAddress, 0, 4); grid->addWidget(wordPicker, 0, 5); grid->addWidget(statePanel, 1, 0, 1, 3); grid->addWidget(m_noticeWordType, 1, 3, 1, 3); grid->addWidget(m_noticeWordValue, 2, 3, 1, 3); layout->addWidget(m_notice, 1, 0, 1, 2); layout->setRowStretch(2, 1);
    connect(m_noticeBit, &QCheckBox::toggled, this, [this, statePanel, bitPicker](bool enabled) { if (enabled) { m_noticeWord->setChecked(false); } m_noticeBitAddress->setEnabled(enabled); statePanel->setEnabled(enabled); bitPicker->setEnabled(enabled); });
    connect(m_noticeWord, &QCheckBox::toggled, this, [this, wordPicker](bool enabled) { if (enabled) { m_noticeBit->setChecked(false); } m_noticeWordAddress->setEnabled(enabled); m_noticeWordType->setEnabled(enabled); m_noticeWordValue->setEnabled(enabled); wordPicker->setEnabled(enabled); });
    m_noticeBitAddress->setEnabled(m_draft.noticeBit); statePanel->setEnabled(m_draft.noticeBit); bitPicker->setEnabled(m_draft.noticeBit); m_noticeWordAddress->setEnabled(m_draft.noticeWord); m_noticeWordType->setEnabled(m_draft.noticeWord); m_noticeWordValue->setEnabled(m_draft.noticeWord); wordPicker->setEnabled(m_draft.noticeWord);
    connect(bitPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_noticeBitAddress, true); }); connect(wordPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_noticeWordAddress, false, m_noticeWordType->currentIndex() == 0 || m_noticeWordType->currentIndex() == 2 ? 1 : 2); });
    return page;
}

/******************************************************************************
 * Function Name   : DropdownDialog::loadRows
 * Description     : Rebuild indexed, read-only IDs and editable scalar/text columns from the surviving draft.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void DropdownDialog::loadRows() //20261112-0 add
{
    QTableWidgetItem *id;
    int index;
    m_table->setRowCount(m_draft.entries.size());
    for (index = 0; index < m_draft.entries.size(); ++index)
    { id = new QTableWidgetItem(QString::number(index)); id->setFlags(id->flags() & ~Qt::ItemIsEditable); m_table->setItem(index, 0, id); m_table->setItem(index, 1, new QTableWidgetItem(m_draft.entries.at(index).value)); m_table->setItem(index, 2, new QTableWidgetItem(m_draft.entries.at(index).label)); }
}

/******************************************************************************
 * Function Name   : DropdownDialog::resizeRows
 * Description     : Store committed table text before resizing, preserving surviving rows and using explicit fresh-tail Lite semantics.
 * Input Parameters: count
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void DropdownDialog::resizeRows(int count) //20261112-0 add
{
    m_draft.entries = object().dropdown.entries; m_draft.setCount(count); loadRows();
}

/******************************************************************************
 * Function Name   : DropdownDialog::object
 * Description     : Collect all detached fields without publishing macros, rows or permission changes to the original document.
 * Input Parameters: None
 * Return Value    : Complete independent dropdown object.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiObject DropdownDialog::object() const //20261112-0 add
{
    HmiObject object = m_original;
    DropdownProperties *properties = &object.dropdown;
    int index;
    *properties = m_draft; object.type = QStringLiteral("dropdown"); object.address.clear(); object.text.clear(); object.hasBitButtonProperties = false; object.hasStaticTextProperties = false;
    properties->entries.clear(); for (index = 0; index < m_table->rowCount(); ++index) { properties->entries.append({m_table->item(index, 1)->text(), m_table->item(index, 2)->text()}); }
    properties->fontSize = m_fontSize->value(); properties->alignment = m_alignment->currentIndex(); properties->direction = m_direction->currentIndex(); properties->dataType = m_type->currentIndex(); properties->address = m_address->text().trimmed(); properties->macroEnabled = m_macro->isChecked(); properties->macroName = m_macroList->currentText();
    properties->visibility = m_visibility->properties(); properties->touch = m_touch->properties(); properties->visibilityBitChosen = m_visibility->bitStateChosen(); properties->touchBitChosen = m_touch->bitStateChosen(); properties->notice = m_notice->isChecked(); properties->noticeBit = m_noticeBit->isChecked(); properties->noticeWord = m_noticeWord->isChecked(); properties->noticeBitValue = m_noticeBitOn->isChecked(); properties->noticeBitAddress = m_noticeBitAddress->text().trimmed(); properties->noticeWordAddress = m_noticeWordAddress->text().trimmed(); properties->noticeWordType = m_noticeWordType->currentIndex(); properties->noticeWordValue = m_noticeWordValue->text(); return object;
}

/******************************************************************************
 * Function Name   : DropdownDialog::macros
 * Description     : Return the local macro pool for the same accepting whole-document transaction.
 * Input Parameters: None
 * Return Value    : Detached macro registry.
 * Date            : 2026.11.12
 ******************************************************************************/
QMap<QString, QString> DropdownDialog::macros() const //20261112-0 add
{
    return m_project.macros;
}

/******************************************************************************
 * Function Name   : DropdownDialog::accept
 * Description     : Check every row, active carrier and macro dependency against the complete project before closing.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void DropdownDialog::accept() //20261112-0 add
{
    HmiObject draft = object();
    HmiProject candidate = m_project;
    QString error;
    bool replaced = false;
    int index;
    if (draft.id.isEmpty()) { draft.id = QUuid::createUuid().toString(QUuid::WithoutBraces); }
    for (index = 0; index < candidate.objects.size(); ++index) { if (candidate.objects.at(index).id == draft.id) { candidate.objects[index] = draft; replaced = true; break; } }
    if (!replaced) { candidate.objects.append(draft); }
    if (!candidate.validateModel(&error)) { QMessageBox::warning(this, QStringLiteral("下拉框属性无效"), error); return; }
    QDialog::accept();
}

/******************************************************************************
 * Function Name   : DropdownDialog::chooseColor
 * Description     : Apply a local text/background color only after color-dialog acceptance.
 * Input Parameters: background
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void DropdownDialog::chooseColor(bool background) //20261112-0 add
{
    QColor color = QColorDialog::getColor(QColor(background ? m_draft.background : m_draft.textColor), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog);
    if (!color.isValid()) { return; }
    if (background) { m_draft.background = color.name(); m_background->setStyleSheet(QStringLiteral("background:%1").arg(color.name())); }
    else { m_draft.textColor = color.name(); m_textColor->setStyleSheet(QStringLiteral("background:%1").arg(color.name())); }
}

/******************************************************************************
 * Function Name   : DropdownDialog::editMacro
 * Description     : Reuse the existing Lite source-editor transaction, keeping its template distinct from verified native editor behavior.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void DropdownDialog::editMacro() //20261112-0 add
{
    QDialog dialog(this);
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QLineEdit *name = new QLineEdit(&dialog);
    QPlainTextEdit *source = new QPlainTextEdit(&dialog);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    QString initial = m_macroList->currentText();
    dialog.setObjectName(QStringLiteral("DropdownMacroEditor")); dialog.setWindowTitle(QStringLiteral("宏指令编辑器")); dialog.resize(760, 540); name->setObjectName(QStringLiteral("MacroName")); source->setObjectName(QStringLiteral("MacroSource")); name->setText(initial.isEmpty() ? QStringLiteral("Macro%1").arg(m_project.macros.size() + 1) : initial); source->setPlainText(m_project.macros.value(initial, QStringLiteral("#include \"macro.h\"\n\nvoid Macro_Entry( )\n{\n\n}\n"))); layout->addWidget(name); layout->addWidget(source); layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject); connect(buttons, &QDialogButtonBox::accepted, &dialog, [&dialog, name, source]() { if (name->text().trimmed().isEmpty() || name->text().size() > 256 || source->toPlainText().size() > 1024 * 1024) { QMessageBox::warning(&dialog, QStringLiteral("宏无效"), QStringLiteral("宏名称须为1..256字符，源代码不能超过本版1 MiB保护范围。")); return; } dialog.accept(); });
    if (dialog.exec() == QDialog::Accepted) { m_project.macros.insert(name->text().trimmed(), source->toPlainText()); m_macroList->clear(); m_macroList->addItems(m_project.macros.keys()); m_macroList->setCurrentText(name->text().trimmed()); }
}
