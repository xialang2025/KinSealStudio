#include "stage2_project_dialogs.h"
#include "bit_button_dialog.h" //20261123-3 add
#include "control_rendering.h" //20261123-2 add
#include "kvg_library.h" //20261123-3 add
#include "macro_workspace.h" //20261123-5 add
#include "plc_address_dialog.h" //20261123-5 add
#include "stage2_system_settings.h" //20261122-4 change

#include <QCheckBox> //20261123-4 add

#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout> //20261123-1 add
#include <QGroupBox>
#include <QHeaderView> //20261123-5 add
#include <QHBoxLayout>
#include <QJsonArray> //20261123-5 add
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter> //20261123-1 add
#include <QPlainTextEdit> //20261123-5 add
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTableWidget> //20261123-5 add
#include <QUuid>
#include <QVBoxLayout>

/******************************************************************************
 * Function Name   : stage2Error
 * Description     : Return a localized transaction error without opening another modal dialog.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.10
 ******************************************************************************/
static bool stage2Error(QString *error, const QString &message) //20261110-7 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : validateStage2PageName
 * Description     : Enforce the common editor page naming policy without changing stable identities.
 * Input Parameters: project, id, name, error.
 * Return Value    : True for a nonempty unique display name within the editor's bounded input.
 * Date            : 2026.11.22
 ******************************************************************************/
static bool validateStage2PageName(const HmiProject &project, const QString &id,
                                   const QString &name, QString *error) //20261122-9 change
{
    HmiPage page;

    if (name.trimmed().isEmpty() || name.size() > 32767)
    {
        return stage2Error(error, QStringLiteral("画面名称不能为空或超过32767字符。"));
    }
    Q_FOREACH (page, project.allPages())
    {
        if (page.id != id && page.name == name)
        {
            return stage2Error(error, QStringLiteral("名字已经存在，请重新输入。"));
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : chooseStage2PageBitmap
 * Description     : Select an existing decoded picture without changing the project gallery.
 * Input Parameters: parent, pictures, name.
 * Return Value    : True after confirming a picture.
 * Date            : 2026.11.23
 ******************************************************************************/
static bool chooseStage2PageBitmap(QWidget *parent, const QMap<QString, QByteArray> &pictures,
                                   QString &name) //20261123-2 add
{
    QDialog dialog(parent);
    QVBoxLayout *layout;
    QListWidget *items;
    QDialogButtonBox *buttons;
    QMap<QString, QByteArray>::const_iterator picture;
    QImage icon;
    QListWidgetItem *entry;

    dialog.setWindowTitle(QStringLiteral("工程图库"));
    dialog.resize(490, 340);
    layout = new QVBoxLayout(&dialog);
    items = new QListWidget(&dialog);
    items->setObjectName(QStringLiteral("pageBitmapGalleryItems"));
    items->setViewMode(QListView::IconMode);
    items->setResizeMode(QListView::Adjust);
    items->setMovement(QListView::Static);
    items->setIconSize(QSize(80, 60));
    items->setSpacing(6);
    layout->addWidget(items);
    for (picture = pictures.cbegin(); picture != pictures.cend(); ++picture)
    {
        icon = decodedControlPng(picture.value());
        if (!icon.isNull())
        {
            entry = new QListWidgetItem(QIcon(QPixmap::fromImage(icon.scaled(items->iconSize(),
                Qt::KeepAspectRatio, Qt::SmoothTransformation))), picture.key(), items);
            if (picture.key() == name)
            {
                items->setCurrentItem(entry);
            }
        }
    }
    if (items->count() == 0)
    {
        layout->addWidget(new QLabel(QStringLiteral("工程中没有可用图片。可在工程图片管理中添加。"), &dialog));
    }
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
    buttons->button(QDialogButtonBox::Ok)->setEnabled(items->currentItem() != nullptr);
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    layout->addWidget(buttons);
    QObject::connect(items, &QListWidget::currentItemChanged, &dialog, [buttons](QListWidgetItem *current) {
        buttons->button(QDialogButtonBox::Ok)->setEnabled(current != nullptr);
    });
    QObject::connect(items, &QListWidget::itemDoubleClicked, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted || items->currentItem() == nullptr)
    {
        return false;
    }
    name = items->currentItem()->text();
    return true;
}

/******************************************************************************
 * Function Name   : createStage2PageMacroSource
 * Description     : Add an ordinary macro source to the script dialog's local transaction.
 * Input Parameters: parent, project, selectedName.
 * Return Value    : True after source creation succeeds.
 * Date            : 2026.11.23
 ******************************************************************************/
static bool createStage2PageMacroSource(QWidget *parent, HmiProject &project, QString &selectedName) //20261123-5 add
{
    QDialog editor(parent);
    QFormLayout layout(&editor);
    QLineEdit name(&editor);
    QPlainTextEdit source(MacroWorkspace::defaultSource(), &editor);
    QLabel validation(&editor);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &editor);

    editor.setWindowTitle(QStringLiteral("宏指令编辑器"));
    editor.resize(640, 480);
    layout.addRow(QStringLiteral("宏名称"), &name);
    layout.addRow(&source);
    validation.setWordWrap(true);
    layout.addRow(&validation);
    layout.addRow(&buttons);
    buttons.button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
    buttons.button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &editor, &QDialog::reject);
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &editor, [&]() {
        QString error;

        if (MacroWorkspace::create(&project, name.text(), source.toPlainText(), &error))
        {
            selectedName = name.text();
            editor.accept();
            return;
        }
        validation.setText(error);
    });
    return editor.exec() == QDialog::Accepted;
}

struct Stage2PageScript //20261123-5 add: One ordered invocation, independent from its shared macro definition.
{
    QString macroId;
    QString address;
    int frequency = 10;
    int count = 0;
};

/******************************************************************************
 * Function Name   : editStage2PageScript
 * Description     : Edit an invocation and keep nested source changes in the page draft.
 * Input Parameters: parent, project, script.
 * Return Value    : True when a source and schedule are confirmed.
 * Date            : 2026.11.23
 ******************************************************************************/
