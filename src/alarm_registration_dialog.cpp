#include "alarm_registration_dialog.h"
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
#include <QSet>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>

/******************************************************************************
 * Function Name   : AlarmRegistrationDialog::AlarmRegistrationDialog
 * Description     : Build the observed registration table, common scan/type fields and selected multilingual row editor.
 * Input Parameters: node, project, family, parent
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
AlarmRegistrationDialog::AlarmRegistrationDialog(const HmiConfigurationNode &node, const HmiProject &project, int family, QWidget *parent)
    : QDialog(parent), m_original(node), m_project(project) //20261113-2 add
{
    QGridLayout *layout = new QGridLayout(this);
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
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("AlarmRegistrationButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons, 8, 0, 1, 5); connect(buttons, &QDialogButtonBox::accepted, this, &AlarmRegistrationDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, windowTitle(), QStringLiteral("配置读取范围、扫描时间、使用条目和各语言消息；数位报警可指定跳转与跳回画面，条件报警按低低/低/高/高高配置阈值。")); }); rebuild();
}

/******************************************************************************
 * Function Name   : AlarmRegistrationDialog::rowLabel
 * Description     : Display native local offsets exactly and keep external offsets explicit rather than guessing family radix.
 * Input Parameters: row
 * Return Value    : Editor address/value label.
 * Date            : 2026.11.13
 ******************************************************************************/
QString AlarmRegistrationDialog::rowLabel(int row) const //20261113-2 add
{
    QRegularExpression expression(QStringLiteral("^(L[BW])([0-9]+)$"));
    QRegularExpressionMatch match = expression.match(m_draft.readAddress);
    int offset = m_draft.family == 1 ? (row / 4) * (m_draft.dataType == 0 || m_draft.dataType == 2 || m_draft.dataType == 4 ? 1 : 2) : m_draft.bitType == 1 ? row / 16 : m_draft.bitType == 2 ? 0 : row;
    QString address = match.hasMatch() ? match.captured(1) + QString::number(match.captured(2).toInt() + offset) : QStringLiteral("%1 + %2").arg(m_draft.readAddress).arg(offset);
    if (m_draft.family == 0 && m_draft.bitType == 1) { address += QStringLiteral(".%1").arg(row % 16); } if (m_draft.family == 0 && m_draft.bitType == 2) { address += QStringLiteral(" = %1").arg(row); } return address;
}

