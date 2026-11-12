#include "move_track_dialog.h"
#include "hmi_property_widgets.h"
#include "bit_button_dialog.h"
#include "control_rendering.h"
#include "kvg_library.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QJsonArray>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QUuid>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : paintMoveTrack
 * Description     : Draw the selected static appearance; direction and distance are not a simulated PLC position.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintMoveTrack(QPainter *painter, const QRectF &rect, const MoveTrackProperties &properties) //20261112-0 add
{
    painter->save(); painter->setClipRect(rect);
    if (!properties.appearance.picturePng.isEmpty()) { painter->drawImage(rect, decodedControlPng(properties.appearance.picturePng)); }
    else if (!properties.appearance.picture.isEmpty()) { paintKvgPicture(painter, rect, properties.appearance.picture, 0); }
    painter->restore();
}

/******************************************************************************
 * Function Name   : MoveTrackDialog::MoveTrackDialog
 * Description     : Create the observed two-page dialog before any canvas placement transaction.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
MoveTrackDialog::MoveTrackDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261112-0 add
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.moveTrack)
{
    QGridLayout *layout = new QGridLayout(this);
    QTabWidget *tabs = new QTabWidget(this);
    QGroupBox *preview = new QGroupBox(QStringLiteral("预览"), this);
    QVBoxLayout *previewLayout = new QVBoxLayout(preview);
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    setObjectName(QStringLiteral("MoveTrackDialog")); setWindowTitle(QStringLiteral("移动轨迹")); resize(825, 598);
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    tabs->setObjectName(QStringLiteral("TrackTabs")); tabs->addTab(createAppearance(), QStringLiteral("外观设置")); tabs->addTab(createPermissions(), QStringLiteral("权限设置")); layout->addWidget(tabs, 0, 0); layout->setColumnStretch(0, 1);
    m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("TrackPreview")); m_preview->setFixedSize(130, 120); previewLayout->addWidget(m_preview); layout->addWidget(preview, 0, 1, Qt::AlignTop);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("TrackButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons, 1, 0, 1, 2);
    connect(buttons, &QDialogButtonBox::accepted, this, &MoveTrackDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("移动轨迹帮助"), QStringLiteral("设置写入地址、移动距离、方向、外观图形和常量或地址范围。预览只显示静态外观。")); }); refresh();
}

/******************************************************************************
 * Function Name   : MoveTrackDialog::createAppearance
 * Description     : Preserve simultaneous constant/address rows with independent values and measured enabling behavior.
 * Input Parameters: None
 * Return Value    : Appearance page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *MoveTrackDialog::createAppearance() //20261112-0 add
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    QGroupBox *properties = new QGroupBox(QStringLiteral("属性设置"), page);
    QGroupBox *range = new QGroupBox(QStringLiteral("范围设置"), page);
    QGridLayout *grid = new QGridLayout(properties);
    QPushButton *picker = new QPushButton(QStringLiteral("…"), properties);
    QPushButton *system = new QPushButton(QStringLiteral("系统图库"), properties);
    QPushButton *project = new QPushButton(QStringLiteral("工程图库"), properties);
    page->setStyleSheet(QStringLiteral("QSpinBox, QComboBox, QLineEdit {background:#caeace; min-height:28px;} QSpinBox:disabled, QComboBox:disabled, QLineEdit:disabled {background:#b1ceed;}"));
    layout->setContentsMargins(20, 24, 10, 15); layout->setSpacing(36); properties->setFixedHeight(185); range->setFixedHeight(165); layout->addWidget(properties); layout->addWidget(range); layout->addStretch(); //20261112-0 change: Match the measured two-group spacing and bounded field columns.
    m_address = new QLineEdit(m_draft.address, properties); m_address->setObjectName(QStringLiteral("TrackAddress")); picker->setObjectName(QStringLiteral("TrackAddressPicker")); picker->setFixedWidth(24);
    m_distance = new QSpinBox(properties); m_distance->setObjectName(QStringLiteral("TrackDistance")); m_distance->setRange(0, 900); m_distance->setValue(m_draft.distance);
    m_direction = new QComboBox(properties); m_direction->setObjectName(QStringLiteral("TrackDirection")); m_direction->addItems(MoveTrackProperties::directions()); m_direction->setCurrentIndex(m_draft.direction);
    grid->addWidget(new QLabel(QStringLiteral("写入地址:"), properties), 0, 0); grid->addWidget(m_address, 0, 1); grid->addWidget(picker, 0, 2); grid->addWidget(new QLabel(QStringLiteral("移动距离:"), properties), 0, 3); grid->addWidget(m_distance, 0, 4);
    grid->addWidget(new QLabel(QStringLiteral("方　向:"), properties), 1, 0); grid->addWidget(m_direction, 1, 1); grid->addWidget(new QLabel(QStringLiteral("选择图形:"), properties), 1, 3); grid->addWidget(system, 1, 4); grid->addWidget(project, 2, 4);
    grid->setContentsMargins(45, 18, 10, 18); m_address->setFixedWidth(130); m_direction->setFixedWidth(150); m_distance->setFixedWidth(150); system->setFixedWidth(150); project->setFixedWidth(150);
    system->setObjectName(QStringLiteral("TrackSystemGallery")); project->setObjectName(QStringLiteral("TrackProjectGallery")); connect(picker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_address, false); }); connect(system, &QPushButton::clicked, this, [this]() { selectPicture(false); }); connect(project, &QPushButton::clicked, this, [this]() { selectPicture(true); });
    grid = new QGridLayout(range); m_range = new QComboBox(range); m_range->setObjectName(QStringLiteral("TrackRange")); m_range->addItems({QStringLiteral("常量"), QStringLiteral("地址")}); m_range->setCurrentIndex(m_draft.dynamicRange ? 1 : 0);
    m_minimum = new QLineEdit(m_draft.minimum, range); m_minimum->setObjectName(QStringLiteral("TrackMinimum")); m_maximum = new QLineEdit(m_draft.maximum, range); m_maximum->setObjectName(QStringLiteral("TrackMaximum")); m_minimumAddress = new QLineEdit(m_draft.minimumAddress, range); m_minimumAddress->setObjectName(QStringLiteral("TrackMinimumAddress")); m_maximumAddress = new QLineEdit(m_draft.maximumAddress, range); m_maximumAddress->setObjectName(QStringLiteral("TrackMaximumAddress"));
    grid->setContentsMargins(45, 12, 10, 12); m_range->setFixedWidth(150); m_minimum->setFixedWidth(130); m_maximum->setFixedWidth(130); m_minimumAddress->setFixedWidth(130); m_maximumAddress->setFixedWidth(130);
    m_minimum->setValidator(new QIntValidator(0, 65535, m_minimum)); m_maximum->setValidator(new QIntValidator(0, 65535, m_maximum)); //20261112-0 add: Match native acceptableInput while retaining inactive drafts.
    m_minimumPicker = new QPushButton(QStringLiteral("…"), range); m_minimumPicker->setObjectName(QStringLiteral("TrackMinimumPicker")); m_maximumPicker = new QPushButton(QStringLiteral("…"), range); m_maximumPicker->setObjectName(QStringLiteral("TrackMaximumPicker")); m_minimumPicker->setFixedWidth(24); m_maximumPicker->setFixedWidth(24);
    grid->addWidget(new QLabel(QStringLiteral("范围类型:"), range), 0, 0); grid->addWidget(m_range, 0, 1); grid->addWidget(new QLabel(QStringLiteral("最 小 值:"), range), 1, 0); grid->addWidget(m_minimum, 1, 1); grid->addWidget(new QLabel(QStringLiteral("最 大 值:"), range), 1, 3); grid->addWidget(m_maximum, 1, 4); grid->addWidget(m_minimumAddress, 2, 1); grid->addWidget(m_minimumPicker, 2, 2); grid->addWidget(m_maximumAddress, 2, 4); grid->addWidget(m_maximumPicker, 2, 5);
    connect(m_range, &QComboBox::currentIndexChanged, this, &MoveTrackDialog::updateRange); connect(m_minimumPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_minimumAddress, false); }); connect(m_maximumPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_maximumAddress, false); }); updateRange(); return page;
}

/******************************************************************************
 * Function Name   : MoveTrackDialog::updateRange
 * Description     : Toggle only enabled states; constants and dynamic address drafts survive repeated switches.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MoveTrackDialog::updateRange() //20261112-0 add
{
    bool dynamic = m_range->currentIndex() == 1;
    m_minimum->setEnabled(!dynamic); m_maximum->setEnabled(!dynamic); m_minimumAddress->setEnabled(dynamic); m_maximumAddress->setEnabled(dynamic); m_minimumPicker->setEnabled(dynamic); m_maximumPicker->setEnabled(dynamic);
}

/******************************************************************************
 * Function Name   : MoveTrackDialog::selectPicture
 * Description     : Commit an accepted gallery result to the local pool; cancelling the parent discards its edits.
 * Input Parameters: projectGallery
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MoveTrackDialog::selectPicture(bool projectGallery) //20261112-0 add
{
    HmiObject adapter;
    HmiProject context = m_project;
    QMap<QString, QByteArray> pool;
    ImageDisplayState selected = m_draft.appearance;
    BitButtonDialog *gallery;
    context.objects.append(object()); adapter.id = QStringLiteral("track-gallery-adapter"); adapter.type = QStringLiteral("switch"); adapter.address = QStringLiteral("LB0"); adapter.hasBitButtonProperties = true; gallery = new BitButtonDialog(adapter, context, this);
    if (gallery->choosePictureForControl(projectGallery, &selected.picture, &selected.picturePng, &pool)) { m_draft.appearance = selected; m_project.pictures = pool; refresh(); }
    delete gallery;
}

/******************************************************************************
 * Function Name   : MoveTrackDialog::object
 * Description     : Collect every page and inactive carrier without changing canvas geometry or a live document.
 * Input Parameters: None
 * Return Value    : Detached complete track object.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiObject MoveTrackDialog::object() const //20261112-0 add
{
    HmiObject object = m_original;
    MoveTrackProperties *properties = &object.moveTrack;
    *properties = m_draft; object.type = QStringLiteral("track"); object.address.clear(); object.text.clear(); object.hasBitButtonProperties = false; object.hasStaticTextProperties = false;
    properties->address = m_address->text().trimmed(); properties->distance = m_distance->value(); properties->direction = m_direction->currentIndex(); properties->dynamicRange = m_range->currentIndex() == 1;
    properties->minimum = m_minimum->text(); properties->maximum = m_maximum->text(); properties->minimumAddress = m_minimumAddress->text().trimmed(); properties->maximumAddress = m_maximumAddress->text().trimmed();
    properties->visibility = m_visibility->properties(); properties->touch = m_touch->properties(); properties->visibilityBitChosen = m_visibility->bitStateChosen() || m_accepted; properties->touchBitChosen = m_touch->bitStateChosen() || m_accepted;
    properties->notice = m_notice->isChecked(); properties->noticeBit = m_noticeBit->isChecked(); properties->noticeWord = m_noticeWord->isChecked(); properties->noticeBitValue = m_noticeBitOn->isChecked(); properties->noticeBitAddress = m_noticeBitAddress->text().trimmed(); properties->noticeWordAddress = m_noticeWordAddress->text().trimmed(); properties->noticeWordType = m_noticeWordType->currentIndex(); properties->noticeWordValue = m_noticeWordValue->text(); return object;
}

/******************************************************************************
 * Function Name   : MoveTrackDialog::pictures
 * Description     : Return the resource pool for the same placement or edit transaction as the object.
 * Input Parameters: None
 * Return Value    : Detached picture registry.
 * Date            : 2026.11.12
 ******************************************************************************/
