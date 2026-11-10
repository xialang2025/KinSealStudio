#include "bit_button_dialog.h"
#include "bit_button_renderer.h"
#include "kvg_library.h" //20261109-0 add
#include "hmi_property_widgets.h"
#include <QBuffer>
#include <QButtonGroup> //20261110-0 add
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile> //20261109-0 add
#include <QFontComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QImageReader>
#include <QJsonArray> //20261109-0 add
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker> //20261109-1 add
#include <QSpinBox>
#include <QTabWidget>
#include <QTreeWidget>
#include <QTimer> //20261109-0 add
#include <QVBoxLayout>
#include <functional> //20261109-1 add

//20261108-15 add: Display the current draft independently of canvas selection or external registers.
class BitButtonPreview : public QWidget
{
public:
    BitButtonState state;
    QString language;
    /******************************************************************************
     * Function Name   : BitButtonPreview::BitButtonPreview
     * Description     : Reserve the documented preview area.
     * Input Parameters: parent
     * Return Value    : None
     * Date            : 2026.11.08
     ******************************************************************************/
    explicit BitButtonPreview(QWidget *parent) : QWidget(parent) { setFixedSize(150, 160); }
protected:
    /******************************************************************************
     * Function Name   : BitButtonPreview::paintEvent
     * Description     : Draw the selected state without inventing a live monitoring connection.
     * Input Parameters: event
     * Return Value    : None
     * Date            : 2026.11.08
     ******************************************************************************/
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), palette().window()); painter.setPen(palette().mid().color());
        painter.drawRect(rect().adjusted(0, 0, -1, -1));
        paintBitButton(&painter, QRectF(25, 30, 100, 100), state, language);
    }
};

/******************************************************************************
 * Function Name   : BitButtonDialog::BitButtonDialog
 * Description     : Build the four-page property transaction and keep the preview shared between pages.
 * Input Parameters: object, project, parent
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
BitButtonDialog::BitButtonDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261108-15 add
    : QDialog(parent), m_working(object), m_project(project)
{
    QGridLayout *layout;
    QTabWidget *tabs;
    QGroupBox *preview;
    QVBoxLayout *previewLayout;
    QDialogButtonBox *buttons;
    QJsonArray connectionsJson; //20261109-0 add
    PlcConnection connection;

    Q_FOREACH (connection, project.connections)
    {
        connectionsJson.append(connection.toJson());
    }
    setProperty("plcConnections", connectionsJson.toVariantList());

    setWindowTitle(m_working.hasFunctionButtonProperties ? QStringLiteral("功能按钮") : //20261110-0 change
                   m_working.hasLampProperties ? QStringLiteral("位指示灯") :
                   m_working.hasWordButtonProperties ? QStringLiteral("字按钮") : QStringLiteral("位按钮")); //20261109-8 change
    setObjectName(QStringLiteral("BitButtonDialog")); resize(820, 578);
    m_working.type = QStringLiteral("switch"); m_working.hasBitButtonProperties = true;
    if (m_working.address.isEmpty() && !m_working.hasLampProperties && !m_working.hasFunctionButtonProperties) { m_working.address = m_working.hasWordButtonProperties ? QStringLiteral("LW0") : QStringLiteral("LB0"); } //20261110-0 change
    if (m_working.hasWordButtonProperties || m_working.hasFunctionButtonProperties) //20261110-0 change: Address-free system functions never create read monitors.
    {
        m_working.bitButton.indicator = false;
        m_working.bitButton.dualMonitor = false;
    }
    if (m_project.languages.isEmpty()) { m_project.languages.append(QStringLiteral("中文")); }
    m_language = m_project.activeLanguage;
    if (!m_project.languages.contains(m_language)) { m_language = m_project.languages.first(); }
    layout = new QGridLayout(this); layout->setContentsMargins(10, 10, 10, 10);
    tabs = new QTabWidget(this); tabs->setObjectName(QStringLiteral("BitButtonTabs"));
    tabs->addTab(createGeneral(), QStringLiteral("通用设置"));
    tabs->addTab(createMonitor(), QStringLiteral("指示灯设置"));
    tabs->addTab(createAppearance(), QStringLiteral("外观设置"));
    tabs->addTab(createPermissions(), QStringLiteral("权限设置"));
    if (m_working.hasLampProperties) { tabs->setCurrentIndex(1); } //20261109-8 add: Native lamp entry opens the monitoring page first.
    layout->addWidget(tabs, 0, 0); layout->setColumnStretch(0, 1);
    preview = new QGroupBox(QStringLiteral("预览"), this); previewLayout = new QVBoxLayout(preview);
    m_preview = new BitButtonPreview(preview); previewLayout->addWidget(m_preview);
    layout->addWidget(preview, 0, 1, Qt::AlignTop);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助"));
    layout->addWidget(buttons, 1, 0, 1, 2);
    connect(buttons, &QDialogButtonBox::accepted, this, &BitButtonDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() {
        if (m_working.hasFunctionButtonProperties) //20261110-0 add
        {
            QMessageBox::information(this, QStringLiteral("功能按钮帮助"),
                QStringLiteral("选择系统功能及对应类型；键盘组件可选择按键或输入字符。\n"
                               "外观固定为状态0，可配置按下效果、显现与触控条件、通知及按压时间。"));
            return;
        }
        if (m_working.hasLampProperties) //20261109-8 add
        {
            QMessageBox::information(this, QStringLiteral("指示灯帮助"),
                QStringLiteral("双态监视显示位地址或字中一位的状态，多态监视显示字地址的状态。\n"
                               "指示灯仅监视，不执行按钮写入操作。可在外观页编辑各状态，在权限页配置显现和通知。"));
            return;
        }
        if (m_working.hasWordButtonProperties) //20261109-6 add
        {
            QMessageBox::information(this, QStringLiteral("字按钮帮助"),
                QStringLiteral("设置字地址、数据类型和操作类型；按下效果允许编辑状态1。\n"
                               "七种操作的设备运行语义与编译兼容性需由原厂样本和实机另行验证。"));
            return;
        }
        QMessageBox::information(this, QStringLiteral("位按钮帮助"),
            QStringLiteral("设置位地址和操作类型，分别配置状态0与状态1，确定后在画布拖框。\n"
                           "指示灯可使用操作地址、独立位地址或字地址中的一位。\n"
                           "右键的模拟状态仅切换编辑外观。下载前会检查设备编译范围，传输后请核对实机显示与触控。")); //20261109-0 change
    });
    m_loading = false; loadState(); refreshMonitor();
}

/******************************************************************************
 * Function Name   : BitButtonDialog::createGeneral
 * Description     : Show the locked bit-button family, operation address and transactional macro source editor.
 * Input Parameters: None
 * Return Value    : General page.
 * Date            : 2026.11.08
 ******************************************************************************/
QWidget *BitButtonDialog::createGeneral() //20261108-15 add
{
    QWidget *page;
    QVBoxLayout *layout;
    QGroupBox *family;
    QGridLayout *familyLayout;
    QRadioButton *radio;
    QStringList names;
    QHBoxLayout *row;
    QGroupBox *address;
    QHBoxLayout *addressLayout;
    QPushButton *picker;
    QGroupBox *operation;
    QHBoxLayout *operationLayout;
    QHBoxLayout *macroLayout;
    QPushButton *write;
    int index;

    if (m_working.hasFunctionButtonProperties) { return createFunctionGeneral(); } //20261110-0 add
    if (m_working.hasWordButtonProperties) { return createWordGeneral(); } //20261109-6 add
    page = new QWidget(this); layout = new QVBoxLayout(page); layout->setContentsMargins(12, 20, 12, 10);
    family = new QGroupBox(page); familyLayout = new QGridLayout(family); family->setMinimumHeight(100);
    names = {QStringLiteral("位按钮"), QStringLiteral("字按钮"), QStringLiteral("画面按钮"),
             QStringLiteral("功能按钮"), QStringLiteral("多态按钮"), QStringLiteral("指示灯")};
    for (index = 0; index < names.size(); ++index)
    {
        radio = new QRadioButton(names.at(index), family); radio->setChecked(index == (m_working.hasLampProperties ? 5 : 0)); //20261109-8 change
        if (index == (m_working.hasLampProperties ? 5 : 0)) { radio->setEnabled(false); }
        else { radio->setAutoExclusive(false); connect(radio, &QRadioButton::clicked, radio, [radio]() { radio->setChecked(false); }); }
        familyLayout->addWidget(radio, index / 3, index % 3);
    }
    layout->addWidget(family); layout->addSpacing(20); row = new QHBoxLayout;
    address = new QGroupBox(QStringLiteral("地址"), page); addressLayout = new QHBoxLayout(address); address->setMinimumHeight(100);
    m_address = new QLineEdit(m_working.address, address); m_address->setObjectName(QStringLiteral("BitOperatingAddress"));
    picker = new QPushButton(QStringLiteral("…"), address); picker->setFixedWidth(28); picker->setObjectName(QStringLiteral("BitOperatingPicker"));
    addressLayout->addWidget(new QLabel(QStringLiteral("位地址:"), address)); addressLayout->addWidget(m_address); addressLayout->addWidget(picker);
    operation = new QGroupBox(QStringLiteral("操作"), page); operationLayout = new QHBoxLayout(operation); operation->setMinimumHeight(100);
    m_operation = new QComboBox(operation); m_operation->addItems(BitButtonProperties::operations()); m_operation->setCurrentIndex(m_working.bitButton.operation);
    m_operation->setObjectName(QStringLiteral("BitOperation")); operationLayout->addWidget(new QLabel(QStringLiteral("操作类型:"), operation)); operationLayout->addWidget(m_operation);
    row->addWidget(address, 1); row->addWidget(operation, 1); layout->addLayout(row); layout->addStretch(1);
    if (m_working.hasLampProperties) { address->hide(); operation->hide(); } //20261109-8 add: Keep hidden shared inputs out of the display-only general page.
    m_macro = new QGroupBox(QStringLiteral("宏指令"), page); m_macro->setCheckable(true); m_macro->setChecked(m_working.bitButton.macroEnabled);
    m_macro->setObjectName(QStringLiteral("BitMacro")); macroLayout = new QHBoxLayout(m_macro);
    m_macroList = new QComboBox(m_macro); m_macroList->addItems(m_project.macros.keys()); m_macroList->setObjectName(QStringLiteral("BitMacroList"));
    if (!m_working.bitButton.macroName.isEmpty() && !m_project.macros.contains(m_working.bitButton.macroName))
    {
        m_macroList->addItem(m_working.bitButton.macroName); //20261108-15 change: Keep missing references visible instead of selecting a different macro.
    }
    m_macroList->setCurrentText(m_working.bitButton.macroName); write = new QPushButton(QStringLiteral("写宏"), m_macro); write->setObjectName(QStringLiteral("BitWriteMacro"));
    macroLayout->addWidget(new QLabel(QStringLiteral("使用宏:"), m_macro)); macroLayout->addWidget(m_macroList, 1); macroLayout->addWidget(write);
    layout->addWidget(m_macro);
    if (m_working.hasLampProperties) { m_macro->setEnabled(false); } //20261109-8 add: Native lamp macros remain unavailable in this control family.
    connect(picker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_address, true); });
    connect(m_address, &QLineEdit::textChanged, this, [this]() { if (!m_loading) { refreshMonitor(); } });
    connect(write, &QPushButton::clicked, this, &BitButtonDialog::editMacro);
    return page;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::createFunctionGeneral
 * Description     : Edit native system-function/type selection without inventing an operating address.
 * Input Parameters: None
 * Return Value    : Function-button general page.
 * Date            : 2026.11.10
 ******************************************************************************/
