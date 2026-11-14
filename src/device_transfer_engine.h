#ifndef DEVICE_TRANSFER_ENGINE_H
#define DEVICE_TRANSFER_ENGINE_H

#include "device_transfer_plan.h"

#include <QJsonObject>
#include <QStringList>
#include <functional>

//20261114-5 add: Callbacks own their native handle; the engine never loads a driver or discovers hardware.
struct DeviceTransferIo
{
    std::function<bool(QString *)> open;
    std::function<int(int)> claim;
    std::function<int(int, const QByteArray &, int)> write;
    std::function<int(int, char *, int, int)> read;
    std::function<int(int)> release;
    std::function<int()> close;
};

//20261114-5 add: Commands and short replies retain raw bytes; payload events retain only their digest and length.
struct DeviceTransferEvent
{
    int sequence = 0;
    QString phase;
    QString operation;
    int command = -1;
    QString segment;
    int requested = 0;
    int returned = 0;
    int expectedReplyLength = 0;
    bool accepted = false;
    QByteArray bytes;
    QByteArray sha256;
    QString message;
};

//20261114-5 add: The first failure remains immutable even if later release or close also fails.
struct DeviceTransferFailure
{
    bool present = false;
    QString kind;
    QString phase;
    int command = -1;
    QString segment;
    int requested = 0;
    int returned = 0;
    int expectedReplyLength = 0;
    QString message;
};

//20261114-5 add: These historical milestones do not imply that a device is still open or that firmware ran correctly.
struct DeviceTransferResult
{
    bool success = false;
    bool planValidated = false;
    bool opened = false;
    bool interfaceClaimed = false;
    bool releaseAttempted = false;
    bool interfaceReleased = false;
    bool closeAttempted = false;
    bool closed = false;
    bool cancelled = false;
    bool anyWriteAttempted = false;
    bool startAccepted = false;
    bool transferAccepted = false;
    bool commitWriteAttempted = false;
    bool commitCommandWritten = false;
    bool commitAccepted = false;
    bool resetWritten = false;
    bool postResetReadReturned = false;
    bool postResetReadCompleted = false;
    bool screenDisplayAndTouchConfirmed = false;
    int completedSteps = 0;
    int plannedSteps = 0;
    int payloadBlocks = 0;
    int postResetReadResult = 0;
    int releaseResult = 0;
    int closeResult = 0;
    QString outcome;
    QString message;
    QStringList writtenSegments;
    QStringList acknowledgedSegments;
    QByteArray modelReply;
    QByteArray parametersReply;
    QByteArray startOptions;
    int timeoutMs = 0;
    DeviceTransferFailure firstError;
    QVector<DeviceTransferEvent> events;
};

//20261114-5 add: Cancellation is sampled at explicit boundaries; event observers must not pump application events.
struct DeviceTransferOptions
{
    QByteArray startOptions = QByteArray::fromHex("000000");
    int timeoutMs = 10000;
    std::function<bool()> cancelRequested;
    std::function<void(const DeviceTransferEvent &)> onEvent;
};

/******************************************************************************
 * Function Name   : runDeviceTransfer
 * Description     : Execute a fully validated transfer through injected bulk I/O and retain every completion milestone.
 * Input Parameters: payloads, io, options
 * Return Value    : Structured transfer result; screen display and touch are never accepted here.
 * Date            : 2026.11.14
 ******************************************************************************/
DeviceTransferResult runDeviceTransfer(const QVector<QByteArray> &payloads,
                                      const DeviceTransferIo &io,
                                      const DeviceTransferOptions &options = DeviceTransferOptions()); //20261114-5 add

/******************************************************************************
 * Function Name   : deviceTransferEventToJson
 * Description     : Serialize one bounded event without exposing full payload contents.
 * Input Parameters: event
 * Return Value    : JSON event with command/reply bytes or payload digest.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject deviceTransferEventToJson(const DeviceTransferEvent &event); //20261114-5 add

/******************************************************************************
 * Function Name   : deviceTransferResultToJson
 * Description     : Serialize milestones, first failure, segment history, and bounded events for CLI or UI consumers.
 * Input Parameters: result
 * Return Value    : JSON result with screenDisplayAndTouchConfirmed fixed to false.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonObject deviceTransferResultToJson(const DeviceTransferResult &result); //20261114-5 add

#endif
