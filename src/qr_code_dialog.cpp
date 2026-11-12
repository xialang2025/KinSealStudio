#include "qr_code_dialog.h"
#include "hmi_property_widgets.h"
#include "../third_party/qrcodegen/qrcodegen.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <exception>
#include <optional>

/******************************************************************************
 * Function Name   : qrDescriptionPreview
 * Description     : Encode the full description as standard UTF8 QR bytes; this is not the unverified PLC string encoder.
 * Input Parameters: description, error
 * Return Value    : One pixel per module, including a four-module quiet zone; null on QR capacity failure.
 * Date            : 2026.11.12
 ******************************************************************************/
QImage qrDescriptionPreview(const QString &description, QString *error) //20261112-0 add
{
    QByteArray bytes;
    std::vector<qrcodegen::QrSegment> segments;
    std::optional<qrcodegen::QrCode> qr;
    QImage image;
    int x;
    int y;
    bytes = (description.isEmpty() ? QStringLiteral("http://www.kinseal.com/") : description).toUtf8();
    // Only the empty preview's URL, version2/LOW/mask1 module matrix has been observed. Other descriptions use explicit UTF8 ECI26.
    try
    {
        if (!description.isEmpty()) { segments.push_back(qrcodegen::QrSegment::makeEci(26)); }
        segments.push_back(qrcodegen::QrSegment::makeBytes(std::vector<u8>(bytes.begin(), bytes.end())));
        qr.emplace(description.isEmpty() ? qrcodegen::QrCode::encodeSegments(segments, qrcodegen::QrCode::Ecc::LOW, 1, 40, 1, false) :
                                          qrcodegen::QrCode::encodeSegments(segments, qrcodegen::QrCode::Ecc::MEDIUM));
        image = QImage(qr->getSize() + 8, qr->getSize() + 8, QImage::Format_RGB32); image.fill(Qt::white);
        for (y = 0; y < qr->getSize(); ++y)
        {
            for (x = 0; x < qr->getSize(); ++x) { if (qr->getModule(x, y)) { image.setPixelColor(x + 4, y + 4, Qt::black); } }
        }
    }
    catch (const std::exception &)
    { if (error != nullptr) { *error = QStringLiteral("描述超过二维码预览容量；原描述仍完整保存。"); } }
    return image;
}

