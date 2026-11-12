#include "animation_resources.h"
#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
/******************************************************************************
 * Function Name   : animationError
 * Description     : Report a resource failure without publishing a partial project draft.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.12
 ******************************************************************************/
bool animationError(QString *error, const QString &message) //20261112-0 add
{
    if (error != nullptr) { *error = message; } return false;
}

/******************************************************************************
 * Function Name   : updateAnimationBindings
 * Description     : Update exact name-and-content matches on every page and explicit typed references.
 * Input Parameters: project, name, newName, oldData, newData
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void updateAnimationBindings(HmiProject *project, const QString &name, const QString &newName, const QByteArray &oldData, const QByteArray &newData) //20261112-0 add
{
    HmiObject *object;
    QList<HmiObject *> objects;
    HmiReference *reference;
    int page;
    int index;
    for (index = 0; index < project->objects.size(); ++index) { objects.append(&project->objects[index]); }
    for (page = 0; page < project->inactivePages.size(); ++page)
    { for (index = 0; index < project->inactivePages[page].objects.size(); ++index) { objects.append(&project->inactivePages[page].objects[index]); } }
    Q_FOREACH (object, objects)
    {
        if (object->type == QStringLiteral("gif") && object->gifDisplay.animation == name && object->gifDisplay.gifData == oldData)
        { object->gifDisplay.animation = newName; object->gifDisplay.gifData = newData; }
    }
    for (index = 0; index < project->references.size(); ++index)
    {
        reference = &project->references[index];
        if (reference->targetKind == QStringLiteral("animation") && reference->targetId == name) { reference->targetId = newName; }
    }
}
}

/******************************************************************************
 * Function Name   : animationUsage
 * Description     : Count bound, detached and explicit uses on active and inactive pages before resource deletion.
 * Input Parameters: project, name
 * Return Value    : Number of uses.
 * Date            : 2026.11.12
 ******************************************************************************/
int animationUsage(const HmiProject &project, const QString &name) //20261112-0 add
{
    HmiPage page;
    HmiObject object;
    HmiReference reference;
    int count = 0;
    Q_FOREACH (page, project.allPages())
    { Q_FOREACH (object, page.objects) { if (object.type == QStringLiteral("gif") && object.gifDisplay.animation == name) { ++count; } } }
    Q_FOREACH (reference, project.references) { if (reference.targetKind == QStringLiteral("animation") && reference.targetId == name) { ++count; } }
    return count;
}

/******************************************************************************
 * Function Name   : importAnimation
 * Description     : Add original GIF bytes to an independent resource pool without silently overwriting names.
 * Input Parameters: source, name, gif, draft, error
 * Return Value    : True after full project validation.
 * Date            : 2026.11.12
 ******************************************************************************/
bool importAnimation(const HmiProject &source, const QString &name, const QByteArray &gif, HmiProject *draft, QString *error) //20261112-0 add
{
    HmiProject candidate = source;
    if (draft == nullptr || name.trimmed().isEmpty() || name.size() > 256 || source.animations.contains(name))
    { return animationError(error, QStringLiteral("动画名称为空、过长或已存在。")); }
    if (!validateGifData(gif, error)) { return false; }
    candidate.animations.insert(name, gif); if (!candidate.validateModel(error)) { return false; } *draft = candidate; return true;
}

/******************************************************************************
 * Function Name   : renameAnimation
 * Description     : Rename only proven pool bindings while retaining detached same-name animation bytes.
 * Input Parameters: source, name, newName, draft, error
 * Return Value    : True after the complete resource transaction validates.
 * Date            : 2026.11.12
 ******************************************************************************/
bool renameAnimation(const HmiProject &source, const QString &name, const QString &newName, HmiProject *draft, QString *error) //20261112-0 add
{
    HmiProject candidate = source;
    QByteArray data = source.animations.value(name);
    if (draft == nullptr || !source.animations.contains(name) || newName.trimmed().isEmpty() || newName.size() > 256 || (name != newName && source.animations.contains(newName)))
    { return animationError(error, QStringLiteral("动画不存在或新名称冲突。")); }
    candidate.animations.remove(name); candidate.animations.insert(newName, data); updateAnimationBindings(&candidate, name, newName, data, data);
    if (!candidate.validateModel(error)) { return false; } *draft = candidate; return true;
}

/******************************************************************************
 * Function Name   : replaceAnimation
 * Description     : Replace raw animation data and exact-content bindings together, preserving all other resources.
 * Input Parameters: source, name, gif, draft, error
 * Return Value    : True after full validation.
 * Date            : 2026.11.12
 ******************************************************************************/
