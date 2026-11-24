#include "main_window.h"
#include "hmi_canvas.h"
#include "simulation_window.h" //20261123-3 add
#include "stage2_project_dialogs.h"
#include "stage2_global_tools.h"
#include "stage3_resources.h" //20261123-2 add
#include "text_payload_compiler.h"
#include "independent_downloader.h"
#include "macro_schedule.h" //20261123-20 add
#include "macro_workspace.h"
#include "native_project_codec.h" //20261123-23 add
#include "number_display_dialog.h" //20261123-25 add
#include <QAction>
#include <QAbstractButton>
#include <QAbstractItemView> //20261124-5 add
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QCheckBox> //20261123-4 add
#include <QColorDialog>
#include <QComboBox>
#include <QDateEdit> //20261123-18 add
#include <QFile>
#include <QFileDialog>
#include <QGroupBox> //20261123-4 add
#include <QInputDialog> //20261124-7 add
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QJsonDocument>
#include <QMessageBox>
#include <QMouseEvent> //20261124-11 add
#include <QPainter> //20261124-5 add
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QSpinBox>
#include <QTabBar>
#include <QTabWidget>
#include <QTableWidget> //20261123-6 add
#include <QTemporaryDir>
#include <QSvgRenderer> //20261123-3 add
#include <QTest>
#include <QTextBrowser> //20261123-10 add
#include <QTimer>
#include <QToolBar>
#include <QToolButton> //20261111-1 add
#include <QDockWidget>
#include <QMenuBar>
#include <QMenu>
#include <QScrollArea>
#include <QScrollBar> //20261124-4 add
#include <QStyleOptionSlider> //20261124-4 add
#include <QStatusBar>
#include <QDir>
#include <QTreeWidget>
#include <QWheelEvent> //20261124-6 add
#include <functional>

//20261110-7 add: Exercise the real main-window transactions without using physical input or USB.
class Stage2WorkflowTests : public QObject
{
    Q_OBJECT
private:
    QTemporaryDir m_settings;
private slots:
    void initTestCase();
    void sharedCommandsAndAvailability();
    void editorCanvasHasScrollableWorkspace(); //20261124-4 add
    void editorCanvasZoomsWithControlWheel(); //20261124-6 add
    void toolbarPreviewsWholePageAndSelectedStates(); //20261124-5 add
    void canvasKeysUseWholeProjectHistory();
    void canvasControlDragDuplicatesSelection(); //20261124-11 add
    void textClipboardDoesNotModifyCanvas();
    void numericDraftCommandsStayLocal();
    void focusedWindowDestructionIsSafe();
    void macroSearchLocationOpensSource();
    void cancelledWizardPreservesDirtyDocument();
    void invalidOpenPreservesDocument();
    void openingSamePathReloadsAfterSave();
    void saveDiscardCancelClose();
    void standardDialogsFollowInterfaceLanguage(); //20261124-7 add
    void cancelledSaveAsBlocksClose();
    void failedSaveBlocksClose();
    void actualTabsComboAndTreeNavigateById();
    void visitingUnsortedLayersIsNavigationOnly();
    void crossPageReplacementUndoAndSavedCheckpoint();
    void invalidDraftAndNativeOverwriteAreAtomic();
    void reopenAfterCloseRestoresEditor();
    void recentFilesAreBoundedAndIsolated();
    void openedProjectShowsProtocolsUnderTheirOwnPorts(); //20261122-6 add
    void serialPortSettingsAreIndependentAndPersisted(); //20261122-6 add
    void protocolTreeEditsAreScopedToTheirInstance(); //20261122-7 add
    void addingProtocolKeepsExistingProtocolsOnThePort(); //20261122-7 add
    void deletingProtocolProtectsProjectReferences(); //20261122-7 add
    void newPageFromSettingsTreeCancelsOrReopens(); //20261122-9 add
    void newPageFromToolbarCancelsOrReopens_data(); //20261124-10 add
    void newPageFromToolbarCancelsOrReopens(); //20261124-9 add
    void ordinaryPageNumbersAreEditableAndPersisted(); //20261122-10 add
    void newPageBackgroundRendersAndSurvivesReopen(); //20261123-0 add
    void newPageGradientKeepsNativeSymmetryAndPersists(); //20261123-1 add
    void newPageProjectBitmapStretchesAndPersists(); //20261123-2 add
    void pageProjectGalleryImportsFromProperties(); //20261124-3 add
    void newPageSystemBitmapStretchesAndPersists(); //20261123-3 add
    void newPageSlideKeepsStableTargetAcrossSave(); //20261123-4 add
    void newPageMacroDefaultsPersistWithStableSource(); //20261123-5 add
    void ordinaryPagePropertiesRestoreAndUpdateDraft(); //20261123-6 add
    void ordinaryPageMenuOpensAndReorders(); //20261123-7 add
    void ordinaryPageMenuCopiesOwnedContent(); //20261123-8 add
    void ordinaryPageMenuDeletionProtectsTargets(); //20261123-9 add
    void ordinaryPageHelpAndSlideVisibilityFollowDraft(); //20261123-10 add
    void ordinaryPageDialogUsesNativeClientLayout(); //20261123-11 add
    void windowTreeCreatesAndReopensPlacement(); //20261123-12 add
    void keyboardTreeAddsSelectedTemplates(); //20261123-24 add
    void screenAndWindowRejectTheSameNumber(); //20261123-13 add
    void windowMenuCopiesAndConfirmsDeletion(); //20261123-13 add
    void projectSettingsWindowCopyKeepsAssignedNumber(); //20261123-14 add
    void languageTreeUsesEditableRowsAndPreservesTranslations(); //20261123-19 add
    void extendedSystemSettingsPersist_data(); //20261123-15 add
    void extendedSystemSettingsPersist();
    void fileProtectionGatesProjectReopen(); //20261123-17 add
    void nativeFileProtectionIsCheckedBeforeImport(); //20261123-17 add
    void macroScheduleCommandsOpenTheirManagers_data(); //20261123-20 add
    void macroScheduleCommandsOpenTheirManagers();
    void projectMacroSchedulesSaveOrderedBindings(); //20261123-20 add
    void nativeProjectMacroSchedulesRoundTrip(); //20261123-23 add
    void emptyWindowKeepsZeroWidthAcrossSave(); //20261123-13 add
    void nativeShellResourcesAndNavigation(); //20261111-1 add
    void nativeShellQuickPropertiesPreserveEditing();
    void nativeShellPendingCommandsAreReadOnly();
    void nativePropertyToolbarEditsSelectedControl(); //20261124-0 add
    void nativePropertyToolbarEditsCurrentPage(); //20261124-0 add
};

/******************************************************************************
 * Function Name   : workflowProject
 * Description     : Build a minimal valid document containing a stable editable control.
 * Input Parameters: None
 * Return Value    : A one-page project.
 * Date            : 2026.11.10
 ******************************************************************************/
static HmiProject workflowProject() //20261110-7 add
{
    HmiProject project;
    HmiObject object;
    object.id = QStringLiteral("workflow-text");
    object.name = QStringLiteral("测试文字");
    object.type = QStringLiteral("text");
    object.text = QStringLiteral("TEST");
    object.address = QStringLiteral("LW0");
    object.geometry = QRect(244, 112, 150, 50);
    project.objects.append(object);
    return project;
}

/******************************************************************************
 * Function Name   : initTestCase
 * Description     : Redirect recent-file preferences to an owned disposable directory.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::initTestCase() //20261110-7 add
{
    QVERIFY(m_settings.isValid());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settings.path());
}

/******************************************************************************
 * Function Name   : sharedCommandsAndAvailability
 * Description     : Check common QAction identity, native property shortcut and document state transitions.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::sharedCommandsAndAvailability() //20261110-7 add
{
    MainWindow window;
    QAction *save;
    QToolBar *toolbar;
    bool found = false;
    save = window.m_commands.value(QStringLiteral("action_save"));
    QVERIFY(save != nullptr);
    QVERIFY(!save->isEnabled());
    Q_FOREACH (toolbar, window.findChildren<QToolBar *>()) {
        if (toolbar->actions().contains(save)) { found = true; }
    }
    QVERIFY(found);
    QCOMPARE(window.m_commands.value(QStringLiteral("action_openpropertiset"))->shortcut(),
             QKeySequence(QStringLiteral("Ctrl+P")));
    window.setProject(workflowProject(), QString());
    QVERIFY(!window.m_commands.value(QStringLiteral("action_delete"))->isEnabled());
    window.m_canvas->selectObject(QStringLiteral("workflow-text"));
    QVERIFY(window.m_commands.value(QStringLiteral("action_delete"))->isEnabled());
    window.dispatchEditCommand(QStringLiteral("delete"));
    QVERIFY(save->isEnabled());
    QVERIFY(window.m_dirty);
    window.dispatchEditCommand(QStringLiteral("undo"));
    QVERIFY(!window.m_dirty);
    QVERIFY(!save->isEnabled());
    window.closeProject();
    QVERIFY(!window.m_commands.value(QStringLiteral("action_USBdownload"))->isEnabled());
    QVERIFY(!window.m_commands.value(QStringLiteral("action_project_settings"))->isEnabled());
    QVERIFY(window.m_commands.value(QStringLiteral("action_open_pro"))->isEnabled());
}

/******************************************************************************
 * Function Name   : editorCanvasHasScrollableWorkspace
 * Description     : Verify the right scrollbar moves the view even when the device page fits.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.24
 ******************************************************************************/
void Stage2WorkflowTests::editorCanvasHasScrollableWorkspace() //20261124-4 add
{
    MainWindow window;
    HmiProject project = workflowProject();
    ScreenCanvas *canvas = window.m_canvas;
    QScrollBar *vertical = canvas->verticalScrollBar();
    QScrollBar *horizontal = canvas->horizontalScrollBar();
    QAction *zoomIn;
    QStyleOptionSlider slider;
    QPoint handle;
    QPoint before;
    bool moved;

    window.setProject(project, QString());
    window.resize(1353, 760);
    window.show();
    QTRY_VERIFY(canvas->viewport()->height() >= project.pageHeight);
    QTRY_VERIFY2(vertical->isVisibleTo(canvas) && vertical->maximum() > vertical->minimum(),
                 "The right scrollbar needs room to pan when the editor can already show the device page height.");
    QCOMPARE(vertical->maximum(), 1536 - canvas->viewport()->height());
    QCOMPARE(horizontal->maximum(), 2048 - canvas->viewport()->width());
    QCOMPARE(canvas->scene()->sceneRect(), QRectF(0, 0, project.pageWidth, project.pageHeight));

    // Use the real thumb exposed by the Windows scrollbar style, through the main window's hit test.
    slider.initFrom(vertical);
    slider.orientation = Qt::Vertical;
    slider.minimum = vertical->minimum();
    slider.maximum = vertical->maximum();
    slider.sliderPosition = vertical->sliderPosition();
    slider.sliderValue = vertical->value();
    slider.pageStep = vertical->pageStep();
    slider.singleStep = vertical->singleStep();
    handle = vertical->style()->subControlRect(QStyle::CC_ScrollBar, &slider, QStyle::SC_ScrollBarSlider,
                                               vertical).center();
    handle = vertical->mapTo(&window, handle);
    QCOMPARE(window.childAt(handle), vertical);
    before = canvas->mapFromScene(QPointF(0, 0));
    QTest::mousePress(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, handle);
    moved = vertical->isSliderDown();
    QTest::mouseMove(window.windowHandle(), handle + QPoint(0, 32));
    QTest::mouseRelease(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, handle + QPoint(0, 32));
    QVERIFY(moved);
    QVERIFY(vertical->value() > 0);
    QVERIFY(canvas->mapFromScene(QPointF(0, 0)).y() < before.y());

    zoomIn = window.findChild<QAction *>(QStringLiteral("action_zoonbig"));
    QVERIFY(zoomIn != nullptr && zoomIn->isEnabled());
    zoomIn->trigger();
    QTRY_COMPARE(vertical->maximum(), 1920 - canvas->viewport()->height());
    QCOMPARE(canvas->scene()->sceneRect(), QRectF(0, 0, project.pageWidth, project.pageHeight));
    QVERIFY(!window.m_dirty);
    QCOMPARE(window.currentEditorProject().objects.first().geometry, project.objects.first().geometry);
}

/******************************************************************************
 * Function Name   : editorCanvasZoomsWithControlWheel
 * Description     : Route wheel input through the viewport and compare zoom, scroll and project state.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.24
 ******************************************************************************/
void Stage2WorkflowTests::editorCanvasZoomsWithControlWheel() //20261124-6 add
{
    MainWindow window;
    const HmiProject project = workflowProject();
    ScreenCanvas *canvas = window.m_canvas;
    QScrollBar *horizontal = canvas->horizontalScrollBar();
    QScrollBar *vertical = canvas->verticalScrollBar();
    const QPoint cursor(190, 150);
    QPointF target;
    QPoint scroll;
    QJsonObject before;
    QAction *zoomOut;
    QAction *reset;

    window.setProject(project, QString());
    window.show();
    QTRY_VERIFY(canvas->viewport()->rect().contains(cursor));
    QTRY_VERIFY(horizontal->maximum() >= 150 && vertical->maximum() >= 150);
    horizontal->setValue(145);
    vertical->setValue(125);
    target = canvas->mapToScene(cursor);
    before = window.currentEditorProject().toJson();
    auto turnWheel = [&](int delta, Qt::KeyboardModifiers modifiers) {
        QWidget *viewport = canvas->viewport();
        QWheelEvent event(cursor, viewport->mapToGlobal(cursor), QPoint(), QPoint(0, delta),
                          Qt::NoButton, modifiers, Qt::NoScrollPhase, false);
        QApplication::sendEvent(viewport, &event);
    };

    turnWheel(120, Qt::ControlModifier);
    QCOMPARE(canvas->transform().m11(), 1.25);
    QVERIFY((canvas->mapFromScene(target) - cursor).manhattanLength() <= 1);
    turnWheel(60, Qt::ControlModifier);
    QCOMPARE(canvas->transform().m11(), 1.25);
    turnWheel(60, Qt::ControlModifier);
    QCOMPARE(canvas->transform().m11(), 1.5);
    turnWheel(-120, Qt::ControlModifier);
    QCOMPARE(canvas->transform().m11(), 1.25);

    zoomOut = window.m_commands.value(QStringLiteral("action_zoonsmall"));
    reset = window.m_commands.value(QStringLiteral("action_zoonfit"));
    QVERIFY(zoomOut != nullptr && reset != nullptr && zoomOut->isEnabled() && reset->isEnabled());
    zoomOut->trigger();
    QCOMPARE(canvas->transform().m11(), 1.0);
    turnWheel(960, Qt::ControlModifier);
    QCOMPARE(canvas->transform().m11(), 2.0);
    scroll = QPoint(horizontal->value(), vertical->value());
    turnWheel(120, Qt::ControlModifier);
    QCOMPARE(QPoint(horizontal->value(), vertical->value()), scroll);
    turnWheel(-960, Qt::ControlModifier);
    QCOMPARE(canvas->transform().m11(), 0.25);

    reset->trigger();
    QCOMPARE(canvas->transform().m11(), 1.0);
    vertical->setValue(125);
    turnWheel(-120, Qt::NoModifier);
    QVERIFY(vertical->value() > 125);
    QCOMPARE(canvas->transform().m11(), 1.0);
    QCOMPARE(window.currentEditorProject().toJson(), before);
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : toolbarPreviewsWholePageAndSelectedStates
 * Description     : Change state artwork through the editor toolbar and compare rendered pixels and project records.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.24
 ******************************************************************************/
void Stage2WorkflowTests::toolbarPreviewsWholePageAndSelectedStates() //20261124-5 add
{
    MainWindow window;
    HmiProject project = workflowProject();
    HmiObject bit;
    HmiObject multi;
    HmiObject picture;
    QComboBox *state;
    QJsonObject before;
    const QList<QColor> colors = {QColor(Qt::red), QColor(Qt::green), QColor(Qt::blue)};
    QPoint choicePoint;
    int index;

    bit.id = QStringLiteral("preview-bit");
    bit.name = bit.id;
    bit.type = QStringLiteral("switch");
    bit.geometry = QRect(24, 24, 80, 48);
    bit.address = QStringLiteral("LB0");
    bit.hasBitButtonProperties = true;
    multi.id = QStringLiteral("preview-multi");
    multi.name = multi.id;
    multi.type = QStringLiteral("multistate");
    multi.geometry = QRect(124, 24, 80, 48);
    picture.id = QStringLiteral("preview-image");
    picture.name = picture.id;
    picture.type = QStringLiteral("image");
    picture.geometry = QRect(224, 24, 80, 48);
    picture.hasImageDisplayProperties = true;
    picture.imageDisplay.displayType = 1;
    picture.imageDisplay.setStateCount(3);
    for (index = 0; index < colors.size(); ++index)
    {
        QImage tile(8, 8, QImage::Format_RGB32);
        QBuffer png(&picture.imageDisplay.states[index].picturePng);
        tile.fill(colors.at(index));
        QVERIFY(png.open(QIODevice::WriteOnly));
        QVERIFY(tile.save(&png, "PNG"));
        picture.imageDisplay.states[index].picture = QStringLiteral("state%1.png").arg(index);
        multi.multiStateButton.state(index)->useImage = false;
        multi.multiStateButton.state(index)->background = colors.at(index);
        if (index < 2)
        {
            bit.bitButton.states[index].useImage = false;
            bit.bitButton.states[index].background = colors.at(index);
        }
    }
    project.objects.append(bit);
    project.objects.append(multi);
    project.objects.append(picture);
    window.setProject(project, QString());
    window.show();
    state = window.findChild<QComboBox *>(QStringLiteral("NativeStatePreview"));
    QVERIFY(state != nullptr);
    before = window.currentEditorProject().toJson();

    auto canvasColor = [&window](const HmiObject &object) {
        QImage frame(800, 480, QImage::Format_RGB32);
        QPainter painter(&frame);
        window.m_canvas->scene()->render(&painter, QRectF(0, 0, 800, 480), QRectF(0, 0, 800, 480));
        painter.end();
        return frame.pixelColor(object.geometry.topLeft() + QPoint(8, 8));
    };

    window.m_canvas->selectObject(multi.id);
    QCOMPARE(state->count(), 3);
    state->setCurrentIndex(2);
    QCOMPARE(canvasColor(multi), colors.at(2));
    QCOMPARE(canvasColor(bit), colors.at(0));
    window.m_canvas->selectObject(picture.id);
    QCOMPARE(state->count(), 3);
    state->setCurrentIndex(2);
    QCOMPARE(canvasColor(picture), colors.at(2));

    window.m_canvas->selectObjects({});
    QVERIFY(state->isEnabled());
    QCOMPARE(state->count(), 256);
    QCOMPARE(state->currentIndex(), -1);
    QCOMPARE(state->itemText(1), QStringLiteral("状态1"));
    QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier,
                      state->mapTo(&window, QPoint(state->width() - 8, state->height() / 2)));
    QTRY_VERIFY(state->view()->isVisible());
    choicePoint = state->view()->visualRect(state->model()->index(1, 0)).center();
    QCOMPARE(state->view()->indexAt(choicePoint).row(), 1);
    QVERIFY(state->view()->viewport()->rect().contains(choicePoint));
    // Qt suppresses the opening mouse release until its double-click timer expires.
    QTest::qWait(QApplication::doubleClickInterval());
    QTest::mouseMove(state->view()->viewport(), choicePoint);
    QTest::mouseClick(state->view()->viewport(), Qt::LeftButton, Qt::NoModifier, choicePoint);
    QTRY_COMPARE(state->currentIndex(), 1);
    QCOMPARE(canvasColor(bit), colors.at(1));
    QCOMPARE(canvasColor(multi), colors.at(1));
    QCOMPARE(canvasColor(picture), colors.at(1));
    state->setCurrentIndex(2);
    QCOMPARE(canvasColor(bit), colors.at(1));
    QCOMPARE(canvasColor(multi), colors.at(2));
    QCOMPARE(canvasColor(picture), colors.at(2));

    window.m_canvas->selectObjects({bit.id, multi.id, project.objects.first().id});
    QCOMPARE(state->count(), 3);
    state->setCurrentIndex(0);
    QCOMPARE(canvasColor(bit), colors.at(0));
    QCOMPARE(canvasColor(multi), colors.at(0));
    QCOMPARE(canvasColor(picture), colors.at(2));
    window.m_canvas->selectObject(project.objects.first().id);
    QCOMPARE(state->count(), 0);
    QVERIFY(!window.m_dirty);
    QCOMPARE(window.m_projectHistory.size(), 1);
    QCOMPARE(window.currentEditorProject().toJson(), before);
    window.closeProject();
    QVERIFY(!state->isEnabled());
}

/******************************************************************************
 * Function Name   : canvasKeysUseWholeProjectHistory
 * Description     : Route hosted canvas edit requests through one shared undo and redo history.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::canvasKeysUseWholeProjectHistory() //20261110-7 add
{
    MainWindow window;
    window.setProject(workflowProject(), QString());
    window.m_canvas->selectObject(QStringLiteral("workflow-text"));
    window.m_canvas->requestEditCommand(QStringLiteral("delete"));
    QCOMPARE(window.m_canvas->objects().size(), 0);
    QCOMPARE(window.m_projectHistoryIndex, 1);
    window.m_canvas->requestEditCommand(QStringLiteral("undo"));
    QCOMPARE(window.m_canvas->objects().size(), 1);
    QVERIFY(!window.m_dirty);
    window.m_canvas->requestEditCommand(QStringLiteral("redo"));
    QCOMPARE(window.m_canvas->objects().size(), 0);
    QVERIFY(window.m_dirty);
}

/******************************************************************************
 * Function Name   : Stage2WorkflowTests::canvasControlDragDuplicatesSelection
 * Description     : Exercise duplication through viewport events and verify atomic project history.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.24
 ******************************************************************************/
void Stage2WorkflowTests::canvasControlDragDuplicatesSelection() //20261124-11 add
{
    MainWindow window;
    HmiProject project;
    HmiObject original;
    HmiObject copy;
    HmiObject expected;
    QTemporaryDir directory;
    HmiProject reloaded;
    QString path;
    QString error;
    ScreenCanvas *canvas;
    QWidget *viewport;
    QPoint start;
    QPoint drop;
    QStringList selection;
    int history;

    project = workflowProject();
    original = project.objects.first();
    window.setProject(project, QString());
    window.resize(1353, 708);
    window.show();
    canvas = window.m_canvas;
    viewport = canvas->viewport();
    canvas->setZoomPercent(125);
    start = canvas->mapFromScene(original.geometry.center());
    drop = start + QPoint(125, 75);
    QVERIFY(viewport->rect().contains(start));
    QVERIFY(viewport->rect().contains(drop));
    QTest::mouseClick(viewport, Qt::LeftButton, Qt::NoModifier, start);
    QCOMPARE(canvas->selectedObjectIds(), QStringList{original.id});
    canvas->setFocus();
    window.dispatchEditCommand(QStringLiteral("copy"));
    history = window.m_projectHistory.size();

    auto move = [&](const QPoint &position, Qt::KeyboardModifiers modifiers) {
        QMouseEvent event(QEvent::MouseMove, position, viewport->mapToGlobal(position),
                          Qt::NoButton, Qt::LeftButton, modifiers);
        QApplication::sendEvent(viewport, &event);
    };

    // A Ctrl click keeps the existing selection toggle without creating a history entry.
    QTest::mouseClick(viewport, Qt::LeftButton, Qt::ControlModifier, start);
    QVERIFY(canvas->selectedObjectIds().isEmpty());
    QCOMPARE(window.m_projectHistory.size(), history);
    QTest::mouseClick(viewport, Qt::LeftButton, Qt::NoModifier, start);

    QTest::mousePress(viewport, Qt::LeftButton, Qt::ControlModifier, start);
    move(drop, Qt::ControlModifier);
    QVERIFY(canvas->hasActiveInteraction());
    QCOMPARE(canvas->objects().size(), 1);
    QCOMPARE(canvas->objects().first().toJson(), original.toJson());
    QTest::keyClick(canvas, Qt::Key_Escape);
    QTest::mouseRelease(viewport, Qt::LeftButton, Qt::ControlModifier, drop);
    QVERIFY(!canvas->hasActiveInteraction());
    QCOMPARE(window.m_projectHistory.size(), history);
    QCOMPARE(canvas->selectedObjectIds(), QStringList{original.id});

    QTest::mousePress(viewport, Qt::LeftButton, Qt::ControlModifier, start);
    move(drop, Qt::ControlModifier);
    QTest::mouseRelease(viewport, Qt::LeftButton, Qt::ControlModifier, drop + QPoint(25, 0));
    QCOMPARE(window.m_project.objects.size(), 2);
    QCOMPARE(window.m_projectHistory.size(), history + 1);
    QCOMPARE(window.m_project.objects.first().toJson(), original.toJson());
    copy = window.m_project.objects.last();
    QVERIFY(copy.id != original.id);
    expected = original;
    expected.id = copy.id;
    expected.geometry.translate(120, 60);
    QCOMPARE(copy.toJson(), expected.toJson());
    selection = canvas->selectedObjectIds();
    QCOMPARE(selection, QStringList{copy.id});
    QVERIFY(window.m_dirty);
    QCOMPARE(window.m_objectClipboard.objectIds, QStringList{original.id});

    window.dispatchEditCommand(QStringLiteral("undo"));
    QCOMPARE(window.m_project.objects.size(), 1);
    QCOMPARE(window.m_project.objects.first().toJson(), original.toJson());
    QVERIFY(!window.m_dirty);
    window.dispatchEditCommand(QStringLiteral("redo"));
    QCOMPARE(window.m_project.objects.last().toJson(), copy.toJson());

    // Regular drags still move the selected control and keep the original separate.
    canvas->selectObject(copy.id);
    start = canvas->mapFromScene(copy.geometry.center());
    drop = start + QPoint(50, 25);
    QTest::mousePress(viewport, Qt::LeftButton, Qt::NoModifier, start);
    move(drop, Qt::NoModifier);
    QTest::mouseRelease(viewport, Qt::LeftButton, Qt::NoModifier, drop);
    QCOMPARE(canvas->objects().size(), 2);
    QCOMPARE(canvas->objects().first().toJson(), original.toJson());
    QCOMPARE(canvas->objects().last().geometry, copy.geometry.translated(40, 20));

    // The copy keeps the original grab point until the page edge stops it.
    start = canvas->mapFromScene(canvas->objects().last().geometry.center());
    drop = canvas->mapFromScene(QPoint(2, 2));
    QVERIFY(viewport->rect().contains(start));
    QVERIFY(viewport->rect().contains(drop));
    QTest::mousePress(viewport, Qt::LeftButton, Qt::ControlModifier, start);
    move(drop, Qt::ControlModifier);
    QTest::mouseRelease(viewport, Qt::LeftButton, Qt::ControlModifier, drop);
    QCOMPARE(window.m_project.objects.size(), 3);
    QCOMPARE(window.m_project.objects.last().geometry, QRect(QPoint(), original.geometry.size()));
    QVERIFY(directory.isValid());
    path = directory.filePath(QStringLiteral("ctrl-drag.ksp"));
    QVERIFY2(window.m_project.save(path, &error), qPrintable(error));
    QVERIFY2(HmiProject::load(path, &reloaded, &error), qPrintable(error));
    QCOMPARE(reloaded.objects.last().toJson(), window.m_project.objects.last().toJson());
}

