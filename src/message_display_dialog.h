#ifndef MESSAGE_DISPLAY_DIALOG_H
#define MESSAGE_DISPLAY_DIALOG_H

#include "project_model.h"
#include <QDialog>

class QComboBox;
class QFontComboBox;
class QSpinBox;
class QLineEdit;
class QLabel;
class QPlainTextEdit;
class QTableWidget;
class HmiPermissionEditor;
class QPainter;
class QRectF;
class QPushButton;

//20261112-0 add: The dialog owns every draft until acceptance; selected state and language remain editor-only.
class MessageDisplayDialog : public QDialog
{
    Q_OBJECT
public:
    MessageDisplayDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    MessageDisplayProperties m_draft;
    StaticTextProperties m_currentAppearance;
    QColor m_currentFrameColor;
    QSpinBox *m_count;
    QComboBox *m_type;
    QLineEdit *m_address;
    QComboBox *m_movement;
    QSpinBox *m_speed;
    QComboBox *m_language;
    QComboBox *m_font;
    QComboBox *m_fontSize;
    QComboBox *m_style;
    QList<QPushButton *> m_colorButtons;
    QTableWidget *m_table;
    QPlainTextEdit *m_text;
    QLabel *m_preview;
    HmiPermissionEditor *m_permission;
    int m_state = 0;
    QString m_currentLanguage;
    bool m_refreshing = false;
    void rebuildTable();
    void selectState(int row);
    void refreshPreview();
    void changeLanguage(const QString &language);
};

void paintMessageDisplay(QPainter *painter, const QRectF &rect, const MessageDisplayProperties &properties,
                         const QString &language, int state);

#endif
