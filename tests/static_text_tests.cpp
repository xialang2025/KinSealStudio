#include "project_model.h"
#include "static_text_dialog.h"
#include "static_text_renderer.h"
#include "hmi_canvas.h"
#include "main_window.h"
#include "independent_downloader.h"
#include "text_payload_compiler.h"
#include "bit_button_dialog.h" //20261108-15 add
#include "bit_button_renderer.h"
#include "bit_button_payload_compiler.h" //20261109-0 add
#include "native_bit_button_control.h"
#include "device_transfer_plan.h"
#include "device_address_compiler.h" //20261109-0 add
#include "static_text_payload_compiler.h" //20261109-0 add
#include "native_static_text_control.h"
#include "lzokay.hpp"
#include "kvg_library.h" //20261109-0 add
#include "hmi_property_widgets.h"
#include <QBuffer>
#include <QListWidget>
#include <QMessageBox>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QCryptographicHash>
#include <QDoubleSpinBox>
#include <QDir>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QFontComboBox> //20261109-6 add
#include <QJsonArray>
#include <QGroupBox>
#include <QGraphicsScene> //20261109-4 add
#include <QInputDialog>
#include <QJsonDocument>
#include <QLineEdit>
#include <QLabel> //20261109-1 add
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton> //20261109-0 add
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTimer>
#include <QTreeWidget> //20261109-1 add
#include <QToolButton> //20261110-0 add
#include <QScopeGuard> //20261109-1 add
#include <QtEndian> //20261109-0 add
#include <QtTest>

//20261108-14 add: These tests send Qt events offscreen and never operate the user's desktop or USB device.
class StaticTextTests : public QObject
{
    Q_OBJECT
private slots:
    void propertyRoundTrip();
    void dialogDependenciesAndDrafts();
    void translationsAndCancel();
    void rendererStylesAndCompilerGuards();
    void canvasGesturesAndHistory();
    void toolbarCreationFlow();
    void colorAndAddressTransactions();
    void nativeFontAndFillEvidence();
    void nativeFillColors();
    void languagePropertySynchronization();
    void bitDefaultsAndRoundTrip(); //20261108-15 add
    void bitStateDraftsAndCopies();
    void bitMonitorAndPermissions();
    void bitMacroAndGalleryTransactions();
    void bitCreationAndCanvas();
    void bitCompilerGuardAndRendering();
    void nativeKvgCatalogAndImport(); //20261109-0 add
    void nativeGalleryNavigationAndSearch(); //20261109-1 add
    void galleryArtworkTransparency(); //20261109-4 add
    void bitDevicePayloadAndOffsets();
    void bitSpriteOddWidthAndTransaction();
    void bitDeviceFailureAndSourceSnapshot();
    void bitFlashImagesAndNativeOrder(); //20261109-0 add
    void plcProjectRoundTripAndGuard();
    void bitStaticLayersAndTransparency(); //20261109-0 add
    void externalAddressPickerContext();
    void externalDeviceScanOracles(); //20261109-0 add
    void localDeviceAddressBounds(); //20261109-0 add
    void nativeAddressPickerInitialValue(); //20261109-0 add
    void bitDeviceLanguages(); //20261109-0 add
    void localizedStaticTextControlPayload(); //20261109-0 add
    void wordProjectAndScanMatrix(); //20261109-6 add
    void wordCreationAndCanvas();
    void wordDeviceSpritesAndTransactions();
    void buttonFontFallbackRetainsSource();
    void lampProjectAndScanMatrix(); //20261109-8 add
    void lampCreationAndCanvas();
    void lampDeviceFramesAndTransactions();
    void functionCreationAndCanvas(); //20261110-0 add
    void functionAddressScanIsolation();
    void functionDeviceImagesAndFailures();
};

/******************************************************************************
 * Function Name   : textObject
 * Description     : Produce one complete property-aware text fixture for interaction tests.
 * Input Parameters: None
 * Return Value    : Static-text object.
 * Date            : 2026.11.08
 ******************************************************************************/
static HmiObject textObject() //20261108-14 add
{
    HmiObject object;
    object.id = QStringLiteral("text-1"); object.type = QStringLiteral("text");
    object.name = QStringLiteral("测试文字"); object.text = QStringLiteral("TEXT");
    object.geometry = QRect(100, 80, 240, 90); object.hasStaticTextProperties = true;
    return object;
}

/******************************************************************************
 * Function Name   : StaticTextTests::propertyRoundTrip
 * Description     : Verify all dormant properties survive save/load and legacy files keep old rendering.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::propertyRoundTrip() //20261108-14 add
{
    HmiProject project;
    HmiProject loaded;
    HmiObject object = textObject();
    QTemporaryDir directory;
    QString error;
    QJsonObject invalid;

    object.staticText.bold = true; object.staticText.italic = true; object.staticText.underline = true;
    object.staticText.adaptive = true; object.staticText.fontSize = 56; object.staticText.spacing = 10;
    object.staticText.style = 12; object.staticText.opacity = 100;
    object.staticText.textColor = Qt::red; object.staticText.backgroundColor = Qt::blue;
    object.staticText.foregroundColor = Qt::cyan;
    object.staticText.languageTexts.insert(QStringLiteral("中文"), QStringLiteral("第一行\n第二行"));
    object.staticText.languageTexts.insert(QStringLiteral("English"), QStringLiteral("Long text"));
    object.staticText.visibilityEnabled = true; object.staticText.bitEnabled = true;
    object.staticText.levelEnabled = true; object.staticText.wordEnabled = true;
    object.staticText.bitAddress = QStringLiteral("LB17"); object.staticText.wordAddress = QStringLiteral("LW32");
    object.staticText.bitState = false; object.staticText.minimumLevel = 8;
    object.staticText.dataType = 4; object.staticText.comparison = 3;
    object.staticText.compareValue = QStringLiteral("-12.5"); object.staticText.tolerance = QStringLiteral("0.25");
    object.layer = 3; object.group = QStringLiteral("group-1");
    project.objects.append(object); project.languages.append(QStringLiteral("English"));
    project.activeLanguage = QStringLiteral("English");
    QCOMPARE(project.toJson().value(QStringLiteral("schemaVersion")).toInt(), 2);
    QVERIFY2(project.save(directory.filePath(QStringLiteral("all.ksp")), &error), qPrintable(error));
    QVERIFY2(HmiProject::load(directory.filePath(QStringLiteral("all.ksp")), &loaded, &error), qPrintable(error));
    QCOMPARE(loaded.toJson(), project.toJson());
    invalid = object.staticText.toJson(); invalid.insert(QStringLiteral("style"), 13);
    QVERIFY(!StaticTextProperties::fromJson(invalid, &object.staticText, &error));
    object.hasStaticTextProperties = false;
    QVERIFY(HmiObject::fromJson(object.toJson(), &object, &error));
    QVERIFY(!object.hasStaticTextProperties);
}

/******************************************************************************
 * Function Name   : StaticTextTests::languagePropertySynchronization
 * Description     : Prevent a stale property editor from overwriting another language after switching.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::languagePropertySynchronization() //20261108-14 add
{
    MainWindow window;
    ScreenCanvas *canvas;
    QComboBox *language;
    QPlainTextEdit *editor;
    HmiProject project;
    HmiObject object;
    bool selected;

    object = textObject();
    object.staticText.languageTexts.insert(QStringLiteral("中文"), QStringLiteral("中文内容"));
    object.staticText.languageTexts.insert(QStringLiteral("English"), QStringLiteral("English text"));
    project.objects.append(object);
    window.show(); QApplication::processEvents();
    canvas = window.findChild<ScreenCanvas *>();
    language = window.findChild<QComboBox *>(QStringLiteral("ProjectLanguage"));
    editor = window.findChild<QPlainTextEdit *>();
    QVERIFY(canvas != nullptr && language != nullptr && editor != nullptr);
    canvas->setProject(project); canvas->documentChanged(); canvas->selectObject(object.id);
    QCOMPARE(editor->toPlainText(), QStringLiteral("中文内容"));
    QTimer::singleShot(0, &window, [&window]() {
        QInputDialog *dialog;
        dialog = window.findChild<QInputDialog *>();
        if (dialog != nullptr)
        {
            dialog->setTextValue(QStringLiteral("中文,English"));
            dialog->accept();
        }
    });
    QVERIFY(QMetaObject::invokeMethod(&window, "configureLanguages", Qt::DirectConnection));
    QCOMPARE(language->count(), 2);
    language->setCurrentText(QStringLiteral("English"));
    QCOMPARE(editor->toPlainText(), QStringLiteral("English text"));
    QVERIFY(QMetaObject::invokeMethod(&window, "applySelectedProperties", Qt::DirectConnection));
    object = canvas->selectedObject(&selected);
    QVERIFY(selected);
    QCOMPARE(object.staticText.languageTexts.value(QStringLiteral("English")), QStringLiteral("English text"));
    QCOMPARE(object.staticText.languageTexts.value(QStringLiteral("中文")), QStringLiteral("中文内容"));
    language->setCurrentText(QStringLiteral("中文"));
    QCOMPARE(editor->toPlainText(), QStringLiteral("中文内容"));
    QVERIFY(window.findChild<QPushButton *>(QStringLiteral("ApplyPropertiesButton"))->isEnabled());
}

/******************************************************************************
 * Function Name   : StaticTextTests::dialogDependenciesAndDrafts
 * Description     : Exercise adaptive gating, independent access toggles and equality-only error input.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::dialogDependenciesAndDrafts() //20261108-14 add
{
    HmiObject original = textObject();
    StaticTextDialog dialog(original, {QStringLiteral("中文")});
    QCheckBox *adaptive;
    QComboBox *size;
    QComboBox *alignment;
    QComboBox *comparison;
    QGroupBox *visibility;
    QGroupBox *bit;
    QGroupBox *word;
    QLineEdit *tolerance;

    dialog.show(); QApplication::processEvents();
    dialog.grab().save(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("static-text-default.png")));
    QCOMPARE(dialog.findChild<QTabWidget *>(QStringLiteral("StaticTextTabs"))->tabText(0), QStringLiteral("外观设置"));
    QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("StaticTextStyle"))->count(), 13);
    adaptive = dialog.findChild<QCheckBox *>(QStringLiteral("StaticTextAdaptive"));
    size = dialog.findChild<QComboBox *>(QStringLiteral("StaticTextSize"));
    alignment = dialog.findChild<QComboBox *>(QStringLiteral("StaticTextAlignment"));
    QCOMPARE(size->currentText(), QStringLiteral("16"));
    adaptive->setChecked(true); QVERIFY(!size->isEnabled()); QVERIFY(!alignment->isEnabled());
    QVERIFY(dialog.findChild<QDoubleSpinBox *>(QStringLiteral("StaticTextSpacing"))->isEnabled());
    adaptive->setChecked(false); QVERIFY(size->isEnabled());
    visibility = dialog.findChild<QGroupBox *>(QStringLiteral("VisibilityEnabled"));
    bit = dialog.findChild<QGroupBox *>(QStringLiteral("VisibilityBit"));
    word = dialog.findChild<QGroupBox *>(QStringLiteral("VisibilityWord"));
    QVERIFY(!bit->isEnabled()); visibility->setChecked(true); QVERIFY(bit->isEnabled());
    bit->setChecked(true); word->setChecked(true);
    dialog.findChild<QGroupBox *>(QStringLiteral("VisibilityLevel"))->setChecked(true);
    QVERIFY(bit->isChecked() && word->isChecked());
    comparison = dialog.findChild<QComboBox *>(QStringLiteral("VisibilityComparison"));
    tolerance = dialog.findChild<QLineEdit *>(QStringLiteral("VisibilityTolerance"));
    QVERIFY(tolerance->isHidden()); comparison->setCurrentIndex(2); QVERIFY(!tolerance->isHidden());
    comparison->setCurrentIndex(3); QVERIFY(!tolerance->isHidden());
    comparison->setCurrentIndex(0); QVERIFY(tolerance->isHidden());
    dialog.findChild<QPlainTextEdit *>(QStringLiteral("StaticTextInput"))->setPlainText(QStringLiteral("测试\nAB12"));
    QCOMPARE(dialog.object().text, QStringLiteral("测试\nAB12"));
    QVERIFY(!original.staticText.bitEnabled);
    dialog.findChild<QTabWidget *>(QStringLiteral("StaticTextTabs"))->setCurrentIndex(1);
    dialog.grab().save(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("static-text-permissions.png")));
}

/******************************************************************************
 * Function Name   : StaticTextTests::translationsAndCancel
 * Description     : Keep separate language drafts and ensure rejected edits never mutate source records.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::translationsAndCancel() //20261108-14 add
{
    HmiObject original = textObject();
    StaticTextDialog dialog(original, {QStringLiteral("中文"),QStringLiteral("English")});
    QPlainTextEdit *text;
    QComboBox *language;
    HmiObject draft;

    text = dialog.findChild<QPlainTextEdit *>(QStringLiteral("StaticTextInput"));
    language = dialog.findChild<QComboBox *>(QStringLiteral("StaticTextLanguage"));
    text->setPlainText(QStringLiteral("中文草稿")); language->setCurrentIndex(1);
    text->setPlainText(QStringLiteral("English draft")); language->setCurrentIndex(0);
    QCOMPARE(text->toPlainText(), QStringLiteral("中文草稿"));
    draft = dialog.object();
    QCOMPARE(staticTextContent(draft, QStringLiteral("English")), QStringLiteral("English draft"));
    dialog.reject(); QCOMPARE(original.text, QStringLiteral("TEXT"));
    QCOMPARE(original.staticText.languageTexts.size(), 0);
}

/******************************************************************************
 * Function Name   : StaticTextTests::rendererStylesAndCompilerGuards
 * Description     : Cover every fill, alpha, font effects and fail-closed dynamic-device properties.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::rendererStylesAndCompilerGuards() //20261108-14 add
{
    HmiProject project;
    HmiObject object = textObject();
    QImage image(240, 90, QImage::Format_ARGB32);
    QPainter painter;
    QSet<QByteArray> hashes;
    QVector<QByteArray> payloads;
    QString error;
    int style;

    object.staticText.backgroundColor = Qt::blue; object.staticText.foregroundColor = Qt::cyan;
    for (style = 0; style < 13; ++style)
    {
        image.fill(Qt::yellow); object.staticText.style = style;
        painter.begin(&image); paintStaticText(&painter, image.rect(), QString(), object.staticText); painter.end();
        hashes.insert(QCryptographicHash::hash(QByteArray(reinterpret_cast<const char *>(image.constBits()), image.sizeInBytes()), QCryptographicHash::Sha256));
    }
    QCOMPARE(hashes.size(), 13);
    image.fill(Qt::yellow); object.staticText.opacity = 0;
    painter.begin(&image); paintStaticText(&painter, image.rect(), QString(), object.staticText); painter.end();
    QCOMPARE(image.pixelColor(100, 50), QColor(Qt::yellow));
    object.staticText.opacity = 255; object.staticText.bold = true; object.staticText.fontSize = 28;
    project.objects.append(object);
    QVERIFY2(buildSingleTextPayload(project, &payloads, &error), qPrintable(error));
    QCOMPARE(payloads.size(), 10);
    project.objects.first().staticText.visibilityEnabled = true;
    QVERIFY(!buildSingleTextPayload(project, &payloads, &error));
    QVERIFY(error.contains(QStringLiteral("显现权限")));
    project.objects.first().staticText.visibilityEnabled = false;
    project.languages = {QStringLiteral("English")}; project.activeLanguage = QStringLiteral("English");
    project.objects.first().staticText.languageTexts.insert(QStringLiteral("English"), QStringLiteral("ABCD"));
    QVERIFY2(buildSingleTextPayload(project, &payloads, &error), qPrintable(error));
    QCOMPARE(qUncompress(payloads.at(7)).count(QByteArray::fromHex("000000080041004200430044")), 3);
}

/******************************************************************************
 * Function Name   : StaticTextTests::canvasGesturesAndHistory
 * Description     : Verify rectangle drawing, eight-point resize, editing activation and copy/undo persistence.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::canvasGesturesAndHistory() //20261108-14 add
{
    ScreenCanvas canvas;
    HmiObject object = textObject();
    QSignalSpy edits(&canvas, &ScreenCanvas::editObjectRequested);
    QPoint start;
    QPoint end;
    bool selected;

    canvas.resize(850, 550); canvas.show(); QApplication::processEvents();
    canvas.beginStaticTextPlacement(object);
    start = canvas.mapFromScene(QPointF(100,80)); end = canvas.mapFromScene(QPointF(340,170));
    QTest::mousePress(canvas.viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QTest::mouseMove(canvas.viewport(), end); QTest::mouseRelease(canvas.viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QCOMPARE(canvas.objects().size(), 1); QCOMPARE(canvas.objects().first().geometry, QRect(100,80,240,90));
    QVERIFY(canvas.objects().first().hasStaticTextProperties);
    QTest::mouseDClick(canvas.viewport(), Qt::LeftButton, Qt::NoModifier, canvas.mapFromScene(QPointF(200,120)));
    QCOMPARE(edits.count(), 1);
    start = canvas.mapFromScene(QPointF(340,170)); end = canvas.mapFromScene(QPointF(440,220));
    QTest::mousePress(canvas.viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QTest::mouseMove(canvas.viewport(), end); QTest::mouseRelease(canvas.viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QCOMPARE(canvas.objects().first().geometry, QRect(100,80,340,140));
    canvas.executeEdit(QStringLiteral("copy")); canvas.executeEdit(QStringLiteral("paste"));
    QCOMPARE(canvas.objects().size(), 2);
    QVERIFY(canvas.selectedObject(&selected).hasStaticTextProperties); QVERIFY(selected);
    canvas.executeEdit(QStringLiteral("undo")); QCOMPARE(canvas.objects().size(), 1);
    canvas.executeEdit(QStringLiteral("redo")); QCOMPARE(canvas.objects().size(), 2);
    canvas.executeEdit(QStringLiteral("delete")); QCOMPARE(canvas.objects().size(), 2); // Redo does not retain selection.
    canvas.executeEdit(QStringLiteral("selectAll")); canvas.executeEdit(QStringLiteral("delete")); QCOMPARE(canvas.objects().size(), 0);
}

/******************************************************************************
 * Function Name   : StaticTextTests::toolbarCreationFlow
 * Description     : Confirm clicking the toolbar opens the dialog before canvas placement.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::toolbarCreationFlow() //20261108-14 add
{
    MainWindow window;
    QAction *action;
    QAction *textAction = nullptr;
    bool opened = false;

    window.show(); QApplication::processEvents();
    Q_FOREACH (action, window.findChildren<QAction *>())
    {
        if (action->objectName() == QStringLiteral("action_text")) { textAction = action; } //20261111-1 change: Native labels differ; command identity remains stable.
    }
    QVERIFY(textAction != nullptr);
    QTimer::singleShot(0, &window, [&window, &opened]() {
        StaticTextDialog *dialog;
        dialog = window.findChild<StaticTextDialog *>();
        opened = dialog != nullptr;
        if (dialog != nullptr) { dialog->reject(); }
    });
    textAction->trigger(); QVERIFY(opened);
    QCOMPARE(window.findChild<ScreenCanvas *>()->objects().size(), 0);
}

/******************************************************************************
 * Function Name   : StaticTextTests::colorAndAddressTransactions
 * Description     : Verify color/address modal acceptance and cancellation keep draft transactions isolated.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::colorAndAddressTransactions() //20261108-14 add
{
    StaticTextDialog dialog(textObject(), {QStringLiteral("中文")});
    bool found = false;

    dialog.show(); QApplication::processEvents();
    QTimer::singleShot(0, &dialog, [&dialog, &found]() {
        QColorDialog *color;
        color = dialog.findChild<QColorDialog *>(); found = color != nullptr;
        if (color != nullptr) { color->setCurrentColor(Qt::red); color->reject(); }
    });
    dialog.findChild<QPushButton *>(QStringLiteral("StaticTextColor0"))->click();
    QVERIFY(found); QCOMPARE(dialog.object().staticText.textColor, QColor(Qt::black));
    QTimer::singleShot(0, &dialog, [&dialog]() {
        QColorDialog *color;
        color = dialog.findChild<QColorDialog *>();
        if (color != nullptr) { color->setCurrentColor(Qt::red); color->accept(); }
    });
    dialog.findChild<QPushButton *>(QStringLiteral("StaticTextColor0"))->click();
    QCOMPARE(dialog.object().staticText.textColor, QColor(Qt::red));
    dialog.findChild<QTabWidget *>(QStringLiteral("StaticTextTabs"))->setCurrentIndex(1);
    dialog.findChild<QGroupBox *>(QStringLiteral("VisibilityEnabled"))->setChecked(true);
    dialog.findChild<QGroupBox *>(QStringLiteral("VisibilityBit"))->setChecked(true);
    QTimer::singleShot(0, &dialog, [&dialog]() {
        QDialog *address;
        address = dialog.findChild<QDialog *>(QStringLiteral("StaticTextAddressDialog"));
        if (address != nullptr)
        {
            address->findChild<QLineEdit *>(QStringLiteral("AddressNumber"))->setText(QStringLiteral("4096"));
            address->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        }
    });
    dialog.findChild<QPushButton *>(QStringLiteral("BitAddressPicker"))->click();
    QCOMPARE(dialog.object().staticText.bitAddress, QStringLiteral("LB4096"));
    QTimer::singleShot(0, &dialog, [&dialog]() {
        QDialog *address;
        address = dialog.findChild<QDialog *>(QStringLiteral("StaticTextAddressDialog"));
        if (address != nullptr)
        {
            address->findChild<QLineEdit *>(QStringLiteral("AddressNumber"))->setText(QStringLiteral("1"));
            address->reject();
        }
    });
    dialog.findChild<QPushButton *>(QStringLiteral("BitAddressPicker"))->click();
    QCOMPARE(dialog.object().staticText.bitAddress, QStringLiteral("LB4096"));
}

/******************************************************************************
 * Function Name   : StaticTextTests::nativeFontAndFillEvidence
 * Description     : Regress the independently decoded native bold raster and export all fill comparisons.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::nativeFontAndFillEvidence() //20261108-14 add
{
    HmiProject project;
    HmiObject object = textObject();
    QByteArray frame;
    QString error;
    QImage image(100, 100, QImage::Format_RGB32);
    QPainter painter;
    QDir directory(QCoreApplication::applicationDirPath());
    int index;

    if (QGuiApplication::platformName() != QStringLiteral("windows"))
    {
        QSKIP("Real Windows font evidence runs only on the private Windows desktop.");
    }
    object.text = QStringLiteral("TEST"); object.geometry = QRect(244,112,150,50);
    object.staticText.bold = true; project.objects.append(object);
    QVERIFY2(renderSingleTextFrame(project, QStringLiteral("微软雅黑"), 12, false, &frame, &error), qPrintable(error));
    QCOMPARE(QCryptographicHash::hash(frame, QCryptographicHash::Sha256).toHex(),
             QByteArray("2bdc57203e07323fb8de6b8bb5b0d156bba82669a549799f2413672edf1c177e"));
    QVERIFY(directory.mkpath(QStringLiteral("style-comparisons")));
    object.staticText = StaticTextProperties();
    for (index = 0; index < 13; ++index)
    {
        image.fill(QColor(240,240,240)); object.staticText.style = index;
        painter.begin(&image); paintStaticText(&painter, image.rect(), QString(), object.staticText); painter.end();
        QVERIFY(image.save(directory.filePath(QStringLiteral("style-comparisons/style-%1.png").arg(index,2,10,QLatin1Char('0')))));
    }
}

/******************************************************************************
 * Function Name   : StaticTextTests::nativeFillColors
 * Description     : Check sampled native preview colors with the measured two-channel-step tolerance.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::nativeFillColors() //20261108-14 add
{
    QFile fixture(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath() + QStringLiteral("/fixtures/native-static-text-fills.json"));
    QJsonObject data;
    QJsonArray cases;
    QJsonArray points;
    QJsonArray colors;
    QJsonArray sample;
    QImage image(100, 100, QImage::Format_RGB32);
    QPainter painter;
    QColor actual;
    StaticTextProperties properties;
    int index;
    int point;
    int tolerance;

    QVERIFY(fixture.open(QIODevice::ReadOnly)); data = QJsonDocument::fromJson(fixture.readAll()).object();
    cases = data.value(QStringLiteral("cases")).toArray(); points = data.value(QStringLiteral("points")).toArray();
    tolerance = data.value(QStringLiteral("tolerance_per_channel")).toInt();
    QCOMPARE(cases.size(), 13);
    for (index = 0; index < cases.size(); ++index)
    {
        properties.style = cases.at(index).toObject().value(QStringLiteral("style")).toInt();
        colors = cases.at(index).toObject().value(QStringLiteral("colors")).toArray();
        image.fill(QColor(240,240,240)); painter.begin(&image);
        paintStaticText(&painter, image.rect(), QString(), properties); painter.end();
        for (point = 0; point < points.size(); ++point)
        {
            sample = points.at(point).toArray(); actual = image.pixelColor(sample.at(0).toInt(), sample.at(1).toInt());
            sample = colors.at(point).toArray();
            QVERIFY(qAbs(actual.red() - sample.at(0).toInt()) <= tolerance);
            QVERIFY(qAbs(actual.green() - sample.at(1).toInt()) <= tolerance);
            QVERIFY(qAbs(actual.blue() - sample.at(2).toInt()) <= tolerance);
        }
    }
}

/******************************************************************************
 * Function Name   : buttonObject
 * Description     : Create a complete bit-button fixture without involving an installation or USB.
 * Input Parameters: None
 * Return Value    : Default button object.
 * Date            : 2026.11.08
 ******************************************************************************/
