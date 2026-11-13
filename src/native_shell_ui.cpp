#include "main_window.h"
#include "graphic_dialog.h" //20261113-8 add
#include "hmi_canvas.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDockWidget>
#include <QDir>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QPainter>
#include <QGridLayout>
#include <QGraphicsScene>
#include <QSignalBlocker>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QStatusBar>
#include <QStyleFactory>
#include <QSet>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <functional>

/******************************************************************************
 * Function Name   : nativeShellLayout
 * Description     : Load the retained native layout without depending on an external installation.
 * Input Parameters: None
 * Return Value    : The immutable resource manifest captured from the original editor.
 * Date            : 2026.11.11
 ******************************************************************************/
static QJsonObject nativeShellLayout() //20261111-1 add
{
    QFile file(QStringLiteral(":/native-shell/layout.json"));

    if (!file.open(QIODevice::ReadOnly)) { return QJsonObject(); }
    return QJsonDocument::fromJson(file.readAll()).object();
}

/******************************************************************************
 * Function Name   : nativeShellIcon
 * Description     : Resolve an original icon exported with its alpha channel intact.
 * Input Parameters: path
 * Return Value    : The original icon, or an empty icon for original text-only entries.
 * Date            : 2026.11.11
 ******************************************************************************/
static QIcon nativeShellIcon(const QString &path) //20261111-1 add
{
    return path.isEmpty() ? QIcon() : QIcon(QStringLiteral(":/native-shell/") + path);
}

/******************************************************************************
 * Function Name   : nativeDockTitle
 * Description     : Keep classic dock caption controls at the original size independently of the host Qt style.
 * Input Parameters: dock
 * Return Value    : None
 * Date            : 2026.11.11
 ******************************************************************************/
static void nativeDockTitle(QDockWidget *dock) //20261111-1 add
{
    QWidget *caption;
    QHBoxLayout *layout;
    QLabel *label;
    QToolButton *floating;
    QToolButton *close;

    caption = new QWidget(dock); caption->setObjectName(QStringLiteral("NativeDockCaption")); caption->setFixedHeight(20);
    caption->setStyleSheet(QStringLiteral("QWidget#NativeDockCaption { border-top: 1px solid #999; border-bottom: 1px solid #999; background: #f0f0f0; } QToolButton { border: 0px; padding: 0px; background: transparent; } QToolButton:hover { background: #d6e6f6; }"));
    layout = new QHBoxLayout(caption); layout->setContentsMargins(2, 0, 2, 0); layout->setSpacing(2);
    label = new QLabel(dock->windowTitle(), caption); layout->addWidget(label, 1);
    floating = new QToolButton(caption); floating->setIcon(nativeShellIcon(QStringLiteral("icons/dock-float.svg")));
    floating->setIconSize(QSize(9, 9)); floating->setFixedSize(12, 12); floating->setToolTip(QStringLiteral("浮动"));
    close = new QToolButton(caption); close->setIcon(nativeShellIcon(QStringLiteral("icons/dock-close.svg")));
    close->setIconSize(QSize(8, 8)); close->setFixedSize(12, 12); close->setToolTip(QStringLiteral("关闭"));
    layout->addWidget(floating); layout->addWidget(close); dock->setTitleBarWidget(caption);
    QObject::connect(close, &QToolButton::clicked, dock, &QWidget::hide);
    QObject::connect(floating, &QToolButton::clicked, dock, [dock]() { dock->setFloating(!dock->isFloating()); });
}

/******************************************************************************
 * Function Name   : MainWindow::initializeNativeShell
 * Description     : Restore the native shell while preserving existing command handlers and unavailable-feature boundaries.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.11
 ******************************************************************************/