static bool editStage2PageScript(QWidget *parent, HmiProject &project, Stage2PageScript &script) //20261123-5 add
{
    QDialog dialog(parent);
    QFormLayout layout(&dialog);
    QWidget modeRow(&dialog);
    QHBoxLayout modeLayout(&modeRow);
    QRadioButton unconditional(QStringLiteral("无条件执行"), &modeRow);
    QRadioButton triggered(QStringLiteral("地址触发"), &modeRow);
    QSpinBox frequency(&dialog);
    QSpinBox count(&dialog);
    QWidget addressRow(&dialog);
    QHBoxLayout addressLayout(&addressRow);
    QLineEdit address(script.address.isEmpty() ? QStringLiteral("LB0") : script.address, &addressRow);
    QPushButton editAddress(QStringLiteral("…"), &addressRow);
    QComboBox macros(&dialog);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QPushButton *write;
    HmiProject draft = project;
    QString selected = draft.macroIds.key(script.macroId);

    dialog.resize(350, 297);
    dialog.setWindowTitle(script.macroId.isEmpty() ? QStringLiteral("新建脚本") : QStringLiteral("修改脚本"));
    modeLayout.setContentsMargins(0, 0, 0, 0);
    modeLayout.addWidget(&unconditional);
    modeLayout.addWidget(&triggered);
    layout.addRow(&modeRow);
    frequency.setObjectName(QStringLiteral("pageMacroFrequency"));
    frequency.setRange(1, 9999);
    frequency.setValue(script.frequency);
    frequency.setSuffix(QStringLiteral(" x100MS"));
    layout.addRow(QStringLiteral("执行频率"), &frequency);
    count.setObjectName(QStringLiteral("pageMacroCount"));
    count.setRange(0, 65535);
    count.setValue(script.count);
    layout.addRow(QStringLiteral("执行次数"), &count);
    layout.addRow(new QLabel(QStringLiteral("0表示一直执行"), &dialog));
    addressLayout.setContentsMargins(0, 0, 0, 0);
    addressLayout.addWidget(new QLabel(QStringLiteral("受控地址"), &addressRow));
    address.setReadOnly(true);
    addressLayout.addWidget(&address);
    addressLayout.addWidget(&editAddress);
    layout.addRow(&addressRow);
    triggered.setChecked(!script.address.isEmpty());
    unconditional.setChecked(script.address.isEmpty());
    addressRow.setVisible(triggered.isChecked());
    macros.setObjectName(QStringLiteral("pageMacroSource"));
    layout.addRow(QStringLiteral("脚本函数"), &macros);
    layout.addRow(&buttons);
    buttons.button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
    buttons.button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    write = buttons.addButton(QStringLiteral("写宏"), QDialogButtonBox::ActionRole);
    auto populateMacros = [&]() {
        macros.clear();
        macros.addItems(draft.macros.keys());
        if (!selected.isEmpty())
        {
            macros.setCurrentText(selected);
        }
        buttons.button(QDialogButtonBox::Ok)->setEnabled(macros.count() != 0);
    };
    populateMacros();
    QObject::connect(&triggered, &QRadioButton::toggled, &addressRow, &QWidget::setVisible);
    QObject::connect(&editAddress, &QPushButton::clicked, &dialog, [&]() {
        QString chosen = address.text();

        if (editPlcAddress(&dialog, true, draft.connections, &chosen))
        {
            address.setText(chosen);
        }
    });
    QObject::connect(write, &QPushButton::clicked, &dialog, [&]() {
        if (createStage2PageMacroSource(&dialog, draft, selected))
        {
            populateMacros();
        }
    });
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted || macros.currentIndex() < 0)
    {
        return false;
    }
    selected = macros.currentText();
    if (!draft.macroIds.contains(selected))
    {
        draft.macroIds.insert(selected, QUuid::createUuid().toString(QUuid::WithoutBraces));
    }
    script.macroId = draft.macroIds.value(selected);
    script.frequency = frequency.value();
    script.count = count.value();
    script.address = triggered.isChecked() ? address.text() : QString();
    project = draft;
    return true;
}

/******************************************************************************
 * Function Name   : createStage2PageScriptList
 * Description     : Display the ordered page invocation draft with bounded add, remove and edit actions.
 * Input Parameters: parent, project, scripts.
 * Return Value    : Editor widget owned by the ordinary page dialog.
 * Date            : 2026.11.23
 ******************************************************************************/
static QWidget *createStage2PageScriptList(QWidget *parent, HmiProject &project,
                                           QList<Stage2PageScript> &scripts) //20261123-5 add
{
    QWidget *widget = new QWidget(parent);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    QTableWidget *table = new QTableWidget(0, 5, widget);
    QHBoxLayout *actions = new QHBoxLayout;
    QPushButton *add = new QPushButton(QStringLiteral("新增"), widget);
    QPushButton *remove = new QPushButton(QStringLiteral("删除"), widget);
    QPushButton *edit = new QPushButton(QStringLiteral("修改"), widget);
    QLabel *usage = new QLabel(widget);

    layout->setContentsMargins(0, 0, 0, 0);
    table->setHorizontalHeaderLabels({QStringLiteral("脚本函数"), QStringLiteral("执行频率"),
        QStringLiteral("执行次数"), QStringLiteral("受控类型"), QStringLiteral("受控地址")});
    table->verticalHeader()->hide();
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(table);
    add->setObjectName(QStringLiteral("pageMacroAdd"));
    actions->addWidget(add);
    actions->addWidget(remove);
    actions->addWidget(edit);
    actions->addWidget(usage);
    actions->addStretch();
    layout->addLayout(actions);
    auto refresh = [=, &project, &scripts](int selection) {
        int row;
        int column;

        table->clearContents();
        table->setRowCount(scripts.size());
        for (row = 0; row < scripts.size(); ++row)
        {
            const Stage2PageScript &script = scripts.at(row);
            const QStringList fields = {project.macroIds.key(script.macroId), QString::number(script.frequency),
                QString::number(script.count), script.address.isEmpty() ? QStringLiteral("无条件执行") :
                QStringLiteral("地址触发"), script.address};

            for (column = 0; column < fields.size(); ++column)
            {
                table->setItem(row, column, new QTableWidgetItem(fields.at(column)));
            }
        }
        if (selection >= 0 && selection < table->rowCount())
        {
            table->selectRow(selection);
        }
        add->setEnabled(scripts.size() < 25);
        usage->setText(QStringLiteral("%1/25").arg(scripts.size()));
        remove->setEnabled(!table->selectedItems().isEmpty());
        edit->setEnabled(remove->isEnabled());
    };
    QObject::connect(table, &QTableWidget::itemSelectionChanged, widget, [=]() {
        remove->setEnabled(!table->selectedItems().isEmpty());
        edit->setEnabled(remove->isEnabled());
    });
    QObject::connect(add, &QPushButton::clicked, widget, [=, &project, &scripts]() {
        Stage2PageScript script;

        if (scripts.size() < 25 && editStage2PageScript(widget, project, script))
        {
            scripts.append(script);
            refresh(scripts.size() - 1);
        }
    });
    QObject::connect(remove, &QPushButton::clicked, widget, [=, &scripts]() {
        const int row = table->currentRow();

        if (row >= 0 && row < scripts.size())
        {
            scripts.removeAt(row);
            refresh(qMin(row, int(scripts.size()) - 1));
        }
    });
    QObject::connect(edit, &QPushButton::clicked, widget, [=, &project, &scripts]() {
        const int row = table->currentRow();

        if (row >= 0 && row < scripts.size() && editStage2PageScript(widget, project, scripts[row]))
        {
            refresh(row);
        }
    });
    refresh(-1);
    return widget;
}

/******************************************************************************
 * Function Name   : storeStage2PageScripts
 * Description     : Store the ordered invocation list with one stable macro reference per row.
 * Input Parameters: project, pageId, scripts.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
static void storeStage2PageScripts(HmiProject &project, const QString &pageId,
                                   const QList<Stage2PageScript> &scripts) //20261123-5 add
{
    HmiConfigurationNode node;
    QJsonArray rows;
    QString rowId;

    if (scripts.isEmpty())
    {
        return;
    }
    node.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    node.kind = QStringLiteral("pageMacro");
    node.name = QStringLiteral("画面宏");
    node.ownerPageId = pageId;
    node.implemented = true;
    node.active = true;
    for (const Stage2PageScript &script : scripts)
    {
        rowId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        rows.append(QJsonObject{{QStringLiteral("id"), rowId},
            {QStringLiteral("frequency"), script.frequency}, {QStringLiteral("count"), script.count},
            {QStringLiteral("address"), script.address}});
        project.references.append({QUuid::createUuid().toString(QUuid::WithoutBraces),
            QStringLiteral("configuration"), node.id, rowId, QStringLiteral("macro"), script.macroId});
    }
    node.properties.insert(QStringLiteral("scripts"), rows);
    project.configurationNodes.append(node);
}

/******************************************************************************
 * Function Name   : editStage2NewNormalPage
 * Description     : Create and activate a normal page on a detached project after the user confirms.
 * Input Parameters: parent, project.
 * Return Value    : True after the complete page transaction is accepted.
 * Date            : 2026.11.22
 ******************************************************************************/
