#include "plc_address_dialog.h"
#include "dialog_button_text.h" //20261124-8 add
#include "plc_native_catalog.h" //20261109-0 add

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QPointer> //20261114-8 add: Guard dialogs and their modal owners through nested event loops.
#include <QRegularExpression>
#include <QSet>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <functional>

//20261109-0 add: These widgets expose project choices, while node/route research stays outside the flow.
namespace
{
struct ConnectionControls
{
    QCheckBox *enabled = nullptr;
    QComboBox *profile = nullptr;
    QLineEdit *station = nullptr;
    QLabel *capability = nullptr;
    PlcConnection original;
    bool hasOriginal = false;
};

/******************************************************************************
 * Function Name   : profileCapabilities
 * Description     : Describe usable address families separately from device driver support.
 * Input Parameters: profile
 * Return Value    : A concise connection capability description.
 * Date            : 2026.11.09
 ******************************************************************************/
QString profileCapabilities(const PlcProfile *profile) //20261109-0 add
{
    PlcNativeFamilyRule rule;
    PlcFamily family;
    int supported;

    if (profile == nullptr) { return QStringLiteral("PLC配置不可用。"); }
    supported = 0;
    Q_FOREACH (family, profile->families)
    {
        if (plcNativeFamilyRule(*profile, family.name, &rule)) { ++supported; }
    }
    return QStringLiteral("可输入%1/%2种地址族。设备下载目前支持默认Modbus RTU COM1连接。")
        .arg(supported).arg(profile->families.size());
}

/******************************************************************************
 * Function Name   : connectionEditorError
 * Description     : Return a setup error before entering the transactional connection dialog.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.09
 ******************************************************************************/
bool connectionEditorError(QString *error, const QString &message)
{
    if (error != nullptr)
    {
        *error = message;
    }
    return false;
}

/******************************************************************************
 * Function Name   : populateAddressFamilies
 * Description     : Populate declared role families and retain an unavailable initial family explicitly.
 * Input Parameters: kind, connectionId, bit, context, catalog, preferred
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void populateAddressFamilies(QComboBox *kind, const QString &connectionId, bool bit,
                               const PlcAddressContext &context, const PlcCatalog &catalog,
                               const QString &preferred)
{
    QSignalBlocker blocker(kind);
    const PlcConnection *connection;
    QVector<PlcFamily> families;
    int index;

    kind->clear();
    if (connectionId.isEmpty())
    {
        kind->addItem(bit ? QStringLiteral("LB") : QStringLiteral("LW"),
                       bit ? QStringLiteral("LB") : QStringLiteral("LW"));
    }
    else
    {
        connection = plcConnection(context, connectionId);
        if (connection != nullptr)
        {
            families = catalog.families(connection->profileId,
                                        bit ? PlcAddressRole::Bit : PlcAddressRole::Word);
            for (index = 0; index < families.size(); ++index)
            {
                kind->addItem(families[index].name, families[index].name);
            }
        }
    }
    if (!preferred.isEmpty())
    {
        index = kind->findData(preferred);
        if (index < 0)
        {
            kind->addItem(preferred + QStringLiteral("（当前用途不可用）"), preferred);
            index = kind->count() - 1;
        }
        kind->setCurrentIndex(index);
    }
}
}

/******************************************************************************
 * Function Name   : editPlcSerialPort
 * Description     : Edit one serial port in a draft using the observed two-group layout.
 * Input Parameters: parent, port, settings, sourceNotice.
 * Return Value    : True only if validated changes were confirmed.
 * Date            : 2026.11.22
 ******************************************************************************/
bool editPlcSerialPort(QWidget *parent, const QString &port, QJsonObject *settings,
                       const QString &sourceNotice) //20261122-6 add
{
    QPointer<QDialog> dialog;
    QGroupBox *connection;
    QGroupBox *protocol;
    QLineEdit *name;
    QDialogButtonBox *buttons;
    QPushButton *defaultsButton;
    QMap<QString, QComboBox *> inputs;
    QJsonObject staged;
    QJsonObject factory;
    int result;

    if (settings == nullptr || (port != QStringLiteral("COM1") && port != QStringLiteral("COM2")))
    {
        return false;
    }
    dialog = new QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                         Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint);
    dialog->setObjectName(QStringLiteral("PlcSerialPortDialog"));
    dialog->setWindowTitle(QStringLiteral("连接设置"));
    dialog->setFixedSize(633, 356);
    dialog->setFont(QFont(QStringLiteral("SimSun"), 9));
    dialog->setStyleSheet(QStringLiteral(
        "QComboBox, QLineEdit { background: #caeace; border: 1px solid #a9a9a9; }"
        "QComboBox:disabled, QLineEdit:read-only { background: #b1ceed; color: #696969; }"
        "QGroupBox { border: 1px solid #d9d9d9; margin-top: 5px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 8px; }"));
    connection = new QGroupBox(QStringLiteral("连接属性"), dialog);
    connection->setGeometry(10, 11, 561, 99);
    protocol = new QGroupBox(QStringLiteral("协议属性"), dialog);
    protocol->setGeometry(10, 131, 561, 149);
    auto addLabel = [](QWidget *group, const QString &text, int x, int y) {
        QLabel *label = new QLabel(text, group);
        label->setGeometry(x, y, 120, 30);
        return label;
    };
    auto addInput = [&](QGroupBox *group, const QString &text, const QString &key,
                        const QString &objectName, const QPoint &labelPosition, const QPoint &boxPosition) {
        QComboBox *box = new QComboBox(group);
        addLabel(group, text, labelPosition.x(), labelPosition.y())->setBuddy(box);
        box->setObjectName(objectName);
        box->setGeometry(QRect(boxPosition, QSize(121, 30)));
        box->addItems(PlcSerialPortSettings::choices().value(key));
        box->setMaxVisibleItems(box->count());
        box->setEnabled(sourceNotice.isEmpty());
        inputs.insert(key, box);
    };
    name = new QLineEdit(port, connection);
    name->setObjectName(QStringLiteral("edit_name"));
    name->setReadOnly(true);
    name->setGeometry(130, 39, 121, 30);
    addLabel(connection, QStringLiteral("连接名称："), 20, 39)->setBuddy(name);
    addInput(connection, QStringLiteral("在线模拟端口："), QStringLiteral("onlinePort"),
             QStringLiteral("PlcPortOnline"), QPoint(270, 39), QPoint(400, 39));
    addInput(protocol, QStringLiteral("波特率："), QStringLiteral("baudRate"),
             QStringLiteral("PlcPortBaud"), QPoint(20, 19), QPoint(140, 19));
    addInput(protocol, QStringLiteral("停  止位："), QStringLiteral("stopBits"),
             QStringLiteral("PlcPortStopBits"), QPoint(280, 19), QPoint(400, 19));
    addInput(protocol, QStringLiteral("检  验："), QStringLiteral("parity"),
             QStringLiteral("PlcPortParity"), QPoint(20, 59), QPoint(140, 59));
    addInput(protocol, QStringLiteral("通信方式："), QStringLiteral("mode"),
             QStringLiteral("PlcPortMode"), QPoint(280, 59), QPoint(400, 59));
    addInput(protocol, QStringLiteral("数据位："), QStringLiteral("dataBits"),
             QStringLiteral("PlcPortDataBits"), QPoint(20, 99), QPoint(140, 99));
    factory = PlcSerialPortSettings::defaults(port);
    auto loadValues = [&inputs](const QJsonObject &values) {
        QMap<QString, QComboBox *>::const_iterator entry;
        for (entry = inputs.constBegin(); entry != inputs.constEnd(); ++entry)
        {
            entry.value()->setCurrentText(values.value(entry.key()).toString());
        }
    };
    loadValues(*settings);
    if (!sourceNotice.isEmpty())
    {
        addLabel(dialog, sourceNotice, 11, 279)->setFixedWidth(608);
    }
    defaultsButton = new QPushButton(QStringLiteral("恢复默认设置"), dialog);
    defaultsButton->setObjectName(QStringLiteral("PlcPortDefaults"));
    defaultsButton->setGeometry(11, 311, 209, 28);
    defaultsButton->setEnabled(sourceNotice.isEmpty());
    defaultsButton->setFocus();
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, dialog);
    buttons->setGeometry(341, 311, 228, 28);
    buttons->button(QDialogButtonBox::Ok)->setText(dialogButtonText(QStringLiteral("确定"), "OK"));
    buttons->button(QDialogButtonBox::Cancel)->setText(dialogButtonText(QStringLiteral("取消"), "Cancel"));
    buttons->button(QDialogButtonBox::Help)->setText(dialogButtonText(QStringLiteral("帮助"), "Help"));
    buttons->button(QDialogButtonBox::Ok)->setEnabled(sourceNotice.isEmpty());
    QObject::connect(defaultsButton, &QPushButton::clicked, dialog, [&]() { loadValues(factory); });
    QObject::connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::helpRequested, dialog, [&]() {
        QMessageBox::information(dialog, QStringLiteral("连接设置帮助"),
            QStringLiteral("连接名称是触摸屏上的串口。在线模拟端口是电脑上的串口。\n"
                           "恢复默认设置只更新本窗口，确定后才写入工程。"));
    });
    QObject::connect(buttons, &QDialogButtonBox::accepted, dialog, [&]() {
        QMap<QString, QComboBox *>::const_iterator entry;
        for (entry = inputs.constBegin(); entry != inputs.constEnd(); ++entry)
        {
            staged.insert(entry.key(), entry.value()->currentText());
        }
        dialog->accept();
    });
    result = dialog->exec();
    if (dialog.isNull())
    {
        return false;
    }
    delete dialog.data();
    if (result != QDialog::Accepted || !PlcSerialPortSettings::validate(staged))
    {
        return false;
    }
    *settings = staged;
    return true;
}

