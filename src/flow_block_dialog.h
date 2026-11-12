#ifndef FLOW_BLOCK_DIALOG_H
#define FLOW_BLOCK_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QPainter;
class QComboBox;
class QSpinBox;
class QRadioButton;
class QCheckBox;
class QButtonGroup;
class QLineEdit;
class QLabel;
class QPushButton;
class HmiPermissionEditor;

void paintFlowBlock(QPainter *painter, const QRectF &rect, const FlowBlockProperties &properties);

//20261112-0 add: Flow preview is a static editor sample; trigger scanning and firmware timing stay outside this dialog.
class FlowBlockDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FlowBlockDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    FlowBlockProperties m_draft;
    bool m_accepted = false;
    QComboBox *m_pattern;
    QComboBox *m_direction;
    QSpinBox *m_count;
    QButtonGroup *m_speed;
    QRadioButton *m_horizontal;
    QRadioButton *m_vertical;
    QCheckBox *m_noBorder;
    QLineEdit *m_trigger;
    QPushButton *m_colors[5];
    QLabel *m_preview;
    HmiPermissionEditor *m_visibility;
    QWidget *createAppearance();
    QString *color(int index);
    void chooseColor(int index);
    void changeOrientation(bool vertical);
    void refresh();
};
#endif