/******************************************************************************
 * Function Name   : textClipboardDoesNotModifyCanvas
 * Description     : Keep focused text edit operations separate from selected canvas controls.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::textClipboardDoesNotModifyCanvas() //20261110-7 add
{
    MainWindow window;
    QLineEdit editor(&window);
    QByteArray before;
    window.setProject(workflowProject(), QString());
    window.m_canvas->selectObject(QStringLiteral("workflow-text"));
    window.show();
    editor.show();
    editor.setFocus();
    QTRY_COMPARE(QApplication::focusWidget(), &editor);
    before = QJsonDocument(window.currentEditorProject().toJson()).toJson();
    editor.setText(QStringLiteral("copy text"));
    editor.selectAll();
    window.dispatchEditCommand(QStringLiteral("copy"));
    QCOMPARE(QApplication::clipboard()->text(), QStringLiteral("copy text"));
    window.dispatchEditCommand(QStringLiteral("cut"));
    QVERIFY(editor.text().isEmpty());
    window.dispatchEditCommand(QStringLiteral("undo"));
    QCOMPARE(editor.text(), QStringLiteral("copy text"));
    QCOMPARE(QJsonDocument(window.currentEditorProject().toJson()).toJson(), before);
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : numericDraftCommandsStayLocal
 * Description     : Preserve project history when numeric property text receives menu clipboard and undo operations.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::numericDraftCommandsStayLocal() //20261110-7 add
{
    MainWindow window;
    HmiProject changed;
    QLineEdit *line;
    int history;
    window.setProject(workflowProject(), QString());
    changed = window.currentEditorProject();
    changed.name = QStringLiteral("retain whole history");
    QVERIFY(window.commitProjectDraft(changed));
    history = window.m_projectHistoryIndex;
    window.show();
    window.m_canvas->selectObject(QStringLiteral("workflow-text"));
    window.m_rightTabs->setCurrentIndex(1);
    window.m_xSpin->setFocus();
    QTest::qWait(20);
    line = window.m_xSpin->findChild<QLineEdit *>();
    QVERIFY(line != nullptr);
    line->selectAll();
    window.dispatchEditCommand(QStringLiteral("copy"));
    QCOMPARE(QApplication::clipboard()->text(), line->selectedText());
    window.dispatchEditCommand(QStringLiteral("undo"));
    QCOMPARE(window.m_projectHistoryIndex, history);
    window.dispatchEditCommand(QStringLiteral("delete"));
    QCOMPARE(window.m_canvas->objects().size(), 1);
    QCOMPARE(window.m_project.name, changed.name);
}

/******************************************************************************
 * Function Name   : focusedWindowDestructionIsSafe
 * Description     : Repeatedly release focused editor windows without calling slots on destroyed derived state.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::focusedWindowDestructionIsSafe() //20261110-7 add
{
    MainWindow *window;
    int index;
    for (index = 0; index < 8; ++index) {
        window = new MainWindow();
        window->setProject(workflowProject(), QString());
        window->show();
        window->m_canvas->selectObject(QStringLiteral("workflow-text"));
        window->m_rightTabs->setCurrentIndex(1);
        window->m_xSpin->setFocus();
        QApplication::processEvents();
        delete window;
        QApplication::processEvents();
    }
}

/******************************************************************************
 * Function Name   : macroSearchLocationOpensSource
 * Description     : Navigate a macro result to a read-only source line without changing its definition.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::macroSearchLocationOpensSource() //20261110-7 add
{
    MainWindow window;
    HmiProject project = workflowProject();
    bool checked = false;
    project.macros.insert(QStringLiteral("macro"), QStringLiteral("line1\nline2\nline3"));
    window.setProject(project, QString());
    QTimer::singleShot(0, [&checked, &window]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QPlainTextEdit *source = dialog->findChild<QPlainTextEdit *>(QStringLiteral("Stage2MacroSource"));
        QString before;
        checked = source != nullptr && source->isReadOnly() && source->textCursor().blockNumber() == 1;
        if (source != nullptr) {
            before = source->toPlainText();
            source->setFocus();
            QApplication::processEvents();
            window.dispatchEditCommand(QStringLiteral("delete"));
            checked = checked && source->toPlainText() == before;
        }
        dialog->reject();
    });
    window.showMacroSource(QStringLiteral("macro"), 2);
    QVERIFY(checked);
    QCOMPARE(window.m_project.macros, project.macros);
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : cancelledWizardPreservesDirtyDocument
 * Description     : Reject the real new-project wizard without replacing dirty data or the current path.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::cancelledWizardPreservesDirtyDocument() //20261110-7 add
{
    MainWindow window;
    HmiProject changed;
    QByteArray before;
    window.setProject(workflowProject(), QStringLiteral("unchanged.ksp"));
    changed = window.currentEditorProject();
    changed.name = QStringLiteral("unsaved");
    QVERIFY(window.commitProjectDraft(changed));
    before = QJsonDocument(window.currentEditorProject().toJson()).toJson();
    QTimer::singleShot(0, []() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (dialog != nullptr) { dialog->reject(); }
    });
    window.createProject();
    QCOMPARE(QJsonDocument(window.currentEditorProject().toJson()).toJson(), before);
    QCOMPARE(window.m_currentPath, QStringLiteral("unchanged.ksp"));
    QVERIFY(window.m_dirty);
}

/******************************************************************************
 * Function Name   : invalidOpenPreservesDocument
 * Description     : Reject malformed input before asking to discard or changing the live document.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::invalidOpenPreservesDocument() //20261110-7 add
{
    MainWindow window;
    QTemporaryDir directory;
    QFile file(directory.filePath(QStringLiteral("broken.ksp")));
    HmiProject changed;
    QString error;
    QByteArray before;
    window.setProject(workflowProject(), QStringLiteral("old.ksp"));
    changed = window.currentEditorProject();
    changed.name = QStringLiteral("unsaved");
    QVERIFY(window.commitProjectDraft(changed));
    before = QJsonDocument(window.currentEditorProject().toJson()).toJson();
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("{invalid");
    file.close();
    QVERIFY(!window.openProjectPath(file.fileName(), &error));
    QVERIFY(!error.isEmpty());
    QCOMPARE(QJsonDocument(window.currentEditorProject().toJson()).toJson(), before);
    QCOMPARE(window.m_currentPath, QStringLiteral("old.ksp"));
    QVERIFY(window.m_dirty);
}

/******************************************************************************
 * Function Name   : openingSamePathReloadsAfterSave
 * Description     : Reload a recent-file target after the unsaved-work prompt saves into that same file.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::openingSamePathReloadsAfterSave() //20261110-7 add
{
    MainWindow window;
    QTemporaryDir directory;
    HmiProject changed;
    QString error;
    QString path = directory.filePath(QStringLiteral("same.ksp"));
    window.setProject(workflowProject(), path);
    QVERIFY(window.writeProject(path));
    changed = window.currentEditorProject();
    changed.name = QStringLiteral("saved during open");
    QVERIFY(window.commitProjectDraft(changed));
    QTimer::singleShot(0, []() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (box != nullptr) { box->button(QMessageBox::Save)->click(); }
    });
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    QCOMPARE(window.m_project.name, changed.name);
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : saveDiscardCancelClose
 * Description     : Exercise all three unsaved-work outcomes through the real close command.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::saveDiscardCancelClose() //20261110-7 add
{
    MainWindow window;
    QTemporaryDir directory;
    HmiProject changed;
    HmiProject loaded;
    QString error;
    QMessageBox::StandardButton response;
    window.setProject(workflowProject(), directory.filePath(QStringLiteral("saved.ksp")));
    changed = window.currentEditorProject();
    changed.name = QStringLiteral("saved change");
    QVERIFY(window.commitProjectDraft(changed));
    Q_FOREACH (response, (QList<QMessageBox::StandardButton>{QMessageBox::Cancel, QMessageBox::Save})) {
        QTimer::singleShot(0, [response]() {
            QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            if (box != nullptr) { box->button(response)->click(); }
        });
        window.closeProject();
        QCOMPARE(window.m_hasProject, response == QMessageBox::Cancel);
    }
    QVERIFY2(HmiProject::load(directory.filePath(QStringLiteral("saved.ksp")), &loaded, &error), qPrintable(error));
    QCOMPARE(loaded.name, QStringLiteral("saved change"));
    window.setProject(workflowProject(), QString());
    QVERIFY(window.commitProjectDraft(changed));
    QTimer::singleShot(0, []() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (box != nullptr) { box->button(QMessageBox::Discard)->click(); }
    });
    window.closeProject();
    QVERIFY(!window.m_hasProject);
}

/******************************************************************************
 * Function Name   : standardDialogsFollowInterfaceLanguage
 * Description     : Check real close and file prompts, standard controls and the persisted interface setting.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.24
 ******************************************************************************/
void Stage2WorkflowTests::standardDialogsFollowInterfaceLanguage() //20261124-7 add
{
    MainWindow window;
    HmiProject changed = workflowProject();
    QAction *chinese = window.m_commands.value(QStringLiteral("action_chinese"));
    QAction *english = window.m_commands.value(QStringLiteral("action_english"));
    QSettings settings(QStringLiteral("KinSealStudioLite"), QStringLiteral("Editor"));

    QVERIFY(chinese != nullptr && english != nullptr && chinese->isEnabled() && english->isEnabled());
    window.setProject(changed, QString());
    changed.name = QStringLiteral("unsaved-language-test");
    QVERIFY(window.commitProjectDraft(changed));
    window.show();
    auto caption = [](QAbstractButton *button) {
        return button == nullptr ? QString() : button->text().section(QLatin1Char('('), 0, 0)
            .remove(QLatin1Char('&')).trimmed();
    };
    auto closeLabels = [&]() {
        QStringList labels;
        QTimer::singleShot(0, &window, [&]() {
            QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            if (box != nullptr)
            {
                labels = {caption(box->button(QMessageBox::Save)), caption(box->button(QMessageBox::Discard)),
                          caption(box->button(QMessageBox::Cancel))};
                box->button(QMessageBox::Cancel)->click();
            }
        });
        window.close();
        return labels;
    };
    auto openLabels = [&]() {
        QStringList labels;
        QTimer::singleShot(0, &window, [&]() {
            QFileDialog *file = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
            if (file != nullptr)
            {
                QDialogButtonBox *buttons = file->findChild<QDialogButtonBox *>();
                if (buttons != nullptr)
                {
                    labels = {caption(buttons->button(QDialogButtonBox::Open)),
                              caption(buttons->button(QDialogButtonBox::Cancel))};
                }
                file->reject();
            }
        });
        window.openProject();
        return labels;
    };
    auto pagePropertyLabels = [&]() {
        QStringList labels;
        QTimer::singleShot(0, &window, [&]() {
            QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (dialog == nullptr)
            {
                return;
            }
            QDialogButtonBox *buttons = dialog->findChild<QDialogButtonBox *>(QString(), Qt::FindDirectChildrenOnly);
            if (buttons != nullptr)
            {
                labels = {caption(buttons->button(QDialogButtonBox::Ok)), caption(buttons->button(QDialogButtonBox::Cancel)),
                          caption(buttons->button(QDialogButtonBox::Help))};
            }
            dialog->reject();
        });
        window.m_commands.value(QStringLiteral("action_openpropertiset"))->trigger();
        return labels;
    };
    auto helpCloseLabel = [&]() {
        window.m_commands.value(QStringLiteral("action_H"))->trigger();
        QApplication::processEvents();
        QDialog *help = window.findChild<QDialog *>(QStringLiteral("LocalHelpDialog"));
        QDialogButtonBox *buttons = help == nullptr ? nullptr : help->findChild<QDialogButtonBox *>();
        if (buttons == nullptr)
        {
            return QString();
        }
        const QString text = caption(buttons->button(QDialogButtonBox::Close));
        buttons->button(QDialogButtonBox::Close)->click();
        return text;
    };

    QCOMPARE(closeLabels(), (QStringList{QStringLiteral("保存"), QStringLiteral("丢弃"), QStringLiteral("取消")}));
    QVERIFY(window.m_dirty && window.isVisible());
    QVERIFY(chinese->isChecked());
    QCOMPARE(openLabels(), (QStringList{QStringLiteral("打开"), QStringLiteral("取消")}));
    QCOMPARE(pagePropertyLabels(), (QStringList{QStringLiteral("确定"), QStringLiteral("取消"), QStringLiteral("帮助")})); //20261124-8 add
    QCOMPARE(helpCloseLabel(), QStringLiteral("关闭"));
    {
        QMessageBox overwrite(QMessageBox::Question, QString(), QString(), QMessageBox::Yes | QMessageBox::No);
        QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Close | QDialogButtonBox::Help);
        QColorDialog color;
        QInputDialog input;
        const auto *colorButtons = color.findChild<QDialogButtonBox *>();
        QCOMPARE(caption(overwrite.button(QMessageBox::Yes)), QStringLiteral("是"));
        QCOMPARE(caption(overwrite.button(QMessageBox::No)), QStringLiteral("否"));
        QCOMPARE(caption(buttons.button(QDialogButtonBox::Ok)), QStringLiteral("确定"));
        QCOMPARE(caption(buttons.button(QDialogButtonBox::Close)), QStringLiteral("关闭"));
        QCOMPARE(caption(buttons.button(QDialogButtonBox::Help)), QStringLiteral("帮助"));
        QVERIFY(colorButtons != nullptr);
        QCOMPARE(caption(colorButtons->button(QDialogButtonBox::Cancel)), QStringLiteral("取消"));
        QCOMPARE(input.cancelButtonText(), QStringLiteral("取消"));
    }

    english->trigger();
    const QStringList englishClose = closeLabels();
    const QStringList englishOpen = openLabels();
    const QStringList englishPageProperties = pagePropertyLabels(); //20261124-8 add
    const QString englishHelpClose = helpCloseLabel();
    QString englishInputCancel;
    bool englishChecked;
    {
        MainWindow reopened;
        QInputDialog input;
        englishInputCancel = input.cancelButtonText();
        englishChecked = reopened.m_commands.value(QStringLiteral("action_english"))->isChecked();
    }
    const QString savedEnglish = settings.value(QStringLiteral("uiLanguage")).toString();
    chinese->trigger();
    QCOMPARE(englishClose, (QStringList{QStringLiteral("Save"), QStringLiteral("Discard"), QStringLiteral("Cancel")}));
    QCOMPARE(englishOpen, (QStringList{QStringLiteral("Open"), QStringLiteral("Cancel")}));
    QCOMPARE(englishPageProperties, (QStringList{QStringLiteral("OK"), QStringLiteral("Cancel"), QStringLiteral("Help")}));
    QCOMPARE(englishHelpClose, QStringLiteral("Close"));
    QCOMPARE(englishInputCancel, QStringLiteral("Cancel"));
    QVERIFY(englishChecked);
    QCOMPARE(savedEnglish, QStringLiteral("en"));
    QCOMPARE(closeLabels(), (QStringList{QStringLiteral("保存"), QStringLiteral("丢弃"), QStringLiteral("取消")}));
    QCOMPARE(helpCloseLabel(), QStringLiteral("关闭"));
    QVERIFY(window.m_dirty);
    settings.remove(QStringLiteral("uiLanguage"));
}