bool editStage2NewNormalPage(QWidget *parent, HmiProject &project) //20261122-9 add
{
    QDialog dialog(parent);
    QFormLayout *layout;
    QLineEdit *name;
    QSpinBox *number;
    QGroupBox *background;
    QGridLayout *backgroundLayout; //20261123-1 change
    QRadioButton *defaultMode; //20261123-2 add
    QRadioButton *bitmapMode;
    QStackedWidget *backgroundOptions;
    QWidget *colorOptions;
    QWidget *bitmapOptions;
    QGridLayout *colorLayout;
    QVBoxLayout *bitmapLayout;
    QHBoxLayout *galleryButtons; //20261123-3 add
    QPushButton *backgroundColorButton;
    QPushButton *foregroundColorButton;
    QComboBox *backgroundStyle;
    QLabel *backgroundPreview;
    QLineEdit *bitmapPath;
    QPushButton *projectGallery;
    QPushButton *systemGallery; //20261123-3 add
    QColor backgroundColor = HmiPageBackground::defaultColor(); //20261123-1 change
    QColor foregroundColor = HmiPageBackground::defaultColor();
    QGroupBox *slideGroup; //20261123-4 add
    QGridLayout *slideLayout;
    QSpinBox *slideDistance;
    QCheckBox *slideDirections[4];
    QComboBox *slideTargets[4];
    const QStringList slideLabels = {QStringLiteral("向左"), QStringLiteral("向右"),
                                     QStringLiteral("向上"), QStringLiteral("向下")};
    const QStringList slideKeys = {QStringLiteral("Left"), QStringLiteral("Right"),
                                   QStringLiteral("Up"), QStringLiteral("Down")};
    const QMap<QString, int> ordinaryNumbers = project.ordinaryPageNumbers();
    const QList<HmiPage> targetPages = project.allPages();
    int directionIndex;
    int column;
    QString bitmapName;
    QString systemPicture; //20261123-3 add
    QImage bitmap;
    QLabel *validation;
    QDialogButtonBox *buttons;
    HmiProject candidate;
    HmiProject macroDraft = project; //20261123-5 add
    QList<Stage2PageScript> scripts;
    QList<int> occupiedNumbers; //20261122-10 add
    QString suggestion;
    int pageNumber;
    int nameNumber;
    bool confirmed = false;

    pageNumber = project.pageCount();
    nameNumber = pageNumber + 1;
    occupiedNumbers = project.ordinaryPageNumbers().values();
    while (occupiedNumbers.contains(pageNumber)) //20261122-10 add: Keep the established order-based suggestion wherever it is available.
    {
        ++pageNumber;
    }
    suggestion = QStringLiteral("画面%1").arg(nameNumber);
    while (!validateStage2PageName(project, QString(), suggestion, nullptr))
    {
        suggestion = QStringLiteral("画面%1").arg(++nameNumber);
    }

    dialog.setWindowTitle(QStringLiteral("新建画面"));
    layout = new QFormLayout(&dialog);
    name = new QLineEdit(suggestion, &dialog);
    name->setObjectName(QStringLiteral("newPageNameEdit"));
    name->setMaxLength(32767);
    number = new QSpinBox(&dialog);
    number->setObjectName(QStringLiteral("newPageNumberSpin"));
    number->setRange(0, 999);
    number->setValue(pageNumber);
    number->setToolTip(QStringLiteral("同一工程内普通画面编号不可重复。")); //20261122-10 change
    validation = new QLabel(&dialog);
    validation->setWordWrap(true);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    layout->addRow(QStringLiteral("名称"), name);
    layout->addRow(QStringLiteral("编号"), number);
    background = new QGroupBox(QStringLiteral("背景"), &dialog); //20261123-0 add
    backgroundLayout = new QGridLayout(background); //20261123-1 change
    defaultMode = new QRadioButton(QStringLiteral("默认"), background); //20261123-2 add
    defaultMode->setChecked(true);
    bitmapMode = new QRadioButton(QStringLiteral("位图"), background);
    bitmapMode->setObjectName(QStringLiteral("newPageBitmapMode"));
    backgroundOptions = new QStackedWidget(background);
    colorOptions = new QWidget(backgroundOptions);
    colorLayout = new QGridLayout(colorOptions);
    colorLayout->setContentsMargins(0, 0, 0, 0);
    bitmapOptions = new QWidget(backgroundOptions);
    bitmapLayout = new QVBoxLayout(bitmapOptions);
    bitmapLayout->setContentsMargins(0, 0, 0, 0);
    bitmapPath = new QLineEdit(bitmapOptions);
    bitmapPath->setReadOnly(true);
    bitmapPath->setAccessibleName(QStringLiteral("背景图片"));
    projectGallery = new QPushButton(QStringLiteral("工程图库"), bitmapOptions);
    projectGallery->setObjectName(QStringLiteral("newPageProjectGallery"));
    systemGallery = new QPushButton(QStringLiteral("系统图库"), bitmapOptions); //20261123-3 add
    systemGallery->setObjectName(QStringLiteral("newPageSystemGallery"));
    galleryButtons = new QHBoxLayout;
    galleryButtons->addWidget(systemGallery);
    galleryButtons->addWidget(projectGallery);
    bitmapLayout->addWidget(bitmapPath);
    bitmapLayout->addLayout(galleryButtons); //20261123-3 change
    bitmapLayout->addStretch();
    backgroundOptions->addWidget(colorOptions);
    backgroundOptions->addWidget(bitmapOptions);
    backgroundColorButton = new QPushButton(colorOptions);
    backgroundColorButton->setObjectName(QStringLiteral("newPageBackgroundColor"));
    backgroundColorButton->setMinimumWidth(180);
    backgroundColorButton->setAccessibleName(QStringLiteral("背景色"));
    foregroundColorButton = new QPushButton(colorOptions);
    foregroundColorButton->setObjectName(QStringLiteral("newPageForegroundColor"));
    foregroundColorButton->setAccessibleName(QStringLiteral("前景色"));
    backgroundStyle = new QComboBox(colorOptions);
    backgroundStyle->setObjectName(QStringLiteral("newPageBackgroundStyle"));
    backgroundStyle->addItems({QStringLiteral("纯色"), QStringLiteral("横向过渡"), QStringLiteral("横向对称过渡"),
        QStringLiteral("纵向过渡"), QStringLiteral("纵向对称过渡"), QStringLiteral("斜上过渡"),
        QStringLiteral("斜上对称过渡"), QStringLiteral("斜下过渡"), QStringLiteral("斜下对称过渡"),
        QStringLiteral("右上角辐射"), QStringLiteral("左上角辐射"), QStringLiteral("中心辐射")});
    backgroundPreview = new QLabel(background);
    backgroundPreview->setObjectName(QStringLiteral("newPageBackgroundPreview")); //20261123-2 add
    backgroundPreview->setFixedSize(110, 110);
    backgroundPreview->setAccessibleName(QStringLiteral("背景预览"));
    colorLayout->addWidget(new QLabel(QStringLiteral("背景色"), colorOptions), 0, 0); //20261123-2 change
    colorLayout->addWidget(backgroundColorButton, 0, 1);
    colorLayout->addWidget(new QLabel(QStringLiteral("前景色"), colorOptions), 1, 0);
    colorLayout->addWidget(foregroundColorButton, 1, 1);
    colorLayout->addWidget(new QLabel(QStringLiteral("样式"), colorOptions), 2, 0);
    colorLayout->addWidget(backgroundStyle, 2, 1);
    backgroundLayout->addWidget(defaultMode, 0, 0);
    backgroundLayout->addWidget(bitmapMode, 1, 0);
    backgroundLayout->addWidget(backgroundOptions, 0, 1, 2, 1);
    backgroundLayout->addWidget(backgroundPreview, 0, 2, 2, 1, Qt::AlignCenter);

    slideGroup = new QGroupBox(QStringLiteral("滑动打开画面"), background); //20261123-4 add
    slideGroup->setObjectName(QStringLiteral("pageSlideGroup"));
    slideGroup->setCheckable(true);
    slideLayout = new QGridLayout(slideGroup);
    slideDistance = new QSpinBox(slideGroup);
    slideDistance->setObjectName(QStringLiteral("pageSlideDistance"));
    slideDistance->setRange(5, 480);
    slideDistance->setValue(50);
    slideLayout->addWidget(new QLabel(QStringLiteral("滑动距离："), slideGroup), 0, 0);
    slideLayout->addWidget(slideDistance, 0, 1);
    for (directionIndex = 0; directionIndex < 4; ++directionIndex)
    {
        column = 2 * (directionIndex % 2);
        slideDirections[directionIndex] = new QCheckBox(slideLabels.at(directionIndex), slideGroup);
        slideTargets[directionIndex] = new QComboBox(slideGroup);
        slideDirections[directionIndex]->setObjectName(QStringLiteral("pageSlide") + slideKeys.at(directionIndex));
        slideTargets[directionIndex]->setObjectName(slideDirections[directionIndex]->objectName() + QStringLiteral("Target"));
        for (const HmiPage &target : targetPages)
        {
            if (ordinaryNumbers.contains(target.id))
            {
                // The label reflects renumbering or renaming while the data carries a stable ID.
                slideTargets[directionIndex]->addItem(QStringLiteral("%1:%2")
                    .arg(ordinaryNumbers.value(target.id), 3, 10, QLatin1Char('0')).arg(target.name), target.id);
            }
        }
        slideLayout->addWidget(slideDirections[directionIndex], 1 + directionIndex / 2, column);
        slideLayout->addWidget(slideTargets[directionIndex], 1 + directionIndex / 2, column + 1);
    }
    slideGroup->setChecked(false);
    backgroundLayout->addWidget(slideGroup, 2, 0, 1, 3);
    layout->addRow(background);
    auto fillDraft = [&]() {
        HmiPageBackground fill;
        if (bitmapMode->isChecked()) //20261123-2 add: Preserve the color choices on the draft while serializing only the selected mode.
        {
            fill.bitmapName = bitmapName;
            fill.systemPicture = systemPicture; //20261123-3 add
            return fill;
        }
        if (backgroundColor != HmiPageBackground::defaultColor())
        {
            fill.color = backgroundColor.name();
        }
        if (foregroundColor != HmiPageBackground::defaultColor())
        {
            fill.foregroundColor = foregroundColor.name();
        }
        fill.style = backgroundStyle->currentIndex();
        return fill;
    };
    auto updateBackgroundPreview = [&]() { //20261123-1 change: Preview and active canvas share the recovered brush geometry.
        QPixmap sample(backgroundPreview->size());
        QPainter painter;
        backgroundColorButton->setStyleSheet(QStringLiteral("background-color: %1").arg(backgroundColor.name()));
        foregroundColorButton->setStyleSheet(QStringLiteral("background-color: %1").arg(foregroundColor.name()));
        sample.fill(Qt::transparent); //20261123-2 add: Keep transparent source pixels in the preview.
        painter.begin(&sample);
        painter.fillRect(sample.rect(), fillDraft().brush(sample.rect(), bitmap));
        painter.end();
        backgroundPreview->setPixmap(sample);
    };
    updateBackgroundPreview();
    auto chooseColor = [&](QColor &current) {
        const QColor chosen = QColorDialog::getColor(current, &dialog, QStringLiteral("选择颜色"),
                                                       QColorDialog::DontUseNativeDialog);
        if (chosen.isValid())
        {
            current = chosen;
            updateBackgroundPreview();
        }
    };
    QObject::connect(backgroundColorButton, &QPushButton::clicked, &dialog, [&]() { chooseColor(backgroundColor); });
    QObject::connect(foregroundColorButton, &QPushButton::clicked, &dialog, [&]() { chooseColor(foregroundColor); });
    QObject::connect(backgroundStyle, &QComboBox::currentIndexChanged, &dialog, updateBackgroundPreview);
    QObject::connect(bitmapMode, &QRadioButton::toggled, &dialog, [&](bool enabled) { //20261123-2 add
        backgroundOptions->setCurrentWidget(enabled ? bitmapOptions : colorOptions);
        validation->clear();
        updateBackgroundPreview();
    });
    QObject::connect(projectGallery, &QPushButton::clicked, &dialog, [&]() {
        if (chooseStage2PageBitmap(&dialog, project.pictures, bitmapName))
        {
            systemPicture.clear(); //20261123-3 add: The last confirmed gallery choice owns the background.
            bitmap = decodedControlPng(project.pictures.value(bitmapName));
            bitmapPath->setText(bitmapName);
            validation->clear();
            updateBackgroundPreview();
        }
    });
    QObject::connect(systemGallery, &QPushButton::clicked, &dialog, [&]() { //20261123-3 add
        HmiObject adapter;
        QString selected = systemPicture;
        QByteArray galleryPng;
        QMap<QString, QByteArray> galleryPool;

        // The shared picker owns browsing and selection; only its catalog ID enters the page draft.
        adapter.type = QStringLiteral("switch");
        adapter.address = QStringLiteral("LB0");
        adapter.hasBitButtonProperties = true;
        BitButtonDialog picker(adapter, project, &dialog);
        if (picker.choosePictureForControl(false, &selected, &galleryPng, &galleryPool) && !selected.isEmpty())
        {
            if (kvgPictureImage(selected, QSize(1, 1), 0).isNull())
            {
                validation->setText(QStringLiteral("无法解析所选系统图片，请选择其他背景。"));
                return;
            }
            systemPicture = selected;
            bitmapName.clear();
            bitmap = QImage();
            bitmapPath->setText(kvgPictureResource(systemPicture));
            validation->clear();
            updateBackgroundPreview();
        }
    });
    layout->addRow(createStage2PageScriptList(&dialog, macroDraft, scripts)); //20261123-5 add
    layout->addRow(validation);
    layout->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        HmiPage page;
        QString error;
        HmiConfigurationNode slide; //20261123-4 add
        HmiReference target;
        bool preserveSlide;
        int direction;

        if (bitmapMode->isChecked() && bitmap.isNull() && systemPicture.isEmpty()) //20261123-3 change
        {
            validation->setText(QStringLiteral("请从系统图库或工程图库选择一张有效的背景图片。"));
            return;
        }
        page.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        page.name = name->text();
        page.number = number->value(); //20261122-10 add: The chosen value participates in the model's add-page transaction.
        page.background = fillDraft(); //20261123-1 change
        candidate = macroDraft; //20261123-5 change: Commit shared source edits only with the outer page.
        // Activate by the new ID so the device startup page is left independent.
        if (!validateStage2PageName(project, QString(), page.name, &error) ||
            !candidate.addPage(page, &error)) //20261123-4 change
        {
            validation->setText(error);
            return;
        }
        slide.id = QUuid::createUuid().toString(QUuid::WithoutBraces); //20261123-4 add
        slide.kind = QStringLiteral("pageSlide");
        slide.name = slideGroup->title();
        slide.ownerPageId = page.id;
        slide.implemented = true;
        slide.active = true;
        slide.properties = {{QStringLiteral("enabled"), slideGroup->isChecked()},
                            {QStringLiteral("distance"), slideDistance->value()}};
        preserveSlide = slideGroup->isChecked() || slideDistance->value() != 50;
        for (direction = 0; direction < 4; ++direction)
        {
            if (!slideDirections[direction]->isChecked())
            {
                continue;
            }
            target.targetId = slideTargets[direction]->currentData().toString();
            if (target.targetId.isEmpty())
            {
                validation->setText(QStringLiteral("请选择“%1”的目标画面。").arg(slideLabels.at(direction)));
                return;
            }
            target.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            target.ownerKind = QStringLiteral("configuration");
            target.ownerId = slide.id;
            target.targetKind = QStringLiteral("page");
            target.field = slideKeys.at(direction).toLower();
            candidate.references.append(target);
            preserveSlide = true;
        }
        if (preserveSlide)
        {
            // Keep selected directions if the master switch was turned back off before confirming.
            candidate.configurationNodes.append(slide);
        }
        storeStage2PageScripts(candidate, page.id, scripts); //20261123-5 add
        if (!candidate.selectPage(page.id, &error))
        {
            validation->setText(error);
            return;
        }
        confirmed = true;
        dialog.accept();
    });
    name->selectAll();
    if (dialog.exec() != QDialog::Accepted || !confirmed)
    {
        return false;
    }
    project = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : addLanguageChoices
 * Description     : Keep inherited startup language distinct from an explicit language selection.
 * Input Parameters: combo, project, inherited
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
static void addLanguageChoices(QComboBox *combo, const HmiProject &project, bool inherited) //20261110-7 add
{
    QString language;

    if (inherited) { combo->addItem(QStringLiteral("沿用工程启动语言"), QString()); }
    Q_FOREACH (language, project.languages) { combo->addItem(language, language); }
}

