"""Build the C24 editor from the audited axis and permission widgets, keeping collection references explicit."""
from pathlib import Path
import re
root = Path(__file__).resolve().parent / 'worktree'
trend = (root/'src/trend_dialog.cpp').read_text(encoding='utf-8')
header = (root/'src/trend_dialog.h').read_text(encoding='utf-8')
header = header.replace('TREND_DIALOG_H','HISTORY_TREND_DIALOG_H').replace('paintTrend','paintHistoryTrend').replace('TrendProperties','HistoryTrendProperties').replace('TrendDialog','HistoryTrendDialog')
header = header.replace('    QWidget *channelPage();','    DataCollectionProperties m_collection;\n    QString m_collectionName;\n    QComboBox *m_source;\n    QWidget *channelPage();')
(root/'src/history_trend_dialog.h').write_text(header,encoding='utf-8')
def documented(name, desc, signature, body, inputs='None', returns='None.'):
 return f'/******************************************************************************\n * Function Name   : {name}\n * Description     : {desc}\n * Input Parameters: {inputs}\n * Return Value    : {returns}\n * Date            : 2026.11.12\n ******************************************************************************/\n{signature} //20261112-1 add\n{{\n{body}\n}}\n\n'
s = trend[:trend.index('/******************************************************************************')].replace('"trend_dialog.h"','"history_trend_dialog.h"')
s += documented('HistoryTrendDialog::channelPage','Bind a named collection and independent curve source, range, line and color drafts.', 'QWidget *HistoryTrendDialog::channelPage()', '''    QWidget *page = new QWidget(this);
    QGridLayout *layout = new QGridLayout(page);
    QGroupBox *collection = new QGroupBox(page);
    QGroupBox *channels = new QGroupBox(QStringLiteral("通道设置"), page);
    QGroupBox *fill = new QGroupBox(QStringLiteral("填充"), page);
    QGroupBox *range = new QGroupBox(QStringLiteral("显示范围"), page);
    QGridLayout *grid = new QGridLayout(collection);
    QComboBox *name = new QComboBox(collection);
    DataCollectionItem item;
    layout->setContentsMargins(12, 16, 16, 26); layout->addWidget(collection, 0, 0); layout->addWidget(channels, 0, 1); layout->addWidget(fill, 1, 0); layout->addWidget(range, 1, 1); layout->setRowStretch(2, 1);
    name->setObjectName(QStringLiteral("HistoryCollection")); name->addItem(m_collectionName, m_draft.collectionId); grid->addWidget(new QLabel(QStringLiteral("数据收集:"), collection), 0, 0); grid->addWidget(name, 0, 1);
    m_count = new QSpinBox(collection); m_count->setObjectName(QStringLiteral("HistoryCurveCount")); m_count->setRange(1, qMin(8, int(m_collection.items.size()))); m_count->setValue(m_draft.curveCount); grid->addWidget(new QLabel(QStringLiteral("曲线总数:"), collection), 1, 0); grid->addWidget(m_count, 1, 1);
    grid = new QGridLayout(fill); color(grid, 0, QStringLiteral("底色:"), QStringLiteral("HistoryBottomColor"), &m_draft.bottomColor); color(grid, 1, QStringLiteral("背景色:"), QStringLiteral("HistoryBackgroundColor"), &m_draft.backgroundColor);
    grid = new QGridLayout(channels); m_channel = new QComboBox(channels); m_channel->setObjectName(QStringLiteral("HistoryCurrentCurve")); grid->addWidget(new QLabel(QStringLiteral("当前通道:"), channels), 0, 0); grid->addWidget(m_channel, 0, 1);
    m_source = new QComboBox(channels); m_source->setObjectName(QStringLiteral("HistorySource")); Q_FOREACH (item, m_collection.items) { m_source->addItem(item.name, item.id); } grid->addWidget(new QLabel(QStringLiteral("数据来源:"), channels), 1, 0); grid->addWidget(m_source, 1, 1);
    m_lineStyle = new QComboBox(channels); m_lineStyle->setObjectName(QStringLiteral("HistoryLineStyle")); m_lineStyle->addItems({QStringLiteral("—0"), QStringLiteral("--1"), QStringLiteral("--2"), QStringLiteral("--3"), QStringLiteral("--4")}); grid->addWidget(new QLabel(QStringLiteral("线型:"), channels), 2, 0); grid->addWidget(m_lineStyle, 2, 1);
    m_channelColor = new QPushButton(channels); m_channelColor->setObjectName(QStringLiteral("HistoryCurveColor")); m_channelColor->setMinimumHeight(25); grid->addWidget(new QLabel(QStringLiteral("颜色:"), channels), 3, 0); grid->addWidget(m_channelColor, 3, 1);
    grid = new QGridLayout(range); m_minimum = new QLineEdit(range); m_minimum->setObjectName(QStringLiteral("HistoryCurveMinimum")); m_minimum->setMaxLength(80); m_maximum = new QLineEdit(range); m_maximum->setObjectName(QStringLiteral("HistoryCurveMaximum")); m_maximum->setMaxLength(80); grid->addWidget(new QLabel(QStringLiteral("最小值:"), range), 0, 0); grid->addWidget(m_minimum, 0, 1); grid->addWidget(new QLabel(QStringLiteral("最大值:"), range), 1, 0); grid->addWidget(m_maximum, 1, 1);
    connect(m_minimum, &QLineEdit::textChanged, this, [this](const QString &text) { m_draft.curves[m_draft.currentCurve].minimum = text; refresh(); }); connect(m_maximum, &QLineEdit::textChanged, this, [this](const QString &text) { m_draft.curves[m_draft.currentCurve].maximum = text; refresh(); }); connect(m_lineStyle, &QComboBox::currentIndexChanged, this, [this](int style) { m_draft.curves[m_draft.currentCurve].lineStyle = style; refresh(); });
    connect(m_source, &QComboBox::currentIndexChanged, this, [this](int index) { if (index >= 0) { m_draft.curves[m_draft.currentCurve].sourceItemId = m_source->itemData(index).toString(); refresh(); } });
    connect(m_channelColor, &QPushButton::clicked, this, [this]() { QColor selected = QColorDialog::getColor(QColor(m_draft.curves[m_draft.currentCurve].color), this, QStringLiteral("选择颜色"), QColorDialog::DontUseNativeDialog); if (selected.isValid()) { m_draft.curves[m_draft.currentCurve].color = selected.name(); showChannel(m_draft.currentCurve); } });
    connect(m_count, &QSpinBox::valueChanged, this, &HistoryTrendDialog::changeCount); connect(m_channel, &QComboBox::currentIndexChanged, this, &HistoryTrendDialog::showChannel); changeCount(m_draft.curveCount); return page;''',returns='General history page.')
