#ifndef SCALE_DIALOG_H
#define SCALE_DIALOG_H

#include "project_model.h"
#include <QDialog>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPainter;
class QRectF;
class QSpinBox;

class ScaleDialog : public QDialog
{
    Q_OBJECT
public:
    ScaleDialog(const HmiObject &object, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    ScaleProperties m_draft;
    QComboBox *m_fontSize;
    QComboBox *m_direction;
    QButtonGroup *m_position;
    QLineEdit *m_minimum;
    QLineEdit *m_maximum;
    QComboBox *m_decimals;
    QCheckBox *m_showValues;
    QSpinBox *m_major;
    QSpinBox *m_minor;
    QLabel *m_preview;
    bool readProperties(ScaleProperties *properties, QString *error = nullptr) const;
    void refreshPreview();
    void updateDirections();
};

void paintScale(QPainter *painter, const QRectF &rect, const ScaleProperties &properties);

#endif
