#include "main_window.h"
#include "hmi_canvas.h"
#include "native_test_project.h" //20261108-7 add
#include "native_project_codec.h" //20261109-0 add
#include <QSaveFile>
#include "independent_downloader.h"
#include "device_package.h" //20261113-11 add //20261108-10 add
#include "text_payload_compiler.h" //20261108-11 add
#include "project_capabilities.h" //20261110-5 add
#include "device_transfer_contract.h" //20261114-5 add
#include "simulation_cli.h" //20261114-6 add
#include "macro_compiler_cli.h" //20261114-7 add
#include "plc_protocol_cli.h" //20261114-8 add
#include "local_help.h" //20261114-10 add

#include <QApplication>
#include <QSettings> //20261111-1 add: Keep automated captures out of the user recent-file preferences.
#include <QDir>
#include <QFile> //20261108-11 add
#include <QFileInfo>
#include <QGraphicsScene>
#include <QImage>
#include <QJsonDocument> //20261108-10 add
#include <QJsonObject> //20261108-10 add
#include <QJsonArray> //20261110-5 add
#include <QCryptographicHash>
#include <QTemporaryDir>
#include <QFontDatabase> //20261114-10 add: Report missing requested fonts without changing system fonts.
#include <QFontInfo>
#include <QAction>
#include <QDialog>
#include <QTextBrowser>

#include <cstdio> //20261108-10 add
#include <memory>

/******************************************************************************
 * Function Name   : writeDownloadResult
 * Description     : Emit one compact result for the editor's background USB worker.
 * Input Parameters: ok, message
 * Return Value    : None
 * Date            : 2026.11.08
 ******************************************************************************/
static void writeDownloadResult(bool ok, const QString &message) //20261108-10 add
{
    QJsonObject result;
    QByteArray bytes;

    result.insert(QStringLiteral("ok"), ok);
    result.insert(QStringLiteral("message"), message);
    bytes = QJsonDocument(result).toJson(QJsonDocument::Compact) + '\n';
    std::fwrite(bytes.constData(), 1, size_t(bytes.size()), stdout);
    std::fflush(stdout);
}

/******************************************************************************
 * Function Name   : writeStructuredTransferResult
 * Description     : Emit a bounded event or final milestone record for the private worker owner.
 * Input Parameters: result
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
static void writeStructuredTransferResult(const QJsonObject &result) //20261114-5 add
{
    QByteArray bytes;

    bytes = QJsonDocument(result).toJson(QJsonDocument::Compact) + '\n';
    std::fwrite(bytes.constData(), 1, size_t(bytes.size()), stdout);
    std::fflush(stdout);
}

/******************************************************************************
 * Function Name   : runProjectRoundTripTest
 * Description     : Verify that a model-bound prototype project saves and reloads without data loss.
 * Input Parameters: None
 * Return Value    : Zero when the round-trip assertions pass; nonzero otherwise.
 * Date            : 2026.11.07
 ******************************************************************************/