s += documented('HistoryTrendDialog::axisPage','Keep axis limits visible while the native checkable mark/time groups hide only their own fields.', 'QWidget *HistoryTrendDialog::axisPage(bool y)', '''    TrendAxis *axis = y ? &m_draft.yAxis : &m_draft.xAxis;
    QString prefix = y ? QStringLiteral("HistoryY") : QStringLiteral("HistoryX");
    QGroupBox *page = new QGroupBox(y ? QStringLiteral("Y轴") : QStringLiteral("X轴"), this);
    QGridLayout *layout = new QGridLayout(page);
    QWidget *base = new QWidget(page);
    QGridLayout *grid = new QGridLayout(base);
    QGroupBox *marks = new QGroupBox(y ? QStringLiteral("刻度") : QStringLiteral("显示时间"), page);
    QWidget *fields = new QWidget(marks);
    QGridLayout *markGrid = new QGridLayout(fields);
    QVBoxLayout *markLayout = new QVBoxLayout(marks);
    QComboBox *font = new QComboBox(fields);
    QCheckBox *ticks;
    QPushButton *axisColor;
    QGroupBox *time;
    QGridLayout *timeGrid;
    toggle(grid, 0, y ? QStringLiteral("显示X轴曲线") : QStringLiteral("显示Y轴曲线"), prefix + QStringLiteral("Grid"), &axis->grid); ticks = toggle(grid, 1, QStringLiteral("显示刻度"), prefix + QStringLiteral("Ticks"), &axis->ticks);
    integer(grid, 2, QStringLiteral("主分隔数:"), prefix + QStringLiteral("Major"), &axis->major, 3, 10); integer(grid, 3, QStringLiteral("次分隔数:"), prefix + QStringLiteral("Minor"), &axis->minor, 1, 3); axisColor = color(grid, 4, QStringLiteral("轴/刻度颜色:"), prefix + QStringLiteral("AxisColor"), &axis->axisColor); color(grid, 5, QStringLiteral("格线颜色:"), prefix + QStringLiteral("GridColor"), &axis->gridColor); axisColor->setVisible(axis->ticks); connect(ticks, &QCheckBox::toggled, axisColor, &QWidget::setVisible);
    if (y) { textField(grid, 6, QStringLiteral("最大值:"), QStringLiteral("HistoryYMaximum"), &m_draft.yMaximum); textField(grid, 7, QStringLiteral("最小值:"), QStringLiteral("HistoryYMinimum"), &m_draft.yMinimum); }
    marks->setObjectName(prefix + QStringLiteral("Marks")); marks->setCheckable(true); marks->setChecked(axis->marks); markLayout->addWidget(fields); fields->setVisible(axis->marks); connect(marks, &QGroupBox::toggled, this, [this, axis, fields](bool checked) { axis->marks = checked; fields->setVisible(checked); refresh(); });
    font->setObjectName(prefix + QStringLiteral("FontSize")); font->addItems({QStringLiteral("12"), QStringLiteral("16"), QStringLiteral("24"), QStringLiteral("48")}); font->setCurrentText(QString::number(axis->fontSize)); markGrid->addWidget(new QLabel(QStringLiteral("字体大小:"), fields), 0, 0); markGrid->addWidget(font, 0, 1); connect(font, &QComboBox::currentTextChanged, this, [this, axis](const QString &value) { axis->fontSize = value.toInt(); refresh(); });
    if (y) { integer(markGrid, 1, QStringLiteral("总位数:"), QStringLiteral("HistoryTotalDigits"), &m_draft.totalDigits, 1, 10); integer(markGrid, 2, QStringLiteral("小数位数:"), QStringLiteral("HistoryFractionalDigits"), &m_draft.fractionalDigits, 0, 3); }
    else { choice(markGrid, 1, QStringLiteral("格式:"), QStringLiteral("HistoryTimeFormat"), {QStringLiteral("HH:MM"), QStringLiteral("HH:MM:SS")}, &m_draft.timeFormat); }
    layout->addWidget(base, 0, 0); layout->addWidget(marks, 0, 1, Qt::AlignTop); layout->setColumnStretch(0, 2); layout->setColumnStretch(1, 1);
    if (!y) { time = new QGroupBox(QStringLiteral("时间范围"), page); timeGrid = new QGridLayout(time); integer(timeGrid, 0, QStringLiteral("小时:"), QStringLiteral("HistoryHours"), &m_draft.hours, 0, 48); integer(timeGrid, 1, QStringLiteral("分钟:"), QStringLiteral("HistoryMinutes"), &m_draft.minutes, 0, 59); integer(timeGrid, 2, QStringLiteral("秒:"), QStringLiteral("HistorySeconds"), &m_draft.seconds, 0, 59); layout->addWidget(time, 1, 0, 1, 2); }
    return page;''', inputs='y', returns='Independent X or Y axis group.')
