#ifndef STAGE2_FILE_PROTECTION_H
#define STAGE2_FILE_PROTECTION_H

#include <QByteArray>
#include <QString>

class QWidget;

inline constexpr qsizetype stage2FilePasswordLimit = 11; //20261123-17 add
inline constexpr qsizetype stage2ProtectedFileOverhead = 55;

bool isStage2ProtectedFile(const QByteArray &bytes);
bool protectStage2ProjectFile(const QByteArray &plain, const QString &password,
                              QByteArray *protectedBytes, QString *error);
bool unprotectStage2ProjectFile(const QByteArray &protectedBytes, const QString &password,
                                QByteArray *plain, bool *needsPassword, QString *error);
bool editStage2FileProtection(QWidget *parent, const QString &initialPassword, QString *result);
bool requestStage2FilePassword(QWidget *parent, QString *password);

#endif
