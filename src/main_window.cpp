#include "main_window.h"
#include "bit_button_renderer.h" //20261108-15 add

#include "hmi_canvas.h"
#include "native_test_project.h" //20261108-7 add
#include "native_project_codec.h" //20261109-0 add
#include "device_macro_compiler.h" //20261109-0 add
#include "stage2_project_dialogs.h" //20261110-7 add
#include <QSettings>
#include <QSaveFile>
#include "static_text_dialog.h" //20261108-14 add
#include <QInputDialog>
#include <QJsonArray> //20261109-0 add
#include <QRegularExpression>

#include <QAction>
#include <QDialogButtonBox> //20261110-7 add
#include <QTextBrowser>
#include <QTabBar>
#include <QFile>
#include <QApplication>
#include <QComboBox>
#include <QCloseEvent>
#include <QCoreApplication> //20261108-8 add
#include <QDir> //20261108-8 add
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo> //20261108-8 add
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit> //20261108-12 add
#include <QMenu>
#include <QMenuBar>
#include <QKeySequence>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPen>
#include <QProcess> //20261108-8 add
#include <QProgressDialog> //20261108-8 add
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyle>
#include <QTabWidget>
#include <QTemporaryDir> //20261108-8 add
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QStringList>
#include <QTimer>
#include <QJsonDocument> //20261108-8 add
#include <QJsonObject> //20261108-8 add

#include <functional> //20261108-8 add

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif

/******************************************************************************
 * Function Name   : paletteIcon
 * Description     : Draw a compact colored control symbol for the vendor-style toolbox grid.
 * Input Parameters: glyph, color
 * Return Value    : A 28 by 28 icon used by one toolbox entry.
 * Date            : 2026.11.08
 ******************************************************************************/
