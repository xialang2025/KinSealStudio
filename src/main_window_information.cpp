#include "main_window.h"
#include "dialog_button_text.h"
#include <QAction>
#include <QDockWidget>
#include <QHeaderView>
#include <QMenu>
#include <QTableWidget>

/******************************************************************************
 * Function Name   : MainWindow::createInformationDock
 * Description     : Bind the native View action to a hidden, movable output table.
 * Input Parameters: action
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void MainWindow::createInformationDock(QAction *action) //20261125-84 add
{
    m_informationDock = new QDockWidget(QStringLiteral("信息输出"), this);
    m_informationDock->setObjectName(QStringLiteral("InformationOutputDock"));
    m_information = new QTableWidget(m_informationDock);
    m_information->setObjectName(QStringLiteral("InformationOutputTable"));
    m_information->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_information->horizontalHeader()->hide();
    m_information->verticalHeader()->hide();
    m_information->verticalHeader()->setMinimumSectionSize(1);
    m_information->verticalHeader()->setDefaultSectionSize(20);
    m_information->horizontalHeader()->setStretchLastSection(true);
    m_information->setContextMenuPolicy(Qt::CustomContextMenu);
    m_informationDock->setWidget(m_information);
    addDockWidget(Qt::BottomDockWidgetArea, m_informationDock);
    m_informationDock->hide();
    action->setCheckable(true);
    connect(action, &QAction::triggered, m_informationDock, &QDockWidget::setVisible);
    connect(m_informationDock->toggleViewAction(), &QAction::toggled, action, &QAction::setChecked);
    connect(m_information, &QTableWidget::customContextMenuRequested, this, [this](const QPoint &point) {
        if (m_information->rowCount() == 0) { return; }
        QMenu menu(m_information);
        menu.addAction(dialogButtonText(QStringLiteral("清除"), "Clear"), this, &MainWindow::clearInformation);
        menu.exec(m_information->viewport()->mapToGlobal(point));
    });
    connect(m_information, &QTableWidget::cellDoubleClicked, this, [this](int row, int column) {
        QTableWidgetItem *cell = m_information->item(row, column);
        if (cell == nullptr || !m_hasProject) { return; }
        const QString page = cell->data(Qt::UserRole).toString();
        const QString object = cell->data(Qt::UserRole + 1).toString();
        if (!page.isEmpty() && !object.isEmpty())
        {
            // Rows survive edits. Resolve the original identity before changing the active page.
            for (const HmiPage &entry : currentEditorProject().allPages())
            {
                if (entry.id != page) { continue; }
                for (const HmiObject &item : entry.objects)
                {
                    if (item.id == object) { activatePageId(page, object); return; }
                }
                return;
            }
        }
    });
}

/******************************************************************************
 * Function Name   : MainWindow::clearInformation
 * Description     : Destroy both displayed rows and their navigation data.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void MainWindow::clearInformation() //20261125-84 add
{
    m_information->setRowCount(0);
}

/******************************************************************************
 * Function Name   : MainWindow::beginInformation
 * Description     : Replace the previous search or compilation output with the current operation.
 * Input Parameters: text
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void MainWindow::beginInformation(const QString &text) //20261125-84 add
{
    clearInformation();
    m_informationDock->show();
    appendInformation(text, OutputKind::Information);
}

/******************************************************************************
 * Function Name   : MainWindow::appendInformation
 * Description     : Display one read-only message with its original severity and optional location.
 * Input Parameters: text, kind, page, object
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void MainWindow::appendInformation(const QString &text, OutputKind kind, const QString &page,
                                   const QString &object) //20261125-84 add
{
    int row = m_information->rowCount();
    QTableWidgetItem *cell;

    if (text.isEmpty()) { return; }
    if (m_information->columnCount() == 0) { m_information->setColumnCount(1); }
    cell = new QTableWidgetItem(text);
    cell->setBackground(Qt::white);
    if (kind == OutputKind::Error) { cell->setForeground(Qt::red); }
    else if (kind == OutputKind::Warning) { cell->setForeground(QColor(170, 85, 0)); }
    cell->setToolTip(text);
    cell->setData(Qt::UserRole, page);
    cell->setData(Qt::UserRole + 1, object);
    m_information->insertRow(row);
    m_information->setItem(row, 0, cell);
    m_information->setCurrentCell(row, 0);
}
