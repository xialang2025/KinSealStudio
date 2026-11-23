#include "stage2_clock_panel.h"

#include "local_help.h"
#include "plc_address_dialog.h"
#include "project_model.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpressionValidator>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStyle>
#include <QTimeEdit>

namespace {

constexpr u32 ClockLocalAddressCount = 65536; //20261123-16 add

struct ClockChannel //20261123-16 add
{
    QGroupBox *enabled;
    QLineEdit *dataAddress;
    QRadioButton *trigger;
    QLineEdit *triggerAddress;
    QSpinBox *interval;
};

/******************************************************************************
 * Function Name   : setupDialog
 * Description     : Apply the captured client dimensions and the original settings palette.
 * Input Parameters: dialog, name, title, size, topic
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void setupDialog(QDialog &dialog, const QString &name, const QString &title,
                 const QSize &size, const QString &topic) //20261123-16 add
{
    QFont font(QStringLiteral("SimSun"));

    dialog.setObjectName(name);
    dialog.setWindowTitle(title);
    dialog.setWindowFlags(dialog.windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowContextHelpButtonHint);
    if (dialog.parentWidget() != nullptr)
    {
        dialog.setWindowIcon(dialog.parentWidget()->windowIcon());
    }
    dialog.setProperty("localHelpTopic", topic);
    dialog.setFixedSize(size);
    font.setPixelSize(12);
    font.setStyleStrategy(QFont::NoAntialias);
    dialog.setFont(font);
    dialog.setStyleSheet(QStringLiteral(
        "QDialog#ClockPropertyDialog, QDialog#HMIProtectionDialog { background: #f0f0f0; }"
        "QCheckBox, QRadioButton { spacing: 4px; }"
        "QLineEdit, QComboBox, QAbstractSpinBox { background: #caeace; }"
        "QLineEdit:disabled, QAbstractSpinBox:disabled { background: #b1ceed; color: #6d6d6d; }"
        "QComboBox:disabled { background: #f0f0f0; color: #6d6d6d; }"
        "QPushButton[addressPicker=true] { border: 0; background: transparent; padding: 0; }"
        "QPushButton[protectionLevel=true]:checked:enabled { background: #b1ceed; border: 1px solid #0078d7; }"));
}

/******************************************************************************
 * Function Name   : addButtons
 * Description     : Add the common accept, cancel and contextual help controls.
 * Input Parameters: dialog, geometry, topic
 * Return Value    : The button box.
 * Date            : 2026.11.23
 ******************************************************************************/
QDialogButtonBox *addButtons(QDialog &dialog, const QRect &geometry, const QString &topic) //20261123-16 add
{
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
                                        QDialogButtonBox::Help, &dialog);
    auto *help = new LocalHelpController(&dialog);

    buttons->setObjectName(QStringLiteral("buttonBox"));
    buttons->setGeometry(geometry);
    buttons->layout()->setContentsMargins(0, 0, 0, 0);
    buttons->layout()->setSpacing(8);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助"));
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::helpRequested, &dialog, [help, topic, &dialog]() {
        help->showTopic(topic, &dialog);
    });
    return buttons;
}

/******************************************************************************
 * Function Name   : calculatorIcon
 * Description     : Render the compact keypad glyph used beside a PLC address field.
 * Input Parameters: None.
 * Return Value    : Normal and disabled icon states.
 * Date            : 2026.11.23
 ******************************************************************************/
QIcon calculatorIcon() //20261123-16 add
{
    QIcon icon;
    QPixmap pixels;
    QPainter painter;
    int row;
    int column;
    int state;
    QColor ink;

    for (state = 0; state < 2; ++state)
    {
        ink = state == 0 ? QColor(51, 76, 112) : QColor(130, 130, 130);
        pixels = QPixmap(17, 17);
        pixels.fill(Qt::transparent);
        painter.begin(&pixels);
        painter.setPen(ink);
        painter.setBrush(QColor(240, 244, 249));
        painter.drawRoundedRect(QRect(1, 1, 14, 14), 2, 2);
        painter.fillRect(3, 3, 11, 3, ink);
        for (row = 0; row < 3; ++row)
        {
            for (column = 0; column < 3; ++column)
            {
                painter.fillRect(3 + column * 4, 7 + row * 2, 2, 1, ink);
            }
        }
        painter.end();
        icon.addPixmap(pixels, state == 0 ? QIcon::Normal : QIcon::Disabled);
    }
    return icon;
}