/******************************************************************************
 * Function Name   : cancelledSaveAsBlocksClose
 * Description     : Preserve an unsaved document when Save is chosen but its destination dialog is cancelled.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::cancelledSaveAsBlocksClose() //20261110-7 add
{
    MainWindow window;
    HmiProject changed;
    QTimer timer;
    window.setProject(workflowProject(), QString());
    changed = window.currentEditorProject();
    changed.name = QStringLiteral("keep me");
    QVERIFY(window.commitProjectDraft(changed));
    connect(&timer, &QTimer::timeout, []() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        QFileDialog *dialog = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
        if (box != nullptr) { box->button(QMessageBox::Save)->click(); }
        if (dialog != nullptr) { dialog->reject(); }
    });
    timer.start(10);
    window.closeProject();
    QVERIFY(window.m_hasProject);
    QVERIFY(window.m_dirty);
    QVERIFY(window.m_currentPath.isEmpty());
}

/******************************************************************************
 * Function Name   : failedSaveBlocksClose
 * Description     : Keep live work and history when the atomic file save fails.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::failedSaveBlocksClose() //20261110-7 add
{
    MainWindow window;
    HmiProject changed;
    QTemporaryDir directory;
    QTimer timer;
    window.setProject(workflowProject(), directory.filePath(QStringLiteral("missing/fail.ksp")));
    changed = window.currentEditorProject();
    changed.name = QStringLiteral("keep failed save");
    QVERIFY(window.commitProjectDraft(changed));
    connect(&timer, &QTimer::timeout, []() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (box != nullptr) {
            box->button(box->standardButtons().testFlag(QMessageBox::Save) ? QMessageBox::Save : QMessageBox::Ok)->click();
        }
    });
    timer.start(10);
    window.closeProject();
    QVERIFY(window.m_hasProject);
    QVERIFY(window.m_dirty);
    QCOMPARE(window.m_project.name, changed.name);
}

/******************************************************************************
 * Function Name   : actualTabsComboAndTreeNavigateById
 * Description     : Switch two pages and a window with identical labels through stable UI identities.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::actualTabsComboAndTreeNavigateById() //20261110-7 add
{
    MainWindow window;
    HmiProject project = workflowProject();
    HmiPage second;
    HmiPage popup;
    QString error;
    QTreeWidgetItem *item;
    int index;
    second.id = QStringLiteral("second");
    second.name = QStringLiteral("second page");
    second.objects = project.objects;
    second.objects[0].id = QStringLiteral("second-object");
    second.objects[0].text = QStringLiteral("SECOND");
    popup.id = QStringLiteral("popup");
    popup.name = QStringLiteral("popup page");
    popup.kind = QStringLiteral("window");
    popup.width = 240;
    popup.height = 180;
    QVERIFY2(project.addPage(second, &error), qPrintable(error));
    QVERIFY2(project.addPage(popup, &error), qPrintable(error));
    window.setProject(project, QString());
    QCOMPARE(window.m_pageTabs->count(), 3);
    index = window.m_pageCombo->findData(second.id);
    window.m_pageCombo->setCurrentIndex(index);
    QCOMPARE(window.m_project.activePageId, second.id);
    QCOMPARE(window.m_canvas->objects().first().text, QStringLiteral("SECOND"));
    window.m_pageTabs->setCurrentIndex(2);
    QCOMPARE(window.m_project.activePageId, popup.id);
    QVERIFY(window.m_canvas->objects().isEmpty());
    QCOMPARE(window.m_pageTabs->currentWidget(), static_cast<QWidget *>(window.m_canvas));
    item = window.m_projectTree->topLevelItem(0)->child(0);
    QVERIFY(item != nullptr);
    window.m_projectTree->setCurrentItem(item);
    QCOMPARE(window.m_project.activePageId, project.activePageId);
    QCOMPARE(window.m_canvas->objects().first().text, QStringLiteral("TEST"));
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : crossPageReplacementUndoAndSavedCheckpoint
 * Description     : Undo and redo a whole-project address batch across pages and retain the saved checkpoint.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::visitingUnsortedLayersIsNavigationOnly() //20261110-7 add
{
    MainWindow window;
    HmiProject project = workflowProject();
    HmiPage second;
    HmiObject lower = project.objects.first();
    QString error;
    second.id = QStringLiteral("unsorted");
    second.name = QStringLiteral("unsorted page");
    second.objects = project.objects;
    second.objects[0].id = QStringLiteral("upper");
    second.objects[0].layer = 2;
    lower.id = QStringLiteral("lower");
    lower.layer = 1;
    second.objects.append(lower);
    QVERIFY(project.addPage(second, &error));
    window.setProject(project, QString());
    QVERIFY(window.activatePageId(second.id));
    QVERIFY(!window.projectIsModified());
    window.setDirty(true);
    QVERIFY(!window.m_dirty);
    QCOMPARE(window.m_projectHistoryIndex, 0);
}

/******************************************************************************
 * Function Name   : crossPageReplacementUndoAndSavedCheckpoint
 * Description     : Restore a cross-page batch as one transaction and preserve its independent save checkpoint.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::crossPageReplacementUndoAndSavedCheckpoint() //20261110-7 add
{
    MainWindow window;
    HmiProject project = workflowProject();
    HmiProject candidate;
    HmiProject loaded;
    HmiPage second;
    Stage2GlobalTools::AddressCandidate preview;
    Stage2GlobalTools::Scope scope;
    QTemporaryDir directory;
    QString error;
    QVector<QByteArray> payloads;
    project.objects[0].type = QStringLiteral("switch");
    project.objects[0].hasBitButtonProperties = true;
    project.objects[0].hasWordButtonProperties = true;
    project.objects[0].bitButton.indicator = false;
    project.objects[0].bitButton.dualMonitor = false;
    second.id = QStringLiteral("second");
    second.name = QStringLiteral("second page");
    second.objects = project.objects;
    second.objects[0].id = QStringLiteral("second-object");
    QVERIFY(project.addPage(second, &error));
    window.setProject(project, QString());
    QVERIFY2(Stage2GlobalTools::previewAddressReplacement(project, QStringLiteral("LW0"), QStringLiteral("LW2"), scope, &preview, &error), qPrintable(error));
    QVERIFY2(Stage2GlobalTools::applyCandidate(project, preview, &candidate, &error), qPrintable(error));
    QVERIFY2(window.commitProjectDraft(candidate, &error), qPrintable(error));
    QVERIFY(window.m_dirty);
    QCOMPARE(window.currentEditorProject().allPages().at(0).objects.first().address, QStringLiteral("LW2"));
    QCOMPARE(window.currentEditorProject().allPages().at(1).objects.first().address, QStringLiteral("LW2"));
    QVERIFY(window.writeProject(directory.filePath(QStringLiteral("batch.ksp"))));
    QVERIFY(!window.m_dirty);
    window.dispatchEditCommand(QStringLiteral("undo"));
    QVERIFY(window.m_dirty);
    QCOMPARE(window.currentEditorProject().allPages().at(1).objects.first().address, QStringLiteral("LW0"));
    window.dispatchEditCommand(QStringLiteral("redo"));
    QVERIFY(!window.m_dirty);
    QVERIFY(HmiProject::load(directory.filePath(QStringLiteral("batch.ksp")), &loaded, &error));
    QCOMPARE(loaded.allPages().at(1).objects.first().address, QStringLiteral("LW2"));
    QVERIFY(!buildProjectPayload(loaded, &payloads, &error));
    QVERIFY(payloads.isEmpty());
}

/******************************************************************************
 * Function Name   : invalidDraftAndNativeOverwriteAreAtomic
 * Description     : Reject invalid configuration and native-source overwrite without publishing partial changes.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::invalidDraftAndNativeOverwriteAreAtomic() //20261110-7 add
{
    MainWindow window;
    HmiProject invalid;
    QTemporaryDir directory;
    QFile native(directory.filePath(QStringLiteral("existing.sm")));
    QString error;
    QByteArray before;
    window.setProject(workflowProject(), QStringLiteral("unchanged.ksp"));
    before = QJsonDocument(window.currentEditorProject().toJson()).toJson();
    invalid = window.currentEditorProject();
    invalid.objects.append(invalid.objects.first());
    QVERIFY(!window.commitProjectDraft(invalid, &error));
    QCOMPARE(QJsonDocument(window.currentEditorProject().toJson()).toJson(), before);
    QVERIFY(native.open(QIODevice::WriteOnly));
    native.write("preserve native");
    native.close();
    QTimer::singleShot(0, []() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (box != nullptr) { box->button(QMessageBox::Ok)->click(); }
    });
    QVERIFY(!window.writeProject(native.fileName()));
    QVERIFY(native.open(QIODevice::ReadOnly));
    QCOMPARE(native.readAll(), QByteArray("preserve native"));
    QCOMPARE(window.m_currentPath, QStringLiteral("unchanged.ksp"));
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : reopenAfterCloseRestoresEditor
 * Description     : Restore all command and navigation surfaces after closing then reopening a document.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::reopenAfterCloseRestoresEditor() //20261110-7 add
{
    MainWindow window;
    window.closeProject();
    QVERIFY(!window.m_systemSettingsTree->isEnabled());
    window.setProject(workflowProject(), QString());
    QVERIFY(window.m_hasProject);
    QVERIFY(window.m_canvas->isEnabled());
    QVERIFY(window.m_pageTabs->isEnabled());
    QVERIFY(window.m_systemSettingsTree->isEnabled());
    QVERIFY(window.m_commands.value(QStringLiteral("action_USBdownload"))->isEnabled());
}

/******************************************************************************
 * Function Name   : openedProjectShowsProtocolsUnderTheirOwnPorts
 * Description     : Reopen a saved project and check protocol ownership in the visible tree.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void Stage2WorkflowTests::openedProjectShowsProtocolsUnderTheirOwnPorts() //20261122-6 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    PlcConnection connection;
    QString error;
    QString path;
    MainWindow window;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> ports;

    QVERIFY(directory.isValid());
    project.targetModel = QStringLiteral("AMZ070W01");
    connection.id = QStringLiteral("com2");
    connection.channel = PlcChannel::Com2;
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.station = QStringLiteral("1");
    project.connections.append(connection);
    path = directory.filePath(QStringLiteral("port-com2.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    ports = tree->findItems(QStringLiteral("COM1"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(ports.size(), 1);
    QCOMPARE(ports.first()->childCount(), 0);
    ports = tree->findItems(QStringLiteral("COM2"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(ports.size(), 1);
    QCOMPARE(ports.first()->childCount(), 1);
    QCOMPARE(ports.first()->child(0)->text(0), QStringLiteral("Modbus_RTU"));
}

/******************************************************************************
 * Function Name   : serialPortSettingsAreIndependentAndPersisted
 * Description     : Open either port from the tree and exercise cancel, save, reopen and reset.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void Stage2WorkflowTests::serialPortSettingsAreIndependentAndPersisted() //20261122-6 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    MainWindow window;
    MainWindow reopened;
    QString error;
    QString path;
    QTreeWidget *tree;
    auto editPort = [](QTreeWidget *settings, const QString &port, const QString &baud,
                       const QString &online, bool confirm, bool reset) {
        QStringList observed;
        const QList<QTreeWidgetItem *> items = settings->findItems(port, Qt::MatchExactly | Qt::MatchRecursive);
        if (items.size() != 1)
        {
            return observed;
        }
        QTimer::singleShot(0, settings, [&]() {
            QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (dialog == nullptr)
            {
                return;
            }
            QLineEdit *name = dialog->findChild<QLineEdit *>(QStringLiteral("edit_name"));
            QComboBox *speed = dialog->findChild<QComboBox *>(QStringLiteral("PlcPortBaud"));
            QComboBox *pcPort = dialog->findChild<QComboBox *>(QStringLiteral("PlcPortOnline"));
            QComboBox *mode = dialog->findChild<QComboBox *>(QStringLiteral("PlcPortMode"));
            QDialogButtonBox *buttons = dialog->findChild<QDialogButtonBox *>();
            QPushButton *defaults = dialog->findChild<QPushButton *>(QStringLiteral("PlcPortDefaults"));
            if (dialog->windowTitle() != QStringLiteral("连接设置") || name == nullptr || !name->isReadOnly() ||
                speed == nullptr || pcPort == nullptr || mode == nullptr || buttons == nullptr || defaults == nullptr)
            {
                observed.append(dialog->windowTitle());
                dialog->reject();
                return;
            }
            observed = {name->text(), speed->currentText(), pcPort->currentText(), mode->currentText()};
            if (!baud.isEmpty()) { speed->setCurrentText(baud); }
            if (!online.isEmpty()) { pcPort->setCurrentText(online); }
            if (reset) { defaults->click(); }
            buttons->button(confirm ? QDialogButtonBox::Ok : QDialogButtonBox::Cancel)->click();
        });
        settings->itemDoubleClicked(items.first(), 0);
        return observed;
    };

    QVERIFY(directory.isValid());
    project.targetModel = QStringLiteral("AMZ070W01");
    path = directory.filePath(QStringLiteral("serial-settings.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    QCOMPARE(editPort(tree, QStringLiteral("COM2"), QStringLiteral("19200"), QStringLiteral("COM11"), false, false),
             (QStringList{QStringLiteral("COM2"), QStringLiteral("9600"), QStringLiteral("COM2"), QStringLiteral("232")}));
    QVERIFY(!window.m_dirty);
    QCOMPARE(editPort(tree, QStringLiteral("COM2"), QStringLiteral("19200"), QStringLiteral("COM11"), true, false),
             (QStringList{QStringLiteral("COM2"), QStringLiteral("9600"), QStringLiteral("COM2"), QStringLiteral("232")}));
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    tree = reopened.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    QCOMPARE(editPort(tree, QStringLiteral("COM1"), {}, {}, false, false),
             (QStringList{QStringLiteral("COM1"), QStringLiteral("9600"), QStringLiteral("COM1"), QStringLiteral("485")}));
    QCOMPARE(editPort(tree, QStringLiteral("COM2"), {}, {}, true, true),
             (QStringList{QStringLiteral("COM2"), QStringLiteral("19200"), QStringLiteral("COM11"), QStringLiteral("232")}));
    QCOMPARE(editPort(tree, QStringLiteral("COM2"), {}, {}, false, false),
             (QStringList{QStringLiteral("COM2"), QStringLiteral("9600"), QStringLiteral("COM2"), QStringLiteral("232")}));
}

/******************************************************************************
 * Function Name   : protocolTreeEditsAreScopedToTheirInstance
 * Description     : Rename and change timing on COM2 while retaining COM1 and the original stable ID.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void Stage2WorkflowTests::protocolTreeEditsAreScopedToTheirInstance() //20261122-7 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    PlcConnection connection;
    MainWindow window;
    MainWindow reopened;
    QString error;
    QString path;
    QTreeWidget *tree;
    auto editProtocol = [](QTreeWidget *settings, const QString &port, bool edit, bool confirm) {
        QStringList fields;
        const QList<QTreeWidgetItem *> ports = settings->findItems(port, Qt::MatchExactly | Qt::MatchRecursive);
        if (ports.size() != 1 || ports.first()->childCount() != 1)
        {
            return fields;
        }
        QTimer::singleShot(0, settings, [&]() {
            QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (dialog == nullptr)
            {
                return;
            }
            QLineEdit *name = dialog->findChild<QLineEdit *>(QStringLiteral("edit_name"));
            QComboBox *series = dialog->findChild<QComboBox *>(QStringLiteral("combo_protocol"));
            QSpinBox *station = dialog->findChild<QSpinBox *>(QStringLiteral("spin_stationIndex"));
            QSpinBox *timeout = dialog->findChild<QSpinBox *>(QStringLiteral("spin_time"));
            QDialogButtonBox *buttons = dialog->findChild<QDialogButtonBox *>();
            if (dialog->windowTitle() != QStringLiteral("协议") || name == nullptr || series == nullptr ||
                station == nullptr || timeout == nullptr || buttons == nullptr)
            {
                fields.append(dialog->windowTitle());
                dialog->reject();
                return;
            }
            fields = {series->currentText(), name->text(), QString::number(station->value()), QString::number(timeout->value())};
            if (edit)
            {
                name->setText(QStringLiteral("PLC-B"));
                timeout->setValue(2200);
            }
            buttons->button(confirm ? QDialogButtonBox::Ok : QDialogButtonBox::Cancel)->click();
        });
        settings->itemDoubleClicked(ports.first()->child(0), 0);
        return fields;
    };

    QVERIFY(directory.isValid());
    project.targetModel = QStringLiteral("AMZ070W01R");
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.id = QStringLiteral("com1");
    connection.channel = PlcChannel::Com1;
    connection.station = QStringLiteral("1");
    project.connections.append(connection);
    connection.id = QStringLiteral("com2");
    connection.channel = PlcChannel::Com2;
    connection.station = QStringLiteral("2");
    project.connections.append(connection);
    path = directory.filePath(QStringLiteral("protocol-settings.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    QCOMPARE(editProtocol(tree, QStringLiteral("COM2"), true, false),
             (QStringList{QStringLiteral("Modbus_RTU"), QStringLiteral("Modbus_RTU"), QStringLiteral("2"), QStringLiteral("1000")}));
    QVERIFY(!window.m_dirty);
    QCOMPARE(editProtocol(tree, QStringLiteral("COM2"), true, true),
             (QStringList{QStringLiteral("Modbus_RTU"), QStringLiteral("Modbus_RTU"), QStringLiteral("2"), QStringLiteral("1000")}));
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    tree = reopened.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    QCOMPARE(editProtocol(tree, QStringLiteral("COM1"), false, false),
             (QStringList{QStringLiteral("Modbus_RTU"), QStringLiteral("Modbus_RTU"), QStringLiteral("1"), QStringLiteral("1000")}));
    QCOMPARE(editProtocol(tree, QStringLiteral("COM2"), false, false),
             (QStringList{QStringLiteral("Modbus_RTU"), QStringLiteral("PLC-B"), QStringLiteral("2"), QStringLiteral("2200")}));
    const QList<QTreeWidgetItem *> named = tree->findItems(QStringLiteral("PLC-B"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(named.size(), 1);
    QCOMPARE(named.first()->parent()->text(0), QStringLiteral("COM2"));
    QCOMPARE(named.first()->data(0, Qt::UserRole + 1).toString(), QStringLiteral("com2"));
}

/******************************************************************************
 * Function Name   : triggerSettingsTreeAction
 * Description     : Select an action from the actual settings tree context menu.
 * Input Parameters: tree, item, actionText, whileDialogOpen, observedActions.
 * Return Value    : True if the menu exposes the requested action.
 * Date            : 2026.11.22
 ******************************************************************************/
static bool triggerSettingsTreeAction(QTreeWidget *tree, QTreeWidgetItem *item, const QString &actionText,
                                      const std::function<void()> &whileDialogOpen = {},
                                      QStringList *observedActions = nullptr) //20261123-9 change
{
    bool triggered = false;
    QTimer menuSelection;

    tree->scrollToItem(item);
    menuSelection.setSingleShot(true);
    QObject::connect(&menuSelection, &QTimer::timeout, tree, [&]() {
        QMenu *menu = qobject_cast<QMenu *>(QApplication::activePopupWidget());
        QAction *action;
        QList<QAction *> actions;
        if (menu == nullptr)
        {
            return;
        }
        actions = menu->actions();
        menu->close();
        if (observedActions != nullptr) //20261123-9 add
        {
            observedActions->clear();
            Q_FOREACH (action, actions)
            {
                observedActions->append(action->text());
            }
        }
        Q_FOREACH (action, actions)
        {
            if (action->text() == actionText && action->isEnabled())
            {
                if (whileDialogOpen)
                {
                    QTimer::singleShot(0, tree, whileDialogOpen);
                }
                triggered = true;
                action->trigger();
                return;
            }
        }
    });
    menuSelection.start(0);
    tree->customContextMenuRequested(tree->visualItemRect(item).center());
    return triggered;
}

/******************************************************************************
 * Function Name   : newPageFromSettingsTreeCancelsOrReopens
 * Description     : Open the ordinary page creation action and verify cancellation, activation and persistence.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void Stage2WorkflowTests::newPageFromSettingsTreeCancelsOrReopens() //20261122-9 add
{
    QTemporaryDir directory;
    HmiProject original = workflowProject();
    HmiProject created;
    MainWindow window;
    MainWindow reopened;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> groups;
    QStringList displayedDefaults;
    QString error;
    QString path;

    QVERIFY(directory.isValid());
    original.targetModel = QStringLiteral("AMZ070W01");
    path = directory.filePath(QStringLiteral("new-page.ksp"));
    QVERIFY2(original.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    groups = tree->findItems(QStringLiteral("画面"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(groups.size(), 1);
    auto finishDraft = [&displayedDefaults](bool confirm) {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QLineEdit *name;
        QSpinBox *number;
        QDialogButtonBox *buttons;

        if (dialog == nullptr)
        {
            return;
        }
        name = dialog->findChild<QLineEdit *>(QStringLiteral("newPageNameEdit"));
        number = dialog->findChild<QSpinBox *>(QStringLiteral("newPageNumberSpin"));
        buttons = dialog->findChild<QDialogButtonBox *>();
        if (name == nullptr || number == nullptr || buttons == nullptr)
        {
            dialog->reject();
            return;
        }
        displayedDefaults = {dialog->windowTitle(), name->text(), QString::number(number->value())};
        if (!confirm)
        {
            name->setText(QStringLiteral("取消的新画面"));
        }
        buttons->button(confirm ? QDialogButtonBox::Ok : QDialogButtonBox::Cancel)->click();
    };
    QVERIFY2(triggerSettingsTreeAction(tree, groups.first(), QStringLiteral("新建画面"), [&]() { finishDraft(false); }),
             "The ordinary page group must offer New Page.");
    QCOMPARE(displayedDefaults, (QStringList{QStringLiteral("新建画面"), QStringLiteral("画面2"), QStringLiteral("1")}));
    QCOMPARE(window.currentEditorProject().toJson(), original.toJson());
    QVERIFY(!window.m_dirty);
    displayedDefaults.clear();
    QVERIFY(triggerSettingsTreeAction(tree, groups.first(), QStringLiteral("新建画面"), [&]() { finishDraft(true); }));
    QCOMPARE(displayedDefaults, (QStringList{QStringLiteral("新建画面"), QStringLiteral("画面2"), QStringLiteral("1")}));
    created = window.currentEditorProject();
    QCOMPARE(created.pageCount(), 2);
    QCOMPARE(created.activePageKind, QStringLiteral("normal"));
    QCOMPARE(created.pageName, QStringLiteral("画面2"));
    QCOMPARE(created.startupPageId, original.startupPageId);
    QVERIFY(created.activePageId != original.activePageId);
    QVERIFY(window.m_canvas->objects().isEmpty());
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    QCOMPARE(reopened.currentEditorProject().toJson(), created.toJson());
    tree = reopened.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    groups = tree->findItems(QStringLiteral("画面"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(groups.size(), 1);
    QCOMPARE(groups.first()->childCount(), 2);
    QCOMPARE(groups.first()->child(1)->text(0), QStringLiteral("001:画面2"));
}

/******************************************************************************
 * Function Name   : newPageFromToolbarCancelsOrReopens_data
 * Description     : Reuse the toolbar workflow with and without existing page identifiers in the way.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.24
 ******************************************************************************/
void Stage2WorkflowTests::newPageFromToolbarCancelsOrReopens_data() //20261124-10 add
{
    QTest::addColumn<bool>("withWindow");
    QTest::newRow("single-page") << false;
    QTest::newRow("existing-window-occupies-suggestions") << true;
}

/******************************************************************************
 * Function Name   : newPageFromToolbarCancelsOrReopens
 * Description     : Click the actual New Page toolbar button and exercise the page transaction.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.24
 ******************************************************************************/
