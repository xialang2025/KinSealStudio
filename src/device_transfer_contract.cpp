#include "device_transfer_contract.h"
#include "device_transfer_plan.h"

#include <QCryptographicHash>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

/******************************************************************************
 * Function Name   : transferContractWouldReplaceInput
 * Description     : Preserve the source when two names resolve to one Windows file.
 * Input Parameters: input, output
 * Return Value    : True for an alias or an existing identity that cannot be safely distinguished.
 * Date            : 2026.11.14
 ******************************************************************************/
bool transferContractWouldReplaceInput(const QString &input, const QString &output) //20261114-5 add
{
    QFileInfo source(input);
    QFileInfo destination(output);
    Qt::CaseSensitivity sensitivity = Qt::CaseSensitive;
    QString sourcePath;
    QString destinationPath;
#ifdef Q_OS_WIN
    HANDLE sourceHandle;
    HANDLE destinationHandle;
    BY_HANDLE_FILE_INFORMATION sourceIdentity = {};
    BY_HANDLE_FILE_INFORMATION destinationIdentity = {};
    bool identified;

    sensitivity = Qt::CaseInsensitive;
#endif
    sourcePath = source.canonicalFilePath().isEmpty() ? source.absoluteFilePath() : source.canonicalFilePath();
    destinationPath = destination.canonicalFilePath().isEmpty() ? destination.absoluteFilePath() : destination.canonicalFilePath();
    if (QDir::cleanPath(sourcePath).compare(QDir::cleanPath(destinationPath), sensitivity) == 0)
    {
        return true;
    }
#ifdef Q_OS_WIN
    if (source.exists() && destination.exists())
    {
        sourceHandle = CreateFileW(reinterpret_cast<LPCWSTR>(source.absoluteFilePath().utf16()), FILE_READ_ATTRIBUTES,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr);
        destinationHandle = CreateFileW(reinterpret_cast<LPCWSTR>(destination.absoluteFilePath().utf16()), FILE_READ_ATTRIBUTES,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr);
        identified = sourceHandle != INVALID_HANDLE_VALUE && destinationHandle != INVALID_HANDLE_VALUE &&
                     GetFileInformationByHandle(sourceHandle, &sourceIdentity) != 0 &&
                     GetFileInformationByHandle(destinationHandle, &destinationIdentity) != 0;
        if (sourceHandle != INVALID_HANDLE_VALUE) { CloseHandle(sourceHandle); }
        if (destinationHandle != INVALID_HANDLE_VALUE) { CloseHandle(destinationHandle); }
        if (!identified) { return true; }
        return sourceIdentity.dwVolumeSerialNumber == destinationIdentity.dwVolumeSerialNumber &&
               sourceIdentity.nFileIndexHigh == destinationIdentity.nFileIndexHigh &&
               sourceIdentity.nFileIndexLow == destinationIdentity.nFileIndexLow;
    }
#endif
    return false;
}

/******************************************************************************
 * Function Name   : transferSha
 * Description     : Hash the actual wire bytes, including the snapshot outer length.
 * Input Parameters: bytes
 * Return Value    : Lowercase SHA256.
 * Date            : 2026.11.14
 ******************************************************************************/
static QString transferSha(const QByteArray &bytes) //20261114-5 add
{
    return QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
}

/******************************************************************************
 * Function Name   : contractCommand
 * Description     : Describe one command write and its separately checked reply.
 * Input Parameters: command, options, name, replyLength, replyByte
 * Return Value    : One side-effect-free command description.
 * Date            : 2026.11.14
 ******************************************************************************/
static QJsonObject contractCommand(u8 command, const QByteArray &options,
                                   const QString &name, int replyLength,
                                   int replyByte) //20261114-5 add
{
    QByteArray frame(32, char(0));
    QJsonObject row;
    QJsonObject reply;

    frame.replace(0, 17, QByteArray("KinSealStudioKiss", 17));
    frame[17] = char(command);
    frame.replace(18, options.size(), options);
    row.insert(QStringLiteral("kind"), QStringLiteral("command"));
    row.insert(QStringLiteral("name"), name);
    row.insert(QStringLiteral("commandHex"), QString::number(command, 16).rightJustified(2, QLatin1Char('0')));
    row.insert(QStringLiteral("optionsHex"), QString::fromLatin1(options.toHex()));
    row.insert(QStringLiteral("wireBytes"), frame.size());
    row.insert(QStringLiteral("wireSha256"), transferSha(frame));
    row.insert(QStringLiteral("frameHex"), QString::fromLatin1(frame.toHex()));
    reply.insert(QStringLiteral("length"), replyLength);
    reply.insert(QStringLiteral("firstByte"), replyByte < 0 ? QString() : QString::number(replyByte, 16).rightJustified(2, QLatin1Char('0')));
    reply.insert(QStringLiteral("readRequestBytes"), replyLength == 24 ? 32 : replyLength > 0 ? 4 : 0);
    row.insert(QStringLiteral("expectedReply"), reply);
    return row;
}

