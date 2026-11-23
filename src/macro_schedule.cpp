#include "macro_schedule.h"
#include "project_model.h"

#include <QJsonArray>
#include <QUuid>
#include <algorithm>

namespace {

/******************************************************************************
 * Function Name   : scheduleKind
 * Description     : Choose the stored kind for a project-scoped schedule.
 * Input Parameters: initialization
 * Return Value    : The configuration kind.
 * Date            : 2026.11.23
 ******************************************************************************/
QString scheduleKind(bool initialization) //20261123-20 add
{
    return initialization ? QStringLiteral("initMacro") : QStringLiteral("globalMacro");
}

} // namespace

/******************************************************************************
 * Function Name   : macroScheduleRows
 * Description     : Read the invocation list in saved order and restore stable macro targets.
 * Input Parameters: project, initialization
 * Return Value    : The selected schedule's rows.
 * Date            : 2026.11.23
 ******************************************************************************/
QList<MacroScheduleRow> macroScheduleRows(const HmiProject &project, bool initialization) //20261123-20 add
{
    const QString kind = scheduleKind(initialization);
    QList<MacroScheduleRow> result;
    QMap<QString, QString> targets;
    QJsonObject values;
    MacroScheduleRow row;

    for (const HmiConfigurationNode &node : project.configurationNodes)
    {
        if (node.kind != kind)
        {
            continue;
        }
        for (const HmiReference &reference : project.references)
        {
            if (reference.ownerKind == QStringLiteral("configuration") &&
                reference.ownerId == node.id && reference.targetKind == QStringLiteral("macro"))
            {
                targets.insert(reference.field, reference.targetId);
            }
        }
        for (const QJsonValue &value : node.properties.value(QStringLiteral("scripts")).toArray())
        {
            values = value.toObject();
            row.id = values.value(QStringLiteral("id")).toString();
            row.macroId = targets.value(row.id);
            row.frequency = values.value(QStringLiteral("frequency")).toInt(1);
            row.count = values.value(QStringLiteral("count")).toInt();
            row.address = values.value(QStringLiteral("address")).toString();
            result.append(row);
        }
        break;
    }
    return result;
}

/******************************************************************************
 * Function Name   : replaceMacroSchedule
 * Description     : Publish one project-scope schedule with one stable source reference per row.
 * Input Parameters: project, initialization, rows, error
 * Return Value    : True after validation and atomic replacement.
 * Date            : 2026.11.23
 ******************************************************************************/
bool replaceMacroSchedule(HmiProject *project, bool initialization,
                          const QList<MacroScheduleRow> &rows, QString *error) //20261123-20 add
{
    const QString kind = scheduleKind(initialization);
    HmiProject candidate;
    HmiConfigurationNode node;
    QMap<QString, QString> referenceIds;
    QJsonArray scripts;
    QJsonObject values;
    QString id;
    QString binding;
    QList<MacroScheduleRow> previous;
    int position;

    if (project == nullptr)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("缺少工程宏调度草稿。");
        }
        return false;
    }
    previous = macroScheduleRows(*project, initialization);
    if (std::equal(previous.cbegin(), previous.cend(), rows.cbegin(), rows.cend(),
        [](const MacroScheduleRow &left, const MacroScheduleRow &right) {
            return left.id == right.id && left.macroId == right.macroId && left.address == right.address &&
                left.frequency == right.frequency && left.count == right.count;
        }))
    {
        return project->validateModel(error);
    }
    candidate = *project;
    for (position = 0; position < candidate.configurationNodes.size(); ++position)
    {
        if (candidate.configurationNodes.at(position).kind == kind)
        {
            node = candidate.configurationNodes.takeAt(position);
            break;
        }
    }
    if (!node.id.isEmpty())
    {
        candidate.references.removeIf([&](const HmiReference &reference) {
            if (reference.ownerKind != QStringLiteral("configuration") || reference.ownerId != node.id)
            {
                return false;
            }
            referenceIds.insert(reference.field, reference.id);
            return true;
        });
    }
    if (!rows.isEmpty())
    {
        if (node.id.isEmpty())
        {
            node.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        }
        node.kind = kind;
        node.name = initialization ? QStringLiteral("初始化宏") : QStringLiteral("全局宏");
        node.implemented = true;
        node.active = true;
        for (const MacroScheduleRow &row : rows)
        {
            id = row.id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : row.id;
            values = {{QStringLiteral("id"), id}};
            if (!initialization)
            {
                values.insert(QStringLiteral("frequency"), row.frequency);
                values.insert(QStringLiteral("count"), row.count);
                values.insert(QStringLiteral("address"), row.address);
            }
            scripts.append(values);
            binding = referenceIds.value(id);
            if (binding.isEmpty())
            {
                binding = QUuid::createUuid().toString(QUuid::WithoutBraces);
            }
            candidate.references.append({binding, QStringLiteral("configuration"), node.id,
                                         id, QStringLiteral("macro"), row.macroId});
        }
        node.properties = {{QStringLiteral("scripts"), scripts}};
        candidate.configurationNodes.insert(position, node);
    }
    if (!candidate.validateModel(error))
    {
        return false;
    }
    *project = candidate;
    return true;
}
