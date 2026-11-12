"""Add rounded-rectangle range, radius persistence, geometry and actual-dialog regressions once."""
from pathlib import Path
import ast

root = Path(__file__).resolve().parent
path = root / 'worktree/tests/stage4_controls_tests.cpp'
source = path.read_text(encoding='utf-8')
assert 'roundrectNativeFieldsAndTypedDrafts' not in source
methods = []
for node in ast.parse((root / 'add_rect_tests.py').read_text(encoding='utf-8')).body:
    if not isinstance(node, ast.Expr) or not isinstance(node.value, ast.Call) or not isinstance(node.value.func, ast.Attribute) or not isinstance(node.value.func.value, ast.Name) or node.value.func.value.id != 'methods' or node.value.func.attr != 'append':
        continue
    name, description, body = ast.literal_eval(node.value.args[0])
    name = name.replace('rect', 'roundrect', 1)
    description = description.replace('rectangle', 'rounded rectangle')
    body = body.replace('QStringLiteral("rect")', 'QStringLiteral("roundrect")').replace('QStringLiteral("action_rect")', 'QStringLiteral("action_RoundedRect")').replace('矩形', '圆角矩形').replace('rect-', 'roundrect-').replace('stage4-evidence/rect/native/', 'stage4-evidence/roundrect/native/')
    for before, after in [('Pattern', 'm_Pattern'), ('Line_Type', 'm_LineType'), ('Line_Width', 'm_LineWidth'), ('Alpha', 'm_Alpha')]:
        body = body.replace('QStringLiteral("' + before + '")', 'QStringLiteral("' + after + '")')
    if name == 'roundrectNativeFieldsAndTypedDrafts':
        before = '    for (index=0;index<13;++index)'
        after = '    QCOMPARE(dialog->findChild<QSpinBox *>(QStringLiteral("m_XR"))->minimum(),0); QCOMPARE(dialog->findChild<QSpinBox *>(QStringLiteral("m_XR"))->maximum(),99); QCOMPARE(dialog->findChild<QSpinBox *>(QStringLiteral("m_XR"))->value(),10); QCOMPARE(dialog->findChild<QSpinBox *>(QStringLiteral("m_YR"))->value(),10); dialog->findChild<QSpinBox *>(QStringLiteral("m_XR"))->setValue(0); dialog->findChild<QSpinBox *>(QStringLiteral("m_YR"))->setValue(99); QCOMPARE(dialog->object().graphic.radiusX,0); QCOMPARE(dialog->object().graphic.radiusY,99);\n' + before
        body = body.replace(before, after, 1)
    if name == 'roundrectStrictPersistenceMixedCopyAndNativeRefusal':
        body = body.replace('object.graphic.fillStyle = 12;', 'object.graphic.radiusX = 0; object.graphic.radiusY = 99; object.graphic.fillStyle = 12;')
        body = body.replace('project.objects.append(object); project.objects.append(existingFiveObjects());', 'project.objects.append(object); object.type = QStringLiteral("rect"); object.id = QStringLiteral("mixed-rect"); project.objects.append(object); object.type = QStringLiteral("ellipse"); object.id = QStringLiteral("mixed-ellipse"); project.objects.append(object); project.objects.append(existingFiveObjects());').replace('loaded.objects.size(),7', 'loaded.objects.size(),9')
    if name == 'roundrectDefaultNativePixelsAndIndependentFillPatterns':
        name = 'roundrectDefaultRasterAndIndependentFillPatterns'
        description = 'Retain a real rounded path raster beside the native default without claiming uncalibrated edge equality.'
        body = body.replace('QCOMPARE(image.copy(roi),native.convertToFormat(QImage::Format_ARGB32).copy(roi));', 'QCOMPARE(image.pixelColor(200,150),QColor(Qt::white)); QCOMPARE(image.pixelColor(100,100),QColor(197,222,255)); QVERIFY(!graphicPath(QStringLiteral("roundrect"),QRectF(100,100,200,100),properties).contains(QPointF(100,100)));')
        body = body.replace('roundrect-native-roi-match.png', 'roundrect-default-raster.png').replace('    QRect roi(90,90,221,121);\n', '')
    methods.append((name, description, body))

methods.append(('roundrectRadiusDraftBoundsAndFamilyIsolation', 'Keep independent observed radius drafts through validation, resize and cancellation without leaking radius fields into other families.', r'''    HmiObject object;
    GraphicProperties parsed;
    GraphicProperties sentinel;
    QJsonObject json;
    GraphicDialog *dialog;
    QString error;
    object.type = QStringLiteral("roundrect"); object.geometry = QRect(100,100,200,100);
    object.graphic.radiusX = 99; object.graphic.radiusY = 30; json = object.graphic.toJson(object.type);
    QVERIFY(GraphicProperties::fromJson(json,&parsed,&error,object.type)); QCOMPARE(parsed.radiusX,99); QCOMPARE(parsed.radiusY,30); sentinel = parsed;
    json.insert(QStringLiteral("radiusX"),100); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,object.type)); QCOMPARE(parsed.toJson(object.type),sentinel.toJson(object.type));
    json = sentinel.toJson(object.type); json.insert(QStringLiteral("radiusY"),-1); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,object.type)); json.insert(QStringLiteral("radiusY"),1.5); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,object.type));
    QVERIFY(!sentinel.toJson(QStringLiteral("rect")).contains(QStringLiteral("radiusX"))); QVERIFY(!sentinel.toJson(QStringLiteral("ellipse")).contains(QStringLiteral("radiusY"))); QVERIFY(!GraphicProperties::fromJson(sentinel.toJson(object.type),&parsed,&error,QStringLiteral("rect")));
    dialog = new GraphicDialog(object); dialog->findChild<QSpinBox *>(QStringLiteral("m_XR"))->setValue(0); dialog->findChild<QSpinBox *>(QStringLiteral("m_YR"))->setValue(99); QCOMPARE(dialog->object().graphic.radiusX,0); QCOMPARE(dialog->object().graphic.radiusY,99); dialog->reject(); QCOMPARE(object.graphic.radiusX,99); QCOMPARE(object.graphic.radiusY,30); delete dialog;
    object.geometry = QRect(100,100,20,10); QVERIFY(object.graphic.validate(object.type)); QCOMPARE(object.graphic.radiusX,99); QCOMPARE(object.graphic.radiusY,30); QVERIFY(!graphicPath(object.type,object.geometry,object.graphic).isEmpty());
'''))
source = source.replace('    void initTestCase();', ''.join('    void ' + name + '();\n' for name, _, _ in methods) + '    void initTestCase();', 1)
definitions = ''
for name, description, body in methods:
    definitions += '\n/******************************************************************************\n * Function Name   : Stage4ControlTests::' + name + '\n * Description     : ' + description + '\n * Input Parameters: None\n * Return Value    : None.\n * Date            : 2026.11.13\n ******************************************************************************/\nvoid Stage4ControlTests::' + name + '() //20261113-2 add\n{\n' + body + '}\n'
source = source.replace('QTEST_MAIN(Stage4ControlTests)', definitions + '\nQTEST_MAIN(Stage4ControlTests)', 1)
path.write_text(source, encoding='utf-8', newline='\n')
print('Added six rounded-rectangle cases.')
