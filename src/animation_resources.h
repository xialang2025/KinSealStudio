#ifndef ANIMATION_RESOURCES_H
#define ANIMATION_RESOURCES_H

#include "project_model.h"
#include <QDialog>

class QListWidget;
class QLabel;

//20261112-0 add: Animation identity includes original bytes; same-name detached copies remain independent.
int animationUsage(const HmiProject &project, const QString &name);
bool importAnimation(const HmiProject &source, const QString &name, const QByteArray &gif, HmiProject *draft, QString *error = nullptr);
bool renameAnimation(const HmiProject &source, const QString &name, const QString &newName, HmiProject *draft, QString *error = nullptr);
bool replaceAnimation(const HmiProject &source, const QString &name, const QByteArray &gif, HmiProject *draft, QString *error = nullptr);
bool deleteAnimation(const HmiProject &source, const QString &name, HmiProject *draft, QString *error = nullptr);
bool mergeAnimation(HmiProject *project, const QString &name, const QByteArray &gif, QString *resolvedName, QString *error = nullptr);

class AnimationGalleryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AnimationGalleryDialog(const HmiProject &project, const QString &selection, QWidget *parent = nullptr);
    HmiProject draftProject() const;
    QString selectedAnimation() const;
    bool importFiles(const QStringList &paths, QString *error = nullptr);
protected:
    void accept() override;
private:
    void rebuild(const QString &selection);
    void refreshMetadata();
    HmiProject m_draft;
    QListWidget *m_list;
    QLabel *m_metadata;
};

#endif
