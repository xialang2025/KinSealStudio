#ifndef NUMBER_DISPLAY_DIALOG_H
#define NUMBER_DISPLAY_DIALOG_H

#include "project_model.h"
#include <QDialog>
class QCheckBox;
class QComboBox;
class QFormLayout;
class QGroupBox;
class QLineEdit;
class QSpinBox;
class QLabel;
class QPainter;
class HmiPermissionEditor;

void paintNumberDisplay(QPainter *painter, const QRectF &rect, const NumberDisplayProperties &properties);

//20261112-0 add: Four complete property pages share a detached object and gallery transaction.
class NumberDisplayDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NumberDisplayDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
    QMap<QString, QByteArray> pictures() const;
protected:
    void accept() override;
private:
    QCheckBox *check(const QString &key, const QString &label, QWidget *parent);
    QGroupBox *group(const QString &key, const QString &label, QWidget *parent);
    QLineEdit *line(const QString &key, QWidget *parent);
    QComboBox *combo(const QString &key, const QStringList &items, QWidget *parent, bool numericItems = false);
    QSpinBox *spin(const QString &key, int minimum, int maximum, QWidget *parent);
    QWidget *address(const QString &key, bool bit, QWidget *parent);
    QWidget *range(const QString &prefix, QWidget *parent);
    void choosePicture(bool projectGallery);
    void updateLinkage();
    void changeType(int type);
    QJsonValue field(const QString &key) const;
    HmiObject m_original;
    HmiProject m_project;
    NumberDisplayProperties m_draft;
    QMap<QString, QWidget *> m_fields;
    QMap<QString, QWidget *> m_addressRows;
    HmiPermissionEditor *m_visibility = nullptr;
    HmiPermissionEditor *m_touch = nullptr;
    QGroupBox *m_inputRange = nullptr;
    QLabel *m_preview = nullptr;
    QWidget *m_keyboardLabel = nullptr;
    QWidget *m_pressUnits = nullptr;
    QWidget *m_noticeBitStates = nullptr;
    bool m_ready = false;
    bool m_updating = false;
    bool m_inputShowingMonitor = false;
};

#endif
