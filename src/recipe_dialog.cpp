#include "recipe_dialog.h"
#include "recipe_csv.h"
#include "hmi_property_widgets.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSaveFile>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QUuid>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : RecipeDialog::RecipeDialog
 * Description     : Edit a detached real recipe definition with stable node identity and project-aware validation.
 * Input Parameters: node, project, parent
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
RecipeDialog::RecipeDialog(const HmiConfigurationNode &node, const HmiProject &project, QWidget *parent)
    : QDialog(parent), m_original(node), m_project(project) //20261113-2 add
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QTabWidget *tabs = new QTabWidget(this);
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    QString error;
    if (!node.properties.isEmpty()) { m_initialValid = RecipeProperties::fromJson(node.properties, &m_draft, &error); }
    if (m_original.id.isEmpty()) { m_original.id = QStringLiteral("recipe-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)); }
    setObjectName(QStringLiteral("RecipeDialog")); setWindowTitle(QStringLiteral("配方")); resize(568, 660); setFont(QFont(QStringLiteral("SimSun"), 9)); setStyleSheet(QStringLiteral("QLineEdit, QComboBox, QSpinBox {background:#caeace; min-height:22px;} QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled {background:#b1ceed;} QTableWidget {background:#caeace;}")); Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    tabs->setObjectName(QStringLiteral("RecipeDefinitionTabs")); tabs->addTab(basicPage(), QStringLiteral("一般")); tabs->addTab(dataPage(), QStringLiteral("数据项")); layout->addWidget(tabs); buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("RecipeDefinitionButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons); connect(buttons, &QDialogButtonBox::accepted, this, &RecipeDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("配方帮助"), QStringLiteral("设置配方长度、总数和类型，在数据项页选择单元格后编辑名称、元素名称和数值。复制编号从0开始，只复制数值。CSV只交换当前可见数据。")); }); refreshRanges(); rebuildTable();
}

/******************************************************************************
 * Function Name   : RecipeDialog::basicPage
 * Description     : Build measured name, dimensions, type, full-span read/write options and readonly memory ranges.
 * Input Parameters: None
 * Return Value    : General recipe definition page.
 * Date            : 2026.11.13
 ******************************************************************************/
QWidget *RecipeDialog::basicPage() //20261113-2 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *dimensions = new QGroupBox(page);
    QGridLayout *grid = new QGridLayout(dimensions);
    QPushButton *writePicker = new QPushButton(QStringLiteral("…"), page);
    QLineEdit *registerNumber = new QLineEdit(QStringLiteral("RWI0"), page);
    QPushButton *exportButton = new QPushButton(QStringLiteral("导出配方"), page);
    QPushButton *importButton = new QPushButton(QStringLiteral("导入配方"), page);
    m_name = new QLineEdit(m_original.name, page); m_name->setObjectName(QStringLiteral("RecipeName")); m_name->setMaxLength(48); m_description = new QLineEdit(m_draft.description, page); m_description->setObjectName(QStringLiteral("RecipeDescription")); m_description->setMaxLength(32767); layout->addWidget(new QLabel(QStringLiteral("名称:"), page), 0, 0); layout->addWidget(m_name, 0, 1, 1, 2); layout->addWidget(new QLabel(QStringLiteral("描述:"), page), 1, 0); layout->addWidget(m_description, 1, 1, 1, 2);
    m_length = new QSpinBox(dimensions); m_length->setObjectName(QStringLiteral("RecipeLength")); m_length->setRange(1, RecipeProperties::maximumLength(m_draft.dataType)); m_length->setValue(m_draft.length); m_count = new QSpinBox(dimensions); m_count->setObjectName(QStringLiteral("RecipeCount")); m_count->setRange(1, 200); m_count->setValue(m_draft.count); m_type = new QComboBox(dimensions); m_type->setObjectName(QStringLiteral("RecipeType")); m_type->addItems(RecipeProperties::dataTypes()); m_type->setCurrentIndex(m_draft.dataType); grid->addWidget(new QLabel(QStringLiteral("配方长度:"), dimensions), 0, 0); grid->addWidget(m_length, 0, 1); grid->addWidget(new QLabel(QStringLiteral("配方总数:"), dimensions), 1, 0); grid->addWidget(m_count, 1, 1); grid->addWidget(new QLabel(QStringLiteral("数据类型:"), dimensions), 2, 0); grid->addWidget(m_type, 2, 1); layout->addWidget(dimensions, 2, 0, 1, 3);
    m_write = new QCheckBox(QStringLiteral("写配方到PLC"), page); m_write->setObjectName(QStringLiteral("RecipeWrite")); m_write->setChecked(m_draft.writeEnabled); m_writeAddress = new QLineEdit(m_draft.writeAddress, page); m_writeAddress->setObjectName(QStringLiteral("RecipeWriteAddress")); m_writeAddress->setMaxLength(1024); writePicker->setObjectName(QStringLiteral("RecipeWritePicker")); writePicker->setFixedWidth(28); layout->addWidget(m_write, 3, 0, 1, 3); layout->addWidget(new QLabel(QStringLiteral("写入地址:"), page), 4, 0); layout->addWidget(m_writeAddress, 4, 1); layout->addWidget(writePicker, 4, 2);
    m_read = new QCheckBox(QStringLiteral("从PLC读取配方"), page); m_read->setObjectName(QStringLiteral("RecipeRead")); m_read->setChecked(m_draft.readEnabled); m_same = new QCheckBox(QStringLiteral("读取地址与写入地址相同"), page); m_same->setObjectName(QStringLiteral("RecipeSameAddress")); m_same->setChecked(m_draft.sameReadWrite); m_readAddress = new QLineEdit(m_draft.readAddress, page); m_readAddress->setObjectName(QStringLiteral("RecipeReadAddress")); m_readAddress->setMaxLength(1024); m_readPicker = new QPushButton(QStringLiteral("…"), page); m_readPicker->setObjectName(QStringLiteral("RecipeReadPicker")); m_readPicker->setFixedWidth(28); layout->addWidget(m_read, 5, 0, 1, 3); layout->addWidget(m_same, 6, 0, 1, 3); layout->addWidget(new QLabel(QStringLiteral("读取地址:"), page), 7, 0); layout->addWidget(m_readAddress, 7, 1); layout->addWidget(m_readPicker, 7, 2);
    m_memory = new QLineEdit(page); m_memory->setObjectName(QStringLiteral("RecipeMemory")); m_memory->setReadOnly(true); m_current = new QLineEdit(page); m_current->setObjectName(QStringLiteral("RecipeCurrent")); m_current->setReadOnly(true); registerNumber->setObjectName(QStringLiteral("RecipeRegister")); registerNumber->setReadOnly(true); layout->addWidget(new QLabel(QStringLiteral("配方记忆体:"), page), 8, 0); layout->addWidget(m_memory, 8, 1, 1, 2); layout->addWidget(new QLabel(QStringLiteral("当前配方:"), page), 9, 0); layout->addWidget(m_current, 9, 1, 1, 2); layout->addWidget(new QLabel(QStringLiteral("配方编号寄存器:"), page), 10, 0); layout->addWidget(registerNumber, 10, 1, 1, 2); exportButton->setObjectName(QStringLiteral("RecipeExport")); importButton->setObjectName(QStringLiteral("RecipeImport")); layout->addWidget(exportButton, 11, 0); layout->addWidget(importButton, 11, 1); layout->setRowStretch(12, 1);
    connect(m_description, &QLineEdit::textChanged, this, [this](const QString &value) { if (!m_updating) { m_draft.description = value; } }); connect(m_length, &QSpinBox::valueChanged, this, [this](int value) { if (m_updating) { return; } m_draft.resizeTable(value, m_draft.count); refreshRanges(); rebuildTable(); }); connect(m_count, &QSpinBox::valueChanged, this, [this](int value) { if (m_updating) { return; } m_draft.resizeTable(m_draft.length, value); refreshRanges(); rebuildTable(); }); connect(m_type, &QComboBox::currentIndexChanged, this, [this](int value) { if (m_updating) { return; } m_draft.changeType(value); m_updating = true; m_length->setMaximum(RecipeProperties::maximumLength(value)); m_length->setValue(m_draft.length); m_updating = false; refreshRanges(); rebuildTable(); });
    connect(m_write, &QCheckBox::toggled, this, [this](bool value) { if (!m_updating) { m_draft.writeEnabled = value; refreshRanges(); } }); connect(m_read, &QCheckBox::toggled, this, [this](bool value) { if (!m_updating) { m_draft.readEnabled = value; refreshRanges(); } }); connect(m_same, &QCheckBox::toggled, this, [this](bool value) { if (!m_updating) { m_draft.sameReadWrite = value; refreshRanges(); } }); connect(m_writeAddress, &QLineEdit::textChanged, this, [this](const QString &value) { if (!m_updating) { m_draft.writeAddress = value; } }); connect(m_readAddress, &QLineEdit::textChanged, this, [this](const QString &value) { if (!m_updating) { m_draft.readAddress = value; } }); connect(writePicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_writeAddress, false, m_draft.length * m_draft.wordSpan()); }); connect(m_readPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_readAddress, false, m_draft.length * m_draft.wordSpan()); }); connect(exportButton, &QPushButton::clicked, this, &RecipeDialog::exportCsv); connect(importButton, &QPushButton::clicked, this, &RecipeDialog::importCsv); return page;
}