static HmiObject buttonObject() //20261108-15 add
{
    HmiObject object;
    object.id = QStringLiteral("bit-1"); object.type = QStringLiteral("switch");
    object.name = QStringLiteral("位按钮1"); object.text = QStringLiteral("OFF");
    object.address = QStringLiteral("LB0"); object.geometry = QRect(244, 112, 120, 60); object.hasBitButtonProperties = true;
    return object;
}

/******************************************************************************
 * Function Name   : StaticTextTests::bitDefaultsAndRoundTrip
 * Description     : Cover native defaults, full-state persistence and the old-editor schema guard.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::bitDefaultsAndRoundTrip() //20261108-15 add
{
    HmiObject object = buttonObject();
    HmiProject project;
    HmiProject loaded;
    QString error;
    QTemporaryDir directory;
    QJsonObject invalid;
    QImage image(10, 10, QImage::Format_ARGB32);
    QByteArray png;
    QBuffer buffer(&png);

    QCOMPARE(object.bitButton.operation, 3); QCOMPARE(object.bitButton.bitIndex, 15);
    QVERIFY(object.bitButton.indicator && object.bitButton.dualMonitor && object.bitButton.monitorSame);
    QCOMPARE(object.bitButton.visibility.minimumLevel, 1); QCOMPARE(object.bitButton.touch.minimumLevel, 2);
    QCOMPARE(object.bitButton.states[0].text.languageTexts.value(QStringLiteral("中文")), QStringLiteral("OFF"));
    QCOMPARE(object.bitButton.states[1].picture, QStringLiteral("switch_ON.kvg"));
    image.fill(Qt::cyan); QVERIFY(buffer.open(QIODevice::WriteOnly)); QVERIFY(image.save(&buffer, "PNG"));
    object.bitButton.operation = 2; object.bitButton.macroEnabled = true; object.bitButton.macroName = QStringLiteral("Main");
    object.bitButton.monitorSame = false; object.bitButton.monitorType = 1;
    object.bitButton.monitorWordAddress = QStringLiteral("LW32"); object.bitButton.monitorBitAddress = QStringLiteral("LB17");
    object.bitButton.states[0].text.bold = true; object.bitButton.states[1].text.italic = true;
    object.bitButton.states[1].text.languageTexts.insert(QStringLiteral("English"), QStringLiteral("START\nNOW"));
    object.bitButton.states[0].picturePng = png; object.bitButton.states[0].picture = QStringLiteral("custom.png");
    object.bitButton.states[0].imageMode = 1; object.bitButton.states[1].opacity = 76;
    object.bitButton.states[1].transparent = true; object.bitButton.states[1].effect3d = true;
    object.bitButton.states[1].flash = true; object.bitButton.states[0].sameLanguage = true;
    object.bitButton.touch.visibilityEnabled = true; object.bitButton.touch.wordEnabled = true;
    object.bitButton.touch.dataType = 4; object.bitButton.touch.compareValue = QStringLiteral("-12.5");
    object.bitButton.touch.comparison = 2; object.bitButton.touch.tolerance = QStringLiteral("0.5");
    object.bitButton.notice = true; object.bitButton.noticeBitAddress = QStringLiteral("LB27");
    object.bitButton.noticeWordAddress = QStringLiteral("LW31"); object.bitButton.pressTimeEnabled = true; object.bitButton.pressTime = 100;
    project.objects.append(object); project.languages.append(QStringLiteral("English"));
    project.macros.insert(QStringLiteral("Main"), QStringLiteral("void Macro_Entry() {}")); project.pictures.insert(QStringLiteral("custom.png"), png);
    QCOMPARE(project.toJson().value(QStringLiteral("schemaVersion")).toInt(), 3);
    QVERIFY2(project.save(directory.filePath(QStringLiteral("button.ksp")), &error), qPrintable(error));
    QVERIFY2(HmiProject::load(directory.filePath(QStringLiteral("button.ksp")), &loaded, &error), qPrintable(error));
    QCOMPARE(loaded.toJson(), project.toJson());
    invalid = object.bitButton.toJson(); invalid.insert(QStringLiteral("states"), QJsonArray());
    QVERIFY(!BitButtonProperties::fromJson(invalid, &object.bitButton, &error));
    object.type = QStringLiteral("text"); invalid = object.toJson(); invalid.insert(QStringLiteral("bitButton"), project.objects.first().bitButton.toJson());
    QVERIFY(!HmiObject::fromJson(invalid, &object, &error));
}

/******************************************************************************
 * Function Name   : StaticTextTests::bitStateDraftsAndCopies
 * Description     : Ensure state/language switching and independent copy commands cannot leak shape or text.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::bitStateDraftsAndCopies() //20261108-15 add
{
    HmiProject project;
    BitButtonDialog *dialog;
    QSpinBox *state;
    QComboBox *language;
    QPlainTextEdit *text;
    HmiObject result;
    project.languages.append(QStringLiteral("English")); dialog = new BitButtonDialog(buttonObject(), project);
    dialog->show(); dialog->findChild<QTabWidget *>()->setCurrentIndex(2); QApplication::processEvents();
    state = dialog->findChild<QSpinBox *>(QStringLiteral("BitState")); language = dialog->findChild<QComboBox *>(QStringLiteral("BitLanguage"));
    text = dialog->findChild<QPlainTextEdit *>(QStringLiteral("BitText"));
    QCOMPARE(text->toPlainText(), QStringLiteral("OFF"));
    text->setPlainText(QStringLiteral("关闭\n中文")); dialog->findChild<QPushButton *>(QStringLiteral("BitBold"))->setChecked(true);
    language->setCurrentText(QStringLiteral("English")); text->setPlainText(QStringLiteral("STOP"));
    state->setValue(1); text->setPlainText(QStringLiteral("START"));
    language->setCurrentText(QStringLiteral("中文")); text->setPlainText(QStringLiteral("开启"));
    state->setValue(0); QCOMPARE(text->toPlainText(), QStringLiteral("关闭\n中文"));
    dialog->findChild<QCheckBox *>(QStringLiteral("BitSameLanguage"))->setChecked(true);
    text->setPlainText(QStringLiteral("共享")); language->setCurrentText(QStringLiteral("English")); QCOMPARE(text->toPlainText(), QStringLiteral("共享"));
    dialog->findChild<QCheckBox *>(QStringLiteral("BitSameLanguage"))->setChecked(false);
    QCOMPARE(text->toPlainText(), QStringLiteral("STOP"));
    language->setCurrentText(QStringLiteral("中文")); QCOMPARE(text->toPlainText(), QStringLiteral("关闭\n中文"));
    dialog->findChild<QPushButton *>(QStringLiteral("BitCopyText"))->click();
    QCOMPARE(text->toPlainText(), QStringLiteral("开启"));
    QCOMPARE(dialog->object().bitButton.states[0].picture, QStringLiteral("switch_OFF.kvg"));
    dialog->findChild<QPushButton *>(QStringLiteral("BitCopyShape"))->click();
    QCOMPARE(dialog->object().bitButton.states[0].picture, QStringLiteral("switch_ON.kvg"));
    QCOMPARE(text->toPlainText(), QStringLiteral("开启"));
    dialog->findChild<QPushButton *>(QStringLiteral("BitNoImage"))->click();
    QVERIFY(!dialog->findChild<QLineEdit *>(QStringLiteral("BitPicture"))->isEnabled());
    QVERIFY(!dialog->object().bitButton.states[0].useImage); QVERIFY(dialog->object().bitButton.states[1].useImage);
    result = dialog->object();
    QCOMPARE(result.bitButton.states[1].text.languageTexts.value(QStringLiteral("English")), QStringLiteral("START"));
    dialog->grab().save(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("bit-button-appearance.png")));
    dialog->reject(); delete dialog;
}

/******************************************************************************
 * Function Name   : StaticTextTests::bitMonitorAndPermissions
 * Description     : Exercise every documented monitor gate and both independent permission hierarchies.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::bitMonitorAndPermissions() //20261108-15 add
{
    HmiProject project;
    BitButtonDialog dialog(buttonObject(), project);
    QCheckBox *same;
    QLineEdit *address;
    QComboBox *type;
    QSpinBox *bit;
    QComboBox *comparison;
    QTabWidget *tabs;

    dialog.show(); tabs = dialog.findChild<QTabWidget *>(); tabs->setCurrentIndex(1); QApplication::processEvents();
    same = dialog.findChild<QCheckBox *>(QStringLiteral("BitMonitorSame")); address = dialog.findChild<QLineEdit *>(QStringLiteral("BitMonitorAddress"));
    type = dialog.findChild<QComboBox *>(QStringLiteral("BitMonitorType")); bit = dialog.findChild<QSpinBox *>(QStringLiteral("BitMonitorIndex"));
    QVERIFY(same->isChecked()); QVERIFY(!address->isEnabled()); QVERIFY(!type->isEnabled()); QVERIFY(!bit->isEnabled());
    same->setChecked(false); QVERIFY(address->isEnabled()); QVERIFY(type->isEnabled()); QVERIFY(!bit->isEnabled());
    address->setText(QStringLiteral("LB17")); type->setCurrentIndex(1); QCOMPARE(address->text(), QStringLiteral("LW0")); QVERIFY(bit->isEnabled());
    address->setText(QStringLiteral("LW32")); same->setChecked(true);
    QCOMPARE(type->currentIndex(), 0);
    dialog.findChild<QLineEdit *>(QStringLiteral("BitOperatingAddress"))->setText(QStringLiteral("LB23")); QCOMPARE(address->text(), QStringLiteral("LB23"));
    same->setChecked(false); QCOMPARE(address->text(), QStringLiteral("LW32")); type->setCurrentIndex(0); QCOMPARE(address->text(), QStringLiteral("LB17"));
    dialog.findChild<QCheckBox *>(QStringLiteral("BitIndicator"))->setChecked(false); QVERIFY(!type->isEnabled());
    dialog.findChild<QCheckBox *>(QStringLiteral("BitIndicator"))->setChecked(true); QVERIFY(type->isEnabled());
    tabs->setCurrentIndex(3); QApplication::processEvents();
    QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("BitTouchMinimum"))->currentText(), QStringLiteral("2"));
    QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("BitVisibilityMinimum"))->count(), 8);
    dialog.findChild<HmiPermissionEditor *>(QStringLiteral("BitTouch"))->setChecked(true);
    dialog.findChild<QGroupBox *>(QStringLiteral("BitTouchWord"))->setChecked(true);
    comparison = dialog.findChild<QComboBox *>(QStringLiteral("BitTouchComparison")); comparison->setCurrentIndex(2);
    QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("BitTouchTolerance"))->isVisible()); comparison->setCurrentIndex(0);
    QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("BitTouchTolerance"))->isVisible());
    dialog.findChild<QGroupBox *>(QStringLiteral("BitNotice"))->setChecked(true);
    QVERIFY(dialog.findChild<QCheckBox *>(QStringLiteral("BitNoticeBit"))->isEnabled()); //20261109-0 change
    QVERIFY(dialog.findChild<QCheckBox *>(QStringLiteral("BitNoticeWord"))->isEnabled());
    QVERIFY(!dialog.findChild<QLineEdit *>(QStringLiteral("BitNoticeBitAddress"))->isEnabled());
    dialog.findChild<QCheckBox *>(QStringLiteral("BitNoticeBit"))->setChecked(true);
    QVERIFY(dialog.findChild<QLineEdit *>(QStringLiteral("BitNoticeBitAddress"))->isEnabled());
    dialog.findChild<QLineEdit *>(QStringLiteral("BitNoticeBitAddress"))->setText(QStringLiteral("LB12"));
    dialog.findChild<QRadioButton *>(QStringLiteral("BitNoticeBitOff"))->setChecked(true);
    dialog.findChild<QCheckBox *>(QStringLiteral("BitNoticeWord"))->setChecked(true);
    dialog.findChild<QLineEdit *>(QStringLiteral("BitNoticeWordAddress"))->setText(QStringLiteral("LW8"));
    dialog.findChild<QComboBox *>(QStringLiteral("BitNoticeWordType"))->setCurrentIndex(2);
    dialog.findChild<QLineEdit *>(QStringLiteral("BitNoticeWordValue"))->setText(QStringLiteral("-7"));
    QCOMPARE(dialog.object().bitButton.noticeBitAddress, QStringLiteral("LB12"));
    QVERIFY(!dialog.object().bitButton.noticeBitValue);
    QCOMPARE(dialog.object().bitButton.noticeWordValue, QStringLiteral("-7"));
    QVERIFY(!dialog.findChild<QSpinBox *>(QStringLiteral("BitPressTime"))->isVisible());
    dialog.findChild<QCheckBox *>(QStringLiteral("BitPressEnabled"))->setChecked(true);
    QVERIFY(dialog.findChild<QSpinBox *>(QStringLiteral("BitPressTime"))->isVisible());
    QCOMPARE(dialog.findChild<QSpinBox *>(QStringLiteral("BitPressTime"))->value(), 255);
    dialog.grab().save(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("bit-button-permissions.png")));
}

/******************************************************************************
 * Function Name   : StaticTextTests::bitMacroAndGalleryTransactions
 * Description     : Check modal commit/cancel for macro sources and built-in/project gallery selection.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::bitMacroAndGalleryTransactions() //20261109-3 change
{
    HmiProject project;
    HmiObject original = buttonObject();
    BitButtonDialog dialog(original, project);
    QString before;
    QImage picture(12, 12, QImage::Format_ARGB32);
    QByteArray png;
    QBuffer imageBuffer(&png);
    QTabWidget *tabs;

    dialog.show(); QApplication::processEvents();
    QCOMPARE(dialog.findChild<QTabWidget *>()->count(), 4);
    QCOMPARE(dialog.findChild<QComboBox *>(QStringLiteral("BitOperation"))->currentText(), QStringLiteral("交替"));
    dialog.grab().save(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("bit-button-general.png")));
    tabs = dialog.findChild<QTabWidget *>();
    tabs->setCurrentIndex(1); QApplication::processEvents();
    dialog.grab().save(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("bit-button-indicator.png")));
    tabs->setCurrentIndex(2); QApplication::processEvents();
    dialog.grab().save(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("bit-button-appearance-default.png")));
    tabs->setCurrentIndex(3); QApplication::processEvents();
    dialog.grab().save(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("bit-button-permissions-default.png")));
    tabs->setCurrentIndex(0);
    dialog.findChild<QGroupBox *>(QStringLiteral("BitMacro"))->setChecked(true);
    QTimer::singleShot(0, &dialog, [&dialog]() {
        QDialog *editor;
        editor = dialog.findChild<QDialog *>(QStringLiteral("BitMacroEditor"));
        if (editor != nullptr)
        {
            editor->findChild<QLineEdit *>(QStringLiteral("MacroName"))->setText(QStringLiteral("TestMacro"));
            editor->findChild<QPlainTextEdit *>(QStringLiteral("MacroSource"))->setPlainText(QStringLiteral("void Macro_Entry() {}"));
            editor->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Save)->click();
        }
    });
    dialog.findChild<QPushButton *>(QStringLiteral("BitWriteMacro"))->click();
    QVERIFY(dialog.macros().contains(QStringLiteral("TestMacro"))); QVERIFY(project.macros.isEmpty());
    dialog.findChild<QTabWidget *>()->setCurrentIndex(2);
    before = dialog.object().bitButton.states[0].picture;
    QTimer::singleShot(0, &dialog, [&dialog]() {
        QDialog *gallery;
        gallery = dialog.findChild<QDialog *>(QStringLiteral("BitGallery"));
        if (gallery != nullptr) { gallery->findChild<QListWidget *>()->setCurrentRow(1); gallery->reject(); }
    });
    dialog.findChild<QPushButton *>(QStringLiteral("BitSystemLibrary"))->click(); QCOMPARE(dialog.object().bitButton.states[0].picture, before);
    QTimer::singleShot(0, &dialog, [&dialog]() {
        QDialog *gallery;
        QListWidget *items;
        QListWidgetItem *selected;
        QLineEdit *search;
        int index;

        gallery = dialog.findChild<QDialog *>(QStringLiteral("BitGallery"));
        if (gallery != nullptr) //20261109-0 change: Allow the native catalog's deferred first thumbnail batch.
        {
            QTimer::singleShot(2000, gallery, &QDialog::reject);
            items = gallery->findChild<QListWidget *>();
            search = gallery->findChild<QLineEdit *>(QStringLiteral("GallerySearch"));
            QVERIFY(search != nullptr); search->setText(QStringLiteral("switch_ON.kvg"));
            QTRY_COMPARE_WITH_TIMEOUT(items->count(), 1, 1500); //20261109-3 change: Legacy buttons remain searchable outside the native visible leaves.
            selected = nullptr;
            for (index = 0; index < items->count(); ++index)
            {
                if (items->item(index)->data(Qt::UserRole + 1).toString() == QStringLiteral("switch_ON.kvg"))
                {
                    selected = items->item(index);
                    break;
                }
            }
            QVERIFY(selected != nullptr);
            items->setCurrentItem(selected);
            gallery->accept();
        }
    });
    dialog.findChild<QPushButton *>(QStringLiteral("BitSystemLibrary"))->click();
    QCOMPARE(dialog.object().bitButton.states[0].picture, QStringLiteral("switch_ON.kvg"));
    dialog.reject(); QCOMPARE(original.bitButton.states[0].picture, QStringLiteral("switch_OFF.kvg")); QVERIFY(project.macros.isEmpty());
    picture.fill(Qt::green); QVERIFY(imageBuffer.open(QIODevice::WriteOnly)); QVERIFY(picture.save(&imageBuffer, "PNG"));
    project.pictures.insert(QStringLiteral("green.png"), png);
    {
        BitButtonDialog embedded(original, project);
        embedded.show(); embedded.findChild<QTabWidget *>()->setCurrentIndex(2);
        QTimer::singleShot(0, &embedded, [&embedded]() {
            QDialog *gallery;
            QListWidget *items;
            gallery = embedded.findChild<QDialog *>(QStringLiteral("BitGallery"));
            if (gallery != nullptr) //20261109-0 change: Project images use the same deferred thumbnail loader.
            {
                QTimer::singleShot(2000, gallery, &QDialog::reject);
                items = gallery->findChild<QListWidget *>();
                QTRY_VERIFY_WITH_TIMEOUT(items->count() == 1, 1500);
                items->setCurrentRow(0);
                gallery->accept();
            }
        });
        embedded.findChild<QPushButton *>(QStringLiteral("BitProjectLibrary"))->click();
        QCOMPARE(embedded.object().bitButton.states[0].picturePng, png);
        QCOMPARE(embedded.object().bitButton.states[0].picture, QStringLiteral("green.png"));
    }
}

/******************************************************************************
 * Function Name   : StaticTextTests::bitCreationAndCanvas
 * Description     : Exercise the real action, drag rectangle, double-click cancellation, state preview and history.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::bitCreationAndCanvas() //20261108-15 add
{
    MainWindow window;
    QAction *action;
    QAction *buttonAction = nullptr;
    ScreenCanvas *canvas;
    HmiProject empty;
    QPoint start;
    QPoint end;
    HmiObject object;
    bool opened = false;
    bool selected;
    QPlainTextEdit *editor;
    window.show(); QApplication::processEvents(); canvas = window.findChild<ScreenCanvas *>(); canvas->setProject(empty);
    Q_FOREACH (action, window.findChildren<QAction *>()) { if (action->text() == QStringLiteral("位按钮")) { buttonAction = action; } }
    QVERIFY(buttonAction != nullptr);
    QTimer::singleShot(0, &window, [&window, &opened]() {
        BitButtonDialog *dialog;
        dialog = window.findChild<BitButtonDialog *>(); opened = dialog != nullptr;
        if (dialog != nullptr)
        {
            dialog->findChild<QComboBox *>(QStringLiteral("BitOperation"))->setCurrentIndex(2);
            dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        }
    });
    buttonAction->trigger(); QVERIFY(opened); QCOMPARE(canvas->objects().size(), 0);
    start = canvas->mapFromScene(QPointF(100, 80)); end = canvas->mapFromScene(QPointF(240, 160));
    QTest::mousePress(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QTest::mouseMove(canvas->viewport(), end); QTest::mouseRelease(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QCOMPARE(canvas->objects().size(), 1); object = canvas->objects().first();
    QCOMPARE(object.geometry, QRect(100, 80, 140, 80)); QCOMPARE(object.bitButton.operation, 2); QVERIFY(object.hasBitButtonProperties);
    QTimer::singleShot(0, &window, [&window]() {
        BitButtonDialog *dialog;
        dialog = window.findChild<BitButtonDialog *>();
        if (dialog != nullptr) { dialog->findChild<QComboBox *>(QStringLiteral("BitOperation"))->setCurrentIndex(0); dialog->reject(); }
    });
    QTest::mouseDClick(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, canvas->mapFromScene(QPointF(150, 110)));
    QCOMPARE(canvas->objects().first().bitButton.operation, 2);
    canvas->executeEdit(QStringLiteral("simulate1")); QCOMPARE(canvas->objects().first().toJson(), object.toJson());
    canvas->executeEdit(QStringLiteral("copy")); canvas->executeEdit(QStringLiteral("paste")); QCOMPARE(canvas->objects().size(), 2);
    QCOMPARE(canvas->selectedObject(&selected).bitButton.toJson(), object.bitButton.toJson()); QVERIFY(selected);
    canvas->executeEdit(QStringLiteral("undo")); QCOMPARE(canvas->objects().size(), 1);
    canvas->executeEdit(QStringLiteral("redo")); QCOMPARE(canvas->objects().size(), 2);
    //20261109-0 change: Device compilation is exercised offline below; UI tests must never launch a USB worker.
    canvas->selectObject(object.id); object.bitButton.states[0].sameLanguage = true; object.bitButton.states[0].sharedText = QStringLiteral("共享文本");
    canvas->updateSelectedObject(object); editor = window.findChild<QPlainTextEdit *>();
    QCOMPARE(editor->toPlainText(), QStringLiteral("共享文本")); editor->setPlainText(QStringLiteral("共享修改"));
    QVERIFY(QMetaObject::invokeMethod(&window, "applySelectedProperties", Qt::DirectConnection));
    object = canvas->selectedObject(&selected); QVERIFY(selected);
    QCOMPARE(object.bitButton.states[0].sharedText, QStringLiteral("共享修改"));
    QCOMPARE(object.bitButton.states[0].text.languageTexts.value(QStringLiteral("中文")), QStringLiteral("OFF"));
}

/******************************************************************************
 * Function Name   : StaticTextTests::bitCompilerGuardAndRendering
 * Description     : Verify independent two-state rendering and refuse unsupported dynamic device data before USB.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void StaticTextTests::bitCompilerGuardAndRendering() //20261108-15 add
{
    HmiProject project;
    QTemporaryDir directory;
    QString error;
    QImage off(120, 60, QImage::Format_ARGB32);
    QImage on(120, 60, QImage::Format_ARGB32);
    QPainter painter;
    project.objects.append(buttonObject()); off.fill(Qt::white); on.fill(Qt::white);
    painter.begin(&off); paintBitButton(&painter, off.rect(), project.objects.first().bitButton.states[0], QStringLiteral("中文")); painter.end();
    painter.begin(&on); paintBitButton(&painter, on.rect(), project.objects.first().bitButton.states[1], QStringLiteral("中文")); painter.end();
    QVERIFY(off != on);
    QVERIFY(!exportSingleTextPayload(project, directory.filePath(QStringLiteral("unsupported")), &error));
    QVERIFY(error.contains(QStringLiteral("位按钮")));
    QVERIFY(!QDir(directory.filePath(QStringLiteral("unsupported"))).exists());
}

/******************************************************************************
 * Function Name   : StaticTextTests::nativeKvgCatalogAndImport
 * Description     : Check recovered category identities, legacy pictures and standalone KVG import.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::nativeKvgCatalogAndImport() //20261109-0 add
{
    QList<KvgLibraryCategory> categories;
    KvgLibraryCategory category;
    QSet<QString> pictures;
    QString picture;
    QString error;
    QImage off;
    QImage on;
    QFile source;
    QByteArray bytes;

    categories = kvgLibraryCategories();
    QCOMPARE(categories.size(), 75); //20261109-1 change: Include original loose startup backgrounds.
    Q_FOREACH (category, categories)
    {
        Q_FOREACH (picture, category.pictures)
        {
            QVERIFY2(!pictures.contains(picture), qPrintable(picture));
            QVERIFY2(!kvgPictureResource(picture).isEmpty(), qPrintable(picture)); //20261109-1 add
            pictures.insert(picture);
        }
    }
    QCOMPARE(pictures.size(), 5647); //20261109-1 change: All 5634 KVGs plus twelve original PNGs and one SVG.
    off = kvgPictureImage(QStringLiteral("switch_OFF.kvg"), QSize(120, 60));
    on = kvgPictureImage(QStringLiteral("switch_ON.kvg"), QSize(120, 60));
    QVERIFY(!off.isNull());
    QVERIFY(!on.isNull());
    QVERIFY(off != on);
    source.setFileName(kvgPictureResource(QStringLiteral("switch_OFF.kvg")));
    QVERIFY(source.open(QIODevice::ReadOnly));
    bytes = source.readAll();
    QVERIFY2(!importKvgImage(bytes, &error).isNull(), qPrintable(error));
    QVERIFY(importKvgImage(QByteArray("<svg xmlns='http://www.w3.org/2000/svg' width='12' height='12'>"
        "<image href='file:///missing-test-image.png'/></svg>"), &error).isNull());
    QVERIFY(importKvgImage(QByteArray("<svg xmlns='http://www.w3.org/2000/svg' width='8192' height='8192'/>"),
                           &error).isNull());
    QVERIFY(importKvgImage(QByteArray("not-an-svg"), &error).isNull());
}

/******************************************************************************
 * Function Name   : StaticTextTests::nativeGalleryNavigationAndSearch
 * Description     : Verify the native ordered tree, complete parent scopes, paging and cross-branch search without USB.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::nativeGalleryNavigationAndSearch() //20261109-3 change
{
    HmiProject project;
    HmiObject object;
    BitButtonDialog dialog(buttonObject(), project);
    BitButtonSpriteSet sprites;
    QList<KvgLibraryCategory> categories;
    KvgLibraryCategory category;
    HmiObject rememberedObject;
    BitButtonDialog *rememberedDialog;
    QTimer watchdog;
    QString rememberedPicture;
    QString rememberedScope;
    int rememberedPage;
    QString error;
    bool visited;

    visited = false;
    watchdog.setSingleShot(true);
    QObject::connect(&watchdog, &QTimer::timeout, &dialog, [&dialog]() {
        QDialog *child;
        Q_FOREACH (child, dialog.findChildren<QDialog *>()) { child->reject(); }
    }); //20261109-1 add: A renamed/missing test target must fail within a bounded modal lifetime.
    QTimer::singleShot(0, &dialog, [&]() {
        QDialog *gallery = nullptr;
        QTreeWidget *tree;
        QTreeWidgetItem *root;
        QTreeWidgetItem *group;
        QTreeWidget treePreview;
        QSet<QString> uniquePictures;
        QStringList branchPictures;
        QStringList expectedSystem;
        QStringList expectedExtended;
        QString picture;
        QListWidget *list;
        QLineEdit *search;
        QLabel *count;
        QPushButton *next;
        QPushButton *previous;
        QString firstPicture;
        int i;
        auto finish = qScopeGuard([&gallery]() { if (gallery != nullptr) { gallery->reject(); } });

        gallery = dialog.findChild<QDialog *>(QStringLiteral("BitGallery"));
        QVERIFY(gallery != nullptr); visited = true;
        tree = gallery->findChild<QTreeWidget *>(QStringLiteral("GalleryCategories"));
        list = gallery->findChild<QListWidget *>(QStringLiteral("GalleryItems"));
        search = gallery->findChild<QLineEdit *>(QStringLiteral("GallerySearch"));
        count = gallery->findChild<QLabel *>(QStringLiteral("GalleryScopeCount"));
        next = gallery->findChild<QPushButton *>(QStringLiteral("GalleryNextPage"));
        previous = gallery->findChild<QPushButton *>(QStringLiteral("GalleryPreviousPage"));
        QVERIFY(tree != nullptr && list != nullptr && search != nullptr && count != nullptr && next != nullptr && previous != nullptr);
        //20261109-3 add: These literal lists follow the supplied native screenshots rather than catalog sorting.
        expectedSystem = QStringLiteral("推钮|立体按钮|金属按钮|开关按钮|数字按钮|圆形按钮指示灯|3D打印机|UPS电源|咖啡机|洗衣机|炒菜机|温控器|热水器|空气净化器|灯|空调|电机|罐体|阀门|警示标|排风机|搅拌机|工业管道|刻度背景|其他|仪表|电脑及外设").split(QLatin1Char('|'));
        expectedExtended = QStringLiteral("按钮|推钮|旋转钮|开关闸|箭头|泵|油罐|锅炉|流量表|电机补充|纸张生产|食物生产|加工设备|成品设备|控制设备|化学设备|化学制剂|材质容器|冷却处理|加热处理|水与污水处理|传感器|操作界面|电线与电缆|器皿|建筑物|采矿|自然界|基本图形|形状|软管|通风管道|暖气设备|电气设备|简易传送机|传送机传送带|传送机杂项|仪器标志|一般制造设备|符号|空调|空调2|常用按键|制冷与空调控制器").split(QLatin1Char('|'));
        QCOMPARE(tree->topLevelItemCount(), 2);
        root = tree->topLevelItem(0); group = tree->topLevelItem(1);
        QVERIFY(root != nullptr && group != nullptr);
        QCOMPARE(root->text(0), QStringLiteral("系统图库"));
        QCOMPARE(group->text(0), QStringLiteral("扩展图库"));
        QCOMPARE(root->childCount(), expectedSystem.size());
        QCOMPARE(group->childCount(), expectedExtended.size());
        for (i = 0; i < root->childCount(); ++i) {
            QCOMPARE(root->child(i)->text(0), expectedSystem.at(i));
            QCOMPARE(root->child(i)->childCount(), 0);
            QCOMPARE(root->child(i)->data(0, Qt::UserRole + 2).toString(), QStringLiteral("extern/") + expectedSystem.at(i));
        }
        for (i = 0; i < group->childCount(); ++i) {
            QCOMPARE(group->child(i)->text(0), expectedExtended.at(i));
            QCOMPARE(group->child(i)->childCount(), 0);
            QCOMPARE(group->child(i)->data(0, Qt::UserRole + 2).toString(), QStringLiteral("externadd/") + expectedExtended.at(i));
        }
        QCOMPARE(root->data(0, Qt::UserRole).toStringList().size(), 3518);
        QCOMPARE(group->data(0, Qt::UserRole).toStringList().size(), 2129);
        branchPictures = root->data(0, Qt::UserRole).toStringList() + group->data(0, Qt::UserRole).toStringList();
        Q_FOREACH (picture, branchPictures) {
            QVERIFY2(!uniquePictures.contains(picture), qPrintable(picture)); uniquePictures.insert(picture);
        }
        QCOMPARE(uniquePictures.size(), 5647);
        treePreview.setFont(tree->font()); treePreview.setHeaderHidden(true);
        treePreview.addTopLevelItem(root->clone()); treePreview.addTopLevelItem(group->clone());
        treePreview.expandAll(); treePreview.resize(270, 1800);
        QVERIFY(treePreview.grab().save(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("../artifacts/system-gallery-native-directories.png"))));
        QCOMPARE(tree->currentItem(), root);
        QTRY_COMPARE(list->count(), 120);
        QVERIFY(count->text().contains(QStringLiteral("3518")));
        firstPicture = list->item(0)->data(Qt::UserRole + 1).toString();
        QCOMPARE(firstPicture, root->child(0)->data(0, Qt::UserRole).toStringList().first());
        QVERIFY(!previous->isEnabled()); QVERIFY(next->isEnabled()); next->click();
        QTRY_COMPARE(list->count(), 120);
        QVERIFY(list->item(0)->data(Qt::UserRole + 1).toString() != firstPicture);
        previous->click(); QTRY_COMPARE(list->count(), 120);
        QCOMPARE(list->item(0)->data(Qt::UserRole + 1).toString(), firstPicture);
        for (i = 0; i < 29; ++i) { QVERIFY(next->isEnabled()); next->click(); }
        QVERIFY(!next->isEnabled());
        QTRY_COMPARE(list->count(), 38);
        QCOMPARE(list->item(37)->data(Qt::UserRole + 1).toString(), root->data(0, Qt::UserRole).toStringList().last());
        QVERIFY(previous->isEnabled());
        for (i = 0; i < 29; ++i) { QVERIFY(previous->isEnabled()); previous->click(); }
        QVERIFY(!previous->isEnabled());
        QTRY_COMPARE(list->count(), 120);
        QCOMPARE(list->item(0)->data(Qt::UserRole + 1).toString(), firstPicture);
        QVERIFY(gallery->grab().save(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("../artifacts/system-gallery-native-tree.png"))));
        tree->setCurrentItem(group);
        QTRY_COMPARE(list->count(), 120);
        QVERIFY(count->text().contains(QStringLiteral("2129")));
        for (i = 0; i < 17; ++i) { QVERIFY(next->isEnabled()); next->click(); }
        QVERIFY(!next->isEnabled()); //20261109-3 change: Bounded loops cannot starve the modal watchdog on a paging regression.
        QTRY_COMPARE(list->count(), 89);
        QCOMPARE(list->item(88)->data(Qt::UserRole + 1).toString(), group->data(0, Qt::UserRole).toStringList().last());
        tree->setCurrentItem(root); QTRY_COMPARE(list->count(), 120);
        // Searching from the system root must still reach extended-only SVGs and unlisted loose PNGs.
        search->setText(QStringLiteral("空调-48.svg")); QTRY_COMPARE(list->count(), 1);
        QVERIFY(list->item(0)->data(Qt::UserRole + 1).toString().endsWith(QStringLiteral("空调-48.svg")));
        QVERIFY(!list->item(0)->icon().isNull());
        search->setText(QStringLiteral("this-image-does-not-exist-20261109")); QTRY_COMPARE(list->count(), 0);
        QVERIFY(!next->isEnabled()); QVERIFY(!previous->isEnabled());
        tree->setCurrentItem(root->child(0)); QTRY_COMPARE(list->count(), 120); QVERIFY(search->text().isEmpty());
        tree->setCurrentItem(root); QTRY_COMPARE(list->count(), 120);
        search->setText(QStringLiteral("启动背景")); QTRY_COMPARE(list->count(), 12);
        QVERIFY(gallery->grab().save(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("../artifacts/system-gallery-backgrounds.png"))));
        search->setText(QStringLiteral("HMIStudiostart_800X480")); QTRY_COMPARE(list->count(), 1);
        QVERIFY(!list->item(0)->icon().isNull()); list->setCurrentRow(0);
        finish.dismiss();
        gallery->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
    });
    watchdog.start(15000);
    dialog.findChild<QPushButton *>(QStringLiteral("BitSystemLibrary"))->click();
    watchdog.stop();
    QVERIFY(visited);
    object = dialog.object();
    QCOMPARE(object.bitButton.states[0].picture, QStringLiteral(":/system-gallery/extern/HMIStudiostart_800X480.png"));
    QVERIFY(object.bitButton.states[0].picturePng.isEmpty());
    QVERIFY2(buildBitButtonSprites(object, QStringLiteral("中文"), {0}, &sprites, &error), qPrintable(error));
    QVERIFY(!sprites.pixelData.isEmpty());
    QTimer::singleShot(0, &dialog, [&]() {
        QDialog *gallery = nullptr;
        QLineEdit *search;
        QListWidget *list;
        auto finish = qScopeGuard([&gallery]() { if (gallery != nullptr) { gallery->reject(); } });

        gallery = dialog.findChild<QDialog *>(QStringLiteral("BitGallery")); QVERIFY(gallery != nullptr);
        search = gallery->findChild<QLineEdit *>(QStringLiteral("GallerySearch"));
        list = gallery->findChild<QListWidget *>(QStringLiteral("GalleryItems"));
        QVERIFY(search != nullptr && list != nullptr);
        search->setText(QStringLiteral("空调-48.svg")); QTRY_COMPARE(list->count(), 1);
        list->setCurrentRow(0);
    }); //20261109-1 add: A highlighted search result must not change the draft when cancelled.
    watchdog.start(15000);
    dialog.findChild<QPushButton *>(QStringLiteral("BitSystemLibrary"))->click();
    watchdog.stop();
    QCOMPARE(dialog.object().toJson(), object.toJson());
    QCOMPARE(dialog.pictures(), project.pictures);
    categories = kvgLibraryCategories();
    Q_FOREACH (category, categories) { //20261109-3 change: Reopen visible and unlisted categories in both source branches.
        if (category.id != QStringLiteral("extern/3D打印机") && category.id != QStringLiteral("externadd/符号") &&
            category.id != QStringLiteral("extern/空调2") && category.id != QStringLiteral("externadd/新风机")) { continue; }
        rememberedScope = category.name; rememberedPage = 2;
        rememberedPicture = category.pictures.at(qMin(120, category.pictures.size() - 1));
        if (category.id == QStringLiteral("extern/空调2")) { rememberedScope = QStringLiteral("系统图库"); rememberedPage = 30; }
        if (category.id == QStringLiteral("externadd/新风机")) { rememberedScope = QStringLiteral("扩展图库"); rememberedPage = 18; }
        rememberedObject = buttonObject(); rememberedObject.bitButton.states[0].picture = rememberedPicture;
        rememberedDialog = new BitButtonDialog(rememberedObject, project, &dialog);
        QTimer::singleShot(0, rememberedDialog, [&]() {
            QDialog *gallery = nullptr;
            QListWidget *list;
            QLabel *page;
            QTreeWidget *tree;
            auto finish = qScopeGuard([&gallery]() { if (gallery != nullptr) { gallery->reject(); } });

            gallery = rememberedDialog->findChild<QDialog *>(QStringLiteral("BitGallery")); QVERIFY(gallery != nullptr);
            list = gallery->findChild<QListWidget *>(QStringLiteral("GalleryItems"));
            page = gallery->findChild<QLabel *>(QStringLiteral("GalleryPage"));
            tree = gallery->findChild<QTreeWidget *>(QStringLiteral("GalleryCategories"));
            QVERIFY(list != nullptr && page != nullptr && tree != nullptr);
            QTRY_VERIFY(list->currentItem() != nullptr);
            QCOMPARE(list->currentItem()->data(Qt::UserRole + 1).toString(), rememberedPicture);
            QVERIFY(tree->currentItem() != nullptr); QCOMPARE(tree->currentItem()->text(0), rememberedScope);
            QVERIFY(page->text().startsWith(QStringLiteral("第%1 /").arg(rememberedPage)));
        });
        watchdog.start(15000);
        rememberedDialog->findChild<QPushButton *>(QStringLiteral("BitSystemLibrary"))->click();
        watchdog.stop(); delete rememberedDialog;
    }
}

/******************************************************************************
 * Function Name   : StaticTextTests::galleryArtworkTransparency
 * Description     : Preserve real gallery alpha through the shared canvas renderer and RGB565 device sprites.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::galleryArtworkTransparency() //20261109-4 add
{
    HmiObject object;
    HmiObject canvasObject;
    HmiProject project;
    HmiObject lowerButton;
    BitButtonState state;
    ScreenCanvas canvas;
    QImage artwork;
    QImage image;
    QImage underlay;
    QImage custom;
    QBuffer buffer;
    QByteArray png;
    QPainter painter;
    BitButtonSpriteSet sprites;
    QByteArray decoded;
    QVector<QByteArray> blocks;
    QString error;
    QColor pageColor(197, 223, 255);
    std::size_t decodedLength;
    u32 compressed;
    int mode;

    object = buttonObject(); object.geometry = QRect(0, 0, 128, 128);
    object.bitButton.states[0].picture = QStringLiteral(":/system-gallery/extern/圆形按钮指示灯/圆形按钮指示灯_02-01.kvg");
    artwork = kvgPictureImage(object.bitButton.states[0].picture, object.geometry.size());
    QVERIFY(!artwork.isNull()); QCOMPARE(artwork.pixelColor(0, 0).alpha(), 0);
    image = QImage(228, 228, QImage::Format_ARGB32_Premultiplied); image.fill(pageColor);
    painter.begin(&image); paintBitButton(&painter, QRect(50, 50, 128, 128), object.bitButton.states[0], QStringLiteral("中文")); painter.end();
    QVERIFY(image.save(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("../artifacts/gallery-alpha-actual.png"))));
    QCOMPARE(image.pixelColor(50, 50), pageColor);
    QVERIFY(image.pixelColor(114, 114) != pageColor);
    canvasObject = object; canvasObject.geometry.moveTopLeft(QPoint(50, 50)); project.objects = {canvasObject};
    canvas.setProject(project);
    painter.begin(&image); canvas.scene()->render(&painter, image.rect(), image.rect()); painter.end();
    QCOMPARE(image.pixelColor(50, 50), QColor(197, 222, 255)); //20261111-1 change: The native shell restores the original default page fill.
    QVERIFY(image.save(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("../artifacts/gallery-alpha-canvas.png"))));
    underlay = QImage(128, 128, QImage::Format_RGB16); underlay.fill(pageColor);
    QVERIFY2(buildBitButtonSprites(object, QStringLiteral("中文"), {0}, &sprites, &error, underlay), qPrintable(error));
    compressed = qFromLittleEndian<u32>(sprites.lengthRecords.constData() + 8);
    decoded = QByteArray(128 * 128 * 2, char(0)); decodedLength = 0;
    QCOMPARE(lzokay::decompress(reinterpret_cast<const u8 *>(sprites.pixelData.constData()), compressed,
        reinterpret_cast<u8 *>(decoded.data()), std::size_t(decoded.size()), decodedLength), lzokay::EResult::Success);
    QCOMPARE(decodedLength, std::size_t(decoded.size()));
    QCOMPARE(qFromLittleEndian<u16>(decoded.constData()), *reinterpret_cast<const u16 *>(underlay.constScanLine(0)));
    state = object.bitButton.states[0]; state.text.languageTexts.clear();
    for (mode = 0; mode < 3; ++mode)
    {
        state.imageMode = mode; state.opacity = 128;
        image = QImage(128, 128, QImage::Format_ARGB32_Premultiplied); image.fill(pageColor);
        painter.begin(&image); paintBitButton(&painter, image.rect(), state, QStringLiteral("中文")); painter.end();
        QCOMPARE(image.pixelColor(0, 0), pageColor);
    }
    custom = QImage(8, 8, QImage::Format_ARGB32_Premultiplied); custom.fill(Qt::transparent);
    custom.setPixelColor(4, 4, Qt::white); custom.setPixelColor(3, 3, QColor(255, 0, 0, 128));
    buffer.setBuffer(&png); QVERIFY(buffer.open(QIODevice::WriteOnly)); QVERIFY(custom.save(&buffer, "PNG"));
    state.picturePng = png; state.imageMode = 0; state.opacity = 255;
    image = QImage(8, 8, QImage::Format_ARGB32_Premultiplied); image.fill(pageColor);
    painter.begin(&image); paintBitButton(&painter, image.rect(), state, QStringLiteral("中文")); painter.end();
    QCOMPARE(image.pixelColor(0, 0), pageColor);
    QCOMPARE(image.pixelColor(4, 4), QColor(Qt::white)); // Artwork white details must remain opaque.
    QVERIFY(image.pixelColor(3, 3) != pageColor && image.pixelColor(3, 3) != QColor(Qt::red));
    state.useImage = false; state.background = QColor(12, 80, 160);
    painter.begin(&image); paintBitButton(&painter, image.rect(), state, QStringLiteral("中文")); painter.end();
    QCOMPARE(image.pixelColor(0, 0), state.background);
    state.transparent = true; image.fill(pageColor);
    painter.begin(&image); paintBitButton(&painter, image.rect(), state, QStringLiteral("中文")); painter.end();
    QCOMPARE(image.pixelColor(0, 0), pageColor);
    object.layer = 2; lowerButton = object; lowerButton.id = QStringLiteral("lower-dynamic-alpha"); lowerButton.layer = 1;
    project.objects = {lowerButton, object};
    QVERIFY(!buildProjectPayload(project, &blocks, &error));
    QVERIFY(error.contains(QStringLiteral("重叠"))); // Alpha-bearing artwork cannot be baked over an independently changing button.
    custom.fill(Qt::white); buffer.close(); png.clear();
    QVERIFY(buffer.open(QIODevice::WriteOnly)); QVERIFY(custom.save(&buffer, "PNG"));
    object.bitButton.states[0].picturePng = png; object.bitButton.states[1].picturePng = png;
    QVERIFY(!bitButtonNeedsUnderlay(object.bitButton.states[0], object.geometry.size()));
    QVERIFY(bitButtonNeedsUnderlay(object.bitButton.states[0], QSize(0, 0)));
    project.objects = {lowerButton, object}; error.clear();
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error)); // A fully opaque image preserves the existing overlap capability.
    object.bitButton.indicator = false; object.bitButton.states[1].picturePng.clear();
    object.bitButton.states[1].picture = QStringLiteral(":/system-gallery/extern/圆形按钮指示灯/圆形按钮指示灯_02-01.kvg");
    project.objects = {lowerButton, object}; error.clear();
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error)); // A dormant ON image cannot change single-state OFF compilation.
}

/******************************************************************************
 * Function Name   : StaticTextTests::bitDevicePayloadAndOffsets
 * Description     : Check sorted native records and independent image offsets for two different sizes.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::bitDevicePayloadAndOffsets() //20261109-0 add
{
    HmiProject project;
    HmiObject first;
    HmiObject second;
    QVector<QByteArray> blocks;
    QVector<HmiDownloadStep> steps;
    QString error;
    QByteArray record;
    u32 firstSize;
    u32 secondSize;

    first = buttonObject();
    first.geometry = QRect(10, 20, 121, 61);
    first.address = QStringLiteral("LB23");
    first.layer = 2;
    second = first;
    second.id = QStringLiteral("button-2");
    second.geometry = QRect(200, 20, 83, 43);
    second.address = QStringLiteral("LB24");
    second.layer = 1;
    second.bitButton.indicator = false;
    project.objects = {first, second};
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    QCOMPARE(blocks.size(), 12);
    QCOMPARE(blocks.at(10).size(), 1280);
    QCOMPARE(u8(blocks.at(1).at(2)), u8(2));
    firstSize = 121 * 61 * 2 * 2;
    secondSize = 83 * 43 * 2;
    record = blocks.at(10).left(640);
    QCOMPARE(qFromLittleEndian<u16>(record.constData() + 4), u16(200));
    QCOMPARE(qFromLittleEndian<u32>(record.constData() + 0x0c), u32(0));
    QCOMPARE(qFromLittleEndian<u32>(record.constData() + 0x10), secondSize);
    QCOMPARE(qFromLittleEndian<u32>(record.constData() + 0x120), u32(24));
    record = blocks.at(10).mid(640, 640);
    QCOMPARE(qFromLittleEndian<u32>(record.constData() + 0x0c), secondSize);
    QCOMPARE(qFromLittleEndian<u32>(record.constData() + 0x10), firstSize);
    QCOMPARE(qFromLittleEndian<u32>(record.constData() + 0x120), u32(23));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(1).constData() + 0x44), firstSize + secondSize);
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(5).constData() + 0x640), u32(3));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(5).constData() + 0x320), u32(blocks.at(11).size()));
    QCOMPARE(blocks.at(5).size(), 2400 + 3 * 12);
    QVERIFY2(buildDeviceTransferPlan(blocks, &steps, &error), qPrintable(error));
    QCOMPARE(steps.size(), 14);
}

/******************************************************************************
 * Function Name   : StaticTextTests::bitSpriteOddWidthAndTransaction
 * Description     : Decode a three-pixel-wide solid sprite and verify packed rows, state order and zero padding.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::bitSpriteOddWidthAndTransaction() //20261109-0 add
{
    HmiObject object;
    BitButtonSpriteSet sprites;
    QByteArray decoded;
    QByteArray original;
    QString error;
    std::size_t decodedLength;
    u32 packedLength;
    u32 compressedLength;
    int state;
    int offset;
    int index;

    object = buttonObject();
    object.geometry = QRect(0, 0, 3, 2);
    for (state = 0; state < 2; ++state)
    {
        object.bitButton.states[state].useImage = false;
        object.bitButton.states[state].text.languageTexts[QStringLiteral("中文")] = QString();
        object.bitButton.states[state].background = state == 0 ? QColor(255, 0, 0) : QColor(0, 255, 0);
    }
    QVERIFY2(buildBitButtonSprites(object, QStringLiteral("中文"), {0, 1}, &sprites, &error), qPrintable(error));
    QCOMPARE(sprites.rawLength, u32(24));
    offset = 0;
    for (state = 0; state < 2; ++state)
    {
        QCOMPARE(qFromLittleEndian<u32>(sprites.lengthRecords.constData() + state * 12), u32(0));
        packedLength = qFromLittleEndian<u32>(sprites.lengthRecords.constData() + state * 12 + 4);
        compressedLength = qFromLittleEndian<u32>(sprites.lengthRecords.constData() + state * 12 + 8);
        QCOMPARE(packedLength % 32, u32(0));
        QVERIFY(compressedLength <= packedLength);
        decoded = QByteArray(12, char(0));
        decodedLength = std::size_t(decoded.size());
        QCOMPARE(lzokay::decompress(reinterpret_cast<const u8 *>(sprites.pixelData.constData() + offset), compressedLength,
            reinterpret_cast<u8 *>(decoded.data()), std::size_t(decoded.size()), decodedLength), lzokay::EResult::Success);
        QCOMPARE(decodedLength, std::size_t(12));
        for (index = 0; index < 6; ++index)
        {
            QCOMPARE(qFromLittleEndian<u16>(decoded.constData() + index * 2), state == 0 ? u16(0xf800) : u16(0x07e0));
        }
        QCOMPARE(sprites.pixelData.mid(offset + int(compressedLength), int(packedLength - compressedLength)),
                 QByteArray(int(packedLength - compressedLength), char(0)));
        offset += int(packedLength);
    }
    QCOMPARE(offset, sprites.pixelData.size());
    original = sprites.pixelData;
    QVERIFY(!buildBitButtonSprites(object, QStringLiteral("中文"), {0, 2}, &sprites, &error));
    QCOMPARE(sprites.pixelData, original);
    object.bitButton.states[1].useImage = true; //20261109-0 add
    object.bitButton.states[1].picture = QStringLiteral("missing-reviewed-image.kvg");
    object.bitButton.states[1].picturePng.clear();
    QVERIFY(!buildBitButtonSprites(object, QStringLiteral("中文"), {0, 1}, &sprites, &error));
    QVERIFY(error.contains(QStringLiteral("图库"))); QCOMPARE(sprites.pixelData, original);
}

/******************************************************************************
 * Function Name   : StaticTextTests::bitDeviceFailureAndSourceSnapshot
 * Description     : Preserve the complete source and reject unsupported active conditions before writing output.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::bitDeviceFailureAndSourceSnapshot() //20261109-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiObject object;
    QVector<QByteArray> blocks;
    QVector<QByteArray> original;
    QTemporaryDir directory;
    QJsonDocument source;
    QString error;
    QString output;

    project.objects = {buttonObject()};
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    original = blocks;
    source = QJsonDocument::fromJson(qUncompress(blocks.at(7)));
    QVERIFY2(HmiProject::fromJson(source.object(), &loaded, &error), qPrintable(error));
    QCOMPARE(loaded.toJson(), project.toJson());
    output = directory.filePath(QStringLiteral("invalid"));
    project.objects[0].bitButton.pressTimeEnabled = true;
    project.objects[0].bitButton.pressTime = 656;
    QVERIFY(!buildProjectPayload(project, &blocks, &error));
    QCOMPARE(blocks, original);
    QVERIFY(!exportProjectPayload(project, output, &error));
    QVERIFY(!QDir(output).exists());
    project.objects[0].bitButton.pressTimeEnabled = false;
    object = textObject();
    object.staticText.visibilityEnabled = true;
    project.objects.append(object);
    QVERIFY(!buildProjectPayload(project, &blocks, &error));
    QCOMPARE(blocks, original);
    project.objects.removeLast();
    project.languages = {QStringLiteral("中文"), QStringLiteral("English"), QStringLiteral("L3"),
        QStringLiteral("L4"), QStringLiteral("L5"), QStringLiteral("L6"), QStringLiteral("L7"),
        QStringLiteral("L8"), QStringLiteral("L9")}; //20261109-0 change: Two languages now compile; keep the unsupported-count transaction regression.
    QVERIFY(!buildProjectPayload(project, &blocks, &error));
    QCOMPARE(blocks, original);
}

/******************************************************************************
 * Function Name   : StaticTextTests::bitFlashImagesAndNativeOrder
 * Description     : Verify ON-first native ordering, per-state blank images and expanded length totals.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::bitFlashImagesAndNativeOrder() //20261109-0 add
{
    HmiProject project;
    HmiObject object;
    QVector<QByteArray> blocks;
    QByteArray decoded;
    QString error;
    std::size_t decodedLength;
    u32 padded;
    u32 compressed;
    int offset;
    int frame;
    int index;

    object = buttonObject();
    object.geometry = QRect(10, 10, 3, 2);
    for (index = 0; index < 2; ++index)
    {
        object.bitButton.states[index].useImage = false;
        object.bitButton.states[index].text.languageTexts.clear();
        object.bitButton.states[index].flash = true;
        object.bitButton.states[index].background = index == 0 ? QColor(255, 0, 0) : QColor(0, 255, 0);
    }
    project.objects = {object};
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    QCOMPARE(u8(blocks.at(10).at(0x10a)), u8(1));
    QCOMPARE(u8(blocks.at(10).at(0x10b)), u8(1));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(10).constData() + 0x10), u32(48));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(5).constData() + 0x640), u32(4));
    QCOMPARE(blocks.at(5).size(), 2448);
    offset = 0;
    for (frame = 0; frame < 4; ++frame)
    {
        padded = qFromLittleEndian<u32>(blocks.at(5).constData() + 2404 + frame * 12);
        compressed = qFromLittleEndian<u32>(blocks.at(5).constData() + 2408 + frame * 12);
        decoded = QByteArray(12, char(0));
        decodedLength = 0;
        QCOMPARE(lzokay::decompress(reinterpret_cast<const u8 *>(blocks.at(11).constData() + offset), compressed,
            reinterpret_cast<u8 *>(decoded.data()), std::size_t(decoded.size()), decodedLength), lzokay::EResult::Success);
        QCOMPARE(decodedLength, std::size_t(12));
        QCOMPARE(qFromLittleEndian<u16>(decoded.constData()), frame < 2 ? u16(0x07e0) : u16(0xf800));
        offset += int(padded);
    }
    QCOMPARE(offset, blocks.at(11).size());
    project.objects[0].bitButton.indicator = false; //20261109-0 add: Native single-state output is the OFF image, even when only hidden ON is flashing.
    project.objects[0].bitButton.states[0].flash = false;
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(5).constData() + 1600), u32(1));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(10).constData() + 16), u32(12));
    compressed = qFromLittleEndian<u32>(blocks.at(5).constData() + 2408);
    decoded = QByteArray(12, char(0)); decodedLength = 0;
    QCOMPARE(lzokay::decompress(reinterpret_cast<const u8 *>(blocks.at(11).constData()), compressed,
        reinterpret_cast<u8 *>(decoded.data()), std::size_t(decoded.size()), decodedLength), lzokay::EResult::Success);
    QCOMPARE(qFromLittleEndian<u16>(decoded.constData()), u16(0xf800));
    project.objects[0].bitButton.indicator = true; project.objects[0].bitButton.dualMonitor = false;
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(5).constData() + 1600), u32(1));
}

/******************************************************************************
 * Function Name   : StaticTextTests::plcProjectRoundTripAndGuard
 * Description     : Preserve explicit connection identity and reject a dangling address or unapplied device route.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::plcProjectRoundTripAndGuard() //20261109-0 add
{
    HmiProject project;
    HmiProject loaded;
    PlcCatalog catalog;
    PlcConnection connection;
    PlcProfile profile;
    QVector<QByteArray> blocks;
    QTemporaryDir directory;
    QString error;
    QString path;

    QVERIFY2(catalog.load(QStringLiteral(":/plc/plc-catalog.json"), &error), qPrintable(error));
    Q_FOREACH (profile, catalog.profilesForModel(project.targetModel))
    {
        if (profile.name == QStringLiteral("Modbus_RTU")) { connection.profileId = profile.id; break; }
    }
    QVERIFY(!connection.profileId.isEmpty());
    connection.id = QStringLiteral("com1");
    connection.channel = PlcChannel::Com1;
    connection.station = QStringLiteral("1");
    project.connections = {connection};
    project.objects = {buttonObject()};
    project.objects[0].address = QStringLiteral("@com1:0x23");
    QVERIFY2(project.validatePlcConfiguration(&error), qPrintable(error));
    QCOMPARE(project.toJson().value(QStringLiteral("schemaVersion")).toInt(), 4);
    path = directory.filePath(QStringLiteral("external.ksp"));
    QVERIFY2(project.save(path, &error), qPrintable(error));
    QVERIFY2(HmiProject::load(path, &loaded, &error), qPrintable(error));
    QCOMPARE(loaded.toJson(), project.toJson());
    QVERIFY(!loaded.connections.first().nodeKnown);
    QVERIFY(!buildProjectPayload(loaded, &blocks, &error));
    QVERIFY(error.contains(QStringLiteral("PLC")));
    project.connections.clear();
    QVERIFY(!project.save(directory.filePath(QStringLiteral("dangling.ksp")), &error));
    QVERIFY(!QFileInfo::exists(directory.filePath(QStringLiteral("dangling.ksp"))));
}

/******************************************************************************
 * Function Name   : StaticTextTests::bitStaticLayersAndTransparency
 * Description     : Keep the real lower static fill and a higher static overlay in every compiled button image.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::bitStaticLayersAndTransparency() //20261109-0 add
{
    HmiProject project;
    HmiObject lower;
    HmiObject button;
    HmiObject upper;
    HmiObject dynamic;
    QVector<QByteArray> blocks;
    QVector<QByteArray> original;
    QByteArray decoded;
    QString error;
    std::size_t decodedLength;
    u32 compressed;
    int index;

    lower = textObject(); lower.id = QStringLiteral("lower"); lower.geometry = QRect(0, 0, 100, 100); lower.layer = 1;
    lower.text.clear(); lower.staticText.languageTexts.clear(); lower.staticText.style = 1; lower.staticText.backgroundColor = Qt::red;
    button = buttonObject(); button.geometry = QRect(20, 20, 20, 20); button.layer = 2;
    for (index = 0; index < 2; ++index)
    {
        button.bitButton.states[index].useImage = false;
        button.bitButton.states[index].text.languageTexts.clear();
        button.bitButton.states[index].opacity = 0;
    }
    upper = lower; upper.id = QStringLiteral("upper"); upper.geometry = QRect(20, 20, 10, 10); upper.layer = 3;
    upper.staticText.backgroundColor = Qt::blue;
    project.objects = {button, upper, lower};
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    compressed = qFromLittleEndian<u32>(blocks.at(5).constData() + 2408);
    decoded = QByteArray(20 * 20 * 2, char(0)); decodedLength = 0;
    QCOMPARE(lzokay::decompress(reinterpret_cast<const u8 *>(blocks.at(11).constData()), compressed,
        reinterpret_cast<u8 *>(decoded.data()), std::size_t(decoded.size()), decodedLength), lzokay::EResult::Success);
    QCOMPARE(decodedLength, std::size_t(decoded.size()));
    QCOMPARE(qFromLittleEndian<u16>(decoded.constData()), u16(0x001f));
    QCOMPARE(qFromLittleEndian<u16>(decoded.constData() + (19 * 20 + 19) * 2), u16(0xf800));
    original = blocks;
    dynamic = button; dynamic.id = QStringLiteral("lower-dynamic"); dynamic.layer = 1.5;
    project.objects.append(dynamic);
    QVERIFY(!buildProjectPayload(project, &blocks, &error));
    QVERIFY(error.contains(QStringLiteral("重叠")));
    QCOMPARE(blocks, original);
}

/******************************************************************************
 * Function Name   : StaticTextTests::externalAddressPickerContext
 * Description     : Preserve external addresses through inherited context and block fallback when a connection is missing.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::externalAddressPickerContext() //20261109-0 add
{
    QWidget owner;
    QWidget child(&owner);
    QLineEdit field(QStringLiteral("@com1:0x23"), &child);
    PlcCatalog catalog;
    PlcProfile profile;
    PlcConnection connection;
    QJsonArray connections;
    QString error;
    bool opened;

    QVERIFY(catalog.load(QStringLiteral(":/plc/plc-catalog.json"), &error));
    Q_FOREACH (profile, catalog.profilesForModel(QStringLiteral("AMT070W124-L800X480R")))
    {
        if (profile.name == QStringLiteral("Modbus_RTU")) { connection.profileId = profile.id; break; }
    }
    connection.id = QStringLiteral("com1"); connection.channel = PlcChannel::Com1; connection.station = QStringLiteral("1");
    connections.append(connection.toJson()); owner.setProperty("plcConnections", connections.toVariantList());
    opened = false;
    QTimer::singleShot(0, &owner, [&owner, &opened]() {
        QDialog *picker;
        picker = owner.findChild<QDialog *>(QStringLiteral("PlcAddressDialog"));
        if (picker == nullptr) { return; }
        opened = true;
        QCOMPARE(picker->findChild<QLineEdit *>(QStringLiteral("PlcValue"))->text(), QStringLiteral("23"));
        picker->findChild<QDialogButtonBox *>(QStringLiteral("PlcAddressButtons"))->button(QDialogButtonBox::Ok)->click();
    });
    editHmiAddress(&child, &field, true);
    QVERIFY(opened); QCOMPARE(field.text(), QStringLiteral("@com1:0x23"));
    owner.setProperty("plcConnections", QJsonArray().toVariantList());
    QTimer::singleShot(0, &owner, [&owner]() {
        QMessageBox *message;
        message = owner.findChild<QMessageBox *>();
        if (message != nullptr) { message->accept(); }
    });
    editHmiAddress(&child, &field, true);
    QCOMPARE(field.text(), QStringLiteral("@com1:0x23"));
}

/******************************************************************************
 * Function Name   : StaticTextTests::externalDeviceScanOracles
 * Description     : Regress three native COM1 scan files and write-only exclusion through production compilation.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::externalDeviceScanOracles() //20261109-0 add
{
    HmiProject project;
    HmiObject button;
    PlcConnection connection;
    DeviceAddressCompiler compiler;
    QVector<QByteArray> payload;
    QByteArray scan;
    QByteArray expected;
    QByteArray record;
    QString error;

    connection.id = QStringLiteral("com1");
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.channel = PlcChannel::Com1; connection.station = QStringLiteral("1");
    connection.nodeKnown = true; connection.node = 1;
    project.connections = {connection};
    button = buttonObject(); button.address = QStringLiteral("@com1:0x0");
    button.bitButton.operation = 3;
    project.objects = {button};
    // Recorded native files: external-modbus-com1 / external-modbus-bit1 / external-modbus-word4x-ui-compiled.
    expected = QByteArray(200, char(0)); expected[0] = 1;
    expected.append(QByteArray::fromHex("4d0100010100000000000000"));
    QVERIFY2(compiler.prepare(project, &scan, &error), qPrintable(error));
    QCOMPARE(scan, expected);
    QVERIFY2(buildProjectPayload(project, &payload, &error), qPrintable(error));
    QCOMPARE(payload.at(4), expected);
    QCOMPARE(payload.at(10).mid(0x118, 12), QByteArray::fromHex("014d01000001000000000000"));
    button.bitButton.monitorSame = false; button.bitButton.monitorType = 0;
    button.bitButton.monitorBitAddress = QStringLiteral("@com1:0x1"); project.objects = {button};
    expected[204] = 2;
    QVERIFY2(compiler.prepare(project, &scan, &error), qPrintable(error)); QCOMPARE(scan, expected);
    button.bitButton.monitorType = 1; button.bitButton.monitorWordAddress = QStringLiteral("@com1:4x0");
    project.objects = {button}; expected[0] = 2; expected[204] = 1;
    expected.append(QByteArray::fromHex("440100010100000000000000"));
    QVERIFY2(compiler.prepare(project, &scan, &error), qPrintable(error)); QCOMPARE(scan, expected);
    button.bitButton.operation = 0; button.bitButton.indicator = false;
    button.bitButton.notice = true; button.bitButton.noticeBit = true;
    button.bitButton.noticeBitAddress = QStringLiteral("@com1:0x23"); project.objects = {button};
    QVERIFY2(buildProjectPayload(project, &payload, &error), qPrintable(error));
    QCOMPARE(payload.at(4), QByteArray(200, char(0)));
    QCOMPARE(payload.at(10).mid(0x70, 12), QByteArray::fromHex("014d01000000010017000000"));
    QVERIFY2(compiler.encode(QStringLiteral("@com1:4x_bit12.03"), false, false, &record, &error), qPrintable(error));
    QCOMPARE(record.mid(0, 12), QByteArray::fromHex("014b010000000000c3000000"));
    expected = scan;
    project.connections[0].nodeKnown = false;
    QVERIFY(!compiler.prepare(project, &scan, &error)); QCOMPARE(scan, expected);
    QVERIFY(error.contains(QStringLiteral("节点")));
    project.connections[0].nodeKnown = true; project.connections[0].channel = PlcChannel::Com2;
    QVERIFY(!compiler.prepare(project, &scan, &error)); QCOMPARE(scan, expected);
    project.connections[0].channel = PlcChannel::Com1;
    project.objects[0].bitButton.touch.visibilityEnabled = true;
    project.objects[0].bitButton.touch.wordEnabled = true;
    project.objects[0].bitButton.touch.wordAddress = QStringLiteral("@com1:4x0");
    project.objects[0].bitButton.touch.dataType = 1;
    expected = QByteArray(200, char(0)); expected[0] = 1;
    expected.append(QByteArray::fromHex("440100010200000000000000"));
    QVERIFY2(compiler.prepare(project, &scan, &error), qPrintable(error)); QCOMPARE(scan, expected);
    project.objects[0].bitButton.touch.wordAddress = QStringLiteral("@com1:4x65535");
    QVERIFY(!compiler.prepare(project, &scan, &error)); QCOMPARE(scan, expected);
    project.objects[0].bitButton.touch.dataType = 2;
    QVERIFY2(compiler.prepare(project, &scan, &error), qPrintable(error));
    QCOMPARE(scan.mid(200, 12), QByteArray::fromHex("4401000101000000ffff0000"));
}

/******************************************************************************
 * Function Name   : StaticTextTests::localDeviceAddressBounds
 * Description     : Enforce actual active local read/write spans while retaining disabled local drafts.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::localDeviceAddressBounds() //20261109-0 add
{
    HmiProject project;
    DeviceAddressCompiler compiler;
    QByteArray table;
    QByteArray before;
    QString error;

    project.objects = {buttonObject()};
    project.objects[0].address = QStringLiteral("LB4096");
    project.objects[0].bitButton.indicator = false;
    QVERIFY2(compiler.prepare(project, &table, &error), qPrintable(error));
    before = table;
    project.objects[0].address = QStringLiteral("LB4097");
    QVERIFY(!compiler.prepare(project, &table, &error)); QCOMPARE(table, before);
    project.objects[0].address = QStringLiteral("LB0");
    project.objects[0].bitButton.touch.visibilityEnabled = true;
    project.objects[0].bitButton.touch.wordEnabled = true;
    project.objects[0].bitButton.touch.dataType = 1;
    project.objects[0].bitButton.touch.wordAddress = QStringLiteral("LW4096");
    QVERIFY(!compiler.prepare(project, &table, &error)); QCOMPARE(table, before);
    project.objects[0].bitButton.touch.wordAddress = QStringLiteral("LW4095");
    QVERIFY2(compiler.prepare(project, &table, &error), qPrintable(error));
    project.objects[0].bitButton.touch.visibilityEnabled = false;
    project.objects[0].bitButton.touch.wordAddress = QStringLiteral("LW9999");
    QVERIFY2(compiler.prepare(project, &table, &error), qPrintable(error));
    project.objects[0].bitButton.notice = true; project.objects[0].bitButton.noticeWord = true;
    project.objects[0].bitButton.noticeWordType = 4;
    project.objects[0].bitButton.noticeWordAddress = QStringLiteral("LW4096");
    QVERIFY(!compiler.prepare(project, &table, &error)); QCOMPARE(table, before);
}

/******************************************************************************
 * Function Name   : StaticTextTests::nativeAddressPickerInitialValue
 * Description     : Start a native fixed-width bit family with valid syntax and commit through the shared picker.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::nativeAddressPickerInitialValue() //20261109-0 add
{
    QWidget owner;
    QLineEdit field(QStringLiteral("LB0"), &owner);
    PlcConnection connection;
    QJsonArray connections;

    connection.id = QStringLiteral("com1");
    connection.profileId = QStringLiteral("Link/serial_amz/DELTA/DELTA_AS.ini");
    connection.channel = PlcChannel::Com1; connection.station = QStringLiteral("1");
    connections.append(connection.toJson()); owner.setProperty("plcConnections", connections.toVariantList());
    QTimer::singleShot(0, &owner, [&owner]() {
        QDialog *picker;
        QComboBox *memory;
        QComboBox *family;

        picker = owner.findChild<QDialog *>(QStringLiteral("PlcAddressDialog"));
        QVERIFY(picker != nullptr); if (picker == nullptr) { return; }
        memory = picker->findChild<QComboBox *>(QStringLiteral("PlcMemory"));
        family = picker->findChild<QComboBox *>(QStringLiteral("PlcFamily"));
        memory->setCurrentIndex(memory->findData(QStringLiteral("com1")));
        family->setCurrentIndex(family->findData(QStringLiteral("X_Bit")));
        QCOMPARE(picker->findChild<QLineEdit *>(QStringLiteral("PlcValue"))->text(), QStringLiteral("0.00"));
        QCOMPARE(picker->findChild<QComboBox *>(QStringLiteral("PlcRadix"))->currentText(), QStringLiteral("原生进制"));
        picker->findChild<QDialogButtonBox *>(QStringLiteral("PlcAddressButtons"))->button(QDialogButtonBox::Ok)->click();
    });
    editHmiAddress(&owner, &field, true);
    QCOMPARE(field.text(), QStringLiteral("@com1:X_Bit0.00"));
}

/******************************************************************************
 * Function Name   : StaticTextTests::bitDeviceLanguages
 * Description     : Regress the native two-language byte counts and retain separate language image sequences.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::bitDeviceLanguages() //20261109-0 add
{
    HmiProject project;
    HmiProject single;
    HmiObject button;
    QVector<QByteArray> blocks;
    QVector<QByteArray> chinese;
    QVector<QByteArray> english;
    QVector<QByteArray> original;
    QString error;
    int index;

    project.languages = {QStringLiteral("中文"), QStringLiteral("English")};
    project.activeLanguage = QStringLiteral("English");
    button = buttonObject(); button.geometry = QRect(10, 10, 120, 60);
    button.bitButton.states[0].text.languageTexts[QStringLiteral("English")] = QStringLiteral("STOP");
    button.bitButton.states[1].text.languageTexts[QStringLiteral("English")] = QStringLiteral("RUN");
    project.objects = {button};
    single = project; single.languages = {QStringLiteral("中文")}; single.activeLanguage = single.languages.first();
    QVERIFY2(buildProjectPayload(single, &chinese, &error), qPrintable(error));
    single.languages = {QStringLiteral("English")}; single.activeLanguage = single.languages.first();
    QVERIFY2(buildProjectPayload(single, &english, &error), qPrintable(error));
    QVERIFY(chinese.at(11) != english.at(11));
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    QCOMPARE(blocks.at(11), chinese.at(11) + english.at(11));
    QCOMPARE(blocks.at(5).mid(2400), chinese.at(5).mid(2400) + english.at(5).mid(2400));
    QVERIFY(bitButtonContent(button.bitButton.states[0], QStringLiteral("French")).isEmpty());
    for (index = 1; index < 4; ++index)
    {
        button.id = QStringLiteral("language-button-%1").arg(index);
        button.geometry.translate(150, 0); project.objects.append(button);
    }
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    // Native two-language four-button oracle: 4*2*2*120*60*2 raw bytes, sixteen indexed images.
    QCOMPARE(u8(blocks.at(0)[2174]), u8(2));
    QCOMPARE(blocks.at(0)[2175], char(0));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(1).constData() + 68), u32(230400));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(5).constData() + 1600), u32(16));
    QCOMPARE(blocks.at(10).size(), 2560);
    for (index = 0; index < 4; ++index)
    {
        QCOMPARE(qFromLittleEndian<u32>(blocks.at(10).constData() + index * 640 + 12), u32(index * 57600));
        QCOMPARE(qFromLittleEndian<u32>(blocks.at(10).constData() + index * 640 + 16), u32(57600));
        QCOMPARE(qFromLittleEndian<u32>(blocks.at(10).constData() + index * 640 + 0x190), u32(2));
    }
    original = blocks; project.languages.clear();
    QVERIFY(!buildProjectPayload(project, &blocks, &error)); QCOMPARE(blocks, original);
}

/******************************************************************************
 * Function Name   : StaticTextTests::localizedStaticTextControlPayload
 * Description     : Regress the original mixed two-language layout and dynamic text visibility subscriptions.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::localizedStaticTextControlPayload() //20261109-0 add
{
    HmiProject project;
    HmiObject text;
    HmiObject button;
    PlcConnection connection;
    QVector<QByteArray> blocks;
    QVector<QByteArray> original;
    QByteArray decoded;
    QString error;
    std::size_t decodedLength;
    int index;

    project.languages = {QStringLiteral("中文"), QStringLiteral("English")};
    for (index = 0; index < 4; ++index)
    {
        text = textObject(); text.id = QStringLiteral("localized-title-%1").arg(index); text.layer = index;
        text.geometry = QRect(40 + index * 180, 100, 160, 40);
        text.staticText.languageTexts[QStringLiteral("中文")] = QStringLiteral("标题%1").arg(index);
        text.staticText.languageTexts[QStringLiteral("English")] = QStringLiteral("Title %1").arg(index);
        project.objects.append(text);
    }
    for (index = 0; index < 4; ++index)
    {
        button = buttonObject(); button.id = QStringLiteral("localized-button-%1").arg(index); button.layer = 10 + index;
        button.geometry = QRect(40 + index * 180, 160, 140, 80); project.objects.append(button);
    }
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    QCOMPARE(blocks.at(10).size(), 5120);
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(1).constData() + 68), u32(460800));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(5).constData() + 1600), u32(24));
    for (index = 0; index < 4; ++index)
    {
        QCOMPARE(u8(blocks.at(10)[index * 640]), u8(28));
        QCOMPARE(qFromLittleEndian<u32>(blocks.at(10).constData() + index * 640 + 12), u32(index * 25600));
        QCOMPARE(qFromLittleEndian<u32>(blocks.at(10).constData() + index * 640 + 16), u32(25600));
        QCOMPARE(qFromLittleEndian<u16>(blocks.at(10).constData() + index * 640 + 0x102), u16(2));
    }
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(10).constData() + 4 * 640 + 12), u32(102400));
    decoded = QByteArray(768000, char(0)); decodedLength = 0;
    QCOMPARE(lzokay::decompress(reinterpret_cast<const u8 *>(blocks.at(8).constData()), blocks.at(8).size(),
        reinterpret_cast<u8 *>(decoded.data()), std::size_t(decoded.size()), decodedLength), lzokay::EResult::Success);
    QCOMPARE(qFromLittleEndian<u16>(decoded.constData() + (120 * 800 + 120) * 2), u16(0xc6ff));
    original = blocks; project.objects[0].geometry = project.objects.last().geometry;
    QVERIFY(!buildProjectPayload(project, &blocks, &error)); QCOMPARE(blocks, original);
    QVERIFY(error.contains(QStringLiteral("重叠")));
    project.languages = {QStringLiteral("中文")}; project.objects = {textObject()};
    project.objects[0].staticText.visibilityEnabled = true;
    project.objects[0].staticText.bitEnabled = true;
    project.objects[0].staticText.bitAddress = QStringLiteral("@com1:0x2");
    connection.id = QStringLiteral("com1"); connection.channel = PlcChannel::Com1;
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.station = QStringLiteral("1"); connection.nodeKnown = true; connection.node = 1;
    project.connections = {connection};
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    QCOMPARE(u8(blocks.at(10)[0]), u8(28)); QCOMPARE(u8(blocks.at(10)[0x15]), u8(1));
    QCOMPARE(blocks.at(4).mid(200), QByteArray::fromHex("4d0100010100000002000000"));
}

/******************************************************************************
 * Function Name   : wordButtonObject
 * Description     : Provide a complete word switch independently of the bit-operation fixture.
 * Input Parameters: None
 * Return Value    : Default signed-16 word button.
 * Date            : 2026.11.09
 ******************************************************************************/
