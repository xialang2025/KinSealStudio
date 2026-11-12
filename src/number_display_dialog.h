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

void paintNumberDisplay(QPainter *painter, const QRectF &rect, const NumberDisplayProperties &properties, const QString &sample = QString(), bool ascii = false, bool wrap = false); //20261112-0 change
void paintAsciiDisplay(QPainter *painter, const QRectF &rect, const AsciiDisplayProperties &properties);
void paintTimeDisplay(QPainter *painter, const QRectF &rect, const TimeDisplayProperties &properties); //20261112-0 add

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
    AsciiDisplayProperties m_asciiDraft;
    TimeDisplayProperties m_timeDraft; //20261112-0 add
    QMap<QString, QWidget *> m_fields;
    QMap<QString, QWidget *> m_addressRows;
    HmiPermissionEditor *m_visibility = nullptr;
    HmiPermissionEditor *m_touch = nullptr;
    QGroupBox *m_inputRange = nullptr;
    QLabel *m_preview = nullptr;
    QWidget *m_keyboardLabel = nullptr;
    QWidget *m_pressUnits = nullptr;
    QWidget *m_noticeBitStates = nullptr;
    QWidget *m_dataTypeLabel = nullptr;
    QWidget *m_totalDigitsLabel = nullptr;
    QWidget *m_characterCountLabel = nullptr;
    QWidget *m_asciiControls = nullptr;
    QWidget *m_dataControls = nullptr;
    QWidget *m_timeControls = nullptr;
    QWidget *m_numberFontControls = nullptr;
    QWidget *m_timeFontControls = nullptr;
    QWidget *m_timeTotal = nullptr;
    QWidget *m_timeTotalLabel = nullptr;
    QWidget *m_otherPage = nullptr;
    QLabel *m_timeSample = nullptr;
    QMap<QString, QLabel *> m_colorLabels; //20261112-0 add: The shared page uses the measured text/numeric labels for its active family.
    bool m_ready = false;
    bool m_updating = false;
    bool m_inputShowingMonitor = false;
    bool m_asciiMode = false;
    bool m_timeMode = false;
};

#endif
