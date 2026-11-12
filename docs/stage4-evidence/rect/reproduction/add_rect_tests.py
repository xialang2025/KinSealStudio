"""Add rectangle-specific model, gestures, original raster and actual-dialog regressions once."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree';p=root/'tests/stage4_controls_tests.cpp';s=p.read_text(encoding='utf-8')
assert 'rectNativeFieldsAndTypedDrafts' not in s
if '#include <QCryptographicHash>' not in s:s='#include <QCryptographicHash>\n'+s
methods=[]
methods.append(('rectNativeFieldsAndTypedDrafts','Preserve three independent colors and all thirteen fills, with observed range guards and no active line-only fields.',r'''    HmiObject object;
    GraphicDialog *dialog;
    GraphicProperties parsed;
    GraphicProperties sentinel;
    QJsonObject json;
    QString error;
    int index;
    object.type = QStringLiteral("rect"); object.id = QStringLiteral("rect-fields"); object.name = QStringLiteral("矩形"); object.geometry = QRect(10,10,200,100); object.graphic.foreground = Qt::red; object.graphic.background = Qt::blue;
    dialog = new GraphicDialog(object); dialog->show(); QVERIFY(QTest::qWaitForWindowExposed(dialog)); QCOMPARE(dialog->windowTitle(),QStringLiteral("矩形")); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("Pattern"))->count(),13); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("Line_Type"))->count(),6); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("Line_Width"))->count(),9); QVERIFY(!dialog->findChild<QComboBox *>(QStringLiteral("End_Arrow"))->isVisible());
    for (index=0;index<13;++index) { dialog->findChild<QComboBox *>(QStringLiteral("Pattern"))->setCurrentIndex(index); QCOMPARE(dialog->object().graphic.fillStyle,index); QCOMPARE(dialog->object().graphic.foreground,QColor(Qt::red)); QCOMPARE(dialog->object().graphic.background,QColor(Qt::blue)); }
    dialog->findChild<QSpinBox *>(QStringLiteral("Alpha"))->setValue(128); QCOMPARE(dialog->object().graphic.alpha,128); json = dialog->object().graphic.toJson(object.type); QVERIFY(!json.contains(QStringLiteral("points"))); QVERIFY(!json.contains(QStringLiteral("endArrow"))); QVERIFY2(GraphicProperties::fromJson(json,&parsed,&error,object.type),qPrintable(error)); QCOMPARE(parsed.toJson(object.type),json); sentinel = parsed; json.insert(QStringLiteral("fillStyle"),13); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,object.type)); QCOMPARE(parsed.toJson(object.type),sentinel.toJson(object.type));
    json = sentinel.toJson(object.type); json.insert(QStringLiteral("endArrow"),1); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,object.type)); dialog->reject(); delete dialog;
'''))
methods.append(('rectStrictPersistenceMixedCopyAndNativeRefusal','Round-trip filled graphics independently of directed lines and known controls, while retaining source and device refusal.',r'''    HmiProject project;
    HmiProject loaded;
    HmiProject target;
    HmiProject result;
    HmiObject object;
    Stage3Clipboard clipboard;
    QString error;
    QByteArray source = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    object.type = QStringLiteral("rect"); object.id = QStringLiteral("rect-model"); object.name = QStringLiteral("矩形"); object.geometry = QRect(100,100,200,100); object.graphic.fillStyle = 12; object.graphic.foreground = Qt::red; object.graphic.background = Qt::blue; object.graphic.alpha = 128; object.graphic.lineWidth = 3; project.objects.append(object);
    QVERIFY2(HmiProject::fromJson(project.toJson(),&loaded,&error),qPrintable(error)); QCOMPARE(loaded.toJson(),project.toJson()); QCOMPARE(loaded.objects.first().graphic.background,QColor(Qt::blue)); QVERIFY(captureStage3Selection(project,{object.id},&clipboard,&error)); QVERIFY2(pasteStage3Selection(target,clipboard,QPoint(20,20),&result,nullptr,&error),qPrintable(error)); QCOMPARE(result.objects.first().graphic.toJson(object.type),object.graphic.toJson(object.type));
    QVERIFY(!exportNativeProjectSource(project,&source,&error)); QCOMPARE(source,QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(project.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("rect-only.ksp")),&error)); }
    object.type = QStringLiteral("line"); object.id = QStringLiteral("mixed-line"); object.name = QStringLiteral("直线"); object.graphic = GraphicProperties(); project.objects.append(object); project.objects.append(existingFiveObjects()); QVERIFY2(HmiProject::fromJson(project.toJson(),&loaded,&error),qPrintable(error)); QCOMPARE(loaded.toJson(),project.toJson()); QCOMPARE(loaded.objects.size(),7);
'''))
methods.append(('rectFourQuadrantsEightHandleResizeAndCancellation','Create normalized boxes in every drag quadrant, retain eight-handle resizing and reject uncommitted or zero-area geometry.',r'''    ScreenCanvas canvas;
    HmiObject draft;
    HmiObject original;
    QVector<QPointF> starts = {QPointF(100,100),QPointF(300,100),QPointF(300,200),QPointF(100,200)};
    QVector<QPointF> ends = {QPointF(300,200),QPointF(100,200),QPointF(100,100),QPointF(300,100)};
    QSignalSpy changed(&canvas,&ScreenCanvas::documentChanged);
    int index;
    draft.type = QStringLiteral("rect"); canvas.setProject(HmiProject()); canvas.resize(900,550); canvas.show(); QVERIFY(QTest::qWaitForWindowExposed(&canvas));
    for (index=0;index<4;++index) { canvas.beginControlPlacement(draft); QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(starts[index])); QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(ends[index])); QTest::mouseRelease(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(ends[index])); QCOMPARE(canvas.objects().last().geometry,QRect(100,100,200,100)); QCOMPARE(canvas.objects().last().type,QStringLiteral("rect")); }
    QCOMPARE(changed.count(),4); original=canvas.objects().last(); canvas.selectObject(original.id); QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(200,100))); QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(QPointF(200,80))); QTest::mouseRelease(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(200,80))); QCOMPARE(canvas.objects().last().geometry,QRect(100,80,200,120)); canvas.executeEdit(QStringLiteral("undo")); QCOMPARE(canvas.objects().last().toJson(),original.toJson());
    canvas.beginControlPlacement(draft); QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(10,10))); QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(QPointF(40,40))); QTest::keyClick(&canvas,Qt::Key_Escape); QCOMPARE(canvas.objects().size(),4); QVERIFY(!canvas.hasActiveInteraction());
    canvas.beginControlPlacement(draft); QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(10,10))); QTest::mouseRelease(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(10,50))); QCOMPARE(canvas.objects().size(),4); QVERIFY(!canvas.hasActiveInteraction());
'''))
methods.append(('rectDefaultNativePixelsAndIndependentFillPatterns','Compare the actual default native background ROI and render every fill while checking transparent compositing and hit geometry.',r'''    GraphicProperties properties;
    QImage image(800,480,QImage::Format_ARGB32);
    QImage native(QFINDTESTDATA("../docs/stage4-evidence/rect/native/sheet-rect-default-r1.png"));
    QImage pixels(260,160,QImage::Format_ARGB32);
    QPainter painter;
    QSet<QByteArray> hashes;
    QRect roi(90,90,221,121);
    int index;
    QVERIFY(!native.isNull()); image.fill(QColor(197,222,255)); painter.begin(&image); paintGraphic(&painter,QStringLiteral("rect"),QRectF(100,100,200,100),properties); painter.end(); QCOMPARE(image.copy(roi),native.convertToFormat(QImage::Format_ARGB32).copy(roi));
    properties.foreground=Qt::red; properties.background=Qt::blue; properties.lineStyle=0;
    for (index=0;index<13;++index) { properties.fillStyle=index; pixels.fill(Qt::magenta); painter.begin(&pixels); paintGraphic(&painter,QStringLiteral("rect"),QRectF(20,20,200,100),properties); painter.end(); QCOMPARE(pixels.pixelColor(0,0),QColor(Qt::magenta)); hashes.insert(QCryptographicHash::hash(QByteArray(reinterpret_cast<const char *>(pixels.constBits()),int(pixels.sizeInBytes())),QCryptographicHash::Sha256)); }
    QCOMPARE(hashes.size(),13); properties.alpha=0; pixels.fill(Qt::magenta); painter.begin(&pixels); paintGraphic(&painter,QStringLiteral("rect"),QRectF(20,20,200,100),properties); painter.end(); QCOMPARE(pixels.pixelColor(120,70),QColor(Qt::magenta)); QVERIFY(graphicHitPath(QStringLiteral("rect"),QRectF(20,20,200,100),properties).contains(QPointF(120,70)));
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(image.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("rect-native-roi-match.png")))); }
'''))
methods.append(('rectNativeMenuDoubleClickDraftAndUndo','Use the actual rectangle menu, placement, style dialog, cancellation and whole-project undo with generic text/address editors protected.',r'''    MainWindow window;
    QJsonObject before;
    QString error;
    window.setProject(HmiProject(),QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); window.m_commands.value(QStringLiteral("action_rect"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QVERIFY(QApplication::activeModalWidget()==nullptr);
    QTest::mousePress(window.m_canvas->viewport(),Qt::LeftButton,Qt::NoModifier,window.m_canvas->mapFromScene(QPointF(100,100))); QTest::mouseMove(window.m_canvas->viewport(),window.m_canvas->mapFromScene(QPointF(300,200))); QTest::mouseRelease(window.m_canvas->viewport(),Qt::LeftButton,Qt::NoModifier,window.m_canvas->mapFromScene(QPointF(300,200))); QCOMPARE(window.currentEditorProject().objects.first().type,QStringLiteral("rect")); QVERIFY2(window.currentEditorProject().validateModel(&error),qPrintable(error)); QVERIFY(!window.m_textEdit->isEnabled()); QVERIFY(!window.m_addressEdit->isEnabled()); before=window.currentEditorProject().toJson();
    QTimer::singleShot(0,[]() { GraphicDialog *dialog=qobject_cast<GraphicDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog!=nullptr); QCOMPARE(dialog->windowTitle(),QStringLiteral("矩形")); QVERIFY(!dialog->findChild<QComboBox *>(QStringLiteral("End_Arrow"))->isVisible()); dialog->findChild<QComboBox *>(QStringLiteral("Pattern"))->setCurrentIndex(12); dialog->findChild<QComboBox *>(QStringLiteral("Line_Width"))->setCurrentIndex(8); dialog->findChild<QSpinBox *>(QStringLiteral("Alpha"))->setValue(128); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog->grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("rect-properties.png")))); } dialog->findChild<QDialogButtonBox *>(QStringLiteral("GraphicButtons"))->button(QDialogButtonBox::Ok)->click(); });
    QTest::mouseDClick(window.m_canvas->viewport(),Qt::LeftButton,Qt::NoModifier,window.m_canvas->mapFromScene(QPointF(200,150))); QCOMPARE(window.currentEditorProject().objects.first().graphic.fillStyle,12); QCOMPARE(window.currentEditorProject().objects.first().graphic.lineWidth,9); QCOMPARE(window.currentEditorProject().objects.first().graphic.alpha,128);
    QTimer::singleShot(0,[]() { GraphicDialog *dialog=qobject_cast<GraphicDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog!=nullptr); dialog->findChild<QComboBox *>(QStringLiteral("Pattern"))->setCurrentIndex(0); dialog->reject(); }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().graphic.fillStyle,12); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().toJson(),before);
'''))
s=s.replace('    void initTestCase();',''.join('    void '+name+'();\n' for name,_,_ in methods)+'    void initTestCase();',1)
definitions=''
for name,description,body in methods:
    definitions+='\n/******************************************************************************\n * Function Name   : Stage4ControlTests::'+name+'\n * Description     : '+description+'\n * Input Parameters: None\n * Return Value    : None.\n * Date            : 2026.11.13\n ******************************************************************************/\nvoid Stage4ControlTests::'+name+'() //20261113-2 add\n{\n'+body+'}\n'
s=s.replace('QTEST_MAIN(Stage4ControlTests)',definitions+'\nQTEST_MAIN(Stage4ControlTests)',1);p.write_text(s,encoding='utf-8',newline='\n')
print('Added five rectangle regressions including a real native decoded ROI.')
