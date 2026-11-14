#ifndef LOCAL_HELP_H
#define LOCAL_HELP_H

#include <QJsonObject>
#include <QObject>
#include <QPointer>
#include <QUrl>

class QDialog;
class QLabel;
class QTextBrowser;
class QWidget;

QJsonObject helpCatalogReport(const QString &root = QString()); //20261114-10 add

//20261114-10 add: The help store has no device, browser-process, or network backend.
class LocalHelpStore
{
public:
    explicit LocalHelpStore(const QString &root = QString());
    QString rootPath() const;
    QString topicForCommand(const QString &commandId) const;
    QString topicForContext(const QWidget *context) const;
    bool readTopic(const QString &topic, QString *html, QUrl *url, QString *error) const;
    bool readPage(const QUrl &target, const QUrl &current, QString *html, QUrl *url, QString *error) const;
    QByteArray readResource(const QUrl &target, const QUrl &current) const;

private:
    QString m_root;
    QJsonObject m_map;
    QString m_mapError;
    bool resolveFile(const QUrl &target, const QUrl &current, bool page, QString *path,
                     QUrl *resolved, QString *error) const;
};

class LocalHelpController : public QObject //20261114-10 add
{
public:
    explicit LocalHelpController(QWidget *owner, const QString &root = QString());
    ~LocalHelpController() override;
    void installContextHelp();
    void showTopic(const QString &topic = QStringLiteral("start"), QWidget *context = nullptr);
    void showCommand(const QString &commandId, QWidget *context = nullptr);
    QString contextTopic(const QWidget *context) const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QPointer<QWidget> m_owner;
    QPointer<QDialog> m_dialog;
    QTextBrowser *m_browser = nullptr;
    QLabel *m_status = nullptr;
    LocalHelpStore m_store;
    QUrl m_current;
    bool m_installed = false;
    void ensureDialog(QWidget *context);
    void showPage(const QString &html, const QUrl &url);
    void showError(const QString &error);
    void navigate(const QUrl &target);
};

#endif