/******************************************************************************
 * Function Name   : RecipeDialog::dataPage
 * Description     : Edit selected recipe/name/value fields and the measured zero-based value-only column copy operation.
 * Input Parameters: None
 * Return Value    : Recipe matrix page.
 * Date            : 2026.11.13
 ******************************************************************************/
QWidget *RecipeDialog::dataPage() //20261113-2 add
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    QGroupBox *fields = new QGroupBox(page);
    QGridLayout *grid = new QGridLayout(fields);
    QComboBox *language = new QComboBox(fields);
    QPushButton *copy = new QPushButton(QStringLiteral("配方数据复制"), fields);
    m_table = new QTableWidget(page); m_table->setObjectName(QStringLiteral("RecipeValues")); m_table->setSelectionMode(QAbstractItemView::SingleSelection); m_table->setEditTriggers(QAbstractItemView::NoEditTriggers); m_table->setMinimumHeight(360); layout->addWidget(m_table, 1); language->setObjectName(QStringLiteral("RecipeLanguage")); language->addItems(m_project.languages); language->setCurrentText(m_project.activeLanguage); m_recipeCaption = new QLineEdit(fields); m_recipeCaption->setObjectName(QStringLiteral("RecipeColumnName")); m_itemCaption = new QLineEdit(fields); m_itemCaption->setObjectName(QStringLiteral("RecipeItemName")); m_value = new QLineEdit(fields); m_value->setObjectName(QStringLiteral("RecipeValue")); m_recipeCaption->setMaxLength(32767); m_itemCaption->setMaxLength(32767); m_value->setMaxLength(128); m_source = new QSpinBox(fields); m_source->setObjectName(QStringLiteral("RecipeCopySource")); m_target = new QSpinBox(fields); m_target->setObjectName(QStringLiteral("RecipeCopyTarget")); copy->setObjectName(QStringLiteral("RecipeCopyValues"));
    grid->addWidget(new QLabel(QStringLiteral("语言:"), fields), 0, 0); grid->addWidget(language, 0, 1); grid->addWidget(new QLabel(QStringLiteral("名称:"), fields), 1, 0); grid->addWidget(m_recipeCaption, 1, 1); grid->addWidget(new QLabel(QStringLiteral("元素名称:"), fields), 2, 0); grid->addWidget(m_itemCaption, 2, 1); grid->addWidget(new QLabel(QStringLiteral("数据:"), fields), 3, 0); grid->addWidget(m_value, 3, 1); grid->addWidget(new QLabel(QStringLiteral("源配方编号:"), fields), 0, 2); grid->addWidget(m_source, 0, 3); grid->addWidget(new QLabel(QStringLiteral("目标配方编号:"), fields), 1, 2); grid->addWidget(m_target, 1, 3); grid->addWidget(copy, 3, 2, 1, 2); layout->addWidget(fields);
    connect(m_table, &QTableWidget::currentCellChanged, this, [this](int row, int column, int, int) { showCell(row, column); }); connect(language, &QComboBox::currentIndexChanged, this, [this]() { showCell(m_table->currentRow(), m_table->currentColumn()); });
    connect(m_recipeCaption, &QLineEdit::textChanged, this, [this](const QString &value) { int column = m_table->currentColumn(); if (column >= 0 && !m_updating) { m_draft.recipeNames[column] = value; m_table->horizontalHeaderItem(column)->setText(value); } }); connect(m_itemCaption, &QLineEdit::textChanged, this, [this](const QString &value) { int row = m_table->currentRow(); if (row >= 0 && !m_updating) { m_draft.itemNames[row] = value; m_table->verticalHeaderItem(row)->setText(value); } }); connect(m_value, &QLineEdit::textChanged, this, [this](const QString &value) { int row = m_table->currentRow(); int column = m_table->currentColumn(); if (row >= 0 && column >= 0 && !m_updating) { m_draft.values[row][column] = value; m_table->item(row, column)->setText(value); } }); connect(copy, &QPushButton::clicked, this, [this]() { m_draft.copyValues(m_source->value(), m_target->value()); rebuildTable(); }); return page;
}

