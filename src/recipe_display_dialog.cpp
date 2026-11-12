#include "recipe_display_dialog.h"
#include "hmi_property_widgets.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFontMetricsF>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : paintRecipeDisplay
 * Description     : Draw bounded illustrative recipe rows with resolved definition headers and an explicit insufficient-width message.
 * Input Parameters: painter, rect, properties, titles, recipeCount
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void paintRecipeDisplay(QPainter *painter, const QRectF &rect, const RecipeDisplayProperties &properties, const QStringList &titles, int recipeCount) //20261113-2 add
{
    QFont font(QStringLiteral("SimSun"));
    qreal minimumWidth;
    qreal columnWidth;
    qreal rowHeight;
    int column;
    int row;
    int rows = qBound(1, recipeCount, 20);
    Qt::Alignment alignment = properties.alignment == 0 ? Qt::AlignLeft : properties.alignment == 2 ? Qt::AlignRight : Qt::AlignHCenter;
    QRectF cell;
    font.setPixelSize(properties.fontSize); minimumWidth = qMax(40, properties.fontSize * 3) + qMax(1, int(titles.size()) - 1) * qMax(80, properties.fontSize * 5);
    if (rect.isEmpty()) { return; } painter->save(); painter->setClipRect(rect, Qt::IntersectClip); painter->fillRect(rect, QColor(properties.backgroundColor)); painter->setFont(font); painter->setPen(QColor(properties.textColor));
    if (titles.isEmpty() || rect.width() < minimumWidth) { painter->drawText(rect.adjusted(2, 2, -2, -2), Qt::AlignCenter | Qt::TextWordWrap, titles.isEmpty() ? QStringLiteral("配方不存在，请先创建配方") : QStringLiteral("控件宽度不足，无法显示数据")); painter->restore(); return; }
    columnWidth = rect.width() / titles.size(); rowHeight = rect.height() / (rows + 1); painter->fillRect(QRectF(rect.left(), rect.top(), rect.width(), rowHeight), QColor(properties.titleBackground)); painter->setPen(QColor(properties.titleColor));
    for (column = 0; column < titles.size(); ++column) { cell = QRectF(rect.left() + column * columnWidth, rect.top(), columnWidth, rowHeight); painter->save(); painter->setClipRect(cell.adjusted(1, 1, -1, -1), Qt::IntersectClip); painter->drawText(cell.adjusted(3, 0, -3, 0), alignment | Qt::AlignVCenter | Qt::TextSingleLine, titles[column]); painter->restore(); }
    if (properties.showGrid) { painter->setPen(QPen(QColor(properties.gridColor), 1)); for (column = 0; column <= titles.size(); ++column) { painter->drawLine(QPointF(rect.left() + column * columnWidth, rect.top()), QPointF(rect.left() + column * columnWidth, rect.bottom())); } for (row = 0; row <= rows + 1; ++row) { painter->drawLine(QPointF(rect.left(), rect.top() + row * rowHeight), QPointF(rect.right(), rect.top() + row * rowHeight)); } }
    painter->restore();
}

/******************************************************************************
 * Function Name   : RecipeDisplayDialog::color
 * Description     : Resolve the five observed independent common color fields.
 * Input Parameters: index
 * Return Value    : Selected color draft.
 * Date            : 2026.11.13
 ******************************************************************************/
QString *RecipeDisplayDialog::color(int index) //20261113-2 add
{
    if (index == 0) { return &m_draft.titleColor; } if (index == 1) { return &m_draft.textColor; } if (index == 2) { return &m_draft.titleBackground; } if (index == 3) { return &m_draft.backgroundColor; } return &m_draft.gridColor;
}

/******************************************************************************
 * Function Name   : RecipeDisplayDialog::addColor
 * Description     : Bind one color swatch to the common picker while retaining canceled parent and child drafts.
 * Input Parameters: layout, row, label, index
 * Return Value    : Bound swatch button.
 * Date            : 2026.11.13
 ******************************************************************************/
