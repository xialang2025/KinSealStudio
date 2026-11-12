#include "number_display_dialog.h"
#include "bit_button_dialog.h"
#include "control_rendering.h"
#include "hmi_property_widgets.h"
#include "kvg_library.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
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
#include <QScrollArea>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : paintNumberDisplay
 * Description     : Paint the editor's measured digit/address sample and selected artwork, without inventing a PLC runtime value.
 * Input Parameters: painter, rect, properties, sample, ascii, wrap
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintNumberDisplay(QPainter *painter, const QRectF &rect, const NumberDisplayProperties &properties, const QString &sample, bool ascii, bool wrap) //20261112-0 change
{
    QImage image;
    QColor background(properties.backgroundColor);
    QFont font(properties.font == 1 ? QStringLiteral("DigifaceWide") : properties.font == 2 ? QStringLiteral("方正中等线简体") : QStringLiteral("SimSun"));
    QString text = sample.isNull() ? QString(properties.totalDigits, QLatin1Char('8')) : sample;
    QRectF numberRect = rect;
    int alignment = properties.alignment == 1 ? Qt::AlignLeft : properties.alignment == 2 ? Qt::AlignRight : Qt::AlignHCenter;
    painter->save(); painter->setClipRect(rect, Qt::IntersectClip);
    if (!properties.appearance.picturePng.isEmpty())
    { image = decodedControlPng(properties.appearance.picturePng); if (!image.isNull()) { painter->drawImage(rect, image); } }
    else if (!properties.appearance.picture.isEmpty()) { paintKvgPicture(painter, rect, properties.appearance.picture, 0); }
    else if (!properties.transparent) { background.setAlpha(properties.opacity); painter->fillRect(rect, background); }
    if (properties.effect3d)
    {
        painter->setPen(Qt::white); painter->drawLine(rect.topLeft(), rect.topRight()); painter->drawLine(rect.topLeft(), rect.bottomLeft());
        painter->setPen(Qt::darkGray); painter->drawLine(rect.bottomLeft(), rect.bottomRight()); painter->drawLine(rect.topRight(), rect.bottomRight());
    }
    if (!ascii && properties.decimalDigits > 0 && NumberDisplayProperties::decimalEnabled(properties.dataType))
    { text = text.rightJustified(properties.decimalDigits + 1, QLatin1Char('0')); text.insert(text.size() - properties.decimalDigits, QLatin1Char('.')); }
    if (properties.password && properties.allowInput) { text.fill(QLatin1Char('*')); }
    if (properties.inputMarker && properties.allowInput) { text = QStringLiteral("input"); }
    if (!ascii && properties.customDisplay) { text = properties.customText; }
    font.setPixelSize(properties.fontSize); painter->setFont(font); painter->setPen(QColor(properties.textColor));
    if (properties.displayAddress) { numberRect.setHeight(rect.height() * 0.62); }
    painter->drawText(numberRect.adjusted(2, 1, -2, -1), alignment | Qt::AlignVCenter | (wrap ? Qt::TextWrapAnywhere : 0), text);
    if (properties.displayAddress)
    { font.setFamily(QStringLiteral("SimSun")); font.setPixelSize(12); painter->setFont(font); painter->drawText(QRectF(rect.left(), numberRect.bottom(), rect.width(), rect.height() - numberRect.height()), Qt::AlignCenter, properties.monitorAddress); }
    painter->restore();
}

/******************************************************************************
 * Function Name   : paintAsciiDisplay
 * Description     : Render a bounded editor alphabet sample; byte swapping and GB2312 flags never pretend to decode unobserved PLC bytes.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintAsciiDisplay(QPainter *painter, const QRectF &rect, const AsciiDisplayProperties &properties) //20261112-0 add
{
    QString sample;
    int index;
    sample.reserve(properties.characterCount);
    for (index = 0; index < properties.characterCount; ++index) { sample.append(QChar(u16('A' + index % 26))); }
    paintNumberDisplay(painter, rect, properties.common, sample, true, properties.autoWrap);
}

/******************************************************************************
 * Function Name   : paintTimeDisplay
 * Description     : Paint measured clock format tokens without binding them to a fabricated PLC carrier or host clock.
 * Input Parameters: painter, rect, properties
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void paintTimeDisplay(QPainter *painter, const QRectF &rect, const TimeDisplayProperties &properties) //20261112-0 add
{
    NumberDisplayProperties visual;
    visual.allowInput = false; visual.displayAddress = false; visual.fontSize = properties.fontSize; visual.alignment = properties.alignment; visual.opacity = properties.opacity;
    visual.textColor = properties.textColor; visual.backgroundColor = properties.backgroundColor; visual.effect3d = properties.effect3d; visual.transparent = properties.transparent; visual.appearance = properties.appearance;
    paintNumberDisplay(painter, rect, visual, properties.sample(), true);
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::field
 * Description     : Read the actual typed widget value while retaining fields not represented by an active editor.
 * Input Parameters: key
 * Return Value    : Boolean, integer or text field value.
 * Date            : 2026.11.12
 ******************************************************************************/
