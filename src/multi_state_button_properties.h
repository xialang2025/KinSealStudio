#ifndef MULTI_STATE_BUTTON_PROPERTIES_H
#define MULTI_STATE_BUTTON_PROPERTIES_H
#include "screen_button_properties.h"

//20261112-0 add: Multi-state input has an independent carrier/action model and owns every extra visual state.
struct MultiStateButtonProperties
{
    BitButtonProperties common;
    int dataType = 0;
    QString monitorAddress = QStringLiteral("LW0");
    QString writeAddress = QStringLiteral("LW0");
    bool sameAddress = false;
    int stateCount = 3;
    int action = 0;
    QVector<BitButtonState> extraStates;
    MultiStateButtonProperties();
    static QStringList actions();
    static BitButtonState freshExtraState();
    void setStateCount(int count);
    BitButtonState *state(int index);
    const BitButtonState *state(int index) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, MultiStateButtonProperties *output, QString *error);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};
#endif