QPushButton *RecipeDisplayDialog::addColor(QGridLayout *layout, int row, const QString &label, int index) //20261113-2 add
{
    QPushButton *button = new QPushButton(this);
    QLabel *caption = new QLabel(label, this);
    button->setObjectName(QStringLiteral("RecipeDisplayColor%1").arg(index)); button->setFixedSize(130, 24); button->setStyleSheet(QStringLiteral("background:%1;").arg(*color(index))); layout->addWidget(caption, row, 0); layout->addWidget(button, row, 1); if (index == 4) { m_gridLabel = caption; }
    connect(button, &QPushButton::clicked, this, [this, button, index]() { QColor selected = QColorDialog::getColor(QColor(*color(index)), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog); if (selected.isValid()) { *color(index) = selected.name(); button->setStyleSheet(QStringLiteral("background:%1;").arg(selected.name())); } }); return button;
}

/******************************************************************************
 * Function Name   : RecipeDisplayDialog::basicPage
 * Description     : Reproduce the recipe selector, edit/keyboard expansion and common text/appearance groups.
 * Input Parameters: None
 * Return Value    : Measured basic recipe-display page.
 * Date            : 2026.11.13
 ******************************************************************************/
QWidget *RecipeDisplayDialog::basicPage() //20261113-2 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QComboBox *recipe = new QComboBox(page);
    QHBoxLayout *selector = new QHBoxLayout();
    QVBoxLayout *editLayout;
    QGridLayout *keyboardLayout;
    QGridLayout *coordinates;
    QGroupBox *text = new QGroupBox(QStringLiteral("文本设置"), page);
    QGroupBox *appearance = new QGroupBox(QStringLiteral("外观设置"), page);
    QGridLayout *grid = new QGridLayout(text);
    QComboBox *font = new QComboBox(text);
    QComboBox *size = new QComboBox(text);
    QComboBox *align = new QComboBox(text);
    QCheckBox *showGrid = new QCheckBox(QStringLiteral("使用网格"), appearance);
    QSpinBox *x;
    QSpinBox *y;
    HmiConfigurationNode node;
    HmiPage keyboardPage;
    int value;
    recipe->setObjectName(QStringLiteral("RecipeDisplayDefinition")); Q_FOREACH (node, m_project.configurationNodes) { if (node.kind == QStringLiteral("recipe") && node.active && node.implemented) { recipe->addItem(node.name, node.id); } } if (m_draft.recipeId.isEmpty() && recipe->count() > 0) { m_draft.recipeId = recipe->itemData(0).toString(); } recipe->setCurrentIndex(recipe->findData(m_draft.recipeId)); selector->addStretch(); selector->addWidget(new QLabel(QStringLiteral("配方名称:"), page)); selector->addWidget(recipe); selector->addStretch(); layout->addLayout(selector, 0, 0, 1, 2);
    m_edit = new QGroupBox(QStringLiteral("编辑配方"), page); m_edit->setObjectName(QStringLiteral("RecipeDisplayEdit")); m_edit->setCheckable(true); m_edit->setChecked(m_draft.editEnabled); m_edit->setMinimumHeight(116); editLayout = new QVBoxLayout(m_edit); m_keyboardPanel = new QWidget(m_edit); keyboardLayout = new QGridLayout(m_keyboardPanel); m_keyboard = new QCheckBox(QStringLiteral("使用自定义键盘"), m_keyboardPanel); m_keyboard->setObjectName(QStringLiteral("RecipeDisplayCustomKeyboard")); m_keyboard->setChecked(m_draft.customKeyboard); m_keyboardPage = new QComboBox(m_keyboardPanel); m_keyboardPage->setObjectName(QStringLiteral("RecipeDisplayKeyboardPage")); Q_FOREACH (keyboardPage, m_project.allPages()) { if (keyboardPage.kind == QStringLiteral("keyboard")) { m_keyboardPage->addItem(keyboardPage.name, keyboardPage.id); } } m_keyboardPage->setCurrentIndex(m_keyboardPage->findData(m_draft.keyboardPageId)); m_keyboardLabel = new QLabel(QStringLiteral("窗口编号:"), m_keyboardPanel); m_position = new QCheckBox(QStringLiteral("自定义键盘位置"), m_keyboardPanel); m_position->setObjectName(QStringLiteral("RecipeDisplayKeyboardPosition")); m_position->setChecked(m_draft.keyboardPosition); keyboardLayout->addWidget(m_keyboard, 0, 0, 1, 4); keyboardLayout->addWidget(m_keyboardLabel, 1, 0); keyboardLayout->addWidget(m_keyboardPage, 1, 1); keyboardLayout->addWidget(m_position, 1, 2, 1, 2); m_coordinates = new QWidget(m_keyboardPanel); coordinates = new QGridLayout(m_coordinates); x = new QSpinBox(m_coordinates); y = new QSpinBox(m_coordinates); x->setObjectName(QStringLiteral("RecipeDisplayKeyboardX")); y->setObjectName(QStringLiteral("RecipeDisplayKeyboardY")); x->setRange(0, 800); y->setRange(0, 800); x->setValue(m_draft.keyboardX); y->setValue(m_draft.keyboardY); coordinates->addWidget(new QLabel(QStringLiteral("X:"), m_coordinates), 0, 0); coordinates->addWidget(x, 0, 1); coordinates->addWidget(new QLabel(QStringLiteral("Y:"), m_coordinates), 0, 2); coordinates->addWidget(y, 0, 3); keyboardLayout->addWidget(m_coordinates, 2, 0, 1, 4); editLayout->addWidget(m_keyboardPanel); editLayout->addStretch(); layout->addWidget(m_edit, 1, 0, 1, 2);
    font->setObjectName(QStringLiteral("RecipeDisplayFont")); font->addItem(QStringLiteral("宋体")); size->setObjectName(QStringLiteral("RecipeDisplayFontSize")); Q_FOREACH (value, (QList<int>{12,16,24,48})) { size->addItem(QString::number(value)); } size->setCurrentText(QString::number(m_draft.fontSize)); align->setObjectName(QStringLiteral("RecipeDisplayAlignment")); align->addItems({QStringLiteral("左"), QStringLiteral("居中"), QStringLiteral("右")}); align->setCurrentIndex(m_draft.alignment); grid->addWidget(new QLabel(QStringLiteral("字体:"), text), 0, 0); grid->addWidget(font, 0, 1); grid->addWidget(new QLabel(QStringLiteral("字体大小:"), text), 1, 0); grid->addWidget(size, 1, 1); grid->addWidget(new QLabel(QStringLiteral("文字对齐:"), text), 2, 0); grid->addWidget(align, 2, 1); addColor(grid, 3, QStringLiteral("标题文本色:"), 0); addColor(grid, 4, QStringLiteral("数据文本色:"), 1); grid->setRowStretch(5, 1); layout->addWidget(text, 2, 0);
    grid = new QGridLayout(appearance); addColor(grid, 0, QStringLiteral("标题背景色:"), 2); addColor(grid, 1, QStringLiteral("背景色:"), 3); showGrid->setObjectName(QStringLiteral("RecipeDisplayGrid")); showGrid->setChecked(m_draft.showGrid); grid->addWidget(showGrid, 2, 0, 1, 2); m_gridColor = addColor(grid, 3, QStringLiteral("网格颜色:"), 4); grid->setRowStretch(4, 1); layout->addWidget(appearance, 2, 1); layout->setRowStretch(2, 1);
    connect(recipe, &QComboBox::currentIndexChanged, this, [this, recipe](int index) { m_draft.recipeId = recipe->itemData(index).toString(); }); connect(m_edit, &QGroupBox::toggled, this, [this](bool value) { m_draft.editEnabled = value; refreshLinkage(); }); connect(m_keyboard, &QCheckBox::toggled, this, [this](bool value) { m_draft.customKeyboard = value; refreshLinkage(); }); connect(m_keyboardPage, &QComboBox::currentIndexChanged, this, [this](int index) { m_draft.keyboardPageId = m_keyboardPage->itemData(index).toString(); }); connect(m_position, &QCheckBox::toggled, this, [this](bool value) { m_draft.keyboardPosition = value; refreshLinkage(); }); connect(x, &QSpinBox::valueChanged, this, [this](int value) { m_draft.keyboardX = value; }); connect(y, &QSpinBox::valueChanged, this, [this](int value) { m_draft.keyboardY = value; }); connect(size, &QComboBox::currentTextChanged, this, [this](const QString &value) { m_draft.fontSize = value.toInt(); }); connect(align, &QComboBox::currentIndexChanged, this, [this](int value) { m_draft.alignment = value; }); connect(showGrid, &QCheckBox::toggled, this, [this](bool value) { m_draft.showGrid = value; refreshLinkage(); }); return page;
}