# Reuse the audited detached bindings, permission transaction and fixed renderer with history-specific fields.
tail=trend[trend.index('/******************************************************************************\n * Function Name   : TrendDialog::displayPage'):]
tail=tail.replace('TrendDialog','HistoryTrendDialog').replace('paintTrend','paintHistoryTrend').replace('TrendProperties','HistoryTrendProperties')
tail=tail.replace('QStringLiteral("trend")','QStringLiteral("historytrend")').replace('object.trend','object.historyTrend').replace('m_draft(object.historyTrend)','m_draft(object.historyTrend)')
tail=tail.replace('candidate.trend','candidate.historyTrend')
tail=tail.replace('QStringLiteral("Trend','QStringLiteral("History').replace('趋势图','历史趋势图').replace('currentChannel','currentCurve').replace('channelCount','curveCount').replace('channels[','curves[').replace('TrendChannel()','HistoryTrendCurve()')
tail=tail.replace('QHBoxLayout *layout = new QHBoxLayout(page);','QVBoxLayout *layout = new QVBoxLayout(page);')
tail=tail.replace('m_draft.curves[index] = HistoryTrendCurve();','m_draft.curves[index] = HistoryTrendCurve(); m_draft.curves[index].sourceItemId = m_collection.items.first().id;')
# Restore the selected stable source without emitting a write to a different curve.
tail=tail.replace('    QSignalBlocker minimum(m_minimum);','    QSignalBlocker source(m_source);\n    QSignalBlocker minimum(m_minimum);')
tail=tail.replace('m_draft.currentCurve = channel; m_minimum','m_draft.currentCurve = channel; m_source->setCurrentIndex(m_source->findData(m_draft.curves[channel].sourceItemId)); m_minimum')
tail=tail.replace('!candidate.historyTrend.validateAddresses(m_project.connections, &error)','!candidate.historyTrend.validateCollection(m_collection, &error) || !candidate.historyTrend.validateAddresses(m_project.connections, &error)')
tail=tail.replace('properties.frameColor','properties.yAxis.axisColor').replace('properties.rightToLeft ? 11 - index : index','index')
tail=tail.replace('QString::number(properties.samplePoints * index / properties.xAxis.major)','QStringLiteral("%1:%2%3").arg((properties.hours * 3600 + properties.minutes * 60 + properties.seconds) * index / properties.xAxis.major / 3600, 2, 10, QLatin1Char(\'0\')).arg(((properties.hours * 3600 + properties.minutes * 60 + properties.seconds) * index / properties.xAxis.major / 60) % 60, 2, 10, QLatin1Char(\'0\')).arg(properties.timeFormat == 0 ? QString() : QStringLiteral(":%1").arg(((properties.hours * 3600 + properties.minutes * 60 + properties.seconds) * index / properties.xAxis.major) % 60, 2, 10, QLatin1Char(\'0\')))')
tail=tail.replace('    PlcConnection connection;\n    m_draft.currentCurve', '''    PlcConnection connection;
    HmiConfigurationNode node;
    QString error;
    int index;
    Q_FOREACH (node, project.configurationNodes)
    {
        if (node.kind == QStringLiteral("dataCollection") && node.implemented && node.active && (m_draft.collectionId.isEmpty() || node.id == m_draft.collectionId) && DataCollectionProperties::fromJson(node.properties, &m_collection, &error))
        {
            m_collectionName = node.name;
            if (m_draft.collectionId.isEmpty()) { m_draft.collectionId = node.id; m_draft.curveCount = qMin(8, int(m_collection.items.size())); for (index = 0; index < 8; ++index) { m_draft.curves[index].sourceItemId = m_collection.items.first().id; } }
            break;
        }
    }
    m_draft.currentCurve''')
