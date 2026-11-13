"""Add real multi-click polyline regressions, including intermediate vertex editing and one-step history."""
from pathlib import Path

root = Path(__file__).resolve().parent
path = root / 'worktree/tests/stage4_controls_tests.cpp'
source = path.read_text(encoding='utf-8')
assert 'polylineMultiClickRightFinishOneTransaction' not in source
methods = [
('polylineNativeFieldsAndOrderedJson', 'Preserve the observed six stroke fields and every ordered vertex, rejecting malformed and adjacent duplicate points atomically.', r'''    HmiObject object;
    GraphicDialog *dialog;
    GraphicProperties parsed;
    GraphicProperties sentinel;
    QJsonObject json;
    QString error;
    object.type = QStringLiteral("polyline"); object.geometry = QRect(100,100,150,100); object.graphic.points = {QPointF(0,0),QPointF(1,0),QPointF(1,1)};
    dialog = new GraphicDialog(object); dialog->show(); QVERIFY(QTest::qWaitForWindowExposed(dialog)); QCOMPARE(dialog->windowTitle(),QStringLiteral("折线")); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("Line_Type"))->count(),6); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("Line_Width"))->count(),9); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("End_Arrow"))->count(),6); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("End_Cap"))->count(),2); QCOMPARE(dialog->findChild<QSpinBox *>(QStringLiteral("Alpha"))->maximum(),255); QVERIFY(dialog->findChild<QComboBox *>(QStringLiteral("Pattern"))==nullptr);
    dialog->findChild<QComboBox *>(QStringLiteral("End_Arrow"))->setCurrentIndex(1); dialog->findChild<QComboBox *>(QStringLiteral("End_Cap"))->setCurrentIndex(1); dialog->findChild<QSpinBox *>(QStringLiteral("Alpha"))->setValue(128); QCOMPARE(dialog->object().graphic.points,object.graphic.points); json = dialog->object().graphic.toJson(object.type); QCOMPARE(json.value(QStringLiteral("points")).toArray().size(),3); QVERIFY(!json.contains(QStringLiteral("fillStyle"))); QVERIFY(GraphicProperties::fromJson(json,&parsed,&error,object.type)); sentinel = parsed;
    json.insert(QStringLiteral("points"),QJsonArray{QJsonArray{0,0},QJsonArray{0,0},QJsonArray{1,1}}); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,object.type)); QCOMPARE(parsed.toJson(object.type),sentinel.toJson(object.type));
    json = sentinel.toJson(object.type); json.insert(QStringLiteral("fillStyle"),1); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,object.type)); dialog->reject(); delete dialog;
'''),
('polylineMultiClickRightFinishOneTransaction', 'Commit three genuine click vertices on right click, without turning each release or the moving preview tail into an object.', r'''    ScreenCanvas canvas;
    HmiObject draft;
    HmiObject object;
    QVector<QPointF> points = {QPointF(100,100),QPointF(250,100),QPointF(250,200)};
    QPointF point;
    QSignalSpy changed(&canvas,&ScreenCanvas::documentChanged);
    draft.type = QStringLiteral("polyline"); canvas.setProject(HmiProject()); canvas.resize(900,550); canvas.show(); QVERIFY(QTest::qWaitForWindowExposed(&canvas)); canvas.beginControlPlacement(draft);
    Q_FOREACH (point,points) { QTest::mouseClick(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(point)); QVERIFY(canvas.objects().isEmpty()); QCOMPARE(changed.size(),0); }
    QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(QPointF(350,250))); QTest::mouseClick(canvas.viewport(),Qt::RightButton,Qt::NoModifier,canvas.mapFromScene(points.last())); QCOMPARE(canvas.objects().size(),1); QCOMPARE(changed.size(),1); QVERIFY(!canvas.hasActiveInteraction());
    object = canvas.objects().first(); QCOMPARE(object.geometry,QRect(100,100,150,100)); QCOMPARE(graphicHandles(object.type,object.geometry,object.graphic),points); QCOMPARE(graphicPath(object.type,object.geometry,object.graphic).elementCount(),3); QVERIFY(!graphicHitPath(object.type,object.geometry,object.graphic).contains(QPointF(175,150)));
'''),
('polylineCancelAndInvalidFinish', 'Discard incomplete click drafts and page-switch drafts without leaving vertices, objects or history behind.', r'''    ScreenCanvas canvas;
    HmiObject draft;
    QSignalSpy changed(&canvas,&ScreenCanvas::documentChanged);
    draft.type = QStringLiteral("polyline"); canvas.setProject(HmiProject()); canvas.resize(900,550); canvas.show(); QVERIFY(QTest::qWaitForWindowExposed(&canvas)); canvas.beginControlPlacement(draft);
    QTest::mouseClick(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(100,100))); QTest::mouseClick(canvas.viewport(),Qt::RightButton,Qt::NoModifier,canvas.mapFromScene(QPointF(100,100))); QVERIFY(canvas.objects().isEmpty()); QVERIFY(!canvas.hasActiveInteraction()); QCOMPARE(changed.size(),0);
    canvas.beginControlPlacement(draft); QTest::mouseClick(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(100,100))); QTest::mouseClick(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(250,100))); canvas.cancelInteraction(); QVERIFY(!canvas.hasActiveInteraction()); QVERIFY(canvas.objects().isEmpty()); QCOMPARE(changed.size(),0);
    canvas.beginControlPlacement(draft); QTest::mouseClick(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(400,300))); canvas.setProject(HmiProject()); canvas.beginControlPlacement(draft); QTest::mouseClick(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(20,20))); QTest::mouseClick(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(80,20))); QTest::mouseClick(canvas.viewport(),Qt::RightButton,Qt::NoModifier,canvas.mapFromScene(QPointF(80,20))); QCOMPARE(canvas.objects().first().graphic.points.size(),2); QCOMPARE(canvas.objects().first().geometry,QRect(20,20,60,1));
'''),
('polylineIntermediateHandleAndCancel', 'Move an actual middle vertex while retaining endpoint coordinates and restore the complete shape when a second gesture is canceled.', r'''    ScreenCanvas canvas;
    HmiProject project;
    HmiObject object;
    HmiObject after;
    QVector<QPointF> handles;
    QSignalSpy changed(&canvas,&ScreenCanvas::documentChanged);
    object.id = QStringLiteral("polyline-resize"); object.name = QStringLiteral("折线"); object.type = QStringLiteral("polyline"); object.geometry = QRect(100,100,150,100); object.graphic.points = {QPointF(0,0),QPointF(1,0),QPointF(1,1)}; project.objects.append(object); canvas.setProject(project); canvas.resize(900,550); canvas.show(); QVERIFY(QTest::qWaitForWindowExposed(&canvas)); canvas.selectObject(object.id);
    QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(250,100))); QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(QPointF(300,150))); QTest::mouseRelease(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(300,150))); after = canvas.objects().first(); handles = graphicHandles(after.type,after.geometry,after.graphic); QCOMPARE(handles.size(),3); QCOMPARE(handles[0],QPointF(100,100)); QCOMPARE(handles[1],QPointF(300,150)); QCOMPARE(handles[2],QPointF(250,200)); QCOMPARE(changed.size(),1);
    QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(handles[1])); QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(QPointF(350,250))); canvas.cancelInteraction(); QVERIFY(!canvas.hasActiveInteraction()); QCOMPARE(canvas.objects().first().toJson(),after.toJson()); QCOMPARE(changed.size(),1);
'''),
('polylinePersistenceMixedCopyAndNativeRefusal', 'Round-trip ordered bends through KSP and clipboard, mix ten known families and preserve explicit native source/device refusal.', r'''    HmiProject project;
    HmiProject loaded;
    HmiProject target;
    HmiProject pasted;
    HmiObject object;
    Stage3Clipboard clipboard;
    QString error;
    QString type;
    QStringList types = {QStringLiteral("line"),QStringLiteral("rect"),QStringLiteral("ellipse"),QStringLiteral("roundrect")};
    QByteArray native = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    object.type = QStringLiteral("polyline"); object.id = QStringLiteral("polyline-model"); object.name = QStringLiteral("折线"); object.geometry = QRect(100,100,150,100); object.graphic.points = {QPointF(0,0),QPointF(1,0),QPointF(1,1)}; object.graphic.endArrow = 1; object.graphic.endCap = 1; object.graphic.alpha = 128; object.graphic.lineWidth = 3; project.objects.append(object);
    QVERIFY2(HmiProject::fromJson(project.toJson(),&loaded,&error),qPrintable(error)); QCOMPARE(loaded.toJson(),project.toJson()); QVERIFY(captureStage3Selection(project,{object.id},&clipboard,&error)); QVERIFY2(pasteStage3Selection(target,clipboard,QPoint(20,20),&pasted,nullptr,&error),qPrintable(error)); QCOMPARE(pasted.objects.first().graphic.toJson(object.type),object.graphic.toJson(object.type)); QVERIFY(!exportNativeProjectSource(project,&native,&error)); QCOMPARE(native,QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(project.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("polyline-only.ksp")),&error)); }
    Q_FOREACH (type,types) { object.type = type; object.id = QStringLiteral("mixed-")+type; object.graphic = GraphicProperties(); project.objects.append(object); } project.objects.append(existingFiveObjects()); QVERIFY2(HmiProject::fromJson(project.toJson(),&loaded,&error),qPrintable(error)); QCOMPARE(loaded.objects.size(),10); QCOMPARE(loaded.toJson(),project.toJson());
'''),
('polylineRealPathAndTailRasterInput', 'Render each segment with no fill and retain the original default raster for an independent pixel comparison.', r'''    GraphicProperties properties;
    QImage image(800,480,QImage::Format_ARGB32);
    QImage native(QFINDTESTDATA("../docs/stage4-evidence/polyline/native/sheet-polyline-right-three-point-r1.png"));
    QPainter painter;
    properties.points = {QPointF(0,0),QPointF(1,0),QPointF(1,1)}; QVERIFY(!native.isNull()); image.fill(QColor(197,222,255)); painter.begin(&image); paintGraphic(&painter,QStringLiteral("polyline"),QRectF(100,100,150,100),properties); painter.end(); QCOMPARE(image.pixelColor(175,100),QColor(Qt::black)); QCOMPARE(image.pixelColor(250,150),QColor(Qt::black)); QCOMPARE(image.pixelColor(175,150),QColor(197,222,255));
    properties.endArrow = 1; QVERIFY(graphicHitPath(QStringLiteral("polyline"),QRectF(100,100,150,100),properties).contains(QPointF(250,198))); properties.lineStyle = 0; image.fill(QColor(197,222,255)); painter.begin(&image); paintGraphic(&painter,QStringLiteral("polyline"),QRectF(100,100,150,100),properties); painter.end(); QCOMPARE(image.pixelColor(250,150),QColor(197,222,255));
    properties.endArrow = 0; properties.lineStyle = 1; image.fill(QColor(197,222,255)); painter.begin(&image); paintGraphic(&painter,QStringLiteral("polyline"),QRectF(100,100,150,100),properties); painter.end(); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(image.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("polyline-default-raster.png")))); }
'''),
('polylineNativeMenuDoubleClickDraftAndUndo', 'Use the actual foldline menu, three-click/right-click creation and stroke dialog while preserving one-step cancel and undo.', r'''    MainWindow window;
    QJsonObject before;
    QPointF point;
    QVector<QPointF> points = {QPointF(100,100),QPointF(250,100),QPointF(250,200)};
    window.setProject(HmiProject(),QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); window.m_commands.value(QStringLiteral("action_foldline"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QVERIFY(QApplication::activeModalWidget()==nullptr);
    Q_FOREACH (point,points) { QTest::mouseClick(window.m_canvas->viewport(),Qt::LeftButton,Qt::NoModifier,window.m_canvas->mapFromScene(point)); } QTest::mouseClick(window.m_canvas->viewport(),Qt::RightButton,Qt::NoModifier,window.m_canvas->mapFromScene(QPointF(250,200))); QCOMPARE(window.currentEditorProject().objects.size(),1); QCOMPARE(window.currentEditorProject().objects.first().graphic.points.size(),3); QVERIFY(!window.m_textEdit->isEnabled()); QVERIFY(!window.m_addressEdit->isEnabled()); before = window.currentEditorProject().toJson();
    QTimer::singleShot(0,[]() { GraphicDialog *dialog = qobject_cast<GraphicDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog!=nullptr); QCOMPARE(dialog->windowTitle(),QStringLiteral("折线")); dialog->findChild<QComboBox *>(QStringLiteral("End_Arrow"))->setCurrentIndex(1); dialog->findChild<QComboBox *>(QStringLiteral("Line_Width"))->setCurrentIndex(7); dialog->findChild<QSpinBox *>(QStringLiteral("Alpha"))->setValue(128); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog->grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("polyline-properties.png")))); } dialog->findChild<QDialogButtonBox *>(QStringLiteral("GraphicButtons"))->button(QDialogButtonBox::Ok)->click(); });
    QTest::mouseDClick(window.m_canvas->viewport(),Qt::LeftButton,Qt::NoModifier,window.m_canvas->mapFromScene(QPointF(175,100))); QCOMPARE(window.currentEditorProject().objects.first().graphic.points.size(),3); QCOMPARE(window.currentEditorProject().objects.first().graphic.endArrow,1); QCOMPARE(window.currentEditorProject().objects.first().graphic.alpha,128);
    QTimer::singleShot(0,[]() { GraphicDialog *dialog = qobject_cast<GraphicDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog!=nullptr); dialog->findChild<QComboBox *>(QStringLiteral("End_Cap"))->setCurrentIndex(1); dialog->reject(); }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().graphic.endCap,0); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().toJson(),before);
''')]
source = source.replace('    void initTestCase();', ''.join('    void ' + name + '();\n' for name, _, _ in methods) + '    void initTestCase();', 1)
definitions = ''
for name, description, body in methods:
    definitions += '\n/******************************************************************************\n * Function Name   : Stage4ControlTests::' + name + '\n * Description     : ' + description + '\n * Input Parameters: None\n * Return Value    : None.\n * Date            : 2026.11.13\n ******************************************************************************/\nvoid Stage4ControlTests::' + name + '() //20261113-2 add\n{\n' + body + '}\n'
source = source.replace('QTEST_MAIN(Stage4ControlTests)', definitions + '\nQTEST_MAIN(Stage4ControlTests)', 1)
path.write_text(source, encoding='utf-8', newline='\n')
print('Added seven actual polyline regressions.')
