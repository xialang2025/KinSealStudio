#include "hmi_canvas.h"
#include <QApplication>
#include <QImage>
#include <QJsonArray>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QSignalSpy>
#include <QtTest>

//20261110-8 add: All pointer events target this test's widgets; the runner owns offscreen/private-desktop setup.
class Stage3CanvasTests : public QObject
{
    Q_OBJECT
private slots:
    void placementGeometry_data();
    void placementGeometry();
    void placementCancellationAndZeroArea();
    void placementFlagsClearBeforeCommit();
    void movementIsOneTransaction_data();
    void movementIsOneTransaction();
    void resizeHandles_data();
    void resizeHandles();
    void canceledGesturesRestoreRecords();
    void pageAndToolSwitchCancel();
    void groupBoundaryPreservesRelativeGeometry();
    void noOpDoesNotCreateHistory();
    void viewportHandlesDoNotScale();
    void hostedCommandsAndExactSelection();
    void rightClickCancellationCannotPlaceFallback();
    void nativeArrangementGeometry_data();
    void nativeArrangementGeometry();
    void arrangementReferencePolicy();
    void arrangementNoOpAndAvailability();
};

/******************************************************************************
 * Function Name   : canvasObject
 * Description     : Produce a valid text object with retained language, access and layer fields.
 * Input Parameters: id, geometry
 * Return Value    : Complete object value
 * Date            : 2026.11.10
 ******************************************************************************/
static HmiObject canvasObject(const QString &id, const QRect &geometry) //20261110-8 add
{
    HmiObject object;

    object.id = id; object.name = id; object.type = QStringLiteral("text"); object.text = QStringLiteral("设备文本");
    object.geometry = geometry; object.hasStaticTextProperties = true;
    object.staticText.languageTexts.insert(QStringLiteral("中文"), object.text);
    object.staticText.languageTexts.insert(QStringLiteral("English"), QStringLiteral("Device text"));
    object.staticText.wordAddress = QStringLiteral("LW123"); object.staticText.fontSize = 18.5;
    return object;
}

/******************************************************************************
 * Function Name   : objectRecords
 * Description     : Compare all persisted properties instead of geometry alone.
 * Input Parameters: canvas
 * Return Value    : Serialized object values in scene stacking order
 * Date            : 2026.11.10
 ******************************************************************************/
static QJsonArray objectRecords(const ScreenCanvas &canvas) //20261110-8 add
{
    QJsonArray records;
    HmiObject object;

    Q_FOREACH (object, canvas.objects()) { records.append(object.toJson()); }
    return records;
}

