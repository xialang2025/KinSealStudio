#include "v31_extract_dialog.h"
#include "dialog_button_text.h"

#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QLockFile>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QSet>
#include <QTemporaryDir>
#include <QtEndian>
#include <memory>
#include <utility>
#include <vector>
#include <zlib.h>

namespace {
using u32 = unsigned int;
constexpr u32 archiveLimit = 256U * 1024U * 1024U;
const QString extractionLockName = QStringLiteral(".kinseal-v31-extract.lock"); //20261125-81 add
struct ArchiveFile {
    QString path;
    QByteArray data;
    QString stage;
    QString backup;
    bool savedPrevious = false;
    bool installed = false;
};

/******************************************************************************
 * Function Name   : fail
 * Description     : Return a localized extraction failure to the caller.
 * Input Parameters: error, message, english
 * Return Value    : False
 * Date            : 2026.11.25
 ******************************************************************************/
bool fail(QString *error, const QString &message, const char *english) //20261125-81 add
{
    if (error) { *error = dialogButtonText(message, english); }
    return false;
}

/******************************************************************************
 * Function Name   : takeWord
 * Description     : Consume one big-endian Qt 4 record length without allocating.
 * Input Parameters: stream, number
 * Return Value    : True when four bytes were available
 * Date            : 2026.11.25
 ******************************************************************************/
bool takeWord(QByteArrayView &stream, u32 &number) //20261125-81 add
{
    if (stream.size() < 4) { return false; }
    number = qFromBigEndian<u32>(stream.data());
    stream = stream.sliced(4);
    return true;
}

/******************************************************************************
 * Function Name   : takeBlock
 * Description     : Bound each length-prefixed byte array against the remaining input.
 * Input Parameters: stream, value
 * Return Value    : True for a complete Qt byte array
 * Date            : 2026.11.25
 ******************************************************************************/
bool takeBlock(QByteArrayView &stream, QByteArrayView &value) //20261125-81 add
{
    u32 size = 0;
    if (!takeWord(stream, size)) { return false; }
    if (size == 0xffffffffU) { size = 0; } // Qt's null QString/QByteArray marker.
    if (size > stream.size()) { return false; }
    value = stream.first(size);
    stream = stream.sliced(size);
    return true;
}

/******************************************************************************
 * Function Name   : takeText
 * Description     : Read bounded big-endian Qt UTF-16 names, including Chinese paths.
 * Input Parameters: stream, text
 * Return Value    : True for a complete string
 * Date            : 2026.11.25
 ******************************************************************************/
bool takeText(QByteArrayView &stream, QString &text) //20261125-81 add
{
    QByteArrayView bytes;
    if (!takeBlock(stream, bytes) || bytes.size() > 65536 || bytes.size() % 2 != 0) { return false; }
    text.resize(bytes.size() / 2);
    qFromBigEndian<char16_t>(bytes.data(), text.size(), text.data());
    return true;
}

/******************************************************************************
 * Function Name   : validName
 * Description     : Keep decoded gallery components inside ordinary Windows data files.
 * Input Parameters: part
 * Return Value    : True for a single portable file or directory name
 * Date            : 2026.11.25
 ******************************************************************************/
bool validName(const QString &part) //20261125-81 add
{
    static const QRegularExpression invalid(QStringLiteral(R"([<>:"/\\|?*\x00-\x1f]|[. ]$|^(?:CON|PRN|AUX|NUL|CONIN\$|CONOUT\$|COM[1-9¹²³]|LPT[1-9¹²³])(?:\.|$))"),
                                            QRegularExpression::CaseInsensitiveOption);
    return !part.isEmpty() && part.compare(extractionLockName, Qt::CaseInsensitive) != 0 && !invalid.match(part).hasMatch();
}

/******************************************************************************
 * Function Name   : isLinkPath
 * Description     : Reject symbolic links and junctions in archive-controlled output paths.
 * Input Parameters: path
 * Return Value    : True when the path traverses a link
 * Date            : 2026.11.25
 ******************************************************************************/
bool isLinkPath(const QString &path) //20261125-81 add
{
    QFileInfo info(path);
    QString current;
    for (;;) {
        if (info.isSymLink() || info.isJunction()) { return true; }
        current = info.absoluteFilePath();
        info.setFile(info.absolutePath());
        if (info.absoluteFilePath() == current) { return false; }
    }
}

