#ifndef DATA_COLLECTION_DIALOG_H
#define DATA_COLLECTION_DIALOG_H
#include "project_model.h"
#include "data_collection_properties.h"
#include <QDialog>
class QGridLayout;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QTableWidget;
class QPushButton;

//20261112-1 add: One collection definition is edited transactionally; no history-file or PLC acquisition engine is implied.
class DataCollectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DataCollectionDialog(const HmiConfigurationNode &node, const HmiProject &project, QWidget *parent = nullptr);
    HmiConfigurationNode node() const;
protected:
    void accept() override;
private:
    HmiConfigurationNode m_original;
    HmiProject m_project;
    DataCollectionProperties m_draft;
    bool m_initialValid = true;
    QLineEdit *m_name;
    QLineEdit *m_memory = nullptr;
    QSpinBox *m_length;
    QTableWidget *m_items;
    QLineEdit *m_cache;
    QLineEdit *m_itemName;
    QComboBox *m_format;
    QSpinBox *m_digits;
    QSpinBox *m_fraction;
    QCheckBox *m_scaled;
    QLineEdit *m_gain;
    QLineEdit *m_offset;
    QWidget *basicPage();
    QWidget *itemsPage();
    QLineEdit *textField(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value, int addressRole = 0);
    QCheckBox *toggle(QGridLayout *layout, int row, const QString &label, const QString &name, bool *value);
    QSpinBox *integer(QGridLayout *layout, int row, const QString &label, const QString &name, int *value, int minimum, int maximum);
    void refresh();
    void changeLength(int count);
    void rebuildItems();
    void showItem(int row);
    bool wouldRemoveReferencedItem(int count) const;
};
#endif