/******************************************************************************
 * Function Name   : addAddressPicker
 * Description     : Bind a typed address selection to just its detached text field.
 * Input Parameters: parent, name, geometry, field, bit, project
 * Return Value    : The picker control.
 * Date            : 2026.11.23
 ******************************************************************************/
QPushButton *addAddressPicker(QWidget *parent, const QString &name, const QRect &geometry, QLineEdit *field,
                              bool bit, const HmiProject &project) //20261123-16 add
{
    auto *button = new QPushButton(parent);

    button->setObjectName(name);
    button->setProperty("addressPicker", true);
    button->setGeometry(geometry);
    button->setAutoDefault(false);
    button->setFlat(true);
    button->setIcon(calculatorIcon());
    button->setIconSize(QSize(17, 17));
    button->setAccessibleName(bit ? QStringLiteral("选择位地址") : QStringLiteral("选择字地址"));
    QObject::connect(button, &QPushButton::clicked, parent, [field, bit, &project]() {
        QString selected = field->text();

        if (editPlcAddress(field->window(), bit, project.connections, &selected, 1, ClockLocalAddressCount))
        {
            // The generic picker selects a single starting address. Confirmation validates the complete clock span.
            field->setText(selected);
        }
    });
    return button;
}

/******************************************************************************
 * Function Name   : addLabel
 * Description     : Add a fixed-position field label at the measured client coordinate.
 * Input Parameters: parent, text, geometry
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void addLabel(QWidget *parent, const QString &text, const QRect &geometry) //20261123-16 add
{
    auto *label = new QLabel(text, parent);

    label->setGeometry(geometry);
}

/******************************************************************************
 * Function Name   : addClockChannel
 * Description     : Reproduce the common time or bit-trigger control group for one direction.
 * Input Parameters: dialog, project, seed, writing
 * Return Value    : The controls needed to read the detached direction.
 * Date            : 2026.11.23
 ******************************************************************************/