/******************************************************************************
 * Function Name   : RecipeDisplayDialog::permissionPage
 * Description     : Build two independent permission columns and the observed mutually exclusive bit/word notification forms.
 * Input Parameters: None
 * Return Value    : Independent visibility/touch and notification page.
 * Date            : 2026.11.13
 ******************************************************************************/
QWidget *RecipeDisplayDialog::permissionPage() //20261113-2 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGridLayout *grid;
    QGridLayout *fields;
    QLineEdit *bitAddress;
    QLineEdit *wordAddress;
    QLineEdit *wordValue;
    QPushButton *bitPicker;
    QPushButton *wordPicker;
    QComboBox *wordType;
    QRadioButton *on;
    QRadioButton *off;
    m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("RecipeDisplayVisible"), m_draft.visibility, false, page); m_touch = new HmiPermissionEditor(QStringLiteral("触控有效"), QStringLiteral("RecipeDisplayTouch"), m_draft.touch, true, page); m_visibility->configureUnchosenBitState(m_draft.visibilityBitChosen); m_touch->configureUnchosenBitState(m_draft.touchBitChosen); layout->addWidget(m_visibility, 0, 0); layout->addWidget(m_touch, 0, 1);
    m_notice = new QGroupBox(QStringLiteral("通知"), page); m_notice->setObjectName(QStringLiteral("RecipeDisplayNotice")); m_notice->setCheckable(true); m_notice->setChecked(m_draft.notice); grid = new QGridLayout(m_notice); m_noteBit = new QCheckBox(QStringLiteral("通知位"), m_notice); m_noteBit->setObjectName(QStringLiteral("RecipeDisplayNoticeBit")); m_noteBit->setChecked(m_draft.noticeBit); m_noteWord = new QCheckBox(QStringLiteral("通知字"), m_notice); m_noteWord->setObjectName(QStringLiteral("RecipeDisplayNoticeWord")); m_noteWord->setChecked(m_draft.noticeWord); grid->addWidget(m_noteBit, 0, 0); grid->addWidget(m_noteWord, 0, 1); m_bitFields = new QWidget(m_notice); fields = new QGridLayout(m_bitFields); bitAddress = new QLineEdit(m_draft.noticeBitAddress, m_bitFields); bitAddress->setObjectName(QStringLiteral("RecipeDisplayNoticeBitAddress")); bitAddress->setMaxLength(1024); bitPicker = new QPushButton(QStringLiteral("…"), m_bitFields); bitPicker->setFixedWidth(28); on = new QRadioButton(QStringLiteral("1"), m_bitFields); off = new QRadioButton(QStringLiteral("0"), m_bitFields); on->setObjectName(QStringLiteral("RecipeDisplayNoticeBitOn")); off->setObjectName(QStringLiteral("RecipeDisplayNoticeBitOff")); on->setChecked(m_draft.noticeBitValue); off->setChecked(!m_draft.noticeBitValue); fields->addWidget(new QLabel(QStringLiteral("地址:"), m_bitFields), 0, 0); fields->addWidget(bitAddress, 0, 1); fields->addWidget(bitPicker, 0, 2); fields->addWidget(new QLabel(QStringLiteral("状态:"), m_bitFields), 1, 0); fields->addWidget(on, 1, 1); fields->addWidget(off, 1, 2); grid->addWidget(m_bitFields, 1, 0);
    m_wordFields = new QWidget(m_notice); fields = new QGridLayout(m_wordFields); wordType = new QComboBox(m_wordFields); wordType->setObjectName(QStringLiteral("RecipeDisplayNoticeWordType")); wordType->addItems(StaticTextProperties::dataTypes()); wordType->setCurrentIndex(m_draft.noticeWordType); wordAddress = new QLineEdit(m_draft.noticeWordAddress, m_wordFields); wordAddress->setObjectName(QStringLiteral("RecipeDisplayNoticeWordAddress")); wordAddress->setMaxLength(1024); wordValue = new QLineEdit(m_draft.noticeWordValue, m_wordFields); wordValue->setObjectName(QStringLiteral("RecipeDisplayNoticeWordValue")); wordValue->setMaxLength(1024); wordPicker = new QPushButton(QStringLiteral("…"), m_wordFields); wordPicker->setFixedWidth(28); fields->addWidget(new QLabel(QStringLiteral("数据类型:"), m_wordFields), 0, 0); fields->addWidget(wordType, 0, 1, 1, 2); fields->addWidget(new QLabel(QStringLiteral("地址:"), m_wordFields), 1, 0); fields->addWidget(wordAddress, 1, 1); fields->addWidget(wordPicker, 1, 2); fields->addWidget(new QLabel(QStringLiteral("数值:"), m_wordFields), 2, 0); fields->addWidget(wordValue, 2, 1, 1, 2); grid->addWidget(m_wordFields, 1, 1); layout->addWidget(m_notice, 1, 0, 1, 2); layout->setRowStretch(2, 1);
    connect(m_notice, &QGroupBox::toggled, this, [this](bool value) { m_draft.notice = value; refreshLinkage(); }); connect(m_noteBit, &QCheckBox::toggled, this, [this](bool value) { m_draft.noticeBit = value; if (value) { m_noteWord->setChecked(false); } refreshLinkage(); }); connect(m_noteWord, &QCheckBox::toggled, this, [this](bool value) { m_draft.noticeWord = value; if (value) { m_noteBit->setChecked(false); } refreshLinkage(); }); connect(bitAddress, &QLineEdit::textChanged, this, [this](const QString &value) { m_draft.noticeBitAddress = value; }); connect(wordAddress, &QLineEdit::textChanged, this, [this](const QString &value) { m_draft.noticeWordAddress = value; }); connect(wordValue, &QLineEdit::textChanged, this, [this](const QString &value) { m_draft.noticeWordValue = value; }); connect(wordType, &QComboBox::currentIndexChanged, this, [this](int value) { m_draft.noticeWordType = value; }); connect(on, &QRadioButton::toggled, this, [this](bool value) { m_draft.noticeBitValue = value; }); connect(bitPicker, &QPushButton::clicked, this, [this, bitAddress]() { editHmiAddress(this, bitAddress, true); }); connect(wordPicker, &QPushButton::clicked, this, [this, wordAddress]() { editHmiAddress(this, wordAddress, false, m_draft.noticeWordType == 0 || m_draft.noticeWordType == 2 ? 1 : 2); }); return page;
}