bool replaceAnimation(const HmiProject &source, const QString &name, const QByteArray &gif, HmiProject *draft, QString *error) //20261112-0 add
{
    HmiProject candidate = source;
    if (draft == nullptr || !source.animations.contains(name)) { return animationError(error, QStringLiteral("待替换动画不存在。")); }
    if (!validateGifData(gif, error)) { return false; }
    candidate.animations[name] = gif; updateAnimationBindings(&candidate, name, name, source.animations.value(name), gif);
    if (!candidate.validateModel(error)) { return false; } *draft = candidate; return true;
}

/******************************************************************************
 * Function Name   : deleteAnimation
 * Description     : Protect every page and explicit reference before removing a pool entry.
 * Input Parameters: source, name, draft, error
 * Return Value    : True only for an unused animation in a valid project.
 * Date            : 2026.11.12
 ******************************************************************************/
bool deleteAnimation(const HmiProject &source, const QString &name, HmiProject *draft, QString *error) //20261112-0 add
{
    HmiProject candidate = source;
    if (draft == nullptr || !source.animations.contains(name)) { return animationError(error, QStringLiteral("待删除动画不存在。")); }
    if (animationUsage(source, name) != 0) { return animationError(error, QStringLiteral("动画仍被控件或显式引用使用，请先解除引用。")); }
    candidate.animations.remove(name); if (!candidate.validateModel(error)) { return false; } *draft = candidate; return true;
}

/******************************************************************************
 * Function Name   : mergeAnimation
 * Description     : Deduplicate exact raw GIF bytes and derive stable content-based names for clipboard collisions.
 * Input Parameters: project, name, gif, resolvedName, error
 * Return Value    : True with the resolved animation resource key.
 * Date            : 2026.11.12
 ******************************************************************************/
bool mergeAnimation(HmiProject *project, const QString &name, const QByteArray &gif, QString *resolvedName, QString *error) //20261112-0 add
{
    QMap<QString, QByteArray>::const_iterator entry;
    QString key = name;
    QString digest = QString::fromLatin1(QCryptographicHash::hash(gif, QCryptographicHash::Sha256).toHex());
    int length = 12;
    if (project == nullptr || resolvedName == nullptr || !validateGifData(gif, error)) { return false; }
    for (entry = project->animations.constBegin(); entry != project->animations.constEnd(); ++entry)
    { if (entry.value() == gif) { *resolvedName = entry.key(); return true; } }
    if (key.trimmed().isEmpty()) { key = QStringLiteral("animation.gif"); }
    while (project->animations.contains(key))
    {
        key = name.left(180) + QStringLiteral("~") + digest.left(length);
        if (length == 64 && project->animations.contains(key)) { return animationError(error, QStringLiteral("动画内容哈希名称冲突。")); }
        length = qMin(64, length + 4);
    }
    project->animations.insert(key, gif); *resolvedName = key; return true;
}

/******************************************************************************
 * Function Name   : AnimationGalleryDialog::AnimationGalleryDialog
 * Description     : Edit a nested animation-only project gallery with real frame thumbnails and raw-file imports.
 * Input Parameters: project, selection, parent
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
AnimationGalleryDialog::AnimationGalleryDialog(const HmiProject &project, const QString &selection, QWidget *parent) //20261112-0 add
    : QDialog(parent), m_draft(project)
{
    QVBoxLayout *root;
    QHBoxLayout *content;
    QHBoxLayout *actions;
    QPushButton *button;
    QDialogButtonBox *buttons;
    int index;
    setObjectName(QStringLiteral("AnimationGallery")); setWindowTitle(QStringLiteral("工程图库")); resize(887, 495);
    root = new QVBoxLayout(this); content = new QHBoxLayout(); m_list = new QListWidget(this); m_list->setObjectName(QStringLiteral("AnimationList"));
    m_list->setViewMode(QListView::IconMode); m_list->setIconSize(QSize(90, 90)); m_list->setResizeMode(QListView::Adjust); m_list->setStyleSheet(QStringLiteral("background:#caeace;"));
    m_metadata = new QLabel(this); m_metadata->setObjectName(QStringLiteral("AnimationMetadata")); m_metadata->setMinimumWidth(180); m_metadata->setAlignment(Qt::AlignTop); m_metadata->setWordWrap(true);
    content->addWidget(m_list, 1); content->addWidget(m_metadata); root->addLayout(content, 1); actions = new QHBoxLayout();
    for (index = 0; index < 3; ++index)
    {
        button = new QPushButton(QStringList{QStringLiteral("添加图片"), QStringLiteral("删除图片"), QStringLiteral("批量添加图片")}.at(index), this);
        button->setObjectName(QStringLiteral("AnimationAction%1").arg(index)); actions->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, index]() {
            QStringList paths;
            QString error;
            HmiProject candidate;
            if (index == 1)
            {
                if (selectedAnimation().isEmpty()) { return; }
                if (!deleteAnimation(m_draft, selectedAnimation(), &candidate, &error)) { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), error); return; }
                m_draft = candidate; rebuild(QString()); return;
            }
            if (index == 0) { paths = {QFileDialog::getOpenFileName(this, QStringLiteral("选择图片文件"), QString(), QStringLiteral("GIF (*.gif)"))}; }
            else { paths = QFileDialog::getOpenFileNames(this, QStringLiteral("选择一个或多个图片文件"), QString(), QStringLiteral("GIF (*.gif)")); }
            paths.removeAll(QString()); if (paths.isEmpty()) { return; }
            if (!importFiles(paths, &error)) { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), error); }
        });
    }
    actions->addStretch(); buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this); buttons->setObjectName(QStringLiteral("AnimationButtons")); actions->addWidget(buttons); root->addLayout(actions);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消")); //20261112-0 add
    connect(buttons, &QDialogButtonBox::accepted, this, &AnimationGalleryDialog::accept); connect(buttons, &QDialogButtonBox::rejected, this, &AnimationGalleryDialog::reject);
    connect(m_list, &QListWidget::currentRowChanged, this, [this]() { refreshMetadata(); }); rebuild(selection);
}

/******************************************************************************
 * Function Name   : AnimationGalleryDialog::importFiles
 * Description     : Import an entire bounded file selection atomically while preserving every original GIF byte.
 * Input Parameters: paths, error
 * Return Value    : True after full validation; failure preserves the gallery draft.
 * Date            : 2026.11.12
 ******************************************************************************/
