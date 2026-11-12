"""Build distinct C25 typed drafts with two numeric axes and a conservative paired-coordinate span."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
def save(path,s):(root/path).write_text(s.replace('//20261112-1','//20261113-1').replace('2026.11.12','2026.11.13'),encoding='utf-8',newline='\n')
s=(root/'src/trend_properties.h').read_text(encoding='utf-8');start=s.index('struct TrendProperties');s='#ifndef XY_CURVE_PROPERTIES_H\n#define XY_CURVE_PROPERTIES_H\n#include "trend_properties.h"\n\n//20261113-1 add: X/Y coordinate drafts are distinct from timed trend channels.\n'+s[start:]
s=s.replace('TrendProperties','XyCurveProperties').replace('    bool rightToLeft = false;\n','').replace('    QString yMinimum', '    QString xMinimum = QStringLiteral("0");\n    QString xMaximum = QStringLiteral("1000");\n    int xTotalDigits = 4;\n    int xFractionalDigits = 0;\n    QString yMinimum').replace('static unsigned int wordCount','static u32 wordCount')
save('src/xy_curve_properties.h',s)
s=(root/'src/trend_properties.cpp').read_text(encoding='utf-8');a=s.rfind('/******************************************************************************',0,s.index(' * Function Name   : TrendChannel::toJson'));b=s.rfind('/******************************************************************************',0,s.index(' * Function Name   : TrendProperties::toJson'));s=s[:a]+s[b:]
s=s.replace('"trend_properties.h"','"xy_curve_properties.h"').replace('TrendProperties','XyCurveProperties').replace('trendScalar','xyScalar').replace('unsigned int XyCurveProperties::wordCount','u32 XyCurveProperties::wordCount').replace('趋势图','X-Y曲线')
s=s.replace('context.transferCount = wordCount(dataType) * channelCount;','context.transferCount = wordCount(dataType) * u32(channelCount) * 2U; // Conservative full X/Y-pair guard; firmware ordering remains unverified.')
s=s.replace('totalDigits < 1 ||','xTotalDigits < 1 || xTotalDigits > 10 || xFractionalDigits < 0 || xFractionalDigits > 3 || totalDigits < 1 ||',1)
s=s.replace('    if (!xyScalar(yMinimum, 6)', '    if (!xyScalar(xMinimum, 6) || !xyScalar(xMaximum, 6) || xMinimum.toDouble() >= xMaximum.toDouble()) { if (error != nullptr) { *error = QStringLiteral("本版要求X刻度范围有效且最小值小于最大值。"); } return false; }\n    if (!xyScalar(yMinimum, 6)')
s=s.replace('    json.insert(QStringLiteral("rightToLeft"), rightToLeft);\n','').replace('    candidate.rightToLeft = json.value(QStringLiteral("rightToLeft")).toBool();\n','')
for name,typ in [('xMinimum','String'),('xMaximum','String'),('xTotalDigits','Int'),('xFractionalDigits','Int')]:
 s=s.replace('    json.insert(QStringLiteral("yMinimum"), yMinimum);',f'    json.insert(QStringLiteral("{name}"), {name});\n    json.insert(QStringLiteral("yMinimum"), yMinimum);')
 s=s.replace('    candidate.yMinimum =',f'    candidate.{name} = json.value(QStringLiteral("{name}")).to{typ}();\n    candidate.yMinimum =')
save('src/xy_curve_properties.cpp',s)
s=(root/'src/trend_dialog.h').read_text(encoding='utf-8').replace('TREND_DIALOG_H','XY_CURVE_DIALOG_H').replace('TrendDialog','XyCurveDialog').replace('TrendProperties','XyCurveProperties').replace('paintTrend','paintXyCurve');save('src/xy_curve_dialog.h',s)
s=(root/'src/trend_dialog.cpp').read_text(encoding='utf-8').replace('"trend_dialog.h"','"xy_curve_dialog.h"').replace('TrendDialog','XyCurveDialog').replace('TrendProperties','XyCurveProperties').replace('paintTrend','paintXyCurve').replace('QStringLiteral("trend")','QStringLiteral("xycurve")').replace('object.trend','object.xyCurve').replace('candidate.trend','candidate.xyCurve').replace('QStringLiteral("Trend','QStringLiteral("Xy').replace('趋势图','X-Y曲线')
s=s.replace('    QGroupBox *direction = new QGroupBox(page);\n','').replace('    QRadioButton *left = new QRadioButton(QStringLiteral("从左到右"), direction);\n','').replace('    QRadioButton *right = new QRadioButton(QStringLiteral("从右到左"), direction);\n','').replace(' layout->addWidget(direction, 2, 1);','')
start=s.index('    grid = new QGridLayout(direction);');end=s.index('\n}',start);s=s[:start]+'    return page;'+s[end:]
# Both mark blocks own all their numeric range and digit fields. Grid-color visibility is independent of tick visibility.
s=s.replace('    QCheckBox *showMarks;','    QCheckBox *showMarks;\n    QCheckBox *gridToggle;\n    QWidget *gridColor = new QWidget(page);\n    QGridLayout *gridColorLayout = new QGridLayout(gridColor);')
s=s.replace('    toggle(base, 1, y ?', '    gridToggle = toggle(base, 1, y ?')
s=s.replace('    base = new QGridLayout; color(base, 0, QStringLiteral("格线颜色:"), prefix + QStringLiteral("GridColor"), &axis->gridColor); integer(base, 1,','    color(gridColorLayout, 0, QStringLiteral("格线颜色:"), prefix + QStringLiteral("GridColor"), &axis->gridColor); gridColor->setVisible(axis->grid); layout->addWidget(gridColor); connect(gridToggle, &QCheckBox::toggled, gridColor, &QWidget::setVisible);\n    base = new QGridLayout; integer(base, 1,')
old='    if (y) { textField(markGrid, 1, QStringLiteral("最小值:"), QStringLiteral("XyYMinimum"), &m_draft.yMinimum); textField(markGrid, 2, QStringLiteral("最大值:"), QStringLiteral("XyYMaximum"), &m_draft.yMaximum); integer(markGrid, 3, QStringLiteral("总位数:"), QStringLiteral("XyTotalDigits"), &m_draft.totalDigits, 1, 10); integer(markGrid, 4, QStringLiteral("小数位数:"), QStringLiteral("XyFractionalDigits"), &m_draft.fractionalDigits, 0, 3); }'
assert old in s
new='    textField(markGrid, 1, QStringLiteral("最小值:"), prefix + QStringLiteral("Minimum"), y ? &m_draft.yMinimum : &m_draft.xMinimum); textField(markGrid, 2, QStringLiteral("最大值:"), prefix + QStringLiteral("Maximum"), y ? &m_draft.yMaximum : &m_draft.xMaximum); integer(markGrid, 3, QStringLiteral("总位数:"), prefix + QStringLiteral("TotalDigits"), y ? &m_draft.totalDigits : &m_draft.xTotalDigits, 1, 10); integer(markGrid, 4, QStringLiteral("小数位数:"), prefix + QStringLiteral("FractionalDigits"), y ? &m_draft.fractionalDigits : &m_draft.xFractionalDigits, 0, 3);'
s=s.replace(old,new).replace('int(XyCurveProperties::wordCount(m_draft.dataType)) * m_draft.channelCount','int(XyCurveProperties::wordCount(m_draft.dataType)) * m_draft.channelCount * 2')
s=s.replace('properties.rightToLeft ? 11 - index : index','index')
s=s.replace('QString::number(properties.samplePoints * index / properties.xAxis.major)','QString::number((properties.xMinimum.toDouble() + (properties.xMaximum.toDouble() - properties.xMinimum.toDouble()) * index / properties.xAxis.major) / std::pow(10.0, properties.xFractionalDigits), \'f\', properties.xFractionalDigits)')
s=s.replace('fixed editor curve','fixed editor X/Y illustration').replace('without exposing native hidden X numeric fields','with two independent numeric axis ranges').replace('sampling carriers','paired-coordinate carriers')
s=s.replace('Reset removed channels as observed natively','Reset removed X/Y drafts by Lite policy')
s=s.replace('设置触发、读取和清除地址，逐通道配置范围、线型、颜色及坐标轴。预览使用固定示意数据。','设置触发/清除位和成对坐标读取地址，逐通道配置范围及两套数值坐标轴。预览使用固定示意数据。')
save('src/xy_curve_dialog.cpp',s)
print('Created distinct X/Y properties and three-page editor without the time-trend direction field.')
