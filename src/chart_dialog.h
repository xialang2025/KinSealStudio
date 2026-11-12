#ifndef CHART_DIALOG_H
#define CHART_DIALOG_H
#include "project_model.h"
#include <QDialog>
class QGridLayout;
class QLineEdit;
class QComboBox;
class QDoubleSpinBox;
class QCheckBox;
class QTabWidget;
class QStackedWidget;
class QRadioButton;
class QLabel;
class QPainter;
class HmiPermissionEditor;
void paintChart(QPainter *painter, const QRectF &rect, const ChartProperties &properties);

//20261112-1 add: All forms and permissions remain detached until acceptance.
class ChartDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ChartDialog(const HmiObject &object, const HmiProject &project, QWidget *parent = nullptr);
    HmiObject object() const;
protected:
    void accept() override;
private:
    HmiObject m_original;
    HmiProject m_project;
    ChartProperties m_draft;
    QTabWidget *m_tabs;
    QStackedWidget *m_basic;
    QStackedWidget *m_appearance;
    QRadioButton *m_shapes[3];
    QLabel *m_preview;
    HmiPermissionEditor *m_visibility;
    QLineEdit *m_barMinimum;
    QLineEdit *m_barMaximum;
    QLineEdit *m_midpoint;
    QCheckBox *m_twoPoles;
    QLineEdit *m_sourceMinimum;
    QLineEdit *m_sourceMaximum;
    QLineEdit *m_meterMinimum;
    QLineEdit *m_meterMaximum;
    QLineEdit *m_donutMinimum;
    QLineEdit *m_donutMaximum;
    QStackedWidget *m_donutAttributes;
    bool m_accepted = false;
    QWidget *basicPage(int shape);
    QWidget *appearancePage(int shape);
    QLineEdit *textField(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value, bool address = false, int words = 1);
    QComboBox *choice(QGridLayout *layout, int row, const QString &label, const QString &name, const QStringList &items, int *value, int offset = 0);
    QCheckBox *toggle(QGridLayout *layout, int row, const QString &label, const QString &name, bool *value);
    QLineEdit *number(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value, int type);
    void integer(QGridLayout *layout, int row, const QString &label, const QString &name, int *value, int minimum, int maximum);
    void color(QGridLayout *layout, int row, const QString &label, const QString &name, QString *value);
    void selectShape(int shape);
    void changeType(int shape, int type);
    void refresh();
};
#endif
