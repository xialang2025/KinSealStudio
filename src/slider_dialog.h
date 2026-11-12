#ifndef SLIDER_DIALOG_H
#define SLIDER_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QPainter;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QSpinBox;
class QRadioButton;
class QPushButton;
class HmiPermissionEditor;

void paintSlider(QPainter *painter, const QRectF &rect, const SliderProperties &properties);

//20261112-0 add: Two native pages edit an independent slider and local macro pool.
class SliderDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SliderDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
    QMap<QString, QString> macros() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    SliderProperties m_draft;
    QComboBox *m_type;
    QLineEdit *m_address;
    QButtonGroup *m_pointing;
    QComboBox *m_direction;
    QComboBox *m_style;
    QSpinBox *m_railHeight;
    QSpinBox *m_thumbWidth;
    QSpinBox *m_thumbHeight;
    QPushButton *m_colors[7];
    QCheckBox *m_transparent[4];
    QCheckBox *m_dynamic;
    QLineEdit *m_minimum;
    QLineEdit *m_maximum;
    QPushButton *m_minimumPicker;
    QPushButton *m_maximumPicker;
    QGroupBox *m_macro;
    QComboBox *m_macroList;
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
    void changePointing(int value);
    void changeDynamic(bool enabled);
    void editMacro();
    QString *color(int index);
    void chooseColor(int index);
};
#endif
