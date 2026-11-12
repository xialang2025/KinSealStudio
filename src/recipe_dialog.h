#ifndef RECIPE_DIALOG_H
#define RECIPE_DIALOG_H
#include "project_model.h"
#include "recipe_properties.h"
#include <QDialog>
class QGridLayout;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QTableWidget;
//20261113-2 add: File imports, values and hidden matrix drafts remain local until parent acceptance.
class RecipeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RecipeDialog(const HmiConfigurationNode &node, const HmiProject &project, QWidget *parent = nullptr);
    HmiConfigurationNode node() const;
protected:
    void accept() override;
private:
    HmiConfigurationNode m_original;
    HmiProject m_project;
    RecipeProperties m_draft;
    bool m_initialValid = true;
    bool m_updating = false;
    QLineEdit *m_name;
    QLineEdit *m_description;
    QSpinBox *m_length;
    QSpinBox *m_count;
    QComboBox *m_type;
    QCheckBox *m_write;
    QCheckBox *m_read;
    QCheckBox *m_same;
    QLineEdit *m_writeAddress;
    QLineEdit *m_readAddress;
    QPushButton *m_readPicker;
    QLineEdit *m_memory;
    QLineEdit *m_current;
    QLineEdit *m_recipeCaption;
    QLineEdit *m_itemCaption;
    QLineEdit *m_value;
    QTableWidget *m_table = nullptr;
    QSpinBox *m_source;
    QSpinBox *m_target;
    QWidget *basicPage();
    QWidget *dataPage();
    void refreshRanges();
    void rebuildTable();
    void showCell(int row, int column);
    void importCsv();
    void exportCsv();
    void refill();
};
#endif