#if 0 //20261111-1 del: Original resource icons replace the earlier generated symbols.
static QIcon paletteIcon(const QString &glyph, const QColor &color) //20261108-7 add
{
    QPixmap image(28, 28);
    QPainter painter;
    QFont font;

    image.fill(Qt::transparent);
    painter.begin(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(color);
    painter.setPen(QPen(color.darker(145), 1.0));
    painter.drawRoundedRect(QRectF(2.5, 3.5, 23.0, 21.0), 3.0, 3.0);
    font = painter.font();
    font.setBold(true);
    font.setPixelSize(glyph.size() > 2 ? 9 : 13);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(image.rect(), Qt::AlignCenter, glyph);
    painter.end();
    return QIcon(image);
}
#endif
#include <windows.h>
#include <setupapi.h>
#endif

/******************************************************************************
 * Function Name   : MainWindow::MainWindow
 * Description     : Construct the editor shell, reference-style docks, and safe device status.
 * Input Parameters: parent
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
MainWindow::MainWindow(QWidget *parent) //20261107-6 add
    : QMainWindow(parent),
      m_dirty(false),
      m_downloadProcess(nullptr), //20261108-8 add
      m_downloadTemp(nullptr), //20261108-8 add
      m_pageTabs(new QTabWidget(this)), //20261108-7 add
      m_canvas(new ScreenCanvas(this)),
      m_projectTree(new QTreeWidget(this)),
      m_leftTabs(new QTabWidget(this)),
      m_rightTabs(new QTabWidget(this)),
      m_nameEdit(nullptr),
      m_textEdit(nullptr),
      m_addressEdit(nullptr),
      m_xSpin(nullptr),
      m_ySpin(nullptr),
      m_widthSpin(nullptr),
      m_heightSpin(nullptr),
      m_connectionLabel(new QLabel(this)),
      m_coordinateLabel(new QLabel(QStringLiteral("X: 0  Y: 0"), this)),
      m_projectLabel(new QLabel(QStringLiteral("800 × 480"), this))
{
    QDockWidget *project_dock;
    QDockWidget *tool_dock;
    QTimer *device_timer;

    setWindowTitle(QStringLiteral("KinSealStudio")); //20261111-1 change
    setMinimumSize(900, 480);
    resize(1353, 708); //20261111-1 change: Match the supplied native client at 96 DPI. //20261108-7 change: Match the measured vendor window dimensions.
    setDockNestingEnabled(true);
    m_pageTabs->addTab(m_canvas, QStringLiteral("画面1"));
    setCentralWidget(m_pageTabs); //20261108-7 change: Preserve the vendor's page tab above the canvas.
    createActionsAndMenus();
    initializeNativeShell(); //20261111-1 add
    createToolbars();

    project_dock = new QDockWidget(QStringLiteral("窗口"), this);
    project_dock->setObjectName(QStringLiteral("ProjectTreeDock"));
    project_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    project_dock->setWidget(m_leftTabs);
    addDockWidget(Qt::LeftDockWidgetArea, project_dock);
    project_dock->setMinimumWidth(150);

    tool_dock = new QDockWidget(QStringLiteral("工具箱"), this);
    tool_dock->setObjectName(QStringLiteral("ToolPropertyDock"));
    tool_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    tool_dock->setWidget(m_rightTabs);
    addDockWidget(Qt::RightDockWidgetArea, tool_dock);
    tool_dock->setMinimumWidth(230);

    m_projectTree->setHeaderHidden(true);
    m_projectTree->setObjectName(QStringLiteral("ProjectObjectTree")); //20261110-7 add
    m_pageTabs->setObjectName(QStringLiteral("ProjectPages"));
    m_projectTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_leftTabs->addTab(createSystemSettingsTree(), QStringLiteral("系统参数"));
    m_leftTabs->addTab(m_projectTree, QStringLiteral("对象列表"));
    m_leftTabs->setTabVisible(1, false); //20261111-1 change: Retain the object navigator without adding a non-native visible tab.
    m_rightTabs->addTab(createToolbox(), QStringLiteral("控件"));
    m_rightTabs->addTab(createPropertyEditor(), QStringLiteral("属性"));
    m_rightTabs->setTabVisible(1, false); //20261111-1 change: Full property dialogs remain the visible editing entry.
    m_rightTabs->addTab(createNativePalette(QStringLiteral("basic")), QStringLiteral("基本控件"));
    createNativeStatusBar(); //20261111-1 change
    resizeDocks({project_dock, tool_dock}, {242, 337}, Qt::Horizontal);

    connect(m_canvas, &ScreenCanvas::objectSelectionChanged,
            this, &MainWindow::updateSelection);
    connect(m_canvas, &ScreenCanvas::pageCoordinatesChanged,
            this, &MainWindow::updateCoordinates);
    connect(m_canvas, &ScreenCanvas::documentChanged,
            this, &MainWindow::addSelectedControl);
    connect(m_canvas, &ScreenCanvas::interactionCancelled, this, [this]() { //20261110-8 add
        m_hasPendingControlResources = false;
        m_pendingControlResources = HmiProject();
    });
    connect(m_canvas, &ScreenCanvas::controlLimitReached, this, [this]() {
        statusBar()->showMessage(QStringLiteral("该机型最多支持 100 个控件。"), 5000);
    });
    connect(m_projectTree, &QTreeWidget::currentItemChanged,
            this, &MainWindow::selectTreeObject);
    connect(m_canvas, &ScreenCanvas::editObjectRequested, this, &MainWindow::editStaticText); //20261108-14 add
    m_canvas->setExternalEditRouting(true); //20261110-7 add: Keys, context menus and menu actions share whole-project undo.
    connect(m_canvas, &ScreenCanvas::editCommandRequested, this, &MainWindow::dispatchEditCommand);
    connect(qApp, &QApplication::focusChanged, this, [this]() { updateCommandStates(); });
    connect(m_pageTabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (!m_updatingNavigation && index >= 0) { activatePageId(m_pageTabs->tabBar()->tabData(index).toString()); }
    });
    connect(m_projectTree, &QTreeWidget::itemDoubleClicked, this, [this]() { editStaticText(); });

    m_project.name = QStringLiteral("触摸屏下载验证");
    m_project.targetModel = QStringLiteral("AMT070W124-L800X480R");
    m_project.pageName = QStringLiteral("画面1");
    m_project.pageWidth = 800;
    m_project.pageHeight = 480;
    m_canvas->setProject(m_project);
    rebuildProjectTree();
    resetProjectHistory(); //20261110-7 add
    updateCommandStates();
    refreshDeviceState();

    device_timer = new QTimer(this);
    device_timer->setInterval(2000);
    connect(device_timer, &QTimer::timeout, this, &MainWindow::refreshDeviceState);
    device_timer->start();
}

/******************************************************************************
 * Function Name   : MainWindow::~MainWindow
 * Description     : Disconnect application-wide focus events before command state and child widgets are destroyed.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
MainWindow::~MainWindow() //20261110-7 add
{
    // QWidget teardown may publish focusChanged after the derived command map has already been destroyed.
    disconnect(qApp, nullptr, this, nullptr);
}

/******************************************************************************
 * Function Name   : MainWindow::createActionsAndMenus
 * Description     : Construct shared command instances and connect all menu aliases to the same transaction.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::createActionsAndMenus() //20261110-7 change
{
    QMenu *file;
    QMenu *edit;
    QMenu *view;
    QMenu *controls;
    QMenu *add;
    QMenu *tools;
    QMenu *help;
    QMenu *align;
    QMenu *layers;
    QAction *action;
    QStringList ids;
    QStringList labels;
    QStringList commands;
    QList<QKeySequence> shortcuts;
    int index;
    auto command = [this](const QString &id, const QString &label, const QKeySequence &shortcut = QKeySequence()) {
        QAction *created;
        created = new QAction(label, this);
        created->setObjectName(id);
        created->setShortcut(shortcut);
        m_commands.insert(id, created);
        return created;
    };

    file = menuBar()->addMenu(QStringLiteral("文件(&F)"));
    edit = menuBar()->addMenu(QStringLiteral("编辑(&E)"));
    view = menuBar()->addMenu(QStringLiteral("视图(&V)"));
    controls = menuBar()->addMenu(QStringLiteral("控件(&K)"));
    add = menuBar()->addMenu(QStringLiteral("添加(&A)"));
    tools = menuBar()->addMenu(QStringLiteral("工具(&T)"));
    help = menuBar()->addMenu(QStringLiteral("帮助(&H)"));
    action = command(QStringLiteral("action_new_pro"), QStringLiteral("新建工程..."), QKeySequence::New);
    action->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    file->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::createProject);
    action = command(QStringLiteral("action_open_pro"), QStringLiteral("打开工程..."), QKeySequence::Open);
    action->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    file->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::openProject);
    action = command(QStringLiteral("action_close"), QStringLiteral("关闭工程"));
    file->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::closeProject);
    action = command(QStringLiteral("action_save"), QStringLiteral("保存工程"), QKeySequence::Save);
    action->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    file->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::saveProject);
    action = command(QStringLiteral("action_save_as"), QStringLiteral("另存为..."), QKeySequence::SaveAs);
    file->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::saveProjectAs);
    action = command(QStringLiteral("action_export_native"), QStringLiteral("导出原厂工程..."));
    file->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::exportNativeTest);
    m_recentProjectsMenu = file->addMenu(QStringLiteral("最近打开的工程"));
    m_recentProjectsMenu->setObjectName(QStringLiteral("RecentProjectsMenu"));
    updateRecentProjects();
    file->addSeparator();
    action = command(QStringLiteral("action_quit"), QStringLiteral("退出"), QKeySequence::Quit);
    file->addAction(action);
    connect(action, &QAction::triggered, this, &QWidget::close);

    ids = {QStringLiteral("action_undo"), QStringLiteral("action_redo"), QStringLiteral("action_cut"),
        QStringLiteral("action_copy"), QStringLiteral("action_paste"), QStringLiteral("action_delete"),
        QStringLiteral("action_checkall"), QStringLiteral("action_copym"), QStringLiteral("action_openpropertiset")};
    labels = {QStringLiteral("撤销"), QStringLiteral("重做"), QStringLiteral("剪切"), QStringLiteral("复制"),
        QStringLiteral("粘贴"), QStringLiteral("删除"), QStringLiteral("全选"), QStringLiteral("多重复制..."), QStringLiteral("属性设置...")};
    commands = {QStringLiteral("undo"), QStringLiteral("redo"), QStringLiteral("cut"), QStringLiteral("copy"),
        QStringLiteral("paste"), QStringLiteral("delete"), QStringLiteral("selectAll"), QStringLiteral("multiCopy"), QStringLiteral("properties")};
    shortcuts = {QKeySequence::Undo, QKeySequence::Redo, QKeySequence::Cut, QKeySequence::Copy, QKeySequence::Paste,
        QKeySequence(Qt::Key_Delete), QKeySequence::SelectAll, QKeySequence(), QKeySequence(QStringLiteral("Ctrl+P"))};
    for (index = 0; index < ids.size(); ++index) {
        action = command(ids.at(index), labels.at(index), shortcuts.at(index));
        action->setProperty("editCommand", commands.at(index));
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        m_canvas->addAction(action);
        edit->addAction(action);
        connect(action, &QAction::triggered, this, [this, commands, index]() { dispatchEditCommand(commands.at(index)); });
    }
    controls->addAction(m_commands.value(QStringLiteral("action_openpropertiset")));
    align = edit->addMenu(QStringLiteral("对齐"));
    layers = edit->addMenu(QStringLiteral("层次"));
    ids = {QStringLiteral("action_left"), QStringLiteral("action_right"), QStringLiteral("action_top"),
        QStringLiteral("action_under"), QStringLiteral("action_vcenter"), QStringLiteral("action_hcenter"),
        QStringLiteral("action_bringtop"), QStringLiteral("action_bringforward"), QStringLiteral("action_sendtoback"),
        QStringLiteral("action_sendbackward"), QStringLiteral("action_group"), QStringLiteral("action_ungroup"), QStringLiteral("action_grid")};
    labels = {QStringLiteral("左对齐"), QStringLiteral("右对齐"), QStringLiteral("顶端对齐"), QStringLiteral("底端对齐"),
        QStringLiteral("水平居中"), QStringLiteral("垂直居中"), QStringLiteral("移到最上层"), QStringLiteral("移到上一层"),
        QStringLiteral("移到最下层"), QStringLiteral("移到下一层"), QStringLiteral("组合"), QStringLiteral("取消组合"), QStringLiteral("网格")};
    commands = {QStringLiteral("alignLeft"), QStringLiteral("alignRight"), QStringLiteral("alignTop"), QStringLiteral("alignBottom"),
        QStringLiteral("alignCenter"), QStringLiteral("alignMiddle"), QStringLiteral("layerTop"), QStringLiteral("layerUp"),
        QStringLiteral("layerBottom"), QStringLiteral("layerDown"), QStringLiteral("group"), QStringLiteral("ungroup"), QStringLiteral("grid")};
    for (index = 0; index < ids.size(); ++index) {
        action = command(ids.at(index), labels.at(index));
        action->setProperty("editCommand", commands.at(index));
        if (index < 6) { align->addAction(action); }
        else if (index < 10) { layers->addAction(action); }
        else { edit->addAction(action); }
        connect(action, &QAction::triggered, this, [this, commands, index]() { dispatchEditCommand(commands.at(index)); });
    }
    //20261110-8 add: The recovered arrangement commands share the same selection and whole-project history routing.
    ids = {QStringLiteral("action_widthsame"), QStringLiteral("action_heightsame"), QStringLiteral("action_widthheightsame"),
           QStringLiteral("action_horizontal"), QStringLiteral("action_vertcal")};
    labels = {QStringLiteral("宽度相同"), QStringLiteral("高度相同"), QStringLiteral("宽度高度都相同"),
              QStringLiteral("水平等间距"), QStringLiteral("垂直等间距")};
    commands = {QStringLiteral("alignWidth"), QStringLiteral("alignHeight"), QStringLiteral("alignSize"),
                QStringLiteral("alignHorizontalSpacing"), QStringLiteral("alignVerticalSpacing")};
    for (index = 0; index < ids.size(); ++index) {
        action = command(ids.at(index), labels.at(index));
        action->setProperty("editCommand", commands.at(index));
        align->addAction(action);
        connect(action, &QAction::triggered, this, [this, commands, index]() { dispatchEditCommand(commands.at(index)); });
    }
    edit->addSeparator();
    ids = {QStringLiteral("action_items_table"), QStringLiteral("action_TextView"), QStringLiteral("action_address_table"), QStringLiteral("action_macro_find")};
    labels = {QStringLiteral("控件一览表..."), QStringLiteral("文本一览表..."), QStringLiteral("地址一览表..."), QStringLiteral("宏指令查找...")};
    for (index = 0; index < ids.size(); ++index) {
        action = command(ids.at(index), labels.at(index));
        edit->addAction(action);
        connect(action, &QAction::triggered, this, [this, index]() { showGlobalTool(index); });
    }
    m_commands.value(QStringLiteral("action_TextView"))->setShortcut(QKeySequence(QStringLiteral("Ctrl+T"))); //20261110-7 add
    m_commands.value(QStringLiteral("action_address_table"))->setShortcut(QKeySequence(QStringLiteral("Ctrl+U")));
    action = command(QStringLiteral("action_address_find"), QStringLiteral("地址查找替换..."), QKeySequence::Find);
    edit->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::replaceProjectAddresses);
    action = command(QStringLiteral("action_project_settings"), QStringLiteral("工程与画面设置..."));
    tools->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::configureProject);
    action = command(QStringLiteral("action_languages"), QStringLiteral("工程语言设置..."));
    tools->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::configureLanguages);
    action = command(QStringLiteral("action_project_pictures"), QStringLiteral("工程图片管理...")); //20261110-8 add
    tools->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::manageProjectPictures);
    action = command(QStringLiteral("action_connections"), QStringLiteral("PLC连接设置..."));
    tools->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::configureConnections);
    action = command(QStringLiteral("action_macro_sdk"), QStringLiteral("ARM宏工具链设置..."));
    tools->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::configureMacroSdk);
    action = command(QStringLiteral("action_detect_device"), QStringLiteral("检测USB下载设备"));
    tools->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::refreshDeviceState);
    action = command(QStringLiteral("action_USBdownload"), QStringLiteral("下载工程..."));
    tools->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::downloadToDevice);
    ids = {QStringLiteral("action_text"), QStringLiteral("action_bitBtn"), QStringLiteral("action_wordbutton"),
        QStringLiteral("action_lamp"), QStringLiteral("action_funbutton")};
    labels = {QStringLiteral("静态文本"), QStringLiteral("位按钮"), QStringLiteral("字按钮"), QStringLiteral("指示灯"), QStringLiteral("功能按钮")};
    for (index = 0; index < ids.size(); ++index) {
        action = command(ids.at(index), labels.at(index));
        add->addAction(action);
        connect(action, &QAction::triggered, this, [this, index]() {
            if (index == 0) { createStaticText(); }
            else if (index == 1) { createBitButton(); }
            else if (index == 2) { createWordButton(); }
            else if (index == 3) { createLamp(); }
            else { createFunctionButton(); }
        });
    }
    action = command(QStringLiteral("action_upscreen"), QStringLiteral("上一幅画面"));
    edit->addAction(action);
    connect(action, &QAction::triggered, this, [this]() {
        int index;
        index = m_project.pageOrder.indexOf(m_project.activePageId);
        if (index > 0) { activatePageId(m_project.pageOrder.at(index - 1)); }
    });
    action = command(QStringLiteral("action_nextscreen"), QStringLiteral("下一幅画面"));
    edit->addAction(action);
    connect(action, &QAction::triggered, this, [this]() {
        int index;
        index = m_project.pageOrder.indexOf(m_project.activePageId);
        if (index + 1 < m_project.pageOrder.size()) { activatePageId(m_project.pageOrder.at(index + 1)); }
    });
    ids = {QStringLiteral("action_42"), QStringLiteral("action_tool"), QStringLiteral("action_40")};
    labels = {QStringLiteral("工程管理器"), QStringLiteral("组态工具箱"), QStringLiteral("标准状态栏")};
    for (index = 0; index < ids.size(); ++index) {
        action = command(ids.at(index), labels.at(index));
        action->setCheckable(true);
        action->setChecked(true);
        view->addAction(action);
        connect(action, &QAction::toggled, this, [this, index](bool checked) {
            QDockWidget *dock;
            if (index == 2) { statusBar()->setVisible(checked); return; }
            dock = findChild<QDockWidget *>(index == 0 ? QStringLiteral("ProjectTreeDock") : QStringLiteral("ToolPropertyDock"));
            if (dock != nullptr) { dock->setVisible(checked); }
        });
    }
    action = command(QStringLiteral("action_H"), QStringLiteral("帮助..."), QKeySequence::HelpContents);
    help->addAction(action);
    connect(action, &QAction::triggered, this, [this]() {
        QDialog dialog(this);
        QVBoxLayout layout(&dialog);
        QTextBrowser browser(&dialog);
        QDialogButtonBox buttons(QDialogButtonBox::Close, &dialog);
        QFile guide(QCoreApplication::applicationDirPath() + QStringLiteral("/docs/阶段2主窗口与工程配置验收.md"));
        dialog.setObjectName(QStringLiteral("Stage2HelpDialog"));
        dialog.setWindowTitle(QStringLiteral("工程与画面工作流帮助"));
        dialog.resize(820, 600);
        if (guide.open(QIODevice::ReadOnly)) { browser.setMarkdown(QString::fromUtf8(guide.readAll())); }
        else { browser.setPlainText(QStringLiteral("使用“工程与画面设置”管理页面和启动设置。\n使用一览表跨页定位，地址替换先预览后确认。\n保存为.ksp；支持范围内的单页工程可导出.sm并下载。\n多页和未恢复配置在下载前给出明确提示。")); }
        layout.addWidget(&browser);
        layout.addWidget(&buttons);
        connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        dialog.exec();
    });
    action = command(QStringLiteral("action_KinSeal"), QStringLiteral("关于 KinSealStudio Lite"));
    help->addAction(action);
    connect(action, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, QStringLiteral("关于"), QStringLiteral("KinSealStudio Lite 0.10.1\n主窗口外观版\n支持的工程与设备功能以随包说明为准。"));
    });
}

/******************************************************************************
 * Function Name   : MainWindow::createToolbars
 * Description     : Build two compact toolbar rows for file operations, control placement, and zoom.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
#if 0 //20261111-1 del: Preserve the earlier simplified toolbar implementation for traceability.
void MainWindow::createToolbars() //20261110-7 change
{
    QToolBar *file;
    QToolBar *config;
    QToolBar *controls;
    QComboBox *zoom;
    QString id;

    file = addToolBar(QStringLiteral("标准工具栏"));
    file->setObjectName(QStringLiteral("StandardToolbar"));
    file->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    Q_FOREACH (id, (QStringList{QStringLiteral("action_new_pro"), QStringLiteral("action_open_pro"),
        QStringLiteral("action_save"), QStringLiteral("action_USBdownload")})) { file->addAction(m_commands.value(id)); }
    addToolBarBreak();
    config = addToolBar(QStringLiteral("工程设置"));
    config->setObjectName(QStringLiteral("ProjectToolbar"));
    config->addAction(m_commands.value(QStringLiteral("action_project_settings")));
    config->addAction(m_commands.value(QStringLiteral("action_detect_device")));
    config->addWidget(new QLabel(QStringLiteral("语言 "), config));
    m_languageCombo = new QComboBox(config);
    m_languageCombo->setObjectName(QStringLiteral("ProjectLanguage"));
    m_languageCombo->addItem(QStringLiteral("中文"));
    config->addWidget(m_languageCombo);
    connect(m_languageCombo, &QComboBox::currentTextChanged, this, [this](const QString &language) {
        if (!language.isEmpty() && m_hasProject && !m_updatingNavigation) {
            m_project.activeLanguage = language;
            m_canvas->setLanguage(language);
            setDirty(true);
        }
    });
    config->addWidget(new QLabel(QStringLiteral("画面 "), config));
    m_pageCombo = new QComboBox(config);
    m_pageCombo->setObjectName(QStringLiteral("ProjectPage"));
    config->addWidget(m_pageCombo);
    connect(m_pageCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (!m_updatingNavigation && index >= 0) { activatePageId(m_pageCombo->itemData(index).toString()); }
    });
    addToolBarBreak();
    controls = addToolBar(QStringLiteral("扩展工具栏"));
    controls->setObjectName(QStringLiteral("ControlToolbar"));
    Q_FOREACH (id, (QStringList{QStringLiteral("action_text"), QStringLiteral("action_bitBtn"),
        QStringLiteral("action_wordbutton"), QStringLiteral("action_lamp"), QStringLiteral("action_funbutton"),
        QStringLiteral("action_openpropertiset")})) { controls->addAction(m_commands.value(id)); }
    zoom = new QComboBox(controls);
    zoom->setObjectName(QStringLiteral("CanvasZoom"));
    zoom->addItems({QStringLiteral("25%"), QStringLiteral("50%"), QStringLiteral("75%"),
        QStringLiteral("100%"), QStringLiteral("150%"), QStringLiteral("200%")});
    zoom->setCurrentText(QStringLiteral("100%"));
    controls->addWidget(new QLabel(QStringLiteral("缩放 "), controls));
    controls->addWidget(zoom);
    connect(zoom, &QComboBox::currentTextChanged, this, [this](const QString &value) {
        m_canvas->setZoomPercent(value.left(value.size() - 1).toInt());
    });
}

#endif

/******************************************************************************
 * Function Name   : MainWindow::createToolbox
 * Description     : Create a small palette of selectable controls that can be placed on the page.
 * Input Parameters: None
 * Return Value    : Toolbox widget with prototype control placement buttons.
 * Date            : 2026.11.07
 ******************************************************************************/
#if 0 //20261111-1 del: Native icons and the complete palette are implemented in native_shell_ui.cpp.
QWidget *MainWindow::createToolbox() //20261107-6 add
{
    QWidget *page;
    QGridLayout *layout;
    QToolButton *button;
    QStringList labels;
    QStringList types;
    QStringList glyphs;
    QList<QColor> colors;
    QString type;
    QString label;
    int button_index;

    page = new QWidget(this);
    layout = new QGridLayout(page);
    layout->setContentsMargins(8, 9, 8, 9);
    layout->setHorizontalSpacing(3);
    layout->setVerticalSpacing(5);
    labels << QStringLiteral("位按钮") << QStringLiteral("字按钮")
           << QStringLiteral("指示灯") << QStringLiteral("功能按钮")
           << QStringLiteral("画面按钮") << QStringLiteral("多态按钮")
           << QStringLiteral("数值显示") << QStringLiteral("ASCII显示")
           << QStringLiteral("时间显示") << QStringLiteral("多功能按钮")
           << QStringLiteral("GIF显示") << QStringLiteral("图片显示")
           << QStringLiteral("动态报警条") << QStringLiteral("历史报警显示")
           << QStringLiteral("报警显示") << QStringLiteral("静态文本")
           << QStringLiteral("消息显示") << QStringLiteral("X-Y曲线")
           << QStringLiteral("趋势图") << QStringLiteral("历史趋势图")
           << QStringLiteral("数据群组");
    types << QStringLiteral("switch") << QStringLiteral("wordSwitch") << QStringLiteral("lamp") //20261109-8 change
          << QStringLiteral("functionSwitch") << QString() << QString() //20261110-0 change
          << QStringLiteral("number") << QString() << QString()
          << QString() << QString() << QStringLiteral("image")
          << QString() << QString() << QString()
          << QStringLiteral("text") << QString() << QString()
          << QString() << QString() << QString();
    glyphs << QStringLiteral("01") << QStringLiteral("10") << QStringLiteral("◉")
           << QStringLiteral("Fn") << QStringLiteral("▦") << QStringLiteral("≡")
           << QStringLiteral("999") << QStringLiteral("ABC") << QStringLiteral("◷")
           << QStringLiteral("✥") << QStringLiteral("GIF") << QStringLiteral("▣")
           << QStringLiteral("▥") << QStringLiteral("▤") << QStringLiteral("!")
           << QStringLiteral("A") << QStringLiteral("✉") << QStringLiteral("XY")
           << QStringLiteral("〽") << QStringLiteral("▥") << QStringLiteral("▤");
    colors << QColor("#36b34a") << QColor("#b844de") << QColor("#4b86e9")
           << QColor("#ffad1f") << QColor("#575757") << QColor("#ff982f")
           << QColor("#2b9ee9") << QColor("#ed9d1d") << QColor("#4d73cd")
           << QColor("#2197d9") << QColor("#4d92be") << QColor("#55a945")
           << QColor("#438cca") << QColor("#2578bd") << QColor("#e63232")
           << QColor("#859aab") << QColor("#3c99d5") << QColor("#1c80d1")
           << QColor("#48addb") << QColor("#82c840") << QColor("#399bcb");
    for (button_index = 0; button_index < labels.size(); ++button_index)
    {
        button = new QToolButton(page);
        button->setText(labels.at(button_index));
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setIcon(paletteIcon(glyphs.at(button_index), colors.at(button_index))); //20261108-7 change
        button->setIconSize(QSize(28, 28));
        button->setFixedSize(99, 62);
        button->setStyleSheet(QStringLiteral(
            "QToolButton { border: 0; background: white; }"
            "QToolButton:hover { border: 1px solid #a8c5e0; background: #eff7ff; }")); //20261108-7 add
        type = types.at(button_index);
        label = labels.at(button_index);
        if (type.isEmpty())
        {
            button->setToolTip(QStringLiteral("该控件暂未纳入本原型的可编辑范围。"));
            connect(button, &QToolButton::clicked, this, [this, label]() {
                statusBar()->showMessage(QStringLiteral("%1 尚未实现。").arg(label), 4000);
            });
        }
        else
        {
            connect(button, &QToolButton::clicked, this, [this, type, label]() {
                if (type == QStringLiteral("text")) //20261108-14 add
                {
                    createStaticText();
                    return;
                }
                if (type == QStringLiteral("switch")) //20261108-15 add
                {
                    createBitButton(); return;
                }
                if (type == QStringLiteral("wordSwitch")) //20261109-6 add: Keep the persistent switch family while dispatching its word subtype.
                {
                    createWordButton(); return;
                }
                if (type == QStringLiteral("lamp")) //20261109-8 add
                {
                    createLamp(); return;
                }
                if (type == QStringLiteral("functionSwitch")) //20261110-0 add
                {
                    createFunctionButton(); return;
                }
                m_canvas->setActiveTool(type);
                statusBar()->showMessage(QStringLiteral("在画面上单击放置 %1 控件。")
                                             .arg(label), 4000);
            });
        }
        layout->addWidget(button, button_index / 3,
                          button_index % 3, Qt::AlignCenter);
    }
    layout->setRowStretch(7, 1);
    return page;
}

#endif

/******************************************************************************
 * Function Name   : MainWindow::createSystemSettingsTree
 * Description     : Present the model and system-parameter sections seen in the vendor editor.
 * Input Parameters: None
 * Return Value    : Non-editable system settings tree for the left dock.
 * Date            : 2026.11.07
 ******************************************************************************/
QWidget *MainWindow::createSystemSettingsTree() //20261110-7 change
{
    QWidget *page; //20261111-1 add
    QVBoxLayout *layout;

    m_systemSettingsTree = new QTreeWidget(this);
    m_systemSettingsTree->setObjectName(QStringLiteral("SystemSettingsTree"));
    m_systemSettingsTree->setHeaderHidden(true);
    m_systemSettingsTree->setStyleSheet(QStringLiteral("QTreeWidget { background: #caeace; border: 1px solid #8e9a8f; }"));
    connect(m_systemSettingsTree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item) {
        QString role;
        QString pageId;
        QString label;
        role = item->data(0, Qt::UserRole).toString();
        pageId = item->data(0, Qt::UserRole + 1).toString();
        label = item->text(0);
        if (role == QStringLiteral("page")) { activatePageId(pageId); }
        else if (role == QStringLiteral("project")) { configureProject(); }
        else if (role == QStringLiteral("system")) { configureSystemSettings(); } //20261110-7 add
        else if (role == QStringLiteral("languages")) { configureLanguages(); }
        else if (role == QStringLiteral("connections")) { configureConnections(); }
        else if (role == QStringLiteral("macros")) { showGlobalTool(3); }
        else if (role == QStringLiteral("unsupported")) {
            QMessageBox::information(this, label, QStringLiteral("此配置尚未开放编辑。现有工程数据会保留；下载前会校验支持范围。"));
        }
    });
    page = new QWidget(this); layout = new QVBoxLayout(page); //20261111-1 change
    layout->setContentsMargins(10, 9, 12, 12); layout->addWidget(m_systemSettingsTree);
    return page;
}