QWidget *BitButtonDialog::createFunctionGeneral() //20261110-0 add
{
    QWidget *page;
    QVBoxLayout *layout;
    QGroupBox *family;
    QGridLayout *familyLayout;
    QRadioButton *radio;
    QStringList names;
    QGroupBox *settings;
    QGridLayout *settingsLayout;
    QHBoxLayout *typeLayout;
    QGridLayout *keyboardLayout;
    QHBoxLayout *macroLayout;
    QPushButton *write;
    int index;

    page = new QWidget(this); layout = new QVBoxLayout(page); layout->setContentsMargins(12, 20, 12, 10);
    family = new QGroupBox(page); familyLayout = new QGridLayout(family); family->setMinimumHeight(100);
    names = {QStringLiteral("位按钮"), QStringLiteral("字按钮"), QStringLiteral("画面按钮"),
             QStringLiteral("功能按钮"), QStringLiteral("多态按钮"), QStringLiteral("指示灯")};
    for (index = 0; index < names.size(); ++index)
    {
        radio = new QRadioButton(names.at(index), family); radio->setChecked(index == 3);
        if (index == 3) { radio->setEnabled(false); }
        else { radio->setAutoExclusive(false); connect(radio, &QRadioButton::clicked, radio, [radio]() { radio->setChecked(false); }); }
        familyLayout->addWidget(radio, index / 3, index % 3);
    }
    layout->addWidget(family); layout->addSpacing(20);
    settings = new QGroupBox(QStringLiteral("功能设置"), page); settingsLayout = new QGridLayout(settings);
    m_function = new QComboBox(settings); m_function->setObjectName(QStringLiteral("FunctionSelection"));
    m_function->addItems(FunctionButtonProperties::functions()); m_function->setCurrentIndex(m_working.functionButton.function);
    settingsLayout->addWidget(new QLabel(QStringLiteral("功能:"), settings), 0, 0); settingsLayout->addWidget(m_function, 0, 1);
    m_functionTypeField = new QWidget(settings); typeLayout = new QHBoxLayout(m_functionTypeField);
    typeLayout->setContentsMargins(0, 0, 0, 0); m_functionType = new QComboBox(m_functionTypeField);
    m_functionType->setObjectName(QStringLiteral("FunctionType"));
    typeLayout->addWidget(new QLabel(QStringLiteral("类型:"), m_functionTypeField)); typeLayout->addWidget(m_functionType, 1);
    settingsLayout->addWidget(m_functionTypeField, 1, 0, 1, 2);
    m_functionKeyboard = new QWidget(settings); m_functionKeyboard->setObjectName(QStringLiteral("FunctionKeyboard"));
    keyboardLayout = new QGridLayout(m_functionKeyboard); keyboardLayout->setContentsMargins(0, 0, 0, 0);
    m_functionKeys = new QButtonGroup(this); m_functionKeys->setExclusive(true);
    names = FunctionButtonProperties::keyboardKeys();
    for (index = 0; index < names.size(); ++index)
    {
        radio = new QRadioButton(names.at(index), m_functionKeyboard);
        radio->setObjectName(QStringLiteral("FunctionKey%1").arg(index));
        m_functionKeys->addButton(radio, index); radio->setChecked(index == m_working.functionButton.keyboardKey);
        keyboardLayout->addWidget(radio, index / 4, index % 4);
    }
    m_functionKeyboardText = new QLineEdit(m_working.functionButton.keyboardText, m_functionKeyboard);
    m_functionKeyboardText->setObjectName(QStringLiteral("FunctionKeyboardText"));
    m_functionKeyboardText->setMaxLength(1); //20261110-0 add: Native input stores one UTF-16 code unit.
    m_functionKeyboardText->setEnabled(true); // Native Qt getters keep this character draft editable for every selected key.
    keyboardLayout->addWidget(m_functionKeyboardText, 2, 2, 1, 2);
    settingsLayout->addWidget(m_functionKeyboard, 1, 0, 1, 2); layout->addWidget(settings); layout->addStretch(1);
    m_macro = new QGroupBox(QStringLiteral("宏指令"), page); m_macro->setCheckable(true); m_macro->setChecked(m_working.bitButton.macroEnabled);
    m_macro->setObjectName(QStringLiteral("BitMacro")); macroLayout = new QHBoxLayout(m_macro);
    m_macroList = new QComboBox(m_macro); m_macroList->addItems(m_project.macros.keys()); m_macroList->setObjectName(QStringLiteral("BitMacroList"));
    if (!m_working.bitButton.macroName.isEmpty() && !m_project.macros.contains(m_working.bitButton.macroName))
    {
        m_macroList->addItem(m_working.bitButton.macroName);
    }
    m_macroList->setCurrentText(m_working.bitButton.macroName);
    write = new QPushButton(QStringLiteral("写宏"), m_macro); write->setObjectName(QStringLiteral("BitWriteMacro"));
    macroLayout->addWidget(new QLabel(QStringLiteral("使用宏:"), m_macro)); macroLayout->addWidget(m_macroList, 1); macroLayout->addWidget(write);
    m_macro->setEnabled(false); // Native Qt property reads confirm the complete macro group is unavailable.
    layout->addWidget(m_macro);
    connect(write, &QPushButton::clicked, this, &BitButtonDialog::editMacro);
    connect(m_function, &QComboBox::currentIndexChanged, this, [this](int function) {
        if (m_loading) { return; }
        m_working.functionButton.function = function; m_working.functionButton.type = 0;
        refreshFunction(); // Every function change resets the dependent type to its first item.
    });
    refreshFunction();
    return page;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::refreshFunction
 * Description     : Rebuild native type choices and replace the type row with the ten-key panel when needed.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void BitButtonDialog::refreshFunction() //20261110-0 add
{
    QSignalBlocker blocker(m_functionType);

    m_functionType->clear();
    m_functionType->addItems(FunctionButtonProperties::types(m_function->currentIndex(), m_project.languages));
    if (m_functionType->count() > 0) { m_functionType->setCurrentIndex(m_working.functionButton.type); }
    m_functionTypeField->setVisible(m_function->currentIndex() != 14);
    m_functionKeyboard->setVisible(m_function->currentIndex() == 14);
}

/******************************************************************************
 * Function Name   : BitButtonDialog::createWordGeneral
 * Description     : Edit the seven word operations while retaining the shared macro draft.
 * Input Parameters: None
 * Return Value    : Word-button general page.
 * Date            : 2026.11.09
 ******************************************************************************/
QWidget *BitButtonDialog::createWordGeneral() //20261109-6 add
{
    QWidget *page;
    QVBoxLayout *layout;
    QGroupBox *family;
    QGridLayout *familyLayout;
    QRadioButton *radio;
    QStringList names;
    QGroupBox *address;
    QGridLayout *addressLayout;
    QPushButton *picker;
    QGroupBox *operation;
    QGridLayout *operationLayout;
    QHBoxLayout *fieldLayout;
    QHBoxLayout *macroLayout;
    QPushButton *write;
    int index;

    page = new QWidget(this); layout = new QVBoxLayout(page);
    layout->setContentsMargins(12, 20, 12, 10);
    family = new QGroupBox(page); familyLayout = new QGridLayout(family); family->setMinimumHeight(100);
    names = {QStringLiteral("位按钮"), QStringLiteral("字按钮"), QStringLiteral("画面按钮"),
             QStringLiteral("功能按钮"), QStringLiteral("多态按钮"), QStringLiteral("指示灯")};
    for (index = 0; index < names.size(); ++index)
    {
        radio = new QRadioButton(names.at(index), family); radio->setChecked(index == 1);
        if (index == 1) { radio->setEnabled(false); }
        else { radio->setAutoExclusive(false); connect(radio, &QRadioButton::clicked, radio, [radio]() { radio->setChecked(false); }); }
        familyLayout->addWidget(radio, index / 3, index % 3);
    }
    layout->addWidget(family);
    address = new QGroupBox(QStringLiteral("地址"), page); addressLayout = new QGridLayout(address);
    m_address = new QLineEdit(m_working.address, address); m_address->setObjectName(QStringLiteral("WordAddress"));
    picker = new QPushButton(QStringLiteral("…"), address); picker->setFixedWidth(28); picker->setObjectName(QStringLiteral("WordAddressPicker"));
    m_wordDataType = new QComboBox(address); m_wordDataType->addItems(WordButtonProperties::dataTypes());
    m_wordDataType->setObjectName(QStringLiteral("WordDataType")); m_wordDataType->setCurrentIndex(m_working.wordButton.dataType);
    addressLayout->addWidget(new QLabel(QStringLiteral("字地址:"), address), 0, 0); addressLayout->addWidget(m_address, 0, 1);
    addressLayout->addWidget(picker, 0, 2); addressLayout->addWidget(new QLabel(QStringLiteral("数据类型:"), address), 0, 3);
    addressLayout->addWidget(m_wordDataType, 0, 4); layout->addWidget(address);
    operation = new QGroupBox(QStringLiteral("操作"), page); operationLayout = new QGridLayout(operation);
    m_operation = new QComboBox(operation); m_operation->addItems(WordButtonProperties::operations());
    m_operation->setObjectName(QStringLiteral("WordOperation")); m_operation->setCurrentIndex(m_working.wordButton.operation);
    operationLayout->addWidget(new QLabel(QStringLiteral("操作类型:"), operation), 0, 0); operationLayout->addWidget(m_operation, 0, 1);
    m_wordConstantField = new QWidget(operation); fieldLayout = new QHBoxLayout(m_wordConstantField); fieldLayout->setContentsMargins(0, 0, 0, 0);
    m_wordConstant = new QLineEdit(m_wordConstantField); m_wordConstant->setObjectName(QStringLiteral("WordConstant"));
    fieldLayout->addWidget(new QLabel(QStringLiteral("常 数:"), m_wordConstantField)); fieldLayout->addWidget(m_wordConstant);
    operationLayout->addWidget(m_wordConstantField, 0, 2, 1, 2);
    m_wordMinimumField = new QWidget(operation); fieldLayout = new QHBoxLayout(m_wordMinimumField); fieldLayout->setContentsMargins(0, 0, 0, 0);
    m_wordMinimum = new QLineEdit(m_wordMinimumField); m_wordMinimum->setObjectName(QStringLiteral("WordMinimum"));
    fieldLayout->addWidget(new QLabel(QStringLiteral("最小值:"), m_wordMinimumField)); fieldLayout->addWidget(m_wordMinimum);
    operationLayout->addWidget(m_wordMinimumField, 1, 0, 1, 2);
    m_wordMaximumField = new QWidget(operation); fieldLayout = new QHBoxLayout(m_wordMaximumField); fieldLayout->setContentsMargins(0, 0, 0, 0);
    m_wordMaximum = new QLineEdit(m_wordMaximumField); m_wordMaximum->setObjectName(QStringLiteral("WordMaximum"));
    fieldLayout->addWidget(new QLabel(QStringLiteral("最大值:"), m_wordMaximumField)); fieldLayout->addWidget(m_wordMaximum);
    operationLayout->addWidget(m_wordMaximumField, 2, 0, 1, 2);
    m_wordTotalField = new QWidget(operation); fieldLayout = new QHBoxLayout(m_wordTotalField); fieldLayout->setContentsMargins(0, 0, 0, 0);
    m_wordTotalDigits = new QSpinBox(m_wordTotalField); m_wordTotalDigits->setRange(1, 10); m_wordTotalDigits->setObjectName(QStringLiteral("WordTotalDigits"));
    fieldLayout->addWidget(new QLabel(QStringLiteral("总位数:"), m_wordTotalField)); fieldLayout->addWidget(m_wordTotalDigits);
    operationLayout->addWidget(m_wordTotalField, 1, 2, 1, 2);
    m_wordDecimalField = new QWidget(operation); fieldLayout = new QHBoxLayout(m_wordDecimalField); fieldLayout->setContentsMargins(0, 0, 0, 0);
    m_wordDecimalDigits = new QSpinBox(m_wordDecimalField); m_wordDecimalDigits->setRange(0, 9); m_wordDecimalDigits->setObjectName(QStringLiteral("WordDecimalDigits"));
    fieldLayout->addWidget(new QLabel(QStringLiteral("小数位:"), m_wordDecimalField)); fieldLayout->addWidget(m_wordDecimalDigits);
    operationLayout->addWidget(m_wordDecimalField, 2, 2, 1, 2);
    layout->addWidget(operation); layout->addStretch(1);
    m_macro = new QGroupBox(QStringLiteral("宏指令"), page); m_macro->setCheckable(true); m_macro->setChecked(m_working.bitButton.macroEnabled);
    m_macro->setObjectName(QStringLiteral("BitMacro")); macroLayout = new QHBoxLayout(m_macro);
    m_macroList = new QComboBox(m_macro); m_macroList->addItems(m_project.macros.keys()); m_macroList->setObjectName(QStringLiteral("BitMacroList"));
    if (!m_working.bitButton.macroName.isEmpty() && !m_project.macros.contains(m_working.bitButton.macroName))
    {
        m_macroList->addItem(m_working.bitButton.macroName);
    }
    m_macroList->setCurrentText(m_working.bitButton.macroName); write = new QPushButton(QStringLiteral("写宏"), m_macro); write->setObjectName(QStringLiteral("BitWriteMacro"));
    macroLayout->addWidget(new QLabel(QStringLiteral("使用宏:"), m_macro)); macroLayout->addWidget(m_macroList, 1); macroLayout->addWidget(write); layout->addWidget(m_macro);
    connect(picker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_address, false, m_working.wordButton.transferWordCount()); }); //20261109-6 change: Validate both words before returning a wide address from the picker.
    connect(write, &QPushButton::clicked, this, &BitButtonDialog::editMacro);
    connect(m_wordDataType, &QComboBox::currentIndexChanged, this, [this](int type) {
        if (m_loading) { return; }
        m_working.wordButton.resetDefaultsForDataType(type); loadWordOperation();
    });
    connect(m_operation, &QComboBox::currentIndexChanged, this, [this](int operationIndex) {
        if (m_loading) { return; }
        m_working.wordButton.resetDefaultsForOperation(operationIndex); loadWordOperation();
    });
    connect(m_wordTotalDigits, &QSpinBox::valueChanged, this, [this](int total) {
        if (!m_loading) { m_wordDecimalDigits->setMaximum(total - 1); }
    });
    loadWordOperation();
    return page;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::loadWordOperation
 * Description     : Restore defaults or saved values and apply the observed operation visibility matrix.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void BitButtonDialog::loadWordOperation() //20261109-6 add
{
    bool loading;
    const WordButtonProperties *properties;

    loading = m_loading; m_loading = true; properties = &m_working.wordButton;
    m_wordConstant->setText(properties->constantValue); m_wordMinimum->setText(properties->minimumValue);
    m_wordMaximum->setText(properties->maximumValue); m_wordTotalDigits->setValue(properties->totalDigits);
    m_wordDecimalDigits->setMaximum(qMax(properties->totalDigits - 1, properties->decimalDigits));
    m_wordDecimalDigits->setValue(properties->decimalDigits);
    m_wordConstantField->setVisible(properties->usesConstant()); m_wordMinimumField->setVisible(properties->usesMinimum());
    m_wordMaximumField->setVisible(properties->usesMaximum()); m_wordTotalField->setVisible(properties->usesTotalDigits());
    m_wordDecimalField->setVisible(properties->usesDecimalDigits()); m_loading = loading;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::refreshWordAppearance
 * Description     : Gate pressed-state editing without clearing either state's retained artwork draft.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void BitButtonDialog::refreshWordAppearance() //20261109-6 add
{
    bool enabled;

    if (!m_working.hasWordButtonProperties) { return; }
    enabled = m_wordPressedEffect->isChecked();
    m_stateSpin->setMaximum(enabled ? 1 : 0);
    m_copyText->setEnabled(enabled && m_state == 0);
    m_copyShape->setEnabled(enabled && m_state == 0);
    m_background->setEnabled(m_state == 0); //20261109-6 add: State-one's gray swatch is a disabled UI state, not a replacement stored color.
    m_opacity->setEnabled(m_transparent->isChecked());
}

/******************************************************************************
 * Function Name   : BitButtonDialog::createMonitor
 * Description     : Preserve separate bit and word addresses while applying the observed enable hierarchy.
 * Input Parameters: None
 * Return Value    : Indicator page.
 * Date            : 2026.11.08
 ******************************************************************************/
QWidget *BitButtonDialog::createMonitor() //20261108-15 add
{
    QWidget *page;
    QVBoxLayout *layout;
    QGridLayout *monitorLayout;

    if (m_working.hasLampProperties) { return createLampMonitor(); } //20261109-8 add
    if (m_working.hasWordButtonProperties || m_working.hasFunctionButtonProperties) { return new QWidget(this); } //20261110-0 change: Word and system-function indicator pages are blank.

    page = new QWidget(this); layout = new QVBoxLayout(page);
    m_indicator = new QCheckBox(QStringLiteral("启用指示灯功能"), page); m_indicator->setChecked(m_working.bitButton.indicator); m_indicator->setObjectName(QStringLiteral("BitIndicator"));
    layout->addWidget(m_indicator); m_monitor = new QGroupBox(QStringLiteral("双态监视"), page);
    m_monitor->setCheckable(true); m_monitor->setChecked(m_working.bitButton.dualMonitor); m_monitor->setObjectName(QStringLiteral("BitDualMonitor"));
    monitorLayout = new QGridLayout(m_monitor);
    m_monitorType = new QComboBox(m_monitor); m_monitorType->addItems({QStringLiteral("位地址"), QStringLiteral("字地址")});
    m_monitorType->setCurrentIndex(m_working.bitButton.monitorType); m_monitorType->setObjectName(QStringLiteral("BitMonitorType"));
    m_monitorSame = new QCheckBox(QStringLiteral("与操作地址相同"), m_monitor); m_monitorSame->setChecked(m_working.bitButton.monitorSame); m_monitorSame->setObjectName(QStringLiteral("BitMonitorSame"));
    m_monitorAddress = new QLineEdit(m_monitor); m_monitorAddress->setObjectName(QStringLiteral("BitMonitorAddress"));
    m_monitorPicker = new QPushButton(QStringLiteral("…"), m_monitor); m_monitorPicker->setObjectName(QStringLiteral("BitMonitorPicker"));
    m_bitIndex = new QSpinBox(m_monitor); m_bitIndex->setRange(0, 15); m_bitIndex->setValue(m_working.bitButton.bitIndex); m_bitIndex->setObjectName(QStringLiteral("BitMonitorIndex"));
    monitorLayout->addWidget(new QLabel(QStringLiteral("地址类型:"), m_monitor), 0, 0); monitorLayout->addWidget(m_monitorType, 0, 1); monitorLayout->addWidget(m_monitorSame, 0, 2);
    monitorLayout->addWidget(new QLabel(QStringLiteral("监视地址:"), m_monitor), 1, 0); monitorLayout->addWidget(m_monitorAddress, 1, 1); monitorLayout->addWidget(m_monitorPicker, 1, 2);
    monitorLayout->addWidget(new QLabel(QStringLiteral("字 的 位:"), m_monitor), 2, 0); monitorLayout->addWidget(m_bitIndex, 2, 1);
    layout->addWidget(m_monitor); layout->addStretch(1);
    connect(m_indicator, &QCheckBox::toggled, this, &BitButtonDialog::refreshMonitor);
    connect(m_monitor, &QGroupBox::toggled, this, &BitButtonDialog::refreshMonitor);
    connect(m_monitorSame, &QCheckBox::toggled, this, &BitButtonDialog::refreshMonitor);
    connect(m_monitorType, &QComboBox::currentIndexChanged, this, [this](int type) {
        if (!m_loading && !m_monitorSame->isChecked()) { m_working.bitButton.monitorType = type; refreshMonitor(); }
    });
    connect(m_monitorAddress, &QLineEdit::textChanged, this, [this](const QString &address) {
        if (!m_loading && !m_monitorSame->isChecked())
        {
            if (m_monitorType->currentIndex() == 0) { m_working.bitButton.monitorBitAddress = address; }
            else { m_working.bitButton.monitorWordAddress = address; }
        }
    });
    connect(m_monitorPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_monitorAddress, m_monitorType->currentIndex() == 0); });
    return page;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::createLampMonitor
 * Description     : Build mutually exclusive read-only lamp monitors without an indicator enable checkbox.
 * Input Parameters: None
 * Return Value    : Lamp monitoring page.
 * Date            : 2026.11.09
 ******************************************************************************/
