#ifndef RECIPE_DISPLAY_DIALOG_H
#define RECIPE_DISPLAY_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QPainter;
class QGridLayout;
class QGroupBox;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;
class HmiPermissionEditor;
void paintRecipeDisplay(QPainter *painter, const QRectF &rect, const RecipeDisplayProperties &properties, const QStringList &titles, int recipeCount);
//20261113-2 add: Native basic settings contain an edit/keyboard panel rather than an invented table preview.
class RecipeDisplayDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RecipeDisplayDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    RecipeDisplayProperties m_draft;
    bool m_accepted = false;
    QGroupBox *m_edit;
    QWidget *m_keyboardPanel;
    QCheckBox *m_keyboard;
    QComboBox *m_keyboardPage;
    QLabel *m_keyboardLabel;
    QCheckBox *m_position;
    QWidget *m_coordinates;
    QPushButton *m_gridColor;
    QLabel *m_gridLabel;
    HmiPermissionEditor *m_visibility;
    HmiPermissionEditor *m_touch;
    QGroupBox *m_notice;
    QCheckBox *m_noteBit;
    QCheckBox *m_noteWord;
    QWidget *m_bitFields;
    QWidget *m_wordFields;
    QWidget *basicPage();
    QWidget *permissionPage();
    QString *color(int index);
    QPushButton *addColor(QGridLayout *layout, int row, const QString &label, int index);
    void refreshLinkage();
};
#endif