void MainWindow::initializeNativeShell() //20261111-1 add
{
    QJsonObject layout;
    QJsonObject spec;
    QJsonObject entry;
    QJsonArray menus;
    QJsonValue value;
    QAction *action;
    QMenu *menu;
    QMenu *extra;
    QString id;
    QString label;
    QSet<QAction *> placed;
    QIcon original;
    QJsonObject::const_iterator it;
    std::function<void(QMenu *, const QJsonArray &)> populate;

    setObjectName(QStringLiteral("NativeEditorShell"));
    setFont(QFont(QStringLiteral("SimSun"), 9));
    setWindowIcon(nativeShellIcon(QStringLiteral("icons/windowIcon.png")));
    setDockOptions(QMainWindow::AllowTabbedDocks | QMainWindow::AllowNestedDocks);
    setStyleSheet(QStringLiteral(
        "QMainWindow#NativeEditorShell { background: #f0f0f0; }"
        "QMainWindow#NativeEditorShell::separator { width: 4px; height: 4px; background: #f0f0f0; }"
        "QMenuBar { background: qlineargradient(x1:0,y1:1,x2:0,y2:0,stop:0 #e9eff9,stop:1 #bfd6f3); color: #082d69; spacing: 0px; }"
        "QMenuBar::item { padding: 2px 9px; }"
        "QToolBar { background: #f0f0f0; border: 0px; border-bottom: 1px solid #a0a0a0; spacing: 0px; padding: 0px; }"
        "QToolBar QToolButton { min-width: 24px; min-height: 24px; padding: 2px; border: 1px solid transparent; }"
        "QToolBar QToolButton:hover { border: 1px solid #7b9ebd; background: #e4effb; }"
        "QToolBar QToolButton:pressed { border: 1px solid #5a809f; background: #c7dff5; }"
        "QToolBar::handle { width: 10px; }"
        "QToolBar::separator { width: 7px; margin: 3px 2px; border-left: 1px solid #aaa; }"
        "QDockWidget { color: black; }"
        "QTabWidget::pane { border: 1px solid #aaa; background: white; }"
        "QTabBar::tab { padding: 0px 11px; min-height: 18px; max-height: 18px; border: 1px solid #d6d6d6; background: #f0f0f0; }"
        "QTabBar::tab:selected { background: white; border-bottom-color: white; }"
        "QTabWidget#ProjectPages::pane { border: 0px; padding-top: 6px; }"
        "QTreeWidget#SystemSettingsTree { background: #caeace; border: 1px solid #8e9a8f; }"
        "QTreeWidget#SystemSettingsTree::item { height: 17px; }"
        "QStatusBar { background: #c5deff; }"
        "QStatusBar::item { border: 0px; }"
        "QLineEdit[NativeStatusField=\"true\"] { border: 1px solid #9ab2cf; background: #c5deff; padding: 0px 2px; }"));
    menuBar()->setFixedHeight(19);
    layout = nativeShellLayout();
    spec = layout.value(QStringLiteral("actions")).toObject();
    for (it = spec.constBegin(); it != spec.constEnd(); ++it)
    {
        id = it.key(); entry = it.value().toObject(); label = entry.value(QStringLiteral("text")).toString();
        action = m_commands.value(id);
        if (action == nullptr)
        {
            action = new QAction(label, this);
            action->setObjectName(id);
            m_commands.insert(id, action);
            if (id == QStringLiteral("action_line") || id == QStringLiteral("action_rect") || id == QStringLiteral("action_ellipse") || id == QStringLiteral("action_RoundedRect") || id == QStringLiteral("action_foldline") || id == QStringLiteral("action_polygon") || id == QStringLiteral("action_freeline") || id == QStringLiteral("action_ArcItem")) //20261113-2 change: Native static shapes enter drawing immediately.
            {
                connect(action, &QAction::triggered, this, [this, id]() { HmiObject object; object.type = id == QStringLiteral("action_line") ? QStringLiteral("line") : id == QStringLiteral("action_rect") ? QStringLiteral("rect") : id == QStringLiteral("action_ellipse") ? QStringLiteral("ellipse") : id == QStringLiteral("action_RoundedRect") ? QStringLiteral("roundrect") : id == QStringLiteral("action_foldline") ? QStringLiteral("polyline") : id == QStringLiteral("action_polygon") ? QStringLiteral("polygon") : id == QStringLiteral("action_freeline") ? QStringLiteral("freeline") : QStringLiteral("sector"); if (object.type == QStringLiteral("polygon")) { object.graphic.foreground = Qt::white; object.graphic.background = Qt::black; } //20261113-4 change: Native polygons default to white foreground and black solid background.
                    if (object.type == QStringLiteral("sector")) { object.graphic.lineStyle = 0; } //20261113-7 add: Native default sector has transparent stroke and white solid fill.
                    object.text.clear(); object.address.clear(); m_canvas->beginControlPlacement(object); });
            }
            else if (id == QStringLiteral("action_table")) //20261113-8 add: The native table workflow requires accepted appearance before drawing.
            {
                connect(action,&QAction::triggered,this,[this]() {
                    HmiObject object;
                    GraphicDialog *dialog;

                    m_canvas->cancelInteraction(); object.type = QStringLiteral("table"); object.text.clear(); object.address.clear(); object.graphic.background = QColor(QStringLiteral("#e0e0e0"));
                    dialog = new GraphicDialog(object,this);
                    if (dialog->exec() == QDialog::Accepted) { object = dialog->object(); delete dialog; m_canvas->beginControlPlacement(object); }
                    else { delete dialog; }
                });
            }
            else if (id == QStringLiteral("action_qrcode")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("qrcode")); });
            }
            else if (id == QStringLiteral("action_mark")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("scale")); });
            }
            else if (id == QStringLiteral("action_messagedisplay")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("message")); });
            }
            else if (id == QStringLiteral("pictruedisplay")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("image")); });
            }
            else if (id == QStringLiteral("action_GIFoutput")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("gif")); });
            }
            else if (id == QStringLiteral("action_dataDisplay"))
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("number")); }); //20261112-0 change: Configure before beginning rectangle placement.
            }
            else if (id == QStringLiteral("graphicsitem")) //20261112-1 add: Verified native menu and btn_metre share this action.
            { connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("chart")); }); }
            else if (id == QStringLiteral("action_rep")) { connect(action, &QAction::triggered, this, [this]() { configureRecipe(); }); } //20261113-2 add
            else if (id == QStringLiteral("action_datasamp")) { connect(action, &QAction::triggered, this, [this]() { configureDataCollection(); }); } //20261112-1 add
            else if (id == QStringLiteral("action_trendchart")) //20261112-1 add: Verified native menu and btn_trend share this action.
            { connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("trend")); }); }
            else if (id == QStringLiteral("action_DatagroupDisplay")) //20261112-1 add: Verified native menu and btn_DatagroupDisplay share this action.
            { connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("historytrend")); }); }
            else if (id == QStringLiteral("action_X_Y")) //20261113-1 add: Verified native menu and btn_XYtrend share this action.
            { connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("xycurve")); }); }
            else if (id == QStringLiteral("action_dynamicarlarm")) //20261113-2 add
            { connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("alarmbar")); }); }
            else if (id == QStringLiteral("action_dataGroup")) //20261113-1 add: Verified native menu and btn_dataGroup share this action.
            { connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("datagroup")); }); }
            else if (id == QStringLiteral("action_histotyarlarm")) //20261113-2 add: Keep the native historical-alarm entry independent of current alarms.
            { connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("historyalarm")); }); }
            else if (id == QStringLiteral("action_arlamcontrol")) //20261113-2 add: Verified native menu and btn_alarmDisplay share this action.
            { connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("alarmdisplay")); }); }
            else if (id == QStringLiteral("action_historyrecord")) //20261113-2 add: Verified native menu and btn_hisdataDisplay share this action.
            { connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("historydata")); }); }
            else if (id == QStringLiteral("action_recipetable")) //20261113-2 add: Verified native menu and btn__recipetable share this action.
            { connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("recipe")); }); }
            else if (id == QStringLiteral("action_moveTrack")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("track")); });
            }
            else if (id == QStringLiteral("action_wheelView")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("wheel")); });
            }
            else if (id == QStringLiteral("action_Slider")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("slider")); });
            }
            else if (id == QStringLiteral("action_flow_block")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("flow")); });
            }
            else if (id == QStringLiteral("action_droplist")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("dropdown")); });
            }
            else if (id == QStringLiteral("action_MFbtn")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("multifunction")); });
            }
            else if (id == QStringLiteral("action_polyBtn")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("multistate")); });
            }
            else if (id == QStringLiteral("action_screenbutton")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("screen")); });
            }
            else if (id == QStringLiteral("action_timeDisplay")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("time")); });
            }
            else if (id == QStringLiteral("action_asciiDisplay")) //20261112-0 add
            {
                connect(action, &QAction::triggered, this, [this]() { createStage4Control(QStringLiteral("ascii")); });
            }
            else if (id == QStringLiteral("action_zoonfit") || id == QStringLiteral("action_zoonbig") || id == QStringLiteral("action_zoonsmall"))
            {
                connect(action, &QAction::triggered, this, [this, id]() {
                    int zoom;
                    zoom = m_canvas->property("nativeShellZoom").toInt();
                    if (zoom == 0) { zoom = 100; }
                    zoom = id == QStringLiteral("action_zoonfit") ? 100 : qBound(25, zoom + (id == QStringLiteral("action_zoonbig") ? 25 : -25), 200);
                    m_canvas->setZoomPercent(zoom); m_canvas->setProperty("nativeShellZoom", zoom);
                });
            }
            else
            {
                action->setProperty("nativeShellPending", true);
                connect(action, &QAction::triggered, this, [this, label]() {
                    QMessageBox::information(this, label, QStringLiteral("此功能将在后续阶段完成。当前操作不会修改工程。"));
                });
            }
        }
        action->setText(label);
        action->setIcon(nativeShellIcon(entry.value(QStringLiteral("icon")).toString()));
        action->setToolTip(label);
        if (id != QStringLiteral("action_save") && id != QStringLiteral("action_undo") && id != QStringLiteral("action_redo"))
        {
            original = action->icon();
            original.addPixmap(original.pixmap(24, 24, QIcon::Normal), QIcon::Disabled);
            action->setIcon(original);
        }
    }
    menus = layout.value(QStringLiteral("menus")).toArray();
    if (menus.isEmpty()) { return; }
    // Reuse every implemented QAction; rebuilding the presentation must not duplicate transactions or shortcuts.
    populate = [this, &populate, &placed](QMenu *parent, const QJsonArray &items) {
        QJsonObject child;
        QAction *command;
        QMenu *sub;
        QJsonValue item;
        Q_FOREACH (item, items)
        {
            child = item.toObject();
            if (child.value(QStringLiteral("separator")).toBool()) { parent->addSeparator(); continue; }
            if (child.contains(QStringLiteral("items")))
            {
                if (child.value(QStringLiteral("text")).toString().contains(QStringLiteral("最近")))
                {
                    parent->addMenu(m_recentProjectsMenu); continue;
                }
                sub = parent->addMenu(child.value(QStringLiteral("text")).toString());
                populate(sub, child.value(QStringLiteral("items")).toArray());
                continue;
            }
            command = m_commands.value(child.value(QStringLiteral("id")).toString());
            if (command != nullptr) { parent->addAction(command); placed.insert(command); }
        }
    };
    menuBar()->clear();
    Q_FOREACH (value, menus)
    {
        entry = value.toObject();
        menu = menuBar()->addMenu(entry.value(QStringLiteral("text")).toString());
        populate(menu, entry.value(QStringLiteral("items")).toArray());
        if (entry.value(QStringLiteral("tools")).toBool())
        {
            extra = menu->addMenu(QStringLiteral("兼容工具"));
            extra->setObjectName(QStringLiteral("CompatibilityToolsMenu"));
        }
    }
    extra = findChild<QMenu *>(QStringLiteral("CompatibilityToolsMenu"));
    if (extra != nullptr)
    {
        Q_FOREACH (action, m_commands)
        {
            if (!placed.contains(action) && !action->property("nativeShellPending").toBool()) { extra->addAction(action); }
        }
        action = extra->addAction(QStringLiteral("简易属性（名称与坐标）..."));
        action->setObjectName(QStringLiteral("ShellQuickProperties"));
        connect(action, &QAction::triggered, this, [this]() {
            QDialog dialog(this);
            QVBoxLayout box(&dialog);
            QWidget *page;
            QDialogButtonBox buttons(QDialogButtonBox::Close, &dialog);

            dialog.setWindowTitle(QStringLiteral("简易属性")); dialog.resize(330, 480);
            page = m_rightTabs->widget(1); m_rightTabs->removeTab(1); box.addWidget(page); page->show(); box.addWidget(&buttons);
            connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
            dialog.exec();
            m_rightTabs->insertTab(1, page, QStringLiteral("属性")); m_rightTabs->setTabVisible(1, false);
        });
        action = extra->addAction(QStringLiteral("对象列表..."));
        connect(action, &QAction::triggered, this, [this]() {
            QDialog dialog(this);
            QVBoxLayout box(&dialog);
            QDialogButtonBox buttons(QDialogButtonBox::Close, &dialog);

            dialog.setWindowTitle(QStringLiteral("对象列表")); dialog.resize(350, 500);
            m_leftTabs->removeTab(1); box.addWidget(m_projectTree); m_projectTree->show(); box.addWidget(&buttons);
            connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
            dialog.exec();
            m_leftTabs->insertTab(1, m_projectTree, QStringLiteral("对象列表")); m_leftTabs->setTabVisible(1, false);
        });
    }
    connect(m_commands.value(QStringLiteral("action_detect_device")), &QAction::triggered, this, [this]() {
        statusBar()->showMessage(m_connectionLabel->text(), 6000);
    });
}

