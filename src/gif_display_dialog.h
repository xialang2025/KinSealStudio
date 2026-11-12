#ifndef GIF_DISPLAY_DIALOG_H
#define GIF_DISPLAY_DIALOG_H

#include "project_model.h"
#include <QBuffer>
#include <QDialog>
#include <QMovie>
class QCheckBox;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QLabel;
class HmiPermissionEditor;
class QPainter;

void paintGifDisplay(QPainter *painter, const QRectF &rect, const GifDisplayProperties &properties);

//20261112-0 add: Preview state belongs to the dialog; no timer or trial frame enters persisted properties.
class GifDisplayDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GifDisplayDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
    QMap<QString, QByteArray> animations() const;
protected:
    void accept() override;
private:
    void selectAnimation();
    void loadPreview();
    void togglePlayback();
    void refreshFrame();
    HmiObject m_original;
    HmiProject m_project;
    GifDisplayProperties m_draft;
    QBuffer m_buffer;
    QMovie m_movie;
    QCheckBox *m_bitControl;
    QLineEdit *m_address;
    QPushButton *m_addressPicker;
    QRadioButton *m_state0;
    QRadioButton *m_state1;
    QPushButton *m_run;
    QLabel *m_preview;
    HmiPermissionEditor *m_visibility;
    bool m_playing = false;
};

#endif