/******************************************************************************
 * Function Name   : decodeArchive
 * Description     : Decode the tagged V31 project and the flattened gallery member list.
 * Input Parameters: archive, projectPath, galleryBase, files, gallery, error
 * Return Value    : True after all members, lengths and output paths have been checked
 * Date            : 2026.11.25
 ******************************************************************************/
bool decodeArchive(QFile &archive, const QString &projectPath, const QString &galleryBase,
                   QList<ArchiveFile> &files, QString &gallery, QString *error) //20261125-81 add
{
    QByteArray encoded, decoded;
    QByteArrayView stream, bytes;
    QString name, folder;
    QStringList parts;
    QSet<QString> targets;
    u32 length = 0, count = 0;
    uLongf actual = 0;
    uLong compressed = 0;
    const auto invalid = [&]() { return fail(error, QStringLiteral("文件不是完整的 V31 标准组态。"), "The file is not a complete V31 configuration."); };
    if (archive.size() < 5 || archive.size() > archiveLimit) { return invalid(); }
    encoded = archive.read(archiveLimit + 1LL);
    if (archive.error() != QFileDevice::NoError || encoded.size() != archive.size() || encoded.size() < 5 ||
        encoded.size() > archiveLimit) { return invalid(); }
    length = qFromBigEndian<u32>(encoded.constData());
    if (length == 0 || length > archiveLimit) { return invalid(); }
    decoded.resize(length);
    actual = length;
    compressed = static_cast<uLong>(encoded.size() - 4);
    if (uncompress2(reinterpret_cast<Bytef *>(decoded.data()), &actual,
                    reinterpret_cast<const Bytef *>(encoded.constData() + 4), &compressed) != Z_OK ||
        actual != length || compressed != static_cast<uLong>(encoded.size() - 4)) { return invalid(); }
    stream = decoded;
    if (!takeText(stream, name) || name != QStringLiteral("begin save project") ||
        !takeBlock(stream, bytes) || bytes.isEmpty() || !takeText(stream, name) ||
        name != QStringLiteral("end save project") || !takeText(stream, folder) ||
        !takeWord(stream, count) || count > 5000) { return invalid(); }
    files.append({projectPath, bytes.toByteArray(), {}, {}});
    targets.insert(projectPath.toCaseFolded());
    folder.replace(QLatin1Char('\\'), QLatin1Char('/'));
    parts = folder.split(QLatin1Char('/'));
    if (!folder.isEmpty() || count > 0) {
        if (parts.first().compare(QStringLiteral("Picture"), Qt::CaseInsensitive) != 0) { return invalid(); }
        for (const QString &part : parts) {
            if (!validName(part)) { return invalid(); }
        }
        gallery = QDir(galleryBase).absoluteFilePath(folder);
        if (isLinkPath(gallery)) { return invalid(); }
    }
    for (u32 i = 0; i < count; ++i) {
        if (!takeText(stream, name)) { return invalid(); }
        if (name.isEmpty()) { continue; } // An empty native member has no data field.
        name.replace(QLatin1Char('\\'), QLatin1Char('/'));
        name = name.section(QLatin1Char('/'), -1);
        if (!validName(name) || !takeBlock(stream, bytes)) { return invalid(); }
        name = QDir(gallery).absoluteFilePath(name);
        if (targets.contains(name.toCaseFolded()) || isLinkPath(name)) { return invalid(); }
        targets.insert(name.toCaseFolded());
        files.append({name, bytes.toByteArray(), {}, {}});
    }
    return stream.empty() || invalid();
}

/******************************************************************************
 * Function Name   : makeParents
 * Description     : Track newly created directories so a failed extraction can remove empty folders.
 * Input Parameters: path, created
 * Return Value    : True when the directory exists
 * Date            : 2026.11.25
 ******************************************************************************/
bool makeParents(const QString &path, QStringList &created) //20261125-81 add
{
    QFileInfo info(path);
    if (info.exists()) { return info.isDir(); }
    if (info.absolutePath() == info.absoluteFilePath() ||
        !makeParents(info.absolutePath(), created)) { return false; }
    if (QDir().mkdir(path)) { created.prepend(path); return true; }
    return QFileInfo(path).isDir();
}

