"""Build the measured C26 three-page form using shared permission, address and axis widgets."""
from pathlib import Path
import re
root=Path(__file__).resolve().parent/'worktree';trend=(root/'src/trend_dialog.cpp').read_text(encoding='utf-8')
header=(root/'src/trend_dialog.h').read_text(encoding='utf-8').replace('TREND_DIALOG_H','DATA_GROUP_DIALOG_H').replace('TrendDialog','DataGroupDialog').replace('TrendProperties','DataGroupProperties').replace('paintTrend','paintDataGroup')
for line in ['    QComboBox *m_channel;\n','    QSpinBox *m_count;\n','    QLineEdit *m_minimum;\n','    QLineEdit *m_maximum;\n','    QComboBox *m_lineStyle;\n','    QPushButton *m_channelColor;\n','    void changeCount(int count);\n','    void showChannel(int channel);\n']:header=header.replace(line,'')
(root/'src/data_group_dialog.h').write_text(header,encoding='utf-8',newline='\n')
def function(name,description,signature,body,inputs='None',returns='None.'):
 return f'/******************************************************************************\n * Function Name   : {name}\n * Description     : {description}\n * Input Parameters: {inputs}\n * Return Value    : {returns}\n * Date            : 2026.11.13\n ******************************************************************************/\n{signature} //20261113-1 add\n{{\n{body}\n}}\n\n'