/******************************************************************************
 * Function Name   : editPlcProtocol
 * Description     : Edit one stable protocol instance in the original Settings page layout.
 * Input Parameters: parent, model, connection, readOnly, error.
 * Return Value    : True after a validated draft is confirmed.
 * Date            : 2026.11.22
 ******************************************************************************/
bool editPlcProtocol(QWidget *parent, const QString &model, PlcConnection *connection,
                     bool readOnly, QString *error) //20261122-7 add
{
    PlcCatalog catalog;
    QVector<PlcProfile> profiles;
    PlcProfile profile;
    PlcConnection staged;
    PlcConnection confirmed;
    QString selectedProfileId;
    const PlcProfile *selectedProfile;
    QPointer<QDialog> dialog;
    QTabWidget *tabs;
    QWidget *page;
    QComboBox *manufacturer;
    QComboBox *series;
    QLineEdit *name;
    QSpinBox *station;
    QSpinBox *timeout;
    QSpinBox *maximumWords;
    QSpinBox *retries;
    QSpinBox *interval;
    QLabel *validation;
    QFrame *separator;
    QPushButton *defaults;
    QDialogButtonBox *buttons;
    QAbstractButton *button;
    int result;
    bool adding;

    if (connection == nullptr || !catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error))
    {
        return false;
    }
    profiles = catalog.profilesForModel(model);
    selectedProfileId = connection->profileId;
    adding = selectedProfileId.isEmpty(); //20261122-7 add: An empty draft starts on the native default series.
    if (adding)
    {
        const auto initial = std::find_if(profiles.cbegin(), profiles.cend(), [](const PlcProfile &entry) {
            return entry.name == QStringLiteral("Mitsubishi_Fx3U");
        });
        if (initial != profiles.cend())
        {
            selectedProfileId = initial->id;
        }
    }
    selectedProfile = catalog.profile(selectedProfileId);
    if (selectedProfile == nullptr || std::none_of(profiles.cbegin(), profiles.cend(), [&](const PlcProfile &entry) {
            return entry.id == selectedProfileId;
        }))
    {
        return connectionEditorError(error, QStringLiteral("当前型号目录没有此工程的 PLC 协议。"));
    }
    dialog = new QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                         Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint);
    dialog->setObjectName(QStringLiteral("PlcProtocolDialog"));
    dialog->setWindowTitle(QStringLiteral("协议"));
    dialog->setFixedSize(405, 504);
    dialog->setFont(QFont(QStringLiteral("SimSun"), 9));
    dialog->setStyleSheet(QStringLiteral(
        "QWidget#PlcProtocolPage { background: white; }"
        "QComboBox, QLineEdit, QSpinBox { background: #caeace; border: 1px solid #a9a9a9; }"
        "QSpinBox QLineEdit { border: none; }"
        "QComboBox:disabled, QLineEdit:disabled, QSpinBox:disabled { background: #b1ceed; }"));
    tabs = new QTabWidget(dialog);
    tabs->setGeometry(10, 10, 379, 460);
    page = new QWidget(tabs);
    page->setObjectName(QStringLiteral("PlcProtocolPage"));
    tabs->addTab(page, QStringLiteral("设置"));
    auto label = [page](const QString &text, int x, int y, int width) {
        QLabel *item = new QLabel(text, page);
        item->setGeometry(x, y, width, 24);
        return item;
    };
    manufacturer = new QComboBox(page);
    manufacturer->setObjectName(QStringLiteral("combo_manu"));
    manufacturer->setGeometry(130, 19, 221, 22);
    manufacturer->setEnabled(!readOnly);
    label(QStringLiteral("制造商："), 20, 18, 100)->setBuddy(manufacturer);
    series = new QComboBox(page);
    series->setObjectName(QStringLiteral("combo_protocol"));
    series->setGeometry(130, 54, 221, 22);
    series->setEnabled(!readOnly);
    label(QStringLiteral("系列："), 20, 53, 100)->setBuddy(series);
    name = new QLineEdit(page);
    name->setObjectName(QStringLiteral("edit_name"));
    name->setGeometry(130, 89, 221, 22);
    name->setMaxLength(48);
    name->setReadOnly(readOnly);
    label(QStringLiteral("名称："), 20, 88, 100)->setBuddy(name);
    label(QStringLiteral("通讯设置"), 15, 124, 60);
    separator = new QFrame(page);
    separator->setFrameShape(QFrame::HLine);
    separator->setGeometry(66, 135, 298, 1);
    auto spin = [page, &label, readOnly](const QString &caption, const QString &objectName,
                                        const QString &unit, int minimum, int maximum, int y) {
        QSpinBox *field = new QSpinBox(page);
        field->setObjectName(objectName);
        field->setGeometry(134, y, 171, 25);
        field->setRange(minimum, maximum);
        field->setReadOnly(readOnly);
        label(caption, 20, y, 110)->setBuddy(field);
        if (!unit.isEmpty())
        {
            label(unit, 312, y, 45);
        }
        return field;
    };
    station = spin(QStringLiteral("站号："), QStringLiteral("spin_stationIndex"), {}, 0, 255, 152);
    timeout = spin(QStringLiteral("超时时间："), QStringLiteral("spin_time"), QStringLiteral("毫秒"), 50, 300000, 188);
    maximumWords = spin(QStringLiteral("最大读写长度："), QStringLiteral("spin_length"), QStringLiteral("字"), 1, 256, 224);
    retries = spin(QStringLiteral("重试次数："), QStringLiteral("spin_retry"), QStringLiteral("次"), 0, 10, 260);
    interval = spin(QStringLiteral("帧间隔时间："), QStringLiteral("spin_interval"), QStringLiteral("毫秒"), 0, 30000, 296);
    validation = label(readOnly ? QStringLiteral("原厂工程中的协议配置仅供查看。") : QString(), 24, 379, 328);
    validation->setStyleSheet(QStringLiteral("background: #f7f7f7;"));
    Q_FOREACH (profile, profiles)
    {
        if (manufacturer->findData(profile.manufacturer) < 0)
        {
            manufacturer->addItem(profile.manufacturer == QStringLiteral("MITSUBISHI") ?
                QStringLiteral("MITSUBISHI(三菱)") : profile.manufacturer, profile.manufacturer);
        }
    }
    auto populateSeries = [&](const QString &preferred) {
        PlcProfile entry;
        QSignalBlocker blocker(series);
        int index;
        series->clear();
        Q_FOREACH (entry, profiles)
        {
            if (entry.manufacturer == manufacturer->currentData().toString())
            {
                series->addItem(entry.name, entry.id);
            }
        }
        index = series->findData(preferred);
        series->setCurrentIndex(index >= 0 ? index : 0);
    };
    auto applyDefaults = [&](bool resetName) {
        const PlcProfile *entry = catalog.profile(series->currentData().toString());
        QJsonObject values;
        if (entry == nullptr)
        {
            return;
        }
        values = entry->serialDefaults;
        station->setValue(values.value(QStringLiteral("DefaultStation")).toString(QStringLiteral("0")).toInt());
        timeout->setValue(values.value(QStringLiteral("DefaultTimeout")).toString(QStringLiteral("1000")).toInt());
        maximumWords->setValue(values.value(QStringLiteral("DefaultMaxRwLen")).toString(QStringLiteral("32")).toInt());
        retries->setValue(values.value(QStringLiteral("DefaultRetryTimes")).toString(QStringLiteral("3")).toInt());
        interval->setValue(values.value(QStringLiteral("DefaultIntervalTime")).toString(QStringLiteral("20")).toInt());
        if (resetName)
        {
            name->setText(entry->name);
        }
    };
    manufacturer->setCurrentIndex(manufacturer->findData(selectedProfile->manufacturer));
    populateSeries(selectedProfileId);
    name->setText(connection->displayName.isEmpty() ? selectedProfile->name : connection->displayName);
    station->setValue(connection->station.toInt());
    timeout->setValue(connection->timeoutMs);
    maximumWords->setValue(connection->maximumWords);
    retries->setValue(connection->retries);
    interval->setValue(connection->frameIntervalMs);
    if (adding)
    {
        applyDefaults(true);
    }
    QObject::connect(manufacturer, &QComboBox::currentIndexChanged, dialog, [&]() { populateSeries({}); applyDefaults(true); });
    QObject::connect(series, &QComboBox::currentIndexChanged, dialog, [&]() { applyDefaults(true); });
    defaults = new QPushButton(QStringLiteral("恢复默认值"), dialog);
    defaults->setGeometry(11, 476, 129, 24);
    defaults->setEnabled(!readOnly);
    QObject::connect(defaults, &QPushButton::clicked, dialog, [&]() { applyDefaults(false); });
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, dialog);
    buttons->setGeometry(191, 476, 198, 24);
    buttons->button(QDialogButtonBox::Ok)->setText(dialogButtonText(QStringLiteral("确定"), "OK"));
    buttons->button(QDialogButtonBox::Cancel)->setText(dialogButtonText(QStringLiteral("取消"), "Cancel"));
    buttons->button(QDialogButtonBox::Help)->setText(dialogButtonText(QStringLiteral("帮助"), "Help"));
    buttons->button(QDialogButtonBox::Ok)->setEnabled(!readOnly);
    Q_FOREACH (button, buttons->buttons())
    {
        button->setFixedSize(58, 24);
    }
    QObject::connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::helpRequested, dialog, [&]() {
        QMessageBox::information(dialog, QStringLiteral("协议帮助"),
            QStringLiteral("选择制造商和系列后，可设置本协议的名称、站号和通讯参数。"));
    });
    QObject::connect(buttons, &QDialogButtonBox::accepted, dialog, [&]() {
        QString message;
        QString newName = name->text().trimmed();
        if (newName.isEmpty())
        {
            validation->setText(QStringLiteral("请输入协议名称。"));
            return;
        }
        staged = *connection;
        staged.displayName = newName == series->currentText() ? QString() : newName;
        staged.profileId = series->currentData().toString();
        staged.station = QString::number(station->value());
        staged.timeoutMs = timeout->value();
        staged.maximumWords = maximumWords->value();
        staged.retries = retries->value();
        staged.frameIntervalMs = interval->value();
        // A driver or station change cannot carry a native node from the previous route.
        if (staged.profileId != connection->profileId || staged.station != connection->station)
        {
            staged.nodeKnown = false;
            staged.node = 0;
        }
        if (staged.channel == PlcChannel::Com1 && staged.station == QStringLiteral("1") &&
            staged.profileId == QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini") &&
            staged.usesDefaultProtocolTiming())
        {
            // This exact combination has an independently verified native node.
            staged.nodeKnown = true;
            staged.node = 1;
        }
        if (!PlcConnection::fromJson(staged.toJson(), &confirmed, &message))
        {
            validation->setText(message);
            return;
        }
        dialog->accept();
    });
    result = dialog->exec();
    if (dialog.isNull())
    {
        return false;
    }
    delete dialog.data();
    if (result != QDialog::Accepted || confirmed.id.isEmpty())
    {
        return false;
    }
    *connection = confirmed;
    return true;
}

