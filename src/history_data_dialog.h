#ifndef HISTORY_DATA_DIALOG_H
#define HISTORY_DATA_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QPainter;
class QComboBox;
class QGridLayout;
class QLineEdit;
class QTableWidget;
class QLabel;
class HmiPermissionEditor;
void paintHistoryData(QPainter *painter, const QRectF &rect, const HistoryDataProperties &properties, const QString &language);
//20261113-2 add: Detached table edits do not create or query device history records.
class HistoryDataDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HistoryDataDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    HistoryDataProperties m_draft;
    DataCollectionProperties m_collection;
    QString m_language;
    bool m_collectionFound = false;
    bool m_accepted = false;
    QLabel *m_preview = nullptr;
    QComboBox *m_languages = nullptr;
    QComboBox *m_dataLanguages = nullptr;
    QLineEdit *m_date = nullptr;
    QLineEdit *m_time = nullptr;
    QLineEdit *m_name = nullptr;
    QTableWidget *m_table = nullptr;
    HmiPermissionEditor *m_visibility;
    QWidget *basicPage();
    QWidget *dataPage();
    void addColor(QGridLayout *grid, int row, const QString &label, int index);
    QString *color(int index);
    void changeLanguage(const QString &language);
    void refreshNames();
    void selectColumn(int row);
    void refresh();
};
#endif
