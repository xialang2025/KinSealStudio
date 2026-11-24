#include "stage2_io_settings.h"
#include "dialog_button_text.h" //20261124-8 add

#include "local_help.h"
#include "plc_address_dialog.h"
#include "project_model.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QFrame>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>

namespace {

//20261123-15 add: The observed row labels and control identities match the original dialogs.
struct IoRow
{
    const char *label;
    const char *checkName;
    const char *editName;
};

constexpr IoRow statusRows[] = {
    {"当前画面序号", "bCurSceneId", "lineEdit_screen"},
    {"当前配方序号", "bCurRecipeId", "lineEdit_formula"},
    {"当前用户等级", "bCurUserLevel", "lineEdit_userlevel"},
};

constexpr IoRow plcRows[] = {
    {"控制画面切换", "bCtlSceneSwitch", "lineEdit_sSwitchSceneAddr"},
    {"控制更改配方", "bCtlRecipeChange", "lineEdit_sChangeRecipeAddr"},
    {"控制写入配方", "bCtlRecipeWrite", "lineEdit_sWriteRecipeAddr"},
    {"控制当前权限等级", "bCtlCurUserId", "lineEdit_sCtlUserIdAddr"},
};

/******************************************************************************
 * Function Name   : ioAddressIcon
 * Description     : Draw a compact calculator glyph for the original address selection affordance.
 * Input Parameters: None.
 * Return Value    : A fixed 16-pixel icon with the Qt disabled state.
 * Date            : 2026.11.23
 ******************************************************************************/
QIcon ioAddressIcon()
{
    QPixmap pixmap(16, 16);
    QPainter painter;
    int x;
    int y;

    pixmap.fill(Qt::transparent);
    painter.begin(&pixmap);
    painter.setPen(QColor(78, 78, 78));
    painter.setBrush(QColor(228, 228, 228));
    painter.drawRoundedRect(QRect(1, 1, 13, 13), 2, 2);
    painter.fillRect(3, 3, 10, 3, QColor(30, 81, 130));
    for (y = 7; y <= 11; y += 2)
    {
        for (x = 3; x <= 11; x += 4)
        {
            painter.fillRect(x, y, 2, 1, QColor(40, 40, 97));
        }
    }
    painter.end();
    return QIcon(pixmap);
}

} // namespace

/******************************************************************************
 * Function Name   : editStage2IoSettings
 * Description     : Edit independent global I/O rows and commit only a valid configuration.
 * Input Parameters: parent, project, kind, initial, result
 * Return Value    : True on confirmed, validated settings.
 * Date            : 2026.11.23
 ******************************************************************************/