/******************************************************************************
 * Function Name   : MainWindow::createToolbars
 * Description     : Recreate native toolbar rows and bind their controls to the current document.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.11
 ******************************************************************************/
void MainWindow::createToolbars() //20261111-1 add
{
    QJsonObject layout;
    QJsonObject row;
    QJsonObject item;
    QToolBar *toolbar;
    QComboBox *combo;
    QWidget *host;
    QLabel *label;
    QAction *action;
    QString kind;
    QString id;
    int rowY;
    QJsonValue value;
    QJsonValue element;

    layout = nativeShellLayout(); rowY = -1;
    Q_FOREACH (value, layout.value(QStringLiteral("toolbars")).toArray())
    {
        row = value.toObject();
        if (rowY >= 0 && rowY != row.value(QStringLiteral("row")).toInt()) { addToolBarBreak(); }
        rowY = row.value(QStringLiteral("row")).toInt();
        toolbar = addToolBar(row.value(QStringLiteral("text")).toString());
        toolbar->setObjectName(row.value(QStringLiteral("id")).toString());
        toolbar->setIconSize(QSize(24, 24));
        toolbar->setFixedHeight(row.value(QStringLiteral("height")).toInt(34));
        if (row.contains(QStringLiteral("width"))) { toolbar->setFixedWidth(row.value(QStringLiteral("width")).toInt()); }
        toolbar->setMovable(true);
        toolbar->setFloatable(false);
        toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
        host = toolbar;
        if (row.value(QStringLiteral("id")).toString() == QStringLiteral("toolBar_Combox"))
        {
            host = new QWidget(toolbar); host->setFixedSize(362, 38); toolbar->addWidget(host);
        }
        Q_FOREACH (element, row.value(QStringLiteral("items")).toArray())
        {
            item = element.toObject(); kind = item.value(QStringLiteral("kind")).toString();
            id = item.value(QStringLiteral("id")).toString();
            if (kind == QStringLiteral("separator")) { toolbar->addSeparator(); continue; }
            if (kind == QStringLiteral("label"))
            {
                label = new QLabel(item.value(QStringLiteral("text")).toString(), host);
                label->setGeometry(item.value(QStringLiteral("x")).toInt(), 9, item.value(QStringLiteral("width")).toInt(24), 20);
                continue;
            }
            if (kind == QStringLiteral("state") || kind == QStringLiteral("language") || kind == QStringLiteral("page"))
            {
                combo = new QComboBox(host); combo->setObjectName(id);
                combo->setFixedWidth(item.value(QStringLiteral("width")).toInt(68));
                combo->setFixedHeight(20);
                combo->setStyleSheet(QStringLiteral("QComboBox { background: #caeace; padding: 0px 3px; border: 1px solid #999; }"));
                if (host == toolbar) { toolbar->addWidget(combo); }
                else { combo->move(item.value(QStringLiteral("x")).toInt(), 9); }
                if (kind == QStringLiteral("language"))
                {
                    m_languageCombo = combo; combo->addItem(QStringLiteral("中文"));
                    connect(combo, &QComboBox::currentTextChanged, this, [this](const QString &language) {
                        if (!language.isEmpty() && m_hasProject && !m_updatingNavigation)
                        { m_project.activeLanguage = language; m_canvas->setLanguage(language); setDirty(true); }
                    });
                }
                else if (kind == QStringLiteral("page"))
                {
                    m_pageCombo = combo;
                    connect(combo, &QComboBox::currentIndexChanged, this, [this](int index) {
                        if (!m_updatingNavigation && index >= 0) { activatePageId(m_pageCombo->itemData(index).toString()); }
                    });
                }
                else
                {
                    combo->addItems({QStringLiteral("0"), QStringLiteral("1")}); combo->setCurrentIndex(-1);
                    combo->setToolTip(QStringLiteral("选中控件的外观状态预览"));
                    connect(combo, &QComboBox::currentIndexChanged, this, [this](int index) {
                        if (index >= 0) { m_canvas->executeEdit(QStringLiteral("simulate%1").arg(index)); }
                    });
                }
                continue;
            }
            action = m_commands.value(id);
            if (action != nullptr) { toolbar->addAction(action); toolbar->widgetForAction(action)->setFixedSize(31, 30); }
        }
    }
}

