#ifndef HMI_PROPERTY_WIDGETS_H
#define HMI_PROPERTY_WIDGETS_H
#include "static_text_properties.h"
#include <QGroupBox>
class QLineEdit;
class QComboBox;
class QRadioButton;
class QLabel;

void editHmiAddress(QWidget *parent, QLineEdit *field, bool bit, int transferCount = 1); //20261109-6 change

//20261108-15 add: Reuse the observed permission hierarchy in both bit-button columns.
class HmiPermissionEditor : public QGroupBox
{
    Q_OBJECT
public:
    explicit HmiPermissionEditor(const QString &title, const QString &prefix,
                                 const StaticTextProperties &properties, bool touch, QWidget *parent);
    StaticTextProperties properties() const;
    void configureUnchosenBitState(bool chosen); //20261112-0 add
    bool bitStateChosen() const;
private:
    StaticTextProperties m_original;
    QGroupBox *m_bit;
    QLineEdit *m_bitAddress;
    QRadioButton *m_on;
    QRadioButton *m_off; //20261112-0 add
    QGroupBox *m_level;
    QComboBox *m_minimum;
    QGroupBox *m_word;
    QComboBox *m_type;
    QLineEdit *m_wordAddress;
    QComboBox *m_comparison;
    QLineEdit *m_value;
    QLineEdit *m_error;
    QLabel *m_errorLabel;
};
#endif
