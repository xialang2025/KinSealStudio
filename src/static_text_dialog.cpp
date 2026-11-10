#include "static_text_dialog.h"
#include "static_text_renderer.h"
#include "hmi_property_widgets.h" //20261108-15 add
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QDoubleValidator>
#include <QFontDatabase>
#include <QFontComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextBrowser>
#include <QVBoxLayout>

//20261108-14 add: Render draft values without committing them to the canvas object.
class StaticTextPreview : public QWidget
{
public:
    HmiObject object;
    QString language;
    explicit StaticTextPreview(QWidget *parent) : QWidget(parent) { setMinimumSize(140, 140); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), palette().window());
        painter.setPen(palette().mid().color());
        painter.drawRect(rect().adjusted(0, 0, -1, -1));
        paintStaticText(&painter, QRectF(rect().center().x() - 50, rect().center().y() - 50, 100, 100),
                         staticTextContent(object, language), object.staticText);
    }
};

/******************************************************************************
 * Function Name   : StaticTextDialog::StaticTextDialog
 * Description     : Build the two native-style tabs and connect every draft field to live preview.
 * Input Parameters: object, languages, parent
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
StaticTextDialog::StaticTextDialog(const HmiObject &object, const QStringList &languages,
                                  QWidget *parent) //20261108-14 add
    : QDialog(parent), m_working(object)
{
    QGridLayout *layout;
    QTabWidget *tabs;
    QWidget *appearance;
    QWidget *permissions;
    QGridLayout *appearanceLayout;
    QVBoxLayout *permissionLayout;
    QGroupBox *flags;
    QGridLayout *flagLayout;
    QGroupBox *general;
    QFormLayout *generalLayout;
    QGroupBox *fontGroup;
    QFormLayout *fontLayout;
    QGroupBox *fill;
    QFormLayout *fillLayout;
    QGroupBox *styleGroup;
    QVBoxLayout *styleLayout;
    QGroupBox *previewGroup;
    QVBoxLayout *previewLayout;
    QVBoxLayout *visibilityLayout;
    QGridLayout *bitLayout;
    QHBoxLayout *levelLayout;
    QGridLayout *wordLayout;
    QRadioButton *bitOff;
    QPushButton *bitAddressButton;
    QPushButton *wordAddressButton;
    QDialogButtonBox *buttons;
    QStringList colorNames;
    QStringList availableLanguages;
    int index;
    int fontSize;
    QPixmap styleIcon;
    QPainter iconPainter;
    StaticTextProperties iconProperties;

    setWindowTitle(QStringLiteral("静态文本"));
    setObjectName(QStringLiteral("StaticTextDialog"));
    resize(820, 570);
    m_working.hasStaticTextProperties = true;
    availableLanguages = languages.isEmpty() ? QStringList{QStringLiteral("中文")} : languages;
    m_language = availableLanguages.first();
    if (!m_working.staticText.languageTexts.contains(m_language))
    {
        m_working.staticText.languageTexts.insert(m_language, object.text);
    }
    layout = new QGridLayout(this);
    layout->setContentsMargins(20, 20, 12, 12);
    tabs = new QTabWidget(this);
    tabs->setObjectName(QStringLiteral("StaticTextTabs"));
    appearance = new QWidget(tabs);
    permissions = new QWidget(tabs);
    tabs->addTab(appearance, QStringLiteral("外观设置"));
    tabs->addTab(permissions, QStringLiteral("权限设置"));
    layout->addWidget(tabs, 0, 0);
    appearanceLayout = new QGridLayout(appearance);
    appearanceLayout->setContentsMargins(20, 20, 70, 8);
    appearanceLayout->setHorizontalSpacing(38);
    m_text = new QPlainTextEdit(appearance);
    m_text->setObjectName(QStringLiteral("StaticTextInput"));
    m_text->setPlainText(m_working.staticText.languageTexts.value(m_language));
    m_text->setFixedHeight(120);
    appearanceLayout->addWidget(m_text, 0, 0, 1, 2);
    flags = new QGroupBox(appearance);
    flags->setFixedSize(220, 80);
    flagLayout = new QGridLayout(flags);
    m_italic = new QCheckBox(QStringLiteral("斜体"), flags);
    m_bold = new QCheckBox(QStringLiteral("粗体"), flags);
    m_adaptive = new QCheckBox(QStringLiteral("自适应"), flags);
    m_underline = new QCheckBox(QStringLiteral("下划线"), flags);
    m_italic->setChecked(object.staticText.italic);
    m_bold->setChecked(object.staticText.bold);
    m_adaptive->setChecked(object.staticText.adaptive);
    m_underline->setChecked(object.staticText.underline);
    m_adaptive->setObjectName(QStringLiteral("StaticTextAdaptive"));
    flagLayout->addWidget(m_italic, 0, 0);
    flagLayout->addWidget(m_bold, 0, 1);
    flagLayout->addWidget(m_adaptive, 1, 0);
    flagLayout->addWidget(m_underline, 1, 1);
    appearanceLayout->addWidget(flags, 1, 0);
    general = new QGroupBox(appearance);
    general->setFixedSize(250, 80);
    generalLayout = new QFormLayout(general);
    m_opacity = new QSpinBox(general);
    m_opacity->setRange(0, 255);
    m_opacity->setValue(object.staticText.opacity);
    m_opacity->setObjectName(QStringLiteral("StaticTextOpacity"));
    m_languages = new QComboBox(general);
    m_languages->addItems(availableLanguages);
    m_languages->setObjectName(QStringLiteral("StaticTextLanguage"));
    generalLayout->addRow(QStringLiteral("透明度:"), m_opacity);
    generalLayout->addRow(QStringLiteral("语言:"), m_languages);
    appearanceLayout->addWidget(general, 1, 1);
    fontGroup = new QGroupBox(QStringLiteral("属性"), appearance);
    fontGroup->setFixedSize(220, 190);
    fontLayout = new QFormLayout(fontGroup);
    m_font = new QFontComboBox(fontGroup);
    m_font->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_font->setMinimumContentsLength(8);
    m_font->setCurrentFont(QFont(object.staticText.fontFamily));
    m_font->setObjectName(QStringLiteral("StaticTextFont"));
    m_spacing = new QDoubleSpinBox(fontGroup);
    m_spacing->setRange(-4096, 4096);
    m_spacing->setValue(object.staticText.spacing);
    m_spacing->setObjectName(QStringLiteral("StaticTextSpacing"));
    m_size = new QComboBox(fontGroup);
    m_size->setEditable(true);
    Q_FOREACH (fontSize, QFontDatabase::standardSizes()) { m_size->addItem(QString::number(fontSize)); }
    m_size->setValidator(new QDoubleValidator(0.1, 4096, 2, m_size));
    m_size->setCurrentText(QString::number(object.staticText.fontSize));
    m_size->setObjectName(QStringLiteral("StaticTextSize"));
    m_alignment = new QComboBox(fontGroup);
    m_alignment->addItems({QStringLiteral("居中"), QStringLiteral("左对齐"), QStringLiteral("右对齐")});
    m_alignment->setCurrentIndex(object.staticText.alignment);
    m_alignment->setObjectName(QStringLiteral("StaticTextAlignment"));
    fontLayout->addRow(QStringLiteral("字体:"), m_font);
    fontLayout->addRow(QStringLiteral("字距:"), m_spacing);
    fontLayout->addRow(QStringLiteral("大小:"), m_size);
    fontLayout->addRow(QStringLiteral("对齐:"), m_alignment);
    appearanceLayout->addWidget(fontGroup, 2, 0, 2, 1);
    fill = new QGroupBox(QStringLiteral("填充"), appearance);
    fill->setFixedSize(250, 120);
    fillLayout = new QFormLayout(fill);
    colorNames = {QStringLiteral("颜色:"), QStringLiteral("背景色:"), QStringLiteral("前景色:")};
    m_colorValues[0] = object.staticText.textColor;
    m_colorValues[1] = object.staticText.backgroundColor;
    m_colorValues[2] = object.staticText.foregroundColor;
    for (index = 0; index < 3; ++index)
    {
        m_colors[index] = new QPushButton(fill);
        m_colors[index]->setMinimumHeight(24);
        m_colors[index]->setObjectName(QStringLiteral("StaticTextColor%1").arg(index));
        fillLayout->addRow(colorNames.at(index), m_colors[index]);
        connect(m_colors[index], &QPushButton::clicked, this, [this, index]() { chooseColor(index); });
    }
    appearanceLayout->addWidget(fill, 2, 1);
    styleGroup = new QGroupBox(QStringLiteral("样式"), appearance);
    styleGroup->setFixedSize(250, 60);
    styleLayout = new QVBoxLayout(styleGroup);
    m_style = new QComboBox(styleGroup);
    for (index = 0; index < StaticTextProperties::styles().size(); ++index)
    {
        styleIcon = QPixmap(18, 18); styleIcon.fill(Qt::white);
        iconProperties.style = index;
        iconProperties.backgroundColor = Qt::white; iconProperties.foregroundColor = Qt::darkGray;
        iconPainter.begin(&styleIcon);
        paintStaticText(&iconPainter, styleIcon.rect(), QString(), iconProperties);
        iconPainter.end();
        m_style->addItem(QIcon(styleIcon), StaticTextProperties::styles().at(index));
    }
    m_style->setCurrentIndex(object.staticText.style);
    m_style->setObjectName(QStringLiteral("StaticTextStyle"));
    styleLayout->addWidget(m_style);
    appearanceLayout->addWidget(styleGroup, 3, 1);
    appearanceLayout->setRowStretch(4, 1);
    permissionLayout = new QVBoxLayout(permissions);
    permissionLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_visibility = new QGroupBox(QStringLiteral("显现"), permissions);
    m_visibility->setCheckable(true);
    m_visibility->setChecked(object.staticText.visibilityEnabled);
    m_visibility->setObjectName(QStringLiteral("VisibilityEnabled"));
    m_visibility->setMaximumWidth(320);
    visibilityLayout = new QVBoxLayout(m_visibility);
    m_bit = new QGroupBox(QStringLiteral("受位控制"), m_visibility);
    m_bit->setCheckable(true);
    m_bit->setChecked(object.staticText.bitEnabled);
    m_bit->setObjectName(QStringLiteral("VisibilityBit"));
    bitLayout = new QGridLayout(m_bit);
    m_bitAddress = new QLineEdit(object.staticText.bitAddress, m_bit);
    m_bitAddress->setObjectName(QStringLiteral("VisibilityBitAddress"));
    bitAddressButton = new QPushButton(QStringLiteral("▦"), m_bit);
    bitAddressButton->setAccessibleName(QStringLiteral("位地址输入"));
    bitAddressButton->setObjectName(QStringLiteral("BitAddressPicker"));
    bitLayout->addWidget(new QLabel(QStringLiteral("地址"), m_bit), 0, 0);
    bitLayout->addWidget(m_bitAddress, 0, 1);
    bitLayout->addWidget(bitAddressButton, 0, 2);
    m_bitOn = new QRadioButton(QStringLiteral("1"), m_bit);
    bitOff = new QRadioButton(QStringLiteral("0"), m_bit);
    m_bitOn->setChecked(object.staticText.bitState);
    bitOff->setChecked(!object.staticText.bitState);
    bitLayout->addWidget(new QLabel(QStringLiteral("显现状态:"), m_bit), 1, 0);
    bitLayout->addWidget(m_bitOn, 1, 1);
    bitLayout->addWidget(bitOff, 1, 2);
    visibilityLayout->addWidget(m_bit);
    m_level = new QGroupBox(QStringLiteral("受用户等级控制"), m_visibility);
    m_level->setCheckable(true);
    m_level->setChecked(object.staticText.levelEnabled);
    m_level->setObjectName(QStringLiteral("VisibilityLevel"));
    levelLayout = new QHBoxLayout(m_level);
    m_minimumLevel = new QComboBox(m_level);
    for (index = 1; index <= 8; ++index)
    {
        m_minimumLevel->addItem(QString::number(index));
    }
    m_minimumLevel->setCurrentIndex(object.staticText.minimumLevel - 1);
    levelLayout->addWidget(new QLabel(QStringLiteral("有效最低等级"), m_level));
    levelLayout->addWidget(m_minimumLevel, 1);
    visibilityLayout->addWidget(m_level);
    m_word = new QGroupBox(QStringLiteral("字控制"), m_visibility);
    m_word->setCheckable(true);
    m_word->setChecked(object.staticText.wordEnabled);
    m_word->setObjectName(QStringLiteral("VisibilityWord"));
    wordLayout = new QGridLayout(m_word);
    m_dataType = new QComboBox(m_word);
    m_dataType->addItems(StaticTextProperties::dataTypes());
    m_dataType->setCurrentIndex(object.staticText.dataType);
    m_dataType->setObjectName(QStringLiteral("VisibilityDataType"));
    m_wordAddress = new QLineEdit(object.staticText.wordAddress, m_word);
    m_wordAddress->setObjectName(QStringLiteral("VisibilityWordAddress"));
    wordAddressButton = new QPushButton(QStringLiteral("▦"), m_word);
    wordAddressButton->setAccessibleName(QStringLiteral("字地址输入"));
    wordAddressButton->setObjectName(QStringLiteral("WordAddressPicker"));
    m_comparison = new QComboBox(m_word);
    m_comparison->addItems(StaticTextProperties::comparisons());
    m_comparison->setCurrentIndex(object.staticText.comparison);
    m_comparison->setObjectName(QStringLiteral("VisibilityComparison"));
    m_value = new QLineEdit(object.staticText.compareValue, m_word);
    m_value->setObjectName(QStringLiteral("VisibilityValue"));
    m_tolerance = new QLineEdit(object.staticText.tolerance, m_word);
    m_tolerance->setObjectName(QStringLiteral("VisibilityTolerance"));
    m_toleranceLabel = new QLabel(QStringLiteral("误差:"), m_word);
    wordLayout->addWidget(new QLabel(QStringLiteral("数据类型:"), m_word), 0, 0);
    wordLayout->addWidget(m_dataType, 0, 1, 1, 3);
    wordLayout->addWidget(m_wordAddress, 1, 0, 1, 2);
    wordLayout->addWidget(wordAddressButton, 1, 2);
    wordLayout->addWidget(m_comparison, 1, 3);
    wordLayout->addWidget(m_value, 2, 0, 1, 2);
    wordLayout->addWidget(m_toleranceLabel, 2, 2);
    wordLayout->addWidget(m_tolerance, 2, 3);
    visibilityLayout->addWidget(m_word);
    permissionLayout->addWidget(m_visibility);
    previewGroup = new QGroupBox(QStringLiteral("预览"), this);
    previewLayout = new QVBoxLayout(previewGroup);
    m_preview = new StaticTextPreview(previewGroup);
    m_preview->setObjectName(QStringLiteral("StaticTextPreview"));
    previewLayout->addWidget(m_preview);
    layout->addWidget(previewGroup, 0, 1, Qt::AlignTop);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助"));
    layout->addWidget(buttons, 1, 0, 1, 2);
    layout->setColumnStretch(0, 1);
    connect(buttons, &QDialogButtonBox::accepted, this, &StaticTextDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::helpRequested, this, [this]() {
        QMessageBox::information(this, QStringLiteral("静态文本帮助"),
            QStringLiteral("先配置文字、字体和填充，确定后在画布拖出文本框。双击控件重新编辑；取消会放弃本次修改。\n"
                           "自适应会按矩形缩放文字，禁用字号和对齐。显现页可独立配置位、用户等级和字条件。\n"
                           "设备端权限组合与地址编码仍须协议验证，启用显现条件的工程目前会在下载前明确拒绝。"));
    });
    connect(m_text, &QPlainTextEdit::textChanged, this, &StaticTextDialog::refresh);
    connect(m_font, &QFontComboBox::currentFontChanged, this, &StaticTextDialog::refresh);
    connect(m_size, &QComboBox::currentTextChanged, this, &StaticTextDialog::refresh);
    connect(m_spacing, &QDoubleSpinBox::valueChanged, this, &StaticTextDialog::refresh);
    connect(m_opacity, &QSpinBox::valueChanged, this, &StaticTextDialog::refresh);
    connect(m_style, &QComboBox::currentIndexChanged, this, &StaticTextDialog::refresh);
    connect(m_alignment, &QComboBox::currentIndexChanged, this, &StaticTextDialog::refresh);
    connect(m_comparison, &QComboBox::currentIndexChanged, this, &StaticTextDialog::refresh);
    connect(m_italic, &QCheckBox::toggled, this, &StaticTextDialog::refresh);
    connect(m_bold, &QCheckBox::toggled, this, &StaticTextDialog::refresh);
    connect(m_underline, &QCheckBox::toggled, this, &StaticTextDialog::refresh);
    connect(m_adaptive, &QCheckBox::toggled, this, &StaticTextDialog::refresh);
    connect(m_languages, &QComboBox::currentTextChanged, this, &StaticTextDialog::switchLanguage);
    connect(bitAddressButton, &QPushButton::clicked, this, [this]() { editAddress(m_bitAddress, true); });
    connect(wordAddressButton, &QPushButton::clicked, this, [this]() { editAddress(m_wordAddress, false); });
    refresh();
}

/******************************************************************************
 * Function Name   : StaticTextDialog::object
 * Description     : Read a complete draft record without mutating the caller's original object.
 * Input Parameters: None
 * Return Value    : Current dialog draft.
 * Date            : 2026.11.08
 ******************************************************************************/