static int runProjectRoundTripTest() //20261107-6 add
{
    QTemporaryDir temporary_directory;
    HmiProject source_project;
    HmiProject loaded_project;
    HmiObject object;
    HmiControlItem *canvas_item;
    ScreenCanvas canvas;
    ScreenCanvas limit_canvas;
    QGraphicsScene test_scene;
    QList<HmiObject> canvas_objects;
    int control_index;
    int limit_signal_count;
    QJsonObject invalid_json;
    QJsonObject invalid_resolution;
    QString path;
    QString error;

    if (!temporary_directory.isValid())
    {
        return 2;
    }
    source_project.name = QStringLiteral("Round-trip test");
    source_project.targetModel = QStringLiteral("AMT070W124-L800X480R");
    source_project.pageName = QStringLiteral("画面1");
    source_project.pageWidth = 800;
    source_project.pageHeight = 480;
    object.id = QStringLiteral("round-trip-object");
    object.type = QStringLiteral("text");
    object.name = QStringLiteral("测试文本");
    object.text = QStringLiteral("下载验证");
    object.address = QStringLiteral("LW0");
    object.geometry = QRect(120, 80, 180, 44);
    source_project.objects.append(object);
    path = QDir(temporary_directory.path()).filePath(QStringLiteral("roundtrip.ksp"));

    if (!source_project.save(path, &error))
    {
        return 3;
    }
    if (!HmiProject::load(path, &loaded_project, &error))
    {
        return 4;
    }
    if (loaded_project.targetModel != source_project.targetModel ||
        loaded_project.pageWidth != 800 || loaded_project.pageHeight != 480 ||
        loaded_project.objects.size() != 1 ||
        loaded_project.objects.first().id != object.id ||
        loaded_project.objects.first().geometry != object.geometry)
    {
        return 5;
    }
    invalid_json = source_project.toJson();
    invalid_resolution = invalid_json.value(QStringLiteral("resolution")).toObject();
    invalid_resolution.insert(QStringLiteral("width"), 1024);
    invalid_json.insert(QStringLiteral("resolution"), invalid_resolution);
    if (HmiProject::fromJson(invalid_json, &loaded_project, &error))
    {
        return 6;
    }
    canvas.addControl(QStringLiteral("text"), QPointF(-50.0, 900.0));
    canvas_objects = canvas.objects();
    if (canvas_objects.size() != 1 ||
        canvas_objects.first().geometry != QRect(0, 448, 120, 32))
    {
        return 7;
    }
    test_scene.setSceneRect(QRectF(0.0, 0.0, 800.0, 480.0));
    canvas_item = new HmiControlItem(object);
    test_scene.addItem(canvas_item);
    canvas_item->setPos(900.0, 900.0);
    if (canvas_item->pos() != QPointF(620.0, 436.0))
    {
        return 8;
    }
    limit_signal_count = 0;
    QObject::connect(&limit_canvas, &ScreenCanvas::controlLimitReached,
                     [&limit_signal_count]() {
                         ++limit_signal_count;
                     });
    for (control_index = 0; control_index < 100; ++control_index)
    {
        limit_canvas.addControl(QStringLiteral("text"), QPointF(0.0, 0.0));
    }
    limit_canvas.addControl(QStringLiteral("text"), QPointF(0.0, 0.0));
    if (limit_canvas.objects().size() != 100 || limit_signal_count != 1)
    {
        return 9;
    }
    return 0;
}

/******************************************************************************
 * Function Name   : main
 * Description     : Launch the editor or run its isolated round-trip and offscreen capture modes.
 * Input Parameters: argc, argv
 * Return Value    : Application exit code.
 * Date            : 2026.11.07
 ******************************************************************************/