bool AnimationGalleryDialog::importFiles(const QStringList &paths, QString *error) //20261112-0 add
{
    HmiProject candidate = m_draft;
    QFile file;
    QString path;
    QString selected;
    QByteArray bytes;
    if (paths.size() > 256) { return animationError(error, QStringLiteral("一次导入超过256项保护上限。")); }
    Q_FOREACH (path, paths)
    {
        file.setFileName(path);
        if (!file.open(QIODevice::ReadOnly) || file.size() > 1024 * 1024) { return animationError(error, QStringLiteral("GIF不存在、无法读取或超过1 MiB。")); }
        bytes = file.read(1024 * 1024 + 1); file.close();
        if (!mergeAnimation(&candidate, QFileInfo(path).fileName(), bytes, &selected, error)) { return false; }
    }
    if (!candidate.validateModel(error)) { return false; } m_draft = candidate; rebuild(selected); return true;
}

/******************************************************************************
 * Function Name   : AnimationGalleryDialog::rebuild
 * Description     : Rebuild actual first-frame thumbnails without converting or replacing animation resources.
 * Input Parameters: selection
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void AnimationGalleryDialog::rebuild(const QString &selection) //20261112-0 add
{
    QMap<QString, QByteArray>::const_iterator entry;
    QListWidgetItem *item;
    m_list->clear();
    for (entry = m_draft.animations.constBegin(); entry != m_draft.animations.constEnd(); ++entry)
    {
        item = new QListWidgetItem(QIcon(QPixmap::fromImage(gifFirstFrame(entry.value()))), entry.key(), m_list);
        if (entry.key() == selection) { m_list->setCurrentItem(item); }
    }
    if (m_list->currentItem() == nullptr && m_list->count() > 0) { m_list->setCurrentRow(0); } refreshMetadata();
}

/******************************************************************************
 * Function Name   : AnimationGalleryDialog::refreshMetadata
 * Description     : Show local resource identity, exact GIF format and actual dimensions without inventing native IDs.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void AnimationGalleryDialog::refreshMetadata() //20261112-0 add
{
    QString name = selectedAnimation();
    QImage image = gifFirstFrame(m_draft.animations.value(name));
    m_metadata->setText(name.isEmpty() ? QString() : QStringLiteral("图片名称: %1\n描述: %1\n类型: GIF\n尺寸: %2x%3").arg(name).arg(image.width()).arg(image.height()));
}

/******************************************************************************
 * Function Name   : AnimationGalleryDialog::draftProject
 * Description     : Return the nested gallery draft without touching the live editor project.
 * Input Parameters: None
 * Return Value    : Detached project.
 * Date            : 2026.11.12
 ******************************************************************************/
HmiProject AnimationGalleryDialog::draftProject() const //20261112-0 add
{
    return m_draft;
}

/******************************************************************************
 * Function Name   : AnimationGalleryDialog::selectedAnimation
 * Description     : Resolve the selected local animation resource key.
 * Input Parameters: None
 * Return Value    : Selected key or an empty string.
 * Date            : 2026.11.12
 ******************************************************************************/
QString AnimationGalleryDialog::selectedAnimation() const //20261112-0 add
{
    return m_list->currentItem() == nullptr ? QString() : m_list->currentItem()->text();
}

/******************************************************************************
 * Function Name   : AnimationGalleryDialog::accept
 * Description     : Keep empty confirmation from inventing an image selection.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.12
 ******************************************************************************/
void AnimationGalleryDialog::accept() //20261112-0 add
{
    if (selectedAnimation().isEmpty()) { reject(); return; } QDialog::accept();
}
