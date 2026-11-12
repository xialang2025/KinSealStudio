#include "bit_button_dialog.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : BitButtonDialog::createScreenGeneral
 * Description     : Build native screen navigation and user-level drafts while reusing the other three button pages.
 * Input Parameters: None
 * Return Value    : Independent screen-button general page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *BitButtonDialog::createScreenGeneral() //20261112-0 add
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    QGroupBox *family = new QGroupBox(page);
    QGridLayout *types = new QGridLayout(family);
    QGroupBox *settings;
    QGridLayout *form;
    QHBoxLayout *row;
    QRadioButton *radio;
    QPushButton *write;
    QStringList names = {QStringLiteral("位按钮"), QStringLiteral("字按钮"), QStringLiteral("画面按钮"), QStringLiteral("功能按钮"), QStringLiteral("多态按钮"), QStringLiteral("指示灯")};
    int index;
    for (index = 0; index < names.size(); ++index)
    {
        radio = new QRadioButton(names.at(index), family); radio->setObjectName(QStringLiteral("ScreenFamily%1").arg(index)); radio->setChecked(index == 2);
        if (index == 2) { radio->setEnabled(false); } else { radio->setAutoExclusive(false); connect(radio, &QRadioButton::clicked, radio, [radio]() { radio->setChecked(false); }); }
        types->addWidget(radio, index / 3, index % 3);
    }
    layout->addWidget(family); settings = new QGroupBox(QStringLiteral("操作设置"), page); form = new QGridLayout(settings);
    m_screenAction = new QComboBox(settings); m_screenAction->setObjectName(QStringLiteral("ScreenAction")); m_screenAction->addItems(ScreenButtonProperties::actions()); m_screenAction->setCurrentIndex(m_working.screenButton.action); m_screenActionDraft = m_working.screenButton.action;
    form->addWidget(new QLabel(QStringLiteral("操作类型:"), settings), 0, 0); form->addWidget(m_screenAction, 0, 1);
    m_screenTargetRow = new QWidget(settings); row = new QHBoxLayout(m_screenTargetRow); row->setContentsMargins(0, 0, 0, 0); m_screenTargetLabel = new QLabel(QStringLiteral("画面名称:"), m_screenTargetRow); m_screenTargetLabel->setObjectName(QStringLiteral("ScreenTargetLabel")); m_screenTarget = new QComboBox(m_screenTargetRow); m_screenTarget->setObjectName(QStringLiteral("ScreenTarget")); row->addWidget(m_screenTargetLabel); row->addWidget(m_screenTarget, 1); form->addWidget(m_screenTargetRow, 1, 0, 1, 2);
    m_screenChangeLevel = new QGroupBox(QStringLiteral("改变用户等级（注销）"), settings); m_screenChangeLevel->setObjectName(QStringLiteral("ScreenChangeLevel")); m_screenChangeLevel->setCheckable(true); m_screenChangeLevel->setChecked(m_working.screenButton.changeLevel); row = new QHBoxLayout(m_screenChangeLevel); m_screenNewLevel = new QComboBox(m_screenChangeLevel); m_screenNewLevel->setObjectName(QStringLiteral("ScreenNewLevel"));
    for (index = 1; index <= 8; ++index) { m_screenNewLevel->addItem(QString::number(index), index); } m_screenNewLevel->setCurrentIndex(m_working.screenButton.newLevel - 1); row->addWidget(new QLabel(QStringLiteral("新用户等级:"), m_screenChangeLevel)); row->addWidget(m_screenNewLevel); form->addWidget(m_screenChangeLevel, 2, 0, 1, 2);
    m_screenRequireLogin = new QCheckBox(QStringLiteral("用户等级不足时要求登录"), settings); m_screenRequireLogin->setObjectName(QStringLiteral("ScreenRequireLogin")); m_screenRequireLogin->setChecked(m_working.screenButton.requireLogin); form->addWidget(m_screenRequireLogin, 3, 0, 1, 2); layout->addWidget(settings); layout->addStretch();
    m_macro = new QGroupBox(QStringLiteral("宏指令"), page); m_macro->setObjectName(QStringLiteral("BitMacro")); m_macro->setCheckable(true); m_macro->setChecked(false); row = new QHBoxLayout(m_macro); row->addWidget(new QLabel(QStringLiteral("使用宏:"), m_macro)); m_macroList = new QComboBox(m_macro); m_macroList->setObjectName(QStringLiteral("BitMacroList")); row->addWidget(m_macroList); write = new QPushButton(QStringLiteral("写宏"), m_macro); row->addWidget(write); m_macro->setEnabled(false); layout->addWidget(m_macro);
    m_address = new QLineEdit(page); m_address->hide(); m_operation = new QComboBox(page); m_operation->addItems(BitButtonProperties::operations()); m_operation->setCurrentIndex(m_working.bitButton.operation); m_operation->hide(); // Retain the shared shell's dormant fields without exposing an invented navigation address.
    connect(m_screenAction, &QComboBox::currentIndexChanged, this, [this]() { refreshScreenTarget(); }); refreshScreenTarget();
    return page;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::refreshScreenTarget
 * Description     : Keep independent screen/window target IDs and reproduce the observed window-label residue within a dialog.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void BitButtonDialog::refreshScreenTarget() //20261112-0 add
{
    QSignalBlocker blocker(m_screenTarget);
    HmiPage page;
    QString target;
    int action = m_screenAction->currentIndex();
    int selected;
    if (m_screenTarget->currentIndex() >= 0)
    {
        if (m_screenActionDraft == 0) { m_working.screenButton.screenPageId = m_screenTarget->currentData().toString(); }
        if (m_screenActionDraft == 2) { m_working.screenButton.windowPageId = m_screenTarget->currentData().toString(); }
    }
    m_screenActionDraft = action; m_screenTargetRow->setVisible(action == 0 || action == 2);
    if (action == 1 || action == 3) { return; }
    if (action == 2) { m_screenTargetLabel->setText(QStringLiteral("窗口名称:")); }
    m_screenTarget->clear(); target = action == 0 ? m_working.screenButton.screenPageId : m_working.screenButton.windowPageId;
    Q_FOREACH (page, m_project.allPages()) { if (page.kind == (action == 0 ? QStringLiteral("normal") : QStringLiteral("window"))) { m_screenTarget->addItem(page.name, page.id); } }
    selected = m_screenTarget->findData(target); if (target.isEmpty() && m_screenTarget->count() > 0) { selected = 0; } m_screenTarget->setCurrentIndex(selected);
}
