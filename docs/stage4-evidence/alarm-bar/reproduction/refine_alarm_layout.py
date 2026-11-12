"""Match C27's observed fill/other/text group layout without changing alarm semantics."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
p=root/'src/alarm_bar_dialog.cpp';s=p.read_text(encoding='utf-8');a=s.index('QWidget *AlarmBarDialog::basicPage()');b=s.index('\n/******************************************************************************',a)
s=s[:a]+'''QWidget *AlarmBarDialog::basicPage() //20261113-2 add
{
    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *fill = new QGroupBox(QStringLiteral("填充"), page);
    QGridLayout *fillGrid = new QGridLayout(fill);
    QGroupBox *other = new QGroupBox(QStringLiteral("其他"), page);
    QVBoxLayout *otherLayout = new QVBoxLayout(other);
    QGroupBox *text = new QGroupBox(QStringLiteral("文本"), page);
    QGridLayout *textGrid = new QGridLayout(text);
    QGridLayout *grid;
    QStringList labels = {QStringLiteral("边框色:"), QStringLiteral("前景色:"), QStringLiteral("背景色:"), QStringLiteral("文本色:")};
    QStringList names = {QStringLiteral("FrameColor"), QStringLiteral("ForegroundColor"), QStringLiteral("BackgroundColor"), QStringLiteral("TextColor")};
    QList<QString *> colors = {&m_draft.frameColor, &m_draft.foregroundColor, &m_draft.backgroundColor, &m_draft.textColor};
    QPushButton *button;
    QComboBox *pattern = new QComboBox(fill);
    QComboBox *font = new QComboBox(text);
    QSpinBox *speed = new QSpinBox(other);
    QHBoxLayout *speedLine = new QHBoxLayout;
    QGroupBox *group;
    QHBoxLayout *line;
    QRadioButton *first;
    QRadioButton *second;
    int *value;
    int row;
    int index;
    layout->setContentsMargins(12, 14, 18, 20); layout->setHorizontalSpacing(20); layout->setVerticalSpacing(12); layout->setColumnStretch(0, 4); layout->setColumnStretch(1, 5); fill->setMinimumHeight(205); other->setMinimumHeight(205);
    for (index = 0; index < 4; ++index)
    {
        grid = index == 3 ? textGrid : fillGrid; row = index == 3 ? 1 : index;
        button = new QPushButton(index == 3 ? text : fill); button->setObjectName(QStringLiteral("Alarm") + names.at(index)); button->setFixedSize(120, 24); button->setStyleSheet(QStringLiteral("background:%1;").arg(*colors.at(index))); grid->addWidget(new QLabel(labels.at(index), page), row, 0); grid->addWidget(button, row, 1, Qt::AlignLeft);
        connect(button, &QPushButton::clicked, this, [this, button, colors, index]() { QColor color = QColorDialog::getColor(QColor(*colors.at(index)), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog); if (color.isValid()) { *colors.at(index) = color.name(); button->setStyleSheet(QStringLiteral("background:%1;").arg(color.name())); refresh(); } });
    }
    pattern->setObjectName(QStringLiteral("AlarmPattern")); pattern->addItems(FlowBlockProperties::patterns()); pattern->setCurrentIndex(m_draft.pattern); pattern->setFixedWidth(120); fillGrid->addWidget(new QLabel(QStringLiteral("样式:"), fill), 3, 0); fillGrid->addWidget(pattern, 3, 1, Qt::AlignLeft); connect(pattern, &QComboBox::currentIndexChanged, this, [this](int v) { m_draft.pattern = v; refresh(); }); layout->addWidget(fill, 0, 0);
    speed->setObjectName(QStringLiteral("AlarmSpeed")); speed->setRange(0, 30); speed->setValue(m_draft.speed); speed->setFixedWidth(120); speedLine->addWidget(new QLabel(QStringLiteral("移动速度:"), other)); speedLine->addWidget(speed); speedLine->addWidget(new QLabel(QStringLiteral("x100ms"), other)); speedLine->addStretch(); otherLayout->addLayout(speedLine); connect(speed, &QSpinBox::valueChanged, this, [this](int v) { m_draft.speed = v; });
    for (index = 0; index < 2; ++index)
    {
        value = index == 0 ? &m_draft.direction : &m_draft.sortOrder; group = new QGroupBox(index == 0 ? QStringLiteral("方向") : QStringLiteral("报警排序"), other); line = new QHBoxLayout(group); first = new QRadioButton(index == 0 ? QStringLiteral("向左") : QStringLiteral("按时间顺序"), group); second = new QRadioButton(index == 0 ? QStringLiteral("向右") : QStringLiteral("按时间逆序"), group); first->setObjectName(index == 0 ? QStringLiteral("AlarmLeft") : QStringLiteral("AlarmTime")); second->setObjectName(index == 0 ? QStringLiteral("AlarmRight") : QStringLiteral("AlarmReverseTime")); first->setChecked(*value == 0); second->setChecked(*value == 1); line->addWidget(first); line->addWidget(second); otherLayout->addWidget(group); connect(first, &QRadioButton::toggled, this, [value](bool checked) { if (checked) { *value = 0; } }); connect(second, &QRadioButton::toggled, this, [value](bool checked) { if (checked) { *value = 1; } });
    }
    otherLayout->addStretch(); layout->addWidget(other, 0, 1); textGrid->setColumnStretch(2, 1); font->setObjectName(QStringLiteral("AlarmFontSize")); font->addItems({QStringLiteral("12"), QStringLiteral("16"), QStringLiteral("24"), QStringLiteral("48"), QStringLiteral("64")}); font->setCurrentText(QString::number(m_draft.fontSize)); font->setFixedWidth(120); textGrid->addWidget(new QLabel(QStringLiteral("字体大小:"), text), 0, 0); textGrid->addWidget(font, 0, 1); connect(font, &QComboBox::currentTextChanged, this, [this](const QString &v) { m_draft.fontSize = v.toInt(); refresh(); }); layout->addWidget(text, 1, 0, 1, 2); layout->setRowStretch(2, 1); return page;
}
''' + s[b:]
s=s.replace('resize(790, 550)', 'resize(820, 560)').replace('m_preview->setFixedSize(140, 120)', 'm_preview->setFixedSize(130, 130)').replace('QImage image(280, 240,', 'QImage image(260, 260,').replace('QRectF(12, 75, 256, 80)', 'QRectF(30, 30, 200, 200)').replace('.scaled(140, 120,', '.scaled(130, 130,')
s=s.replace('QImage image(260, 260,', 'QImage image(130, 130,').replace('QRectF(30, 30, 200, 200)', 'QRectF(15, 15, 100, 100)').replace('QPixmap::fromImage(image).scaled(130, 130, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)', 'QPixmap::fromImage(image)')
p.write_text(s,encoding='utf-8',newline='\n')
print('Matched native fill/other/text groups, horizontal radios and square sample preview.')