QJsonValue NumberDisplayDialog::field(const QString &key) const //20261112-0 add
{
    QWidget *widget = m_fields.value(key);
    QCheckBox *checkbox = qobject_cast<QCheckBox *>(widget);
    QRadioButton *radio = qobject_cast<QRadioButton *>(widget);
    QGroupBox *box = qobject_cast<QGroupBox *>(widget);
    QComboBox *selection = qobject_cast<QComboBox *>(widget);
    QSpinBox *number = qobject_cast<QSpinBox *>(widget);
    QLineEdit *text = qobject_cast<QLineEdit *>(widget);
    if (checkbox != nullptr) { return checkbox->isChecked(); }
    if (radio != nullptr) { return radio->isChecked(); }
    if (box != nullptr) { return box->isChecked(); }
    if (selection != nullptr) { return key == QStringLiteral("keyboardPageId") ? QJsonValue(selection->currentIndex() < 0 ? m_draft.keyboardPageId : selection->currentData().toString()) : QJsonValue(selection->currentData().toInt()); }
    if (number != nullptr) { return number->value(); }
    if (text != nullptr) { return text->text(); }
    return m_draft.toJson().value(key);
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::check
 * Description     : Bind a boolean editor and its linkage signal to a named typed field.
 * Input Parameters: key, label, parent
 * Return Value    : Bound checkbox.
 * Date            : 2026.11.12
 ******************************************************************************/
QCheckBox *NumberDisplayDialog::check(const QString &key, const QString &label, QWidget *parent) //20261112-0 add
{
    QCheckBox *widget = new QCheckBox(label, parent);
    widget->setObjectName(QStringLiteral("Number_") + key); widget->setChecked(m_draft.toJson().value(key).toBool()); m_fields.insert(key, widget);
    connect(widget, &QCheckBox::toggled, this, [this]() { updateLinkage(); }); return widget;
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::group
 * Description     : Bind a checkable native group while allowing disabled child drafts to survive.
 * Input Parameters: key, label, parent
 * Return Value    : Bound checkable group.
 * Date            : 2026.11.12
 ******************************************************************************/
QGroupBox *NumberDisplayDialog::group(const QString &key, const QString &label, QWidget *parent) //20261112-0 add
{
    QGroupBox *widget = new QGroupBox(label, parent);
    widget->setObjectName(QStringLiteral("Number_") + key); widget->setCheckable(true); widget->setChecked(m_draft.toJson().value(key).toBool()); m_fields.insert(key, widget);
    connect(widget, &QGroupBox::toggled, this, [this]() { updateLinkage(); }); return widget;
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::line
 * Description     : Bind an exact textual constant or address without coercing hidden values through numeric spin boxes.
 * Input Parameters: key, parent
 * Return Value    : Bound text field.
 * Date            : 2026.11.12
 ******************************************************************************/
QLineEdit *NumberDisplayDialog::line(const QString &key, QWidget *parent) //20261112-0 add
{
    QLineEdit *widget = new QLineEdit(m_draft.toJson().value(key).toString(), parent);
    widget->setObjectName(QStringLiteral("Number_") + key); m_fields.insert(key, widget); connect(widget, &QLineEdit::textChanged, this, [this]() { updateLinkage(); }); return widget;
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::combo
 * Description     : Bind a measured option list, keeping semantic item values independent of displayed indices.
 * Input Parameters: key, items, parent, numericItems
 * Return Value    : Bound selection.
 * Date            : 2026.11.12
 ******************************************************************************/
QComboBox *NumberDisplayDialog::combo(const QString &key, const QStringList &items, QWidget *parent, bool numericItems) //20261112-0 add
{
    QComboBox *widget = new QComboBox(parent);
    int index;
    widget->setObjectName(QStringLiteral("Number_") + key);
    for (index = 0; index < items.size(); ++index) { widget->addItem(items.at(index), numericItems ? items.at(index).toInt() : index); }
    widget->setCurrentIndex(widget->findData(m_draft.toJson().value(key).toInt())); m_fields.insert(key, widget);
    connect(widget, &QComboBox::currentIndexChanged, this, [this]() { updateLinkage(); }); return widget;
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::spin
 * Description     : Bind a measured bounded integer editor without silently expanding native digit and position limits.
 * Input Parameters: key, minimum, maximum, parent
 * Return Value    : Bound integer editor.
 * Date            : 2026.11.12
 ******************************************************************************/
QSpinBox *NumberDisplayDialog::spin(const QString &key, int minimum, int maximum, QWidget *parent) //20261112-0 add
{
    QSpinBox *widget = new QSpinBox(parent);
    widget->setObjectName(QStringLiteral("Number_") + key); widget->setRange(minimum, maximum); widget->setValue(m_draft.toJson().value(key).toInt()); m_fields.insert(key, widget);
    connect(widget, &QSpinBox::valueChanged, this, [this]() { updateLinkage(); }); return widget;
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::address
 * Description     : Pair a textual carrier with the existing role/span-aware address picker.
 * Input Parameters: key, bit, parent
 * Return Value    : Address row widget.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *NumberDisplayDialog::address(const QString &key, bool bit, QWidget *parent) //20261112-0 add
{
    QWidget *row = new QWidget(parent);
    QHBoxLayout *layout = new QHBoxLayout(row);
    QLineEdit *text = line(key, row);
    QPushButton *picker = new QPushButton(QStringLiteral("…"), row);
    layout->setContentsMargins(0, 0, 0, 0); picker->setObjectName(QStringLiteral("Number_") + key + QStringLiteral("Picker")); picker->setFixedWidth(26); layout->addWidget(text); layout->addWidget(picker); m_addressRows.insert(key, row);
    connect(picker, &QPushButton::clicked, this, [this, text, bit, key]() {
        NumberDisplayProperties draft = object().numberDisplay;
        int span = key == QStringLiteral("offsetAddress") ? 1 : key == QStringLiteral("noticeWordAddress") ? (draft.noticeWordType == 0 || draft.noticeWordType == 2 ? 1 : 2) : m_asciiMode ? 1 : NumberDisplayProperties::wordSpan(draft.dataType); //20261112-0 change: ASCII only exposes the proven base carrier until its complete string span is recovered.
        editHmiAddress(this, text, bit, bit ? 1 : span);
    }); return row;
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::range
 * Description     : Build independent constant/address input and display ranges; toggling sources retains both drafts.
 * Input Parameters: prefix, parent
 * Return Value    : Range form.
 * Date            : 2026.11.12
 ******************************************************************************/
QWidget *NumberDisplayDialog::range(const QString &prefix, QWidget *parent) //20261112-0 add
{
    QWidget *widget = new QWidget(parent);
    QFormLayout *layout = new QFormLayout(widget);
    QWidget *row;
    QHBoxLayout *rowLayout;
    QString key;
    int index;
    layout->setContentsMargins(0, 0, 0, 0); layout->addRow(QStringLiteral("范围类型:"), combo(prefix + QStringLiteral("RangeSource"), {QStringLiteral("常量"), QStringLiteral("地址")}, widget));
    for (index = 0; index < 2; ++index)
    {
        key = prefix + (index == 0 ? QStringLiteral("Minimum") : QStringLiteral("Maximum")); row = new QWidget(widget); rowLayout = new QHBoxLayout(row); rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->addWidget(line(key, row)); rowLayout->addWidget(address(key + QStringLiteral("Address"), false, row)); layout->addRow(index == 0 ? QStringLiteral("最小值:") : QStringLiteral("最大值:"), row);
    }
    return widget;
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::NumberDisplayDialog
 * Description     : Build numeric basic/display/security/other pages from the measured native field set and preserve gallery drafts.
 * Input Parameters: object, project, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
NumberDisplayDialog::NumberDisplayDialog(const HmiObject &object, const HmiProject &project, QWidget *parent) //20261112-0 add
    : QDialog(parent), m_original(object), m_project(project), m_draft(object.type == QStringLiteral("ascii") ? object.asciiDisplay.common : object.numberDisplay), m_asciiDraft(object.asciiDisplay), m_timeDraft(object.timeDisplay), m_asciiMode(object.type == QStringLiteral("ascii")), m_timeMode(object.type == QStringLiteral("time")) //20261112-0 change
{
    QVBoxLayout *root;
    QHBoxLayout *content;
    QTabWidget *tabs;
    QWidget *page;
    QVBoxLayout *layout;
    QHBoxLayout *row;
    QFormLayout *form;
    QGroupBox *box;
    QGroupBox *macro;
    QRadioButton *type;
    QRadioButton *noticeOn;
    QRadioButton *noticeOff;
    QHBoxLayout *noticeStateLayout;
    QComboBox *dataType;
    QComboBox *keyboard;
    QSpinBox *digits;
    QSpinBox *characterCount;
    QLineEdit *colorText;
    QPushButton *button;
    QHBoxLayout *columns;
    QDialogButtonBox *buttons;
    QGroupBox *preview;
    QVBoxLayout *previewLayout;
    HmiPage keyboardPage;
    QString key;
    QStringList names;
    QJsonArray connections;
    PlcConnection connection;
    int index;
    if (m_timeMode) //20261112-0 add: Seed only shared visual fields; clock records contain no numeric address or input draft.
    {
        m_draft.fontSize = 16; m_draft.alignment = m_timeDraft.alignment; m_draft.opacity = m_timeDraft.opacity; m_draft.textColor = m_timeDraft.textColor; m_draft.backgroundColor = m_timeDraft.backgroundColor;
        m_draft.effect3d = m_timeDraft.effect3d; m_draft.transparent = m_timeDraft.transparent; m_draft.appearance = m_timeDraft.appearance; m_draft.visibility = m_timeDraft.visibility;
    }
    setObjectName(QStringLiteral("NumberDisplayDialog")); resize(820, 650); setFont(QFont(QStringLiteral("SimSun"), 9));
    setStyleSheet(QStringLiteral("QLineEdit,QComboBox,QSpinBox {background:#caeace;} QLineEdit:disabled,QComboBox:disabled,QSpinBox:disabled {background:#d0e9f4;}"));
    Q_FOREACH (connection, project.connections) { connections.append(connection.toJson()); } setProperty("plcConnections", connections.toVariantList());
    root = new QVBoxLayout(this); content = new QHBoxLayout(); tabs = new QTabWidget(this); tabs->setObjectName(QStringLiteral("NumberTabs")); content->addWidget(tabs, 1);
    page = new QWidget(tabs); layout = new QVBoxLayout(page); layout->setSpacing(6); row = new QHBoxLayout(); names = {QStringLiteral("数值输入/显示"), QStringLiteral("ASCII输入/显示"), QStringLiteral("日期/时间显示")};
    for (index = 0; index < 3; ++index)
    {
        type = new QRadioButton(names.at(index), page); type->setObjectName(QStringLiteral("NumberFamily%1").arg(index)); type->setChecked(index == (m_timeMode ? 2 : m_asciiMode ? 1 : 0)); row->addWidget(type);
        connect(type, &QRadioButton::toggled, this, [this, index](bool checked) { if (checked) { m_asciiMode = index == 1; m_timeMode = index == 2; updateLinkage(); } }); //20261112-0 change: Share the native family shell while retaining distinct typed records.
    }
    layout->addLayout(row); m_dataControls = new QWidget(page); form = new QFormLayout(m_dataControls); dataType = combo(QStringLiteral("dataType"), NumberDisplayProperties::dataTypes(), page); form->addRow(QStringLiteral("数据类型:"), dataType); m_dataTypeLabel = form->labelForField(dataType); form->addRow(QStringLiteral("监控地址:"), address(QStringLiteral("monitorAddress"), false, page));
    row = new QHBoxLayout(); row->addWidget(check(QStringLiteral("offsetEnabled"), QStringLiteral("偏移地址"), page)); row->addWidget(address(QStringLiteral("offsetAddress"), false, page)); row->addWidget(check(QStringLiteral("displayAddress"), QStringLiteral("显示监控地址"), page)); form->addRow(row);
    row = new QHBoxLayout(); row->addWidget(check(QStringLiteral("allowInput"), QStringLiteral("允许输入"), page)); row->addWidget(check(QStringLiteral("sameMonitor"), QStringLiteral("与监视地址相同"), page)); row->addWidget(check(QStringLiteral("password"), QStringLiteral("密码"), page)); row->addWidget(check(QStringLiteral("inputMarker"), QStringLiteral("是否显示输入标志"), page)); form->addRow(row); form->addRow(QStringLiteral("输入地址:"), address(QStringLiteral("inputAddress"), false, page)); layout->addWidget(m_dataControls);
    m_asciiControls = new QWidget(page); form = new QFormLayout(m_asciiControls); row = new QHBoxLayout(); row->addWidget(check(QStringLiteral("asciiAutoWrap"), QStringLiteral("自动换行"), m_asciiControls)); row->addWidget(check(QStringLiteral("asciiSwapBytes"), QStringLiteral("高低字节互换"), m_asciiControls)); form->addRow(row); form->addRow(QStringLiteral("编码方式:"), combo(QStringLiteral("asciiEncoding"), {QStringLiteral("ASCII"), QStringLiteral("GB2312")}, m_asciiControls)); layout->addWidget(m_asciiControls);
    qobject_cast<QCheckBox *>(m_fields.value(QStringLiteral("asciiAutoWrap")))->setChecked(m_asciiDraft.autoWrap); qobject_cast<QCheckBox *>(m_fields.value(QStringLiteral("asciiSwapBytes")))->setChecked(m_asciiDraft.swapBytes); qobject_cast<QComboBox *>(m_fields.value(QStringLiteral("asciiEncoding")))->setCurrentIndex(m_asciiDraft.encoding);
    m_timeControls = new QGroupBox(QStringLiteral("监控日期/时间"), page); m_timeControls->setObjectName(QStringLiteral("TimeControls")); form = new QFormLayout(m_timeControls);
    row = new QHBoxLayout(); row->addWidget(check(QStringLiteral("timeShowDate"), QStringLiteral("日期"), m_timeControls)); row->addWidget(combo(QStringLiteral("timeDateFormat"), TimeDisplayProperties::dateFormats(), m_timeControls)); form->addRow(row);
    form->addRow(check(QStringLiteral("timeShowWeek"), QStringLiteral("星期"), m_timeControls)); row = new QHBoxLayout(); row->addWidget(check(QStringLiteral("timeShowTime"), QStringLiteral("时间"), m_timeControls)); row->addWidget(combo(QStringLiteral("timeTimeFormat"), TimeDisplayProperties::timeFormats(), m_timeControls)); form->addRow(row);
    m_timeSample = new QLabel(m_timeDraft.sample(), m_timeControls); m_timeSample->setObjectName(QStringLiteral("TimeFormatSample")); m_timeSample->setStyleSheet(QStringLiteral("background:#ffb64b;")); form->addRow(QStringLiteral("预览:"), m_timeSample); layout->addWidget(m_timeControls);
    qobject_cast<QCheckBox *>(m_fields.value(QStringLiteral("timeShowDate")))->setChecked(m_timeDraft.showDate); qobject_cast<QCheckBox *>(m_fields.value(QStringLiteral("timeShowWeek")))->setChecked(m_timeDraft.showWeek); qobject_cast<QCheckBox *>(m_fields.value(QStringLiteral("timeShowTime")))->setChecked(m_timeDraft.showTime);
    qobject_cast<QComboBox *>(m_fields.value(QStringLiteral("timeDateFormat")))->setCurrentIndex(m_timeDraft.dateFormat); qobject_cast<QComboBox *>(m_fields.value(QStringLiteral("timeTimeFormat")))->setCurrentIndex(m_timeDraft.timeFormat);
    for (index = 0; index < 3; ++index)
    {
        key = QStringList{QStringLiteral("timeShowDate"), QStringLiteral("timeShowWeek"), QStringLiteral("timeShowTime")}.at(index);
        connect(qobject_cast<QCheckBox *>(m_fields.value(key)), &QCheckBox::toggled, this, [this]() { if (m_ready && m_timeMode && !field(QStringLiteral("timeShowDate")).toBool() && !field(QStringLiteral("timeShowWeek")).toBool() && !field(QStringLiteral("timeShowTime")).toBool()) { QMessageBox::information(this, QStringLiteral("KinSealStudio"), QStringLiteral("不能隐藏所有时间和星期")); } }); //20261112-0 add: Evidence observes all three unchecked while the immediate message remains open.
    }
    m_inputRange = new QGroupBox(QStringLiteral("数值输入范围"), page); form = new QFormLayout(m_inputRange); form->addRow(range(QStringLiteral("input"), m_inputRange)); layout->addWidget(m_inputRange);
    box = group(QStringLiteral("scaling"), QStringLiteral("缩放设置"), page); form = new QFormLayout(box); row = new QHBoxLayout(); row->addWidget(new QLabel(QStringLiteral("增益:"), box)); row->addWidget(line(QStringLiteral("gain"), box)); row->addWidget(new QLabel(QStringLiteral("偏移量:"), box)); row->addWidget(line(QStringLiteral("offset"), box)); form->addRow(row); layout->addWidget(box);
    macro = group(QStringLiteral("macroEnabled"), QStringLiteral("宏指令"), page); row = new QHBoxLayout(macro); row->addWidget(new QLabel(QStringLiteral("使用宏:"), macro)); row->addWidget(line(QStringLiteral("macroName"), macro)); row->addWidget(new QPushButton(QStringLiteral("写宏"), macro)); macro->setEnabled(false); layout->addWidget(macro); layout->addStretch(); tabs->addTab(page, QStringLiteral("基本"));
    page = new QWidget(tabs); layout = new QVBoxLayout(page); m_numberFontControls = new QWidget(page); form = new QFormLayout(m_numberFontControls); form->addRow(QStringLiteral("字体:"), combo(QStringLiteral("font"), {QStringLiteral("一般字体"), QStringLiteral("数码字体"), QStringLiteral("方正中等线简体")}, page)); form->addRow(QStringLiteral("字体大小:"), combo(QStringLiteral("fontSize"), {QStringLiteral("16"), QStringLiteral("24"), QStringLiteral("32"), QStringLiteral("40"), QStringLiteral("48"), QStringLiteral("64"), QStringLiteral("96"), QStringLiteral("192")}, page, true));
    layout->addWidget(m_numberFontControls); m_timeFontControls = new QWidget(page); form = new QFormLayout(m_timeFontControls); form->addRow(QStringLiteral("字体:"), combo(QStringLiteral("timeFont"), {QStringLiteral("汉字库字体")}, m_timeFontControls)); form->addRow(QStringLiteral("字体大小:"), combo(QStringLiteral("timeFontSize"), TimeDisplayProperties::fontSizes(), m_timeFontControls, true)); qobject_cast<QComboBox *>(m_fields.value(QStringLiteral("timeFontSize")))->setCurrentIndex(TimeDisplayProperties::fontSizes().indexOf(QString::number(m_timeDraft.fontSize))); qobject_cast<QComboBox *>(m_fields.value(QStringLiteral("timeFont")))->setCurrentIndex(0); layout->addWidget(m_timeFontControls); form = new QFormLayout();
    m_timeTotal = spin(QStringLiteral("timeTotalDigits"), 1, 1, page); m_timeTotal->setEnabled(false); form->addRow(QStringLiteral("总位数:"), m_timeTotal); m_timeTotalLabel = form->labelForField(m_timeTotal);
    digits = spin(QStringLiteral("totalDigits"), 1, NumberDisplayProperties::maximumDigits(m_draft.dataType), page); form->addRow(QStringLiteral("总位数:"), digits); m_totalDigitsLabel = form->labelForField(digits);
    characterCount = spin(QStringLiteral("asciiCharacterCount"), 1, 256, page); characterCount->setValue(m_asciiDraft.characterCount); form->addRow(QStringLiteral("显示字符数:"), characterCount); m_characterCountLabel = form->labelForField(characterCount);
    form->addRow(QStringLiteral("小数位数:"), spin(QStringLiteral("decimalDigits"), 0, 99, page)); form->addRow(QStringLiteral("样式:"), combo(QStringLiteral("alignment"), {QStringLiteral("居中"), QStringLiteral("左对齐"), QStringLiteral("右对齐")}, page)); form->addRow(QStringLiteral("调整:"), combo(QStringLiteral("adjustment"), {QStringLiteral("前面去0"), QStringLiteral("0领头"), QStringLiteral("空格领头")}, page));
    row = new QHBoxLayout(); row->addWidget(check(QStringLiteral("customDisplay"), QStringLiteral("自定义显示"), page)); row->addWidget(line(QStringLiteral("customText"), page)); form->addRow(row); layout->addLayout(form);
    box = new QGroupBox(QStringLiteral("颜色设置"), page); form = new QFormLayout(box); names = {QStringLiteral("textColor"), QStringLiteral("backgroundColor"), QStringLiteral("lowColor"), QStringLiteral("highColor")};
    for (index = 0; index < names.size(); ++index)
    {
        key = names.at(index); row = new QHBoxLayout(); colorText = line(key, box); colorText->setReadOnly(true); button = new QPushButton(QStringLiteral("选择颜色"), box); button->setObjectName(QStringLiteral("Number_") + key + QStringLiteral("Picker")); row->addWidget(colorText); row->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, colorText]() { QColor color = QColorDialog::getColor(QColor(colorText->text()), this, QStringLiteral("选择颜色")); if (color.isValid()) { colorText->setText(color.name()); } });
        form->addRow(QStringList{QStringLiteral("数值颜色:"), QStringLiteral("数值背景颜色:"), QStringLiteral("低位背景色:"), QStringLiteral("高位背景色:")}.at(index), row);
        m_colorLabels.insert(key, qobject_cast<QLabel *>(form->labelForField(row))); m_colorLabels.value(key)->setObjectName(QStringLiteral("Number_") + key + QStringLiteral("Label")); //20261112-0 add
    }
    form->addRow(QStringLiteral("透明度:"), spin(QStringLiteral("opacity"), 0, 255, box)); layout->addWidget(box);
    row = new QHBoxLayout(); names = {QStringLiteral("系统图库"), QStringLiteral("工程图库"), QStringLiteral("删除图片")};
    for (index = 0; index < 3; ++index)
    {
        button = new QPushButton(names.at(index), page); button->setObjectName(QStringLiteral("NumberPicture%1").arg(index)); row->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, index]() { if (index == 2) { m_draft.appearance = ImageDisplayState(); updateLinkage(); } else { choosePicture(index == 1); } });
    }
    layout->addLayout(row); box = group(QStringLiteral("displayRange"), QStringLiteral("范围显示设置"), page); form = new QFormLayout(box); form->addRow(range(QStringLiteral("display"), box)); layout->addWidget(box);
    row = new QHBoxLayout(); row->addWidget(check(QStringLiteral("effect3d"), QStringLiteral("终端3D按钮效果"), page)); row->addWidget(check(QStringLiteral("transparent"), QStringLiteral("透明效果"), page)); layout->addLayout(row); layout->addStretch(); tabs->addTab(page, QStringLiteral("显示"));
    page = new QWidget(tabs); layout = new QVBoxLayout(page); columns = new QHBoxLayout(); m_visibility = new HmiPermissionEditor(QStringLiteral("显现"), QStringLiteral("NumberVisibility"), m_draft.visibility, false, page); m_touch = new HmiPermissionEditor(QStringLiteral("触控有效"), QStringLiteral("NumberTouch"), m_draft.touch, true, page); columns->addWidget(m_visibility); columns->addWidget(m_touch); layout->addLayout(columns);
    box = group(QStringLiteral("notice"), QStringLiteral("通知"), page); form = new QFormLayout(box); row = new QHBoxLayout(); row->addWidget(check(QStringLiteral("noticeBit"), QStringLiteral("通知位"), box)); row->addWidget(address(QStringLiteral("noticeBitAddress"), true, box)); form->addRow(row);
    m_noticeBitStates = new QWidget(box); noticeStateLayout = new QHBoxLayout(m_noticeBitStates); noticeStateLayout->addWidget(new QLabel(QStringLiteral("状态:"), m_noticeBitStates));
    noticeOn = new QRadioButton(QStringLiteral("1"), m_noticeBitStates); noticeOff = new QRadioButton(QStringLiteral("0"), m_noticeBitStates); noticeOn->setObjectName(QStringLiteral("Number_noticeBitValue")); noticeOff->setObjectName(QStringLiteral("Number_noticeBitZero")); noticeOn->setChecked(m_draft.noticeBitValue); noticeOff->setChecked(!m_draft.noticeBitValue); m_fields.insert(QStringLiteral("noticeBitValue"), noticeOn); noticeStateLayout->addWidget(noticeOn); noticeStateLayout->addWidget(noticeOff); form->addRow(m_noticeBitStates); //20261112-0 change: Native notifications use a 1/0 radio pair.
    row = new QHBoxLayout(); row->addWidget(check(QStringLiteral("noticeWord"), QStringLiteral("通知字"), box)); row->addWidget(address(QStringLiteral("noticeWordAddress"), false, box)); form->addRow(row); form->addRow(QStringLiteral("数据类型:"), combo(QStringLiteral("noticeWordType"), StaticTextProperties::dataTypes(), box)); form->addRow(QStringLiteral("数值:"), line(QStringLiteral("noticeWordValue"), box)); layout->addWidget(box);
    row = new QHBoxLayout(); row->addWidget(check(QStringLiteral("pressTimeEnabled"), QStringLiteral("按压时间"), page)); row->addWidget(spin(QStringLiteral("pressTime"), 0, 255, page)); m_pressUnits = new QLabel(QStringLiteral("x100毫秒"), page); row->addWidget(m_pressUnits); layout->addLayout(row); layout->addStretch(); tabs->addTab(page, QStringLiteral("权限设置"));
    page = new QWidget(tabs); m_otherPage = page; layout = new QVBoxLayout(page); box = new QGroupBox(QStringLiteral("键盘设置"), page); form = new QFormLayout(box); form->addRow(check(QStringLiteral("customKeyboard"), QStringLiteral("使用自定义键盘"), box)); form->addRow(check(QStringLiteral("asciiModifyInput"), QStringLiteral("修改输入"), box)); qobject_cast<QCheckBox *>(m_fields.value(QStringLiteral("asciiModifyInput")))->setChecked(m_asciiDraft.modifyInput); keyboard = new QComboBox(box); keyboard->setObjectName(QStringLiteral("Number_keyboardPageId"));
    Q_FOREACH (keyboardPage, project.allPages()) { if (keyboardPage.kind == QStringLiteral("keyboard")) { keyboard->addItem(keyboardPage.name, keyboardPage.id); } }
    keyboard->setCurrentIndex(keyboard->findData(m_draft.keyboardPageId)); m_fields.insert(QStringLiteral("keyboardPageId"), keyboard); form->addRow(QStringLiteral("窗口编号:"), keyboard); m_keyboardLabel = form->labelForField(keyboard);
    box = group(QStringLiteral("keyboardPosition"), QStringLiteral("键盘自定义位置"), box); form->addRow(box); form = new QFormLayout(box); form->addRow(QStringLiteral("X位置:"), spin(QStringLiteral("keyboardX"), 0, 800, box)); form->addRow(QStringLiteral("Y位置:"), spin(QStringLiteral("keyboardY"), 0, 480, box)); layout->addWidget(box->parentWidget()); layout->addStretch(); tabs->addTab(page, QStringLiteral("其它"));
    preview = new QGroupBox(QStringLiteral("预览"), this); preview->setFixedWidth(170); previewLayout = new QVBoxLayout(preview); m_preview = new QLabel(preview); m_preview->setObjectName(QStringLiteral("NumberPreview")); m_preview->setFixedSize(145, 190); previewLayout->addWidget(m_preview); previewLayout->addStretch(); content->addWidget(preview); root->addLayout(content, 1);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this); buttons->setObjectName(QStringLiteral("NumberButtons")); buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); root->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &NumberDisplayDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &NumberDisplayDialog::reject);
    connect(dataType, &QComboBox::currentIndexChanged, this, &NumberDisplayDialog::changeType); m_ready = true; updateLinkage();
    connect(qobject_cast<QCheckBox *>(m_fields.value(QStringLiteral("noticeBit"))), &QCheckBox::toggled, this, [this](bool selected) { if (selected) { qobject_cast<QCheckBox *>(m_fields.value(QStringLiteral("noticeWord")))->setChecked(false); } }); //20261112-0 add
    connect(qobject_cast<QCheckBox *>(m_fields.value(QStringLiteral("noticeWord"))), &QCheckBox::toggled, this, [this](bool selected) { if (selected) { qobject_cast<QCheckBox *>(m_fields.value(QStringLiteral("noticeBit")))->setChecked(false); } });
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::changeType
 * Description     : Apply measured type transitions without interpreting UI indices as native source/device enum values.
 * Input Parameters: type
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void NumberDisplayDialog::changeType(int type) //20261112-0 add
{
    NumberDisplayProperties draft = object().numberDisplay;
    if (!m_ready || m_updating) { return; } m_updating = true; draft.changeDataType(type);
    qobject_cast<QLineEdit *>(m_fields.value(QStringLiteral("inputMinimum")))->setText(draft.inputMinimum); qobject_cast<QLineEdit *>(m_fields.value(QStringLiteral("inputMaximum")))->setText(draft.inputMaximum);
    qobject_cast<QLineEdit *>(m_fields.value(QStringLiteral("displayMinimum")))->setText(draft.displayMinimum); qobject_cast<QLineEdit *>(m_fields.value(QStringLiteral("displayMaximum")))->setText(draft.displayMaximum);
    qobject_cast<QSpinBox *>(m_fields.value(QStringLiteral("totalDigits")))->setMaximum(NumberDisplayProperties::maximumDigits(type)); m_updating = false; updateLinkage();
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::updateLinkage
 * Description     : Apply input/source/permission/keyboard enablement and refresh the shared editor sample without losing dormant fields.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void NumberDisplayDialog::updateLinkage() //20261112-0 add
{
    NumberDisplayProperties draft;
    QPixmap image(145, 190);
    QPainter painter;
    QString prefix;
    QString suffix;
    int index;
    int end;
    bool fromAddress;
    if (!m_ready || m_updating) { return; } m_updating = true; draft = object().numberDisplay;
    m_draft.inputAddress = draft.inputAddress; //20261112-0 add: Display the synchronized address while retaining the independent input-address draft.
    qobject_cast<QLineEdit *>(m_fields.value(QStringLiteral("inputAddress")))->setText(draft.sameMonitor ? draft.monitorAddress : draft.inputAddress); m_inputShowingMonitor = draft.sameMonitor;
    m_dataControls->setVisible(!m_timeMode); m_timeControls->setVisible(m_timeMode); m_numberFontControls->setVisible(!m_timeMode); m_timeFontControls->setVisible(m_timeMode); m_timeTotal->setVisible(m_timeMode); m_timeTotalLabel->setVisible(m_timeMode);
    if (m_timeMode && findChild<QTabWidget *>(QStringLiteral("NumberTabs"))->indexOf(m_otherPage) >= 0) { findChild<QTabWidget *>(QStringLiteral("NumberTabs"))->removeTab(3); }
    if (!m_timeMode && findChild<QTabWidget *>(QStringLiteral("NumberTabs"))->indexOf(m_otherPage) < 0) { findChild<QTabWidget *>(QStringLiteral("NumberTabs"))->addTab(m_otherPage, QStringLiteral("其它")); }
    setObjectName(m_timeMode ? QStringLiteral("TimeDisplayDialog") : m_asciiMode ? QStringLiteral("AsciiDisplayDialog") : QStringLiteral("NumberDisplayDialog"));
    m_colorLabels.value(QStringLiteral("textColor"))->setText(m_asciiMode ? QStringLiteral("文本颜色:") : QStringLiteral("数值颜色:")); m_colorLabels.value(QStringLiteral("backgroundColor"))->setText(m_asciiMode ? QStringLiteral("文本背景颜色:") : QStringLiteral("数值背景颜色:")); //20261112-0 change
    setWindowTitle(m_timeMode ? QStringLiteral("时间显示") : m_asciiMode ? draft.allowInput ? QStringLiteral("ASCII输入") : QStringLiteral("ASCII显示") : draft.allowInput ? QStringLiteral("数值输入") : QStringLiteral("数值显示"));
    m_fields.value(QStringLiteral("dataType"))->setVisible(!m_asciiMode && !m_timeMode); m_dataTypeLabel->setVisible(!m_asciiMode && !m_timeMode); m_inputRange->setVisible(!m_asciiMode && !m_timeMode); m_fields.value(QStringLiteral("scaling"))->setVisible(!m_asciiMode && !m_timeMode); m_fields.value(QStringLiteral("macroEnabled"))->setVisible(!m_timeMode); m_fields.value(QStringLiteral("displayRange"))->setVisible(!m_asciiMode && !m_timeMode);
    m_asciiControls->setVisible(m_asciiMode && !m_timeMode); m_fields.value(QStringLiteral("asciiEncoding"))->setEnabled(draft.allowInput); m_fields.value(QStringLiteral("totalDigits"))->setVisible(!m_asciiMode && !m_timeMode); m_totalDigitsLabel->setVisible(!m_asciiMode && !m_timeMode); m_fields.value(QStringLiteral("asciiCharacterCount"))->setVisible(m_asciiMode); m_characterCountLabel->setVisible(m_asciiMode); m_fields.value(QStringLiteral("asciiModifyInput"))->setVisible(m_asciiMode);
    m_fields.value(QStringLiteral("adjustment"))->setEnabled(!m_asciiMode && !m_timeMode); m_fields.value(QStringLiteral("customDisplay"))->setVisible(!m_asciiMode && !m_timeMode); m_fields.value(QStringLiteral("customText"))->setVisible(!m_asciiMode && !m_timeMode);
    m_fields.value(QStringLiteral("lowColor"))->setEnabled(!m_asciiMode && !m_timeMode); m_fields.value(QStringLiteral("highColor"))->setEnabled(!m_asciiMode && !m_timeMode); findChild<QPushButton *>(QStringLiteral("Number_lowColorPicker"))->setEnabled(!m_asciiMode && !m_timeMode); findChild<QPushButton *>(QStringLiteral("Number_highColorPicker"))->setEnabled(!m_asciiMode && !m_timeMode);
    m_addressRows.value(QStringLiteral("offsetAddress"))->setVisible(draft.offsetEnabled); m_addressRows.value(QStringLiteral("inputAddress"))->setEnabled(draft.allowInput && !draft.sameMonitor);
    m_fields.value(QStringLiteral("sameMonitor"))->setEnabled(draft.allowInput); m_fields.value(QStringLiteral("password"))->setEnabled(draft.allowInput); m_fields.value(QStringLiteral("inputMarker"))->setEnabled(draft.allowInput); m_inputRange->setEnabled(draft.allowInput);
    m_fields.value(QStringLiteral("decimalDigits"))->setEnabled(!m_asciiMode && !m_timeMode && NumberDisplayProperties::decimalEnabled(draft.dataType)); m_fields.value(QStringLiteral("customText"))->setEnabled(draft.customDisplay);
    m_touch->setVisible(!m_timeMode); m_touch->setEnabled(draft.allowInput); m_fields.value(QStringLiteral("notice"))->setVisible(draft.allowInput && !m_timeMode); m_fields.value(QStringLiteral("pressTimeEnabled"))->setVisible(draft.allowInput && !m_timeMode); m_fields.value(QStringLiteral("pressTime"))->setVisible(draft.allowInput && draft.pressTimeEnabled && !m_timeMode); m_pressUnits->setVisible(draft.allowInput && draft.pressTimeEnabled && !m_timeMode);
    for (index = 0; index < 2; ++index)
    {
        prefix = index == 0 ? QStringLiteral("input") : QStringLiteral("display"); fromAddress = index == 0 ? draft.inputRangeSource == 1 : draft.displayRangeSource == 1;
        for (end = 0; end < 2; ++end) { suffix = end == 0 ? QStringLiteral("Minimum") : QStringLiteral("Maximum"); m_fields.value(prefix + suffix)->setVisible(!fromAddress); m_addressRows.value(prefix + suffix + QStringLiteral("Address"))->setVisible(fromAddress); }
    }
    m_addressRows.value(QStringLiteral("noticeBitAddress"))->setEnabled(draft.noticeBit); m_noticeBitStates->setEnabled(draft.noticeBit); m_addressRows.value(QStringLiteral("noticeWordAddress"))->setEnabled(draft.noticeWord); m_fields.value(QStringLiteral("noticeWordType"))->setEnabled(draft.noticeWord); m_fields.value(QStringLiteral("noticeWordValue"))->setEnabled(draft.noticeWord);
    m_fields.value(QStringLiteral("keyboardPageId"))->setVisible(draft.customKeyboard); m_keyboardLabel->setVisible(draft.customKeyboard); m_fields.value(QStringLiteral("keyboardPosition"))->setVisible(draft.customKeyboard);
    if (m_timeMode && object().timeDisplay.sample().isEmpty()) { m_updating = false; return; } //20261112-0 add: Preserve the last visible format and raster while the all-hidden draft is invalid.
    if (m_timeMode) { m_timeSample->setText(object().timeDisplay.sample()); }
    image.fill(Qt::white); painter.begin(&image); if (m_timeMode) { paintTimeDisplay(&painter, QRectF(5, 25, 135, 100), object().timeDisplay); } else if (m_asciiMode) { paintAsciiDisplay(&painter, QRectF(5, 25, 135, 100), object().asciiDisplay); } else { paintNumberDisplay(&painter, QRectF(5, 25, 135, 100), draft); } painter.end(); m_preview->setPixmap(image); m_updating = false;
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::choosePicture
 * Description     : Reuse the existing static-image gallery as a nested numeric-artwork transaction and reject GIF flattening.
 * Input Parameters: projectGallery
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void NumberDisplayDialog::choosePicture(bool projectGallery) //20261112-0 add
{
    HmiObject adapter;
    HmiProject context = m_project;
    BitButtonDialog *gallery;
    QMap<QString, QByteArray> pool;
    ImageDisplayState appearance = m_draft.appearance;
    adapter.id = QStringLiteral("number-gallery-adapter"); adapter.type = QStringLiteral("switch"); adapter.address = QStringLiteral("LB0"); adapter.hasBitButtonProperties = true; context.objects.append(object()); gallery = new BitButtonDialog(adapter, context, this);
    if (gallery->choosePictureForControl(projectGallery, &appearance.picture, &appearance.picturePng, &pool)) { m_project.pictures = pool; m_draft.appearance = appearance; updateLinkage(); } delete gallery;
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::object
 * Description     : Collect all four pages and disabled drafts into a complete independent numeric object.
 * Input Parameters: None
 * Return Value    : Detached object.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiObject NumberDisplayDialog::object() const //20261112-0 add
{
    HmiObject result = m_original;
    result.type = QStringLiteral("number"); result.hasNumberDisplayProperties = true; result.numberDisplay = m_draft;
#define READ_NUMBER_BOOL(name, initial) result.numberDisplay.name = field(QStringLiteral(#name)).toBool();
    NUMBER_BOOLEAN_FIELDS(READ_NUMBER_BOOL)
#undef READ_NUMBER_BOOL
#define READ_NUMBER_INT(name, initial) result.numberDisplay.name = field(QStringLiteral(#name)).toInt();
    NUMBER_INTEGER_FIELDS(READ_NUMBER_INT)
#undef READ_NUMBER_INT
#define READ_NUMBER_STRING(name, initial) result.numberDisplay.name = field(QStringLiteral(#name)).toString();
    NUMBER_STRING_FIELDS(READ_NUMBER_STRING)
#undef READ_NUMBER_STRING
    if (m_visibility != nullptr) { result.numberDisplay.visibility = m_visibility->properties(); result.numberDisplay.touch = m_touch->properties(); }
    if (m_inputShowingMonitor) { result.numberDisplay.inputAddress = m_draft.inputAddress; }
    if (m_asciiMode) //20261112-0 add: One accepted family is persisted, with inactive numeric fields preserved in the ASCII common draft.
    {
        result.type = QStringLiteral("ascii"); result.hasNumberDisplayProperties = false; result.asciiDisplay = m_asciiDraft; result.asciiDisplay.common = result.numberDisplay;
        result.asciiDisplay.characterCount = field(QStringLiteral("asciiCharacterCount")).toInt(); result.asciiDisplay.encoding = field(QStringLiteral("asciiEncoding")).toInt(); result.asciiDisplay.autoWrap = field(QStringLiteral("asciiAutoWrap")).toBool(); result.asciiDisplay.swapBytes = field(QStringLiteral("asciiSwapBytes")).toBool(); result.asciiDisplay.modifyInput = field(QStringLiteral("asciiModifyInput")).toBool();
    }
    result.address = result.numberDisplay.monitorAddress;
    if (m_timeMode) //20261112-0 add: Clock fields are independent; the shared shell never serializes dormant numeric addresses or keyboard references.
    {
        result.type = QStringLiteral("time"); result.hasNumberDisplayProperties = false; result.address.clear(); result.timeDisplay = m_timeDraft;
        result.timeDisplay.showDate = field(QStringLiteral("timeShowDate")).toBool(); result.timeDisplay.showWeek = field(QStringLiteral("timeShowWeek")).toBool(); result.timeDisplay.showTime = field(QStringLiteral("timeShowTime")).toBool(); result.timeDisplay.dateFormat = field(QStringLiteral("timeDateFormat")).toInt(); result.timeDisplay.timeFormat = field(QStringLiteral("timeTimeFormat")).toInt(); result.timeDisplay.fontSize = field(QStringLiteral("timeFontSize")).toInt();
        result.timeDisplay.alignment = result.numberDisplay.alignment; result.timeDisplay.opacity = result.numberDisplay.opacity; result.timeDisplay.textColor = result.numberDisplay.textColor; result.timeDisplay.backgroundColor = result.numberDisplay.backgroundColor; result.timeDisplay.effect3d = result.numberDisplay.effect3d; result.timeDisplay.transparent = result.numberDisplay.transparent; result.timeDisplay.appearance = result.numberDisplay.appearance; result.timeDisplay.visibility = result.numberDisplay.visibility;
    }
    return result;
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::pictures
 * Description     : Return only the accepted nested gallery draft for atomic object/resource history.
 * Input Parameters: None
 * Return Value    : Picture pool.
 * Date            : 2026.11.12
 ******************************************************************************/
QMap<QString, QByteArray> NumberDisplayDialog::pictures() const //20261112-0 add
{
    return m_project.pictures;
}

/******************************************************************************
 * Function Name   : NumberDisplayDialog::accept
 * Description     : Validate the complete numeric draft and active keyboard reference before closing the property transaction.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void NumberDisplayDialog::accept() //20261112-0 add
{
    HmiObject candidate = object();
    HmiPage page;
    bool found = false;
    QString error;
    if (m_timeMode) //20261112-0 add: Saving an empty display is rejected even when a previous immediate warning was dismissed.
    {
        if (!candidate.timeDisplay.validate(&error) || !candidate.timeDisplay.validateAddresses(m_project.connections, &error)) { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), error); return; }
        QDialog::accept(); return;
    }
    if (m_asciiMode ? !candidate.asciiDisplay.validate(&error) || !candidate.asciiDisplay.validateAddresses(m_project.connections, &error) : !candidate.numberDisplay.validate(&error) || !candidate.numberDisplay.validateAddresses(m_project.connections, &error))
    { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), error); return; }
    Q_FOREACH (page, m_project.allPages()) { if (page.id == candidate.numberDisplay.keyboardPageId && page.kind == QStringLiteral("keyboard")) { found = true; } }
    if (candidate.numberDisplay.allowInput && candidate.numberDisplay.customKeyboard && !found)
    { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), QStringLiteral("自定义键盘画面不存在或不是键盘类型。")); return; }
    QDialog::accept();
}
