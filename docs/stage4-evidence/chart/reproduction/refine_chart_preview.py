"""Use the observed zero-valued chart preview and cover nondefault inactive draft persistence."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
p=root/'src/chart_dialog.cpp'
s=p.read_text(encoding='utf-8')
s=s.replace('qMin(rect.width(), rect.height()) * .43','qMin(rect.width(), rect.height()) * .30')
s=s.replace('        {\n            angle = (start - span * index / divisions)', '        {\n            if (index == divisions && span == 360) { continue; } // Skip the duplicate full-circle endpoint label.\n            angle = (start - span * index / divisions)')
s=s.replace('std::cos(angle) * radius * .7','std::cos(angle) * radius * 1.4').replace('std::sin(angle) * radius * .7','std::sin(angle) * radius * 1.4')
s=s.replace('painter->translate(rect.center()); painter->rotate(meter.direction == 0 ? 20 : -20);', 'painter->translate(rect.center());')
s=s.replace('qMin(rect.width(), rect.height()) / 2 - qMax(donut.lineWidth, donut.fillWidth)', 'qMin(rect.width(), rect.height()) * .30 - qMax(donut.lineWidth, donut.fillWidth) / 2.0')
p.write_text(s,encoding='utf-8',newline='\n')
p=root/'tests/stage4_controls_tests.cpp'
s=p.read_text(encoding='utf-8')
a=s.index('void Stage4ControlTests::chartStrictPersistenceAndBackendGates()')
b=s.index('/******************************************************************************',a)
part=s[a:b]
part=part.replace('    project.objects = existingFiveObjects();', '    object.chart.shape = 2; object.chart.bar.minimum = QStringLiteral("7"); object.chart.meter.address = QStringLiteral("LW20"); object.chart.donut.arc = true; object.chart.donut.spanAngle = QStringLiteral("270");\n    project.objects = existingFiveObjects();')
part=part.replace('parsed.shape = 2;','parsed.shape = 1;').replace('QCOMPARE(parsed.shape, 2);','QCOMPARE(parsed.shape, 1);')
s=s[:a]+part+s[b:]
p.write_text(s,encoding='utf-8',newline='\n')
