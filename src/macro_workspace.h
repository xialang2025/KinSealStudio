#ifndef MACRO_WORKSPACE_H
#define MACRO_WORKSPACE_H

#include "project_model.h"
#include <QJsonArray>

//20261114-7 add: Ordinary-source transactions have no file, compiler, scheduler or hardware dependency.
namespace MacroWorkspace
{
constexpr int kMaximumMacros = 100;
constexpr int kMaximumSourceBytes = 1024 * 1024;
constexpr int kMaximumEncodedSourceBytes = kMaximumSourceBytes + 3;

enum class BomPolicy { Allow, Reject, Require };
enum class SourceKind { Ordinary, AddressVariables, UnverifiedScheduling };

struct Entry
{
    QString name;
    QString id;
    QString source;
    qsizetype utf8Bytes = 0;
    QStringList references;
};

//20261114-7 add: Names remain case-sensitive Unicode model keys, never inferred C function identifiers.
bool checkName(const QString &name, QString *error = nullptr);
bool checkSourceKind(SourceKind kind, QString *error = nullptr);
bool macroBindable(const HmiObject &object, QString *reason = nullptr);
QJsonArray bindingCapabilities();
QString defaultSource();
QString nextCopyName(const HmiProject &project, const QString &name);
bool list(const HmiProject &project, QList<Entry> *entries, QString *error = nullptr);

//20261114-7 add: Failed operations preserve the complete caller project; new/copy operations allocate distinct IDs.
bool create(HmiProject *project, const QString &name, const QString &source, QString *error = nullptr);
bool update(HmiProject *project, const QString &name, const QString &source, QString *error = nullptr);
bool copy(HmiProject *project, const QString &name, const QString &newName, QString *error = nullptr);
bool rename(HmiProject *project, const QString &name, const QString &newName, QString *error = nullptr);
bool remove(HmiProject *project, const QString &name, QString *error = nullptr);

//20261114-7 add: These functions handle UTF-8 ordinary .c bytes only; caller owns path checks and atomic file output.
bool decodeUtf8Source(const QByteArray &bytes, BomPolicy policy, QString *source, QString *error = nullptr);
bool encodeUtf8Source(const QString &source, bool emitBom, QByteArray *bytes, QString *error = nullptr);
bool importSource(HmiProject *project, const QString &name, const QByteArray &bytes, BomPolicy policy,
                  bool replaceExisting, QString *error = nullptr, SourceKind kind = SourceKind::Ordinary);
bool exportSource(const HmiProject &project, const QString &name, bool emitBom, QByteArray *bytes,
                  QString *error = nullptr, SourceKind kind = SourceKind::Ordinary);
}

#endif
