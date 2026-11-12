#ifndef MESSAGE_DISPLAY_PROPERTIES_H
#define MESSAGE_DISPLAY_PROPERTIES_H

#include "static_text_properties.h"
#include "plc_address.h"
#include <QVector>

//20261112-0 add: Keep message states and dormant editing drafts separate from the selected preview state.
struct MessageDisplayProperties
{
    int stateCount = 2;
    int dataType = 0;
    int movement = 0;
    int speed = 5;
    QString monitorAddress = QStringLiteral("LW0");
    StaticTextProperties appearance;
    QColor frameColor = QColor(224, 224, 224);
    QMap<QString, StaticTextProperties> languageAppearances;
    QMap<QString, QColor> languageFrameColors;
    QVector<QMap<QString, QString>> states;

    MessageDisplayProperties();
    void ensureStates(int count);
    QString text(int state, const QString &language) const;
    StaticTextProperties languageAppearance(const QString &language) const;
    QColor languageFrameColor(const QString &language) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, MessageDisplayProperties *output, QString *error);
    bool validate(QString *error) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error) const;
};

#endif