/******************************************************************************
 * Function Name   : installFiles
 * Description     : Prepare all new bytes on their destination volumes, then replace with rollback.
 * Input Parameters: files, projectFolder, gallery, replaceProject, error, progress
 * Return Value    : True after every output has been installed
 * Date            : 2026.11.25
 ******************************************************************************/
bool installFiles(QList<ArchiveFile> &files, const QString &projectFolder, const QString &gallery,
                  bool replaceProject, QString *error, const std::function<void(int)> &progress) //20261125-81 add
{
    QStringList folders{QDir(projectFolder).canonicalPath()};
    std::vector<std::unique_ptr<QLockFile>> locks;
    QTemporaryDir projectStage(QDir(projectFolder).filePath(QStringLiteral(".v31-XXXXXX")));
    std::unique_ptr<QTemporaryDir> galleryStage;
    qsizetype installedCount = 0;
    bool restored = true;
    if (files.size() > 1 && QFileInfo(projectFolder) != QFileInfo(gallery)) { folders.append(QDir(gallery).canonicalPath()); }
    folders.sort(Qt::CaseInsensitive);
    for (const QString &folder : std::as_const(folders)) {
        auto lock = std::make_unique<QLockFile>(QDir(folder).filePath(extractionLockName));
        lock->setStaleLockTime(0); // An active extraction can take longer than the default stale-lock timeout.
        if (folder.isEmpty() || !lock->tryLock(0)) {
            return fail(error, QStringLiteral("无法锁定输出目录。请检查写入权限，或在其他解压操作结束后重试。"),
                        "Cannot lock an output directory. Check permissions or retry after the other extraction finishes.");
        }
        locks.push_back(std::move(lock));
    }
    if (!projectStage.isValid()) { return fail(error, QStringLiteral("无法写入工程目录。"), "Cannot write to the project directory."); }
    if (files.size() > 1) {
        galleryStage = std::make_unique<QTemporaryDir>(QDir(gallery).filePath(QStringLiteral(".v31-XXXXXX")));
        if (!galleryStage->isValid()) { return fail(error, QStringLiteral("无法写入图库目录。"), "Cannot write to the gallery directory."); }
    }
    for (qsizetype i = 0; i < files.size(); ++i) {
        ArchiveFile &entry = files[i];
        const QDir stage(i ? galleryStage->path() : projectStage.path());
        QFile staged(stage.filePath(QString::number(i)));
        QFileInfo dest(entry.path);
        if (isLinkPath(entry.path) || (dest.exists() && ((i == 0 && !replaceProject) || !dest.isFile()))) {
            return fail(error, QStringLiteral("目标已存在或不是普通文件。"), "A destination already exists or is not a regular file.");
        }
        if (!staged.open(QIODevice::WriteOnly) || staged.write(entry.data) != entry.data.size() || !staged.flush()) {
            return fail(error, QStringLiteral("写入解压临时文件失败。"), "Could not stage the extracted data.");
        }
        staged.close();
        entry.stage = staged.fileName();
        entry.backup = entry.stage + QStringLiteral(".old");
        entry.data.clear();
        if (progress) { progress(static_cast<int>((i + 1) * 90 / files.size())); }
    }
    for (ArchiveFile &entry : files) {
        const QFileInfo dest(entry.path);
        if (isLinkPath(entry.path)) { break; }
        if (dest.exists()) {
            if ((installedCount == 0 && !replaceProject) || !dest.isFile() || !QFile::rename(entry.path, entry.backup)) { break; }
            entry.savedPrevious = true;
        }
        if (!QFile::rename(entry.stage, entry.path)) { break; }
        entry.installed = true;
        ++installedCount;
        if (progress && installedCount < files.size()) { progress(90 + static_cast<int>(installedCount * 9 / files.size())); }
    }
    if (files.last().installed) {
        if (progress) { progress(100); }
        return true;
    }
    for (auto it = files.rbegin(); it != files.rend(); ++it) {
        if (it->installed && !QFile::remove(it->path)) { restored = false; }
        if (it->savedPrevious && !QFile::rename(it->backup, it->path)) { restored = false; }
    }
    if (!restored) {
        projectStage.setAutoRemove(false);
        if (galleryStage) { galleryStage->setAutoRemove(false); }
        if (error) { *error = dialogButtonText(QStringLiteral("还原目标失败，备份目录："), "Recovery directories: ") +
                      projectStage.path() + (galleryStage ? QLatin1Char('\n') + galleryStage->path() : QString()); }
        return false;
    }
    return fail(error, QStringLiteral("输出文件被占用或无法替换。"), "Could not replace an output file.");
}
} // namespace

