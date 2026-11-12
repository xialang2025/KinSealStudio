#ifndef MOVE_TRACK_PROPERTIES_H
#define MOVE_TRACK_PROPERTIES_H
#include "screen_button_properties.h"
#include "image_display_properties.h"

//20261112-0 add: Independent editor carriers; no native source or firmware layout is inferred.
#define TRACK_BOOL_FIELDS(X) X(dynamicRange, false) X(notice, false) X(noticeBit, false) X(noticeWord, false) X(noticeBitValue, true) X(visibilityBitChosen, false) X(touchBitChosen, false)
#define TRACK_INT_FIELDS(X) X(direction, 3) X(distance, 100) X(noticeWordType, 0)
#define TRACK_STRING_FIELDS(X) X(address, "LW0") X(minimum, "0") X(maximum, "100") X(minimumAddress, "LW0") X(maximumAddress, "LW0") X(noticeBitAddress, "LB0") X(noticeWordAddress, "LW0") X(noticeWordValue, "0")

struct MoveTrackProperties
{
#define DECLARE_TRACK_BOOL(name, initial) bool name = initial;
    TRACK_BOOL_FIELDS(DECLARE_TRACK_BOOL)
#undef DECLARE_TRACK_BOOL
#define DECLARE_TRACK_INT(name, initial) int name = initial;
    TRACK_INT_FIELDS(DECLARE_TRACK_INT)
#undef DECLARE_TRACK_INT
#define DECLARE_TRACK_STRING(name, initial) QString name = QStringLiteral(initial);
    TRACK_STRING_FIELDS(DECLARE_TRACK_STRING)
#undef DECLARE_TRACK_STRING
    ImageDisplayState appearance;
    StaticTextProperties visibility;
    StaticTextProperties touch;
    MoveTrackProperties();
    static QStringList directions();
    BitButtonProperties accessDraft() const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, MoveTrackProperties *output, QString *error);
    bool validate(QString *error = nullptr) const;
    bool validateAddresses(const QVector<PlcConnection> &connections, QString *error = nullptr) const;
};
#endif
