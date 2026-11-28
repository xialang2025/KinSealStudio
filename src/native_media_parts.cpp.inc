#include "native_media_snapshot.cpp.inc" //20261128-7 add
#include "native_system_compile_time.cpp.inc" //20261128-10 add

/******************************************************************************
 * Function Name   : compileNativeMediaParts
 * Description     : Publish the supported loose V122 media members from a fresh verified USB build.
 * Input Parameters: project, destination, omitDriver, original source location, nativeCompileTime, createdDrawing, error
 * Return Value    : True after the staged directory has been published.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool compileNativeMediaParts(const HmiProject &project, const QString &destination,
                                    bool omitDriver, const NativeMediaSourceLocation &location, //20261128-22 change
                                    const QString &nativeCompileTime, bool createdDrawing, QString *error) //20261128-19 change
{
    const QFileInfo target(destination);
    const QDir parent(target.absolutePath());
    const QString driverName = QStringLiteral("Modbus_RTU_com1.bin");
    const QString macroName = QStringLiteral("MarcoVar.dat");
    std::unique_ptr<QTemporaryDir> stage;
    QDir media;
    QDir usb;
    QString sourceName;
    QString contextName;
    QFile file;
    QByteArray source;
    QByteArray context;
    QByteArray mediaSnapshot;
    QByteArray systemBytes;
    QStringList payloadNames;
    QJsonObject manifest;

    if (destination.trimmed().isEmpty() || target.fileName().isEmpty() || target.exists() ||
        target.isSymLink() || !QDir().mkpath(parent.absolutePath())) {
        *error = QStringLiteral("介质散件须写入新的目录。");
        return false;
    }
    stage = std::make_unique<QTemporaryDir>(parent.filePath(QStringLiteral(".kinseal-media-XXXXXX")));
    if (!stage->isValid()) {
        *error = QStringLiteral("无法在目标所在卷创建介质暂存目录。");
        return false;
    }
    usb = QDir(stage->filePath(QStringLiteral("usb")));
    media = QDir(stage->filePath(QStringLiteral("media")));
    if (!exportProjectPayload(project, usb.absolutePath(), error, createdDrawing) ||
        !verifyDevicePackage(usb.absolutePath(), &manifest, error)) {
        return false;
    }
    // An opaque Lite snapshot or an active alarm configuration cannot justify an empty native alarm table.
    file.setFileName(usb.filePath(QStringLiteral("upLoad.knr")));
    if (!file.open(QIODevice::ReadOnly)) { *error = file.errorString(); return false; }
    source = qUncompress(file.readAll());
    file.close();
    if (!nativeProjectUsesSupportedDeviceDefaults(source, error) ||
        !buildNativeMediaSnapshot(source, &mediaSnapshot, error, location)) { return false; }
    //20261128-21 change: Five original picture-only projects, including unused gallery PNGs,
    // emit the same empty assetPairs list. The images travel in the control frames and source snapshot.
    const bool pngPicturePage = !project.objects.isEmpty() &&
        std::all_of(project.objects.cbegin(), project.objects.cend(), [](const HmiObject &object) {
            return object.type == QStringLiteral("image") && object.hasImageDisplayProperties;
        });
    const bool gifPage = !project.objects.isEmpty() && std::all_of(project.objects.cbegin(), project.objects.cend(),
        [](const HmiObject &object) { return object.type == QStringLiteral("gif"); });
    if ((!project.pictures.isEmpty() && !pngPicturePage) || (!project.animations.isEmpty() && !gifPage)) {
        *error = QStringLiteral("此介质导出尚未核实工程图库资源对表。");
        return false;
    }
    if (!QDir().mkpath(media.absolutePath())) { *error = QStringLiteral("介质暂存目录创建失败。"); return false; }
    payloadNames = manifest.value(QStringLiteral("files")).toObject().keys();
    for (const QString &name : payloadNames) {
        if (omitDriver && name == driverName) { continue; }
        if (!nativeCompileTime.isEmpty() && name == QStringLiteral("SysPara.dat")) { //20261128-10 add
            // Replay the local clock on the staged media copy, after validating the USB package.
            file.setFileName(usb.filePath(name));
            if (!file.open(QIODevice::ReadOnly)) { *error = file.errorString(); return false; }
            systemBytes = file.readAll();
            if (file.error() != QFileDevice::NoError) { *error = file.errorString(); return false; }
            file.close();
            if (!applyNativeSystemCompileTime(&systemBytes, nativeCompileTime, error)) { return false; }
            file.setFileName(media.filePath(name));
            if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly) ||
                file.write(systemBytes) != systemBytes.size() || !file.flush()) {
                *error = file.errorString(); return false;
            }
            file.close();
            continue;
        }
        if (name == QStringLiteral("upLoad.knr")) { //20261128-7 add
            // The original media writer fills its current-page slot before compressing its own copy.
            file.setFileName(media.filePath(name));
            if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly) ||
                file.write(mediaSnapshot) != mediaSnapshot.size() || !file.flush()) {
                *error = file.errorString(); return false;
            }
            file.close();
            continue;
        }
        file.setFileName(usb.filePath(name));
        if (!file.rename(media.filePath(name))) { *error = file.errorString(); return false; }
    }
    if (!media.exists(macroName)) {
        if (!project.macros.isEmpty()) {
            *error = QStringLiteral("工程声明了宏，但没有生成介质宏变量表。");
            return false;
        }
        file.setFileName(media.filePath(macroName));
        if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly)) { *error = file.errorString(); return false; }
        file.close();
    }
    file.setFileName(media.filePath(QStringLiteral("AlarmMuti.dat")));
    if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly)) { *error = file.errorString(); return false; }
    file.close();

    // Give the existing metadata writer only this build's decoded source and newly produced members.
    sourceName = stage->filePath(QStringLiteral("source.sm"));
    contextName = stage->filePath(QStringLiteral("context.json"));
    context = QJsonDocument(QJsonObject{{QStringLiteral("compiledDirectory"), media.absolutePath()},
                                        {QStringLiteral("assetPairs"), QJsonArray()}}).toJson();
    file.setFileName(sourceName);
    if (!file.open(QIODevice::WriteOnly) || file.write(source) != source.size() || !file.flush()) {
        *error = file.errorString(); return false;
    }
    file.close();
    file.setFileName(contextName);
    if (!file.open(QIODevice::WriteOnly) || file.write(context) != context.size() || !file.flush()) {
        *error = file.errorString(); return false;
    }
    file.close();
    if (!buildNativeLoadMetadata(sourceName, media.filePath(QStringLiteral("load.bin")), contextName, error)) {
        return false;
    }
    if (!QDir().rename(media.absolutePath(), target.absoluteFilePath())) {
        *error = QStringLiteral("介质散件目录发布失败，目标可能已被占用。");
        return false;
    }
    return true;
}

/******************************************************************************
 * Function Name   : exportNativeMediaSnapshot
 * Description     : Write the verified native project archive without compiling device payloads.
 * Input Parameters: input SM, destination KNR, original directory or opened SM path, error
 * Return Value    : True after the complete snapshot is committed.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool exportNativeMediaSnapshot(const QString &inputPath, const QString &outputPath,
                                      const NativeMediaSourceLocation &location, QString *error) //20261128-22 change
{
    QFile input(inputPath);
    QSaveFile output(outputPath);
    QByteArray source;
    QByteArray archive;

    if (QFileInfo(inputPath).suffix().compare(QStringLiteral("sm"), Qt::CaseInsensitive) != 0 ||
        QFileInfo(outputPath).suffix().compare(QStringLiteral("knr"), Qt::CaseInsensitive) != 0) {
        *error = QStringLiteral("输入文件须为 .sm，输出文件须为 .knr。");
        return false;
    }
    if (!input.open(QIODevice::ReadOnly) || input.size() > 64 * 1024 * 1024) {
        *error = QStringLiteral("原厂工程无法读取或超过 64 MiB。");
        return false;
    }
    source = input.readAll();
    if (input.error() != QFileDevice::NoError) { *error = input.errorString(); return false; }
    // A source-only public command must observe the same active protection as the editor.
    if (!authenticateNativeProjectFile(source, QString(), nullptr, nullptr, error) ||
        !buildNativeMediaSnapshot(source, &archive, error, location)) { return false; }
    if (!output.open(QIODevice::WriteOnly) || output.write(archive) != archive.size() || !output.commit()) {
        *error = output.errorString(); return false;
    }
    return true;
}