QMap<QString, QByteArray> MoveTrackDialog::pictures() const //20261112-0 add
{
    return m_project.pictures;
}

/******************************************************************************
 * Function Name   : MoveTrackDialog::refresh
 * Description     : Reuse canvas rendering for a static silver-ball or selected-picture preview.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MoveTrackDialog::refresh() //20261112-0 add
{
    QImage image(130, 120, QImage::Format_ARGB32);
    QPainter painter;
    image.fill(QColor(QStringLiteral("#f0f0f0"))); painter.begin(&image); paintMoveTrack(&painter, QRectF(15, 10, 100, 100), m_draft); painter.end(); m_preview->setPixmap(QPixmap::fromImage(image));
}

/******************************************************************************
 * Function Name   : MoveTrackDialog::accept
 * Description     : Validate the whole draft and resource bindings before accepting one document transaction.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MoveTrackDialog::accept() //20261112-0 add
{
    HmiObject draft = object();
    HmiProject candidate = m_project;
    QString error;
    bool replaced = false;
    int index;
    if (draft.id.isEmpty()) { draft.id = QUuid::createUuid().toString(QUuid::WithoutBraces); }
    for (index = 0; index < candidate.objects.size(); ++index) { if (candidate.objects.at(index).id == draft.id) { candidate.objects[index] = draft; replaced = true; break; } }
    if (!replaced) { candidate.objects.append(draft); }
    if (!candidate.validateModel(&error)) { QMessageBox::warning(this, QStringLiteral("移动轨迹属性无效"), error); return; }
    m_accepted = true; QDialog::accept();
}


/******************************************************************************
 * Function Name   : MoveTrackDialog::createPermissions
 * Description     : Preserve the native fresh empty bit conditions, expose notification drafts and omit the absent hold-time row.
 * Input Parameters: None
 * Return Value    : Permission page.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *MoveTrackDialog::createPermissions() //20261112-0 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGridLayout *grid;
    QWidget *statePanel;
    QHBoxLayout *row;
    QRadioButton *off;
    QPushButton *bitPicker;
    QPushButton *wordPicker;
    m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("TrackVisibility"), m_draft.visibility, false, page); m_touch = new HmiPermissionEditor(QStringLiteral("触控有效"), QStringLiteral("TrackTouch"), m_draft.touch, true, page); m_visibility->configureUnchosenBitState(m_draft.visibilityBitChosen); m_touch->configureUnchosenBitState(m_draft.touchBitChosen); layout->addWidget(m_visibility, 0, 0); layout->addWidget(m_touch, 0, 1);
    m_notice = new QGroupBox(QStringLiteral("通知"), page); m_notice->setObjectName(QStringLiteral("TrackNotice")); m_notice->setCheckable(true); m_notice->setChecked(m_draft.notice); grid = new QGridLayout(m_notice);
    m_noticeBit = new QCheckBox(QStringLiteral("通知位"), m_notice); m_noticeBit->setObjectName(QStringLiteral("TrackNoticeBit")); m_noticeBit->setChecked(m_draft.noticeBit); m_noticeWord = new QCheckBox(QStringLiteral("通知字"), m_notice); m_noticeWord->setObjectName(QStringLiteral("TrackNoticeWord")); m_noticeWord->setChecked(m_draft.noticeWord);
    m_noticeBitAddress = new QLineEdit(m_draft.noticeBitAddress, m_notice); m_noticeBitAddress->setObjectName(QStringLiteral("TrackNoticeBitAddress")); m_noticeWordAddress = new QLineEdit(m_draft.noticeWordAddress, m_notice); m_noticeWordAddress->setObjectName(QStringLiteral("TrackNoticeWordAddress")); bitPicker = new QPushButton(QStringLiteral("…"), m_notice); wordPicker = new QPushButton(QStringLiteral("…"), m_notice);
    statePanel = new QWidget(m_notice); row = new QHBoxLayout(statePanel); row->setContentsMargins(0, 0, 0, 0); m_noticeBitOn = new QRadioButton(QStringLiteral("1"), statePanel); m_noticeBitOn->setObjectName(QStringLiteral("TrackNoticeBitOn")); off = new QRadioButton(QStringLiteral("0"), statePanel); off->setObjectName(QStringLiteral("TrackNoticeBitOff")); m_noticeBitOn->setChecked(m_draft.noticeBitValue); off->setChecked(!m_draft.noticeBitValue); row->addWidget(m_noticeBitOn); row->addWidget(off);
    m_noticeWordType = new QComboBox(m_notice); m_noticeWordType->setObjectName(QStringLiteral("TrackNoticeWordType")); m_noticeWordType->addItems(StaticTextProperties::dataTypes()); m_noticeWordType->setCurrentIndex(m_draft.noticeWordType); m_noticeWordValue = new QLineEdit(m_draft.noticeWordValue, m_notice); m_noticeWordValue->setObjectName(QStringLiteral("TrackNoticeWordValue"));
    grid->addWidget(m_noticeBit, 0, 0); grid->addWidget(m_noticeBitAddress, 0, 1); grid->addWidget(bitPicker, 0, 2); grid->addWidget(m_noticeWord, 0, 3); grid->addWidget(m_noticeWordAddress, 0, 4); grid->addWidget(wordPicker, 0, 5); grid->addWidget(statePanel, 1, 0, 1, 3); grid->addWidget(m_noticeWordType, 1, 3, 1, 3); grid->addWidget(m_noticeWordValue, 2, 3, 1, 3); layout->addWidget(m_notice, 1, 0, 1, 2); layout->setRowStretch(2, 1);
    connect(m_noticeBit, &QCheckBox::toggled, this, [this, statePanel, bitPicker](bool enabled) { if (enabled) { m_noticeWord->setChecked(false); } m_noticeBitAddress->setEnabled(enabled); statePanel->setEnabled(enabled); bitPicker->setEnabled(enabled); });
    connect(m_noticeWord, &QCheckBox::toggled, this, [this, wordPicker](bool enabled) { if (enabled) { m_noticeBit->setChecked(false); } m_noticeWordAddress->setEnabled(enabled); m_noticeWordType->setEnabled(enabled); m_noticeWordValue->setEnabled(enabled); wordPicker->setEnabled(enabled); });
    m_noticeBitAddress->setEnabled(m_draft.noticeBit); statePanel->setEnabled(m_draft.noticeBit); bitPicker->setEnabled(m_draft.noticeBit); m_noticeWordAddress->setEnabled(m_draft.noticeWord); m_noticeWordType->setEnabled(m_draft.noticeWord); m_noticeWordValue->setEnabled(m_draft.noticeWord); wordPicker->setEnabled(m_draft.noticeWord);
    connect(bitPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_noticeBitAddress, true); }); connect(wordPicker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_noticeWordAddress, false, m_noticeWordType->currentIndex() == 0 || m_noticeWordType->currentIndex() == 2 ? 1 : 2); });
    return page;
}


