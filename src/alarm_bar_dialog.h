#ifndef ALARM_BAR_DIALOG_H
#define ALARM_BAR_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QPainter;
class QLabel;
class HmiPermissionEditor;
void paintAlarmBar(QPainter *painter, const QRectF &rect, const AlarmBarProperties &properties);
//20261113-2 add: The preview is an editor sample, independent of alarm/PLC runtime.
class AlarmBarDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AlarmBarDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    AlarmBarProperties m_draft;
    bool m_accepted = false;
    QLabel *m_preview = nullptr;
    HmiPermissionEditor *m_visibility;
    QWidget *basicPage();
    void refresh();
};
#endif
