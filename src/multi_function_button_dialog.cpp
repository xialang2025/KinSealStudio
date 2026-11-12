#include "bit_button_dialog.h"
#include "hmi_property_widgets.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : BitButtonDialog::createMfGeneral
 * Description     : Build the observed ordered command list with three mutually exclusive parameter groups.
 * Input Parameters: None
 * Return Value    : MF basic settings page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *BitButtonDialog::createMfGeneral() //20261112-0 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGridLayout *form;
    QHBoxLayout *row;
    QPushButton *bitPicker;
    QPushButton *wordPicker;
    QPushButton *addBit;
    QPushButton *addScreen;
    QPushButton *addWord;
    QPushButton *remove;
    MultiFunctionCommand command;
    m_mfBitGroup = new QGroupBox(QStringLiteral("线圈地址设置"), page); m_mfBitGroup->setObjectName(QStringLiteral("MfBitGroup")); form = new QGridLayout(m_mfBitGroup);
    m_mfBitAddress = new QLineEdit(QStringLiteral("LB0"), m_mfBitGroup); m_mfBitAddress->setObjectName(QStringLiteral("MfBitAddress")); bitPicker = new QPushButton(QStringLiteral("…"), m_mfBitGroup);
    m_mfBitOperation = new QComboBox(m_mfBitGroup); m_mfBitOperation->setObjectName(QStringLiteral("MfBitOperation")); m_mfBitOperation->addItems(MultiFunctionCommand::bitOperations());
    form->addWidget(new QLabel(QStringLiteral("位地址:"), m_mfBitGroup), 0, 0); form->addWidget(m_mfBitAddress, 0, 1); form->addWidget(bitPicker, 0, 2); form->addWidget(new QLabel(QStringLiteral("操作类型:"), m_mfBitGroup), 1, 0); form->addWidget(m_mfBitOperation, 1, 1, 1, 2); layout->addWidget(m_mfBitGroup, 0, 0);
    m_mfScreenGroup = new QGroupBox(QStringLiteral("画面操作设置"), page); m_mfScreenGroup->setObjectName(QStringLiteral("MfScreenGroup")); form = new QGridLayout(m_mfScreenGroup);
    m_mfScreenOperation = new QComboBox(m_mfScreenGroup); m_mfScreenOperation->setObjectName(QStringLiteral("MfScreenOperation")); m_mfScreenOperation->addItems(ScreenButtonProperties::actions()); m_mfTarget = new QComboBox(m_mfScreenGroup); m_mfTarget->setObjectName(QStringLiteral("MfTarget"));
    form->addWidget(new QLabel(QStringLiteral("操作类型:"), m_mfScreenGroup), 0, 0); form->addWidget(m_mfScreenOperation, 0, 1); form->addWidget(new QLabel(QStringLiteral("名称:"), m_mfScreenGroup), 1, 0); form->addWidget(m_mfTarget, 1, 1); layout->addWidget(m_mfScreenGroup, 1, 0);
    m_mfWordGroup = new QGroupBox(QStringLiteral("字地址设置"), page); m_mfWordGroup->setObjectName(QStringLiteral("MfWordGroup")); form = new QGridLayout(m_mfWordGroup);
    m_mfWordAddress = new QLineEdit(QStringLiteral("LW0"), m_mfWordGroup); m_mfWordAddress->setObjectName(QStringLiteral("MfWordAddress")); wordPicker = new QPushButton(QStringLiteral("…"), m_mfWordGroup); m_mfDataType = new QComboBox(m_mfWordGroup); m_mfDataType->setObjectName(QStringLiteral("MfDataType")); m_mfDataType->addItems(MultiFunctionCommand::dataTypes());
    m_mfWordOperation = new QComboBox(m_mfWordGroup); m_mfWordOperation->setObjectName(QStringLiteral("MfWordOperation")); m_mfWordOperation->addItems(MultiFunctionCommand::wordOperations()); m_mfWordOperation->setCurrentIndex(1);
    form->addWidget(new QLabel(QStringLiteral("字地址:"), m_mfWordGroup), 0, 0); form->addWidget(m_mfWordAddress, 0, 1); form->addWidget(wordPicker, 0, 2); form->addWidget(new QLabel(QStringLiteral("数据类型:"), m_mfWordGroup), 1, 0); form->addWidget(m_mfDataType, 1, 1, 1, 2); form->addWidget(new QLabel(QStringLiteral("操作类型:"), m_mfWordGroup), 2, 0); form->addWidget(m_mfWordOperation, 2, 1, 1, 2);
    m_mfValueRow = new QWidget(m_mfWordGroup); row = new QHBoxLayout(m_mfValueRow); row->setContentsMargins(0, 0, 0, 0); m_mfValue = new QLineEdit(QStringLiteral("1"), m_mfValueRow); m_mfValue->setObjectName(QStringLiteral("MfValue")); row->addWidget(new QLabel(QStringLiteral("数据值:"), m_mfValueRow)); row->addWidget(m_mfValue); form->addWidget(m_mfValueRow, 3, 0, 1, 3); layout->addWidget(m_mfWordGroup, 2, 0);
    m_mfList = new QListWidget(page); m_mfList->setObjectName(QStringLiteral("MfCommands")); layout->addWidget(new QLabel(QStringLiteral("功能列表:"), page), 0, 1); layout->addWidget(m_mfList, 1, 1, 2, 2);
    addBit = new QPushButton(QStringLiteral("添加线圈操作"), page); addBit->setObjectName(QStringLiteral("MfAddBit")); addScreen = new QPushButton(QStringLiteral("添加画面操作"), page); addScreen->setObjectName(QStringLiteral("MfAddScreen")); addWord = new QPushButton(QStringLiteral("添加字操作"), page); addWord->setObjectName(QStringLiteral("MfAddWord")); remove = new QPushButton(QStringLiteral("删除"), page); remove->setObjectName(QStringLiteral("MfRemove")); layout->addWidget(addBit, 3, 1); layout->addWidget(addScreen, 3, 2); layout->addWidget(addWord, 4, 1); layout->addWidget(remove, 4, 2);
    page->setStyleSheet(QStringLiteral("QLineEdit, QComboBox {background:#caeace;} QLineEdit:disabled, QComboBox:disabled {background:#b1ceed;}"));
    m_macro = new QGroupBox(QStringLiteral("使用宏指令"), page); m_macro->setObjectName(QStringLiteral("BitMacro")); m_macro->setCheckable(true); m_macro->setChecked(false); row = new QHBoxLayout(m_macro); row->addWidget(new QLabel(QStringLiteral("名称:"), m_macro)); m_macroList = new QComboBox(m_macro); row->addWidget(m_macroList); row->addWidget(new QPushButton(QStringLiteral("写宏"), m_macro)); m_macro->setEnabled(false); layout->addWidget(m_macro, 5, 0, 1, 3);
    m_address = new QLineEdit(page); m_address->hide(); m_operation = new QComboBox(page); m_operation->addItems(BitButtonProperties::operations()); m_operation->hide();
    Q_FOREACH (command, m_working.multiFunctionButton.commands) { m_mfList->addItem(command.label()); }
    connect(m_mfList, &QListWidget::currentRowChanged, this, &BitButtonDialog::loadMfCommand);
    connect(addBit, &QPushButton::clicked, this, [this]() { addMfCommand(0); }); connect(addScreen, &QPushButton::clicked, this, [this]() { addMfCommand(1); }); connect(addWord, &QPushButton::clicked, this, [this]() { addMfCommand(2); });
    connect(remove, &QPushButton::clicked, this, [this]() {
        int index = m_mfList->currentRow();
        if (index < 0) { return; }
        storeMfCommand(); m_mfIndex = -1;
        { QSignalBlocker blocker(m_mfList); m_working.multiFunctionButton.commands.removeAt(index); delete m_mfList->takeItem(index); m_mfList->setCurrentRow(qMin(index, m_mfList->count() - 1)); }
        loadMfCommand(m_mfList->currentRow());
    });
    connect(m_mfBitAddress, &QLineEdit::textChanged, this, [this]() { storeMfCommand(); }); connect(m_mfWordAddress, &QLineEdit::textChanged, this, [this]() { storeMfCommand(); }); connect(m_mfValue, &QLineEdit::textChanged, this, [this]() { storeMfCommand(); });
    connect(m_mfBitOperation, &QComboBox::currentIndexChanged, this, [this]() { storeMfCommand(); }); connect(m_mfDataType, &QComboBox::currentIndexChanged, this, [this]() { storeMfCommand(); });
    connect(m_mfWordOperation, &QComboBox::currentIndexChanged, this, [this]() { m_mfValueRow->setVisible(m_mfWordOperation->currentIndex() == 1); storeMfCommand(); });
    connect(m_mfScreenOperation, &QComboBox::currentIndexChanged, this, [this]() { if (!m_mfLoading) { refreshMfTarget(); storeMfCommand(); } }); connect(m_mfTarget, &QComboBox::currentIndexChanged, this, [this]() { storeMfCommand(); });
    connect(bitPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_mfBitAddress, true); }); connect(wordPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_mfWordAddress, false, m_mfDataType->currentIndex() == 1 || m_mfDataType->currentIndex() == 3 || m_mfDataType->currentIndex() >= 5 ? 2 : 1); });
    if (m_mfList->count() > 0) { m_mfList->setCurrentRow(0); } else { loadMfCommand(-1); }
    return page;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::storeMfCommand
 * Description     : Commit only the selected kind's controls and refresh its list caption.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void BitButtonDialog::storeMfCommand() //20261112-0 add
{
    MultiFunctionCommand *command;
    if (m_mfLoading || m_mfIndex < 0 || m_mfIndex >= m_working.multiFunctionButton.commands.size()) { return; }
    command = &m_working.multiFunctionButton.commands[m_mfIndex];
    if (command->kind == 0) { command->address = m_mfBitAddress->text().trimmed(); command->operation = m_mfBitOperation->currentIndex(); }
    if (command->kind == 2) { command->address = m_mfWordAddress->text().trimmed(); command->operation = m_mfWordOperation->currentIndex(); command->dataType = m_mfDataType->currentIndex(); command->value = m_mfValue->text(); }
    if (command->kind == 1)
    {
        command->operation = m_mfScreenOperation->currentIndex();
        if (command->operation == 0) { command->screenPageId = m_mfTarget->currentData().toString(); }
        if (command->operation == 2) { command->windowPageId = m_mfTarget->currentData().toString(); }
    }
    m_mfList->item(m_mfIndex)->setText(command->label());
}

