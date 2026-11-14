#ifndef MACRO_MANAGER_DIALOG_H
#define MACRO_MANAGER_DIALOG_H

#include "device_macro_compiler.h"
#include "project_model.h"
#include <QDialog>
#include <memory>

class QCloseEvent;
class QLabel;
class QListWidget;
class QPlainTextEdit;
class QPushButton;
class QThread;
struct MacroCompileJob;

//20261114-7 add: Ordinary source drafts and compiler results never mutate the live editor until acceptance.
class MacroManagerDialog : public QDialog
{
    Q_OBJECT
    friend class MacroUiTests;
public:
    explicit MacroManagerDialog(const HmiProject &project, QWidget *parent = nullptr);
    ~MacroManagerDialog() override;
    const HmiProject &draftProject() const;
    bool selectMacro(const QString &name, QString *error = nullptr);
    bool applySource(QString *error = nullptr);
    bool createSource(const QString &name, const QString &source, QString *error = nullptr);
    bool renameSource(const QString &name, const QString &newName, QString *error = nullptr);
    bool removeSource(const QString &name, QString *error = nullptr);
    bool importSourceFile(const QString &path, const QString &name, bool replaceExisting, QString *error = nullptr);
    bool exportSourceFile(const QString &path, const QString &name, QString *error = nullptr);
    bool insertLibraryExample(const QString &name, QString *error = nullptr); //20261114-7 add
    bool compiling() const;
    QJsonObject compilationReport() const;
    void startCompilation(const QString &sdk = QString(), const DeviceMacroCompileOptions &options = {});
    void cancelCompilation();
public slots:
    void accept() override;
    void reject() override;
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    HmiProject m_draft;
    QString m_currentName;
    QListWidget *m_list;
    QPlainTextEdit *m_source;
    QPlainTextEdit *m_output;
    QLabel *m_identity;
    QLabel *m_error;
    QPushButton *m_compile;
    QPushButton *m_cancelCompile;
    QList<QWidget *> m_editingWidgets;
    std::shared_ptr<MacroCompileJob> m_job;
    QThread *m_thread = nullptr;
    QJsonObject m_report;
    bool m_rebuilding = false;
    bool m_closeWhenFinished = false;
    void rebuild(const QString &selected);
    void showError(const QString &error);
    void setCompiling(bool running);
    void promptNew();
    void promptCopy();
    void promptRename();
    void promptDelete();
    void promptImport();
    void promptExport();
    void findSource();
};

#endif