/******************************************************************************
 * Function Name   : editPlcConnections
 * Description     : Select observed model profiles and independent station values for two stable ports.
 * Input Parameters: parent, modelName, connections, error
 * Return Value    : True after confirmation; cancellation and failures preserve the input vector
 * Date            : 2026.11.09
 ******************************************************************************/
bool editPlcConnections(QWidget *parent, const QString &modelName,
                         QVector<PlcConnection> *connections, QString *error) //20261109-0 add //20261114-8 change
{
    PlcCatalog catalog;
    QVector<PlcProfile> profiles;
    QVector<PlcConnection> staged;
    ConnectionControls controls[2];
    QSet<QString> ids;
    QPointer<QDialog> dialog; //20261114-8 change: A parent may destroy the child while exec is still active.
    QPointer<QWidget> guardedParent(parent);
    bool confirmed = false;
    int modalResult;
    QVBoxLayout *layout;
    QGroupBox *group;
    QGridLayout *grid;
    QDialogButtonBox *buttons;
    QLabel *validation;
    PlcConnection original;
    const PlcProfile *selectedProfile;
    QString id;
    QString defaultProfile;
    QString loadError;
    int index;
    int port;
    int selected;

    if (connections == nullptr)
    {
        return connectionEditorError(error, QStringLiteral("PLC 连接集合为空。"));
    }
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), &loadError))
    {
        return connectionEditorError(error, loadError);
    }
    profiles = catalog.profilesForModel(modelName);
    if (profiles.isEmpty())
    {
        return connectionEditorError(error, QStringLiteral("没有找到型号 %1 对应的 PLC 目录。").arg(modelName));
    }
    for (index = 0; index < connections->size(); ++index)
    {
        original = connections->at(index);
        if ((original.id != QStringLiteral("com1") && original.id != QStringLiteral("com2")) ||
            ids.contains(original.id) || (original.id == QStringLiteral("com1") && original.channel != PlcChannel::Com1) ||
            (original.id == QStringLiteral("com2") && original.channel != PlcChannel::Com2))
        {
            return connectionEditorError(error, QStringLiteral("PLC 连接需要互不重复且对应端口的 com1/com2 标识。"));
        }
        ids.insert(original.id);
        port = original.id == QStringLiteral("com1") ? 0 : 1;
        controls[port].original = original;
        controls[port].hasOriginal = true;
    }
    dialog = new QDialog(parent); //20261114-8 add: Allocate only after preflight validation succeeds.
    dialog->setWindowTitle(QStringLiteral("PLC 连接设置"));
    dialog->setObjectName(QStringLiteral("PlcConnectionDialog"));
    layout = new QVBoxLayout(dialog.data());
    layout->addWidget(new QLabel(QStringLiteral("型号：%1").arg(modelName), dialog.data()));
    defaultProfile = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    for (port = 0; port < 2; ++port)
    {
        id = port == 0 ? QStringLiteral("PlcCom1") : QStringLiteral("PlcCom2");
        group = new QGroupBox(port == 0 ? QStringLiteral("COM1") : QStringLiteral("COM2"), dialog.data());
        grid = new QGridLayout(group);
        controls[port].enabled = new QCheckBox(QStringLiteral("启用连接"), group);
        controls[port].enabled->setObjectName(id + QStringLiteral("Enabled"));
        controls[port].enabled->setChecked(controls[port].hasOriginal);
        controls[port].profile = new QComboBox(group);
        controls[port].profile->setObjectName(id + QStringLiteral("Profile"));
        for (index = 0; index < profiles.size(); ++index)
        {
            controls[port].profile->addItem(profiles[index].manufacturer + QStringLiteral(" / ") +
                                             profiles[index].name, profiles[index].id);
        }
        selected = controls[port].profile->findData(controls[port].hasOriginal ?
                    controls[port].original.profileId : defaultProfile);
        if (selected < 0 && controls[port].hasOriginal)
        {
            controls[port].profile->addItem(controls[port].original.profileId + QStringLiteral("（当前型号不可用）"),
                                             controls[port].original.profileId);
            selected = controls[port].profile->count() - 1;
        }
        controls[port].profile->setCurrentIndex(selected < 0 ? 0 : selected);
        controls[port].station = new QLineEdit(group);
        controls[port].station->setObjectName(id + QStringLiteral("Station"));
        controls[port].station->setText(controls[port].hasOriginal ? controls[port].original.station :
            catalog.profile(controls[port].profile->currentData().toString())->serialDefaults.value(QStringLiteral("DefaultStation")).toString());
        controls[port].capability = new QLabel(group);
        controls[port].capability->setWordWrap(true);
        grid->addWidget(controls[port].enabled, 0, 0, 1, 2);
        grid->addWidget(new QLabel(QStringLiteral("PLC 型号"), group), 1, 0);
        grid->addWidget(controls[port].profile, 1, 1);
        grid->addWidget(new QLabel(QStringLiteral("站号（十进制）"), group), 2, 0);
        grid->addWidget(controls[port].station, 2, 1);
        grid->addWidget(controls[port].capability, 3, 0, 1, 2);
        layout->addWidget(group);
        QObject::connect(controls[port].enabled, &QCheckBox::toggled, dialog.data(), [&, port](bool enabled) {
            controls[port].profile->setEnabled(enabled);
            controls[port].station->setEnabled(enabled);
        });
        QObject::connect(controls[port].profile, &QComboBox::currentIndexChanged, dialog.data(), [&, port]() {
            const PlcProfile *profile;

            profile = catalog.profile(controls[port].profile->currentData().toString());
            controls[port].station->setText(profile == nullptr ? QString() :
                profile->serialDefaults.value(QStringLiteral("DefaultStation")).toString());
            controls[port].capability->setText(profileCapabilities(profile)); //20261109-0 change
        });
        controls[port].profile->setEnabled(controls[port].hasOriginal);
        controls[port].station->setEnabled(controls[port].hasOriginal);
        selectedProfile = catalog.profile(controls[port].profile->currentData().toString());
        controls[port].capability->setText(profileCapabilities(selectedProfile)); //20261109-0 change
    }
    validation = new QLabel(dialog.data());
    validation->setObjectName(QStringLiteral("PlcConnectionError"));
    validation->setWordWrap(true);
    layout->addWidget(validation);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog.data());
    buttons->setObjectName(QStringLiteral("PlcConnectionButtons"));
    buttons->button(QDialogButtonBox::Ok)->setText(dialogButtonText(QStringLiteral("确认"), "OK"));
    buttons->button(QDialogButtonBox::Cancel)->setText(dialogButtonText(QStringLiteral("取消"), "Cancel"));
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::rejected, dialog.data(), &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::accepted, dialog.data(), [&]() {
        PlcConnection connection;
        QString profileId;
        int selectedPort;
        int profileIndex;
        bool available;

        staged.clear();
        for (selectedPort = 0; selectedPort < 2; ++selectedPort)
        {
            if (!controls[selectedPort].enabled->isChecked())
            {
                continue;
            }
            profileId = controls[selectedPort].profile->currentData().toString();
            available = false;
            for (profileIndex = 0; profileIndex < profiles.size(); ++profileIndex)
            {
                if (profiles[profileIndex].id == profileId)
                {
                    available = true;
                    break;
                }
            }
            if (!available || !QRegularExpression(QStringLiteral("^[0-9]+$")).match(controls[selectedPort].station->text().trimmed()).hasMatch())
            {
                validation->setText(QStringLiteral("请选择当前型号可用的 PLC，并填写十进制站号。"));
                return;
            }
            connection = controls[selectedPort].hasOriginal ? controls[selectedPort].original : PlcConnection();
            if (connection.profileId != profileId)
            {
                connection.nodeKnown = false; // A profile change invalidates any previously verified source node.
                connection.node = 0;
            }
            connection.id = selectedPort == 0 ? QStringLiteral("com1") : QStringLiteral("com2");
            connection.channel = selectedPort == 0 ? PlcChannel::Com1 : PlcChannel::Com2;
            connection.profileId = profileId;
            connection.station = controls[selectedPort].station->text().trimmed();
            if (!connection.nodeKnown && selectedPort == 0 && connection.station == QStringLiteral("1") &&
                connection.profileId == QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini"))
            {
                //20261109-0 add: This exact default COM1 configuration has an independent native node-one oracle.
                connection.nodeKnown = true; connection.node = 1;
            }
            staged.append(connection);
        }
        confirmed = true; //20261114-8 add: An external accept cannot publish an unvalidated result.
        dialog->accept();
    });
    if (error != nullptr)
    {
        error->clear();
    }
    modalResult = dialog->exec(); //20261114-8 change: The owned child may already have been deleted by its owner.
    if (dialog.isNull()) { return false; }
    delete dialog.data();
    if (modalResult != QDialog::Accepted || !confirmed || (parent != nullptr && guardedParent.isNull()))
    {
        return false;
    }
    *connections = staged;
    return true;
}

