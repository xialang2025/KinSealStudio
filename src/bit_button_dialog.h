#ifndef BIT_BUTTON_DIALOG_H
#define BIT_BUTTON_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QCheckBox;
class QButtonGroup; //20261110-0 add
class QComboBox;
class QFontComboBox;
class QGroupBox;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QRadioButton; //20261109-0 add
class QSpinBox;
class HmiPermissionEditor;
class BitButtonPreview;
class QLabel;

//20261108-15 add: Edit a detached project/object draft and publish only after acceptance.
class BitButtonDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BitButtonDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object();
    QMap<QString, QString> macros() const;
    QMap<QString, QByteArray> pictures() const;
    bool choosePictureForControl(bool projectGallery, QString *name, QByteArray *png,
                                 QMap<QString, QByteArray> *pool,
                                 QVector<QPair<QString, QByteArray>> *choices = nullptr); //20261112-0 add
protected:
    void accept() override;
private:
    HmiObject m_working;
    HmiProject m_project;
    QString m_language;
    int m_state = 0;
    bool m_loading = true;
    bool m_fontEdited = false; //20261109-6 add: A platform fallback is a display choice until the font is explicitly edited.
    QLineEdit *m_address;
    QComboBox *m_operation;
    QGroupBox *m_macro;
    QComboBox *m_macroList;
    QCheckBox *m_indicator;
    QGroupBox *m_monitor;
    QCheckBox *m_monitorSame;
    QComboBox *m_monitorType;
    QLineEdit *m_monitorAddress;
    QPushButton *m_monitorPicker;
    QSpinBox *m_bitIndex;
    QSpinBox *m_stateSpin = nullptr; //20261109-8 change
    QComboBox *m_languageCombo;
    QPlainTextEdit *m_text;
    QCheckBox *m_flash;
    QCheckBox *m_sameLanguage;
    QPushButton *m_bold;
    QPushButton *m_italic;
    QPushButton *m_underline;
    QFontComboBox *m_font;
    QSpinBox *m_size;
    QComboBox *m_alignment;
    QPushButton *m_textColor;
    QLineEdit *m_picture;
    QPushButton *m_copyText;
    QPushButton *m_copyShape;
    QCheckBox *m_effect3d;
    QCheckBox *m_transparent;
    QPushButton *m_background;
    QSpinBox *m_opacity;
    HmiPermissionEditor *m_visibility;
    HmiPermissionEditor *m_touch;
    QGroupBox *m_notice;
    QCheckBox *m_noticeBit;
    QCheckBox *m_noticeWord;
    QLineEdit *m_noticeBitAddress; //20261109-0 add
    QRadioButton *m_noticeBitOn;
    QLineEdit *m_noticeWordAddress;
    QComboBox *m_noticeWordType;
    QLineEdit *m_noticeWordValue;
    QCheckBox *m_press;
    QSpinBox *m_pressTime;
    QWidget *m_pressValues;
    BitButtonPreview *m_preview;
    QComboBox *m_wordDataType = nullptr; //20261109-6 add: Word-only controls share the other three property pages.
    QLineEdit *m_wordConstant = nullptr;
    QLineEdit *m_wordMinimum = nullptr;
    QLineEdit *m_wordMaximum = nullptr;
    QSpinBox *m_wordTotalDigits = nullptr;
    QSpinBox *m_wordDecimalDigits = nullptr;
    QCheckBox *m_wordPressedEffect = nullptr;
    QWidget *m_wordConstantField = nullptr;
    QWidget *m_wordMinimumField = nullptr;
    QWidget *m_wordMaximumField = nullptr;
    QWidget *m_wordTotalField = nullptr;
    QWidget *m_wordDecimalField = nullptr;
    QGroupBox *m_lampMultiMonitor = nullptr; //20261109-8 add: Lamps select read-only monitoring without touch operations.
    QCheckBox *m_lampMultiSame = nullptr;
    QComboBox *m_lampMultiType = nullptr;
    QLineEdit *m_lampMultiAddress = nullptr;
    QPushButton *m_lampMultiPicker = nullptr;
    QComboBox *m_lampMultiDataType = nullptr;
    QSpinBox *m_lampMultiCount = nullptr;
    bool m_lampBitIndexEdited = false;
    QComboBox *m_function = nullptr; //20261110-0 add: System-function controls have no operating PLC address.
    QComboBox *m_functionType = nullptr;
    QWidget *m_functionTypeField = nullptr;
    QWidget *m_functionKeyboard = nullptr;
    QButtonGroup *m_functionKeys = nullptr;
    QLineEdit *m_functionKeyboardText = nullptr;
    QCheckBox *m_functionPressedEffect = nullptr;
    QComboBox *m_screenAction = nullptr; //20261112-0 add
    QComboBox *m_screenTarget = nullptr;
    QLabel *m_screenTargetLabel = nullptr;
    QGroupBox *m_screenChangeLevel = nullptr;
    QComboBox *m_screenNewLevel = nullptr;
    QCheckBox *m_screenRequireLogin = nullptr;
    QWidget *m_screenTargetRow = nullptr;
    int m_screenActionDraft = 0;
    QWidget *createScreenGeneral();
    void refreshScreenTarget();

    void storeState();
    void loadState();
    void refresh();
    void refreshMonitor();
    bool selectPicture(bool projectGallery, bool rejectGif = false,
                       QVector<QPair<QString, QByteArray>> *choices = nullptr); //20261112-0 change
    void editMacro();
    void chooseColor(bool background);
    QWidget *createGeneral();
    QWidget *createWordGeneral(); //20261109-6 add
    QWidget *createFunctionGeneral(); //20261110-0 add
    void refreshFunction();
    void loadWordOperation();
    void refreshWordAppearance();
    QWidget *createMonitor();
    QWidget *createLampMonitor(); //20261109-8 add
    void refreshLampMonitor();
    BitButtonState &appearance(int index);
    QWidget *createAppearance();
    QWidget *createPermissions();
};
#endif
