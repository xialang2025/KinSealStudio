#ifndef STAGE2_PROJECT_DIALOGS_H
#define STAGE2_PROJECT_DIALOGS_H

#include "project_model.h"
#include <QDialog>

class QComboBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;
class QStackedWidget;

bool editStage2NewNormalPage(QWidget *parent, HmiProject &project); //20261122-9 add
bool editStage2NormalPageProperties(QWidget *parent, HmiProject &project, const QString &pageId); //20261123-6 add
bool copyStage2Page(QWidget *parent, HmiProject &project, const QString &pageId, bool popup); //20261123-13 change
bool editStage2Window(QWidget *parent, HmiProject &project, const QString &pageId); //20261123-12 add
bool chooseStage2PageBitmap(QWidget *parent, const QMap<QString, QByteArray> &pictures, QString &name); //20261123-24 add

//20261110-7 add: Callers publish these detached values only after QDialog::Accepted.
class Stage2NewProjectDialog : public QDialog
{
    Q_OBJECT
public:
    explicit Stage2NewProjectDialog(const HmiProject &seed, const QString &suggestedPath,
                                    QWidget *parent = nullptr);
    HmiProject draftProject() const;
    QString projectPath() const;
protected:
    void accept() override;
private:
    HmiProject m_working;
    QLineEdit *m_name;
    QLineEdit *m_path;
    QLineEdit *m_pageName;
    QComboBox *m_target; //20261122-3 add
    QComboBox *m_startupLanguage;
    QLabel *m_error;
    QStackedWidget *m_steps;
    QPushButton *m_back;
    QPushButton *m_next;
    QPushButton *m_finish;
    bool collectDraft(HmiProject *candidate, bool includePage, QString *error) const;
    void updateStep(int step);
};

class Stage2ProjectDialog : public QDialog
{
    Q_OBJECT
public:
    explicit Stage2ProjectDialog(const HmiProject &project, QWidget *parent = nullptr);
    HmiProject draftProject() const;
    bool addPageDraft(const HmiPage &page, QString *error = nullptr);
    bool renamePageDraft(const QString &id, const QString &name, QString *error = nullptr);
    bool copyPageDraft(const QString &id, const QString &name, QString *newId = nullptr,
                       QString *error = nullptr);
    bool removePageDraft(const QString &id, QString *error = nullptr);
    bool selectPageDraft(const QString &id, QString *error = nullptr);
    bool setStartupDraft(const QString &id, const QString &language, QString *error = nullptr);
    bool setEditorLanguageDraft(const QString &language, QString *error = nullptr);
protected:
    void accept() override;
private:
    HmiProject m_working;
    QLineEdit *m_projectName;
    QComboBox *m_target; //20261122-3 add
    QLineEdit *m_pageName;
    QListWidget *m_pages;
    QComboBox *m_pageKind;
    QSpinBox *m_width;
    QSpinBox *m_height;
    QComboBox *m_editorPage;
    QComboBox *m_startupPage;
    QComboBox *m_editorLanguage;
    QComboBox *m_startupLanguage;
    QLabel *m_error;
    QDialogButtonBox *m_buttons;
    void refresh(const QString &selectedId = QString());
    void showSelectedPage();
    QString selectedPageId() const;
    void addFromFields();
    void renameFromFields();
    void copyFromFields();
    void removeSelected();
    void reportError(const QString &error);
};

#endif
