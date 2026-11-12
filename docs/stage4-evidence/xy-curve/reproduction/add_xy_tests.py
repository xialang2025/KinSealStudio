"""Register C25-specific tests for two numeric axes, paired spans and an independent typed family."""
from pathlib import Path
import ast
root=Path(__file__).resolve().parent/'worktree'
p=root/'src/static_text_actions.cpp';s=p.read_text(encoding='utf-8').replace('QStringLiteral("historytrend" ||','QStringLiteral("historytrend") ||');p.write_text(s,encoding='utf-8',newline='\n')
p=root/'src/project_model.h';s=p.read_text(encoding='utf-8').replace('XyCurveProperties xyCurve; //20261113-1 add: Separate bar, meter and donut drafts.','XyCurveProperties xyCurve; //20261113-1 add: Paired-coordinate sampling with two independent numeric axes.');p.write_text(s,encoding='utf-8',newline='\n')
p=root/'tests/stage4_controls_tests.cpp';s=p.read_text(encoding='utf-8').replace('#include "history_trend_dialog.h"','#include "xy_curve_dialog.h" //20261113-1 add\n#include "history_trend_dialog.h"')
tree=ast.parse((root.parent/'add_trend_tests.py').read_text(encoding='utf-8'));tests=ast.literal_eval(next(n.value for n in tree.body if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='tests' for t in n.targets)))
def head(name,description):return f'/******************************************************************************\n * Function Name   : {name}\n * Description     : {description}\n * Input Parameters: None\n * Return Value    : None.\n * Date            : 2026.11.13\n ******************************************************************************/\n'
addition=head('xyObject','Create an independent paired-coordinate fixture without generic operation-address aliases.')+'''static HmiObject xyObject() //20261113-1 add
{
    HmiObject object;
    object.id = QStringLiteral("xy-object"); object.type = QStringLiteral("xycurve"); object.name = QStringLiteral("X-Y曲线"); object.geometry = QRect(100, 100, 240, 160); return object;
}

'''
for name,desc,body in tests:
 name=name.replace('trend','xy',1);body=body.replace('TrendDialog','XyCurveDialog').replace('TrendProperties','XyCurveProperties').replace('paintTrend','paintXyCurve').replace('trendObject','xyObject').replace('.trend','.xyCurve').replace('"trend"','"xycurve"').replace('"Trend','"Xy').replace('趋势图','X-Y曲线').replace('action_trendchart','action_X_Y').replace('trend-','xy-')
 # Independent X and Y marks must hide their whole own numeric/digit block.
 if name=='xyAxisHiddenFieldsAndAccessSpans':
  body=body.replace('"XyTotalDigits"','"XyYTotalDigits"').replace('"XyFractionalDigits"','"XyYFractionalDigits"').replace('"LW65521"','"LW4066"').replace('"LW65520"','"LW4065"')
  body=body.replace('options.columns = 2;','options.columns = 2;').replace('Stage3AddressField::Operation, 16, 0','Stage3AddressField::Operation, 32, 0').replace('QStringLiteral("LW16")','QStringLiteral("LW32")')
  body=body.replace('    object.xyCurve.dataType = 1;', '''    dialog.findChild<QLineEdit *>(QStringLiteral("XyXMinimum"))->setText(QStringLiteral("-100")); dialog.findChild<QLineEdit *>(QStringLiteral("XyXMaximum"))->setText(QStringLiteral("4000")); dialog.findChild<QSpinBox *>(QStringLiteral("XyXFractionalDigits"))->setValue(2); dialog.findChild<QCheckBox *>(QStringLiteral("XyXMarks"))->setChecked(false); QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("XyXMaximum"))->isVisible()); QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("XyYMaximum"))->isVisible()); dialog.findChild<QCheckBox *>(QStringLiteral("XyXMarks"))->setChecked(true); QCOMPARE(dialog.object().xyCurve.xMaximum, QStringLiteral("4000")); dialog.findChild<QCheckBox *>(QStringLiteral("XyXGrid"))->setChecked(false); QVERIFY(!dialog.findChild<QPushButton *>(QStringLiteral("XyXGridColor"))->isVisible()); dialog.findChild<QCheckBox *>(QStringLiteral("XyXGrid"))->setChecked(true);
    object.xyCurve.dataType = 1;''')
 if name=='xyStrictPersistenceAndBackendGates':
  body=body.replace('    object.xyCurve.channelCount = 3;', '    object.xyCurve.xMinimum = QStringLiteral("-100"); object.xyCurve.xMaximum = QStringLiteral("4000"); object.xyCurve.xTotalDigits = 6; object.xyCurve.xFractionalDigits = 2;\n    object.xyCurve.channelCount = 3;')
  body=body.replace('    HmiProject project;','    HmiProject project = historyProject();\n    HmiObject history = project.objects.first();',1)
  body=body.replace('chartObject(), object','chartObject(), trendObject(), history, object').replace('loaded.objects.size(), 23','loaded.objects.size(), 25')
  body=body.replace('QStringLiteral("xycurve")','QStringLiteral("xycurve")')
  body=body.replace('    json = object.xyCurve.toJson();','    QVERIFY(!object.xyCurve.toJson().contains(QStringLiteral("rightToLeft")));\n    json = object.xyCurve.toJson();',1)
  # The property carrier is independent even though the shared lower-level axis/channel structs are reused.
  body=body.replace('json = object.xyCurve.toJson(); json.insert(QStringLiteral("unknown"), false);','json = object.xyCurve.toJson(); json.insert(QStringLiteral("rightToLeft"), false);')
 if name=='xyStaticRasterAndClipping':
  body=body.replace('properties.rightToLeft = true;','properties.xMaximum = QStringLiteral("4000"); properties.xFractionalDigits = 2;')
 # The original authoring fixture used an over-wide local address; the new span test uses actual LW0..4096 bounds.
 body=body.replace('only.save(QDir','only.save(QDir')
 s=s.replace('    void collectionSharedTypeMemoryAndFullAddressSpan();','    void '+name+'(); //20261113-1 add\n    void collectionSharedTypeMemoryAndFullAddressSpan();',1)
 addition+=head('Stage4ControlTests::'+name,desc.replace('trend','X/Y').replace('23 families','25 families').replace('deterministic direction','independent numeric axis labels'))+'void Stage4ControlTests::'+name+'() //20261113-1 add\n{\n'+body+'}\n\n'
s=s.replace('QTEST_MAIN(Stage4ControlTests)',addition+'QTEST_MAIN(Stage4ControlTests)')
p.write_text(s,encoding='utf-8',newline='\n')
print('Registered six C25 tests before building; paired 32-word boundaries use LW4065/4066.')
