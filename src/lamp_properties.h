#ifndef LAMP_PROPERTIES_H
#define LAMP_PROPERTIES_H

#include "bit_button_properties.h"
#include <QVector>

//20261109-8 add: Keep read-only lamp monitoring separate from bit/word touch operations.
struct LampProperties
{
    int monitorMode = 1; // None, dual-state, multi-state.
    int multiDataType = 0; // Unsigned 16-bit or unsigned 32-bit.
    int multiStateCount = 2;
    bool multiMonitorSame = false;
    QString multiMonitorAddress = QStringLiteral("LW0");
    QVector<BitButtonState> extraStates; // State zero and one remain in shared BitButtonProperties.

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, LampProperties *properties, QString *error);
    bool validate(QString *error) const;
    int stateCount() const;
    int transferWordCount() const;
    void ensureStateCount(int count, const BitButtonProperties &shared);
    BitButtonState *state(BitButtonProperties &shared, int index);
    const BitButtonState *state(const BitButtonProperties &shared, int index) const;
    static QStringList dataTypes();
};

#endif
