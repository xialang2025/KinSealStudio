#ifndef WHEEL_DIALOG_H
#define WHEEL_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QPainter;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;
class HmiPermissionEditor;
void paintWheel(QPainter *painter, const QRectF &rect, const WheelProperties &properties);

//20261112-0 add: Wheel appearance and nested galleries remain detached until accepted placement/edit.
class WheelDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WheelDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
    QMap<QString, QByteArray> pictures() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    WheelProperties m_draft;
    bool m_accepted = false;
    QLineEdit *m_address;
    QCheckBox *m_rollover;
    QSpinBox *m_sensitivity;
    QSpinBox *m_opacity;
    QSpinBox *m_rows;
    QLineEdit *m_minimum;
    QLineEdit *m_maximum;
    QLineEdit *m_step;
    QComboBox *m_lowFont;
    QComboBox *m_highFont;
    QPushButton *m_colors[3];
    QLabel *m_preview = nullptr;
    HmiPermissionEditor *m_visibility;
    HmiPermissionEditor *m_touch;
    QWidget *createAppearance();
    QWidget *createPermissions();
    QString *color(int index);
    void chooseColor(int index);
    void selectPicture(bool projectGallery);
    void refresh();
};
#endif