/******************************************************************************
 * Function Name   : extractV31Configuration
 * Description     : Extract a V31 archive without replacing any existing data on validation failure.
 * Input Parameters: archivePath, projectPath, galleryBase, replaceProject, error, progress
 * Return Value    : True when the project and all gallery resources have been written
 * Date            : 2026.11.25
 ******************************************************************************/
bool extractV31Configuration(const QString &archivePath, const QString &projectPath,
                             const QString &galleryBase, bool replaceProject, QString *error,
                             const std::function<void(int)> &progress) //20261125-81 add
{
    const QFileInfo source(archivePath), project(projectPath);
    QFile archive(archivePath);
    QString gallery;
    QStringList created;
    QList<ArchiveFile> files;
    bool done;
    if (error) { error->clear(); }
    if (progress) { progress(0); }
    if (source.suffix().compare(QStringLiteral("knr"), Qt::CaseInsensitive) ||
        project.suffix().compare(QStringLiteral("sm"), Qt::CaseInsensitive)) {
        return fail(error, QStringLiteral("请选择 .knr 压缩文件并保存为 .sm 工程。"), "Select a .knr file and save it as a .sm project.");
    }
    if (!archive.open(QIODevice::ReadOnly)) { return fail(error, QStringLiteral("无法打开压缩文件。"), "Cannot open compressed file."); }
    if (!decodeArchive(archive, project.absoluteFilePath(), galleryBase, files, gallery, error)) { return false; }
    for (const ArchiveFile &entry : std::as_const(files)) {
        if (QFileInfo(entry.path) == source || isLinkPath(entry.path)) { return fail(error, QStringLiteral("输出路径与源文件冲突或包含链接。"), "An output conflicts with the source or traverses a link."); }
    }
    done = makeParents(project.absolutePath(), created) && (gallery.isEmpty() || makeParents(gallery, created));
    if (done) { done = installFiles(files, project.absolutePath(), gallery, replaceProject, error, progress); }
    else { fail(error, QStringLiteral("无法创建输出目录。"), "Cannot create the output directory."); }
    if (!done) {
        for (const QString &directory : std::as_const(created)) { QDir().rmdir(directory); }
    }
    return done;
}