s=trend[:trend.index('/******************************************************************************')].replace('"trend_dialog.h"','"data_group_dialog.h"\n#include "trend_dialog.h"')
s+=function('DataGroupDialog::channelPage','Keep exactly one curve and preserve the sampling-address draft while its mode is inactive.','QWidget *DataGroupDialog::channelPage()','''    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *addresses = new QGroupBox(QStringLiteral("地址"), page);
    QGroupBox *sampling = new QGroupBox(page);
    QGroupBox *channel = new QGroupBox(QStringLiteral("通道设置"), page);
    QGroupBox *fill = new QGroupBox(QStringLiteral("填充"), page);
    QGridLayout *grid = new QGridLayout(addresses);
    QComboBox *sampleMode;
    QLineEdit *sampleAddress;
    QPushButton *samplePicker;
    QSpinBox *count = new QSpinBox(channel);
    QComboBox *current = new QComboBox(channel);
    layout->setContentsMargins(12, 16, 16, 26); layout->addWidget(addresses, 0, 0); layout->addWidget(sampling, 1, 0); layout->addWidget(channel, 0, 1); layout->addWidget(fill, 1, 1); layout->setRowStretch(2, 1);
    textField(grid, 0, QStringLiteral("触发地址:"), QStringLiteral("GroupTriggerAddress"), &m_draft.triggerAddress, 1); textField(grid, 1, QStringLiteral("读取地址:"), QStringLiteral("GroupReadAddress"), &m_draft.readAddress, 2);
    grid = new QGridLayout(sampling); choice(grid, 0, QStringLiteral("数据类型:"), QStringLiteral("GroupDataType"), DataGroupProperties::dataTypes(), &m_draft.dataType); sampleMode = choice(grid, 1, QStringLiteral("采样方式:"), QStringLiteral("GroupSampleMode"), {QStringLiteral("常量"), QStringLiteral("地址")}, &m_draft.sampleMode); integer(grid, 2, QStringLiteral("采样点数:"), QStringLiteral("GroupSamplePoints"), &m_draft.samplePoints, 10, 800); sampleAddress = textField(grid, 3, QStringLiteral("采样地址:"), QStringLiteral("GroupSampleAddress"), &m_draft.sampleAddress, 3); samplePicker = findChild<QPushButton *>(QStringLiteral("GroupSampleAddressPicker")); sampleAddress->setEnabled(m_draft.sampleMode == 1); samplePicker->setEnabled(m_draft.sampleMode == 1); connect(sampleMode, &QComboBox::currentIndexChanged, this, [sampleAddress, samplePicker](int mode) { sampleAddress->setEnabled(mode == 1); samplePicker->setEnabled(mode == 1); });
    grid = new QGridLayout(channel); count->setObjectName(QStringLiteral("GroupChannelCount")); count->setRange(1, 1); current->setObjectName(QStringLiteral("GroupCurrentChannel")); current->addItem(QStringLiteral("1")); grid->addWidget(new QLabel(QStringLiteral("通道总数:"), channel), 0, 0); grid->addWidget(count, 0, 1); grid->addWidget(new QLabel(QStringLiteral("当前通道:"), channel), 1, 0); grid->addWidget(current, 1, 1); choice(grid, 2, QStringLiteral("线型:"), QStringLiteral("GroupLineStyle"), {QStringLiteral("—0"), QStringLiteral("--1"), QStringLiteral("--2"), QStringLiteral("--3"), QStringLiteral("--4")}, &m_draft.lineStyle); color(grid, 3, QStringLiteral("显示颜色:"), QStringLiteral("GroupChannelColor"), &m_draft.channelColor);
    grid = new QGridLayout(fill); color(grid, 0, QStringLiteral("底色:"), QStringLiteral("GroupBottomColor"), &m_draft.bottomColor); color(grid, 1, QStringLiteral("边框色:"), QStringLiteral("GroupFrameColor"), &m_draft.frameColor); color(grid, 2, QStringLiteral("背景色:"), QStringLiteral("GroupBackgroundColor"), &m_draft.backgroundColor); return page;
''',returns='General data-group page.')
s+=function('DataGroupDialog::axisPage','Expose only the X font and use a constant minimum with an exclusive constant/address maximum source.','QWidget *DataGroupDialog::axisPage(bool y)','''    TrendAxis *axis = y ? &m_draft.yAxis : &m_draft.xAxis;
    QString prefix = y ? QStringLiteral("GroupY") : QStringLiteral("GroupX");
    QGroupBox *page = new QGroupBox(y ? QStringLiteral("Y轴") : QStringLiteral("X轴"), this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    QGridLayout *grid = new QGridLayout;
    QWidget *axisColor = new QWidget(page);
    QGridLayout *colorGrid = new QGridLayout(axisColor);
    QWidget *gridColor = new QWidget(page);
    QGridLayout *gridColorGrid = new QGridLayout(gridColor);
    QWidget *marks = new QWidget(page);
    QGridLayout *markGrid = new QGridLayout(marks);
    QCheckBox *ticks = toggle(grid, 0, QStringLiteral("显示刻度"), prefix + QStringLiteral("Ticks"), &axis->ticks);
    QCheckBox *gridlines = toggle(grid, 1, y ? QStringLiteral("显示X轴曲线") : QStringLiteral("显示Y轴曲线"), prefix + QStringLiteral("Grid"), &axis->grid);
    QCheckBox *showMarks;
    QComboBox *font = new QComboBox(marks);
    QComboBox *source;
    QWidget *constant;
    QWidget *address;
    QGridLayout *sub;
    color(colorGrid, 0, QStringLiteral("轴/刻度颜色:"), prefix + QStringLiteral("AxisColor"), &axis->axisColor); axisColor->setVisible(axis->ticks); layout->addLayout(grid); layout->addWidget(axisColor); connect(ticks, &QCheckBox::toggled, axisColor, &QWidget::setVisible);
    color(gridColorGrid, 0, QStringLiteral("格线颜色:"), prefix + QStringLiteral("GridColor"), &axis->gridColor); gridColor->setVisible(axis->grid); layout->addWidget(gridColor); connect(gridlines, &QCheckBox::toggled, gridColor, &QWidget::setVisible);
    grid = new QGridLayout; integer(grid, 0, QStringLiteral("主分隔数:"), prefix + QStringLiteral("Major"), &axis->major, 3, 10); integer(grid, 1, QStringLiteral("次分隔数:"), prefix + QStringLiteral("Minor"), &axis->minor, 1, 3); showMarks = toggle(grid, 2, QStringLiteral("显示刻度标示"), prefix + QStringLiteral("Marks"), &axis->marks); layout->addLayout(grid);
    font->setObjectName(prefix + QStringLiteral("FontSize")); font->addItems({QStringLiteral("12"), QStringLiteral("16"), QStringLiteral("24"), QStringLiteral("48")}); font->setCurrentText(QString::number(axis->fontSize)); markGrid->addWidget(new QLabel(QStringLiteral("字体大小:"), marks), 0, 0); markGrid->addWidget(font, 0, 1); connect(font, &QComboBox::currentTextChanged, this, [this, axis](const QString &value) { axis->fontSize = value.toInt(); refresh(); });
    if (y)
    {
        source = choice(markGrid, 1, QStringLiteral("范围类型:"), QStringLiteral("GroupRangeSource"), {QStringLiteral("常量"), QStringLiteral("地址")}, &m_draft.rangeSource); textField(markGrid, 2, QStringLiteral("最小值:"), QStringLiteral("GroupYMinimum"), &m_draft.yMinimum);
        constant = new QWidget(marks); sub = new QGridLayout(constant); sub->setContentsMargins(0, 0, 0, 0); textField(sub, 0, QStringLiteral("最大值:"), QStringLiteral("GroupYMaximum"), &m_draft.yMaximum); markGrid->addWidget(constant, 3, 0, 1, 3);
        address = new QWidget(marks); sub = new QGridLayout(address); sub->setContentsMargins(0, 0, 0, 0); textField(sub, 0, QStringLiteral("最大值:"), QStringLiteral("GroupMaximumAddress"), &m_draft.maximumAddress, 4); markGrid->addWidget(address, 3, 0, 1, 3); constant->setVisible(m_draft.rangeSource == 0); address->setVisible(m_draft.rangeSource == 1); connect(source, &QComboBox::currentIndexChanged, this, [constant, address](int mode) { constant->setVisible(mode == 0); address->setVisible(mode == 1); });
        integer(markGrid, 4, QStringLiteral("总位数:"), QStringLiteral("GroupTotalDigits"), &m_draft.totalDigits, 1, 10); integer(markGrid, 5, QStringLiteral("小数位数:"), QStringLiteral("GroupFractionalDigits"), &m_draft.fractionalDigits, 0, 3);
    }
    marks->setVisible(axis->marks); layout->addWidget(marks); connect(showMarks, &QCheckBox::toggled, marks, &QWidget::setVisible); layout->addStretch(); return page;
''',inputs='y',returns='Independent X or Y configuration group.')
# Keep the audited constructor, detached transaction and primitive binding helpers; replace only family-specific fields.
start=trend.index('/******************************************************************************\n * Function Name   : TrendDialog::displayPage');tail=trend[start:]
for name in ['changeCount','showChannel']:
 a=tail.rfind('/******************************************************************************',0,tail.index(' * Function Name   : TrendDialog::'+name));b=tail.find('/******************************************************************************',a+5);tail=tail[:a]+tail[b:]
