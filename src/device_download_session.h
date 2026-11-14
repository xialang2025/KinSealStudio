#ifndef DEVICE_DOWNLOAD_SESSION_H
#define DEVICE_DOWNLOAD_SESSION_H

#include <QByteArray>
#include <QJsonObject>
#include <QObject>

class QProcess;
class QTemporaryDir;

//20261114-5 add: This owner keeps the immutable input and cancellation marker alive until its hidden worker exits.
class DeviceDownloadSession : public QObject
{
    Q_OBJECT
public:
    explicit DeviceDownloadSession(QObject *parent = nullptr);
    ~DeviceDownloadSession() override;

    bool start(const QByteArray &snapshot, const QString &workerProgram = QString());
    bool isRunning() const;
    bool requestCancel();
    QString tempDirectory() const;
    QJsonObject result() const;
    QString lastError() const;

signals:
    void progress(const QJsonObject &event);
    void finished(bool success, const QJsonObject &result);

private:
    void consumeStdout(QProcess *process, bool finalDrain);
    void consumeLine(const QByteArray &line);
    void mergeProgress(const QJsonObject &event);
    void mergeFinal(const QJsonObject &result);
    void rememberSessionError(const QString &kind, const QString &message);
    void finishProcess(QProcess *process, bool normalExit, int exitCode);

    QProcess *m_process = nullptr;
    QTemporaryDir *m_temp = nullptr;
    QString m_cancelPath;
    QString m_lastError;
    QByteArray m_lineBuffer;
    QByteArray m_stderrTail;
    qint64 m_stdoutBytes = 0;
    bool m_running = false;
    bool m_cancelRequested = false;
    bool m_commitStarted = false;
    bool m_finalSeen = false;
    bool m_streamInvalid = false;
    bool m_outputLimitExceeded = false;
    QJsonObject m_result;
    QJsonObject m_finalResult;
    QJsonObject m_sessionError;
};

#endif