/******************************************************************************
 * Function Name   : Stage2NewProjectDialog::Stage2NewProjectDialog
 * Description     : Build the observed three-step workflow without creating a directory or file.
 * Input Parameters: seed, suggestedPath, parent
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
Stage2NewProjectDialog::Stage2NewProjectDialog(const HmiProject &seed, const QString &suggestedPath,
                                             QWidget *parent) //20261110-7 add
    : QDialog(parent), m_working(seed)
{
    QVBoxLayout *layout;
    QHBoxLayout *navigation;
    QFormLayout *identityForm;
    QFormLayout *pageForm;
    QFormLayout *connectionForm;
    QWidget *identityStep;
    QWidget *connectionStep;
    QWidget *pageStep;
    QLineEdit *manufacturer;
    QLineEdit *series;
    QLineEdit *connectionName;
    QRadioButton *logo;
    QRadioButton *noLogo;
    QLabel *logoBoundary;
    QSpinBox *logoTime;
    QPushButton *cancel;

    setObjectName(QStringLiteral("stage2NewProjectDialog"));
    setWindowTitle(QStringLiteral("新项目"));
    resize(590, 330);
    layout = new QVBoxLayout(this);
    m_steps = new QStackedWidget(this);
    m_steps->setObjectName(QStringLiteral("wizardSteps"));
    identityStep = new QWidget(m_steps);
    identityForm = new QFormLayout(identityStep);
    m_name = new QLineEdit(identityStep);
    m_name->setObjectName(QStringLiteral("projectNameEdit"));
    m_name->setMaxLength(32767);
    m_path = new QLineEdit(suggestedPath, identityStep);
    m_path->setObjectName(QStringLiteral("projectPathEdit"));
    m_target = new QComboBox(identityStep); //20261122-3 change
    m_target->setObjectName(QStringLiteral("targetModelEdit")); //20261122-4 change
    populateStage2TargetChoices(m_target, seed.targetModel);
    identityForm->addRow(QStringLiteral("工程名称"), m_name);
    identityForm->addRow(QStringLiteral("工程文件（.ksp）"), m_path);
    identityForm->addRow(QStringLiteral("目标型号"), m_target);
    identityForm->addRow(QStringLiteral("分辨率"), new QLabel(QStringLiteral("800 × 480"), identityStep));
    identityForm->addRow(QStringLiteral("显示方向"), new QLabel(QStringLiteral("水平"), identityStep));
    connectionStep = new QWidget(m_steps);
    connectionForm = new QFormLayout(connectionStep);
    manufacturer = new QLineEdit(QStringLiteral("MODBUS"), connectionStep);
    manufacturer->setObjectName(QStringLiteral("connectionManufacturerEdit"));
    manufacturer->setReadOnly(true);
    series = new QLineEdit(QStringLiteral("Modbus_RTU"), connectionStep);
    series->setObjectName(QStringLiteral("connectionSeriesEdit"));
    series->setReadOnly(true);
    connectionName = new QLineEdit(QStringLiteral("Modbus_RTU"), connectionStep);
    connectionName->setObjectName(QStringLiteral("connectionNameEdit"));
    connectionName->setReadOnly(true);
    connectionForm->addRow(QStringLiteral("制造商"), manufacturer);
    connectionForm->addRow(QStringLiteral("系列"), series);
    connectionForm->addRow(QStringLiteral("名称"), connectionName);
    connectionForm->addRow(new QLabel(QStringLiteral("此向导使用已验证的默认通信配置；其他连接可在工程连接设置中编辑。"), connectionStep));
    pageStep = new QWidget(m_steps);
    pageForm = new QFormLayout(pageStep);
    logo = new QRadioButton(QStringLiteral("需要启动画面"), pageStep);
    logo->setObjectName(QStringLiteral("startupLogoRadio"));
    logo->setEnabled(false);
    logo->setToolTip(QStringLiteral("上电Logo与启动动画的非默认设备编码尚未恢复。"));
    noLogo = new QRadioButton(QStringLiteral("不需要启动画面"), pageStep);
    noLogo->setObjectName(QStringLiteral("noStartupLogoRadio"));
    noLogo->setEnabled(false);
    // Preserve the verified baseline behavior without inferring byte identity with the native wizard sample.
    noLogo->setChecked(true);
    logoTime = new QSpinBox(pageStep);
    logoTime->setObjectName(QStringLiteral("startupLogoTimeSpin"));
    logoTime->setRange(1, 60);
    logoTime->setValue(1);
    logoTime->setSuffix(QStringLiteral(" 秒"));
    logoTime->setEnabled(false);
    pageForm->addRow(logo);
    pageForm->addRow(noLogo);
    pageForm->addRow(QStringLiteral("单个启动画面显示时间"), logoTime);
    logoBoundary = new QLabel(QStringLiteral("沿用已验证基线的不显示开机Logo或动画设置；下面的普通启动画面与其独立。"), pageStep);
    logoBoundary->setObjectName(QStringLiteral("startupLogoBoundaryLabel"));
    logoBoundary->setWordWrap(true);
    pageForm->addRow(logoBoundary);
    m_pageName = new QLineEdit(seed.pageName, pageStep);
    m_pageName->setObjectName(QStringLiteral("pageNameEdit"));
    m_pageName->setMaxLength(qMax<int>(32767, seed.pageName.size()));
    m_startupLanguage = new QComboBox(pageStep);
    m_startupLanguage->setObjectName(QStringLiteral("startupLanguageCombo"));
    addLanguageChoices(m_startupLanguage, seed, true);
    m_startupLanguage->setCurrentIndex(m_startupLanguage->findData(seed.startupLanguage));
    pageForm->addRow(QStringLiteral("初始画面名称"), m_pageName);
    pageForm->addRow(QStringLiteral("设备启动语言"), m_startupLanguage);
    pageForm->addRow(new QLabel(QStringLiteral("完成后交由主窗口保存工程；取消会放弃本次草稿。"), pageStep));
    m_steps->addWidget(identityStep);
    m_steps->addWidget(connectionStep);
    m_steps->addWidget(pageStep);
    layout->addWidget(m_steps);
    m_error = new QLabel(this);
    m_error->setObjectName(QStringLiteral("validationErrorLabel"));
    m_error->setWordWrap(true);
    layout->addWidget(m_error);
    navigation = new QHBoxLayout;
    navigation->addStretch();
    m_back = new QPushButton(QStringLiteral("上一步"), this);
    m_back->setObjectName(QStringLiteral("wizardBackButton"));
    m_next = new QPushButton(QStringLiteral("下一步"), this);
    m_next->setObjectName(QStringLiteral("wizardNextButton"));
    m_finish = new QPushButton(QStringLiteral("完成"), this);
    m_finish->setObjectName(QStringLiteral("wizardFinishButton"));
    cancel = new QPushButton(QStringLiteral("取消"), this);
    cancel->setObjectName(QStringLiteral("wizardCancelButton"));
    navigation->addWidget(m_back);
    navigation->addWidget(m_next);
    navigation->addWidget(m_finish);
    navigation->addWidget(cancel);
    layout->addLayout(navigation);
    connect(m_back, &QPushButton::clicked, this, [this]() { updateStep(m_steps->currentIndex() - 1); });
    connect(m_next, &QPushButton::clicked, this, [this]()
    {
        HmiProject candidate;
        QString error;

        if (!collectDraft(&candidate, false, &error)) { m_error->setText(error); return; }
        updateStep(m_steps->currentIndex() + 1);
    });
    connect(m_finish, &QPushButton::clicked, this, &Stage2NewProjectDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    updateStep(0);
}

/******************************************************************************
 * Function Name   : Stage2NewProjectDialog::draftProject
 * Description     : Return the accepted value for publication by the caller.
 * Input Parameters: None
 * Return Value    : Detached project value; caller must check Accepted.
 * Date            : 2026.11.10
 ******************************************************************************/