/******************************************************************************
 * Function Name   : RecipeDialog::refreshRanges
 * Description     : Refresh readonly RW/RWI labels and measured read/same/write enablement without changing disabled drafts.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void RecipeDialog::refreshRanges() //20261113-2 add
{
    m_memory->setText(QStringLiteral("RW1 - RW%1").arg(m_draft.length * m_draft.count * m_draft.wordSpan())); m_current->setText(QStringLiteral("RWI%1 - RWI%2").arg(m_draft.wordSpan()).arg(m_draft.length * m_draft.wordSpan())); m_writeAddress->setEnabled(m_draft.writeEnabled); m_same->setEnabled(m_draft.readEnabled); m_readAddress->setEnabled(m_draft.readEnabled && !m_draft.sameReadWrite); m_readPicker->setEnabled(m_draft.readEnabled && !m_draft.sameReadWrite);
}

/******************************************************************************
 * Function Name   : RecipeDialog::rebuildTable
 * Description     : Expose only active dimensions while keeping hidden cache cells and user header names intact.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void RecipeDialog::rebuildTable() //20261113-2 add
{
    QSignalBlocker blocker(m_table);
    int row;
    int column;
    int selectedRow;
    int selectedColumn;
    if (m_table == nullptr) { return; }
    selectedRow = qBound(0, m_table->currentRow(), m_draft.length - 1); selectedColumn = qBound(0, m_table->currentColumn(), m_draft.count - 1); m_table->setRowCount(m_draft.length); m_table->setColumnCount(m_draft.count); m_table->setHorizontalHeaderLabels(m_draft.recipeNames.mid(0, m_draft.count)); m_table->setVerticalHeaderLabels(m_draft.itemNames.mid(0, m_draft.length));
    for (row = 0; row < m_draft.length; ++row) { for (column = 0; column < m_draft.count; ++column) { m_table->setItem(row, column, new QTableWidgetItem(m_draft.values[row][column])); } }
    m_source->setRange(0, m_draft.count - 1); m_target->setRange(0, m_draft.count - 1); m_table->setCurrentCell(selectedRow, selectedColumn); showCell(selectedRow, selectedColumn);
}

/******************************************************************************
 * Function Name   : RecipeDialog::showCell
 * Description     : Refill one selected recipe header, item header and value under signal blockers.
 * Input Parameters: row, column
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void RecipeDialog::showCell(int row, int column) //20261113-2 add
{
    QSignalBlocker recipe(m_recipeCaption);
    QSignalBlocker item(m_itemCaption);
    QSignalBlocker value(m_value);
    bool selected = row >= 0 && row < m_draft.length && column >= 0 && column < m_draft.count;
    m_recipeCaption->setEnabled(selected); m_itemCaption->setEnabled(selected); m_value->setEnabled(selected); m_recipeCaption->setText(selected ? m_draft.recipeNames[column] : QString()); m_itemCaption->setText(selected ? m_draft.itemNames[row] : QString()); m_value->setText(selected ? m_draft.values[row][column] : QString());
}

/******************************************************************************
 * Function Name   : RecipeDialog::refill
 * Description     : Reflect an imported detached definition without firing size/type transformations or publishing project state.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void RecipeDialog::refill() //20261113-2 add
{
    m_updating = true; m_description->setText(m_draft.description); m_type->setCurrentIndex(m_draft.dataType); m_length->setMaximum(RecipeProperties::maximumLength(m_draft.dataType)); m_length->setValue(m_draft.length); m_count->setValue(m_draft.count); m_write->setChecked(m_draft.writeEnabled); m_read->setChecked(m_draft.readEnabled); m_same->setChecked(m_draft.sameReadWrite); m_writeAddress->setText(m_draft.writeAddress); m_readAddress->setText(m_draft.readAddress); m_updating = false; refreshRanges(); rebuildTable();
}

/******************************************************************************
 * Function Name   : RecipeDialog::importCsv
 * Description     : Read a bounded selected native CSV into a local candidate and preserve the current draft on any failure.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void RecipeDialog::importCsv() //20261113-2 add
{
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("导入配方"), QString(), QStringLiteral("配方CSV (*.csv)"));
    QFile file(path);
    RecipeProperties candidate;
    QString name;
    QString error;
    if (path.isEmpty()) { return; }
    if (!file.open(QIODevice::ReadOnly) || file.size() > 8 * 1024 * 1024 || !importRecipeCsv(file.readAll(), &name, &candidate, &error)) { QMessageBox::warning(this, QStringLiteral("配方导入失败"), error.isEmpty() ? file.errorString() : error); return; } m_draft = candidate; m_name->setText(name); refill();
}

/******************************************************************************
 * Function Name   : RecipeDialog::exportCsv
 * Description     : Atomically save only a completely encoded native-format CSV chosen by the user.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void RecipeDialog::exportCsv() //20261113-2 add
{
    QByteArray bytes;
    QString error;
    QString path;
    QSaveFile file;
    if (!exportRecipeCsv(m_name->text().trimmed(), m_draft, &bytes, &error)) { QMessageBox::warning(this, QStringLiteral("配方导出失败"), error); return; }
    path = QFileDialog::getSaveFileName(this, QStringLiteral("导出配方"), m_name->text().trimmed() + QStringLiteral(".csv"), QStringLiteral("配方CSV (*.csv)")); if (path.isEmpty()) { return; } file.setFileName(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) { QMessageBox::warning(this, QStringLiteral("配方导出失败"), file.errorString()); }
}

/******************************************************************************
 * Function Name   : RecipeDialog::node
 * Description     : Return a complete detached recipe node with its stable identity and inactive extension metadata.
 * Input Parameters: None
 * Return Value    : Detached complete recipe definition.
 * Date            : 2026.11.13
 ******************************************************************************/
