#ifndef IMAGE_DISPLAY_DIALOG_H
#define IMAGE_DISPLAY_DIALOG_H

#include "project_model.h"
#include <QDialog>
class QPainter;
class QButtonGroup;
class QGroupBox;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QTableWidget;
class QLabel;
class QPushButton;
class HmiPermissionEditor;

void paintImageDisplay(QPainter *painter, const QRectF &rect, const ImageDisplayProperties &properties, int state);

//20261112-0 add: Picture selection, nested gallery resources and all three property pages remain in a detached draft.
class ImageDisplayDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ImageDisplayDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
    QMap<QString, QByteArray> pictures() const;
protected:
    void accept() override;
private:
    void updateMode(bool conditionChanged);
    void rebuildTable();
    void refreshPreview();
    void selectPicture(bool projectGallery);
    void deletePictures();
    void movePicture(int offset);
    HmiObject m_original;
    HmiProject m_project;
    ImageDisplayProperties m_draft;
    QButtonGroup *m_type;
    QButtonGroup *m_condition;
    QGroupBox *m_conditions;
    QSpinBox *m_count;
    QLineEdit *m_address;
    QPushButton *m_picker;
    QSpinBox *m_interval;
    QCheckBox *m_batchAdd;
    QCheckBox *m_batchDelete;
    QSpinBox *m_start;
    QSpinBox *m_end;
    QTableWidget *m_table;
    QLabel *m_preview;
    HmiPermissionEditor *m_visibility;
    int m_state = 0;
    bool m_updating = false;
};

#endif