void Stage2WorkflowTests::newPageFromToolbarCancelsOrReopens() //20261124-9 add
{
    QTemporaryDir directory;
    HmiProject original = workflowProject();
    const QString path = directory.filePath(QStringLiteral("toolbar-page.ksp"));
    MainWindow window;
    MainWindow reopened;
    QAction *action = window.m_commands.value(QStringLiteral("action_new_screen"));
    QToolBar *toolbar = window.findChild<QToolBar *>(QStringLiteral("standardToolBar"));
    QToolButton *button;
    QString error;
    QStringList defaults;
    QPoint clickPoint;
    HmiProject created;
    HmiPage existing;
    int historySize;

    QFETCH(bool, withWindow); //20261124-10 add
    QVERIFY(directory.isValid() && toolbar != nullptr && action != nullptr);
    original.targetModel = QStringLiteral("AMZ070W01");
    if (withWindow)
    {
        existing.id = QStringLiteral("toolbar-existing-window");
        existing.name = QStringLiteral("画面3");
        existing.kind = QStringLiteral("window");
        existing.number = 2;
        existing.width = 240;
        existing.height = 180;
        QVERIFY2(original.addPage(existing, &error), qPrintable(error));
    }
    const QStringList expectedDefaults = {QStringLiteral("新建画面"),
        withWindow ? QStringLiteral("画面4") : QStringLiteral("画面2"),
        withWindow ? QStringLiteral("3") : QStringLiteral("1")};
    button = qobject_cast<QToolButton *>(toolbar->widgetForAction(action));
    QVERIFY(button != nullptr);
    window.closeProject();
    QVERIFY(!button->isEnabled());
    QVERIFY2(original.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    QTRY_VERIFY(button->isVisible() && button->isEnabled());
    clickPoint = button->mapTo(&window, button->rect().center());
    QCOMPARE(window.childAt(clickPoint), button);
    historySize = window.m_projectHistory.size();
    for (bool accept : {false, true})
    {
        defaults.clear();
        QTimer::singleShot(0, &window, [&]() {
            QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (dialog == nullptr)
            {
                return;
            }
            QLineEdit *name = dialog->findChild<QLineEdit *>(QStringLiteral("newPageNameEdit"));
            QSpinBox *number = dialog->findChild<QSpinBox *>(QStringLiteral("newPageNumberSpin"));
            QDialogButtonBox *buttons = dialog->findChild<QDialogButtonBox *>(QString(), Qt::FindDirectChildrenOnly);
            if (name == nullptr || number == nullptr || buttons == nullptr)
            {
                dialog->reject();
                return;
            }
            defaults = {dialog->windowTitle(), name->text(), QString::number(number->value())};
            name->setText(QStringLiteral("参数画面"));
            buttons->button(accept ? QDialogButtonBox::Ok : QDialogButtonBox::Cancel)->click();
        });
        QTest::mouseClick(window.windowHandle(), Qt::LeftButton, Qt::NoModifier, clickPoint);
        QCOMPARE(defaults, expectedDefaults); //20261124-10 change
        if (!accept)
        {
            QCOMPARE(window.currentEditorProject().toJson(), original.toJson());
            QCOMPARE(window.m_projectHistory.size(), historySize);
        }
    }
    created = window.currentEditorProject();
    QCOMPARE(created.pageCount(), original.pageCount() + 1); //20261124-10 change
    QCOMPARE(created.pageName, QStringLiteral("参数画面"));
    QCOMPARE(created.activePageNumber.value_or(-1), expectedDefaults.last().toInt());
    QCOMPARE(created.startupPageId, original.startupPageId);
    QCOMPARE(window.m_projectHistory.size(), historySize + 1);
    QVERIFY(window.m_dirty);

    //20261124-10 add: Exercise both history commands against the complete page records.
    window.m_commands.value(QStringLiteral("action_undo"))->trigger();
    QCOMPARE(window.currentEditorProject().toJson(), original.toJson());
    QVERIFY(!window.m_dirty);
    window.m_commands.value(QStringLiteral("action_redo"))->trigger();
    QCOMPARE(window.currentEditorProject().toJson(), created.toJson());
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    QCOMPARE(reopened.currentEditorProject().toJson(), created.toJson());
    QVERIFY(!reopened.currentEditorProject().validateSinglePageBackend(&error)); //20261124-10 add
    QVERIFY(error.contains(QStringLiteral("多页")));
    window.closeProject();
    QVERIFY(!button->isEnabled());
}

/******************************************************************************
 * Function Name   : newPageBackgroundRendersAndSurvivesReopen
 * Description     : Choose a solid background, save the project and observe the same color on the reopened canvas.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::newPageBackgroundRendersAndSurvivesReopen() //20261123-0 add
{
    QTemporaryDir directory;
    HmiProject original = workflowProject();
    MainWindow window;
    MainWindow reopened;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> groups;
    QColor selected(QStringLiteral("#32658a"));
    QColor initial;
    QString error;
    QString path;

    QVERIFY(directory.isValid());
    original.targetModel = QStringLiteral("AMZ070W01");
    path = directory.filePath(QStringLiteral("page-color.ksp"));
    QVERIFY2(original.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    groups = tree->findItems(QStringLiteral("画面"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(groups.size(), 1);
    QVERIFY(triggerSettingsTreeAction(tree, groups.first(), QStringLiteral("新建画面"), [&]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QPushButton *color;
        QDialogButtonBox *buttons;

        if (dialog == nullptr)
        {
            return;
        }
        color = dialog->findChild<QPushButton *>(QStringLiteral("newPageBackgroundColor"));
        buttons = dialog->findChild<QDialogButtonBox *>();
        if (color == nullptr || buttons == nullptr)
        {
            dialog->reject();
            return;
        }
        QTimer::singleShot(0, dialog, [&]() {
            QColorDialog *picker = qobject_cast<QColorDialog *>(QApplication::activeModalWidget());
            if (picker == nullptr)
            {
                return;
            }
            initial = picker->currentColor();
            picker->setCurrentColor(selected);
            picker->accept();
        });
        color->click();
        buttons->button(QDialogButtonBox::Ok)->click();
    }));
    QCOMPARE(initial, QColor(QStringLiteral("#c5deff")));
    auto sampledCanvasColor = [](QGraphicsView *view) {
        QImage pixels(800, 480, QImage::Format_RGB32);
        QPainter painter(&pixels);
        view->scene()->render(&painter, QRectF(0, 0, 800, 480), QRectF(0, 0, 800, 480));
        painter.end();
        return pixels.pixelColor(400, 240);
    };
    QCOMPARE(sampledCanvasColor(window.m_canvas), selected);
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    QCOMPARE(sampledCanvasColor(reopened.m_canvas), selected);
}

/******************************************************************************
 * Function Name   : newPageGradientKeepsNativeSymmetryAndPersists
 * Description     : Use the property window to create a horizontal symmetric gradient and sample the scene after reopening.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::newPageGradientKeepsNativeSymmetryAndPersists() //20261123-1 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    MainWindow window;
    MainWindow reopened;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> groups;
    QColor foreground(1, 2, 3);
    QString path;
    QString error;
    bool choseStyle = false;

    QVERIFY(directory.isValid());
    project.targetModel = QStringLiteral("AMZ070W01");
    path = directory.filePath(QStringLiteral("page-gradient.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    groups = tree->findItems(QStringLiteral("画面"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(groups.size(), 1);
    QVERIFY(triggerSettingsTreeAction(tree, groups.first(), QStringLiteral("新建画面"), [&]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QPushButton *color;
        QComboBox *style;
        QDialogButtonBox *buttons;

        if (dialog == nullptr)
        {
            return;
        }
        color = dialog->findChild<QPushButton *>(QStringLiteral("newPageForegroundColor"));
        style = dialog->findChild<QComboBox *>(QStringLiteral("newPageBackgroundStyle"));
        buttons = dialog->findChild<QDialogButtonBox *>();
        if (color == nullptr || style == nullptr || buttons == nullptr ||
            style->count() != 12 || style->itemText(2) != QStringLiteral("横向对称过渡"))
        {
            dialog->reject();
            return;
        }
        QTimer::singleShot(0, dialog, [&]() {
            QColorDialog *picker = qobject_cast<QColorDialog *>(QApplication::activeModalWidget());
            if (picker != nullptr)
            {
                picker->setCurrentColor(foreground);
                picker->accept();
            }
        });
        color->click();
        style->setCurrentIndex(2);
        choseStyle = true;
        buttons->button(QDialogButtonBox::Ok)->click();
    }));
    QVERIFY(choseStyle);
    auto scan = [](QGraphicsView *view) {
        QImage pixels(800, 480, QImage::Format_RGB32);
        QPainter painter(&pixels);
        view->scene()->render(&painter, QRectF(0, 0, 800, 480), QRectF(0, 0, 800, 480));
        painter.end();
        return QList<QColor>{pixels.pixelColor(4, 240), pixels.pixelColor(399, 240), pixels.pixelColor(795, 240)};
    };
    const QList<QColor> values = scan(window.m_canvas);
    // The sampled pixels lie 4.5 pixels inside each edge and already contain a small part of the background color.
    const int edgeBlue = qRound(foreground.blue() + (255 - foreground.blue()) * 4.5 / 400.0);
    QVERIFY2(qAbs(values.first().blue() - edgeBlue) <= 2, qPrintable(values.first().name()));
    QVERIFY(qAbs(values.at(1).blue() - 255) <= 1);
    QVERIFY2(qAbs(values.last().blue() - edgeBlue) <= 2, qPrintable(values.last().name()));
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    QCOMPARE(scan(reopened.m_canvas), values);
}

/******************************************************************************
 * Function Name   : newPageProjectBitmapStretchesAndPersists
 * Description     : Select an asymmetric PNG through the new-page gallery and check both axes, persistence and resource protection.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::newPageProjectBitmapStretchesAndPersists() //20261123-2 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    HmiProject renamed;
    HmiProject unused;
    HmiProject single;
    MainWindow window;
    MainWindow reopened;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> groups;
    QImage bitmap(16, 4, QImage::Format_RGB32);
    QImage preview;
    QByteArray png;
    QBuffer buffer(&png);
    QString name = QStringLiteral("四色背景");
    QString newName = QStringLiteral("已改名背景");
    QString error;
    QString path;
    QString pageId;
    QColor quadrant;
    bool picked = false;
    int x;
    int y;
    QPoint point;

    QVERIFY(directory.isValid());
    for (y = 0; y < bitmap.height(); ++y)
    {
        for (x = 0; x < bitmap.width(); ++x)
        {
            quadrant = y < 2 ? (x < 8 ? QColor(Qt::red) : QColor(Qt::green)) :
                               (x < 8 ? QColor(Qt::blue) : QColor(Qt::yellow));
            bitmap.setPixelColor(x, y, quadrant);
        }
    }
    QVERIFY(buffer.open(QIODevice::WriteOnly));
    QVERIFY(bitmap.save(&buffer, "PNG"));
    project.pictures.insert(name, png);
    path = directory.filePath(QStringLiteral("page-bitmap.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    groups = tree->findItems(QStringLiteral("画面"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(groups.size(), 1);
    QVERIFY(triggerSettingsTreeAction(tree, groups.first(), QStringLiteral("新建画面"), [&]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QRadioButton *bitmapMode;
        QPushButton *gallery;
        QLabel *sample;
        QDialogButtonBox *buttons;

        if (dialog == nullptr)
        {
            return;
        }
        bitmapMode = dialog->findChild<QRadioButton *>(QStringLiteral("newPageBitmapMode"));
        gallery = dialog->findChild<QPushButton *>(QStringLiteral("newPageProjectGallery"));
        sample = dialog->findChild<QLabel *>(QStringLiteral("newPageBackgroundPreview"));
        buttons = dialog->findChild<QDialogButtonBox *>();
        if (bitmapMode == nullptr || gallery == nullptr || sample == nullptr || buttons == nullptr)
        {
            dialog->reject();
            return;
        }
        bitmapMode->click();
        QTimer::singleShot(0, dialog, [&]() {
            QDialog *picker = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QListWidget *items;
            QList<QListWidgetItem *> matches;
            QDialogButtonBox *pickerButtons;

            if (picker == nullptr || picker == dialog)
            {
                return;
            }
            items = picker->findChild<QListWidget *>(QStringLiteral("GalleryItems")); //20261124-3 change
            pickerButtons = picker->findChild<QDialogButtonBox *>();
            if (items == nullptr || pickerButtons == nullptr ||
                !QTest::qWaitFor([items]() { return items->count() != 0; }, 1000))
            {
                picker->reject();
                return;
            }
            matches = items->findItems(name, Qt::MatchExactly);
            if (matches.size() != 1)
            {
                picker->reject();
                return;
            }
            items->setCurrentItem(matches.first());
            picked = true;
            pickerButtons->button(QDialogButtonBox::Ok)->click();
        });
        gallery->click();
        if (!picked)
        {
            dialog->reject();
            return;
        }
        preview = sample->pixmap().toImage();
        buttons->button(QDialogButtonBox::Ok)->click();
        if (dialog->isVisible())
        {
            dialog->reject();
        }
    }));
    QVERIFY2(picked, "A new bitmap page must let the user choose an existing PNG in the project gallery.");
    QVERIFY(!preview.isNull());
    QCOMPARE(preview.width(), preview.height());
    const QImage expectedPreview = bitmap.scaled(preview.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    Q_FOREACH (point, QList<QPoint>({QPoint(0, 0), QPoint(preview.width() - 1, 0), QPoint(0, preview.height() - 1),
                                   QPoint(preview.width() - 1, preview.height() - 1), preview.rect().center()}))
    {
        QCOMPARE(preview.pixelColor(point), expectedPreview.pixelColor(point));
    }
    auto renderPage = [](QGraphicsView *view) {
        QImage pixels(800, 480, QImage::Format_RGB32);
        QPainter painter(&pixels);
        view->scene()->render(&painter, QRectF(0, 0, 800, 480), QRectF(0, 0, 800, 480));
        painter.end();
        return pixels;
    };
    const QImage expectedPage = bitmap.scaled(800, 480, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    const QList<QPoint> samples = {QPoint(0, 0), QPoint(799, 0), QPoint(0, 479), QPoint(799, 479),
                                   QPoint(399, 239), QPoint(401, 241)};
    pageId = window.currentEditorProject().activePageId;
    QVERIFY(pageId != project.activePageId);
    const QImage drawn = renderPage(window.m_canvas);
    Q_FOREACH (point, samples)
    {
        QCOMPARE(drawn.pixelColor(point), expectedPage.pixelColor(point));
    }
    QVERIFY(window.activatePageId(project.activePageId));
    QVERIFY(!deleteStage3Picture(window.currentEditorProject(), name, &unused, &error));
    QVERIFY2(renameStage3Picture(window.currentEditorProject(), name, newName, &renamed, &error), qPrintable(error));
    QVERIFY2(window.commitProjectDraft(renamed, &error), qPrintable(error));
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    QVERIFY(reopened.activatePageId(pageId));
    QVERIFY(!deleteStage3Picture(reopened.currentEditorProject(), newName, &unused, &error));
    const QImage reopenedPage = renderPage(reopened.m_canvas);
    Q_FOREACH (point, samples)
    {
        QCOMPARE(reopenedPage.pixelColor(point), expectedPage.pixelColor(point));
    }
    single = reopened.currentEditorProject();
    single.startupPageId = pageId;
    single.activePageNumber = 0;
    QVERIFY2(single.removePage(project.activePageId, &error), qPrintable(error));
    QVERIFY(!single.validateSinglePageBackend(&error));
    QVERIFY(error.contains(QStringLiteral("背景")));
}

/******************************************************************************
 * Function Name   : newPageSystemBitmapStretchesAndPersists
 * Description     : Select a catalog vector in the ordinary-page editor and verify preview and canvas after reopening.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::newPageSystemBitmapStretchesAndPersists() //20261123-3 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    const QString originalPageId = project.activePageId;
    MainWindow window;
    MainWindow reopened;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> groups;
    QString path;
    QString error;
    QString pageId;
    QImage preview;
    QImage expected;
    QImage drawn(800, 480, QImage::Format_ARGB32_Premultiplied);
    QPainter painter;
    SimulationRuntime runtime;
    QSvgRenderer source(QStringLiteral(":/system-gallery/default/switch_OFF.kvg"));
    bool picked = false;

    QVERIFY(directory.isValid());
    QVERIFY(source.isValid());
    path = directory.filePath(QStringLiteral("system-bitmap.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    groups = tree->findItems(QStringLiteral("画面"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(groups.size(), 1);
    QVERIFY(triggerSettingsTreeAction(tree, groups.first(), QStringLiteral("新建画面"), [&]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QRadioButton *bitmapMode;
        QPushButton *gallery;
        QLabel *sample;
        QDialogButtonBox *buttons;

        if (dialog == nullptr)
        {
            return;
        }
        bitmapMode = dialog->findChild<QRadioButton *>(QStringLiteral("newPageBitmapMode"));
        gallery = dialog->findChild<QPushButton *>(QStringLiteral("newPageSystemGallery"));
        sample = dialog->findChild<QLabel *>(QStringLiteral("newPageBackgroundPreview"));
        buttons = dialog->findChild<QDialogButtonBox *>();
        if (bitmapMode == nullptr || gallery == nullptr || sample == nullptr || buttons == nullptr)
        {
            dialog->reject();
            return;
        }
        bitmapMode->click();
        QTimer::singleShot(0, dialog, [&]() {
            QDialog *picker = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QLineEdit *search;
            QListWidget *items;
            QDialogButtonBox *pickerButtons;
            QList<QListWidgetItem *> matches;

            if (picker == nullptr || picker == dialog)
            {
                return;
            }
            search = picker->findChild<QLineEdit *>(QStringLiteral("GallerySearch"));
            items = picker->findChild<QListWidget *>(QStringLiteral("GalleryItems"));
            pickerButtons = picker->findChild<QDialogButtonBox *>();
            if (search == nullptr || items == nullptr || pickerButtons == nullptr)
            {
                picker->reject();
                return;
            }
            search->setText(QStringLiteral("switch_OFF.kvg"));
            if (!QTest::qWaitFor([&]() {
                matches = items->findItems(QStringLiteral("switch_OFF.kvg"), Qt::MatchExactly);
                return matches.size() == 1;
            }, 3000))
            {
                picker->reject();
                return;
            }
            items->setCurrentItem(matches.first());
            picked = true;
            pickerButtons->button(QDialogButtonBox::Ok)->click();
        });
        gallery->click();
        preview = sample->pixmap().toImage();
        buttons->button(QDialogButtonBox::Ok)->click();
        if (dialog->isVisible())
        {
            dialog->reject();
        }
    }));
    QVERIFY2(picked, "A bitmap background must let the user select a vector from the system gallery.");
    QVERIFY(!preview.isNull());
    expected = QImage(preview.size(), QImage::Format_ARGB32_Premultiplied);
    expected.fill(Qt::transparent);
    QVERIFY(painter.begin(&expected));
    source.setAspectRatioMode(Qt::IgnoreAspectRatio);
    source.render(&painter, expected.rect());
    painter.end();
    QCOMPARE(preview.convertToFormat(expected.format()), expected);
    pageId = window.currentEditorProject().activePageId;
    QVERIFY(pageId != project.activePageId);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    QVERIFY(reopened.activatePageId(pageId));
    QCOMPARE(reopened.currentEditorProject().pictures, project.pictures);
    drawn.fill(Qt::transparent);
    QVERIFY(painter.begin(&drawn));
    reopened.m_canvas->scene()->render(&painter, drawn.rect(), drawn.rect());
    painter.end();
    expected = QImage(drawn.size(), drawn.format());
    expected.fill(Qt::transparent);
    QVERIFY(painter.begin(&expected));
    source.render(&painter, expected.rect());
    painter.end();
    QCOMPARE(drawn.pixelColor(400, 40), expected.pixelColor(400, 40));
    QCOMPARE(drawn.pixelColor(400, 240), expected.pixelColor(400, 240));
    project = reopened.currentEditorProject();
    project.startupPageId = pageId;
    project.activePageNumber = 0;
    QVERIFY2(project.removePage(originalPageId, &error), qPrintable(error));
    QVERIFY2(runtime.start(project, &error), qPrintable(error));
    QCOMPARE(simulationFrame(runtime), expected);
    QVERIFY(!project.validateSinglePageBackend(&error));
    QVERIFY(error.contains(QStringLiteral("背景")));
}

/******************************************************************************
 * Function Name   : newPageSlideKeepsStableTargetAcrossSave
 * Description     : Enable left swipe, save the page and verify the target survives renaming.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::newPageSlideKeepsStableTargetAcrossSave() //20261123-4 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    HmiProject reopened;
    HmiPage target;
    HmiPage window;
    QString error;
    QString path;
    QString pageId;
    bool configured = false;

    QVERIFY(directory.isValid());
    target.id = QStringLiteral("swipe-target");
    target.name = QStringLiteral("目标画面");
    target.number = 5;
    QVERIFY2(project.addPage(target, &error), qPrintable(error));
    window.id = QStringLiteral("popup-target");
    window.kind = QStringLiteral("window");
    window.name = QStringLiteral("弹出窗口");
    QVERIFY2(project.addPage(window, &error), qPrintable(error));

    QTimer::singleShot(0, this, [&]() {
        QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QGroupBox *group;
        QSpinBox *distance;
        QCheckBox *left;
        QComboBox *leftTarget;
        QDialogButtonBox *buttons;
        int targetIndex;

        if (editor == nullptr)
        {
            return;
        }
        group = editor->findChild<QGroupBox *>(QStringLiteral("pageSlideGroup"));
        distance = editor->findChild<QSpinBox *>(QStringLiteral("pageSlideDistance"));
        left = editor->findChild<QCheckBox *>(QStringLiteral("pageSlideLeft"));
        leftTarget = editor->findChild<QComboBox *>(QStringLiteral("pageSlideLeftTarget"));
        buttons = editor->findChild<QDialogButtonBox *>();
        if (group == nullptr || distance == nullptr || left == nullptr || leftTarget == nullptr || buttons == nullptr ||
            group->isChecked() || distance->isEnabled() || left->isEnabled() || leftTarget->isEnabled() ||
            distance->minimum() != 5 || distance->maximum() != 480 || distance->value() != 50)
        {
            editor->reject();
            return;
        }
        group->setChecked(true);
        targetIndex = leftTarget->findText(QStringLiteral("005:目标画面"));
        if (!leftTarget->isEnabled() || left->isChecked() || targetIndex < 0 || leftTarget->count() != 2)
        {
            editor->reject();
            return;
        }
        distance->setValue(75);
        left->click();
        leftTarget->setCurrentIndex(targetIndex);
        configured = true;
        buttons->button(QDialogButtonBox::Ok)->click();
        if (editor->isVisible())
        {
            editor->reject();
        }
    });
    const bool accepted = editStage2NewNormalPage(nullptr, project);
    QVERIFY2(configured && accepted, "The new-page dialog must accept 75 px left swipe to ordinary page 005.");
    pageId = project.activePageId;
    QVERIFY2(project.renamePage(target.id, QStringLiteral("已改名目标"), &error), qPrintable(error));
    path = directory.filePath(QStringLiteral("page-slide.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    QCOMPARE(reopened.configurationNodes.size(), 1);
    QCOMPARE(reopened.configurationNodes.first().ownerPageId, pageId);
    QCOMPARE(reopened.configurationNodes.first().properties.value(QStringLiteral("enabled")).toBool(), true);
    QCOMPARE(reopened.configurationNodes.first().properties.value(QStringLiteral("distance")).toInt(), 75);
    QCOMPARE(reopened.references.size(), 1);
    QCOMPARE(reopened.references.first().field, QStringLiteral("left"));
    QCOMPARE(reopened.references.first().targetId, target.id);
    QVERIFY(!reopened.removePage(target.id, &error));
    QVERIFY(error.contains(QStringLiteral("被引用")));
    QVERIFY(!reopened.validateSinglePageBackend(&error));
}

/******************************************************************************
 * Function Name   : newPageMacroDefaultsPersistWithStableSource
 * Description     : Add a default screen script and verify its source stays bound across a rename and reload.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::newPageMacroDefaultsPersistWithStableSource() //20261123-5 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    HmiProject reopened;
    const QString initialName = QStringLiteral("up_test");
    const QString renamed = QStringLiteral("page_tick");
    QString error;
    QString path;
    QString macroId;
    bool added = false;

    QVERIFY(directory.isValid());
    project.macros.insert(initialName, QStringLiteral("void main() {}"));
    QTimer::singleShot(0, this, [&]() {
        QDialog *page = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QPushButton *add;
        QDialogButtonBox *pageButtons;

        if (page == nullptr)
        {
            return;
        }
        add = page->findChild<QPushButton *>(QStringLiteral("pageMacroAdd"));
        pageButtons = page->findChild<QDialogButtonBox *>();
        if (add == nullptr || pageButtons == nullptr)
        {
            page->reject();
            return;
        }
        QTimer::singleShot(0, page, [&]() {
            QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QComboBox *macros;
            QSpinBox *frequency;
            QSpinBox *count;
            QDialogButtonBox *buttons;

            if (editor == nullptr || editor == page)
            {
                return;
            }
            macros = editor->findChild<QComboBox *>(QStringLiteral("pageMacroSource"));
            frequency = editor->findChild<QSpinBox *>(QStringLiteral("pageMacroFrequency"));
            count = editor->findChild<QSpinBox *>(QStringLiteral("pageMacroCount"));
            buttons = editor->findChild<QDialogButtonBox *>();
            if (editor->windowTitle() != QStringLiteral("新建脚本") || macros == nullptr ||
                macros->currentText() != initialName || frequency == nullptr || count == nullptr || buttons == nullptr ||
                frequency->value() != 10 || frequency->minimum() != 1 || frequency->maximum() != 9999 ||
                count->value() != 0 || count->minimum() != 0 || count->maximum() != 65535)
            {
                editor->reject();
                return;
            }
            added = true;
            buttons->button(QDialogButtonBox::Ok)->click();
            if (editor->isVisible())
            {
                editor->reject();
            }
        });
        add->click();
        if (added)
        {
            pageButtons->button(QDialogButtonBox::Ok)->click();
        }
        if (page->isVisible())
        {
            page->reject();
        }
    });
    const bool accepted = editStage2NewNormalPage(nullptr, project);
    QVERIFY2(added && accepted, "New ordinary pages require a macro table that accepts existing macros.");
    macroId = project.macroIds.value(initialName);
    QVERIFY(!macroId.isEmpty());
    QVERIFY2(project.renameMacro(initialName, renamed, &error), qPrintable(error));
    path = directory.filePath(QStringLiteral("page-macro.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    QCOMPARE(reopened.macroIds.value(renamed), macroId);
    QCOMPARE(reopened.configurationNodes.size(), 1);
    QCOMPARE(reopened.configurationNodes.first().ownerPageId, project.activePageId);
    const QJsonArray rows = reopened.configurationNodes.first().properties.value(QStringLiteral("scripts")).toArray();
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows.first().toObject().value(QStringLiteral("frequency")).toInt(), 10);
    QCOMPARE(rows.first().toObject().value(QStringLiteral("count")).toInt(), 0);
    QCOMPARE(rows.first().toObject().value(QStringLiteral("address")).toString(), QString());
    QCOMPARE(reopened.references.size(), 1);
    QCOMPARE(reopened.references.first().targetId, macroId);
    QVERIFY(!reopened.validateSinglePageBackend(&error));
}

/******************************************************************************
 * Function Name   : ordinaryPagePropertiesRestoreAndUpdateDraft
 * Description     : Open an existing page from the tree and verify cancel, target filtering and saved edits.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::ordinaryPagePropertiesRestoreAndUpdateDraft() //20261123-6 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    HmiProject reopened;
    HmiPage target;
    MainWindow window;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> treePages;
    HmiConfigurationNode slide;
    HmiConfigurationNode macro; //20261123-6 add
    const QString pageId = project.activePageId;
    QString error;
    QString path;
    bool restored = false;
    int attempt; //20261123-6 change

    QVERIFY(directory.isValid());
    target.id = QStringLiteral("target-005");
    target.name = QStringLiteral("滑动目标");
    target.number = 5;
    QVERIFY2(project.addPage(target, &error), qPrintable(error));
    project.activePageBackground.systemPicture = QStringLiteral("switch_OFF.kvg");
    slide.id = QStringLiteral("existing-slide");
    slide.name = QStringLiteral("滑动打开画面");
    slide.ownerPageId = pageId;
    slide.kind = QStringLiteral("pageSlide");
    slide.active = true;
    slide.implemented = true;
    slide.properties = {{QStringLiteral("enabled"), true}, {QStringLiteral("distance"), 75}};
    project.configurationNodes.append(slide);
    project.references.append({QStringLiteral("existing-left"), QStringLiteral("configuration"),
        slide.id, QStringLiteral("left"), QStringLiteral("page"), target.id});
    project.macros.insert(QStringLiteral("page_tick"), QStringLiteral("void main() {}")); //20261123-6 add
    project.macroIds.insert(QStringLiteral("page_tick"), QStringLiteral("source-id"));
    macro.id = QStringLiteral("existing-scripts");
    macro.name = QStringLiteral("画面宏");
    macro.ownerPageId = pageId;
    macro.kind = QStringLiteral("pageMacro");
    macro.active = true;
    macro.implemented = true;
    macro.properties = {{QStringLiteral("scripts"), QJsonArray{QJsonObject{
        {QStringLiteral("id"), QStringLiteral("existing-row")},
        {QStringLiteral("frequency"), 12}, {QStringLiteral("count"), 3},
        {QStringLiteral("address"), QStringLiteral("LB7")}}}}};
    project.configurationNodes.append(macro);
    project.references.append({QStringLiteral("existing-macro-binding"), QStringLiteral("configuration"),
        macro.id, QStringLiteral("existing-row"), QStringLiteral("macro"), QStringLiteral("source-id")});
    project.configurationNodes.swapItemsAt(0, 1); //20261123-6 add: Keep the saved order even when the editor rebuilds both records.
    project.references.swapItemsAt(0, 1);
    path = directory.filePath(QStringLiteral("page-properties.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    treePages = tree->findItems(QStringLiteral("000:%1").arg(project.pageName), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(treePages.size(), 1);

    for (attempt = 0; attempt < 3; ++attempt) //20261123-6 change
    {
        restored = false;
        QVERIFY(triggerSettingsTreeAction(tree, treePages.first(), QStringLiteral("属性(&P)"), [&]() { //20261123-7 change
            QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QLineEdit *name;
            QSpinBox *distance;
            QGroupBox *group;
            QRadioButton *bitmapMode;
            QCheckBox *left;
            QComboBox *targetList;
            QTableWidget *scriptTable; //20261123-6 add
            QDialogButtonBox *buttons;

            if (editor == nullptr)
            {
                return;
            }
            name = editor->findChild<QLineEdit *>(QStringLiteral("newPageNameEdit"));
            group = editor->findChild<QGroupBox *>(QStringLiteral("pageSlideGroup"));
            bitmapMode = editor->findChild<QRadioButton *>(QStringLiteral("newPageBitmapMode"));
            distance = editor->findChild<QSpinBox *>(QStringLiteral("pageSlideDistance"));
            left = editor->findChild<QCheckBox *>(QStringLiteral("pageSlideLeft"));
            targetList = editor->findChild<QComboBox *>(QStringLiteral("pageSlideLeftTarget"));
            scriptTable = editor->findChild<QTableWidget *>(); //20261123-6 add
            buttons = editor->findChild<QDialogButtonBox *>();
            if (editor->windowTitle() != QStringLiteral("画面属性") || name == nullptr || distance == nullptr ||
                group == nullptr || left == nullptr || targetList == nullptr || buttons == nullptr ||
                bitmapMode == nullptr || !bitmapMode->isChecked() || !group->isChecked() || !left->isChecked() ||
                targetList->count() != 1 || targetList->currentData().toString() != target.id || distance->value() != 75 ||
                scriptTable == nullptr || scriptTable->rowCount() != 1 || scriptTable->item(0, 4) == nullptr ||
                scriptTable->item(0, 4)->text() != QStringLiteral("LB7")) //20261123-6 change
            {
                editor->reject();
                return;
            }
            restored = true;
            if (attempt != 1) //20261123-6 add: First cancel edits, then accept an unchanged dialog, then commit actual edits.
            {
                name->setText(QStringLiteral("属性修改后的画面"));
                distance->setValue(90);
            }
            buttons->button(attempt == 0 ? QDialogButtonBox::Cancel : QDialogButtonBox::Ok)->click();
            if (editor->isVisible())
            {
                editor->reject();
            }
        }));
        QVERIFY2(restored, "The page properties menu must restore the existing bitmap and left swipe without including the current page.");
        if (attempt < 2) //20261123-6 change
        {
            QVERIFY(!window.m_dirty);
            QCOMPARE(window.currentEditorProject().pageName, project.pageName);
        }
    }
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    QCOMPARE(reopened.pageName, QStringLiteral("属性修改后的画面"));
    QCOMPARE(reopened.startupPageId, pageId);
    QCOMPARE(reopened.activePageBackground.systemPicture, project.activePageBackground.systemPicture);
    QCOMPARE(reopened.configurationNodes.last().id, slide.id); //20261123-6 change
    QCOMPARE(reopened.configurationNodes.last().properties.value(QStringLiteral("distance")).toInt(), 90);
    QCOMPARE(reopened.references.last().targetId, target.id);
    QCOMPARE(reopened.configurationNodes.first().toJson(), macro.toJson());
    QCOMPARE(reopened.references.first().id, QStringLiteral("existing-macro-binding"));
}

/******************************************************************************
 * Function Name   : ordinaryPageMenuOpensAndReorders
 * Description     : Open a page and move it among ordinary siblings without changing startup or other page slots.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::ordinaryPageMenuOpensAndReorders() //20261123-7 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    HmiProject reopened;
    const QString startupId = project.startupPageId;
    const QString firstLabel = QStringLiteral("000:%1").arg(project.pageName);
    HmiPage second;
    HmiPage popup;
    HmiPage third;
    MainWindow window;
    QTreeWidget *tree;
    QString path;
    QString error;

    QVERIFY(directory.isValid());
    second.id = QStringLiteral("ordinary-nine");
    second.name = QStringLiteral("第二个普通画面");
    second.number = 9;
    popup.id = QStringLiteral("interleaved-window");
    popup.name = QStringLiteral("弹出窗口");
    popup.kind = QStringLiteral("window");
    popup.number = 11; //20261123-13 add: Keep the interleaved window independent from ordinary screen 002.
    third.id = QStringLiteral("ordinary-two");
    third.name = QStringLiteral("第三个普通画面");
    third.number = 2;
    QVERIFY2(project.addPage(second, &error), qPrintable(error));
    QVERIFY2(project.addPage(popup, &error), qPrintable(error));
    QVERIFY2(project.addPage(third, &error), qPrintable(error));
    project.references.append({QStringLiteral("stable-page-target"), QStringLiteral("page"), startupId,
        QStringLiteral("test-target"), QStringLiteral("page"), third.id}); //20261123-7 add
    path = directory.filePath(QStringLiteral("ordinary-navigation.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    auto navigate = [tree](const QString &label, const QString &action) {
        const QList<QTreeWidgetItem *> entries = tree->findItems(label, Qt::MatchExactly | Qt::MatchRecursive);

        return entries.size() == 1 && triggerSettingsTreeAction(tree, entries.first(), action);
    };
    QVERIFY(navigate(QStringLiteral("002:第三个普通画面"), QStringLiteral("打开(&O)")));
    QCOMPARE(window.currentEditorProject().activePageId, third.id);
    QVERIFY(!window.m_dirty);
    QVERIFY(navigate(QStringLiteral("002:第三个普通画面"), QStringLiteral("下一个画面")));
    QVERIFY(navigate(firstLabel, QStringLiteral("上一个画面")));
    QCOMPARE(window.currentEditorProject().pageOrder, project.pageOrder);
    QVERIFY(!window.m_dirty);
    QVERIFY(navigate(QStringLiteral("002:第三个普通画面"), QStringLiteral("上一个画面")));
    QCOMPARE(window.currentEditorProject().pageOrder, (QStringList{startupId, third.id, popup.id, second.id}));
    QCOMPARE(window.currentEditorProject().activePageId, third.id);
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger(); //20261123-7 add
    QVERIFY(!window.m_dirty);
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    QCOMPARE(reopened.pageOrder, (QStringList{startupId, third.id, popup.id, second.id}));
    QCOMPARE(reopened.startupPageId, startupId);
    QCOMPARE(reopened.references.first().targetId, third.id);
    QVERIFY(navigate(QStringLiteral("002:第三个普通画面"), QStringLiteral("下一个画面")));
    QCOMPARE(window.currentEditorProject().pageOrder, project.pageOrder);
    QCOMPARE(window.currentEditorProject().activePageId, third.id);
    QCOMPARE(window.currentEditorProject().startupPageId, startupId);
    QVERIFY(window.m_dirty); //20261123-7 change: The saved checkpoint is now the moved order.
    window.m_commands.value(QStringLiteral("action_undo"))->trigger();
    QCOMPARE(window.currentEditorProject().pageOrder, reopened.pageOrder);
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : ordinaryPageMenuCopiesOwnedContent
 * Description     : Copy an ordinary page from the tree and check its new identities and preserved external target.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::ordinaryPageMenuCopiesOwnedContent() //20261123-8 add
{
    QTemporaryDir directory;
    HmiProject original = workflowProject();
    HmiProject reopened;
    HmiPage target;
    HmiConfigurationNode slide;
    const QString baseId = original.activePageId;
    const QString sourceLabel = QStringLiteral("000:%1").arg(original.pageName);
    MainWindow window;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> pages;
    QString path;
    QString error;
    QString copyId;
    bool dialogSeen;
    bool confirm;

    QVERIFY(directory.isValid());
    target.id = QStringLiteral("copy-slide-target");
    target.name = QStringLiteral("滑动目的页");
    QVERIFY2(original.addPage(target, &error), qPrintable(error));
    slide.id = QStringLiteral("source-slide");
    slide.kind = QStringLiteral("pageSlide");
    slide.name = QStringLiteral("滑动打开画面");
    slide.ownerPageId = baseId;
    slide.active = true;
    slide.implemented = true;
    slide.properties = {{QStringLiteral("enabled"), true}, {QStringLiteral("distance"), 75}};
    original.configurationNodes.append(slide);
    original.references.append({QStringLiteral("source-slide-left"), QStringLiteral("configuration"), slide.id,
        QStringLiteral("left"), QStringLiteral("page"), target.id});
    path = directory.filePath(QStringLiteral("copy-page.ksp"));
    QVERIFY2(original.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    for (confirm = false; ; confirm = true)
    {
        dialogSeen = false;
        pages = tree->findItems(sourceLabel, Qt::MatchExactly | Qt::MatchRecursive);
        QCOMPARE(pages.size(), 1);
        QVERIFY(triggerSettingsTreeAction(tree, pages.first(), QStringLiteral("复制(&C)"), [&]() {
            QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QLineEdit *name;
            QPushButton *button;

            if (editor == nullptr)
            {
                return;
            }
            name = editor->findChild<QLineEdit *>(QStringLiteral("lineEdit"));
            button = editor->findChild<QPushButton *>(confirm ? QStringLiteral("pushButton_ok") :
                QStringLiteral("pushButton_cancle"));
            if (editor->windowTitle() != QStringLiteral("复制到当前画面") ||
                editor->size() != QSize(273, 72) || name == nullptr || !name->text().isEmpty() || button == nullptr)
            {
                editor->reject();
                return;
            }
            dialogSeen = true;
            name->setText(QStringLiteral("画面副本"));
            button->click();
            if (editor->isVisible())
            {
                editor->reject();
            }
        }));
        QVERIFY(dialogSeen);
        if (confirm)
        {
            break;
        }
        QCOMPARE(window.currentEditorProject().pageCount(), original.pageCount());
        QVERIFY(!window.m_dirty);
    }
    copyId = window.currentEditorProject().activePageId;
    QVERIFY(copyId != baseId && copyId != target.id);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    QCOMPARE(reopened.startupPageId, baseId);
    QCOMPARE(reopened.activePageId, copyId);
    QCOMPARE(reopened.pageName, QStringLiteral("画面副本"));
    QCOMPARE(reopened.pageCount(), 3);
    QCOMPARE(reopened.objects.size(), original.objects.size());
    QCOMPARE(reopened.objects.first().text, original.objects.first().text);
    QVERIFY(reopened.objects.first().id != original.objects.first().id);
    QCOMPARE(reopened.configurationNodes.size(), 2);
    QCOMPARE(reopened.configurationNodes.last().ownerPageId, copyId);
    QVERIFY(reopened.configurationNodes.last().id != slide.id);
    QCOMPARE(reopened.references.size(), 2);
    QCOMPARE(reopened.references.last().ownerId, reopened.configurationNodes.last().id);
    QCOMPARE(reopened.references.last().targetId, target.id);
}

/******************************************************************************
 * Function Name   : ordinaryPageMenuDeletionProtectsTargets
 * Description     : Confirm native menu order, protected-page warnings and removal of only the selected page.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::ordinaryPageMenuDeletionProtectsTargets() //20261123-9 add
{
    QTemporaryDir directory;
    HmiProject original = workflowProject();
    HmiProject reopened;
    HmiPage target;
    HmiPage disposable;
    HmiConfigurationNode slide;
    const QString startupId = original.startupPageId;
    const QString startupLabel = QStringLiteral("000:%1").arg(original.pageName);
    MainWindow window;
    QTreeWidget *tree;
    QString path;
    QString error;
    QString warning;
    QStringList observedActions;
    const QStringList expectedActions = {QStringLiteral("打开(&O)"), QStringLiteral("删除(&D)"),
        QStringLiteral("属性(&P)"), QStringLiteral("复制(&C)"), QStringLiteral("上一个画面"), QStringLiteral("下一个画面")};

    QVERIFY(directory.isValid());
    target.id = QStringLiteral("protected-target");
    target.name = QStringLiteral("受引用画面");
    target.number = 3;
    disposable.id = QStringLiteral("unused-page");
    disposable.name = QStringLiteral("可删除画面");
    disposable.number = 7;
    QVERIFY2(original.addPage(target, &error), qPrintable(error));
    QVERIFY2(original.addPage(disposable, &error), qPrintable(error));
    slide.id = QStringLiteral("startup-slide");
    slide.kind = QStringLiteral("pageSlide");
    slide.name = QStringLiteral("滑动打开画面");
    slide.ownerPageId = startupId;
    slide.active = true;
    slide.implemented = true;
    slide.properties = {{QStringLiteral("enabled"), true}, {QStringLiteral("distance"), 75}};
    original.configurationNodes.append(slide);
    original.references.append({QStringLiteral("startup-left"), QStringLiteral("configuration"), slide.id,
        QStringLiteral("left"), QStringLiteral("page"), target.id});
    path = directory.filePath(QStringLiteral("delete-page.ksp"));
    QVERIFY2(original.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    auto found = [tree](const QString &label) -> QTreeWidgetItem * {
        const QList<QTreeWidgetItem *> entries = tree->findItems(label, Qt::MatchExactly | Qt::MatchRecursive);

        return entries.size() == 1 ? entries.first() : nullptr;
    };
    auto dismissWarning = [&]() {
        QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());

        if (box != nullptr)
        {
            warning = box->text();
            box->accept();
        }
    };
    QVERIFY(found(startupLabel) != nullptr);
    QVERIFY(triggerSettingsTreeAction(tree, found(startupLabel), QStringLiteral("删除(&D)"),
        dismissWarning, &observedActions));
    QCOMPARE(observedActions, expectedActions);
    QVERIFY(warning.contains(QStringLiteral("启动页")));
    QVERIFY(!window.m_dirty);
    warning.clear();
    QVERIFY(found(QStringLiteral("003:受引用画面")) != nullptr);
    QVERIFY(triggerSettingsTreeAction(tree, found(QStringLiteral("003:受引用画面")), QStringLiteral("删除(&D)"), dismissWarning));
    QVERIFY(warning.contains(QStringLiteral("被引用")));
    QCOMPARE(window.currentEditorProject().pageOrder, original.pageOrder);
    QVERIFY(!window.m_dirty);
    QVERIFY(found(QStringLiteral("007:可删除画面")) != nullptr);
    QVERIFY(triggerSettingsTreeAction(tree, found(QStringLiteral("007:可删除画面")), QStringLiteral("打开(&O)")));
    QCOMPARE(window.currentEditorProject().activePageId, disposable.id);
    QVERIFY(triggerSettingsTreeAction(tree, found(QStringLiteral("007:可删除画面")), QStringLiteral("删除(&D)")));
    QCOMPARE(window.currentEditorProject().activePageId, startupId);
    QCOMPARE(window.currentEditorProject().pageOrder, (QStringList{startupId, target.id}));
    window.m_commands.value(QStringLiteral("action_undo"))->trigger(); //20261123-9 add
    QCOMPARE(window.currentEditorProject().pageOrder, original.pageOrder);
    window.m_commands.value(QStringLiteral("action_redo"))->trigger();
    QCOMPARE(window.currentEditorProject().pageOrder, (QStringList{startupId, target.id}));
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    QCOMPARE(reopened.pageOrder, (QStringList{startupId, target.id}));
    QCOMPARE(reopened.startupPageId, startupId);
    QCOMPARE(reopened.references.first().targetId, target.id);
}

/******************************************************************************
 * Function Name   : ordinaryPageHelpAndSlideVisibilityFollowDraft
 * Description     : Check that the optional targets appear only when enabled and context help leaves the dialog draft intact.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::ordinaryPageHelpAndSlideVisibilityFollowDraft() //20261123-10 add
{
    HmiProject project = workflowProject();
    const QJsonObject original = project.toJson();
    bool correct = false;

    QTimer::singleShot(0, this, [&]() {
        QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QGroupBox *slide;
        QComboBox *target;
        QLineEdit *name;
        QDialogButtonBox *buttons;
        QDialog *help;

        if (editor == nullptr)
        {
            return;
        }
        slide = editor->findChild<QGroupBox *>(QStringLiteral("pageSlideGroup"));
        target = editor->findChild<QComboBox *>(QStringLiteral("pageSlideLeftTarget"));
        name = editor->findChild<QLineEdit *>(QStringLiteral("newPageNameEdit"));
        buttons = editor->findChild<QDialogButtonBox *>();
        if (slide == nullptr || target == nullptr || name == nullptr || buttons == nullptr ||
            buttons->button(QDialogButtonBox::Help) == nullptr || !target->isVisible() || target->isEnabled()) //20261123-11 change
        {
            editor->reject();
            return;
        }
        slide->setChecked(true);
        if (!target->isVisible() || !target->isEnabled())
        {
            editor->reject();
            return;
        }
        slide->setChecked(false);
        name->setText(QStringLiteral("帮助窗口返回后保留"));
        buttons->button(QDialogButtonBox::Help)->click();
        help = editor->findChild<QDialog *>(QStringLiteral("LocalHelpDialog"));
        correct = target->isVisible() && !target->isEnabled() && help != nullptr && help->isVisible() && //20261123-11 change
            help->findChild<QTextBrowser *>(QStringLiteral("LocalHelpBrowser")) != nullptr &&
            editor->property("localHelpTopic").toString() == QStringLiteral("screen-properties") &&
            name->text() == QStringLiteral("帮助窗口返回后保留");
        if (help != nullptr)
        {
            help->close();
        }
        editor->reject();
    });
    QVERIFY(!editStage2NewNormalPage(nullptr, project));
    QVERIFY(correct);
    QCOMPARE(project.toJson(), original);
}

/******************************************************************************
 * Function Name   : ordinaryPageDialogUsesNativeClientLayout
 * Description     : Check both entry points against the captured V5.1 client geometry and target availability.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::ordinaryPageDialogUsesNativeClientLayout() //20261123-11 add
{
    HmiProject project = workflowProject();
    const QString captures = qEnvironmentVariable("KINSEAL_SCREEN_DIALOG_CAPTURE_DIR");
    bool existing;
    bool geometryMatches;
    bool targetMatches;
    bool captured;

    for (existing = false; ; existing = true)
    {
        geometryMatches = false;
        targetMatches = false;
        captured = captures.isEmpty();
        QTimer::singleShot(0, this, [&]() {
            QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            const QList<QPair<QString, QRect>> expected = {
                {QStringLiteral("newPageNameEdit"), QRect(90, 5, 191, 30)},
                {QStringLiteral("newPageNumberSpin"), QRect(428, 10, 161, 30)},
                {QStringLiteral("newPageBackground"), QRect(10, 80, 601, 301)},
                {QStringLiteral("newPageBackgroundPreview"), QRect(460, 120, 110, 110)},
                {QStringLiteral("pageSlideGroup"), QRect(30, 265, 571, 111)},
                {QStringLiteral("pageMacroTable"), QRect(10, 390, 601, 221)},
                {QStringLiteral("pageMacroAdd"), QRect(10, 620, 75, 31)}
            };
            QWidget *field;
            QComboBox *target;

            if (editor == nullptr)
            {
                return;
            }
            geometryMatches = editor->size() == QSize(638, 662);
            for (const auto &entry : expected)
            {
                field = editor->findChild<QWidget *>(entry.first);
                geometryMatches = geometryMatches && field != nullptr &&
                    QRect(field->mapTo(editor, QPoint()), field->size()) == entry.second;
            }
            target = editor->findChild<QComboBox *>(QStringLiteral("pageSlideLeftTarget"));
            targetMatches = target != nullptr && target->isVisible() == !existing && !target->isEnabled();
            if (!captures.isEmpty())
            {
                captured = editor->grab().save(QDir(captures).filePath(existing ?
                    QStringLiteral("lite-page-properties.png") : QStringLiteral("lite-page-new.png")));
            }
            editor->reject();
        });
        const bool accepted = existing ? editStage2NormalPageProperties(nullptr, project, project.activePageId) :
                                        editStage2NewNormalPage(nullptr, project);
        QVERIFY(!accepted);
        QVERIFY(geometryMatches);
        QVERIFY(targetMatches);
        QVERIFY(captured);
        if (existing)
        {
            break;
        }
    }
}

/******************************************************************************
 * Function Name   : screenAndWindowRejectTheSameNumber
 * Description     : Submit duplicate numbers from each tree entry and keep the project unchanged.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::screenAndWindowRejectTheSameNumber() //20261123-13 add
{
    HmiProject original = workflowProject();
    HmiPage popup;
    MainWindow window;
    QTreeWidget *tree;
    QString error;
    const QStringList groups = {QStringLiteral("窗口"), QStringLiteral("画面")};
    int index;

    popup.id = QStringLiteral("window-number-test");
    popup.name = QStringLiteral("测试窗口");
    popup.kind = QStringLiteral("window");
    popup.number = 17;
    QVERIFY2(original.addPage(popup, &error), qPrintable(error));
    window.setProject(original, QString());
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);

    for (index = 0; index < groups.size(); ++index)
    {
        const QList<QTreeWidgetItem *> entries = tree->findItems(groups.at(index), Qt::MatchExactly | Qt::MatchRecursive);
        bool rejected = false;
        QString message;

        QCOMPARE(entries.size(), 1);
        QVERIFY(triggerSettingsTreeAction(tree, entries.first(), QStringLiteral("新建") + groups.at(index), [&]() {
            QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QSpinBox *number;
            QDialogButtonBox *buttons;

            if (editor == nullptr)
            {
                return;
            }
            number = editor->findChild<QSpinBox *>(QStringLiteral("newPageNumberSpin"));
            buttons = editor->findChild<QDialogButtonBox *>();
            if (number != nullptr && buttons != nullptr)
            {
                number->setValue(index == 0 ? 0 : 17);
                QTimer::singleShot(0, editor, [&]() {
                    QMessageBox *warning = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());

                    if (warning != nullptr)
                    {
                        message = warning->text();
                        warning->accept();
                    }
                });
                buttons->button(QDialogButtonBox::Ok)->click();
                rejected = editor->isVisible();
            }
            editor->reject();
        }));
        QVERIFY(rejected);
        QCOMPARE(message, QStringLiteral("画面和窗口编号不能重复"));
        QCOMPARE(window.currentEditorProject().toJson(), original.toJson());
        QVERIFY(!window.m_dirty);
    }
}

/******************************************************************************
 * Function Name   : windowMenuCopiesAndConfirmsDeletion
 * Description     : Exercise the original window menu, copy placement and confirm or cancel deletion.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::windowMenuCopiesAndConfirmsDeletion() //20261123-13 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    HmiProject reopened;
    HmiPage popup;
    HmiObject control;
    HmiConfigurationNode placement;
    MainWindow window;
    QTreeWidget *tree;
    QString error;
    QString path;
    QString copyId;
    QStringList actions;
    bool copyDialogSeen = false;
    bool confirmationSeen;
    bool confirmed;
    const auto expectedMenu = QStringList{QStringLiteral("打开(&O)"), QStringLiteral("删除(&D)"),
        QStringLiteral("属性(&P)"), QStringLiteral("复制(&C)")};

    QVERIFY(directory.isValid());
    popup.id = QStringLiteral("popup-original");
    popup.name = QStringLiteral("源窗口");
    popup.kind = QStringLiteral("window");
    popup.number = 17; //20261123-13 change: Exercise the lowest available number when the project contains a gap.
    popup.width = 320;
    popup.height = 200;
    control = project.objects.first();
    control.id = QStringLiteral("popup-control");
    control.geometry = QRect(10, 10, 120, 40);
    popup.objects = {control};
    QVERIFY2(project.addPage(popup, &error), qPrintable(error));
    placement.id = QStringLiteral("popup-placement");
    placement.name = QStringLiteral("窗口显示位置");
    placement.ownerPageId = popup.id;
    placement.kind = QStringLiteral("windowPlacement");
    placement.implemented = true;
    placement.active = true;
    placement.properties = {{QStringLiteral("centered"), false}, {QStringLiteral("closeButton"), false},
        {QStringLiteral("x"), 55}, {QStringLiteral("y"), 66}};
    project.configurationNodes.append(placement);
    path = directory.filePath(QStringLiteral("window-menu.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    auto findEntry = [tree](const QString &label) {
        const QList<QTreeWidgetItem *> matches = tree->findItems(label, Qt::MatchExactly | Qt::MatchRecursive);

        return matches.size() == 1 ? matches.first() : nullptr;
    };
    QVERIFY(findEntry(QStringLiteral("017:源窗口")) != nullptr);
    QVERIFY(triggerSettingsTreeAction(tree, findEntry(QStringLiteral("017:源窗口")),
        QStringLiteral("打开(&O)"), {}, &actions));
    QCOMPARE(actions, expectedMenu);
    QCOMPARE(window.currentEditorProject().activePageId, popup.id);
    QVERIFY(!window.m_dirty);

    QVERIFY(triggerSettingsTreeAction(tree, findEntry(QStringLiteral("017:源窗口")), QStringLiteral("复制(&C)"), [&]() {
        QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QLineEdit *name;
        QPushButton *ok;

        if (editor == nullptr)
        {
            return;
        }
        name = editor->findChild<QLineEdit *>(QStringLiteral("lineEdit"));
        ok = editor->findChild<QPushButton *>(QStringLiteral("pushButton_ok"));
        if (editor->windowTitle() == QStringLiteral("复制到当前窗口") && editor->size() == QSize(268, 69) &&
            name != nullptr && name->text().isEmpty() && ok != nullptr)
        {
            copyDialogSeen = true;
            name->setText(QStringLiteral("窗口副本"));
            ok->click();
        }
        if (editor->isVisible())
        {
            editor->reject();
        }
    }));
    QVERIFY(copyDialogSeen);
    const HmiProject copied = window.currentEditorProject();
    copyId = copied.activePageId;
    QVERIFY(copyId != popup.id);
    QCOMPARE(copied.startupPageId, project.startupPageId);
    QCOMPARE(copied.activePageNumber.value_or(-1), 1);
    QVERIFY(findEntry(QStringLiteral("001:窗口副本")) != nullptr);
    QCOMPARE(copied.objects.size(), 1);
    QVERIFY(copied.objects.first().id != control.id);
    QCOMPARE(copied.configurationNodes.size(), 2);
    QCOMPARE(copied.configurationNodes.last().ownerPageId, copyId);
    QCOMPARE(copied.configurationNodes.last().properties, placement.properties);

    for (confirmed = false; ; confirmed = true)
    {
        confirmationSeen = false;
        QVERIFY(triggerSettingsTreeAction(tree, findEntry(QStringLiteral("001:窗口副本")), QStringLiteral("删除(&D)"), [&]() {
            QMessageBox *dialog = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
            QAbstractButton *ok;
            QAbstractButton *no;

            if (dialog == nullptr)
            {
                return;
            }
            ok = dialog->button(QMessageBox::Ok);
            no = dialog->button(QMessageBox::No);
            confirmationSeen = dialog->text() == QStringLiteral("是否删除所选窗口?") &&
                ok != nullptr && no != nullptr && ok->text() == QStringLiteral("确定") && no->text() == QStringLiteral("否");
            if (confirmationSeen)
            {
                (confirmed ? ok : no)->click();
            }
            else
            {
                dialog->reject();
            }
        }));
        QVERIFY(confirmationSeen);
        if (confirmed)
        {
            break;
        }
        QCOMPARE(window.currentEditorProject().toJson(), copied.toJson());
    }

    QVERIFY(findEntry(QStringLiteral("001:窗口副本")) == nullptr);
    QCOMPARE(window.currentEditorProject().activePageId, project.startupPageId);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    QCOMPARE(reopened.toJson(), project.toJson());
}

/******************************************************************************
 * Function Name   : projectSettingsWindowCopyKeepsAssignedNumber
 * Description     : Copy a window through project settings and reopen it with the same number as the tree command.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::projectSettingsWindowCopyKeepsAssignedNumber() //20261123-14 add
{
    HmiProject seed = workflowProject();
    HmiProject reopened;
    HmiPage popup;
    QTemporaryDir directory;
    MainWindow window;
    QString path;
    QString error;

    QVERIFY(directory.isValid());
    popup.id = QStringLiteral("settings-source-window");
    popup.name = QStringLiteral("窗口原件");
    popup.kind = QStringLiteral("window");
    popup.number = 17;
    QVERIFY2(seed.addPage(popup, &error), qPrintable(error));
    path = directory.filePath(QStringLiteral("settings-window-copy.ksp"));
    QVERIFY2(seed.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.show();
    QTimer::singleShot(0, &window, []() {
        Stage2ProjectDialog *dialog = qobject_cast<Stage2ProjectDialog *>(QApplication::activeModalWidget());
        QListWidget *pages;
        QLineEdit *name;
        QPushButton *copy;
        QDialogButtonBox *buttons;

        if (dialog == nullptr)
        {
            return;
        }
        pages = dialog->findChild<QListWidget *>(QStringLiteral("pageList"));
        name = dialog->findChild<QLineEdit *>(QStringLiteral("pageNameEdit"));
        copy = dialog->findChild<QPushButton *>(QStringLiteral("copyPageButton"));
        buttons = dialog->findChild<QDialogButtonBox *>();
        if (pages != nullptr && pages->count() == 2 && name != nullptr && copy != nullptr && buttons != nullptr)
        {
            pages->setCurrentRow(1);
            name->setText(QStringLiteral("设置窗口副本"));
            copy->click();
            buttons->button(QDialogButtonBox::Ok)->click();
        }
        if (dialog->isVisible())
        {
            dialog->reject();
        }
    });
    window.m_commands.value(QStringLiteral("action_project_settings"))->trigger();
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    QCOMPARE(reopened.pageCount(), 3);
    QCOMPARE(reopened.allPages().last().name, QStringLiteral("设置窗口副本"));
    QCOMPARE(reopened.allPages().last().number.value_or(-1), 1);
    QCOMPARE(reopened.allPages().at(1).number.value_or(-1), 17);
    QCOMPARE(reopened.startupPageId, seed.startupPageId);
}

/******************************************************************************
 * Function Name   : languageTreeUsesEditableRowsAndPreservesTranslations
 * Description     : Rename a language slot through the settings tree and retain its text after reopening.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::languageTreeUsesEditableRowsAndPreservesTranslations() //20261123-19 add
{
    HmiProject project = workflowProject();
    HmiProject reopened;
    QTemporaryDir directory;
    MainWindow window;
    QTreeWidget *tree;
    QString error;
    QString path;
    bool confirmed;
    bool correctDialog;

    QVERIFY(directory.isValid());
    project.objects.first().hasStaticTextProperties = true;
    project.objects.first().staticText.languageTexts.insert(QStringLiteral("中文"), QStringLiteral("保留本行文字"));
    project.startupLanguage = QStringLiteral("中文");
    path = directory.filePath(QStringLiteral("languages.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    for (confirmed = false; ; confirmed = true)
    {
        const auto items = tree->findItems(QStringLiteral("语言设置"), Qt::MatchExactly | Qt::MatchRecursive);

        QCOMPARE(items.size(), 1);
        correctDialog = false;
        QTimer::singleShot(0, &window, [&]() {
            QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QComboBox *count;
            QComboBox *current;
            QLineEdit *first;
            QLineEdit *third;
            QDialogButtonBox *buttons;

            if (dialog == nullptr)
            {
                return;
            }
            count = dialog->findChild<QComboBox *>(QStringLiteral("lanComboBox"));
            current = dialog->findChild<QComboBox *>(QStringLiteral("curLanComboBox"));
            first = dialog->findChild<QLineEdit *>(QStringLiteral("lineEdit_1"));
            third = dialog->findChild<QLineEdit *>(QStringLiteral("lineEdit_3"));
            buttons = dialog->findChild<QDialogButtonBox *>();
            if (dialog->windowTitle() == QStringLiteral("语言设置") && count != nullptr && current != nullptr &&
                first != nullptr && third != nullptr && buttons != nullptr && count->count() == 6 && !third->isVisible())
            {
                count->setCurrentIndex(2);
                first->setText(QStringLiteral("简体中文"));
                current->setCurrentIndex(2);
                correctDialog = third->isVisible() && current->currentText() == QStringLiteral("德语");
                buttons->button(confirmed ? QDialogButtonBox::Ok : QDialogButtonBox::Cancel)->click();
            }
            if (dialog->isVisible())
            {
                dialog->reject();
            }
        });
        Q_EMIT tree->itemDoubleClicked(items.first(), 0);
        QVERIFY(correctDialog);
        if (confirmed)
        {
            break;
        }
        QCOMPARE(window.currentEditorProject().toJson(), project.toJson());
        QVERIFY(!window.m_dirty);
    }
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    QCOMPARE(reopened.languages, (QStringList{QStringLiteral("简体中文"), QStringLiteral("英语"), QStringLiteral("德语")}));
    QCOMPARE(reopened.activeLanguage, QStringLiteral("德语"));
    QCOMPARE(reopened.startupLanguage, QStringLiteral("简体中文"));
    QCOMPARE(reopened.objects.first().staticText.languageTexts.value(QStringLiteral("简体中文")), QStringLiteral("保留本行文字"));
}

/******************************************************************************
 * Function Name   : extendedSystemSettingsPersist_data
 * Description     : Supply each group of device-facing settings and its original tree label.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::extendedSystemSettingsPersist_data() //20261123-15 add
{
    QTest::addColumn<QString>("label");
    QTest::addColumn<QString>("kind");
    QTest::newRow("hmi-status") << QStringLiteral("HMI状态") << QStringLiteral("hmiStatus");
    QTest::newRow("plc-control") << QStringLiteral("PLC控制") << QStringLiteral("plcControl");
    QTest::newRow("clock") << QStringLiteral("时钟") << QStringLiteral("clockSettings");
    QTest::newRow("panel") << QStringLiteral("HMI保护") << QStringLiteral("panelProtection");
}

/******************************************************************************
 * Function Name   : extendedSystemSettingsPersist
 * Description     : Open each real editor, apply its dependent fields and reopen the saved KSP.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::extendedSystemSettingsPersist() //20261123-15 add
{
    QFETCH(QString, label);
    QFETCH(QString, kind);
    const HmiProject seed = workflowProject();
    HmiProject reopened;
    QTemporaryDir directory;
    MainWindow window;
    QTreeWidget *tree;
    QJsonObject stored;
    QString error;
    QString path;
    bool changed = false;

    QVERIFY(directory.isValid());
    path = directory.filePath(QStringLiteral("settings.ksp"));
    QVERIFY2(seed.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    const auto entries = tree->findItems(label, Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(entries.size(), 1);
    QTimer::singleShot(0, &window, [&]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QDialogButtonBox *buttons;

        if (dialog == nullptr)
        {
            return;
        }
        buttons = dialog->findChild<QDialogButtonBox *>();
        if (kind == QStringLiteral("hmiStatus") || kind == QStringLiteral("plcControl"))
        {
            const bool status = kind == QStringLiteral("hmiStatus");
            QCheckBox *enable = dialog->findChild<QCheckBox *>(status ? QStringLiteral("bCurSceneId") :
                QStringLiteral("bCtlRecipeWrite"));
            QLineEdit *address = dialog->findChild<QLineEdit *>(status ? QStringLiteral("lineEdit_screen") :
                QStringLiteral("lineEdit_sWriteRecipeAddr"));

            if (enable != nullptr && address != nullptr && !address->isEnabled())
            {
                enable->click();
                address->setText(status ? QStringLiteral("LW65535") : QStringLiteral("LW22"));
                changed = address->isEnabled();
            }
        }
        else if (kind == QStringLiteral("clockSettings"))
        {
            QGroupBox *write = dialog->findChild<QGroupBox *>(QStringLiteral("bWriteToPLC"));
            QGroupBox *read = dialog->findChild<QGroupBox *>(QStringLiteral("bReadFromPLC"));
            QSpinBox *interval = dialog->findChild<QSpinBox *>(QStringLiteral("nWriteTimeInterval"));

            if (write != nullptr && read != nullptr && interval != nullptr)
            {
                write->setChecked(true);
                interval->setValue(2);
                changed = !read->isEnabled() && interval->isEnabled();
            }
        }
        else
        {
            QCheckBox *enable = dialog->findChild<QCheckBox *>(QStringLiteral("bEnabled"));
            QComboBox *count = dialog->findChild<QComboBox *>(QStringLiteral("nProtectNum"));
            QAbstractButton *level = dialog->findChild<QAbstractButton *>(QStringLiteral("ProtectButton2"));
            QDateEdit *date = dialog->findChild<QDateEdit *>(QStringLiteral("mExpiredDate"));
            QLineEdit *password = dialog->findChild<QLineEdit *>(QStringLiteral("sUnlockPWD"));

            if (enable != nullptr && count != nullptr && level != nullptr && date != nullptr &&
                password != nullptr && !count->isEnabled())
            {
                enable->click();
                count->setCurrentIndex(1);
                password->setText(QStringLiteral("12345"));
                level->click();
                password->setText(QStringLiteral("26901"));
                date->setDate(QDate(2030, 5, 2));
                changed = date->isEnabled();
            }
        }
        if (changed && buttons != nullptr)
        {
            buttons->button(QDialogButtonBox::Ok)->click();
        }
        if (dialog->isVisible())
        {
            dialog->reject();
        }
    });
    Q_EMIT tree->itemDoubleClicked(entries.first(), 0);
    QVERIFY(changed);
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    QCOMPARE(reopened.configurationNodes.size(), seed.configurationNodes.size() + 1);
    QCOMPARE(reopened.configurationNodes.last().kind, kind);
    QVERIFY(reopened.configurationNodes.last().active);
    QVERIFY(!reopened.validateSinglePageBackend(&error));
    QVERIFY(error.contains(label));
    stored = reopened.configurationNodes.last().properties;
    if (kind == QStringLiteral("hmiStatus"))
    {
        QCOMPARE(stored.value(QStringLiteral("screenIndex")).toObject().value(QStringLiteral("address")).toString(),
            QStringLiteral("LW65535"));
    }
    else if (kind == QStringLiteral("plcControl"))
    {
        QCOMPARE(stored.value(QStringLiteral("writeRecipe")).toObject().value(QStringLiteral("address")).toString(),
            QStringLiteral("LW22"));
    }
    else if (kind == QStringLiteral("clockSettings"))
    {
        QCOMPARE(stored.value(QStringLiteral("write")).toObject().value(QStringLiteral("interval")).toInt(), 2);
    }
    else
    {
        QCOMPARE(stored.value(QStringLiteral("levelCount")).toInt(), 2);
        QCOMPARE(stored.value(QStringLiteral("levels")).toArray().at(1).toObject().value(QStringLiteral("date")).toString(),
            QStringLiteral("2030-05-02"));
        const auto current = tree->findItems(label, Qt::MatchExactly | Qt::MatchRecursive);

        QCOMPARE(current.size(), 1);
        QTimer::singleShot(0, &window, []() {
            QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QComboBox *count;
            QDialogButtonBox *buttons;

            if (dialog == nullptr)
            {
                return;
            }
            count = dialog->findChild<QComboBox *>(QStringLiteral("nProtectNum"));
            buttons = dialog->findChild<QDialogButtonBox *>();
            if (count != nullptr && buttons != nullptr)
            {
                count->setCurrentIndex(0);
                buttons->button(QDialogButtonBox::Ok)->click();
            }
            if (dialog->isVisible())
            {
                dialog->reject();
            }
        });
        Q_EMIT tree->itemDoubleClicked(current.first(), 0);
        window.m_commands.value(QStringLiteral("action_save"))->trigger();
        QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
        stored = reopened.configurationNodes.last().properties;
        QCOMPARE(stored.value(QStringLiteral("levelCount")).toInt(), 1);
        QCOMPARE(stored.value(QStringLiteral("levels")).toArray().at(1).toObject().value(QStringLiteral("date")).toString(),
            QStringLiteral("2021-01-01"));
        QVERIFY(stored.value(QStringLiteral("levels")).toArray().at(1).toObject().value(QStringLiteral("password")).toString().isEmpty());
    }
}

/******************************************************************************
 * Function Name   : fileProtectionGatesProjectReopen
 * Description     : Enable file protection and verify an independent editor can open it with the saved password.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::fileProtectionGatesProjectReopen() //20261123-17 add
{
#ifndef Q_OS_WIN
    QSKIP("File protection uses the Windows cryptography provider.");
#else
    HmiProject seed = workflowProject();
    HmiProject unauthenticated;
    QTemporaryDir directory;
    MainWindow window;
    MainWindow reopened;
    QTreeWidget *tree;
    QFile file;
    const QString password = QStringLiteral("KspCase7");
    QString error;
    QString path;
    bool needsPassword = false;

    QVERIFY(directory.isValid());
    seed.name = QStringLiteral("ProtectedWorkflowFixture");
    path = directory.filePath(QStringLiteral("protected.ksp"));
    QVERIFY2(seed.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    const auto entries = tree->findItems(QStringLiteral("文件保护"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(entries.size(), 1);
    QTimer::singleShot(0, &window, [&]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QCheckBox *enabled;
        QLineEdit *field;
        QDialogButtonBox *buttons;

        if (dialog == nullptr)
        {
            return;
        }
        enabled = dialog->findChild<QCheckBox *>(QStringLiteral("bUsePWD"));
        field = dialog->findChild<QLineEdit *>(QStringLiteral("sPWD"));
        buttons = dialog->findChild<QDialogButtonBox *>();
        if (enabled != nullptr && field != nullptr && field->maxLength() == 11 && buttons != nullptr)
        {
            enabled->click();
            field->setText(password);
            buttons->button(QDialogButtonBox::Ok)->click();
        }
        if (dialog->isVisible())
        {
            dialog->reject();
        }
    });
    Q_EMIT tree->itemDoubleClicked(entries.first(), 0);
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!HmiProject::load(path, &unauthenticated, &error, {}, &needsPassword));
    QVERIFY(needsPassword);
    QVERIFY(!HmiProject::load(path, &unauthenticated, &error, password.toLower(), &needsPassword));
    QVERIFY(needsPassword);
    file.setFileName(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray bytes = file.readAll();
    file.close();
    QVERIFY(!bytes.contains(seed.name.toUtf8()));
    QVERIFY(!bytes.contains(password.toUtf8()));
    QTimer::singleShot(0, &reopened, [&]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QLineEdit *field;
        QPushButton *ok;

        if (dialog == nullptr)
        {
            return;
        }
        field = dialog->findChild<QLineEdit *>(QStringLiteral("FilePassWordEntry"));
        ok = dialog->findChild<QPushButton *>(QStringLiteral("OKBtn"));
        if (field == nullptr || ok == nullptr)
        {
            dialog->reject();
            return;
        }
        field->setText(password);
        ok->click();
    });
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    QCOMPARE(reopened.currentEditorProject().name, seed.name);
#endif
}

/******************************************************************************
 * Function Name   : nativeFileProtectionIsCheckedBeforeImport
 * Description     : Reject protected vendor files before import and preserve access policy during migration.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::nativeFileProtectionIsCheckedBeforeImport() //20261123-17 add
{
    const QString path = QFINDTESTDATA("fixtures/native-source/file-protection-probe.sm");
    const QString password = QStringLiteral("ProbeA01");
    const QByteArray nativePassword = QByteArray::fromHex("00500072006f00620065004100300031");
    QTemporaryDir directory;
    HmiProject imported = workflowProject();
    HmiProject reopened;
    const QString originalName = imported.name;
    QString error;
    QString plainFile;
    bool requiresPassword = false;

    QVERIFY(!path.isEmpty());
    QVERIFY(directory.isValid());
    QVERIFY(!HmiProject::load(path, &imported, &error, {}, &requiresPassword));
    QVERIFY(requiresPassword);
    QVERIFY(!HmiProject::load(path, &imported, &error, password.toLower(), &requiresPassword));
    QVERIFY(requiresPassword);
    QCOMPARE(imported.name, originalName);
    QVERIFY2(HmiProject::load(path, &imported, &error, password, &requiresPassword), qPrintable(error));
    QCOMPARE(imported.filePassword, password);
    QVERIFY(imported.nativeSourceSnapshot.contains(nativePassword));
#ifdef Q_OS_WIN
    const QString encryptedFile = directory.filePath(QStringLiteral("migrated-protected.ksp"));
    QVERIFY2(imported.save(encryptedFile, &error), qPrintable(error));
    QVERIFY(!HmiProject::load(encryptedFile, &reopened, &error, {}, &requiresPassword));
    QVERIFY(requiresPassword);
    QVERIFY2(HmiProject::load(encryptedFile, &reopened, &error, password), qPrintable(error));
#endif
    imported.filePassword.clear();
    plainFile = directory.filePath(QStringLiteral("migrated-unprotected.ksp"));
    QVERIFY2(imported.save(plainFile, &error), qPrintable(error));
    QVERIFY2(HmiProject::load(plainFile, &reopened, &error), qPrintable(error));
    QVERIFY(!reopened.nativeSourceSnapshot.isEmpty());
    QVERIFY(!reopened.nativeSourceSnapshot.contains(nativePassword));
}

/******************************************************************************
 * Function Name   : macroScheduleCommandsOpenTheirManagers_data
 * Description     : Define the two original tool-bar actions and their corresponding tree entries.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::macroScheduleCommandsOpenTheirManagers_data() //20261123-20 add
{
    QTest::addColumn<QString>("actionName");
    QTest::addColumn<QString>("treeLabel");
    QTest::addColumn<QString>("managerName");
    QTest::newRow("global") << QStringLiteral("action_GlobalMacro") << QStringLiteral("全局宏")
        << QStringLiteral("GlobalMacroManager");
    QTest::newRow("initialization") << QStringLiteral("action_InitMacro") << QStringLiteral("初始化宏")
        << QStringLiteral("InitMacroManager");
}

/******************************************************************************
 * Function Name   : macroScheduleCommandsOpenTheirManagers
 * Description     : Check the shared native tool-bar and project-tree entry points for each macro manager.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::macroScheduleCommandsOpenTheirManagers() //20261123-20 add
{
    QFETCH(QString, actionName);
    QFETCH(QString, treeLabel);
    QFETCH(QString, managerName);
    MainWindow window;
    QAction *action;
    QTreeWidget *tree;
    bool foundButton = false;
    bool managerOpened;
    int entry;

    window.setProject(workflowProject(), QString());
    window.show();
    action = window.m_commands.value(actionName);
    QVERIFY(action != nullptr);
    QVERIFY(action->isEnabled());
    QVERIFY(!action->property("nativeShellPending").toBool());
    for (const QToolButton *button : window.findChildren<QToolButton *>())
    {
        foundButton |= button->defaultAction() == action;
    }
    QVERIFY(foundButton);
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    const auto nodes = tree->findItems(treeLabel, Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(nodes.size(), 1);
    for (entry = 0; entry < 2; ++entry)
    {
        managerOpened = false;
        QTimer::singleShot(0, &window, [&]() {
            QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());

            if (dialog != nullptr)
            {
                managerOpened = dialog->objectName() == managerName;
                dialog->reject();
            }
        });
        if (entry == 0)
        {
            action->trigger();
        }
        else
        {
            Q_EMIT tree->itemDoubleClicked(nodes.first(), 0);
        }
        QVERIFY(managerOpened);
    }
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : projectMacroSchedulesSaveOrderedBindings
 * Description     : Verify global and startup drafts, ordered targets and persistent reference protection.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::projectMacroSchedulesSaveOrderedBindings() //20261123-20 add
{
    HmiProject seed = workflowProject();
    HmiProject loaded;
    QTemporaryDir directory;
    MainWindow window;
    QList<MacroScheduleRow> rows;
    QString error;
    QString path;
    QString macroId;
    bool checked;

    QVERIFY(directory.isValid());
    QVERIFY2(MacroWorkspace::create(&seed, QStringLiteral("Alpha"), MacroWorkspace::defaultSource(), &error), qPrintable(error));
    QVERIFY2(MacroWorkspace::create(&seed, QStringLiteral("Startup"), MacroWorkspace::defaultSource(), &error), qPrintable(error));
    macroId = seed.macroIds.value(QStringLiteral("Alpha"));
    path = directory.filePath(QStringLiteral("macro-schedules.ksp"));
    QVERIFY2(seed.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.show();

    auto schedule = [&](bool initialization, bool keep) {
        QAction *command = window.m_commands.value(initialization ? QStringLiteral("action_InitMacro") :
            QStringLiteral("action_GlobalMacro"));

        checked = false;
        QTimer::singleShot(0, &window, [&]() {
            QDialog *manager = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QTableWidget *table;
            QPushButton *add;
            QPushButton *confirm;
            bool defaults;

            if (manager == nullptr)
            {
                return;
            }
            table = manager->findChild<QTableWidget *>(QStringLiteral("MacroScheduleTable"));
            add = manager->findChild<QPushButton *>(QStringLiteral("MacroScheduleAdd"));
            confirm = manager->findChild<QPushButton *>(QStringLiteral("MacroScheduleConfirm"));
            if (table == nullptr || add == nullptr || confirm == nullptr)
            {
                manager->reject();
                return;
            }
            auto addRow = [&](const QString &name) {
                defaults = false;
                QTimer::singleShot(0, manager, [&]() {
                    QDialog *row = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                    QComboBox *source;
                    QPushButton *ok;
                    QSpinBox *frequency;
                    QSpinBox *count;
                    QCheckBox *trigger;
                    QLineEdit *address;

                    if (row == nullptr)
                    {
                        return;
                    }
                    source = row->findChild<QComboBox *>(QStringLiteral("MacroScheduleSource"));
                    ok = row->findChild<QPushButton *>(QStringLiteral("MacroScheduleConfirm"));
                    if (source == nullptr || ok == nullptr || source->findText(name) == -1)
                    {
                        row->reject();
                        return;
                    }
                    source->setCurrentText(name);
                    if (initialization)
                    {
                        defaults = row->windowTitle() == QStringLiteral("新建初始化宏");
                    }
                    else
                    {
                        frequency = row->findChild<QSpinBox *>(QStringLiteral("MacroScheduleFrequency"));
                        count = row->findChild<QSpinBox *>(QStringLiteral("MacroScheduleCount"));
                        trigger = row->findChild<QCheckBox *>(QStringLiteral("MacroScheduleTrigger"));
                        address = row->findChild<QLineEdit *>(QStringLiteral("MacroScheduleAddress"));
                        if (frequency != nullptr && count != nullptr && trigger != nullptr && address != nullptr)
                        {
                            defaults = frequency->value() == 1 && frequency->maximum() == 65535 &&
                                count->value() == 0 && !trigger->isChecked() && !address->isEnabled();
                            frequency->setValue(65535);
                            count->setValue(3);
                            trigger->click();
                            defaults = defaults && address->isEnabled();
                        }
                    }
                    if (defaults)
                    {
                        ok->click();
                    }
                    else
                    {
                        row->reject();
                    }
                });
                add->click();
                return defaults;
            };
            checked = addRow(initialization ? QStringLiteral("Startup") : QStringLiteral("Alpha"));
            if (initialization && checked)
            {
                checked = addRow(QStringLiteral("Alpha"));
            }
            checked = checked && table->rowCount() == (initialization ? 2 : 1);
            if (!initialization && checked)
            {
                checked = table->item(0, 1) != nullptr && table->item(0, 1)->text() == QStringLiteral("6553500");
            }
            if (keep && checked)
            {
                if (initialization)
                {
                    confirm->click();
                }
                else
                {
                    manager->close(); //20261123-20 add: The native title-bar close retains confirmed rows.
                }
            }
            else
            {
                QTest::keyClick(manager, Qt::Key_Escape);
            }
        });
        command->trigger();
        return checked;
    };
    QVERIFY(schedule(false, true));
    QVERIFY(schedule(true, false));
    QVERIFY(macroScheduleRows(window.currentEditorProject(), true).isEmpty());
    QVERIFY(schedule(true, true));
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY2(HmiProject::load(path, &loaded, &error), qPrintable(error));
    rows = macroScheduleRows(loaded, false);
    QCOMPARE(rows.size(), 1);
    QCOMPARE(rows.first().macroId, macroId);
    QCOMPARE(rows.first().frequency, 65535);
    QCOMPARE(rows.first().count, 3);
    QCOMPARE(rows.first().address, QStringLiteral("LB0"));
    rows = macroScheduleRows(loaded, true);
    QCOMPARE(rows.size(), 2);
    QCOMPARE(rows.first().macroId, seed.macroIds.value(QStringLiteral("Startup")));
    QCOMPARE(rows.last().macroId, macroId);
    QVERIFY(!MacroWorkspace::remove(&loaded, QStringLiteral("Alpha"), &error));
    QVERIFY2(MacroWorkspace::rename(&loaded, QStringLiteral("Alpha"), QStringLiteral("Renamed"), &error), qPrintable(error));
    QCOMPARE(loaded.macroIds.value(QStringLiteral("Renamed")), macroId);
    QCOMPARE(macroScheduleRows(loaded, false).first().macroId, macroId);
    QVERIFY2(loaded.validateSinglePageBackend(&error), qPrintable(error));
}

/******************************************************************************
 * Function Name   : nativeProjectMacroSchedulesRoundTrip
 * Description     : Reopen the original editor's two schedules, preserve unchanged bytes and verify edits.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::nativeProjectMacroSchedulesRoundTrip() //20261123-23 add
{
    QFile file(QFINDTESTDATA("fixtures/native-source/macro-schedule/amt070-blank-schedules.sm"));
    QByteArray original;
    QByteArray exported;
    HmiProject source;
    HmiProject reopened;
    QList<MacroScheduleRow> global;
    QList<MacroScheduleRow> initialization;
    QString error;

    QVERIFY(file.open(QIODevice::ReadOnly));
    original = file.readAll();
    QVERIFY2(importNativeProjectSource(original, &source, &error), qPrintable(error));
    global = macroScheduleRows(source, false);
    initialization = macroScheduleRows(source, true);
    QCOMPARE(global.size(), 1);
    QCOMPARE(initialization.size(), 1);
    QCOMPARE(global.first().macroId, source.macroIds.value(QStringLiteral("SetLB1")));
    QCOMPARE(initialization.first().macroId, global.first().macroId);
    QVERIFY2(exportNativeProjectSource(source, &exported, &error), qPrintable(error));
    QCOMPARE(exported, original);

    global.first().frequency = 300;
    global.first().count = 3;
    global.first().address = QStringLiteral("LB17");
    QVERIFY2(replaceMacroSchedule(&source, false, global, &error), qPrintable(error));
    QVERIFY2(replaceMacroSchedule(&source, true, {}, &error), qPrintable(error));
    QVERIFY2(exportNativeProjectSource(source, &exported, &error), qPrintable(error));
    QVERIFY2(importNativeProjectSource(exported, &reopened, &error), qPrintable(error));
    global = macroScheduleRows(reopened, false);
    QCOMPARE(global.size(), 1);
    QCOMPARE(global.first().macroId, reopened.macroIds.value(QStringLiteral("SetLB1")));
    QCOMPARE(global.first().frequency, 300);
    QCOMPARE(global.first().count, 3);
    QCOMPARE(global.first().address, QStringLiteral("LB17"));
    QVERIFY(macroScheduleRows(reopened, true).isEmpty());
}

/******************************************************************************
 * Function Name   : emptyWindowKeepsZeroWidthAcrossSave
 * Description     : Preserve an empty zero-width window through save and fresh editor properties.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::emptyWindowKeepsZeroWidthAcrossSave() //20261123-13 add
{
    const HmiProject seed = workflowProject();
    QTemporaryDir directory;
    MainWindow window;
    MainWindow reopened;
    QTreeWidget *tree;
    QTreeWidgetItem *group;
    QString path;
    QString error;
    bool widthRestored = false;

    QVERIFY(directory.isValid());
    path = directory.filePath(QStringLiteral("zero-width.ksp"));
    QVERIFY2(seed.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    const auto groups = tree->findItems(QStringLiteral("窗口"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(groups.size(), 1);
    group = groups.first();
    QVERIFY(triggerSettingsTreeAction(tree, group, QStringLiteral("新建窗口"), []() {
        QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QSpinBox *width;
        QDialogButtonBox *buttons;

        if (editor == nullptr)
        {
            return;
        }
        width = editor->findChild<QSpinBox *>(QStringLiteral("windowWidth"));
        buttons = editor->findChild<QDialogButtonBox *>();
        if (width != nullptr && buttons != nullptr)
        {
            width->setValue(0);
            buttons->button(QDialogButtonBox::Ok)->click();
        }
        if (editor->isVisible())
        {
            editor->reject();
        }
    }));
    QCOMPARE(window.currentEditorProject().activePageKind, QStringLiteral("window"));
    QCOMPARE(window.currentEditorProject().pageWidth, 0);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    reopened.show();
    tree = reopened.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    const auto entries = tree->findItems(QStringLiteral("001:窗口2"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(entries.size(), 1);
    QVERIFY(triggerSettingsTreeAction(tree, entries.first(), QStringLiteral("属性(&P)"), [&]() {
        QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QSpinBox *width;
        QSpinBox *height;

        if (editor == nullptr)
        {
            return;
        }
        width = editor->findChild<QSpinBox *>(QStringLiteral("windowWidth"));
        height = editor->findChild<QSpinBox *>(QStringLiteral("windowHeight"));
        widthRestored = width != nullptr && height != nullptr && width->value() == 0 && height->value() == 180;
        editor->reject();
    }));
    QVERIFY(widthRestored);
    QVERIFY(!reopened.currentEditorProject().validateSinglePageBackend(&error));
}

/******************************************************************************
 * Function Name   : keyboardTreeAddsSelectedTemplates
 * Description     : Create all four keyboard templates from the project tree and reopen their editable pages.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::keyboardTreeAddsSelectedTemplates() //20261123-24 add
{
    QTemporaryDir directory;
    const QString path = directory.filePath(QStringLiteral("keyboard.ksp"));
    const HmiProject seed = workflowProject();
    const QStringList names = {QStringLiteral("Keyboard_Number"), QStringLiteral("Keyboard_Ascii_Small"),
        QStringLiteral("Keyboard_Ascii_Full"), QStringLiteral("Keyboard_Hex")};
    MainWindow window;
    HmiProject reopened;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> groups;
    QList<QCheckBox *> selections;
    QList<HmiPage> pages;
    QList<QTreeWidgetItem *> keyboardEntries;
    QString error;
    int index;
    bool defaultSelection = false;
    bool propertyEdited = false;
    bool previewTyped = false;

    QVERIFY2(seed.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    groups = tree->findItems(QStringLiteral("自定义键盘"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(groups.size(), 1);
    QVERIFY(triggerSettingsTreeAction(tree, groups.first(), QStringLiteral("添加自定义键盘"), [&]() {
        QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QDialogButtonBox *buttons;

        if (editor == nullptr) { return; }
        selections = editor->findChildren<QCheckBox *>();
        buttons = editor->findChild<QDialogButtonBox *>();
        if (editor->windowTitle() != QStringLiteral("添加自定义键盘") || selections.size() != 4 || buttons == nullptr)
        {
            editor->reject();
            return;
        }
        defaultSelection = selections.first()->text() == names.first() && selections.first()->isChecked();
        for (QCheckBox *choice : std::as_const(selections))
        {
            choice->setChecked(true);
        }
        buttons->button(QDialogButtonBox::Ok)->click();
    }));
    QVERIFY(defaultSelection);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    pages = reopened.allPages();
    pages.removeIf([](const HmiPage &page) { return page.kind != QStringLiteral("keyboard"); });
    QCOMPARE(pages.size(), 4);
    for (index = 0; index < names.size(); ++index)
    {
        QCOMPARE(pages.at(index).name, names.at(index));
        QCOMPARE(pages.at(index).number.value_or(-1), index);
        QVERIFY(!pages.at(index).objects.isEmpty());
    }
    QCOMPARE(QSize(pages.first().width, pages.first().height), QSize(320, 266));
    keyboardEntries = tree->findItems(QStringLiteral("000:Keyboard_Number"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(keyboardEntries.size(), 1);
    QVERIFY(triggerSettingsTreeAction(tree, keyboardEntries.first(), QStringLiteral("属性(&P)"), [&]() {
        QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QLineEdit *name;
        QSpinBox *number;
        QSpinBox *width;
        QComboBox *type;
        QDialogButtonBox *buttons;

        if (editor == nullptr) { return; }
        name = editor->findChild<QLineEdit *>(QStringLiteral("Stage2KeyboardName"));
        number = editor->findChild<QSpinBox *>(QStringLiteral("Stage2KeyboardNumber"));
        width = editor->findChild<QSpinBox *>(QStringLiteral("Stage2KeyboardWidth"));
        type = editor->findChild<QComboBox *>(QStringLiteral("Stage2KeyboardType"));
        buttons = editor->findChild<QDialogButtonBox *>();
        if (!name || !number || !width || !type || !buttons || type->isEnabled() ||
            type->currentText() != QStringLiteral("10进制键盘") || number->value() != 0 || width->value() != 320)
        {
            editor->reject();
            return;
        }
        propertyEdited = true;
        name->setText(QStringLiteral("自定义数字输入"));
        number->setValue(12);
        width->setValue(340);
        buttons->button(QDialogButtonBox::Ok)->click();
    }));
    QVERIFY(propertyEdited);
    keyboardEntries = tree->findItems(QStringLiteral("012:自定义数字输入"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(keyboardEntries.size(), 1);
    QVERIFY(triggerSettingsTreeAction(tree, keyboardEntries.first(), QStringLiteral("按键预览"), [&]() {
        QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QLineEdit *input;
        QWidget *surface;
        QPoint seven;
        QPoint minus;
        QPoint backspace;
        QPoint enter;

        if (editor == nullptr) { return; }
        input = editor->findChild<QLineEdit *>(QStringLiteral("KeyboardPreviewInput"));
        surface = editor->findChild<QWidget *>(QStringLiteral("KeyboardPreviewSurface"));
        if (input == nullptr || surface == nullptr)
        {
            editor->reject();
            return;
        }
        for (const HmiObject &object : pages.first().objects)
        {
            if (!object.hasFunctionButtonProperties) { continue; }
            if (object.functionButton.keyboardText == QStringLiteral("7")) { seven = object.geometry.center(); }
            if (object.functionButton.keyboardText == QStringLiteral("-")) { minus = object.geometry.center(); }
            if (object.functionButton.keyboardKey == 1) { backspace = object.geometry.center(); }
            if (object.functionButton.keyboardKey == 0) { enter = object.geometry.center(); }
        }
        if (seven.isNull() || minus.isNull() || backspace.isNull() || enter.isNull())
        {
            editor->reject();
            return;
        }
        QTest::mouseClick(surface, Qt::LeftButton, Qt::NoModifier, minus);
        QTest::mouseClick(surface, Qt::LeftButton, Qt::NoModifier, seven);
        QTest::mouseClick(surface, Qt::LeftButton, Qt::NoModifier, backspace);
        previewTyped = input->text() == QStringLiteral("-");
        QTest::mouseClick(surface, Qt::LeftButton, Qt::NoModifier, enter);
        if (editor->isVisible()) { editor->reject(); }
    }));
    QVERIFY(previewTyped);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    QVERIFY2(reopened.selectPage(pages.first().id, &error), qPrintable(error));
    QCOMPARE(reopened.pageName, QStringLiteral("自定义数字输入"));
    QCOMPARE(reopened.activePageNumber.value_or(-1), 12);
    QCOMPARE(reopened.pageWidth, 340);
    QCOMPARE(reopened.activePageKeyboardType.value_or(-1), 0);

    QVERIFY2(reopened.selectPage(reopened.startupPageId, &error), qPrintable(error));
    HmiObject numeric;
    numeric.id = QStringLiteral("numeric-keyboard-reference");
    numeric.name = QStringLiteral("数值显示器");
    numeric.type = QStringLiteral("number");
    numeric.hasNumberDisplayProperties = true;
    numeric.geometry = QRect(40, 200, 130, 45);
    numeric.address = QStringLiteral("LW20");
    NumberDisplayDialog inputEditor(numeric, reopened);
    auto *binding = inputEditor.findChild<QComboBox *>(QStringLiteral("Number_keyboardPageId"));
    auto *useKeyboard = inputEditor.findChild<QCheckBox *>(QStringLiteral("Number_customKeyboard"));
    QVERIFY(binding != nullptr && useKeyboard != nullptr);
    QCOMPARE(binding->count(), 4);
    QCOMPARE(binding->currentIndex(), -1);
    index = binding->findData(pages.first().id);
    QVERIFY(index >= 0);
    QCOMPARE(binding->itemText(index), QStringLiteral("012:自定义数字输入"));
    useKeyboard->setChecked(true);
    binding->setCurrentIndex(index);
    numeric = inputEditor.object();
    QCOMPARE(numeric.numberDisplay.keyboardPageId, pages.first().id);
    reopened.objects.append(numeric);
    QVERIFY2(reopened.validateModel(&error), qPrintable(error));
    QVERIFY(!reopened.removePage(pages.first().id, &error));
    QCOMPARE(reopened.keyboardNumbers().value(pages.first().id), 12);
    QVERIFY(!reopened.validateSinglePageBackend(&error));
}

/******************************************************************************
 * Function Name   : clickDialogWidgetAtCenter
 * Description     : Click a visible control through the window's mouse hit test.
 * Input Parameters: dialog, widget.
 * Return Value    : True when the control is exposed and the click was sent.
 * Date            : 2026.11.24
 ******************************************************************************/
static bool clickDialogWidgetAtCenter(QDialog *dialog, QWidget *widget) //20261124-3 add
{
    QPoint point;

    if (widget == nullptr || !widget->isVisible() || !widget->isEnabled() || dialog->windowHandle() == nullptr)
    {
        return false;
    }
    point = widget->mapTo(dialog, widget->rect().center());
    if (dialog->childAt(point) != widget)
    {
        return false;
    }
    QTest::mouseClick(dialog->windowHandle(), Qt::LeftButton, Qt::NoModifier, point);
    return true;
}

/******************************************************************************
 * Function Name   : clickDialogButtonAtCenter
 * Description     : Find and click a standard button using the dialog's mouse hit test.
 * Input Parameters: dialog, role.
 * Return Value    : True when the button is exposed and the click was sent.
 * Date            : 2026.11.24
 ******************************************************************************/
static bool clickDialogButtonAtCenter(QDialog *dialog, QDialogButtonBox::StandardButton role) //20261124-3 change
{
    QDialogButtonBox *box = dialog->findChild<QDialogButtonBox *>(QString(), Qt::FindDirectChildrenOnly);

    return box != nullptr && clickDialogWidgetAtCenter(dialog, box->button(role));
}

/******************************************************************************
 * Function Name   : pageProjectGalleryImportsFromProperties
 * Description     : Import from an empty gallery and exercise both confirmation boundaries in the editor.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.24
 ******************************************************************************/
void Stage2WorkflowTests::pageProjectGalleryImportsFromProperties() //20261124-3 add
{
    QTemporaryDir directory;
    HmiProject seed = workflowProject();
    MainWindow window;
    MainWindow reopened;
    QImage source(8, 4, QImage::Format_RGB32);
    QImage second(3, 5, QImage::Format_RGB32);
    const QString pictureName = QStringLiteral("测试背景.png");
    const QString secondName = QStringLiteral("批量图片.png");
    const QString imagePath = directory.filePath(pictureName);
    const QString projectPath = directory.filePath(QStringLiteral("gallery.ksp"));
    QAction *properties;
    QString error;
    int attempt;
    bool protectedBackground = false;
    bool removedUnused = false;

    QVERIFY(directory.isValid());
    seed.pictures.clear();
    source.fill(QColor(QStringLiteral("#158040")));
    second.fill(QColor(QStringLiteral("#4433a1")));
    QVERIFY(source.save(imagePath));
    QVERIFY(second.save(directory.filePath(secondName)));
    QVERIFY2(seed.save(projectPath, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(projectPath, &error), qPrintable(error));
    window.show();
    properties = window.findChild<QAction *>(QStringLiteral("action_openpropertiset"));
    QVERIFY(properties != nullptr && properties->isEnabled());
    for (attempt = 0; attempt < 3; ++attempt)
    {
        bool opened = false;
        bool actionsAvailable = false;
        bool imported = false;
        bool previewMatches = false;
        bool closed = false;

        QTimer::singleShot(0, &window, [&]() {
            QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (editor == nullptr)
            {
                return;
            }
            clickDialogWidgetAtCenter(editor, editor->findChild<QRadioButton *>(QStringLiteral("newPageBitmapMode")));
            QTimer::singleShot(0, editor, [&]() {
                QDialog *gallery = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                if (gallery == nullptr || gallery == editor)
                {
                    return;
                }
                opened = true;
                QPushButton *add = gallery->findChild<QPushButton *>(QStringLiteral("GalleryImport"));
                QPushButton *batch = gallery->findChild<QPushButton *>(QStringLiteral("GalleryBatchImport"));
                QPushButton *remove = gallery->findChild<QPushButton *>(QStringLiteral("GalleryDelete"));
                QListWidget *items = gallery->findChild<QListWidget *>(QStringLiteral("GalleryItems"));
                actionsAvailable = add != nullptr && add->isEnabled() && batch != nullptr && batch->isVisible() &&
                    remove != nullptr && remove->isVisible() && items != nullptr && items->count() == 0;
                if (actionsAvailable)
                {
                    QTimer::singleShot(0, gallery, [&]() {
                        QFileDialog *file = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
                        if (file != nullptr)
                        {
                            if (attempt < 2)
                            {
                                file->selectFile(imagePath);
                            }
                            else
                            {
                                QLineEdit *fileNames = file->findChild<QLineEdit *>(QStringLiteral("fileNameEdit"));
                                if (fileNames == nullptr)
                                {
                                    file->reject();
                                    return;
                                }
                                file->setDirectory(directory.path());
                                fileNames->setText(QStringLiteral("\"%1\" \"%2\"").arg(pictureName, secondName));
                            }
                            // The editor enters through the actual single or multi-file dialog.
                            QMetaObject::invokeMethod(file, "accept", Qt::DirectConnection);
                            if (file->isVisible())
                            {
                                file->reject();
                            }
                        }
                    });
                    const bool nativeDialogsDisabled = QApplication::testAttribute(Qt::AA_DontUseNativeDialogs);
                    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
                    clickDialogWidgetAtCenter(gallery, attempt < 2 ? add : batch);
                    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs, nativeDialogsDisabled);
                    const QList<QListWidgetItem *> matches = items->findItems(pictureName, Qt::MatchExactly);
                    imported = items->count() == (attempt < 2 ? 1 : 2) && matches.size() == 1;
                    if (imported)
                    {
                        items->setCurrentItem(matches.first());
                    }
                    clickDialogButtonAtCenter(gallery, attempt == 0 ? QDialogButtonBox::Cancel : QDialogButtonBox::Ok);
                }
                if (gallery->isVisible())
                {
                    gallery->reject();
                }
            });
            clickDialogWidgetAtCenter(editor, editor->findChild<QPushButton *>(QStringLiteral("newPageProjectGallery")));
            QLineEdit *path = editor->findChild<QLineEdit *>(QStringLiteral("pageBitmapPath"));
            QLabel *sample = editor->findChild<QLabel *>(QStringLiteral("newPageBackgroundPreview"));
            if (path != nullptr && sample != nullptr)
            {
                const QImage preview = sample->pixmap().toImage();
                previewMatches = attempt == 0 ? path->text().isEmpty() && preview.isNull() :
                    path->text() == pictureName && !preview.isNull() &&
                    preview.pixelColor(preview.rect().center()) == source.pixelColor(0, 0);
            }
            closed = clickDialogButtonAtCenter(editor, attempt == 2 ? QDialogButtonBox::Ok : QDialogButtonBox::Cancel) &&
                !editor->isVisible();
            if (editor->isVisible())
            {
                editor->reject();
            }
        });
        properties->trigger();
        QVERIFY2(opened, "A mouse click on Project Gallery must open the picker.");
        QVERIFY2(actionsAvailable, "A new project's empty gallery must offer import, batch import and delete.");
        QVERIFY2(imported, "The picture must be added and selected inside the gallery.");
        QVERIFY(previewMatches);
        QVERIFY(closed);
        if (attempt < 2)
        {
            QVERIFY(window.currentEditorProject().pictures.isEmpty());
            QVERIFY(window.currentEditorProject().activePageBackground.bitmapName.isEmpty());
        }
    }
    QCOMPARE(window.currentEditorProject().activePageBackground.bitmapName, pictureName);
    window.saveProject();
    QVERIFY2(reopened.openProjectPath(projectPath, &error), qPrintable(error));
    QCOMPARE(reopened.currentEditorProject().activePageBackground.bitmapName, pictureName);
    QCOMPARE(QImage::fromData(reopened.currentEditorProject().pictures.value(pictureName)).pixelColor(0, 0),
             source.pixelColor(0, 0));
    const QImage reopenedSecond = QImage::fromData(reopened.currentEditorProject().pictures.value(secondName));
    QCOMPARE(reopenedSecond.size(), second.size());
    QCOMPARE(reopenedSecond.pixelColor(0, 0), second.pixelColor(0, 0));

    reopened.show();
    properties = reopened.findChild<QAction *>(QStringLiteral("action_openpropertiset"));
    QVERIFY(properties != nullptr && properties->isEnabled());
    QTimer::singleShot(0, &reopened, [&]() {
        QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (editor == nullptr)
        {
            return;
        }
        QTimer::singleShot(0, editor, [&]() {
            QDialog *gallery = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (gallery == nullptr || gallery == editor)
            {
                return;
            }
            QListWidget *items = gallery->findChild<QListWidget *>(QStringLiteral("GalleryItems"));
            QPushButton *remove = gallery->findChild<QPushButton *>(QStringLiteral("GalleryDelete"));
            if (items != nullptr && QTest::qWaitFor([items]() { return items->count() == 2; }, 1000) &&
                items->currentItem() != nullptr && items->currentItem()->text() == pictureName)
            {
                QTimer::singleShot(0, gallery, [&]() {
                    QMessageBox *warning = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
                    if (warning != nullptr)
                    {
                        protectedBackground = warning->windowTitle() == QStringLiteral("图片仍被使用");
                        warning->accept();
                    }
                });
                clickDialogWidgetAtCenter(gallery, remove);
                const QList<QListWidgetItem *> unused = items->findItems(secondName, Qt::MatchExactly);
                if (protectedBackground && items->count() == 2 && unused.size() == 1)
                {
                    items->setCurrentItem(unused.first());
                    removedUnused = clickDialogWidgetAtCenter(gallery, remove) && items->count() == 1 &&
                        clickDialogButtonAtCenter(gallery, QDialogButtonBox::Ok);
                }
            }
            if (gallery->isVisible())
            {
                gallery->reject();
            }
        });
        clickDialogWidgetAtCenter(editor, editor->findChild<QPushButton *>(QStringLiteral("newPageProjectGallery")));
        clickDialogButtonAtCenter(editor, QDialogButtonBox::Ok);
        if (editor->isVisible())
        {
            editor->reject();
        }
    });
    properties->trigger();
    QVERIFY(protectedBackground);
    QVERIFY(removedUnused);
    QCOMPARE(reopened.currentEditorProject().activePageBackground.bitmapName, pictureName);
    QCOMPARE(reopened.currentEditorProject().pictures.keys(), QStringList{pictureName});
}

/******************************************************************************
 * Function Name   : windowTreeCreatesAndReopensPlacement
 * Description     : Open the native window entries and verify dimensions, position and close-button persistence.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void Stage2WorkflowTests::windowTreeCreatesAndReopensPlacement() //20261123-12 add
{
    QTemporaryDir directory;
    HmiProject seed = workflowProject();
    HmiProject reopened;
    MainWindow window;
    MainWindow loaded;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> entries;
    QString path;
    QString error;
    QString id;
    bool cancel;
    bool observed;
    const QString captures = qEnvironmentVariable("KINSEAL_SCREEN_DIALOG_CAPTURE_DIR");

    QVERIFY(directory.isValid());
    seed.macros.insert(QStringLiteral("windowTick"), QStringLiteral("void main() {}")); //20261123-12 add
    path = directory.filePath(QStringLiteral("popup-window.ksp"));
    QVERIFY2(seed.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    for (cancel = true; ; cancel = false)
    {
        observed = false;
        entries = tree->findItems(QStringLiteral("窗口"), Qt::MatchExactly | Qt::MatchRecursive);
        QCOMPARE(entries.size(), 1);
        QVERIFY(triggerSettingsTreeAction(tree, entries.first(), QStringLiteral("新建窗口"), [&]() {
            QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QSpinBox *number;
            QSpinBox *width;
            QSpinBox *height;
            QSpinBox *x;
            QSpinBox *y;
            QRadioButton *position;
            QCheckBox *close;
            QLineEdit *name;
            QDialogButtonBox *buttons;
            QPushButton *backgroundButton; //20261123-12 add
            QPushButton *addScript;
            bool colorChosen = false;
            bool scriptChosen = false;

            if (editor == nullptr)
            {
                return;
            }
            number = editor->findChild<QSpinBox *>(QStringLiteral("newPageNumberSpin"));
            width = editor->findChild<QSpinBox *>(QStringLiteral("windowWidth"));
            height = editor->findChild<QSpinBox *>(QStringLiteral("windowHeight"));
            x = editor->findChild<QSpinBox *>(QStringLiteral("windowX"));
            y = editor->findChild<QSpinBox *>(QStringLiteral("windowY"));
            position = editor->findChild<QRadioButton *>(QStringLiteral("windowSpecified"));
            close = editor->findChild<QCheckBox *>(QStringLiteral("windowClose"));
            name = editor->findChild<QLineEdit *>(QStringLiteral("newPageNameEdit"));
            buttons = editor->findChild<QDialogButtonBox *>();
            if (editor->windowTitle() != QStringLiteral("新建窗口") || editor->size() != QSize(631, 620) ||
                number == nullptr || width == nullptr || height == nullptr || x == nullptr || y == nullptr ||
                close == nullptr || position == nullptr || name == nullptr || buttons == nullptr ||
                name->text() != QStringLiteral("窗口2") || number->value() != 1 || width->value() != 240 ||
                height->value() != 180 || !close->isChecked() || x->isVisible() || y->isVisible())
            {
                editor->reject();
                return;
            }
            if (!captures.isEmpty() && !editor->grab().save(QDir(captures).filePath(QStringLiteral("lite-window-new.png"))))
            {
                editor->reject();
                return;
            }
            position->click();
            observed = x->isVisible() && y->isVisible() && x->maximum() == 1000 && y->maximum() == 1000;
            if (!captures.isEmpty() && !editor->grab().save(QDir(captures).filePath(QStringLiteral("lite-window-position.png"))))
            {
                observed = false;
            }
            number->setValue(12);
            width->setValue(320);
            height->setValue(200);
            x->setValue(55);
            y->setValue(66);
            close->setChecked(false);
            if (!cancel && observed) //20261123-12 add: Exercise popup appearance and script editing through their real dialogs.
            {
                backgroundButton = editor->findChild<QPushButton *>(QStringLiteral("newPageBackgroundColor"));
                addScript = editor->findChild<QPushButton *>(QStringLiteral("pageMacroAdd"));
                if (backgroundButton == nullptr || addScript == nullptr)
                {
                    observed = false;
                    editor->reject();
                    return;
                }
                QTimer::singleShot(0, editor, [&]() {
                    QColorDialog *colors = qobject_cast<QColorDialog *>(QApplication::activeModalWidget());

                    if (colors != nullptr)
                    {
                        colors->setCurrentColor(QColor(QStringLiteral("#158040")));
                        colorChosen = true;
                        colors->accept();
                    }
                });
                backgroundButton->click();
                QTimer::singleShot(0, editor, [&]() {
                    QDialog *script = qobject_cast<QDialog *>(QApplication::activeModalWidget());
                    QDialogButtonBox *scriptButtons;
                    QComboBox *source;

                    if (script == nullptr || script == editor)
                    {
                        return;
                    }
                    scriptButtons = script->findChild<QDialogButtonBox *>();
                    source = script->findChild<QComboBox *>(QStringLiteral("pageMacroSource"));
                    scriptChosen = scriptButtons != nullptr && source != nullptr &&
                        source->currentText() == QStringLiteral("windowTick");
                    if (scriptChosen)
                    {
                        scriptButtons->button(QDialogButtonBox::Ok)->click();
                    }
                    else
                    {
                        script->reject();
                    }
                });
                addScript->click();
                observed = colorChosen && scriptChosen;
            }
            observed = clickDialogButtonAtCenter(editor, cancel ? QDialogButtonBox::Cancel : QDialogButtonBox::Ok) &&
                       !editor->isVisible() && observed; //20261124-2 change
            if (editor->isVisible())
            {
                editor->reject();
            }
        }));
        QVERIFY(observed);
        if (!cancel)
        {
            break;
        }
        QCOMPARE(window.currentEditorProject().pageCount(), 1);
        QVERIFY(!window.m_dirty);
    }
    id = window.currentEditorProject().activePageId;
    QVERIFY(id != seed.activePageId);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    QCOMPARE(reopened.activePageId, id);
    QCOMPARE(reopened.activePageKind, QStringLiteral("window"));
    QCOMPARE(reopened.startupPageId, seed.startupPageId);
    QCOMPARE(reopened.activePageNumber.value_or(-1), 12);
    QCOMPARE(QSize(reopened.pageWidth, reopened.pageHeight), QSize(320, 200));
    QCOMPARE(reopened.activePageBackground.color, QStringLiteral("#158040")); //20261123-12 add
    QCOMPARE(reopened.configurationNodes.size(), 2);
    QCOMPARE(reopened.configurationNodes.last().ownerPageId, id);
    QCOMPARE(reopened.configurationNodes.last().name, QStringLiteral("窗口宏"));
    QCOMPARE(reopened.references.size(), 1);
    QCOMPARE(reopened.references.first().targetId, reopened.macroIds.value(QStringLiteral("windowTick")));
    QVERIFY(!reopened.validateSinglePageBackend(&error));
    QVERIFY2(loaded.openProjectPath(path, &error), qPrintable(error));
    loaded.resize(1353, 708);
    loaded.show();
    tree = loaded.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    entries = tree->findItems(QStringLiteral("012:窗口2"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(entries.size(), 1);
    observed = false;
    QVERIFY(triggerSettingsTreeAction(tree, entries.first(), QStringLiteral("属性(&P)"), [&]() {
        QDialog *editor = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QRadioButton *position;
        QSpinBox *x;
        QSpinBox *y;
        QCheckBox *close;
        QTableWidget *scripts; //20261123-12 add
        QDialog *help; //20261124-2 add
        QTextBrowser *browser;

        if (editor == nullptr)
        {
            return;
        }
        position = editor->findChild<QRadioButton *>(QStringLiteral("windowSpecified"));
        x = editor->findChild<QSpinBox *>(QStringLiteral("windowX"));
        y = editor->findChild<QSpinBox *>(QStringLiteral("windowY"));
        close = editor->findChild<QCheckBox *>(QStringLiteral("windowClose"));
        scripts = editor->findChild<QTableWidget *>(QStringLiteral("pageMacroTable")); //20261123-12 add
        observed = editor->windowTitle() == QStringLiteral("窗口属性") && position != nullptr &&
            x != nullptr && y != nullptr && close != nullptr && position->isChecked() &&
            x->value() == 55 && y->value() == 66 && !close->isChecked() &&
            scripts != nullptr && scripts->rowCount() == 1 && scripts->item(0, 0) != nullptr &&
            scripts->item(0, 0)->text() == QStringLiteral("windowTick"); //20261123-12 change
        if (observed)
        {
            x->setValue(75); //20261124-2 add: Help must leave unsaved window coordinates available for editing.
        }
        observed = observed && clickDialogButtonAtCenter(editor, QDialogButtonBox::Help);
        help = editor->findChild<QDialog *>(QStringLiteral("LocalHelpDialog"));
        browser = help == nullptr ? nullptr : help->findChild<QTextBrowser *>(QStringLiteral("LocalHelpBrowser"));
        observed = observed && help != nullptr && help->isVisible() && browser != nullptr &&
                   QFileInfo(browser->document()->baseUrl().toLocalFile()).fileName() == QStringLiteral("project.html");
        if (help != nullptr && !clickDialogButtonAtCenter(help, QDialogButtonBox::Close))
        {
            observed = false;
            help->close();
        }
        observed = observed && !help->isVisible() && editor->isVisible() && x->value() == 75;
        observed = clickDialogButtonAtCenter(editor, QDialogButtonBox::Cancel) &&
                   !editor->isVisible() && editor->result() == QDialog::Rejected && observed;
        if (editor->isVisible())
        {
            editor->reject();
        }
    }));
    QVERIFY(observed);
    QVERIFY(!loaded.m_dirty);
    QCOMPARE(loaded.currentEditorProject().toJson(), reopened.toJson()); //20261124-2 add
}

/******************************************************************************
 * Function Name   : ordinaryPageNumbersAreEditableAndPersisted
 * Description     : Reject an occupied page number and round-trip a non-default ordinary number without changing window labels.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void Stage2WorkflowTests::ordinaryPageNumbersAreEditableAndPersisted() //20261122-10 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    HmiProject single;
    HmiPage popup;
    MainWindow window;
    MainWindow reopened;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> items;
    QString path;
    QString pageId;
    QString error;
    bool numberIsEditable = false;
    bool duplicateRejected = false;

    QVERIFY(directory.isValid());
    popup.id = QStringLiteral("popup-number-test");
    popup.kind = QStringLiteral("window");
    popup.name = QStringLiteral("对照窗口");
    popup.width = 240;
    popup.height = 180;
    QVERIFY2(project.addPage(popup, &error), qPrintable(error));
    path = directory.filePath(QStringLiteral("numbered-page.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    items = tree->findItems(QStringLiteral("画面"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(items.size(), 1);
    QVERIFY(triggerSettingsTreeAction(tree, items.first(), QStringLiteral("新建画面"), [&]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QDialogButtonBox *buttons;
        QSpinBox *number;

        if (dialog == nullptr)
        {
            return;
        }
        buttons = dialog->findChild<QDialogButtonBox *>();
        number = dialog->findChild<QSpinBox *>(QStringLiteral("newPageNumberSpin"));
        if (buttons == nullptr || number == nullptr)
        {
            dialog->reject();
            return;
        }
        numberIsEditable = !number->isReadOnly() && number->minimum() == 0 && number->maximum() == 999;
        number->setValue(0);
        QTimer::singleShot(0, dialog, []() { //20261123-13 add: Acknowledge the original editor's duplicate-number message.
            QMessageBox *warning = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());

            if (warning != nullptr)
            {
                warning->accept();
            }
        });
        buttons->button(QDialogButtonBox::Ok)->click();
        duplicateRejected = dialog->isVisible();
        if (duplicateRejected)
        {
            number->setValue(999);
            buttons->button(QDialogButtonBox::Ok)->click();
        }
        if (dialog->isVisible())
        {
            dialog->reject();
        }
    }));
    QVERIFY2(numberIsEditable, "Ordinary page numbers must be selectable from 0 to 999.");
    QVERIFY2(duplicateRejected, "A new ordinary page must reject the first page's effective number 0.");
    pageId = window.currentEditorProject().activePageId;
    QVERIFY(pageId != project.activePageId);
    QCOMPARE(tree->findItems(QStringLiteral("001:对照窗口"), Qt::MatchExactly | Qt::MatchRecursive).size(), 1);
    QCOMPARE(tree->findItems(QStringLiteral("999:画面3"), Qt::MatchExactly | Qt::MatchRecursive).size(), 1);
    QVERIFY(window.activatePageId(project.activePageId));
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);

    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    QCOMPARE(reopened.currentEditorProject().activePageId, project.activePageId);
    tree = reopened.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    QCOMPARE(tree->findItems(QStringLiteral("000:画面1"), Qt::MatchExactly | Qt::MatchRecursive).size(), 1);
    items = tree->findItems(QStringLiteral("999:画面3"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(items.size(), 1);
    tree->itemDoubleClicked(items.first(), 0);
    QCOMPARE(reopened.currentEditorProject().activePageId, pageId);
    single = reopened.currentEditorProject();
    single.startupPageId = pageId;
    QVERIFY2(single.removePage(project.activePageId, &error), qPrintable(error));
    QVERIFY2(single.removePage(popup.id, &error), qPrintable(error));
    QVERIFY2(!single.validateSinglePageBackend(&error), "An explicit nonzero number must not compile as page zero.");
    QVERIFY(error.contains(QStringLiteral("编号")));
}

/******************************************************************************
 * Function Name   : addingProtocolKeepsExistingProtocolsOnThePort
 * Description     : Add a second COM2 instance from the context menu and reload both entries.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void Stage2WorkflowTests::addingProtocolKeepsExistingProtocolsOnThePort() //20261122-7 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    PlcConnection connection;
    MainWindow window;
    MainWindow reopened;
    QString error;
    QString path;
    QStringList fields;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> ports;

    QVERIFY(directory.isValid());
    project.targetModel = QStringLiteral("AMZ070W01R");
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.id = QStringLiteral("com2");
    connection.channel = PlcChannel::Com2;
    connection.station = QStringLiteral("1");
    project.connections.append(connection);
    path = directory.filePath(QStringLiteral("new-protocol.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    ports = tree->findItems(QStringLiteral("COM2"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(ports.size(), 1);
    QVERIFY(triggerSettingsTreeAction(tree, ports.first(), QStringLiteral("增加协议"), [&]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QComboBox *series;
        QSpinBox *station;
        QDialogButtonBox *buttons;
        if (dialog == nullptr)
        {
            return;
        }
        series = dialog->findChild<QComboBox *>(QStringLiteral("combo_protocol"));
        station = dialog->findChild<QSpinBox *>(QStringLiteral("spin_stationIndex"));
        buttons = dialog->findChild<QDialogButtonBox *>();
        if (series == nullptr || station == nullptr || buttons == nullptr)
        {
            dialog->reject();
            return;
        }
        fields = {series->currentText(), QString::number(station->value())};
        buttons->button(QDialogButtonBox::Ok)->click();
    }));
    QCOMPARE(fields, (QStringList{QStringLiteral("Mitsubishi_Fx3U"), QStringLiteral("0")}));
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(reopened.openProjectPath(path, &error), qPrintable(error));
    tree = reopened.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    ports = tree->findItems(QStringLiteral("COM2"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(ports.size(), 1);
    QCOMPARE(ports.first()->childCount(), 2);
    QCOMPARE(ports.first()->child(0)->text(0), QStringLiteral("Modbus_RTU"));
    QCOMPARE(ports.first()->child(1)->text(0), QStringLiteral("Mitsubishi_Fx3U"));
    QVERIFY(ports.first()->child(0)->data(0, Qt::UserRole + 1) != ports.first()->child(1)->data(0, Qt::UserRole + 1));
}

/******************************************************************************
 * Function Name   : deletingProtocolProtectsProjectReferences
 * Description     : Reject deletion of a referenced protocol and persist removal of its unreferenced neighbor.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.22
 ******************************************************************************/
void Stage2WorkflowTests::deletingProtocolProtectsProjectReferences() //20261122-7 add
{
    QTemporaryDir directory;
    HmiProject project = workflowProject();
    HmiProject reloaded;
    HmiReference reference;
    PlcConnection connection;
    MainWindow window;
    QString error;
    QString path;
    QString warning;
    QTreeWidget *tree;
    QList<QTreeWidgetItem *> ports;

    QVERIFY(directory.isValid());
    project.targetModel = QStringLiteral("AMZ070W01R");
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.id = QStringLiteral("com2");
    connection.channel = PlcChannel::Com2;
    connection.station = QStringLiteral("1");
    project.connections.append(connection);
    connection.id = QStringLiteral("spare-com2");
    connection.displayName = QStringLiteral("PLC-SPARE");
    project.connections.append(connection);
    reference.id = QStringLiteral("system-route");
    reference.ownerKind = QStringLiteral("project");
    reference.ownerId = project.projectId;
    reference.field = QStringLiteral("route");
    reference.targetKind = QStringLiteral("connection");
    reference.targetId = QStringLiteral("com2");
    project.references.append(reference);
    path = directory.filePath(QStringLiteral("delete-protocol.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    tree = window.findChild<QTreeWidget *>(QStringLiteral("SystemSettingsTree"));
    QVERIFY(tree != nullptr);
    ports = tree->findItems(QStringLiteral("COM2"), Qt::MatchExactly | Qt::MatchRecursive);
    QCOMPARE(ports.size(), 1);
    QCOMPARE(ports.first()->childCount(), 2);
    QVERIFY(triggerSettingsTreeAction(tree, ports.first()->child(0), QStringLiteral("删除"), [&]() {
        QMessageBox *message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (message != nullptr)
        {
            warning = message->windowTitle();
            message->accept();
        }
    }));
    QCOMPARE(warning, QStringLiteral("协议仍被使用"));
    QCOMPARE(ports.first()->childCount(), 2);
    QVERIFY(!window.m_dirty);
    QVERIFY(triggerSettingsTreeAction(tree, ports.first()->child(1), QStringLiteral("删除")));
    QVERIFY(window.m_dirty);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY(!window.m_dirty);
    QVERIFY2(HmiProject::load(path, &reloaded, &error), qPrintable(error));
    QCOMPARE(reloaded.connections.size(), 1);
    QCOMPARE(reloaded.connections.first().id, QStringLiteral("com2"));
    QCOMPARE(reloaded.references.first().targetId, QStringLiteral("com2"));
}

/******************************************************************************
 * Function Name   : recentFilesAreBoundedAndIsolated
 * Description     : Keep ten exact recent paths while moving an existing entry to the front without duplication.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2WorkflowTests::recentFilesAreBoundedAndIsolated() //20261110-7 add
{
    MainWindow window;
    QTemporaryDir directory;
    QSettings settings(QStringLiteral("KinSealStudioLite"), QStringLiteral("Editor"));
    QString path;
    QStringList paths;
    int index;
    settings.clear();
    for (index = 0; index < 12; ++index) {
        window.updateRecentProjects(directory.filePath(QStringLiteral("project%1.ksp").arg(index)));
    }
    path = directory.filePath(QStringLiteral("project5.ksp"));
    window.updateRecentProjects(path);
    paths = settings.value(QStringLiteral("recentProjects")).toStringList();
    QCOMPARE(paths.size(), 10);
    QCOMPARE(paths.first(), path);
    QCOMPARE(paths.count(path), 1);
}

/******************************************************************************
 * Function Name   : nativeShellResourcesAndNavigation
 * Description     : Check retained icons, native shell geometry, recent-file routing and live status clearing.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.11
 ******************************************************************************/
void Stage2WorkflowTests::nativeShellResourcesAndNavigation() //20261111-1 add
{
    MainWindow window;
    QToolBar *bar;
    QScrollArea *palette;
    QLineEdit *field;
    QTemporaryDir files;
    QMenu *fileMenu;
    QAction *action;
    bool recentFound;
    QString captures;

    window.setProject(workflowProject(), QString()); window.resize(1353, 708); window.show();
    QTest::qWait(30);
    QCOMPARE(window.menuBar()->actions().size(), 8);
    QCOMPARE(window.findChildren<QToolBar *>().size(), 7);
    bar = window.findChild<QToolBar *>(QStringLiteral("standardToolBar"));
    QVERIFY(bar != nullptr); QCOMPARE(bar->geometry(), QRect(0, 19, 198, 34));
    bar = window.findChild<QToolBar *>(QStringLiteral("toolBar_Control"));
    QCOMPARE(bar->geometry().y(), 95); QCOMPARE(bar->height(), 34);
    QCOMPARE(window.findChild<QDockWidget *>(QStringLiteral("ProjectTreeDock"))->width(), 242);
    QCOMPARE(window.findChild<QDockWidget *>(QStringLiteral("ToolPropertyDock"))->width(), 337);
    QCOMPARE(window.statusBar()->height(), 25);
    QCOMPARE(window.m_systemSettingsTree->mapTo(&window, QPoint()), QPoint(20, 188));
    QCOMPARE(window.m_rightTabs->mapTo(&window, QPoint()).y(), 149);
    QCOMPARE(window.m_rightTabs->tabText(2), QStringLiteral("基本控件"));
    QVERIFY(!window.m_rightTabs->isTabVisible(1));
    palette = window.findChild<QScrollArea *>(QStringLiteral("NativeControlPalette"));
    QCOMPARE(palette->findChildren<QToolButton *>().size(), 31);
    palette = window.findChild<QScrollArea *>(QStringLiteral("NativeBasicPalette"));
    QCOMPARE(palette->findChildren<QToolButton *>().size(), 10);
    QVERIFY(!window.m_commands.value(QStringLiteral("action_H"))->icon().isNull());
    QVERIFY(!window.m_commands.value(QStringLiteral("action_h"))->icon().isNull());
    QVERIFY(window.m_commands.value(QStringLiteral("action_H"))->icon().pixmap(24).toImage() !=
            window.m_commands.value(QStringLiteral("action_h"))->icon().pixmap(24).toImage());
    QVERIFY(!window.windowIcon().isNull());
    window.m_commands.value(QStringLiteral("action_zoonbig"))->trigger();
    QCOMPARE(window.m_canvas->transform().m11(), 1.25);
    window.m_commands.value(QStringLiteral("action_zoonfit"))->trigger();
    QCOMPARE(window.m_canvas->transform().m11(), 1.0); QVERIFY(!window.m_dirty);
    QVERIFY(!window.m_commands.value(QStringLiteral("action_dataDisplay"))->property("nativeShellPending").toBool());
    QVERIFY(!window.m_commands.value(QStringLiteral("pictruedisplay"))->property("nativeShellPending").toBool());
    QVERIFY(files.isValid()); window.updateRecentProjects(files.filePath(QStringLiteral("visible-recent.ksp")));
    fileMenu = window.menuBar()->actions().first()->menu(); recentFound = false;
    Q_FOREACH (action, fileMenu->actions()) { if (action->menu() == window.m_recentProjectsMenu) { recentFound = true; } }
    QVERIFY(recentFound); QVERIFY(!window.m_recentProjectsMenu->actions().isEmpty());
    window.m_canvas->selectObject(QStringLiteral("workflow-text"));
    field = window.findChild<QLineEdit *>(QStringLiteral("NativeObjectX"));
    QVERIFY(field != nullptr); QCOMPARE(field->text(), QStringLiteral("244"));
    captures = qEnvironmentVariable("KINSEAL_SHELL_CAPTURE_DIR");
    if (!captures.isEmpty())
    {
        QVERIFY(window.grab().save(QDir(captures).filePath(QStringLiteral("selected-control.png"))));
        window.m_rightTabs->setCurrentIndex(2); QTest::qWait(20);
        QVERIFY(window.grab().save(QDir(captures).filePath(QStringLiteral("basic-controls.png"))));
    }
    window.closeProject();
    QCOMPARE(window.findChild<QLineEdit *>(QStringLiteral("NativeControlCount"))->text(), QString());
    QCOMPARE(window.findChild<QLineEdit *>(QStringLiteral("NativeResolution"))->text(), QString());
    QCOMPARE(field->text(), QString());
}

/******************************************************************************
 * Function Name   : nativePropertyToolbarEditsSelectedControl
 * Description     : Exercise the native gear's selected-control editor, cancel and persisted confirmation.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.24
 ******************************************************************************/
void Stage2WorkflowTests::nativePropertyToolbarEditsSelectedControl() //20261124-0 add
{
    QTemporaryDir directory;
    const QString path = directory.filePath(QStringLiteral("properties.ksp"));
    const HmiProject seed = workflowProject();
    const QString changedText = QStringLiteral("属性设置回归");
    MainWindow window;
    HmiProject reopened;
    QToolBar *toolbar;
    QToolButton *gear;
    QAction *settings;
    QString error;
    bool opened;

    QVERIFY(directory.isValid());
    QVERIFY2(seed.save(path, &error), qPrintable(error));
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    toolbar = window.findChild<QToolBar *>(QStringLiteral("toolBar_Tool"));
    settings = window.findChild<QAction *>(QStringLiteral("action_openpropertiset"));
    QVERIFY(toolbar != nullptr && settings != nullptr);
    gear = qobject_cast<QToolButton *>(toolbar->widgetForAction(settings));
    QVERIFY(gear != nullptr);
    window.m_canvas->selectObject(QStringLiteral("workflow-text"));
    QVERIFY(window.m_canvas->selectedObjectIds().contains(QStringLiteral("workflow-text")));

    const auto edit = [&](bool confirm) {
        opened = false;
        QTimer::singleShot(0, &window, [&]() {
            QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            QPlainTextEdit *input;

            if (dialog == nullptr) { return; }
            input = dialog->findChild<QPlainTextEdit *>(QStringLiteral("StaticTextInput"));
            if (input == nullptr)
            {
                dialog->reject();
                return;
            }
            opened = input->toPlainText() == QStringLiteral("TEST");
            input->setPlainText(changedText);
            if (confirm)
            {
                dialog->accept();
            }
            else
            {
                dialog->reject();
            }
        });
        gear->click();
        return opened;
    };
    QVERIFY(edit(false));
    QCOMPARE(window.currentEditorProject().objects.first().text, QStringLiteral("TEST"));
    QVERIFY(!window.m_dirty);
    QVERIFY(edit(true));
    QVERIFY(window.m_dirty);
    QCOMPARE(settings->shortcut(), QKeySequence(QStringLiteral("Ctrl+P")));
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    QCOMPARE(reopened.objects.first().text, changedText);
}

/******************************************************************************
 * Function Name   : nativePropertyToolbarEditsCurrentPage
 * Description     : Route the empty selection to current page properties while rejecting multi-selection.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.24
 ******************************************************************************/
void Stage2WorkflowTests::nativePropertyToolbarEditsCurrentPage() //20261124-0 add
{
    QTemporaryDir directory;
    const QString path = directory.filePath(QStringLiteral("page-properties.ksp"));
    const QString changedName = QStringLiteral("齿轮修改的画面");
    HmiProject seed = workflowProject();
    HmiProject reopened;
    HmiObject second = seed.objects.first();
    MainWindow window;
    QToolBar *toolbar;
    QToolButton *gear;
    QAction *settings;
    QString error;
    bool opened = false;
    bool multiOpened = false;
    bool helpClicked = false; //20261124-2 change
    bool helpOpened = false;
    bool helpClosed = false;
    bool cancelled = false;

    QVERIFY(directory.isValid());
    second.id = QStringLiteral("workflow-text-second");
    second.geometry.translate(0, 82);
    seed.objects.append(second);
    QVERIFY2(seed.save(path, &error), qPrintable(error));
    window.closeProject();
    settings = window.findChild<QAction *>(QStringLiteral("action_openpropertiset"));
    QVERIFY(settings != nullptr && !settings->isEnabled());
    QVERIFY2(window.openProjectPath(path, &error), qPrintable(error));
    window.resize(1353, 708);
    window.show();
    toolbar = window.findChild<QToolBar *>(QStringLiteral("toolBar_Tool"));
    QVERIFY(toolbar != nullptr);
    gear = qobject_cast<QToolButton *>(toolbar->widgetForAction(settings));
    QVERIFY(gear != nullptr);
    window.m_canvas->selectObjects({seed.objects.first().id, second.id});
    QVERIFY(gear->isEnabled());
    QTimer::singleShot(0, &window, [&]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());

        if (dialog != nullptr)
        {
            multiOpened = true;
            dialog->reject();
        }
    });
    gear->click();
    QCoreApplication::processEvents();
    QVERIFY(!multiOpened);
    QVERIFY(!window.m_dirty);
    window.m_canvas->selectObjects({});
    QVERIFY(gear->isEnabled());

    QTimer::singleShot(0, &window, [&]() { //20261124-1 add: Send mouse input through the dialog surface, including obscured controls.
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QLineEdit *name;
        QDialog *help;
        QTextBrowser *browser;

        if (dialog == nullptr) { return; }
        name = dialog->findChild<QLineEdit *>(QStringLiteral("newPageNameEdit"));
        if (name == nullptr)
        {
            dialog->reject();
            return;
        }
        name->setText(changedName);
        helpClicked = clickDialogButtonAtCenter(dialog, QDialogButtonBox::Help); //20261124-2 change
        help = dialog->findChild<QDialog *>(QStringLiteral("LocalHelpDialog"));
        browser = help == nullptr ? nullptr : help->findChild<QTextBrowser *>(QStringLiteral("LocalHelpBrowser"));
        helpOpened = help != nullptr && help->isVisible() && browser != nullptr &&
            browser->document()->baseUrl().fragment() == QStringLiteral("screen-properties") &&
            name->text() == changedName;
        if (help != nullptr)
        {
            helpClosed = clickDialogButtonAtCenter(help, QDialogButtonBox::Close) &&
                         !help->isVisible() && dialog->isVisible() && name->text() == changedName;
            if (!helpClosed)
            {
                help->close();
            }
        }
        cancelled = clickDialogButtonAtCenter(dialog, QDialogButtonBox::Cancel) &&
                    !dialog->isVisible() && dialog->result() == QDialog::Rejected;
        if (!cancelled)
        {
            dialog->reject();
        }
    });
    gear->click();
    QVERIFY2(helpClicked, "The visible Help button cannot be reached by mouse.");
    QVERIFY(helpOpened);
    QVERIFY(helpClosed);
    QVERIFY(cancelled);
    QCOMPARE(window.currentEditorProject().pageName, seed.pageName);
    QVERIFY(!window.m_dirty);

    QTimer::singleShot(0, &window, [&]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        QLineEdit *name;

        if (dialog == nullptr) { return; }
        name = dialog->findChild<QLineEdit *>(QStringLiteral("newPageNameEdit"));
        if (name != nullptr && dialog->windowTitle() == QStringLiteral("画面属性"))
        {
            opened = name->text() == seed.pageName;
            name->setText(changedName);
            opened = clickDialogButtonAtCenter(dialog, QDialogButtonBox::Ok) && opened;
        }
        if (dialog->isVisible()) { dialog->reject(); }
    });
    gear->click();
    QVERIFY(opened);
    window.m_commands.value(QStringLiteral("action_save"))->trigger();
    QVERIFY2(HmiProject::load(path, &reopened, &error), qPrintable(error));
    QCOMPARE(reopened.pageName, changedName);
    QCOMPARE(reopened.objects.first().text, QStringLiteral("TEST"));
    QCOMPARE(reopened.objects.size(), 2);
}