/******************************************************************************
 * Function Name   : AlarmRegistrationDialog::rebuild
 * Description     : Refresh used flags and language-specific cells without writing selection changes into another row.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void AlarmRegistrationDialog::rebuild() //20261113-2 add
{
    QSignalBlocker blocker(m_table);
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
    m_table->setCurrentCell(m_draft.rows.isEmpty() ? -1 : selected, 0); m_loading = false; selectRow(m_table->currentRow());
}

/******************************************************************************
 * Function Name   : AlarmRegistrationDialog::selectRow
 * Description     : Restore one row without emitting message, threshold or navigation writes.
 * Input Parameters: row
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void AlarmRegistrationDialog::selectRow(int row) //20261113-2 add
{
    bool valid = row >= 0 && row < m_draft.rows.size();
    QSignalBlocker jumpBlock(m_jump);
    QSignalBlocker backBlock(m_back);
    QStringList levels = {QStringLiteral("低低"), QStringLiteral("低"), QStringLiteral("高"), QStringLiteral("高高")};
    m_loading = true; m_message->setEnabled(valid); m_limit->setEnabled(valid); m_jump->setEnabled(valid); m_back->setEnabled(valid); m_rowAddress->setText(valid ? rowLabel(row) : QString()); m_rowType->setText(valid && m_draft.family == 1 ? levels.at(row % 4) : QString()); m_message->setText(valid ? m_draft.rows.at(row).messages.value(m_language->currentText()) : QString()); m_limit->setText(valid ? m_draft.rows.at(row).limit : QString()); m_jump->setCurrentIndex(valid ? m_jump->findData(m_draft.rows.at(row).jumpPageId) : 0); m_back->setCurrentIndex(valid ? m_back->findData(m_draft.rows.at(row).backPageId) : 0); m_loading = false;
}

/******************************************************************************
 * Function Name   : AlarmRegistrationDialog::saveRow
 * Description     : Write exactly the selected language and row while keeping disabled navigation drafts bounded.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void AlarmRegistrationDialog::saveRow() //20261113-2 add
{
    int index = m_table->currentRow();
    AlarmRegistrationRow *row;
    QSignalBlocker blocker(m_table);
    if (m_loading || index < 0 || index >= m_draft.rows.size()) { return; } row = &m_draft.rows[index]; row->messages.insert(m_language->currentText(), m_message->text()); row->limit = m_limit->text(); if (m_draft.family == 0) { row->jumpPageId = m_jump->currentData().toString(); row->backPageId = m_back->currentData().toString(); m_table->item(index, 3)->setText(m_jump->currentText()); m_table->item(index, 4)->setText(m_back->currentText()); } m_table->item(index, m_draft.family == 0 ? 2 : 1)->setText(m_message->text());
}

/******************************************************************************
 * Function Name   : AlarmRegistrationDialog::changeMode
 * Description     : Require the observed confirmation and rebuild type-specific rows only after acceptance.
 * Input Parameters: mode
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void AlarmRegistrationDialog::changeMode(int mode) //20261113-2 add
{
    QSignalBlocker blocker(m_mode);
    int count = m_draft.count;
    if (mode == m_draft.bitType) { return; }
    if (QMessageBox::question(this, QStringLiteral("提示"), QStringLiteral("是否更改当前报警类型"), QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok) { m_mode->setCurrentIndex(m_draft.bitType); return; }
    m_draft.bitType = mode; m_draft.readAddress = mode == 0 ? QStringLiteral("LB0") : QStringLiteral("LW0"); m_draft.rows.clear(); m_draft.resizeRows(count, m_project.languages); m_read->setText(m_draft.readAddress); rebuild();
}

/******************************************************************************
 * Function Name   : AlarmRegistrationDialog::node
 * Description     : Publish a typed active global node while preserving its existing stable identity.
 * Input Parameters: None
 * Return Value    : Complete configuration node.
 * Date            : 2026.11.13
 ******************************************************************************/
HmiConfigurationNode AlarmRegistrationDialog::node() const //20261113-2 add
{
    HmiConfigurationNode node = m_original;
    HmiConfigurationNode peer;
    QString base = m_draft.family == 0 ? QStringLiteral("alarm-bit") : QStringLiteral("alarm-condition");
    QSet<QString> identities;
    int suffix = 1;
    Q_FOREACH (peer, m_project.configurationNodes) { identities.insert(peer.id); }
    if (node.id.isEmpty()) { node.id = base; while (identities.contains(node.id)) { node.id = base + QStringLiteral("-%1").arg(suffix++); } } // Preserve unrelated inactive configuration identities.
    node.kind = QStringLiteral("alarm"); node.name = m_draft.family == 0 ? QStringLiteral("数位地址报警") : QStringLiteral("条件报警"); node.ownerPageId.clear(); node.active = true; node.implemented = true; node.properties = m_draft.toJson(); return node;
}

/******************************************************************************
 * Function Name   : AlarmRegistrationDialog::accept
 * Description     : Validate every definition dependency before accepting a whole-project transaction.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void AlarmRegistrationDialog::accept() //20261113-2 add
{
    QString error;
    if (!m_initialValid || !m_draft.validateContext(m_project, &error)) { QMessageBox::warning(this, QStringLiteral("报警登录未提交"), error.isEmpty() ? QStringLiteral("原报警登录字段无效。") : error); return; } QDialog::accept();
}

