#ifndef BIT_BUTTON_PROPERTIES_H
#define BIT_BUTTON_PROPERTIES_H

#include "static_text_properties.h"
#include <QByteArray>

//20261108-15 add: Persist the two independent visual states and their source assets.
struct BitButtonState
{
    StaticTextProperties text;
    bool flash = false;
    bool sameLanguage = false;
    QString sharedText;
    bool useImage = true;
    QString picture;
    QByteArray picturePng;
    bool effect3d = false;
    bool transparent = false;
    QColor background = Qt::white;
    int opacity = 255;
    int imageMode = 0;

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, BitButtonState *state, QString *error);
};

bool validateButtonImage(const QByteArray &png);

struct BitButtonProperties
{
    int operation = 3;
    bool macroEnabled = false;
    QString macroName;
    bool indicator = true;
    bool dualMonitor = true;
    bool monitorSame = true;
    int monitorType = 0;
    QString monitorBitAddress = QStringLiteral("LB0");
    QString monitorWordAddress = QStringLiteral("LW0");
    int bitIndex = 15;
    BitButtonState states[2];
    StaticTextProperties visibility;
    StaticTextProperties touch;
    bool notice = false;
    bool noticeBit = false;
    QString noticeBitAddress = QStringLiteral("LB0");
    bool noticeBitValue = true;
    bool noticeWord = false;
    QString noticeWordAddress = QStringLiteral("LW0");
    int noticeWordType = 0;
    QString noticeWordValue = QStringLiteral("0");
    bool pressTimeEnabled = false;
    int pressTime = 255;

    BitButtonProperties();
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, BitButtonProperties *properties, QString *error,
                         bool allowZeroPressTime = false); //20261110-0 change
    bool validate(QString *error, bool allowZeroPressTime = false) const; //20261110-0 change
    static QStringList operations();
};

#endif
