#include "stage2_global_tools.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QFormLayout>

namespace Stage2GlobalTools
{
namespace
{
/******************************************************************************
 * Function Name   : fail
 * Description     : Report a transaction error without publishing any draft.
 * Input Parameters: error, message
 * Return Value    : False
 * Date            : 2026.11.10
 ******************************************************************************/
bool fail(QString *error, const QString &message) //20261110-7 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : fingerprint
 * Description     : Bind a preview to all persisted project fields, not list positions.
 * Input Parameters: project
 * Return Value    : SHA256 of the canonical project JSON
 * Date            : 2026.11.10
 ******************************************************************************/
QByteArray fingerprint(const HmiProject &project) //20261110-7 add
{
    return QCryptographicHash::hash(QJsonDocument(project.toJson()).toJson(QJsonDocument::Compact),
                                    QCryptographicHash::Sha256);
}

/******************************************************************************
 * Function Name   : includesObject
 * Description     : Apply the requested stable page and object scope.
 * Input Parameters: scope, pageId, objectId
 * Return Value    : True when the object is within the requested range
 * Date            : 2026.11.10
 ******************************************************************************/
bool includesObject(const Scope &scope, const QString &pageId, const QString &objectId) //20261110-7 add
{
    if (scope.range == Scope::Range::AllPages) { return true; }
    if (pageId != scope.pageId) { return false; }
    return scope.range == Scope::Range::CurrentPage || scope.objectIds.contains(objectId);
}

/******************************************************************************
 * Function Name   : objectRow
 * Description     : Capture navigation identities independently of current page indexes.
 * Input Parameters: page, object, field, label, value
 * Return Value    : One immutable result row
 * Date            : 2026.11.10
 ******************************************************************************/
Row objectRow(const HmiPage &page, const HmiObject &object, const QString &field,
              const QString &label, const QString &value) //20261110-7 add
{
    Row row;

    row.location.pageId = page.id; row.location.objectId = object.id;
    row.pageName = page.name; row.objectName = object.name; row.type = object.type;
    row.fieldPath = field; row.label = label; row.value = value;
    return row;
}

/******************************************************************************
 * Function Name   : appendAddress
 * Description     : Describe the role and full transfer width of one known address field.
 * Input Parameters: rows, page, object, field, label, value, role, count, active
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void appendAddress(QList<Row> *rows, const HmiPage &page, const HmiObject &object,
                   const QString &field, const QString &label, const QString &value,
                   PlcAddressRole role, u32 count, bool active) //20261110-7 add
{
    Row row;

    row = objectRow(page, object, field, label, value);
    row.role = role; row.transferCount = count; row.active = active;
    row.replaceable = true;
    rows->append(row);
}

/******************************************************************************
 * Function Name   : conditionAddresses
 * Description     : Separate dormant drafts from active bit and numeric conditions.
 * Input Parameters: rows, page, object, prefix, label, properties, allowed
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void conditionAddresses(QList<Row> *rows, const HmiPage &page, const HmiObject &object,
                         const QString &prefix, const QString &label,
                         const StaticTextProperties &properties, bool allowed = true) //20261110-7 add
{
    u32 count;

    count = properties.dataType == 1 || properties.dataType == 3 || properties.dataType == 4 ? 2 : 1;
    appendAddress(rows, page, object, prefix + QStringLiteral(".bitAddress"), label + QStringLiteral("·位条件"),
                   properties.bitAddress, PlcAddressRole::Bit, 1,
                   allowed && properties.visibilityEnabled && properties.bitEnabled);
    appendAddress(rows, page, object, prefix + QStringLiteral(".wordAddress"), label + QStringLiteral("·字条件"),
                   properties.wordAddress, PlcAddressRole::Word, count,
                   allowed && properties.visibilityEnabled && properties.wordEnabled);
}

/******************************************************************************
 * Function Name   : unknownStrings
 * Description     : Expose untyped string leaves as read-only so exact matches cannot be rewritten.
 * Input Parameters: rows, base, value, path
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void unknownStrings(QList<Row> *rows, const Row &base, const QJsonValue &value,
                     const QString &path) //20261110-7 add
{
    QList<QPair<QString, QJsonValue>> pending;
    QPair<QString, QJsonValue> item;
    QJsonObject object;
    QJsonObject::const_iterator field;
    QJsonArray array;
    Row row;
    int index;

    // An iterative traversal also reports deeply nested unknown strings instead of silently dropping them.
    pending.append(qMakePair(path, value));
    while (!pending.isEmpty())
    {
        item = pending.takeLast();
        if (item.second.isString())
        {
            row = base; row.fieldPath = item.first; row.value = item.second.toString(); row.replaceable = false;
            row.label = QStringLiteral("未核实字段（只读）");
            row.diagnostic = QStringLiteral("此字段没有已核实的地址用途和跨度，不能批量替换。");
            rows->append(row);
        }
        else if (item.second.isObject())
        {
            object = item.second.toObject();
            for (field = object.constBegin(); field != object.constEnd(); ++field)
            {
                pending.append(qMakePair(item.first + QLatin1Char('.') + field.key(), field.value()));
            }
        }
        else if (item.second.isArray())
        {
            array = item.second.toArray();
            for (index = 0; index < array.size(); ++index)
            {
                pending.append(qMakePair(item.first + QStringLiteral("[%1]").arg(index), array.at(index)));
            }
        }
    }
}

/******************************************************************************
 * Function Name   : objectAddresses
 * Description     : Enumerate five supported control families without touching macro source text.
 * Input Parameters: page, object
 * Return Value    : Known address rows plus explicit read-only unknown fields
 * Date            : 2026.11.10
 ******************************************************************************/
QList<Row> objectAddresses(const HmiPage &page, const HmiObject &object) //20261110-7 add
{
    QList<Row> rows;
    Row row;
    const BitButtonProperties *shared;
    bool bit;
    bool dual;
    bool independent;
    u32 count;

    shared = &object.bitButton;
    bit = object.hasBitButtonProperties && !object.hasWordButtonProperties &&
          !object.hasLampProperties && !object.hasFunctionButtonProperties;
    if (bit || object.hasWordButtonProperties)
    {
        appendAddress(&rows, page, object, QStringLiteral("address"), QStringLiteral("操作地址"), object.address,
                       bit ? PlcAddressRole::Bit : PlcAddressRole::Word,
                       bit ? 1 : u32(object.wordButton.transferWordCount()), true);
    }
    else if (!object.address.isEmpty())
    {
        row = objectRow(page, object, QStringLiteral("address"), QStringLiteral("未核实通用地址"), object.address);
        row.diagnostic = QStringLiteral("此控件没有已核实的操作地址；不能按名称猜测用途。");
        rows.append(row);
    }
    if (object.hasStaticTextProperties)
    {
        conditionAddresses(&rows, page, object, QStringLiteral("staticText"), QStringLiteral("显现"), object.staticText);
    }
    if (object.hasBitButtonProperties)
    {
        dual = object.hasLampProperties ? object.lamp.monitorMode == 1 : bit && shared->indicator && shared->dualMonitor;
        independent = object.hasLampProperties || !shared->monitorSame;
        if (bit && dual && shared->monitorSame)
        {
            // The effective same-address monitor reads the operation field; its dormant independent draft stays intact.
            appendAddress(&rows, page, object, QStringLiteral("address"), QStringLiteral("监视（与操作地址相同）"),
                           object.address, PlcAddressRole::Bit, 1, true);
        }
        appendAddress(&rows, page, object, QStringLiteral("bitButton.monitorBitAddress"), QStringLiteral("独立位监视"),
                       shared->monitorBitAddress, PlcAddressRole::Bit, 1, dual && independent && shared->monitorType == 0);
        appendAddress(&rows, page, object, QStringLiteral("bitButton.monitorWordAddress"), QStringLiteral("字中位监视"),
                       shared->monitorWordAddress, PlcAddressRole::Word, 1, dual && independent && shared->monitorType == 1);
        if (object.hasLampProperties)
        {
            appendAddress(&rows, page, object, QStringLiteral("lamp.multiMonitorAddress"), QStringLiteral("多态监视"),
                           object.lamp.multiMonitorAddress, PlcAddressRole::Word,
                           u32(object.lamp.transferWordCount()), object.lamp.monitorMode == 2);
        }
        conditionAddresses(&rows, page, object, QStringLiteral("bitButton.visibility"), QStringLiteral("显现"), shared->visibility);
        conditionAddresses(&rows, page, object, QStringLiteral("bitButton.touch"), QStringLiteral("触控"), shared->touch, !object.hasLampProperties);
        appendAddress(&rows, page, object, QStringLiteral("bitButton.noticeBitAddress"), QStringLiteral("位通知"),
                       shared->noticeBitAddress, PlcAddressRole::Bit, 1, !object.hasLampProperties && shared->notice && shared->noticeBit);
        count = shared->noticeWordType == 1 || shared->noticeWordType == 3 || shared->noticeWordType == 4 ? 2 : 1;
        appendAddress(&rows, page, object, QStringLiteral("bitButton.noticeWordAddress"), QStringLiteral("字通知"),
                       shared->noticeWordAddress, PlcAddressRole::Word, count, !object.hasLampProperties && shared->notice && shared->noticeWord);
    }
    row = objectRow(page, object, QString(), QString(), QString());
    row.active = false;
    unknownStrings(&rows, row, object.extensions, QStringLiteral("extensions"));
    return rows;
}

/******************************************************************************
 * Function Name   : appendTexts
 * Description     : Retain language and state ownership in the read-only text index.
 * Input Parameters: rows, base, texts, languages, fallback
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void appendTexts(QList<Row> *rows, const Row &base, const QMap<QString, QString> &texts,
                  const QStringList &languages, const QString &fallback) //20261110-7 add
{
    QStringList keys;
    QString language;
    Row row;

    keys = languages;
    Q_FOREACH (language, texts.keys())
    {
        if (!keys.contains(language)) { keys.append(language); }
    }
    Q_FOREACH (language, keys)
    {
        row = base; row.language = language; row.value = texts.value(language, fallback);
        rows->append(row);
    }
}

/******************************************************************************
 * Function Name   : objectTexts
 * Description     : List independent translations and artwork states without flattening them.
 * Input Parameters: project, page, object
 * Return Value    : Read-only text records
 * Date            : 2026.11.10
 ******************************************************************************/
QList<Row> objectTexts(const HmiProject &project, const HmiPage &page, const HmiObject &object) //20261110-7 add
{
    QList<Row> rows;
    Row row;
    const BitButtonState *state;
    int index;
    int count;

    if (object.hasStaticTextProperties)
    {
        row = objectRow(page, object, QStringLiteral("staticText.languageTexts"), QStringLiteral("文本"), QString());
        appendTexts(&rows, row, object.staticText.languageTexts, project.languages, object.text);
    }
    else if (object.hasBitButtonProperties)
    {
        count = object.hasLampProperties ? object.lamp.stateCount() : 2;
        for (index = 0; index < count; ++index)
        {
            state = object.hasLampProperties ? object.lamp.state(object.bitButton, index) : &object.bitButton.states[index];
            if (state == nullptr) { continue; }
            row = objectRow(page, object, QStringLiteral("states[%1].text").arg(index), QStringLiteral("状态%1文本").arg(index), QString());
            row.active = index == 0 || (object.hasLampProperties && index < object.lamp.stateCount()) ||
                         (object.hasWordButtonProperties ? object.wordButton.pressedEffect :
                          object.hasFunctionButtonProperties ? object.functionButton.pressedEffect : object.bitButton.indicator && object.bitButton.dualMonitor);
            if (state->sameLanguage)
            {
                row.language = QStringLiteral("所有语言"); row.value = state->sharedText;
                rows.append(row);
            }
            else { appendTexts(&rows, row, state->text.languageTexts, project.languages, object.text); }
        }
        if (object.hasFunctionButtonProperties && !object.functionButton.keyboardText.isEmpty())
        {
            rows.append(objectRow(page, object, QStringLiteral("functionButton.keyboardText"), QStringLiteral("键盘字符"), object.functionButton.keyboardText));
        }
    }
    else { rows.append(objectRow(page, object, QStringLiteral("text"), QStringLiteral("通用文本"), object.text)); }
    return rows;
}

/******************************************************************************
 * Function Name   : rowLocation
 * Description     : Make unsafe-field and span errors actionable at the actual control.
 * Input Parameters: row
 * Return Value    : Human-readable stable location
 * Date            : 2026.11.10
 ******************************************************************************/
QString rowLocation(const Row &row) //20261110-7 add
{
    return QStringLiteral("画面%1[%2] 控件%3[%4] 字段%5").arg(row.pageName, row.location.pageId,
        row.objectName, row.location.objectId, row.fieldPath);
}

/******************************************************************************
 * Function Name   : fieldIdentity
 * Description     : Encode compound stable IDs without delimiter collisions.
 * Input Parameters: location, field
 * Return Value    : Unambiguous JSON-array key
 * Date            : 2026.11.10
 ******************************************************************************/
QString fieldIdentity(const Location &location, const QString &field) //20261110-7 add
{
    return QString::fromUtf8(QJsonDocument(QJsonArray{location.pageId, location.objectId, field}).toJson(QJsonDocument::Compact));
}

/******************************************************************************
 * Function Name   : parseRowAddress
 * Description     : Validate the entire typed transfer against the chosen protocol family.
 * Input Parameters: project, catalog, row, text, parsed, error
 * Return Value    : True for a supported role and complete address span
 * Date            : 2026.11.10
 ******************************************************************************/
bool parseRowAddress(const HmiProject &project, const PlcCatalog &catalog, const Row &row,
                      const QString &text, PlcAddress *parsed, QString *error) //20261110-7 add
{
    PlcAddressContext context;
    QString detail;

    context.connections = project.connections; context.role = row.role; context.transferCount = row.transferCount;
    if (!parseQualifiedPlcAddressText(text, context, catalog, parsed, &detail))
    {
        return fail(error, rowLocation(row) + QStringLiteral("：") + detail);
    }
    return true;
}

/******************************************************************************
 * Function Name   : sameAddress
 * Description     : Match a complete address, including its explicit connection identity.
 * Input Parameters: project, catalog, row, find
 * Return Value    : True for exact text or equal parsed whole addresses
 * Date            : 2026.11.10
 ******************************************************************************/
bool sameAddress(const HmiProject &project, const PlcCatalog &catalog, const Row &row,
                  const QString &find) //20261110-7 add
{
    PlcAddress left;
    PlcAddress right;
    PlcAddressContext context;

    if (row.value.trimmed() == find.trimmed()) { return true; }
    if (!row.replaceable) { return false; }
    context.connections = project.connections; context.role = row.role; context.transferCount = 1;
    return parseQualifiedPlcAddressText(row.value, context, catalog, &left) &&
           parseQualifiedPlcAddressText(find, context, catalog, &right) &&
           plcQualifiedAddressText(left) == plcQualifiedAddressText(right);
}

/******************************************************************************
 * Function Name   : mutableObject
 * Description     : Resolve an editable object by stable page and object IDs.
 * Input Parameters: project, location
 * Return Value    : Matching object pointer or nullptr
 * Date            : 2026.11.10
 ******************************************************************************/
HmiObject *mutableObject(HmiProject *project, const Location &location) //20261110-7 add
{
    QList<HmiObject> *objects;
    int page;
    int object;

    objects = nullptr;
    if (location.pageId == project->activePageId) { objects = &project->objects; }
    else
    {
        for (page = 0; page < project->inactivePages.size(); ++page)
        {
            if (project->inactivePages[page].id == location.pageId) { objects = &project->inactivePages[page].objects; break; }
        }
    }
    if (objects == nullptr) { return nullptr; }
    for (object = 0; object < objects->size(); ++object)
    {
        if ((*objects)[object].id == location.objectId) { return &(*objects)[object]; }
    }
    return nullptr;
}

/******************************************************************************
 * Function Name   : assignAddress
 * Description     : Update only named model address fields; never edit JSON or macro strings.
 * Input Parameters: object, field, value
 * Return Value    : True after assigning a supported field
 * Date            : 2026.11.10
 ******************************************************************************/
bool assignAddress(HmiObject *object, const QString &field, const QString &value) //20261110-7 add
{
    StaticTextProperties *condition;
    QString suffix;

    if (object == nullptr) { return false; }
    if (field == QStringLiteral("address")) { object->address = value; return true; }
    if (field == QStringLiteral("bitButton.monitorBitAddress")) { object->bitButton.monitorBitAddress = value; return true; }
    if (field == QStringLiteral("bitButton.monitorWordAddress")) { object->bitButton.monitorWordAddress = value; return true; }
    if (field == QStringLiteral("bitButton.noticeBitAddress")) { object->bitButton.noticeBitAddress = value; return true; }
    if (field == QStringLiteral("bitButton.noticeWordAddress")) { object->bitButton.noticeWordAddress = value; return true; }
    if (field == QStringLiteral("lamp.multiMonitorAddress")) { object->lamp.multiMonitorAddress = value; return true; }
    condition = nullptr;
    if (field.startsWith(QStringLiteral("staticText."))) { condition = &object->staticText; suffix = field.mid(11); }
    else if (field.startsWith(QStringLiteral("bitButton.visibility."))) { condition = &object->bitButton.visibility; suffix = field.mid(21); }
    else if (field.startsWith(QStringLiteral("bitButton.touch."))) { condition = &object->bitButton.touch; suffix = field.mid(16); }
    if (condition == nullptr) { return false; }
    if (suffix == QStringLiteral("bitAddress")) { condition->bitAddress = value; return true; }
    if (suffix == QStringLiteral("wordAddress")) { condition->wordAddress = value; return true; }
    return false;
}

/******************************************************************************
 * Function Name   : dialogScope
 * Description     : Interpret common range controls using current stable page identity.
 * Input Parameters: project, combo, selected, dormant
 * Return Value    : Explicit search or replacement range
 * Date            : 2026.11.10
 ******************************************************************************/
Scope dialogScope(const HmiProject &project, const QComboBox *combo,
                    const QSet<QString> &selected, bool dormant) //20261110-7 add
{
    Scope scope;

    scope.range = Scope::Range(combo->currentIndex()); scope.pageId = project.activePageId;
    scope.objectIds = selected; scope.includeDormant = dormant;
    return scope;
}

/******************************************************************************
 * Function Name   : configureTable
 * Description     : Keep result indexes read-only and select complete navigation rows.
 * Input Parameters: table, headers
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void configureTable(QTableWidget *table, const QStringList &headers) //20261110-7 add
{
    table->setColumnCount(headers.size()); table->setHorizontalHeaderLabels(headers);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->horizontalHeader()->setStretchLastSection(true);
}
}

/******************************************************************************
 * Function Name   : enumerate
 * Description     : Build read-only global indexes without selecting a page or altering models.
 * Input Parameters: project, kind, scope
 * Return Value    : Result rows retaining stable navigation identity and field ownership
 * Date            : 2026.11.10
 ******************************************************************************/
QList<Row> enumerate(const HmiProject &project, Kind kind, const Scope &scope) //20261110-7 add
{
    QList<Row> rows;
    QList<Row> objectRows;
    HmiPage page;
    HmiObject object;
    HmiConfigurationNode node;
    Row row;
    QSet<QString> scopedMacroNames;
    QMap<QString, QString>::const_iterator macro;

    Q_FOREACH (page, project.allPages())
    {
        Q_FOREACH (object, page.objects)
        {
            if (!includesObject(scope, page.id, object.id)) { continue; }
            objectRows.clear();
            if (kind == Kind::Controls)
            {
                row = objectRow(page, object, QStringLiteral("object"), QStringLiteral("控件"), object.text);
                row.type = object.hasLampProperties ? QStringLiteral("指示灯") : object.hasWordButtonProperties ? QStringLiteral("字按钮") :
                    object.hasFunctionButtonProperties ? QStringLiteral("功能按钮") : object.hasBitButtonProperties ? QStringLiteral("位按钮") :
                    object.hasStaticTextProperties ? QStringLiteral("静态文本") : object.type;
                objectRows.append(row);
            }
            else if (kind == Kind::Texts) { objectRows = objectTexts(project, page, object); }
            else if (kind == Kind::Addresses) { objectRows = objectAddresses(page, object); }
            else if (kind == Kind::Macros && object.hasBitButtonProperties && !object.bitButton.macroName.isEmpty())
            {
                row = objectRow(page, object, QStringLiteral("bitButton.macroName"), QStringLiteral("控件宏引用"), object.bitButton.macroName);
                row.location.macroName = object.bitButton.macroName;
                row.location.macroId = project.macroIds.value(row.location.macroName);
                row.active = object.bitButton.macroEnabled; objectRows.append(row);
                if (row.active || scope.includeDormant) { scopedMacroNames.insert(row.location.macroName); }
            }
            Q_FOREACH (row, objectRows)
            {
                if (row.active || scope.includeDormant) { rows.append(row); }
            }
        }
    }
    if (kind == Kind::Macros)
    {
        for (macro = project.macros.constBegin(); macro != project.macros.constEnd(); ++macro)
        {
            if (scope.range != Scope::Range::AllPages && !scopedMacroNames.contains(macro.key())) { continue; }
            row = Row(); row.location.macroName = macro.key(); row.location.macroId = project.macroIds.value(macro.key());
            row.objectName = macro.key(); row.fieldPath = QStringLiteral("macros.source");
            row.label = QStringLiteral("宏源码（只读）"); row.value = macro.value(); rows.append(row);
        }
    }
    if (kind == Kind::Addresses)
    {
        Q_FOREACH (node, project.configurationNodes)
        {
            if (!includesObject(scope, node.ownerPageId, QString())) { continue; }
            if (!node.active && !scope.includeDormant) { continue; }
            row = Row(); row.location.pageId = node.ownerPageId; row.objectName = node.name; row.active = node.active;
            unknownStrings(&rows, row, node.properties, QStringLiteral("configuration[%1].properties").arg(node.id));
        }
    }
    return rows;
}

/******************************************************************************
 * Function Name   : search
 * Description     : Filter indexes and report macro source line numbers without modifying code.
 * Input Parameters: project, kind, needle, scope, sensitivity
 * Return Value    : Read-only matching rows
 * Date            : 2026.11.10
 ******************************************************************************/
QList<Row> search(const HmiProject &project, Kind kind, const QString &needle,
                  const Scope &scope, Qt::CaseSensitivity sensitivity) //20261110-7 add
{
    QList<Row> rows;
    QList<Row> result;
    Row row;
    Row match;
    QStringList lines;
    int index;

    rows = enumerate(project, kind, scope);
    Q_FOREACH (row, rows)
    {
        if (needle.isEmpty()) { result.append(row); continue; }
        if (kind == Kind::Macros && row.fieldPath == QStringLiteral("macros.source"))
        {
            if (row.location.macroName.contains(needle, sensitivity))
            {
                match = row; match.value = row.location.macroName; match.fieldPath = QStringLiteral("macros.name"); result.append(match);
            }
            lines = row.value.split(QLatin1Char('\n'));
            for (index = 0; index < lines.size(); ++index)
            {
                if (lines[index].contains(needle, sensitivity))
                {
                    match = row; match.value = lines[index]; match.location.line = index + 1; result.append(match);
                }
            }
        }
        else if (row.value.contains(needle, sensitivity) || row.objectName.contains(needle, sensitivity) ||
                 row.pageName.contains(needle, sensitivity) || row.label.contains(needle, sensitivity)) { result.append(row); }
    }
    return result;
}

/******************************************************************************
 * Function Name   : previewAddressReplacement
 * Description     : Validate every exact match before publishing one atomic replacement candidate.
 * Input Parameters: project, findAddress, replacementAddress, scope, candidate, error
 * Return Value    : True with a complete candidate; output is unchanged on any error
 * Date            : 2026.11.10
 ******************************************************************************/
bool previewAddressReplacement(const HmiProject &project, const QString &findAddress,
                                const QString &replacementAddress, const Scope &scope,
                                AddressCandidate *candidate, QString *error) //20261110-7 add
{
    AddressCandidate draft;
    AddressChange change;
    Scope fullScope;
    QList<Row> rows;
    Row row;
    PlcCatalog catalog;
    PlcAddress oldAddress;
    PlcAddress newAddress;
    QSet<QString> changedFields;
    QString key;
    QString detail;

    if (candidate == nullptr || findAddress.trimmed().isEmpty() || replacementAddress.trimmed().isEmpty())
    {
        return fail(error, QStringLiteral("查找地址、替换地址和候选接收位置不能为空。"));
    }
    if (!project.connections.isEmpty() && !catalog.load(QStringLiteral(":/plc/plc-catalog.json"), &detail))
    {
        return fail(error, QStringLiteral("PLC目录不可用：") + detail);
    }
    fullScope = scope; fullScope.includeDormant = true; rows = enumerate(project, Kind::Addresses, fullScope);
    draft.projectFingerprint = fingerprint(project);
    Q_FOREACH (row, rows)
    {
        if (!sameAddress(project, catalog, row, findAddress)) { continue; }
        if (!row.active && !scope.includeDormant) { ++draft.excludedDormantMatches; continue; }
        if (!row.replaceable) { return fail(error, rowLocation(row) + QStringLiteral("：") + row.diagnostic); }
        if (!parseRowAddress(project, catalog, row, row.value, &oldAddress, error) ||
            !parseRowAddress(project, catalog, row, replacementAddress, &newAddress, error)) { return false; }
        if (plcQualifiedAddressText(oldAddress) == plcQualifiedAddressText(newAddress)) { continue; }
        key = fieldIdentity(row.location, row.fieldPath);
        if (changedFields.contains(key)) { continue; } // Same-address monitoring aliases the operation field.
        changedFields.insert(key); change = AddressChange(); change.location = row.location;
        change.fieldPath = row.fieldPath; change.before = row.value; change.after = plcQualifiedAddressText(newAddress);
        draft.changes.append(change);
    }
    if (draft.changes.isEmpty())
    {
        return fail(error, QStringLiteral("所选范围没有需要改变的匹配活动地址；排除的休眠字段%1项。").arg(draft.excludedDormantMatches));
    }
    *candidate = draft;
    return true;
}

/******************************************************************************
 * Function Name   : applyCandidate
 * Description     : Revalidate a preview against its source and publish a complete project once.
 * Input Parameters: project, candidate, output, error
 * Return Value    : True with a validated draft; project and output remain intact on failure
 * Date            : 2026.11.10
 ******************************************************************************/
bool applyCandidate(const HmiProject &project, const AddressCandidate &candidate,
                     HmiProject *output, QString *error) //20261110-7 add
{
    HmiProject draft;
    PlcCatalog catalog;
    Scope scope;
    QList<Row> rows;
    Row row;
    Row matched;
    AddressChange change;
    PlcAddress parsed;
    QSet<QString> changedFields;
    QString key;
    QString detail;
    bool found;

    if (output == nullptr || candidate.changes.isEmpty() || candidate.changes.size() > 20000 ||
        candidate.projectFingerprint != fingerprint(project))
    {
        return fail(error, QStringLiteral("替换预览为空、已过期或工程已改变，请重新预览。"));
    }
    if (!project.connections.isEmpty() && !catalog.load(QStringLiteral(":/plc/plc-catalog.json"), &detail))
    {
        return fail(error, QStringLiteral("PLC目录不可用：") + detail);
    }
    scope.includeDormant = true; rows = enumerate(project, Kind::Addresses, scope); draft = project;
    Q_FOREACH (change, candidate.changes)
    {
        found = false;
        Q_FOREACH (row, rows)
        {
            if (row.location.pageId == change.location.pageId && row.location.objectId == change.location.objectId &&
                row.fieldPath == change.fieldPath && row.value == change.before && row.replaceable)
            {
                matched = row; found = true; break;
            }
        }
        key = fieldIdentity(change.location, change.fieldPath);
        if (!found || changedFields.contains(key)) { return fail(error, QStringLiteral("替换目标不存在、重复或字段不可安全编辑：") + key); }
        if (!parseRowAddress(project, catalog, matched, change.before, &parsed, error) ||
            !parseRowAddress(project, catalog, matched, change.after, &parsed, error)) { return false; }
        changedFields.insert(key);
        if (!assignAddress(mutableObject(&draft, change.location), change.fieldPath, plcQualifiedAddressText(parsed)))
        {
            return fail(error, rowLocation(matched) + QStringLiteral("：字段没有安全赋值规则。"));
        }
    }
    if (!draft.validateModel(&detail)) { return fail(error, QStringLiteral("替换后的完整工程验证失败：") + detail); }
    *output = draft;
    return true;
}

/******************************************************************************
 * Function Name   : GlobalToolsDialog::GlobalToolsDialog
 * Description     : Present original global tool entry names over a read-only project copy.
 * Input Parameters: project, kind, parent
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
GlobalToolsDialog::GlobalToolsDialog(const HmiProject &project, Kind kind, QWidget *parent) //20261110-7 add
    : QDialog(parent), m_project(project), m_kind(kind)
{
    QVBoxLayout *layout;
    QFormLayout *form;
    QDialogButtonBox *buttons;
    QPushButton *locateButton;
    QStringList titles;

    titles = {QStringLiteral("控件一览表"), QStringLiteral("文本一览表"), QStringLiteral("地址一览表"), QStringLiteral("宏查找")};
    setObjectName(QStringLiteral("Stage2GlobalToolsDialog")); setWindowTitle(titles[int(kind)]); resize(940, 520);
    layout = new QVBoxLayout(this); form = new QFormLayout();
    m_filter = new QLineEdit(this); m_filter->setObjectName(QStringLiteral("Stage2GlobalFilter"));
    m_scope = new QComboBox(this); m_scope->setObjectName(QStringLiteral("Stage2GlobalScope"));
    m_scope->addItems({QStringLiteral("全部画面"), QStringLiteral("当前画面"), QStringLiteral("选中控件")});
    form->addRow(QStringLiteral("查找:"), m_filter); form->addRow(QStringLiteral("范围:"), m_scope); layout->addLayout(form);
    m_table = new QTableWidget(this); m_table->setObjectName(QStringLiteral("Stage2GlobalResults"));
    configureTable(m_table, {QStringLiteral("画面"), QStringLiteral("控件/宏"), QStringLiteral("类型/字段"), QStringLiteral("语言/状态"), QStringLiteral("内容"), QStringLiteral("活动/休眠")});
    layout->addWidget(m_table, 1); m_status = new QLabel(this); m_status->setObjectName(QStringLiteral("Stage2GlobalStatus")); layout->addWidget(m_status);
    buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    buttons->button(QDialogButtonBox::Close)->setText(QStringLiteral("关闭"));
    locateButton = buttons->addButton(QStringLiteral("定位"), QDialogButtonBox::ActionRole);
    locateButton->setObjectName(QStringLiteral("Stage2GlobalLocate")); layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(locateButton, &QPushButton::clicked, this, [this]() { locate(); });
    connect(m_filter, &QLineEdit::textChanged, this, [this]() { refresh(); });
    connect(m_scope, &QComboBox::currentIndexChanged, this, [this]() { refresh(); });
    connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int, int) { locate(); });
    refresh();
}

/******************************************************************************
 * Function Name   : GlobalToolsDialog::refresh
 * Description     : Refresh a read-only view; editor page and selection remain unchanged.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void GlobalToolsDialog::refresh() //20261110-7 add
{
    Row row;
    QStringList cells;
    int index;
    int column;

    m_rows = search(m_project, m_kind, m_filter->text(), dialogScope(m_project, m_scope, m_selectedObjectIds, true));
    m_table->setRowCount(m_rows.size());
    for (index = 0; index < m_rows.size(); ++index)
    {
        row = m_rows[index]; cells = {row.pageName, row.objectName, row.type + QStringLiteral(" / ") + row.label,
            row.language + (row.location.line > 0 ? QStringLiteral(" 行%1").arg(row.location.line) : QString()), row.value,
            row.active ? QStringLiteral("活动") : QStringLiteral("休眠")};
        for (column = 0; column < cells.size(); ++column)
        {
            m_table->setItem(index, column, new QTableWidgetItem(cells[column]));
            m_table->item(index, column)->setToolTip(row.diagnostic.isEmpty() ? row.fieldPath : row.diagnostic);
        }
    }
    m_status->setText(QStringLiteral("%1项；只读查看，定位后返回对应画面或宏。").arg(m_rows.size()));
}

/******************************************************************************
 * Function Name   : GlobalToolsDialog::locate
 * Description     : Return the selected stable IDs without modifying the project.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void GlobalToolsDialog::locate() //20261110-7 add
{
    int index;

    index = m_table->currentRow();
    if (index < 0 || index >= m_rows.size()) { m_status->setText(QStringLiteral("请先选择一条结果。")); return; }
    m_selected = m_rows[index].location; QDialog::accept();
}

/******************************************************************************
 * Function Name   : GlobalToolsDialog::selectedLocation
 * Description     : Expose accepted navigation without using list row numbers as identities.
 * Input Parameters: None
 * Return Value    : Selected location or an empty location after cancellation
 * Date            : 2026.11.10
 ******************************************************************************/
Location GlobalToolsDialog::selectedLocation() const //20261110-7 add
{
    return result() == QDialog::Accepted ? m_selected : Location();
}

/******************************************************************************
 * Function Name   : GlobalToolsDialog::setSelectedObjectIds
 * Description     : Supply the caller's current selection for the selected-control range.
 * Input Parameters: ids
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void GlobalToolsDialog::setSelectedObjectIds(const QSet<QString> &ids) //20261110-7 add
{
    m_selectedObjectIds = ids; refresh();
}

/******************************************************************************
 * Function Name   : AddressReplaceDialog::AddressReplaceDialog
 * Description     : Require an explicit preview and confirmation before returning any project.
 * Input Parameters: project, parent
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
AddressReplaceDialog::AddressReplaceDialog(const HmiProject &project, QWidget *parent) //20261110-7 add
    : QDialog(parent), m_project(project)
{
    QVBoxLayout *layout;
    QFormLayout *form;
    QPushButton *previewButton;
    QDialogButtonBox *buttons;

    setObjectName(QStringLiteral("Stage2AddressReplaceDialog")); setWindowTitle(QStringLiteral("地址查找替换")); resize(940, 520);
    layout = new QVBoxLayout(this); form = new QFormLayout();
    m_find = new QLineEdit(this); m_find->setObjectName(QStringLiteral("Stage2AddressFind"));
    m_replacement = new QLineEdit(this); m_replacement->setObjectName(QStringLiteral("Stage2AddressReplacement"));
    m_scope = new QComboBox(this); m_scope->setObjectName(QStringLiteral("Stage2AddressScope"));
    m_scope->addItems({QStringLiteral("全部画面"), QStringLiteral("当前画面"), QStringLiteral("选中控件")});
    m_dormant = new QCheckBox(QStringLiteral("包括休眠地址草稿"), this); m_dormant->setObjectName(QStringLiteral("Stage2AddressDormant"));
    form->addRow(QStringLiteral("完整查找地址:"), m_find); form->addRow(QStringLiteral("替换为:"), m_replacement);
    form->addRow(QStringLiteral("范围:"), m_scope); form->addRow(QString(), m_dormant); layout->addLayout(form);
    previewButton = new QPushButton(QStringLiteral("预览"), this); previewButton->setObjectName(QStringLiteral("Stage2AddressPreview")); layout->addWidget(previewButton);
    m_table = new QTableWidget(this); m_table->setObjectName(QStringLiteral("Stage2AddressPreviewTable"));
    configureTable(m_table, {QStringLiteral("画面ID"), QStringLiteral("控件ID"), QStringLiteral("字段"), QStringLiteral("原地址"), QStringLiteral("新地址")}); layout->addWidget(m_table, 1);
    m_status = new QLabel(QStringLiteral("仅替换完整地址；宏源码和未核实字段不会被修改。"), this);
    m_status->setObjectName(QStringLiteral("Stage2AddressStatus")); m_status->setWordWrap(true); layout->addWidget(m_status);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this); layout->addWidget(buttons);
    m_confirm = buttons->button(QDialogButtonBox::Ok); m_confirm->setObjectName(QStringLiteral("Stage2AddressConfirm")); m_confirm->setText(QStringLiteral("确定")); m_confirm->setEnabled(false);
    buttons->button(QDialogButtonBox::Cancel)->setObjectName(QStringLiteral("Stage2AddressCancel")); buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    connect(previewButton, &QPushButton::clicked, this, [this]() { preview(); });
    connect(m_find, &QLineEdit::textChanged, this, [this]() { invalidatePreview(); });
    connect(m_replacement, &QLineEdit::textChanged, this, [this]() { invalidatePreview(); });
    connect(m_scope, &QComboBox::currentIndexChanged, this, [this]() { invalidatePreview(); });
    connect(m_dormant, &QCheckBox::toggled, this, [this]() { invalidatePreview(); });
    connect(buttons, &QDialogButtonBox::accepted, this, &AddressReplaceDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

/******************************************************************************
 * Function Name   : AddressReplaceDialog::invalidatePreview
 * Description     : Prevent confirmation after changing the query, scope or dormant-field choice.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void AddressReplaceDialog::invalidatePreview() //20261110-7 add
{
    m_hasPreview = false; m_hasAcceptedProject = false; m_candidate = AddressCandidate();
    m_confirm->setEnabled(false); m_table->setRowCount(0); m_status->setText(QStringLiteral("条件已改变，请重新预览。"));
}

/******************************************************************************
 * Function Name   : AddressReplaceDialog::preview
 * Description     : Show the complete candidate or one precise blocking location.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void AddressReplaceDialog::preview() //20261110-7 add
{
    AddressCandidate draft;
    AddressChange change;
    HmiProject checked;
    QString error;
    QStringList values;
    int index;
    int column;

    invalidatePreview();
    if (!previewAddressReplacement(m_project, m_find->text(), m_replacement->text(),
        dialogScope(m_project, m_scope, m_selectedObjectIds, m_dormant->isChecked()), &draft, &error) ||
        !applyCandidate(m_project, draft, &checked, &error)) { m_status->setText(error); return; }
    m_candidate = draft; m_hasPreview = true; m_table->setRowCount(draft.changes.size());
    for (index = 0; index < draft.changes.size(); ++index)
    {
        change = draft.changes[index]; values = {change.location.pageId, change.location.objectId, change.fieldPath, change.before, change.after};
        for (column = 0; column < values.size(); ++column) { m_table->setItem(index, column, new QTableWidgetItem(values[column])); }
    }
    m_status->setText(QStringLiteral("将替换%1个字段；范围外的休眠匹配%2项。确定后一次提交，可撤销。").arg(draft.changes.size()).arg(draft.excludedDormantMatches));
    m_confirm->setEnabled(true);
}

/******************************************************************************
 * Function Name   : AddressReplaceDialog::accept
 * Description     : Revalidate and expose one candidate while leaving the caller's project untouched.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void AddressReplaceDialog::accept() //20261110-7 add
{
    HmiProject draft;
    QString error;

    if (!m_hasPreview) { m_status->setText(QStringLiteral("请先生成有效预览。")); return; }
    if (!applyCandidate(m_project, m_candidate, &draft, &error)) { invalidatePreview(); m_status->setText(error); return; }
    m_acceptedProject = draft; m_hasAcceptedProject = true; QDialog::accept();
}

/******************************************************************************
 * Function Name   : AddressReplaceDialog::candidateProject
 * Description     : Keep canceled and merely previewed drafts inaccessible to the committing caller.
 * Input Parameters: output
 * Return Value    : True only after explicit acceptance
 * Date            : 2026.11.10
 ******************************************************************************/
bool AddressReplaceDialog::candidateProject(HmiProject *output) const //20261110-7 add
{
    if (output == nullptr || result() != QDialog::Accepted || !m_hasAcceptedProject) { return false; }
    *output = m_acceptedProject; return true;
}

/******************************************************************************
 * Function Name   : AddressReplaceDialog::candidate
 * Description     : Return the accepted transaction for callers that maintain project undo history.
 * Input Parameters: None
 * Return Value    : Accepted candidate or an empty transaction
 * Date            : 2026.11.10
 ******************************************************************************/
AddressCandidate AddressReplaceDialog::candidate() const //20261110-7 add
{
    return result() == QDialog::Accepted && m_hasAcceptedProject ? m_candidate : AddressCandidate();
}

/******************************************************************************
 * Function Name   : AddressReplaceDialog::setSelectedObjectIds
 * Description     : Supply selection identity without changing the original project or preview.
 * Input Parameters: ids
 * Return Value    : None
 * Date            : 2026.11.10
 ******************************************************************************/
void AddressReplaceDialog::setSelectedObjectIds(const QSet<QString> &ids) //20261110-7 add
{
    m_selectedObjectIds = ids; invalidatePreview();
}
}
