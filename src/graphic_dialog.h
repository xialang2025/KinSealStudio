#ifndef GRAPHIC_DIALOG_H
#define GRAPHIC_DIALOG_H

#include "project_model.h"
#include <QDialog>

class QComboBox;
class QCheckBox;
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
    QComboBox *m_style = nullptr;
    QComboBox *m_width = nullptr;
    QComboBox *m_arrow = nullptr;
    QComboBox *m_cap = nullptr;
    QSpinBox *m_alpha = nullptr;
    QLabel *m_preview = nullptr;
    QPushButton *m_color = nullptr;
    QPushButton *m_foreground = nullptr;
    QPushButton *m_background = nullptr;
    QComboBox *m_pattern = nullptr;
    QSpinBox *m_radiusX = nullptr; //20261113-2 add: Independent native radius drafts exist only for roundrect.
    QSpinBox *m_radiusY = nullptr;
    QSpinBox *m_rows = nullptr; //20261113-8 add
    QSpinBox *m_columns = nullptr;
    QCheckBox *m_horizontal = nullptr;
    QCheckBox *m_vertical = nullptr;
    QCheckBox *m_outline = nullptr;
    QPushButton *m_frame = nullptr;
    void initializeTableUi();
    void refreshTablePreview();
    void refreshPreview();
};

#endif
