#ifndef STAGE2_SYSTEM_SETTINGS_H
#define STAGE2_SYSTEM_SETTINGS_H

#include "project_model.h"
#include <QDialog>

class QCheckBox;
class QDialogButtonBox;
class QLabel;
class QSpinBox;

//20261110-7 add: These five values are an observed UI/persistence slice, not a native device contract.
struct Stage2SystemSettings
{
    int screensaverMinutes = 1;
    int flickerTenths = 10;
    bool touchSound = true;
    int defaultUserLevel = 1;
    bool hexWordAddress = false;

    QJsonObject toJson() const;
};

bool validateStage2SystemSettingsProperties(const QJsonObject &properties, QString *error = nullptr);
bool readStage2SystemSettings(const HmiProject &project, Stage2SystemSettings *settings,
                              QString *error = nullptr);
bool applyStage2SystemSettings(const HmiProject &source, const Stage2SystemSettings &settings,
                               HmiProject *draft, QString *error = nullptr);

class Stage2SystemSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit Stage2SystemSettingsDialog(const HmiProject &project, QWidget *parent = nullptr);
    HmiProject draftProject() const;
protected:
    void accept() override;
private:
    HmiProject m_working;
    bool m_readable;
    QSpinBox *m_screensaver;
    QSpinBox *m_flicker;
    QCheckBox *m_touchSound;
    QSpinBox *m_userLevel;
    QCheckBox *m_hexAddress;
    QLabel *m_error;
    QDialogButtonBox *m_buttons;
};

#endif
