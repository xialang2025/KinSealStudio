#include <QtEndian>

/******************************************************************************
 * Function Name   : applyNativeSingleLampMonitorFields
 * Description     : Recover three original control words from the exact source packaged for this build.
 * Input Parameters: source, control
 * Return Value    : None. Unsupported sources keep their model-generated control words.
 * Date            : 2026.11.28
 ******************************************************************************/
void applyNativeSingleLampMonitorFields(const QByteArray &source, QByteArray *control) //20261128-16 add
{
    static const char *const paths[] = {"switch.248.1", "switch.248.4", "switch.274.0"};
    static const int offsets[] = {0x188, 0x18c, 0x194};
    SourceDocument document(source);
    QMap<QString, SourceField>::const_iterator field;
    QString prefix;
    u32 words[3];
    int index;

    if (!control || control->size() != 640 || control->at(0) != 10 ||
        qFromLittleEndian<u16>(control->constData() + 0x106) != 4 ||
        source.isEmpty() || source.startsWith('{') || !document.parse() ||
        document.objectPrefixes.size() != 1) {
        return;
    }
    prefix = document.objectPrefixes.first();
    if (document.value(prefix + QStringLiteral("type")).toInt() != 10 ||
        document.value(prefix + QStringLiteral("switch.mode")).toInt() != 4 ||
        document.value(prefix + QStringLiteral("switch.248.5")).toUInt() != qFromLittleEndian<u32>(control->constData() + 0x184) ||
        document.value(prefix + QStringLiteral("switch.248.3")).toUInt() != qFromLittleEndian<u32>(control->constData() + 0x190)) {
        return;
    }
    for (index = 0; index < 3; ++index) {
        field = document.values.constFind(prefix + QLatin1String(paths[index]));
        if (field == document.values.constEnd() || field->kind != 'i' || field->end - field->start != 4) {
            return;
        }
        // The native writer copies all four bytes even for dormant signed or out-of-range editor values.
        words[index] = qFromBigEndian<u32>(source.constData() + field->start);
    }
    for (index = 0; index < 3; ++index) {
        qToLittleEndian<u32>(words[index], control->data() + offsets[index]);
    }
}
