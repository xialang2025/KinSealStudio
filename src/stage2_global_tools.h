#ifndef STAGE2_GLOBAL_TOOLS_H
#define STAGE2_GLOBAL_TOOLS_H

#include "project_model.h"
#include <QDialog>
#include <QSet>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

//20261110-7 add: Global indexes use stable identities and never retain canvas pointers.
namespace Stage2GlobalTools
{
enum class Kind { Controls, Texts, Addresses, Macros };

struct Scope
{
    enum class Range { AllPages, CurrentPage, SelectedObjects };
    Range range = Range::AllPages;
    QString pageId;
    QSet<QString> objectIds;
    bool includeDormant = false;
};

struct Location
{
    QString pageId;
    QString objectId;
    QString macroId;
    QString macroName;
    int line = 0;
};

struct Row
{
    Location location;
    QString pageName;
    QString objectName;
    QString type;
    QString fieldPath;
    QString label;
    QString value;
    QString language;
    bool active = true;
    bool replaceable = false;
    PlcAddressRole role = PlcAddressRole::Any;
    u32 transferCount = 1;
    QString diagnostic;
};

struct AddressChange
{
    Location location;
    QString fieldPath;
    QString before;
    QString after;
};

struct AddressCandidate
{
    QByteArray projectFingerprint;
    QList<AddressChange> changes;
    int excludedDormantMatches = 0;
};

QList<Row> enumerate(const HmiProject &project, Kind kind, const Scope &scope = Scope());
QList<Row> search(const HmiProject &project, Kind kind, const QString &needle,
                  const Scope &scope = Scope(), Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive);
bool previewAddressReplacement(const HmiProject &project, const QString &findAddress,
                                const QString &replacementAddress, const Scope &scope,
                                AddressCandidate *candidate, QString *error = nullptr);
bool applyCandidate(const HmiProject &project, const AddressCandidate &candidate,
                     HmiProject *output, QString *error = nullptr);

//20261110-7 add: Accepting an index returns navigation only; callers own page switching.
class GlobalToolsDialog : public QDialog
{
public:
    GlobalToolsDialog(const HmiProject &project, Kind kind, QWidget *parent = nullptr);
    Location selectedLocation() const;
    void setSelectedObjectIds(const QSet<QString> &ids);

private:
    HmiProject m_project;
    Kind m_kind;
    QList<Row> m_rows;
    Location m_selected;
    QSet<QString> m_selectedObjectIds;
    QLineEdit *m_filter;
    QComboBox *m_scope;
    QTableWidget *m_table;
    QLabel *m_status;
    void refresh();
    void locate();
};

//20261110-7 add: A preview is a local draft; only Accepted exposes a candidate project.
class AddressReplaceDialog : public QDialog
{
public:
    explicit AddressReplaceDialog(const HmiProject &project, QWidget *parent = nullptr);
    bool candidateProject(HmiProject *output) const;
    AddressCandidate candidate() const;
    void setSelectedObjectIds(const QSet<QString> &ids);
    void accept() override;

private:
    HmiProject m_project;
    HmiProject m_acceptedProject;
    AddressCandidate m_candidate;
    bool m_hasPreview = false;
    bool m_hasAcceptedProject = false;
    QSet<QString> m_selectedObjectIds;
    QLineEdit *m_find;
    QLineEdit *m_replacement;
    QComboBox *m_scope;
    QCheckBox *m_dormant;
    QTableWidget *m_table;
    QLabel *m_status;
    QPushButton *m_confirm;
    void invalidatePreview();
    void preview();
};
}

#endif