HmiProject Stage2NewProjectDialog::draftProject() const //20261110-7 add
{
    return m_working;
}

/******************************************************************************
 * Function Name   : Stage2NewProjectDialog::projectPath
 * Description     : Return the normalized destination without writing it.
 * Input Parameters: None
 * Return Value    : Absolute file path.
 * Date            : 2026.11.10
 ******************************************************************************/
QString Stage2NewProjectDialog::projectPath() const //20261110-7 add
{
    if (result() != QDialog::Accepted) { return QString(); }
    return QDir::cleanPath(m_path->text().trimmed());
}

/******************************************************************************
 * Function Name   : Stage2NewProjectDialog::collectDraft
 * Description     : Validate project identity and a fresh writable destination before accepting a draft.
 * Input Parameters: candidate, includePage, error
 * Return Value    : True for a valid detached model and unused absolute .ksp destination.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2NewProjectDialog::collectDraft(HmiProject *candidate, bool includePage, QString *error) const //20261110-7 add
{
    QFileInfo destination;
    QFileInfo directory;
    QString path;

    if (candidate == nullptr) { return stage2Error(error, QStringLiteral("工程草稿输出无效。")); }
    if (m_name->text().trimmed().isEmpty()) { return stage2Error(error, QStringLiteral("请输入工程名称。")); }
    path = QDir::cleanPath(m_path->text().trimmed());
    destination = QFileInfo(path);
    directory = QFileInfo(destination.absolutePath());
    if (m_path->text().trimmed().isEmpty() || !destination.isAbsolute() ||
        destination.suffix().compare(QStringLiteral("ksp"), Qt::CaseInsensitive) != 0 ||
        destination.fileName() == QStringLiteral(".ksp"))
    {
        return stage2Error(error, QStringLiteral("请选择带文件名的绝对 .ksp 工程路径。"));
    }
    if (destination.exists()) { return stage2Error(error, QStringLiteral("新建工程路径已存在，请选择其他文件名。")); }
    if (!directory.exists() || !directory.isDir() || !directory.isWritable())
    {
        return stage2Error(error, QStringLiteral("工程所在目录不存在或不可写。"));
    }
    *candidate = m_working;
    candidate->name = m_name->text();
    candidate->targetModel = m_target->currentData().toString(); //20261122-3 add
    if (includePage)
    {
        candidate->pageName = m_pageName->text();
        candidate->startupLanguage = m_startupLanguage->currentData().toString();
    }
    return candidate->validateModel(error);
}

/******************************************************************************
 * Function Name   : Stage2NewProjectDialog::updateStep
 * Description     : Change wizard visibility while preserving all steps' input widgets.
 * Input Parameters: step
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2NewProjectDialog::updateStep(int step) //20261110-7 add
{
    QStringList titles;

    titles = {QStringLiteral("新项目"), QStringLiteral("通信连接"), QStringLiteral("启动画面设置")};
    m_steps->setCurrentIndex(step);
    setWindowTitle(titles.value(step));
    m_back->setEnabled(step != 0);
    m_next->setVisible(step < m_steps->count() - 1);
    m_finish->setVisible(step == m_steps->count() - 1);
    m_finish->setDefault(step == m_steps->count() - 1);
    m_next->setDefault(step < m_steps->count() - 1);
    m_error->clear();
}

/******************************************************************************
 * Function Name   : Stage2NewProjectDialog::accept
 * Description     : Publish only a completely validated draft and leave all file I/O to its caller.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2NewProjectDialog::accept() //20261110-7 add
{
    HmiProject candidate;
    QString error;

    if (m_steps->currentIndex() < m_steps->count() - 1) { m_next->click(); return; }
    if (!collectDraft(&candidate, true, &error)) { m_error->setText(error); return; }
    m_working = candidate;
    QDialog::accept();
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::Stage2ProjectDialog
 * Description     : Edit project identity, page transactions and independent startup choices on a copy.
 * Input Parameters: project, parent
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
Stage2ProjectDialog::Stage2ProjectDialog(const HmiProject &project, QWidget *parent) //20261110-7 add
    : QDialog(parent), m_working(project)
{
    QVBoxLayout *layout;
    QVBoxLayout *pageLayout;
    QHBoxLayout *actions;
    QFormLayout *projectForm;
    QFormLayout *pageForm;
    QTabWidget *tabs;
    QWidget *projectTab;
    QWidget *pagesTab;
    QPushButton *add;
    QPushButton *rename;
    QPushButton *copy;
    QPushButton *remove;
    QLabel *boundary;

    setObjectName(QStringLiteral("stage2ProjectDialog"));
    setWindowTitle(QStringLiteral("工程与画面设置"));
    resize(660, 560);
    layout = new QVBoxLayout(this);
    tabs = new QTabWidget(this);
    tabs->setObjectName(QStringLiteral("projectSettingsTabs"));
    projectTab = new QWidget(tabs);
    projectForm = new QFormLayout(projectTab);
    m_projectName = new QLineEdit(project.name, projectTab);
    m_projectName->setObjectName(QStringLiteral("projectNameEdit"));
    m_projectName->setMaxLength(qMax<int>(32767, project.name.size()));
    m_target = new QComboBox(projectTab); //20261122-3 change
    m_target->setObjectName(QStringLiteral("targetModelEdit")); //20261122-4 change
    populateStage2TargetChoices(m_target, project.targetModel);
    m_target->setEnabled(project.nativeSourceSnapshot.isEmpty()); // The native snapshot retains its own model selector.
    m_editorPage = new QComboBox(projectTab);
    m_editorPage->setObjectName(QStringLiteral("editorPageCombo"));
    m_startupPage = new QComboBox(projectTab);
    m_startupPage->setObjectName(QStringLiteral("startupPageCombo"));
    m_editorLanguage = new QComboBox(projectTab);
    m_editorLanguage->setObjectName(QStringLiteral("editorLanguageCombo"));
    m_startupLanguage = new QComboBox(projectTab);
    m_startupLanguage->setObjectName(QStringLiteral("startupLanguageCombo"));
    addLanguageChoices(m_editorLanguage, project, false);
    addLanguageChoices(m_startupLanguage, project, true);
    projectForm->addRow(QStringLiteral("工程名称"), m_projectName);
    projectForm->addRow(QStringLiteral("目标型号"), m_target);
    projectForm->addRow(QStringLiteral("当前编辑画面"), m_editorPage);
    projectForm->addRow(QStringLiteral("设备启动画面"), m_startupPage);
    projectForm->addRow(QStringLiteral("当前编辑语言"), m_editorLanguage);
    projectForm->addRow(QStringLiteral("设备启动语言"), m_startupLanguage);
    pagesTab = new QWidget(tabs);
    pageLayout = new QVBoxLayout(pagesTab);
    m_pages = new QListWidget(pagesTab);
    m_pages->setObjectName(QStringLiteral("pageList"));
    pageLayout->addWidget(m_pages);
    pageForm = new QFormLayout;
    m_pageName = new QLineEdit(pagesTab);
    m_pageName->setObjectName(QStringLiteral("pageNameEdit"));
    m_pageName->setMaxLength(32767);
    m_pageKind = new QComboBox(pagesTab);
    m_pageKind->setObjectName(QStringLiteral("pageKindCombo"));
    m_pageKind->addItem(QStringLiteral("普通画面"), QStringLiteral("normal"));
    m_pageKind->addItem(QStringLiteral("窗口"), QStringLiteral("window"));
    m_width = new QSpinBox(pagesTab);
    m_width->setObjectName(QStringLiteral("pageWidthSpin"));
    m_width->setRange(1, 800);
    m_width->setValue(800);
    m_height = new QSpinBox(pagesTab);
    m_height->setObjectName(QStringLiteral("pageHeightSpin"));
    m_height->setRange(1, 480);
    m_height->setValue(480);
    pageForm->addRow(QStringLiteral("名称（新增／复制时填写新名称）"), m_pageName);
    pageForm->addRow(QStringLiteral("新增类型"), m_pageKind);
    pageForm->addRow(QStringLiteral("新增窗口宽度"), m_width);
    pageForm->addRow(QStringLiteral("新增窗口高度"), m_height);
    pageLayout->addLayout(pageForm);
    actions = new QHBoxLayout;
    add = new QPushButton(QStringLiteral("新增"), pagesTab);
    add->setObjectName(QStringLiteral("addPageButton"));
    rename = new QPushButton(QStringLiteral("改名"), pagesTab);
    rename->setObjectName(QStringLiteral("renamePageButton"));
    copy = new QPushButton(QStringLiteral("复制"), pagesTab);
    copy->setObjectName(QStringLiteral("copyPageButton"));
    remove = new QPushButton(QStringLiteral("删除"), pagesTab);
    remove->setObjectName(QStringLiteral("removePageButton"));
    actions->addWidget(add);
    actions->addWidget(rename);
    actions->addWidget(copy);
    actions->addWidget(remove);
    pageLayout->addLayout(actions);
    tabs->addTab(projectTab, QStringLiteral("工程设置"));
    tabs->addTab(pagesTab, QStringLiteral("画面管理"));
    layout->addWidget(tabs);
    boundary = new QLabel(QStringLiteral("多画面和窗口可以保存、重开；当前设备下载仅支持单个800×480普通画面。"), this);
    boundary->setObjectName(QStringLiteral("backendBoundaryLabel"));
    boundary->setWordWrap(true);
    layout->addWidget(boundary);
    m_error = new QLabel(this);
    m_error->setObjectName(QStringLiteral("validationErrorLabel"));
    m_error->setWordWrap(true);
    layout->addWidget(m_error);
    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttons->setObjectName(QStringLiteral("buttonBox"));
    layout->addWidget(m_buttons);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &Stage2ProjectDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_pages, &QListWidget::currentRowChanged, this, [this](int) { showSelectedPage(); });
    connect(add, &QPushButton::clicked, this, &Stage2ProjectDialog::addFromFields);
    connect(rename, &QPushButton::clicked, this, &Stage2ProjectDialog::renameFromFields);
    connect(copy, &QPushButton::clicked, this, &Stage2ProjectDialog::copyFromFields);
    connect(remove, &QPushButton::clicked, this, &Stage2ProjectDialog::removeSelected);
    connect(m_editorPage, &QComboBox::currentIndexChanged, this, [this](int)
    {
        QString error;

        if (!selectPageDraft(m_editorPage->currentData().toString(), &error)) { reportError(error); }
    });
    connect(m_startupPage, &QComboBox::currentIndexChanged, this, [this](int)
    {
        QString error;

        if (!setStartupDraft(m_startupPage->currentData().toString(), m_working.startupLanguage, &error)) { reportError(error); }
    });
    connect(m_editorLanguage, &QComboBox::currentIndexChanged, this, [this](int)
    {
        QString error;

        if (!setEditorLanguageDraft(m_editorLanguage->currentData().toString(), &error)) { reportError(error); }
    });
    connect(m_startupLanguage, &QComboBox::currentIndexChanged, this, [this](int)
    {
        QString error;

        if (!setStartupDraft(m_working.startupPageId, m_startupLanguage->currentData().toString(), &error)) { reportError(error); }
    });
    connect(m_pageKind, &QComboBox::currentIndexChanged, this, [this](int)
    {
        bool window;

        window = m_pageKind->currentData().toString() == QStringLiteral("window");
        if (window) { m_width->setValue(240); m_height->setValue(180); }
        m_width->setEnabled(window);
        m_height->setEnabled(window);
    });
    refresh(project.activePageId);
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::draftProject
 * Description     : Read the detached transaction without touching the caller's active model.
 * Input Parameters: None
 * Return Value    : Project draft; publish only after Accepted.
 * Date            : 2026.11.10
 ******************************************************************************/
