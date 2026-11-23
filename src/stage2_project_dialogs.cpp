#include "stage2_project_dialogs.h"
#include "bit_button_dialog.h" //20261123-3 add
#include "control_rendering.h" //20261123-2 add
#include "kvg_library.h" //20261123-3 add
#include "local_help.h" //20261123-10 add
#include "macro_workspace.h" //20261123-5 add
#include "plc_address_dialog.h" //20261123-5 add
#include "stage2_system_settings.h" //20261122-4 change

#include <QCheckBox> //20261123-4 add

#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QFont> //20261123-11 add
#include <QFrame> //20261123-11 add
#include <QFormLayout>
#include <QGridLayout> //20261123-1 add
#include <QGroupBox>
#include <QHeaderView> //20261123-5 add
#include <QHBoxLayout>
#include <QJsonArray> //20261123-5 add
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox> //20261123-8 add
#include <QPainter> //20261123-1 add
#include <QPalette> //20261123-8 add
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
#include <algorithm> //20261123-6 add

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
    QString rowId; //20261123-6 add: Preserve existing row and reference identities when editing a page.
    QString bindingId;
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
 * Input Parameters: parent, project, scripts, popup.
 * Return Value    : Editor widget owned by the page dialog.
 * Date            : 2026.11.23
 ******************************************************************************/
