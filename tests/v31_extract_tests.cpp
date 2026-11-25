#include "v31_extract_dialog.h"

#include <QApplication>
#include <QDataStream>
#include <QDialog>
#include <QFile>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTest>
#include <QTimer>
#include <QCryptographicHash>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

namespace {
/******************************************************************************
 * Function Name   : saveBytes
 * Description     : Save exact fixture bytes to an isolated test directory.
 * Input Parameters: path, data
 * Return Value    : True when the whole fixture was written
 * Date            : 2026.11.25
 ******************************************************************************/
bool saveBytes(const QString &path, const QByteArray &data) //20261125-81 add
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
}

/******************************************************************************
 * Function Name   : readBytes
 * Description     : Read and compare an extracted fixture or pre-existing destination.
 * Input Parameters: path
 * Return Value    : Output bytes
 * Date            : 2026.11.25
 ******************************************************************************/
QByteArray readBytes(const QString &path) //20261125-81 add
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

/******************************************************************************
 * Function Name   : testArchive
 * Description     : Emit the observed native tags and Qt4 member frames for boundary tests.
 * Input Parameters: gallery, name
 * Return Value    : Qt compressed envelope containing an SM and one resource
 * Date            : 2026.11.25
 ******************************************************************************/
QByteArray testArchive(const QString &gallery, const QString &name) //20261125-81 add
{
    QByteArray bytes;
    QDataStream output(&bytes, QIODevice::WriteOnly);
    output.setVersion(QDataStream::Qt_4_0);
    output << QStringLiteral("begin save project") << QByteArray("source")
           << QStringLiteral("end save project") << gallery << qint32(1)
           << name << QByteArray("image");
    return qCompress(bytes);
}
}

class V31ExtractTests : public QObject {
    Q_OBJECT
private slots:
    void acceptsAndOverwrites();
    void rejectsDamagedOrConflictingOutputs();
    void respectsProjectCreatedDuringStaging();
    void rollsBackAfterInstallingProject();
    void serializesContendingProcesses();
    void extractsThroughDialog();
    void confirmsProjectReplacement();
    void officialCases();
};

