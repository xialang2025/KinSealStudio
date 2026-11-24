#include "macro_schedule_dialogs.h"
#include "dialog_button_text.h" //20261124-8 add
#include "local_help.h"
#include "macro_manager_dialog.h"
#include "macro_schedule.h"
#include "plc_address_dialog.h"
#include "project_model.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QUuid>

namespace {

class MacroScheduleManagerDialog : public QDialog //20261123-20 add
{
public:
    using QDialog::QDialog;

protected:
    /******************************************************************************
     * Function Name   : closeEvent
     * Description     : Retain confirmed row changes when the original manager is closed.
     * Input Parameters: event
     * Return Value    : None.
     * Date            : 2026.11.23
     ******************************************************************************/
    void closeEvent(QCloseEvent *event) override
    {
        accept();
        event->accept();
    }
};

/******************************************************************************
 * Function Name   : positionControl
 * Description     : Place a control at its original client coordinates and assign an automation name.
 * Input Parameters: widget, name, geometry
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void positionControl(QWidget &widget, const QString &name, const QRect &geometry) //20261123-20 add
{
    widget.setObjectName(name);
    widget.setGeometry(geometry);
}

/******************************************************************************
 * Function Name   : editScheduleRow
 * Description     : Edit a scheduled invocation and any associated macro sources as one local draft.
 * Input Parameters: parent, project, row, initialization
 * Return Value    : True if a valid invocation was selected and accepted.
 * Date            : 2026.11.23
 ******************************************************************************/
bool editScheduleRow(QWidget *parent, HmiProject *project, MacroScheduleRow *row,
                     bool initialization) //20261123-20 add
{
    QDialog dialog(parent);
    HmiProject draft = *project;
    QGroupBox properties(QStringLiteral("属性设置"), &dialog);
    QGroupBox triggerFields(&dialog);
    QLabel sourceLabel(initialization ? QStringLiteral("宏名称：") : QStringLiteral("宏指令名称："), &dialog);
    QLabel frequencyLabel(QStringLiteral("执行频率："), &dialog);
    QLabel countLabel(QStringLiteral("执行次数："), &dialog);
    QLabel frequencyUnit(QStringLiteral("x100MS"), &dialog);
    QLabel countHint(QStringLiteral("0表示一直执行"), &dialog);
    QLabel typeLabel(QStringLiteral("受控类型："), &triggerFields);
    QLabel addressLabel(QStringLiteral("受控地址："), &triggerFields);
    QComboBox source(&dialog);
    QComboBox triggerType(&triggerFields);
    QSpinBox frequency(&dialog);
    QSpinBox count(&dialog);
    QCheckBox trigger(QStringLiteral("受控触发"), &dialog);
    QLineEdit address(row->address.isEmpty() ? QStringLiteral("LB0") : row->address, &triggerFields);
    QPushButton picker(QStringLiteral("…"), &triggerFields);
    QPushButton write(initialization ? QStringLiteral("写宏") : QStringLiteral("写脚本"), &dialog);
    QPushButton confirm(dialogButtonText(QStringLiteral("确定"), "OK"), &dialog);
    QPushButton cancel(dialogButtonText(QStringLiteral("取消"), "Cancel"), &dialog);
    QString selected = draft.macroIds.key(row->macroId);
    const bool editing = !row->macroId.isEmpty();

    dialog.setObjectName(QStringLiteral("MacroScheduleRowEditor"));
    dialog.setWindowTitle(initialization ? (editing ? QStringLiteral("修改初始化宏") : QStringLiteral("新建初始化宏")) :
        (editing ? QStringLiteral("修改全局宏") : QStringLiteral("新建全局宏")));
    dialog.setFixedSize(initialization ? QSize(317, 81) : QSize(440, 359));
    dialog.setStyleSheet(QStringLiteral(
        "QComboBox#MacroScheduleSource, QSpinBox#MacroScheduleFrequency, QSpinBox#MacroScheduleCount, "
        "QComboBox#MacroScheduleTriggerType, QLineEdit#MacroScheduleAddress {background:#caeace;} "
        "QComboBox#MacroScheduleTriggerType:disabled {background:#f0f0f0;} "
        "QLineEdit#MacroScheduleAddress:disabled {background:#b0ceec;}"));
    sourceLabel.setGeometry(initialization ? QRect(10, 10, 80, 22) : QRect(44, 30, 90, 22));
    positionControl(source, QStringLiteral("MacroScheduleSource"),
        initialization ? QRect(90, 10, 211, 22) : QRect(140, 30, 191, 22));
    positionControl(write, QStringLiteral("MacroScheduleWrite"),
        initialization ? QRect(8, 46, 129, 21) : QRect(31, 321, 109, 28));
    positionControl(confirm, QStringLiteral("MacroScheduleConfirm"),
        initialization ? QRect(148, 46, 73, 21) : QRect(201, 321, 89, 28));
    positionControl(cancel, QStringLiteral("MacroScheduleCancel"),
        initialization ? QRect(228, 46, 73, 21) : QRect(331, 321, 89, 28));
    confirm.setDefault(true);
    write.setAutoDefault(false);
    cancel.setAutoDefault(false);

    if (initialization) //20261123-21 add: Startup invocations only bind a macro definition.
    {
        properties.hide();
        triggerFields.hide();
        frequencyLabel.hide();
        countLabel.hide();
        frequencyUnit.hide();
        countHint.hide();
        frequency.hide();
        count.hide();
        trigger.hide();
    }
    else
    {
        properties.setGeometry(30, 10, 345, 130);
        frequencyLabel.setGeometry(44, 60, 90, 30);
        countLabel.setGeometry(44, 100, 90, 30);
        frequencyUnit.setGeometry(299, 60, 60, 30);
        countHint.setGeometry(254, 100, 100, 30);
        positionControl(frequency, QStringLiteral("MacroScheduleFrequency"), QRect(140, 60, 150, 30));
        positionControl(count, QStringLiteral("MacroScheduleCount"), QRect(140, 100, 111, 30));
        frequency.setRange(1, 65535);
        frequency.setValue(row->frequency);
        count.setRange(0, 65535);
        count.setValue(row->count);
        positionControl(trigger, QStringLiteral("MacroScheduleTrigger"), QRect(30, 152, 100, 24));
        triggerFields.setGeometry(30, 187, 345, 104);
        typeLabel.setGeometry(10, 18, 80, 30);
        addressLabel.setGeometry(10, 63, 80, 30);
        positionControl(triggerType, QStringLiteral("MacroScheduleTriggerType"), QRect(90, 18, 191, 30));
        positionControl(address, QStringLiteral("MacroScheduleAddress"), QRect(90, 63, 171, 30));
        positionControl(picker, QStringLiteral("MacroScheduleAddressPicker"), QRect(276, 67, 22, 22));
        triggerType.addItem(QStringLiteral("位地址"));
        address.setReadOnly(true);
        picker.setAutoDefault(false);
        trigger.setChecked(!row->address.isEmpty());
        triggerFields.setEnabled(trigger.isChecked());
        QObject::connect(&trigger, &QCheckBox::toggled, &triggerFields, &QWidget::setEnabled);
        QObject::connect(&picker, &QPushButton::clicked, &dialog, [&]() {
            QString choice = address.text();

            if (editPlcAddress(&dialog, true, draft.connections, &choice))
            {
                address.setText(choice);
            }
        });
    }

    auto updateSources = [&]() {
        source.clear();
        source.addItems(draft.macros.keys());
        if (draft.macros.contains(selected))
        {
            source.setCurrentText(selected);
        }
        confirm.setEnabled(source.currentIndex() >= 0);
    };
    updateSources();
    QObject::connect(&write, &QPushButton::clicked, &dialog, [&]() {
        MacroManagerDialog editor(draft, &dialog);
        QListWidget *macroList;

        if (source.currentIndex() >= 0)
        {
            editor.selectMacro(source.currentText());
        }
        if (editor.exec() == QDialog::Accepted)
        {
            draft = editor.draftProject();
            // The list preserves the macro the user selected or created before saving the editor draft.
            macroList = editor.findChild<QListWidget *>(QStringLiteral("MacroList"));
            selected = macroList && macroList->currentItem() ? macroList->currentItem()->text() : QString();
            updateSources();
        }
    });
    QObject::connect(&confirm, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(&cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted || source.currentIndex() < 0)
    {
        return false;
    }
    selected = source.currentText();
    if (!draft.macroIds.contains(selected))
    {
        draft.macroIds.insert(selected, QUuid::createUuid().toString(QUuid::WithoutBraces));
    }
    row->macroId = draft.macroIds.value(selected);
    row->frequency = initialization ? 1 : frequency.value();
    row->count = initialization ? 0 : count.value();
    row->address = !initialization && trigger.isChecked() ? address.text() : QString();
    *project = draft;
    return true;
}

} // namespace

/******************************************************************************
 * Function Name   : editMacroSchedules
 * Description     : Present the original project-wide invocation list and publish one confirmed project draft.
 * Input Parameters: parent, project, initialization
 * Return Value    : True on acceptance.
 * Date            : 2026.11.23
 ******************************************************************************/
bool editMacroSchedules(QWidget *parent, HmiProject *project, bool initialization) //20261123-20 add
{
    const int limit = initialization ? 16 : 64;
    const QString helpTopic = initialization ? QStringLiteral("init-macro") : QStringLiteral("global-macro");

    if (project == nullptr)
    {
        return false;
    }

    HmiProject draft = *project;
    QList<MacroScheduleRow> rows = macroScheduleRows(draft, initialization);
    MacroScheduleManagerDialog dialog(parent);
    QGroupBox group(QStringLiteral("宏指令列表"), &dialog);
    QTableWidget table(0, initialization ? 1 : 6, &group);
    QLabel usage(&dialog);
    QPushButton add(QStringLiteral("新增"), &dialog);
    QPushButton remove(QStringLiteral("删除"), &dialog);
    QPushButton edit(QStringLiteral("修改"), &dialog);
    QPushButton confirm(initialization ? QStringLiteral("退出") : QStringLiteral("确定"), &dialog);
    QPushButton help(dialogButtonText(QStringLiteral("帮助"), "Help"), &dialog);
    LocalHelpController helpController(&dialog);
    QRect addGeometry;
    QRect removeGeometry;
    QRect editGeometry;
    QRect confirmGeometry;
    QRect helpGeometry;

    dialog.setObjectName(initialization ? QStringLiteral("InitMacroManager") : QStringLiteral("GlobalMacroManager"));
    dialog.setWindowTitle(initialization ? QStringLiteral("初始化宏管理器") : QStringLiteral("全局宏管理器"));
    dialog.setProperty("localHelpTopic", helpTopic);
    dialog.setFixedSize(initialization ? QSize(412, 455) : QSize(662, 462));
    group.setGeometry(10, 10, initialization ? 391 : 641, 401);
    table.setObjectName(QStringLiteral("MacroScheduleTable"));
    table.setGeometry(10, initialization ? 20 : 17, initialization ? 371 : 618, 371);
    table.setSelectionBehavior(QAbstractItemView::SelectRows);
    table.setSelectionMode(QAbstractItemView::SingleSelection);
    table.setEditTriggers(QAbstractItemView::NoEditTriggers);
    table.verticalHeader()->hide();
    table.horizontalHeader()->setFixedHeight(25);
    table.horizontalHeader()->setDefaultSectionSize(100);
    table.horizontalHeader()->setStretchLastSection(initialization);
    table.setStyleSheet(QStringLiteral("QTableWidget {background:#caeace; gridline-color:#dcdcdc;} "
        "QHeaderView::section {background:white; border:0; border-right:1px solid #dcdcdc;}"));
    if (initialization) //20261123-21 add: The initialization count belongs in its single table heading.
    {
        addGeometry = QRect(11, 422, 73, 21);
        removeGeometry = QRect(91, 422, 73, 21);
        editGeometry = QRect(171, 422, 73, 21);
        confirmGeometry = QRect(251, 422, 73, 21);
        helpGeometry = QRect(331, 422, 73, 21);
        usage.hide();
    }
    else
    {
        table.setHorizontalHeaderLabels({QStringLiteral("宏指令名称"), QStringLiteral("频率"),
            QStringLiteral("执行次数"), QStringLiteral("是否受控触发"), QStringLiteral("受控类型"), QStringLiteral("受控地址")});
        addGeometry = QRect(11, 422, 77, 29);
        removeGeometry = QRect(111, 422, 78, 29);
        editGeometry = QRect(211, 422, 78, 29);
        confirmGeometry = QRect(471, 422, 78, 29);
        helpGeometry = QRect(571, 422, 78, 29);
        positionControl(usage, QStringLiteral("MacroScheduleUsage"), QRect(339, 422, 88, 29));
    }
    positionControl(add, QStringLiteral("MacroScheduleAdd"), addGeometry);
    positionControl(remove, QStringLiteral("MacroScheduleRemove"), removeGeometry);
    positionControl(edit, QStringLiteral("MacroScheduleEdit"), editGeometry);
    positionControl(confirm, QStringLiteral("MacroScheduleConfirm"), confirmGeometry);
    positionControl(help, QStringLiteral("MacroScheduleHelp"), helpGeometry);
    add.setDefault(true);
    remove.setAutoDefault(false);
    edit.setAutoDefault(false);
    help.setAutoDefault(false);

    auto refresh = [&](int selected) {
        int rowIndex;
        int column;
        QStringList cells;
        bool triggered;

        rows = macroScheduleRows(draft, initialization);
        table.clearContents();
        table.setRowCount(rows.size());
        for (rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
        {
            const MacroScheduleRow &row = rows.at(rowIndex);

            cells = {draft.macroIds.key(row.macroId)};
            if (!initialization)
            {
                triggered = !row.address.isEmpty();
                cells << QString::number(row.frequency * 100) << QString::number(row.count) //20261123-20 change: The original list displays elapsed milliseconds.
                      << (triggered ? QStringLiteral("是") : QStringLiteral("否"))
                      << (triggered ? QStringLiteral("位地址") : QString()) << row.address;
            }
            for (column = 0; column < cells.size(); ++column)
            {
                table.setItem(rowIndex, column, new QTableWidgetItem(cells.at(column)));
            }
        }
        if (initialization)
        {
            table.setHorizontalHeaderLabels({QStringLiteral("宏指令名称 %1/%2").arg(rows.size()).arg(limit)});
        }
        else
        {
            usage.setText(QStringLiteral("%1/%2").arg(rows.size()).arg(limit));
        }
        if (selected >= 0 && selected < rows.size())
        {
            table.selectRow(selected);
        }
        add.setEnabled(rows.size() < limit);
    };
    auto publishRows = [&](HmiProject &candidate, const QList<MacroScheduleRow> &updated, int selected) {
        QString error;

        // Apply each accepted row to the local project so nested source deletion sees all of its references.
        if (!replaceMacroSchedule(&candidate, initialization, updated, &error))
        {
            QMessageBox::warning(&dialog, QStringLiteral("宏调度未提交"), error);
            return;
        }
        draft = candidate;
        refresh(selected);
    };
    auto changeRow = [&](int index) {
        HmiProject candidate = draft;
        QList<MacroScheduleRow> updated = rows;
        MacroScheduleRow row = index < 0 ? MacroScheduleRow() : rows.at(index);

        if (!editScheduleRow(&dialog, &candidate, &row, initialization))
        {
            return;
        }
        if (index < 0)
        {
            index = updated.size();
            updated.append(row);
        }
        else
        {
            updated[index] = row;
        }
        publishRows(candidate, updated, index);
    };
    QObject::connect(&add, &QPushButton::clicked, &dialog, [&]() { changeRow(-1); });
    QObject::connect(&edit, &QPushButton::clicked, &dialog, [&]() {
        if (!table.selectedItems().isEmpty())
        {
            changeRow(table.currentRow());
        }
    });
    QObject::connect(&table, &QTableWidget::cellDoubleClicked, &dialog, [&](int rowIndex, int) { changeRow(rowIndex); });
    QObject::connect(&remove, &QPushButton::clicked, &dialog, [&]() {
        HmiProject candidate = draft;
        QList<MacroScheduleRow> updated = rows;
        const int index = table.currentRow();

        if (!table.selectedItems().isEmpty() && index >= 0 && index < updated.size())
        {
            updated.removeAt(index);
            publishRows(candidate, updated, qMin(index, int(updated.size()) - 1));
        }
    });
    QObject::connect(&confirm, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(&help, &QPushButton::clicked, &dialog, [&]() { helpController.showTopic(helpTopic, &dialog); });
    refresh(-1);
    if (dialog.exec() != QDialog::Accepted)
    {
        return false;
    }
    *project = draft;
    return true;
}