/******************************************************************************
 * Function Name   : MainWindow::createPropertyEditor
 * Description     : Create editable fields for the currently selected prototype object.
 * Input Parameters: None
 * Return Value    : Property editor form widget.
 * Date            : 2026.11.07
 ******************************************************************************/
QWidget *MainWindow::createPropertyEditor() //20261107-6 add
{
    QWidget *page;
    QVBoxLayout *layout;
    QFormLayout *form;
    QPushButton *apply_button;
    QPushButton *full_properties; //20261108-14 add

    page = new QWidget(this);
    layout = new QVBoxLayout(page);
    form = new QFormLayout();
    m_nameEdit = new QLineEdit(page);
    m_textEdit = new QPlainTextEdit(page); //20261108-12 change: Accept multiline Unicode text.
    m_textEdit->setFixedHeight(84);
    m_addressEdit = new QLineEdit(page);
    m_xSpin = new QSpinBox(page);
    m_ySpin = new QSpinBox(page);
    m_widthSpin = new QSpinBox(page);
    m_heightSpin = new QSpinBox(page);
    m_xSpin->setRange(0, 799);
    m_ySpin->setRange(0, 479);
    m_widthSpin->setRange(1, 800);
    m_heightSpin->setRange(1, 480);
    form->addRow(QStringLiteral("名称"), m_nameEdit);
    form->addRow(QStringLiteral("文本"), m_textEdit);
    form->addRow(QStringLiteral("地址"), m_addressEdit);
    form->addRow(QStringLiteral("X"), m_xSpin);
    form->addRow(QStringLiteral("Y"), m_ySpin);
    form->addRow(QStringLiteral("宽"), m_widthSpin);
    form->addRow(QStringLiteral("高"), m_heightSpin);
    apply_button = new QPushButton(QStringLiteral("应用属性"), page);
    apply_button->setEnabled(false);
    apply_button->setObjectName(QStringLiteral("ApplyPropertiesButton"));
    layout->addLayout(form);
    layout->addWidget(apply_button);
    full_properties = new QPushButton(QStringLiteral("完整控件属性..."), page); //20261108-15 change
    layout->addWidget(full_properties);
    connect(full_properties, &QPushButton::clicked, this, &MainWindow::editStaticText);
    layout->addStretch(1);
    connect(apply_button, &QPushButton::clicked,
            this, &MainWindow::applySelectedProperties);
    return page;
}

