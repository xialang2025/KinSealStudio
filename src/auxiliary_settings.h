#ifndef AUXILIARY_SETTINGS_H
#define AUXILIARY_SETTINGS_H

#include <QString>

class QWidget;

//20261125-82 add: User preferences apply across projects, not to the downloadable HMI format.
struct AuxiliarySettings
{
    bool autoSave = false;
    int minutes = 3;
    bool backup = false;
    bool timedMode = false;
    bool customPath = false;
    int copies = 5;
    QString directory;

    static AuxiliarySettings load();
    bool store() const;
    QString backUpSavedFile(const QString &fileName) const;
};

bool editAuxiliarySettings(QWidget *parent, const QString &projectFile, AuxiliarySettings *settings);
#endif