/******************************************************************************
 * Function Name   : showV31ExtractDialog
 * Description     : Offer the native two-path V31 extractor independently of the active editor project.
 * Input Parameters: parent
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void showV31ExtractDialog(QWidget *parent) //20261125-81 add
{
    QDialog dialog(parent);
    QGroupBox group(dialogButtonText(QStringLiteral("解压缩文件"), "Extract compressed file"), &dialog);
    QPushButton start(dialogButtonText(QStringLiteral("开始解压"), "Start extraction"), &group);
    QPushButton sourceBrowse(dialogButtonText(QStringLiteral("浏览..."), "Browse..."), &group);
    QPushButton destinationBrowse(sourceBrowse.text(), &group);
    QLabel sourceCaption(dialogButtonText(QStringLiteral("压缩文件："), "Archive:"), &group);
    QLabel destinationCaption(dialogButtonText(QStringLiteral("保存路径："), "Save to:"), &group);
    QLineEdit source(&group), destination(&group);
    QPushButton close(dialogButtonText(QStringLiteral("关闭"), "Close"), &dialog);
    QProgressBar progress(&dialog);
    QLabel note(dialogButtonText(QStringLiteral("仅支持113,114版本旧组态"), "For legacy version 113 and 114 projects"), &dialog);
    dialog.setObjectName(QStringLiteral("Compress"));
    dialog.setWindowTitle(dialogButtonText(QStringLiteral("解压"), "decompression"));
    dialog.resize(652, 277);
    group.setObjectName(QStringLiteral("groupBox_2")); group.setGeometry(30, 20, 591, 151);
    sourceCaption.setObjectName(QStringLiteral("label_4")); sourceCaption.setGeometry(20, 30, 75, 21);
    destinationCaption.setObjectName(QStringLiteral("label_3")); destinationCaption.setGeometry(20, 70, 75, 21);
    source.setObjectName(QStringLiteral("EditFilePath_2")); source.setGeometry(100, 30, 390, 20);
    destination.setObjectName(QStringLiteral("EditFileName_2")); destination.setGeometry(100, 70, 390, 20);
    sourceBrowse.setObjectName(QStringLiteral("FileNameBTN_2")); sourceBrowse.setGeometry(500, 30, 71, 23);
    destinationBrowse.setObjectName(QStringLiteral("FilePathBTN_2")); destinationBrowse.setGeometry(500, 70, 71, 23);
    start.setObjectName(QStringLiteral("StartUncompress")); start.setGeometry(90, 110, 481, 30);
    progress.setObjectName(QStringLiteral("progressBar")); progress.setGeometry(30, 180, 601, 23); progress.setValue(0);
    note.setObjectName(QStringLiteral("label")); note.setGeometry(30, 210, 381, 21);
    close.setObjectName(QStringLiteral("close")); close.setGeometry(540, 240, 81, 25);
    QObject::connect(&sourceBrowse, &QPushButton::clicked, &dialog, [&]() {
        const QString path = QFileDialog::getOpenFileName(&dialog,
            dialogButtonText(QStringLiteral("选择要解压的文件"), "Select the file you want to extract"),
            source.text(), QStringLiteral("Compressed File (*.knr)"));
        if (!path.isEmpty()) { source.setText(QDir::toNativeSeparators(path)); }
    });
    QObject::connect(&destinationBrowse, &QPushButton::clicked, &dialog, [&]() {
        const QString path = QFileDialog::getSaveFileName(&dialog,
            dialogButtonText(QStringLiteral("选择要存储的位置"), "Select the location you want to store"),
            destination.text(), QStringLiteral("Smart Device Files (*.sm)"), nullptr, QFileDialog::DontConfirmOverwrite);
        if (!path.isEmpty()) { destination.setText(QDir::toNativeSeparators(path)); }
    });
    QObject::connect(&close, &QPushButton::clicked, &dialog, &QDialog::reject);
    QObject::connect(&start, &QPushButton::clicked, &dialog, [&]() {
        const QFileInfo input(source.text()), output(destination.text());
        bool replaceProject;
        QString error;
        const auto confirm = [&](const QString &title, const QString &message) {
            return QMessageBox::question(&dialog, title, message, QMessageBox::Yes | QMessageBox::Cancel,
                                         QMessageBox::Cancel) == QMessageBox::Yes;
        };
        progress.setValue(0);
        if (input.suffix().compare(QStringLiteral("knr"), Qt::CaseInsensitive) != 0) {
            QMessageBox::warning(&dialog, dialogButtonText(QStringLiteral("文件格式不正确"), "Incorrect file format"),
                                 dialogButtonText(QStringLiteral("仅支持.knr格式解压"), "Only .knr archives can be extracted.")); return;
        }
        if (!input.isFile()) {
            QMessageBox::warning(&dialog, dialogButtonText(QStringLiteral("文件不存在"), "File does not exist"),
                                 dialogButtonText(QStringLiteral("请检查路径是否正确"), "Check whether the path is correct")); return;
        }
        if (output.suffix().compare(QStringLiteral("sm"), Qt::CaseInsensitive) != 0) {
            QMessageBox::warning(&dialog, dialogButtonText(QStringLiteral("文件名不正确"), "Incorrect file name"),
                                 dialogButtonText(QStringLiteral("请保存为“.sm”格式"), "Please save as .sm.")); return;
        }
        if (!output.absoluteDir().exists() && !confirm(dialogButtonText(QStringLiteral("路径不存在"), "Path does not exist"),
            dialogButtonText(QStringLiteral("是否创建该路径"), "Create this path?"))) { return; }
        replaceProject = output.exists();
        if (replaceProject && !confirm(dialogButtonText(QStringLiteral("文件已存在"), "File already exists"),
            dialogButtonText(QStringLiteral("是否覆盖该文件"), "Overwrite this file?"))) { return; }
        // The native reader resolves the archive's Picture folder from the editor's working directory.
        if (!extractV31Configuration(input.absoluteFilePath(), output.absoluteFilePath(), QDir::currentPath(), replaceProject,
                                     &error, [&](int value) { progress.setValue(value); })) {
            progress.setValue(0);
            QMessageBox::warning(&dialog, dialog.windowTitle(), error);
        }
    });
    dialog.exec();
}