/******************************************************************************
 * Function Name   : nativeShellQuickPropertiesPreserveEditing
 * Description     : Exercise the relocated exact-geometry editor and restore the native toolbox after modal closing.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.11
 ******************************************************************************/
void Stage2WorkflowTests::nativeShellQuickPropertiesPreserveEditing() //20261111-1 add
{
    MainWindow window;
    QAction *quick;
    bool edited;

    window.setProject(workflowProject(), QString()); window.show();
    window.m_canvas->selectObject(QStringLiteral("workflow-text"));
    quick = window.findChild<QAction *>(QStringLiteral("ShellQuickProperties"));
    QVERIFY(quick != nullptr); edited = false;
    QTimer::singleShot(0, &window, [&window, &edited]() {
        QDialog *dialog;
        QPushButton *apply;
        dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (dialog == nullptr) { return; }
        QVERIFY(window.m_xSpin->isVisible());
        QVERIFY(window.m_nameEdit->isVisible());
        window.m_xSpin->setValue(123);
        apply = dialog->findChild<QPushButton *>(QStringLiteral("ApplyPropertiesButton"));
        if (apply != nullptr) { apply->click(); edited = true; }
        dialog->reject();
    });
    quick->trigger(); QVERIFY(edited);
    QCOMPARE(window.m_canvas->objects().first().geometry.x(), 123);
    QCOMPARE(window.m_rightTabs->count(), 3); QVERIFY(!window.m_rightTabs->isTabVisible(1));
    QCOMPARE(window.m_rightTabs->tabText(2), QStringLiteral("基本控件"));
}

/******************************************************************************
 * Function Name   : nativeShellPendingCommandsAreReadOnly
 * Description     : Confirm future-stage icons give an explicit message and cannot mutate the current project.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.11
 ******************************************************************************/
void Stage2WorkflowTests::nativeShellPendingCommandsAreReadOnly() //20261111-1 add
{
    MainWindow window;
    QJsonObject before;
    bool explained;

    window.setProject(workflowProject(), QString()); window.show();
    before = window.currentEditorProject().toJson(); explained = false;
    QTimer::singleShot(0, &window, [&explained]() {
        QMessageBox *message;
        message = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        if (message != nullptr) { explained = message->text().contains(QStringLiteral("后续阶段")); message->accept(); }
    });
    window.m_commands.value(QStringLiteral("action_upload"))->trigger(); //20261114-6 change: Online simulation now has a specific capability explanation; upload remains pending.
    QVERIFY(explained); QCOMPARE(window.currentEditorProject().toJson(), before);
    QVERIFY(!window.m_dirty);
}

QTEST_MAIN(Stage2WorkflowTests)
#include "stage2_workflow_tests.moc"