ClockChannel addClockChannel(QDialog &dialog, const HmiProject &project,
                             const QJsonObject &seed, bool writing) //20261123-16 add
{
    ClockChannel result;
    const QString prefix = writing ? QStringLiteral("Write") : QStringLiteral("Read");
    QComboBox *dataType;
    QComboBox *length;
    QGroupBox *startMode;
    QRadioButton *timer;
    QLabel *unit;
    QPushButton *triggerPicker;
    QLabel *intervalLabel;
    QLabel *triggerLabel;

    result.enabled = new QGroupBox(writing ? QStringLiteral("写时间到PLC") : QStringLiteral("与PLC时间同步"), &dialog);
    result.enabled->setObjectName(writing ? QStringLiteral("bWriteToPLC") : QStringLiteral("bReadFromPLC"));
    result.enabled->setCheckable(true);
    result.enabled->setGeometry(writing ? 21 : 381, 59, 341, 262);

    addLabel(result.enabled, QStringLiteral("数据类型："), QRect(19, 41, 78, 23));
    dataType = new QComboBox(result.enabled);
    dataType->setObjectName(QStringLiteral("n") + prefix + QStringLiteral("DataType"));
    dataType->setGeometry(110, 42, 170, 22);
    dataType->addItem(QStringLiteral("16位BCD"));
    addLabel(result.enabled, QStringLiteral("长    度："), QRect(19, 71, 78, 23));
    length = new QComboBox(result.enabled);
    length->setObjectName(QStringLiteral("n") + prefix + QStringLiteral("DataLen"));
    length->setGeometry(110, 72, 80, 22);
    length->addItem(QStringLiteral("7"));
    addLabel(result.enabled, writing ? QStringLiteral("写入地址：") : QStringLiteral("读取地址："), QRect(19, 101, 78, 23));
    result.dataAddress = new QLineEdit(seed.value(QStringLiteral("dataAddress")).toString(), result.enabled);
    result.dataAddress->setObjectName(QStringLiteral("s") + prefix + QStringLiteral("Addr"));
    result.dataAddress->setGeometry(100, 102, 171, 21);
    result.dataAddress->setMaxLength(512);
    addAddressPicker(result.enabled, writing ? QStringLiteral("pushButton_addrBegin") : QStringLiteral("pushButton_addrBegin_3"),
        QRect(290, 100, 22, 24), result.dataAddress, false, project);

    startMode = new QGroupBox(QStringLiteral("启动方式"), result.enabled);
    startMode->setObjectName(writing ? QStringLiteral("groupBox_3") : QStringLiteral("groupBox_4"));
    startMode->setGeometry(20, 139, 301, 103);
    timer = new QRadioButton(QStringLiteral("计时"), startMode);
    timer->setObjectName(QStringLiteral("b") + prefix + QStringLiteral("ByTimeInterval"));
    timer->setGeometry(19, 23, 63, 26);
    result.trigger = new QRadioButton(QStringLiteral("触发"), startMode);
    result.trigger->setObjectName(QStringLiteral("b") + prefix + QStringLiteral("ByAddrTrigger"));
    result.trigger->setGeometry(19, 63, 63, 26);
    intervalLabel = new QLabel(QStringLiteral("间隔："), startMode);
    intervalLabel->setGeometry(90, 23, 58, 26);
    result.interval = new QSpinBox(startMode);
    result.interval->setObjectName(QStringLiteral("n") + prefix + QStringLiteral("TimeInterval"));
    result.interval->setGeometry(150, 23, 80, 30);
    result.interval->setRange(1, 255);
    result.interval->setValue(seed.value(QStringLiteral("interval")).toInt());
    unit = new QLabel(QStringLiteral("s"), startMode);
    unit->setGeometry(240, 23, 18, 26);
    triggerLabel = new QLabel(QStringLiteral("触发位："), startMode);
    triggerLabel->setGeometry(90, 63, 58, 29);
    result.triggerAddress = new QLineEdit(seed.value(QStringLiteral("triggerAddress")).toString(), startMode);
    result.triggerAddress->setObjectName(QStringLiteral("s") + prefix + QStringLiteral("TriggerAddr"));
    result.triggerAddress->setGeometry(150, 63, 91, writing ? 30 : 21);
    result.triggerAddress->setMaxLength(512);
    triggerPicker = addAddressPicker(startMode,
        writing ? QStringLiteral("pushButton_addrBegin_2") : QStringLiteral("pushButton_addrBegin_4"),
        QRect(254, writing ? 66 : 61, 22, 24), result.triggerAddress, true, project);

    auto refresh = [result, triggerPicker](bool isTrigger) {
        result.interval->setEnabled(!isTrigger);
        result.triggerAddress->setEnabled(isTrigger);
        triggerPicker->setEnabled(isTrigger);
    };
    QObject::connect(result.trigger, &QRadioButton::toggled, startMode, refresh);
    timer->setChecked(!seed.value(QStringLiteral("trigger")).toBool());
    result.trigger->setChecked(seed.value(QStringLiteral("trigger")).toBool());
    result.enabled->setChecked(seed.value(QStringLiteral("enabled")).toBool());
    // Checkable QGroupBox changes its children's enabled state. Reapply the selected trigger branch after the toggle.
    QObject::connect(result.enabled, &QGroupBox::toggled, startMode, [result, refresh]() {
        refresh(result.trigger->isChecked());
    });
    refresh(result.trigger->isChecked());
    return result;
}

/******************************************************************************
 * Function Name   : clockChannelDraft
 * Description     : Collect the selected clock direction without discarding the inactive branch.
 * Input Parameters: channel
 * Return Value    : A complete editable direction record.
 * Date            : 2026.11.23
 ******************************************************************************/
QJsonObject clockChannelDraft(const ClockChannel &channel) //20261123-16 add
{
    return {{QStringLiteral("enabled"), channel.enabled->isChecked()},
        {QStringLiteral("trigger"), channel.trigger->isChecked()},
        {QStringLiteral("interval"), channel.interval->value()},
        {QStringLiteral("dataAddress"), channel.dataAddress->text().trimmed()},
        {QStringLiteral("triggerAddress"), channel.triggerAddress->text().trimmed()}};
}

