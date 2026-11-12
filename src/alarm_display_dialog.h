#ifndef ALARM_DISPLAY_DIALOG_H
#define ALARM_DISPLAY_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QPainter;
class QGridLayout;
class QComboBox;
class QLineEdit;
class QLabel;
class QPushButton;
class HmiPermissionEditor;
void paintAlarmDisplay(QPainter *painter, const QRectF &rect, const AlarmDisplayProperties &properties, const QString &language);
//20261113-2 add: Table samples are editor illustrations; registration and firmware runtime remain separate.
class AlarmDisplayDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AlarmDisplayDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    AlarmDisplayProperties m_draft;
    QString m_language;
    bool m_accepted = false;
    QLabel *m_preview = nullptr;
    QComboBox *m_languages;
    QLineEdit *m_message;
    QLineEdit *m_date;
    QLineEdit *m_time;
    HmiPermissionEditor *m_visibility;
    QWidget *basicPage();
    QString *color(int index);
    void addColor(QGridLayout *layout, int row, const QString &label, int index);
    void changeLanguage(const QString &language);
    void saveCaptions();
    void refresh();
};
#endif