a=tail.rfind('/******************************************************************************',0,tail.index(' * Function Name   : paintTrend'));b=tail.find('/******************************************************************************',a+5);tail=tail[:a]+tail[b:]
tail=tail.replace('TrendDialog','DataGroupDialog').replace('TrendProperties','DataGroupProperties').replace('paintTrend','paintDataGroup').replace('object.trend','object.dataGroup').replace('candidate.trend','candidate.dataGroup').replace('QStringLiteral("trend")','QStringLiteral("datagroup")').replace('QStringLiteral("Trend','QStringLiteral("Group').replace('趋势图','数据群组').replace('m_draft.currentChannel = 0; ','').replace('resize(795, 584)','resize(795, 618)')
old='if (addressRole != 0) { picker = new QPushButton(QStringLiteral("…"), this); picker->setFixedWidth(26); picker->setObjectName(name + QStringLiteral("Picker")); layout->addWidget(picker, row, 2); connect(picker, &QPushButton::clicked, this, [this, edit, addressRole]() { editHmiAddress(this, edit, addressRole == 1, addressRole == 1 ? 1 : int(DataGroupProperties::wordCount(m_draft.dataType)) * m_draft.channelCount); }); }'
new='if (addressRole != 0) { picker = new QPushButton(QStringLiteral("…"), this); picker->setFixedWidth(26); picker->setObjectName(name + QStringLiteral("Picker")); layout->addWidget(picker, row, 2); connect(picker, &QPushButton::clicked, this, [this, edit, addressRole]() { u32 count = addressRole == 2 ? m_draft.readWordSpan() : addressRole == 4 ? DataGroupProperties::wordCount(m_draft.dataType) : 1U; editHmiAddress(this, edit, addressRole == 1, int(count)); }); }'
assert old in tail;tail=tail.replace(old,new).replace('设置触发、读取和清除地址，逐通道配置范围、线型、颜色及坐标轴。预览使用固定示意数据。','配置单通道的读取缓冲区、常量/地址采样以及Y轴最大值来源。预览使用固定示意数据。')
tail=tail.replace('return; } QDialog::accept();', 'return; }\n    if (m_draft.rangeSource == 1) { m_draft.yMinimum = QStringLiteral("0"); m_draft.yMaximum = QStringLiteral("1000"); } //20261113-1 add: Match native address-range confirmation.\n    QDialog::accept();')
tail=tail.replace('    setObjectName(QStringLiteral("DataGroupDialog"));', '    if (m_draft.rangeSource == 1) { m_draft.yMinimum = QStringLiteral("0"); m_draft.yMaximum = QStringLiteral("1000"); } //20261113-1 add: Match the native property reopen state.\n    setObjectName(QStringLiteral("DataGroupDialog"));')
s+=tail
s+=function('paintDataGroup','Share a fixed editor illustration while preserving outer/plot fill ownership and dormant-range drafts.','void paintDataGroup(QPainter *painter, const QRectF &rect, const DataGroupProperties &properties)','''    TrendProperties display;
    bool minimumOk;
    bool maximumOk;
    double minimum = properties.yMinimum.toDouble(&minimumOk);
    double maximum = properties.yMaximum.toDouble(&maximumOk);
    display.samplePoints = properties.samplePoints; display.backgroundColor = properties.bottomColor; display.bottomColor = properties.backgroundColor; display.frameColor = properties.frameColor; display.channels[0].color = properties.channelColor; display.channels[0].lineStyle = properties.lineStyle; display.xAxis = properties.xAxis; display.yAxis = properties.yAxis; display.totalDigits = properties.totalDigits; display.fractionalDigits = properties.fractionalDigits;
    if (minimumOk && maximumOk && std::isfinite(minimum) && std::isfinite(maximum) && minimum < maximum) { display.yMinimum = properties.yMinimum; display.yMaximum = properties.yMaximum; }
    paintTrend(painter, rect, display);
''',inputs='painter, rect, properties')
s=s.replace('//20261112-1','//20261113-1').replace('2026.11.12','2026.11.13')
(root/'src/data_group_dialog.cpp').write_text(s,encoding='utf-8',newline='\n')
print('Created C26 single-channel and independent sample/range-source dialog pages.')
