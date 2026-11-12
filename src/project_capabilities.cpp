#include "project_capabilities.h"
#include "project_model.h"
#include "native_project_codec.h"
#include "device_address_compiler.h" //20261110-5 add

#include <QJsonArray>

/******************************************************************************
 * Function Name   : describeProjectCapabilities
 * Description     : Report structural eligibility without claiming export, USB acceptance or firmware execution.
 * Input Parameters: project
 * Return Value    : Model, native source and AMT backend gates with their independent limitations.
 * Date            : 2026.11.10
 ******************************************************************************/
QJsonObject describeProjectCapabilities(const HmiProject &project) //20261110-5 add
{
    QJsonObject result;
    QJsonObject target;
    QJsonObject model;
    QJsonObject native;
    QJsonObject device;
    QJsonArray limits;
    QString modelError;
    QString backendError;
    QString defaultsError;
    QString deviceError;
    DeviceAddressCompiler addressCompiler;
    QByteArray scan;
    bool deviceAllowed;
    bool modelAllowed;
    bool backendAllowed;
    bool targetAllowed;
    bool defaultsAllowed;
    bool nativeObjectsAllowed;
    HmiObject object;

    modelAllowed = project.validateModel(&modelError);
    backendAllowed = modelAllowed && project.validateSinglePageBackend(&backendError);
    targetAllowed = project.targetModel == QStringLiteral("AMT070W124-L800X480R") &&
                    project.pageWidth == 800 && project.pageHeight == 480;
    defaultsAllowed = project.nativeSourceSnapshot.isEmpty() ||
                      nativeProjectUsesSupportedDeviceDefaults(project.nativeSourceSnapshot, &defaultsError);
    nativeObjectsAllowed = true;
    Q_FOREACH (object, project.objects)
    {
        if (object.type != QStringLiteral("text") && object.type != QStringLiteral("switch"))
        {
            nativeObjectsAllowed = false;
        }
    }
    if (modelAllowed && backendAllowed && !targetAllowed)
    {
        backendError = QStringLiteral("原厂源与设备后端目前只核定AMT070W124-L800X480R、800×480目标。");
    }
    if (modelAllowed && backendAllowed && !nativeObjectsAllowed)
    {
        backendError = QStringLiteral("所含对象类型尚未恢复原厂源或设备编码。");
    }
    deviceAllowed = backendAllowed && targetAllowed && nativeObjectsAllowed;
    deviceError = !modelAllowed ? modelError : backendError;
    if (deviceAllowed && !defaultsAllowed)
    {
        deviceAllowed = false;
        deviceError = defaultsError;
    }
    if (deviceAllowed && (project.objects.isEmpty() || project.objects.size() > 100 ||
                          project.languages.isEmpty() || project.languages.size() > 8))
    {
        deviceAllowed = false;
        deviceError = QStringLiteral("设备后端需要1至100个控件、1至8种语言。");
    }
    if (deviceAllowed && !addressCompiler.prepare(project, &scan, &deviceError))
    {
        deviceAllowed = false; // Pure offline preparation also identifies known unsupported connection routes.
    }
    target.insert(QStringLiteral("model"), project.targetModel);
    target.insert(QStringLiteral("knownBackend"), targetAllowed ? QStringLiteral("AMT070W124-L800X480R-single-page") : QString());
    target.insert(QStringLiteral("firmwareVersionConfirmed"), false);
    target.insert(QStringLiteral("sourceVersion"), 122);
    result.insert(QStringLiteral("target"), target);
    result.insert(QStringLiteral("pageCount"), project.pageCount());
    result.insert(QStringLiteral("activePageId"), project.activePageId);
    result.insert(QStringLiteral("startupPageId"), project.startupPageId);
    result.insert(QStringLiteral("startupLanguage"), project.startupLanguage);
    model.insert(QStringLiteral("allowed"), modelAllowed);
    model.insert(QStringLiteral("reason"), modelError);
    model.insert(QStringLiteral("format"), QStringLiteral("KSP-schema1-to9-and11")); //20261112-0 change
    result.insert(QStringLiteral("modelPersistence"), model);
    native.insert(QStringLiteral("status"), backendAllowed && targetAllowed && nativeObjectsAllowed ? QStringLiteral("candidate") : QStringLiteral("blocked"));
    native.insert(QStringLiteral("reason"), !modelAllowed ? modelError : backendError);
    native.insert(QStringLiteral("actualExportRequired"), true);
    native.insert(QStringLiteral("opaqueBodyEditable"), false);
    result.insert(QStringLiteral("nativeSource"), native);
    device.insert(QStringLiteral("status"), deviceAllowed ? QStringLiteral("candidate") : QStringLiteral("blocked"));
    device.insert(QStringLiteral("reason"), deviceError);
    device.insert(QStringLiteral("offlineCompileRequired"), true);
    device.insert(QStringLiteral("usbAccessed"), false);
    result.insert(QStringLiteral("deviceCompile"), device);
    limits.append(QStringLiteral("Eligibility does not establish all field semantics, original rendering or physical touch acceptance."));
    limits.append(QStringLiteral("Multi-page device records and active new configuration nodes remain unsupported."));
    limits.append(QStringLiteral("Object/language limits describe this Lite backend, not original firmware maxima."));
    result.insert(QStringLiteral("limits"), limits);
    return result;
}