static QWidget *createStage2PageScriptList(QWidget *parent, HmiProject &project,
                                           QList<Stage2PageScript> &scripts, bool popup) //20261123-12 change
{
    QWidget *widget = new QWidget(parent);
    QTableWidget *table = new QTableWidget(0, 5, widget);
    QPushButton *add = new QPushButton(QStringLiteral("新增"), widget);
    QPushButton *remove = new QPushButton(QStringLiteral("删除"), widget);
    QPushButton *edit = new QPushButton(QStringLiteral("修改"), widget);
    QLabel *usage = new QLabel(widget);

    // The script list and its actions occupy the same client rectangles as in V5.1.
    table->setObjectName(QStringLiteral("pageMacroTable")); //20261123-11 add
    table->setGeometry(0, 0, popup ? 591 : 601, popup ? 211 : 221); //20261123-12 change
    table->setHorizontalHeaderLabels({QStringLiteral("脚本函数"), QStringLiteral("执行频率"),
        QStringLiteral("执行次数"), QStringLiteral("受控类型"), QStringLiteral("受控地址")});
    table->verticalHeader()->hide();
    table->horizontalHeader()->setDefaultSectionSize(100); //20261123-11 add
    table->horizontalHeader()->setFixedHeight(25);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    add->setObjectName(QStringLiteral("pageMacroAdd"));
    add->setGeometry(popup ? QRect(0, 220, 61, 31) : QRect(0, 230, 75, 31)); //20261123-12 change
    remove->setGeometry(popup ? QRect(70, 220, 71, 31) : QRect(80, 230, 75, 31));
    edit->setGeometry(popup ? QRect(150, 220, 71, 31) : QRect(160, 230, 75, 31));
    usage->setGeometry(popup ? QRect(260, 220, 100, 30) : QRect(253, 230, 71, 30));
    add->setAutoDefault(false);
    remove->setAutoDefault(false);
    edit->setAutoDefault(false);
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
    };
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

        if (row >= 0 && row < scripts.size() && !table->selectedItems().isEmpty()) //20261123-11 change
        {
            scripts.removeAt(row);
            refresh(qMin(row, int(scripts.size()) - 1));
        }
    });
    QObject::connect(edit, &QPushButton::clicked, widget, [=, &project, &scripts]() {
        const int row = table->currentRow();

        if (row >= 0 && row < scripts.size() && !table->selectedItems().isEmpty() && //20261123-11 change
            editStage2PageScript(widget, project, scripts[row]))
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
 * Input Parameters: project, page, scripts, previous.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
static void storeStage2PageScripts(HmiProject &project, const HmiPage &page, //20261123-12 change
                                   const QList<Stage2PageScript> &scripts,
                                   const HmiConfigurationNode &previous) //20261123-6 change
{
    HmiConfigurationNode node = previous;
    QJsonArray rows;
    QString rowId;

    if (scripts.isEmpty())
    {
        return;
    }
    if (node.id.isEmpty()) //20261123-6 add
    {
        node.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    node.kind = QStringLiteral("pageMacro");
    node.name = page.kind == QStringLiteral("window") ? QStringLiteral("窗口宏") : QStringLiteral("画面宏"); //20261123-12 change
    node.ownerPageId = page.id;
    node.implemented = true;
    node.active = true;
    for (const Stage2PageScript &script : scripts)
    {
        rowId = script.rowId.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : script.rowId; //20261123-6 change
        rows.append(QJsonObject{{QStringLiteral("id"), rowId},
            {QStringLiteral("frequency"), script.frequency}, {QStringLiteral("count"), script.count},
            {QStringLiteral("address"), script.address}});
        project.references.append({script.bindingId.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) :
            script.bindingId, //20261123-6 change
            QStringLiteral("configuration"), node.id, rowId, QStringLiteral("macro"), script.macroId});
    }
    node.properties.insert(QStringLiteral("scripts"), rows);
    project.configurationNodes.append(node);
}

/******************************************************************************
 * Function Name   : loadStage2PageScripts
 * Description     : Read existing invocation rows in order and resolve their stable source references.
 * Input Parameters: project, node.
 * Return Value    : Existing page script draft.
 * Date            : 2026.11.23
 ******************************************************************************/
static QList<Stage2PageScript> loadStage2PageScripts(const HmiProject &project,
                                                   const HmiConfigurationNode &node) //20261123-6 add
{
    QList<Stage2PageScript> scripts;
    QMap<QString, HmiReference> bindings;
    Stage2PageScript script;
    QJsonObject row;
    HmiReference binding;

    if (node.id.isEmpty())
    {
        return scripts;
    }
    for (const HmiReference &reference : project.references)
    {
        if (reference.ownerKind == QStringLiteral("configuration") && reference.ownerId == node.id)
        {
            bindings.insert(reference.field, reference);
        }
    }
    for (const QJsonValue &value : node.properties.value(QStringLiteral("scripts")).toArray())
    {
        row = value.toObject();
        script.rowId = row.value(QStringLiteral("id")).toString();
        script.address = row.value(QStringLiteral("address")).toString();
        script.frequency = row.value(QStringLiteral("frequency")).toInt();
        script.count = row.value(QStringLiteral("count")).toInt();
        binding = bindings.value(script.rowId);
        script.bindingId = binding.id;
        script.macroId = binding.targetId;
        scripts.append(script);
    }
    return scripts;
}

struct Stage2WindowFields //20261123-12 add
{
    QSpinBox *width = nullptr;
    QSpinBox *height = nullptr;
    QSpinBox *x = nullptr;
    QSpinBox *y = nullptr;
    QRadioButton *position = nullptr;
    QCheckBox *close = nullptr;
};

/******************************************************************************
 * Function Name   : createStage2WindowFields
 * Description     : Lay out the observed popup size and position fields and restore their draft values.
 * Input Parameters: dialog, page, placement.
 * Return Value    : Window form controls owned by the dialog.
 * Date            : 2026.11.23
 ******************************************************************************/
static Stage2WindowFields createStage2WindowFields(QDialog &dialog, const HmiPage &page,
                                                   const QJsonObject &placement) //20261123-12 add
{
    QGroupBox *size = new QGroupBox(QStringLiteral("窗口显示大小"), &dialog);
    QGroupBox *position = new QGroupBox(QStringLiteral("窗口显示位置"), &dialog);
    QLabel *widthLabel = new QLabel(QStringLiteral("窗口宽度："), size);
    QLabel *heightLabel = new QLabel(QStringLiteral("窗口高度："), size);
    QLabel *xLabel = new QLabel(QStringLiteral("坐标X位置："), position);
    QLabel *yLabel = new QLabel(QStringLiteral("坐标Y位置："), position);
    QRadioButton *center = new QRadioButton(QStringLiteral("屏幕居中显示"), position);
    Stage2WindowFields fields;

    size->setGeometry(10, 210, 431, 51);
    position->setGeometry(10, 270, 431, 81);
    widthLabel->setGeometry(20, 20, 71, 25);
    heightLabel->setGeometry(240, 20, 71, 25);
    xLabel->setGeometry(15, 50, 80, 25);
    yLabel->setGeometry(225, 50, 80, 25);
    auto makeSpin = [](QWidget *host, const QString &name, const QRect &bounds, int limit, int value) {
        QSpinBox *spin = new QSpinBox(host);

        spin->setObjectName(name);
        spin->setProperty("referenceWindowSpin", true);
        spin->setGeometry(bounds);
        spin->setRange(0, limit);
        spin->setValue(value);
        return spin;
    };
    fields.width = makeSpin(size, QStringLiteral("windowWidth"), QRect(100, 20, 101, 25), 800, page.width);
    fields.height = makeSpin(size, QStringLiteral("windowHeight"), QRect(320, 20, 101, 25), 480, page.height);
    fields.x = makeSpin(position, QStringLiteral("windowX"), QRect(100, 50, 91, 25),
                        1000, placement.value(QStringLiteral("x")).toInt());
    fields.y = makeSpin(position, QStringLiteral("windowY"), QRect(310, 50, 91, 25),
                        1000, placement.value(QStringLiteral("y")).toInt());
    center->setGeometry(15, 23, 140, 16);
    fields.position = new QRadioButton(QStringLiteral("指定坐标位置"), position);
    fields.position->setObjectName(QStringLiteral("windowSpecified"));
    fields.position->setGeometry(160, 23, 140, 16);
    fields.close = new QCheckBox(QStringLiteral("关闭按钮"), position);
    fields.close->setObjectName(QStringLiteral("windowClose"));
    fields.close->setGeometry(305, 23, 100, 16);
    fields.close->setChecked(placement.value(QStringLiteral("closeButton")).toBool(true));
    center->setChecked(placement.value(QStringLiteral("centered")).toBool(true));
    fields.position->setChecked(!center->isChecked());
    auto showCoordinates = [fields, xLabel, yLabel](bool shown) {
        xLabel->setVisible(shown);
        yLabel->setVisible(shown);
        fields.x->setVisible(shown);
        fields.y->setVisible(shown);
    };
    QObject::connect(fields.position, &QRadioButton::toggled, position, showCoordinates);
    showCoordinates(fields.position->isChecked());
    return fields;
}

/******************************************************************************
 * Function Name   : storeStage2WindowPlacement
 * Description     : Retain one stable nondefault placement record owned by the popup page.
 * Input Parameters: project, pageId, fields, previous.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
static void storeStage2WindowPlacement(HmiProject &project, const QString &pageId, const Stage2WindowFields &fields,
                                       const HmiConfigurationNode &previous) //20261123-12 add
{
    HmiConfigurationNode placement = previous;
    const bool centered = !fields.position->isChecked();
    const bool close = fields.close->isChecked();
    const int x = fields.x->value();
    const int y = fields.y->value();

    if (placement.id.isEmpty() && centered && close && x == 0 && y == 0)
    {
        return;
    }
    if (placement.id.isEmpty())
    {
        placement.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        placement.name = QStringLiteral("窗口显示位置");
        placement.kind = QStringLiteral("windowPlacement");
        placement.ownerPageId = pageId;
        placement.active = true;
        placement.implemented = true;
    }
    placement.properties = {{QStringLiteral("centered"), centered}, {QStringLiteral("x"), x},
                            {QStringLiteral("y"), y}, {QStringLiteral("closeButton"), close}};
    project.configurationNodes.append(placement);
}

/******************************************************************************
 * Function Name   : editStage2DesignerPage
 * Description     : Create or edit an ordinary screen or popup in a detached transaction.
 * Input Parameters: parent, project, pageId (empty for creation), kind.
 * Return Value    : True after a valid transaction is confirmed.
 * Date            : 2026.11.23
 ******************************************************************************/
static bool editStage2DesignerPage(QWidget *parent, HmiProject &project, const QString &pageId,
                                    const QString &kind) //20261123-12 change
{
    QDialog dialog(parent);
    QLineEdit *name;
    QSpinBox *number;
    QLabel *nameLabel; //20261123-11 add
    QLabel *numberLabel;
    QGroupBox *background;
    QRadioButton *defaultMode; //20261123-2 add
    QRadioButton *bitmapMode;
    QStackedWidget *backgroundOptions;
    QWidget *colorOptions;
    QWidget *bitmapOptions;
    QLabel *backgroundLabel; //20261123-11 add
    QLabel *foregroundLabel;
    QLabel *styleLabel;
    QFrame *previewFrame;
    QPushButton *backgroundColorButton;
    QPushButton *foregroundColorButton;
    QComboBox *backgroundStyle;
    QPixmap solidStyleIcon(16, 16); //20261123-11 add
    QPainter stylePainter;
    QLabel *backgroundPreview;
    QLineEdit *bitmapPath;
    QPushButton *projectGallery;
    QPushButton *systemGallery; //20261123-3 add
    QColor backgroundColor = HmiPageBackground::defaultColor(); //20261123-1 change
    QColor foregroundColor = HmiPageBackground::defaultColor();
    QGroupBox *slideGroup; //20261123-4 add
    QLabel *distanceLabel; //20261123-11 add
    QSpinBox *slideDistance;
    QCheckBox *slideDirections[4];
    QComboBox *slideTargets[4];
    const QStringList slideLabels = {QStringLiteral("向左"), QStringLiteral("向右"),
                                     QStringLiteral("向上"), QStringLiteral("向下")};
    const QStringList slideKeys = {QStringLiteral("Left"), QStringLiteral("Right"),
                                   QStringLiteral("Up"), QStringLiteral("Down")};
    const bool popup = kind == QStringLiteral("window"); //20261123-12 add
    const QString pageLabel = popup ? QStringLiteral("窗口") : QStringLiteral("画面");
    const QMap<QString, int> pageNumbers = project.screenAndWindowNumbers(); //20261123-13 change
    const QList<HmiPage> targetPages = project.allPages();
    int directionIndex;
    int column;
    int row; //20261123-11 add
    const bool editing = !pageId.isEmpty(); //20261123-6 add
    HmiPage existing;
    HmiConfigurationNode priorSlide;
    HmiConfigurationNode priorScripts;
    HmiConfigurationNode priorPlacement; //20261123-12 add
    Stage2WindowFields windowFields;
    QMap<QString, HmiReference> priorDirections;
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

    if (editing) //20261123-6 add
    {
        if (!pageNumbers.contains(pageId))
        {
            return false;
        }
        for (const HmiPage &page : targetPages)
        {
            if (page.id == pageId)
            {
                existing = page;
                break;
            }
        }
        if (existing.kind != kind) //20261123-13 add: Do not open an ordinary screen through the popup property editor.
        {
            return false;
        }
        for (const HmiConfigurationNode &node : project.configurationNodes)
        {
            if (node.ownerPageId != pageId)
            {
                continue;
            }
            if (node.kind == QStringLiteral("pageSlide"))
            {
                priorSlide = node;
            }
            else if (node.kind == QStringLiteral("pageMacro"))
            {
                priorScripts = node;
            }
            else if (node.kind == QStringLiteral("windowPlacement")) //20261123-12 add
            {
                priorPlacement = node;
            }
        }
        for (const HmiReference &reference : project.references)
        {
            if (reference.ownerKind == QStringLiteral("configuration") && reference.ownerId == priorSlide.id)
            {
                priorDirections.insert(reference.field, reference);
            }
        }
        scripts = loadStage2PageScripts(project, priorScripts);
        suggestion = existing.name;
        pageNumber = pageNumbers.value(pageId); //20261123-12 change
        bitmapName = existing.background.bitmapName;
        systemPicture = existing.background.systemPicture;
        bitmap = decodedControlPng(project.pictures.value(bitmapName));
        if (!existing.background.color.isEmpty())
        {
            backgroundColor = QColor(existing.background.color);
        }
        if (!existing.background.foregroundColor.isEmpty())
        {
            foregroundColor = QColor(existing.background.foregroundColor);
        }
    }
    else
    {
        existing.kind = kind; //20261123-12 add
        if (popup)
        {
            existing.width = 240;
            existing.height = 180;
        }
        pageNumber = project.pageCount();
        nameNumber = pageNumber + 1;
        occupiedNumbers = pageNumbers.values(); //20261123-12 change
        while (occupiedNumbers.contains(pageNumber)) //20261122-10 add: Keep the established order-based suggestion wherever it is available.
        {
            ++pageNumber;
        }
        suggestion = pageLabel + QString::number(nameNumber); //20261123-12 change
        while (!validateStage2PageName(project, QString(), suggestion, nullptr))
        {
            suggestion = pageLabel + QString::number(++nameNumber); //20261123-12 change
        }
    }

    dialog.setWindowTitle(editing ? pageLabel + QStringLiteral("属性") : QStringLiteral("新建") + pageLabel); //20261123-12 change
    dialog.setProperty("localHelpTopic", popup ? QStringLiteral("project") : QStringLiteral("screen-properties"));
    dialog.setObjectName(QStringLiteral("ordinaryPageEditor")); //20261123-11 add
    dialog.setWindowFlag(Qt::WindowContextHelpButtonHint);
    dialog.setFont(QFont(QStringLiteral("SimSun"), 9));
    dialog.setFixedSize(popup ? QSize(631, 620) : QSize(638, 662)); //20261123-12 change
    dialog.setStyleSheet(QStringLiteral(
        "QDialog#ordinaryPageEditor { background: #f0f0f0; } "
        "QLineEdit#newPageNameEdit, QSpinBox#newPageNumberSpin, QComboBox#newPageBackgroundStyle,"
        "QSpinBox#pageSlideDistance, QSpinBox[referenceWindowSpin='true'],"
        "QComboBox[pageSlideTarget='true'], QTableWidget#pageMacroTable " //20261123-12 change
        "{ background: #caeace; } "
        "QLineEdit#pageBitmapPath, QSpinBox#pageSlideDistance:disabled { background: #b1ceed; } "
        "QComboBox[pageSlideTarget='true']:disabled { background: #f0f0f0; } "
        "QFrame#pageBackgroundPreviewFrame { background: #caeace; border: 1px solid #888; } "
        "QTableWidget#pageMacroTable QHeaderView::section { background: white; border: none;"
        "border-right: 1px solid #dcdcdc; }"));
    nameLabel = new QLabel(QStringLiteral("名称："), &dialog);
    nameLabel->setGeometry(popup ? QRect(10, 10, 41, 25) : QRect(10, 5, 81, 30)); //20261123-12 change
    numberLabel = new QLabel(QStringLiteral("编号："), &dialog);
    numberLabel->setGeometry(popup ? QRect(240, 10, 41, 25) : QRect(370, 10, 51, 30)); //20261123-12 change
    name = new QLineEdit(suggestion, &dialog);
    name->setObjectName(QStringLiteral("newPageNameEdit"));
    name->setMaxLength(32767);
    name->setGeometry(popup ? QRect(60, 10, 131, 25) : QRect(90, 5, 191, 30)); //20261123-12 change
    number = new QSpinBox(&dialog);
    number->setObjectName(QStringLiteral("newPageNumberSpin"));
    number->setRange(0, 999);
    number->setValue(pageNumber);
    number->setToolTip(QStringLiteral("画面和窗口编号不能重复")); //20261123-13 change
    number->setGeometry(popup ? QRect(290, 10, 111, 25) : QRect(428, 10, 161, 30));
    validation = new QLabel(&dialog);
    validation->setWordWrap(true);
    validation->setGeometry(popup ? QRect(411, 4, 210, 36) : QRect(10, 43, 601, 34)); //20261123-12 change
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
                                   QDialogButtonBox::Help, &dialog); //20261123-10 change
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    buttons->button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助")); //20261123-10 add
    buttons->setGeometry(popup ? QRect(400, 580, 201, 31) : QRect(360, 620, 251, 31)); //20261123-12 change
    buttons->layout()->setContentsMargins(0, 0, 0, 0);
    buttons->layout()->setSpacing(9);
    buttons->button(QDialogButtonBox::Ok)->setFixedSize(popup ? 61 : 81, 31); //20261123-12 change
    buttons->button(QDialogButtonBox::Cancel)->setFixedSize(popup ? 61 : 81, 31);
    buttons->button(QDialogButtonBox::Help)->setFixedSize(popup ? 61 : 71, 31);
    background = new QGroupBox(QStringLiteral("背景"), &dialog); //20261123-0 add
    background->setObjectName(QStringLiteral("newPageBackground")); //20261123-11 add
    background->setGeometry(popup ? QRect(10, 40, 591, 161) : QRect(10, 80, 601, 301)); //20261123-12 change
    defaultMode = new QRadioButton(QStringLiteral("默认"), background); //20261123-2 add
    defaultMode->setGeometry(popup ? QRect(18, 30, 65, 22) : QRect(10, 30, 70, 25)); //20261123-12 change
    defaultMode->setChecked(true);
    bitmapMode = new QRadioButton(QStringLiteral("位图"), background);
    bitmapMode->setGeometry(popup ? QRect(18, 100, 65, 22) : QRect(10, 118, 70, 21)); //20261123-12 change
    bitmapMode->setObjectName(QStringLiteral("newPageBitmapMode"));
    backgroundOptions = new QStackedWidget(background);
    backgroundOptions->setGeometry(popup ? QRect(90, 30, 491, 126) : QRect(80, 30, 340, 140)); //20261123-12 change
    colorOptions = new QWidget(backgroundOptions);
    bitmapOptions = new QWidget(backgroundOptions);
    bitmapPath = new QLineEdit(bitmapOptions);
    bitmapPath->setGeometry(popup ? QRect(0, 70, 491, 22) : QRect(6, 88, 331, 21)); //20261123-12 change
    bitmapPath->setObjectName(QStringLiteral("pageBitmapPath"));
    bitmapPath->setReadOnly(true);
    bitmapPath->setText(systemPicture.isEmpty() ? bitmapName : kvgPictureResource(systemPicture)); //20261123-6 add
    bitmapPath->setAccessibleName(QStringLiteral("背景图片"));
    projectGallery = new QPushButton(QStringLiteral("工程图库"), bitmapOptions);
    projectGallery->setObjectName(QStringLiteral("newPageProjectGallery"));
    projectGallery->setGeometry(popup ? QRect(370, 100, 120, 25) : QRect(220, 115, 120, 25)); //20261123-12 change
    projectGallery->setAutoDefault(false);
    systemGallery = new QPushButton(QStringLiteral("系统图库"), bitmapOptions); //20261123-3 add
    systemGallery->setObjectName(QStringLiteral("newPageSystemGallery"));
    systemGallery->setGeometry(popup ? QRect(0, 100, 120, 25) : QRect(0, 115, 120, 25)); //20261123-12 change
    systemGallery->setAutoDefault(false);
    backgroundOptions->addWidget(colorOptions);
    backgroundOptions->addWidget(bitmapOptions);
    backgroundColorButton = new QPushButton(colorOptions);
    backgroundColorButton->setObjectName(QStringLiteral("newPageBackgroundColor"));
    backgroundColorButton->setGeometry(popup ? QRect(85, 0, 157, 26) : QRect(89, 0, 181, 23)); //20261123-12 change
    backgroundColorButton->setAutoDefault(false);
    backgroundColorButton->setAccessibleName(QStringLiteral("背景色"));
    foregroundColorButton = new QPushButton(colorOptions);
    foregroundColorButton->setObjectName(QStringLiteral("newPageForegroundColor"));
    foregroundColorButton->setGeometry(popup ? QRect(333, 0, 157, 26) : QRect(90, 30, 181, 23)); //20261123-12 change
    foregroundColorButton->setAutoDefault(false);
    foregroundColorButton->setAccessibleName(QStringLiteral("前景色"));
    backgroundStyle = new QComboBox(colorOptions);
    backgroundStyle->setObjectName(QStringLiteral("newPageBackgroundStyle"));
    backgroundStyle->setGeometry(popup ? QRect(85, 32, 406, 26) : QRect(89, 60, 251, 20)); //20261123-12 change
    backgroundStyle->addItems({QStringLiteral("纯色"), QStringLiteral("横向过渡"), QStringLiteral("横向对称过渡"),
        QStringLiteral("纵向过渡"), QStringLiteral("纵向对称过渡"), QStringLiteral("斜上过渡"),
        QStringLiteral("斜上对称过渡"), QStringLiteral("斜下过渡"), QStringLiteral("斜下对称过渡"),
        QStringLiteral("右上角辐射"), QStringLiteral("左上角辐射"), QStringLiteral("中心辐射")});
    solidStyleIcon.fill(Qt::white); //20261123-11 add: Reproduce the bordered pure-color sample in the reference field.
    stylePainter.begin(&solidStyleIcon);
    stylePainter.drawRect(0, 0, 15, 15);
    stylePainter.end();
    backgroundStyle->setItemIcon(0, QIcon(solidStyleIcon));
    backgroundStyle->setCurrentIndex(existing.background.style); //20261123-6 add
    if (!bitmapName.isEmpty() || !systemPicture.isEmpty())
    {
        bitmapMode->setChecked(true);
        backgroundOptions->setCurrentWidget(bitmapOptions);
    }
    previewFrame = new QFrame(popup ? static_cast<QWidget *>(&dialog) : background); //20261123-12 change
    previewFrame->setObjectName(QStringLiteral("pageBackgroundPreviewFrame"));
    previewFrame->setGeometry(popup ? QRect(450, 220, 151, 131) : QRect(430, 30, 151, 131)); //20261123-12 change
    backgroundPreview = new QLabel(previewFrame);
    backgroundPreview->setObjectName(QStringLiteral("newPageBackgroundPreview")); //20261123-2 add
    backgroundPreview->setGeometry(20, 10, 110, 110); //20261123-11 change
    backgroundPreview->setAccessibleName(QStringLiteral("背景预览"));
    backgroundLabel = new QLabel(QStringLiteral("背景色："), colorOptions); //20261123-11 add
    backgroundLabel->setGeometry(popup ? QRect(0, 0, 81, 26) : QRect(10, 0, 73, 23)); //20261123-12 change
    foregroundLabel = new QLabel(QStringLiteral("前景色："), colorOptions);
    foregroundLabel->setGeometry(popup ? QRect(247, 0, 81, 26) : QRect(11, 30, 73, 23)); //20261123-12 change
    styleLabel = new QLabel(popup ? QStringLiteral("样式：") : QStringLiteral("样  式："), colorOptions);
    styleLabel->setGeometry(popup ? QRect(0, 32, 81, 26) : QRect(10, 60, 73, 21));

    if (popup) //20261123-12 add
    {
        windowFields = createStage2WindowFields(dialog, existing, priorPlacement.properties);
    }
    else
    {
        slideGroup = new QGroupBox(QStringLiteral("滑动打开画面"), background); //20261123-4 add
        slideGroup->setObjectName(QStringLiteral("pageSlideGroup"));
        slideGroup->setCheckable(true);
        slideGroup->setGeometry(20, 185, 571, 111); //20261123-11 change
        distanceLabel = new QLabel(QStringLiteral("滑动距离："), slideGroup);
        distanceLabel->setGeometry(10, 25, 61, 16);
        slideDistance = new QSpinBox(slideGroup);
        slideDistance->setObjectName(QStringLiteral("pageSlideDistance"));
        slideDistance->setRange(5, 480);
        slideDistance->setValue(priorSlide.properties.value(QStringLiteral("distance")).toInt(50)); //20261123-6 change
        slideDistance->setGeometry(80, 20, 151, 22); //20261123-11 change
        for (directionIndex = 0; directionIndex < 4; ++directionIndex)
        {
            column = 312 * (directionIndex % 2); //20261123-11 change
            row = 55 + 30 * (directionIndex / 2);
            slideDirections[directionIndex] = new QCheckBox(slideLabels.at(directionIndex), slideGroup);
            slideTargets[directionIndex] = new QComboBox(slideGroup);
            slideDirections[directionIndex]->setGeometry(13 + column, row, 58, 20); //20261123-11 add
            slideTargets[directionIndex]->setGeometry(78 + column, row, 151, 21);
            slideTargets[directionIndex]->setProperty("pageSlideTarget", true);
            slideDirections[directionIndex]->setObjectName(QStringLiteral("pageSlide") + slideKeys.at(directionIndex));
            slideTargets[directionIndex]->setObjectName(slideDirections[directionIndex]->objectName() + QStringLiteral("Target"));
            for (const HmiPage &target : targetPages)
            {
                if (target.id != pageId && target.kind == QStringLiteral("normal")) //20261123-13 change
                {
                    // The label reflects renumbering or renaming while the data carries a stable ID.
                    slideTargets[directionIndex]->addItem(QStringLiteral("%1:%2")
                        .arg(pageNumbers.value(target.id), 3, 10, QLatin1Char('0')).arg(target.name), target.id);
                }
            }
            const HmiReference previous = priorDirections.value(slideKeys.at(directionIndex).toLower()); //20261123-6 add
            if (!previous.id.isEmpty())
            {
                slideDirections[directionIndex]->setChecked(true);
                slideTargets[directionIndex]->setCurrentIndex(slideTargets[directionIndex]->findData(previous.targetId));
            }
            // A populated target stays visible while disabled; an empty target list is hidden.
            slideTargets[directionIndex]->setVisible(slideTargets[directionIndex]->count() != 0); //20261123-11 change
        }
        slideGroup->setChecked(priorSlide.properties.value(QStringLiteral("enabled")).toBool()); //20261123-6 change
    }
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
        if (bitmapMode->isChecked() && bitmapName.isEmpty() && systemPicture.isEmpty()) //20261123-11 add
        {
            backgroundPreview->clear();
            return;
        }
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
    createStage2PageScriptList(&dialog, macroDraft, scripts, popup)->setGeometry(
        popup ? QRect(10, 360, 591, 251) : QRect(10, 390, 601, 261)); //20261123-12 change
    QObject::connect(buttons, &QDialogButtonBox::helpRequested, &dialog, [&dialog]() { //20261123-10 add
        LocalHelpController *help = dialog.findChild<LocalHelpController *>(QString(), Qt::FindDirectChildrenOnly);

        if (help == nullptr)
        {
            help = new LocalHelpController(&dialog);
        }
        help->showTopic(dialog.property("localHelpTopic").toString(), &dialog); //20261123-12 change
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        HmiPage page;
        QString error;
        HmiConfigurationNode slide = priorSlide; //20261123-6 change
        HmiReference target;
        bool preserveSlide;
        int direction;

        if (bitmapMode->isChecked() && bitmap.isNull() && systemPicture.isEmpty()) //20261123-3 change
        {
            validation->setText(QStringLiteral("请从系统图库或工程图库选择一张有效的背景图片。"));
            return;
        }
        if ((!editing || number->value() != pageNumber) &&
            pageNumbers.values().contains(number->value())) //20261123-13 change
        {
            QMessageBox::critical(&dialog, QStringLiteral("错误"), QStringLiteral("画面和窗口编号不能重复"));
            return;
        }
        page = existing; //20261123-6 add: Preserve dimensions, controls and extensions on an existing page.
        page.id = editing ? pageId : QUuid::createUuid().toString(QUuid::WithoutBraces);
        page.name = name->text();
        if (!editing || existing.number.has_value() || number->value() != pageNumber)
        {
            page.number = number->value();
        }
        page.background = fillDraft(); //20261123-1 change
        if (popup) //20261123-12 add
        {
            page.width = windowFields.width->value();
            page.height = windowFields.height->value();
        }
        candidate = macroDraft; //20261123-5 change: Commit shared source edits only with the outer page.
        if (!validateStage2PageName(project, pageId, page.name, &error) ||
            (editing ? !candidate.selectPage(pageId, &error) : !candidate.addPage(page, &error))) //20261123-6 change
        {
            validation->setText(error);
            return;
        }
        if (editing) //20261123-6 add
        {
            candidate.pageName = page.name;
            candidate.activePageNumber = page.number;
            candidate.activePageBackground = page.background;
            candidate.pageWidth = page.width; //20261123-12 add
            candidate.pageHeight = page.height;
            // Replace the owned draft records and their outgoing bindings as one transaction.
            candidate.configurationNodes.removeIf([&](const HmiConfigurationNode &node) {
                return node.id == priorSlide.id || node.id == priorScripts.id || node.id == priorPlacement.id; //20261123-12 change
            });
            candidate.references.removeIf([&](const HmiReference &reference) {
                return reference.ownerKind == QStringLiteral("configuration") &&
                    (reference.ownerId == priorSlide.id || reference.ownerId == priorScripts.id);
            });
        }
        if (popup) //20261123-12 add
        {
            storeStage2WindowPlacement(candidate, page.id, windowFields, priorPlacement);
        }
        else
        {
            if (slide.id.isEmpty())
            {
                slide.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            }
            slide.kind = QStringLiteral("pageSlide");
            slide.name = slideGroup->title();
            slide.ownerPageId = page.id;
            slide.implemented = true;
            slide.active = true;
            slide.properties = {{QStringLiteral("enabled"), slideGroup->isChecked()},
                                {QStringLiteral("distance"), slideDistance->value()}};
            preserveSlide = !priorSlide.id.isEmpty() || slideGroup->isChecked() || slideDistance->value() != 50; //20261123-6 change
            for (direction = 0; direction < 4; ++direction)
            {
                if (!slideDirections[direction]->isChecked())
                {
                    continue;
                }
                target = priorDirections.value(slideKeys.at(direction).toLower()); //20261123-6 add
                target.targetId = slideTargets[direction]->currentData().toString();
                if (target.targetId.isEmpty())
                {
                    validation->setText(QStringLiteral("请选择“%1”的目标画面。").arg(slideLabels.at(direction)));
                    return;
                }
                if (target.id.isEmpty())
                {
                    target.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                }
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
        }
        storeStage2PageScripts(candidate, page, scripts, priorScripts); //20261123-12 change
        if (editing) //20261123-6 add: Keeping the original record order makes a no-op confirmation leave the project clean.
        {
            auto keepOrder = [](const auto &previous, auto &updated) {
                QMap<QString, qsizetype> positions;
                qsizetype index;

                for (index = 0; index < previous.size(); ++index)
                {
                    positions.insert(previous.at(index).id, index);
                }
                std::stable_sort(updated.begin(), updated.end(), [&](const auto &left, const auto &right) {
                    return positions.value(left.id, previous.size()) < positions.value(right.id, previous.size());
                });
            };
            keepOrder(project.configurationNodes, candidate.configurationNodes);
            keepOrder(project.references, candidate.references);
        }
        // Editing keeps the active canvas independent; creation opens the new page.
        if (!candidate.selectPage(editing ? project.activePageId : page.id, &error))
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
 * Function Name   : editStage2NewNormalPage
 * Description     : Open the ordinary page creation dialog.
 * Input Parameters: parent, project.
 * Return Value    : True when a new page is created.
 * Date            : 2026.11.23
 ******************************************************************************/
bool editStage2NewNormalPage(QWidget *parent, HmiProject &project) //20261123-6 change
{
    return editStage2DesignerPage(parent, project, QString(), QStringLiteral("normal")); //20261123-12 change
}

/******************************************************************************
 * Function Name   : editStage2NormalPageProperties
 * Description     : Open the property dialog for the ordinary page selected in the tree.
 * Input Parameters: parent, project, pageId.
 * Return Value    : True after the new property values are accepted.
 * Date            : 2026.11.23
 ******************************************************************************/
bool editStage2NormalPageProperties(QWidget *parent, HmiProject &project, const QString &pageId) //20261123-6 add
{
    if (pageId.isEmpty())
    {
        return false;
    }
    return editStage2DesignerPage(parent, project, pageId, QStringLiteral("normal")); //20261123-12 change
}

/******************************************************************************
 * Function Name   : editStage2Window
 * Description     : Use the popup-specific form while sharing the page appearance and macro draft.
 * Input Parameters: parent, project, pageId (empty for creation).
 * Return Value    : True for a confirmed window edit.
 * Date            : 2026.11.23
 ******************************************************************************/
bool editStage2Window(QWidget *parent, HmiProject &project, const QString &pageId) //20261123-12 add
{
    return editStage2DesignerPage(parent, project, pageId, QStringLiteral("window"));
}

/******************************************************************************
 * Function Name   : copyStage2Page
 * Description     : Request a new name and publish a validated screen or window copy when confirmed.
 * Input Parameters: parent, project, pageId, popup (selects the original window dialog).
 * Return Value    : True after the copy and its outgoing references are ready to commit.
 * Date            : 2026.11.23
 ******************************************************************************/
bool copyStage2Page(QWidget *parent, HmiProject &project, const QString &pageId, bool popup) //20261123-13 change
{
    QDialog dialog(parent);
    QLabel label(popup ? QStringLiteral("窗口名称：") : QStringLiteral("画面名称："), &dialog);
    QLineEdit name(&dialog);
    QPushButton confirm(popup ? QStringLiteral("确定") : QStringLiteral(" 确定 "), &dialog);
    QPushButton cancel(QStringLiteral("取消"), &dialog);
    QPalette palette = dialog.palette();
    HmiProject candidate;

    dialog.setObjectName(popup ? QStringLiteral("QCopyWindowDlg") : QStringLiteral("QCopyScreenDlg"));
    dialog.setWindowTitle(popup ? QStringLiteral("复制到当前窗口") : QStringLiteral("复制到当前画面"));
    dialog.setFixedSize(popup ? QSize(268, 69) : QSize(273, 72));
    palette.setColor(QPalette::Window, QColor(224, 237, 254));
    dialog.setPalette(palette);
    // Original V5.1 client geometry, measured in the dialog's logical pixels.
    label.setGeometry(popup ? QRect(7, 10, 80, 21) : QRect(6, 11, 70, 21));
    name.setGeometry(popup ? QRect(90, 10, 170, 20) : QRect(80, 10, 181, 20));
    name.setObjectName(QStringLiteral("lineEdit"));
    name.setMaxLength(32767);
    confirm.setGeometry(popup ? QRect(107, 40, 75, 23) : QRect(110, 40, 75, 23));
    confirm.setObjectName(QStringLiteral("pushButton_ok"));
    confirm.setDefault(true);
    cancel.setGeometry(popup ? QRect(187, 40, 75, 23) : QRect(190, 40, 75, 23));
    cancel.setObjectName(QStringLiteral("pushButton_cancle"));
    name.setFocus();

    QObject::connect(&cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    QObject::connect(&confirm, &QPushButton::clicked, &dialog, [&]() {
        QString copyId;
        QString error;

        if (name.text().trimmed().isEmpty())
        {
            const QString message = QStringLiteral("画面名字不可以为空，请输入一个名字");

            if (popup)
            {
                QMessageBox::information(&dialog, QStringLiteral("KinSealStudio5"), message);
            }
            else
            {
                QMessageBox::warning(&dialog, QStringLiteral("KinSealStudio"), message);
            }
            return;
        }
        candidate = project;
        if (!validateStage2PageName(project, QString(), name.text(), &error) ||
            !candidate.copyPage(pageId, name.text(), &copyId, &error) || !candidate.selectPage(copyId, &error))
        {
            QMessageBox::warning(&dialog, QStringLiteral("KinSealStudio"), error);
            return;
        }
        dialog.accept();
    });
    if (dialog.exec() != QDialog::Accepted)
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