bool editStage2IoSettings(QWidget *parent, const HmiProject &project, const QString &kind,
                          const QJsonObject &initial, QJsonObject *result) //20261123-15 add
{
    const bool status = kind == QStringLiteral("hmiStatus");
    const QStringList keys = stage2IoSettingsKeys(kind);
    const QString title = status ? QStringLiteral("HMI状态") : QStringLiteral("PLC控制");
    const IoRow *rows = status ? statusRows : plcRows;
    const QString topic = status ? QStringLiteral("hmi-status") : QStringLiteral("plc-control");
    QJsonObject settings = initial.isEmpty() ? defaultStage2IoSettings(kind) : initial;
    QDialog dialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                           Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint |
                           Qt::MSWindowsFixedSizeDialogHint);
    QFont font(QStringLiteral("SimSun"));
    QFrame frame(&dialog);
    QLabel legend(status ? QStringLiteral("HMI信息写入PLC") : title, &dialog);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, &dialog);
    QCheckBox *enabled[4] = {};
    QLineEdit *address[4] = {};
    QPushButton *picker;
    QPushButton *button;
    QBoxLayout *buttonLayout;
    QJsonObject row;
    QString error;
    QString pickerName;
    int index;
    int y;

    if (result == nullptr || !validateStage2IoSettings(kind, settings, &error))
    {
        return false;
    }
    dialog.setWindowTitle(title);
    dialog.setObjectName(status ? QStringLiteral("HmiStatusDialog") : QStringLiteral("PlcControlDialog"));
    dialog.setProperty("localHelpTopic", topic);
    dialog.setFixedSize(status ? QSize(401, 331) : QSize(399, 313));
    if (parent != nullptr)
    {
        dialog.setWindowIcon(parent->windowIcon());
    }
    font.setPixelSize(12);
    font.setStyleStrategy(QFont::NoAntialias);
    dialog.setFont(font);
    dialog.setStyleSheet(QStringLiteral(
        "QDialog { background: #f0f0f0; }"
        "QFrame#ioSettingsFrame { border: 1px solid #dcdcdc; }"
        "QLabel#ioSettingsLegend { background: #f0f0f0; }"
        "QCheckBox { spacing: 3px; }"
        "QLineEdit { background: #caeace; border: 1px solid #abadb3; padding-left: 2px; }"
        "QLineEdit:disabled { background: #b1ceed; color: #6d6d6d; }"
        "QPushButton[addressPicker=true] { border: none; background: transparent; }"
        "QDialogButtonBox QPushButton { background: #dedede; border: 1px solid #a9a9a9; }"
        "QDialogButtonBox QPushButton:pressed { background: #c2c2c2; }"
        "QDialogButtonBox QPushButton:default { border: 2px solid #0078d7; }"));
    frame.setObjectName(QStringLiteral("ioSettingsFrame"));
    frame.setGeometry(11, 17, 371, status ? 265 : 245);
    legend.setObjectName(QStringLiteral("ioSettingsLegend"));
    legend.setGeometry(19, 8, status ? 89 : 51, 18);
    legend.setAlignment(Qt::AlignCenter);

    for (index = 0; index < keys.size(); ++index)
    {
        row = settings.value(keys.at(index)).toObject();
        y = (status ? 51 : 41) + index * 40;
        enabled[index] = new QCheckBox(QString::fromUtf8(rows[index].label), &dialog);
        enabled[index]->setObjectName(QString::fromLatin1(rows[index].checkName));
        enabled[index]->setGeometry(status ? 31 : 16, y, status ? 124 : 166, 21);
        enabled[index]->setChecked(row.value(QStringLiteral("enabled")).toBool());
        address[index] = new QLineEdit(row.value(QStringLiteral("address")).toString(), &dialog);
        address[index]->setObjectName(QString::fromLatin1(rows[index].editName));
        address[index]->setGeometry(status ? 161 : 191, y, status ? 171 : 160, 21);
        address[index]->setMaxLength(512);
        address[index]->setEnabled(enabled[index]->isChecked());
        pickerName = index == 0 ? QStringLiteral("pushButton_addrBegin") :
            QStringLiteral("pushButton_addrBegin_%1").arg(index + 1);
        picker = new QPushButton(ioAddressIcon(), QString(), &dialog);
        picker->setObjectName(pickerName);
        picker->setProperty("addressPicker", true);
        picker->setGeometry(status ? 344 : 354, y + 2, 16, 16);
        picker->setIconSize(QSize(16, 16));
        picker->setAccessibleName(QStringLiteral("选择地址"));
        picker->setEnabled(enabled[index]->isChecked());
        picker->setAutoDefault(false);
        QObject::connect(enabled[index], &QCheckBox::toggled, address[index], &QWidget::setEnabled);
        QObject::connect(enabled[index], &QCheckBox::toggled, picker, &QWidget::setEnabled);
        QObject::connect(picker, &QPushButton::clicked, &dialog, [&, field = address[index]]() {
            QString chosen = field->text();

            if (editPlcAddress(&dialog, false, project.connections, &chosen, 1, stage2IoLocalAddressCount))
            {
                field->setText(chosen);
            }
        });
    }

    buttons.setGeometry(status ? 154 : 152, status ? 298 : 280, 235, 21);
    buttonLayout = qobject_cast<QBoxLayout *>(buttons.layout());
    if (buttonLayout != nullptr)
    {
        buttonLayout->setContentsMargins(0, 0, 0, 0);
        buttonLayout->setSpacing(8);
    }
    buttons.button(QDialogButtonBox::Ok)->setText(dialogButtonText(QStringLiteral("确定"), "OK"));
    buttons.button(QDialogButtonBox::Cancel)->setText(dialogButtonText(QStringLiteral("取消"), "Cancel"));
    buttons.button(QDialogButtonBox::Help)->setText(dialogButtonText(QStringLiteral("帮助"), "Help"));
    Q_FOREACH (button, buttons.findChildren<QPushButton *>())
    {
        button->setFixedSize(73, 21);
    }
    buttons.button(QDialogButtonBox::Ok)->setDefault(true);

    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(&buttons, &QDialogButtonBox::helpRequested, &dialog, [&]() {
        LocalHelpController *help = dialog.findChild<LocalHelpController *>(QString(), Qt::FindDirectChildrenOnly);

        if (help == nullptr)
        {
            help = new LocalHelpController(&dialog);
        }
        help->showTopic(topic, &dialog);
    });
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        QJsonObject candidate;
        int index;

        for (index = 0; index < keys.size(); ++index)
        {
            candidate.insert(keys.at(index), QJsonObject{
                {QStringLiteral("enabled"), enabled[index]->isChecked()},
                {QStringLiteral("address"), address[index]->text().trimmed()}});
        }
        if (!validateStage2IoAddresses(kind, candidate, project.connections, &error))
        {
            QMessageBox::warning(&dialog, QStringLiteral("地址无效"), error);
            return;
        }
        settings = candidate;
        dialog.accept();
    });
    if (dialog.exec() != QDialog::Accepted)
    {
        return false;
    }
    *result = settings;
    return true;
}