static HmiObject wordButtonObject() //20261109-6 add
{
    HmiObject object;

    object.id = QStringLiteral("word-1"); object.type = QStringLiteral("switch");
    object.name = QStringLiteral("字按钮1"); object.text = QStringLiteral("OFF");
    object.address = QStringLiteral("LW0"); object.geometry = QRect(20, 20, 80, 40);
    object.hasBitButtonProperties = true; object.hasWordButtonProperties = true;
    object.bitButton.indicator = false; object.bitButton.dualMonitor = false;
    object.bitButton.states[1].background = QColor(Qt::gray);
    return object;
}

/******************************************************************************
 * Function Name   : StaticTextTests::wordProjectAndScanMatrix
 * Description     : Preserve all operation/type combinations and validate native read spans and schema guards.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::wordProjectAndScanMatrix() //20261109-6 add
{
    HmiProject project;
    HmiProject loaded;
    HmiObject object;
    PlcConnection connection;
    DeviceAddressCompiler compiler;
    QJsonObject json;
    QJsonArray pages;
    QJsonObject page;
    QJsonArray objects;
    QJsonObject control;
    QTemporaryDir directory;
    QByteArray scan;
    QByteArray expected;
    QString error;
    int operation;
    int type;
    int count;

    connection.id = QStringLiteral("com1"); connection.channel = PlcChannel::Com1;
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.station = QStringLiteral("1"); connection.nodeKnown = true; connection.node = 1;
    project.connections = {connection};
    for (operation = 0; operation < 7; ++operation)
    {
        for (type = 0; type < 7; ++type)
        {
            object = wordButtonObject(); object.address = QStringLiteral("@com1:4x0");
            object.wordButton.operation = operation; object.wordButton.resetDefaultsForDataType(type);
            object.wordButton.pressedEffect = (operation % 2) != 0;
            object.bitButton.states[1].text.languageTexts[QStringLiteral("English")] = QStringLiteral("PRESS");
            project.objects = {object}; json = project.toJson();
            QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 6);
            QVERIFY2(HmiProject::fromJson(json, &loaded, &error), qPrintable(error));
            QCOMPARE(loaded.toJson(), json);
            count = object.wordButton.transferWordCount();
            expected = QByteArray(200, char(0));
            if (operation >= 3)
            {
                expected[0] = 1;
                expected.append(QByteArray::fromHex("440100010100000000000000"));
                expected[204] = char(count);
            }
            QVERIFY2(compiler.prepare(project, &scan, &error), qPrintable(error));
            QCOMPARE(scan, expected);
            project.objects[0].address = count == 2 ? QStringLiteral("LW4095") : QStringLiteral("LW4096");
            QVERIFY2(compiler.prepare(project, &scan, &error), qPrintable(error));
            expected = scan; project.objects[0].address = count == 2 ? QStringLiteral("LW4096") : QStringLiteral("LW4097");
            QVERIFY(!compiler.prepare(project, &scan, &error)); QCOMPARE(scan, expected);
        }
    }
    project.connections.clear(); project.objects = {wordButtonObject()};
    QVERIFY2(project.save(directory.filePath(QStringLiteral("word.ksp")), &error), qPrintable(error));
    QVERIFY2(HmiProject::load(directory.filePath(QStringLiteral("word.ksp")), &loaded, &error), qPrintable(error));
    QCOMPARE(loaded.toJson(), project.toJson());
    json = project.toJson(); json.insert(QStringLiteral("schemaVersion"), 5);
    QVERIFY(!HmiProject::fromJson(json, &loaded, &error));
    json = project.toJson(); pages = json.value(QStringLiteral("pages")).toArray(); page = pages[0].toObject();
    objects = page.value(QStringLiteral("objects")).toArray(); control = objects[0].toObject();
    control.remove(QStringLiteral("bitButton")); objects[0] = control; page.insert(QStringLiteral("objects"), objects);
    pages[0] = page; json.insert(QStringLiteral("pages"), pages);
    QVERIFY(!HmiProject::fromJson(json, &loaded, &error));
    project.objects[0].bitButton.indicator = true;
    QVERIFY(!project.validatePlcConfiguration(&error));
}

/******************************************************************************
 * Function Name   : StaticTextTests::wordCreationAndCanvas
 * Description     : Exercise actual word actions, cancel, placement, copying and history without USB.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::wordCreationAndCanvas() //20261109-6 add
{
    MainWindow window;
    QAction *action;
    QAction *wordAction;
    ScreenCanvas *canvas;
    HmiProject empty;
    HmiObject object;
    QPoint start;
    QPoint end;
    bool opened;
    bool selected;

    wordAction = nullptr; opened = false;
    window.show(); QApplication::processEvents(); canvas = window.findChild<ScreenCanvas *>(); canvas->setProject(empty);
    Q_FOREACH (action, window.findChildren<QAction *>())
    {
        if (action->text() == QStringLiteral("字按钮")) { wordAction = action; }
    }
    QVERIFY(wordAction != nullptr);
    QTimer::singleShot(0, &window, [&window, &opened]() {
        BitButtonDialog *dialog;
        dialog = window.findChild<BitButtonDialog *>(); opened = dialog != nullptr;
        if (dialog != nullptr)
        {
            QCOMPARE(dialog->windowTitle(), QStringLiteral("字按钮"));
            dialog->findChild<QComboBox *>(QStringLiteral("WordOperation"))->setCurrentIndex(5);
            dialog->findChild<QLineEdit *>(QStringLiteral("WordConstant"))->setText(QStringLiteral("9"));
            dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        }
    });
    wordAction->trigger(); QVERIFY(opened); QCOMPARE(canvas->objects().size(), 0);
    start = canvas->mapFromScene(QPointF(100, 80)); end = canvas->mapFromScene(QPointF(240, 160));
    QTest::mousePress(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QTest::mouseMove(canvas->viewport(), end); QTest::mouseRelease(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QCOMPARE(canvas->objects().size(), 1); object = canvas->objects().first();
    QVERIFY(object.hasWordButtonProperties); QCOMPARE(object.wordButton.operation, 5);
    QCOMPARE(object.address, QStringLiteral("LW0")); QCOMPARE(object.wordButton.constantValue, QStringLiteral("9"));
    QVERIFY(object.name.startsWith(QStringLiteral("字按钮")));
    QTimer::singleShot(0, &window, [&window]() {
        BitButtonDialog *dialog;
        dialog = window.findChild<BitButtonDialog *>();
        if (dialog != nullptr) { dialog->findChild<QLineEdit *>(QStringLiteral("WordConstant"))->setText(QStringLiteral("777")); dialog->reject(); }
    });
    QTest::mouseDClick(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, canvas->mapFromScene(QPointF(150, 110)));
    QCOMPARE(canvas->objects().first().wordButton.toJson(), object.wordButton.toJson());
    canvas->executeEdit(QStringLiteral("simulate1")); QCOMPARE(canvas->objects().first().toJson(), object.toJson());
    canvas->executeEdit(QStringLiteral("copy")); canvas->executeEdit(QStringLiteral("paste"));
    QCOMPARE(canvas->objects().size(), 2);
    QCOMPARE(canvas->selectedObject(&selected).wordButton.toJson(), object.wordButton.toJson()); QVERIFY(selected);
    canvas->executeEdit(QStringLiteral("undo")); QCOMPARE(canvas->objects().size(), 1);
    canvas->executeEdit(QStringLiteral("redo")); QCOMPARE(canvas->objects().size(), 2);
    QTimer::singleShot(0, &window, [&window]() {
        BitButtonDialog *dialog;
        dialog = window.findChild<BitButtonDialog *>(); if (dialog != nullptr) { dialog->reject(); }
    });
    wordAction->trigger();
    QTest::mousePress(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QTest::mouseMove(canvas->viewport(), end); QTest::mouseRelease(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QCOMPARE(canvas->objects().size(), 2);
}

/******************************************************************************
 * Function Name   : StaticTextTests::wordDeviceSpritesAndTransactions
 * Description     : Check active pressed frames, multilingual offsets, scan bindings and failure atomicity.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::wordDeviceSpritesAndTransactions() //20261109-6 add
{
    HmiProject project;
    HmiObject object;
    QVector<QByteArray> blocks;
    QVector<QByteArray> normal;
    QVector<QByteArray> before;
    QString error;
    QByteArray decoded;
    std::size_t decodedLength;
    u32 compressed;
    u32 offset;
    int index;

    object = wordButtonObject(); project.objects = {object};
    QVERIFY2(buildProjectPayload(project, &normal, &error), qPrintable(error));
    QCOMPARE(normal.at(10).size(), 640); QCOMPARE(u8(normal.at(10)[0]), u8(10));
    QCOMPARE(qFromLittleEndian<u16>(normal.at(10).constData() + 0x106), u16(1)); //20261109-6 change
    QCOMPARE(qFromLittleEndian<u32>(normal.at(5).constData() + 1600), u32(1));
    project.objects[0].bitButton.states[1].flash = true;
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    QCOMPARE(blocks.at(10), normal.at(10)); QCOMPARE(blocks.at(11), normal.at(11));
    QCOMPARE(blocks.at(5), normal.at(5)); // A dormant pressed state changes only the editable upload snapshot.
    project.objects[0].bitButton.states[1].flash = false;
    project.objects[0].wordButton.pressedEffect = true;
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(5).constData() + 1600), u32(2));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(10).constData() + 16), u32(80 * 40 * 2 * 2));
    for (index = 0; index < 2; ++index)
    {
        project.objects[0].bitButton.states[index].useImage = false;
        project.objects[0].bitButton.states[index].text.languageTexts[QStringLiteral("中文")] = QString();
        project.objects[0].bitButton.states[index].background = index == 0 ? QColor(Qt::red) : QColor(Qt::green);
    }
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    offset = 0;
    for (index = 0; index < 2; ++index) // Native subtype1 uses the opposite two-state order from bit monitors.
    {
        compressed = qFromLittleEndian<u32>(blocks.at(5).constData() + 2400 + index * 12 + 8);
        decoded = QByteArray(80 * 40 * 2, char(0)); decodedLength = 0;
        QCOMPARE(lzokay::decompress(reinterpret_cast<const u8 *>(blocks.at(11).constData() + offset), compressed,
            reinterpret_cast<u8 *>(decoded.data()), std::size_t(decoded.size()), decodedLength), lzokay::EResult::Success);
        QCOMPARE(decodedLength, std::size_t(decoded.size()));
        QCOMPARE(qFromLittleEndian<u16>(decoded.constData() + (20 * 80 + 40) * 2), index == 0 ? u16(0xf800) : u16(0x07e0));
        offset += qFromLittleEndian<u32>(blocks.at(5).constData() + 2400 + index * 12 + 4); //20261109-6 change: Advance over the recorded 32-byte padding, not just the LZO stream.
    }
    project.objects[0] = object; project.objects[0].wordButton.pressedEffect = true;
    project.languages = {QStringLiteral("中文"), QStringLiteral("English")};
    project.objects[0].bitButton.states[0].text.languageTexts[QStringLiteral("English")] = QStringLiteral("SET");
    project.objects[0].bitButton.states[1].text.languageTexts[QStringLiteral("English")] = QStringLiteral("PRESSED");
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(5).constData() + 1600), u32(4));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(10).constData() + 16), u32(80 * 40 * 2 * 4));
    before = blocks; project.objects[0].wordButton.dataType = 99;
    QVERIFY(!buildProjectPayload(project, &blocks, &error)); QCOMPARE(blocks, before);
    project.objects[0].wordButton.dataType = 1; project.objects[0].address = QStringLiteral("LW4096");
    QVERIFY(!buildProjectPayload(project, &blocks, &error)); QCOMPARE(blocks, before);
}

/******************************************************************************
 * Function Name   : StaticTextTests::buttonFontFallbackRetainsSource
 * Description     : Keep missing font names through unrelated edits and persist deliberate replacements.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::buttonFontFallbackRetainsSource() //20261109-6 add
{
    HmiProject project;
    HmiObject object;
    BitButtonDialog *dialog;
    QFontComboBox *font;
    QString missing;
    QString selected;
    int word;

    missing = QStringLiteral("KinSeal unavailable family 20261109");
    for (word = 0; word < 2; ++word)
    {
        object = word ? wordButtonObject() : buttonObject();
        object.bitButton.states[0].text.fontFamily = missing;
        dialog = new BitButtonDialog(object, project);
        QCOMPARE(dialog->object().bitButton.states[0].text.fontFamily, missing);
        dialog->findChild<QPlainTextEdit *>(QStringLiteral("BitText"))->setPlainText(QStringLiteral("changed caption"));
        QCOMPARE(dialog->object().bitButton.states[0].text.fontFamily, missing);
        font = dialog->findChild<QFontComboBox *>(QStringLiteral("BitFont"));
        QVERIFY(font != nullptr);
        if (font->count() > 1) // The private Windows run also checks replacement; a fontless offscreen backend still checks preservation for both subtypes.
        {
            font->setCurrentIndex((font->currentIndex() + 1) % font->count()); selected = font->currentFont().family();
            QCOMPARE(dialog->object().bitButton.states[0].text.fontFamily, selected);
        }
        delete dialog;
    }
}

/******************************************************************************
 * Function Name   : lampObject
 * Description     : Build a read-only lamp with separate monitor addresses and retained state drafts.
 * Input Parameters: count
 * Return Value    : Complete lamp object.
 * Date            : 2026.11.09
 ******************************************************************************/
