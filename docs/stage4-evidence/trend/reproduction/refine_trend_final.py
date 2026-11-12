"""Use real local address endpoints and scale preview text without changing the canvas pixel-font contract."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
p=root/'tests/stage4_controls_tests.cpp'; s=p.read_text(encoding='utf-8')
a=s.index('void Stage4ControlTests::trendAxisHiddenFieldsAndAccessSpans()'); b=s.index('/******************************************************************************',a)
part=s[a:b].replace('LW65521','LW4082').replace('LW65520','LW4081').replace('QVERIFY(object.trend.validateAddresses(project.connections, &error));','QVERIFY2(object.trend.validateAddresses(project.connections, &error), qPrintable(error));')
s=s[:a]+part+s[b:]
a=s.index('void Stage4ControlTests::chartAccessSpansAndAddressCopies()'); b=s.index('/******************************************************************************',a)
s=s[:a]+s[a:b].replace('LW65535','LW4096')+s[b:]
a=s.index('void Stage4ControlTests::trendNativeBoundsAndDataTypeLifetime()'); b=s.index('/******************************************************************************',a)
part=s[a:b].replace('    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR"))', '    type->setCurrentIndex(0);\n    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR"))')
s=s[:a]+part+s[b:]
p.write_text(s,encoding='utf-8',newline='\n')
p=root/'src/trend_dialog.cpp'; s=p.read_text(encoding='utf-8').replace('qMin(32.0, rect.width() * .2)','qMin(40.0, rect.width() * .2)')
a=s.index('void TrendDialog::refresh()'); b=s.index('/******************************************************************************',a)
part=s[a:b].replace('QImage image(130, 155,','QImage image(260, 310,').replace('QRectF(5, 5, 120, 145)','QRectF(10, 10, 240, 290)').replace('QPixmap::fromImage(image)','QPixmap::fromImage(image).scaled(130, 155, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)')
s=s[:a]+part+s[b:]
p.write_text(s,encoding='utf-8',newline='\n')
