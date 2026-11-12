#ifndef XY_CURVE_DIALOG_H
#define XY_CURVE_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QGridLayout;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QLabel;
class QPushButton;
class QPainter;
class HmiPermissionEditor;
void paintXyCurve(QPainter *painter, const QRectF &rect, const XyCurveProperties &properties);

//20261113-1 add: Sampling configuration remains separate from any unverified live acquisition buffer.
class XyCurveDialog : public QDialog
{
    Q_OBJECT
public:
    explicit XyCurveDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    XyCurveProperties m_draft;
    QComboBox *m_channel;
    QSpinBox *m_count;
    QLineEdit *m_minimum;
    QLineEdit *m_maximum;
    QComboBox *m_lineStyle;
    QPushButton *m_channelColor;
    QLabel *m_preview = nullptr;
    HmiPermissionEditor *m_visibility;
    bool m_accepted = false;
    QWidget *channelPage();
    QWidget *displayPage();
    QWidget *axisPage(bool y);
    QLineEdit *textField(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value, int addressRole = 0);
    QComboBox *choice(QGridLayout *layout, int row, const QString &label, const QString &name, const QStringList &items, int *value, int offset = 0);
    QCheckBox *toggle(QGridLayout *layout, int row, const QString &label, const QString &name, bool *value);
    QSpinBox *integer(QGridLayout *layout, int row, const QString &label, const QString &name, int *value, int minimum, int maximum);
    QPushButton *color(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value);
    void changeCount(int count);
    void showChannel(int channel);
    void refresh();
};
#endif
