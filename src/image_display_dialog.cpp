#include "image_display_dialog.h"
#include "bit_button_dialog.h"
#include "hmi_property_widgets.h"
#include "control_rendering.h"
#include "kvg_library.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : paintImageDisplay
 * Description     : Clip and stretch the selected actual picture state without inventing PLC values or timed runtime transitions.
 * Input Parameters: painter, rect, properties, state
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintImageDisplay(QPainter *painter, const QRectF &rect, const ImageDisplayProperties &properties, int state) //20261112-0 add
{
    ImageDisplayState selected;
    QImage image;
    if (properties.states.isEmpty()) { return; }
    selected = properties.states.at(qBound(0, state, int(properties.states.size()) - 1));
    painter->save(); painter->setClipRect(rect, Qt::IntersectClip);
    if (!selected.picturePng.isEmpty())
    {
        image = decodedControlPng(selected.picturePng);
        if (!image.isNull()) { painter->drawImage(rect, image); }
    }
    else if (!selected.picture.isEmpty()) { paintKvgPicture(painter, rect, selected.picture, 0); }
    painter->restore();
}

/******************************************************************************
 * Function Name   : ImageDisplayDialog::ImageDisplayDialog
 * Description     : Reproduce picture mode, state table, batch controls and visibility pages with a resource-aware child transaction.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
ImageDisplayDialog::ImageDisplayDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261112-0 add
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.imageDisplay)
{
    QVBoxLayout *root;
    QHBoxLayout *content;
    QTabWidget *tabs;
    QWidget *general;
    QVBoxLayout *generalLayout;
    QGroupBox *type;
    QHBoxLayout *typeLayout;
    QHBoxLayout *conditionLayout;
    QGroupBox *controlled;
    QFormLayout *controlledLayout;
    QHBoxLayout *addressRow;
    QHBoxLayout *intervalRow;
    QRadioButton *radio;
    QWidget *picturesPage;
    QVBoxLayout *picturesLayout;
    QGroupBox *batch;
    QHBoxLayout *batchLayout;
    QGroupBox *shape;
    QHBoxLayout *shapeLayout;
    QHBoxLayout *tableLayout;
    QVBoxLayout *movement;
    QPushButton *button;
    QWidget *security;
    QVBoxLayout *securityLayout;
    QVBoxLayout *side;
    QGroupBox *preview;
    QVBoxLayout *previewLayout;
    QGroupBox *hint;
    QVBoxLayout *hintLayout;
    QLabel *hintText;
    QDialogButtonBox *buttons;
    QStringList names;
    QJsonArray connections;
    PlcConnection connection;
    int index;
    setObjectName(QStringLiteral("ImageDisplayDialog")); setWindowTitle(QStringLiteral("图片显示器")); resize(795, 578); setFont(QFont(QStringLiteral("SimSun"), 9));
    setStyleSheet(QStringLiteral("QComboBox,QSpinBox,QLineEdit { background:#caeace; } QGroupBox { background:white; }"));
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); }
    setProperty("plcConnections", connections.toVariantList());
    root = new QVBoxLayout(this); content = new QHBoxLayout(); tabs = new QTabWidget(this); tabs->setObjectName(QStringLiteral("ImageTabs"));
    general = new QWidget(tabs); generalLayout = new QVBoxLayout(general); type = new QGroupBox(QStringLiteral("显示类型"), general); typeLayout = new QHBoxLayout(type); m_type = new QButtonGroup(this);
    names = {QStringLiteral("静态图片"), QStringLiteral("多幅切换")};
    for (index = 0; index < names.size(); ++index) { radio = new QRadioButton(names.at(index), type); radio->setObjectName(QStringLiteral("ImageType%1").arg(index)); m_type->addButton(radio, index); radio->setChecked(m_draft.displayType == index); typeLayout->addWidget(radio); }
    generalLayout->addWidget(type); m_conditions = new QGroupBox(QStringLiteral("状态切换条件"), general); m_conditions->setObjectName(QStringLiteral("ImageConditions")); m_conditions->setCheckable(true); m_conditions->setChecked(m_draft.conditionEnabled);
    conditionLayout = new QHBoxLayout(m_conditions); m_condition = new QButtonGroup(this); names = {QStringLiteral("时间"), QStringLiteral("数值"), QStringLiteral("位")};
    for (index = 0; index < names.size(); ++index) { radio = new QRadioButton(names.at(index), m_conditions); radio->setObjectName(QStringLiteral("ImageCondition%1").arg(index)); m_condition->addButton(radio, index); radio->setChecked(m_draft.condition == index); conditionLayout->addWidget(radio); }
    generalLayout->addWidget(m_conditions); controlled = new QGroupBox(QStringLiteral("受控设置"), general); controlledLayout = new QFormLayout(controlled);
    m_count = new QSpinBox(controlled); m_count->setObjectName(QStringLiteral("ImageStateCount")); m_count->setRange(1, 128); m_count->setValue(m_draft.stateCount);
    m_address = new QLineEdit(m_draft.monitorAddress, controlled); m_address->setObjectName(QStringLiteral("ImageMonitorAddress")); m_picker = new QPushButton(QStringLiteral("…"), controlled); m_picker->setObjectName(QStringLiteral("ImageAddressPicker")); m_picker->setFixedWidth(28);
    addressRow = new QHBoxLayout(); addressRow->addWidget(m_address); addressRow->addWidget(m_picker);
    m_interval = new QSpinBox(controlled); m_interval->setObjectName(QStringLiteral("ImageInterval")); m_interval->setRange(1, 1000); m_interval->setValue(m_draft.interval); intervalRow = new QHBoxLayout(); intervalRow->addWidget(m_interval); intervalRow->addWidget(new QLabel(QStringLiteral("x100ms"), controlled));
    controlledLayout->addRow(QStringLiteral("总状态数:"), m_count); controlledLayout->addRow(QStringLiteral("监视地址:"), addressRow); controlledLayout->addRow(QStringLiteral("间隔时间:"), intervalRow); generalLayout->addWidget(controlled); generalLayout->addStretch(); tabs->addTab(general, QStringLiteral("基本"));
    picturesPage = new QWidget(tabs); picturesLayout = new QVBoxLayout(picturesPage); batch = new QGroupBox(QStringLiteral("批量操作"), picturesPage); batchLayout = new QHBoxLayout(batch);
    m_batchAdd = new QCheckBox(QStringLiteral("批量加"), batch); m_batchAdd->setObjectName(QStringLiteral("ImageBatchAdd")); m_batchDelete = new QCheckBox(QStringLiteral("批量删"), batch); m_batchDelete->setObjectName(QStringLiteral("ImageBatchDelete"));
    m_start = new QSpinBox(batch); m_start->setObjectName(QStringLiteral("ImageBatchStart")); m_end = new QSpinBox(batch); m_end->setObjectName(QStringLiteral("ImageBatchEnd")); m_start->setRange(0, m_draft.stateCount - 1); m_end->setRange(0, m_draft.stateCount - 1); m_end->setValue(m_draft.stateCount - 1); m_start->setEnabled(false); m_end->setEnabled(false);
    batchLayout->addWidget(m_batchAdd); batchLayout->addWidget(m_batchDelete); batchLayout->addWidget(new QLabel(QStringLiteral("起始位置:"), batch)); batchLayout->addWidget(m_start); batchLayout->addWidget(new QLabel(QStringLiteral("结束位置:"), batch)); batchLayout->addWidget(m_end); picturesLayout->addWidget(batch);
    shape = new QGroupBox(QStringLiteral("外形属性"), picturesPage); shapeLayout = new QHBoxLayout(shape); names = {QStringLiteral("删除该状态图片"), QStringLiteral("系统图库"), QStringLiteral("工程图库")};
    for (index = 0; index < names.size(); ++index)
    {
        button = new QPushButton(names.at(index), shape); button->setObjectName(QStringLiteral("ImagePictureAction%1").arg(index)); shapeLayout->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, index]() { if (index == 0) { deletePictures(); } else { selectPicture(index == 2); } });
    }
    picturesLayout->addWidget(shape); tableLayout = new QHBoxLayout(); m_table = new QTableWidget(picturesPage); m_table->setObjectName(QStringLiteral("ImageStates")); m_table->setColumnCount(2); m_table->setHorizontalHeaderLabels({QStringLiteral("S#"), QStringLiteral("图片")}); m_table->verticalHeader()->hide(); m_table->setSelectionBehavior(QAbstractItemView::SelectRows); m_table->setSelectionMode(QAbstractItemView::SingleSelection); m_table->setEditTriggers(QAbstractItemView::NoEditTriggers); m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch); m_table->setColumnWidth(0, 65); tableLayout->addWidget(m_table, 1);
    movement = new QVBoxLayout(); names = {QStringLiteral("▲"), QStringLiteral("▼")};
    for (index = 0; index < names.size(); ++index) { button = new QPushButton(names.at(index), picturesPage); button->setObjectName(QStringLiteral("ImageMove%1").arg(index)); movement->addWidget(button); connect(button, &QPushButton::clicked, this, [this, index]() { movePicture(index == 0 ? -1 : 1); }); }
    movement->addStretch(); tableLayout->addLayout(movement); picturesLayout->addLayout(tableLayout, 1); tabs->addTab(picturesPage, QStringLiteral("图片"));
    security = new QWidget(tabs); securityLayout = new QVBoxLayout(security); m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("ImageVisibility"), m_draft.visibility, false, security); securityLayout->addWidget(m_visibility); securityLayout->addStretch(); tabs->addTab(security, QStringLiteral("权限设置"));
    side = new QVBoxLayout(); preview = new QGroupBox(QStringLiteral("预览"), this); previewLayout = new QVBoxLayout(preview); m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("ImagePreview")); m_preview->setFixedSize(140, 150); previewLayout->addWidget(m_preview); side->addWidget(preview);
    hint = new QGroupBox(QStringLiteral("提示"), this); hintLayout = new QVBoxLayout(hint); hintText = new QLabel(QStringLiteral("支持png、jpg、bmp格式图片"), hint); hintText->setWordWrap(true); hintText->setStyleSheet(QStringLiteral("color:blue")); hintText->setMaximumWidth(140); hintLayout->addWidget(hintText); side->addWidget(hint); side->addStretch(); content->addWidget(tabs, 1); content->addLayout(side); root->addLayout(content, 1);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("ImageButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); root->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &ImageDisplayDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("图片显示帮助"), QStringLiteral("参见design_plan/图片显示功能与交互复刻手册.md。时间、数值和位条件用于运行时选图，编辑器通过状态下拉预览每一张图片。")); });
    connect(m_type, &QButtonGroup::idClicked, this, [this]() { updateMode(false); }); connect(m_condition, &QButtonGroup::idClicked, this, [this]() { updateMode(true); });
    connect(m_count, &QSpinBox::valueChanged, this, [this](int count) { if (!m_updating) { m_draft.setStateCount(count); rebuildTable(); } });
    connect(m_picker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_address, m_condition->checkedId() == 2); });
    connect(m_table, &QTableWidget::currentCellChanged, this, [this](int row, int, int, int) { if (!m_updating && row >= 0) { m_state = row; refreshPreview(); } });
    connect(m_batchAdd, &QCheckBox::toggled, this, [this]() { m_start->setEnabled(m_batchAdd->isChecked() || m_batchDelete->isChecked()); m_end->setEnabled(m_start->isEnabled()); });
    connect(m_batchDelete, &QCheckBox::toggled, this, [this]() { m_start->setEnabled(m_batchAdd->isChecked() || m_batchDelete->isChecked()); m_end->setEnabled(m_start->isEnabled()); });
    updateMode(false);
}

/******************************************************************************
 * Function Name   : ImageDisplayDialog::updateMode
 * Description     : Apply observed static/time/numeric/bit enable rules and destructively resize the active state list.
 * Input Parameters: conditionChanged
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ImageDisplayDialog::updateMode(bool conditionChanged) //20261112-0 add
{
    QSignalBlocker blocker(m_count);
    bool multiple = m_type->checkedId() == 1;
    bool wasMultiple = m_draft.displayType == 1;
    int condition = m_condition->checkedId();
    m_draft.displayType = multiple ? 1 : 0; m_draft.condition = condition;
    if (multiple) { m_draft.conditionEnabled = true; }
    m_conditions->setCheckable(!multiple); m_conditions->setChecked(m_draft.conditionEnabled); m_conditions->setEnabled(multiple);
    m_conditions->setTitle(!multiple && m_draft.conditionEnabled ? QStringLiteral("闪烁状态") : QStringLiteral("状态切换条件")); m_condition->button(1)->setVisible(multiple);
    m_count->setRange(multiple ? 2 : 1, multiple && condition != 2 ? 128 : multiple ? 2 : 1); m_count->setValue(qBound(m_count->minimum(), m_draft.stateCount, m_count->maximum())); m_count->setEnabled(multiple && condition != 2); m_draft.setStateCount(m_count->value());
    m_address->setEnabled(multiple && condition != 0); m_picker->setEnabled(m_address->isEnabled()); m_interval->setEnabled(multiple && condition == 0);
    if (conditionChanged && condition != 0) { m_address->setText(condition == 2 ? QStringLiteral("LB0") : QStringLiteral("LW0")); }
    rebuildTable();
    if (multiple && !wasMultiple) { m_end->setValue(m_draft.stateCount - 1); } //20261112-0 change: Entering two-state mode initializes 0..1; subsequent count growth preserves the end draft.
}

/******************************************************************************
 * Function Name   : ImageDisplayDialog::rebuildTable
 * Description     : Rebuild fixed numbered rows, clamp batch bounds and retain a valid current preview state.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ImageDisplayDialog::rebuildTable() //20261112-0 add
{
    int row;
    m_updating = true; m_table->setRowCount(m_draft.stateCount);
    for (row = 0; row < m_draft.stateCount; ++row) { m_table->setItem(row, 0, new QTableWidgetItem(QString::number(row))); m_table->setItem(row, 1, new QTableWidgetItem(m_draft.states.at(row).picture)); }
    m_start->setMaximum(m_draft.stateCount - 1); m_end->setMaximum(m_draft.stateCount - 1); m_state = qBound(0, m_state, m_draft.stateCount - 1); m_table->setCurrentCell(m_state, 0); m_updating = false; refreshPreview();
}

/******************************************************************************
 * Function Name   : ImageDisplayDialog::refreshPreview
 * Description     : Render the current table state through the same actual-image painter as the canvas.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ImageDisplayDialog::refreshPreview() //20261112-0 add
{
    QPixmap image(m_preview->size());
    QPainter painter;
    image.fill(QColor(240, 240, 240)); painter.begin(&image); paintImageDisplay(&painter, QRectF(15, 20, 110, 110), m_draft, m_state); painter.end(); m_preview->setPixmap(image);
}

/******************************************************************************
 * Function Name   : ImageDisplayDialog::selectPicture
 * Description     : Reuse the existing bounded gallery and commit its picture/pool only after nested confirmation.
 * Input Parameters: projectGallery
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ImageDisplayDialog::selectPicture(bool projectGallery) //20261112-0 add
{
    HmiObject adapter;
    HmiProject context = m_project;
    ImageDisplayState state = m_draft.states.at(m_state);
    QMap<QString, QByteArray> pool;
    QVector<QPair<QString, QByteArray>> choices;
    BitButtonDialog *gallery;
    int row;
    if (m_batchAdd->isChecked() && m_start->value() > m_end->value()) //20261112-0 change: Native batch-add reports an invalid interval before opening the gallery.
    { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), QStringLiteral("批量加起始结束位置异常，请重新设置。")); return; }
    context.objects.append(object()); adapter.id = QStringLiteral("picture-gallery-adapter"); adapter.type = QStringLiteral("switch"); adapter.address = QStringLiteral("LB0"); adapter.hasBitButtonProperties = true;
    gallery = new BitButtonDialog(adapter, context, this);
    if (gallery->choosePictureForControl(projectGallery, &state.picture, &state.picturePng, &pool, m_batchAdd->isChecked() ? &choices : nullptr))
    {
        m_project.pictures = pool;
        if (m_batchAdd->isChecked())
        {
            for (row = m_start->value(); row <= m_end->value() && row - m_start->value() < choices.size(); ++row)
            { m_draft.states[row] = {choices.at(row - m_start->value()).first, choices.at(row - m_start->value()).second}; }
        }
        else { m_draft.states[m_state] = state; }
        rebuildTable();
    }
    delete gallery;
}

/******************************************************************************
 * Function Name   : ImageDisplayDialog::deletePictures
 * Description     : Clear selected image bytes while preserving row numbers; a batch checkbox alone never deletes anything.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ImageDisplayDialog::deletePictures() //20261112-0 add
{
    int row;
    if (m_batchDelete->isChecked())
    {
        if (m_start->value() > m_end->value()) { return; } //20261112-0 change: Native reversed deletion leaves every row unchanged and opens no message.
        for (row = m_start->value(); row <= m_end->value(); ++row) { m_draft.states[row] = ImageDisplayState(); }
    }
    else if (!m_table->selectedItems().isEmpty()) { m_draft.states[m_state] = ImageDisplayState(); }
    else { return; }
    rebuildTable();
}

/******************************************************************************
 * Function Name   : ImageDisplayDialog::movePicture
 * Description     : Exchange adjacent picture bindings without moving fixed state numbers or changing resources.
 * Input Parameters: offset
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ImageDisplayDialog::movePicture(int offset) //20261112-0 add
{
    int target = m_state + offset;
    if (m_table->selectedItems().isEmpty() || target < 0 || target >= m_draft.states.size()) { return; }
    m_draft.states.swapItemsAt(m_state, target); m_state = target; rebuildTable();
}

/******************************************************************************
 * Function Name   : ImageDisplayDialog::object
 * Description     : Return all image properties and the monitor mirror without exposing mutable live project state.
 * Input Parameters: None
 * Return Value    : Complete detached image object.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiObject ImageDisplayDialog::object() const //20261112-0 add
{
    HmiObject result = m_original;
    result.type = QStringLiteral("image"); result.hasImageDisplayProperties = true; result.imageDisplay = m_draft;
    result.imageDisplay.monitorAddress = m_address->text().trimmed(); result.imageDisplay.interval = m_interval->value(); result.imageDisplay.visibility = m_visibility->properties(); result.address = result.imageDisplay.monitorAddress; return result;
}

/******************************************************************************
 * Function Name   : ImageDisplayDialog::pictures
 * Description     : Return the accepted child-gallery resource draft for one whole-object transaction.
 * Input Parameters: None
 * Return Value    : Detached picture pool.
 * Date            : 2026.11.12
 ******************************************************************************/
QMap<QString, QByteArray> ImageDisplayDialog::pictures() const //20261112-0 add
{
    return m_project.pictures;
}

/******************************************************************************
 * Function Name   : ImageDisplayDialog::accept
 * Description     : Keep the dialog open for empty states or invalid active addresses before publishing any resources.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void ImageDisplayDialog::accept() //20261112-0 add
{
    HmiObject candidate = object();
    QString error;
    if (!candidate.imageDisplay.validate(&error) || !candidate.imageDisplay.validateAddresses(m_project.connections, &error))
    { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), error); return; }
    QDialog::accept();
}