/******************************************************************************
 * Function Name   : MainWindow::createNativePalette
 * Description     : Present the complete original icon grid with independent scrolling for each native tab.
 * Input Parameters: group
 * Return Value    : A scrollable toolbox page.
 * Date            : 2026.11.11
 ******************************************************************************/
QWidget *MainWindow::createNativePalette(const QString &group) //20261111-1 add
{
    QScrollArea *scroll;
    QWidget *page;
    QFrame *frame;
    QToolButton *button;
    QLabel *caption;
    QJsonObject item;
    QJsonObject geometry;
    QJsonValue value;
    QAction *action;
    QString id;
    int height;

    scroll = new QScrollArea(this); page = new QWidget(scroll); frame = new QFrame(page);
    scroll->setObjectName(group == QStringLiteral("basic") ? QStringLiteral("NativeBasicPalette") : QStringLiteral("NativeControlPalette"));
    scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    height = group == QStringLiteral("basic") ? 361 : 841;
    page->setMinimumSize(321, height + 18); frame->setGeometry(9, 9, 303, height);
    frame->setObjectName(QStringLiteral("NativePaletteFrame"));
    frame->setStyleSheet(QStringLiteral("QFrame#NativePaletteFrame { border: 1px solid #dadada; background: white; }"));
    page->setStyleSheet(QStringLiteral("QWidget { background: white; color: #082d69; } QToolButton { border: 1px solid transparent; background: white; padding: 0px; } QToolButton:hover { border: 1px solid #8bb4d7; background: #eef6ff; } QToolButton:pressed { background: #dceafb; }"));
    Q_FOREACH (value, nativeShellLayout().value(group).toArray())
    {
        item = value.toObject(); id = item.value(QStringLiteral("id")).toString();
        geometry = item.value(QStringLiteral("buttonRect")).toObject();
        button = new QToolButton(frame); button->setObjectName(item.value(QStringLiteral("widget")).toString());
        button->setText(item.value(QStringLiteral("text")).toString()); //20261111-1 add: Keep semantic labels even when the native button paints only its icon.
        button->setAccessibleName(item.value(QStringLiteral("text")).toString());
        button->setGeometry(geometry.value(QStringLiteral("x")).toInt(), geometry.value(QStringLiteral("y")).toInt(),
                            geometry.value(QStringLiteral("width")).toInt(), geometry.value(QStringLiteral("height")).toInt());
        button->setIcon(nativeShellIcon(item.value(QStringLiteral("icon")).toString()));
        button->setIconSize(QSize(32, 32)); button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        geometry = item.value(QStringLiteral("captionRect")).toObject();
        caption = new QLabel(item.value(QStringLiteral("text")).toString(), frame);
        caption->setFont(QFont(QStringLiteral("Microsoft YaHei"), 9)); caption->setAlignment(Qt::AlignCenter);
        caption->setWordWrap(true);
        caption->setGeometry(geometry.value(QStringLiteral("x")).toInt(), geometry.value(QStringLiteral("y")).toInt(),
                             geometry.value(QStringLiteral("width")).toInt(), geometry.value(QStringLiteral("height")).toInt());
        action = m_commands.value(id);
        if (action != nullptr)
        {
            button->setToolTip(action->property("nativeShellPending").toBool() ? QStringLiteral("%1（后续阶段实现）").arg(action->text()) : action->text());
            connect(button, &QToolButton::clicked, action, &QAction::trigger);
            connect(action, &QAction::changed, button, [button, action]() { button->setEnabled(action->isEnabled()); });
            button->setEnabled(action->isEnabled());
        }
    }
    scroll->setWidget(page);
    return scroll;
}

