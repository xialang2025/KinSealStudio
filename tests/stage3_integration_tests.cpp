#include "main_window.h"
#include "hmi_canvas.h"
#include "stage3_resources.h"
#include "control_rendering.h"
#include "static_text_renderer.h"
#include <QAction>
#include <QBuffer>
#include <QJsonDocument>
#include <QPainter>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>

//20261110-8 add: Verify transaction boundaries through real editor commands, not a duplicate implementation.
class Stage3IntegrationTests : public QObject
{
    Q_OBJECT
private:
    QTemporaryDir m_settings;
private slots:
    void initTestCase();
    void placementCancelAndCommitAreAtomic();
    void switchDocumentDiscardsPendingResources();
    void rejectedPlacementCannotLeakResources();
    void saveCancelsUncommittedGeometry();
    void editResourceOnlyIsUndoable();
    void clipboardSurvivesPageAndDocumentChanges();
    void pictureIdentityReplacementAndPersistence();
    void invalidImagesDoNotPolluteSourceCache();
    void alphaCacheRetainsRgb565Rounding();
    void pixelFontsIgnorePaintDeviceDpi();
    void pictureDialogCancellationKeepsDocument();
};

/******************************************************************************
 * Function Name   : pngForStage3
 * Description     : Encode a small odd-width transparent image to exercise resource and cache identity.
 * Input Parameters: color
 * Return Value    : Valid PNG bytes
 * Date            : 2026.11.10
 ******************************************************************************/
static QByteArray pngForStage3(const QColor &color) //20261110-8 add
{
    QImage image(17, 13, QImage::Format_ARGB32);
    QByteArray result;
    QBuffer buffer(&result);
    image.fill(color);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return result;
}

/******************************************************************************
 * Function Name   : resourceProjectForStage3
 * Description     : Build a bounded valid object with a real embedded picture binding.
 * Input Parameters: None
 * Return Value    : Complete single-page project
 * Date            : 2026.11.10
 ******************************************************************************/
static HmiProject resourceProjectForStage3() //20261110-8 add
{
    HmiProject project;
    HmiObject object;
    object.id = QStringLiteral("stage3-resource-object");
    object.type = QStringLiteral("switch");
    object.name = QStringLiteral("图片对象");
    object.address = QStringLiteral("LB0");
    object.geometry = QRect(40, 40, 81, 51);
    object.hasBitButtonProperties = true;
    project.pictures.insert(QStringLiteral("asset"), pngForStage3(QColor(32, 128, 224, 120)));
    object.bitButton.states[0].picture = QStringLiteral("asset");
    object.bitButton.states[0].picturePng = project.pictures.value(QStringLiteral("asset"));
    project.objects.append(object);
    return project;
}

