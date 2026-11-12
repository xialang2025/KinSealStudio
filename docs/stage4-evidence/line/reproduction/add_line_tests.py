"""Add meaningful directed-line interaction, persistence and renderer regressions once."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
p=root/'tests/stage4_controls_tests.cpp';s=p.read_text(encoding='utf-8')
assert 'lineDirectedPlacementAndAxisAlignedGeometry' not in s
s=s.replace('#include <QTest>','#include <QTest>\n#include <QSignalSpy>',1)
s=s.replace('#include "main_window.h"','#include "graphic_dialog.h"\n#include "main_window.h"',1) if '#include "main_window.h"' in s else s
if '#include "graphic_dialog.h"' not in s:s='#include "graphic_dialog.h"\n'+s
methods=[]
methods.append(('lineStrictModelPersistenceAndBackendRefusal','Validate exact stroke limits, schema protection, atomic malformed JSON and independent native-layer refusal.',r'''    HmiProject project;
    HmiProject loaded;
    HmiObject object;
    GraphicProperties sentinel;
    GraphicProperties parsed;
    QJsonObject json;
    QString error;
    QByteArray native = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    object.id = QStringLiteral("line-1"); object.type = QStringLiteral("line"); object.name = QStringLiteral("直线1"); object.geometry = QRect(100, 100, 200, 100);
    object.graphic.points = {QPointF(1,0), QPointF(0,1)}; object.graphic.lineStyle = 5; object.graphic.lineWidth = 9; object.graphic.endArrow = 5; object.graphic.endCap = 1; object.graphic.alpha = 100;
    project.objects.append(object); QVERIFY2(project.validateModel(&error), qPrintable(error)); QCOMPARE(project.toJson().value(QStringLiteral("schemaVersion")).toInt(),12); QVERIFY2(HmiProject::fromJson(project.toJson(),&loaded,&error),qPrintable(error)); QCOMPARE(loaded.toJson(),project.toJson());
    json = project.toJson(); json.insert(QStringLiteral("schemaVersion"),11); QVERIFY(!HmiProject::fromJson(json,&loaded,&error));
    sentinel.alpha = 77; parsed = sentinel; json = object.graphic.toJson(); json.insert(QStringLiteral("lineWidth"),1.5); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error)); QCOMPARE(parsed.toJson(),sentinel.toJson());
    json = object.graphic.toJson(); json.insert(QStringLiteral("points"),QJsonArray{QJsonArray{0,0},QJsonArray{0,0}}); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error)); QCOMPARE(parsed.toJson(),sentinel.toJson());
    json = object.graphic.toJson(); json.insert(QStringLiteral("unknown"),true); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error));
    object.graphic.lineWidth = 0; QVERIFY(!object.graphic.validate(object.type,&error)); object.graphic.lineWidth = 10; QVERIFY(!object.graphic.validate(object.type,&error));
    QVERIFY(!exportNativeProjectSource(project,&native,&error)); QCOMPARE(native,QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")});
    if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(project.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("line-only.ksp")),&error)); }
'''))
methods.append(('lineDirectedPlacementAndAxisAlignedGeometry','Create lines in four quadrants and both axes while preserving ordered endpoints and a single transaction.',r'''    ScreenCanvas canvas;
    HmiObject draft;
    HmiObject object;
    QVector<QPointF> starts = {QPointF(100,100),QPointF(300,100),QPointF(300,200),QPointF(100,200),QPointF(100,100),QPointF(100,100)};
    QVector<QPointF> ends = {QPointF(300,200),QPointF(100,200),QPointF(100,100),QPointF(300,100),QPointF(300,100),QPointF(100,300)};
    QVector<QPointF> handles;
    QSignalSpy changed(&canvas,&ScreenCanvas::documentChanged);
    int index;
    draft.type = QStringLiteral("line"); canvas.setProject(HmiProject()); canvas.resize(900,550); canvas.show(); QVERIFY(QTest::qWaitForWindowExposed(&canvas));
    for (index = 0; index < starts.size(); ++index) { canvas.beginControlPlacement(draft); QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(starts[index])); QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(ends[index])); QTest::mouseRelease(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(ends[index])); QCOMPARE(canvas.objects().size(),index+1); object = canvas.objects().last(); handles = graphicHandles(object.type,object.geometry,object.graphic); QCOMPARE(handles[0],starts[index]); QCOMPARE(handles[1],ends[index]); QVERIFY(object.geometry.width()>0); QVERIFY(object.geometry.height()>0); QVERIFY(!canvas.hasActiveInteraction()); }
    QCOMPARE(changed.count(),6); canvas.executeEdit(QStringLiteral("undo")); QCOMPARE(canvas.objects().size(),5); canvas.executeEdit(QStringLiteral("redo")); QCOMPARE(canvas.objects().size(),6);
'''))
methods.append(('lineEndpointRoleAndCancellation','Move the first endpoint across the tail, then verify one-step undo and escape rollback without an extra transaction.',r'''    HmiProject project;
    ScreenCanvas canvas;
    HmiObject object;
    QVector<QPointF> handles;
    QSignalSpy changed(&canvas,&ScreenCanvas::documentChanged);
    object.id = QStringLiteral("line-edit"); object.type = QStringLiteral("line"); object.name = QStringLiteral("直线"); object.geometry = QRect(100,100,200,100); object.graphic.points = {QPointF(1,0),QPointF(0,1)}; object.graphic.endArrow = 1; project.objects.append(object);
    canvas.setProject(project); canvas.resize(900,550); canvas.show(); QVERIFY(QTest::qWaitForWindowExposed(&canvas)); canvas.selectObject(object.id);
    QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(300,100))); QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(QPointF(50,250))); QTest::mouseRelease(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(50,250)));
    handles = graphicHandles(object.type,canvas.objects().first().geometry,canvas.objects().first().graphic); QCOMPARE(handles[0],QPointF(50,250)); QCOMPARE(handles[1],QPointF(100,200)); QCOMPARE(canvas.objects().first().graphic.endArrow,1); QCOMPARE(changed.count(),1);
    canvas.executeEdit(QStringLiteral("undo")); QCOMPARE(canvas.objects().first().toJson(),object.toJson()); canvas.selectObject(object.id);
    QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(300,100))); QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(QPointF(350,160))); canvas.cancelInteraction(); QCOMPARE(canvas.objects().first().toJson(),object.toJson()); QVERIFY(!canvas.hasActiveInteraction());
    object.geometry = QRect(100,100,200,101); object.graphic.points = {QPointF(1,0),QPointF(.00125,100.5/101)}; project.objects = {object}; canvas.setProject(project); canvas.selectObject(object.id);
    QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(300,100))); QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(QPointF(50,250))); QTest::mouseRelease(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(50,250))); handles = graphicHandles(object.type,canvas.objects().first().geometry,canvas.objects().first().graphic); QCOMPARE(handles[0],QPointF(50,250)); QCOMPARE(handles[1],QPointF(100.25,200.5));
'''))
methods.append(('lineDraftCancelAndClipboardPreserveDirection','Cancel uncommitted and zero-length lines, and preserve directed geometry through cross-project clipboard and resizing.',r'''    HmiProject project;
    HmiProject target;
    HmiProject result;
    HmiObject object;
    Stage3Clipboard clipboard;
    ScreenCanvas canvas;
    QString error;
    object.id = QStringLiteral("directed"); object.type = QStringLiteral("line"); object.name = QStringLiteral("直线"); object.geometry = QRect(30,40,200,100); object.graphic.points = {QPointF(1,1),QPointF(0,0)}; object.graphic.endArrow = 4; project.objects.append(object);
    QVERIFY(captureStage3Selection(project,{object.id},&clipboard,&error)); QVERIFY2(pasteStage3Selection(target,clipboard,QPoint(10,10),&result,nullptr,&error),qPrintable(error)); QCOMPARE(result.objects.first().graphic.toJson(),object.graphic.toJson());
    canvas.setProject(HmiProject()); canvas.resize(900,550); canvas.show(); QVERIFY(QTest::qWaitForWindowExposed(&canvas)); canvas.beginControlPlacement(object); QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(50,50))); QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(QPointF(120,80))); QTest::keyClick(&canvas,Qt::Key_Escape); QVERIFY(canvas.objects().isEmpty()); QVERIFY(!canvas.hasActiveInteraction());
    canvas.beginControlPlacement(object); QTest::mouseClick(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(50,50))); QVERIFY(canvas.objects().isEmpty()); QVERIFY(!canvas.hasActiveInteraction());
'''))
methods.append(('lineRendererHitAndOpacity','Check actual stroke hit-testing, transparent compositing, marker repaint bounds and painter-state isolation.',r'''    HmiObject object;
    HmiControlItem item(([]() { HmiObject value; value.type = QStringLiteral("line"); value.id = QStringLiteral("hit"); value.name = QStringLiteral("Line"); value.geometry = QRect(0,0,200,100); return value; })());
    GraphicProperties properties;
    QImage image(280,180,QImage::Format_ARGB32);
    QPainter painter;
    QPen original(Qt::green,3);
    object = item.toHmiObject(); QVERIFY(item.shape().contains(QPointF(100,50))); QVERIFY(!item.shape().contains(QPointF(10,90))); object.graphic.lineWidth = 9; object.graphic.endArrow = 5; item.setObject(object); QVERIFY(item.boundingRect().contains(graphicHitPath(object.type,QRectF(0,0,200,100),object.graphic).boundingRect()));
    properties.lineWidth = 3; properties.alpha = 0; image.fill(Qt::white); painter.begin(&image); painter.setPen(original); paintGraphic(&painter,QStringLiteral("line"),QRectF(20,20,200,100),properties); QCOMPARE(painter.pen(),original); painter.end(); QCOMPARE(image.pixelColor(120,70),QColor(Qt::white));
    properties.alpha = 255; painter.begin(&image); paintGraphic(&painter,QStringLiteral("line"),QRectF(20,20,200,100),properties); painter.end(); QCOMPARE(image.pixelColor(120,70),QColor(Qt::black)); QCOMPARE(image.pixelColor(0,0),QColor(Qt::white));
'''))
methods.append(('lineNativeMenuAndDoubleClickStyleTransaction','Use the actual menu and double-click editor, including style acceptance, cancellation, compact panel protection and whole-project undo.',r'''    MainWindow window;
    QString error;
    QJsonObject before;
    window.setProject(HmiProject(),QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); window.m_commands.value(QStringLiteral("action_line"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QVERIFY(QApplication::activeModalWidget()==nullptr);
    QTest::mousePress(window.m_canvas->viewport(),Qt::LeftButton,Qt::NoModifier,window.m_canvas->mapFromScene(QPointF(100,100))); QTest::mouseMove(window.m_canvas->viewport(),window.m_canvas->mapFromScene(QPointF(300,200))); QTest::mouseRelease(window.m_canvas->viewport(),Qt::LeftButton,Qt::NoModifier,window.m_canvas->mapFromScene(QPointF(300,200))); QCOMPARE(window.currentEditorProject().objects.first().type,QStringLiteral("line")); QVERIFY2(window.currentEditorProject().validateModel(&error),qPrintable(error)); QVERIFY(!window.m_textEdit->isEnabled()); QVERIFY(!window.m_addressEdit->isEnabled());
    before = window.currentEditorProject().toJson(); QTimer::singleShot(0,[]() { GraphicDialog *dialog=qobject_cast<GraphicDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog!=nullptr); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("Line_Type"))->count(),6); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("End_Arrow"))->count(),6); dialog->findChild<QComboBox *>(QStringLiteral("Line_Width"))->setCurrentIndex(8); dialog->findChild<QComboBox *>(QStringLiteral("End_Arrow"))->setCurrentIndex(4); dialog->findChild<QSpinBox *>(QStringLiteral("Alpha"))->setValue(100); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog->grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("line-properties.png")))); } dialog->findChild<QDialogButtonBox *>(QStringLiteral("GraphicButtons"))->button(QDialogButtonBox::Ok)->click(); });
    QTest::mouseDClick(window.m_canvas->viewport(),Qt::LeftButton,Qt::NoModifier,window.m_canvas->mapFromScene(QPointF(200,150))); QCOMPARE(window.currentEditorProject().objects.first().graphic.lineWidth,9); QCOMPARE(window.currentEditorProject().objects.first().graphic.endArrow,4); QCOMPARE(window.currentEditorProject().objects.first().graphic.alpha,100);
    QTimer::singleShot(0,[]() { GraphicDialog *dialog=qobject_cast<GraphicDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog!=nullptr); dialog->findChild<QComboBox *>(QStringLiteral("Line_Width"))->setCurrentIndex(0); dialog->reject(); }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().graphic.lineWidth,9); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().toJson(),before);
'''))
declarations=''.join('    void '+name+'();\n' for name,_,_ in methods)
s=s.replace('    void initTestCase();',declarations+'    void initTestCase();',1)
definitions=''
for name,description,body in methods:
    definitions+='\n/******************************************************************************\n * Function Name   : Stage4ControlTests::'+name+'\n * Description     : '+description+'\n * Input Parameters: None\n * Return Value    : None.\n * Date            : 2026.11.13\n ******************************************************************************/\nvoid Stage4ControlTests::'+name+'() //20261113-2 add\n{\n'+body+'}\n'
s=s.replace('QTEST_MAIN(Stage4ControlTests)',definitions+'\nQTEST_MAIN(Stage4ControlTests)',1)
p.write_text(s,encoding='utf-8',newline='\n')
print('Added six directed-line regression methods.')
