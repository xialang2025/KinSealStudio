#ifndef INDEPENDENT_DOWNLOADER_H
#define INDEPENDENT_DOWNLOADER_H

#include "project_model.h"
#include "device_transfer_engine.h" //20261114-5 add

#include <QString>
#include <QVector> //20261108-11 add
#include <QByteArray>

/******************************************************************************
 * Function Name   : verifyFixedTestPayload
 * Description     : Verify the embedded TEST-only device payload before USB access.
 * Input Parameters: error
 * Return Value    : True when all embedded files match their verified hashes.
 * Date            : 2026.11.08
 ******************************************************************************/
bool verifyFixedTestPayload(QString *error); //20261108-10 add

/******************************************************************************
 * Function Name   : verifySingleTextCompiler
 * Description     : Check fixed and edited text payloads against native raster samples.
 * Input Parameters: fixedProject, error
 * Return Value    : True when supported variants and unsafe-input rejection pass.
 * Date            : 2026.11.08
 ******************************************************************************/
bool verifySingleTextCompiler(const HmiProject &fixedProject, QString *error); //20261108-11 add

/******************************************************************************
 * Function Name   : downloadFixedTestProject
 * Description     : Transfer the fixed TEST program with the system USB driver only.
 * Input Parameters: project, error
 * Return Value    : True when all acknowledged writes and the reset request complete.
 * Date            : 2026.11.08
 ******************************************************************************/
bool downloadFixedTestProject(const HmiProject &project, QString *error); //20261108-10 add: Retained for the original fixed fixture.

/******************************************************************************
 * Function Name   : buildSingleTextPayload
 * Description     : Generate text and position dependent bytes from verified static resources.
 * Input Parameters: project, payloads, error
 * Return Value    : True when all ten USB payload blocks are ready without USB access.
 * Date            : 2026.11.08
 ******************************************************************************/
bool buildSingleTextPayload(const HmiProject &project,
                            QVector<QByteArray> *payloads, QString *error); //20261108-11 add

/******************************************************************************
 * Function Name   : buildProjectPayload
 * Description     : Compile the supported one-page text and bit-button project before USB access.
 * Input Parameters: project, payloads, error
 * Return Value    : True for complete named static, control, and optional macro blocks.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildProjectPayload(const HmiProject &project, QVector<QByteArray> *payloads,
                         QString *error, QJsonObject *issue = nullptr, bool createdDrawing = false, bool reopenedProject = false); //20261128-37 change

bool exportProjectPayload(const HmiProject &project, const QString &outputDirectory,
                          QString *error, bool createdDrawing = false, bool reopenedProject = false); //20261128-37 change
bool downloadProject(const HmiProject &project, QString *error); //20261109-0 add

/******************************************************************************
 * Function Name   : probeConnectedHmi
 * Description     : Read the identity of exactly one attached supported panel.
 * Input Parameters: details
 * Return Value    : True when both replies and interface cleanup succeed.
 * Date            : 2026.11.25
 ******************************************************************************/
bool probeConnectedHmi(QJsonObject *details); //20261125-9 add

/******************************************************************************
 * Function Name   : probeConnectedHmiVersion
 * Description     : Read the raw V5.1 firmware version text from one connected panel.
 * Input Parameters: details
 * Return Value    : True when the dedicated query and USB cleanup succeed.
 * Date            : 2026.11.25
 ******************************************************************************/
bool probeConnectedHmiVersion(QJsonObject *details); //20261125-9 add

/******************************************************************************
 * Function Name   : downloadProjectDetailed
 * Description     : Compile before USB and retain cancellation, commit, reset, and cleanup results.
 * Input Parameters: project, result, error, options
 * Return Value    : True only after the observed transport lifecycle has completed.
 * Date            : 2026.11.14
 ******************************************************************************/
bool downloadProjectDetailed(const HmiProject &project, QJsonObject *result, QString *error,
                             const DeviceTransferOptions &options = DeviceTransferOptions()); //20261114-5 add

/******************************************************************************
 * Function Name   : exportSingleTextPayload
 * Description     : Save the generated USB blocks for offline format verification.
 * Input Parameters: project, outputDirectory, error
 * Return Value    : True when every generated block was written atomically.
 * Date            : 2026.11.08
 ******************************************************************************/
bool exportSingleTextPayload(const HmiProject &project,
                             const QString &outputDirectory,
                             QString *error); //20261108-11 add

/******************************************************************************
 * Function Name   : downloadSingleTextProject
 * Description     : Transfer one independently generated text page over the system USB driver.
 * Input Parameters: project, error
 * Return Value    : True when all acknowledgements and the reset request complete.
 * Date            : 2026.11.08
 ******************************************************************************/
bool downloadSingleTextProject(const HmiProject &project, QString *error); //20261108-11 add

bool writeNativeMessageTable(const HmiProject &project, const QString &path, const QString &session,
                             QString *error); //20261128-24 change
//20261128-31 add: Export only independently identified control-record fields for offline comparison.
bool exportNativeValueRecordFields(const HmiProject &project, const QString &destination, QString *error);
bool exportNativeValueDisplayParts(const HmiProject &project, const QString &destination,
                                   QString *error); //20261128-25 add

#endif
