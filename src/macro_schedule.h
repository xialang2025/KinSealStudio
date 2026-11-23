#ifndef MACRO_SCHEDULE_H
#define MACRO_SCHEDULE_H

#include <QList>
#include <QString>

struct HmiProject;

struct MacroScheduleRow //20261123-20 add: One invocation, separate from its macro definition.
{
    QString id;
    QString macroId;
    QString address;
    int frequency = 1;
    int count = 0;
};

inline bool isProjectMacroSchedule(const QString &kind)
{
    return kind == QStringLiteral("globalMacro") || kind == QStringLiteral("initMacro");
}

QList<MacroScheduleRow> macroScheduleRows(const HmiProject &project, bool initialization);
bool replaceMacroSchedule(HmiProject *project, bool initialization, const QList<MacroScheduleRow> &rows,
                          QString *error = nullptr);

#endif