static HmiObject lampObject(int count = 2) //20261109-8 add
{
    HmiObject object;

    object.id = QStringLiteral("lamp-1"); object.type = QStringLiteral("switch");
    object.name = QStringLiteral("指示灯1"); object.text = QStringLiteral("OFF");
    object.geometry = QRect(20, 20, 80, 40);
    object.hasBitButtonProperties = true; object.hasLampProperties = true;
    object.bitButton.monitorSame = false; object.bitButton.bitIndex = 0;
    if (count > 2)
    {
        object.lamp.monitorMode = 2; object.lamp.multiStateCount = count;
        object.bitButton.dualMonitor = false; object.lamp.ensureStateCount(count, object.bitButton);
    }
    return object;
}

/******************************************************************************
 * Function Name   : StaticTextTests::lampProjectAndScanMatrix
 * Description     : Regress schema seven, read-only scan spans, wide local bounds and forbidden operations.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::lampProjectAndScanMatrix() //20261109-8 add
{
    HmiProject project;
    HmiProject loaded;
    HmiObject object;
    PlcConnection connection;
    DeviceAddressCompiler compiler;
    QJsonObject json;
    QTemporaryDir directory;
    QByteArray scan;
    QByteArray before;
    QByteArray expected;
    QString error;
    int type;
    int count;

    connection.id = QStringLiteral("com1"); connection.channel = PlcChannel::Com1;
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.station = QStringLiteral("1"); connection.nodeKnown = true; connection.node = 1;
    project.connections = {connection};
    for (type = 0; type < 2; ++type)
    {
        object = lampObject(5); object.lamp.multiDataType = type;
        object.lamp.multiMonitorAddress = QStringLiteral("@com1:4x10"); project.objects = {object};
        json = project.toJson(); QCOMPARE(json.value(QStringLiteral("schemaVersion")).toInt(), 7);
        QVERIFY2(HmiProject::fromJson(json, &loaded, &error), qPrintable(error)); QCOMPARE(loaded.toJson(), json);
        QVERIFY2(compiler.prepare(project, &scan, &error), qPrintable(error));
        expected = QByteArray(200, char(0)); expected[0] = 1;
        expected.append(QByteArray::fromHex("44010001010000000a000000")); expected[204] = char(type + 1);
        QCOMPARE(scan, expected);
        project.objects[0].lamp.multiMonitorAddress = type == 0 ? QStringLiteral("LW4096") : QStringLiteral("LW4095");
        QVERIFY2(compiler.prepare(project, &scan, &error), qPrintable(error)); before = scan;
        project.objects[0].lamp.multiMonitorAddress = type == 0 ? QStringLiteral("LW4097") : QStringLiteral("LW4096");
        QVERIFY(!compiler.prepare(project, &scan, &error)); QCOMPARE(scan, before);
    }
    project.objects = {lampObject()}; project.objects[0].bitButton.monitorBitAddress = QStringLiteral("@com1:0x2");
    QVERIFY2(compiler.prepare(project, &scan, &error), qPrintable(error));
    QCOMPARE(scan.mid(200), QByteArray::fromHex("4d0100010100000002000000"));
    project.objects[0].bitButton.monitorType = 1; project.objects[0].bitButton.monitorWordAddress = QStringLiteral("@com1:4x10");
    project.objects[0].bitButton.bitIndex = 7;
    QVERIFY2(compiler.prepare(project, &scan, &error), qPrintable(error));
    QCOMPARE(scan.mid(200), QByteArray::fromHex("44010001010000000a000000"));
    project.connections.clear(); project.objects = {lampObject(32)};
    QVERIFY2(project.save(directory.filePath(QStringLiteral("lamp.ksp")), &error), qPrintable(error));
    QVERIFY2(HmiProject::load(directory.filePath(QStringLiteral("lamp.ksp")), &loaded, &error), qPrintable(error));
    QCOMPARE(loaded.toJson(), project.toJson()); json = project.toJson(); json.insert(QStringLiteral("schemaVersion"), 6);
    QVERIFY(!HmiProject::fromJson(json, &loaded, &error));
    for (count = 0; count < 6; ++count)
    {
        project.objects = {lampObject()};
        if (count == 0) { project.objects[0].bitButton.macroEnabled = true; }
        if (count == 1) { project.objects[0].bitButton.notice = true; }
        if (count == 2) { project.objects[0].bitButton.touch.visibilityEnabled = true; }
        if (count == 3) { project.objects[0].bitButton.pressTimeEnabled = true; }
        if (count == 4) { project.objects[0].bitButton.monitorSame = true; }
        if (count == 5) { project.objects[0].lamp.monitorMode = 0; project.objects[0].bitButton.indicator = false; project.objects[0].bitButton.dualMonitor = false; }
        QVERIFY(!project.validatePlcConfiguration(&error));
    }
}

/******************************************************************************
 * Function Name   : StaticTextTests::lampCreationAndCanvas
 * Description     : Exercise the real lamp entry, placement, extended preview, cancel and history transactions.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::lampCreationAndCanvas() //20261109-8 add
{
    MainWindow window;
    QAction *action;
    QAction *lampAction;
    ScreenCanvas *canvas;
    HmiProject empty;
    HmiObject object;
    QPoint start;
    QPoint end;
    bool opened;
    bool selected;

    lampAction = nullptr; opened = false; window.show(); QApplication::processEvents();
    canvas = window.findChild<ScreenCanvas *>(); canvas->setProject(empty);
    Q_FOREACH (action, window.findChildren<QAction *>())
    {
        if (action->text() == QStringLiteral("指示灯")) { lampAction = action; }
    }
    QVERIFY(lampAction != nullptr);
    QTimer::singleShot(0, &window, [&window, &opened]() {
        BitButtonDialog *dialog;
        dialog = window.findChild<BitButtonDialog *>(); opened = dialog != nullptr;
        if (dialog != nullptr)
        {
            QCOMPARE(dialog->windowTitle(), QStringLiteral("位指示灯"));
            dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        }
    });
    lampAction->trigger(); QVERIFY(opened); QCOMPARE(canvas->objects().size(), 0);
    start = canvas->mapFromScene(QPointF(100, 80)); end = canvas->mapFromScene(QPointF(240, 160));
    QTest::mousePress(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QTest::mouseMove(canvas->viewport(), end); QTest::mouseRelease(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QCOMPARE(canvas->objects().size(), 1); object = canvas->objects().first();
    QVERIFY(object.hasLampProperties); QVERIFY(object.address.isEmpty()); QVERIFY(object.name.startsWith(QStringLiteral("指示灯")));
    object.lamp.monitorMode = 2; object.lamp.multiStateCount = 5; object.lamp.ensureStateCount(5, object.bitButton);
    object.bitButton.dualMonitor = false; canvas->updateSelectedObject(object);
    canvas->executeEdit(QStringLiteral("simulate4")); QCOMPARE(canvas->objects().first().toJson(), object.toJson());
    canvas->executeEdit(QStringLiteral("copy")); canvas->executeEdit(QStringLiteral("paste"));
    QCOMPARE(canvas->objects().size(), 2); QCOMPARE(canvas->selectedObject(&selected).lamp.toJson(), object.lamp.toJson()); QVERIFY(selected);
    canvas->executeEdit(QStringLiteral("undo")); QCOMPARE(canvas->objects().size(), 1);
    canvas->executeEdit(QStringLiteral("redo")); QCOMPARE(canvas->objects().size(), 2);
    QTimer::singleShot(0, &window, [&window]() {
        BitButtonDialog *dialog;
        dialog = window.findChild<BitButtonDialog *>(); if (dialog != nullptr) { dialog->reject(); }
    });
    lampAction->trigger();
    QTest::mousePress(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QTest::mouseMove(canvas->viewport(), end); QTest::mouseRelease(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QCOMPARE(canvas->objects().size(), 2);
}

/******************************************************************************
 * Function Name   : StaticTextTests::lampDeviceFramesAndTransactions
 * Description     : Decode native lamp images to verify logical ordering, languages and bounded compilation failures.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void StaticTextTests::lampDeviceFramesAndTransactions() //20261109-8 add
{
    HmiProject project;
    HmiObject object;
    BitButtonState *state;
    QVector<QByteArray> blocks;
    QVector<QByteArray> before;
    QByteArray decoded;
    QString error;
    std::size_t decodedLength;
    u32 offset;
    u32 compressed;
    int index;
    int language;
    int logical;
    int mode;
    const QColor colors[] = {Qt::red, Qt::green, Qt::blue, Qt::white, Qt::black};
    const u16 pixels[] = {0xf800, 0x07e0, 0x001f, 0xffff, 0x0000};

    project.languages = {QStringLiteral("中文"), QStringLiteral("English")};
    for (mode = 1; mode <= 2; ++mode)
    {
        object = lampObject(mode == 1 ? 2 : 5);
        for (index = 0; index < object.lamp.stateCount(); ++index)
        {
            state = object.lamp.state(object.bitButton, index); state->useImage = false;
            state->sameLanguage = false; state->sharedText.clear(); state->text.languageTexts.clear(); //20261109-8 change: These two-language ordering fixtures deliberately use separate language state sets.
            state->background = colors[index]; state->effect3d = false;
        }
        project.objects = {object};
        QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
        QCOMPARE(qFromLittleEndian<u16>(blocks.at(10).constData() + 0x106), u16(4));
        QCOMPARE(qFromLittleEndian<u32>(blocks.at(5).constData() + 1600), u32(object.lamp.stateCount() * 2));
        QCOMPARE(qFromLittleEndian<u32>(blocks.at(10).constData() + 16), u32(80 * 40 * 2 * object.lamp.stateCount() * 2));
        offset = 0;
        for (language = 0; language < 2; ++language)
        {
            for (index = 0; index < object.lamp.stateCount(); ++index)
            {
                compressed = qFromLittleEndian<u32>(blocks.at(5).constData() + 2400 + (language * object.lamp.stateCount() + index) * 12 + 8);
                decoded = QByteArray(80 * 40 * 2, char(0)); decodedLength = 0;
                QCOMPARE(lzokay::decompress(reinterpret_cast<const u8 *>(blocks.at(11).constData() + offset), compressed,
                    reinterpret_cast<u8 *>(decoded.data()), std::size_t(decoded.size()), decodedLength), lzokay::EResult::Success);
                logical = mode == 1 ? 1 - index : index;
                QCOMPARE(qFromLittleEndian<u16>(decoded.constData() + (20 * 80 + 40) * 2), pixels[logical]);
                offset += qFromLittleEndian<u32>(blocks.at(5).constData() + 2400 + (language * object.lamp.stateCount() + index) * 12 + 4);
            }
        }
    }
    project.objects[0].lamp.extraStates[0].flash = true; before = blocks;
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    QCOMPARE(blocks.at(5), before.at(5)); QCOMPARE(blocks.at(11), before.at(11));
    for (index = 0; index < project.objects[0].lamp.stateCount(); ++index)
    {
        project.objects[0].lamp.state(project.objects[0].bitButton, index)->sameLanguage = true;
    }
    QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(5).constData() + 1600), u32(5));
    QCOMPARE(qFromLittleEndian<u32>(blocks.at(10).constData() + 16), u32(80 * 40 * 2 * 5));
    before = blocks; project.objects[0].lamp.multiDataType = 1; project.objects[0].lamp.multiMonitorAddress = QStringLiteral("LW4096");
    QVERIFY(!buildProjectPayload(project, &blocks, &error)); QCOMPARE(blocks, before);
    project.objects[0] = lampObject(5); project.objects[0].lamp.extraStates.clear();
    QVERIFY(!buildProjectPayload(project, &blocks, &error)); QCOMPARE(blocks, before);
}

/******************************************************************************
 * Function Name   : functionObject
 * Description     : Create a system-function switch without an operating PLC address.
 * Input Parameters: None
 * Return Value    : Complete single-state function button.
 * Date            : 2026.11.10
 ******************************************************************************/