HmiObject StaticTextDialog::object() const //20261108-14 add
{
    HmiObject result = m_working;
    StaticTextProperties *properties = &result.staticText;

    properties->fontFamily = m_font->currentFont().family();
    properties->fontSize = m_size->currentText().toDouble();
    properties->spacing = m_spacing->value();
    properties->alignment = m_alignment->currentIndex();
    properties->italic = m_italic->isChecked();
    properties->bold = m_bold->isChecked();
    properties->adaptive = m_adaptive->isChecked();
    properties->underline = m_underline->isChecked();
    properties->opacity = m_opacity->value();
    properties->style = m_style->currentIndex();
    properties->textColor = m_colorValues[0];
    properties->backgroundColor = m_colorValues[1];
    properties->foregroundColor = m_colorValues[2];
    properties->languageTexts.insert(m_language, m_text->toPlainText());
    result.text = properties->languageTexts.value(m_languages->itemText(0), result.text);
    properties->visibilityEnabled = m_visibility->isChecked();
    properties->bitEnabled = m_bit->isChecked();
    properties->bitAddress = m_bitAddress->text().trimmed();
    properties->bitState = m_bitOn->isChecked();
    properties->levelEnabled = m_level->isChecked();
    properties->minimumLevel = m_minimumLevel->currentIndex() + 1;
    properties->wordEnabled = m_word->isChecked();
    properties->dataType = m_dataType->currentIndex();
    properties->wordAddress = m_wordAddress->text().trimmed();
    properties->comparison = m_comparison->currentIndex();
    properties->compareValue = m_value->text();
    properties->tolerance = m_tolerance->text();
    return result;
}

