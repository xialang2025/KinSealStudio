#include "data_collection_dialog.h"
#include "hmi_property_widgets.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSet>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QUuid>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : DataCollectionDialog::DataCollectionDialog
 * Description     : Edit a detached named collection and preserve existing identities and inactive drafts.
 * Input Parameters: node, project, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
DataCollectionDialog::DataCollectionDialog(const HmiConfigurationNode &node, const HmiProject &project, QWidget *parent) //20261112-1 add
    : QDialog(parent), m_original(node), m_project(project)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QTabWidget *tabs = new QTabWidget(this);
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    QString error;
    if (!node.properties.isEmpty()) { m_initialValid = DataCollectionProperties::fromJson(node.properties, &m_draft, &error); }
    if (m_original.id.isEmpty()) { m_original.id = QStringLiteral("collection-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)); }
    setObjectName(QStringLiteral("DataCollectionDialog")); setWindowTitle(QStringLiteral("数据采集")); resize(595, 720);
    setStyleSheet(QStringLiteral("QLineEdit, QComboBox, QSpinBox {background:#caeace; min-height:22px;} QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled {background:#b1ceed;} QTableWidget {background:#caeace;}"));
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    tabs->setObjectName(QStringLiteral("CollectionTabs")); tabs->addTab(basicPage(), QStringLiteral("一般")); tabs->addTab(itemsPage(), QStringLiteral("数据项")); layout->addWidget(tabs);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("CollectionButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &DataCollectionDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("数据采集帮助"), QStringLiteral("定义取样长度、总数、共用数据类别及取样方式；数据项使用固定LH序号，名称和显示格式独立。采集内存上限为63 kbyte。")); }); refresh();
}

/******************************************************************************
 * Function Name   : DataCollectionDialog::basicPage
 * Description     : Reproduce collection-wide memory, sampling-mode and full-buffer notification controls.
 * Input Parameters: None
 * Return Value    : General collection page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *DataCollectionDialog::basicPage() //20261112-1 add
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    QGridLayout *nameRow = new QGridLayout;
    QGroupBox *memory = new QGroupBox(QStringLiteral("内存"), page);
    QGridLayout *grid = new QGridLayout(memory);
    QLabel *formula = new QLabel(QStringLiteral("备注：单位字节，所需内存=(20+取样长度×数据类型参数)×取样总数/1024。\n20为存储采集时间日期；16位类型参数为2，32位类型参数为4。\n\n该系列触摸屏：数据采集内存总数不能超过63 kbyte。"), memory);
    QGridLayout *addressRow = new QGridLayout;
    QHBoxLayout *bottom = new QHBoxLayout;
    QGroupBox *mode = new QGroupBox(QStringLiteral("取样方式"), page);
    QGridLayout *modeGrid = new QGridLayout(mode);
    QGroupBox *full = new QGroupBox(QStringLiteral("取满处理"), page);
    QGridLayout *fullGrid = new QGridLayout(full);
    QButtonGroup *group = new QButtonGroup(this);
    QRadioButton *radio;
    QWidget *timer = new QWidget(mode);
    QWidget *trigger = new QWidget(mode);
    QWidget *clock = new QWidget(mode);
    QGridLayout *sub;
    QCheckBox *notice;
    QCheckBox *upload;
    QSpinBox *sampleCount;
    QWidget *sampleCountLabel;
    QLineEdit *noticeAddress;
    QPushButton *noticePicker;
    QSpinBox *percent;
    QStringList modes = {QStringLiteral("计时"), QStringLiteral("触发"), QStringLiteral("整点")};
    int index;
    m_name = new QLineEdit(m_original.name, page); m_name->setObjectName(QStringLiteral("CollectionName")); m_name->setMaxLength(48); nameRow->addWidget(new QLabel(QStringLiteral("名称:"), page), 0, 0); nameRow->addWidget(m_name, 0, 1); upload = toggle(nameRow, 1, QStringLiteral("上传云端"), QStringLiteral("CollectionUpload"), &m_draft.upload); nameRow->removeWidget(upload); nameRow->addWidget(upload, 0, 2); layout->addLayout(nameRow);
    m_length = integer(grid, 0, QStringLiteral("取样长度:"), QStringLiteral("CollectionLength"), &m_draft.sampleLength, 1, 32); sampleCount = integer(grid, 1, QStringLiteral("取样总数:"), QStringLiteral("CollectionCount"), &m_draft.sampleCount, 1, 65535); sampleCountLabel = grid->itemAtPosition(1, 0)->widget(); grid->removeWidget(sampleCount); grid->removeWidget(sampleCountLabel); grid->addWidget(sampleCountLabel, 0, 2); grid->addWidget(sampleCount, 0, 3);
    m_memory = new QLineEdit(memory); m_memory->setObjectName(QStringLiteral("CollectionMemory")); m_memory->setReadOnly(true); grid->addWidget(new QLabel(QStringLiteral("所需内存:"), memory), 2, 0); grid->addWidget(m_memory, 2, 1); formula->setWordWrap(true); formula->setMinimumHeight(220); grid->addWidget(formula, 3, 0, 1, 4); layout->addWidget(memory);
    textField(addressRow, 0, QStringLiteral("取样地址:"), QStringLiteral("CollectionReadAddress"), &m_draft.readAddress, 2); layout->addLayout(addressRow);
    for (index = 0; index < 3; ++index) { radio = new QRadioButton(modes.at(index), mode); radio->setObjectName(QStringLiteral("CollectionMode%1").arg(index)); group->addButton(radio, index); radio->setChecked(m_draft.samplingMode == index); modeGrid->addWidget(radio, index, 0); }
    sub = new QGridLayout(timer); integer(sub, 0, QStringLiteral("时间（秒）:"), QStringLiteral("CollectionInterval"), &m_draft.interval, 1, 3600); modeGrid->addWidget(timer, 0, 1);
    sub = new QGridLayout(trigger); textField(sub, 0, QStringLiteral("触发位:"), QStringLiteral("CollectionTriggerAddress"), &m_draft.triggerAddress, 1); modeGrid->addWidget(trigger, 1, 1);
    sub = new QGridLayout(clock); integer(sub, 0, QStringLiteral("于（分钟）:"), QStringLiteral("CollectionTriggerMinute"), &m_draft.triggerMinute, 1, 255); modeGrid->addWidget(clock, 2, 1); timer->setVisible(m_draft.samplingMode == 0); trigger->setVisible(m_draft.samplingMode == 1); clock->setVisible(m_draft.samplingMode == 2);
    connect(group, &QButtonGroup::idClicked, this, [this, timer, trigger, clock](int value) { m_draft.samplingMode = value; timer->setVisible(value == 0); trigger->setVisible(value == 1); clock->setVisible(value == 2); });
    toggle(fullGrid, 0, QStringLiteral("停止取样"), QStringLiteral("CollectionFullStop"), &m_draft.fullStop); notice = toggle(fullGrid, 1, QStringLiteral("通知"), QStringLiteral("CollectionNotice"), &m_draft.notice); noticeAddress = textField(fullGrid, 2, QString(), QStringLiteral("CollectionNoticeAddress"), &m_draft.noticeAddress, 1); percent = integer(fullGrid, 3, QStringLiteral("满界限（%）:"), QStringLiteral("CollectionFullPercent"), &m_draft.fullPercent, 0, 100); noticePicker = findChild<QPushButton *>(QStringLiteral("CollectionNoticeAddressPicker")); noticeAddress->setEnabled(m_draft.notice); noticePicker->setEnabled(m_draft.notice); percent->setEnabled(m_draft.notice);
    connect(notice, &QCheckBox::toggled, this, [noticeAddress, noticePicker, percent](bool checked) { noticeAddress->setEnabled(checked); noticePicker->setEnabled(checked); percent->setEnabled(checked); }); bottom->addWidget(mode, 2); bottom->addWidget(full, 1); layout->addLayout(bottom); connect(m_length, &QSpinBox::valueChanged, this, &DataCollectionDialog::changeLength); return page;
}

/******************************************************************************
 * Function Name   : DataCollectionDialog::itemsPage
 * Description     : Keep cache indices read-only and bind editable item names/formats while sharing one collection data type.
 * Input Parameters: None
 * Return Value    : Item configuration page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *DataCollectionDialog::itemsPage() //20261112-1 add
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    QGroupBox *properties = new QGroupBox(QStringLiteral("数据项属性"), page);
    QGridLayout *grid = new QGridLayout(properties);
    QComboBox *type = new QComboBox(properties);
    m_items = new QTableWidget(0, 1, page); m_items->setObjectName(QStringLiteral("CollectionItems")); m_items->setHorizontalHeaderLabels({QStringLiteral("名称")}); m_items->horizontalHeader()->setStretchLastSection(true); m_items->setSelectionBehavior(QAbstractItemView::SelectRows); m_items->setSelectionMode(QAbstractItemView::SingleSelection); m_items->setFixedHeight(240); layout->addWidget(m_items);
    m_cache = new QLineEdit(properties); m_cache->setObjectName(QStringLiteral("CollectionCacheAddress")); m_cache->setReadOnly(true); grid->addWidget(new QLabel(QStringLiteral("地址:"), properties), 0, 0); grid->addWidget(m_cache, 0, 1);
    m_itemName = new QLineEdit(properties); m_itemName->setObjectName(QStringLiteral("CollectionItemName")); m_itemName->setMaxLength(48); grid->addWidget(new QLabel(QStringLiteral("名称:"), properties), 1, 0); grid->addWidget(m_itemName, 1, 1);
    m_digits = new QSpinBox(properties); m_digits->setObjectName(QStringLiteral("CollectionItemDigits")); m_digits->setRange(1, 10); grid->addWidget(new QLabel(QStringLiteral("总位数:"), properties), 2, 0); grid->addWidget(m_digits, 2, 1); m_fraction = new QSpinBox(properties); m_fraction->setObjectName(QStringLiteral("CollectionItemFraction")); m_fraction->setRange(0, 9); grid->addWidget(new QLabel(QStringLiteral("小数位数:"), properties), 3, 0); grid->addWidget(m_fraction, 3, 1);
    type->setObjectName(QStringLiteral("CollectionDataType")); type->addItems(DataCollectionProperties::dataTypes()); type->setCurrentIndex(m_draft.dataType); grid->addWidget(new QLabel(QStringLiteral("数据类别:"), properties), 0, 2); grid->addWidget(type, 0, 3); connect(type, &QComboBox::currentIndexChanged, this, [this](int value) { m_draft.dataType = value; refresh(); });
    m_format = new QComboBox(properties); m_format->setObjectName(QStringLiteral("CollectionItemFormat")); m_format->addItems(DataCollectionProperties::displayTypes()); grid->addWidget(new QLabel(QStringLiteral("显示类别:"), properties), 1, 2); grid->addWidget(m_format, 1, 3); m_scaled = new QCheckBox(QStringLiteral("缩放"), properties); m_scaled->setObjectName(QStringLiteral("CollectionItemScaled")); grid->addWidget(m_scaled, 2, 2, 1, 2);
    m_gain = new QLineEdit(properties); m_gain->setObjectName(QStringLiteral("CollectionItemGain")); m_gain->setMaxLength(80); grid->addWidget(new QLabel(QStringLiteral("增益:"), properties), 3, 2); grid->addWidget(m_gain, 3, 3); m_offset = new QLineEdit(properties); m_offset->setObjectName(QStringLiteral("CollectionItemOffset")); m_offset->setMaxLength(80); grid->addWidget(new QLabel(QStringLiteral("偏移量:"), properties), 4, 2); grid->addWidget(m_offset, 4, 3); layout->addWidget(properties); layout->addStretch();
    connect(m_items, &QTableWidget::currentCellChanged, this, [this](int row) { showItem(row); });
    connect(m_itemName, &QLineEdit::textChanged, this, [this](const QString &name) { int row = m_items->currentRow(); if (row >= 0 && row < m_draft.items.size()) { m_draft.items[row].name = name; m_items->item(row, 0)->setText(name); } });
    connect(m_digits, &QSpinBox::valueChanged, this, [this](int value) { int row = m_items->currentRow(); if (row >= 0) { m_draft.items[row].digits = value; } }); connect(m_fraction, &QSpinBox::valueChanged, this, [this](int value) { int row = m_items->currentRow(); if (row >= 0) { m_draft.items[row].fractionalDigits = value; } }); connect(m_format, &QComboBox::currentIndexChanged, this, [this](int value) { int row = m_items->currentRow(); if (row >= 0) { m_draft.items[row].displayType = value; } });
    connect(m_scaled, &QCheckBox::toggled, this, [this](bool checked) { int row = m_items->currentRow(); if (row >= 0) { m_draft.items[row].scaled = checked; } m_gain->setEnabled(checked); m_offset->setEnabled(checked); }); connect(m_gain, &QLineEdit::textChanged, this, [this](const QString &text) { int row = m_items->currentRow(); if (row >= 0) { m_draft.items[row].gain = text; } }); connect(m_offset, &QLineEdit::textChanged, this, [this](const QString &text) { int row = m_items->currentRow(); if (row >= 0) { m_draft.items[row].offset = text; } }); rebuildItems(); return page;
}

/******************************************************************************
 * Function Name   : DataCollectionDialog::refresh
 * Description     : Display the observed floor-kilobyte memory estimate without publishing document edits.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void DataCollectionDialog::refresh() //20261112-1 add
{
    if (m_memory != nullptr) { m_memory->setText(QStringLiteral("%1 kbyte").arg(m_draft.memoryBytes() / 1024U)); }
}

/******************************************************************************
 * Function Name   : DataCollectionDialog::wouldRemoveReferencedItem
 * Description     : Protect active curve item identities on every page before shortening a sampled collection.
 * Input Parameters: count
 * Return Value    : True when the proposed length would orphan an existing history curve.
 * Date            : 2026.11.12
 ******************************************************************************/
bool DataCollectionDialog::wouldRemoveReferencedItem(int count) const //20261112-1 add
{
    QSet<QString> removed;
    HmiPage page;
    HmiObject object;
    int index;
    for (index = count; index < m_draft.items.size(); ++index) { removed.insert(m_draft.items[index].id); }
    Q_FOREACH (page, m_project.allPages())
    {
        Q_FOREACH (object, page.objects)
        {
            if (object.type != QStringLiteral("historytrend") || object.historyTrend.collectionId != m_original.id) { continue; }
            if (object.historyTrend.curveCount > count) { return true; }
            for (index = 0; index < object.historyTrend.curveCount; ++index) { if (removed.contains(object.historyTrend.curves[index].sourceItemId)) { return true; } }
        }
    }
    return false;
}

/******************************************************************************
 * Function Name   : DataCollectionDialog::changeLength
 * Description     : Preserve retained item identities, reset removed rows, and reject active-reference loss atomically.
 * Input Parameters: count
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void DataCollectionDialog::changeLength(int count) //20261112-1 add
{
    int oldCount = int(m_draft.items.size());
    int index;
    if (count < oldCount && wouldRemoveReferencedItem(count))
    {
        QSignalBlocker blocker(m_length);
        m_draft.sampleLength = oldCount; m_length->setValue(oldCount); refresh(); QMessageBox::warning(this, QStringLiteral("数据项正在使用"), QStringLiteral("历史趋势图仍引用将被移除的数据项，请先更换或删除对应曲线。")); return;
    }
    m_draft.sampleLength = count;
    if (count < oldCount) { m_draft.items.resize(count); }
    else { for (index = oldCount; index < count; ++index) { m_draft.items.append(DataCollectionProperties::createItem(index)); } }
    rebuildItems(); refresh();
}

/******************************************************************************
 * Function Name   : DataCollectionDialog::rebuildItems
 * Description     : Refresh the sampled cache table after length changes without sending partial item writes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void DataCollectionDialog::rebuildItems() //20261112-1 add
{
    QSignalBlocker blocker(m_items);
    QTableWidgetItem *cell;
    QStringList headers;
    int index;
    int selected = qBound(0, m_items->currentRow(), int(m_draft.items.size()) - 1);
    m_items->setRowCount(int(m_draft.items.size()));
    for (index = 0; index < m_draft.items.size(); ++index) { cell = new QTableWidgetItem(m_draft.items[index].name); cell->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled); m_items->setItem(index, 0, cell); headers.append(QStringLiteral("LH%1").arg(index)); }
    m_items->setVerticalHeaderLabels(headers); m_items->setCurrentCell(selected, 0); showItem(selected);
}

/******************************************************************************
 * Function Name   : DataCollectionDialog::showItem
 * Description     : Refill per-item fields without overwriting another row or resetting the shared collection type.
 * Input Parameters: row
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void DataCollectionDialog::showItem(int row) //20261112-1 add
{
    QSignalBlocker name(m_itemName);
    QSignalBlocker format(m_format);
    QSignalBlocker digits(m_digits);
    QSignalBlocker fraction(m_fraction);
    QSignalBlocker scaled(m_scaled);
    QSignalBlocker gain(m_gain);
    QSignalBlocker offset(m_offset);
    DataCollectionItem item;
    if (row < 0 || row >= m_draft.items.size()) { return; }
    item = m_draft.items[row]; m_cache->setText(QStringLiteral("LH%1").arg(item.cacheIndex)); m_itemName->setText(item.name); m_format->setCurrentIndex(item.displayType); m_digits->setValue(item.digits); m_fraction->setValue(item.fractionalDigits); m_scaled->setChecked(item.scaled); m_gain->setText(item.gain); m_offset->setText(item.offset); m_gain->setEnabled(item.scaled); m_offset->setEnabled(item.scaled);
}

/******************************************************************************
 * Function Name   : DataCollectionDialog::node
 * Description     : Return one complete definition retaining its stable node ID and inactive extension metadata.
 * Input Parameters: None
 * Return Value    : Detached active collection configuration.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiConfigurationNode DataCollectionDialog::node() const //20261112-1 add
{
    HmiConfigurationNode result = m_original;
    result.kind = QStringLiteral("dataCollection"); result.name = m_name->text().trimmed(); result.ownerPageId.clear(); result.implemented = true; result.active = true; result.properties = m_draft.toJson(); return result;
}

/******************************************************************************
 * Function Name   : DataCollectionDialog::accept
 * Description     : Validate the whole proposed project and the observed single-collection target limit before acceptance.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void DataCollectionDialog::accept() //20261112-1 add
{
    HmiConfigurationNode result = node();
    HmiProject candidate = m_project;
    HmiConfigurationNode peer;
    QString error;
    int index;
    int existing = -1;
    if (!m_initialValid) { QMessageBox::warning(this, QStringLiteral("配置未提交"), QStringLiteral("原有数据采集属性无效，已保留原始配置。")); return; }
    if (result.name.isEmpty() || result.name.size() > 48 || !m_draft.validate(&error) || !m_draft.validateAddresses(m_project.connections, &error)) { QMessageBox::warning(this, QStringLiteral("配置未提交"), error.isEmpty() ? QStringLiteral("数据采集名称不能为空且不能超过48个字符。") : error); return; }
    for (index = 0; index < candidate.configurationNodes.size(); ++index)
    {
        peer = candidate.configurationNodes[index];
        if (peer.id == result.id) { existing = index; }
        else if (peer.kind == QStringLiteral("dataCollection") && peer.implemented && peer.active) { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), QStringLiteral("已经达到最大数据采集数量，请删除后再新建。")); return; }
    }
    if (existing < 0) { candidate.configurationNodes.append(result); } else { candidate.configurationNodes[existing] = result; }
    if (!candidate.validateModel(&error)) { QMessageBox::warning(this, QStringLiteral("配置未提交"), error); return; } QDialog::accept();
}

/******************************************************************************
 * Function Name   : DataCollectionDialog::textField
 * Description     : Bind a detached string and optional full-span word-address picker.
 * Input Parameters: layout, row, label, name, value, addressRole
 * Return Value    : Bound line edit.
 * Date            : 2026.11.12
 ******************************************************************************/
QLineEdit *DataCollectionDialog::textField(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value, int addressRole) //20261112-1 add
{
    QLineEdit *edit = new QLineEdit(*value, this);
    QPushButton *picker;
    edit->setMaximumWidth(addressRole ? 210 : 140); edit->setObjectName(name); edit->setMaxLength(addressRole ? 1024 : 80); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(edit, row, 1);
    connect(edit, &QLineEdit::textChanged, this, [this, value](const QString &text) { *value = text; refresh(); });
    if (addressRole != 0) { picker = new QPushButton(QStringLiteral("…"), this); picker->setFixedWidth(26); picker->setObjectName(name + QStringLiteral("Picker")); layout->addWidget(picker, row, 2); connect(picker, &QPushButton::clicked, this, [this, edit, addressRole]() { editHmiAddress(this, edit, addressRole == 1, addressRole == 1 ? 1 : int(m_draft.wordCount()) * m_draft.sampleLength); }); }
    return edit;
}



/******************************************************************************
 * Function Name   : DataCollectionDialog::toggle
 * Description     : Bind a persistent checkbox to its detached boolean.
 * Input Parameters: layout, row, label, name, value
 * Return Value    : Bound checkbox.
 * Date            : 2026.11.12
 ******************************************************************************/
QCheckBox *DataCollectionDialog::toggle(QGridLayout *layout, int row, const QString &label, const QString &name, bool *value) //20261112-1 add
{
    QCheckBox *check = new QCheckBox(label, this);
    check->setObjectName(name); check->setChecked(*value); layout->addWidget(check, row, 0, 1, 2); connect(check, &QCheckBox::toggled, this, [this, value](bool checked) { *value = checked; refresh(); }); return check;
}



/******************************************************************************
 * Function Name   : DataCollectionDialog::integer
 * Description     : Bind bounded editor integers with explicit Lite limits.
 * Input Parameters: layout, row, label, name, value, minimum, maximum
 * Return Value    : Bounded spin editor.
 * Date            : 2026.11.12
 ******************************************************************************/
QSpinBox *DataCollectionDialog::integer(QGridLayout *layout, int row, const QString &label, const QString &name, int *value, int minimum, int maximum) //20261112-1 add
{
    QSpinBox *spin = new QSpinBox(this);
    spin->setObjectName(name); spin->setRange(minimum, maximum); spin->setValue(*value); layout->addWidget(new QLabel(label, this), row, 0); layout->addWidget(spin, row, 1); connect(spin, &QSpinBox::valueChanged, this, [this, value](int number) { *value = number; refresh(); });
    return spin;
}


