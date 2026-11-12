#ifndef FLOW_BLOCK_PROPERTIES_H
#define FLOW_BLOCK_PROPERTIES_H
#include "screen_button_properties.h"

//20261112-0 add: Flow fields are an independent editor record; no guessed native animation or parameter union is persisted.
struct FlowBlockProperties
{
    QString triggerAddress = QStringLiteral("LB0");
    QString flowFront = QStringLiteral("#ffffff");
    QString flowBack = QStringLiteral("#ff0000");
    QString pipeFront = QStringLiteral("#ffffff");
    QString pipeBack = QStringLiteral("#ffffff");
    QString borderColor = QStringLiteral("#000000");
    int pattern = 5;
    int direction = 0;
    int count = 3;
    int speed = 1;
    bool vertical = false;
    bool noBorder = true;
    bool visibilityBitChosen = false;
    StaticTextProperties visibility;
    static QStringList patterns();
    QStringList directions() const;
    void setVertical(bool value);
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, FlowBlockProperties *output, QString *error);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};
#endif