HmiConfigurationNode RecipeDialog::node() const //20261113-2 add
{
    HmiConfigurationNode result = m_original;
    result.kind = QStringLiteral("recipe"); result.name = m_name->text().trimmed(); result.ownerPageId.clear(); result.implemented = true; result.active = true; result.properties = m_draft.toJson(); return result;
}

/******************************************************************************
 * Function Name   : RecipeDialog::accept
 * Description     : Validate one complete candidate project before closing the recipe definition editor.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void RecipeDialog::accept() //20261113-2 add
{
    HmiProject candidate = m_project;
    HmiConfigurationNode result = node();
    QString error;
    int index;
    bool found = false;
    if (!m_initialValid) { QMessageBox::warning(this, QStringLiteral("配方未提交"), QStringLiteral("原有配方属性无效，已保留原始配置。")); return; }
    if (result.name.isEmpty()) { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), QStringLiteral("配方名称不能为空")); return; }
    for (index = 0; index < candidate.configurationNodes.size(); ++index) { if (candidate.configurationNodes[index].id == result.id) { candidate.configurationNodes[index] = result; found = true; break; } }
    if (!found) { candidate.configurationNodes.append(result); }
    if (!m_draft.validate(&error) || !m_draft.validateAddresses(candidate.connections, &error) || !candidate.validateModel(&error)) { QMessageBox::warning(this, QStringLiteral("配方未提交"), error); return; } QDialog::accept();
}