/******************************************************************************
 * Function Name   : pointerEvent
 * Description     : Deliver a genuine Qt mouse event to the owned viewport without global input.
 * Input Parameters: canvas, type, scenePoint, button, buttons
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
static void pointerEvent(ScreenCanvas *canvas, QEvent::Type type, const QPointF &scenePoint,
                          Qt::MouseButton button, Qt::MouseButtons buttons) //20261110-8 add
{
    QPoint point = canvas->mapFromScene(scenePoint);
    QMouseEvent event(type, QPointF(point), QPointF(canvas->viewport()->mapToGlobal(point)), button, buttons, Qt::NoModifier);

    QApplication::sendEvent(canvas->viewport(), &event);
}

/******************************************************************************
 * Function Name   : drag
 * Description     : Run one complete pointer gesture with an intermediate move.
 * Input Parameters: canvas, start, end
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
static void drag(ScreenCanvas *canvas, const QPointF &start, const QPointF &end) //20261110-8 add
{
    pointerEvent(canvas, QEvent::MouseButtonPress, start, Qt::LeftButton, Qt::LeftButton);
    pointerEvent(canvas, QEvent::MouseMove, (start + end) / 2, Qt::NoButton, Qt::LeftButton);
    pointerEvent(canvas, QEvent::MouseMove, end, Qt::NoButton, Qt::LeftButton);
    pointerEvent(canvas, QEvent::MouseButtonRelease, end, Qt::LeftButton, Qt::NoButton);
}

/******************************************************************************
 * Function Name   : escape
 * Description     : Cancel through the real canvas key handler, with no application focus changes.
 * Input Parameters: canvas
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
static void escape(ScreenCanvas *canvas) //20261110-8 add
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);

    QApplication::sendEvent(canvas, &event);
}

/******************************************************************************
 * Function Name   : placementGeometry_data
 * Description     : Cover zoom factors that previously changed viewport and page-coordinate relationships.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::placementGeometry_data() //20261110-8 add
{
    QTest::addColumn<int>("zoom");
    QTest::newRow("half") << 50; QTest::newRow("normal") << 100; QTest::newRow("double") << 200;
}

/******************************************************************************
 * Function Name   : placementGeometry
 * Description     : Preserve device geometry and every accepted property across zoomed reverse-direction placement.
 * Input Parameters: zoom from data
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::placementGeometry() //20261110-8 add
{
    QFETCH(int, zoom);
    ScreenCanvas canvas;
    HmiProject project;
    HmiObject draft;
    QSignalSpy changes(&canvas, &ScreenCanvas::documentChanged);

    canvas.resize(1000, 720); canvas.setProject(project); canvas.setZoomPercent(zoom);
    draft = canvasObject(QStringLiteral("draft"), QRect(0, 0, 1, 1)); canvas.beginControlPlacement(draft);
    drag(&canvas, QPointF(200, 160), QPointF(100, 80));
    QCOMPARE(canvas.objects().size(), 1); QCOMPARE(canvas.objects()[0].geometry, QRect(100, 80, 100, 80));
    QCOMPARE(canvas.objects()[0].staticText.toJson(), draft.staticText.toJson());
    QCOMPARE(changes.count(), 1); QVERIFY(!canvas.hasPendingPlacement()); QVERIFY(!canvas.hasActiveInteraction());
    canvas.executeEdit(QStringLiteral("undo")); QVERIFY(canvas.objects().isEmpty());
    canvas.executeEdit(QStringLiteral("redo")); QCOMPARE(canvas.objects().size(), 1);
}

/******************************************************************************
 * Function Name   : placementCancellationAndZeroArea
 * Description     : Cancel drafts and reject zero-area clicks without half objects, dirty signals or history.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::placementCancellationAndZeroArea() //20261110-8 add
{
    ScreenCanvas canvas;
    HmiObject draft;
    QSignalSpy changes(&canvas, &ScreenCanvas::documentChanged);
    QSignalSpy canceled(&canvas, &ScreenCanvas::interactionCancelled);

    draft = canvasObject(QStringLiteral("draft"), QRect(0, 0, 1, 1)); canvas.beginControlPlacement(draft);
    pointerEvent(&canvas, QEvent::MouseButtonPress, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton);
    pointerEvent(&canvas, QEvent::MouseMove, QPointF(150, 100), Qt::NoButton, Qt::LeftButton);
    escape(&canvas); QCOMPARE(canceled.count(), 1); QVERIFY(canvas.objects().isEmpty());
    QVERIFY(!canvas.hasPendingPlacement()); QCOMPARE(changes.count(), 0);
    pointerEvent(&canvas, QEvent::MouseButtonRelease, QPointF(150, 100), Qt::LeftButton, Qt::NoButton);
    QVERIFY(canvas.objects().isEmpty());
    canvas.beginControlPlacement(draft); drag(&canvas, QPointF(80, 80), QPointF(80, 80));
    QCOMPARE(canceled.count(), 2); QCOMPARE(changes.count(), 0); QVERIFY(!canvas.canExecuteEdit(QStringLiteral("undo")));
    canvas.cancelInteraction(); QCOMPARE(canceled.count(), 2);
}

/******************************************************************************
 * Function Name   : placementFlagsClearBeforeCommit
 * Description     : Give the host a clean placement state before its single deferred resource commit.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::placementFlagsClearBeforeCommit() //20261110-8 add
{
    ScreenCanvas canvas;
    bool clearAtCommit;
    int commits;

    clearAtCommit = false; commits = 0;
    connect(&canvas, &ScreenCanvas::documentChanged, &canvas, [&]() {
        clearAtCommit = !canvas.hasPendingPlacement() && !canvas.hasActiveInteraction(); ++commits;
    });
    canvas.beginControlPlacement(canvasObject(QStringLiteral("draft"), QRect(0, 0, 1, 1)));
    drag(&canvas, QPointF(-20, -20), QPointF(900, 600));
    QCOMPARE(canvas.objects()[0].geometry, QRect(0, 0, 800, 480)); QVERIFY(clearAtCommit); QCOMPARE(commits, 1);
}

/******************************************************************************
 * Function Name   : movementIsOneTransaction_data
 * Description     : Reuse the measured viewport/scene zoom coverage for object movement.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::movementIsOneTransaction_data() //20261110-8 add
{
    placementGeometry_data();
}

/******************************************************************************
 * Function Name   : movementIsOneTransaction
 * Description     : Commit multiple drag updates once and undo/redo the complete logical geometry.
 * Input Parameters: zoom from data
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::movementIsOneTransaction() //20261110-8 add
{
    QFETCH(int, zoom);
    ScreenCanvas canvas;
    HmiProject project;
    QSignalSpy changes(&canvas, &ScreenCanvas::documentChanged);

    project.objects.append(canvasObject(QStringLiteral("move"), QRect(100, 80, 80, 60)));
    canvas.resize(1000, 720); canvas.setProject(project); canvas.setZoomPercent(zoom);
    drag(&canvas, QPointF(140, 110), QPointF(180, 140));
    QCOMPARE(changes.count(), 1); QCOMPARE(canvas.objects()[0].geometry, QRect(140, 110, 80, 60));
    canvas.executeEdit(QStringLiteral("undo")); QCOMPARE(canvas.objects()[0].geometry, QRect(100, 80, 80, 60));
    canvas.executeEdit(QStringLiteral("redo")); QCOMPARE(canvas.objects()[0].geometry, QRect(140, 110, 80, 60));
}

/******************************************************************************
 * Function Name   : resizeHandles_data
 * Description     : Exercise all eight resize handles using asymmetric geometry.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::resizeHandles_data() //20261110-8 add
{
    int index;

    QTest::addColumn<int>("handle");
    for (index = 0; index < 9; ++index)
    {
        if (index != 4) { QTest::newRow(qPrintable(QString::number(index))) << index; }
    }
}

/******************************************************************************
 * Function Name   : resizeHandles
 * Description     : Move only the requested rectangle edges and emit one completed resize transaction.
 * Input Parameters: handle from data
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::resizeHandles() //20261110-8 add
{
    QFETCH(int, handle);
    ScreenCanvas canvas;
    HmiProject project;
    QPointF start;
    QPointF end;
    int left;
    int top;
    int right;
    int bottom;
    QSignalSpy changes(&canvas, &ScreenCanvas::documentChanged);

    project.objects.append(canvasObject(QStringLiteral("resize"), QRect(100, 80, 80, 60)));
    canvas.resize(1000, 720); canvas.setProject(project); canvas.selectObject(QStringLiteral("resize")); canvas.setZoomPercent(200);
    start = QPointF(100 + (handle % 3) * 40, 80 + (handle / 3) * 30); end = start;
    left = 100; top = 80; right = 180; bottom = 140;
    if (handle % 3 == 0) { end.rx() -= 10; left -= 10; }
    if (handle % 3 == 2) { end.rx() += 20; right += 20; }
    if (handle / 3 == 0) { end.ry() -= 10; top -= 10; }
    if (handle / 3 == 2) { end.ry() += 20; bottom += 20; }
    drag(&canvas, start, end); QCOMPARE(changes.count(), 1);
    QCOMPARE(canvas.objects()[0].geometry, QRect(left, top, right - left, bottom - top));
    canvas.executeEdit(QStringLiteral("undo")); QCOMPARE(canvas.objects()[0].geometry, project.objects[0].geometry);
}

/******************************************************************************
 * Function Name   : canceledGesturesRestoreRecords
 * Description     : Escape restores every original field after resize or movement, with no dirty/history event.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::canceledGesturesRestoreRecords() //20261110-8 add
{
    ScreenCanvas canvas;
    HmiProject project;
    QJsonArray original;
    QSignalSpy changes(&canvas, &ScreenCanvas::documentChanged);
    QSignalSpy canceled(&canvas, &ScreenCanvas::interactionCancelled);

    project.objects.append(canvasObject(QStringLiteral("original"), QRect(100, 80, 80, 60))); canvas.setProject(project);
    canvas.selectObject(QStringLiteral("original")); original = objectRecords(canvas);
    pointerEvent(&canvas, QEvent::MouseButtonPress, QPointF(180, 140), Qt::LeftButton, Qt::LeftButton);
    pointerEvent(&canvas, QEvent::MouseMove, QPointF(220, 170), Qt::NoButton, Qt::LeftButton);
    QVERIFY(objectRecords(canvas) != original); escape(&canvas);
    QCOMPARE(objectRecords(canvas), original); QCOMPARE(changes.count(), 0); QCOMPARE(canceled.count(), 1);
    pointerEvent(&canvas, QEvent::MouseButtonPress, QPointF(140, 110), Qt::LeftButton, Qt::LeftButton);
    pointerEvent(&canvas, QEvent::MouseMove, QPointF(170, 140), Qt::NoButton, Qt::LeftButton);
    QVERIFY(objectRecords(canvas) != original); escape(&canvas);
    QCOMPARE(objectRecords(canvas), original); QCOMPARE(changes.count(), 0); QCOMPARE(canceled.count(), 2);
    QVERIFY(!canvas.canExecuteEdit(QStringLiteral("undo")));
}

/******************************************************************************
 * Function Name   : pageAndToolSwitchCancel
 * Description     : Replacing a page or selecting even the text tool discards unfinished placement and geometry.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::pageAndToolSwitchCancel() //20261110-8 add
{
    ScreenCanvas canvas;
    HmiProject empty;
    HmiObject draft;
    QSignalSpy canceled(&canvas, &ScreenCanvas::interactionCancelled);
    QSignalSpy changes(&canvas, &ScreenCanvas::documentChanged);

    draft = canvasObject(QStringLiteral("draft"), QRect(0, 0, 1, 1)); canvas.beginControlPlacement(draft);
    pointerEvent(&canvas, QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton);
    pointerEvent(&canvas, QEvent::MouseMove, QPointF(90, 60), Qt::NoButton, Qt::LeftButton);
    canvas.setActiveTool(QStringLiteral("text")); QVERIFY(!canvas.hasPendingPlacement()); QVERIFY(!canvas.hasActiveInteraction());
    QCOMPARE(canceled.count(), 1); QCOMPARE(changes.count(), 0);
    canvas.beginControlPlacement(draft); canvas.setProject(empty);
    QCOMPARE(canceled.count(), 2); QCOMPARE(changes.count(), 0); QVERIFY(canvas.objects().isEmpty());
}

/******************************************************************************
 * Function Name   : groupBoundaryPreservesRelativeGeometry
 * Description     : Clamp a selected group as one rectangle so members cannot collapse at the page edge.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::groupBoundaryPreservesRelativeGeometry() //20261110-8 add
{
    ScreenCanvas canvas;
    HmiProject project;
    HmiObject first;
    HmiObject second;
    QSignalSpy changes(&canvas, &ScreenCanvas::documentChanged);

    first = canvasObject(QStringLiteral("first"), QRect(650, 100, 50, 40)); first.group = QStringLiteral("group");
    second = canvasObject(QStringLiteral("second"), QRect(720, 120, 60, 40)); second.group = first.group; second.layer = 2;
    project.objects = {first, second}; canvas.resize(1000, 720); canvas.setProject(project);
    canvas.selectObjects({first.id, second.id}); drag(&canvas, QPointF(675, 120), QPointF(775, 120));
    QCOMPARE(changes.count(), 1); QCOMPARE(canvas.objects()[0].geometry, QRect(670, 100, 50, 40));
    QCOMPARE(canvas.objects()[1].geometry, QRect(740, 120, 60, 40));
    QCOMPARE(canvas.objects()[1].geometry.topLeft() - canvas.objects()[0].geometry.topLeft(), QPoint(70, 20));
    QCOMPARE(canvas.objects()[0].group, first.group); QCOMPARE(canvas.objects()[1].group, second.group);
}

/******************************************************************************
 * Function Name   : noOpDoesNotCreateHistory
 * Description     : Clicks, unchanged property confirmation and ineffective alignment add no document transaction.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::noOpDoesNotCreateHistory() //20261110-8 add
{
    ScreenCanvas canvas;
    HmiProject project;
    QSignalSpy changes(&canvas, &ScreenCanvas::documentChanged);

    project.objects.append(canvasObject(QStringLiteral("same"), QRect(100, 80, 80, 60)));
    project.objects[0].layer = 2.5; canvas.setProject(project);
    canvas.selectObject(QStringLiteral("same")); drag(&canvas, QPointF(180, 140), QPointF(180, 140));
    canvas.updateSelectedObject(project.objects[0]); canvas.executeEdit(QStringLiteral("alignLeft"));
    canvas.executeEdit(QStringLiteral("layerTop")); canvas.executeEdit(QStringLiteral("layerDown"));
    QCOMPARE(canvas.objects()[0].layer, 2.5);
    canvas.setZoomPercent(50); canvas.setZoomPercent(200); canvas.setLanguage(QStringLiteral("English"));
    QCOMPARE(changes.count(), 0); QVERIFY(!canvas.canExecuteEdit(QStringLiteral("undo")));
}

//20261110-8 add: Expose the real overlay painter for a deterministic device-independent rendering check.
class OverlayCanvas : public ScreenCanvas
{
public:
    void paintOverlay(QPainter *painter);
};

/******************************************************************************
 * Function Name   : OverlayCanvas::paintOverlay
 * Description     : Render the real editor overlay into an owned image for fixed-pixel checks.
 * Input Parameters: painter
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void OverlayCanvas::paintOverlay(QPainter *painter) //20261110-8 add
{
    drawForeground(painter, QRectF(0, 0, 800, 480));
}

/******************************************************************************
 * Function Name   : viewportHandlesDoNotScale
 * Description     : Keep handle fill area fixed when only the world transform changes, preserving object values at DPR2.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::viewportHandlesDoNotScale() //20261110-8 add
{
    OverlayCanvas canvas;
    HmiProject project;
    QImage image;
    QPainter painter;
    QJsonArray before;
    QPoint center;
    double scale;
    int index;
    int x;
    int y;
    int white;
    int expected;

    project.objects.append(canvasObject(QStringLiteral("handles"), QRect(100, 80, 80, 60))); canvas.setProject(project);
    canvas.selectObject(QStringLiteral("handles")); before = objectRecords(canvas); expected = -1;
    for (index = 0; index < 3; ++index)
    {
        scale = index == 0 ? 0.5 : index == 1 ? 1.0 : 2.0;
        image = QImage(600, 500, QImage::Format_ARGB32_Premultiplied); image.fill(Qt::transparent);
        painter.begin(&image); painter.translate(20, 20); painter.scale(scale, scale); canvas.paintOverlay(&painter); painter.end();
        center = QPoint(20 + qRound(100 * scale), 20 + qRound(80 * scale)); white = 0;
        for (y = center.y() - 5; y <= center.y() + 5; ++y)
        {
            for (x = center.x() - 5; x <= center.x() + 5; ++x)
            {
                if (image.pixelColor(x, y) == QColor(Qt::white)) { ++white; }
            }
        }
        QVERIFY(white > 0); if (expected < 0) { expected = white; } QCOMPARE(white, expected);
    }
    image = QImage(1200, 1000, QImage::Format_ARGB32_Premultiplied); image.setDevicePixelRatio(2); image.fill(Qt::transparent);
    painter.begin(&image); painter.scale(2, 2); canvas.paintOverlay(&painter); painter.end();
    QCOMPARE(objectRecords(canvas), before);
}

/******************************************************************************
 * Function Name   : hostedCommandsAndExactSelection
 * Description     : Keep hosted clipboard dispatch outside the canvas and select exact stable IDs without mutations.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::hostedCommandsAndExactSelection() //20261110-8 add
{
    ScreenCanvas canvas;
    HmiProject project;
    QSignalSpy requests(&canvas, &ScreenCanvas::editCommandRequested);
    QSignalSpy changes(&canvas, &ScreenCanvas::documentChanged);

    project.objects = {canvasObject(QStringLiteral("one"), QRect(30, 30, 50, 40)), canvasObject(QStringLiteral("two"), QRect(120, 30, 50, 40))};
    canvas.setProject(project); canvas.selectObjects({QStringLiteral("two")});
    QCOMPARE(canvas.selectedObjectIds(), QStringList{QStringLiteral("two")}); canvas.setExternalEditRouting(true);
    canvas.requestEditCommand(QStringLiteral("copy")); canvas.requestEditCommand(QStringLiteral("cut"));
    canvas.requestEditCommand(QStringLiteral("paste")); canvas.requestEditCommand(QStringLiteral("multiCopy"));
    QCOMPARE(requests.count(), 4); QCOMPARE(canvas.objects().size(), 2); QCOMPARE(changes.count(), 0);
}

/******************************************************************************
 * Function Name   : rightClickCancellationCannotPlaceFallback
 * Description     : Cancel the accepted draft and return to selection instead of silently placing a legacy object.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::rightClickCancellationCannotPlaceFallback() //20261110-8 add
{
    ScreenCanvas canvas;
    QSignalSpy changes(&canvas, &ScreenCanvas::documentChanged);
    QSignalSpy canceled(&canvas, &ScreenCanvas::interactionCancelled);

    canvas.beginControlPlacement(canvasObject(QStringLiteral("draft"), QRect(0, 0, 1, 1)));
    pointerEvent(&canvas, QEvent::MouseButtonPress, QPointF(50, 50), Qt::RightButton, Qt::RightButton);
    QCOMPARE(canceled.count(), 1); QVERIFY(!canvas.hasActiveInteraction());
    drag(&canvas, QPointF(80, 80), QPointF(140, 140));
    QVERIFY(canvas.objects().isEmpty()); QCOMPARE(changes.count(), 0);
}

/******************************************************************************
 * Function Name   : nativeArrangementGeometry_data
 * Description     : Retain exact asymmetric native r2/r3 results instead of reproducing implementation arithmetic in assertions.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::nativeArrangementGeometry_data() //20261110-8 add
{
    QTest::addColumn<QString>("command"); QTest::addColumn<QString>("reference");
    QTest::addColumn<QRect>("a"); QTest::addColumn<QRect>("b"); QTest::addColumn<QRect>("c");
    // Sources: docs/stage3-evidence/native/native-probe-alignment-r2.json and native-probe-alignment-r3.json.
    QTest::newRow("r2-left") << QStringLiteral("alignLeft") << QStringLiteral("B") << QRect(233, 47, 81, 39) << QRect(233, 117, 125, 63) << QRect(233, 289, 57, 91);
    QTest::newRow("r2-right") << QStringLiteral("alignRight") << QStringLiteral("B") << QRect(277, 47, 81, 39) << QRect(233, 117, 125, 63) << QRect(301, 289, 57, 91);
    QTest::newRow("r2-x-center") << QStringLiteral("alignCenter") << QStringLiteral("B") << QRect(255, 47, 81, 39) << QRect(233, 117, 125, 63) << QRect(267, 289, 57, 91);
    QTest::newRow("r2-y-center") << QStringLiteral("alignMiddle") << QStringLiteral("B") << QRect(31, 129, 81, 39) << QRect(233, 117, 125, 63) << QRect(487, 103, 57, 91);
    QTest::newRow("r3-top") << QStringLiteral("alignTop") << QStringLiteral("C") << QRect(31, 289, 81, 39) << QRect(233, 289, 125, 63) << QRect(487, 289, 57, 91);
    QTest::newRow("r3-bottom") << QStringLiteral("alignBottom") << QStringLiteral("C") << QRect(31, 341, 81, 39) << QRect(233, 317, 125, 63) << QRect(487, 289, 57, 91);
    QTest::newRow("r3-width") << QStringLiteral("alignWidth") << QStringLiteral("A") << QRect(31, 47, 81, 39) << QRect(233, 117, 81, 63) << QRect(487, 289, 81, 91);
    QTest::newRow("r3-height") << QStringLiteral("alignHeight") << QStringLiteral("A") << QRect(31, 47, 81, 39) << QRect(233, 117, 125, 39) << QRect(487, 289, 57, 39);
    QTest::newRow("r3-size") << QStringLiteral("alignSize") << QStringLiteral("A") << QRect(31, 47, 81, 39) << QRect(233, 117, 81, 39) << QRect(487, 289, 81, 39);
    QTest::newRow("r3-horizontal-gap") << QStringLiteral("alignHorizontalSpacing") << QString() << QRect(31, 47, 81, 39) << QRect(237, 117, 125, 63) << QRect(487, 289, 57, 91);
    QTest::newRow("r3-vertical-gap") << QStringLiteral("alignVerticalSpacing") << QString() << QRect(31, 47, 81, 39) << QRect(233, 156, 125, 63) << QRect(487, 289, 57, 91);
}

/******************************************************************************
 * Function Name   : nativeArrangementGeometry
 * Description     : Match all eleven native arrangement commands, retaining complete unrelated values and one-step undo/redo.
 * Input Parameters: command, reference, a, b, c from data
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::nativeArrangementGeometry() //20261110-8 add
{
    QFETCH(QString, command); QFETCH(QString, reference); QFETCH(QRect, a); QFETCH(QRect, b); QFETCH(QRect, c);
    ScreenCanvas canvas;
    HmiProject project;
    QJsonArray before;
    QJsonArray expected;
    HmiObject object;
    int index;
    QSignalSpy changes(&canvas, &ScreenCanvas::documentChanged);

    project.objects = {canvasObject(QStringLiteral("A"), QRect(31, 47, 81, 39)), canvasObject(QStringLiteral("B"), QRect(233, 117, 125, 63)),
                       canvasObject(QStringLiteral("C"), QRect(487, 289, 57, 91)), canvasObject(QStringLiteral("D"), QRect(650, 20, 50, 40))};
    for (index = 0; index < project.objects.size(); ++index) { project.objects[index].layer = index + 1; }
    canvas.setProject(project); canvas.selectObjects({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")});
    canvas.setArrangementReferenceObject(reference); before = objectRecords(canvas);
    QVERIFY(canvas.canExecuteEdit(command)); QCOMPARE(canvas.arrangementReferenceObjectId(command), reference);
    project.objects[0].geometry = a; project.objects[1].geometry = b; project.objects[2].geometry = c;
    Q_FOREACH (object, project.objects) { expected.append(object.toJson()); }
    canvas.executeEdit(command); QCOMPARE(objectRecords(canvas), expected); QCOMPARE(changes.count(), 1);
    canvas.executeEdit(QStringLiteral("undo")); QCOMPARE(objectRecords(canvas), before); QCOMPARE(changes.count(), 2);
    canvas.executeEdit(QStringLiteral("redo")); QCOMPARE(objectRecords(canvas), expected); QCOMPARE(changes.count(), 3);
}

/******************************************************************************
 * Function Name   : arrangementReferencePolicy
 * Description     : Expose native list operands and a selected explicit override without presenting them as mouse chronology.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::arrangementReferencePolicy() //20261110-8 add
{
    ScreenCanvas canvas;
    HmiProject project;
    QStringList selection;
    QSignalSpy changes(&canvas, &ScreenCanvas::documentChanged);

    project.objects = {canvasObject(QStringLiteral("A"), QRect(31, 47, 81, 39)), canvasObject(QStringLiteral("B"), QRect(233, 117, 125, 63)),
                       canvasObject(QStringLiteral("C"), QRect(487, 289, 57, 91))};
    canvas.setProject(project); canvas.selectObjects({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")});
    selection = canvas.selectedObjectIds(); QCOMPARE(canvas.arrangementReferenceObjectId(QStringLiteral("alignLeft")), selection.first());
    QCOMPARE(canvas.arrangementReferenceObjectId(QStringLiteral("alignSize")), selection.last());
    canvas.setArrangementReferenceObject(QStringLiteral("B")); QCOMPARE(canvas.arrangementReferenceObjectId(QStringLiteral("alignLeft")), QStringLiteral("B"));
    QCOMPARE(canvas.arrangementReferenceObjectId(QStringLiteral("alignSize")), QStringLiteral("B"));
    canvas.selectObject(QStringLiteral("C")); QCOMPARE(canvas.arrangementReferenceObjectId(QStringLiteral("alignLeft")), QStringLiteral("C"));
    canvas.selectObjects({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")}); canvas.setArrangementReferenceObject(QStringLiteral("missing"));
    QCOMPARE(canvas.arrangementReferenceObjectId(QStringLiteral("alignSize")), canvas.selectedObjectIds().last());
    QCOMPARE(canvas.arrangementReferenceObjectId(QStringLiteral("alignHorizontalSpacing")), QString()); QCOMPARE(changes.count(), 0);
}

/******************************************************************************
 * Function Name   : arrangementNoOpAndAvailability
 * Description     : Require two size operands or three spacing operands, retaining clean history for already arranged controls.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3CanvasTests::arrangementNoOpAndAvailability() //20261110-8 add
{
    ScreenCanvas canvas;
    HmiProject project;
    QSignalSpy changes(&canvas, &ScreenCanvas::documentChanged);

    project.objects = {canvasObject(QStringLiteral("A"), QRect(20, 20, 60, 40)), canvasObject(QStringLiteral("B"), QRect(120, 100, 60, 40)),
                       canvasObject(QStringLiteral("C"), QRect(220, 180, 60, 40))};
    canvas.setProject(project); QVERIFY(!canvas.canExecuteEdit(QStringLiteral("alignWidth")));
    canvas.selectObject(QStringLiteral("A")); QVERIFY(!canvas.canExecuteEdit(QStringLiteral("alignSize")));
    canvas.selectObjects({QStringLiteral("A"), QStringLiteral("B")}); QVERIFY(canvas.canExecuteEdit(QStringLiteral("alignSize")));
    QVERIFY(!canvas.canExecuteEdit(QStringLiteral("alignHorizontalSpacing"))); canvas.executeEdit(QStringLiteral("alignHorizontalSpacing"));
    canvas.selectObjects({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")});
    canvas.executeEdit(QStringLiteral("alignWidth")); canvas.executeEdit(QStringLiteral("alignHeight")); canvas.executeEdit(QStringLiteral("alignSize"));
    canvas.executeEdit(QStringLiteral("alignHorizontalSpacing")); canvas.executeEdit(QStringLiteral("alignVerticalSpacing"));
    QCOMPARE(changes.count(), 0); QVERIFY(!canvas.canExecuteEdit(QStringLiteral("undo")));
}

QTEST_MAIN(Stage3CanvasTests)
#include "stage3_canvas_tests.moc"