/******************************************************************************
 * Function Name   : buildDeviceTransferContract
 * Description     : Freeze the supported wire order and distinguish transport from screen evidence.
 * Input Parameters: payloads, identity, contract, error, startOptions
 * Return Value    : True without USB access; failure leaves the previous contract intact.
 * Date            : 2026.11.14
 ******************************************************************************/
bool buildDeviceTransferContract(const QVector<QByteArray> &payloads,
                                 const QJsonObject &identity, QJsonObject *contract,
                                 QString *error, const QByteArray &startOptions) //20261114-5 add
{
    QVector<HmiDownloadStep> steps;
    HmiDownloadStep step;
    QJsonArray operations;
    QJsonArray segments;
    QJsonObject prepared;
    QJsonObject row;
    QJsonObject segment;
    int index;

    if (contract == nullptr ||
        identity.value(QStringLiteral("targetModel")).toString() != QStringLiteral("AMT070W124-L800X480R") ||
        (startOptions != QByteArray::fromHex("000000") && startOptions != QByteArray::fromHex("010101")))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("传输契约的目标或选项不在已验证范围。");
        }
        return false;
    }
    if (!buildDeviceTransferPlan(payloads, &steps, error))
    {
        return false;
    }
    operations.append(contractCommand(0x31, QByteArray(1, char(2)), QStringLiteral("identify"), 24, 0x4c));
    operations.append(contractCommand(0x53, QByteArray(1, char(0)), QStringLiteral("parameters"), 24, 0x00));
    operations.append(contractCommand(0x34, startOptions, QStringLiteral("begin"), 1, 0x4f));
    operations.append(contractCommand(0x35, QByteArray(), QStringLiteral("begin-status"), 1, 0x4f));
    Q_FOREACH (step, steps)
    {
        operations.append(contractCommand(step.command, step.options, step.name + QStringLiteral("/command"), 1, 0x4f));
        if (!step.bytes.isEmpty())
        {
            row = QJsonObject();
            row.insert(QStringLiteral("kind"), QStringLiteral("payload"));
            row.insert(QStringLiteral("name"), step.name);
            row.insert(QStringLiteral("wireBytes"), step.bytes.size());
            row.insert(QStringLiteral("wireSha256"), transferSha(step.bytes));
            row.insert(QStringLiteral("expectedReply"), QStringLiteral("none-until-following-status"));
            operations.append(row);
            segment = row;
            segment.insert(QStringLiteral("snapshotOuterLengthBE32"), step.command == 0x1e);
            segments.append(segment);
        }
        operations.append(contractCommand(0x35, QByteArray(), step.name + QStringLiteral("/status"), 1, 0x4f));
    }
    operations.append(contractCommand(0x1b, QByteArray(), QStringLiteral("commit"), 1, 0x4f));
    operations.append(contractCommand(0x1a, QByteArray(), QStringLiteral("reset"), 0, -1));
    row = QJsonObject();
    row.insert(QStringLiteral("kind"), QStringLiteral("post-reset-read"));
    row.insert(QStringLiteral("name"), QStringLiteral("reset-read-completion"));
    row.insert(QStringLiteral("readRequestBytes"), 4);
    row.insert(QStringLiteral("observedReturn"), -5);
    row.insert(QStringLiteral("ackRequired"), false);
    operations.append(row);
    for (index = 0; index < operations.size(); ++index)
    {
        row = operations.at(index).toObject();
        row.insert(QStringLiteral("sequence"), index + 1);
        row.insert(QStringLiteral("nextSequence"), index + 1 < operations.size() ? index + 2 : 0);
        operations[index] = row;
    }
    prepared.insert(QStringLiteral("format"), QStringLiteral("KinSealStage6-transfer-contract-v1"));
    prepared.insert(QStringLiteral("identity"), identity);
    prepared.insert(QStringLiteral("payloadCount"), payloads.size());
    prepared.insert(QStringLiteral("startOptionsHex"), QString::fromLatin1(startOptions.toHex()));
    prepared.insert(QStringLiteral("timeoutMs"), 10000);
    prepared.insert(QStringLiteral("usbAccessed"), false);
    prepared.insert(QStringLiteral("deviceAcknowledged"), false);
    prepared.insert(QStringLiteral("screenDisplayAndTouchConfirmed"), false);
    prepared.insert(QStringLiteral("segments"), segments);
    prepared.insert(QStringLiteral("operations"), operations);
    *contract = prepared;
    return true;
}