tail=tail.replace('resize(795, 584)','resize(805, 650)').replace('QStringLiteral("通道设置")','QStringLiteral("一般")').replace('QStringLiteral("显示设置")','QStringLiteral("XY轴")')
tail=tail.replace('设置触发、读取和清除地址，逐通道配置范围、线型、颜色及坐标轴。预览使用固定示意数据。','选择数据采集和逐曲线数据来源，配置独立范围、线型、颜色与时间轴。预览使用固定示意数据。')
tail=tail.replace('int(HistoryTrendProperties::wordCount(m_draft.dataType)) * m_draft.curveCount','1')
s+=tail
(root/'src/history_trend_dialog.cpp').write_text(s,encoding='utf-8',newline='\n')
# Fill the previously drafted collection editor helpers from their reviewed counterparts.
p=root/'src/data_collection_dialog.cpp'; s=p.read_text(encoding='utf-8')
for function in ('textField','toggle','integer'):
 block=re.search(r'/\*\*[^*]*?(?:\*(?!/)[^*]*)*?Function Name   : TrendDialog::'+function+r'\n.*?(?=\n/\*\*|\Z)',trend,re.S)
 # Select by the function-name marker, then include its preceding documentation block only.
 start=trend.rfind('/******************************************************************************',0,trend.index(' * Function Name   : TrendDialog::'+function))
 end=trend.find('/******************************************************************************',start+5)
 piece=trend[start:end if end>=0 else len(trend)].replace('TrendDialog::','DataCollectionDialog::').replace('int(TrendProperties::wordCount(m_draft.dataType)) * m_draft.channelCount','int(m_draft.wordCount()) * m_draft.sampleLength')
 s+='\n'+piece
s=s.replace('#include <QGridLayout>','#include <QGridLayout>\n#include <QGroupBox>')
s=s.replace('if (object.type != QStringLiteral("historytrend")','if (object.type != QStringLiteral("historytrend")')
# Even a curve bound to the first item cannot outlive a shorter item-count limit.
s=s.replace('for (index = 0; index < object.historyTrend.curveCount;', 'if (object.historyTrend.curveCount > count) { return true; }\n            for (index = 0; index < object.historyTrend.curveCount;')
p.write_text(s,encoding='utf-8',newline='\n')
p=root/'src/history_trend_properties.cpp';s=p.read_text(encoding='utf-8').replace('qMin(8, collection.items.size())','qMin(8, int(collection.items.size()))');p.write_text(s,encoding='utf-8',newline='\n')
p=root/'src/history_trend_properties.h';s=p.read_text(encoding='utf-8').replace('backgroundColor = QStringLiteral("#ffffff")','backgroundColor = QStringLiteral("#f0f0f0")');p.write_text(s,encoding='utf-8',newline='\n')
print('Completed transactional collection editor and three-page history editor; integration is the next step.')
