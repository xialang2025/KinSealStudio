#ifndef DATA_GROUP_DIALOG_H
#define DATA_GROUP_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QGridLayout;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QLabel;
class QPushButton;
class QPainter;
class HmiPermissionEditor;
void paintDataGroup(QPainter *painter, const QRectF &rect, const DataGroupProperties &properties);

//20261113-1 add: Sampling configuration remains separate from any unverified live acquisition buffer.
class DataGroupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DataGroupDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    DataGroupProperties m_draft;
    QLabel *m_preview = nullptr;
    HmiPermissionEditor *m_visibility;
    bool m_accepted = false;
    QWidget *channelPage();
    QWidget *displayPage();
    QWidget *axisPage(bool y);
    QLineEdit *textField(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value, int addressRole = 0);
    QComboBox *choice(QGridLayout *layout, int row, const QString &label, const QString &name, const QStringList &items, int *value, int offset = 0);
    QCheckBox *toggle(QGridLayout *layout, int row, const QString &label, const QString &name, bool *value);
    QSpinBox *integer(QGridLayout *layout, int row, const QString &label, const QString &name, int *value, int minimum, int maximum);
    QPushButton *color(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value);
    void refresh();
};
#endif
