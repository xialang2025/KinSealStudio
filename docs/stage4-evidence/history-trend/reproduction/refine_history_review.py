"""Apply the compile-review corrections and compact the history axes to the observed horizontal forms."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
p=root/'tests/stage4_controls_tests.cpp';s=p.read_text(encoding='utf-8');s=s.replace('pasteStage3Selection(target, clipboard, 0, 0,','pasteStage3Selection(target, clipboard, QPoint(0, 0),');p.write_text(s,encoding='utf-8',newline='\n')
p=root/'src/history_trend_dialog.cpp';s=p.read_text(encoding='utf-8');start=s.index('QWidget *HistoryTrendDialog::axisPage(bool y)');end=s.index('/******************************************************************************',start)
s=s[:start]+'''QWidget *HistoryTrendDialog::axisPage(bool y) //20261112-1 add
{
    TrendAxis *axis = y ? &m_draft.yAxis : &m_draft.xAxis;
    QString prefix = y ? QStringLiteral("HistoryY") : QStringLiteral("HistoryX");
    QGroupBox *page = new QGroupBox(y ? QStringLiteral("Y轴") : QStringLiteral("X轴"), this);
    QGridLayout *layout = new QGridLayout(page);
    QWidget *flags = new QWidget(page);
    QHBoxLayout *flagLayout = new QHBoxLayout(flags);
    QCheckBox *ticks = new QCheckBox(QStringLiteral("显示刻度"), flags);
    QCheckBox *gridlines = new QCheckBox(y ? QStringLiteral("显示X轴曲线") : QStringLiteral("显示Y轴曲线"), flags);
    QWidget *numbers = new QWidget(page);
    QGridLayout *numberGrid = new QGridLayout(numbers);
    QWidget *palette = new QWidget(page);
    QGridLayout *paletteGrid = new QGridLayout(palette);
    QGroupBox *marks = new QGroupBox(y ? QStringLiteral("刻度") : QStringLiteral("显示时间"), page);
    QWidget *fields = new QWidget(marks);
    QGridLayout *markGrid = new QGridLayout(fields);
    QVBoxLayout *markLayout = new QVBoxLayout(marks);
    QComboBox *font = new QComboBox(fields);
    QPushButton *axisColor;
    QGroupBox *time;
    QHBoxLayout *timeLayout;
    QWidget *part;
    QGridLayout *partGrid;
    int index;
    int *values[3] = {&m_draft.hours, &m_draft.minutes, &m_draft.seconds};
    QStringList names = {QStringLiteral("Hours"), QStringLiteral("Minutes"), QStringLiteral("Seconds")};
    QStringList labels = {QStringLiteral("小时:"), QStringLiteral("分钟:"), QStringLiteral("秒:")};
    ticks->setObjectName(prefix + QStringLiteral("Ticks")); ticks->setChecked(axis->ticks); gridlines->setObjectName(prefix + QStringLiteral("Grid")); gridlines->setChecked(axis->grid); flagLayout->addWidget(ticks); flagLayout->addWidget(gridlines); flagLayout->setContentsMargins(0, 0, 0, 0);
    connect(ticks, &QCheckBox::toggled, this, [this, axis](bool checked) { axis->ticks = checked; refresh(); }); connect(gridlines, &QCheckBox::toggled, this, [this, axis](bool checked) { axis->grid = checked; refresh(); });
    numberGrid->setContentsMargins(0, 0, 0, 0); paletteGrid->setContentsMargins(0, 0, 0, 0);
    integer(numberGrid, 0, QStringLiteral("主分隔数:"), prefix + QStringLiteral("Major"), &axis->major, 3, 10); integer(numberGrid, 1, QStringLiteral("次分隔数:"), prefix + QStringLiteral("Minor"), &axis->minor, 1, 3);
    axisColor = color(paletteGrid, 0, QStringLiteral("轴/刻度颜色:"), prefix + QStringLiteral("AxisColor"), &axis->axisColor); color(paletteGrid, 1, QStringLiteral("格线颜色:"), prefix + QStringLiteral("GridColor"), &axis->gridColor); axisColor->setVisible(axis->ticks); connect(ticks, &QCheckBox::toggled, axisColor, &QWidget::setVisible);
    if (y) { textField(numberGrid, 2, QStringLiteral("最大值:"), QStringLiteral("HistoryYMaximum"), &m_draft.yMaximum); textField(paletteGrid, 2, QStringLiteral("最小值:"), QStringLiteral("HistoryYMinimum"), &m_draft.yMinimum); }
    marks->setObjectName(prefix + QStringLiteral("Marks")); marks->setCheckable(true); marks->setChecked(axis->marks); markLayout->addWidget(fields); fields->setVisible(axis->marks); connect(marks, &QGroupBox::toggled, this, [this, axis, fields](bool checked) { axis->marks = checked; fields->setVisible(checked); refresh(); });
    font->setObjectName(prefix + QStringLiteral("FontSize")); font->addItems({QStringLiteral("12"), QStringLiteral("16"), QStringLiteral("24"), QStringLiteral("48")}); font->setCurrentText(QString::number(axis->fontSize)); markGrid->addWidget(new QLabel(QStringLiteral("字体大小:"), fields), 0, 0); markGrid->addWidget(font, 0, 1); connect(font, &QComboBox::currentTextChanged, this, [this, axis](const QString &value) { axis->fontSize = value.toInt(); refresh(); });
    if (y) { integer(markGrid, 1, QStringLiteral("总位数:"), QStringLiteral("HistoryTotalDigits"), &m_draft.totalDigits, 1, 10); integer(markGrid, 2, QStringLiteral("小数位数:"), QStringLiteral("HistoryFractionalDigits"), &m_draft.fractionalDigits, 0, 3); }
    else { choice(markGrid, 1, QStringLiteral("格式:"), QStringLiteral("HistoryTimeFormat"), {QStringLiteral("HH:MM"), QStringLiteral("HH:MM:SS")}, &m_draft.timeFormat); }
    layout->addWidget(flags, 0, 0, 1, 2); layout->addWidget(numbers, 1, 0); layout->addWidget(palette, 1, 1); layout->addWidget(marks, 0, 2, 2, 1, Qt::AlignTop); layout->setColumnStretch(0, 1); layout->setColumnStretch(1, 1); layout->setColumnStretch(2, 1);
    if (!y)
    {
        time = new QGroupBox(QStringLiteral("时间范围"), page); timeLayout = new QHBoxLayout(time);
        for (index = 0; index < 3; ++index) { part = new QWidget(time); partGrid = new QGridLayout(part); partGrid->setContentsMargins(0, 0, 0, 0); integer(partGrid, 0, labels[index], QStringLiteral("History") + names[index], values[index], 0, index == 0 ? 48 : 59); timeLayout->addWidget(part); }
        layout->addWidget(time, 2, 0, 1, 3);
    }
    return page;
}

'''+s[end:]
s=s.replace('resize(805, 650)','resize(805, 584)')
# History curves deliberately expose no direct PLC address field; permission addresses use their shared widget.
s=s.replace('    QPushButton *picker;\n    edit->setMaximumWidth', '    Q_UNUSED(addressRole);\n    edit->setMaximumWidth')
s=s.replace('    if (addressRole != 0) { picker = new QPushButton(QStringLiteral("…"), this); picker->setFixedWidth(26); picker->setObjectName(name + QStringLiteral("Picker")); layout->addWidget(picker, row, 2); connect(picker, &QPushButton::clicked, this, [this, edit, addressRole]() { editHmiAddress(this, edit, addressRole == 1, addressRole == 1 ? 1 : 1); }); }\n','')
p.write_text(s,encoding='utf-8',newline='\n')
p=root/'src/data_collection_dialog.cpp';s=p.read_text(encoding='utf-8').replace('resize(595, 690)','resize(595, 720)');s=s.replace('    QCheckBox *notice;','    QCheckBox *notice;\n    QCheckBox *upload;\n    QSpinBox *sampleCount;\n    QWidget *sampleCountLabel;')
s=s.replace('toggle(nameRow, 1, QStringLiteral("上传云端"), QStringLiteral("CollectionUpload"), &m_draft.upload);','upload = toggle(nameRow, 1, QStringLiteral("上传云端"), QStringLiteral("CollectionUpload"), &m_draft.upload); nameRow->removeWidget(upload); nameRow->addWidget(upload, 0, 2);')
s=s.replace('integer(grid, 1, QStringLiteral("取样总数:"), QStringLiteral("CollectionCount"), &m_draft.sampleCount, 1, 65535);','sampleCount = integer(grid, 1, QStringLiteral("取样总数:"), QStringLiteral("CollectionCount"), &m_draft.sampleCount, 1, 65535); sampleCountLabel = grid->itemAtPosition(1, 0)->widget(); grid->removeWidget(sampleCount); grid->removeWidget(sampleCountLabel); grid->addWidget(sampleCountLabel, 0, 2); grid->addWidget(sampleCount, 0, 3);')
s=s.replace('formula->setMinimumHeight(130)','formula->setMinimumHeight(220)').replace('grid->addWidget(formula, 3, 0, 1, 2)','grid->addWidget(formula, 3, 0, 1, 4)')
p.write_text(s,encoding='utf-8',newline='\n')
print('Fixed renderer carrier and clipboard argument review findings; compacted actual native dialog forms.')