/******************************************************************************
 * Function Name   : editPlcAddress
 * Description     : Edit a role-filtered address without rewriting text on cancellation or failed parsing.
 * Input Parameters: parent, bit, connections, qualifiedText
 * Return Value    : True only after a validated address is committed
 * Date            : 2026.11.09
 ******************************************************************************/
bool editPlcAddress(QWidget *parent, bool bit, const QVector<PlcConnection> &connections,
                    QString *qualifiedText, int transferCount, u32 localAddressCount) //20261123-15 change
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress initial;
    PlcAddress committed;
    QPointer<QDialog> dialog; //20261114-8 change: A parent may destroy the child while exec is still active.
    QPointer<QWidget> guardedParent(parent);
    bool confirmed = false;
    int modalResult;
    QComboBox *memory;
    QComboBox *kind;
    QComboBox *radix;
    QLineEdit *number;
    QLabel *range;
    QLabel *inputType;
    QLabel *validation;
    QDialogButtonBox *buttons;
    QPushButton *keyButton;
    QVector<QPushButton *> keypad;
    QString keys;
    int keyIndex;
    const PlcProfile *profile;
    const PlcConnection *connection;
    QString error;
    QString connectionId;
    QString body;
    QString initialFamily;
    QString initialValue;
    std::function<void()> refresh;
    std::function<void()> resetAddressValue; //20261109-0 add
    int previousRadix;
    qsizetype delimiter; //20261114-8 change: Keep QString offsets in their native index width.
    int index;
    int selected;

    if (qualifiedText == nullptr || transferCount < 1 || transferCount > 2 ||
        localAddressCount < u32(transferCount) || localAddressCount > 65536) //20261123-15 change
    {
        return false;
    }
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), &error))
    {
        QMessageBox::warning(parent, QStringLiteral("PLC 目录不可用"), error);
        return false;
    }
    context.connections = connections;
    context.role = bit ? PlcAddressRole::Bit : PlcAddressRole::Word;
    context.transferCount = u32(transferCount); //20261109-6 add: Validate the complete DWORD span in the live picker as well as the compiler.
    context.localAddressCount = localAddressCount; //20261123-15 add: Global settings use the wider range shown by the original editor.
    body = qualifiedText->trimmed();
    if (body.isEmpty())
    {
        body = bit ? QStringLiteral("LB0") : QStringLiteral("LW0");
    }
    if (parseQualifiedPlcAddressText(body, context, catalog, &initial, &error))
    {
        connectionId = initial.connectionId;
        initialFamily = initial.family;
        initialValue = initial.value;
    }
    else
    {
        if (body.startsWith(QChar(u'@')))
        {
            delimiter = body.indexOf(QChar(u':'));
            if (delimiter > 1)
            {
                connectionId = body.mid(1, delimiter - 1);
                body = body.mid(delimiter + 1);
            }
        }
        if (connectionId.isEmpty())
        {
            initialFamily = body.left(2).toUpper();
        }
        else
        {
            connection = plcConnection(context, connectionId);
            profile = connection == nullptr ? nullptr : catalog.profile(connection->profileId);
            if (profile != nullptr)
            {
                for (index = 0; index < profile->families.size(); ++index)
                {
                    if (body.startsWith(profile->families[index].name, Qt::CaseInsensitive) &&
                        profile->families[index].name.size() > initialFamily.size())
                    {
                        initialFamily = profile->families[index].name;
                    }
                }
            }
        }
        initialValue = body.mid(initialFamily.size());
    }
    dialog = new QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                         Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint); //20261122-8 change
    dialog->setWindowTitle(QStringLiteral("地址输入"));
    dialog->setObjectName(QStringLiteral("PlcAddressDialog"));
    dialog->setFixedSize(380, 311);
    dialog->setFont(QFont(QStringLiteral("SimSun"), 9));
    dialog->setStyleSheet(QStringLiteral(
        "QComboBox, QLineEdit { background: #caeace; border: 1px solid #a9a9a9; }"
        "QComboBox:disabled, QLineEdit:disabled { background: #b1ceed; }"
        "QPushButton { background: #dedede; border: 1px solid #a9a9a9; }"
        "QPushButton:pressed { background: #c2c2c2; }"
        "QPushButton:default { border: 2px solid #0078d7; }"));
    memory = new QComboBox(dialog.data());
    memory->setObjectName(QStringLiteral("PlcMemory"));
    memory->addItem(QStringLiteral("内部存储器"), QString());
    for (index = 0; index < connections.size(); ++index)
    {
        profile = catalog.profile(connections[index].profileId);
        QString port = connections[index].channel == PlcChannel::Com1 ? QStringLiteral("COM1") :
            connections[index].channel == PlcChannel::Com2 ? QStringLiteral("COM2") : connections[index].id.toUpper();
        QString name = connections[index].displayName; //20261122-7 change: Keep qualified address IDs separate from the visible route.
        if (name.isEmpty())
        {
            name = profile == nullptr ? QStringLiteral("未知 PLC") : profile->name;
        }
        memory->addItem(port + QStringLiteral(" / ") + name, connections[index].id);
    }
    selected = memory->findData(connectionId);
    if (selected < 0)
    {
        memory->addItem(connectionId + QStringLiteral("（连接不可用）"), connectionId);
        selected = memory->count() - 1;
    }
    memory->setCurrentIndex(selected);
    kind = new QComboBox(dialog.data());
    kind->setObjectName(QStringLiteral("PlcFamily"));
    populateAddressFamilies(kind, connectionId, bit, context, catalog, initialFamily);
    radix = new QComboBox(dialog.data());
    radix->setObjectName(QStringLiteral("PlcRadix"));
    radix->addItem(QStringLiteral("十进制"), 10);
    radix->addItem(QStringLiteral("十六进制"), 16);
    previousRadix = 10;
    number = new QLineEdit(initialValue, dialog.data());
    number->setObjectName(QStringLiteral("PlcValue"));
    range = new QLabel(dialog.data());
    range->setObjectName(QStringLiteral("PlcAddressRange")); //20261114-8 add: Retain the explicit span in UI evidence.
    range->setWordWrap(true);
    inputType = new QLabel(QStringLiteral("输入类型："), dialog);
    validation = new QLabel(error, dialog.data());
    validation->setObjectName(QStringLiteral("PlcAddressError"));
    validation->setWordWrap(true);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog.data());
    buttons->setObjectName(QStringLiteral("PlcAddressButtons"));
    // The input key spans the last two columns while Leave occupies the right return key.
    buttons->setGeometry(211, 201, 98, 78);
    buttons->layout()->setEnabled(false);
    buttons->button(QDialogButtonBox::Ok)->setText(dialogButtonText(QStringLiteral("输入"), "Enter")); //20261124-8 change
    buttons->button(QDialogButtonBox::Ok)->setGeometry(0, 40, 98, 38);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    buttons->button(QDialogButtonBox::Cancel)->setText(dialogButtonText(QStringLiteral("离开"), "Exit"));
    buttons->button(QDialogButtonBox::Cancel)->setGeometry(50, 0, 48, 38);
    buttons->button(QDialogButtonBox::Cancel)->setAutoDefault(false);
    memory->setGeometry(10, 10, 160, 22);
    memory->setAccessibleName(QStringLiteral("存储器"));
    kind->setGeometry(10, 40, 160, 22);
    kind->setAccessibleName(QStringLiteral("寄存器"));
    inputType->setGeometry(179, 10, 76, 22);
    inputType->setBuddy(radix);
    radix->setGeometry(257, 10, 109, 22);
    range->setGeometry(179, 38, 193, 29);
    number->setGeometry(180, 70, 131, 22);
    validation->setGeometry(11, 280, 358, 30);
    auto makeKey = [&](const QString &caption, int column, int row) {
        QPushButton *key = new QPushButton(caption, dialog);
        key->setObjectName(QStringLiteral("PlcKey") + caption);
        key->setGeometry(11 + column * 50, 121 + row * 40, 48, 38);
        key->setFocusPolicy(Qt::NoFocus);
        key->setAutoDefault(false);
        keypad.append(key);
        return key;
    };
    keys = QStringLiteral("1234567890ABCDEF./:");
    for (keyIndex = 0; keyIndex < keys.size(); ++keyIndex)
    {
        const QString value = keys.mid(keyIndex, 1);
        keyButton = makeKey(value, keyIndex % 5, keyIndex / 5);
        QObject::connect(keyButton, &QPushButton::clicked, number, [number, value]() { number->insert(value); });
    }
    keyButton = makeKey(QStringLiteral("清除"), 5, 0);
    QObject::connect(keyButton, &QPushButton::clicked, number, &QLineEdit::clear);
    keyButton = makeKey(QStringLiteral("后退"), 5, 1);
    QObject::connect(keyButton, &QPushButton::clicked, number, &QLineEdit::backspace);
    number->setFocus();
    number->selectAll();

    refresh = [&]() {
        const PlcConnection *selectedConnection;
        const PlcProfile *selectedProfile;
        QString family;
        QString rangeText;
        PlcNativeFamilyRule rule; //20261109-0 add
        PlcAddress parsed; //20261114-8 add: Use the same parser as confirmation for live draft validity.
        QString parseError;
        bool validDraft;
        bool supported;
        bool standardRadix;
        int familyIndex;
        QPushButton *key;
        QString rangeHelp;

        family = kind->currentData().toString();
        selectedConnection = plcConnection(context, memory->currentData().toString());
        selectedProfile = selectedConnection == nullptr ? nullptr : catalog.profile(selectedConnection->profileId);
        supported = memory->currentData().toString().isEmpty() ||
                    (selectedProfile != nullptr && plcNativeFamilyRule(*selectedProfile, family, &rule)); //20261109-0 change
        standardRadix = memory->currentData().toString().isEmpty() ||
                        (selectedProfile != nullptr && plcProfileHasModbusCodec(*selectedProfile));
        rangeText = memory->currentData().toString().isEmpty() ?
            QStringLiteral("地址范围：0~%1").arg(context.localAddressCount - context.transferCount) : QString(); //20261123-15 change
        if (selectedProfile != nullptr)
        {
            for (familyIndex = 0; familyIndex < selectedProfile->families.size(); ++familyIndex)
            {
                if (selectedProfile->families[familyIndex].name == family)
                {
                    rangeText = QStringLiteral("地址范围：") + selectedProfile->families[familyIndex].rawRange;
                    break;
                }
            }
        }
        range->setText(rangeText);
        rangeHelp = rangeText;
        if (standardRadix && family.endsWith(QStringLiteral("_bit")))
        {
            rangeHelp += QStringLiteral("\nA.B 表示字地址和位号。位号 0~15，始终使用十进制。");
        }
        rangeHelp += QStringLiteral("\n连续传送数量：%1").arg(context.transferCount);
        range->setToolTip(rangeHelp);
        number->setEnabled(supported);
        Q_FOREACH (key, keypad)
        {
            key->setEnabled(supported);
        }
        radix->setItemText(0, standardRadix ? QStringLiteral("十进制") : QStringLiteral("原生进制")); //20261109-0 change
        radix->setEnabled(supported && standardRadix);
        validDraft = supported && !family.isEmpty() &&
            parsePlcAddress(family, number->text(), memory->currentData().toString(), context, catalog,
                            &parsed, &parseError, radix->currentData().toInt()); //20261114-8 change
        buttons->button(QDialogButtonBox::Ok)->setEnabled(validDraft);
        validation->setText(supported ? parseError : QStringLiteral("此 PLC 的地址输入暂未支持，请选择其他连接或取消。"));
    };
    QObject::connect(number, &QLineEdit::textChanged, dialog.data(), [&]() {
        refresh(); //20261114-8 add: Keep malformed text editable and clear only errors the parser no longer reports.
    });
    resetAddressValue = [&]() {
        const PlcConnection *selectedConnection;
        const PlcProfile *selectedProfile;
        QString value;
        QString initialError;

        selectedConnection = plcConnection(context, memory->currentData().toString());
        selectedProfile = selectedConnection == nullptr ? nullptr : catalog.profile(selectedConnection->profileId);
        value = kind->currentData().toString().endsWith(QStringLiteral("_bit")) ? QStringLiteral("0.00") : QStringLiteral("0");
        if (selectedProfile != nullptr && !plcProfileHasModbusCodec(*selectedProfile) &&
            !plcNativeInitialAddressValue(*selectedProfile, kind->currentData().toString(), &value, &initialError))
        {
            value.clear();
        }
        number->setText(value);
    }; //20261109-0 add: Seed native bit, bank and nonzero-minimum families with valid syntax.
    QObject::connect(memory, &QComboBox::currentIndexChanged, dialog.data(), [&]() {
        QSignalBlocker blocker(radix);

        radix->setCurrentIndex(0);
        previousRadix = 10;
        populateAddressFamilies(kind, memory->currentData().toString(), bit, context, catalog, QString());
        resetAddressValue();
        refresh();
    });
    QObject::connect(kind, &QComboBox::currentIndexChanged, dialog.data(), [&]() {
        resetAddressValue(); refresh();
    }); //20261109-0 change
    QObject::connect(radix, &QComboBox::currentIndexChanged, dialog.data(), [&]() {
        QSignalBlocker blocker(radix);
        PlcAddress parsed;
        QString parseError;
        int newRadix;

        newRadix = radix->currentData().toInt();
        if (!parsePlcAddress(kind->currentData().toString(), number->text(), memory->currentData().toString(),
                              context, catalog, &parsed, &parseError, previousRadix))
        {
            radix->setCurrentIndex(previousRadix == 10 ? 0 : 1);
            validation->setText(parseError);
            return;
        }
        number->setText(plcAddressValueText(parsed, newRadix));
        previousRadix = newRadix;
        refresh();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, dialog.data(), &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::accepted, dialog.data(), [&]() {
        QString parseError;

        if (!parsePlcAddress(kind->currentData().toString(), number->text(), memory->currentData().toString(),
                              context, catalog, &committed, &parseError, radix->currentData().toInt()))
        {
            validation->setText(parseError);
            return;
        }
        confirmed = true; //20261114-8 add: An external accept cannot publish an unvalidated result.
        dialog->accept();
    });
    refresh();
    modalResult = dialog->exec(); //20261114-8 change: Never access widgets or caller output after owner destruction.
    if (dialog.isNull()) { return false; }
    delete dialog.data();
    if (modalResult != QDialog::Accepted || !confirmed || (parent != nullptr && guardedParent.isNull()))
    {
        return false;
    }
    *qualifiedText = plcQualifiedAddressText(committed);
    return true;
}