/******************************************************************************
 * Function Name   : BitButtonDialog::loadMfCommand
 * Description     : Store the previous row before selecting and enabling exactly one kind-specific parameter group.
 * Input Parameters: index
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void BitButtonDialog::loadMfCommand(int index) //20261112-0 add
{
    MultiFunctionCommand command;
    int kind = -1;
    storeMfCommand(); m_mfLoading = true; m_mfIndex = index;
    if (index >= 0 && index < m_working.multiFunctionButton.commands.size())
    {
        command = m_working.multiFunctionButton.commands.at(index); kind = command.kind;
        if (kind == 0) { m_mfBitAddress->setText(command.address); m_mfBitOperation->setCurrentIndex(command.operation); }
        if (kind == 2) { m_mfWordAddress->setText(command.address); m_mfDataType->setCurrentIndex(command.dataType); m_mfWordOperation->setCurrentIndex(command.operation); m_mfValue->setText(command.value); }
        if (kind == 1) { m_mfScreenOperation->setCurrentIndex(command.operation); m_mfScreenDraft = command.operation; }
    }
    m_mfBitGroup->setEnabled(kind == 0); m_mfWordGroup->setEnabled(kind == 2); m_mfScreenGroup->setEnabled(kind == 1); m_mfValueRow->setVisible(m_mfWordOperation->currentIndex() == 1);
    if (kind == 1) { refreshMfTarget(); } m_mfLoading = false;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::addMfCommand
 * Description     : Append a fresh command in native defaults, silently ignoring additions after the seventh item.
 * Input Parameters: kind
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void BitButtonDialog::addMfCommand(int kind) //20261112-0 add
{
    MultiFunctionCommand command;
    if (m_working.multiFunctionButton.commands.size() >= 7) { return; }
    storeMfCommand(); command.kind = kind;
    if (kind == 2) { command.address = QStringLiteral("LW0"); command.operation = 1; }
    if (kind == 1) { command.address.clear(); }
    m_working.multiFunctionButton.commands.append(command); m_mfList->addItem(command.label()); m_mfList->setCurrentRow(m_mfList->count() - 1);
}

/******************************************************************************
 * Function Name   : BitButtonDialog::refreshMfTarget
 * Description     : Keep separate stable screen/window targets and preserve native empty initial selection.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void BitButtonDialog::refreshMfTarget() //20261112-0 add
{
    QSignalBlocker blocker(m_mfTarget);
    MultiFunctionCommand *command;
    HmiPage page;
    int action = m_mfScreenOperation->currentIndex();
    bool loading = m_mfLoading;
    if (m_mfIndex < 0 || m_mfIndex >= m_working.multiFunctionButton.commands.size()) { return; }
    command = &m_working.multiFunctionButton.commands[m_mfIndex];
    if (!loading)
    {
        if (m_mfScreenDraft == 0) { command->screenPageId = m_mfTarget->currentData().toString(); }
        if (m_mfScreenDraft == 2) { command->windowPageId = m_mfTarget->currentData().toString(); }
    }
    m_mfLoading = true; m_mfScreenDraft = action; m_mfTarget->clear();
    if (action == 0 || action == 2)
    {
        Q_FOREACH (page, m_project.allPages()) { if (page.kind == (action == 0 ? QStringLiteral("normal") : QStringLiteral("window"))) { m_mfTarget->addItem(page.name, page.id); } }
        m_mfTarget->setCurrentIndex(m_mfTarget->findData(action == 0 ? command->screenPageId : command->windowPageId));
    }
    m_mfTarget->setEnabled(action == 0 || action == 2); m_mfLoading = loading;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::refreshMfAppearance
 * Description     : Gate second-state editing and copy buttons while retaining dormant second-state drafts in Lite.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void BitButtonDialog::refreshMfAppearance() //20261112-0 add
{
    bool enabled;
    if (m_mfPressedEffect == nullptr || m_copyText == nullptr) { return; }
    enabled = m_mfPressedEffect->isChecked(); m_stateSpin->setEnabled(enabled); m_copyText->setEnabled(enabled); m_copyShape->setEnabled(enabled);
    if (!enabled && m_stateSpin->value() != 0) { m_stateSpin->setValue(0); }
    m_stateSpin->setRange(0, enabled ? 1 : 0);
}
