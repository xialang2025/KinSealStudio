"""Verify genuine closed-vertex drawing, fill drafts and atomic project persistence."""
from pathlib import Path
root = Path(__file__).resolve().parent
path = root / 'worktree/tests/stage4_controls_tests.cpp'
source = path.read_text(encoding='utf-8')
assert 'polygonOrderedClosedPathAndFill' not in source
methods = [
('polygonObservedFieldsAndStrictJson', 'Keep all vertices plus fill fields, and reject arrows, repeated closure and invalid points atomically.', r'''    HmiObject object;
    GraphicDialog dialog;
'''.replace('    GraphicDialog dialog;\n','') + r'''    GraphicDialog *dialog;
    GraphicProperties parsed;
    GraphicProperties sentinel;
    QJsonObject json;
    QString error;
    object.type = QStringLiteral("polygon"); object.graphic.points = {QPointF(0,0),QPointF(1,0),QPointF(.75,1)}; object.graphic.foreground = Qt::white; object.graphic.background = Qt::black;
    dialog = new GraphicDialog(object); dialog->show(); QVERIFY(QTest::qWaitForWindowExposed(dialog)); QCOMPARE(dialog->windowTitle(),QStringLiteral("多边形")); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("Line_Type"))->count(),6); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("Line_Width"))->count(),9); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("Pattern"))->count(),13); QVERIFY(!dialog->findChild<QComboBox *>(QStringLiteral("End_Arrow"))->isVisible()); QCOMPARE(dialog->object().graphic.foreground,QColor(Qt::white)); QCOMPARE(dialog->object().graphic.background,QColor(Qt::black));
    dialog->findChild<QComboBox *>(QStringLiteral("Pattern"))->setCurrentIndex(12); dialog->findChild<QSpinBox *>(QStringLiteral("Alpha"))->setValue(128); json = dialog->object().graphic.toJson(object.type); QCOMPARE(json.value(QStringLiteral("points")).toArray().size(),3); QVERIFY(!json.contains(QStringLiteral("endArrow"))); QVERIFY(GraphicProperties::fromJson(json,&parsed,&error,object.type)); sentinel = parsed;
    json.insert(QStringLiteral("endArrow"),1); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,object.type)); QCOMPARE(parsed.toJson(object.type),sentinel.toJson(object.type)); json = sentinel.toJson(object.type); json.insert(QStringLiteral("points"),QJsonArray{QJsonArray{0,0},QJsonArray{1,0},QJsonArray{0,0}}); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,object.type));
    json = sentinel.toJson(object.type); json.insert(QStringLiteral("points"),QJsonArray{QJsonArray{0,0},QJsonArray{1,0}}); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,object.type)); json = sentinel.toJson(object.type); json.insert(QStringLiteral("points"),QJsonArray{QJsonArray{0,0},QJsonArray{1,0},QJsonArray{1.01,1}}); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,object.type)); dialog->reject(); delete dialog;
'''),
('polygonOrderedClosedPathAndFill', 'Commit three clicks as one closed filled triangle, excluding the moving preview tail from the saved topology.', r'''    ScreenCanvas canvas;
    HmiObject draft;
    HmiObject object;
    QVector<QPointF> points = {QPointF(100,100),QPointF(300,100),QPointF(250,220)};
    QPointF point;
    QPainterPath path;
    QSignalSpy changed(&canvas,&ScreenCanvas::documentChanged);
    draft.type = QStringLiteral("polygon"); draft.graphic.foreground = Qt::white; draft.graphic.background = Qt::black; canvas.setProject(HmiProject()); canvas.resize(900,550); canvas.show(); QVERIFY(QTest::qWaitForWindowExposed(&canvas)); canvas.beginControlPlacement(draft);
    Q_FOREACH (point,points) { QTest::mouseClick(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(point)); QVERIFY(canvas.objects().isEmpty()); QCOMPARE(changed.size(),0); }
    QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(QPointF(400,300))); QTest::mouseClick(canvas.viewport(),Qt::RightButton,Qt::NoModifier,canvas.mapFromScene(points.last())); QCOMPARE(canvas.objects().size(),1); QCOMPARE(changed.size(),1); QVERIFY(!canvas.hasActiveInteraction()); object = canvas.objects().first(); QCOMPARE(graphicHandles(object.type,object.geometry,object.graphic),points); QCOMPARE(object.geometry,QRect(100,100,200,120)); path = graphicPath(object.type,object.geometry,object.graphic); QCOMPARE(path.elementCount(),4); QCOMPARE(path.currentPosition(),points.first()); QVERIFY(path.contains(QPointF(220,140))); QVERIFY(!path.contains(QPointF(110,210)));
'''),
('polygonIncompleteCancelAndPageReset', 'Cancel insufficient local polygons without creating an object or history, then begin a clean triangle after a page reset.', r'''    ScreenCanvas canvas;
    HmiObject draft;
    QPointF point;
    QVector<QPointF> points = {QPointF(20,20),QPointF(80,20),QPointF(60,70)};
    QSignalSpy changed(&canvas,&ScreenCanvas::documentChanged);
    draft.type = QStringLiteral("polygon"); canvas.setProject(HmiProject()); canvas.resize(900,550); canvas.show(); QVERIFY(QTest::qWaitForWindowExposed(&canvas)); canvas.beginControlPlacement(draft);
    QTest::mouseClick(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(100,100))); QTest::mouseClick(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(200,100))); QTest::mouseClick(canvas.viewport(),Qt::RightButton,Qt::NoModifier,canvas.mapFromScene(QPointF(200,100))); QVERIFY(canvas.objects().isEmpty()); QCOMPARE(changed.size(),0); QVERIFY(!canvas.hasActiveInteraction());
    canvas.beginControlPlacement(draft); QTest::mouseClick(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(400,300))); canvas.cancelInteraction(); QVERIFY(!canvas.hasActiveInteraction()); canvas.beginControlPlacement(draft); QTest::mouseClick(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(400,300))); canvas.setProject(HmiProject()); canvas.beginControlPlacement(draft); Q_FOREACH (point,points) { QTest::mouseClick(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(point)); } QTest::mouseClick(canvas.viewport(),Qt::RightButton,Qt::NoModifier,canvas.mapFromScene(points.last())); QCOMPARE(graphicHandles(QStringLiteral("polygon"),canvas.objects().first().geometry,canvas.objects().first().graphic),points); QCOMPARE(changed.size(),1);
'''),
('polygonVertexEditPreservesTopologyAndCancel', 'Move one polygon vertex and restore the full filled object when the next vertex gesture is canceled.', r'''    ScreenCanvas canvas;
    HmiProject project;
    HmiObject object;
    HmiObject after;
    QVector<QPointF> handles;
    QSignalSpy changed(&canvas,&ScreenCanvas::documentChanged);
    object.id = QStringLiteral("polygon-resize"); object.name = QStringLiteral("多边形"); object.type = QStringLiteral("polygon"); object.geometry = QRect(100,100,200,120); object.graphic.points = {QPointF(0,0),QPointF(1,0),QPointF(.75,1)}; object.graphic.fillStyle = 12; project.objects.append(object); canvas.setProject(project); canvas.resize(900,550); canvas.show(); QVERIFY(QTest::qWaitForWindowExposed(&canvas)); canvas.selectObject(object.id);
    QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(300,100))); QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(QPointF(330,150))); QTest::mouseRelease(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(330,150))); after = canvas.objects().first(); handles = graphicHandles(after.type,after.geometry,after.graphic); QCOMPARE(handles.size(),3); QCOMPARE(handles[0],QPointF(100,100)); QCOMPARE(handles[1],QPointF(330,150)); QCOMPARE(handles[2],QPointF(250,220)); QCOMPARE(after.graphic.fillStyle,12); QCOMPARE(changed.size(),1);
    QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(handles[1])); QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(QPointF(350,250))); canvas.cancelInteraction(); QCOMPARE(canvas.objects().first().toJson(),after.toJson()); QCOMPARE(changed.size(),1);
'''),
('polygonMixedPersistenceCopyAndNativeRefusal', 'Round-trip eleven mixed families and polygon fill/points through KSP and clipboard while refusing unknown native output.', r'''    HmiProject project;
    HmiProject loaded;
    HmiProject target;
    HmiProject pasted;
    HmiObject object;
    Stage3Clipboard clipboard;
    QString error;
    QString type;
    QStringList types = {QStringLiteral("line"),QStringLiteral("rect"),QStringLiteral("ellipse"),QStringLiteral("roundrect"),QStringLiteral("polyline")};
    QByteArray native = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    object.type = QStringLiteral("polygon"); object.id = QStringLiteral("polygon-model"); object.name = QStringLiteral("多边形"); object.geometry = QRect(100,100,200,120); object.graphic.points = {QPointF(0,0),QPointF(1,0),QPointF(.75,1)}; object.graphic.foreground = Qt::white; object.graphic.background = Qt::black; object.graphic.alpha = 128; object.graphic.lineWidth = 3; project.objects.append(object);
    QVERIFY2(HmiProject::fromJson(project.toJson(),&loaded,&error),qPrintable(error)); QCOMPARE(loaded.toJson(),project.toJson()); QVERIFY(captureStage3Selection(project,{object.id},&clipboard,&error)); QVERIFY2(pasteStage3Selection(target,clipboard,QPoint(20,20),&pasted,nullptr,&error),qPrintable(error)); QCOMPARE(pasted.objects.first().graphic.toJson(object.type),object.graphic.toJson(object.type)); QVERIFY(!exportNativeProjectSource(project,&native,&error)); QCOMPARE(native,QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")}); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(project.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("polygon-only.ksp")),&error)); }
    Q_FOREACH (type,types) { object.type = type; object.id = QStringLiteral("mixed-")+type; object.graphic = GraphicProperties(); project.objects.append(object); } project.objects.append(existingFiveObjects()); QVERIFY2(HmiProject::fromJson(project.toJson(),&loaded,&error),qPrintable(error)); QCOMPARE(loaded.objects.size(),11); QCOMPARE(loaded.toJson(),project.toJson());
'''),
('polygonFilledRasterAndTransparentHitShape', 'Render the actual closed triangle with native default black fill, then independently disable its fill and stroke.', r'''    GraphicProperties properties;
    QImage image(800,480,QImage::Format_ARGB32);
    QImage native(QFINDTESTDATA("../docs/stage4-evidence/polygon/native/sheet-polygon-right-three-point-r1.png"));
    QPainter painter;
    properties.points = {QPointF(0,0),QPointF(1,0),QPointF(.75,1)}; properties.foreground = Qt::white; properties.background = Qt::black; QVERIFY(!native.isNull()); image.fill(QColor(197,222,255)); painter.begin(&image); paintGraphic(&painter,QStringLiteral("polygon"),QRectF(100,100,200,120),properties); painter.end(); QCOMPARE(image.pixelColor(220,140),QColor(Qt::black)); QCOMPARE(image.pixelColor(110,210),QColor(197,222,255)); QVERIFY(graphicHitPath(QStringLiteral("polygon"),QRectF(100,100,200,120),properties).contains(QPointF(220,140))); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(image.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("polygon-default-raster.png")))); }
    properties.fillStyle = 0; image.fill(QColor(197,222,255)); painter.begin(&image); paintGraphic(&painter,QStringLiteral("polygon"),QRectF(100,100,200,120),properties); painter.end(); QCOMPARE(image.pixelColor(220,140),QColor(197,222,255)); QCOMPARE(image.pixelColor(200,100),QColor(Qt::black)); properties.lineStyle = 0; image.fill(QColor(197,222,255)); painter.begin(&image); paintGraphic(&painter,QStringLiteral("polygon"),QRectF(100,100,200,120),properties); painter.end(); QCOMPARE(image.pixelColor(200,100),QColor(197,222,255));
'''),
('polygonNativeMenuDialogDraftAndUndo', 'Use the real polygon command and inspect its fill dialog without losing points, then cancel edits and undo the accepted style transaction.', r'''    MainWindow window;
    QJsonObject before;
    QPointF point;
    QVector<QPointF> points = {QPointF(100,100),QPointF(300,100),QPointF(250,220)};
    window.setProject(HmiProject(),QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); window.m_commands.value(QStringLiteral("action_polygon"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QVERIFY(QApplication::activeModalWidget()==nullptr); Q_FOREACH (point,points) { QTest::mouseClick(window.m_canvas->viewport(),Qt::LeftButton,Qt::NoModifier,window.m_canvas->mapFromScene(point)); } QTest::mouseClick(window.m_canvas->viewport(),Qt::RightButton,Qt::NoModifier,window.m_canvas->mapFromScene(points.last())); QCOMPARE(window.currentEditorProject().objects.size(),1); QCOMPARE(window.currentEditorProject().objects.first().graphic.background,QColor(Qt::black)); QVERIFY(!window.m_textEdit->isEnabled()); QVERIFY(!window.m_addressEdit->isEnabled()); before = window.currentEditorProject().toJson();
    QTimer::singleShot(0,[]() { GraphicDialog *dialog = qobject_cast<GraphicDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog!=nullptr); QCOMPARE(dialog->windowTitle(),QStringLiteral("多边形")); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog->grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("polygon-default-properties.png")))); } dialog->findChild<QComboBox *>(QStringLiteral("Pattern"))->setCurrentIndex(12); dialog->findChild<QComboBox *>(QStringLiteral("Line_Width"))->setCurrentIndex(2); dialog->findChild<QSpinBox *>(QStringLiteral("Alpha"))->setValue(128); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog->grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("polygon-properties.png")))); } dialog->findChild<QDialogButtonBox *>(QStringLiteral("GraphicButtons"))->button(QDialogButtonBox::Ok)->click(); }); QTest::mouseDClick(window.m_canvas->viewport(),Qt::LeftButton,Qt::NoModifier,window.m_canvas->mapFromScene(QPointF(220,140))); QCOMPARE(window.currentEditorProject().objects.first().graphic.points.size(),3); QCOMPARE(window.currentEditorProject().objects.first().graphic.fillStyle,12); QCOMPARE(window.currentEditorProject().objects.first().graphic.alpha,128);
    QTimer::singleShot(0,[]() { GraphicDialog *dialog = qobject_cast<GraphicDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog!=nullptr); dialog->findChild<QComboBox *>(QStringLiteral("Pattern"))->setCurrentIndex(0); dialog->reject(); }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().graphic.fillStyle,12); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().toJson(),before);
''')]
source = source.replace('    void initTestCase();', ''.join('    void '+name+'();\n' for name,_,_ in methods)+'    void initTestCase();',1)
definitions = ''
for name,description,body in methods:
    definitions += '\n/******************************************************************************\n * Function Name   : Stage4ControlTests::'+name+'\n * Description     : '+description+'\n * Input Parameters: None\n * Return Value    : None.\n * Date            : 2026.11.13\n ******************************************************************************/\nvoid Stage4ControlTests::'+name+'() //20261113-4 add\n{\n'+body+'}\n'
source = source.replace('QTEST_MAIN(Stage4ControlTests)',definitions+'\nQTEST_MAIN(Stage4ControlTests)',1)
path.write_text(source,encoding='utf-8',newline='\n')
print('Added seven polygon behavior regressions.')
