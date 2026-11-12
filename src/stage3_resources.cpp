#include "stage3_resources.h"
#include "control_rendering.h"
#include <QBuffer>
#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QImageReader>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QPixmap>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
/******************************************************************************
 * Function Name   : resourceError
 * Description     : Return an actionable failure without publishing a partial resource draft.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.10
 ******************************************************************************/
bool resourceError(QString *error, const QString &message) //20261110-8 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : visitPictureStates
 * Description     : Visit both button appearances and every additional lamp state, including inactive drafts.
 * Input Parameters: object, visitor
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
template<typename Visitor> void visitPictureStates(HmiObject &object, Visitor visitor) //20261110-8 add
{
    int index;
    if (object.hasNumberDisplayProperties) { visitor(object.numberDisplay.appearance); } //20261112-0 add
    if (object.type == QStringLiteral("multistate")) { for (index = 0; index < object.multiStateButton.stateCount; ++index) { visitor(*object.multiStateButton.state(index)); } } //20261112-0 add
    if (object.type == QStringLiteral("screen")) { for (index = 0; index < 2; ++index) { visitor(object.screenButton.common.states[index]); } } //20261112-0 add
    if (object.type == QStringLiteral("time")) { visitor(object.timeDisplay.appearance); } //20261112-0 add
    if (object.type == QStringLiteral("ascii")) { visitor(object.asciiDisplay.common.appearance); } //20261112-0 add
    if (object.hasImageDisplayProperties) //20261112-0 add: Include picture-display rows in rename/replace/delete protection on every page.
    { for (index = 0; index < object.imageDisplay.states.size(); ++index) { visitor(object.imageDisplay.states[index]); } }
    if (!object.hasBitButtonProperties) { return; }
    for (index = 0; index < 2; ++index) { visitor(object.bitButton.states[index]); }
    for (index = 0; index < object.lamp.extraStates.size(); ++index) { visitor(object.lamp.extraStates[index]); }
}

/******************************************************************************
 * Function Name   : updatePictureBindings
 * Description     : Remap only name-and-content matched embedded states while preserving detached same-name image bytes.
 * Input Parameters: project, oldName, newName, oldPng, newPng
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void updatePictureBindings(HmiProject *project, const QString &oldName, const QString &newName,
                           const QByteArray &oldPng, const QByteArray &newPng) //20261110-8 add
{
    QList<HmiObject>::iterator object;
    QList<HmiPage>::iterator page;
    auto visitor = [&](auto &state) {
        if (state.picture == oldName && state.picturePng == oldPng) {
            state.picture = newName;
            state.picturePng = newPng;
        }
    };
    for (object = project->objects.begin(); object != project->objects.end(); ++object) { visitPictureStates(*object, visitor); }
    for (page = project->inactivePages.begin(); page != project->inactivePages.end(); ++page) {
        for (object = page->objects.begin(); object != page->objects.end(); ++object) { visitPictureStates(*object, visitor); }
    }
}
}

/******************************************************************************
 * Function Name   : stage3PictureUsage
 * Description     : Count pool-bound, detached same-name and explicit picture references across all pages.
 * Input Parameters: project, name
 * Return Value    : Reference counts for deletion protection and resource identity inspection.
 * Date            : 2026.11.10
 ******************************************************************************/
Stage3PictureUsage stage3PictureUsage(const HmiProject &project, const QString &name) //20261110-8 add
{
    Stage3PictureUsage usage;
    HmiPage page;
    HmiObject object;
    HmiReference reference;
    QByteArray pool;
    pool = project.pictures.value(name);
    Q_FOREACH (page, project.allPages()) {
        Q_FOREACH (object, page.objects) {
            visitPictureStates(object, [&](const auto &state) {
                if (state.picture == name) {
                    if (!pool.isEmpty() && state.picturePng == pool) { ++usage.boundStates; }
                    else { ++usage.detachedStates; }
                }
            });
        }
    }
    Q_FOREACH (reference, project.references) {
        if (reference.targetKind == QStringLiteral("picture") && reference.targetId == name) { ++usage.explicitReferences; }
    }
    return usage;
}

/******************************************************************************
 * Function Name   : importStage3Picture
 * Description     : Add a bounded decoded PNG to a detached whole-project candidate without overwriting an existing name.
 * Input Parameters: source, name, png, draft, error
 * Return Value    : True after the complete candidate validates.
 * Date            : 2026.11.10
 ******************************************************************************/
