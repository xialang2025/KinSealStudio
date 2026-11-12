#ifndef STAGE3_PROJECT_EDITS_H
#define STAGE3_PROJECT_EDITS_H

#include "project_model.h"
#include <QDialog>
#include <QPoint>

class QComboBox;
class QCheckBox;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QSpinBox;
class QTableWidget;

//20261110-8 add: This value belongs to the application document, never the operating-system clipboard.
struct Stage3Clipboard
{
    HmiProject source;
    QStringList objectIds;
};

enum class Stage3AddressField
{
    Operation, MonitorBit, MonitorWord, LampMultiMonitor, NoticeBit, NoticeWord,
    VisibilityBit, VisibilityWord, TouchBit, TouchWord,
    OffsetWord, InputMinimumWord, InputMaximumWord, DisplayMinimumWord, DisplayMaximumWord, //20261112-0 add
    TriggerBit, ClearBit, SamplingWord //20261113-1 change: Keep sampling-control bits and the independent sample-word role distinct.
};

struct Stage3AddressIncrement
{
    Stage3AddressField field = Stage3AddressField::Operation;
    int columnStep = 0;
    int rowStep = 0;
};

//20261110-8 add: The pure model grid includes the unchanged source cell at row zero, column zero.
struct Stage3DuplicateOptions
{
    int rows = 1;
    int columns = 1;
    int offsetX = 16;
    int offsetY = 16;
    QList<Stage3AddressIncrement> increments;
};

bool captureStage3Selection(const HmiProject &source, const QStringList &objectIds,
                            Stage3Clipboard *clipboard, QString *error = nullptr);
bool pasteStage3Selection(const HmiProject &target, const Stage3Clipboard &clipboard,
                          const QPoint &offset, HmiProject *draft, QStringList *newIds = nullptr,
                          QString *error = nullptr);
bool deleteStage3Selection(const HmiProject &source, const QStringList &objectIds,
                           HmiProject *draft, QString *error = nullptr);
bool cutStage3Selection(const HmiProject &source, const QStringList &objectIds,
                        Stage3Clipboard *clipboard, HmiProject *draft, QString *error = nullptr);
bool duplicateStage3Selection(const HmiProject &source, const QStringList &objectIds,
                              const Stage3DuplicateOptions &options, HmiProject *draft,
                              QStringList *newIds = nullptr, QString *error = nullptr);

class Stage3DuplicateDialog : public QDialog
{
    Q_OBJECT
public:
    explicit Stage3DuplicateDialog(const HmiProject &project, const QStringList &objectIds,
                                    QWidget *parent = nullptr);
    HmiProject draftProject() const;
    QStringList newObjectIds() const;
protected:
    void accept() override;
private:
    HmiProject m_source;
    HmiProject m_working;
    HmiProject m_preview;
    QStringList m_selection;
    QStringList m_newIds;
    QStringList m_previewIds;
    QSpinBox *m_rows;
    QSpinBox *m_columns;
    QSpinBox *m_offsetX;
    QSpinBox *m_offsetY;
    QCheckBox *m_incrementEnabled;
    QComboBox *m_incrementMode;
    QComboBox *m_incrementDirection;
    QSpinBox *m_incrementStep;
    QGroupBox *m_nativeIncrementGroup;
    QGroupBox *m_advancedIncrementGroup;
    QComboBox *m_addressField;
    QSpinBox *m_columnAddressStep;
    QSpinBox *m_rowAddressStep;
    QTableWidget *m_table;
    QLabel *m_error;
    QLabel *m_summary;
    QDialogButtonBox *m_buttons;
    bool refreshPreview();
};

#endif