int main(int argc, char *argv[]) //20261107-6 add
{
    QApplication application(argc, argv);
    QStringList arguments;
    int simulationExit = 0; //20261114-6 add
    std::unique_ptr<MainWindow> window;
    QString capture_path;
    std::unique_ptr<QTemporaryDir> captureSettings; //20261114-8 change: Read-only CLI commands must not create capture settings directories.
    QJsonArray captureWidgets;
    QJsonObject captureWidget;
    QWidget *captureChild;
    QPoint capturePosition;
    QFile captureMetadata;
    HmiProject validation_project;
    QString validation_error;
    QImage preview;
    QByteArray probe_frame; //20261108-11 add
    QFile probe_output;
    int probe_point_size;
    QString probe_font_family; //20261108-11 add
    bool probe_pixel_size;
    QByteArray compressed_probe; //20261108-11 add
    QByteArray nativeBytes; //20261109-0 add
    QSaveFile nativeFile;
    QByteArray capabilityBytes; //20261110-5 add
    QFile inspectInput;
    QByteArray inspectBytes;
    QList<NativeSourceFieldSpan> nativeFields;
    NativeSourceFieldSpan nativeField;
    QJsonObject inspectObject;
    QJsonObject inspectField;
    QJsonArray inspectFields;
    QVector<QByteArray> transferPayloads; //20261114-5 add
    QJsonObject transferContract;
    QJsonObject transferIdentity;
    QJsonObject transferResult;
    DeviceTransferOptions transferOptions;
    QFile transferSource;
    QByteArray transferSourceBytes;
    std::unique_ptr<QTemporaryDir> transferSnapshotDirectory;
    QSaveFile transferFrozenSource;
    QString transferFrozenPath;
    QString cancelPath;
    int cancelIndex;
    QJsonObject releaseInfo; //20261114-10 add
    QJsonArray releaseFonts;
    QJsonObject releaseFont;
    QString releaseFamily;
    QStringList releaseRequestedFamilies;
    QString releaseRequestedFamily;
    int releaseFontIndex;
    QAction *releaseHelpAction;
    QDialog *releaseHelpDialog;
    QTextBrowser *releaseHelpBrowser;

    application.setApplicationName(QStringLiteral("KinSealStudioLite")); //20261114-10 add
    application.setApplicationVersion(QStringLiteral(KINSEAL_RELEASE_VERSION));
    arguments = application.arguments();
    if (arguments.size() == 2 && arguments.at(1) == QStringLiteral("--release-info")) //20261114-10 add
    {
        releaseInfo.insert(QStringLiteral("product"), QStringLiteral("KinSealStudio Lite"));
        releaseInfo.insert(QStringLiteral("version"), application.applicationVersion());
        releaseInfo.insert(QStringLiteral("qtVersion"), QString::fromLatin1(qVersion()));
        releaseInfo.insert(QStringLiteral("distribution"), QStringLiteral("restricted_internal_validation"));
        releaseInfo.insert(QStringLiteral("installerProvided"), false);
        releaseInfo.insert(QStringLiteral("hardwareIo"), false);
        releaseInfo.insert(QStringLiteral("fullReplicaCertified"), false);
        releaseRequestedFamilies = {QStringLiteral("微软雅黑"), QStringLiteral("Arial")};
        for (releaseFontIndex = 0; releaseFontIndex < releaseRequestedFamilies.size(); ++releaseFontIndex)
        {
            releaseRequestedFamily = releaseRequestedFamilies.at(releaseFontIndex);
            releaseFamily = QFontInfo(QFont(releaseRequestedFamily)).family();
            releaseFont = QJsonObject();
            releaseFont.insert(QStringLiteral("requested"), releaseRequestedFamily);
            releaseFont.insert(QStringLiteral("available"), QFontDatabase::families().contains(releaseRequestedFamily, Qt::CaseInsensitive));
            releaseFont.insert(QStringLiteral("resolved"), releaseFamily);
            releaseFonts.append(releaseFont);
        }
        releaseInfo.insert(QStringLiteral("fonts"), releaseFonts);
        writeStructuredTransferResult(releaseInfo);
        return 0;
    }
    if (arguments.size() == 2 && arguments.at(1) == QStringLiteral("--help-catalog")) //20261114-10 add: Audit shipped local files without opening a device or window.
    {
        releaseInfo = helpCatalogReport();
        writeStructuredTransferResult(releaseInfo);
        return releaseInfo.value(QStringLiteral("valid")).toBool() ? 0 : 28;
    }
    if (handlePlcProtocolCli(arguments, &simulationExit)) { return simulationExit; } //20261114-8 add
    if (handleMacroCompilerCli(arguments, &simulationExit)) { return simulationExit; } //20261114-7 add
    if (handleSimulationCli(arguments, &simulationExit)) { return simulationExit; } //20261114-6 add
    if (arguments.size() >= 4 &&
        arguments.at(1) == QStringLiteral("--render-text-frame")) //20261108-11 add: Offline raster comparison only.
    {
        probe_point_size = arguments.size() >= 5 ? arguments.at(4).toInt() : 11;
        probe_font_family = arguments.size() >= 6 ? arguments.at(5) : QStringLiteral("微软雅黑");
        probe_pixel_size = arguments.size() >= 7 && arguments.at(6) == QStringLiteral("pixel");
        if (!HmiProject::load(arguments.at(2), &validation_project, &validation_error) ||
            !renderSingleTextFrame(validation_project, probe_font_family,
                                   probe_point_size, probe_pixel_size,
                                   &probe_frame, &validation_error))
        {
            return 15;
        }
        probe_output.setFileName(arguments.at(3));
        if (!probe_output.open(QIODevice::WriteOnly) ||
            probe_output.write(probe_frame) != probe_frame.size())
        {
            return 16;
        }
        return 0;
    }
    if (arguments.size() >= 4 &&
        arguments.at(1) == QStringLiteral("--compress-rgb565-frame")) //20261108-11 add: Cross-check against the known codec.
    {
        probe_output.setFileName(arguments.at(2));
        if (!probe_output.open(QIODevice::ReadOnly) ||
            !compressRgb565Frame(probe_output.readAll(), &compressed_probe,
                                 &validation_error))
        {
            return 17;
        }
        probe_output.close();
        probe_output.setFileName(arguments.at(3));
        if (!probe_output.open(QIODevice::WriteOnly) ||
            probe_output.write(compressed_probe) != compressed_probe.size())
        {
            return 18;
        }
        return 0;
    }
    if (arguments.contains(QStringLiteral("--self-test")))
    {
        return runProjectRoundTripTest();
    }
    if (arguments.contains(QStringLiteral("--verify-independent-payload"))) //20261108-10 add
    {
        if (!verifyFixedTestPayload(&validation_error))
        {
            writeDownloadResult(false, validation_error);
            return 12;
        }
        writeDownloadResult(true, QStringLiteral("TEST 测试数据校验通过。"));
        return 0;
    }
    if (arguments.size() >= 3 &&
        arguments.at(1) == QStringLiteral("--verify-single-text-compiler")) //20261108-11 add
    {
        if (!HmiProject::load(arguments.at(2), &validation_project, &validation_error) ||
            !verifySingleTextCompiler(validation_project, &validation_error))
        {
            writeDownloadResult(false, validation_error);
            return 20;
        }
        writeDownloadResult(true, QStringLiteral("单文本差分编译与输入拒绝测试通过。"));
        return 0;
    }
    if (arguments.size() >= 3 && arguments.at(1) == QStringLiteral("--inspect-native-source")) //20261110-5 add
    {
        inspectInput.setFileName(arguments.at(2));
        if (!inspectInput.open(QIODevice::ReadOnly) || inspectInput.size() > 64 * 1024 * 1024)
        {
            writeDownloadResult(false, QStringLiteral("原厂源无法读取或超过64 MiB结构检查保护。"));
            return 22;
        }
        inspectBytes = inspectInput.readAll();
        if (inspectInput.error() != QFileDevice::NoError ||
            !inspectNativeProjectSourceFields(inspectBytes, &nativeFields, &validation_error))
        {
            writeDownloadResult(false, validation_error);
            return 22;
        }
        Q_FOREACH (nativeField, nativeFields)
        {
            inspectField = QJsonObject();
            inspectField.insert(QStringLiteral("path"), nativeField.path);
            inspectField.insert(QStringLiteral("kind"), QString::fromLatin1(&nativeField.kind, 1));
            inspectField.insert(QStringLiteral("start"), double(nativeField.start));
            inspectField.insert(QStringLiteral("end"), double(nativeField.end));
            inspectFields.append(inspectField);
        }
        inspectObject.insert(QStringLiteral("bodyValidated"), true);
        inspectObject.insert(QStringLiteral("semanticImportPerformed"), false);
        inspectObject.insert(QStringLiteral("usbAccessed"), false);
        inspectObject.insert(QStringLiteral("sha256"), QString::fromLatin1(QCryptographicHash::hash(inspectBytes, QCryptographicHash::Sha256).toHex()));
        inspectObject.insert(QStringLiteral("bytes"), inspectBytes.size());
        inspectObject.insert(QStringLiteral("fields"), inspectFields);
        capabilityBytes = QJsonDocument(inspectObject).toJson(QJsonDocument::Compact) + '\n';
        if (arguments.size() >= 4)
        {
            if (QFileInfo(arguments.at(3)).suffix().compare(QStringLiteral("json"), Qt::CaseInsensitive) != 0) //20261110-5 add: A read-only field report must not overwrite a native source path.
            {
                writeDownloadResult(false, QStringLiteral("原厂源结构报告输出必须为.json，原始.sm保持不变。"));
                return 22;
            }
            nativeFile.setFileName(arguments.at(3));
            if (!nativeFile.open(QIODevice::WriteOnly) || nativeFile.write(capabilityBytes) != capabilityBytes.size() || !nativeFile.commit())
            {
                writeDownloadResult(false, nativeFile.errorString());
                return 22;
            }
            writeDownloadResult(true, QStringLiteral("原厂源typed边界已完整检查并记录；未进行语义编辑或访问USB。"));
        }
        else
        {
            std::fwrite(capabilityBytes.constData(), 1, size_t(capabilityBytes.size()), stdout);
            std::fflush(stdout);
        }
        return 0;
    }
    if (arguments.size() >= 3 && arguments.at(1) == QStringLiteral("--project-capabilities")) //20261110-5 add
    {
        if (!HmiProject::load(arguments.at(2), &validation_project, &validation_error))
        {
            writeDownloadResult(false, validation_error);
            return 13;
        }
        capabilityBytes = QJsonDocument(describeProjectCapabilities(validation_project)).toJson(QJsonDocument::Compact) + '\n';
        std::fwrite(capabilityBytes.constData(), 1, size_t(capabilityBytes.size()), stdout);
        std::fflush(stdout);
        return 0;
    }
    if (arguments.size() >= 4 && arguments.at(1) == QStringLiteral("--migrate-project")) //20261110-5 add
    {
        if (QFileInfo(arguments.at(3)).suffix().compare(QStringLiteral("ksp"), Qt::CaseInsensitive) != 0)
        {
            writeDownloadResult(false, QStringLiteral("工程迁移的输出必须为.ksp，不能用JSON覆写原厂.sm源文件。"));
            return 21;
        }
        if (!HmiProject::load(arguments.at(2), &validation_project, &validation_error) ||
            !validation_project.save(arguments.at(3), &validation_error))
        {
            writeDownloadResult(false, validation_error);
            return 21;
        }
        writeDownloadResult(true, QStringLiteral("工程已验证并原子保存；保留页面与引用，尚未访问USB。"));
        return 0;
    }
    if (arguments.size() >= 4 && arguments.at(1) == QStringLiteral("--transfer-contract")) //20261114-5 add: This entry never initializes the USB adapter.
    {
        if (QFileInfo(arguments.at(3)).suffix().compare(QStringLiteral("json"), Qt::CaseInsensitive) != 0 ||
            transferContractWouldReplaceInput(arguments.at(2), arguments.at(3)))
        {
            writeDownloadResult(false, QStringLiteral("传输契约必须写入独立.json文件。"));
            return 24;
        }
        transferSource.setFileName(arguments.at(2));
        if (!transferSource.open(QIODevice::ReadOnly) || transferSource.size() > 128 * 1024 * 1024)
        {
            writeDownloadResult(false, QStringLiteral("无法冻结源输入，或超过128MiB读取保护。"));
            return 24;
        }
        transferSourceBytes = transferSource.readAll();
        if (transferSource.error() != QFileDevice::NoError)
        {
            writeDownloadResult(false, transferSource.errorString());
            return 24;
        }
        transferSource.close();
        transferSnapshotDirectory.reset(new QTemporaryDir());
        transferFrozenPath = transferSnapshotDirectory->filePath(
            QFileInfo(arguments.at(2)).suffix().compare(QStringLiteral("sm"), Qt::CaseInsensitive) == 0
                ? QStringLiteral("source.sm") : QStringLiteral("source.ksp"));
        transferFrozenSource.setFileName(transferFrozenPath);
        if (!transferSnapshotDirectory->isValid() || !transferFrozenSource.open(QIODevice::WriteOnly) ||
            transferFrozenSource.write(transferSourceBytes) != transferSourceBytes.size() || !transferFrozenSource.commit())
        {
            writeDownloadResult(false, QStringLiteral("无法保存本次不可变编译输入。"));
            return 24;
        }
        if (!HmiProject::load(transferFrozenPath, &validation_project, &validation_error) ||
            !buildProjectPayload(validation_project, &transferPayloads, &validation_error) ||
            !validateDevicePayloadStructure(transferPayloads, &validation_error))
        {
            writeDownloadResult(false, validation_error);
            return 24;
        }
        transferIdentity.insert(QStringLiteral("sourceSha256"), QString::fromLatin1(QCryptographicHash::hash(transferSourceBytes, QCryptographicHash::Sha256).toHex()));
        transferIdentity.insert(QStringLiteral("modelSha256"), QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(validation_project.toJson()).toJson(QJsonDocument::Compact), QCryptographicHash::Sha256).toHex()));
        transferIdentity.insert(QStringLiteral("targetModel"), validation_project.targetModel);
        transferIdentity.insert(QStringLiteral("width"), validation_project.pageWidth);
        transferIdentity.insert(QStringLiteral("height"), validation_project.pageHeight);
        transferIdentity.insert(QStringLiteral("firmwareVersion"), QStringLiteral("not-observed"));
        transferIdentity.insert(QStringLiteral("compiled"), true);
        if (!buildDeviceTransferContract(transferPayloads, transferIdentity, &transferContract, &validation_error))
        {
            writeDownloadResult(false, validation_error);
            return 24;
        }
        nativeFile.setFileName(arguments.at(3));
        capabilityBytes = QJsonDocument(transferContract).toJson(QJsonDocument::Indented);
        if (transferContractWouldReplaceInput(arguments.at(2), arguments.at(3)) ||
            !nativeFile.open(QIODevice::WriteOnly) || nativeFile.write(capabilityBytes) != capabilityBytes.size() || !nativeFile.commit())
        {
            writeDownloadResult(false, nativeFile.errorString());
            return 24;
        }
        writeDownloadResult(true, QStringLiteral("逐步传输契约已生成；未打开USB，未执行下载。"));
        return 0;
    }
    if (arguments.size() >= 3 &&
        arguments.at(1) == QStringLiteral("--download-project")) //20261109-0 add
    {
        transferResult = deviceTransferResultToJson(DeviceTransferResult());
        transferResult.insert(QStringLiteral("compiled"), false);
        transferResult.insert(QStringLiteral("kind"), QStringLiteral("transfer-result"));
        if (!HmiProject::load(arguments.at(2), &validation_project, &validation_error))
        {
            transferResult.insert(QStringLiteral("message"), validation_error);
            transferResult.insert(QStringLiteral("outcome"), QStringLiteral("source_rejected"));
            writeStructuredTransferResult(transferResult);
            return 13;
        }
        cancelIndex = arguments.indexOf(QStringLiteral("--cancel-file"));
        if (cancelIndex >= 0)
        {
            if (cancelIndex + 1 >= arguments.size())
            {
                transferResult.insert(QStringLiteral("message"), QStringLiteral("缺少本次下载取消标记路径，未访问USB。"));
                writeStructuredTransferResult(transferResult);
                return 14;
            }
            cancelPath = arguments.at(cancelIndex + 1);
            transferOptions.cancelRequested = [cancelPath]() { return QFileInfo::exists(cancelPath); };
        }
        transferOptions.onEvent = [](const DeviceTransferEvent &event) {
            QJsonObject record;

            record = deviceTransferEventToJson(event);
            record.insert(QStringLiteral("kind"), QStringLiteral("transfer-event"));
            writeStructuredTransferResult(record);
        };
        if (!downloadProjectDetailed(validation_project, &transferResult, &validation_error, transferOptions))
        {
            transferResult.insert(QStringLiteral("kind"), QStringLiteral("transfer-result"));
            writeStructuredTransferResult(transferResult);
            return 14;
        }
        transferResult.insert(QStringLiteral("kind"), QStringLiteral("transfer-result"));
        writeStructuredTransferResult(transferResult);
        return 0;
    }
    if (arguments.size() >= 3 && arguments.at(1) == QStringLiteral("--verify-compiled-package")) //20261113-11 add: No USB access is involved in artifact verification.
    {
        if (!verifyDevicePackage(arguments.at(2), &inspectObject, &validation_error))
        { writeDownloadResult(false,validation_error); return 23; }
        capabilityBytes = QJsonDocument(inspectObject).toJson(QJsonDocument::Compact) + '\n';
        std::fwrite(capabilityBytes.constData(),1,size_t(capabilityBytes.size()),stdout); std::fflush(stdout);
        return 0;
    }
    if (arguments.size() >= 4 &&
        arguments.at(1) == QStringLiteral("--compile-project")) //20261109-0 add
    {
        if (!HmiProject::load(arguments.at(2), &validation_project, &validation_error) ||
            !exportProjectPayload(validation_project, arguments.at(3), &validation_error))
        {
            writeDownloadResult(false, validation_error);
            return 19;
        }
        writeDownloadResult(true, QStringLiteral("工程设备数据编译完成，尚未访问USB。"));
        return 0;
    }
    if (arguments.size() >= 3 &&
        (arguments.at(1) == QStringLiteral("--download-native-test") ||
         arguments.at(1) == QStringLiteral("--download-single-text"))) //20261108-11 change
    {
        if (!HmiProject::load(arguments.at(2), &validation_project, &validation_error))
        {
            writeDownloadResult(false, validation_error);
            return 13;
        }
        if (!downloadSingleTextProject(validation_project, &validation_error)) //20261108-11 change
        {
            writeDownloadResult(false, validation_error);
            return 14;
        }
        writeDownloadResult(true, QStringLiteral("单文本工程的 USB 数据段和提交命令均获设备确认，重启命令已发送。请核对屏幕文字和位置。")); //20261108-11 change
        return 0;
    }
    if (arguments.size() >= 4 &&
        arguments.at(1) == QStringLiteral("--compile-single-text")) //20261108-11 add: Offline compiler inspection.
    {
        if (!HmiProject::load(arguments.at(2), &validation_project, &validation_error) ||
            !exportSingleTextPayload(validation_project, arguments.at(3), &validation_error))
        {
            writeDownloadResult(false, validation_error);
            return 19;
        }
        writeDownloadResult(true, QStringLiteral("单文本 USB 数据编译完成。"));
        return 0;
    }
    if (arguments.size() >= 3 && arguments.at(1) == QStringLiteral("--validate-project"))
    {
        if (!HmiProject::load(arguments.at(2), &validation_project, &validation_error))
        {
            return 7;
        }
        return 0;
    }
    if (arguments.size() >= 4 && arguments.at(1) == QStringLiteral("--export-native-project")) //20261109-0 add
    {
        if (!HmiProject::load(arguments.at(2), &validation_project, &validation_error) ||
            !exportNativeProjectSource(validation_project, &nativeBytes, &validation_error))
        {
            writeDownloadResult(false, validation_error); return 20;
        }
        nativeFile.setFileName(arguments.at(3));
        if (!nativeFile.open(QIODevice::WriteOnly) || nativeFile.write(nativeBytes) != nativeBytes.size() || !nativeFile.commit())
        {
            writeDownloadResult(false, nativeFile.errorString()); return 20;
        }
        writeDownloadResult(true, QStringLiteral("原厂工程导出完成。")); return 0;
    }
    if (arguments.size() >= 4 && arguments.at(1) == QStringLiteral("--export-native-test")) //20261108-7 add
    {
        if (!HmiProject::load(arguments.at(2), &validation_project, &validation_error))
        {
            return 10;
        }
        if (!exportNativeTestProject(validation_project, arguments.at(3), &validation_error))
        {
            return 11;
        }
        return 0;
    }
    if (arguments.size() >= 3 && arguments.at(1) == QStringLiteral("--capture-help")) //20261114-10 add: Exercise the actual help QAction on an owned desktop.
    {
        capture_path = QFileInfo(arguments.at(2)).absoluteFilePath();
        QSettings::setDefaultFormat(QSettings::IniFormat);
        captureSettings = std::make_unique<QTemporaryDir>();
        if (!captureSettings->isValid())
        {
            return 27;
        }
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, captureSettings->path());
        window = std::make_unique<MainWindow>();
        releaseHelpAction = window->findChild<QAction *>(QStringLiteral("action_H"));
        if (releaseHelpAction == nullptr)
        {
            return 28;
        }
        releaseHelpAction->trigger();
        application.processEvents();
        releaseHelpDialog = window->findChild<QDialog *>(QStringLiteral("LocalHelpDialog"));
        releaseHelpBrowser = window->findChild<QTextBrowser *>(QStringLiteral("LocalHelpBrowser"));
        if (releaseHelpDialog == nullptr || releaseHelpBrowser == nullptr)
        {
            return 28;
        }
        releaseInfo = QJsonObject();
        releaseInfo.insert(QStringLiteral("version"), application.applicationVersion());
        releaseInfo.insert(QStringLiteral("helpText"), releaseHelpBrowser->toPlainText());
        releaseInfo.insert(QStringLiteral("deviceTransferPerformed"), false);
        writeStructuredTransferResult(releaseInfo);
        preview = releaseHelpDialog->grab().toImage();
        if (preview.isNull() || !preview.save(capture_path))
        {
            return 6;
        }
        releaseHelpDialog->close();
        application.processEvents();
        return 0;
    }
    if (arguments.size() >= 3 && arguments.at(1) == QStringLiteral("--capture-preview"))
    {
        capture_path = QFileInfo(arguments.at(2)).absoluteFilePath();
        QSettings::setDefaultFormat(QSettings::IniFormat); //20261111-1 add
        captureSettings = std::make_unique<QTemporaryDir>();
        if (!captureSettings->isValid()) { return 27; }
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, captureSettings->path());
        window = std::make_unique<MainWindow>();
        window->resize(1353, 708); //20261111-1 change: Compare the same logical client extent as the native reference.
        if (arguments.size() >= 4 && !window->openProjectPath(arguments.at(3), &validation_error)) { return 7; }
        //20261108-7 //20261108-7 change: Capture at the vendor comparison size.
        window->show();
        application.processEvents();
        Q_FOREACH (captureChild, window->findChildren<QWidget *>()) //20261111-1 add: Retain layout evidence separately from screenshots.
        {
            capturePosition = captureChild->mapTo(window.get(), QPoint());
            captureWidget = QJsonObject();
            captureWidget.insert(QStringLiteral("id"), captureChild->objectName());
            captureWidget.insert(QStringLiteral("class"), QString::fromLatin1(captureChild->metaObject()->className()));
            captureWidget.insert(QStringLiteral("rect"), QJsonArray{capturePosition.x(), capturePosition.y(), captureChild->width(), captureChild->height()});
            captureWidget.insert(QStringLiteral("visible"), captureChild->isVisible()); captureWidgets.append(captureWidget);
        }
        captureMetadata.setFileName(capture_path + QStringLiteral(".json"));
        if (captureMetadata.open(QIODevice::WriteOnly)) { captureMetadata.write(QJsonDocument(captureWidgets).toJson()); }
        preview = window->grab().toImage();
        if (preview.isNull() || !preview.save(capture_path))
        {
            return 6;
        }
        return 0;
    }
    window = std::make_unique<MainWindow>();
    window->showMaximized(); //20261111-1 change: Start the editor in the native full-workspace presentation.
    return application.exec();
}