static HmiObject functionObject() //20261110-0 add
{
    HmiObject object;

    object.id = QStringLiteral("function-1"); object.type = QStringLiteral("switch");
    object.name = QStringLiteral("功能按钮1"); object.text = QStringLiteral("OFF");
    object.geometry = QRect(20, 20, 80, 40);
    object.hasBitButtonProperties = true; object.hasFunctionButtonProperties = true;
    object.bitButton.indicator = false; object.bitButton.dualMonitor = false;
    return object;
}

/******************************************************************************
 * Function Name   : StaticTextTests::functionCreationAndCanvas
 * Description     : Exercise toolbar/palette creation, placement, property cancellation and edit history.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void StaticTextTests::functionCreationAndCanvas() //20261110-0 add
{
    MainWindow window;
    HmiProject empty;
    HmiObject object;
    ScreenCanvas *canvas;
    QAction *action;
    QAction *functionAction;
    QToolButton *button;
    QToolButton *functionPalette;
    QPoint start;
    QPoint end;
    bool opened;
    bool selected;

    functionAction = nullptr; functionPalette = nullptr; opened = false;
    window.show(); QApplication::processEvents(); canvas = window.findChild<ScreenCanvas *>(); canvas->setProject(empty);
    Q_FOREACH (action, window.findChildren<QAction *>())
    {
        if (action->text() == QStringLiteral("功能按钮")) { functionAction = action; }
    }
    Q_FOREACH (button, window.findChildren<QToolButton *>())
    {
        if (button->text() == QStringLiteral("功能按钮") && button->defaultAction() == nullptr) { functionPalette = button; }
    }
    QVERIFY(functionAction != nullptr); QVERIFY(functionPalette != nullptr);
    QTimer::singleShot(0, &window, [&window, &opened]() {
        BitButtonDialog *dialog;

        dialog = window.findChild<BitButtonDialog *>(); opened = dialog != nullptr;
        if (dialog != nullptr)
        {
            QCOMPARE(dialog->windowTitle(), QStringLiteral("功能按钮"));
            dialog->findChild<QComboBox *>(QStringLiteral("FunctionSelection"))->setCurrentIndex(11);
            dialog->findChild<QComboBox *>(QStringLiteral("FunctionType"))->setCurrentIndex(3);
            dialog->findChild<QDialogButtonBox *>()->button(QDialogButtonBox::Ok)->click();
        }
    });
    functionAction->trigger(); QVERIFY(opened); QCOMPARE(canvas->objects().size(), 0);
    start = canvas->mapFromScene(QPointF(100, 80)); end = canvas->mapFromScene(QPointF(240, 160));
    QTest::mousePress(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QTest::mouseMove(canvas->viewport(), end); QTest::mouseRelease(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QCOMPARE(canvas->objects().size(), 1); object = canvas->objects().first();
    QVERIFY(object.hasFunctionButtonProperties); QVERIFY(object.address.isEmpty());
    QCOMPARE(object.functionButton.function, 11); QCOMPARE(object.functionButton.type, 3);
    QVERIFY(object.name.startsWith(QStringLiteral("功能按钮")));
    QTimer::singleShot(0, &window, [&window]() {
        BitButtonDialog *dialog;

        dialog = window.findChild<BitButtonDialog *>(); QVERIFY(dialog != nullptr);
        QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("FunctionType"))->currentIndex(), 3);
        dialog->findChild<QComboBox *>(QStringLiteral("FunctionSelection"))->setCurrentIndex(12); dialog->reject();
    });
    QTest::mouseDClick(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, canvas->mapFromScene(QPointF(150, 110)));
    QCOMPARE(canvas->objects().first().toJson(), object.toJson());
    canvas->executeEdit(QStringLiteral("simulate1")); QCOMPARE(canvas->objects().first().toJson(), object.toJson());
    canvas->executeEdit(QStringLiteral("copy")); canvas->executeEdit(QStringLiteral("paste"));
    QCOMPARE(canvas->objects().size(), 2); QCOMPARE(canvas->selectedObject(&selected).functionButton.toJson(), object.functionButton.toJson()); QVERIFY(selected);
    canvas->executeEdit(QStringLiteral("undo")); QCOMPARE(canvas->objects().size(), 1);
    canvas->executeEdit(QStringLiteral("redo")); QCOMPARE(canvas->objects().size(), 2);
    opened = false;
    QTimer::singleShot(0, &window, [&window, &opened]() {
        BitButtonDialog *dialog;

        dialog = window.findChild<BitButtonDialog *>(); opened = dialog != nullptr;
        if (dialog != nullptr) { dialog->reject(); }
    });
    functionPalette->click(); QVERIFY(opened);
    QTest::mousePress(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QTest::mouseMove(canvas->viewport(), end); QTest::mouseRelease(canvas->viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QCOMPARE(canvas->objects().size(), 2);
}

/******************************************************************************
 * Function Name   : StaticTextTests::functionAddressScanIsolation
 * Description     : Scan only active access conditions and validate notification writes without an operation address.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void StaticTextTests::functionAddressScanIsolation() //20261110-0 add
{
    HmiProject project;
    HmiProject loaded;
    HmiObject object;
    PlcConnection connection;
    DeviceAddressCompiler compiler;
    QByteArray scan;
    QByteArray expected;
    QByteArray before;
    QString error;

    connection.id = QStringLiteral("com1"); connection.channel = PlcChannel::Com1;
    connection.profileId = QStringLiteral("Link/serial_amz/MODBUS/Modbus_RTU.ini");
    connection.station = QStringLiteral("1"); connection.nodeKnown = true; connection.node = 1;
    project.connections = {connection}; object = functionObject(); project.objects = {object};
    QVERIFY2(compiler.prepare(project, &scan, &error), qPrintable(error)); QCOMPARE(scan, QByteArray(200, char(0)));
    object.bitButton.touch.visibilityEnabled = true; object.bitButton.touch.wordEnabled = true;
    object.bitButton.touch.wordAddress = QStringLiteral("@com1:4x10"); object.bitButton.touch.dataType = 3;
    object.bitButton.visibility.visibilityEnabled = true; object.bitButton.visibility.bitEnabled = true;
    object.bitButton.visibility.bitAddress = QStringLiteral("@com1:0x7");
    object.bitButton.notice = true; object.bitButton.noticeWord = true;
    object.bitButton.noticeWordAddress = QStringLiteral("@com1:4x20"); object.bitButton.noticeWordType = 3;
    project.objects = {object};
    QVERIFY2(compiler.prepare(project, &scan, &error), qPrintable(error));
    expected = QByteArray(200, char(0)); expected[0] = 2;
    expected.append(QByteArray::fromHex("4d010001010000000700000044010001020000000a000000"));
    QCOMPARE(scan, expected); before = scan;
    project.objects[0].bitButton.noticeWordAddress = QStringLiteral("LW4096");
    QVERIFY(!compiler.prepare(project, &scan, &error)); QCOMPARE(scan, before);
    project.objects[0] = functionObject(); project.objects[0].address = QStringLiteral("LB0");
    QVERIFY(!project.validatePlcConfiguration(&error)); QVERIFY(!HmiProject::fromJson(project.toJson(), &loaded, &error));
    project.objects[0] = functionObject(); project.objects[0].bitButton.noticeBit = true; project.objects[0].bitButton.noticeWord = true;
    QVERIFY(!project.validatePlcConfiguration(&error)); QVERIFY(!HmiProject::fromJson(project.toJson(), &loaded, &error));
    project.objects[0] = functionObject(); project.objects[0].functionButton.pressedEffect = true;
    project.objects[0].bitButton.states[0].sameLanguage = true; project.objects[0].bitButton.states[1].sameLanguage = false;
    QVERIFY(!project.validatePlcConfiguration(&error)); QVERIFY(!HmiProject::fromJson(project.toJson(), &loaded, &error));
}

/******************************************************************************
 * Function Name   : StaticTextTests::functionDeviceImagesAndFailures
 * Description     : Decode normal/flash/hidden-pressed frames and reject invalid system functions transactionally.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void StaticTextTests::functionDeviceImagesAndFailures() //20261110-0 add
{
    HmiProject project;
    HmiObject object;
    QVector<QByteArray> blocks;
    QVector<QByteArray> before;
    QByteArray decoded;
    QString error;
    std::size_t decodedLength;
    u32 offset;
    u32 compressed;
    int pressed;
    int flash;
    int shared;
    int frames;
    int languages;
    int language;
    int frame;
    int state;

    project.languages = {QStringLiteral("中文"), QStringLiteral("English")};
    for (pressed = 0; pressed < 2; ++pressed)
    {
        for (flash = 0; flash < 2; ++flash)
        {
            for (shared = 0; shared < 2; ++shared)
            {
                object = functionObject(); object.functionButton.pressedEffect = pressed != 0;
                for (state = 0; state < 2; ++state)
                {
                    object.bitButton.states[state].useImage = false;
                    object.bitButton.states[state].sameLanguage = shared != 0;
                    object.bitButton.states[state].sharedText.clear();
                    object.bitButton.states[state].text.languageTexts.clear();
                    object.bitButton.states[state].background = state == 0 ? Qt::red : Qt::green;
                }
                object.bitButton.states[0].flash = flash != 0;
                object.bitButton.states[1].flash = true; // Native function records keep this hidden flash flag inactive.
                object.bitButton.pressTime = 0; object.bitButton.pressTimeEnabled = true;
                project.objects = {object}; error.clear();
                QVERIFY2(buildProjectPayload(project, &blocks, &error), qPrintable(error));
                frames = 1 + pressed + flash; languages = shared ? 1 : 2;
                QCOMPARE(blocks.at(10).size(), 640);
                QCOMPARE(qFromLittleEndian<u16>(blocks.at(10).constData() + 0x106), u16(3));
                QCOMPARE(u8(blocks.at(10)[0x103]), u8(pressed));
                QCOMPARE(u8(blocks.at(10)[0x10b]), u8(0));
                QCOMPARE(u8(blocks.at(10)[0x1bc]), u8(shared));
                QCOMPARE(qFromLittleEndian<u16>(blocks.at(10).constData() + 0x1be), u16(0));
                QCOMPARE(qFromLittleEndian<u32>(blocks.at(5).constData() + 1600), u32(frames * languages));
                QCOMPARE(qFromLittleEndian<u32>(blocks.at(10).constData() + 16), u32(80 * 40 * 2 * frames * languages));
                offset = 0;
                for (language = 0; language < languages; ++language)
                {
                    for (frame = 0; frame < frames; ++frame)
                    {
                        compressed = qFromLittleEndian<u32>(blocks.at(5).constData() + 2400 + (language * frames + frame) * 12 + 8);
                        decoded = QByteArray(80 * 40 * 2, char(0)); decodedLength = 0;
                        QCOMPARE(lzokay::decompress(reinterpret_cast<const u8 *>(blocks.at(11).constData() + offset), compressed,
                            reinterpret_cast<u8 *>(decoded.data()), std::size_t(decoded.size()), decodedLength), lzokay::EResult::Success);
                        QCOMPARE(decodedLength, std::size_t(80 * 40 * 2));
                        QCOMPARE(qFromLittleEndian<u16>(decoded.constData() + (20 * 80 + 2) * 2),
                                 u16(pressed && frame == frames - 1 ? 0x07e0 : 0xf800));
                        offset += qFromLittleEndian<u32>(blocks.at(5).constData() + 2400 + (language * frames + frame) * 12 + 4);
                    }
                }
            }
        }
    }
    before = blocks; project.objects[0].functionButton.function = 18;
    QVERIFY(!buildProjectPayload(project, &blocks, &error)); QCOMPARE(blocks, before);
    project.objects[0] = functionObject(); project.objects[0].bitButton.macroEnabled = true;
    QVERIFY(!buildProjectPayload(project, &blocks, &error)); QCOMPARE(blocks, before);
    project.objects[0] = functionObject(); project.objects[0].bitButton.pressTime = 256;
    QVERIFY(!buildProjectPayload(project, &blocks, &error)); QCOMPARE(blocks, before);
    project.objects[0] = functionObject(); project.objects[0].functionButton.function = 13; project.objects[0].functionButton.type = 2;
    QVERIFY(!buildProjectPayload(project, &blocks, &error)); QCOMPARE(blocks, before);
}

QTEST_MAIN(StaticTextTests)
#include "static_text_tests.moc"