QWidget *BitButtonDialog::createLampMonitor() //20261109-8 add
{
    QWidget *page;
    QVBoxLayout *layout;
    QGroupBox *container;
    QVBoxLayout *containerLayout;
    QGridLayout *dualLayout;
    QGridLayout *multiLayout;
    QGroupBox *states;
    QHBoxLayout *statesLayout;

    page = new QWidget(this); page->setObjectName(QStringLiteral("LampMonitorPage"));
    layout = new QVBoxLayout(page);
    container = new QGroupBox(QStringLiteral("启用指示灯功能"), page);
    container->setObjectName(QStringLiteral("LampMonitorContainer"));
    containerLayout = new QVBoxLayout(container);
    m_monitor = new QGroupBox(QStringLiteral("双态监视"), container);
    m_monitor->setCheckable(true); m_monitor->setChecked(m_working.lamp.monitorMode == 1);
    m_monitor->setObjectName(QStringLiteral("BitDualMonitor"));
    dualLayout = new QGridLayout(m_monitor);
    m_monitorType = new QComboBox(m_monitor); m_monitorType->addItems({QStringLiteral("位地址"), QStringLiteral("字地址")});
    m_monitorType->setCurrentIndex(m_working.bitButton.monitorType); m_monitorType->setObjectName(QStringLiteral("BitMonitorType"));
    m_monitorSame = new QCheckBox(QStringLiteral("与操作地址相同"), m_monitor);
    m_monitorSame->setChecked(m_working.bitButton.monitorSame); m_monitorSame->setObjectName(QStringLiteral("BitMonitorSame"));
    m_monitorAddress = new QLineEdit(m_monitor); m_monitorAddress->setObjectName(QStringLiteral("BitMonitorAddress"));
    m_monitorPicker = new QPushButton(QStringLiteral("…"), m_monitor); m_monitorPicker->setFixedWidth(28);
    m_monitorPicker->setObjectName(QStringLiteral("BitMonitorPicker"));
    m_bitIndex = new QSpinBox(m_monitor); m_bitIndex->setRange(0, 15);
    m_bitIndex->setValue(m_working.id.isEmpty() ? 15 : m_working.bitButton.bitIndex);
    m_bitIndex->setObjectName(QStringLiteral("BitMonitorIndex"));
    dualLayout->addWidget(new QLabel(QStringLiteral("地址类型:"), m_monitor), 0, 0); dualLayout->addWidget(m_monitorType, 0, 1); dualLayout->addWidget(m_monitorSame, 0, 2);
    dualLayout->addWidget(new QLabel(QStringLiteral("监视地址:"), m_monitor), 1, 0); dualLayout->addWidget(m_monitorAddress, 1, 1); dualLayout->addWidget(m_monitorPicker, 1, 2);
    dualLayout->addWidget(new QLabel(QStringLiteral("字 的 位:"), m_monitor), 2, 0); dualLayout->addWidget(m_bitIndex, 2, 1);
    containerLayout->addWidget(m_monitor);
    m_lampMultiMonitor = new QGroupBox(QStringLiteral("多态监视"), container);
    m_lampMultiMonitor->setCheckable(true); m_lampMultiMonitor->setChecked(m_working.lamp.monitorMode == 2);
    m_lampMultiMonitor->setObjectName(QStringLiteral("LampMultiMonitor"));
    multiLayout = new QGridLayout(m_lampMultiMonitor);
    m_lampMultiType = new QComboBox(m_lampMultiMonitor); m_lampMultiType->addItem(QStringLiteral("字地址"));
    m_lampMultiType->setObjectName(QStringLiteral("LampMultiAddressType"));
    m_lampMultiSame = new QCheckBox(QStringLiteral("与操作地址相同"), m_lampMultiMonitor);
    m_lampMultiSame->setChecked(m_working.lamp.multiMonitorSame); m_lampMultiSame->setObjectName(QStringLiteral("LampMultiSame"));
    m_lampMultiAddress = new QLineEdit(m_working.lamp.multiMonitorAddress, m_lampMultiMonitor);
    m_lampMultiAddress->setObjectName(QStringLiteral("LampMultiAddress"));
    m_lampMultiPicker = new QPushButton(QStringLiteral("…"), m_lampMultiMonitor); m_lampMultiPicker->setFixedWidth(28);
    m_lampMultiPicker->setObjectName(QStringLiteral("LampMultiPicker"));
    m_lampMultiDataType = new QComboBox(m_lampMultiMonitor); m_lampMultiDataType->addItems(LampProperties::dataTypes());
    m_lampMultiDataType->setCurrentIndex(m_working.lamp.multiDataType); m_lampMultiDataType->setObjectName(QStringLiteral("LampMultiDataType"));
    multiLayout->addWidget(new QLabel(QStringLiteral("地址类型:"), m_lampMultiMonitor), 0, 0); multiLayout->addWidget(m_lampMultiType, 0, 1); multiLayout->addWidget(m_lampMultiSame, 0, 2);
    multiLayout->addWidget(new QLabel(QStringLiteral("监视地址:"), m_lampMultiMonitor), 1, 0); multiLayout->addWidget(m_lampMultiAddress, 1, 1); multiLayout->addWidget(m_lampMultiPicker, 1, 2);
    multiLayout->addWidget(new QLabel(QStringLiteral("数据类型:"), m_lampMultiMonitor), 2, 0); multiLayout->addWidget(m_lampMultiDataType, 2, 1);
    states = new QGroupBox(QStringLiteral("状态"), m_lampMultiMonitor); statesLayout = new QHBoxLayout(states);
    m_lampMultiCount = new QSpinBox(states); m_lampMultiCount->setRange(2, 32); m_lampMultiCount->setValue(m_working.lamp.multiStateCount);
    m_lampMultiCount->setObjectName(QStringLiteral("LampMultiStateCount"));
    statesLayout->addWidget(new QLabel(QStringLiteral("状态数:"), states)); statesLayout->addWidget(m_lampMultiCount);
    multiLayout->addWidget(states, 3, 0, 1, 2); containerLayout->addWidget(m_lampMultiMonitor);
    layout->addWidget(container); layout->addStretch(1);
    connect(m_monitor, &QGroupBox::toggled, this, [this](bool checked) {
        if (m_loading) { return; }
        if (checked)
        {
            const QSignalBlocker blocker(m_lampMultiMonitor);
            m_lampMultiMonitor->setChecked(false);
        }
        refreshLampMonitor();
    });
    connect(m_lampMultiMonitor, &QGroupBox::toggled, this, [this](bool checked) {
        if (m_loading) { return; }
        {
            const QSignalBlocker blocker(m_monitor);
            m_monitor->setChecked(!checked);
        }
        refreshLampMonitor();
    });
    connect(m_monitorType, &QComboBox::currentIndexChanged, this, [this](int type) {
        if (m_loading) { return; }
        m_working.bitButton.monitorType = type; refreshLampMonitor();
    });
    connect(m_monitorAddress, &QLineEdit::textChanged, this, [this](const QString &address) {
        if (m_loading) { return; }
        if (m_monitorType->currentIndex() == 0) { m_working.bitButton.monitorBitAddress = address; }
        else { m_working.bitButton.monitorWordAddress = address; }
    });
    connect(m_bitIndex, &QSpinBox::valueChanged, this, [this]() {
        if (!m_loading) { m_lampBitIndexEdited = true; }
    });
    connect(m_monitorPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_monitorAddress, m_monitorType->currentIndex() == 0); });
    connect(m_lampMultiPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_lampMultiAddress, false, m_lampMultiDataType->currentIndex() == 1 ? 2 : 1); });
    connect(m_lampMultiCount, &QSpinBox::valueChanged, this, [this](int count) {
        if (m_loading) { return; }
        storeState(); m_working.lamp.multiStateCount = count;
        m_working.lamp.ensureStateCount(count, m_working.bitButton);
        refreshLampMonitor();
    });
    return page;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::refreshLampMonitor
 * Description     : Apply lamp monitor gates while retaining the two independent address drafts.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void BitButtonDialog::refreshLampMonitor() //20261109-8 add
{
    bool loading;

    loading = m_loading; m_loading = true;
    setWindowTitle(m_lampMultiMonitor->isChecked() ? QStringLiteral("多态指示灯") : QStringLiteral("位指示灯")); //20261109-8 add
    m_monitorSame->setEnabled(false); m_lampMultiSame->setEnabled(false);
    m_monitorAddress->setText(m_monitorType->currentIndex() == 0 ? m_working.bitButton.monitorBitAddress : m_working.bitButton.monitorWordAddress);
    m_bitIndex->setEnabled(m_monitor->isChecked() && m_monitorType->currentIndex() == 1);
    if (m_stateSpin != nullptr) //20261109-8 add: Native multi-state monitoring expands the same appearance editor.
    {
        m_working.lamp.ensureStateCount(m_lampMultiCount->value(), m_working.bitButton);
        m_stateSpin->setMaximum(m_lampMultiMonitor->isChecked() ? m_lampMultiCount->value() - 1 : 1);
        if (m_state > m_stateSpin->maximum())
        {
            m_state = m_stateSpin->maximum();
            loadState();
        }
    }
    m_loading = loading;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::appearance
 * Description     : Resolve shared and lamp-only appearances through one bounded editor accessor.
 * Input Parameters: index
 * Return Value    : Valid appearance reference.
 * Date            : 2026.11.09
 ******************************************************************************/
BitButtonState &BitButtonDialog::appearance(int index) //20261109-8 add
{
    BitButtonState *state;

    if (m_working.hasLampProperties)
    {
        state = m_working.lamp.state(m_working.bitButton, index);
        if (state != nullptr) { return *state; }
    }
    return m_working.bitButton.states[qBound(0, index, 1)];
}

/******************************************************************************
 * Function Name   : BitButtonDialog::createAppearance
 * Description     : Build a single editor for two fully independent visual and language drafts.
 * Input Parameters: None
 * Return Value    : Appearance page.
 * Date            : 2026.11.08
 ******************************************************************************/
QWidget *BitButtonDialog::createAppearance() //20261108-15 add
{
    QWidget *page;
    QGridLayout *layout;
    QGroupBox *text;
    QGridLayout *textLayout;
    QGroupBox *shape;
    QGridLayout *shapeLayout;
    QPushButton *systemLibrary;
    QPushButton *projectLibrary;
    QPushButton *noImage;

    page = new QWidget(this); layout = new QGridLayout(page);
    m_stateSpin = new QSpinBox(page); m_stateSpin->setRange(0, 1); m_stateSpin->setObjectName(QStringLiteral("BitState"));
    m_languageCombo = new QComboBox(page); m_languageCombo->addItems(m_project.languages); m_languageCombo->setCurrentText(m_language); m_languageCombo->setObjectName(QStringLiteral("BitLanguage"));
    m_flash = new QCheckBox(QStringLiteral("文本闪烁"), page); m_flash->setObjectName(QStringLiteral("BitFlash"));
    m_sameLanguage = new QCheckBox(QStringLiteral("使用同一种语言"), page); m_sameLanguage->setObjectName(QStringLiteral("BitSameLanguage"));
    layout->addWidget(new QLabel(QStringLiteral("选择状态:"), page), 0, 0); layout->addWidget(m_stateSpin, 0, 1);
    layout->addWidget(new QLabel(QStringLiteral("语言:"), page), 0, 2); layout->addWidget(m_languageCombo, 0, 3);
    layout->addWidget(m_flash, 1, 1); layout->addWidget(m_sameLanguage, 1, 3);
    if (m_working.hasWordButtonProperties) //20261109-6 add
    {
        m_wordPressedEffect = new QCheckBox(QStringLiteral("按下效果"), page);
        m_wordPressedEffect->setObjectName(QStringLiteral("WordPressedEffect"));
        m_wordPressedEffect->setChecked(m_working.wordButton.pressedEffect);
        layout->addWidget(m_wordPressedEffect, 1, 2);
    }
    if (m_working.hasFunctionButtonProperties) //20261110-0 add: Pressed rendering never creates a second editable function state.
    {
        m_functionPressedEffect = new QCheckBox(QStringLiteral("按下效果"), page);
        m_functionPressedEffect->setObjectName(QStringLiteral("FunctionPressedEffect"));
        m_functionPressedEffect->setChecked(m_working.functionButton.pressedEffect);
        m_stateSpin->setRange(0, 0); m_stateSpin->setEnabled(false);
        layout->addWidget(m_functionPressedEffect, 1, 2);
    }
    text = new QGroupBox(QStringLiteral("文本属性"), page); textLayout = new QGridLayout(text);
    m_text = new QPlainTextEdit(text); m_text->setFixedHeight(112); m_text->setObjectName(QStringLiteral("BitText"));
    textLayout->addWidget(m_text, 0, 0, 1, 6);
    m_bold = new QPushButton(QStringLiteral("B"), text); m_bold->setCheckable(true); m_bold->setObjectName(QStringLiteral("BitBold"));
    m_italic = new QPushButton(QStringLiteral("I"), text); m_italic->setCheckable(true); m_italic->setObjectName(QStringLiteral("BitItalic"));
    m_underline = new QPushButton(QStringLiteral("U"), text); m_underline->setCheckable(true); m_underline->setObjectName(QStringLiteral("BitUnderline"));
    m_bold->setFont(QFont(QStringLiteral("Times New Roman"), 10, QFont::Bold));
    m_italic->setFont(QFont(QStringLiteral("Times New Roman"), 10, QFont::Normal, true));
    m_bold->setFixedWidth(32); m_italic->setFixedWidth(32); m_underline->setFixedWidth(32);
    textLayout->addWidget(m_bold, 1, 0); textLayout->addWidget(m_italic, 1, 1); textLayout->addWidget(m_underline, 1, 2);
    m_font = new QFontComboBox(text); m_font->setMinimumContentsLength(6); m_font->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon); m_font->setObjectName(QStringLiteral("BitFont"));
    m_size = new QSpinBox(text); m_size->setRange(1, m_working.hasFunctionButtonProperties ? 128 : 4096); m_size->setObjectName(QStringLiteral("BitSize")); //20261110-0 change: Match the native function caption editor's bounds.
    m_alignment = new QComboBox(text); m_alignment->addItems({QStringLiteral("居中"), QStringLiteral("左对齐"), QStringLiteral("右对齐")}); m_alignment->setObjectName(QStringLiteral("BitAlignment"));
    m_textColor = new QPushButton(text); m_textColor->setMinimumHeight(24); m_textColor->setObjectName(QStringLiteral("BitTextColor"));
    textLayout->addWidget(new QLabel(QStringLiteral("字体:"), text), 2, 0); textLayout->addWidget(m_font, 2, 1, 1, 2);
    textLayout->addWidget(new QLabel(QStringLiteral("大小:"), text), 3, 0); textLayout->addWidget(m_size, 3, 1, 1, 2);
    textLayout->addWidget(new QLabel(QStringLiteral("对齐:"), text), 4, 0); textLayout->addWidget(m_alignment, 4, 1, 1, 2);
    textLayout->addWidget(new QLabel(QStringLiteral("文本色:"), text), 5, 0); textLayout->addWidget(m_textColor, 5, 1, 1, 2);
    m_copyText = new QPushButton(text); m_copyText->setObjectName(QStringLiteral("BitCopyText")); textLayout->addWidget(m_copyText, 6, 0, 1, 3);
    layout->addWidget(text, 2, 0, 1, 4);
    shape = new QGroupBox(QStringLiteral("外形属性"), text); shapeLayout = new QGridLayout(shape);
    m_picture = new QLineEdit(shape); m_picture->setObjectName(QStringLiteral("BitPicture"));
    shapeLayout->addWidget(new QLabel(QStringLiteral("图库路径:"), shape), 0, 0); shapeLayout->addWidget(m_picture, 0, 1);
    systemLibrary = new QPushButton(QStringLiteral("系统图库"), shape); systemLibrary->setObjectName(QStringLiteral("BitSystemLibrary"));
    projectLibrary = new QPushButton(QStringLiteral("工程图库"), shape); projectLibrary->setObjectName(QStringLiteral("BitProjectLibrary"));
    noImage = new QPushButton(QStringLiteral("不使用图片"), shape); noImage->setObjectName(QStringLiteral("BitNoImage"));
    m_effect3d = new QCheckBox(QStringLiteral("终端3D按钮效果"), shape); m_effect3d->setObjectName(QStringLiteral("Bit3D"));
    m_transparent = new QCheckBox(QStringLiteral("使用透明效果"), shape); m_transparent->setObjectName(QStringLiteral("BitTransparent"));
    m_background = new QPushButton(shape); m_background->setMinimumHeight(24); m_background->setObjectName(QStringLiteral("BitBackground"));
    m_opacity = new QSpinBox(shape); m_opacity->setRange(0, 255); m_opacity->setObjectName(QStringLiteral("BitOpacity"));
    shapeLayout->addWidget(systemLibrary, 1, 0); shapeLayout->addWidget(m_effect3d, 1, 1);
    shapeLayout->addWidget(projectLibrary, 2, 0); shapeLayout->addWidget(m_transparent, 2, 1);
    shapeLayout->addWidget(noImage, 3, 0); shapeLayout->addWidget(new QLabel(QStringLiteral("背景色:"), shape), 4, 0); shapeLayout->addWidget(m_background, 4, 1);
    shapeLayout->addWidget(new QLabel(QStringLiteral("透明度:"), shape), 5, 0); shapeLayout->addWidget(m_opacity, 5, 1);
    m_copyShape = new QPushButton(shape); m_copyShape->setObjectName(QStringLiteral("BitCopyShape")); shapeLayout->addWidget(m_copyShape, 6, 0, 1, 2);
    textLayout->addWidget(shape, 1, 3, 6, 3); layout->setRowStretch(3, 1);
    connect(m_stateSpin, &QSpinBox::valueChanged, this, [this](int state) {
        if (!m_loading) { storeState(); m_state = state; loadState(); }
    });
    connect(m_languageCombo, &QComboBox::currentTextChanged, this, [this](const QString &language) {
        if (!m_loading) { storeState(); m_language = language; loadState(); }
    });
    connect(m_text, &QPlainTextEdit::textChanged, this, &BitButtonDialog::refresh);
    connect(m_font, &QFontComboBox::currentFontChanged, this, [this]() { //20261109-6 change
        if (m_loading) { return; }
        m_fontEdited = true; refresh();
    });
    connect(m_font, &QComboBox::activated, this, [this]() { //20261109-6 add: Choosing the already displayed fallback still constitutes an explicit font selection.
        if (m_loading) { return; }
        m_fontEdited = true; refresh();
    });
    connect(m_size, &QSpinBox::valueChanged, this, &BitButtonDialog::refresh);
    connect(m_alignment, &QComboBox::currentIndexChanged, this, &BitButtonDialog::refresh);
    connect(m_opacity, &QSpinBox::valueChanged, this, &BitButtonDialog::refresh);
    connect(m_bold, &QPushButton::toggled, this, &BitButtonDialog::refresh);
    connect(m_italic, &QPushButton::toggled, this, &BitButtonDialog::refresh);
    connect(m_underline, &QPushButton::toggled, this, &BitButtonDialog::refresh);
    connect(m_effect3d, &QCheckBox::toggled, this, [this](bool enabled) { //20261109-0 change: The terminal applies this flag to both states.
        BitButtonState *extraState; //20261109-8 add
        int index;
        if (m_loading) { return; }
        m_working.bitButton.states[0].effect3d = enabled; m_working.bitButton.states[1].effect3d = enabled; refresh();
        if (m_working.hasLampProperties)
        {
            for (index = 2; index < m_working.lamp.extraStates.size() + 2; ++index)
            {
                extraState = m_working.lamp.state(m_working.bitButton, index); extraState->effect3d = enabled;
            }
        }
    });
    connect(m_transparent, &QCheckBox::toggled, this, [this](bool enabled) { //20261109-0 change
        BitButtonState *extraState; //20261109-8 add
        int index;
        if (m_loading) { return; }
        m_working.bitButton.states[0].transparent = enabled; m_working.bitButton.states[1].transparent = enabled; refresh();
        if (m_working.hasLampProperties)
        {
            for (index = 2; index < m_working.lamp.extraStates.size() + 2; ++index)
            {
                extraState = m_working.lamp.state(m_working.bitButton, index); extraState->transparent = enabled;
            }
        }
    });
    connect(m_flash, &QCheckBox::toggled, this, &BitButtonDialog::refresh);
    connect(m_sameLanguage, &QCheckBox::toggled, this, [this](bool shared) {
        BitButtonState *state; //20261109-8 add
        int index;
        if (m_loading) { return; }
        if (m_working.hasLampProperties || m_working.hasFunctionButtonProperties) //20261110-0 change: Function pressed artwork shares the global language flag while retaining its own caption.
        {
            for (index = 0; index < (m_working.hasLampProperties ? m_working.lamp.extraStates.size() + 2 : 2); ++index)
            {
                state = &appearance(index);
                if (shared)
                {
                    state->sharedText = index == m_state ? m_text->toPlainText() :
                                        state->sameLanguage ? state->sharedText : state->text.languageTexts.value(m_language);
                }
                else if (index == m_state) { state->sharedText = m_text->toPlainText(); }
                state->sameLanguage = shared;
            }
            m_loading = true;
            m_text->setPlainText(bitButtonContent(appearance(m_state), m_language));
            m_loading = false;
            refresh();
            return;
        }
        if (shared)
        {
            appearance(m_state).sharedText = m_text->toPlainText();
            appearance(m_state).sameLanguage = true;
        }
        else
        {
            appearance(m_state).sharedText = m_text->toPlainText();
            appearance(m_state).sameLanguage = false;
            m_loading = true;
            m_text->setPlainText(appearance(m_state).text.languageTexts.value(m_language));
            m_loading = false;
        }
        refresh();
    });
    connect(m_picture, &QLineEdit::textEdited, this, [this](const QString &path) {
        appearance(m_state).picturePng = m_project.pictures.value(path);
        refresh();
    });
    connect(m_textColor, &QPushButton::clicked, this, [this]() { chooseColor(false); });
    connect(m_background, &QPushButton::clicked, this, [this]() { chooseColor(true); });
    connect(systemLibrary, &QPushButton::clicked, this, [this]() { selectPicture(false); });
    connect(projectLibrary, &QPushButton::clicked, this, [this]() { selectPicture(true); });
    connect(noImage, &QPushButton::clicked, this, [this]() { storeState(); appearance(m_state).useImage = false; loadState(); });
    connect(m_copyText, &QPushButton::clicked, this, [this]() {
        storeState(); appearance(m_state).text = appearance(m_state == 0 ? 1 : 0).text;
        appearance(m_state).flash = appearance(m_state == 0 ? 1 : 0).flash;
        appearance(m_state).sameLanguage = appearance(m_state == 0 ? 1 : 0).sameLanguage;
        appearance(m_state).sharedText = appearance(m_state == 0 ? 1 : 0).sharedText;
        loadState();
    });
    connect(m_copyShape, &QPushButton::clicked, this, [this]() {
        BitButtonState source;
        storeState(); source = appearance(m_state == 0 ? 1 : 0);
        source.text = appearance(m_state).text;
        source.flash = appearance(m_state).flash;
        source.sameLanguage = appearance(m_state).sameLanguage;
        source.sharedText = appearance(m_state).sharedText;
        appearance(m_state) = source; loadState();
    });
    if (m_working.hasWordButtonProperties) //20261109-6 add
    {
        connect(m_wordPressedEffect, &QCheckBox::toggled, this, [this](bool enabled) {
            if (m_loading) { return; }
            storeState(); m_working.wordButton.pressedEffect = enabled; refreshWordAppearance();
        });
        refreshWordAppearance();
    }
    if (m_working.hasFunctionButtonProperties) //20261110-0 add: No editable source state exists for cross-state copying.
    {
        m_copyText->setEnabled(false); m_copyShape->setEnabled(false);
    }
    return page;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::createPermissions
 * Description     : Mirror both permission columns and the observed disabled notification children.
 * Input Parameters: None
 * Return Value    : Permission page.
 * Date            : 2026.11.08
 ******************************************************************************/
QWidget *BitButtonDialog::createPermissions() //20261108-15 add
{
    QWidget *page;
    QGridLayout *layout;
    QGridLayout *noticeLayout; //20261109-0 change
    QHBoxLayout *noticeStates;
    QWidget *noticeStatePanel;
    QRadioButton *noticeBitOff;
    QPushButton *noticeBitPicker;
    QPushButton *noticeWordPicker;
    QLabel *noticeTypeLabel;
    QLabel *noticeValueLabel;
    QHBoxLayout *pressLayout;

    page = new QWidget(this); layout = new QGridLayout(page);
    m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("BitVisibility"), m_working.bitButton.visibility, false, page);
    m_touch = new HmiPermissionEditor(QStringLiteral("触控有效"), QStringLiteral("BitTouch"), m_working.bitButton.touch, true, page);
    layout->addWidget(m_visibility, 0, 0); layout->addWidget(m_touch, 0, 1);
    m_notice = new QGroupBox(QStringLiteral("通知"), page); m_notice->setCheckable(true); m_notice->setChecked(m_working.bitButton.notice); m_notice->setObjectName(QStringLiteral("BitNotice"));
    noticeLayout = new QGridLayout(m_notice); //20261109-0 change: Native notification fields are available after enabling their parent group.
    m_noticeBit = new QCheckBox(QStringLiteral("通知位"), m_notice); m_noticeBit->setChecked(m_working.bitButton.noticeBit); m_noticeBit->setObjectName(QStringLiteral("BitNoticeBit"));
    m_noticeWord = new QCheckBox(QStringLiteral("通知字"), m_notice); m_noticeWord->setChecked(m_working.bitButton.noticeWord); m_noticeWord->setObjectName(QStringLiteral("BitNoticeWord"));
    m_noticeBitAddress = new QLineEdit(m_working.bitButton.noticeBitAddress, m_notice); m_noticeBitAddress->setObjectName(QStringLiteral("BitNoticeBitAddress"));
    m_noticeWordAddress = new QLineEdit(m_working.bitButton.noticeWordAddress, m_notice); m_noticeWordAddress->setObjectName(QStringLiteral("BitNoticeWordAddress"));
    noticeBitPicker = new QPushButton(QStringLiteral("…"), m_notice); noticeBitPicker->setFixedWidth(28); noticeBitPicker->setObjectName(QStringLiteral("BitNoticeBitPicker"));
    noticeWordPicker = new QPushButton(QStringLiteral("…"), m_notice); noticeWordPicker->setFixedWidth(28); noticeWordPicker->setObjectName(QStringLiteral("BitNoticeWordPicker"));
    noticeStatePanel = new QWidget(m_notice); noticeStates = new QHBoxLayout(noticeStatePanel); noticeStates->setContentsMargins(0, 0, 0, 0);
    noticeStates->addWidget(new QLabel(QStringLiteral("状态:"), noticeStatePanel));
    m_noticeBitOn = new QRadioButton(QStringLiteral("1"), noticeStatePanel); m_noticeBitOn->setObjectName(QStringLiteral("BitNoticeBitOn"));
    noticeBitOff = new QRadioButton(QStringLiteral("0"), noticeStatePanel); noticeBitOff->setObjectName(QStringLiteral("BitNoticeBitOff"));
    m_noticeBitOn->setChecked(m_working.bitButton.noticeBitValue); noticeBitOff->setChecked(!m_working.bitButton.noticeBitValue);
    noticeStates->addWidget(m_noticeBitOn); noticeStates->addWidget(noticeBitOff); noticeStates->addStretch();
    m_noticeWordType = new QComboBox(m_notice); m_noticeWordType->addItems(StaticTextProperties::dataTypes());
    m_noticeWordType->setCurrentIndex(m_working.bitButton.noticeWordType); m_noticeWordType->setObjectName(QStringLiteral("BitNoticeWordType"));
    m_noticeWordValue = new QLineEdit(m_working.bitButton.noticeWordValue, m_notice); m_noticeWordValue->setObjectName(QStringLiteral("BitNoticeWordValue"));
    noticeTypeLabel = new QLabel(QStringLiteral("数据类型:"), m_notice); noticeValueLabel = new QLabel(QStringLiteral("数值:"), m_notice);
    noticeLayout->addWidget(m_noticeBit, 0, 0); noticeLayout->addWidget(m_noticeBitAddress, 0, 1); noticeLayout->addWidget(noticeBitPicker, 0, 2);
    noticeLayout->addWidget(m_noticeWord, 0, 3); noticeLayout->addWidget(m_noticeWordAddress, 0, 4); noticeLayout->addWidget(noticeWordPicker, 0, 5);
    noticeLayout->addWidget(noticeStatePanel, 1, 0, 1, 3);
    noticeLayout->addWidget(noticeTypeLabel, 1, 3); noticeLayout->addWidget(m_noticeWordType, 1, 4, 1, 2);
    noticeLayout->addWidget(noticeValueLabel, 2, 3); noticeLayout->addWidget(m_noticeWordValue, 2, 4, 1, 2);
    connect(m_noticeBit, &QCheckBox::toggled, this, [this, noticeStatePanel, noticeBitPicker](bool enabled) {
        if (enabled) { m_noticeWord->setChecked(false); } //20261109-0 add: The native UI selects one notification destination at a time.
        m_noticeBitAddress->setEnabled(enabled); noticeStatePanel->setEnabled(enabled); noticeBitPicker->setEnabled(enabled);
        if (m_working.hasWordButtonProperties || m_working.hasLampProperties || m_working.hasFunctionButtonProperties) //20261110-0 change: Function notification details follow the selected destination.
        {
            m_noticeBitAddress->setVisible(enabled); noticeStatePanel->setVisible(enabled); noticeBitPicker->setVisible(enabled);
        }
    });
    connect(m_noticeWord, &QCheckBox::toggled, this, [this, noticeWordPicker, noticeTypeLabel, noticeValueLabel](bool enabled) {
        if (enabled) { m_noticeBit->setChecked(false); } //20261109-0 add
        if (m_working.hasWordButtonProperties) { m_noticeBit->setEnabled(m_notice->isChecked() && !enabled); } //20261109-6 add: Native word notifications disable only the opposite bit choice.
        m_noticeWordAddress->setEnabled(enabled); m_noticeWordType->setEnabled(enabled); m_noticeWordValue->setEnabled(enabled);
        noticeWordPicker->setEnabled(enabled); noticeTypeLabel->setEnabled(enabled); noticeValueLabel->setEnabled(enabled);
        if (m_working.hasWordButtonProperties || m_working.hasLampProperties || m_working.hasFunctionButtonProperties) //20261110-0 change
        {
            m_noticeWordAddress->setVisible(enabled); m_noticeWordType->setVisible(enabled); m_noticeWordValue->setVisible(enabled);
            noticeWordPicker->setVisible(enabled); noticeTypeLabel->setVisible(enabled); noticeValueLabel->setVisible(enabled);
        }
    });
    m_noticeBitAddress->setEnabled(m_noticeBit->isChecked()); noticeStatePanel->setEnabled(m_noticeBit->isChecked()); noticeBitPicker->setEnabled(m_noticeBit->isChecked());
    m_noticeWordAddress->setEnabled(m_noticeWord->isChecked()); m_noticeWordType->setEnabled(m_noticeWord->isChecked()); m_noticeWordValue->setEnabled(m_noticeWord->isChecked());
    noticeWordPicker->setEnabled(m_noticeWord->isChecked()); noticeTypeLabel->setEnabled(m_noticeWord->isChecked()); noticeValueLabel->setEnabled(m_noticeWord->isChecked());
    if (m_working.hasWordButtonProperties) //20261109-6 add
    {
        m_noticeBit->setEnabled(m_notice->isChecked() && !m_noticeWord->isChecked());
        m_noticeWord->setEnabled(m_notice->isChecked());
        connect(m_notice, &QGroupBox::toggled, this, [this](bool enabled) {
            m_noticeBit->setEnabled(enabled && !m_noticeWord->isChecked());
            m_noticeWord->setEnabled(enabled);
        });
        m_noticeBitAddress->setVisible(m_noticeBit->isChecked()); noticeStatePanel->setVisible(m_noticeBit->isChecked()); noticeBitPicker->setVisible(m_noticeBit->isChecked());
        m_noticeWordAddress->setVisible(m_noticeWord->isChecked()); m_noticeWordType->setVisible(m_noticeWord->isChecked()); m_noticeWordValue->setVisible(m_noticeWord->isChecked());
        noticeWordPicker->setVisible(m_noticeWord->isChecked()); noticeTypeLabel->setVisible(m_noticeWord->isChecked()); noticeValueLabel->setVisible(m_noticeWord->isChecked());
    }
    if (m_working.hasLampProperties || m_working.hasFunctionButtonProperties) //20261110-0 change: Function notifications retain both mutually exclusive destination choices.
    {
        m_noticeBitAddress->setVisible(m_noticeBit->isChecked()); noticeStatePanel->setVisible(m_noticeBit->isChecked()); noticeBitPicker->setVisible(m_noticeBit->isChecked());
        m_noticeWordAddress->setVisible(m_noticeWord->isChecked()); m_noticeWordType->setVisible(m_noticeWord->isChecked()); m_noticeWordValue->setVisible(m_noticeWord->isChecked());
        noticeWordPicker->setVisible(m_noticeWord->isChecked()); noticeTypeLabel->setVisible(m_noticeWord->isChecked()); noticeValueLabel->setVisible(m_noticeWord->isChecked());
    }
    connect(noticeBitPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_noticeBitAddress, true); });
    connect(noticeWordPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_noticeWordAddress, false); });
    layout->addWidget(m_notice, 1, 0, 1, 2);
    m_press = new QCheckBox(QStringLiteral("按压时间"), page); m_press->setChecked(m_working.bitButton.pressTimeEnabled); m_press->setObjectName(QStringLiteral("BitPressEnabled"));
    m_pressValues = new QWidget(page); pressLayout = new QHBoxLayout(m_pressValues); pressLayout->setContentsMargins(0, 0, 0, 0);
    m_pressTime = new QSpinBox(m_pressValues); m_pressTime->setRange(m_working.hasFunctionButtonProperties ? 0 : 1, m_working.hasFunctionButtonProperties ? 255 : 65535); m_pressTime->setValue(m_working.bitButton.pressTime); m_pressTime->setObjectName(QStringLiteral("BitPressTime")); //20261110-0 change: Native function hold time is an unsigned byte, including zero.
    pressLayout->addWidget(m_pressTime); pressLayout->addWidget(new QLabel(QStringLiteral("x100毫秒"), m_pressValues));
    layout->addWidget(m_press, 2, 0); layout->addWidget(m_pressValues, 2, 1); layout->setRowStretch(3, 1);
    m_pressValues->setVisible(m_press->isChecked()); connect(m_press, &QCheckBox::toggled, m_pressValues, &QWidget::setVisible);
    if (m_working.hasLampProperties) //20261109-8 add: Display-only controls have no touch or hold-time column.
    {
        m_touch->hide(); m_press->hide(); m_pressValues->hide();
        m_notice->setEnabled(false); // Native Qt getters confirm the entire notification group is disabled.
        m_visibility->setMaximumWidth(340); layout->setColumnStretch(0, 1); layout->setColumnStretch(1, 1);
    }
    return page;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::storeState
 * Description     : Save every current-state input before switching state, language or preview.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void BitButtonDialog::storeState() //20261108-15 add
{
    BitButtonState *state;
    if (m_loading) { return; }
    state = &appearance(m_state);
    if (m_fontEdited) { state->text.fontFamily = m_font->currentFont().family(); } //20261109-6 change
    state->text.fontSize = m_size->value();
    state->text.alignment = m_alignment->currentIndex(); state->text.bold = m_bold->isChecked();
    state->text.italic = m_italic->isChecked(); state->text.underline = m_underline->isChecked();
    state->sameLanguage = m_sameLanguage->isChecked(); state->flash = m_flash->isChecked();
    if (state->sameLanguage)
    {
        state->sharedText = m_text->toPlainText();
    }
    else { state->text.languageTexts.insert(m_language, m_text->toPlainText()); }
    state->picture = m_picture->text(); state->effect3d = m_effect3d->isChecked();
    state->transparent = m_transparent->isChecked(); state->opacity = m_opacity->value();
}