/******************************************************************************
 * Function Name   : paintQrCode
 * Description     : Share crisp square-module rendering between the offline description preview and page canvas.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintQrCode(QPainter *painter, const QRectF &rect, const QrCodeProperties &properties) //20261112-0 add
{
    QImage image;
    QRectF square;
    qreal side;
    QString error;
    painter->save(); painter->setClipRect(rect, Qt::IntersectClip); painter->fillRect(rect, Qt::white);
    image = qrDescriptionPreview(properties.description, &error);
    if (image.isNull())
    { painter->setPen(Qt::black); painter->drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, error); painter->restore(); return; }
    side = qMin(rect.width(), rect.height()); square = QRectF(rect.center().x() - side / 2, rect.center().y() - side / 2, side, side);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false); painter->drawImage(square, image); painter->restore();
}

/******************************************************************************
 * Function Name   : QrCodeDialog::QrCodeDialog
 * Description     : Restore both native QR pages, measured limits, shared address pickers and visibility-only permissions.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
QrCodeDialog::QrCodeDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261112-0 add
    : QDialog(parent), m_original(object), m_project(project)
{
    QVBoxLayout *root;
    QHBoxLayout *content;
    QTabWidget *tabs;
    QWidget *general;
    QVBoxLayout *generalLayout;
    QGroupBox *addressGroup;
    QFormLayout *form;
    QWidget *addressRow;
    QHBoxLayout *addressLayout;
    QPushButton *picker;
    QWidget *security;
    QVBoxLayout *securityLayout;
    QGroupBox *previewGroup;
    QVBoxLayout *previewLayout;
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    setObjectName(QStringLiteral("QrCodeDialog")); setWindowTitle(QStringLiteral("二维码")); resize(825, 598); setFont(QFont(QStringLiteral("SimSun"), 9));
    setStyleSheet(QStringLiteral("QComboBox,QLineEdit,QSpinBox { background:#caeace; } QGroupBox { background:white; }"));
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); }
    setProperty("plcConnections", connections.toVariantList());
    root = new QVBoxLayout(this); content = new QHBoxLayout(); tabs = new QTabWidget(this); tabs->setObjectName(QStringLiteral("QrTabs"));
    general = new QWidget(tabs); generalLayout = new QVBoxLayout(general); addressGroup = new QGroupBox(QStringLiteral("地址"), general); form = new QFormLayout(addressGroup);
    m_address = new QLineEdit(object.qrCode.monitorAddress, addressGroup); m_address->setObjectName(QStringLiteral("QrMonitorAddress"));
    addressRow = new QWidget(addressGroup); addressLayout = new QHBoxLayout(addressRow); addressLayout->setContentsMargins(0, 0, 0, 0);
    picker = new QPushButton(QStringLiteral("…"), addressRow); picker->setObjectName(QStringLiteral("QrAddressPicker")); picker->setFixedWidth(28); addressLayout->addWidget(m_address); addressLayout->addWidget(picker);
    m_encoding = new QComboBox(addressGroup); m_encoding->setObjectName(QStringLiteral("QrEncoding")); m_encoding->addItems({QStringLiteral("ASCII"), QStringLiteral("GB2312")}); m_encoding->setCurrentIndex(object.qrCode.encoding);
    m_swap = new QCheckBox(QStringLiteral("高低字节互换"), addressGroup); m_swap->setObjectName(QStringLiteral("QrSwapBytes")); m_swap->setChecked(object.qrCode.swapBytes);
    m_length = new QSpinBox(addressGroup); m_length->setObjectName(QStringLiteral("QrCharacterLength")); m_length->setRange(0, 255); m_length->setValue(object.qrCode.characterLength);
    m_description = new QLineEdit(object.qrCode.description, addressGroup); m_description->setObjectName(QStringLiteral("QrDescription")); m_description->setMaxLength(32767);
    form->addRow(QStringLiteral("监视地址:"), addressRow); form->addRow(QStringLiteral("编码方式:"), m_encoding); form->addRow(m_swap); form->addRow(QStringLiteral("字符长度:"), m_length); form->addRow(QStringLiteral("描  述:"), m_description);
    generalLayout->addWidget(addressGroup); generalLayout->addStretch(); tabs->addTab(general, QStringLiteral("外观设置"));
    security = new QWidget(tabs); securityLayout = new QVBoxLayout(security); m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("QrVisibility"), object.qrCode.visibility, false, security);
    securityLayout->addWidget(m_visibility); securityLayout->addStretch(); tabs->addTab(security, QStringLiteral("权限设置"));
    previewGroup = new QGroupBox(QStringLiteral("预览"), this); previewLayout = new QVBoxLayout(previewGroup); m_preview = new QLabel(previewGroup); m_preview->setObjectName(QStringLiteral("QrPreview")); m_preview->setFixedSize(150, 150);
    m_preview->setToolTip(QStringLiteral("描述的离线预览使用UTF-8；空描述显示原厂默认网址。设备编码、字节互换及监视长度的实际效果待验证。"));
    previewLayout->addWidget(m_preview); content->addWidget(tabs, 1); content->addWidget(previewGroup, 0, Qt::AlignTop); root->addLayout(content, 1);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("QrButtons"));
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确认")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); root->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QrCodeDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("二维码帮助"), QStringLiteral("描述用于二维码默认内容，监视地址用于运行时字符串。详见design_plan/二维码功能与交互复刻手册.md。")); });
    connect(picker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_address, false); });
    connect(m_description, &QLineEdit::textChanged, this, [this]() { refreshPreview(); }); refreshPreview();
}

/******************************************************************************
 * Function Name   : QrCodeDialog::refreshPreview
 * Description     : Refresh the full description without truncating, swapping or simulating unknown PLC bytes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void QrCodeDialog::refreshPreview() //20261112-0 add
{
    QPixmap image(m_preview->size());
    QPainter painter(&image);
    QrCodeProperties properties;
    properties.description = m_description->text(); paintQrCode(&painter, image.rect(), properties); painter.end(); m_preview->setPixmap(image);
}

/******************************************************************************
 * Function Name   : QrCodeDialog::object
 * Description     : Return a detached QR object and its compact-panel mirrors without changing project state.
 * Input Parameters: None
 * Return Value    : Complete QR draft.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiObject QrCodeDialog::object() const //20261112-0 add
{
    HmiObject result = m_original;
    result.qrCode.monitorAddress = m_address->text().trimmed(); result.qrCode.encoding = m_encoding->currentIndex(); result.qrCode.swapBytes = m_swap->isChecked();
    result.qrCode.characterLength = m_length->value(); result.qrCode.description = m_description->text(); result.qrCode.visibility = m_visibility->properties();
    result.address = result.qrCode.monitorAddress; result.text = result.qrCode.description; return result;
}

/******************************************************************************
 * Function Name   : QrCodeDialog::accept
 * Description     : Validate observed properties and address carriers before publishing one complete dialog transaction.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void QrCodeDialog::accept() //20261112-0 add
{
    HmiObject candidate = object();
    QString error;
    if (!candidate.qrCode.validate(&error) || !candidate.qrCode.validateAddresses(m_project.connections, &error))
    { QMessageBox::warning(this, QStringLiteral("二维码"), error); return; }
    QDialog::accept();
}
