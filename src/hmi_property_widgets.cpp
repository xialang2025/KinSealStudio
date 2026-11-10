#include "hmi_property_widgets.h"
#include "plc_address_dialog.h" //20261109-0 add
#include <QJsonArray>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : HmiPermissionEditor::HmiPermissionEditor
 * Description     : Build independently checkable bit, level and word conditions with native defaults.
 * Input Parameters: title, prefix, properties, touch, parent
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
HmiPermissionEditor::HmiPermissionEditor(const QString &title, const QString &prefix,
                                         const StaticTextProperties &properties, bool touch, QWidget *parent) //20261108-15 add
    : QGroupBox(title, parent), m_original(properties)
{
    QVBoxLayout *layout;
    QGridLayout *bitLayout;
    QGridLayout *levelLayout;
    QGridLayout *wordLayout;
    QRadioButton *off;
    QPushButton *bitPicker;
    QPushButton *wordPicker;
    int index;

    setCheckable(true); setChecked(properties.visibilityEnabled); setObjectName(prefix);
    layout = new QVBoxLayout(this); layout->setContentsMargins(8, 12, 8, 6);
    m_bit = new QGroupBox(QStringLiteral("受位控制"), this);
    m_bit->setCheckable(true); m_bit->setChecked(properties.bitEnabled); m_bit->setObjectName(prefix + QStringLiteral("Bit"));
    bitLayout = new QGridLayout(m_bit);
    m_bitAddress = new QLineEdit(properties.bitAddress, m_bit);
    m_bitAddress->setObjectName(prefix + QStringLiteral("BitAddress"));
    bitPicker = new QPushButton(QStringLiteral("…"), m_bit); bitPicker->setFixedWidth(28);
    bitPicker->setObjectName(prefix + QStringLiteral("BitPicker"));
    m_on = new QRadioButton(QStringLiteral("1"), m_bit); off = new QRadioButton(QStringLiteral("0"), m_bit);
    m_on->setChecked(properties.bitState); off->setChecked(!properties.bitState);
    bitLayout->addWidget(new QLabel(QStringLiteral("地址:"), m_bit), 0, 0);
    bitLayout->addWidget(m_bitAddress, 0, 1); bitLayout->addWidget(bitPicker, 0, 2);
    bitLayout->addWidget(new QLabel(touch ? QStringLiteral("有效状态:") : QStringLiteral("显现状态:"), m_bit), 1, 0);
    bitLayout->addWidget(m_on, 1, 1); bitLayout->addWidget(off, 1, 2); layout->addWidget(m_bit);
    m_level = new QGroupBox(QStringLiteral("受用户等级控制"), this);
    m_level->setCheckable(true); m_level->setChecked(properties.levelEnabled); m_level->setObjectName(prefix + QStringLiteral("Level"));
    levelLayout = new QGridLayout(m_level); m_minimum = new QComboBox(m_level);
    m_minimum->setObjectName(prefix + QStringLiteral("Minimum"));
    for (index = 1; index <= 8; ++index) { m_minimum->addItem(QString::number(index)); }
    m_minimum->setCurrentIndex(properties.minimumLevel - 1);
    levelLayout->addWidget(new QLabel(QStringLiteral("有效最低等级:"), m_level), 0, 0);
    levelLayout->addWidget(m_minimum, 0, 1); layout->addWidget(m_level);
    m_word = new QGroupBox(QStringLiteral("字控制"), this);
    m_word->setCheckable(true); m_word->setChecked(properties.wordEnabled); m_word->setObjectName(prefix + QStringLiteral("Word"));
    wordLayout = new QGridLayout(m_word);
    m_type = new QComboBox(m_word); m_type->addItems(StaticTextProperties::dataTypes()); m_type->setCurrentIndex(properties.dataType);
    m_type->setObjectName(prefix + QStringLiteral("Type"));
    m_wordAddress = new QLineEdit(properties.wordAddress, m_word); m_wordAddress->setObjectName(prefix + QStringLiteral("WordAddress"));
    wordPicker = new QPushButton(QStringLiteral("…"), m_word); wordPicker->setFixedWidth(28);
    wordPicker->setObjectName(prefix + QStringLiteral("WordPicker"));
    m_comparison = new QComboBox(m_word); m_comparison->addItems(StaticTextProperties::comparisons());
    m_comparison->setCurrentIndex(properties.comparison); m_comparison->setObjectName(prefix + QStringLiteral("Comparison"));
    m_value = new QLineEdit(properties.compareValue, m_word); m_value->setObjectName(prefix + QStringLiteral("Value"));
    m_error = new QLineEdit(properties.tolerance, m_word); m_error->setObjectName(prefix + QStringLiteral("Tolerance"));
    m_errorLabel = new QLabel(QStringLiteral("误差:"), m_word);
    wordLayout->addWidget(new QLabel(QStringLiteral("数据类型:"), m_word), 0, 0);
    wordLayout->addWidget(m_type, 0, 1, 1, 3); wordLayout->addWidget(m_wordAddress, 1, 0, 1, 2);
    wordLayout->addWidget(wordPicker, 1, 2); wordLayout->addWidget(m_comparison, 1, 3);
    wordLayout->addWidget(m_value, 2, 0, 1, 2); wordLayout->addWidget(m_errorLabel, 2, 2); wordLayout->addWidget(m_error, 2, 3);
    layout->addWidget(m_word);
    QObject::connect(bitPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_bitAddress, true); });
    QObject::connect(wordPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_wordAddress, false); });
    QObject::connect(m_comparison, &QComboBox::currentIndexChanged, this, [this](int comparison) {
        m_error->setVisible(comparison == 2 || comparison == 3);
        m_errorLabel->setVisible(comparison == 2 || comparison == 3);
    });
    m_error->setVisible(properties.comparison == 2 || properties.comparison == 3);
    m_errorLabel->setVisible(properties.comparison == 2 || properties.comparison == 3);
}

