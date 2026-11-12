#include "bit_button_dialog.h"
#include "hmi_property_widgets.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : BitButtonDialog::createMultiGeneral
 * Description     : Build independent unsigned monitor/write carriers, bounded state count and six action drafts.
 * Input Parameters: None
 * Return Value    : Native multi-state general page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *BitButtonDialog::createMultiGeneral() //20261112-0 add
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    QGroupBox *family = new QGroupBox(page);
    QGridLayout *grid = new QGridLayout(family);
    QRadioButton *radio;
    QGroupBox *data;
    QGroupBox *actions;
    QHBoxLayout *columns;
    QHBoxLayout *row;
    QPushButton *monitorPicker;
    QStringList names = {QStringLiteral("位按钮"), QStringLiteral("字按钮"), QStringLiteral("画面按钮"), QStringLiteral("功能按钮"), QStringLiteral("多态按钮"), QStringLiteral("指示灯")};
    int index;
    for (index = 0; index < names.size(); ++index)
    {
        radio = new QRadioButton(names.at(index), family); radio->setChecked(index == 4); radio->setObjectName(QStringLiteral("MultiFamily%1").arg(index));
        if (index == 4) { radio->setEnabled(false); } else { radio->setAutoExclusive(false); connect(radio, &QRadioButton::clicked, radio, [radio]() { radio->setChecked(false); }); }
        grid->addWidget(radio, index / 3, index % 3);
    }
    layout->addWidget(family); columns = new QHBoxLayout(); data = new QGroupBox(QStringLiteral("数据"), page); grid = new QGridLayout(data);
    m_multiDataType = new QComboBox(data); m_multiDataType->setObjectName(QStringLiteral("MultiDataType")); m_multiDataType->addItems({QStringLiteral("16位正整数"), QStringLiteral("32位正整数")}); m_multiDataType->setCurrentIndex(m_working.multiStateButton.dataType); grid->addWidget(new QLabel(QStringLiteral("数据类别:"), data), 0, 0); grid->addWidget(m_multiDataType, 0, 1);
    m_multiMonitor = new QLineEdit(m_working.multiStateButton.monitorAddress, data); m_multiMonitor->setObjectName(QStringLiteral("MultiMonitor")); monitorPicker = new QPushButton(QStringLiteral("…"), data); grid->addWidget(new QLabel(QStringLiteral("监视地址:"), data), 1, 0); grid->addWidget(m_multiMonitor, 1, 1); grid->addWidget(monitorPicker, 1, 2);
    m_multiSame = new QCheckBox(QStringLiteral("写入地址和监视地址相同"), data); m_multiSame->setObjectName(QStringLiteral("MultiSameAddress")); m_multiSame->setChecked(m_working.multiStateButton.sameAddress); grid->addWidget(m_multiSame, 2, 0, 1, 3);
    m_multiWrite = new QLineEdit(m_working.multiStateButton.writeAddress, data); m_multiWrite->setObjectName(QStringLiteral("MultiWrite")); m_multiWritePicker = new QPushButton(QStringLiteral("…"), data); grid->addWidget(new QLabel(QStringLiteral("写入地址:"), data), 3, 0); grid->addWidget(m_multiWrite, 3, 1); grid->addWidget(m_multiWritePicker, 3, 2);
    m_multiCount = new QSpinBox(data); m_multiCount->setObjectName(QStringLiteral("MultiStateCount")); m_multiCount->setRange(2, 32); m_multiCount->setValue(m_working.multiStateButton.stateCount); grid->addWidget(new QLabel(QStringLiteral("总状态数:"), data), 4, 0); grid->addWidget(m_multiCount, 4, 1); columns->addWidget(data, 2);
    actions = new QGroupBox(QStringLiteral("功能"), page); grid = new QGridLayout(actions); m_multiActions = new QButtonGroup(this); m_multiActions->setExclusive(true); names = MultiStateButtonProperties::actions();
    for (index = 0; index < names.size(); ++index) { radio = new QRadioButton(names.at(index), actions); radio->setObjectName(QStringLiteral("MultiAction%1").arg(index)); m_multiActions->addButton(radio, index); radio->setChecked(index == m_working.multiStateButton.action); grid->addWidget(radio, index, 0); }
    columns->addWidget(actions, 1); layout->addLayout(columns); layout->addStretch(); m_macro = new QGroupBox(QStringLiteral("宏指令"), page); m_macro->setObjectName(QStringLiteral("BitMacro")); m_macro->setCheckable(true); m_macro->setChecked(false); row = new QHBoxLayout(m_macro); row->addWidget(new QLabel(QStringLiteral("使用宏:"), m_macro)); m_macroList = new QComboBox(m_macro); row->addWidget(m_macroList); row->addWidget(new QPushButton(QStringLiteral("写宏"), m_macro)); m_macro->setEnabled(false); layout->addWidget(m_macro);
    m_address = new QLineEdit(page); m_address->hide(); m_operation = new QComboBox(page); m_operation->addItems(BitButtonProperties::operations()); m_operation->setCurrentIndex(m_working.bitButton.operation); m_operation->hide();
    connect(m_multiMonitor, &QLineEdit::textChanged, this, [this]() { refreshMultiAddresses(); }); connect(m_multiSame, &QCheckBox::toggled, this, [this]() { refreshMultiAddresses(); }); connect(m_multiCount, &QSpinBox::valueChanged, this, &BitButtonDialog::resizeMultiStates);
    connect(monitorPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_multiMonitor, false, m_multiDataType->currentIndex() == 0 ? 1 : 2); }); connect(m_multiWritePicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_multiWrite, false, m_multiDataType->currentIndex() == 0 ? 1 : 2); }); refreshMultiAddresses();
    return page;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::refreshMultiAddresses
 * Description     : Mirror writes while same-address is checked and retain the last mirrored value on uncheck, as documented.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void BitButtonDialog::refreshMultiAddresses() //20261112-0 add
{
    if (m_multiSame->isChecked()) { m_multiWrite->setText(m_multiMonitor->text()); }
    m_multiWrite->setEnabled(!m_multiSame->isChecked()); m_multiWritePicker->setEnabled(!m_multiSame->isChecked());
}

/******************************************************************************
 * Function Name   : BitButtonDialog::resizeMultiStates
 * Description     : Commit the visible draft before deleting truncated states, then clamp selection without copying deleted values into surviving states.
 * Input Parameters: count
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void BitButtonDialog::resizeMultiStates(int count) //20261112-0 add
{
    if (m_loading || m_stateSpin == nullptr) { return; }
    storeState(); m_working.multiStateButton.setStateCount(count); m_state = qMin(m_state, count - 1);
    { QSignalBlocker blocker(m_stateSpin); m_stateSpin->setRange(0, count - 1); m_stateSpin->setValue(m_state); }
    loadState();
}
