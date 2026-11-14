#include "macro_workspace.h"

#include <QSet>
#include <QStringDecoder>
#include <QUuid>

namespace
{
/******************************************************************************
 * Function Name   : workspaceError
 * Description     : Return a recoverable transaction error without changing caller data.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.14
 ******************************************************************************/
bool workspaceError(QString *error, const QString &message) //20261114-7 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : checkSource
 * Description     : Bound UTF-8 content and reject lossy Unicode or NUL before source mutation.
 * Input Parameters: source, error
 * Return Value    : True for plain ordinary source text; C compilation is a separate operation.
 * Date            : 2026.11.14
 ******************************************************************************/
bool checkSource(const QString &source, QString *error) //20261114-7 add
{
    QByteArray encoded;

    if (source.size() > MacroWorkspace::kMaximumSourceBytes || source.contains(QChar(0)) ||
        source.startsWith(QChar(0xfeff)))
    { return workspaceError(error, QStringLiteral("普通源码含NUL/内嵌起始BOM或超过1MiB保护；BOM应由编码选项指定。")); }
    encoded = source.toUtf8();
    if (encoded.size() > MacroWorkspace::kMaximumSourceBytes || QString::fromUtf8(encoded) != source)
    { return workspaceError(error, QStringLiteral("普通源码超过1MiB UTF-8内容或含无法无损编码的Unicode。")); }
    return true;
}

/******************************************************************************
 * Function Name   : checkProject
 * Description     : Validate the complete input model without reducing legacy projects to the new-source count limit.
 * Input Parameters: project, error
 * Return Value    : True for a bounded existing project.
 * Date            : 2026.11.14
 ******************************************************************************/
bool checkProject(const HmiProject *project, QString *error) //20261114-7 add
{
    if (project == nullptr) { return workspaceError(error, QStringLiteral("宏工作区工程接收位置为空。")); }
    return project->validateModel(error);
}

/******************************************************************************
 * Function Name   : ensureIds
 * Description     : Add identities only where a legacy source has none, retaining every existing stable ID.
 * Input Parameters: project
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void ensureIds(HmiProject *project) //20261114-7 add
{
    QMap<QString, QString>::const_iterator macro;
    QSet<QString> used;
    QString id;

    used = QSet<QString>(project->macroIds.constBegin(), project->macroIds.constEnd());
    for (macro = project->macros.constBegin(); macro != project->macros.constEnd(); ++macro)
    {
        if (project->macroIds.contains(macro.key())) { continue; }
        do { id = QUuid::createUuid().toString(QUuid::WithoutBraces); } while (used.contains(id));
        project->macroIds.insert(macro.key(), id);
        used.insert(id);
    }
}

/******************************************************************************
 * Function Name   : objectMacroFields
 * Description     : Inspect every serialized macro-name carrier, including disabled drafts with no runtime binding.
 * Input Parameters: object
 * Return Value    : Field-name to stored macro-name pairs.
 * Date            : 2026.11.14
 ******************************************************************************/
QMap<QString, QString> objectMacroFields(const HmiObject &object) //20261114-7 add
{
    QMap<QString, QString> fields;

    if (object.hasBitButtonProperties) { fields.insert(QStringLiteral("bitButton.macroName"), object.bitButton.macroName); }
    if (object.type == QStringLiteral("slider")) { fields.insert(QStringLiteral("slider.macroName"), object.slider.macroName); }
    if (object.type == QStringLiteral("dropdown")) { fields.insert(QStringLiteral("dropdown.macroName"), object.dropdown.macroName); }
    if (object.type == QStringLiteral("screen")) { fields.insert(QStringLiteral("screenButton.common.macroName"), object.screenButton.common.macroName); }
    if (object.type == QStringLiteral("multistate")) { fields.insert(QStringLiteral("multiStateButton.common.macroName"), object.multiStateButton.common.macroName); }
    if (object.type == QStringLiteral("multifunction")) { fields.insert(QStringLiteral("multiFunctionButton.common.macroName"), object.multiFunctionButton.common.macroName); }
    if (object.hasNumberDisplayProperties) { fields.insert(QStringLiteral("numberDisplay.macroName"), object.numberDisplay.macroName); }
    if (object.type == QStringLiteral("ascii")) { fields.insert(QStringLiteral("asciiDisplay.common.macroName"), object.asciiDisplay.common.macroName); }
    return fields;
}

/******************************************************************************
 * Function Name   : sourceReferences
 * Description     : Find all-page active/dormant names and generic stable-ID inbound references before deletion.
 * Input Parameters: project, name
 * Return Value    : Human-readable dependency locations, with no guessed configuration or opaque-byte interpretation.
 * Date            : 2026.11.14
 ******************************************************************************/
QStringList sourceReferences(const HmiProject &project, const QString &name) //20261114-7 add
{
    QList<HmiPage> pages;
    HmiPage page;
    HmiObject object;
    HmiReference reference;
    QMap<QString, QString> fields;
    QMap<QString, QString>::const_iterator field;
    QStringList result;
    QString stableId;

    pages = project.allPages();
    stableId = project.macroIds.value(name);
    Q_FOREACH (page, pages)
    {
        Q_FOREACH (object, page.objects)
        {
            fields = objectMacroFields(object);
            for (field = fields.constBegin(); field != fields.constEnd(); ++field)
            {
                if (field.value() == name)
                { result.append(QStringLiteral("page:%1/object:%2/%3").arg(page.id, object.id, field.key())); }
            }
        }
    }
    Q_FOREACH (reference, project.references)
    {
        if (reference.targetKind == QStringLiteral("macro") && !stableId.isEmpty() && reference.targetId == stableId)
        { result.append(QStringLiteral("reference:%1/%2:%3/%4").arg(reference.id, reference.ownerKind, reference.ownerId, reference.field)); }
    }
    return result;
}

/******************************************************************************
 * Function Name   : renameDormantFields
 * Description     : Complete the existing model rename for serialized carriers whose native macro UI is disabled.
 * Input Parameters: object, oldName, newName
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
void renameDormantFields(HmiObject *object, const QString &oldName, const QString &newName) //20261114-7 add
{
    if (object->type == QStringLiteral("screen") && object->screenButton.common.macroName == oldName) { object->screenButton.common.macroName = newName; }
    if (object->type == QStringLiteral("multistate") && object->multiStateButton.common.macroName == oldName) { object->multiStateButton.common.macroName = newName; }
    if (object->type == QStringLiteral("multifunction") && object->multiFunctionButton.common.macroName == oldName) { object->multiFunctionButton.common.macroName = newName; }
    if (object->hasNumberDisplayProperties && object->numberDisplay.macroName == oldName) { object->numberDisplay.macroName = newName; }
    if (object->type == QStringLiteral("ascii") && object->asciiDisplay.common.macroName == oldName) { object->asciiDisplay.common.macroName = newName; }
}
}

/******************************************************************************
 * Function Name   : MacroWorkspace::checkName
 * Description     : Validate a display key without asserting that the key is a C entry-point identifier.
 * Input Parameters: name, error
 * Return Value    : True for an exact, bounded, case-sensitive Unicode key.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroWorkspace::checkName(const QString &name, QString *error) //20261114-7 add
{
    QChar character;

    if (name.isEmpty() || name != name.trimmed() || name.size() > 256 || QString::fromUtf8(name.toUtf8()) != name)
    { return workspaceError(error, QStringLiteral("宏名称须为1..256字符且首尾无空白的有效Unicode；区分大小写。")); }
    Q_FOREACH (character, name)
    {
        if (character.category() == QChar::Other_Control)
        { return workspaceError(error, QStringLiteral("宏名称不能包含控制字符。")); }
    }
    return true;
}

/******************************************************************************
 * Function Name   : MacroWorkspace::checkSourceKind
 * Description     : Keep unknown address-variable and scheduling formats outside ordinary-source import/export.
 * Input Parameters: kind, error
 * Return Value    : True only for ordinary source.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroWorkspace::checkSourceKind(SourceKind kind, QString *error) //20261114-7 add
{
    if (kind != SourceKind::Ordinary)
    { return workspaceError(error, QStringLiteral("仅支持普通宏.c源码；地址变量与未恢复调度格式不能导入、导出或猜测生成。")); }
    return true;
}

/******************************************************************************
 * Function Name   : MacroWorkspace::macroBindable
 * Description     : Gate macro UI by the existing independent property carrier instead of shared artwork.
 * Input Parameters: object, reason
 * Return Value    : True for the four currently supported binding families.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroWorkspace::macroBindable(const HmiObject &object, QString *reason) //20261114-7 add
{
    bool bindable;

    bindable = ((object.type == QStringLiteral("switch") && object.hasBitButtonProperties &&
                 !object.hasLampProperties && !object.hasFunctionButtonProperties) ||
                ((object.type == QStringLiteral("slider") || object.type == QStringLiteral("dropdown")) &&
                 !object.hasBitButtonProperties && !object.hasWordButtonProperties &&
                 !object.hasLampProperties && !object.hasFunctionButtonProperties));
    if (reason != nullptr)
    { *reason = bindable ? QString() : QStringLiteral("该控件当前不支持活动宏绑定；仅位/字按钮、滑动开关与下拉框开放，保留的禁用宏名不代表可绑定。"); }
    return bindable;
}

/******************************************************************************
 * Function Name   : MacroWorkspace::bindingCapabilities
 * Description     : Expose the same finite UI-binding matrix without claiming compiler, scheduler or device support.
 * Input Parameters: None
 * Return Value    : JSON rows with independent model carrier names and active-binding availability.
 * Date            : 2026.11.14
 ******************************************************************************/
QJsonArray MacroWorkspace::bindingCapabilities() //20261114-7 add
{
    QJsonArray result;
    QStringList types;
    QStringList fields;
    QStringList labels;
    int index;

    types = {QStringLiteral("switch:bit"), QStringLiteral("switch:word"), QStringLiteral("slider"), QStringLiteral("dropdown"),
             QStringLiteral("switch:lamp"), QStringLiteral("switch:function"), QStringLiteral("screen"), QStringLiteral("multistate"),
             QStringLiteral("multifunction"), QStringLiteral("number"), QStringLiteral("ascii"), QStringLiteral("*")};
    fields = {QStringLiteral("bitButton.macroName"), QStringLiteral("bitButton.macroName"), QStringLiteral("slider.macroName"), QStringLiteral("dropdown.macroName"),
              QStringLiteral("bitButton.macroName"), QStringLiteral("bitButton.macroName"), QStringLiteral("screenButton.common.macroName"), QStringLiteral("multiStateButton.common.macroName"),
              QStringLiteral("multiFunctionButton.common.macroName"), QStringLiteral("numberDisplay.macroName"), QStringLiteral("asciiDisplay.common.macroName"), QString()};
    labels = {QStringLiteral("位按钮"), QStringLiteral("字按钮"), QStringLiteral("滑动开关"), QStringLiteral("下拉框"), QStringLiteral("指示灯"), QStringLiteral("功能按钮"),
              QStringLiteral("画面按钮"), QStringLiteral("多态按钮"), QStringLiteral("多功能按钮"), QStringLiteral("数值控件"), QStringLiteral("ASCII控件"), QStringLiteral("其它控件")};
    for (index = 0; index < types.size(); ++index)
    {
        result.append(QJsonObject{{QStringLiteral("type"), types.at(index)}, {QStringLiteral("label"), labels.at(index)},
                      {QStringLiteral("field"), fields.at(index)}, {QStringLiteral("bindable"), index < 4},
                      {QStringLiteral("sourceOnly"), true}, {QStringLiteral("hardwareExecutionVerified"), false}});
    }
    return result;
}

/******************************************************************************
 * Function Name   : MacroWorkspace::defaultSource
 * Description     : Supply a deliberately empty ordinary-source draft, without inventing address or scheduling APIs.
 * Input Parameters: None
 * Return Value    : Lite template using the existing ordinary Macro_Entry source convention.
 * Date            : 2026.11.14
 ******************************************************************************/
QString MacroWorkspace::defaultSource() //20261114-7 add
{
    return QStringLiteral("#include \"macro.h\"\n\nvoid Macro_Entry(void)\n{\n    /* Add ordinary macro source here. */\n}\n");
}

/******************************************************************************
 * Function Name   : MacroWorkspace::nextCopyName
 * Description     : Suggest the first free case-sensitive _copy/_copyN key as an explicit Lite naming rule.
 * Input Parameters: project, name
 * Return Value    : Bounded unused key or empty when the source does not exist.
 * Date            : 2026.11.14
 ******************************************************************************/
QString MacroWorkspace::nextCopyName(const HmiProject &project, const QString &name) //20261114-7 add
{
    QString stem;
    QString suffix;
    QString candidate;
    int index;

    if (!project.macros.contains(name)) { return QString(); }
    for (index = 1; index <= 129; ++index)
    {
        suffix = index == 1 ? QStringLiteral("_copy") : QStringLiteral("_copy%1").arg(index);
        stem = name.left(256 - suffix.size());
        if (!stem.isEmpty() && stem.back().isHighSurrogate()) { stem.chop(1); }
        candidate = stem + suffix;
        if (!project.macros.contains(candidate) && checkName(candidate)) { return candidate; }
    }
    return QString();
}

/******************************************************************************
 * Function Name   : MacroWorkspace::list
 * Description     : Inspect sources and dependencies without migrating IDs or changing the editable project.
 * Input Parameters: project, entries, error
 * Return Value    : True after publishing the complete validated list.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroWorkspace::list(const HmiProject &project, QList<Entry> *entries, QString *error) //20261114-7 add
{
    QList<Entry> result;
    Entry entry;
    QMap<QString, QString>::const_iterator macro;

    if (entries == nullptr) { return workspaceError(error, QStringLiteral("宏清单接收位置为空。")); }
    if (!checkProject(&project, error)) { return false; }
    for (macro = project.macros.constBegin(); macro != project.macros.constEnd(); ++macro)
    {
        entry.name = macro.key(); entry.id = project.macroIds.value(macro.key()); entry.source = macro.value();
        entry.utf8Bytes = macro.value().toUtf8().size(); entry.references = sourceReferences(project, macro.key());
        result.append(entry);
    }
    *entries = result;
    return true;
}

/******************************************************************************
 * Function Name   : MacroWorkspace::create
 * Description     : Add a bounded ordinary source and fresh identity in one complete-model transaction.
 * Input Parameters: project, name, source, error
 * Return Value    : True on commit; existing projects above 100 remain editable but cannot add a source.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroWorkspace::create(HmiProject *project, const QString &name, const QString &source, QString *error) //20261114-7 add
{
    HmiProject candidate;

    if (!checkProject(project, error) || !checkName(name, error) || !checkSource(source, error)) { return false; }
    if (project->macros.contains(name) || project->macros.size() >= kMaximumMacros)
    { return workspaceError(error, QStringLiteral("宏名称冲突或新增将超过100个普通宏；已有101..128项仍可更新/删除。")); }
    candidate = *project; candidate.macros.insert(name, source); ensureIds(&candidate);
    if (!candidate.validateModel(error)) { return false; }
    *project = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : MacroWorkspace::update
 * Description     : Replace only one source while preserving names, identities and every model dependency.
 * Input Parameters: project, name, source, error
 * Return Value    : True after the source and aggregate model budgets pass.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroWorkspace::update(HmiProject *project, const QString &name, const QString &source, QString *error) //20261114-7 add
{
    HmiProject candidate;

    if (!checkProject(project, error) || !checkSource(source, error)) { return false; }
    if (!project->macros.contains(name)) { return workspaceError(error, QStringLiteral("待更新宏不存在。")); }
    candidate = *project; candidate.macros[name] = source; ensureIds(&candidate);
    if (!candidate.validateModel(error)) { return false; }
    *project = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : MacroWorkspace::copy
 * Description     : Copy source bytes while allocating an independent stable identity and leaving bindings untouched.
 * Input Parameters: project, name, newName, error
 * Return Value    : True after the normal creation checks commit.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroWorkspace::copy(HmiProject *project, const QString &name, const QString &newName, QString *error) //20261114-7 add
{
    QString source;

    if (!checkProject(project, error)) { return false; }
    if (!project->macros.contains(name)) { return workspaceError(error, QStringLiteral("待复制宏不存在。")); }
    source = project->macros.value(name);
    return create(project, newName, source, error);
}

/******************************************************************************
 * Function Name   : MacroWorkspace::rename
 * Description     : Reuse stable-ID model rename, then repair every serialized dormant carrier across all pages.
 * Input Parameters: project, name, newName, error
 * Return Value    : True after all live/dormant references validate; source text is never rewritten.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroWorkspace::rename(HmiProject *project, const QString &name, const QString &newName, QString *error) //20261114-7 add
{
    HmiProject candidate;
    int pageIndex;
    int objectIndex;

    if (!checkProject(project, error) || !checkName(newName, error)) { return false; }
    candidate = *project;
    if (!candidate.renameMacro(name, newName, error)) { return false; }
    for (objectIndex = 0; objectIndex < candidate.objects.size(); ++objectIndex)
    { renameDormantFields(&candidate.objects[objectIndex], name, newName); }
    for (pageIndex = 0; pageIndex < candidate.inactivePages.size(); ++pageIndex)
    {
        for (objectIndex = 0; objectIndex < candidate.inactivePages.at(pageIndex).objects.size(); ++objectIndex)
        { renameDormantFields(&candidate.inactivePages[pageIndex].objects[objectIndex], name, newName); }
    }
    if (!candidate.validateModel(error)) { return false; }
    *project = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : MacroWorkspace::remove
 * Description     : Refuse all live/dormant or stable references and unknown opaque consumers before deleting a source.
 * Input Parameters: project, name, error
 * Return Value    : True after an unreferenced removal commits atomically.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroWorkspace::remove(HmiProject *project, const QString &name, QString *error) //20261114-7 add
{
    HmiProject candidate;
    QStringList references;

    if (!checkProject(project, error)) { return false; }
    if (!project->macros.contains(name)) { return workspaceError(error, QStringLiteral("待删除宏不存在。")); }
    references = sourceReferences(*project, name);
    if (!references.isEmpty())
    { return workspaceError(error, QStringLiteral("宏仍被活动或禁用草稿/稳定引用使用，不能删除：%1。").arg(references.first())); }
    if (!project->nativeSourceSnapshot.isEmpty())
    { return workspaceError(error, QStringLiteral("原厂不透明快照的隐藏宏依赖未完全恢复，暂不删除该工程的宏；可保留源码并更新。")); }
    candidate = *project; candidate.macros.remove(name); candidate.macroIds.remove(name);
    if (!candidate.validateModel(error)) { return false; }
    *project = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : MacroWorkspace::decodeUtf8Source
 * Description     : Decode strict UTF-8 with an explicit optional/required/rejected single BOM and no newline normalization.
 * Input Parameters: bytes, policy, source, error
 * Return Value    : True without replacing the destination on malformed or oversized input.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroWorkspace::decodeUtf8Source(const QByteArray &bytes, BomPolicy policy, QString *source, QString *error) //20261114-7 add
{
    QByteArray content;
    QString decoded;
    QStringDecoder decoder(QStringDecoder::Utf8, QStringConverter::Flag::Stateless);
    bool hasBom;

    if (source == nullptr) { return workspaceError(error, QStringLiteral("源码接收位置为空。")); }
    if (policy != BomPolicy::Allow && policy != BomPolicy::Reject && policy != BomPolicy::Require)
    { return workspaceError(error, QStringLiteral("UTF-8 BOM策略无效。")); }
    if (bytes.size() > kMaximumEncodedSourceBytes) { return workspaceError(error, QStringLiteral("导入文件超过1MiB内容加BOM保护。")); }
    hasBom = bytes.startsWith(QByteArray::fromHex("efbbbf"));
    if ((hasBom && policy == BomPolicy::Reject) || (!hasBom && policy == BomPolicy::Require))
    { return workspaceError(error, QStringLiteral("导入UTF-8 BOM与所选策略不符。")); }
    content = hasBom ? bytes.mid(3) : bytes;
    if (content.startsWith(QByteArray::fromHex("efbbbf"))) { return workspaceError(error, QStringLiteral("导入源码含重复起始BOM。")); }
    decoded = decoder(content);
    if (decoder.hasError()) { return workspaceError(error, QStringLiteral("导入源码不是严格UTF-8；不会用替换字符修复。")); }
    if (!checkSource(decoded, error)) { return false; }
    *source = decoded;
    return true;
}

/******************************************************************************
 * Function Name   : MacroWorkspace::encodeUtf8Source
 * Description     : Encode plain UTF-8 source with an explicit BOM choice and no file-format masquerading.
 * Input Parameters: source, emitBom, bytes, error
 * Return Value    : True after publishing bounded bytes; file writing remains UI-owned.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroWorkspace::encodeUtf8Source(const QString &source, bool emitBom, QByteArray *bytes, QString *error) //20261114-7 add
{
    QByteArray encoded;

    if (bytes == nullptr) { return workspaceError(error, QStringLiteral("UTF-8字节接收位置为空。")); }
    if (!checkSource(source, error)) { return false; }
    encoded = source.toUtf8();
    if (emitBom) { encoded.prepend(QByteArray::fromHex("efbbbf")); }
    *bytes = encoded;
    return true;
}

/******************************************************************************
 * Function Name   : MacroWorkspace::importSource
 * Description     : Decode one ordinary .c payload, then add or explicitly replace it using the same source transaction.
 * Input Parameters: project, name, bytes, policy, replaceExisting, error, kind
 * Return Value    : True on a complete model commit; no files are read or written.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroWorkspace::importSource(HmiProject *project, const QString &name, const QByteArray &bytes, BomPolicy policy,
                                 bool replaceExisting, QString *error, SourceKind kind) //20261114-7 add
{
    QString source;

    if (!checkSourceKind(kind, error) || !checkProject(project, error) || !checkName(name, error) ||
        !decodeUtf8Source(bytes, policy, &source, error)) { return false; }
    if (project->macros.contains(name))
    {
        if (!replaceExisting) { return workspaceError(error, QStringLiteral("导入宏已存在；未明确选择替换，保持原工程。")); }
        return update(project, name, source, error);
    }
    return create(project, name, source, error);
}

/******************************************************************************
 * Function Name   : MacroWorkspace::exportSource
 * Description     : Produce only a validated ordinary .c candidate; unknown schedule/address-variable formats stay refused.
 * Input Parameters: project, name, emitBom, bytes, error, kind
 * Return Value    : True with strict UTF-8 bytes; callers must check .c paths and atomically save.
 * Date            : 2026.11.14
 ******************************************************************************/
bool MacroWorkspace::exportSource(const HmiProject &project, const QString &name, bool emitBom, QByteArray *bytes,
                                 QString *error, SourceKind kind) //20261114-7 add
{
    if (!checkSourceKind(kind, error) || !checkProject(&project, error)) { return false; }
    if (!project.macros.contains(name)) { return workspaceError(error, QStringLiteral("待导出宏不存在。")); }
    return encodeUtf8Source(project.macros.value(name), emitBom, bytes, error);
}
