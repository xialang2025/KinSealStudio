"""Reuse verified common cases as data, and replace the rectangle-only raster oracle with true ellipse checks."""
from pathlib import Path
import ast
root=Path(__file__).resolve().parent;work=root/'worktree';p=work/'tests/stage4_controls_tests.cpp';s=p.read_text(encoding='utf-8')
assert 'ellipseNativeFieldsAndTypedDrafts' not in s
tree=ast.parse((root/'add_rect_tests.py').read_text(encoding='utf-8'))
methods=[]
for node in tree.body:
    if not isinstance(node,ast.Expr) or not isinstance(node.value,ast.Call) or not isinstance(node.value.func,ast.Attribute) or not isinstance(node.value.func.value,ast.Name) or node.value.func.value.id!='methods' or node.value.func.attr!='append':continue
    name,description,body=ast.literal_eval(node.value.args[0])
    name=name.replace('rect','ellipse',1);description=description.replace('rectangle','ellipse').replace('rectangles','ellipses')
    body=body.replace('QStringLiteral("rect")','QStringLiteral("ellipse")').replace('QStringLiteral("action_rect")','QStringLiteral("action_ellipse")').replace('矩形','圆/椭圆').replace('rect-','ellipse-').replace('stage4-evidence/rect/native/', 'stage4-evidence/ellipse/native/')
    if name=='ellipseStrictPersistenceMixedCopyAndNativeRefusal':
        before='project.objects.append(object); project.objects.append(existingFiveObjects());'
        after='project.objects.append(object); object.type = QStringLiteral("rect"); object.id = QStringLiteral("mixed-rect"); object.name = QStringLiteral("矩形"); object.graphic = GraphicProperties(); project.objects.append(object); project.objects.append(existingFiveObjects());'
        assert body.count(before)==1;body=body.replace(before,after,1).replace('loaded.objects.size(),7','loaded.objects.size(),8')
    if name=='ellipseDefaultNativePixelsAndIndependentFillPatterns':
        name='ellipseCurvedRasterFillAndNativeComparisonInput'
        description='Verify genuine elliptical clipping and circle geometry while retaining raw raster output for an honest native RGB565 comparison.'
        body=body.replace('sheet-ellipse-default-r1.png','sheet-ellipse-empty-default-r2.png').replace('ellipse-native-roi-match.png','ellipse-default-raster.png')
        a='QCOMPARE(image.copy(roi),native.convertToFormat(QImage::Format_ARGB32).copy(roi));'
        b='QCOMPARE(image.pixelColor(200,150),QColor(Qt::white)); QCOMPARE(image.pixelColor(110,110),QColor(197,222,255)); QVERIFY(image.pixelColor(200,100)!=QColor(197,222,255)); QVERIFY(!graphicPath(QStringLiteral("ellipse"),QRectF(100,100,200,100),properties).contains(QPointF(110,110))); QVERIFY(graphicPath(QStringLiteral("ellipse"),QRectF(100,100,200,100),properties).contains(QPointF(200,150)));'
        assert body.count(a)==1;body=body.replace(a,b,1)
        body=body.replace('    QRect roi(90,90,221,121);\n','')
        a='    QCOMPARE(hashes.size(),13);'
        b='    QVERIFY(!graphicHitPath(QStringLiteral("ellipse"),QRectF(20,20,200,100),properties).contains(QPointF(20,20))); QCOMPARE(graphicPath(QStringLiteral("ellipse"),QRectF(20,20,100,100),properties).boundingRect(),QRectF(20,20,100,100));\n'+a
        assert body.count(a)==1;body=body.replace(a,b,1)
    methods.append((name,description,body))
assert len(methods)==5
s=s.replace('    void initTestCase();',''.join('    void '+name+'();\n' for name,_,_ in methods)+'    void initTestCase();',1)
definitions=''
for name,description,body in methods:
    definitions+='\n/******************************************************************************\n * Function Name   : Stage4ControlTests::'+name+'\n * Description     : '+description+'\n * Input Parameters: None\n * Return Value    : None.\n * Date            : 2026.11.13\n ******************************************************************************/\nvoid Stage4ControlTests::'+name+'() //20261113-2 add\n{\n'+body+'}\n'
s=s.replace('QTEST_MAIN(Stage4ControlTests)',definitions+'\nQTEST_MAIN(Stage4ControlTests)',1);p.write_text(s,encoding='utf-8',newline='\n')
print('Added five ellipse cases, including true path curvature and eight-type mixed persistence.')