/******************************************************************************
 * Function Name   : acceptsAndOverwrites
 * Description     : Exercise Chinese member names, path flattening and declined replacements.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void V31ExtractTests::acceptsAndOverwrites() //20261125-81 add
{
    QTemporaryDir dir;
    const QString input = dir.filePath(QStringLiteral("包.KNR"));
    const QString project = dir.filePath(QStringLiteral("new/工程.SM"));
    const QString image = dir.filePath(QStringLiteral("Picture/工程/图.kvg"));
    QString error;
    int last = -1;
    QVERIFY(saveBytes(input, testArchive(QStringLiteral("Picture/工程"), QStringLiteral("resource\\图.kvg"))));
    QVERIFY2(extractV31Configuration(input, project, dir.path(), false, &error, [&](int p) { last = p; }), qPrintable(error));
    QCOMPARE(readBytes(project), QByteArray("source"));
    QCOMPARE(readBytes(image), QByteArray("image"));
    QCOMPARE(last, 100);
    QVERIFY(saveBytes(project, "previous"));
    QVERIFY(!extractV31Configuration(input, project, dir.path(), false, &error));
    QCOMPARE(readBytes(project), QByteArray("previous"));
    QVERIFY2(extractV31Configuration(input, project, dir.path(), true, &error), qPrintable(error));
    QCOMPARE(readBytes(project), QByteArray("source"));
}

/******************************************************************************
 * Function Name   : rejectsDamagedOrConflictingOutputs
 * Description     : Preserve old files on incomplete records, altered lengths or unusable gallery paths.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void V31ExtractTests::rejectsDamagedOrConflictingOutputs() //20261125-81 add
{
    QTemporaryDir dir;
    const QString input = dir.filePath(QStringLiteral("bad.knr"));
    const QString project = dir.filePath(QStringLiteral("keep.sm"));
    const QByteArray good = testArchive(QStringLiteral("Picture/旧"), QStringLiteral("a.png"));
    QByteArray wrongLength(good), damaged(good);
    QString error;
    wrongLength[3] = static_cast<char>(wrongLength[3] ^ 1);
    damaged[damaged.size() - 1] = static_cast<char>(damaged.back() ^ 0x20);
    QVERIFY(saveBytes(project, "keep"));
    for (const QByteArray &data : {wrongLength, damaged, good.chopped(4), good + QByteArray(1, 'x'),
            qCompress(qUncompress(good).chopped(2)), qCompress(QByteArray("direct SM snapshot")),
            testArchive(QStringLiteral("Picture/../escape"), QStringLiteral("a.png")),
            testArchive(QStringLiteral("Picture/user"), QStringLiteral("CON.txt"))}) {
        QVERIFY(saveBytes(input, data));
        QVERIFY(!extractV31Configuration(input, project, dir.path(), true, &error));
        QVERIFY(!error.isEmpty());
        QCOMPARE(readBytes(project), QByteArray("keep"));
    }
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("Picture/旧/a.png")));
    QVERIFY(saveBytes(input, good));
    QVERIFY(!extractV31Configuration(input, project, dir.path(), true, &error));
    QCOMPARE(readBytes(project), QByteArray("keep"));
}

/******************************************************************************
 * Function Name   : respectsProjectCreatedDuringStaging
 * Description     : Keep an independently created SM when the user selected a previously unused name.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void V31ExtractTests::respectsProjectCreatedDuringStaging() //20261125-81 add
{
    QTemporaryDir dir;
    const QString input = dir.filePath(QStringLiteral("archive.knr"));
    const QString project = dir.filePath(QStringLiteral("new.sm"));
    const QString image = dir.filePath(QStringLiteral("Picture/demo/icon.png"));
    QString error;
    bool created = false;
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("Picture/demo")));
    QVERIFY(saveBytes(input, testArchive(QStringLiteral("Picture/demo"), QStringLiteral("icon.png"))));
    QVERIFY(saveBytes(image, "previous image"));
    QVERIFY(!extractV31Configuration(input, project, dir.path(), false, &error, [&](int progress) {
        if (progress == 90) { created = saveBytes(project, "independent project"); }
    }));
    QVERIFY(created);
    QCOMPARE(readBytes(project), QByteArray("independent project"));
    QCOMPARE(readBytes(image), QByteArray("previous image"));
    QVERIFY(QFile::remove(project));
    QVERIFY2(extractV31Configuration(input, project, dir.path(), false, &error), qPrintable(error));
    QCOMPARE(readBytes(project), QByteArray("source"));
    QCOMPARE(readBytes(image), QByteArray("image"));
}

/******************************************************************************
 * Function Name   : rollsBackAfterInstallingProject
 * Description     : Deny replacement of the second file after the SM is installed, restore it, then retry.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void V31ExtractTests::rollsBackAfterInstallingProject() //20261125-81 add
{
    QTemporaryDir dir;
    const QString input = dir.filePath(QStringLiteral("archive.knr"));
    const QString project = dir.filePath(QStringLiteral("old.sm"));
    const QString image = dir.filePath(QStringLiteral("Picture/demo/icon.png"));
    QString error;
    bool installed = false, blocked = false, done;
#ifdef Q_OS_WIN
    HANDLE handle = INVALID_HANDLE_VALUE;
#endif
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("Picture/demo")));
    QVERIFY(saveBytes(input, testArchive(QStringLiteral("Picture/demo"), QStringLiteral("icon.png"))));
    QVERIFY(saveBytes(project, "previous project"));
    QVERIFY(saveBytes(image, "previous image"));
    done = extractV31Configuration(input, project, dir.path(), true, &error, [&](int progress) {
        if (progress <= 90 || progress >= 100) { return; }
        installed = readBytes(project) == QByteArray("source");
#ifdef Q_OS_WIN
        handle = CreateFileW(reinterpret_cast<LPCWSTR>(image.utf16()), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                             nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        blocked = handle != INVALID_HANDLE_VALUE;
#else
        blocked = QFile::rename(image, image + QStringLiteral(".held")) && QDir().mkdir(image);
#endif
    });
#ifdef Q_OS_WIN
    if (handle != INVALID_HANDLE_VALUE) { CloseHandle(handle); }
#else
    if (blocked) { QVERIFY(QDir().rmdir(image)); QVERIFY(QFile::rename(image + QStringLiteral(".held"), image)); }
#endif
    QVERIFY(installed && blocked);
    QVERIFY(!done);
    QCOMPARE(readBytes(project), QByteArray("previous project"));
    QCOMPARE(readBytes(image), QByteArray("previous image"));
    QVERIFY2(extractV31Configuration(input, project, dir.path(), true, &error), qPrintable(error));
    QCOMPARE(readBytes(project), QByteArray("source"));
    QCOMPARE(readBytes(image), QByteArray("image"));
}

/******************************************************************************
 * Function Name   : serializesContendingProcesses
 * Description     : Start separate extractor processes that overlap just the project or just the gallery.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void V31ExtractTests::serializesContendingProcesses() //20261125-81 add
{
    QTemporaryDir dir;
    const QString input = dir.filePath(QStringLiteral("archive.knr"));
    const QString project = dir.filePath(QStringLiteral("p/first.sm"));
    const QString otherProject = dir.filePath(QStringLiteral("q/second.sm"));
    const QString otherGallery = dir.filePath(QStringLiteral("other"));
    const auto run = [&](const QString &output, const QString &gallery) {
        QProcess child;
        child.start(QCoreApplication::applicationFilePath(), {QStringLiteral("--v31-extract-probe"), input, output, gallery});
        if (!child.waitForFinished(10000) || child.exitStatus() != QProcess::NormalExit) { return -1; }
        return child.exitCode();
    };
    QString error;
    QList<int> exits;
    QVERIFY(saveBytes(input, testArchive(QStringLiteral("Picture/demo"), QStringLiteral("icon.png"))));
    QVERIFY2(extractV31Configuration(input, project, dir.path(), false, &error, [&](int progress) {
        if (progress == 90) {
            exits.append(run(project, otherGallery));
            exits.append(run(otherProject, dir.path()));
        }
    }), qPrintable(error));
    QCOMPARE(exits, QList<int>({2, 2}));
    QVERIFY(!QFileInfo::exists(otherProject));
    QCOMPARE(run(otherProject, dir.path()), 0);
    QCOMPARE(readBytes(otherProject), QByteArray("source"));
}

/******************************************************************************
 * Function Name   : extractsThroughDialog
 * Description     : Trigger the real extraction button and verify its two destinations and progress.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void V31ExtractTests::extractsThroughDialog() //20261125-81 add
{
    QTemporaryDir dir;
    const QString previous = QDir::currentPath();
    const QString input = dir.filePath(QStringLiteral("in.knr"));
    const QString project = dir.filePath(QStringLiteral("out.sm"));
    int progress = -1;
    QVERIFY(saveBytes(input, testArchive(QStringLiteral("Picture/demo"), QStringLiteral("a.png"))));
    QVERIFY(QDir::setCurrent(dir.path()));
    QTimer::singleShot(0, [&]() {
        QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
        if (!dialog) { return; }
        dialog->findChild<QLineEdit *>(QStringLiteral("EditFilePath_2"))->setText(input);
        dialog->findChild<QLineEdit *>(QStringLiteral("EditFileName_2"))->setText(project);
        dialog->findChild<QPushButton *>(QStringLiteral("StartUncompress"))->click();
        progress = dialog->findChild<QProgressBar *>(QStringLiteral("progressBar"))->value();
        dialog->findChild<QPushButton *>(QStringLiteral("close"))->click();
    });
    showV31ExtractDialog(nullptr);
    QVERIFY(QDir::setCurrent(previous));
    QCOMPARE(progress, 100);
    QCOMPARE(readBytes(project), QByteArray("source"));
    QCOMPARE(readBytes(dir.filePath(QStringLiteral("Picture/demo/a.png"))), QByteArray("image"));
}

/******************************************************************************
 * Function Name   : confirmsProjectReplacement
 * Description     : Declining the native SM overwrite preserves both outputs; accepting replaces both.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void V31ExtractTests::confirmsProjectReplacement() //20261125-81 add
{
    QTemporaryDir dir;
    const QString previous = QDir::currentPath();
    const QString input = dir.filePath(QStringLiteral("in.knr"));
    const QString project = dir.filePath(QStringLiteral("out.sm"));
    const QString image = dir.filePath(QStringLiteral("Picture/demo/a.png"));
    int prompts = 0;
    QVERIFY(saveBytes(input, testArchive(QStringLiteral("Picture/demo"), QStringLiteral("a.png"))));
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("Picture/demo")));
    QVERIFY(saveBytes(project, "old SM"));
    QVERIFY(saveBytes(image, "old image"));
    QVERIFY(QDir::setCurrent(dir.path()));
    for (const auto answer : {QMessageBox::Cancel, QMessageBox::Yes}) {
        QTimer::singleShot(0, [&]() {
            QDialog *dialog = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!dialog) { return; }
            dialog->findChild<QLineEdit *>(QStringLiteral("EditFilePath_2"))->setText(input);
            dialog->findChild<QLineEdit *>(QStringLiteral("EditFileName_2"))->setText(project);
            QTimer::singleShot(0, [&]() {
                QMessageBox *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
                if (box && box->standardButtons() == (QMessageBox::Yes | QMessageBox::Cancel)) {
                    ++prompts;
                    box->button(answer)->click();
                } else if (box) { box->reject(); }
            });
            dialog->findChild<QPushButton *>(QStringLiteral("StartUncompress"))->click();
            dialog->reject();
        });
        showV31ExtractDialog(nullptr);
        QCOMPARE(readBytes(project), answer == QMessageBox::Yes ? QByteArray("source") : QByteArray("old SM"));
        QCOMPARE(readBytes(image), answer == QMessageBox::Yes ? QByteArray("image") : QByteArray("old image"));
    }
    QVERIFY(QDir::setCurrent(previous));
    QCOMPARE(prompts, 2);
}

/******************************************************************************
 * Function Name   : officialCases
 * Description     : Compare unpacked project SHA-256 and gallery counts against three original legacy packages.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void V31ExtractTests::officialCases() //20261125-81 add
{
    struct Case { QString file; QByteArray digest; QString gallery; int count; };
    const QString originals = qEnvironmentVariable("KINSEAL_V31_SAMPLES");
    const QList<Case> cases = {
        {QStringLiteral("标准出厂组态-4.3寸"), "bd6dd4043cf095d2a342b9e09ddb7607a746d15a299cc8f7080827f8f13c8142", QStringLiteral("43寸标准出厂.sm"), 9},
        {QStringLiteral("MODBUS通讯示例"), "00c17a549eb954c942e195a58214b2bcc8d8bf9f3e88be5539d79c62fb4d2647", QStringLiteral("MODBUS通讯示例"), 21},
        {QStringLiteral("三菱FX系列PLC通讯示例"), "e1353122f95e1be70ff6306573d4dadde770d5fe1aaf66bdb701320fa2b3bd54", QStringLiteral("三菱FX系列PLC通讯示例"), 25}
    };
    if (originals.isEmpty()) { QSKIP("Set KINSEAL_V31_SAMPLES to the original sample folder"); }
    for (const Case &sample : cases) {
        QTemporaryDir dir;
        const QString input = QDir(originals).filePath(sample.file + QStringLiteral(".knr"));
        const QString project = dir.filePath(QStringLiteral("output.sm"));
        const QDir gallery(dir.filePath(QStringLiteral("Picture/") + sample.gallery));
        QString error;
        QVERIFY2(extractV31Configuration(input, project, dir.path(), false, &error), qPrintable(error));
        QCOMPARE(QCryptographicHash::hash(readBytes(project), QCryptographicHash::Sha256).toHex(), sample.digest);
        QCOMPARE(gallery.entryList(QDir::Files).size(), sample.count);
        QByteArray members = qUncompress(readBytes(input)), source, memberBytes;
        QDataStream reader(members);
        QString field, path;
        int count;
        reader.setVersion(QDataStream::Qt_4_0);
        reader >> field >> source >> field >> path >> count;
        for (int i = 0; i < count; ++i) {
            reader >> path >> memberBytes;
            QCOMPARE(readBytes(gallery.filePath(path.replace(QLatin1Char('\\'), QLatin1Char('/')).section(QLatin1Char('/'), -1))), memberBytes);
        }
        QVERIFY(reader.atEnd() && reader.status() == QDataStream::Ok);
    }
}
/******************************************************************************
 * Function Name   : main
 * Description     : Run the offscreen Qt tests or one independent contender with the supplied destinations.
 * Input Parameters: argc, argv
 * Return Value    : Qt test status, or an extractor result for the process-lock probe
 * Date            : 2026.11.25
 ******************************************************************************/
int main(int argc, char **argv) //20261125-81 add
{
    if (argc == 5 && QByteArray(argv[1]) == "--v31-extract-probe") {
        QCoreApplication app(argc, argv);
        const QStringList args = app.arguments();
        QString error;
        if (extractV31Configuration(args[2], args[3], args[4], true, &error)) { return 0; }
        return error.contains(QStringLiteral("锁定输出目录")) ? 2 : 3;
    }
    QApplication app(argc, argv);
    V31ExtractTests tests;
    return QTest::qExec(&tests, argc, argv);
}
#include "v31_extract_tests.moc"
