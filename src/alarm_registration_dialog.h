#ifndef ALARM_REGISTRATION_DIALOG_H
#define ALARM_REGISTRATION_DIALOG_H
#include "project_model.h"
#include "alarm_registration_properties.h"
#include <QDialog>
class QComboBox;
class QLineEdit;
class QTableWidget;
class QSpinBox;
//20261113-2 add: Registration edits publish once and preserve row/language selection as editor state only.
class AlarmRegistrationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AlarmRegistrationDialog(const HmiConfigurationNode &node, const HmiProject &project, int family, QWidget *parent = nullptr);
    HmiConfigurationNode node() const;
protected:
    void accept() override;
private:
    HmiConfigurationNode m_original;
    HmiProject m_project;
    AlarmRegistrationProperties m_draft;
    bool m_initialValid = true;
    bool m_loading = false;
    QLineEdit *m_read;
    QComboBox *m_mode;
    QComboBox *m_count;
    QSpinBox *m_scan;
    QTableWidget *m_table;
    QComboBox *m_language;
    QLineEdit *m_message;
    QComboBox *m_dataType;
    QLineEdit *m_limit;
    QLineEdit *m_rowAddress;
    QLineEdit *m_rowType;
    QComboBox *m_jump;
    QComboBox *m_back;
    QString rowLabel(int row) const;
    void rebuild();
    void selectRow(int row);
    void saveRow();
    void changeMode(int mode);
};
#endif
