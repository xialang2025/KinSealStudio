#include <QDate>
#include <QTime>
#include <QtEndian>

/******************************************************************************
 * Function Name   : applyNativeSystemCompileTime
 * Description     : Rebuild the seven native local calendar and clock words in a V122 system segment.
 * Input Parameters: system, civilTime in yyyy-MM-ddTHH:mm:ss, error
 * Return Value    : True after all 28 bytes have been written to a validated segment.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool applyNativeSystemCompileTime(QByteArray *system, const QString &civilTime, QString *error) //20261128-10 add
{
    using u32 = quint32;
    const QDate date = QDate::fromString(civilTime.left(10), Qt::ISODate);
    const QTime time = QTime::fromString(civilTime.mid(11), QStringLiteral("HH:mm:ss"));
    const int words[] = {time.second(), time.minute(), time.hour(), date.dayOfWeek(),
                         date.day(), date.month(), date.year()};
    const QString canonical = date.toString(Qt::ISODate) + QLatin1Char('T') + time.toString(QStringLiteral("HH:mm:ss"));

    if (!system || system->size() != 4998 || !date.isValid() || !time.isValid() ||
        civilTime.size() != 19 || civilTime != canonical) {
        if (error) {
            *error = QStringLiteral("需要 4998 字节的原厂系统段及 yyyy-MM-ddTHH:mm:ss 格式的本地编译时间。");
        }
        return false;
    }
    // The original samples date and time separately and packs Monday as weekday 1.
    for (int index = 0; index < 7; ++index) {
        qToLittleEndian<u32>(static_cast<u32>(words[index]), system->data() + 324 + 4 * index);
    }
    return true;
}