/******************************************************************************
 * Function Name   : StaticTextDialog::refresh
 * Description     : Keep adaptive dependencies, equality error fields and draft preview synchronized.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextDialog::refresh() //20261108-14 add
{
    int index;
    bool equality;

    m_size->setEnabled(!m_adaptive->isChecked());
    m_alignment->setEnabled(!m_adaptive->isChecked());
    equality = m_comparison->currentIndex() == 2 || m_comparison->currentIndex() == 3;
    m_toleranceLabel->setVisible(equality);
    m_tolerance->setVisible(equality);
    for (index = 0; index < 3; ++index)
    {
        m_colors[index]->setStyleSheet(QStringLiteral("background-color:%1;border:1px solid #777;").arg(m_colorValues[index].name()));
    }
    m_preview->object = object();
    m_preview->language = m_language;
    m_preview->update();
}

/******************************************************************************
 * Function Name   : StaticTextDialog::switchLanguage
 * Description     : Preserve the previous language draft before loading the next translation.
 * Input Parameters: language
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextDialog::switchLanguage(const QString &language) //20261108-14 add
{
    m_working.staticText.languageTexts.insert(m_language, m_text->toPlainText());
    m_language = language;
    m_text->setPlainText(m_working.staticText.languageTexts.value(language));
    refresh();
}

/******************************************************************************
 * Function Name   : StaticTextDialog::chooseColor
 * Description     : Commit a color only when the standard full-featured color dialog is accepted.
 * Input Parameters: index
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextDialog::chooseColor(int index) //20261108-14 add
{
    QColor color;

    color = QColorDialog::getColor(m_colorValues[index], this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog);
    if (color.isValid())
    {
        m_colorValues[index] = color;
        refresh();
    }
}

/******************************************************************************
 * Function Name   : StaticTextDialog::editAddress
 * Description     : Provide the observed internal LB/LW range and decimal/hex keypad without inventing PLC families.
 * Input Parameters: field, bit
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextDialog::editAddress(QLineEdit *field, bool bit) //20261108-14 add
{
    editHmiAddress(this, field, bit); //20261108-15 change: Shared by static text and bit buttons.
}

/******************************************************************************
 * Function Name   : StaticTextDialog::accept
 * Description     : Reject invalid drafts and commit only through the caller after an accepted result.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextDialog::accept() //20261108-14 add
{
    QString error;

    if (!object().staticText.validate(&error))
    {
        QMessageBox::warning(this, QStringLiteral("属性无效"), error);
        return;
    }
    QDialog::accept();
}