/******************************************************************************
 * Function Name   : MainWindow::createProject
 * Description     : Start a fresh fixed-resolution project after protecting unsaved edits.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void MainWindow::createProject() //20261110-7 change
{
    HmiProject seed;
    HmiProject candidate;
    QString path;
    QString error;
    Stage2NewProjectDialog dialog(seed, QDir::current().filePath(QStringLiteral("untitled.ksp")), this);

    if (dialog.exec() != QDialog::Accepted) { return; }
    candidate = dialog.draftProject();
    path = dialog.projectPath();
    if (!candidate.validateModel(&error)) {
        QMessageBox::warning(this, QStringLiteral("新建失败"), error);
        return;
    }
    if (QFileInfo::exists(path) && QMessageBox::question(this, QStringLiteral("覆盖工程"),
        QStringLiteral("目标工程已存在，是否覆盖？\n%1").arg(path), QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel) != QMessageBox::Yes) { return; }
    if (!confirmDiscardChanges()) { return; }
    if (!candidate.save(path, &error)) {
        QMessageBox::critical(this, QStringLiteral("新建工程写入失败"), error);
        return;
    }
    setProject(candidate, path);
    updateRecentProjects(path);
}

/******************************************************************************
 * Function Name   : MainWindow::openProject
 * Description     : Select and parse a candidate before changing the current document or its dirty state.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void MainWindow::openProject() //20261110-7 change
{
    QString path;
    QString error;

    path = QFileDialog::getOpenFileName(this, QStringLiteral("打开工程"), m_currentPath,
        QStringLiteral("支持的工程 (*.ksp *.sm);;Lite工程 (*.ksp);;原厂源工程 (*.sm)"));
    if (path.isEmpty()) { return; }
    if (!openProjectPath(path, &error)) {
        if (!error.isEmpty()) { QMessageBox::critical(this, QStringLiteral("打开失败"), error); }
        return;
    }
    if (QFileInfo(path).suffix().compare(QStringLiteral("sm"), Qt::CaseInsensitive) == 0) {
        statusBar()->showMessage(QStringLiteral("已导入原厂源工程；保存请选择独立.ksp文件。"), 10000);
    }
    if (!m_project.nativeSourceWarnings.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("原厂源保全提示"), m_project.nativeSourceWarnings.join(QLatin1Char('\n')));
    }
}

/******************************************************************************
 * Function Name   : MainWindow::configureMacroSdk
 * Description     : Persist a complete user-selected ARM SDK for the editor and hidden download worker.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void MainWindow::configureMacroSdk() //20261109-0 add
{
    QString path;
    QString error;
    QSettings settings(QStringLiteral("KinSealStudioLite"), QStringLiteral("DeviceCompiler"));

    path = QFileDialog::getExistingDirectory(this, QStringLiteral("选择ARM宏工具链目录"), deviceMacroSdkDirectory());
    if (path.isEmpty()) { return; }
    if (!validateDeviceMacroSdkDirectory(path, &error))
    {
        QMessageBox::warning(this, QStringLiteral("工具链配置失败"), error); return;
    }
    settings.setValue(QStringLiteral("armSdkDirectory"), QFileInfo(path).canonicalFilePath());
    settings.sync();
    if (settings.status() != QSettings::NoError)
    {
        QMessageBox::warning(this, QStringLiteral("工具链配置失败"), QStringLiteral("无法保存ARM工具链设置。")); return;
    }
    statusBar()->showMessage(qEnvironmentVariableIsSet("KINSEAL_ARM_SDK") ?
        QStringLiteral("工具链路径已保存；当前KINSEAL_ARM_SDK环境变量优先生效。") :
        QStringLiteral("ARM宏工具链已配置：%1").arg(path), 8000);
}

/******************************************************************************
 * Function Name   : MainWindow::saveProject
 * Description     : Save the current project or choose its first destination.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void MainWindow::saveProject() //20261107-6 add
{
    if (!m_hasProject) { return; } //20261110-7 add
    if (m_currentPath.isEmpty())
    {
        saveProjectAs();
        return;
    }
    writeProject(m_currentPath);
}

/******************************************************************************
 * Function Name   : MainWindow::saveProjectAs
 * Description     : Choose a new destination for the independent .ksp prototype project.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void MainWindow::saveProjectAs() //20261107-6 add
{
    QString path;

    if (!m_hasProject) { return; } //20261110-7 add
    path = QFileDialog::getSaveFileName(this, QStringLiteral("另存为工程"),
        m_currentPath.isEmpty() ? m_project.name + QStringLiteral(".ksp") : m_currentPath,
        QStringLiteral("KinSealStudio Lite 工程 (*.ksp)"));
    if (path.isEmpty())
    {
        return;
    }
    if (QFileInfo(path).suffix().isEmpty()) //20261110-7 change: Never reinterpret an explicit native-source destination as a project save.
    {
        path.append(QStringLiteral(".ksp"));
    }
    writeProject(path);
}

/******************************************************************************
 * Function Name   : MainWindow::exportNativeTest
 * Description     : Save the restricted one-text project in the vendor editor's native source format.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void MainWindow::exportNativeTest() //20261108-7 add
{
    QString path;
    QString error;
    QByteArray bytes; //20261109-0 add
    QSaveFile file;
    bool nativeSource;
    HmiObject object;

    path = QFileDialog::getSaveFileName(this, QStringLiteral("导出原厂工程"), //20261109-0 change
                                        QStringLiteral("CodexProbeTest.sm"),
                                        QStringLiteral("KinSealStudio 原生工程 (*.sm)"));
    if (path.isEmpty())
    {
        return;
    }
    if (!path.endsWith(QStringLiteral(".sm"), Qt::CaseInsensitive))
    {
        path.append(QStringLiteral(".sm"));
    }
    m_canvas->cancelInteraction(); //20261110-8 add: Export committed page geometry only.
    m_project.objects = m_canvas->objects(); //20261109-0 add: Export the current edited geometry and properties.
    nativeSource = !m_project.nativeSourceSnapshot.isEmpty();
    Q_FOREACH (object, m_project.objects) { nativeSource = nativeSource || object.hasBitButtonProperties; }
    if (nativeSource || !exportNativeTestProject(m_project, path, &error)) //20261109-0 change: General text exports can fall back to the typed codec.
    {
        if (!exportNativeProjectSource(m_project, &bytes, &error))
        {
            QMessageBox::warning(this, QStringLiteral("导出失败"), error); return;
        }
        file.setFileName(path);
        if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        {
            QMessageBox::warning(this, QStringLiteral("导出失败"), file.errorString()); return;
        }
    }
    statusBar()->showMessage(QStringLiteral("已导出原厂工程：%1").arg(path), 8000); //20261109-0 change
}

/******************************************************************************
 * Function Name   : MainWindow::addSelectedControl
 * Description     : Refresh model contents from the canvas and mark the document as edited.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void MainWindow::addSelectedControl() //20261110-5 change
{
    HmiProject candidate;
    QString error;
    bool completedPlacement; //20261110-8 add

    candidate = m_project;
    candidate.objects = m_canvas->objects();
    completedPlacement = m_hasPendingControlResources && !m_canvas->hasPendingPlacement();
    if (completedPlacement) { //20261110-8 add
        candidate.macros = m_pendingControlResources.macros;
        candidate.pictures = m_pendingControlResources.pictures;
        candidate.animations = m_pendingControlResources.animations; //20261112-0 add
    }
    if (!candidate.validateModel(&error))
    {
        // A canvas deletion must not leave persisted cross-page references dangling.
        m_hasPendingControlResources = false; //20261110-8 add: Failed final placement drops its resource draft even after canvas flags have cleared.
        m_pendingControlResources = HmiProject();
        m_canvas->setProject(m_project);
        rebuildProjectTree();
        QMessageBox::warning(this, QStringLiteral("工程修改未提交"), error);
        return;
    }
    m_project = candidate;
    if (completedPlacement) { //20261110-8 change: An unrelated edit cannot consume a still-pending placement's resources.
        m_hasPendingControlResources = false;
        m_pendingControlResources = HmiProject();
    }
    rebuildProjectTree();
    setDirty(true);
}

/******************************************************************************
 * Function Name   : MainWindow::applySelectedProperties
 * Description     : Apply property form values to the selected canvas object and tree entry.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void MainWindow::applySelectedProperties() //20261107-6 add
{
    HmiObject object;
    QRect geometry;
    QString controlError; //20261112-0 add
    bool selected;
    int maximum_x;
    int maximum_y;

    object = m_canvas->selectedObject(&selected);
    if (!selected)
    {
        return;
    }
    object.name = m_nameEdit->text();
    object.text = m_textEdit->toPlainText(); //20261108-12 change
    if (object.hasBitButtonProperties) //20261108-15 add: The basic editor represents the OFF-state text.
    {
        if (object.bitButton.states[0].sameLanguage) { object.bitButton.states[0].sharedText = object.text; }
        else { object.bitButton.states[0].text.languageTexts.insert(m_project.activeLanguage, object.text); }
        object.text = bitButtonContent(object.bitButton.states[0], m_project.languages.first());
    }
    if (object.hasStaticTextProperties) //20261108-14 add
    {
        object.staticText.languageTexts.insert(m_project.activeLanguage, object.text);
        object.text = object.staticText.languageTexts.value(m_project.languages.first(), object.text);
    }
    if (object.type == QStringLiteral("multistate")) //20261112-0 add: Preserve per-language OFF text in the independent multi-state record.
    { if (object.multiStateButton.common.states[0].sameLanguage) { object.multiStateButton.common.states[0].sharedText = object.text; } else { object.multiStateButton.common.states[0].text.languageTexts.insert(m_project.activeLanguage, object.text); } object.text = bitButtonContent(object.multiStateButton.common.states[0], m_project.languages.first()); }
    if (object.type == QStringLiteral("screen")) //20261112-0 add: Compact text edits the real OFF state; page targets are never numeric addresses.
    {
        if (object.screenButton.common.states[0].sameLanguage) { object.screenButton.common.states[0].sharedText = object.text; }
        else { object.screenButton.common.states[0].text.languageTexts.insert(m_project.activeLanguage, object.text); }
        object.text = bitButtonContent(object.screenButton.common.states[0], m_project.languages.first());
    }
    object.address = object.type == QStringLiteral("screen") ? QString() : m_addressEdit->text();
    if (object.type == QStringLiteral("multistate")) //20261112-0 add: The compact address edits the full-width monitor only.
    { object.multiStateButton.monitorAddress = object.address.trimmed(); if (object.multiStateButton.sameAddress) { object.multiStateButton.writeAddress = object.multiStateButton.monitorAddress; } object.address.clear(); if (!object.multiStateButton.validateAddresses(m_project.connections, &controlError)) { QMessageBox::warning(this, QStringLiteral("多态按钮属性未提交"), controlError); return; } }
    if (object.type == QStringLiteral("ascii")) //20261112-0 add: ASCII monitor editing validates a base carrier without assuming a recovered string span.
    {
        object.asciiDisplay.common.monitorAddress = object.address.trimmed();
        if (!object.asciiDisplay.validateAddresses(m_project.connections, &controlError)) { QMessageBox::warning(this, QStringLiteral("ASCII属性未提交"), controlError); return; }
    }
    if (object.hasNumberDisplayProperties) //20261112-0 add: The compact address field edits the actual numeric monitor carrier.
    {
        object.numberDisplay.monitorAddress = object.address.trimmed();
        if (!object.numberDisplay.validateAddresses(m_project.connections, &controlError)) { QMessageBox::warning(this, QStringLiteral("数值属性未提交"), controlError); return; }
    }
    if (object.type == QStringLiteral("qrcode")) //20261112-0 add: Keep the compact description and address synchronized with the typed QR record.
    {
        object.qrCode.description = object.text; object.qrCode.monitorAddress = object.address.trimmed();
        if (!object.qrCode.validate(&controlError) || !object.qrCode.validateAddresses(m_project.connections, &controlError))
        { QMessageBox::warning(this, QStringLiteral("二维码属性未提交"), controlError); return; }
    }
    if (object.type == QStringLiteral("message")) //20261112-0 add: The compact property panel edits the first state in the current language.
    {
        object.messageDisplay.states[0].insert(m_project.activeLanguage, object.text);
        object.messageDisplay.monitorAddress = object.address.trimmed();
        object.text = object.messageDisplay.text(0, m_project.languages.first()); object.address = object.messageDisplay.monitorAddress;
        if (!object.messageDisplay.validateAddresses(m_project.connections, &controlError))
        { QMessageBox::warning(this, QStringLiteral("属性无效"), controlError); return; }
    }
    if (object.name.isEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("属性无效"),
                             QStringLiteral("控件名称不能为空。"));
        return;
    }
    geometry = QRect(0, 0, m_widthSpin->value(), m_heightSpin->value());
    maximum_x = qMax(0, m_project.pageWidth - geometry.width());
    maximum_y = qMax(0, m_project.pageHeight - geometry.height());
    geometry.moveTopLeft(QPoint(qBound(0, m_xSpin->value(), maximum_x),
                                qBound(0, m_ySpin->value(), maximum_y)));
    object.geometry = geometry;
    m_canvas->cancelInteraction(); //20261110-8 add: Apply captured property values only after cancelling uncommitted geometry or placement.
    m_canvas->updateSelectedObject(object);
}

/******************************************************************************
 * Function Name   : MainWindow::refreshDeviceState
 * Description     : Query the read-only Windows device list for the vendor USB download VID/PID.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void MainWindow::refreshDeviceState() //20261107-6 add
{
    QString device;

    device = connectedDownloadDevice();
    if (device.isEmpty())
    {
        m_connectionLabel->setText(QStringLiteral("USB 下载：未检测到"));
        m_connectionLabel->setStyleSheet(QStringLiteral("color: #a33;"));
    }
    else
    {
        m_connectionLabel->setText(QStringLiteral("USB 下载：已检测到"));
        m_connectionLabel->setStyleSheet(QStringLiteral("color: #217a39;"));
    }
    m_connectionLabel->setToolTip(
        QStringLiteral("%1\n状态栏仅检查 Windows 设备枚举；下载按钮通过系统 USB 驱动直接传输。") //20261108-10 change
            .arg(device.isEmpty() ? QStringLiteral("未发现 VID/PID 7365:616C") : device));
}

/******************************************************************************
 * Function Name   : MainWindow::downloadToDevice
 * Description     : Compile and transfer the current single-text page in a hidden process.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
void MainWindow::downloadToDevice() //20261108-10 change
{
    QString device;
    QString error;
    QString project_path; //20261108-11 change
    QProcess *process;
    QProgressDialog *progress;
    std::function<void(bool, const QString &)> finalize;
    //20261109-0 del: The background project compiler now validates each enabled button feature.

    if (m_downloadProcess != nullptr)
    {
        QMessageBox::information(this, QStringLiteral("下载中"),
                                 QStringLiteral("当前下载任务仍在进行。"));
        return;
    }

    m_canvas->cancelInteraction(); //20261110-8 add: Never send a move/resize preview to the device.
    m_project.objects = m_canvas->objects();
    if (!m_project.validateSinglePageBackend(&error)) //20261110-5 add: Show the whole-document boundary before starting a worker.
    {
        QMessageBox::warning(this, QStringLiteral("工程暂不支持下载"), error);
        return;
    }
    device = connectedDownloadDevice();
    if (device.isEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("下载不可用"),
                             QStringLiteral("未检测到目标触摸屏的 7365:616C USB 下载设备。"));
        return;
    }

    // 20261108-11 change: Persist unsaved canvas edits into a private .ksp snapshot for the worker.
    m_project.objects = m_canvas->objects();
    m_downloadTemp = new QTemporaryDir(QDir::tempPath() +
                                       QStringLiteral("/KinSealLiteDownload-XXXXXX"));
    if (!m_downloadTemp->isValid())
    {
        delete m_downloadTemp;
        m_downloadTemp = nullptr;
        QMessageBox::critical(this, QStringLiteral("下载失败"),
                              QStringLiteral("无法建立临时下载工程目录。"));
        return;
    }
    project_path = m_downloadTemp->filePath(QStringLiteral("LiteDownload.ksp"));
    if (!m_project.save(project_path, &error)) //20261108-11 change: The independent compiler reads the edited project directly.
    {
        delete m_downloadTemp;
        m_downloadTemp = nullptr;
        QMessageBox::warning(this, QStringLiteral("工程暂不支持下载"), error);
        return;
    }

    process = new QProcess(this);
    progress = new QProgressDialog(QStringLiteral("正在编译工程并通过 USB 下载…"), //20261109-0 change
                                   QString(), 0, 0, this);
    progress->setCancelButton(nullptr);
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);
    progress->show();
    process->setProgram(QCoreApplication::applicationFilePath()); //20261108-10 change
    process->setArguments({QStringLiteral("--download-project"), project_path}); //20261109-0 change
    process->setProcessChannelMode(QProcess::MergedChannels);
#ifdef Q_OS_WIN
    process->setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *arguments) {
        arguments->flags |= CREATE_NO_WINDOW; //20261108-10 change: Keep this executable's downloader process off the user's desktop.
    });
#endif
    m_downloadProcess = process;
    updateCommandStates(); //20261110-7 add: Disable the shared transfer command for the lifetime of this worker.
    finalize = [this, process, progress](bool success, const QString &detail) {
        if (m_downloadProcess != process)
        {
            return;
        }
        m_downloadProcess = nullptr;
        updateCommandStates(); //20261110-7 add: Restore command availability on both transfer success and failure.
        progress->close();
        progress->deleteLater();
        process->deleteLater();
        delete m_downloadTemp;
        m_downloadTemp = nullptr;
        refreshDeviceState();
        if (success)
        {
            statusBar()->showMessage(QStringLiteral("USB传输完成：请核对实物屏显示和触控行为。"), 10000); //20261109-0 change
            QMessageBox::information(this, QStringLiteral("USB 传输完成"), detail); //20261108-10 change
        }
        else
        {
            statusBar()->showMessage(QStringLiteral("USB 下载未完成。"), 10000);
            QMessageBox::critical(this, QStringLiteral("下载失败"), detail);
        }
    };
    connect(process, &QProcess::errorOccurred, this,
            [process, finalize](QProcess::ProcessError process_error) {
                if (process_error == QProcess::FailedToStart)
                {
                    finalize(false, QStringLiteral("无法启动独立 USB 下载进程：%1") //20261108-10 change
                                        .arg(process->errorString()));
                }
            });
    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [process, finalize](int exit_code, QProcess::ExitStatus exit_status) {
                QString process_output;
                QStringList lines;
                QJsonDocument parsed;
                QJsonParseError json_error;
                QString detail;
                bool success;
                int index;

                process_output = QString::fromUtf8(process->readAllStandardOutput()).trimmed();
                lines = process_output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
                for (index = lines.size() - 1; index >= 0; --index)
                {
                    parsed = QJsonDocument::fromJson(lines.at(index).toUtf8(), &json_error);
                    if (json_error.error == QJsonParseError::NoError && parsed.isObject())
                    {
                        break;
                    }
                }
                success = exit_status == QProcess::NormalExit && exit_code == 0 &&
                          parsed.isObject() && parsed.object().value(QStringLiteral("ok")).toBool();
                detail = parsed.isObject()
                             ? parsed.object().value(QStringLiteral("message")).toString()
                             : process_output.right(700);
                if (detail.isEmpty())
                {
                    detail = QStringLiteral("独立下载进程异常退出，退出码 %1。").arg(exit_code);
                }
                finalize(success, detail);
            });
    statusBar()->showMessage(QStringLiteral("正在后台下载到触摸屏…"));
    process->start();
}

/******************************************************************************
 * Function Name   : MainWindow::updateCoordinates
 * Description     : Show the pointer location in the fixed 800 by 480 screen coordinate space.
 * Input Parameters: point
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void MainWindow::updateCoordinates(const QPoint &point) //20261107-6 add
{
    m_coordinateLabel->setText(QStringLiteral("X: %1  Y: %2").arg(point.x()).arg(point.y()));
}

/******************************************************************************
 * Function Name   : MainWindow::updateSelection
 * Description     : Populate the property form and hierarchy when the canvas selection changes.
 * Input Parameters: object, selected
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void MainWindow::updateSelection(const HmiObject &object, bool selected) //20261107-6 add
{
    fillProperties(object, selected);
    rebuildProjectTree();
    updateNativeStatusBar(); //20261111-1 change: Selection updates native status fields without replacing the toolbox tab.
    if (selected) //20261108-14 add: Show the selected rectangle rather than only the pointer coordinate.
    {
        m_coordinateLabel->setText(QStringLiteral("%1  左:%2 上:%3 宽:%4 高:%5")
            .arg(object.type == QStringLiteral("text") ? QStringLiteral("静态文本") : object.type == QStringLiteral("switch") ? QStringLiteral("开关") : object.type) //20261108-15 change
            .arg(object.geometry.x()).arg(object.geometry.y()).arg(object.geometry.width()).arg(object.geometry.height()));
    }
}

/******************************************************************************
 * Function Name   : MainWindow::selectTreeObject
 * Description     : Keep hierarchy selection aligned with canvas selection without recursing.
 * Input Parameters: current, previous
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void MainWindow::selectTreeObject(QTreeWidgetItem *current,
                                  QTreeWidgetItem *previous) //20261110-5 change
{
    QString id;
    QString pageId;
    Q_UNUSED(previous);
    if (current == nullptr)
    {
        return;
    }
    id = current->data(0, Qt::UserRole).toString();
    pageId = current->data(0, Qt::UserRole + 1).toString();
    if (!pageId.isEmpty()) { activatePageId(pageId, id); } //20261110-7 change: Capture IDs before any navigation rebuild invalidates tree pointers.
}

/******************************************************************************
 * Function Name   : MainWindow::rebuildProjectTree
 * Description     : Rebuild the screen/control hierarchy from current canvas objects.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void MainWindow::rebuildProjectTree() //20261110-5 change
{
    QTreeWidgetItem *root_item;
    QTreeWidgetItem *screen_item;
    QTreeWidgetItem *control_item;
    QTreeWidgetItem *selected_item;
    HmiProject snapshot;
    QList<HmiPage> pages;
    HmiPage page;
    int object_index;

    snapshot = m_project;
    snapshot.objects = m_canvas->objects();
    pages = snapshot.allPages();
    selected_item = nullptr;
    m_projectTree->blockSignals(true);
    m_projectTree->clear();
    root_item = new QTreeWidgetItem(m_projectTree,
                                    QStringList() << m_project.name);
    root_item->setData(0, Qt::UserRole, QString());
    Q_FOREACH (page, pages)
    {
        screen_item = new QTreeWidgetItem(root_item, QStringList() << page.name);
        screen_item->setData(0, Qt::UserRole + 1, page.id);
        screen_item->setData(0, Qt::UserRole + 2, QStringLiteral("page"));
        if (page.id == m_project.activePageId) { selected_item = screen_item; }
        for (object_index = 0; object_index < page.objects.size(); ++object_index)
        {
            control_item = new QTreeWidgetItem(screen_item, QStringList() << page.objects.at(object_index).name);
            control_item->setData(0, Qt::UserRole, page.objects.at(object_index).id);
            control_item->setData(0, Qt::UserRole + 1, page.id);
            control_item->setData(0, Qt::UserRole + 2, QStringLiteral("object"));
            if (page.id == m_project.activePageId && page.objects.at(object_index).id == m_selectedId)
            {
                selected_item = control_item;
            }
        }
    }
    m_projectTree->expandAll();
    if (selected_item != nullptr) { m_projectTree->setCurrentItem(selected_item); }
    m_projectTree->blockSignals(false);
    refreshPageNavigation(); //20261110-7 add
    updateCommandStates();
    updateNativeStatusBar(); //20261111-1 add
}

/******************************************************************************
 * Function Name   : MainWindow::setProject
 * Description     : Replace editor state with a loaded or newly created validated project.
 * Input Parameters: project, path
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void MainWindow::setProject(const HmiProject &project, const QString &path) //20261107-6 add
{
    QJsonArray connectionsJson; //20261109-0 add
    PlcConnection connection;

    m_hasPendingControlResources = false; //20261110-8 add: Replacing or restoring a document discards unplaced property drafts.
    m_pendingControlResources = HmiProject();
    m_project = project;
    m_hasProject = true; //20261110-7 add
    m_canvas->setEnabled(true);
    m_rightTabs->setEnabled(true);
    m_pageTabs->setEnabled(true);
    m_systemSettingsTree->setEnabled(true); //20261110-7 add: Reopening a closed document restores configuration navigation.
    Q_FOREACH (connection, project.connections) { connectionsJson.append(connection.toJson()); }
    setProperty("plcConnections", connectionsJson.toVariantList());
    m_currentPath = path;
    m_selectedId.clear();
    m_canvas->setProject(m_project);
    if (m_languageCombo != nullptr) //20261108-14 add
    {
        m_languageCombo->blockSignals(true);
        m_languageCombo->clear();
        m_languageCombo->addItems(m_project.languages);
        m_languageCombo->setCurrentText(m_project.activeLanguage);
        m_languageCombo->blockSignals(false);
    }
    m_projectLabel->setText(QStringLiteral("%1 × %2")
                                .arg(m_project.pageWidth)
                                .arg(m_project.pageHeight));
    rebuildProjectTree();
    fillProperties(HmiObject(), false);
    if (!m_restoringProject) { resetProjectHistory(); } //20261110-7 add
    setDirty(false);
}

/******************************************************************************
 * Function Name   : MainWindow::setDirty
 * Description     : Reflect unsaved project changes in the title and document state.
 * Input Parameters: dirty
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void MainWindow::setDirty(bool dirty) //20261107-6 add
{
    QString title;

    if (dirty) { recordProjectHistory(); } //20261110-7 add
    m_dirty = dirty && m_hasProject && projectIsModified();
    if (!dirty && !m_restoringProject && m_hasProject) { storeSavedCheckpoint(); }
    title = QStringLiteral("KinSealStudio--%1--%2%3") //20261111-1 change
                .arg(m_currentPath.isEmpty() ? m_project.name : QDir::toNativeSeparators(m_currentPath))
                .arg(m_project.pageName)
                .arg(m_dirty ? QStringLiteral(" *") : QString());
    setWindowTitle(title);
    updateNativeStatusBar(); //20261111-1 add: Closing a document clears the visible native fields as well.
    updateCommandStates(); //20261110-7 add
}

/******************************************************************************
 * Function Name   : MainWindow::confirmDiscardChanges
 * Description     : Ask before discarding edits when the document has unsaved changes.
 * Input Parameters: None
 * Return Value    : True when switching documents is safe to continue.
 * Date            : 2026.11.07
 ******************************************************************************/