/******************************************************************************
 * Function Name   : editClock
 * Description     : Edit independent download and mutually exclusive PLC clock settings.
 * Input Parameters: dialog, project, seed, result
 * Return Value    : True if the validated draft was accepted.
 * Date            : 2026.11.23
 ******************************************************************************/
bool editClock(QDialog &dialog, const HmiProject &project, const QJsonObject &seed,
               QJsonObject *result) //20261123-16 add
{
    const QString kind = QStringLiteral("clockSettings");
    const QString topic = QStringLiteral("clock-settings");
    QCheckBox *download;
    ClockChannel write;
    ClockChannel read;
    QDialogButtonBox *buttons;
    QJsonObject draft;
    QString error;

    setupDialog(dialog, QStringLiteral("ClockPropertyDialog"), QStringLiteral("时钟"), QSize(739, 388), topic);
    download = new QCheckBox(QStringLiteral("下载系统时钟到HMI"), &dialog);
    download->setObjectName(QStringLiteral("bDownloadToHMI"));
    download->setGeometry(21, 18, 240, 26);
    download->setChecked(seed.value(QStringLiteral("downloadToHmi")).toBool());
    write = addClockChannel(dialog, project, seed.value(QStringLiteral("write")).toObject(), true);
    read = addClockChannel(dialog, project, seed.value(QStringLiteral("read")).toObject(), false);
    auto updateDirections = [write, read]() {
        write.enabled->setEnabled(!read.enabled->isChecked());
        read.enabled->setEnabled(!write.enabled->isChecked());
    };
    QObject::connect(write.enabled, &QGroupBox::toggled, &dialog, updateDirections);
    QObject::connect(read.enabled, &QGroupBox::toggled, &dialog, updateDirections);
    updateDirections();
    buttons = addButtons(dialog, QRect(492, 352, 235, 27), topic);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        draft = {{QStringLiteral("downloadToHmi"), download->isChecked()},
            {QStringLiteral("write"), clockChannelDraft(write)}, {QStringLiteral("read"), clockChannelDraft(read)}};
        if (!validateStage2ClockPanel(kind, draft, &error) ||
            !validateStage2ClockPanelAddresses(kind, draft, project.connections, &error))
        {
            QMessageBox::warning(&dialog, QStringLiteral("时钟设置无效"), error);
            return;
        }
        dialog.accept();
    });
    if (dialog.exec() != QDialog::Accepted)
    {
        return false;
    }
    *result = draft;
    return true;
}

/******************************************************************************
 * Function Name   : editPanelProtection
 * Description     : Edit all ten detached levels while only displaying the selected protection range.
 * Input Parameters: dialog, project, seed, result
 * Return Value    : True if the validated draft was accepted.
 * Date            : 2026.11.23
 ******************************************************************************/