/******************************************************************************
 * Function Name   : Stage3IntegrationTests::initTestCase
 * Description     : Redirect preferences and recent files away from the user's running editor.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3IntegrationTests::initTestCase() //20261110-8 add
{
    QVERIFY(m_settings.isValid());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, m_settings.path());
}

/******************************************************************************
 * Function Name   : Stage3IntegrationTests::placementCancelAndCommitAreAtomic
 * Description     : Cancel accepted resource drafts without dirty state, then place the same draft in exactly one undo transaction.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3IntegrationTests::placementCancelAndCommitAreAtomic() //20261110-8 add
{
    MainWindow window;
    HmiProject project;
    HmiProject resources;
    HmiObject object;
    QString error;
    int history;
    QSignalSpy changes(window.m_canvas, &ScreenCanvas::documentChanged);
    resources = resourceProjectForStage3();
    object = resources.objects.first();
    project = resources;
    project.objects.clear();
    project.pictures.clear();
    window.setProject(project, QString());
    history = window.m_projectHistory.size();
    QVERIFY(window.applyControlDraft(object, resources.macros, resources.pictures, false, &error));
    window.beginControlDraftPlacement(object, resources.macros, resources.pictures);
    QVERIFY(window.m_hasPendingControlResources);
    QVERIFY(window.m_project.pictures.isEmpty());
    QVERIFY(!window.m_dirty);
    QTest::keyClick(window.m_canvas, Qt::Key_Escape);
    QVERIFY(!window.m_hasPendingControlResources);
    QVERIFY(window.m_project.pictures.isEmpty());
    QVERIFY(!window.m_dirty);
    QCOMPARE(window.m_projectHistory.size(), history);
    window.beginControlDraftPlacement(object, resources.macros, resources.pictures);
    window.m_canvas->resize(900, 560);
    QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(100, 100));
    QTest::mouseMove(window.m_canvas->viewport(), window.m_canvas->mapFromScene(181, 151));
    QTest::mouseRelease(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, window.m_canvas->mapFromScene(181, 151));
    QCOMPARE(changes.size(), 1);
    QCOMPARE(window.m_project.objects.size(), 1);
    QCOMPARE(window.m_project.pictures, resources.pictures);
    QCOMPARE(window.m_projectHistory.size(), history + 1);
    QVERIFY(window.m_dirty);
    window.restoreProjectHistory(-1);
    QVERIFY(window.m_project.objects.isEmpty());
    QVERIFY(window.m_project.pictures.isEmpty());
    QVERIFY(!window.m_dirty);
    window.restoreProjectHistory(1);
    QCOMPARE(window.m_project.pictures, resources.pictures);
}

/******************************************************************************
 * Function Name   : Stage3IntegrationTests::switchDocumentDiscardsPendingResources
 * Description     : Prevent accepted unplaced images from leaking into a different document.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3IntegrationTests::switchDocumentDiscardsPendingResources() //20261110-8 add
{
    MainWindow window;
    HmiProject source;
    HmiProject empty;
    source = resourceProjectForStage3();
    window.beginControlDraftPlacement(source.objects.first(), source.macros, source.pictures);
    window.setProject(empty, QString());
    QVERIFY(!window.m_hasPendingControlResources);
    QVERIFY(!window.m_canvas->hasPendingPlacement());
    QCOMPARE(window.m_project.toJson(), empty.toJson());
}

/******************************************************************************
 * Function Name   : Stage3IntegrationTests::rejectedPlacementCannotLeakResources
 * Description     : Reach the actual canvas limit then ensure an unrelated existing-object edit cannot publish rejected draft images.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3IntegrationTests::rejectedPlacementCannotLeakResources() //20261110-8 add
{
    MainWindow window;
    HmiProject project;
    HmiProject draft;
    HmiObject object;
    int index;
    draft = resourceProjectForStage3();
    object = draft.objects.first();
    project = draft;
    project.objects.clear(); project.pictures.clear();
    object.bitButton.states[0].picture.clear(); object.bitButton.states[0].picturePng.clear();
    for (index = 0; index < 100; ++index) {
        object.id = QStringLiteral("limit-%1").arg(index);
        project.objects.append(object);
    }
    window.setProject(project, QString());
    window.beginControlDraftPlacement(draft.objects.first(), draft.macros, draft.pictures);
    QVERIFY(!window.m_hasPendingControlResources);
    QVERIFY(!window.m_canvas->hasPendingPlacement());
    window.m_canvas->selectObject(object.id);
    object.geometry.translate(1, 0);
    window.m_canvas->updateSelectedObject(object);
    QVERIFY(window.m_project.pictures.isEmpty());
    QCOMPARE(window.m_project.objects.size(), 100);
    QVERIFY(window.m_dirty);
}

/******************************************************************************
 * Function Name   : Stage3IntegrationTests::saveCancelsUncommittedGeometry
 * Description     : Resolve an active drag before capturing a saved checkpoint so Escape cannot leave an incorrect clean document.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3IntegrationTests::saveCancelsUncommittedGeometry() //20261110-8 add
{
    MainWindow window;
    HmiProject source;
    HmiProject saved;
    QTemporaryDir directory;
    QString error;
    QPoint press;
    QPoint move;
    source = resourceProjectForStage3();
    window.setProject(source, QString());
    window.m_canvas->resize(900, 560);
    press = window.m_canvas->mapFromScene(source.objects.first().geometry.center());
    move = press + QPoint(30, 20);
    QTest::mousePress(window.m_canvas->viewport(), Qt::LeftButton, Qt::NoModifier, press);
    QTest::mouseMove(window.m_canvas->viewport(), move);
    QVERIFY(window.m_canvas->hasActiveInteraction());
    QCOMPARE(window.currentEditorProject().objects.first().geometry, source.objects.first().geometry);
    QVERIFY(window.writeProject(directory.filePath(QStringLiteral("saved.ksp"))));
    QVERIFY(!window.m_canvas->hasActiveInteraction());
    QVERIFY(HmiProject::load(directory.filePath(QStringLiteral("saved.ksp")), &saved, &error));
    QCOMPARE(saved.objects.first().geometry, source.objects.first().geometry);
    QTest::keyClick(window.m_canvas, Qt::Key_Escape);
    QVERIFY(!window.m_dirty);
    QCOMPARE(window.m_canvas->objects().first().geometry, source.objects.first().geometry);
}

/******************************************************************************
 * Function Name   : Stage3IntegrationTests::editResourceOnlyIsUndoable
 * Description     : Preserve resource-only property changes when the object itself remains identical.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3IntegrationTests::editResourceOnlyIsUndoable() //20261110-8 add
{
    MainWindow window;
    HmiProject source;
    QMap<QString, QByteArray> pictures;
    QString error;
    source = resourceProjectForStage3();
    window.setProject(source, QString());
    pictures = source.pictures;
    pictures.insert(QStringLiteral("unused"), pngForStage3(Qt::red));
    QVERIFY(window.applyControlDraft(source.objects.first(), source.macros, pictures, true, &error));
    QCOMPARE(window.m_projectHistory.size(), 2);
    QVERIFY(window.m_project.pictures.contains(QStringLiteral("unused")));
    window.restoreProjectHistory(-1);
    QCOMPARE(window.m_project.pictures, source.pictures);
    QVERIFY(!window.m_dirty);
}

/******************************************************************************
 * Function Name   : Stage3IntegrationTests::clipboardSurvivesPageAndDocumentChanges
 * Description     : Exercise host copy, cross-page paste, undo, and a new-document paste with complete image resources.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3IntegrationTests::clipboardSurvivesPageAndDocumentChanges() //20261110-8 add
{
    MainWindow window;
    HmiProject project;
    HmiProject target;
    HmiPage page;
    QString error;
    QString originalId;
    project = resourceProjectForStage3();
    page.id = QStringLiteral("stage3-second");
    page.name = QStringLiteral("第二页");
    QVERIFY2(project.addPage(page, &error), qPrintable(error));
    originalId = project.objects.first().id;
    window.setProject(project, QString());
    window.m_canvas->selectObject(originalId);
    window.dispatchEditCommand(QStringLiteral("copy"));
    QVERIFY(!window.m_dirty);
    QVERIFY(window.activatePageId(page.id));
    window.dispatchEditCommand(QStringLiteral("paste"));
    QCOMPARE(window.m_project.objects.size(), 1);
    QVERIFY(window.m_project.objects.first().id != originalId);
    QCOMPARE(window.m_project.objects.first().geometry, QRect(56, 56, 81, 51));
    QCOMPARE(window.m_canvas->selectedObjectIds().size(), 1);
    window.restoreProjectHistory(-1);
    QVERIFY(window.activatePageId(page.id));
    QVERIFY(window.m_project.objects.isEmpty());
    window.setProject(target, QString());
    QVERIFY(window.m_commands.value(QStringLiteral("action_paste"))->isEnabled());
    window.dispatchEditCommand(QStringLiteral("paste"));
    QCOMPARE(window.m_project.objects.size(), 1);
    QCOMPARE(window.m_project.pictures, project.pictures);
}

/******************************************************************************
 * Function Name   : Stage3IntegrationTests::pictureIdentityReplacementAndPersistence
 * Description     : Replace proven bindings across pages, preserve detached same-name bytes and reject referenced deletion atomically.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3IntegrationTests::pictureIdentityReplacementAndPersistence() //20261110-8 add
{
    HmiProject source;
    HmiProject candidate;
    HmiProject restored;
    HmiPage page;
    HmiObject detached;
    QTemporaryDir directory;
    QString error;
    QByteArray replacement;
    QJsonObject before;
    source = resourceProjectForStage3();
    detached = source.objects.first();
    detached.id = QStringLiteral("detached");
    detached.bitButton.states[0].picturePng = pngForStage3(Qt::green);
    page.id = QStringLiteral("hidden"); page.name = QStringLiteral("隐藏页"); page.objects.append(detached);
    QVERIFY(source.addPage(page, &error));
    QCOMPARE(stage3PictureUsage(source, QStringLiteral("asset")).boundStates, 1);
    QCOMPARE(stage3PictureUsage(source, QStringLiteral("asset")).detachedStates, 1);
    replacement = pngForStage3(Qt::blue);
    QVERIFY(replaceStage3Picture(source, QStringLiteral("asset"), replacement, &candidate, &error));
    QCOMPARE(candidate.objects.first().bitButton.states[0].picturePng, replacement);
    QCOMPARE(candidate.inactivePages.first().objects.first().bitButton.states[0].picturePng, detached.bitButton.states[0].picturePng);
    QVERIFY(renameStage3Picture(candidate, QStringLiteral("asset"), QStringLiteral("renamed"), &candidate, &error));
    QCOMPARE(candidate.objects.first().bitButton.states[0].picture, QStringLiteral("renamed"));
    QCOMPARE(candidate.inactivePages.first().objects.first().bitButton.states[0].picture, QStringLiteral("asset"));
    before = candidate.toJson();
    QVERIFY(!deleteStage3Picture(candidate, QStringLiteral("renamed"), &candidate, &error));
    QCOMPARE(candidate.toJson(), before);
    QVERIFY(!replaceStage3Picture(candidate, QStringLiteral("renamed"), QByteArrayLiteral("broken"), &candidate, &error));
    QCOMPARE(candidate.toJson(), before);
    QVERIFY(candidate.save(directory.filePath(QStringLiteral("resources.ksp")), &error));
    QVERIFY(HmiProject::load(directory.filePath(QStringLiteral("resources.ksp")), &restored, &error));
    QCOMPARE(restored.toJson(), before);
    QVERIFY(importStage3Picture(candidate, QStringLiteral("unused"), pngForStage3(Qt::yellow), &candidate, &error));
    QVERIFY(deleteStage3Picture(candidate, QStringLiteral("unused"), &candidate, &error));
    QCOMPARE(candidate.toJson(), before);
}

/******************************************************************************
 * Function Name   : Stage3IntegrationTests::invalidImagesDoNotPolluteSourceCache
 * Description     : Verify alpha, copy-on-write isolation and content invalidation independently of pool display names.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3IntegrationTests::invalidImagesDoNotPolluteSourceCache() //20261110-8 add
{
    QByteArray first;
    QByteArray second;
    QImage decoded;
    QString error;
    first = pngForStage3(QColor(100, 120, 140, 127));
    second = pngForStage3(Qt::red);
    decoded = decodedControlPng(first, &error);
    QCOMPARE(decoded.size(), QSize(17, 13));
    QCOMPARE(qAlpha(decoded.pixel(1, 1)), 127);
    decoded.fill(Qt::black);
    QVERIFY(decodedControlPng(first).pixel(1, 1) != decoded.pixel(1, 1));
    QVERIFY(decodedControlPng(second).pixel(1, 1) != decodedControlPng(first).pixel(1, 1));
    QVERIFY(decodedControlPng(first.left(28), &error).isNull());
    QVERIFY(!error.isEmpty());
    QVERIFY(!decodedControlPng(first, &error).isNull());
    QVERIFY(error.isEmpty());
}

/******************************************************************************
 * Function Name   : Stage3IntegrationTests::alphaCacheRetainsRgb565Rounding
 * Description     : Preserve the previous PNG reader's RGB565 blending result instead of quantizing source channels early.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3IntegrationTests::alphaCacheRetainsRgb565Rounding() //20261110-8 add
{
    QByteArray png;
    QImage previous(121, 65, QImage::Format_RGB16);
    QImage cached(121, 65, QImage::Format_RGB16);
    QPainter painter;
    png = pngForStage3(QColor(40, 160, 220, 140));
    previous.fill(QColor(197, 223, 255));
    cached.fill(QColor(197, 223, 255));
    painter.begin(&previous);
    painter.drawImage(previous.rect(), QImage::fromData(png, "PNG"));
    painter.end();
    painter.begin(&cached);
    painter.drawImage(cached.rect(), decodedControlPng(png));
    painter.end();
    QCOMPARE(cached, previous);
}

/******************************************************************************
 * Function Name   : Stage3IntegrationTests::pixelFontsIgnorePaintDeviceDpi
 * Description     : Render identical page text to different-DPI paint devices without changing persisted glyph dimensions.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3IntegrationTests::pixelFontsIgnorePaintDeviceDpi() //20261110-8 add
{
    StaticTextProperties properties;
    QImage first(201, 61, QImage::Format_ARGB32);
    QImage second(201, 61, QImage::Format_ARGB32);
    QPainter painter;
    properties.fontSize = 23;
    first.setDotsPerMeterX(3780); first.setDotsPerMeterY(3780);
    second.setDotsPerMeterX(7560); second.setDotsPerMeterY(7560);
    first.fill(Qt::white); second.fill(Qt::white);
    painter.begin(&first);
    paintStaticText(&painter, first.rect(), QStringLiteral("DPI 123"), properties);
    painter.end();
    painter.begin(&second);
    paintStaticText(&painter, second.rect(), QStringLiteral("DPI 123"), properties);
    painter.end();
    QCOMPARE(first, second);
    QCOMPARE(controlPixelFont(properties).pixelSize(), 23);
    QCOMPARE(controlFontDiagnostics(properties).value(QStringLiteral("requestedPixelSize")).toInt(), 23);
    properties.fontSize = 17.5;
    first.fill(Qt::white); second.fill(Qt::white);
    painter.begin(&first);
    paintStaticText(&painter, first.rect(), QStringLiteral("DPI 17.5"), properties);
    painter.end();
    painter.begin(&second);
    paintStaticText(&painter, second.rect(), QStringLiteral("DPI 17.5"), properties);
    painter.end();
    QCOMPARE(first, second);
    QCOMPARE(controlPixelFont(properties, 96).pointSizeF(), 13.125);
    QCOMPARE(controlPixelFont(properties, 192).pointSizeF(), 6.5625);
    qInfo().noquote() << QJsonDocument(controlFontDiagnostics(properties)).toJson(QJsonDocument::Compact);
}

/******************************************************************************
 * Function Name   : Stage3IntegrationTests::pictureDialogCancellationKeepsDocument
 * Description     : Reject an image-management dialog without publishing its detached resource draft.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3IntegrationTests::pictureDialogCancellationKeepsDocument() //20261110-8 add
{
    HmiProject source;
    Stage3PicturesDialog *dialog;
    source = resourceProjectForStage3();
    dialog = new Stage3PicturesDialog(source);
    dialog->reject();
    QCOMPARE(source.toJson(), dialog->draftProject().toJson());
    delete dialog;
}

QTEST_MAIN(Stage3IntegrationTests)
#include "stage3_integration_tests.moc"