bool MainWindow::confirmDiscardChanges() //20261107-6 add
{
    QMessageBox::StandardButton response;

    if (!m_dirty)
    {
        return true;
    }
    response = QMessageBox::warning(this, QStringLiteral("未保存的工程"),
        QStringLiteral("是否保存当前工程的修改？"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Cancel); //20261110-7 change
    if (response == QMessageBox::Cancel) { return false; }
    if (response == QMessageBox::Discard) { return true; }
    if (response == QMessageBox::Save) { saveProject(); return !m_dirty; }
    return false;
}

/******************************************************************************
 * Function Name   : MainWindow::writeProject
 * Description     : Synchronize canvas state, save a prototype project, and refresh the title.
 * Input Parameters: path
 * Return Value    : True when the project file was written successfully.
 * Date            : 2026.11.07
 ******************************************************************************/
bool MainWindow::writeProject(const QString &path) //20261107-6 add
{
    QString error;
    HmiProject candidate; //20261110-7 add

    if (!m_hasProject) { return false; } //20261110-7 add
    m_canvas->cancelInteraction(); //20261110-8 add: Saving resolves the interaction before capturing geometry and the clean checkpoint.
    if (QFileInfo(path).suffix().compare(QStringLiteral("ksp"), Qt::CaseInsensitive) != 0) {
        QMessageBox::critical(this, QStringLiteral("保存失败"), QStringLiteral("工程保存目标必须为.ksp，原厂源请使用导出命令。"));
        return false;
    }
    candidate = currentEditorProject();
    if (!candidate.save(path, &error))
    {
        QMessageBox::critical(this, QStringLiteral("保存失败"), error);
        return false;
    }
    m_currentPath = path;
    m_project = candidate; //20261110-7 add: Publish the synchronized records only after the atomic file replacement succeeds.
    updateRecentProjects(path);
    setDirty(false);
    statusBar()->showMessage(QStringLiteral("已保存原型工程：%1").arg(path), 5000);
    return true;
}

/******************************************************************************
 * Function Name   : MainWindow::fillProperties
 * Description     : Populate or clear property fields and retain the selected object identity.
 * Input Parameters: object, selected
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void MainWindow::fillProperties(const HmiObject &object, bool selected) //20261107-6 add
{
    QPushButton *apply_button;
    QList<QPushButton *> buttons;

    if (m_nameEdit == nullptr)
    {
        return;
    }
    m_nameEdit->setEnabled(selected);
    m_textEdit->setEnabled(selected);
    m_addressEdit->setEnabled(selected && !object.hasLampProperties && !object.hasFunctionButtonProperties); //20261110-0 change: Function switches do not expose an operating address.
    if (selected && object.hasNumberDisplayProperties) { m_textEdit->setEnabled(false); } //20261112-0 add
    if (selected && object.type == QStringLiteral("screen")) { m_addressEdit->setEnabled(false); } //20261112-0 add
    if (selected && object.type == QStringLiteral("ascii")) { m_textEdit->setEnabled(false); } //20261112-0 add
    if (selected && (object.type == QStringLiteral("time") || object.type == QStringLiteral("scale") || object.type == QStringLiteral("gif") || object.hasImageDisplayProperties)) { m_textEdit->setEnabled(false); m_addressEdit->setEnabled(false); } //20261112-0 change: Picture rows have no generic text or operation-address field.
    m_xSpin->setEnabled(selected);
    m_ySpin->setEnabled(selected);
    m_widthSpin->setEnabled(selected);
    m_heightSpin->setEnabled(selected);
    buttons = findChildren<QPushButton *>(QStringLiteral("ApplyPropertiesButton"));
    apply_button = buttons.isEmpty() ? nullptr : buttons.first();
    if (apply_button != nullptr)
    {
        apply_button->setEnabled(selected);
    }
    if (selected)
    {
        m_selectedId = object.id;
        m_nameEdit->setText(object.name);
        m_textEdit->setPlainText(object.hasStaticTextProperties ?
            object.staticText.languageTexts.value(m_project.activeLanguage, object.text) : object.text); //20261108-14 change
        if (object.hasBitButtonProperties) //20261108-15 add
        {
            m_textEdit->setPlainText(bitButtonContent(object.bitButton.states[0], m_project.activeLanguage));
        }
        if (object.type == QStringLiteral("multistate")) { m_textEdit->setPlainText(bitButtonContent(object.multiStateButton.common.states[0], m_project.activeLanguage)); } //20261112-0 add
        if (object.type == QStringLiteral("screen")) { m_textEdit->setPlainText(bitButtonContent(object.screenButton.common.states[0], m_project.activeLanguage)); } //20261112-0 add: A compact edit must read the same language that it will update.
        if (object.type == QStringLiteral("qrcode")) { m_textEdit->setPlainText(object.qrCode.description); } //20261112-0 add
        if (object.type == QStringLiteral("message")) { m_textEdit->setPlainText(object.messageDisplay.text(0, m_project.activeLanguage)); } //20261112-0 add
        m_addressEdit->setText(object.address);
        if (object.type == QStringLiteral("multistate")) { m_addressEdit->setText(object.multiStateButton.monitorAddress); } //20261112-0 add
        if (object.type == QStringLiteral("ascii")) { m_addressEdit->setText(object.asciiDisplay.common.monitorAddress); } //20261112-0 add
        if (object.hasNumberDisplayProperties) { m_addressEdit->setText(object.numberDisplay.monitorAddress); } //20261112-0 add
        if (object.type == QStringLiteral("qrcode")) { m_addressEdit->setText(object.qrCode.monitorAddress); } //20261112-0 add
        if (object.type == QStringLiteral("message")) { m_addressEdit->setText(object.messageDisplay.monitorAddress); } //20261112-0 add
        m_xSpin->setValue(object.geometry.x());
        m_ySpin->setValue(object.geometry.y());
        m_widthSpin->setValue(object.geometry.width());
        m_heightSpin->setValue(object.geometry.height());
    }
    else
    {
        m_selectedId.clear();
        m_nameEdit->clear();
        m_textEdit->clear();
        m_addressEdit->clear();
        m_xSpin->setValue(0);
        m_ySpin->setValue(0);
        m_widthSpin->setValue(1);
        m_heightSpin->setValue(1);
    }
}

/******************************************************************************
 * Function Name   : MainWindow::connectedDownloadDevice
 * Description     : Find the vendor's present USB device by its documented VID/PID without claiming it.
 * Input Parameters: None
 * Return Value    : Friendly name and identifier, or an empty string when absent or unsupported.
 * Date            : 2026.11.07
 ******************************************************************************/
QString MainWindow::connectedDownloadDevice() const //20261107-6 add
{
#ifdef Q_OS_WIN
    HDEVINFO device_set;
    SP_DEVINFO_DATA device_data;
    DWORD device_index;
    WCHAR instance_id[512];
    DWORD required_size;
    QString identifier;
    QString result;

    device_set = SetupDiGetClassDevsW(nullptr, L"USB", nullptr,
                                      DIGCF_PRESENT | DIGCF_ALLCLASSES);
    if (device_set == INVALID_HANDLE_VALUE)
    {
        return QString();
    }
    ZeroMemory(&device_data, sizeof(device_data));
    device_data.cbSize = sizeof(device_data);
    device_index = 0;
    while (SetupDiEnumDeviceInfo(device_set, device_index, &device_data))
    {
        required_size = 0;
        if (SetupDiGetDeviceInstanceIdW(device_set, &device_data, instance_id,
                                        static_cast<DWORD>(sizeof(instance_id) / sizeof(WCHAR)),
                                        &required_size))
        {
            identifier = QString::fromWCharArray(instance_id);
            if (identifier.contains(QStringLiteral("VID_7365&PID_616C"),
                                   Qt::CaseInsensitive))
            {
                result = QStringLiteral("%1 (%2)")
                             .arg(QStringLiteral("Smart Control Device"), identifier);
                break;
            }
        }
        ++device_index;
        ZeroMemory(&device_data, sizeof(device_data));
        device_data.cbSize = sizeof(device_data);
    }
    SetupDiDestroyDeviceInfoList(device_set);
    return result;
#else
    return QString();
#endif
}

/******************************************************************************
 * Function Name   : MainWindow::closeEvent
 * Description     : Preserve unsaved prototype edits when the editor window is closed.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.07
 ******************************************************************************/
void MainWindow::closeEvent(QCloseEvent *event) //20261107-6 add
{
    if (m_downloadProcess != nullptr) //20261108-8 change: Keep the private transfer owner alive until the bridge exits.
    {
        QMessageBox::information(this, QStringLiteral("下载进行中"),
                                 QStringLiteral("请等待当前 USB 下载任务结束。"));
        event->ignore();
        return;
    }
    if (confirmDiscardChanges())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}
