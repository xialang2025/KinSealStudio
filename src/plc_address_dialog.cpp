#include "plc_address_dialog.h"
#include "plc_native_catalog.h" //20261109-0 add

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
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
#include <QVBoxLayout>
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
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
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
                    QString *qualifiedText, int transferCount) //20261109-6 change //20261114-8 change
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress initial;
    PlcAddress committed;
    QPointer<QDialog> dialog; //20261114-8 change: A parent may destroy the child while exec is still active.
    QPointer<QWidget> guardedParent(parent);
    bool confirmed = false;
    int modalResult;
    QGridLayout *layout;
    QComboBox *memory;
    QComboBox *kind;
    QComboBox *radix;
    QLineEdit *number;
    QLabel *range;
    QLabel *validation;
    QDialogButtonBox *buttons;
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

    if (qualifiedText == nullptr || transferCount < 1 || transferCount > 2) //20261109-6 change
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
    dialog = new QDialog(parent); //20261114-8 add: Allocate only after preflight validation succeeds.
    dialog->setWindowTitle(QStringLiteral("地址输入"));
    dialog->setObjectName(QStringLiteral("PlcAddressDialog"));
    layout = new QGridLayout(dialog.data());
    memory = new QComboBox(dialog.data());
    memory->setObjectName(QStringLiteral("PlcMemory"));
    memory->addItem(QStringLiteral("内部存储器"), QString());
    for (index = 0; index < connections.size(); ++index)
    {
        profile = catalog.profile(connections[index].profileId);
        memory->addItem(connections[index].id.toUpper() + QStringLiteral(" / ") +
                         (profile == nullptr ? QStringLiteral("未知 PLC") : profile->name), connections[index].id);
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
    validation = new QLabel(error, dialog.data());
    validation->setObjectName(QStringLiteral("PlcAddressError"));
    validation->setWordWrap(true);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog.data());
    buttons->setObjectName(QStringLiteral("PlcAddressButtons"));
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    layout->addWidget(new QLabel(QStringLiteral("存储器"), dialog.data()), 0, 0);
    layout->addWidget(memory, 0, 1, 1, 2);
    layout->addWidget(new QLabel(QStringLiteral("地址种类"), dialog.data()), 1, 0);
    layout->addWidget(kind, 1, 1);
    layout->addWidget(radix, 1, 2);
    layout->addWidget(range, 2, 0, 1, 3);
    layout->addWidget(number, 3, 0, 1, 3);
    layout->addWidget(validation, 4, 0, 1, 3);
    layout->addWidget(buttons, 5, 0, 1, 3);

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

        family = kind->currentData().toString();
        selectedConnection = plcConnection(context, memory->currentData().toString());
        selectedProfile = selectedConnection == nullptr ? nullptr : catalog.profile(selectedConnection->profileId);
        supported = memory->currentData().toString().isEmpty() ||
                    (selectedProfile != nullptr && plcNativeFamilyRule(*selectedProfile, family, &rule)); //20261109-0 change
        standardRadix = memory->currentData().toString().isEmpty() ||
                        (selectedProfile != nullptr && plcProfileHasModbusCodec(*selectedProfile));
        rangeText = memory->currentData().toString().isEmpty() ? QStringLiteral("地址范围：0~%1").arg(4097 - transferCount) : QString(); //20261109-6 change
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
        if (standardRadix && family.endsWith(QStringLiteral("_bit")))
        {
            rangeText += QStringLiteral("\nA.B 表示字地址和位号；位号 0~15，始终使用十进制。");
        }
        rangeText += QStringLiteral("\n连续传送数量：%1").arg(context.transferCount); //20261114-8 add
        range->setText(rangeText);
        number->setEnabled(supported);
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