/******************************************************************************
 * Function Name   : MainWindow::createToolbox
 * Description     : Construct the original control tab from the retained icon manifest.
 * Input Parameters: None
 * Return Value    : The complete scrollable control grid.
 * Date            : 2026.11.11
 ******************************************************************************/
QWidget *MainWindow::createToolbox() //20261111-1 add
{
    return createNativePalette(QStringLiteral("controls"));
}

/******************************************************************************
 * Function Name   : MainWindow::createNativeStatusBar
 * Description     : Restore native status fields while keeping unavailable compile measurements visibly empty.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.11
 ******************************************************************************/
void MainWindow::createNativeStatusBar() //20261111-1 add
{
    QLabel *label;
    QLineEdit *field;
    QJsonObject item;
    QString text;
    QString id;
    QWidget *container;
    QVBoxLayout *box;
    QDockWidget *dock;
    QJsonValue value;

    // Dock content margins are independent of the document's page geometry and device pixel scale.
    dock = findChild<QDockWidget *>(QStringLiteral("ProjectTreeDock"));
    nativeDockTitle(dock);
    container = new QWidget(dock); box = new QVBoxLayout(container); box->setContentsMargins(9, 14, 9, 9);
    box->addWidget(m_leftTabs); dock->setWidget(container);
    dock = findChild<QDockWidget *>(QStringLiteral("ToolPropertyDock"));
    nativeDockTitle(dock);
    container = new QWidget(dock); box = new QVBoxLayout(container); box->setContentsMargins(0, 5, 0, 0);
    box->addWidget(m_rightTabs); dock->setWidget(container);
    takeCentralWidget(); container = new QWidget(this); box = new QVBoxLayout(container);
    box->setContentsMargins(9, 9, 9, 9); box->addWidget(m_pageTabs); setCentralWidget(container);
    m_canvas->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_canvas->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_canvas->setStyleSheet(QStringLiteral("ScreenCanvas { border: 1px solid #828790; }"));
    m_canvas->setBackgroundBrush(QBrush(QColor(202, 234, 206)));
    m_systemSettingsTree->setStyle(QStyleFactory::create(QStringLiteral("windows")));
    m_systemSettingsTree->style()->setParent(m_systemSettingsTree);
    m_projectLabel->hide(); m_coordinateLabel->hide(); m_connectionLabel->hide();
    statusBar()->setSizeGripEnabled(true); statusBar()->setFixedHeight(25);
    statusBar()->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    Q_FOREACH (value, nativeShellLayout().value(QStringLiteral("status")).toArray())
    {
        item = value.toObject(); id = item.value(QStringLiteral("id")).toString();
        if (item.value(QStringLiteral("kind")).toString() == QStringLiteral("field"))
        {
            field = new QLineEdit(statusBar()); field->setObjectName(id);
            field->setProperty("NativeStatusField", true); field->setReadOnly(true); field->setEnabled(false);
            field->setFocusPolicy(Qt::NoFocus); field->setFixedSize(item.value(QStringLiteral("width")).toInt(60), 20);
            if (id == QStringLiteral("NativeCompressedSize") || id == QStringLiteral("NativePageMemory"))
            { field->setToolTip(QStringLiteral("编译后统计；尚无本工程的编译结果。")); }
            statusBar()->addWidget(field);
        }
        else
        {
            text = item.value(QStringLiteral("text")).toString(); label = new QLabel(text, statusBar());
            label->setObjectName(id); label->setFixedSize(item.value(QStringLiteral("width")).toInt(60), 20);
            label->setMargin(0); statusBar()->addWidget(label);
        }
    }
}

