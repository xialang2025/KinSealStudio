#ifndef MOVE_TRACK_DIALOG_H
#define MOVE_TRACK_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QPainter;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QRadioButton;
class HmiPermissionEditor;
void paintMoveTrack(QPainter *painter, const QRectF &rect, const MoveTrackProperties &properties);

//20261112-0 add: Native appearance/access pages edit a detached model and resource pool.
class MoveTrackDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MoveTrackDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
    QMap<QString, QByteArray> pictures() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    MoveTrackProperties m_draft;
    bool m_accepted = false;
    QLineEdit *m_address;
    QSpinBox *m_distance;
    QComboBox *m_direction;
    QComboBox *m_range;
    QLineEdit *m_minimum;
    QLineEdit *m_maximum;
    QLineEdit *m_minimumAddress;
    QLineEdit *m_maximumAddress;
    QPushButton *m_minimumPicker;
    QPushButton *m_maximumPicker;
    QLabel *m_preview;
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
    QWidget *createAppearance();
    QWidget *createPermissions();
    void updateRange();
    void selectPicture(bool projectGallery);
    void refresh();
};
#endif
