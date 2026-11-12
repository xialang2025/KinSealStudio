#ifndef DROPDOWN_DIALOG_H
#define DROPDOWN_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QPainter;
class QSpinBox;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QTableWidget;
class QCheckBox;
class QRadioButton;
class QPushButton;
class HmiPermissionEditor;

void paintDropdown(QPainter *painter, const QRectF &rect, const DropdownProperties &properties);

//20261112-0 add: Three native pages edit a detached table and macro pool until full-model acceptance.
class DropdownDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DropdownDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
    QMap<QString, QString> macros() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    DropdownProperties m_draft;
    QSpinBox *m_count;
    QSpinBox *m_fontSize;
    QComboBox *m_alignment;
    QComboBox *m_direction;
    QComboBox *m_type;
    QLineEdit *m_address;
    QPushButton *m_background;
    QPushButton *m_textColor;
    QGroupBox *m_macro;
    QComboBox *m_macroList;
    QTableWidget *m_table;
    HmiPermissionEditor *m_visibility;
    HmiPermissionEditor *m_touch;
    QGroupBox *m_notice;
    QCheckBox *m_noticeBit;
    QCheckBox *m_noticeWord;
    QLineEdit *m_noticeBitAddress;
    QLineEdit *m_noticeWordAddress;
    QComboBox *m_noticeWordType;
    QLineEdit *m_noticeWordValue;
    QRadioButton *m_noticeBitOn;
    QWidget *createGeneral();
    QWidget *createPermissions();
    void resizeRows(int count);
    void loadRows();
    void editMacro();
    void chooseColor(bool background);
};
#endif