/******************************************************************************
 * Function Name   : BitButtonDialog::loadState
 * Description     : Load a complete state without allowing intermediate signals to overwrite the draft.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void BitButtonDialog::loadState() //20261108-15 add
{
    BitButtonState state = appearance(m_state);
    m_loading = true;
    m_text->setPlainText(bitButtonContent(state, m_language)); //20261109-0 change
    m_font->setCurrentFont(QFont(state.text.fontFamily)); m_size->setValue(qRound(state.text.fontSize));
    m_alignment->setCurrentIndex(state.text.alignment); m_bold->setChecked(state.text.bold);
    m_italic->setChecked(state.text.italic); m_underline->setChecked(state.text.underline);
    m_flash->setChecked(state.flash); m_sameLanguage->setChecked(state.sameLanguage);
    m_picture->setText(state.picture); m_picture->setEnabled(state.useImage);
    m_effect3d->setChecked(state.effect3d); m_transparent->setChecked(state.transparent); m_opacity->setValue(state.opacity);
    m_copyText->setText(QStringLiteral("从%1状态复制文字属性").arg(m_state == 0 ? 1 : 0)); //20261109-8 change
    m_copyShape->setText(QStringLiteral("从%1状态复制外形").arg(m_state == 0 ? 1 : 0));
    refreshWordAppearance(); //20261109-6 add
    m_fontEdited = false; //20261109-6 add: Preserve the stored family when Qt resolves an unavailable family to another font.
    m_loading = false; refresh();
}

/******************************************************************************
 * Function Name   : BitButtonDialog::refresh
 * Description     : Keep color swatches and the selected-state preview in sync with the draft.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void BitButtonDialog::refresh() //20261108-15 add
{
    BitButtonState state;
    if (m_loading) { return; }
    storeState(); state = appearance(m_state);
    m_textColor->setStyleSheet(QStringLiteral("background:%1;border:1px solid #777;").arg(state.text.textColor.name()));
    m_background->setStyleSheet(QStringLiteral("background:%1;border:1px solid #777;").arg(state.background.name()));
    if (m_working.hasWordButtonProperties) //20261109-6 add: Preserve the saved color while displaying the observed disabled gray swatch.
    {
        m_background->setStyleSheet(QStringLiteral("QPushButton {background:%1;border:1px solid #777;} QPushButton:disabled {background:#a0a0a0;}").arg(state.background.name()));
        refreshWordAppearance();
    }
    if (m_working.hasLampProperties) //20261109-8 add
    {
        m_background->setStyleSheet(QStringLiteral("QPushButton {background:%1;border:1px solid #777;} QPushButton:disabled {background:#a0a0a0;}").arg(state.background.name()));
        m_background->setEnabled(!m_effect3d->isChecked());
        m_opacity->setEnabled(m_transparent->isChecked());
    }
    if (m_working.hasFunctionButtonProperties) //20261110-0 add: Native function fill controls edit the no-image rectangle only.
    {
        m_background->setEnabled(!state.useImage);
        m_opacity->setEnabled(!state.useImage);
    }
    m_preview->state = state; m_preview->language = m_language; m_preview->update();
}

/******************************************************************************
 * Function Name   : BitButtonDialog::refreshMonitor
 * Description     : Apply enablement from indicator, dual-state and same-address gates without losing caches.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void BitButtonDialog::refreshMonitor() //20261108-15 add
{
    bool independent;
    bool loading;
    if (m_working.hasLampProperties) { refreshLampMonitor(); return; } //20261109-8 add
    if (m_working.hasWordButtonProperties || m_working.hasFunctionButtonProperties) { return; } //20261110-0 change
    loading = m_loading; m_loading = true;
    m_monitor->setEnabled(m_indicator->isChecked());
    independent = !m_monitorSame->isChecked();
    m_monitorType->setCurrentIndex(independent ? m_working.bitButton.monitorType : 0);
    m_monitorType->setEnabled(independent); m_monitorAddress->setEnabled(independent); m_monitorPicker->setEnabled(independent);
    m_bitIndex->setEnabled(independent && m_monitorType->currentIndex() == 1);
    m_monitorAddress->setText(m_monitorSame->isChecked() ? m_address->text() : m_monitorType->currentIndex() == 0 ?
                             m_working.bitButton.monitorBitAddress : m_working.bitButton.monitorWordAddress);
    m_loading = loading;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::object
 * Description     : Collect every page while leaving the original object and project untouched.
 * Input Parameters: None
 * Return Value    : Complete button draft.
 * Date            : 2026.11.08
 ******************************************************************************/