HmiProject Stage2ProjectDialog::draftProject() const //20261110-7 add
{
    return m_working;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::addPageDraft
 * Description     : Append only supported editor page roles through the model's atomic transaction.
 * Input Parameters: page, error
 * Return Value    : True on valid addition; keyboard and Logo creation await native field evidence.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::addPageDraft(const HmiPage &page, QString *error) //20261110-7 add
{
    if (!validateStage2PageName(m_working, QString(), page.name, error)) { return false; } //20261122-9 change
    if (page.kind != QStringLiteral("normal") && page.kind != QStringLiteral("window"))
    {
        return stage2Error(error, QStringLiteral("此入口仅创建普通画面或窗口；键盘和Logo字段尚未核定。"));
    }
    if (page.kind == QStringLiteral("normal") && (page.width != 800 || page.height != 480))
    {
        return stage2Error(error, QStringLiteral("当前AMT普通画面尺寸必须为800×480。"));
    }
    if (!m_working.addPage(page, error)) { return false; }
    refresh(page.id);
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::renamePageDraft
 * Description     : Rename the selected entity while preserving all inbound reference targets.
 * Input Parameters: id, name, error
 * Return Value    : True on validated rename.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::renamePageDraft(const QString &id, const QString &name, QString *error) //20261110-7 add
{
    if (!validateStage2PageName(m_working, id, name, error) || !m_working.renamePage(id, name, error)) { return false; } //20261122-9 change
    refresh(id);
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::copyPageDraft
 * Description     : Delegate every cloned page, object and owned-reference identity remap to the model.
 * Input Parameters: id, name, newId, error
 * Return Value    : True after an independent clone validates.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::copyPageDraft(const QString &id, const QString &name, QString *newId, QString *error) //20261110-7 add
{
    QString copiedId;

    if (!validateStage2PageName(m_working, QString(), name, error) || !m_working.copyPage(id, name, &copiedId, error)) { return false; } //20261122-9 change
    if (newId != nullptr) { *newId = copiedId; }
    refresh(copiedId);
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::removePageDraft
 * Description     : Refuse startup and inbound-reference deletion before changing the draft list.
 * Input Parameters: id, error
 * Return Value    : True after atomic removal.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::removePageDraft(const QString &id, QString *error) //20261110-7 add
{
    if (!m_working.removePage(id, error)) { return false; }
    refresh(m_working.activePageId);
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::selectPageDraft
 * Description     : Change the editor's active storage independently from device startup selection.
 * Input Parameters: id, error
 * Return Value    : True after model selection validates.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::selectPageDraft(const QString &id, QString *error) //20261110-7 add
{
    if (!m_working.selectPage(id, error)) { return false; }
    refresh(id);
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::setStartupDraft
 * Description     : Validate startup page role and language atomically without changing the editor page.
 * Input Parameters: id, language, error
 * Return Value    : True after both startup fields validate.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::setStartupDraft(const QString &id, const QString &language, QString *error) //20261110-7 add
{
    HmiProject candidate;

    candidate = m_working;
    candidate.startupPageId = id;
    candidate.startupLanguage = language;
    if (!candidate.validateModel(error)) { return false; }
    m_working = candidate;
    refresh(selectedPageId());
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::setEditorLanguageDraft
 * Description     : Change current editing language without silently changing the device startup choice.
 * Input Parameters: language, error
 * Return Value    : True after a registered language is selected.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage2ProjectDialog::setEditorLanguageDraft(const QString &language, QString *error) //20261110-7 add
{
    HmiProject candidate;

    candidate = m_working;
    candidate.activeLanguage = language;
    if (!candidate.validateModel(error)) { return false; }
    m_working = candidate;
    refresh(selectedPageId());
    return true;
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::refresh
 * Description     : Rebuild labels from stable IDs and block accidental combo/list selection transactions.
 * Input Parameters: selectedId
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::refresh(const QString &selectedId) //20261110-7 add
{
    QSignalBlocker listBlocker(m_pages);
    QSignalBlocker editorBlocker(m_editorPage);
    QSignalBlocker startupBlocker(m_startupPage);
    QSignalBlocker editorLanguageBlocker(m_editorLanguage);
    QSignalBlocker startupLanguageBlocker(m_startupLanguage);
    HmiPage page;
    QListWidgetItem *item;
    int row;

    m_pages->clear();
    m_editorPage->clear();
    m_startupPage->clear();
    row = 0;
    Q_FOREACH (page, m_working.allPages())
    {
        item = new QListWidgetItem(QStringLiteral("%1  [%2]  %3×%4").arg(page.name, page.kind).arg(page.width).arg(page.height), m_pages);
        item->setData(Qt::UserRole, page.id);
        m_editorPage->addItem(page.name, page.id);
        if (page.kind == QStringLiteral("normal")) { m_startupPage->addItem(page.name, page.id); }
        if (page.id == selectedId) { m_pages->setCurrentRow(row); }
        ++row;
    }
    if (m_pages->currentRow() < 0 && m_pages->count() != 0) { m_pages->setCurrentRow(0); }
    m_editorPage->setCurrentIndex(m_editorPage->findData(m_working.activePageId));
    m_startupPage->setCurrentIndex(m_startupPage->findData(m_working.startupPageId));
    m_editorLanguage->setCurrentIndex(m_editorLanguage->findData(m_working.activeLanguage));
    m_startupLanguage->setCurrentIndex(m_startupLanguage->findData(m_working.startupLanguage));
    m_error->clear();
    showSelectedPage();
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::selectedPageId
 * Description     : Read stable identity instead of treating a visual row number as a device page ID.
 * Input Parameters: None
 * Return Value    : Selected stable page ID or empty.
 * Date            : 2026.11.10
 ******************************************************************************/
QString Stage2ProjectDialog::selectedPageId() const //20261110-7 add
{
    if (m_pages->currentItem() == nullptr) { return QString(); }
    return m_pages->currentItem()->data(Qt::UserRole).toString();
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::showSelectedPage
 * Description     : Populate page fields while keeping unavailable native roles preserved in the draft.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::showSelectedPage() //20261110-7 add
{
    HmiPage page;
    QString id;
    bool window;

    id = selectedPageId();
    Q_FOREACH (page, m_working.allPages())
    {
        if (page.id != id) { continue; }
        m_pageName->setMaxLength(qMax<int>(32767, page.name.size()));
        m_pageName->setText(page.name);
        m_pageKind->setCurrentIndex(m_pageKind->findData(page.kind));
        m_width->setValue(page.width);
        m_height->setValue(page.height);
        window = page.kind == QStringLiteral("window");
        m_width->setEnabled(window);
        m_height->setEnabled(window);
        return;
    }
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::addFromFields
 * Description     : Generate an editor identity and add the fully validated page fields to the draft.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::addFromFields() //20261110-7 add
{
    HmiPage page;
    QString error;

    page.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    page.kind = m_pageKind->currentData().toString();
    page.name = m_pageName->text();
    page.width = page.kind == QStringLiteral("window") ? m_width->value() : 800;
    page.height = page.kind == QStringLiteral("window") ? m_height->value() : 480;
    if (!addPageDraft(page, &error)) { reportError(error); }
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::renameFromFields
 * Description     : Apply the name field only, never reinterpret geometry or role as an unverified edit.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::renameFromFields() //20261110-7 add
{
    QString error;

    if (!renamePageDraft(selectedPageId(), m_pageName->text(), &error)) { reportError(error); }
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::copyFromFields
 * Description     : Clone the selected entity with the supplied fresh name through the model transaction.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::copyFromFields() //20261110-7 add
{
    QString error;

    if (!copyPageDraft(selectedPageId(), m_pageName->text(), nullptr, &error)) { reportError(error); }
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::removeSelected
 * Description     : Remove a page inside the outer cancelable draft after reference validation.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::removeSelected() //20261110-7 add
{
    QString error;

    if (!removePageDraft(selectedPageId(), &error)) { reportError(error); }
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::reportError
 * Description     : Keep the edit dialog open and display a failed transaction without partial mutation.
 * Input Parameters: error
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::reportError(const QString &error) //20261110-7 add
{
    m_error->setText(error);
}

/******************************************************************************
 * Function Name   : Stage2ProjectDialog::accept
 * Description     : Validate identity and all editor/startup choices before publishing the final draft.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage2ProjectDialog::accept() //20261110-7 add
{
    HmiProject candidate;
    QString error;

    candidate = m_working;
    candidate.name = m_projectName->text();
    candidate.targetModel = m_target->currentData().toString(); //20261122-3 add
    candidate.startupPageId = m_startupPage->currentData().toString();
    candidate.startupLanguage = m_startupLanguage->currentData().toString();
    candidate.activeLanguage = m_editorLanguage->currentData().toString();
    if (!candidate.validateModel(&error) || !candidate.selectPage(m_editorPage->currentData().toString(), &error))
    {
        reportError(error);
        return;
    }
    m_working = candidate;
    QDialog::accept();
}
