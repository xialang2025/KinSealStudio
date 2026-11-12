#ifndef GRAPHIC_DIALOG_H
#define GRAPHIC_DIALOG_H

#include "project_model.h"
#include <QDialog>

class QComboBox;
class QSpinBox;
class QLabel;
class QPushButton;

class GraphicDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GraphicDialog(const HmiObject &object, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    GraphicProperties m_draft;
    QComboBox *m_style;
    QComboBox *m_width;
    QComboBox *m_arrow;
    QComboBox *m_cap;
    QSpinBox *m_alpha;
    QLabel *m_preview;
    QPushButton *m_color;
    QPushButton *m_foreground = nullptr;
    QPushButton *m_background = nullptr;
    QComboBox *m_pattern = nullptr;
    void refreshPreview();
};

#endif