/******************************************************************************
 * Function Name   : HmiPermissionEditor::properties
 * Description     : Retain condition inputs even while their owning group is unchecked.
 * Input Parameters: None
 * Return Value    : Detached permission configuration.
 * Date            : 2026.11.08
 ******************************************************************************/
StaticTextProperties HmiPermissionEditor::properties() const //20261108-15 add
{
    StaticTextProperties result = m_original;
    result.visibilityEnabled = isChecked(); result.bitEnabled = m_bit->isChecked();
    result.bitAddress = m_bitAddress->text().trimmed(); result.bitState = m_on->isChecked();
    result.levelEnabled = m_level->isChecked(); result.minimumLevel = m_minimum->currentIndex() + 1;
    result.wordEnabled = m_word->isChecked(); result.dataType = m_type->currentIndex();
    result.wordAddress = m_wordAddress->text().trimmed(); result.comparison = m_comparison->currentIndex();
    result.compareValue = m_value->text(); result.tolerance = m_error->text();
    return result;
}

/******************************************************************************
 * Function Name   : editHmiAddress
 * Description     : Share the verified internal LB/LW decimal/hex address dialog.
 * Input Parameters: parent, field, bit
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void editHmiAddress(QWidget *parent, QLineEdit *field, bool bit, int transferCount) //20261109-6 change
{
    QDialog dialog(parent);
    QGridLayout *layout;
    QComboBox *memory;
    QComboBox *kind;
    QComboBox *radix;
    QLineEdit *number;
    QDialogButtonBox *buttons;
    QPushButton *button;
    QStringList keys;
    QRegularExpressionMatch match;
    int index;
    QObject *contextOwner; //20261109-0 add
    QVector<PlcConnection> connections;
    PlcConnection connection;
    QJsonArray connectionsJson;
    QString qualified;
    QString error;
    int maximumAddress; //20261109-6 add

    if (field == nullptr || transferCount < 1 || transferCount > 2) { return; }
    maximumAddress = 4097 - transferCount; // The final source word must remain within the verified local bank.
    contextOwner = parent;
    while (contextOwner != nullptr && !contextOwner->property("plcConnections").isValid())
    {
        contextOwner = contextOwner->parent();
    }
    if (contextOwner != nullptr)
    {
        connectionsJson = QJsonArray::fromVariantList(contextOwner->property("plcConnections").toList());
        for (index = 0; index < connectionsJson.size(); ++index)
        {
            if (!PlcConnection::fromJson(connectionsJson.at(index).toObject(), &connection, &error)) { return; }
            connections.append(connection);
        }
        if (!connections.isEmpty())
        {
            qualified = field->text();
            if (editPlcAddress(parent, bit, connections, &qualified, transferCount)) { field->setText(qualified); } //20261109-6 change
            return;
        }
    }
    if (field->text().startsWith(QLatin1Char('@'))) //20261109-0 add: A missing route must not reset an existing external address to LB0/LW0.
    {
        QMessageBox::warning(parent, QStringLiteral("连接配置缺失"), QStringLiteral("此地址引用的PLC连接不存在，请先恢复连接设置。"));
        return;
    }

    dialog.setWindowTitle(QStringLiteral("地址输入"));
    dialog.setObjectName(QStringLiteral("StaticTextAddressDialog"));
    layout = new QGridLayout(&dialog);
    memory = new QComboBox(&dialog);
    memory->addItem(QStringLiteral("内部存储器"));
    kind = new QComboBox(&dialog);
    kind->addItem(bit ? QStringLiteral("LB") : QStringLiteral("LW"));
    radix = new QComboBox(&dialog);
    radix->addItems({QStringLiteral("10进制"), QStringLiteral("16进制")});
    number = new QLineEdit(&dialog);
    number->setObjectName(QStringLiteral("AddressNumber"));
    match = QRegularExpression(QStringLiteral("^(LB|LW)([0-9]+)$")).match(field->text());
    number->setText(match.hasMatch() ? match.captured(2) : QStringLiteral("0"));
    layout->addWidget(new QLabel(QStringLiteral("存储器"), &dialog), 0, 0);
    layout->addWidget(memory, 0, 1, 1, 3);
    layout->addWidget(new QLabel(QStringLiteral("地址种类"), &dialog), 1, 0);
    layout->addWidget(kind, 1, 1);
    layout->addWidget(radix, 1, 2, 1, 2);
    layout->addWidget(new QLabel(QStringLiteral("地址范围: 0~%1").arg(maximumAddress), &dialog), 2, 0, 1, 4); //20261109-6 change
    layout->addWidget(number, 3, 0, 1, 4);
    keys = {QStringLiteral("7"),QStringLiteral("8"),QStringLiteral("9"),QStringLiteral("A"),
            QStringLiteral("4"),QStringLiteral("5"),QStringLiteral("6"),QStringLiteral("B"),
            QStringLiteral("1"),QStringLiteral("2"),QStringLiteral("3"),QStringLiteral("C"),
            QStringLiteral("0"),QStringLiteral("D"),QStringLiteral("E"),QStringLiteral("F"),
            QStringLiteral("清除"),QStringLiteral("后退")};
    for (index = 0; index < keys.size(); ++index)
    {
        button = new QPushButton(keys.at(index), &dialog);
        button->setProperty("hexDigit", index < 16 && !keys.at(index).at(0).isDigit());
        button->setEnabled(!button->property("hexDigit").toBool());
        layout->addWidget(button, 4 + index / 4, index % 4);
        QObject::connect(button, &QPushButton::clicked, &dialog, [number, radix, keys, index]() {
            if (index == 16) { number->clear(); }
            else if (index == 17) { number->backspace(); }
            else if (keys.at(index).at(0).isDigit() || radix->currentIndex() == 1) { number->insert(keys.at(index)); }
        });
    }
    QObject::connect(radix, &QComboBox::currentIndexChanged, &dialog, [number, &dialog](int selected) {
        bool ok;
        int value;
        QPushButton *key;
        value = number->text().toInt(&ok, selected == 0 ? 16 : 10);
        if (ok) { number->setText(QString::number(value, selected == 0 ? 10 : 16).toUpper()); }
        Q_FOREACH (key, dialog.findChildren<QPushButton *>())
        {
            if (key->property("hexDigit").toBool()) { key->setEnabled(selected == 1); }
        }
    });
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("输入"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("离开"));
    layout->addWidget(buttons, 9, 0, 1, 4);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&dialog, number, radix, kind, field, maximumAddress]() { //20261109-6 change
        bool ok;
        int value;
        value = number->text().toInt(&ok, radix->currentIndex() == 0 ? 10 : 16);
        if (!ok || value < 0 || value > maximumAddress) //20261109-6 change
        {
            QMessageBox::warning(&dialog, QStringLiteral("地址无效"), QStringLiteral("请输入 0~%1 内的地址。").arg(maximumAddress));
            return;
        }
        field->setText(kind->currentText() + QString::number(value));
        dialog.accept();
    });
    dialog.exec();
}