/******************************************************************************
 * Function Name   : RecipeDisplayDialog::RecipeDisplayDialog
 * Description     : Build the measured two-page display editor with a detached recipe/keyboard draft and no fabricated preview widget.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
RecipeDisplayDialog::RecipeDisplayDialog(const HmiObject &object, const HmiProject &project, QWidget *parent)
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.recipeDisplay) //20261113-2 add
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QTabWidget *tabs = new QTabWidget(this);
    QDialogButtonBox *buttons;
    QJsonArray connections;
    PlcConnection connection;
    setObjectName(QStringLiteral("RecipeDisplayDialog")); setWindowTitle(QStringLiteral("配方显示器")); resize(614, 540); setFont(QFont(QStringLiteral("SimSun"), 9)); setStyleSheet(QStringLiteral("QLineEdit, QComboBox, QSpinBox {background:#caeace;} QLineEdit:disabled, QComboBox:disabled, QSpinBox:disabled {background:#b1ceed;}")); Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList()); tabs->setObjectName(QStringLiteral("RecipeDisplayTabs")); tabs->addTab(basicPage(), QStringLiteral("基本属性")); tabs->addTab(permissionPage(), QStringLiteral("权限设置")); layout->addWidget(tabs); buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("RecipeDisplayButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); layout->addWidget(buttons); connect(buttons, &QDialogButtonBox::accepted, this, &RecipeDisplayDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject); connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() { QMessageBox::information(this, QStringLiteral("配方显示器帮助"), QStringLiteral("先建立配方，再设置显示样式。编辑配方可使用真实自定义键盘，权限页设置显现、触控有效和通知。")); }); refreshLinkage();
}

/******************************************************************************
 * Function Name   : RecipeDisplayDialog::refreshLinkage
 * Description     : Show only observed active edit/keyboard, grid-color and notification subforms while retaining hidden value drafts.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void RecipeDisplayDialog::refreshLinkage() //20261113-2 add
{
    m_keyboardPanel->setVisible(m_draft.editEnabled); m_keyboardLabel->setVisible(m_draft.customKeyboard); m_keyboardPage->setVisible(m_draft.customKeyboard); m_position->setVisible(m_draft.customKeyboard); m_coordinates->setVisible(m_draft.customKeyboard && m_draft.keyboardPosition); m_gridColor->setVisible(m_draft.showGrid); m_gridLabel->setVisible(m_draft.showGrid); m_bitFields->setVisible(m_draft.notice && m_draft.noticeBit); m_wordFields->setVisible(m_draft.notice && m_draft.noticeWord);
}

/******************************************************************************
 * Function Name   : RecipeDisplayDialog::object
 * Description     : Return a complete distinct display type and normalize disabled notification selectors as observed on native reopen.
 * Input Parameters: None
 * Return Value    : Complete detached display draft.
 * Date            : 2026.11.13
 ******************************************************************************/
