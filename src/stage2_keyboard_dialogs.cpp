#include "stage2_keyboard_dialogs.h"

#include "bit_button_dialog.h"
#include "bit_button_renderer.h"
#include "kvg_library.h"
#include "project_model.h"
#include "stage2_project_dialogs.h"
#include "static_text_renderer.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QGroupBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QUuid>
#include <algorithm>

namespace {

/******************************************************************************
 * Function Name   : templatePage
 * Description     : Decode the verified template layout into ordinary editable controls.
 * Input Parameters: source
 * Return Value    : An unnumbered keyboard page with its own stable object IDs.
 * Date            : 2026.11.23
 ******************************************************************************/
HmiPage templatePage(const QJsonObject &source) //20261123-24 add
{
    static const QMap<QString, int> types = {{QStringLiteral("number"), 0}, {QStringLiteral("hex"), 1},
        {QStringLiteral("asciiFull"), 2}, {QStringLiteral("asciiSmall"), 3}};
    const QJsonArray size = source.value(QStringLiteral("size")).toArray();
    HmiPage page;
    QJsonObject control;
    QJsonArray bounds;
    QJsonArray images;
    HmiObject object;
    QString kind;
    bool caps;
    int state;

    page.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    page.name = source.value(QStringLiteral("name")).toString();
    page.kind = QStringLiteral("keyboard");
    page.keyboardType = types.value(source.value(QStringLiteral("type")).toString(), -1);
    page.width = size.at(0).toInt();
    page.height = size.at(1).toInt();
    page.background.color = source.value(QStringLiteral("background")).toString();
    for (const QJsonValue &row : source.value(QStringLiteral("controls")).toArray())
    {
        control = row.toObject();
        bounds = control.value(QStringLiteral("geometry")).toArray();
        kind = control.value(QStringLiteral("kind")).toString();
        object = HmiObject();
        object.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        object.text = control.value(QStringLiteral("text")).toString();
        object.geometry = QRect(bounds.at(0).toInt(), bounds.at(1).toInt(),
                                bounds.at(2).toInt(), bounds.at(3).toInt());
        object.layer = page.objects.size();
        if (kind == QStringLiteral("key") || kind == QStringLiteral("caps"))
        {
            caps = kind == QStringLiteral("caps");
            images = control.value(QStringLiteral("assets")).toArray();
            object.type = QStringLiteral("switch");
            object.name = caps ? QStringLiteral("Caps") : QStringLiteral("键盘组件 %1").arg(object.text);
            object.hasBitButtonProperties = true;
            object.hasFunctionButtonProperties = !caps;
            object.bitButton.indicator = caps;
            object.bitButton.dualMonitor = caps;
            if (caps)
            {
                object.address = control.value(QStringLiteral("address")).toString();
                object.bitButton.operation = control.value(QStringLiteral("bitOperation")).toInt();
            }
            else
            {
                object.functionButton.function = 14;
                object.functionButton.keyboardKey = control.value(QStringLiteral("keyCode")).toInt();
                object.functionButton.keyboardText = control.value(QStringLiteral("keyText")).toString();
                object.functionButton.pressedEffect = true;
            }
            for (state = 0; state < 2; ++state)
            {
                BitButtonState &appearance = object.bitButton.states[state];

                appearance.picture = images.at(state).toString();
                appearance.useImage = !appearance.picture.isEmpty();
                appearance.sharedText = object.text;
                appearance.text.fontFamily = control.value(QStringLiteral("fontFamily")).toString();
                appearance.text.fontSize = control.value(QStringLiteral("sourceFontSize")).toInt();
            }
        }
        else
        {
            object.type = QStringLiteral("text");
            object.name = object.text;
            object.hasStaticTextProperties = true;
            object.staticText.fontFamily = control.value(QStringLiteral("fontFamily")).toString();
            object.staticText.fontSize = control.value(QStringLiteral("sourceFontSize")).toInt();
            object.staticText.alignment = kind == QStringLiteral("caption") ? 1 : 2;
            if (control.value(QStringLiteral("role")).toString() == QStringLiteral("value"))
            {
                object.staticText.style = 1;
            }
            else
            {
                object.staticText.textColor = Qt::white;
            }
        }
        page.objects.append(object);
    }
    return page;
}

/******************************************************************************
 * Function Name   : templateImage
 * Description     : Paint the selected editable template at the reference dialog scale.
 * Input Parameters: page, language
 * Return Value    : Preview image.
 * Date            : 2026.11.23
 ******************************************************************************/
QPixmap templateImage(const HmiPage &page, const QString &language) //20261123-24 add
{
    const QRect bounds(0, 0, page.width, page.height);
    QPixmap image(bounds.size());
    QPainter painter;

    image.fill(Qt::transparent);
    painter.begin(&image);
    painter.fillRect(bounds, page.background.brush(bounds));
    for (const HmiObject &object : page.objects)
    {
        if (object.hasBitButtonProperties)
        {
            paintBitButton(&painter, object.geometry, object.bitButton.states[0], language);
        }
        else
        {
            paintStaticText(&painter, object.geometry, object.text, object.staticText);
        }
    }
    painter.end();
    return image.scaled(page.width * 3 / 4, page.height * 3 / 4, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

/******************************************************************************
 * Function Name   : formatButtons
 * Description     : Use the shared native confirm and cancel captions on both keyboard dialogs.
 * Input Parameters: buttons
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void formatButtons(QDialogButtonBox &buttons) //20261123-24 add
{
    buttons.button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
    buttons.button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
}

} // namespace

/******************************************************************************
 * Function Name   : addStage2Keyboards
 * Description     : Select one or more original layouts and publish all selected pages in a single transaction.
 * Input Parameters: parent, project
 * Return Value    : True after confirming valid pages.
 * Date            : 2026.11.23
 ******************************************************************************/
bool addStage2Keyboards(QWidget *parent, HmiProject &project) //20261123-24 add
{
    QFile resource(QStringLiteral(":/native-shell/keyboard_templates.json"));
    QDialog dialog(parent);
    QComboBox source(&dialog);
    QGroupBox types(QStringLiteral("键盘类型"), &dialog);
    QLabel preview(&dialog);
    QLabel sourceLabel(QStringLiteral("键盘样式"), &dialog);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QJsonArray templates;
    QList<HmiPage> layouts;
    QList<QCheckBox *> choices;
    QList<int> assigned = project.keyboardNumbers().values();
    QSet<QString> names;
    HmiProject draft;
    HmiPage page;
    int index;
    int number;
    QString error;
    QString templateName;
    int suffix;

    if (resource.open(QIODevice::ReadOnly))
    {
        templates = QJsonDocument::fromJson(resource.readAll()).array();
    }
    if (templates.size() != 4)
    {
        QMessageBox::warning(parent, QStringLiteral("添加自定义键盘"), QStringLiteral("无法读取键盘模板。"));
        return false;
    }
    dialog.setObjectName(QStringLiteral("Stage2AddKeyboards"));
    dialog.setWindowTitle(QStringLiteral("添加自定义键盘"));
    dialog.setFixedSize(795, 434);
    sourceLabel.setGeometry(12, 1, 95, 20);
    source.setGeometry(8, 19, 149, 22);
    source.addItem(QStringLiteral("KeyBoardFile"));
    types.setGeometry(8, 44, 149, 351);
    preview.setObjectName(QStringLiteral("Stage2KeyboardTemplatePreview"));
    preview.setGeometry(163, 3, 628, 395);
    preview.setAlignment(Qt::AlignCenter);
    preview.setStyleSheet(QStringLiteral("background:#caeace; border:1px solid #8d938f;"));
    buttons.setGeometry(634, 399, 157, 32);
    formatButtons(buttons);
    for (index = 0; index < templates.size(); ++index)
    {
        layouts.append(templatePage(templates.at(index).toObject()));
        choices.append(new QCheckBox(layouts.last().name, &types));
        choices.last()->setGeometry(9, 22 + index * 22, 138, 22);
        choices.last()->setChecked(index == 0);
        QObject::connect(choices.last(), &QCheckBox::toggled, &dialog, [&, index](bool checked) {
            if (checked)
            {
                preview.setPixmap(templateImage(layouts.at(index), project.activeLanguage));
            }
            buttons.button(QDialogButtonBox::Ok)->setEnabled(std::any_of(choices.cbegin(), choices.cend(),
                [](const QCheckBox *choice) { return choice->isChecked(); }));
        });
    }
    preview.setPixmap(templateImage(layouts.first(), project.activeLanguage));
    for (const HmiPage &existing : project.allPages())
    {
        names.insert(existing.name);
    }
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        QList<int> used = assigned;
        QSet<QString> usedNames = names;

        draft = project;
        for (index = 0; index < choices.size(); ++index)
        {
            if (!choices.at(index)->isChecked())
            {
                continue;
            }
            number = 0;
            while (used.contains(number))
            {
                ++number;
            }
            if (number > 99)
            {
                QMessageBox::warning(&dialog, dialog.windowTitle(), QStringLiteral("键盘编号 0–99 已用完。"));
                return;
            }
            page = layouts.at(index);
            page.number = number;
            templateName = page.name;
            suffix = 2;
            while (usedNames.contains(page.name))
            {
                page.name = QStringLiteral("%1_%2").arg(templateName).arg(suffix++);
            }
            if (!draft.addPage(page, &error))
            {
                QMessageBox::warning(&dialog, dialog.windowTitle(), error);
                return;
            }
            used.append(number);
            usedNames.insert(page.name);
        }
        dialog.accept();
    });
    if (dialog.exec() != QDialog::Accepted)
    {
        return false;
    }
    project = draft;
    return true;
}

/******************************************************************************
 * Function Name   : editStage2KeyboardProperties
 * Description     : Edit the keyboard identity, displayed dimensions and background without rebinding its consumers.
 * Input Parameters: parent, project, id
 * Return Value    : True after publishing a valid draft.
 * Date            : 2026.11.23
 ******************************************************************************/
bool editStage2KeyboardProperties(QWidget *parent, HmiProject &project, const QString &id) //20261123-24 add
{
    const QList<HmiPage> pages = project.allPages();
    const auto found = std::find_if(pages.cbegin(), pages.cend(), [&](const HmiPage &page) {
        return page.id == id && page.kind == QStringLiteral("keyboard");
    });
    const QStringList types = {QStringLiteral("10进制键盘"), QStringLiteral("16进制键盘"),
        QStringLiteral("ASCII键盘"), QStringLiteral("ASCII小键盘")};
    QDialog dialog(parent);
    QLineEdit name(&dialog);
    QSpinBox number(&dialog);
    QComboBox type(&dialog);
    QGroupBox background(QStringLiteral("背景"), &dialog);
    QRadioButton normal(QStringLiteral("默认"), &background);
    QRadioButton bitmap(QStringLiteral("位图"), &background);
    QWidget colors(&background);
    QWidget galleries(&background);
    QPushButton foreground(&colors);
    QPushButton backgroundColor(&colors);
    QComboBox style(&colors);
    QLineEdit picture(&galleries);
    QPushButton projectGallery(QStringLiteral("工程图库"), &galleries);
    QPushButton systemGallery(QStringLiteral("系统图库"), &galleries);
    QGroupBox size(QStringLiteral("键盘显示大小"), &dialog);
    QSpinBox width(&size);
    QSpinBox height(&size);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    HmiPage page;
    HmiProject galleryDraft = project; //20261124-3 add: Keep resources private until the outer dialog is accepted.
    HmiProject draft;
    QString error;
    QString projectPicture;
    QString systemPicture;
    QColor front;
    QColor back;

    if (found == pages.cend())
    {
        return false;
    }
    page = *found;
    projectPicture = page.background.bitmapName;
    systemPicture = page.background.systemPicture;
    front = page.background.foregroundColor.isEmpty() ? HmiPageBackground::defaultColor() : QColor(page.background.foregroundColor);
    back = page.background.color.isEmpty() ? HmiPageBackground::defaultColor() : QColor(page.background.color);
    dialog.setObjectName(QStringLiteral("Stage2KeyboardProperties"));
    dialog.setWindowTitle(QStringLiteral("修改自定义键盘"));
    dialog.setFixedSize(495, 382);
    dialog.setStyleSheet(QStringLiteral("QLineEdit, QSpinBox, QComboBox:enabled { background:#caeace; }"));
    auto label = [](QWidget *owner, const QString &text, const QRect &bounds) {
        (new QLabel(text, owner))->setGeometry(bounds);
    };
    label(&dialog, QStringLiteral("名称："), QRect(20, 22, 68, 22));
    label(&dialog, QStringLiteral("编号："), QRect(220, 22, 63, 22));
    label(&dialog, QStringLiteral("键盘类型："), QRect(20, 62, 75, 22));
    name.setObjectName(QStringLiteral("Stage2KeyboardName"));
    name.setGeometry(91, 22, 113, 22);
    name.setText(page.name);
    number.setObjectName(QStringLiteral("Stage2KeyboardNumber"));
    number.setGeometry(291, 22, 101, 22);
    number.setRange(0, 99);
    number.setValue(project.keyboardNumbers().value(id));
    type.setObjectName(QStringLiteral("Stage2KeyboardType"));
    type.setGeometry(91, 62, 301, 22);
    type.addItems(types);
    type.setCurrentIndex(page.keyboardType.value_or(-1));
    type.setEnabled(false);
    background.setGeometry(10, 90, 471, 172);
    normal.setGeometry(20, 32, 64, 24);
    bitmap.setGeometry(20, 109, 64, 24);
    colors.setGeometry(88, 30, 345, 72);
    label(&colors, QStringLiteral("前景色："), QRect(0, 0, 66, 23));
    label(&colors, QStringLiteral("背景色："), QRect(189, 0, 60, 23));
    label(&colors, QStringLiteral("样式："), QRect(0, 40, 62, 22));
    foreground.setGeometry(73, 1, 80, 22);
    backgroundColor.setGeometry(252, 1, 80, 22);
    style.setObjectName(QStringLiteral("Stage2KeyboardBackgroundStyle"));
    style.setGeometry(73, 40, 261, 22);
    style.addItems(StaticTextProperties::styles());
    style.setCurrentIndex(page.background.style == 12 ? 0 : page.background.style + 1);
    galleries.setGeometry(88, 108, 345, 56);
    picture.setGeometry(0, 0, 332, 21);
    picture.setReadOnly(true);
    projectGallery.setGeometry(140, 27, 91, 22);
    systemGallery.setGeometry(241, 27, 91, 22);
    const bool usesBitmap = !projectPicture.isEmpty() || !systemPicture.isEmpty();
    normal.setChecked(!usesBitmap);
    bitmap.setChecked(usesBitmap);
    size.setGeometry(10, 270, 471, 61);
    label(&size, QStringLiteral("宽度："), QRect(30, 26, 66, 23));
    label(&size, QStringLiteral("高度："), QRect(248, 26, 66, 23));
    width.setObjectName(QStringLiteral("Stage2KeyboardWidth"));
    width.setGeometry(110, 26, 80, 22);
    width.setRange(0, 800);
    width.setValue(page.width);
    height.setObjectName(QStringLiteral("Stage2KeyboardHeight"));
    height.setGeometry(331, 26, 80, 22);
    height.setRange(0, 480);
    height.setValue(page.height);
    buttons.setGeometry(324, 341, 157, 32);
    formatButtons(buttons);
    auto refresh = [&]() {
        colors.setVisible(normal.isChecked());
        galleries.setVisible(bitmap.isChecked());
        foreground.setStyleSheet(QStringLiteral("background:%1").arg(front.name()));
        backgroundColor.setStyleSheet(QStringLiteral("background:%1").arg(back.name()));
        picture.setText(projectPicture.isEmpty() ? systemPicture.section(QLatin1Char('/'), -1) : projectPicture);
    };
    auto chooseColor = [&](QColor &current) {
        const QColor selected = QColorDialog::getColor(current, &dialog, QStringLiteral("选择颜色"));

        if (selected.isValid())
        {
            current = selected;
            refresh();
        }
    };
    QObject::connect(&bitmap, &QRadioButton::toggled, &dialog, refresh);
    QObject::connect(&foreground, &QPushButton::clicked, &dialog, [&]() { chooseColor(front); });
    QObject::connect(&backgroundColor, &QPushButton::clicked, &dialog, [&]() { chooseColor(back); });
    QObject::connect(&projectGallery, &QPushButton::clicked, &dialog, [&]() {
        if (chooseStage2PageBitmap(&dialog, galleryDraft, projectPicture)) //20261124-3 change
        {
            if (!projectPicture.isEmpty())
            {
                systemPicture.clear();
            }
            refresh();
        }
    });
    QObject::connect(&systemGallery, &QPushButton::clicked, &dialog, [&]() {
        HmiObject adapter;
        QString selected = systemPicture;
        QByteArray png;
        QMap<QString, QByteArray> pool;

        adapter.type = QStringLiteral("switch");
        adapter.address = QStringLiteral("LB0");
        adapter.hasBitButtonProperties = true;
        BitButtonDialog picker(adapter, project, &dialog);
        if (picker.choosePictureForControl(false, &selected, &png, &pool) && !kvgPictureResource(selected).isEmpty())
        {
            systemPicture = selected;
            projectPicture.clear();
            refresh();
        }
    });
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        const QString newName = name.text();
        const auto numbers = project.keyboardNumbers();

        if (newName.trimmed().isEmpty() || std::any_of(pages.cbegin(), pages.cend(), [&](const HmiPage &existing) {
            return existing.id != id && existing.name == newName;
        }))
        {
            QMessageBox::warning(&dialog, dialog.windowTitle(), QStringLiteral("请输入非空且未使用的键盘名称。"));
            return;
        }
        if (numbers.key(number.value(), id) != id)
        {
            QMessageBox::warning(&dialog, dialog.windowTitle(), QStringLiteral("键盘编号已被使用。"));
            return;
        }
        page.name = newName;
        page.number = number.value();
        page.width = width.value();
        page.height = height.value();
        page.background = HmiPageBackground();
        if (bitmap.isChecked())
        {
            if (projectPicture.isEmpty() && systemPicture.isEmpty())
            {
                QMessageBox::warning(&dialog, dialog.windowTitle(), QStringLiteral("请选择键盘背景图片。"));
                return;
            }
            page.background.bitmapName = projectPicture;
            page.background.systemPicture = systemPicture;
        }
        else
        {
            page.background.color = back.name();
            page.background.foregroundColor = front.name();
            page.background.style = style.currentIndex() == 0 ? 12 : style.currentIndex() - 1;
        }
        draft = galleryDraft; //20261124-3 change
        if (draft.activePageId == id)
        {
            draft.pageName = page.name;
            draft.activePageNumber = page.number;
            draft.pageWidth = page.width;
            draft.pageHeight = page.height;
            draft.activePageBackground = page.background;
        }
        else
        {
            for (HmiPage &inactive : draft.inactivePages)
            {
                if (inactive.id == id)
                {
                    inactive = page;
                    break;
                }
            }
        }
        if (!draft.validateModel(&error))
        {
            QMessageBox::warning(&dialog, dialog.windowTitle(), error);
            return;
        }
        dialog.accept();
    });
    refresh();
    if (dialog.exec() != QDialog::Accepted)
    {
        return false;
    }
    project = draft;
    return true;
}
