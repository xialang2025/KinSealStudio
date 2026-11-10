#ifndef STATIC_TEXT_DIALOG_H
#define STATIC_TEXT_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QFontComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class StaticTextPreview;

class StaticTextDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StaticTextDialog(const HmiObject &object, const QStringList &languages,
                              QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_working;
    QString m_language;
    QPlainTextEdit *m_text;
    QFontComboBox *m_font;
    QComboBox *m_size;
    QDoubleSpinBox *m_spacing;
    QComboBox *m_alignment;
    QComboBox *m_style;
    QComboBox *m_languages;
    QCheckBox *m_italic;
    QCheckBox *m_bold;
    QCheckBox *m_adaptive;
    QCheckBox *m_underline;
    QSpinBox *m_opacity;
    QPushButton *m_colors[3];
    QColor m_colorValues[3];
    QGroupBox *m_visibility;
    QGroupBox *m_bit;
    QGroupBox *m_level;
    QGroupBox *m_word;
    QLineEdit *m_bitAddress;
    QRadioButton *m_bitOn;
    QComboBox *m_minimumLevel;
    QComboBox *m_dataType;
    QLineEdit *m_wordAddress;
    QComboBox *m_comparison;
    QLineEdit *m_value;
    QLineEdit *m_tolerance;
    QLabel *m_toleranceLabel;
    StaticTextPreview *m_preview;
    void refresh();
    void chooseColor(int index);
    void editAddress(QLineEdit *field, bool bit);
    void switchLanguage(const QString &language);
};
#endif
