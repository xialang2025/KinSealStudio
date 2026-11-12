#include "message_display_dialog.h"
#include "hmi_property_widgets.h"
#include "static_text_renderer.h"
#include "plc_address_dialog.h"
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : paintMessageDisplay
 * Description     : Share one clipped state/language renderer between message preview and the canvas.
 * Input Parameters: painter, rect, properties, language, state
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintMessageDisplay(QPainter *painter, const QRectF &rect, const MessageDisplayProperties &properties,
                         const QString &language, int state) //20261112-0 add
{
    painter->save();
    painter->setClipRect(rect, Qt::IntersectClip);
    paintStaticText(painter, rect, properties.text(qBound(0, state, properties.stateCount - 1), language), properties.languageAppearance(language));
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(properties.languageFrameColor(language), 1));
    painter->drawRect(rect.adjusted(0.5, 0.5, -0.5, -0.5));
    painter->restore();
}

/******************************************************************************
 * Function Name   : MessageDisplayDialog::MessageDisplayDialog
 * Description     : Reproduce both message pages, language/state drafts and their observed enable/selection behavior.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
MessageDisplayDialog::MessageDisplayDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261112-0 add
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.messageDisplay)
{
    QVBoxLayout *root;
    QHBoxLayout *content;
    QTabWidget *tabs;
    QWidget *general;
    QGridLayout *grid;
    QGroupBox *data;
    QGroupBox *fill;
    QFormLayout *dataForm;
    QFormLayout *fillForm;
    QWidget *addressRow;
    QHBoxLayout *addressLayout;
    QPushButton *picker;
    QLabel *speedLabel;
    QGroupBox *previewGroup;
    QVBoxLayout *previewLayout;
    QWidget *security;
    QVBoxLayout *securityLayout;
    QDialogButtonBox *buttons;
    QPushButton *colorButton;
    QStringList colorNames;
    QStringList colorIds;
    QColor *color;
    QJsonArray connections;
    PlcConnection connection;
    int index;

    setObjectName(QStringLiteral("MessageDisplayDialog")); setWindowTitle(QStringLiteral("消息显示器"));
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); }
    setProperty("plcConnections", connections.toVariantList()); // Preserve the project context for nested monitor and permission pickers.
    resize(850, 614); setFont(QFont(QStringLiteral("SimSun"), 9));
    setStyleSheet(QStringLiteral("QComboBox, QSpinBox, QLineEdit { background-color:#caeace; } QGroupBox { background:white; }"));
    m_currentLanguage = project.languages.contains(project.activeLanguage) ? project.activeLanguage : project.languages.value(0, QStringLiteral("中文"));
    m_currentAppearance = m_draft.languageAppearance(m_currentLanguage); m_currentFrameColor = m_draft.languageFrameColor(m_currentLanguage);
    root = new QVBoxLayout(this); content = new QHBoxLayout(); tabs = new QTabWidget(this);
    tabs->setObjectName(QStringLiteral("MessageTabs")); general = new QWidget(tabs); grid = new QGridLayout(general);
    data = new QGroupBox(QStringLiteral("数据"), general); dataForm = new QFormLayout(data);
    data->setFixedSize(260, 184);
    m_count = new QSpinBox(data); m_count->setObjectName(QStringLiteral("MessageStateCount")); m_count->setRange(1, 128); m_count->setValue(m_draft.stateCount);
    m_type = new QComboBox(data); m_type->setObjectName(QStringLiteral("MessageDataType"));
    m_type->addItems({QStringLiteral("16位正整数"), QStringLiteral("32位正整数")}); m_type->setCurrentIndex(m_draft.dataType);
    m_address = new QLineEdit(m_draft.monitorAddress, data); m_address->setObjectName(QStringLiteral("MessageMonitorAddress"));
    addressRow = new QWidget(data); addressLayout = new QHBoxLayout(addressRow); addressLayout->setContentsMargins(0, 0, 0, 0);
    picker = new QPushButton(QStringLiteral("…"), addressRow); picker->setObjectName(QStringLiteral("MessageAddressPicker")); picker->setFixedWidth(28);
    addressLayout->addWidget(m_address); addressLayout->addWidget(picker);
    m_movement = new QComboBox(data); m_movement->setObjectName(QStringLiteral("MessageMovement"));
    m_movement->addItems({QStringLiteral("不移动"), QStringLiteral("向左移动"), QStringLiteral("向右移动")}); m_movement->setCurrentIndex(m_draft.movement);
    m_speed = new QSpinBox(data); m_speed->setObjectName(QStringLiteral("MessageSpeed")); m_speed->setRange(1, 10); m_speed->setValue(m_draft.speed);
    speedLabel = new QLabel(QStringLiteral("移动速度:"), data);
    dataForm->addRow(QStringLiteral("总状态数:"), m_count); dataForm->addRow(QStringLiteral("数据类别:"), m_type);
    dataForm->addRow(QStringLiteral("监视地址:"), addressRow); dataForm->addRow(QStringLiteral("走 马 灯:"), m_movement); dataForm->addRow(speedLabel, m_speed);
    m_speed->setVisible(m_draft.movement != 0); speedLabel->setVisible(m_draft.movement != 0);
    fill = new QGroupBox(QStringLiteral("填充"), general); fillForm = new QFormLayout(fill);
    fill->setFixedSize(260, 254);
    m_language = new QComboBox(fill); m_language->setObjectName(QStringLiteral("MessageLanguage")); m_language->addItems(project.languages); m_language->setCurrentText(m_currentLanguage);
    m_font = new QComboBox(fill); m_font->setObjectName(QStringLiteral("MessageFont")); m_font->addItem(QStringLiteral("宋体"));
    if (m_currentAppearance.fontFamily != QStringLiteral("宋体")) { m_font->addItem(m_currentAppearance.fontFamily); }
    m_font->setCurrentText(m_currentAppearance.fontFamily);
    m_fontSize = new QComboBox(fill); m_fontSize->setObjectName(QStringLiteral("MessageFontSize"));
    m_fontSize->addItems({QStringLiteral("12"), QStringLiteral("16"), QStringLiteral("24"), QStringLiteral("48"), QStringLiteral("64"), QStringLiteral("96"), QStringLiteral("192")}); m_fontSize->setCurrentText(QString::number(m_currentAppearance.fontSize));
    fillForm->addRow(QStringLiteral("语    言:"), m_language); fillForm->addRow(QStringLiteral("字    体:"), m_font); fillForm->addRow(QStringLiteral("字体大小:"), m_fontSize);
    colorNames = {QStringLiteral("字体颜色:"), QStringLiteral("前 景 色:"), QStringLiteral("背 景 色:"), QStringLiteral("边 框 色:")};
    colorIds = {QStringLiteral("MessageTextColor"), QStringLiteral("MessageForegroundColor"), QStringLiteral("MessageBackgroundColor"), QStringLiteral("MessageFrameColor")};
    for (index = 0; index < colorNames.size(); ++index)
    {
        color = index == 0 ? &m_currentAppearance.textColor : index == 1 ? &m_currentAppearance.foregroundColor : index == 2 ? &m_currentAppearance.backgroundColor : &m_currentFrameColor;
        colorButton = new QPushButton(fill); colorButton->setObjectName(colorIds.at(index)); colorButton->setStyleSheet(QStringLiteral("background-color:%1;").arg(color->name()));
        m_colorButtons.append(colorButton);
        fillForm->addRow(colorNames.at(index), colorButton);
        connect(colorButton, &QPushButton::clicked, this, [this, color, colorButton]() {
            QColor selected;
            selected = QColorDialog::getColor(*color, this, QStringLiteral("选择颜色"));
            if (selected.isValid()) { *color = selected; colorButton->setStyleSheet(QStringLiteral("background-color:%1;").arg(selected.name())); refreshPreview(); }
        });
    }
    m_style = new QComboBox(fill); m_style->setObjectName(QStringLiteral("MessageStyle")); m_style->addItems(StaticTextProperties::styles()); m_style->setCurrentIndex(m_currentAppearance.style);
    fillForm->addRow(QStringLiteral("样    式:"), m_style);
    m_table = new QTableWidget(general); m_table->setObjectName(QStringLiteral("MessageStates")); m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({QStringLiteral("列"), QStringLiteral("文本")}); m_table->verticalHeader()->hide(); m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection); m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed); m_table->setColumnWidth(0, 100); m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_text = new QPlainTextEdit(general); m_text->setObjectName(QStringLiteral("MessageTextEditor")); m_text->setMaximumHeight(130);
    grid->addWidget(data, 0, 0, Qt::AlignTop); grid->addWidget(fill, 1, 0, 2, 1, Qt::AlignBottom); grid->addWidget(m_table, 0, 1, 2, 1); grid->addWidget(m_text, 2, 1); grid->setColumnStretch(1, 1);
    tabs->addTab(general, QStringLiteral("一般"));
    security = new QWidget(tabs); securityLayout = new QVBoxLayout(security);
    m_permission = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("MessageVisibility"), m_draft.appearance, false, security);
    securityLayout->addWidget(m_permission); securityLayout->addStretch(); tabs->addTab(security, QStringLiteral("权限设置"));
    previewGroup = new QGroupBox(QStringLiteral("预览"), this); previewLayout = new QVBoxLayout(previewGroup);
    m_preview = new QLabel(previewGroup); m_preview->setObjectName(QStringLiteral("MessagePreview")); m_preview->setFixedSize(130, 141); previewLayout->addWidget(m_preview);
    content->addWidget(tabs, 1); content->addWidget(previewGroup, 0, Qt::AlignTop); root->addLayout(content, 1);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("MessageButtons"));
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); root->addWidget(buttons);
    buttons->button(QDialogButtonBox::Ok)->setMinimumSize(72, 28); buttons->button(QDialogButtonBox::Cancel)->setMinimumSize(72, 28); buttons->button(QDialogButtonBox::Help)->setMinimumSize(72, 28);
    connect(buttons, &QDialogButtonBox::accepted, this, &MessageDisplayDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("消息显示帮助"), QStringLiteral("参见design_plan/消息显示功能与交互复刻手册.md。监视地址选择状态，移动方向仅用于设备运行；当前PC预览不模拟PLC或滚动时序。")); });
    connect(picker, &QPushButton::clicked, this, [this]() { editHmiAddress(this, m_address, false, m_type->currentIndex() == 0 ? 1 : 2); });
    connect(m_count, &QSpinBox::valueChanged, this, [this](int count) { m_draft.stateCount = count; m_draft.ensureStates(count); rebuildTable(); });
    connect(m_movement, &QComboBox::currentIndexChanged, this, [this, speedLabel](int movement) { m_draft.movement = movement; m_speed->setVisible(movement != 0); speedLabel->setVisible(movement != 0); });
    connect(m_language, &QComboBox::currentTextChanged, this, &MessageDisplayDialog::changeLanguage);
    connect(m_table, &QTableWidget::currentCellChanged, this, [this](int row, int, int, int) { if (!m_refreshing) { selectState(row); } });
    connect(m_text, &QPlainTextEdit::textChanged, this, [this]() {
        if (m_refreshing) { return; }
        m_draft.states[m_state].insert(m_currentLanguage, m_text->toPlainText());
        m_table->item(m_state, 1)->setText(m_text->toPlainText()); refreshPreview();
    });
    connect(m_font, &QComboBox::currentTextChanged, this, [this](const QString &font) { m_currentAppearance.fontFamily = font; refreshPreview(); });
    connect(m_fontSize, &QComboBox::currentTextChanged, this, [this](const QString &size) { m_currentAppearance.fontSize = size.toDouble(); refreshPreview(); });
    connect(m_style, &QComboBox::currentIndexChanged, this, [this](int style) { m_currentAppearance.style = style; refreshPreview(); });
    rebuildTable();
}

/******************************************************************************
 * Function Name   : MessageDisplayDialog::rebuildTable
 * Description     : Rebuild active rows without mixing translations or deleting hidden state drafts.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MessageDisplayDialog::rebuildTable() //20261112-0 add
{
    int row;
    m_refreshing = true; m_table->setRowCount(m_draft.stateCount);
    for (row = 0; row < m_draft.stateCount; ++row)
    {
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(row)));
        m_table->setItem(row, 1, new QTableWidgetItem(m_draft.text(row, m_currentLanguage)));
    }
    m_state = qBound(0, m_state, m_draft.stateCount - 1); m_table->setCurrentCell(m_state, 0); m_refreshing = false;
    selectState(m_state);
}

/******************************************************************************
 * Function Name   : MessageDisplayDialog::selectState
 * Description     : Load and select all of the chosen state's current-language text for replacement.
 * Input Parameters: row
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MessageDisplayDialog::selectState(int row) //20261112-0 add
{
    if (row < 0 || row >= m_draft.stateCount) { return; }
    m_state = row; m_refreshing = true; m_text->setPlainText(m_draft.text(row, m_currentLanguage)); m_text->selectAll(); m_refreshing = false;
    refreshPreview();
}

/******************************************************************************
 * Function Name   : MessageDisplayDialog::refreshPreview
 * Description     : Paint only the selected state while keeping PLC changes and runtime timing outside this editor.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MessageDisplayDialog::refreshPreview() //20261112-0 add
{
    QPixmap image(m_preview->size());
    QPainter painter;
    image.fill(QColor(240, 240, 240));
    m_draft.languageAppearances.insert(m_currentLanguage, m_currentAppearance);
    m_draft.languageFrameColors.insert(m_currentLanguage, m_currentFrameColor);
    painter.begin(&image);
    paintMessageDisplay(&painter, QRectF(15, 20, 100, 100), m_draft, m_currentLanguage, m_state);
    painter.end(); m_preview->setPixmap(image);
}

/******************************************************************************
 * Function Name   : MessageDisplayDialog::changeLanguage
 * Description     : Save the current language draft and restore every visual field of the newly selected language.
 * Input Parameters: language
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MessageDisplayDialog::changeLanguage(const QString &language) //20261112-0 add
{
    QSignalBlocker fontBlocker(m_font);
    QSignalBlocker sizeBlocker(m_fontSize);
    QSignalBlocker styleBlocker(m_style);
    QList<QColor> colors;
    int index;
    m_draft.languageAppearances.insert(m_currentLanguage, m_currentAppearance);
    m_draft.languageFrameColors.insert(m_currentLanguage, m_currentFrameColor);
    m_currentLanguage = language; m_currentAppearance = m_draft.languageAppearance(language); m_currentFrameColor = m_draft.languageFrameColor(language);
    if (m_font->findText(m_currentAppearance.fontFamily) < 0) { m_font->addItem(m_currentAppearance.fontFamily); }
    m_font->setCurrentText(m_currentAppearance.fontFamily); m_fontSize->setCurrentText(QString::number(m_currentAppearance.fontSize)); m_style->setCurrentIndex(m_currentAppearance.style);
    colors = {m_currentAppearance.textColor, m_currentAppearance.foregroundColor, m_currentAppearance.backgroundColor, m_currentFrameColor};
    for (index = 0; index < colors.size(); ++index) { m_colorButtons.at(index)->setStyleSheet(QStringLiteral("background-color:%1;").arg(colors.at(index).name())); }
    rebuildTable();
}

/******************************************************************************
 * Function Name   : MessageDisplayDialog::object
 * Description     : Return an independent draft with current data fields and the original resource-independent identity.
 * Input Parameters: None
 * Return Value    : A complete message object; live project state is untouched.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiObject MessageDisplayDialog::object() const //20261112-0 add
{
    HmiObject result = m_original;
    result.messageDisplay = m_draft;
    result.messageDisplay.dataType = m_type->currentIndex(); result.messageDisplay.monitorAddress = m_address->text().trimmed(); result.messageDisplay.speed = m_speed->value();
    result.messageDisplay.languageAppearances.insert(m_currentLanguage, m_currentAppearance); result.messageDisplay.languageFrameColors.insert(m_currentLanguage, m_currentFrameColor);
    result.messageDisplay.appearance = m_permission->properties(); result.text = result.messageDisplay.text(0, m_project.languages.value(0)); result.address = result.messageDisplay.monitorAddress;
    return result;
}

/******************************************************************************
 * Function Name   : MessageDisplayDialog::accept
 * Description     : Validate all active and dormant fields plus project-qualified address spans before closing.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void MessageDisplayDialog::accept() //20261112-0 add
{
    HmiObject candidate;
    HmiProject project;
    QString error;
    candidate = object(); project = m_project; project.objects.clear();
    candidate.id = QStringLiteral("message-dialog-validation"); candidate.name = QStringLiteral("消息显示器");
    project.objects.append(candidate); project.inactivePages.clear(); project.pageOrder = {project.activePageId}; project.startupPageId = project.activePageId;
    project.references.clear();
    if (!candidate.messageDisplay.validate(&error) || !project.validatePlcConfiguration(&error))
    { QMessageBox::warning(this, QStringLiteral("消息显示属性无效"), error); return; }
    QDialog::accept();
}