/******************************************************************************
 * Function Name   : MainWindow::updateNativeStatusBar
 * Description     : Keep native geometry and count fields synchronized with the actual active document and selection.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.11
 ******************************************************************************/
void MainWindow::updateNativeStatusBar() //20261111-1 add
{
    QLineEdit *field;
    HmiObject object;
    bool selected;
    QMap<QString, QString> values;
    QMap<QString, QString>::const_iterator it;
    QComboBox *state;
    QLabel *typeLabel;
    bool blocked;
    int count;
    int index;

    object = m_canvas->selectedObject(&selected);
    state = findChild<QComboBox *>(QStringLiteral("NativeStatePreview"));
    if (state != nullptr)
    {
        blocked = state->blockSignals(true);
        count = !selected || object.type == QStringLiteral("recipe") || object.type == QStringLiteral("historydata") || object.type == QStringLiteral("historyalarm") || object.type == QStringLiteral("alarmdisplay") || object.type == QStringLiteral("alarmbar") || object.type == QStringLiteral("datagroup") || object.type == QStringLiteral("xycurve") || object.type == QStringLiteral("historytrend") || object.type == QStringLiteral("trend") || object.type == QStringLiteral("chart") || object.type == QStringLiteral("track") || object.type == QStringLiteral("wheel") || object.type == QStringLiteral("slider") || object.type == QStringLiteral("flow") || object.type == QStringLiteral("dropdown") || object.type == QStringLiteral("time") || object.type == QStringLiteral("ascii") || object.hasNumberDisplayProperties || object.type == QStringLiteral("text") || object.type == QStringLiteral("scale") || object.type == QStringLiteral("qrcode") || object.type == QStringLiteral("gif") ? 0 : object.type == QStringLiteral("multifunction") ? (object.multiFunctionButton.pressedEffect ? 2 : 1) : object.type == QStringLiteral("multistate") ? object.multiStateButton.stateCount : object.hasImageDisplayProperties ? object.imageDisplay.stateCount : object.type == QStringLiteral("message") ? object.messageDisplay.stateCount : object.hasLampProperties ? object.lamp.stateCount() : object.hasFunctionButtonProperties ? 1 : 2; //20261112-0 change
        state->clear();
        for (index = 0; index < count; ++index) { state->addItem(QString::number(index)); }
        state->setEnabled(m_hasProject && count > 1);
        if (count == 0) { state->setCurrentIndex(-1); }
        else if (!m_canvas->scene()->selectedItems().isEmpty()) { state->setCurrentIndex(m_canvas->scene()->selectedItems().first()->data(11).toInt()); }
        state->blockSignals(blocked);
    }
    typeLabel = findChild<QLabel *>(QStringLiteral("NativeSelectionType"));
    if (typeLabel != nullptr)
    {
        typeLabel->setText(!selected ? QString() : object.type == QStringLiteral("text") ? QStringLiteral("静态文本") :
                           object.type == QStringLiteral("chart") ? QStringLiteral("图表") : object.type == QStringLiteral("trend") ? QStringLiteral("趋势图") : object.type == QStringLiteral("historytrend") ? QStringLiteral("历史趋势图") : object.type == QStringLiteral("xycurve") ? QStringLiteral("X-Y曲线") : object.type == QStringLiteral("alarmbar") ? QStringLiteral("动态报警条") : object.type == QStringLiteral("datagroup") ? QStringLiteral("数据群组") : object.type == QStringLiteral("historyalarm") ? QStringLiteral("历史报警显示器") : object.type == QStringLiteral("alarmdisplay") ? QStringLiteral("报警显示器") : object.type == QStringLiteral("historydata") ? QStringLiteral("历史数据显示器") : isSupportedGraphic(object.type) ? graphicName(object.type) : object.type == QStringLiteral("recipe") ? QStringLiteral("配方显示器") : object.type == QStringLiteral("track") ? QStringLiteral("移动轨迹") : object.type == QStringLiteral("wheel") ? QStringLiteral("滚轮控件") : object.type == QStringLiteral("slider") ? QStringLiteral("滑动开关") : object.type == QStringLiteral("flow") ? QStringLiteral("流动块") : object.type == QStringLiteral("dropdown") ? QStringLiteral("下拉框") : object.type == QStringLiteral("multifunction") ? QStringLiteral("多功能按钮") : object.type == QStringLiteral("gif") ? QStringLiteral("GIF显示器") : object.type == QStringLiteral("qrcode") ? QStringLiteral("二维码") : object.type == QStringLiteral("scale") ? QStringLiteral("刻度") : object.type == QStringLiteral("message") ? QStringLiteral("消息显示器") : object.type == QStringLiteral("lamp") ? QStringLiteral("指示灯") : object.type == QStringLiteral("image") ? QStringLiteral("图片显示器") : //20261112-0 change
                           (object.type == QStringLiteral("time") || object.type == QStringLiteral("number") || object.type == QStringLiteral("ascii")) ? QStringLiteral("数值显示器") : QStringLiteral("开关"));
    }
    values.insert(QStringLiteral("NativeControlCount"), m_hasProject ? QString::number(m_canvas->objects().size()) : QString());
    values.insert(QStringLiteral("NativeResolution"), m_hasProject ? QStringLiteral("%1*%2").arg(m_project.pageWidth).arg(m_project.pageHeight) : QString());
    values.insert(QStringLiteral("NativeObjectX"), selected ? QString::number(object.geometry.x()) : QString());
    values.insert(QStringLiteral("NativeObjectY"), selected ? QString::number(object.geometry.y()) : QString());
    values.insert(QStringLiteral("NativeObjectWidth"), selected ? QString::number(object.geometry.width()) : QString());
    values.insert(QStringLiteral("NativeObjectHeight"), selected ? QString::number(object.geometry.height()) : QString());
    setWindowTitle(!m_hasProject ? QStringLiteral("KinSealStudio") : QStringLiteral("KinSealStudio--%1--%2%3")
        .arg(m_currentPath.isEmpty() ? m_project.name : QDir::toNativeSeparators(m_currentPath)).arg(m_project.pageName)
        .arg(m_dirty ? QStringLiteral(" *") : QString()));
    for (it = values.constBegin(); it != values.constEnd(); ++it)
    {
        field = findChild<QLineEdit *>(it.key());
        if (field != nullptr) { field->setText(it.value()); }
    }
}

/******************************************************************************
 * Function Name   : MainWindow::decorateNativeSystemTree
 * Description     : Apply recovered tree icons without confusing visible placeholders with saved configuration data.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.11
 ******************************************************************************/
void MainWindow::decorateNativeSystemTree() //20261111-1 add
{
    QJsonObject icons;
    QTreeWidgetItem *item;
    QString label;
    QTreeWidgetItemIterator iterator(m_systemSettingsTree);

    icons = nativeShellLayout().value(QStringLiteral("treeIcons")).toObject();
    m_systemSettingsTree->setFont(QFont(QStringLiteral("Microsoft YaHei UI"), 9));
    m_systemSettingsTree->setIndentation(20); m_systemSettingsTree->setIconSize(QSize(16, 16));
    while (*iterator != nullptr)
    {
        item = *iterator; label = item->text(0);
        if (item->data(0, Qt::UserRole).toString() == QStringLiteral("page")) { label = QStringLiteral("000:画面1"); }
        item->setIcon(0, nativeShellIcon(icons.value(label).toString())); ++iterator;
    }
}