HmiObject BitButtonDialog::object() //20261108-15 add
{
    BitButtonProperties *properties;
    storeState(); properties = &m_working.bitButton;
    if (!m_working.hasLampProperties && !m_working.hasFunctionButtonProperties) { m_working.address = m_address->text().trimmed(); } //20261110-0 change: System functions have no operation address.
    if (m_working.hasFunctionButtonProperties) //20261110-0 add: Collect system-function fields independently of bit/word operations.
    {
        m_working.functionButton.function = m_function->currentIndex();
        m_working.functionButton.type = qMax(0, m_functionType->currentIndex());
        m_working.functionButton.keyboardKey = m_functionKeys->checkedId();
        m_working.functionButton.keyboardText = m_functionKeyboardText->text();
        m_working.functionButton.pressedEffect = m_functionPressedEffect->isChecked();
        properties->indicator = false; properties->dualMonitor = false;
    }
    else if (m_working.hasLampProperties) //20261109-8 add
    {
        m_working.lamp.monitorMode = m_lampMultiMonitor->isChecked() ? 2 : m_monitor->isChecked() ? 1 : 0;
        m_working.lamp.multiDataType = m_lampMultiDataType->currentIndex();
        m_working.lamp.multiStateCount = m_lampMultiCount->value();
        m_working.lamp.multiMonitorSame = m_lampMultiSame->isChecked();
        m_working.lamp.multiMonitorAddress = m_lampMultiAddress->text().trimmed();
        properties->indicator = m_working.lamp.monitorMode != 0; properties->dualMonitor = m_working.lamp.monitorMode == 1;
        properties->monitorType = m_monitorType->currentIndex(); properties->monitorSame = m_monitorSame->isChecked();
        if (m_lampBitIndexEdited || properties->monitorType == 1) { properties->bitIndex = m_bitIndex->value(); } //20261109-8 change: Entering word-address monitoring commits the visible default fifteen.
        else if (properties->monitorType == 0) { properties->bitIndex = 0; }
    }
    else if (m_working.hasWordButtonProperties)
    {
        m_working.wordButton.dataType = m_wordDataType->currentIndex(); m_working.wordButton.operation = m_operation->currentIndex();
        m_working.wordButton.constantValue = m_wordConstant->text(); m_working.wordButton.minimumValue = m_wordMinimum->text();
        m_working.wordButton.maximumValue = m_wordMaximum->text(); m_working.wordButton.totalDigits = m_wordTotalDigits->value();
        m_working.wordButton.decimalDigits = m_wordDecimalDigits->value(); m_working.wordButton.pressedEffect = m_wordPressedEffect->isChecked();
        properties->indicator = false; properties->dualMonitor = false;
    }
    else
    {
        properties->operation = m_operation->currentIndex();
        properties->indicator = m_indicator->isChecked(); properties->dualMonitor = m_monitor->isChecked();
        properties->monitorSame = m_monitorSame->isChecked(); properties->bitIndex = m_bitIndex->value();
    }
    properties->macroEnabled = m_macro->isChecked(); properties->macroName = m_macroList->currentText();
    properties->visibility = m_visibility->properties(); properties->touch = m_touch->properties();
    properties->notice = m_notice->isChecked(); properties->noticeBit = m_noticeBit->isChecked(); properties->noticeWord = m_noticeWord->isChecked();
    properties->noticeBitAddress = m_noticeBitAddress->text(); properties->noticeBitValue = m_noticeBitOn->isChecked(); //20261109-0 add
    properties->noticeWordAddress = m_noticeWordAddress->text(); properties->noticeWordType = m_noticeWordType->currentIndex(); properties->noticeWordValue = m_noticeWordValue->text();
    properties->pressTimeEnabled = m_press->isChecked(); properties->pressTime = m_pressTime->value();
    if (m_working.hasLampProperties) //20261109-8 add
    {
        properties->touch.visibilityEnabled = false;
        properties->pressTimeEnabled = false;
    }
    m_working.text = bitButtonContent(properties->states[0], m_project.languages.first());
    return m_working;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::macros
 * Description     : Expose only the local macro draft for the accepting caller.
 * Input Parameters: None
 * Return Value    : Project macro source map.
 * Date            : 2026.11.08
 ******************************************************************************/
QMap<QString, QString> BitButtonDialog::macros() const //20261108-15 add
{
    return m_project.macros;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::pictures
 * Description     : Expose only the local gallery draft for the accepting caller.
 * Input Parameters: None
 * Return Value    : Project image map.
 * Date            : 2026.11.08
 ******************************************************************************/
QMap<QString, QByteArray> BitButtonDialog::pictures() const //20261108-15 add
{
    return m_project.pictures;
}

/******************************************************************************
 * Function Name   : BitButtonDialog::chooseColor
 * Description     : Commit state color changes only when the color dialog is accepted.
 * Input Parameters: background
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void BitButtonDialog::chooseColor(bool background) //20261108-15 add
{
    QColor color;
    storeState(); color = QColorDialog::getColor(background ? appearance(m_state).background :
                    appearance(m_state).text.textColor, this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog);
    if (color.isValid())
    {
        if (background) { appearance(m_state).background = color; }
        else { appearance(m_state).text.textColor = color; }
        refresh();
    }
}

/******************************************************************************
 * Function Name   : BitButtonDialog::editMacro
 * Description     : Save named source drafts transactionally; firmware macro compilation is a separate step.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void BitButtonDialog::editMacro() //20261108-15 add
{
    QDialog dialog(this);
    QVBoxLayout *layout;
    QLineEdit *name;
    QPlainTextEdit *source;
    QDialogButtonBox *buttons;
    QString initial;

    dialog.setObjectName(QStringLiteral("BitMacroEditor")); dialog.setWindowTitle(QStringLiteral("宏指令编辑器")); dialog.resize(760, 540);
    layout = new QVBoxLayout(&dialog); name = new QLineEdit(&dialog); name->setObjectName(QStringLiteral("MacroName"));
    initial = m_macroList->currentText(); name->setText(initial.isEmpty() ? QStringLiteral("Macro%1").arg(m_project.macros.size() + 1) : initial);
    source = new QPlainTextEdit(&dialog); source->setObjectName(QStringLiteral("MacroSource"));
    source->setPlainText(m_project.macros.value(initial,
        QStringLiteral("#include \"macro.h\"\n#include \"include\\macrolib.h\"\n#include \"include\\glib.h\"\n\nvoid Macro_Entry( )\n{\n\n}\n")));
    layout->addWidget(new QLabel(QStringLiteral("名称:"), &dialog)); layout->addWidget(name); layout->addWidget(source);
    buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog); layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&dialog, name, source]() {
        if (name->text().trimmed().isEmpty() || source->toPlainText().size() > 1024 * 1024)
        {
            QMessageBox::warning(&dialog, QStringLiteral("宏无效"), QStringLiteral("请输入名称，源代码长度不能超过本版 1 MiB 保护范围。")); return;
        }
        dialog.accept();
    });
    if (dialog.exec() == QDialog::Accepted)
    {
        m_project.macros.insert(name->text().trimmed(), source->toPlainText());
        m_macroList->clear(); m_macroList->addItems(m_project.macros.keys()); m_macroList->setCurrentText(name->text().trimmed());
    }
}

/******************************************************************************
 * Function Name   : BitButtonDialog::selectPicture
 * Description     : Browse recovered SVG categories in small batches and import portable project images.
 * Input Parameters: projectGallery
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void BitButtonDialog::selectPicture(bool projectGallery) //20261109-3 change
{
    QDialog dialog(this);
    QGridLayout *layout;
    QTreeWidget *tree;
    QTreeWidgetItem *root;
    QListWidget *list;
    QTreeWidgetItem *categoryItem;
    QTreeWidgetItem *selectedCategory;
    QTreeWidgetItem *groupItem;
    QDialogButtonBox *buttons;
    QPushButton *import;
    QPushButton *background;
    QLabel *hint;
    QLineEdit *search;
    QLabel *scopeCount;
    QLabel *pageLabel;
    QPushButton *previousPage;
    QPushButton *nextPage;
    QTimer *loader;
    QList<KvgLibraryCategory> categories;
    QList<KvgLibraryBranch> branches;
    KvgLibraryBranch branch;
    KvgLibraryCategory category;
    QMap<QString, KvgLibraryCategory> categoryById;
    QString categoryId;
    QStringList pending;
    QStringList allPictures;
    QStringList filteredPictures;
    QStringList branchPictures;
    QMap<QString, QString> pictureSearchTexts;
    QString pictureId;
    QString scopeTitle;
    std::function<void()> showPage;
    std::function<void()> updateFilter;
    QString currentPicture;
    int page;
    const int pageSize = 120;
    bool initialSelection = true;
    int cursor;
    int i;
    QMap<QString, QByteArray> gallery;
    QString selected;
    QByteArray selectedData;
    QObject *signalSource;
    int dialogResult;

    gallery = m_project.pictures; dialog.setObjectName(QStringLiteral("BitGallery"));
    dialog.setWindowTitle(projectGallery ? QStringLiteral("工程图库") : QStringLiteral("系统图库")); dialog.resize(1040, 660);
    layout = new QGridLayout(&dialog); tree = new QTreeWidget(&dialog); tree->setHeaderHidden(true); tree->setMinimumWidth(210); tree->setMaximumWidth(260);
    tree->setObjectName(QStringLiteral("GalleryCategories")); //20261109-1 add: Parent nodes expose their complete descendant image list.
    root = new QTreeWidgetItem(tree, QStringList{dialog.windowTitle()});
    root->setExpanded(true);
    list = new QListWidget(&dialog); list->setObjectName(QStringLiteral("GalleryItems")); list->setViewMode(QListView::IconMode);
    list->setIconSize(QSize(90, 90)); list->setGridSize(QSize(140, 125)); list->setResizeMode(QListView::Adjust);
    search = new QLineEdit(&dialog); search->setObjectName(QStringLiteral("GallerySearch"));
    search->setPlaceholderText(QStringLiteral("搜索全部图库：输入图片名称或分类")); search->setClearButtonEnabled(true);
    scopeCount = new QLabel(&dialog); scopeCount->setObjectName(QStringLiteral("GalleryScopeCount"));
    previousPage = new QPushButton(QStringLiteral("上一页"), &dialog); previousPage->setObjectName(QStringLiteral("GalleryPreviousPage"));
    nextPage = new QPushButton(QStringLiteral("下一页"), &dialog); nextPage->setObjectName(QStringLiteral("GalleryNextPage"));
    pageLabel = new QLabel(&dialog); pageLabel->setObjectName(QStringLiteral("GalleryPage")); pageLabel->setAlignment(Qt::AlignCenter);
    search->setVisible(!projectGallery); scopeCount->setVisible(!projectGallery);
    previousPage->setVisible(!projectGallery); nextPage->setVisible(!projectGallery); pageLabel->setVisible(!projectGallery);
    layout->addWidget(tree, 0, 0, 3, 1); layout->addWidget(search, 0, 1, 1, 2); layout->addWidget(scopeCount, 0, 3);
    layout->addWidget(list, 1, 1, 1, 3); layout->setRowStretch(1, 1);
    layout->addWidget(previousPage, 2, 1); layout->addWidget(pageLabel, 2, 2); layout->addWidget(nextPage, 2, 3);
    hint = new QLabel(&dialog);
    hint->setWordWrap(true); layout->addWidget(hint, 3, 0, 1, 4);
    background = new QPushButton(QStringLiteral("修改图库底色"), &dialog); layout->addWidget(background, 4, 0);
    import = new QPushButton(QStringLiteral("导入图片…"), &dialog); import->setVisible(projectGallery); import->setObjectName(QStringLiteral("GalleryImport")); layout->addWidget(import, 4, 1);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog); layout->addWidget(buttons, 4, 2, 1, 2);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(list, &QListWidget::itemDoubleClicked, &dialog, [&dialog]() { dialog.accept(); });
    connect(list, &QListWidget::currentItemChanged, &dialog, [hint, projectGallery](QListWidgetItem *current, QListWidgetItem *) {
        QString warning;
        if (current == nullptr || projectGallery) { return; }
        warning = kvgPictureWarning(current->data(Qt::UserRole + 1).toString());
        hint->setText(QStringLiteral("当前图片：%1。%2").arg(current->text(), warning.isEmpty() ?
                      QStringLiteral("双击或按确定使用选中的图片。") : warning));
    });
    loader = new QTimer(&dialog); loader->setInterval(10); cursor = 0;
    currentPicture = appearance(m_state).picture;
    //20261109-0 add: Decode at most 20 small icons per event-loop turn; category changes cancel unfinished work.
    connect(loader, &QTimer::timeout, &dialog, [list, loader, &gallery, &pending, &cursor, currentPicture, projectGallery]() {
        int end;
        QString picture;
        QString identifier;
        QString warning;
        QImage thumbnail;
        QListWidgetItem *entry;
        end = qMin(cursor + 20, pending.size());
        while (cursor < end) {
            picture = pending.at(cursor++); identifier = picture;
            if (projectGallery) {
                thumbnail = QImage::fromData(gallery.value(picture), "PNG").scaled(90, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            } else {
                thumbnail = kvgPictureImage(picture, QSize(90, 90));
                if (picture.startsWith(QStringLiteral(":/system-gallery/default/")) &&
                    bitButtonBuiltinPictures().contains(QFileInfo(picture).fileName())) {
                    identifier = QFileInfo(picture).fileName();
                }
            }
            entry = new QListWidgetItem(QIcon(QPixmap::fromImage(thumbnail)), QFileInfo(picture).fileName(), list);
            warning = projectGallery ? QString() : kvgPictureWarning(identifier);
            entry->setToolTip(warning.isEmpty() ? picture : picture + QLatin1Char('\n') + warning);
            entry->setData(Qt::UserRole + 1, identifier);
            if (projectGallery) { entry->setData(Qt::UserRole, gallery.value(picture)); }
            if (identifier == currentPicture && list->currentItem() == nullptr) { list->setCurrentItem(entry); }
        }
        if (cursor >= pending.size()) { loader->stop(); }
    });
    selectedCategory = nullptr;
    if (projectGallery) {
        categoryItem = new QTreeWidgetItem(root, QStringList{QStringLiteral("自定义图片")});
        selectedCategory = categoryItem;
        pending = gallery.keys();
        hint->setText(QStringLiteral("导入的图片随工程保存；右键可删除图库条目。支持KVG/SVG，导入后保存为PNG。"));
        loader->start();
    } else {
        categories = kvgLibraryCategories();
        branches = kvgLibraryBranches(); selectedCategory = root; page = 0;
        Q_FOREACH (category, categories) {
            categoryById.insert(category.id, category); allPictures.append(category.pictures);
            Q_FOREACH (pictureId, category.pictures) {
                pictureSearchTexts.insert(pictureId, category.group + QLatin1Char('/') + category.name + QLatin1Char('/') + pictureId);
            }
        }
        for (i = 0; i < branches.size(); ++i) {
            branch = branches.at(i);
            groupItem = i == 0 ? root : new QTreeWidgetItem(tree);
            groupItem->setText(0, branch.name); groupItem->setExpanded(true); branchPictures.clear();
            Q_FOREACH (categoryId, branch.categoryIds) {
                if (!categoryById.contains(categoryId)) { continue; }
                category = categoryById.value(categoryId);
                categoryItem = new QTreeWidgetItem(groupItem, QStringList{category.name});
                categoryItem->setData(0, Qt::UserRole, category.pictures);
                categoryItem->setData(0, Qt::UserRole + 1, branch.name + QStringLiteral(" / ") + category.name);
                categoryItem->setData(0, Qt::UserRole + 2, category.id);
                branchPictures.append(category.pictures);
                if (category.pictures.contains(kvgPictureResource(currentPicture))) { selectedCategory = categoryItem; }
            }
            //20261109-3 add: Preserve files not exposed as native tree leaves in their source branch overview and global search.
            Q_FOREACH (category, categories) {
                if (!branch.categoryIds.contains(category.id) &&
                    ((i == 0 && (category.id == QStringLiteral("default") || category.id.startsWith(QStringLiteral("extern/")))) ||
                     (i == 1 && category.id.startsWith(QStringLiteral("externadd/"))))) {
                    branchPictures.append(category.pictures);
                    if (category.id != QStringLiteral("default") && category.pictures.contains(kvgPictureResource(currentPicture))) { selectedCategory = groupItem; }
                }
            }
            groupItem->setData(0, Qt::UserRole, branchPictures);
            groupItem->setData(0, Qt::UserRole + 1, branch.name);
        }
        search->setPlaceholderText(QStringLiteral("搜索全部图库（%1张）：图片名称或分类").arg(allPictures.size()));
        showPage = [&]() {
            int pageCount;

            loader->stop(); list->clear(); cursor = 0;
            pageCount = (filteredPictures.size() + pageSize - 1) / pageSize;
            page = qBound(0, page, qMax(0, pageCount - 1));
            pending = filteredPictures.mid(page * pageSize, pageSize);
            scopeCount->setText(QStringLiteral("共%1张").arg(filteredPictures.size()));
            pageLabel->setText(pageCount == 0 ? QStringLiteral("没有匹配图片") :
                               QStringLiteral("第%1 / %2页（%3–%4）").arg(page + 1).arg(pageCount).arg(page * pageSize + 1).arg(page * pageSize + pending.size()));
            previousPage->setEnabled(page > 0); nextPage->setEnabled(page + 1 < pageCount);
            hint->setText(scopeTitle + QStringLiteral("。双击或按确定使用图片；每页最多120张。"));
            if (!pending.isEmpty()) { loader->start(); }
        }; //20261109-1 add: Keep thumbnail memory bounded while every parent scope remains browsable.
        updateFilter = [&]() {
            QString query;
            QString picture;
            QStringList scope;

            query = search->text().trimmed(); filteredPictures.clear(); page = 0;
            scope = query.isEmpty() && tree->currentItem() != nullptr ? tree->currentItem()->data(0, Qt::UserRole).toStringList() : allPictures;
            scopeTitle = query.isEmpty() && tree->currentItem() != nullptr ? tree->currentItem()->data(0, Qt::UserRole + 1).toString() :
                         QStringLiteral("全库搜索：%1").arg(query);
            Q_FOREACH (picture, scope) {
                if (query.isEmpty() || pictureSearchTexts.value(picture, picture).contains(query, Qt::CaseInsensitive)) { filteredPictures.append(picture); }
            }
            if (initialSelection && query.isEmpty()) {
                page = kvgPictureResource(currentPicture).startsWith(QStringLiteral(":/system-gallery/default/")) ? 0 :
                       qMax(0, filteredPictures.indexOf(kvgPictureResource(currentPicture))) / pageSize;
                initialSelection = false;
            } //20261109-1 add: Reopen the page containing an existing selection, including images after the first 120.
            showPage();
        };
        connect(tree, &QTreeWidget::currentItemChanged, &dialog, [search, &updateFilter](QTreeWidgetItem *, QTreeWidgetItem *) {
            QSignalBlocker blocker(search);
            search->clear(); updateFilter();
        });
        connect(search, &QLineEdit::textChanged, &dialog, [&updateFilter]() { updateFilter(); });
        connect(previousPage, &QPushButton::clicked, &dialog, [&page, &showPage]() { --page; showPage(); });
        connect(nextPage, &QPushButton::clicked, &dialog, [&page, &showPage]() { ++page; showPage(); });
        if (categories.isEmpty()) { hint->setText(QStringLiteral("系统图库资源不可用。")); }
    }
    if (selectedCategory != nullptr) { tree->setCurrentItem(selectedCategory); }
    connect(background, &QPushButton::clicked, &dialog, [&dialog, list]() {
        QColor color;
        QPalette palette;
        color = QColorDialog::getColor(Qt::white, &dialog, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog);
        if (color.isValid()) { palette = list->palette(); palette.setColor(QPalette::Base, color); list->setPalette(palette); }
    });
    connect(import, &QPushButton::clicked, &dialog, [&dialog, list, &gallery]() {
        QString path;
        QString name;
        QImageReader reader;
        QImage image;
        QFile source;
        QString error;
        QByteArray png;
        QBuffer buffer;
        QListWidgetItem *entry;
        int suffix;
        path = QFileDialog::getOpenFileName(&dialog, QStringLiteral("导入工程图片"), QString(), QStringLiteral("图片 (*.png *.jpg *.jpeg *.bmp *.gif *.svg *.kvg)"));
        if (path.isEmpty()) { return; }
        reader.setFileName(path); reader.setDecideFormatFromContent(true);
        if (QFileInfo(path).size() > 16 * 1024 * 1024)
        {
            QMessageBox::warning(&dialog, QStringLiteral("图片过大"), QStringLiteral("本版支持不超过16 MiB、4096×4096的图片。")); return;
        }
        if (QFileInfo(path).suffix().compare(QStringLiteral("kvg"), Qt::CaseInsensitive) == 0 ||
            QFileInfo(path).suffix().compare(QStringLiteral("svg"), Qt::CaseInsensitive) == 0) {
            source.setFileName(path);
            if (source.open(QIODevice::ReadOnly)) { image = importKvgImage(source.readAll(), &error); }
        } else if (reader.size().width() <= 4096 && reader.size().height() <= 4096) {
            image = reader.read();
        }
        buffer.setBuffer(&png); buffer.open(QIODevice::WriteOnly);
        if (image.isNull() || image.width() > 4096 || image.height() > 4096 || !image.save(&buffer, "PNG") || png.size() > 16 * 1024 * 1024)
        {
            QMessageBox::warning(&dialog, QStringLiteral("导入失败"), error.isEmpty() ? QStringLiteral("无法读取图片或转换后的图片过大。") : error); return;
        }
        name = QFileInfo(path).completeBaseName() + QStringLiteral(".png"); suffix = 1;
        while (gallery.contains(name)) { name = QFileInfo(path).completeBaseName() + QStringLiteral("_%1.png").arg(suffix++); }
        gallery.insert(name, png); entry = new QListWidgetItem(QIcon(QPixmap::fromImage(image.scaled(90, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation))), name, list);
        entry->setData(Qt::UserRole, png); entry->setData(Qt::UserRole + 1, name); list->setCurrentItem(entry);
    });
    list->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(list, &QListWidget::customContextMenuRequested, &dialog, [list, projectGallery, &gallery, &pending, &cursor](const QPoint &point) {
        QMenu menu;
        QListWidgetItem *entry;
        entry = list->itemAt(point); if (!projectGallery || entry == nullptr) { return; }
        menu.addAction(QStringLiteral("删除图片"));
        if (menu.exec(list->viewport()->mapToGlobal(point)) != nullptr) {
            gallery.remove(entry->text());
            if (pending.indexOf(entry->text()) >= cursor) { pending.removeAll(entry->text()); }
            delete entry;
        }
    });
    dialogResult = dialog.exec();
    loader->stop();
    Q_FOREACH (signalSource, dialog.findChildren<QObject *>()) {
        QObject::disconnect(signalSource, nullptr, &dialog, nullptr);
    } //20261109-1 add: Drop closures before their referenced filter/draft state leaves this stack frame.
    if (dialogResult == QDialog::Accepted)
    {
        m_project.pictures = gallery; //20261108-15 change: Confirm deletion even when the gallery is now empty.
        if (list->currentItem() == nullptr) { return; }
        selected = list->currentItem()->data(Qt::UserRole + 1).toString(); selectedData = list->currentItem()->data(Qt::UserRole).toByteArray();
        storeState(); m_project.pictures = gallery; appearance(m_state).picture = selected;
        appearance(m_state).picturePng = selectedData; appearance(m_state).useImage = true;
        loadState();
    }
}

/******************************************************************************
 * Function Name   : BitButtonDialog::accept
 * Description     : Validate the draft before exposing any object, macro or gallery changes to the caller.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void BitButtonDialog::accept() //20261108-15 add
{
    HmiObject draft;
    QString error;
    PlcCatalog catalog; //20261109-6 add
    PlcAddressContext context;
    PlcAddress address;
    bool validWordAddress;
    QString monitorAddress; //20261109-8 add

    draft = object();
    if (draft.hasLampProperties && draft.lamp.monitorMode == 0) //20261109-8 add: Native acceptance resolves the unchecked draft to multi-state monitoring.
    {
        m_lampMultiMonitor->setChecked(true);
        draft = object();
    }
    validWordAddress = true;
    if (draft.hasLampProperties) //20261109-8 add: Validate the active monitor independently of the hidden operation-address draft.
    {
        context.connections = m_project.connections;
        context.role = draft.lamp.monitorMode == 1 && draft.bitButton.monitorType == 0 ? PlcAddressRole::Bit : PlcAddressRole::Word;
        context.transferCount = u32(draft.lamp.monitorMode == 2 ? draft.lamp.transferWordCount() : 1);
        monitorAddress = draft.lamp.monitorMode == 2 ? draft.lamp.multiMonitorAddress : draft.bitButton.monitorType == 0 ?
                         draft.bitButton.monitorBitAddress : draft.bitButton.monitorWordAddress;
        if (draft.lamp.monitorMode != 0)
        {
            validWordAddress = catalog.load(QStringLiteral(":/plc/plc-catalog.json"), &error) &&
                               parseQualifiedPlcAddressText(monitorAddress, context, catalog, &address, &error);
        }
    }
    if (draft.hasWordButtonProperties) //20261109-6 add: Direct text entry uses the same word role and width as the picker and project guard.
    {
        context.connections = m_project.connections; context.role = PlcAddressRole::Word;
        context.transferCount = u32(draft.wordButton.transferWordCount());
        validWordAddress = catalog.load(QStringLiteral(":/plc/plc-catalog.json"), &error) &&
                           parseQualifiedPlcAddressText(draft.address, context, catalog, &address, &error);
    }
    if (draft.hasFunctionButtonProperties && draft.functionButton.function == 13 && //20261110-0 add: Language types index the project list rather than a fixed global list.
        (draft.functionButton.type < 0 || draft.functionButton.type >= m_project.languages.size()))
    {
        error = QStringLiteral("语言切换类型不属于当前工程语言。"); validWordAddress = false;
    }
    if (!validWordAddress || (!draft.hasLampProperties && !draft.hasFunctionButtonProperties && draft.address.isEmpty()) || !draft.bitButton.validate(&error, draft.hasFunctionButtonProperties) || //20261110-0 change
        (draft.hasLampProperties && !draft.lamp.validate(&error)) || //20261109-8 add
        (draft.hasFunctionButtonProperties && !draft.functionButton.validate(&error)) || //20261110-0 add
        (draft.hasWordButtonProperties && !draft.wordButton.validate(&error)) || //20261109-6 add
        (draft.bitButton.macroEnabled && !m_project.macros.contains(draft.bitButton.macroName)))
    {
        if (error.isEmpty()) { error = draft.hasFunctionButtonProperties ? QStringLiteral("请选择有效的功能、类型或键盘键；使用宏时需先选择或编写宏。") : draft.hasLampProperties ? QStringLiteral("请填写活动监视地址；使用宏时需先选择或编写宏。") : draft.hasWordButtonProperties ? QStringLiteral("请填写字地址；使用宏时需先选择或编写宏。") : QStringLiteral("请填写位地址；使用宏时需先选择或编写宏。"); } //20261110-0 change
        QMessageBox::warning(this, draft.hasFunctionButtonProperties ? QStringLiteral("功能按钮属性无效") : draft.hasLampProperties ? QStringLiteral("指示灯属性无效") : draft.hasWordButtonProperties ? QStringLiteral("字按钮属性无效") : QStringLiteral("位按钮属性无效"), error); return; //20261110-0 change
    }
    QDialog::accept();
}
