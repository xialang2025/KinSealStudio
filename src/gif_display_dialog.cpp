#include "gif_display_dialog.h"
#include "animation_resources.h"
#include "hmi_property_widgets.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QTabWidget>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : paintGifDisplay
 * Description     : Stretch the actual first GIF frame into the editor rectangle without claiming device playback.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintGifDisplay(QPainter *painter, const QRectF &rect, const GifDisplayProperties &properties) //20261112-0 add
{
    QImage frame = gifFirstFrame(properties.gifData);
    if (frame.isNull()) { return; }
    painter->save(); painter->setClipRect(rect, Qt::IntersectClip); painter->drawImage(rect, frame); painter->restore();
}

/******************************************************************************
 * Function Name   : GifDisplayDialog::GifDisplayDialog
 * Description     : Reproduce the two GIF pages, independent playback bit, nested gallery and real animation trial.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
GifDisplayDialog::GifDisplayDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261112-0 add
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.gifDisplay)
{
    QVBoxLayout *root;
    QHBoxLayout *content;
    QTabWidget *tabs;
    QWidget *general;
    QVBoxLayout *layout;
    QGroupBox *image;
    QHBoxLayout *imageLayout;
    QPushButton *choose;
    QGroupBox *control;
    QFormLayout *controlLayout;
    QHBoxLayout *addressRow;
    QHBoxLayout *stateRow;
    QWidget *security;
    QVBoxLayout *securityLayout;
    QGroupBox *preview;
    QVBoxLayout *previewLayout;
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    setObjectName(QStringLiteral("GifDisplayDialog")); setWindowTitle(QStringLiteral("GIF显示器")); resize(850, 580); setFont(QFont(QStringLiteral("SimSun"), 9));
    setStyleSheet(QStringLiteral("QComboBox,QLineEdit { background:#caeace; } QLineEdit:disabled { background:#d0e9f4; } QGroupBox { background:white; }"));
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    root = new QVBoxLayout(this); root->setContentsMargins(20, 10, 20, 16); content = new QHBoxLayout(); content->setSpacing(20); tabs = new QTabWidget(this); tabs->setObjectName(QStringLiteral("GifTabs"));
    general = new QWidget(tabs); layout = new QVBoxLayout(general); layout->setContentsMargins(12, 16, 12, 12); layout->setSpacing(16); image = new QGroupBox(QStringLiteral("图片属性"), general); image->setFixedSize(471, 104); imageLayout = new QHBoxLayout(image); imageLayout->setContentsMargins(28, 20, 36, 20);
    imageLayout->addWidget(new QLabel(QStringLiteral("选择图片:"), image)); m_run = new QPushButton(QStringLiteral("运行测试"), image); m_run->setObjectName(QStringLiteral("GifRun")); imageLayout->addWidget(m_run, 1);
    choose = new QPushButton(QStringLiteral("…"), image); choose->setObjectName(QStringLiteral("GifChoose")); choose->setFixedWidth(30); imageLayout->addWidget(choose); layout->addWidget(image);
    control = new QGroupBox(QStringLiteral("控制"), general); control->setFixedSize(471, 176); controlLayout = new QFormLayout(control); controlLayout->setContentsMargins(28, 20, 36, 20); controlLayout->setVerticalSpacing(18);
    m_bitControl = new QCheckBox(QStringLiteral("受位控制"), control); m_bitControl->setObjectName(QStringLiteral("GifBitControl")); m_bitControl->setChecked(m_draft.bitControl); controlLayout->addRow(m_bitControl);
    m_address = new QLineEdit(m_draft.controlAddress, control); m_address->setObjectName(QStringLiteral("GifControlAddress")); m_address->setFixedWidth(210); m_addressPicker = new QPushButton(QStringLiteral("…"), control); m_addressPicker->setObjectName(QStringLiteral("GifAddressPicker")); m_addressPicker->setFixedWidth(24);
    addressRow = new QHBoxLayout(); addressRow->addWidget(m_address); addressRow->addWidget(m_addressPicker); controlLayout->addRow(QStringLiteral("控制位:"), addressRow);
    m_state1 = new QRadioButton(QStringLiteral("1"), control); m_state1->setObjectName(QStringLiteral("GifState1")); m_state0 = new QRadioButton(QStringLiteral("0"), control); m_state0->setObjectName(QStringLiteral("GifState0"));
    m_state1->setChecked(m_draft.activeState == 1); m_state0->setChecked(m_draft.activeState == 0); stateRow = new QHBoxLayout(); stateRow->addWidget(m_state1); stateRow->addStretch(); controlLayout->addRow(QStringLiteral("有效状态:"), stateRow); controlLayout->addRow(QString(), m_state0); layout->addWidget(control); layout->addStretch(); tabs->addTab(general, QStringLiteral("基本"));
    security = new QWidget(tabs); securityLayout = new QVBoxLayout(security); m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("GifVisibility"), m_draft.visibility, false, security); m_visibility->setObjectName(QStringLiteral("GifVisibility")); securityLayout->addWidget(m_visibility); securityLayout->addStretch(); tabs->addTab(security, QStringLiteral("权限设置"));
    content->addWidget(tabs, 1); preview = new QGroupBox(QStringLiteral("预览"), this); preview->setFixedSize(170, 220); previewLayout = new QVBoxLayout(preview); m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("GifPreview")); m_preview->setFixedSize(150, 180); m_preview->setAlignment(Qt::AlignCenter); m_preview->setStyleSheet(QStringLiteral("background:white; border:1px solid #8b929a;")); previewLayout->addWidget(m_preview); previewLayout->addStretch(); content->addWidget(preview, 0, Qt::AlignTop); root->addLayout(content, 1);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("GifButtons")); root->addWidget(buttons);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); //20261112-0 change: Keep native Chinese captions independent of the system Qt translation installation.
    connect(buttons, &QDialogButtonBox::accepted, this, &GifDisplayDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &GifDisplayDialog::reject);
    connect(choose, &QPushButton::clicked, this, &GifDisplayDialog::selectAnimation); connect(m_run, &QPushButton::clicked, this, &GifDisplayDialog::togglePlayback);
    connect(m_addressPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_address, true); });
    connect(m_bitControl, &QCheckBox::toggled, this, [this](bool enabled) { m_address->setEnabled(enabled); m_addressPicker->setEnabled(enabled); m_state0->setEnabled(enabled); m_state1->setEnabled(enabled); });
    m_address->setEnabled(m_draft.bitControl); m_addressPicker->setEnabled(m_draft.bitControl); m_state0->setEnabled(m_draft.bitControl); m_state1->setEnabled(m_draft.bitControl);
    m_movie.setObjectName(QStringLiteral("GifMovie")); m_movie.setCacheMode(QMovie::CacheAll); m_movie.setDevice(&m_buffer); m_movie.setFormat(QByteArrayLiteral("gif"));
    connect(&m_movie, &QMovie::frameChanged, this, [this]() { refreshFrame(); });
    connect(&m_movie, &QMovie::finished, this, [this]() { m_playing = false; m_run->setText(QStringLiteral("运行测试")); }); loadPreview();
}

/******************************************************************************
 * Function Name   : GifDisplayDialog::loadPreview
 * Description     : Reset trial playback to the selected original GIF's first frame after gallery confirmation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void GifDisplayDialog::loadPreview() //20261112-0 add
{
    m_movie.stop(); m_movie.setDevice(nullptr); m_buffer.close(); m_buffer.setData(m_draft.gifData); m_buffer.open(QIODevice::ReadOnly);
    m_movie.setDevice(&m_buffer); m_movie.setFormat(QByteArrayLiteral("gif")); m_playing = false; m_run->setText(QStringLiteral("运行测试")); m_preview->clear();
    if (!m_draft.gifData.isEmpty()) { m_movie.jumpToFrame(0); refreshFrame(); }
}

/******************************************************************************
 * Function Name   : GifDisplayDialog::refreshFrame
 * Description     : Display each actual decoded trial frame at the measured square preview size.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void GifDisplayDialog::refreshFrame() //20261112-0 add
{
    m_preview->setPixmap(m_movie.currentPixmap().scaled(100, 100, Qt::IgnoreAspectRatio, Qt::FastTransformation));
}

/******************************************************************************
 * Function Name   : GifDisplayDialog::togglePlayback
 * Description     : Run the actual GIF file or pause at its current frame; playback never changes saved properties.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void GifDisplayDialog::togglePlayback() //20261112-0 add
{
    if (m_draft.gifData.isEmpty() || !m_movie.isValid())
    { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), QStringLiteral("GIF数据为空，请重新选择。")); return; }
    if (m_playing) { m_movie.setPaused(true); m_playing = false; }
    else { if (m_movie.state() == QMovie::Paused) { m_movie.setPaused(false); } else { m_movie.start(); } m_playing = true; }
    m_run->setText(m_playing ? QStringLiteral("停止运行") : QStringLiteral("运行测试"));
}

/******************************************************************************
 * Function Name   : GifDisplayDialog::selectAnimation
 * Description     : Commit nested gallery bytes only after confirmation and leave an ongoing preview untouched on cancel.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void GifDisplayDialog::selectAnimation() //20261112-0 add
{
    HmiProject context = m_project;
    HmiObject current = object();
    AnimationGalleryDialog *gallery;
    int index;
    if (!current.gifDisplay.gifData.isEmpty())
    {
        for (index = context.objects.size() - 1; index >= 0; --index) { if (context.objects.at(index).id == current.id) { context.objects.removeAt(index); } }
        current.id = current.id.isEmpty() ? QStringLiteral("gif-gallery-draft") : current.id; context.objects.append(current);
    }
    gallery = new AnimationGalleryDialog(context, m_draft.animation, this);
    if (gallery->exec() == QDialog::Accepted)
    {
        m_project.animations = gallery->draftProject().animations; m_draft.animation = gallery->selectedAnimation();
        m_draft.gifData = m_project.animations.value(m_draft.animation); loadPreview();
    }
    delete gallery;
}

/******************************************************************************
 * Function Name   : GifDisplayDialog::object
 * Description     : Return a detached complete object with independent playback and visibility controls.
 * Input Parameters: None
 * Return Value    : GIF control draft.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiObject GifDisplayDialog::object() const //20261112-0 add
{
    HmiObject result = m_original;
    result.type = QStringLiteral("gif"); result.address.clear(); result.gifDisplay = m_draft; result.gifDisplay.bitControl = m_bitControl->isChecked();
    result.gifDisplay.controlAddress = m_address->text().trimmed(); result.gifDisplay.activeState = m_state0->isChecked() ? 0 : 1; result.gifDisplay.visibility = m_visibility->properties(); return result;
}

/******************************************************************************
 * Function Name   : GifDisplayDialog::animations
 * Description     : Return the gallery resource draft for atomic acceptance and placement.
 * Input Parameters: None
 * Return Value    : Original GIF resource pool.
 * Date            : 2026.11.12
 ******************************************************************************/
QMap<QString, QByteArray> GifDisplayDialog::animations() const //20261112-0 add
{
    return m_project.animations;
}

/******************************************************************************
 * Function Name   : GifDisplayDialog::accept
 * Description     : Keep empty or invalid GIF controls open without publishing resource or object changes.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void GifDisplayDialog::accept() //20261112-0 add
{
    HmiObject candidate = object();
    QString error;
    if (!candidate.gifDisplay.validate(&error) || !candidate.gifDisplay.validateAddresses(m_project.connections, &error))
    { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), error); return; }
    m_movie.stop(); QDialog::accept();
}
