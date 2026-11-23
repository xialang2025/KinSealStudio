#ifndef STAGE2_KEYBOARD_DIALOGS_H
#define STAGE2_KEYBOARD_DIALOGS_H

#include <QString>

class QWidget;
struct HmiProject;

bool addStage2Keyboards(QWidget *parent, HmiProject &project);
bool editStage2KeyboardProperties(QWidget *parent, HmiProject &project, const QString &id);

#endif
