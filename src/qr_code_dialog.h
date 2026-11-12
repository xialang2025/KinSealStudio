#ifndef QR_CODE_DIALOG_H
#define QR_CODE_DIALOG_H

#include "project_model.h"
#include <QDialog>
#include <QImage>
class QPainter;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QLineEdit;
class QLabel;
class HmiPermissionEditor;

QImage qrDescriptionPreview(const QString &description, QString *error = nullptr);
void paintQrCode(QPainter *painter, const QRectF &rect, const QrCodeProperties &properties);

//20261112-0 add: The two native pages own a detached draft until a valid acceptance.
class QrCodeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QrCodeDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    void refreshPreview();
    HmiObject m_original;
    HmiProject m_project;
    QLineEdit *m_address;
    QComboBox *m_encoding;
    QCheckBox *m_swap;
    QSpinBox *m_length;
    QLineEdit *m_description;
    QLabel *m_preview;
    HmiPermissionEditor *m_visibility;
};

#endif