bool importStage3Picture(const HmiProject &source, const QString &name, const QByteArray &png,
                         HmiProject *draft, QString *error) //20261110-8 add
{
    HmiProject candidate;
    if (draft == nullptr || name.trimmed().isEmpty() || name.size() > 256 || source.pictures.contains(name)) {
        return resourceError(error, QStringLiteral("图片名称为空、超过256字符或已存在；导入未提交。"));
    }
    if (decodedControlPng(png, error).isNull()) { return false; }
    candidate = source;
    candidate.pictures.insert(name, png);
    if (!candidate.validateModel(error)) { return false; }
    *draft = candidate;
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : renameStage3Picture
 * Description     : Rename a pool entry and its proven embedded and typed references as one transaction.
 * Input Parameters: source, oldName, newName, draft, error
 * Return Value    : True after all references and the model validate.
 * Date            : 2026.11.10
 ******************************************************************************/
bool renameStage3Picture(const HmiProject &source, const QString &oldName, const QString &newName,
                         HmiProject *draft, QString *error) //20261110-8 add
{
    HmiProject candidate;
    QList<HmiReference>::iterator reference;
    QByteArray png;
    if (draft == nullptr || !source.pictures.contains(oldName) || newName.trimmed().isEmpty() || newName.size() > 256 ||
        (oldName != newName && source.pictures.contains(newName))) {
        return resourceError(error, QStringLiteral("图片不存在或新名称无效/冲突；重命名未提交。"));
    }
    candidate = source;
    png = source.pictures.value(oldName);
    candidate.pictures.remove(oldName);
    candidate.pictures.insert(newName, png);
    updatePictureBindings(&candidate, oldName, newName, png, png);
    for (reference = candidate.references.begin(); reference != candidate.references.end(); ++reference) {
        if (reference->targetKind == QStringLiteral("picture") && reference->targetId == oldName) { reference->targetId = newName; }
    }
    if (!candidate.validateModel(error)) { return false; }
    *draft = candidate;
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : replaceStage3Picture
 * Description     : Replace image content and refresh proven embedded bindings without touching unrelated same-name images.
 * Input Parameters: source, name, png, draft, error
 * Return Value    : True after complete candidate validation.
 * Date            : 2026.11.10
 ******************************************************************************/
bool replaceStage3Picture(const HmiProject &source, const QString &name, const QByteArray &png,
                          HmiProject *draft, QString *error) //20261110-8 add
{
    HmiProject candidate;
    if (draft == nullptr || !source.pictures.contains(name)) { return resourceError(error, QStringLiteral("待替换的工程图片不存在。")); }
    if (decodedControlPng(png, error).isNull()) { return false; }
    candidate = source;
    candidate.pictures[name] = png;
    updatePictureBindings(&candidate, name, name, source.pictures.value(name), png);
    if (!candidate.validateModel(error)) { return false; }
    *draft = candidate;
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : deleteStage3Picture
 * Description     : Refuse deletion while any embedded named state or explicit reference still uses the pool entry.
 * Input Parameters: source, name, draft, error
 * Return Value    : True after removing an unreferenced entry from a valid candidate.
 * Date            : 2026.11.10
 ******************************************************************************/
bool deleteStage3Picture(const HmiProject &source, const QString &name, HmiProject *draft, QString *error) //20261110-8 add
{
    HmiProject candidate;
    Stage3PictureUsage usage;
    if (draft == nullptr || !source.pictures.contains(name)) { return resourceError(error, QStringLiteral("待删除的工程图片不存在。")); }
    usage = stage3PictureUsage(source, name);
    if (usage.boundStates + usage.detachedStates + usage.explicitReferences != 0) {
        return resourceError(error, QStringLiteral("图片仍被状态或显式引用使用，请先在控件属性中解除引用。"));
    }
    candidate = source;
    candidate.pictures.remove(name);
    if (!candidate.validateModel(error)) { return false; }
    *draft = candidate;
    if (error != nullptr) { error->clear(); }
    return true;
}

/******************************************************************************
 * Function Name   : Stage3PicturesDialog::Stage3PicturesDialog
 * Description     : Edit project-owned images on a private draft with explicit identities, usage protection and cancellation.
 * Input Parameters: project, parent
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
Stage3PicturesDialog::Stage3PicturesDialog(const HmiProject &project, QWidget *parent) //20261110-8 add
    : QDialog(parent), m_draft(project), m_table(new QTableWidget(this)), m_error(new QLabel(this))
{
    QVBoxLayout *layout;
    QHBoxLayout *actions;
    QDialogButtonBox *buttons;
    QPushButton *button;
    QStringList labels;
    int index;
    setObjectName(QStringLiteral("Stage3PicturesDialog"));
    setWindowTitle(QStringLiteral("工程图片管理"));
    resize(880, 500);
    layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(QStringLiteral("导入后图片内嵌于工程；重命名和替换同步更新使用该图片的控件。"), this));
    m_table->setObjectName(QStringLiteral("Stage3PicturesTable"));
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({QStringLiteral("名称"), QStringLiteral("像素"), QStringLiteral("预览"),
        QStringLiteral("内容SHA256"), QStringLiteral("绑定/独立/显式引用"), QStringLiteral("字节")});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    layout->addWidget(m_table);
    actions = new QHBoxLayout();
    labels = {QStringLiteral("导入..."), QStringLiteral("重命名..."), QStringLiteral("替换..."), QStringLiteral("删除")};
    for (index = 0; index < labels.size(); ++index) {
        button = new QPushButton(labels.at(index), this);
        button->setObjectName(QStringLiteral("Stage3PictureAction%1").arg(index));
        actions->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, index]() { editPicture(index); });
    }
    layout->addLayout(actions);
    m_error->setWordWrap(true);
    m_error->setObjectName(QStringLiteral("Stage3PictureError"));
    layout->addWidget(m_error);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    rebuild();
}

/******************************************************************************
 * Function Name   : Stage3PicturesDialog::draftProject
 * Description     : Return the detached image-management result for a single main-window commit.
 * Input Parameters: None
 * Return Value    : Complete candidate project
 * Date            : 2026.11.10
 ******************************************************************************/
HmiProject Stage3PicturesDialog::draftProject() const //20261110-8 add
{
    return m_draft;
}

/******************************************************************************
 * Function Name   : Stage3PicturesDialog::rebuild
 * Description     : Show the current image content and whole-project usage from the accepted dialog draft.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3PicturesDialog::rebuild() //20261110-8 add
{
    QMap<QString, QByteArray>::const_iterator picture;
    Stage3PictureUsage usage;
    QImage image;
    QTableWidgetItem *item;
    int row;
    m_table->setRowCount(m_draft.pictures.size());
    row = 0;
    for (picture = m_draft.pictures.constBegin(); picture != m_draft.pictures.constEnd(); ++picture) {
        usage = stage3PictureUsage(m_draft, picture.key());
        image = decodedControlPng(picture.value());
        m_table->setItem(row, 0, new QTableWidgetItem(picture.key()));
        m_table->setItem(row, 1, new QTableWidgetItem(QStringLiteral("%1×%2").arg(image.width()).arg(image.height())));
        item = new QTableWidgetItem();
        item->setData(Qt::DecorationRole, QPixmap::fromImage(image.scaled(64, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        m_table->setItem(row, 2, item);
        m_table->setItem(row, 3, new QTableWidgetItem(QString::fromLatin1(QCryptographicHash::hash(picture.value(), QCryptographicHash::Sha256).toHex())));
        m_table->setItem(row, 4, new QTableWidgetItem(QStringLiteral("%1 / %2 / %3").arg(usage.boundStates).arg(usage.detachedStates).arg(usage.explicitReferences)));
        m_table->setItem(row, 5, new QTableWidgetItem(QString::number(picture.value().size())));
        m_table->setRowHeight(row, 44);
        ++row;
    }
}

/******************************************************************************
 * Function Name   : Stage3PicturesDialog::editPicture
 * Description     : Decode an explicitly selected bounded file and validate the full candidate before changing this dialog's draft.
 * Input Parameters: operation (import, rename, replace or delete)
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3PicturesDialog::editPicture(int operation) //20261110-8 add
{
    QString name;
    QString newName;
    QString path;
    QString error;
    HmiProject candidate;
    QImageReader reader;
    QImage image;
    QByteArray png;
    QBuffer buffer(&png);
    QFileInfo info;
    bool accepted;
    bool success;
    int row;
    row = m_table->currentRow();
    if (operation != 0 && row < 0) { m_error->setText(QStringLiteral("请先选择工程图片。")); return; }
    if (row >= 0) { name = m_table->item(row, 0)->text(); }
    if (operation == 0 || operation == 2) {
        path = QFileDialog::getOpenFileName(this, QStringLiteral("选择工程图片"), QString(), QStringLiteral("图片 (*.png *.bmp *.jpg *.jpeg)"));
        if (path.isEmpty()) { return; }
        info.setFile(path);
        reader.setFileName(path);
        if (info.size() > 16 * 1024 * 1024 || !reader.size().isValid() || reader.size().width() > 4096 || reader.size().height() > 4096) {
            m_error->setText(QStringLiteral("源图片必须不超过16 MiB且宽高为1..4096像素。")); return;
        }
        image = reader.read();
        buffer.open(QIODevice::WriteOnly);
        if (image.isNull() || !image.save(&buffer, "PNG")) { m_error->setText(QStringLiteral("图片解码或内嵌PNG转换失败。")); return; }
        if (operation == 0) { name = info.completeBaseName(); }
    }
    if (operation == 0 || operation == 1) {
        newName = QInputDialog::getText(this, QStringLiteral("工程图片名称"), QStringLiteral("名称"), QLineEdit::Normal, name, &accepted);
        if (!accepted) { return; }
        newName = newName.trimmed();
    }
    success = operation == 0 ? importStage3Picture(m_draft, newName, png, &candidate, &error) :
        operation == 1 ? renameStage3Picture(m_draft, name, newName, &candidate, &error) :
        operation == 2 ? replaceStage3Picture(m_draft, name, png, &candidate, &error) :
        deleteStage3Picture(m_draft, name, &candidate, &error);
    m_error->setText(error);
    if (success) { m_draft = candidate; rebuild(); }
}