bool editPanelProtection(QDialog &dialog, const HmiProject &project, const QJsonObject &seed,
                         QJsonObject *result) //20261123-18 add
{
    const QString kind = QStringLiteral("panelProtection");
    const QString topic = QStringLiteral("panel-protection");
    const QList<HmiPage> pages = project.allPages();
    QJsonArray levels = seed.value(QStringLiteral("levels")).toArray();
    QJsonObject draft;
    QString error;
    QString fallbackPage;
    QCheckBox *enabled;
    QWidget *enabledFields;
    QComboBox *levelCount;
    QButtonGroup *levelButtons;
    QGroupBox *settings;
    QDateEdit *expiryDate;
    QTimeEdit *expiryTime;
    QComboBox *expiryPage;
    QLineEdit *password;
    QLabel *warning;
    QDialogButtonBox *buttons;
    HmiPage page;
    QPushButton *levelButton;
    const QJsonValue defaultLevel = defaultStage2ClockPanel(kind)
        .value(QStringLiteral("levels")).toArray().at(0); //20261123-18 add
    int index;
    int currentLevel = 0;

    setupDialog(dialog, QStringLiteral("HMIProtectionDialog"), QStringLiteral("HMI保护"), QSize(400, 484), topic);
    enabled = new QCheckBox(QStringLiteral("使用HMI保护功能"), &dialog);
    enabled->setObjectName(QStringLiteral("bEnabled"));
    enabled->setGeometry(24, 22, 243, 27);
    enabled->setChecked(seed.value(QStringLiteral("enabled")).toBool());
    enabledFields = new QWidget(&dialog);
    enabledFields->setObjectName(QStringLiteral("fEnabled"));
    enabledFields->setGeometry(24, 54, 322, 253);
    addLabel(enabledFields, QStringLiteral("密码级别总数："), QRect(56, 0, 93, 24));
    levelCount = new QComboBox(enabledFields);
    levelCount->setObjectName(QStringLiteral("nProtectNum"));
    levelCount->setGeometry(150, 2, 151, 22);
    levelButtons = new QButtonGroup(enabledFields);
    for (index = 0; index < 10; ++index)
    {
        levelCount->addItem(QString::number(index + 1));
        levelButton = new QPushButton(QString::number(index + 1), enabledFields);
        levelButton->setObjectName(QStringLiteral("ProtectButton%1").arg(index + 1));
        levelButton->setProperty("protectionLevel", true);
        levelButton->setGeometry(index * 30, 43, 28, 28);
        levelButton->setAutoDefault(false);
        levelButton->setCheckable(true);
        levelButtons->addButton(levelButton, index);
    }
    levelButtons->button(0)->setChecked(true);
    levelCount->setCurrentIndex(seed.value(QStringLiteral("levelCount")).toInt() - 1);
    settings = new QGroupBox(QStringLiteral("参数设置"), enabledFields);
    settings->setObjectName(QStringLiteral("groupBox"));
    settings->setGeometry(0, 81, 321, 171);
    addLabel(settings, QStringLiteral("使用期限："), QRect(70, 28, 69, 26));
    expiryDate = new QDateEdit(settings);
    expiryDate->setObjectName(QStringLiteral("mExpiredDate"));
    expiryDate->setGeometry(140, 31, 151, 22);
    expiryDate->setDateRange(QDate(1, 1, 1), QDate(9999, 12, 31));
    expiryDate->setDisplayFormat(QStringLiteral("yyyy/MM/dd"));
    expiryDate->setCalendarPopup(true);
    expiryTime = new QTimeEdit(settings);
    expiryTime->setObjectName(QStringLiteral("mExpiredTime"));
    expiryTime->setGeometry(140, 61, 151, 30);
    expiryTime->setDisplayFormat(QStringLiteral("HH:mm:ss"));
    addLabel(settings, QStringLiteral("期限提示屏幕："), QRect(56, 99, 84, 26));
    expiryPage = new QComboBox(settings);
    expiryPage->setObjectName(QStringLiteral("nExpiredScene"));
    expiryPage->setGeometry(140, 101, 151, 22);
    Q_FOREACH (page, pages)
    {
        if (page.kind == QStringLiteral("normal"))
        {
            expiryPage->addItem(page.name, page.id);
        }
    }
    fallbackPage = expiryPage->itemData(0).toString();
    addLabel(settings, QStringLiteral("解除保护密码："), QRect(56, 129, 84, 24));
    password = new QLineEdit(settings);
    password->setObjectName(QStringLiteral("sUnlockPWD"));
    password->setGeometry(140, 131, 151, 21);
    password->setMaxLength(8);
    password->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[0-9]{0,8}")), password));

    auto populateLevel = [&]() {
        const QJsonObject level = levels.at(currentLevel).toObject();
        const int pageIndex = expiryPage->findData(level.value(QStringLiteral("pageId")).toString());
        const QSignalBlocker blockPageWrite(expiryPage);

        expiryDate->setDate(QDate::fromString(level.value(QStringLiteral("date")).toString(), Qt::ISODate));
        expiryTime->setTime(QTime::fromString(level.value(QStringLiteral("time")).toString(), Qt::ISODate));
        expiryPage->setCurrentIndex(pageIndex == -1 ? 0 : pageIndex);
        password->setText(level.value(QStringLiteral("password")).toString());
    };
    auto storeLevel = [&]() {
        QJsonObject level = levels.at(currentLevel).toObject();

        level.insert(QStringLiteral("date"), expiryDate->date().toString(Qt::ISODate));
        level.insert(QStringLiteral("time"), expiryTime->time().toString(Qt::ISODate));
        level.insert(QStringLiteral("password"), password->text());
        levels.replace(currentLevel, level);
    };
    QObject::connect(expiryPage, &QComboBox::currentIndexChanged, settings, [&](int pageIndex) {
        QJsonObject level = levels.at(currentLevel).toObject();

        // Preserve an unedited inactive target while recording a page explicitly picked in the editor.
        level.insert(QStringLiteral("pageId"), expiryPage->itemData(pageIndex).toString());
        levels.replace(currentLevel, level);
    });
    auto switchLevel = [&](int next) {
        storeLevel();
        currentLevel = next;
        levelButtons->button(next)->setChecked(true);
        populateLevel();
    };
    auto showLevels = [&]() {
        const int visible = levelCount->currentIndex() + 1;
        int level;

        if (currentLevel >= visible)
        {
            switchLevel(0);
        }
        for (level = 0; level < 10; ++level)
        {
            levelButtons->button(level)->setVisible(level < visible);
        }
    };
    QObject::connect(levelButtons, &QButtonGroup::idClicked, settings, switchLevel);
    QObject::connect(levelCount, &QComboBox::currentIndexChanged, settings, showLevels);
    QObject::connect(enabled, &QCheckBox::toggled, enabledFields, &QWidget::setEnabled);
    populateLevel();
    showLevels();
    enabledFields->setEnabled(enabled->isChecked());

    warning = new QLabel(&dialog);
    warning->setGeometry(24, 345, 48, 40);
    warning->setPixmap(dialog.style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(40, 40));
    addLabel(&dialog, QStringLiteral("设置保护功能后，解除保护需要此密码。"), QRect(78, 339, 310, 24));
    addLabel(&dialog, QStringLiteral("解除密码使用功能按钮-面板保护解锁。"), QRect(78, 367, 310, 24));
    buttons = addButtons(dialog, QRect(153, 448, 235, 27), topic);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        const int activeCount = levelCount->currentIndex() + 1;
        QJsonArray savedLevels;
        QJsonObject level;
        int levelIndex;

        storeLevel();
        savedLevels = levels;
        if (enabled->isChecked())
        {
            for (levelIndex = 0; levelIndex < activeCount; ++levelIndex)
            {
                level = savedLevels.at(levelIndex).toObject();
                if (expiryPage->findData(level.value(QStringLiteral("pageId")).toString()) < 0)
                {
                    level.insert(QStringLiteral("pageId"), fallbackPage);
                    savedLevels.replace(levelIndex, level);
                }
            }
        }
        // Native confirmation persists only the retained level range. The open dialog keeps all edits until then.
        for (levelIndex = activeCount; levelIndex < savedLevels.size(); ++levelIndex) //20261123-18 add
        {
            savedLevels.replace(levelIndex, defaultLevel);
        }
        draft = {{QStringLiteral("enabled"), enabled->isChecked()},
            {QStringLiteral("levelCount"), activeCount}, {QStringLiteral("levels"), savedLevels}};
        if (!validateStage2ClockPanel(kind, draft, &error))
        {
            QMessageBox::critical(&dialog, QStringLiteral("错误"), error);
            return;
        }
        dialog.accept();
    });
    if (dialog.exec() != QDialog::Accepted)
    {
        return false;
    }
    *result = draft;
    return true;
}

} // namespace

/******************************************************************************
 * Function Name   : editStage2ClockPanel
 * Description     : Open a clock or protection editor on an isolated draft.
 * Input Parameters: parent, project, kind, initial, result
 * Return Value    : True when the user confirms a valid property record.
 * Date            : 2026.11.23
 ******************************************************************************/
bool editStage2ClockPanel(QWidget *parent, const HmiProject &project, const QString &kind,
                          const QJsonObject &initial, QJsonObject *result) //20261123-16 add
{
    const QJsonObject seed = initial.isEmpty() ? defaultStage2ClockPanel(kind) : initial;
    QDialog dialog(parent);

    if (result == nullptr || !validateStage2ClockPanel(kind, seed))
    {
        return false;
    }
    if (kind == QStringLiteral("clockSettings"))
    {
        return editClock(dialog, project, seed, result);
    }
    return editPanelProtection(dialog, project, seed, result);
}
