"""Test the pure-image table against native field bounds and eight independently compiled switch states."""
from pathlib import Path
p=Path(__file__).resolve().parent/'worktree/tests/stage4_controls_tests.cpp';s=p.read_text(encoding='utf-8');assert 'tableNativeFieldsCountsAndEnabledDrafts' not in s
methods=[
('tableNativeFieldsCountsAndEnabledDrafts','Keep the native h-to-rows and V-to-columns mapping, all field bounds and enabled drafts when all three switches are off.',r'''    HmiObject object;
    GraphicDialog *dialog;
    QSpinBox *spin;
    QPushButton *button;
    object.type = QStringLiteral("table"); object.graphic.background = QColor(QStringLiteral("#e0e0e0")); dialog = new GraphicDialog(object); dialog->show(); QVERIFY(QTest::qWaitForWindowExposed(dialog)); QCOMPARE(dialog->windowTitle(),QStringLiteral("表格")); QCOMPARE(dialog->findChild<QTabWidget *>(QStringLiteral("TableAppearanceTabs"))->tabText(0),QStringLiteral("外观设置")); QCOMPARE(dialog->findChildren<QSpinBox *>().size(),3); QCOMPARE(dialog->findChild<QSpinBox *>(QStringLiteral("m_hnumberspin"))->minimum(),1); QCOMPARE(dialog->findChild<QSpinBox *>(QStringLiteral("m_hnumberspin"))->maximum(),30); QCOMPARE(dialog->findChild<QSpinBox *>(QStringLiteral("m_Vnumberspin"))->maximum(),30); QCOMPARE(dialog->findChild<QComboBox *>(QStringLiteral("m_linetypecmb"))->count(),6); QVERIFY(dialog->findChild<QComboBox *>(QStringLiteral("Line_Width"))==nullptr); QVERIFY(dialog->findChild<QTableWidget *>()==nullptr);
    dialog->findChild<QSpinBox *>(QStringLiteral("m_hnumberspin"))->setValue(30); dialog->findChild<QSpinBox *>(QStringLiteral("m_Vnumberspin"))->setValue(1); dialog->findChild<QCheckBox *>(QStringLiteral("m_hcheck"))->setChecked(false); dialog->findChild<QCheckBox *>(QStringLiteral("m_vcheck"))->setChecked(false); dialog->findChild<QCheckBox *>(QStringLiteral("m_bordercheck"))->setChecked(false); QCOMPARE(dialog->object().graphic.rows,30); QCOMPARE(dialog->object().graphic.columns,1); Q_FOREACH (spin,dialog->findChildren<QSpinBox *>()) { QVERIFY(spin->isEnabled()); } Q_FOREACH (button,dialog->findChildren<QPushButton *>()) { QVERIFY(button->isEnabled()); } QVERIFY(dialog->findChild<QComboBox *>(QStringLiteral("m_linetypecmb"))->isEnabled()); QCOMPARE(dialog->object().graphic.background,QColor(QStringLiteral("#e0e0e0"))); dialog->reject(); delete dialog;
'''),
('tableStrictFamilyJsonAndAtomicBounds','Reject malformed counts, non-boolean flags and cell/width/binding keys without replacing the valid table draft.',r'''    GraphicProperties properties;
    GraphicProperties parsed;
    QJsonObject json;
    QJsonObject original;
    QString error;
    int invalid;
    QString key;
    properties.background = QColor(QStringLiteral("#e0e0e0")); original = properties.toJson(QStringLiteral("table")); QVERIFY(GraphicProperties::fromJson(original,&parsed,&error,QStringLiteral("table"))); QVERIFY(!original.contains(QStringLiteral("lineWidth"))); QVERIFY(!original.contains(QStringLiteral("fillStyle"))); QVERIFY(!original.contains(QStringLiteral("points")));
    Q_FOREACH (invalid,(QList<int>{0,31,-1})) { json = original; json.insert(QStringLiteral("rows"),invalid); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,QStringLiteral("table"))); QCOMPARE(parsed.toJson(QStringLiteral("table")),original); }
    json = original; json.insert(QStringLiteral("columns"),1.5); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,QStringLiteral("table"))); json = original; json.insert(QStringLiteral("horizontal"),0); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,QStringLiteral("table")));
    Q_FOREACH (key,(QStringList{QStringLiteral("cells"),QStringLiteral("lineWidth"),QStringLiteral("address"),QStringLiteral("radiusX")})) { json = original; json.insert(key,1); QVERIFY(!GraphicProperties::fromJson(json,&parsed,&error,QStringLiteral("table"))); QCOMPARE(parsed.toJson(QStringLiteral("table")),original); }
    QVERIFY(!GraphicProperties::fromJson(original,&parsed,&error,QStringLiteral("rect"))); properties.lineWidth = 2; QVERIFY(!properties.validate(QStringLiteral("table"),&error));
'''),
('tableEightNativeSwitchRasterMasks','Compare all eight actual native grid/frame switch bitmaps by their opaque black line masks, independently of RGB565 fill quantization.',r'''    GraphicProperties properties;
    QImage image(800,480,QImage::Format_ARGB32);
    QImage native;
    QPainter painter;
    QString manifest = QFINDTESTDATA("../docs/stage4-evidence/table/native/flags-investigation/compiled-observations.json");
    QDir directory(QFileInfo(manifest).absolutePath());
    int mask;
    int x;
    int y;
    int different;
    QVERIFY(!manifest.isEmpty()); properties.background = QColor(QStringLiteral("#e0e0e0"));
    for (mask=0;mask<8;++mask) { properties.showHorizontal = (mask&1)!=0; properties.showVertical = (mask&2)!=0; properties.showOutline = (mask&4)!=0; image.fill(QColor(197,222,255)); painter.begin(&image); paintGraphic(&painter,QStringLiteral("table"),QRectF(100,100,200,100),properties); painter.end(); native = QImage(directory.filePath(QStringLiteral("sheet-table-flags-%1-r1.png").arg(mask))); QVERIFY(!native.isNull()); different = 0; for (y=90;y<211;++y) { for (x=90;x<311;++x) { if ((image.pixelColor(x,y)==QColor(Qt::black))!=(native.pixelColor(x,y)==QColor(Qt::black))) { ++different; } } } QCOMPARE(different,0); if (mask==7 && !qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(image.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("table-default-raster.png")))); } }
'''),
('tableBackgroundOnlyAlphaAndTransparentGridFrame','Keep independent grid/frame pens opaque when the background alpha is zero or partial, including native line-style-zero frame behavior.',r'''    GraphicProperties properties;
    QImage image(800,480,QImage::Format_ARGB32);
    QPainter painter;
    properties.background = Qt::red; properties.lineColor = Qt::blue; properties.frameColor = Qt::green; properties.alpha = 0; image.fill(Qt::white); painter.begin(&image); paintGraphic(&painter,QStringLiteral("table"),QRectF(100,100,200,100),properties); painter.end(); QCOMPARE(image.pixelColor(125,112),QColor(Qt::white)); QCOMPARE(image.pixelColor(200,112),QColor(Qt::blue)); QCOMPARE(image.pixelColor(100,112),QColor(Qt::green)); properties.alpha = 128; properties.lineStyle = 0; image.fill(Qt::white); painter.begin(&image); paintGraphic(&painter,QStringLiteral("table"),QRectF(100,100,200,100),properties); painter.end(); QCOMPARE(image.pixelColor(100,112),QColor(Qt::green)); QVERIFY(image.pixelColor(200,112)!=QColor(Qt::blue)); QCOMPARE(image.pixelColor(125,112).red(),255); QVERIFY(qAbs(image.pixelColor(125,112).green()-127)<=1); properties.showOutline = false; properties.showHorizontal = false; properties.showVertical = false; image.fill(Qt::white); painter.begin(&image); paintGraphic(&painter,QStringLiteral("table"),QRectF(100,100,200,100),properties); painter.end(); QVERIFY(image.pixelColor(100,112)!=QColor(Qt::green));
'''),
('tableNativeThreeRowsTwoColumnsAndOneCell','Match the named native 3-by-2 grid positions and remove internal lines at the 1-by-1 minimum.',r'''    GraphicProperties properties;
    QImage image(800,480,QImage::Format_ARGB32);
    QImage native(QFINDTESTDATA("../docs/stage4-evidence/table/native/sheet-table-custom-three-by-two-r2.png"));
    QPainter painter;
    QVERIFY(!native.isNull()); properties.background = QColor(QStringLiteral("#e0e0e0")); properties.rows = 3; properties.columns = 2; image.fill(Qt::white); painter.begin(&image); paintGraphic(&painter,QStringLiteral("table"),QRectF(100,100,200,100),properties); painter.end(); QCOMPARE(native.pixelColor(125,133),QColor(Qt::black)); QCOMPARE(image.pixelColor(125,133),QColor(Qt::black)); QCOMPARE(native.pixelColor(200,112),QColor(Qt::black)); QCOMPARE(image.pixelColor(200,112),QColor(Qt::black)); QVERIFY(native.pixelColor(150,112)!=QColor(Qt::black)); QVERIFY(image.pixelColor(150,112)!=QColor(Qt::black)); properties.rows = properties.columns = 1; image.fill(Qt::white); painter.begin(&image); paintGraphic(&painter,QStringLiteral("table"),QRectF(100,100,200,100),properties); painter.end(); QCOMPARE(image.pixelColor(200,150),properties.background); QCOMPARE(image.pixelColor(100,150),QColor(Qt::black));
'''),
('tableKspMixedCopyAndNativeRefusal','Persist fourteen mixed families while preserving the pure-image table flags, colors and source/device refusal.',r'''    HmiProject project;
    HmiProject loaded;
    HmiProject pasted;
    HmiObject object;
    Stage3Clipboard clipboard;
    QString error;
    QString type;
    QStringList types = {QStringLiteral("line"),QStringLiteral("rect"),QStringLiteral("ellipse"),QStringLiteral("roundrect"),QStringLiteral("polyline"),QStringLiteral("polygon"),QStringLiteral("freeline"),QStringLiteral("sector")};
    QByteArray native = QByteArrayLiteral("keep");
    QVector<QByteArray> payloads = {QByteArrayLiteral("keep")};
    object.type = QStringLiteral("table"); object.id = QStringLiteral("table-model"); object.name = QStringLiteral("表格"); object.geometry = QRect(100,100,200,100); object.graphic.background = QColor(QStringLiteral("#e0e0e0")); project.objects.append(object); QVERIFY2(HmiProject::fromJson(project.toJson(),&loaded,&error),qPrintable(error)); QCOMPARE(loaded.toJson(),project.toJson()); QVERIFY(captureStage3Selection(project,{object.id},&clipboard,&error)); QVERIFY(pasteStage3Selection(HmiProject(),clipboard,QPoint(20,20),&pasted,nullptr,&error)); QCOMPARE(pasted.objects.first().graphic.toJson(object.type),object.graphic.toJson(object.type)); QVERIFY(!exportNativeProjectSource(project,&native,&error)); QCOMPARE(native,QByteArrayLiteral("keep")); QVERIFY(!buildProjectPayload(project,&payloads,&error)); QCOMPARE(payloads,QVector<QByteArray>{QByteArrayLiteral("keep")}); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(project.save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("table-only.ksp")),&error)); }
    Q_FOREACH (type,types) { object.type = type; object.id = QStringLiteral("mixed-")+type; object.graphic = GraphicProperties(); if (type==QStringLiteral("polygon")) { object.graphic.points = {QPointF(0,0),QPointF(1,0),QPointF(.75,1)}; } project.objects.append(object); } project.objects.append(existingFiveObjects()); QVERIFY2(HmiProject::fromJson(project.toJson(),&loaded,&error),qPrintable(error)); QCOMPARE(loaded.objects.size(),14); QCOMPARE(loaded.toJson(),project.toJson());
'''),
('tablePropertyFirstMenuCancelPlacementAndUndo','Open native table appearance before placement and retain a single creation/edit history transaction with canceled drafts isolated.',r'''    MainWindow window;
    QJsonObject before;
    window.setProject(HmiProject(),QString()); window.show(); QVERIFY(QTest::qWaitForWindowExposed(&window)); QTimer::singleShot(0,[&window]() { GraphicDialog *dialog = qobject_cast<GraphicDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog!=nullptr); QVERIFY(!window.m_canvas->hasPendingPlacement()); QVERIFY(window.currentEditorProject().objects.isEmpty()); dialog->reject(); }); window.m_commands.value(QStringLiteral("action_table"))->trigger(); QVERIFY(!window.m_canvas->hasPendingPlacement()); QVERIFY(window.currentEditorProject().objects.isEmpty());
    QTimer::singleShot(0,[&window]() { GraphicDialog *dialog = qobject_cast<GraphicDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog!=nullptr); QVERIFY(!window.m_canvas->hasPendingPlacement()); if (!qEnvironmentVariableIsEmpty("KINSEAL_STAGE4_CAPTURE_DIR")) { QVERIFY(dialog->grab().save(QDir(qEnvironmentVariable("KINSEAL_STAGE4_CAPTURE_DIR")).filePath(QStringLiteral("table-default-properties.png")))); } dialog->findChild<QDialogButtonBox *>(QStringLiteral("GraphicButtons"))->button(QDialogButtonBox::Ok)->click(); }); window.m_commands.value(QStringLiteral("action_table"))->trigger(); QVERIFY(window.m_canvas->hasPendingPlacement()); QVERIFY(window.currentEditorProject().objects.isEmpty()); QTest::mousePress(window.m_canvas->viewport(),Qt::LeftButton,Qt::NoModifier,window.m_canvas->mapFromScene(QPointF(100,100))); QTest::mouseMove(window.m_canvas->viewport(),window.m_canvas->mapFromScene(QPointF(300,200))); QTest::mouseRelease(window.m_canvas->viewport(),Qt::LeftButton,Qt::NoModifier,window.m_canvas->mapFromScene(QPointF(300,200))); QCOMPARE(window.currentEditorProject().objects.size(),1); QVERIFY(!window.m_textEdit->isEnabled()); QVERIFY(!window.m_addressEdit->isEnabled()); before = window.currentEditorProject().toJson();
    QTimer::singleShot(0,[]() { GraphicDialog *dialog = qobject_cast<GraphicDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog!=nullptr); dialog->findChild<QSpinBox *>(QStringLiteral("m_hnumberspin"))->setValue(3); dialog->findChild<QSpinBox *>(QStringLiteral("m_Vnumberspin"))->setValue(2); dialog->findChild<QSpinBox *>(QStringLiteral("m_TransparentspBox"))->setValue(128); dialog->findChild<QDialogButtonBox *>(QStringLiteral("GraphicButtons"))->button(QDialogButtonBox::Ok)->click(); }); QTest::mouseDClick(window.m_canvas->viewport(),Qt::LeftButton,Qt::NoModifier,window.m_canvas->mapFromScene(QPointF(200,150))); QCOMPARE(window.currentEditorProject().objects.first().graphic.rows,3); QCOMPARE(window.currentEditorProject().objects.first().graphic.columns,2);
    QTimer::singleShot(0,[]() { GraphicDialog *dialog = qobject_cast<GraphicDialog *>(QApplication::activeModalWidget()); QVERIFY(dialog!=nullptr); dialog->findChild<QSpinBox *>(QStringLiteral("m_hnumberspin"))->setValue(30); dialog->reject(); }); window.editStage4Control(); QCOMPARE(window.currentEditorProject().objects.first().graphic.rows,3); window.restoreProjectHistory(-1); QCOMPARE(window.currentEditorProject().toJson(),before); window.restoreProjectHistory(-1); QVERIFY(window.currentEditorProject().objects.isEmpty());
'''),
('tableEightResizeHandlesAndDraftCancel','Preserve rows, columns and switches through all eight box handles and cancel an additional unfinished placement.',r'''    ScreenCanvas canvas;
    HmiProject project;
    HmiObject object;
    QPointF handle;
    int index;
    QSignalSpy changed(&canvas,&ScreenCanvas::documentChanged);
    object.type = QStringLiteral("table"); object.id = QStringLiteral("table-resize"); object.name = QStringLiteral("表格"); object.geometry = QRect(100,100,200,100); object.graphic.background = QColor(QStringLiteral("#e0e0e0")); object.graphic.rows = 3; object.graphic.columns = 2; object.graphic.showVertical = false; project.objects.append(object); canvas.resize(900,550); canvas.show(); QVERIFY(QTest::qWaitForWindowExposed(&canvas));
    for (index=0;index<9;++index) { if (index==4) { continue; } canvas.setProject(project); canvas.selectObject(object.id); changed.clear(); handle = QPointF(100+(index%3)*100,100+(index/3)*50); QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(handle)); QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(handle+QPointF(8,8))); QTest::mouseRelease(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(handle+QPointF(8,8))); QCOMPARE(changed.size(),1); QCOMPARE(canvas.objects().first().graphic.toJson(object.type),object.graphic.toJson(object.type)); }
    canvas.beginControlPlacement(object); QTest::mousePress(canvas.viewport(),Qt::LeftButton,Qt::NoModifier,canvas.mapFromScene(QPointF(400,300))); QTest::mouseMove(canvas.viewport(),canvas.mapFromScene(QPointF(500,400))); canvas.cancelInteraction(); QCOMPARE(canvas.objects().size(),1); QVERIFY(!canvas.hasActiveInteraction());
''')]
s=s.replace('    void initTestCase();',''.join('    void '+n+'();\n' for n,_,_ in methods)+'    void initTestCase();',1)
defs=''
for n,d,b in methods:defs+='\n/******************************************************************************\n * Function Name   : Stage4ControlTests::'+n+'\n * Description     : '+d+'\n * Input Parameters: None\n * Return Value    : None.\n * Date            : 2026.11.13\n ******************************************************************************/\nvoid Stage4ControlTests::'+n+'() //20261113-8 add\n{\n'+b+'}\n'
s=s.replace('QTEST_MAIN(Stage4ControlTests)',defs+'\nQTEST_MAIN(Stage4ControlTests)',1);p.write_text(s,encoding='utf-8',newline='\n');print('Added eight native-grounded table regressions.')
