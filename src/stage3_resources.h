#ifndef STAGE3_RESOURCES_H
#define STAGE3_RESOURCES_H

#include "project_model.h"
#include <QDialog>

class QLabel;
class QTableWidget;

//20261110-8 add: Embedded source bytes determine binding; a display name alone never identifies an image.
struct Stage3PictureUsage
{
    int boundStates = 0;
    int detachedStates = 0;
    int explicitReferences = 0;
};

Stage3PictureUsage stage3PictureUsage(const HmiProject &project, const QString &name);
bool importStage3Picture(const HmiProject &source, const QString &name, const QByteArray &png,
                         HmiProject *draft, QString *error = nullptr);
bool renameStage3Picture(const HmiProject &source, const QString &oldName, const QString &newName,
                         HmiProject *draft, QString *error = nullptr);
bool replaceStage3Picture(const HmiProject &source, const QString &name, const QByteArray &png,
                          HmiProject *draft, QString *error = nullptr);
bool deleteStage3Picture(const HmiProject &source, const QString &name,
                         HmiProject *draft, QString *error = nullptr);

class Stage3PicturesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit Stage3PicturesDialog(const HmiProject &project, QWidget *parent = nullptr);
    HmiProject draftProject() const;
private:
    HmiProject m_draft;
    QTableWidget *m_table;
    QLabel *m_error;
    void rebuild();
    void editPicture(int operation);
};

#endif