HmiObject RecipeDisplayDialog::object() const //20261113-2 add
{
    HmiObject object = m_original;
    object.type = QStringLiteral("recipe"); object.recipeDisplay = m_draft; object.address.clear(); object.text.clear(); object.hasBitButtonProperties = false; object.hasStaticTextProperties = false; object.hasNumberDisplayProperties = false; object.hasImageDisplayProperties = false; object.recipeDisplay.visibility = m_visibility->properties(); object.recipeDisplay.touch = m_touch->properties(); object.recipeDisplay.visibilityBitChosen = m_visibility->bitStateChosen() || m_accepted; object.recipeDisplay.touchBitChosen = m_touch->bitStateChosen() || m_accepted;
    if (!object.recipeDisplay.notice) { object.recipeDisplay.noticeBit = false; object.recipeDisplay.noticeWord = false; } return object;
}

/******************************************************************************
 * Function Name   : RecipeDisplayDialog::accept
 * Description     : Reject missing recipes, invalid active keyboard identities and malformed complete permission/notice spans before placement.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.13
 ******************************************************************************/
void RecipeDisplayDialog::accept() //20261113-2 add
{
    HmiObject candidate;
    QString error;
    m_accepted = true; candidate = object(); if (!candidate.recipeDisplay.validate(&error) || !candidate.recipeDisplay.validateAddresses(m_project.connections, &error) || !candidate.recipeDisplay.validateContext(m_project, &error)) { m_accepted = false; QMessageBox::warning(this, QStringLiteral("配方显示器"), error); return; } QDialog::accept();
}
